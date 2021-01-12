/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBProvidersControllerServiceMWClient.h"

#include <stdlib.h>
#include <stdbool.h>

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <QBConf.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <Menus/carouselMenu.h>
#include <Menus/QBMWClientMenuParams.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBMenu/QBMenu.h>
#include <Menus/Innov8on/innov8onCarouselItemController.h>
#include <Utils/value.h>
#include <main.h>
#include <SvCore/SvEnv.h>

#include "QBProvidersControllerService.h"

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "QBProvidersControllerServiceMWClientLogLevel", "0:error and warning, 1:state, 2:debug");

#define log_error(fmt, ...)   do { if (env_log_level() >= 0) SvLogError(COLBEG() "[%s] " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#define log_warning(fmt, ...) do { if (env_log_level() >= 0) SvLogWarning(COLBEG() "[%s] " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
#define log_state(fmt, ...)   do { if (env_log_level() >= 1) SvLogNotice(COLBEG() "[%s] " fmt COLEND_COL(cyan), __func__, ## __VA_ARGS__); } while (0)
#define log_debug(fmt, ...)   do { if (env_log_level() >= 2) SvLogNotice(COLBEG() "[%s] " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)

struct QBProvidersControllerServiceMWClient_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    SvHashTable menuNodes; // service Tag -> menu node ID
    SvHashTable mounts;    // menu node ID -> array of QBCarouselMenuItemService

    SvWeakList serviceListeners;
    QBMWServiceMonitor serviceMonitor;
    SvDBObject vodService; // mw vod service, digitalSmith vod should be available when vod in MW is enabled

    SvHashTable submenuFactories;
    QBInnov8onCarouselMenuItemController innov8onMenuController;

    bool isStarted;
};

typedef struct QBProvidersControllerServiceMWClient_t *QBProvidersControllerServiceMWClient;

typedef enum {
    QBProvidersControllerServiceNotificationType_ServiceAdded = 0,
    QBProvidersControllerServiceNotificationType_ServiceRemoved,
    QBProvidersControllerServiceNotificationType_ServiceModified
} QBProvidersControllerServiceNotificationType;

SvLocal void
QBProvidersControllerServiceMWClientAddMenuNode(SvObject self_)
{
    QBProvidersControllerServiceMWClient self = (QBProvidersControllerServiceMWClient) self_;
    QBTreeIterator iter = QBActiveTreeCreatePostOrderTreeIterator(self->appGlobals->menuTree);
    QBActiveTreeNode menuNode = NULL;

    const char *configVODServiceType = QBConfigGet("PROVIDERS.VOD");
    const bool configVODServiceTypeIsMWClient = (configVODServiceType && strcmp(configVODServiceType, "digitalsmiths") == 0);
    if (!configVODServiceTypeIsMWClient)
        return;
    while ((menuNode = (QBActiveTreeNode) QBTreeIteratorGetNextNode(&iter))) {
        SvString vodServiceType = (SvString) QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("VODServiceType"));
        if (vodServiceType && SvObjectIsInstanceOf((SvObject) vodServiceType, SvString_getType())) {
            const bool vodServiceTypeIsConfig = SvStringEqualToCString(vodServiceType, "CONFIG");
            if (SvStringEqualToCString(vodServiceType, "mwclient") || vodServiceTypeIsConfig) {
                SvString menuNodeID = QBActiveTreeNodeGetID(menuNode);
                SvString serviceTag = (SvString) QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("MWClientServiceTag"));
                if (!serviceTag)
                    serviceTag = menuNodeID;
                SvHashTableInsert(self->menuNodes, (SvObject) serviceTag, (SvObject) menuNodeID);
            }
        }
    }
}

SvLocal SvObject
QBProvidersControllerServiceMWClientCreateMenuParams(AppGlobals appGlobals, SvObject serviceId)
{
    QBMWClientMenuParams params = (QBMWClientMenuParams)
                                  SvTypeAllocateInstance(QBMWClientMenuParams_getType(), NULL);
    params->appGlobals = appGlobals;
    params->serviceType = SVSTRING("MWClient");
    params->serviceID = (SvString) SVRETAIN(serviceId);
    params->serviceName = SVSTRING("MWClient");
    params->enableSlaveStaticCategories = false;
    params->serviceLayout = SVSTRING("digitalsmiths");

    return (SvObject) params;
}

