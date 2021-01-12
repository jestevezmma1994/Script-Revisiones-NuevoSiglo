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

#include "QBProvidersControllerService.h"

#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreePath.h>
#include <stdlib.h>
#include <main.h>
#include <settings.h>
#include <QBConf.h>
#include <XMB2/XMBMenuBar.h>
#include <Windows/mainmenu.h>
#include <Menus/menuchoice.h>
#include <Utils/value.h>
#include <Services/core/QBMiddlewareManager.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBTimeDateMonitor.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBMenu/QBMenu.h>
#include "QBProvidersControllerServiceDLNA.h"
#include "QBProvidersControllerServiceMStore.h"
#include "QBProvidersControllerServiceTraxis.h"
#include "QBProvidersControllerServiceInnov8on.h"
#include "QBProvidersControllerServiceMWClient.h"
#include "QBProvidersControllerServiceDSMCC.h"

#define log_error(fmt, ...)                                                 \
    do { if (1) { SvLogError("%s() :: " fmt, __func__, ##__VA_ARGS__); } } while (0)
#define log_debug(fmt, ...)                                                 \
    do { if (0) { SvLogNotice(COLBEG() "%s() :: " fmt COLEND_COL(blue), __func__, ##__VA_ARGS__); } } while (0)

struct QBProvidersControllerService_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    bool menuScanned;

    SvArray providers;
};

SvInterface QBProvidersControllerServiceChild_getInterface(void)
{
    static SvInterface interface = NULL;
    if (unlikely(!interface)) {
        SvInterfaceCreateManaged("QBProvidersControllerServiceChild",
                                 sizeof(struct QBProvidersControllerServiceChild_t),
                                 NULL, &interface, NULL);
    }
    return interface;
}

SvInterface
QBProvidersControllerServiceObservable_getInterface(void)
{
    static SvInterface interface = NULL;
    if (unlikely(!interface)) {
        SvInterfaceCreateManaged("QBProvidersControllerServiceObservable",
                                 sizeof(struct QBProvidersControllerServiceObservable_t),
                                 NULL, &interface, NULL);
    }
    return interface;
}

SvLocal void
QBProvidersControllerService__dtor__(void *self_)
{
    QBProvidersControllerService self = self_;

    SVRELEASE(self->providers);
}

SvLocal void
QBProvidersControllerServiceScanMenu(QBProvidersControllerService self)
{
    assert(self);

    SvObject provider;
    SvIterator it = SvArrayGetIterator(self->providers);
    while ((provider = SvIteratorGetNext(&it)))
        SvInvokeInterface(QBProvidersControllerServiceChild, provider, addNode);
}

void
QBProvidersControllerServiceDoMount(AppGlobals appGlobals, QBCarouselMenuItemService carouselMenu, SvGenericObject path, QBActiveTree externalTree)
{
    SvWidget menuBar = SvInvokeInterface(QBMainMenu, appGlobals->main, getMenu);
    QBTreePathMap pathMap = NULL;
    if (externalTree == appGlobals->menuTree) {
        pathMap = SvInvokeInterface(QBMainMenu, appGlobals->main, getPathMap);
    }
    QBTreePathMap skinPathMap = SvInvokeInterface(QBMainMenu, appGlobals->main, getSkinPathMap);
    QBCarouselMenuMount(carouselMenu, menuBar, path, pathMap, externalTree, skinPathMap);
}

void
QBProvidersControllerServiceDoUnmount(AppGlobals appGlobals, QBCarouselMenuItemService carouselMenu)
{
    SvWidget menuBar = SvInvokeInterface(QBMainMenu, appGlobals->main, getMenu);
    QBTreePathMap pathMap = SvInvokeInterface(QBMainMenu, appGlobals->main, getPathMap);
    QBTreePathMap skinPathMap = SvInvokeInterface(QBMainMenu, appGlobals->main, getSkinPathMap);
    QBCarouselMenuUnmount(carouselMenu, menuBar, pathMap, skinPathMap);
}

void
QBProvidersControllerServiceUnmountService(QBProvidersControllerService self, QBProvidersControllerServiceMountParams params)
{
    if (!self || !params || !params->carouselMenu || !params->menuNodeID || !params->mounts)
        return;

    assert(params->isUnfoldable);

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
    SVRELEASE(menuNodePath);

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
}

SvLocal void
QBProvidersControllerServiceUnmountTag(QBProvidersControllerService self, SvString menuNodeID, SvHashTable mounts, SvHashTable services)
{
    SvArray currentMounts = (SvArray) SvHashTableFind(mounts, (SvGenericObject) menuNodeID);
    if (!currentMounts || SvArrayCount(currentMounts) <= 0)
        return;

    QBCarouselMenuItemService menu;
    while (SvArrayCount(currentMounts)) {
        menu = (QBCarouselMenuItemService) SvArrayAt(currentMounts, SvArrayCount(currentMounts) - 1);
        struct QBProvidersControllerServiceMountParams_t params = {
            .carouselMenu  = menu,
            .menuNodeID    = menuNodeID,
            .mounts        = mounts,
            .services      = services,
            .isUnfoldable  = true,
        };
        QBProvidersControllerServiceUnmountService(self, &params);
    }
}

void
QBProvidersControllerServiceMountService(QBProvidersControllerService self,
                                         QBProvidersControllerServiceMountParams params)
{
    assert(params->isUnfoldable);

    QBActiveTree tree = params->externalTree;
    if (!params->externalTree)
        tree = self->appGlobals->menuTree;

    SvObject rootPath = QBActiveTreeCreateNodePath(tree, params->menuNodeID);
    if (!rootPath)
        return;

    SvArray currentMounts = (SvArray) SvHashTableFind(params->mounts, (SvGenericObject) params->menuNodeID);
    if (!currentMounts) {
        SvArray curMounts = SvArrayCreateWithCapacity(3, NULL);
        SvHashTableInsert(params->mounts, (SvGenericObject) params->menuNodeID, (SvGenericObject) curMounts);
        SVRELEASE(curMounts);
        currentMounts = (SvArray) SvHashTableFind(params->mounts, (SvGenericObject) params->menuNodeID);
    }
    assert(SvArrayCount(currentMounts) == 0);

    SvArrayAddObject(currentMounts, (SvGenericObject) params->carouselMenu);

    QBProvidersControllerServiceDoMount(self->appGlobals, params->carouselMenu, rootPath, tree);

    SVRELEASE(rootPath);
}

void
QBProvidersControllerServiceDestroyServices(QBProvidersControllerService self, SvHashTable mounts, SvHashTable menuNodes, SvHashTable services)
{
    SvString menuNodeID;
    SvIterator it = SvHashTableValuesIterator(menuNodes);
    while ((menuNodeID = (SvString) SvIteratorGetNext(&it))) {
        QBProvidersControllerServiceUnmountTag(self, menuNodeID, mounts, services);
    }
}

SvLocal void
QBProvidersControllerServiceClear(QBProvidersControllerService self);

SvLocal void
QBProvidersControllerServiceConfigChanged(SvObject self_, const char *key, const char *value)
{
    QBProvidersControllerService self = (QBProvidersControllerService) self_;
    if (strncmp(key, "LANG", 4)) {
        SvString langCode = SvStringCreateWithCStringAndLength(value, 2, NULL);
        TraxisWebSessionManagerSetLanguage(self->appGlobals->traxisWebSessionManager, langCode, NULL);
        SVRELEASE(langCode);

        QBProvidersControllerServiceClear(self->appGlobals->providersController);
    }
}

SvLocal SvType
QBProvidersControllerService_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBProvidersControllerService__dtor__
    };
    static SvType type = NULL;

    if (!type) {
        static const struct QBConfigListener_t configListener = {
            .changed = QBProvidersControllerServiceConfigChanged,
        };
        SvTypeCreateManaged("QBProvidersControllerService",
                            sizeof(struct QBProvidersControllerService_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBConfigListener_getInterface(), &configListener,
                            NULL);
    }

    return type;
}

void
QBProvidersControllerServiceAddProvider(QBProvidersControllerService self, QBProvidersControllerServiceType type)
{
    assert(self);
    assert(!self->menuScanned);

    SvObject provider = NULL;
    SvIterator it = SvArrayGetIterator(self->providers);
    while ((provider = SvIteratorGetNext(&it)))
        if (type == SvInvokeInterface(QBProvidersControllerServiceChild, provider, getType))
            return;

    provider = NULL;
    switch (type) {
    case QBProvidersControllerService_DLNA:
        provider = QBProvidersControllerServiceDLNACreate(self->appGlobals);
        break;
    case QBProvidersControllerService_MSTORE:
        provider = QBProvidersControllerServiceMStoreCreate(self->appGlobals);
        break;
    case QBProvidersControllerService_INNOV8ON:
        provider = QBProvidersControllerServiceInnov8onCreate(self->appGlobals);
        break;
    case QBProvidersControllerService_TRAXIS:
        provider = QBProvidersControllerServiceTraxisCreate(self->appGlobals);
        break;
    case QBProvidersControllerService_MWClient:
        provider = QBProvidersControllerServiceMWClientCreate(self->appGlobals);
        break;
    case QBProvidersControllerService_DSMCC:
        provider = QBProvidersControllerServiceDSMCCCreate(self->appGlobals);
        break;
    default:
        SvLogError("Invalid provider");
        break;
    }
    if (provider) {
        SvArrayAddObject(self->providers, provider);
        SVRELEASE(provider);
    }
}

QBProvidersControllerService
QBProvidersControllerServiceCreate(AppGlobals appGlobals)
{
    QBProvidersControllerService self = (QBProvidersControllerService) SvTypeAllocateInstance(QBProvidersControllerService_getType(), NULL);
    self->appGlobals = appGlobals;
    self->providers = SvArrayCreateWithCapacity(2, NULL);
    QBConfigAddListener((SvGenericObject) self, "LANG");
    return self;
}

void
QBProvidersControllerServiceStart(QBProvidersControllerService self)
{
    if (!self->menuScanned) {
        self->menuScanned = true;
        QBProvidersControllerServiceScanMenu(self);
    }

    if (!self->appGlobals->middlewareManager) {
        SvLogError("%s(): failed, QBMiddlewareManager is unavailable!", __func__);
        return;
    }

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    if (!timeDateMonitor) {
        log_error("failed, QBTimeDateMonitor is unavailable!");
        return;
    }

    SvObject provider;
    SvIterator it = SvArrayGetIterator(self->providers);
    while ((provider = SvIteratorGetNext(&it)))
        SvInvokeInterface(QBProvidersControllerServiceChild, provider, start);
}

void
QBProvidersControllerServiceStop(QBProvidersControllerService self)
{
    SvObject provider;
    SvIterator it = SvArrayGetIterator(self->providers);
    while ((provider = SvIteratorGetNext(&it)))
        SvInvokeInterface(QBProvidersControllerServiceChild, provider, stop);
}

SvLocal void
QBProvidersControllerServiceClear(QBProvidersControllerService self)
{
    QBProvidersControllerServiceStop(self);
    QBProvidersControllerServiceStart(self);
}

SvLocal SvObject
QBProvidersControllerServiceGetProviders(QBProvidersControllerService self, enum QBProvidersControllerServiceType type)
{
    SvObject providers = NULL;
    SvIterator it = SvArrayGetIterator(self->providers);
    while ((providers = SvIteratorGetNext(&it)))
        if (SvInvokeInterface(QBProvidersControllerServiceChild, providers, getType) == type)
            break;
    return providers;
}

SvArray
QBProvidersControllerServiceGetServices(QBProvidersControllerService self, SvString tag,
                                        enum QBProvidersControllerServiceType type)
{
    SvObject providers = QBProvidersControllerServiceGetProviders(self, type);
    if (!providers)
        return NULL;
    return SvInvokeInterface(QBProvidersControllerServiceChild, providers, getServices, tag);
}

void QBProvidersControllerServiceReinitialize(QBProvidersControllerService self,
                                              SvArray itemList)
{
    SvObject providers = NULL;
    SvIterator it = SvArrayGetIterator(self->providers);
    while ((providers = SvIteratorGetNext(&it)))
        SvInvokeInterface(QBProvidersControllerServiceChild, providers, reinitialize, itemList);
}

void
QBProvidersControllerServiceAddListener(QBProvidersControllerService self,
                                        SvGenericObject listener,
                                        enum QBProvidersControllerServiceType type)
{
    if (!self || !listener) {
        SvLogError("%s() failed. NULL arguments passed", __func__);
        return;
    }

    SvObject providers = QBProvidersControllerServiceGetProviders(self, type);
    if (!providers || !SvObjectIsImplementationOf(providers, QBProvidersControllerServiceObservable_getInterface()))
        return;

    if (SvObjectIsImplementationOf(listener, QBProvidersControllerServiceListener_getInterface())) {
        SvInvokeInterface(QBProvidersControllerServiceObservable, providers, addListener, listener);
    } else {
        SvLogWarning("%s(): listener %p of type %s doesn't implement QBProvidersControllerServiceListener interface", __func__, listener, listener ? SvObjectGetTypeName(listener) : "--");
    }
}

void
QBProvidersControllerServiceRemoveListener(QBProvidersControllerService self,
                                           SvGenericObject listener,
                                           enum QBProvidersControllerServiceType type)
{
    if (!self || !listener) {
        SvLogError("%s() failed. NULL arguments passed", __func__);
        return;
    }

    SvObject providers = QBProvidersControllerServiceGetProviders(self, type);
    if (!providers || !SvObjectIsImplementationOf(providers, QBProvidersControllerServiceObservable_getInterface()))
        return;
    SvInvokeInterface(QBProvidersControllerServiceObservable, providers, removeListener, listener);
}

SvInterface
QBProvidersControllerServiceListener_getInterface(void)
{
    static SvInterface interface = NULL;
    if(!interface) {
        SvInterfaceCreateManaged("QBProvidersControllerServiceListener", sizeof(struct QBProvidersControllerServiceListener_t),
                                 NULL, &interface, NULL);
    }
    return interface;
}

void
QBProvidersControllerServiceRegisterSubmenuFactory(QBProvidersControllerService self,
                                                   QBProvidersControllerServiceType type,
                                                   SvString id,
                                                   SvObject submenuFactory)
{
    SvObject provider = QBProvidersControllerServiceGetProviders(self,  type);
    if (!provider) {
        SvLogError("%s(): no provider found for given type", __func__);
        return;
    }
    SvInvokeInterface(QBProvidersControllerServiceChild, provider, registerSubmenuFactory, id, submenuFactory);
}

SvInterface
QBSubmenuFactory_getInterface(void)
{
    static SvInterface interface = NULL;

    if (!interface) {
        SvErrorInfo error = NULL;
        SvInterfaceCreateManaged("QBSubmenuFactory",
                                 sizeof(struct QBSubmenuFactory_),
                                 NULL, &interface, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            abort();
        }
    }
    return interface;
}

// BEGIN AMERELES Update VoD Services
void QBProvidersControllerServiceUpdateVODServices(QBProvidersControllerService self)
{
    SvObject provider = QBProvidersControllerServiceGetProviders(self, QBProvidersControllerService_INNOV8ON);

    if (provider)
    {
        QBProvidersControllerServiceInnov8onUpdateVODServices(provider);
    }
}

int QBProvidersControllerServiceGetUpdateVODServicesCount(QBProvidersControllerService self)
{
    SvObject provider = QBProvidersControllerServiceGetProviders(self, QBProvidersControllerService_INNOV8ON);

    if (provider)
    {
        return QBProvidersControllerServiceInnov8onGetUpdateVODServicesCount(provider);
    }
    else
    {
        return 0;
    }
}

void QBProvidersControllerServiceSetUpdateVODServicesCount(QBProvidersControllerService self, int val)
{
    SvObject provider = QBProvidersControllerServiceGetProviders(self, QBProvidersControllerService_INNOV8ON);

    if (provider)
    {
        QBProvidersControllerServiceInnov8onSetUpdateVODServicesCount(provider, val);
    }
}
// END AMERELES Update VoD Services

// AMERELES #2206 Canal lineal que lleve a VOD
SvObject QBProvidersControllerServiceGetServiceByID(QBProvidersControllerService self, SvString tag, SvString serviceID)
{
    SvObject provider = QBProvidersControllerServiceGetProviders(self, QBProvidersControllerService_INNOV8ON);

    if (provider)
    {
        return QBProvidersControllerServiceInnov8onGetInnov8onServiceByID(provider, tag, serviceID);
    }
    else
    {
        return NULL;
    }
}
// AMERELES #2206 Canal lineal que lleve a VOD
