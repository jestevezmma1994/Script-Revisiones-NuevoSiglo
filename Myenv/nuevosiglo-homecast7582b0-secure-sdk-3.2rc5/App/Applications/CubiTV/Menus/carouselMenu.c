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

#include "carouselMenu.h"
#include "carouselMenuPrivate.h"

#include <QBSecureLogManager.h>
#include <libintl.h>
#include <settings.h>
#include "QBCarousel/QBVerticalCarousel.h"
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentProviderListener.h>
#include <QBContentManager/QBContentProvider.h>
#include <Windows/mainmenu.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <Menus/menuchoice.h>
#include <Widgets/XMBCarousel.h>
#include <XMB2/XMBMenuBar.h>
#include <Widgets/confirmationDialog.h>
#include <QBWidgets/QBDialog.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <Utils/authenticators.h>
#include <DataModels/loadingProxy.h>
#include <Services/QBProvidersControllerService.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <main.h>
#include <QBJSONUtils.h>

// defines the depth of the path to look for provider services
// i.e. if provider is mounted in main menu, proper depth is 2,
// if it should be mounted in node already present in main menu
// the depth shuld be 3, and so on.
#define PROVIDER_SEARCH_DEPTH 3

#define log_debug(fmt, ...)  do { if(1) SvLogNotice(COLBEG() "%s:%d " fmt  COLEND_COL(blue), __func__,__LINE__,##__VA_ARGS__); } while (0)

SvLocal void QBCarouselMenuItemServiceDestroy(void *self_)
{
    QBCarouselMenuItemService self = self_;

    SVTESTRELEASE(self->path);

    SVTESTRELEASE(self->service);
    SVTESTRELEASE(self->serviceType);
    SVTESTRELEASE(self->serviceName);
    SVTESTRELEASE(self->serviceLayout);

    SVRELEASE(self->provider);
    SVRELEASE(self->itemController);
    SVRELEASE(self->itemChoice);
    SVTESTRELEASE(self->skinController);
    SVRELEASE(self->contentTree);
}

SvLocal void QBCarouselMenuItemServiceMounted_(QBCarouselMenuItemService self)
{
}

SvLocal void QBCarouselMenuItemServiceUnmounted_(QBCarouselMenuItemService self)
{
}

SvLocal void QBCarouselMenu_dialogCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBCarouselMenuItemService self = self_;

    self->errorDialog = NULL;
}

SvLocal const char* QBCarouselMenuGetCString(SvString string) {
    return string ? SvStringCString(string) : "unknown";
}

SvLocal void
QBCarouselMenu_providerErrorOccured(SvGenericObject self_, SvErrorInfo errorInfo)
{
    QBCarouselMenuItemService self = (QBCarouselMenuItemService) self_;

    SvErrorInfoWriteLogMessage(errorInfo);

    if (self->topLevel)
        return;

    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvInvokeInterface(QBMenu, menuBar->prv, setPosition, self->path, NULL);

    if (self->errorDialog)
        return;

    svSettingsRestoreContext(self->settingsCtx);

    QBConfirmationDialogParams_t params = {
        .title = gettext("Error"),
        .message = SvErrorInfoGetMessage(errorInfo),
        .local = true,
        .focusOK = true,
        .isCancelButtonVisible = false
    };

    self->errorDialog = QBConfirmationDialogCreate(self->appGlobals->res, &params);

    SvLogNotice("Self error dialog: %p", self->errorDialog);

    if (self->errorDialog) {
        char* escapedMsg = QBStringCreateJSONEscapedString(params.message);
        QBSecureLogEvent("CarouselMenu", "ErrorShown.Menus.ProviderError", "JSON:{\"description\":\"%s\",\"serviceID\":\"%s\",\"serviceType\":\"%s\",\"serviceName\":\"%s\"}",
                         escapedMsg,
                         QBCarouselMenuGetCString(self->service),
                         QBCarouselMenuGetCString(self->serviceType),
                         QBCarouselMenuGetCString(self->serviceName));
        free(escapedMsg);
        QBDialogRun(self->errorDialog, (void *) self, QBCarouselMenu_dialogCallback);
    }

    svSettingsPopComponent();
}

