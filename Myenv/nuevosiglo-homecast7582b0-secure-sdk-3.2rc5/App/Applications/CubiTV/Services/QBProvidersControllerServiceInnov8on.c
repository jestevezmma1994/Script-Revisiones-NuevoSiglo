/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
 **
 ** Any rights which are not expressly granted in this License are entirely and
 ** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
 ** modify, translate, reverse engineer, decompile, disassemble, or create
 ** derivative works based on this Software. You may not make access to this
 ** Software available to others in connection with a service bureau,
 ** application service provider, or similar business, or make any other use of
 ** this Software without express written permission from Cubiware Sp. z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#include "QBProvidersControllerServiceInnov8on.h"
#include "QBProvidersControllerService.h"
#include <stdlib.h>
#include <main.h>
#include <settings.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Services/core/QBContentManagers.h>
#include <CubiwareMWClient/Monitor/QBMWServiceMonitor.h>
#include <QBContentManager/Innov8onContentManager.h>
#include <Menus/Innov8on/innov8onCarouselItemController.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <Menus/SocialMedia/socialMediaCarouselItemController.h>
#include <Utils/value.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBConf.h>
#include <QBMenu/QBMenu.h>
#include <Menus/QBMWMenuParams.h>
#include <SvCore/SvEnv.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_log, 0, "QBProvidersControllerServiceInnov8onLogLevel", "0:error, 1:debug");

#define log_debug(fmt, ...) do { if (env_log() >= 1) SvLogNotice("[%s] " fmt, __func__, ## __VA_ARGS__); } while (0)
#define log_error(fmt, ...) do { if (env_log() >= 0) SvLogError(COLBEG() "[%s] " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)

struct QBProvidersControllerServiceInnov8on_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    Innov8onContentManager innov8onContentManager;
    QBMWServiceMonitor serviceMonitor;

    SvHashTable menuTreeInactiveNodes; // Table with QBActiveTreeNodes, inactive menu positions to be turned on (moved to menuTree) when content available
    SvHashTable menuTreeOriginalIdxs;  // Table for storing the order of inactive menu positions, to be put correctly into menu

    SvHashTable innov8onServices;      // service ID -> service tag
    SvHashTable innov8onMenuNodes;     // service tag -> menu node ID
    SvHashTable innov8onMounts;        // menu node ID -> array of QBCarouselMenuItemService

    SvWeakList serviceListeners;

    QBInnov8onCarouselMenuItemController innov8onMenuController;

    SvHashTable submenuFactories;
};
typedef struct QBProvidersControllerServiceInnov8on_t *QBProvidersControllerServiceInnov8on;

typedef enum {
    QBProvidersControllerServiceNotificationType_ServiceAdded = 0,
    QBProvidersControllerServiceNotificationType_ServiceRemoved,
    QBProvidersControllerServiceNotificationType_ServiceModified
} QBProvidersControllerServiceNotificationType;

typedef enum {
    QBProvidersControllerServiceMonitorNotificationType_MonitorCreated = 0,
    QBProvidersControllerServiceMonitorNotificationType_MonitorDestroyed
} QBProvidersControllerServiceMonitorNotificationType;

SvLocal void
unset_skin_controller(QBProvidersControllerServiceInnov8on self, SvGenericObject path)
{
    QBTreePathMap skinPathMap = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getSkinPathMap);
    SvErrorInfo error = NULL;
    QBTreePathMapRemove(skinPathMap, path, &error);
    if (error) {
        SvErrorInfoDestroy(error);
    }
}

SvLocal void
set_controller(QBProvidersControllerServiceInnov8on self, SvGenericObject path, SvGenericObject controller)
{
    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, controller, NULL);
}

SvLocal void
unset_controller(QBProvidersControllerServiceInnov8on self, SvGenericObject path)
{
    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvErrorInfo error = NULL;
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, NULL, &error);
    if (error) {
        SvErrorInfoDestroy(error);
    }
}

SvLocal void
QBProvidersControllerServiceInnov8onAddListener(SvObject self_, SvGenericObject listener)
{
    QBProvidersControllerServiceInnov8on self = (QBProvidersControllerServiceInnov8on) self_;
    if (SvObjectIsImplementationOf(listener, QBProvidersControllerServiceListener_getInterface()))
        SvWeakListPushFront(self->serviceListeners, listener, NULL);
}

SvLocal void
QBProvidersControllerServiceInnov8onRemoveListener(SvObject self_, SvGenericObject listener)
{
    QBProvidersControllerServiceInnov8on self = (QBProvidersControllerServiceInnov8on) self_;
    SvWeakListRemoveObject(self->serviceListeners, listener);
}

SvLocal void
QBProvidersControllerServiceInnov8onNotifyListeners(QBProvidersControllerServiceInnov8on self, SvString serviceId,
                                                    QBProvidersControllerServiceNotificationType notificationType)
{
    SvIterator iterator = SvWeakListIterator(self->serviceListeners);
    SvGenericObject listener;
    if (notificationType == QBProvidersControllerServiceNotificationType_ServiceAdded) {
        while((listener = SvIteratorGetNext(&iterator)))
            SvInvokeInterface(QBProvidersControllerServiceListener, listener, serviceAdded, serviceId);
    } else if (notificationType == QBProvidersControllerServiceNotificationType_ServiceRemoved) {
        while((listener = SvIteratorGetNext(&iterator)))
            SvInvokeInterface(QBProvidersControllerServiceListener, listener, serviceRemoved, serviceId);
    } else if (notificationType == QBProvidersControllerServiceNotificationType_ServiceModified) {
        while((listener = SvIteratorGetNext(&iterator)))
            SvInvokeInterface(QBProvidersControllerServiceListener, listener, serviceModified, serviceId);
    }
}

SvLocal void
QBProvidersControllerServiceInnov8onRemoveIfInactiveNodeFromMenu(QBProvidersControllerServiceInnov8on self, SvString serviceTag, bool removingOnlyThisNode)
{
    // Removes one node from menuTree and puts it to temp hashTable
    bool nodeHasServices;
    SvGenericObject nodeID = SvHashTableFind(self->innov8onMenuNodes, (SvGenericObject) serviceTag);
    if (!nodeID)
        return;
    SvArray mounts = (SvArray) SvHashTableFind(self->innov8onMounts, nodeID);

    if (!mounts) {
        nodeHasServices = false;
    } else {
        if (SvArrayCount(mounts) <= 0)
            nodeHasServices = false;
        else
            nodeHasServices = true;
    }

    QBActiveTreeNode node = QBActiveTreeFindNode(self->appGlobals->menuTree, (SvString) nodeID);
    if (!node)
        return;
    SvGenericObject displayMode = QBActiveTreeNodeGetAttribute(node, SVSTRING("displayMode"));

    if (!nodeHasServices && displayMode) {
        if (SvStringEqualToCString( (SvString) displayMode, "hideAlways") || SvStringEqualToCString( (SvString) displayMode, "hideByDefault")) {
            SvHashTableInsert(self->menuTreeInactiveNodes, (SvGenericObject) nodeID, (SvGenericObject) node);
            QBActiveTreeRemoveSubTree(self->appGlobals->menuTree, node, NULL);
        }
    }
}