SvLocal void
QBProvidersControllerServiceMWClientAddListener(SvObject self_, SvObject listener)
{
    QBProvidersControllerServiceMWClient self = (QBProvidersControllerServiceMWClient) self_;
    if (SvObjectIsImplementationOf(listener, QBProvidersControllerServiceListener_getInterface()))
        SvWeakListPushFront(self->serviceListeners, listener, NULL);
}

SvLocal void
QBProvidersControllerServiceMWClientRemoveListener(SvObject self_, SvObject listener)
{
    QBProvidersControllerServiceMWClient self = (QBProvidersControllerServiceMWClient) self_;
    SvWeakListRemoveObject(self->serviceListeners, listener);
}

SvLocal void
QBProvidersControllerServiceMWClientNotifyListeners(QBProvidersControllerServiceMWClient self, SvString serviceId,
                                                    QBProvidersControllerServiceNotificationType notificationType)
{
    SvIterator iterator = SvWeakListIterator(self->serviceListeners);
    SvGenericObject listener;
    if (notificationType == QBProvidersControllerServiceNotificationType_ServiceAdded) {
        while ((listener = SvIteratorGetNext(&iterator)))
            SvInvokeInterface(QBProvidersControllerServiceListener, listener, serviceAdded, serviceId);
    } else if (notificationType == QBProvidersControllerServiceNotificationType_ServiceRemoved) {
        while ((listener = SvIteratorGetNext(&iterator)))
            SvInvokeInterface(QBProvidersControllerServiceListener, listener, serviceRemoved, serviceId);
    } else if (notificationType == QBProvidersControllerServiceNotificationType_ServiceModified) {
        while ((listener = SvIteratorGetNext(&iterator)))
            SvInvokeInterface(QBProvidersControllerServiceListener, listener, serviceModified, serviceId);
    }
}

SvLocal QBActiveTreeNode
QBProvidersControllerServiceCreateNode(QBProvidersControllerServiceMWClient self, SvString tag, QBCarouselMenuItemService menu)
{
    QBActiveTreeNode node = NULL;
    SvString nodeID = NULL;
    SvString name = QBCarouselMenuGetServiceName(menu);
    SvString serviceId = QBCarouselMenuGetServiceID(menu);

    if (!tag || !name) {
        log_error("Can't create node id (tag:%p, name:%p).", tag, name);
        return NULL;
    }

    nodeID = SvStringCreateWithFormat("%s#%s", SvStringCString(tag), SvStringCString(name));
    node = QBActiveTreeNodeCreate(nodeID, NULL, NULL);

    QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvGenericObject) tag);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("tag"), (SvGenericObject) tag);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("type"), (SvGenericObject) tag);

    if (serviceId)
        QBActiveTreeNodeSetAttribute(node, SVSTRING("serviceId"), (SvObject) serviceId);

    SVRELEASE(nodeID);

    return node;
}

SvLocal void
QBProvidersControllerServiceMWClientSetController(QBProvidersControllerServiceMWClient self, SvGenericObject path, SvGenericObject controller)
{
    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, controller, NULL);
}

SvLocal void
QBProvidersControllerServiceMWClientUnsetController(QBProvidersControllerServiceMWClient self, SvGenericObject path)
{
    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvErrorInfo error = NULL;
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, NULL, &error);
    if (error) {
        SvErrorInfoDestroy(error);
    }
}