SvLocal void
QBCarouselMenu_providerActivityStarted(SvObject self_, QBContentCategory category)
{
}

SvLocal void
QBCarouselMenu_providerActivityEnded(SvObject self_, QBContentCategory category)
{
}

SvLocal void QBCarouselMenu_subtreeEntered(SvGenericObject self_)
{
    QBCarouselMenuItemService self = (QBCarouselMenuItemService) self_;
    self->focused = true;
    if (!self->topLevel) {
        QBContentProviderAddListener((QBContentProvider) self->provider, (SvObject) self);
        QBContentProviderStart((QBContentProvider) self->provider, self->appGlobals->scheduler);
    }
    svSettingsRestoreContext(self->settingsCtx);
    svSettingsPopComponent();
}

SvLocal void QBCarouselMenu_subtreeLeft(SvGenericObject self_)
{
    QBCarouselMenuItemService self = (QBCarouselMenuItemService) self_;

    if (!self->topLevel) {
        QBContentProviderRemoveListener((QBContentProvider) self->provider, (SvObject) self);
        QBContentProviderStop((QBContentProvider) self->provider);
    }
    if (self->clearOnFocusLost) {
        QBContentProviderClear((QBContentProvider) self->provider);
    }

    self->focused = false;
}

SvType
QBCarouselMenuItemService_getType(void)
{
    static const struct QBCarouselMenuItemServiceVTable_ objectVTable = {
        .super_    = {
            .destroy = QBCarouselMenuItemServiceDestroy
        },
        .mounted   = QBCarouselMenuItemServiceMounted_,
        .unmounted = QBCarouselMenuItemServiceUnmounted_
    };
    static SvType type = NULL;

    static const struct QBSubtreeNotifierListener_t subtreeNotifierMethods = {
        .subtreeEntered = QBCarouselMenu_subtreeEntered,
        .subtreeLeft = QBCarouselMenu_subtreeLeft,
    };

    static const struct QBContentProviderListener_ providerListenerMethods = {
        .errorOccured    = QBCarouselMenu_providerErrorOccured,
        .activityStarted = QBCarouselMenu_providerActivityStarted,
        .activityEnded   = QBCarouselMenu_providerActivityEnded
    };

    if (unlikely(!type)) {
        SvTypeCreateVirtual("QBCarouselMenuItemService",
                            sizeof(struct QBCarouselMenuItemService_),
                            SvObject_getType(),
                            sizeof(objectVTable), &objectVTable,
                            &type,
                            QBSubtreeNotifierListener_getInterface(), &subtreeNotifierMethods,
                            QBContentProviderListener_getInterface(), &providerListenerMethods,
                            NULL);
    }

    return type;
}

SvLocal bool
QBCarouselMenuCheckIfSettingsExists(SvString settingsName)
{
    if (strstr(SvStringCString(settingsName), "/"))
        goto err1;
    if (!svSettingsIsComponentAvailable(SvStringCString(settingsName)))
        goto err2;

    return true;

err1:
    SvLogWarning("CarouselMenuPushSettings() : Incorrect path passed");
    return false;

err2:
    SvLogWarning("CarouselMenuPushSettings() : %s does not exist", SvStringCString(settingsName));
    return false;
}

int
QBCarouselMenuPushSettingsComponent(SvString serviceName, SvString serviceType)
{
    SvString tmp;
    int res = 0;

    if (serviceName) {
        tmp = SvStringCreateWithFormat("Carousel_%s.settings", SvStringCString(serviceName));
        if (QBCarouselMenuCheckIfSettingsExists(tmp))
            res = svSettingsPushComponent(SvStringCString(tmp));
        SVRELEASE(tmp);
        if (res != 0)
            return res;
    }

    if (serviceType) {
        tmp = SvStringCreateWithFormat("Carousel_%s.settings", SvStringCString(serviceType));
        if (QBCarouselMenuCheckIfSettingsExists(tmp))
            res = svSettingsPushComponent(SvStringCString(tmp));
        SVRELEASE(tmp);
        if (res != 0)
            return res;
    }

    return svSettingsPushComponent("Carousel_VOD.settings");
}