SvLocal void
QBProvidersControllerServiceInnov8onRemoveIfInactiveNodesFromMenu(QBProvidersControllerServiceInnov8on self, SvString nodeID)
{
    // Checks if menu nodes have any services connected to them. If not delete them from menu tree and store in hashTable.
    if (!self)
        return;

    if (!nodeID) {  // Check all nodes and remove inactive nodes
        SvIterator iter = SvHashTableValuesIterator(self->innov8onMenuNodes);
        SvString iterNodeID;
        while ((iterNodeID = (SvString) SvIteratorGetNext(&iter))) {
            QBProvidersControllerServiceInnov8onRemoveIfInactiveNodeFromMenu(self, iterNodeID, false);
        }
    } else {    // Check specific node and remove if inactive
        QBProvidersControllerServiceInnov8onRemoveIfInactiveNodeFromMenu(self, nodeID, true);
    }
}

SvLocal void
QBProvidersControllerServiceInnov8onAddInactiveNodeToMenu(QBProvidersControllerServiceInnov8on self, SvString nodeID)
{
    if (!self || !nodeID)
        return;

    QBActiveTreeNode currentNode = NULL;
    size_t idx, idxOrig;
    currentNode = (QBActiveTreeNode) SvHashTableFind(self->menuTreeInactiveNodes, (SvGenericObject) nodeID);
    if (!currentNode)
        return;
    SvGenericObject displayMode = QBActiveTreeNodeGetAttribute(currentNode, SVSTRING("displayMode"));

    if (!(displayMode && (SvStringEqualToCString( (SvString) displayMode, "hideAlways")))) {
        SvValue idxValue = (SvValue) SvHashTableFind(self->menuTreeOriginalIdxs, (SvGenericObject) nodeID);
        if (idxValue) {
            idx = SvValueGetInteger(idxValue);
            idxOrig = idx;

            // Update position. Check all inactive nodes. Some of them may have been before our node originally. If so decrease our idx.
            SvIterator iter = SvHashTableKeysIterator(self->menuTreeInactiveNodes);
            SvString nodeTmp;
            SvValue idxTmp;
            while ((nodeTmp = (SvString) SvIteratorGetNext(&iter))) {
                if (nodeTmp != nodeID) {
                    idxTmp = (SvValue) SvHashTableFind(self->menuTreeOriginalIdxs, (SvGenericObject) nodeTmp);
                    if ((unsigned) SvValueGetInteger(idxTmp) < idxOrig) {
                        idx--;
                    }
                }
            }
            QBActiveTreeInsertSubTree(self->appGlobals->menuTree, QBActiveTreeGetRootNode(self->appGlobals->menuTree), currentNode, idx, NULL);
            SvHashTableRemove(self->menuTreeInactiveNodes, (SvGenericObject) nodeID);
        }
    }
}

SvLocal void
QBProvidersControllerServiceInnov8onGetOriginalTreeOrder(QBProvidersControllerServiceInnov8on self)
{
    // Saves original positions of menu nodes in hashTable
    if (!self)
        return;

    SvGenericObject path = NULL;
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, path, 0);
    QBActiveTreeNode node = NULL;
    SvString currentNodeID;
    while ((node = (QBActiveTreeNode) QBTreeIteratorGetNextNode(&iter))) {
        currentNodeID = QBActiveTreeNodeGetID(node);
        if (currentNodeID) {
            SvValue idxValue = SvValueCreateWithInteger(QBActiveTreeNodeGetChildNodeIndex(QBActiveTreeNodeGetParentNode(node), node), NULL);
            SvHashTableInsert(self->menuTreeOriginalIdxs, (SvGenericObject) currentNodeID, (SvGenericObject) idxValue);
            SVRELEASE(idxValue);
        }
    }
}

SvLocal void
QBProvidersControllerServiceInnov8onAddInactiveNodesToMenu(QBProvidersControllerServiceInnov8on self)
{
    // Add all inactive nodes back to menu
    QBActiveTreeNode currentNode = NULL;
    SvString nodeID = NULL;
    SvIterator it = SvHashTableKeysIterator(self->menuTreeInactiveNodes);
    while ((nodeID = (SvString) SvIteratorGetNext(&it))) {
        currentNode = (QBActiveTreeNode) SvHashTableFind(self->menuTreeInactiveNodes, (SvGenericObject) nodeID);
        SvGenericObject displayMode = QBActiveTreeNodeGetAttribute(currentNode, SVSTRING("displayMode"));

        if (displayMode && (SvStringEqualToCString( (SvString) displayMode, "showByDefault"))) {
            SvValue idxValue = (SvValue) SvHashTableFind(self->menuTreeOriginalIdxs, (SvGenericObject) nodeID);
            size_t idx, idxOrig;
            if (idxValue) {
                idx = SvValueGetInteger(idxValue);
                idxOrig = idx;

                // Update position. Check all inactive nodes. Some of them may have been before our node originally. If so decrease our idx.
                SvIterator iter = SvHashTableKeysIterator(self->menuTreeInactiveNodes);
                SvString nodeTmp;
                SvValue idxTmp;
                while ((nodeTmp = (SvString) SvIteratorGetNext(&iter))) {
                    if (nodeTmp != nodeID) {
                        idxTmp = (SvValue) SvHashTableFind(self->menuTreeOriginalIdxs, (SvGenericObject) nodeTmp);
                        if ((unsigned) SvValueGetInteger(idxTmp) < idxOrig) {
                            idx--;
                        }
                    }
                }
                QBActiveTreeInsertSubTree(self->appGlobals->menuTree, QBActiveTreeGetRootNode(self->appGlobals->menuTree), currentNode, idx, NULL);
                SvHashTableRemove(self->menuTreeInactiveNodes, (SvGenericObject) nodeID);
            }
        }
    }
}

