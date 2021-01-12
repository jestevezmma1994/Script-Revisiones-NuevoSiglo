/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBProvidersControllerServiceMStore.h"
#include <stdlib.h>
#include <main.h>
#include <settings.h>
#include <Menus/menuchoice.h>
#include <QBConf.h>
#include "QBProvidersControllerService.h"
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <Services/QBPVODStorage.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBTimeDateMonitor.h>
#include <Menus/MStore/MStoreCarouselItemController.h>
#include <QBContentManager/QBContentProvider.h>

#define log_debug(fmt, ...)                                                 \
    do { if (0) { SvLogNotice(COLBEG() "%s() :: " fmt COLEND_COL(blue), __func__, ##__VA_ARGS__); } } while (0)

struct QBProvidersControllerServiceMStore_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    SvString     path;
    SvHashTable  menuNodes;
    SvHashTable  mounts;

    bool         storageConnected;
    bool         timeSet;
    bool         created;
};

typedef struct QBProvidersControllerServiceMStore_t *QBProvidersControllerServiceMStore;

SvLocal void
QBProvidersControllerServiceMStoreProcessNode(QBProvidersControllerServiceMStore self,
                                              QBActiveTreeNode menuNode,
                                              SvString vodServiceType)
{
    assert(self && menuNode && vodServiceType);

    if (!SvStringEqualToCString(vodServiceType, "MStore"))
        return;

    SvString menuNodeID = QBActiveTreeNodeGetID(menuNode);
    SvHashTableInsert(self->menuNodes, (SvObject) SVSTRING("MSTORE"), (SvObject) menuNodeID);
}

SvLocal void
QBProvidersControllerServiceMStoreAddMenuNode(SvObject self_)
{
    QBProvidersControllerServiceMStore self = (QBProvidersControllerServiceMStore) self_;
    QBTreeIterator iter = QBActiveTreeCreatePostOrderTreeIterator(self->appGlobals->menuTree);
    QBActiveTreeNode menuNode = NULL;

    while ((menuNode = (QBActiveTreeNode) QBTreeIteratorGetNextNode(&iter))) {
        SvString vodServiceType = (SvString) QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("VODServiceType"));
        if (vodServiceType && SvObjectIsInstanceOf((SvObject) vodServiceType, SvString_getType())) {
            if (SvStringEqualToCString(vodServiceType, "Composite")) {
                SvIterator childIterator = QBActiveTreeNodeChildNodesIterator(menuNode);
                QBActiveTreeNode childNode = NULL;
                while ((childNode = (QBActiveTreeNode) SvIteratorGetNext(&childIterator))) {
                    SvString childVodServiceType = (SvString) QBActiveTreeNodeGetAttribute(childNode, SVSTRING("VODServiceType"));
                    if (!childVodServiceType || !SvObjectIsInstanceOf((SvGenericObject) childVodServiceType, SvString_getType()))
                        continue;

                    QBProvidersControllerServiceMStoreProcessNode(self, childNode, childVodServiceType);
                }

                continue;
            }

            QBProvidersControllerServiceMStoreProcessNode(self, menuNode, vodServiceType);
        }
    }
}

SvLocal void
QBProvidersControllerServiceMStoreStart(SvObject self_)
{
    QBProvidersControllerServiceMStore self = (QBProvidersControllerServiceMStore) self_;

    SvString nodeID = (SvString) SvHashTableFind(self->menuNodes, (SvGenericObject) SVSTRING("MSTORE"));
    if (!nodeID || !QBActiveTreeFindNode(self->appGlobals->menuTree, nodeID)) {
        SvLogWarning("%s(): can't find MStore menu node!", __func__);
        return;
    }

    QBCarouselMenuItemService mstoreMenu =
        QBMStoreCarouselMenuCreate(self->appGlobals, self->path, NULL);
    if (mstoreMenu) {
        struct QBProvidersControllerServiceMountParams_t params = {
            .carouselMenu = mstoreMenu,
            .menuNodeID   = nodeID,
            .mounts       = self->mounts,
            .isUnfoldable = true,
        };
        QBProvidersControllerServiceMountService(self->appGlobals->providersController, &params);
        SVRELEASE(mstoreMenu);
    } else {
        SvLogError("%s(): can't create carousel menu for MSTORE service!", __func__);
    }

    self->created = true;
}

SvLocal void
QBProvidersControllerServiceMStoreStartWithCheck(SvObject self_)
{
    QBProvidersControllerServiceMStore self = (QBProvidersControllerServiceMStore) self_;

    if (self->timeSet && !self->created)
        QBProvidersControllerServiceMStoreStart(self_);
}


SvLocal void
QBProvidersControllerServiceMStoreStop(SvObject self_)
{
    QBProvidersControllerServiceMStore self = (QBProvidersControllerServiceMStore) self_;
    QBProvidersControllerServiceDestroyServices(self->appGlobals->providersController,
                                                self->mounts, self->menuNodes, NULL);
}

SvLocal void
QBProvidersControllerServiceMStoreReinitialize(SvObject self_, SvArray itemList)
{
    QBProvidersControllerServiceMStore self = (QBProvidersControllerServiceMStore) self_;

    SvArray currentMounts = (SvArray) SvHashTableFind(self->mounts, (SvGenericObject) SVSTRING("MSTORE"));
    if (!currentMounts)
        return;

    const char *lang = QBConfigGet("LANG");
    SvString langStr = SvStringCreateWithCStringAndLength(lang, 2, NULL);

    SvIterator it = SvArrayIterator(currentMounts);
    QBCarouselMenuItemService menu = NULL;
    while ((menu = (QBCarouselMenuItemService) SvIteratorGetNext(&it))) {
        SvGenericObject provider = QBCarouselMenuGetProvider(menu);
        QBContentProviderSetLanguage((QBContentProvider) provider, langStr);
    }

    SVRELEASE(langStr);
}

SvLocal QBProvidersControllerServiceType
QBProvidersControllerServiceMStoreType(SvObject self_)
{
    return QBProvidersControllerService_MSTORE;
}

SvLocal SvArray
QBProvidersControllerServiceMStoreGetServices(SvObject self_, SvString tag)
{
    QBProvidersControllerServiceMStore self = (QBProvidersControllerServiceMStore) self_;
    return (SvArray) SvHashTableFind(self->mounts, (SvGenericObject) tag);
}

SvLocal void
QBProvidersControllerServicePVODStorageConnected(SvGenericObject self_,
                                                 SvString path,
                                                 bool forced)
{
    QBProvidersControllerServiceMStore self = (QBProvidersControllerServiceMStore) self_;

    SVTESTRETAIN(path);
    SVTESTRELEASE(self->path);
    self->path = path;
    self->storageConnected = true;
    if (forced || (self->timeSet && !self->created))
        QBProvidersControllerServiceMStoreStart(self_);
}

SvLocal void
QBProvidersControllerServicePVODStorageDisconnected(SvGenericObject self_)
{
    QBProvidersControllerServiceMStore self = (QBProvidersControllerServiceMStore) self_;

    SVTESTRELEASE(self->path);
    self->path = NULL;
    self->storageConnected = false;
    if (self->created) {
        QBProvidersControllerServiceDestroyServices(self->appGlobals->providersController,
                                                    self->mounts, self->menuNodes, NULL);
        self->created = false;
    }
}

SvLocal void
QBProvidersControllerServiceTimeSet(SvGenericObject self_, bool firstTime, QBTimeDateUpdateSource source)
{
    QBProvidersControllerServiceMStore self = (QBProvidersControllerServiceMStore) self_;

    if (firstTime && source == QBTimeDateUpdateSource_DVB)
        self->timeSet = true;

    if (self->storageConnected && !self->created)
        QBProvidersControllerServiceMStoreStart(self_);
}

SvLocal void
QBProvidersControllerServiceMStoreRegisterSubmenuFactory(SvObject self_, SvString id, SvObject submenuFactory)
{
    SvLogError("%s(): not yet implemented", __func__);
}

SvLocal void
QBProvidersControllerServiceMStore__dtor__(void *self_)
{
    QBProvidersControllerServiceMStore self = self_;

    if (self->appGlobals->pvodStorage) {
        log_debug("Remove QBPVODStorageListener");
        QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                            SVSTRING("QBTimeDateMonitor"));
        QBTimeDateMonitorRemoveListener(timeDateMonitor, (SvObject) self, NULL);
        QBPVODStorageServiceRemoveListener(self->appGlobals->pvodStorage, (SvGenericObject) self);
    }

    SVRELEASE(self->menuNodes);
    SVRELEASE(self->mounts);
    SVTESTRELEASE(self->path);
}