void
QBCarouselMenuInitWithParams(QBCarouselMenuItemService self, AppGlobals appGlobals, struct QBCarouselMenuInitParams_s *params)
{
    self->service = SVRETAIN(params->serviceID);
    self->serviceType = SVTESTRETAIN(params->serviceType);
    self->serviceLayout = SVTESTRETAIN(params->serviceLayout);
    self->serviceName = SVTESTRETAIN(params->serviceName);
    self->appGlobals = appGlobals;
    self->provider = SVRETAIN(params->provider);
    self->itemController = SVRETAIN(params->itemController);
    self->itemChoice = SVRETAIN(params->itemChoice);
    self->contentTree = SVRETAIN(params->contentTree);
    QBCarouselMenuPushSettingsComponent(self->serviceName, self->serviceType);
    self->settingsCtx = svSettingsSaveContext();
    svSettingsPopComponent();
}

QBCarouselMenuItemService
QBCarouselMenuCreate(AppGlobals appGlobals, SvString serviceName, SvString serviceType,
                     SvString serviceID, SvString serviceLayout,
                     SvGenericObject provider, SvGenericObject itemController,
                     SvGenericObject itemChoice, SvGenericObject skinController, SvGenericObject contentTree)
{
    QBCarouselMenuItemService self = (QBCarouselMenuItemService) SvTypeAllocateInstance(QBCarouselMenuItemService_getType(), NULL);

    struct QBCarouselMenuInitParams_s initParams;
    memset(&initParams, 0, sizeof(initParams));

    initParams.serviceName = serviceName;
    initParams.serviceType = serviceType;
    initParams.serviceID = serviceID;
    initParams.serviceLayout = serviceLayout;
    initParams.provider = provider;
    initParams.itemController = itemController;
    initParams.itemChoice = itemChoice;
    initParams.contentTree = (SvGenericObject) contentTree;

    QBCarouselMenuInitWithParams(self, appGlobals, &initParams);

    return self;
}

int
QBCarouselMenuMount(QBCarouselMenuItemService self, SvWidget menuBar, SvGenericObject path, QBTreePathMap pathMap, QBActiveTree externalTree, QBTreePathMap skinPathMap)
{
    QBActiveTree tree = externalTree;
    if (!tree)
        tree = self->appGlobals->menuTree;

    QBActiveTreeNode node = QBActiveTreeFindNodeByPath(tree, path);
    if (!node) {
        log_debug("No node");
        return -1;
    }

    svSettingsRestoreContext(self->settingsCtx);

    self->path = SvObjectCopy(path, NULL);

    QBActiveTreeMountSubTree(tree, self->contentTree, path, NULL);

    if (SvInvokeInterface(QBTreePath, path, getLength) < PROVIDER_SEARCH_DEPTH)
        self->topLevel = true;

    if (self->topLevel) {
        QBContentProviderAddListener((QBContentProvider) self->provider, (SvObject) self);
        QBContentProviderStart((QBContentProvider) self->provider, self->appGlobals->scheduler);
    }

    if (self->skinController)
        QBTreePathMapInsert(skinPathMap, path, self->skinController, NULL);

    if (pathMap)
        QBTreePathMapInsert(pathMap, path, self->itemChoice, NULL);

    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, self->itemController, NULL);
    SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, path, self->itemChoice, NULL);

    SvObject parentPath = SvObjectCopy(path, NULL);

    if (SvInvokeInterface(QBTreePath, path, getLength) > 1) {
        SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
    }

    if (parentPath) {
        QBSubtreeNotifierAddListener(self->appGlobals->subtreeNotifier, parentPath, (SvGenericObject) self);
        SVRELEASE(parentPath);
    }

    svSettingsPopComponent();

    self->ownerTree = tree;

    QBCarouselMenuItemServiceMounted(self);

    return 0;
}