SvLocal QBActiveTreeNode
QBProvidersControllerServiceCreateNode(QBProvidersControllerServiceInnov8on self, SvDBRawObject service)
{
    QBActiveTreeNode node = NULL;
    SvString nodeID = NULL;
    SvString name = (SvString) SvDBRawObjectGetStringAttr(service, "name");
    SvString thumbnail = (SvString) SvDBRawObjectGetStringAttr(service, "thumbnail");
    SvString tag = (SvString) SvDBRawObjectGetStringAttr(service, "tag");
    SvString type = (SvString) SvDBRawObjectGetStringAttr(service, "type");
    SvString displayMode = (SvString) SvDBRawObjectGetStringAttr(service, "displayMode");
    SvString logoURI = (SvString) SvDBRawObjectGetStringAttr(service, "logoURI");
    SvValue serviceId = SvDBObjectGetID((SvDBObject) service);
    SvValue requiresLogin = (SvValue) SvDBRawObjectGetAttrValue(service, "requires_login");
    if (!tag || !name)
        return NULL;
    if (!type)
        type = tag;
    nodeID = SvStringCreateWithFormat("%s#%s", SvStringCString(tag), SvStringCString(name));
    node = QBActiveTreeNodeCreate(nodeID, NULL, NULL);
    if (logoURI) {
        QBActiveTreeNodeSetAttribute(node, SVSTRING("serviceIcon"), (SvGenericObject) logoURI);
    }
    if (thumbnail) {
        SvString fullURL = NULL;
        SvString thumbnailPrefixUrl = QBMiddlewareManagerGetThumbnailsPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        if (!strncmp(SvStringCString(thumbnail), "http://", 7) || !thumbnailPrefixUrl || SvStringLength(thumbnailPrefixUrl) == 0)
            fullURL = SVRETAIN(thumbnail);
        else
            fullURL = SvStringCreateWithFormat("%s/%s", SvStringCString(thumbnailPrefixUrl), SvStringCString(thumbnail));
        QBActiveTreeNodeSetAttribute(node, SVSTRING("icon"), (SvGenericObject) fullURL);
        if (!logoURI) {
            QBActiveTreeNodeSetAttribute(node, SVSTRING("serviceIcon"), (SvGenericObject) fullURL);
        }
        SVRELEASE(fullURL);
    }

    if (name) {
        QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvGenericObject) name);
    }

    if (type) {
        QBActiveTreeNodeSetAttribute(node, SVSTRING("type"), (SvGenericObject) type);
        QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvGenericObject) type);
    }

    if (displayMode) {
        QBActiveTreeNodeSetAttribute(node, SVSTRING("displayMode"), (SvGenericObject) displayMode);
    }

    if (requiresLogin) {
        QBActiveTreeNodeSetAttribute(node, SVSTRING("requiresLogin"), (SvGenericObject) requiresLogin);
    }

    if (serviceId) {
        QBActiveTreeNodeSetAttribute(node, SVSTRING("serviceId"), (SvGenericObject) serviceId);
    }

    SVRELEASE(nodeID);

    return node;
}

SvLocal bool
QBProvidersControllerServiceIsServiceUnfoldable(QBProvidersControllerServiceInnov8on self,
                                                const QBCarouselMenuItemService carouselMenu)
{
    bool isUnfoldable = true;
    SvString serviceID = QBCarouselMenuGetServiceID(carouselMenu);
    SvObject service = QBMWServiceMonitorFindService(self->serviceMonitor, serviceID);
    if (service) {
        SvValue unfoldableVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) service, "unfoldable");
        if (unfoldableVal && SvObjectIsInstanceOf((SvObject) unfoldableVal, SvValue_getType())) {
            if (SvValueIsBoolean(unfoldableVal))
                isUnfoldable = SvValueGetBoolean(unfoldableVal);
            else if (SvValueIsInteger(unfoldableVal))
                isUnfoldable = SvValueGetInteger(unfoldableVal);
        }
    }
    return isUnfoldable;
}

SvLocal void
QBProvidersControllerServiceUnmountServicePriv(QBProvidersControllerServiceInnov8on self,
                                               QBProvidersControllerServiceMountParams params)
{
    if (!self || !params || !params->carouselMenu || !params->menuNodeID || !params->mounts)
        return;

    QBActiveTree tree = params->externalTree;
    if (!params->externalTree)
        tree = self->appGlobals->menuTree;

    SvArray currentMounts = (SvArray) SvHashTableFind(params->mounts, (SvGenericObject) params->menuNodeID);
    if (!currentMounts || (SvArrayIndexOfObject(currentMounts, (SvGenericObject) params->carouselMenu) < 0)) {
        SvLogError("%s(): non-existing service in tag", __func__);
        return;
    }

    SvObject menuNodePath = QBActiveTreeCreateNodePath(tree, params->menuNodeID);
    if (!menuNodePath) {
        SvLogError("No root path for node!");
        return;
    }

    // Unmount
    QBProvidersControllerServiceDoUnmount(self->appGlobals, params->carouselMenu);

    // Delete unmounted entry from menuTree
    SvString unmountedNodeID = SvStringCreateWithFormat("%s#%s", SvStringCString(params->menuNodeID), SvStringCString(QBCarouselMenuGetServiceName(params->carouselMenu)));
    QBActiveTreeNode unmountedNode = QBActiveTreeFindNode(tree, unmountedNodeID);
    if (unmountedNode) {  // If tree exists.
        QBActiveTreeRemoveSubTree(tree, unmountedNode, NULL);
    }
    SVRELEASE(unmountedNodeID);
    SvArrayRemoveObject(currentMounts, (SvGenericObject) params->carouselMenu);

    // in some cases (e.g. foldable services) removal requires additional actions
    size_t mountCount = SvArrayCount(currentMounts);
    if (mountCount == 0) {
        // Last service removed
        if (!params->isUnfoldable) {
            // it was foldable -> unset controller
            unset_controller(self, menuNodePath);
        }
    } else if (mountCount == 1) {
        // After removal one service is left
        QBCarouselMenuItemService carouselMenuTmp = (QBCarouselMenuItemService) SvArrayAt(currentMounts, 0);
        SvString serviceID = QBCarouselMenuGetServiceID(carouselMenuTmp);
        SvGenericObject service = QBMWServiceMonitorFindService(self->serviceMonitor, serviceID);
        if (!service) {
            // Remaining service is deleted but notification did not come yet
            SvString lastNodeID = SvStringCreateWithFormat("%s#%s", SvStringCString(params->menuNodeID), SvStringCString(QBCarouselMenuGetServiceName(carouselMenuTmp)));
            QBProvidersControllerServiceDoUnmount(self->appGlobals, carouselMenuTmp);
            SvArrayRemoveObject(currentMounts, (SvGenericObject) carouselMenuTmp);
            QBActiveTreeNode oldNode = QBActiveTreeFindNode(tree, lastNodeID);
            SVRELEASE(lastNodeID);
            if (oldNode) {  // node exists -> delete
                QBActiveTreeRemoveSubTree(tree, oldNode, NULL);
            }
            unset_controller(self, menuNodePath);
            if (params->services) {
                SvHashTableRemove(params->services, (SvGenericObject) serviceID);
            }
            SVRELEASE(menuNodePath);
            return;
        }

        // Check if remaining service is unfoldable (this is the default)
        if (QBProvidersControllerServiceIsServiceUnfoldable(self, carouselMenuTmp)) {
            // Only one service left, unfoldable => delete tree, unset controller & mount to root
            QBProvidersControllerServiceDoUnmount(self->appGlobals, (QBCarouselMenuItemService) SvArrayAt(currentMounts, 0));
            QBCarouselMenuItemService lastCarousel = (QBCarouselMenuItemService) SvArrayAt(currentMounts, 0);
            SvString lastNodeID = SvStringCreateWithFormat("%s#%s", SvStringCString(params->menuNodeID), SvStringCString(QBCarouselMenuGetServiceName(lastCarousel)));
            QBActiveTreeNode lastNode = QBActiveTreeFindNode(tree, lastNodeID);
            SVRELEASE(lastNodeID);
            if (lastNode) {
                QBActiveTreeRemoveSubTree(tree, lastNode, NULL);
            }
            unset_controller(self, menuNodePath);
            QBProvidersControllerServiceDoMount(self->appGlobals, (QBCarouselMenuItemService) SvArrayAt(currentMounts, 0), menuNodePath, tree);
        }
    }

    SVRELEASE(menuNodePath);
}