SvLocal SvType
QBProvidersControllerServiceMStore_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        static const struct SvObjectVTable_ objectVTable = {
            .destroy = QBProvidersControllerServiceMStore__dtor__
        };
        static struct QBPVODStorageListener_ pvodStorageMethods = {
            .storageConnected    = QBProvidersControllerServicePVODStorageConnected,
            .storageDisconnected = QBProvidersControllerServicePVODStorageDisconnected
        };
        static struct QBTimeDateMonitorListener_ timeDateListenerMethods = {
            .systemTimeSet = QBProvidersControllerServiceTimeSet
        };
        static const struct QBProvidersControllerServiceChild_t serviceMethods = {
            .start                      = QBProvidersControllerServiceMStoreStartWithCheck,
            .stop                       = QBProvidersControllerServiceMStoreStop,
            .addNode                    = QBProvidersControllerServiceMStoreAddMenuNode,
            .reinitialize               = QBProvidersControllerServiceMStoreReinitialize,
            .getType                    = QBProvidersControllerServiceMStoreType,
            .getServices                = QBProvidersControllerServiceMStoreGetServices,
            .registerSubmenuFactory     = QBProvidersControllerServiceMStoreRegisterSubmenuFactory,
        };

        SvTypeCreateManaged("QBProvidersControllerServiceMStore",
                            sizeof(struct QBProvidersControllerServiceMStore_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPVODStorageListener_getInterface(), &pvodStorageMethods,
                            QBProvidersControllerServiceChild_getInterface(), &serviceMethods,
                            QBTimeDateMonitorListener_getInterface(), &timeDateListenerMethods,
                            NULL);
    }
    return type;
}

SvObject
QBProvidersControllerServiceMStoreCreate(AppGlobals appGlobals)
{
    QBProvidersControllerServiceMStore self;

    self = (QBProvidersControllerServiceMStore) SvTypeAllocateInstance(QBProvidersControllerServiceMStore_getType(), NULL);
    if (!self)
        return NULL;

    self->appGlobals = appGlobals;
    self->menuNodes = SvHashTableCreate(3, NULL);
    self->mounts = SvHashTableCreate(3, NULL);

    if (appGlobals->pvodStorage) {
        log_debug("Add QBPVODStorageListener");
        QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                            SVSTRING("QBTimeDateMonitor"));
        QBTimeDateMonitorAddListener(timeDateMonitor, (SvObject) self, NULL);
        QBPVODStorageServiceAddListener(self->appGlobals->pvodStorage, (SvGenericObject) self);
        self->path = QBPVODStorageServiceGetStoragePath(self->appGlobals->pvodStorage, SVSTRING("MStoreVoD"));
    }

    return (SvObject)self;
}