SvLocal void
QBProvidersControllerServiceMWClientMountService(QBProvidersControllerServiceMWClient self, QBCarouselMenuItemService menu, SvString menuNodeID)
{
    QBActiveTree tree = self->appGlobals->menuTree;

    SvGenericObject rootPath = NULL;
    QBActiveTreeNode rootNode = QBActiveTreeFindNodeByID(tree, menuNodeID, &rootPath);
    if (!rootNode || !rootPath)
        return;

    SvArray currentMounts = (SvArray) SvHashTableFind(self->mounts, (SvObject) menuNodeID);
    if (!currentMounts) {
        SvArray curMounts = SvArrayCreateWithCapacity(3, NULL);
        SvHashTableInsert(self->mounts, (SvObject) menuNodeID, (SvGenericObject) curMounts);
        SVRELEASE(curMounts);
        currentMounts = (SvArray) SvHashTableFind(self->mounts, (SvObject) menuNodeID);
    }

    ssize_t currentCount = SvArrayCount(currentMounts);
    SvArrayAddObject(currentMounts, (SvObject) menu);

    if (currentCount == 0)
        QBProvidersControllerServiceMWClientSetController(self, rootPath, (SvObject) self->innov8onMenuController);

    if (!QBCarouselMenuIsMounted(menu)) {
        QBActiveTreeNode node = QBProvidersControllerServiceCreateNode(self, menuNodeID, menu);
        if (node) {
            SvGenericObject path = NULL;
            QBActiveTreeAddSubTree(tree, rootNode, node, NULL);
            QBActiveTreeFindNodeByID(tree, QBActiveTreeNodeGetID(node), &path);
            QBProvidersControllerServiceDoMount(self->appGlobals, menu, path, tree);
            SVRELEASE(node);
        }
    }
}

SvLocal void
QBProvidersControllerServiceMWClientMountServices(QBProvidersControllerServiceMWClient self)
{
    SvIterator it = SvHashTableKeysIterator(self->menuNodes);
    SvObject key = NULL;
    while ((key = SvIteratorGetNext(&it))) {
        SvString nodeID = (SvString) SvHashTableFind(self->menuNodes, (SvObject) key);
        if (!nodeID || !QBActiveTreeFindNode(self->appGlobals->menuTree, nodeID)) {
            log_error("There's no such node in the tree: %s", nodeID ? SvStringCString(nodeID) : NULL);
            continue;
        }
        SvObject factory = SvHashTableFind(self->submenuFactories, (SvObject) nodeID);
        if (!factory) {
            // this is valid situation
            log_state("There's no factory for given nodeID:%s yet.", SvStringCString(nodeID));
            continue;
        }
        SvObject params = QBProvidersControllerServiceMWClientCreateMenuParams(self->appGlobals, key);
        QBCarouselMenuItemService menu = SvInvokeInterface(QBSubmenuFactory, factory, createSubmenu, params);
        SVRELEASE(params);
        if (menu) {
            log_debug("Mouting: ServiceID:%s ServiceName:%s", SvStringCString(QBCarouselMenuGetServiceID(menu)), SvStringCString(QBCarouselMenuGetServiceName(menu)));
            QBProvidersControllerServiceMWClientMountService(self, menu, nodeID);
            SVRELEASE(menu);
        } else {
            log_error("can't create carousel menu for DigitalSmiths service!");
        }
    }
}

SvLocal void
QBProvidersControllerServiceMWClientStart(SvObject self_)
{
    QBProvidersControllerServiceMWClient self = (QBProvidersControllerServiceMWClient) self_;
    self->isStarted = true;

    if (self->serviceMonitor)
        QBMWServiceMonitorAddListener(self->serviceMonitor, (SvObject) self);

    QBProvidersControllerServiceMWClientMountServices(self);
}

SvLocal void
QBProvidersControllerServiceMWClientStop(SvObject self_)
{
    QBProvidersControllerServiceMWClient self = (QBProvidersControllerServiceMWClient) self_;
    self->isStarted = false;

    if (self->serviceMonitor) {
        QBMWServiceMonitorRemoveListener(self->serviceMonitor, (SvObject) self);
    }

    QBProvidersControllerServiceDestroyServices(self->appGlobals->providersController,
                                                self->mounts, self->menuNodes, NULL);
}

SvLocal void
QBProvidersControllerServiceMWClientReinitialize(SvObject self_, SvArray itemList)
{
}

SvLocal QBProvidersControllerServiceType
QBProvidersControllerServiceMWClientType(SvObject self_)
{
    return QBProvidersControllerService_MWClient;
}