SvLocal void
QBProvidersControllerServiceMountServicePriv(QBProvidersControllerServiceInnov8on self,
                                             QBProvidersControllerServiceMountParams params)
{
    QBActiveTree tree = params->externalTree;
    if (!params->externalTree)
        tree = self->appGlobals->menuTree;

    QBActiveTreeNode rootNode = QBActiveTreeFindNode(tree, params->menuNodeID);
    SvObject rootPath = QBActiveTreeCreateNodePath(tree, params->menuNodeID);
    if (!rootNode || !rootPath)
        return;

    SvArray currentMounts = (SvArray) SvHashTableFind(params->mounts, (SvGenericObject) params->menuNodeID);
    if (!currentMounts) {
        SvArray curMounts = SvArrayCreateWithCapacity(3, NULL);
        SvHashTableInsert(params->mounts, (SvGenericObject) params->menuNodeID, (SvGenericObject) curMounts);
        SVRELEASE(curMounts);
        currentMounts = (SvArray) SvHashTableFind(params->mounts, (SvGenericObject) params->menuNodeID);
    }

    ssize_t currentCount = SvArrayCount(currentMounts);
    SvArrayAddObject(currentMounts, (SvGenericObject) params->carouselMenu);

    if (currentCount == 0) {
        if (params->isUnfoldable) {
            // mount it to root & return.
            QBProvidersControllerServiceDoMount(self->appGlobals, params->carouselMenu, rootPath, tree);
            SVRELEASE(rootPath);
            return;
        } else {
            // set controller, create tree and mount service
            set_controller(self, rootPath, params->controller);
        }
    } else if (currentCount == 1) {
        // After removal one service is left
        // Check if existing service is not being deleted (notification may have not come yet)
        QBCarouselMenuItemService carouselMenuTmp = (QBCarouselMenuItemService) SvArrayAt(currentMounts, 0);
        SvString serviceID = QBCarouselMenuGetServiceID(carouselMenuTmp);
        SvGenericObject service = QBMWServiceMonitorFindService(self->serviceMonitor, serviceID);
        if (!service) {
            SvString lastNodeID = SvStringCreateWithFormat("%s#%s", SvStringCString(params->menuNodeID), SvStringCString(QBCarouselMenuGetServiceName(carouselMenuTmp)));
            QBProvidersControllerServiceDoUnmount(self->appGlobals, carouselMenuTmp);
            SvArrayRemoveObject(currentMounts, (SvGenericObject) carouselMenuTmp);
            currentCount--;
            QBActiveTreeNode oldNode = QBActiveTreeFindNode(tree, lastNodeID);
            if (oldNode) {  // node exists -> delete
                QBActiveTreeRemoveSubTree(tree, oldNode, NULL);
                unset_controller(self, rootPath);
            }
            SVRELEASE(lastNodeID);
            if (params->services) {
                SvHashTableRemove(params->services, (SvGenericObject) serviceID);
            }

            if (params->isUnfoldable) {   // Mount new directly to root
                QBProvidersControllerServiceDoMount(self->appGlobals, params->carouselMenu, rootPath, tree);
                SVRELEASE(rootPath);
                return;
            } else
                set_controller(self, rootPath, params->controller);
        } else {
            // unmount existing service, set controller, create tree node & mount it back
            QBProvidersControllerServiceDoUnmount(self->appGlobals, (QBCarouselMenuItemService) SvArrayAt(currentMounts, 0));
            set_controller(self, rootPath, params->controller);
        }
    }

    // Create activeTree nodes and mount all unmounted services
    for (ssize_t i = currentCount; i >= 0; i--) {
        QBCarouselMenuItemService carouselMenuTmp = (QBCarouselMenuItemService) SvArrayAt(currentMounts, i);
        if (!QBCarouselMenuIsMounted(carouselMenuTmp)) {
            SvString serviceID = QBCarouselMenuGetServiceID(carouselMenuTmp);
            SvGenericObject service = QBMWServiceMonitorFindService(self->serviceMonitor, serviceID);
            if (!service) {     // Service is being deleted -> remove from mounts & delete node from menuTree
                SvString lastNodeID = SvStringCreateWithFormat("%s#%s", SvStringCString(params->menuNodeID), SvStringCString(QBCarouselMenuGetServiceName(carouselMenuTmp)));
                SvArrayRemoveObjectAtIndex(currentMounts, i);
                QBActiveTreeNode oldNode = QBActiveTreeFindNode(tree, lastNodeID);
                SVRELEASE(lastNodeID);
                if (oldNode)
                    QBActiveTreeRemoveSubTree(tree, oldNode, NULL);
                if (params->services) {
                    SvHashTableRemove(params->services, (SvGenericObject) serviceID);
                }
                continue;
            }
            QBActiveTreeNode node = QBProvidersControllerServiceCreateNode(self, (SvDBRawObject) service);
            if (node) {
                QBActiveTreeAddSubTree(tree, rootNode, node, NULL);
                SvObject path = QBActiveTreeCreateNodePath(tree, QBActiveTreeNodeGetID(node));
                if (!path) {  // node does not exists
                    QBActiveTreeAddSubTree(tree, rootNode, node, NULL);
                    path = QBActiveTreeCreateNodePath(tree, QBActiveTreeNodeGetID(node));
                    QBProvidersControllerServiceDoMount(self->appGlobals, carouselMenuTmp, path, tree);
                } else {    // node exists
                    QBProvidersControllerServiceDoMount(self->appGlobals, carouselMenuTmp, path, tree);
                }
                SVRELEASE(path);
            }
            SVRELEASE(node);
        }
    }

    SVRELEASE(rootPath);
}