int
QBCarouselMenuUnmount(QBCarouselMenuItemService self, SvWidget menuBar, QBTreePathMap pathMap, QBTreePathMap skinPathMap)
{
    if (!self->path) {
        SvLogError("%s: service %s not mounted", __func__, SvStringCString(self->service));
        return -1;
    }

    QBCarouselMenuItemServiceUnmounted(self);

    if (self->focused) {
        self->focused = false;
    }

    if (self->contentTree)
        QBActiveTreeUmountSubTree(self->appGlobals->menuTree, self->contentTree, NULL);

    if (self->provider) {
        QBContentProviderRemoveListener((QBContentProvider) self->provider, (SvObject) self);
        QBContentProviderStop((QBContentProvider) self->provider);
    }

    if (pathMap) {
        SvErrorInfo error = NULL;
        QBTreePathMapRemove(pathMap, self->path, &error);
        if (error) {
            SvErrorInfoDestroy(error);
        }
    }
    if (skinPathMap) {
        SvErrorInfo error = NULL;
        QBTreePathMapRemove(skinPathMap, self->path, &error);
        if (error) {
            SvErrorInfoDestroy(error);
        }
    }

    if (menuBar) {
        SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, self->path, NULL, NULL);
        SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, self->path, NULL, NULL);
    }

    SvObject parentPath = SvObjectCopy(self->path, NULL);

    if (SvInvokeInterface(QBTreePath, self->path, getLength) > 1) {
        SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
    }

    if (parentPath) {
        QBSubtreeNotifierRemoveListener(self->appGlobals->subtreeNotifier, parentPath, (SvObject) self);
        SVRELEASE(parentPath);
    }

    SVRELEASE(self->path);
    self->path = NULL;

    self->ownerTree = NULL;

    return 0;
}

SvGenericObject QBCarouselMenuGetPath(QBCarouselMenuItemService self)
{
    if (self)
        return self->path;
    return NULL;
}

bool QBCarouselMenuIsMounted(QBCarouselMenuItemService self)
{
    if (!self)
        return false;
    return self->path;
}

SvString QBCarouselMenuGetServiceID(QBCarouselMenuItemService self)
{
    return self->service;
}

SvString QBCarouselMenuGetServiceName(QBCarouselMenuItemService self)
{
    return self->serviceName;
}

SvString QBCarouselMenuGetServiceLayout(QBCarouselMenuItemService self)
{
    return self->serviceLayout;
}

QBActiveTree QBCarouselMenuGetOwnerTree(QBCarouselMenuItemService self)
{
    return self->ownerTree;
}

SvGenericObject QBCarouselMenuGetProvider(QBCarouselMenuItemService self)
{
    return self->provider;
}


bool QBCarouselMenuPushContext(QBCarouselMenuItemService self, QBActiveTree externalTree)
{
    if (!self->path)
        return false;

    if (self->itemController && SvStringEqualToCString(self->serviceType, "MWClient")) {
        return SvInvokeInterface(QBMenuEventHandler, self->itemController, nodeSelected, (SvObject) self, self->path);
    } else if (SvStringEqualToCString(self->serviceType, "VOD")) {
        QBContentProviderStart((QBContentProvider) self->provider,
                               self->appGlobals->scheduler);
        QBWindowContext ctx =
            QBGUILogicPrepareVoDContext(self->appGlobals->guiLogic,
                                        self->serviceLayout, self->path, self->provider,
                                        self->service, self->serviceType, false, externalTree);
        if (ctx) {
            QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
            QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
            SVRELEASE(ctx);
        } else {
            return false;
        }

        return true;
    }

    return false;
}