SvLocal SvArray
QBProvidersControllerServiceMWClientGetServices(SvObject self_, SvString tag)
{
    QBProvidersControllerServiceMWClient self = (QBProvidersControllerServiceMWClient) self_;
    return (SvArray) SvHashTableFind(self->mounts, (SvObject) tag);
}

SvLocal void
QBProvidersControllerServiceMWClientDestroy(void *self_)
{
    QBProvidersControllerServiceMWClient self = self_;

    SVRELEASE(self->menuNodes);
    SVRELEASE(self->mounts);
    SVRELEASE(self->serviceListeners);
    SVTESTRELEASE(self->serviceMonitor);
    SVRELEASE(self->submenuFactories);
    SVRELEASE(self->innov8onMenuController);
    SVTESTRELEASE(self->vodService);
}

SvLocal void
QBProvidersControllerServiceMWClientRegisterSubmenuFactory(SvObject self_, SvString id, SvObject submenuFactory)
{
    log_debug(" Register submenu factory with id: %s", SvStringCString(id));

    if (!id) {
        log_error("id is NULL");
        return;
    }
    QBProvidersControllerServiceMWClient self = (QBProvidersControllerServiceMWClient) self_;
    SvHashTableInsert(self->submenuFactories, (SvObject) id, submenuFactory);

    if (self->isStarted && self->vodService) {
        QBProvidersControllerServiceMWClientMountServices(self);
    }
}

SvLocal void
QBProvidersControllerServiceMWClientDestroyServices(QBProvidersControllerServiceMWClient self)
{
    QBActiveTree tree = self->appGlobals->menuTree;
    SvObject menuNodePath = NULL;

    SvString menuNodeID;
    SvIterator it = SvHashTableValuesIterator(self->menuNodes);
    while ((menuNodeID = (SvString) SvIteratorGetNext(&it))) {
        SvArray currentMounts = (SvArray) SvHashTableFind(self->mounts, (SvObject) menuNodeID);
        if (!currentMounts)
            continue;

        QBActiveTreeFindNodeByID(tree, menuNodeID, &menuNodePath);
        if (!menuNodePath) {
            log_error("No root path for node: %s", SvStringCString(menuNodeID));
            continue;
        }

        for (size_t i = 0; i < SvArrayCount(currentMounts); i++) {
            QBCarouselMenuItemService menu = (QBCarouselMenuItemService) SvArrayAt(currentMounts, i);
            QBProvidersControllerServiceMWClientNotifyListeners(self, QBCarouselMenuGetServiceID(menu), QBProvidersControllerServiceNotificationType_ServiceRemoved);
            // Delete entry from menuTree
            SvString unmountedNodeID = SvStringCreateWithFormat("%s#%s", SvStringCString(menuNodeID), SvStringCString(QBCarouselMenuGetServiceName(menu)));
            log_state("trying to unmount node: %s", SvStringCString(unmountedNodeID));
            QBActiveTreeNode unmountedNode = QBActiveTreeFindNodeByID(tree, unmountedNodeID, NULL);
            if (unmountedNode) {
                QBActiveTreeRemoveSubTree(tree, unmountedNode, NULL);
            }
            SVRELEASE(unmountedNodeID);
        }
        QBProvidersControllerServiceMWClientUnsetController(self, menuNodePath);
        SvHashTableRemove(self->mounts, (SvObject) menuNodeID);
    }
}