SvLocal void
QBProvidersControllerServiceInnov8onMountInnov8onService(QBProvidersControllerServiceInnov8on self,
                                                         QBCarouselMenuItemService carouselMenu,
                                                         SvString serviceTag,
                                                         QBActiveTree externalTree)
{
    QBActiveTree tree = externalTree;
    if (!tree)
        tree = self->appGlobals->menuTree;

    // Get menuNodeID from menunodes[serviceTag].
    SvString menuNodeID = (SvString) SvHashTableFind(self->innov8onMenuNodes, (SvGenericObject) serviceTag);
    QBActiveTreeNode rootNode = NULL;

    if (!menuNodeID || !(rootNode = QBActiveTreeFindNode(tree, menuNodeID))) {
        SvLogWarning("%s(): can't find node to mount Innov8on service '%s' on", __func__, SvStringCString(serviceTag));
        return;
    }

    bool isUnfoldable = QBProvidersControllerServiceIsServiceUnfoldable(self, carouselMenu);
    struct QBProvidersControllerServiceMountParams_t params = {
        .carouselMenu  = carouselMenu,
        .menuNodeID    = menuNodeID,
        .mounts        = self->innov8onMounts,
        .services      = self->innov8onServices,
        .isUnfoldable  = isUnfoldable,
        .controller    = (SvGenericObject) self->innov8onMenuController,
        .externalTree  = tree,
    };
    QBProvidersControllerServiceMountServicePriv(self, &params);
}

SvLocal QBCarouselMenuItemService
QBProvidersControllerServiceInnov8onGetCarouselMenu(QBProvidersControllerServiceInnov8on self, SvString serviceID,
                                                    SvHashTable services, SvHashTable mounts, SvHashTable menuNodes)
{
    SvString tag = (SvString) SvHashTableFind(services, (SvGenericObject) serviceID);
    if (!tag)
        return NULL;

    SvString menuNodeID = (SvString) SvHashTableFind(menuNodes, (SvGenericObject) tag);
    if (!menuNodeID)
        return NULL;

    SvArray currentMounts = (SvArray) SvHashTableFind(mounts, (SvGenericObject) menuNodeID);
    if (currentMounts) {
        QBCarouselMenuItemService service;
        SvIterator it = SvArrayIterator(currentMounts);
        while ((service = (QBCarouselMenuItemService) SvIteratorGetNext(&it))) {
            if (SvObjectEquals((SvGenericObject) QBCarouselMenuGetServiceID(service), (SvGenericObject) serviceID))
                return service;
        }
    }

    return NULL;
}

SvLocal void
QBProvidersControllerServiceInnov8onMonitorServiceAdded(QBProvidersControllerServiceInnov8on self, SvDBObject service, SvString serviceID)
{
    QBCarouselMenuItemService carouselMenu = NULL;
    SvString serviceTag = (SvString) SvDBRawObjectGetStringAttr((SvDBRawObject) service, "tag");
    SvString serviceType = (SvString) SvDBRawObjectGetStringAttr((SvDBRawObject) service, "type");
    SvString serviceName = (SvString) SvDBRawObjectGetStringAttr((SvDBRawObject) service, "name");

    if (!serviceTag || !serviceType || !serviceName) {
        log_error("Got service (ID %s, name %s, type %s, serviceTag %s), not mounting!",
                  serviceID ? SvStringCString(serviceID) : "-", serviceName ? SvStringCString(serviceName) : "-", serviceType ? SvStringCString(serviceType) : "-",
                  serviceTag ? SvStringCString(serviceTag) : "-");
        return;
    }

    log_debug("Add new service (ID: %s Name: %s Type: %s Tag: %s)", SvStringCString(serviceID), SvStringCString(serviceName), SvStringCString(serviceType), SvStringCString(serviceTag));

    bool searchable = SvDBRawObjectGetBoolAttr((SvDBRawObject) service, "searchable");
    bool allVisible = SvDBRawObjectGetBoolAttr((SvDBRawObject) service, "all_visible");
    bool requiresLogin = SvDBRawObjectGetBoolAttr((SvDBRawObject) service, "requires_login");
    bool enableSlaveStaticCategories = SvDBRawObjectGetBoolAttr((SvDBRawObject) service, "enableSlaveStaticCategories");
    SvString serviceLayout = SvDBRawObjectGetStringAttr((SvDBRawObject) service, "service_layout");

    log_debug("New service (searchable: %d allVisible: %d requiresLogin: %d enableSlaveStaticCategories: %d serviceLayout: %s)",
              searchable, allVisible, requiresLogin, enableSlaveStaticCategories, serviceLayout ? SvStringCString(serviceLayout) : "null");

    // If node is inactive and hidden add it back to menu
    SvString menuNodeID = (SvString) SvHashTableFind(self->innov8onMenuNodes, (SvObject) serviceTag);
    if (!(QBActiveTreeFindNode(self->appGlobals->menuTree, menuNodeID))) {
        QBProvidersControllerServiceInnov8onAddInactiveNodeToMenu(self, menuNodeID);
    }

    if (serviceType && SvStringEqualToCString(serviceType, "Social media")) {
        carouselMenu = (QBCarouselMenuItemService)
                       QBSocialMediaCarouselMenuCreate(self->appGlobals, serviceType,
                                                       serviceID, serviceName,
                                                       searchable,
                                                       allVisible, requiresLogin,
                                                       enableSlaveStaticCategories);

    } else if (serviceType && SvStringEqualToCString(serviceType, "Widget")) {
        carouselMenu = QBInnov8onCarouselMenuCreate(self->appGlobals, serviceType, serviceID,
                                                    serviceName, serviceLayout,
                                                    (SvDBRawObject) service,
                                                    false, NULL);

    } else {
        SvObject vodFactory = SvHashTableFind(self->submenuFactories, (SvObject) SVSTRING("VOD"));
        if (!vodFactory) {
            SvString externalUrl = NULL;
            SvString externalID = SvDBRawObjectGetStringAttr((SvDBRawObject) service, "externalId");

            SvValue call2ActionValue = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) service, "call2action");
            bool call2Action = false;
            if (call2ActionValue && SvObjectIsInstanceOf((SvObject) call2ActionValue, SvValue_getType())) {
                if (SvValueIsBoolean(call2ActionValue))
                    call2Action = SvValueGetBoolean(call2ActionValue);
                if (call2Action) {
                    externalUrl = SvDBRawObjectGetStringAttr((SvDBRawObject) service, "externalURL");
                }
            }
            carouselMenu =
                QBInnov8onCarouselMenuCreate(self->appGlobals, serviceType, call2Action ? externalID : serviceID,
                                             serviceName, serviceLayout,
                                             (SvDBRawObject) service, call2Action, externalUrl);
            log_debug("Create carouselMenu by function QBInnov8onCarouselMenuCreate");
        } else {
            log_debug("Create carouselMenu by factory");
            QBMWMenuParams params = (QBMWMenuParams) SvTypeAllocateInstance(QBMWMenuParams_getType(), NULL);
            {
                params->appGlobals = self->appGlobals;
                params->serviceType = SVRETAIN(serviceType);
                params->serviceID = SVRETAIN(serviceID);
                params->serviceName = SVRETAIN(serviceName);
                params->serviceLayout = SVTESTRETAIN(serviceLayout);
                params->searchable = searchable;
                params->allVisible = allVisible;
                params->enableSlaveStaticCategories = enableSlaveStaticCategories;
            }
            carouselMenu = SvInvokeInterface(QBSubmenuFactory, vodFactory, createSubmenu, (SvObject) params);
            SVRELEASE(params);
        }
    }

    if (carouselMenu) {
        SvHashTableInsert(self->innov8onServices, (SvObject) serviceID, (SvObject) serviceTag);
        QBProvidersControllerServiceInnov8onNotifyListeners(self, serviceID, QBProvidersControllerServiceNotificationType_ServiceAdded);
        QBProvidersControllerServiceInnov8onMountInnov8onService(self, carouselMenu, serviceTag, NULL);

        SvObject provider = QBCarouselMenuGetProvider(carouselMenu);
        if (provider) {
            log_debug("Setup provider");
            QBContentProviderSetup((QBContentProvider) provider);
        }
        SVRELEASE(carouselMenu);
    } else {
        log_error("can't create carousel menu for Innov8on service '%s'!", SvStringCString(serviceName));
    }
}

SvLocal void
QBProvidersControllerServiceInnov8onMonitorServiceRemoved(QBProvidersControllerServiceInnov8on self, SvString serviceTag, SvString serviceID, QBCarouselMenuItemService carouselMenu)
{
    SvString menuNodeID = (SvString) SvHashTableFind(self->innov8onMenuNodes, (SvObject) serviceTag);
    if (SvHashTableFind(self->innov8onServices, (SvObject) serviceID)) {
        bool isUnfoldable = QBProvidersControllerServiceIsServiceUnfoldable(self, carouselMenu);
        struct QBProvidersControllerServiceMountParams_t params = {
            .carouselMenu = carouselMenu,
            .menuNodeID   = menuNodeID,
            .mounts       = self->innov8onMounts,
            .services     = self->innov8onServices,
            .isUnfoldable = isUnfoldable,
        };
        QBProvidersControllerServiceInnov8onNotifyListeners(self, serviceID, QBProvidersControllerServiceNotificationType_ServiceRemoved);
        QBProvidersControllerServiceUnmountServicePriv(self, &params);
        SvHashTableRemove(self->innov8onServices, (SvObject) serviceID);
    }
    QBProvidersControllerServiceInnov8onRemoveIfInactiveNodesFromMenu(self, serviceTag);
}

SvLocal void
QBProvidersControllerServiceInnov8onMonitorServiceChanged(QBProvidersControllerServiceInnov8on self, SvString serviceTag, SvString serviceID, QBCarouselMenuItemService carouselMenu)
{
    log_debug("Changed service (ID: %s Tag: %s)", SvStringCString(serviceID), SvStringCString(serviceTag));

    SvString oldTag = (SvString) SvHashTableFind(self->innov8onServices, (SvObject) serviceID);
    if (oldTag && !SvObjectEquals((SvObject) serviceTag, (SvObject) oldTag)) {
        SVRETAIN(oldTag);
        SvHashTableInsert(self->innov8onServices, (SvObject) serviceID, (SvObject) serviceTag);
        SvString oldMenuNodeID = (SvString) SvHashTableFind(self->innov8onMenuNodes, (SvObject) oldTag);
        bool isUnfoldable = QBProvidersControllerServiceIsServiceUnfoldable(self, carouselMenu);
        struct QBProvidersControllerServiceMountParams_t params = {
            .carouselMenu = carouselMenu,
            .menuNodeID   = oldMenuNodeID,
            .mounts       = self->innov8onMounts,
            .services     = self->innov8onServices,
            .isUnfoldable = isUnfoldable,
        };
        QBProvidersControllerServiceInnov8onNotifyListeners(self, serviceID, QBProvidersControllerServiceNotificationType_ServiceModified);
        QBProvidersControllerServiceUnmountServicePriv(self, &params);
        QBProvidersControllerServiceInnov8onMountInnov8onService(self, carouselMenu, serviceTag, NULL);
        SVRELEASE(oldTag);
    }
}
SvLocal void
QBProvidersControllerServiceInnov8onMonitorStatusChanged(SvObject self_, SvDBObject service, QBMWServiceMonitorEvent event)
{
    QBProvidersControllerServiceInnov8on self = (QBProvidersControllerServiceInnov8on) self_;
    QBCarouselMenuItemService carouselMenu = NULL;

    enum EventState_e {
        EventState_serviceChanged = 0,
        EventState_serviceAdded,
        EventState_serviceRemoved
    };
    enum EventState_e eventState = EventState_serviceChanged;

    SvValue serviceIDVal = SvDBObjectGetID((SvDBObject) service);
    SvString serviceID = serviceIDVal ? SvValueGetString(serviceIDVal) : NULL;
    if (!serviceID) {
        log_error("got service without ID!");
        return;
    }

    SvString serviceTag = (SvString) SvDBRawObjectGetStringAttr((SvDBRawObject) service, "tag");
    if (!serviceTag) {
        log_error("got service without Tag!");
        return;
    }

    carouselMenu = QBProvidersControllerServiceInnov8onGetCarouselMenu(self, serviceID, self->innov8onServices, self->innov8onMounts, self->innov8onMenuNodes);
    if (carouselMenu && event == QBMWServiceMonitor_serviceEnabled) {            // Service enabled & exists => nothing
        return;
    } else if (!carouselMenu && event == QBMWServiceMonitor_serviceEnabled) {   // Service enabled & !exists -> added
        eventState = EventState_serviceAdded;
    } else if (carouselMenu && event == QBMWServiceMonitor_serviceDisabled) {   // Service disabled & exists -> removed
        eventState = EventState_serviceRemoved;
    } else if (!carouselMenu && event == QBMWServiceMonitor_serviceDisabled) {  // Service disabled & !exists -> remove nodes from menus & return
        QBProvidersControllerServiceInnov8onRemoveIfInactiveNodesFromMenu(self, serviceTag);
        return;
    } else if (!carouselMenu && event == QBMWServiceMonitor_serviceChanged) {   // Does not exists & Service changed -> added
        eventState = EventState_serviceAdded;
    } else if (carouselMenu && event == QBMWServiceMonitor_serviceChanged) {
        eventState = EventState_serviceChanged;
    }

    if (eventState == EventState_serviceAdded) {
        QBProvidersControllerServiceInnov8onMonitorServiceAdded(self, service, serviceID);
    } else if (eventState == EventState_serviceRemoved) {
        QBProvidersControllerServiceInnov8onMonitorServiceRemoved(self, serviceTag, serviceID, carouselMenu);
    } else {    // changed
        QBProvidersControllerServiceInnov8onMonitorServiceChanged(self, serviceTag, serviceID, carouselMenu);
    }
}