SvLocal void
QBProvidersControllerServiceMWClientMonitorStatusChanged(SvObject self_, SvDBObject service, QBMWServiceMonitorEvent event)
{
    QBProvidersControllerServiceMWClient self = (QBProvidersControllerServiceMWClient) self_;

    SvString serviceTag = (SvString) SvDBRawObjectGetStringAttr((SvDBRawObject) service, "tag");
    SvString serviceType = (SvString) SvDBRawObjectGetStringAttr((SvDBRawObject) service, "type");
    SvString serviceName = (SvString) SvDBRawObjectGetStringAttr((SvDBRawObject) service, "name");

    if (!serviceTag || !serviceType || !serviceName) {
        log_error("Got wrong service: name %s, type %s, serviceTag %s",
                  serviceName ? SvStringCString(serviceName) : "-", serviceType ? SvStringCString(serviceType) : "-", serviceTag ? SvStringCString(serviceTag) : "-");
        return;
    }

    if (SvStringEqualToCString(serviceName, "VOD") && SvStringEqualToCString(serviceType, "VOD") && SvStringEqualToCString(serviceTag, "VOD")) {
        if (event == QBMWServiceMonitor_serviceEnabled) {
            SVTESTRELEASE(self->vodService);
            self->vodService = SVRETAIN(service);
            if (self->isStarted) {
                QBProvidersControllerServiceMWClientMountServices(self);
                log_state("Mounting service (Name: %s Type: %s Tag: %s) event: %d", SvStringCString(serviceName), SvStringCString(serviceType), SvStringCString(serviceTag), event);
            }
        } else if (event == QBMWServiceMonitor_serviceDisabled) {
            QBProvidersControllerServiceMWClientDestroyServices(self);
            SVTESTRELEASE(self->vodService);
            self->vodService = NULL;
            log_state("Destroying service (Name: %s Type: %s Tag: %s) event: %d", SvStringCString(serviceName), SvStringCString(serviceType), SvStringCString(serviceTag), event);
        }
    } else {
        log_debug("Ignoring service (Name: %s Type: %s Tag: %s) event: %d", SvStringCString(serviceName), SvStringCString(serviceType), SvStringCString(serviceTag), event);
    }
}

SvLocal SvType
QBProvidersControllerServiceMWClient_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        static const struct SvObjectVTable_ objectVTable = {
            .destroy = QBProvidersControllerServiceMWClientDestroy
        };
        static const struct QBProvidersControllerServiceChild_t serviceMethods = {
            .start                  = QBProvidersControllerServiceMWClientStart,
            .stop                   = QBProvidersControllerServiceMWClientStop,
            .addNode                = QBProvidersControllerServiceMWClientAddMenuNode,
            .reinitialize           = QBProvidersControllerServiceMWClientReinitialize,
            .getType                = QBProvidersControllerServiceMWClientType,
            .getServices            = QBProvidersControllerServiceMWClientGetServices,
            .registerSubmenuFactory = QBProvidersControllerServiceMWClientRegisterSubmenuFactory,
        };
        static struct QBMWServiceMonitorListener_t monitorMethods = {
            .serviceStatusChanged = QBProvidersControllerServiceMWClientMonitorStatusChanged,
        };

        static struct QBProvidersControllerServiceObservable_t observerableMethods = {
            .addListener    = QBProvidersControllerServiceMWClientAddListener,
            .removeListener = QBProvidersControllerServiceMWClientRemoveListener,
        };

        SvTypeCreateManaged("QBProvidersControllerServiceMWClient",
                            sizeof(struct QBProvidersControllerServiceMWClient_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBProvidersControllerServiceObservable_getInterface(), &observerableMethods,
                            QBMWServiceMonitorListener_getInterface(), &monitorMethods,
                            QBProvidersControllerServiceChild_getInterface(), &serviceMethods,
                            NULL);
    }
    return type;
}

SvObject
QBProvidersControllerServiceMWClientCreate(AppGlobals appGlobals)
{
    QBProvidersControllerServiceMWClient self;

    self = (QBProvidersControllerServiceMWClient) SvTypeAllocateInstance(QBProvidersControllerServiceMWClient_getType(), NULL);
    if (!self)
        return NULL;

    self->appGlobals = appGlobals;
    self->menuNodes = SvHashTableCreate(11, NULL);
    self->mounts = SvHashTableCreate(11, NULL);
    self->submenuFactories = SvHashTableCreate(11, NULL);
    self->serviceListeners = SvWeakListCreate(NULL);
    self->innov8onMenuController = QBInnov8onCarouselMenuItemControllerCreate(appGlobals, NULL);
    self->serviceMonitor = SVTESTRETAIN(appGlobals->serviceMonitor);

    return (SvObject) self;
}