SvLocal void
QBProvidersControllerServiceDestroyServicesPriv(QBProvidersControllerServiceInnov8on self)
{
    SvString menuNodeID;
    SvIterator it = SvHashTableValuesIterator(self->innov8onMenuNodes);
    while ((menuNodeID = (SvString) SvIteratorGetNext(&it))) {
        SvArray currentMounts = (SvArray) SvHashTableFind(self->innov8onMounts, (SvGenericObject) menuNodeID);
        if (!currentMounts || SvArrayCount(currentMounts) <= 0)
            continue;
        for (size_t i = 0; i < SvArrayCount(currentMounts); i++) {
            QBCarouselMenuItemService menu = (QBCarouselMenuItemService) SvArrayAt(currentMounts, i);
            QBProvidersControllerServiceInnov8onNotifyListeners(self, QBCarouselMenuGetServiceID(menu),
                                                                QBProvidersControllerServiceNotificationType_ServiceRemoved);
        }
    }
    QBProvidersControllerServiceDestroyServices(self->appGlobals->providersController, self->innov8onMounts,
                                                self->innov8onMenuNodes, self->innov8onServices);
}

SvLocal void
QBProvidersControllerServiceInnov8onMiddlewareDataChanged(SvGenericObject self_, QBMiddlewareManagerType middlewareType)
{
    QBProvidersControllerServiceInnov8on self = (QBProvidersControllerServiceInnov8on) self_;
    SvString mwId = NULL;
    SvString prefixUrl = NULL;

    if (middlewareType == QBMiddlewareManagerType_Innov8on) {
        prefixUrl = QBMiddlewareManagerGetPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        mwId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        bool hasNetwork = QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);
        if (!prefixUrl || !mwId || !hasNetwork) {
            QBProvidersControllerServiceDestroyServicesPriv(self);
            QBProvidersControllerServiceInnov8onRemoveIfInactiveNodesFromMenu(self, NULL);
            QBProvidersControllerServiceInnov8onAddInactiveNodesToMenu(self);
        }
        if (!self->innov8onContentManager) {
            QBContentManagersService contentManagers = (QBContentManagersService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                      SVSTRING("QBContentManagersService"));
            if (contentManagers) {
                self->innov8onContentManager = (Innov8onContentManager) QBContentManagersServiceFindFirst(contentManagers, QBContentManagerType_cubiware);
            }
        }
        if (self->innov8onContentManager) {
            Innov8onContentManagerSetDeviceId(self->innov8onContentManager, mwId, NULL);
            if (mwId) {
                Innov8onProviderParams params = Innov8onProviderParamsCreate(Innov8onContentManagerGetServerInfo(self->innov8onContentManager, NULL),
                                                                             mwId, 3, 10, 200, 400, 60, NULL, NULL, NULL, false, NULL);
                Innov8onContentManagerSetParams(self->innov8onContentManager, params, NULL);
                SVRELEASE(params);
            }
        }
    }
}

SvLocal void
QBProvidersControllerServiceInnv8onProcessNode(QBProvidersControllerServiceInnov8on self,
                                               QBActiveTreeNode menuNode,
                                               SvString vodServiceType,
                                               const bool configVODServiceTypeIsCubimw)
{
    assert(self && menuNode && vodServiceType);

    const bool vodServiceTypeIsConfig = SvStringEqualToCString(vodServiceType, "CONFIG");
    if (configVODServiceTypeIsCubimw && (SvStringEqualToCString(vodServiceType, "cubimw") || vodServiceTypeIsConfig)) {
        SvString menuNodeID = QBActiveTreeNodeGetID(menuNode);
        SvString serviceTag = (SvString) QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("Innov8onServiceTag"));
        if (!serviceTag)
            serviceTag = menuNodeID;
        SvHashTableInsert(self->innov8onMenuNodes, (SvObject) serviceTag, (SvObject) menuNodeID);
    }
}

SvLocal void
QBProvidersControllerServiceInnov8onAddMenuNode(SvObject self_)
{
    QBProvidersControllerServiceInnov8on self = (QBProvidersControllerServiceInnov8on) self_;

    QBTreeIterator iter = QBActiveTreeCreatePostOrderTreeIterator(self->appGlobals->menuTree);
    QBActiveTreeNode menuNode = NULL;

    const char *configVODServiceType = QBConfigGet("PROVIDERS.VOD");
    const bool configVODServiceTypeIsCubimw = (!configVODServiceType || strcmp(configVODServiceType, "cubimw") == 0);
    const char *configWEBServiteType = QBConfigGet("PROVIDERS.WEBSERVICES");
    const bool configWEBServiteTypeIsCubimw = (!configWEBServiteType || strcmp(configWEBServiteType, "cubimw") == 0);

    while ((menuNode = (QBActiveTreeNode) QBTreeIteratorGetNextNode(&iter))) {
        SvString vodServiceType = (SvString) QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("VODServiceType"));

        if (vodServiceType && SvObjectIsInstanceOf((SvGenericObject) vodServiceType, SvString_getType())) {
            if (SvStringEqualToCString(vodServiceType, "Composite")) {
                SvIterator childIterator = QBActiveTreeNodeChildNodesIterator(menuNode);
                QBActiveTreeNode childNode = NULL;
                while ((childNode = (QBActiveTreeNode) SvIteratorGetNext(&childIterator))) {
                    SvString childVodServiceType = (SvString) QBActiveTreeNodeGetAttribute(childNode, SVSTRING("VODServiceType"));
                    if (!childVodServiceType || !SvObjectIsInstanceOf((SvGenericObject) childVodServiceType, SvString_getType()))
                        continue;

                    QBProvidersControllerServiceInnv8onProcessNode(self, childNode, childVodServiceType, configVODServiceTypeIsCubimw);
                }

                continue;
            }
            QBProvidersControllerServiceInnv8onProcessNode(self, menuNode, vodServiceType, configVODServiceTypeIsCubimw);
        }
        SvString webServiceType = (SvString) QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("WebServiceType"));
        if (webServiceType && SvObjectIsInstanceOf((SvGenericObject) webServiceType, SvString_getType())) {
            const bool webServiceTypeIsConfig = SvStringEqualToCString(webServiceType, "CONFIG");
            if (configWEBServiteTypeIsCubimw && (SvStringEqualToCString(webServiceType, "cubimw") || webServiceTypeIsConfig)) {
                SvString menuNodeID = QBActiveTreeNodeGetID(menuNode);
                SvString serviceTag = (SvString) QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("Innov8onServiceTag"));
                if (!serviceTag)
                    serviceTag = menuNodeID;
                SvHashTableInsert(self->innov8onMenuNodes, (SvGenericObject) serviceTag, (SvGenericObject) menuNodeID);
            }
        }
    }
}

SvLocal void
QBProvidersControllerServiceInnov8onStart(SvObject self_)
{
    log_debug("Start providers controller service monitor");
    QBProvidersControllerServiceInnov8on self = (QBProvidersControllerServiceInnov8on) self_;

    QBMiddlewareManagerAddListener(self->appGlobals->middlewareManager, (SvGenericObject) self);

    QBProvidersControllerServiceInnov8onGetOriginalTreeOrder(self);

    SvString innov8onId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    if (self->innov8onContentManager) {
        Innov8onContentManagerSetDeviceId(self->innov8onContentManager, innov8onId, NULL);
        if (innov8onId) {
            Innov8onProviderParams params = Innov8onProviderParamsCreate(Innov8onContentManagerGetServerInfo(self->innov8onContentManager, NULL),
                                                                         innov8onId, 3, 10, 200, 400, 60, NULL, NULL, NULL, false, NULL);
            Innov8onContentManagerSetParams(self->innov8onContentManager, params, NULL);
            SVRELEASE(params);
        }
    }

    if (self->appGlobals->serviceMonitor) {
        self->serviceMonitor = SVRETAIN(self->appGlobals->serviceMonitor);
        QBMWServiceMonitorAddListener(self->serviceMonitor, (SvObject) self);
    }

    QBProvidersControllerServiceInnov8onRemoveIfInactiveNodesFromMenu(self, NULL);
}

SvLocal void
QBProvidersControllerServiceInnov8onStop(SvObject self_)
{
    QBProvidersControllerServiceInnov8on self = (QBProvidersControllerServiceInnov8on) self_;

    QBProvidersControllerServiceDestroyServicesPriv(self);

    if (self->serviceMonitor) {
        QBMWServiceMonitorRemoveListener(self->serviceMonitor, (SvObject) self);
        SVRELEASE(self->serviceMonitor);
        self->serviceMonitor = NULL;
    }

    SvIterator it = SvHashTableValuesIterator(self->innov8onMenuNodes);
    SvString nodeId;
    while ((nodeId = (SvString) SvIteratorGetNext(&it))) {
        SvObject path = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, nodeId);
        unset_skin_controller(self, path);
        SVRELEASE(path);
    }

    QBProvidersControllerServiceInnov8onAddInactiveNodesToMenu(self);

    if (self->appGlobals->middlewareManager) {
        QBMiddlewareManagerRemoveListener(self->appGlobals->middlewareManager, (SvGenericObject) self);
    }
}

SvLocal SvArray
QBProvidersControllerServiceInnov8onGetServices(SvObject self_, SvString tag)
{
    QBProvidersControllerServiceInnov8on self = (QBProvidersControllerServiceInnov8on) self_;
    return (SvArray) SvHashTableFind(self->innov8onMounts, (SvGenericObject) tag);
}

SvLocal QBProvidersControllerServiceType
QBProvidersControllerServiceInnov8onType(SvObject self_)
{
    return QBProvidersControllerService_INNOV8ON;
}

SvLocal void
QBProvidersControllerServiceInnov8onReinitialize(SvObject self_, SvArray itemList)
{
}

SvLocal void
QBProvidersControllerServiceInnov8onRegisterSubmenuFactory(SvObject self_, SvString id, SvObject submenuFactory)
{
    if (!id) {
        SvLogError("%s(): id is NULL", __func__);
        return;
    }
    QBProvidersControllerServiceInnov8on self = (QBProvidersControllerServiceInnov8on) self_;
    SvHashTableInsert(self->submenuFactories, (SvObject) id, submenuFactory);
}

SvLocal void
QBProvidersControllerServiceInnov8on__dtor__(void *self_)
{
    QBProvidersControllerServiceInnov8on self = self_;

    SVTESTRELEASE(self->serviceMonitor);
    SVRELEASE(self->serviceListeners);
    SVRELEASE(self->menuTreeOriginalIdxs);
    SVRELEASE(self->menuTreeInactiveNodes);
    SVRELEASE(self->innov8onMenuNodes);
    SVRELEASE(self->innov8onMounts);
    SVRELEASE(self->innov8onServices);
    SVRELEASE(self->innov8onMenuController);

    SVRELEASE(self->submenuFactories);
}

SvLocal SvType
QBProvidersControllerServiceInnov8on_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        static const struct SvObjectVTable_ objectVTable = {
            .destroy = QBProvidersControllerServiceInnov8on__dtor__
        };
        static const struct QBProvidersControllerServiceChild_t serviceMethods = {
            .start                      = QBProvidersControllerServiceInnov8onStart,
            .stop                       = QBProvidersControllerServiceInnov8onStop,
            .addNode                    = QBProvidersControllerServiceInnov8onAddMenuNode,
            .reinitialize               = QBProvidersControllerServiceInnov8onReinitialize,
            .getType                    = QBProvidersControllerServiceInnov8onType,
            .getServices                = QBProvidersControllerServiceInnov8onGetServices,
            .registerSubmenuFactory     = QBProvidersControllerServiceInnov8onRegisterSubmenuFactory,
        };
        static struct QBMWServiceMonitorListener_t monitorMethods = {
            .serviceStatusChanged = QBProvidersControllerServiceInnov8onMonitorStatusChanged,
        };
        static struct QBMiddlewareManagerListener_t middlewareIdListener = {
            .middlewareDataChanged = QBProvidersControllerServiceInnov8onMiddlewareDataChanged,
        };
        static struct QBProvidersControllerServiceObservable_t observerableMethods = {
            .addListener           = QBProvidersControllerServiceInnov8onAddListener,
            .removeListener        = QBProvidersControllerServiceInnov8onRemoveListener,
        };

        SvTypeCreateManaged("QBProvidersControllerServiceInnov8on",
                            sizeof(struct QBProvidersControllerServiceInnov8on_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBProvidersControllerServiceObservable_getInterface(), &observerableMethods,
                            QBProvidersControllerServiceChild_getInterface(), &serviceMethods,
                            QBMWServiceMonitorListener_getInterface(), &monitorMethods,
                            QBMiddlewareManagerListener_getInterface(), &middlewareIdListener,
                            NULL);
    }
    return type;
}

SvObject
QBProvidersControllerServiceInnov8onCreate(AppGlobals appGlobals)
{
    QBProvidersControllerServiceInnov8on self;
    self = (QBProvidersControllerServiceInnov8on) SvTypeAllocateInstance(QBProvidersControllerServiceInnov8on_getType(), NULL);
    if (!self)
        return NULL;

    self->appGlobals = appGlobals;
    self->menuTreeInactiveNodes = SvHashTableCreate(6, NULL);
    self->menuTreeOriginalIdxs = SvHashTableCreate(15, NULL);

    QBContentManagersService contentManagers = (QBContentManagersService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                      SVSTRING("QBContentManagersService"));
    if (contentManagers)
        self->innov8onContentManager = (Innov8onContentManager) QBContentManagersServiceFindFirst(contentManagers, QBContentManagerType_cubiware);
    self->innov8onServices = SvHashTableCreate(17, NULL);
    self->innov8onMenuNodes = SvHashTableCreate(17, NULL);
    self->innov8onMounts = SvHashTableCreate(17, NULL);
    self->serviceListeners = SvWeakListCreate(NULL);
    self->submenuFactories = SvHashTableCreate(5, NULL);

    svSettingsPushComponent("Carousel_VOD.settings");
    self->innov8onMenuController = QBInnov8onCarouselMenuItemControllerCreate(appGlobals, NULL);
    svSettingsPopComponent();

    return (SvObject) self;
}
