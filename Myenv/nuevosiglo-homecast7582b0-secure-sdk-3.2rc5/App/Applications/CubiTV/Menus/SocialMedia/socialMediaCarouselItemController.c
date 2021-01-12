/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#include "socialMediaCarouselItemController.h"

#include <libintl.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include "QBCarousel/QBVerticalCarousel.h"
#include "Windows/pvrplayer.h"
#include <Innov8on.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBContentManager/QBContentStub.h>
#include <Windows/mainmenu.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Menus/menuchoice.h>
#include <Menus/carouselMenu.h>
#include <Menus/carouselMenuPrivate.h>
#include <Widgets/XMBCarousel.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuController.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <Widgets/movieInfo.h>
#include <Widgets/extendedInfo.h>
#include <Utils/authenticators.h>
#include <Utils/value.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <SWL/QBFrame.h>
#include <DataModels/loadingProxy.h>
#include <QBConf.h>
#include <Services/core/appState.h>
#include <Services/QBAuthenticationService.h>
#include <ctype.h>
#include <main.h>
#include <QBContentManager/QBLoadingStub.h>

#include "socialMediaCarouselItemChoice.h"
#include "socialMediaInfo.h"

struct QBSocialMediaCarouselMenuItemController_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvBitmap icon;
    QBFrameConstructor *focus;
    QBFrameConstructor *inactiveFocus;
    QBXMBItemConstructor itemConstructor;
    unsigned int settingsCtx;

    SvGenericObject provider;

    SvRID iconRID, searchIconRID;

    SvString service;
};

struct QBSocialMediaCarouselMenuItemService_ {
    struct QBCarouselMenuItemService_ super_;

    bool requiresAuthentication;
};

SvLocal void
QBSocialMediaCarouselMenuItemServiceCurrentUserChanged(SvGenericObject self_, SvString userName)
{
    QBSocialMediaCarouselMenuItemService self = (QBSocialMediaCarouselMenuItemService) self_;
    AppGlobals appGlobals = ((QBCarouselMenuItemService) self)->appGlobals;

    if (!self->requiresAuthentication)
        return;

    int serviceId = atoi(SvStringCString(QBCarouselMenuGetServiceID((QBCarouselMenuItemService) self)));
    SvString login = NULL, password = NULL;
    QBAuthenticationServiceGetCredentials(appGlobals->authenticationService,
                                          serviceId, &login, &password, NULL);
    if (!login || !password)
        return;

    SvGenericObject provider = QBCarouselMenuGetProvider((QBCarouselMenuItemService) self);
    Innov8onProviderParams params = Innov8onProviderGetParams((Innov8onProvider) provider, NULL);

    Innov8onProviderParamsSetLogin(params, login);
    Innov8onProviderParamsSetPassword(params, password);
}

SvLocal void QBSocialMediaCarouselMenuItemServiceDestroy(void *self_)
{
}

SvLocal SvType
QBSocialMediaCarouselMenuItemService_getType(void)
{
    static const struct QBCarouselMenuItemServiceVTable_ serviceVTable = {
        .super_ = {
            .destroy = QBSocialMediaCarouselMenuItemServiceDestroy
        }
    };
    static SvType type = NULL;

    static const struct QBAuthenticationServiceListener_t authenticationMethods = {
        .currentUserChanged = QBSocialMediaCarouselMenuItemServiceCurrentUserChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSocialMediaCarouselMenuItemService",
                            sizeof(struct QBSocialMediaCarouselMenuItemService_),
                            QBCarouselMenuItemService_getType(),
                            &type,
                            QBCarouselMenuItemService_getType(), &serviceVTable,
                            QBAuthenticationServiceListener_getInterface(), &authenticationMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBSocialMediaCarouselMenuItemControllerDestroy(void *self_)
{
    QBSocialMediaCarouselMenuItemController self = self_;
    SVTESTRELEASE(self->icon);
    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    if (self->itemConstructor)
       QBXMBItemConstructorDestroy(self->itemConstructor);
    if (self->provider) {
        QBContentProviderStop((QBContentProvider) self->provider);
        SVRELEASE(self->provider);
    }
}

SvLocal SvWidget
QBSocialMediaCarouselMenuItemControllerCreateItem(SvGenericObject self_,
                                                  SvGenericObject node_,
                                                  SvGenericObject path,
                                                  SvApplication app,
                                                  XMBMenuState initialState)
{
    QBSocialMediaCarouselMenuItemController self = (QBSocialMediaCarouselMenuItemController) self_;

    QBXMBItem item = QBXMBItemCreate();

    item->caption = SVSTRING("????");
    item->loadingRID = self->iconRID;
    SvString titleIconURI = NULL;

    if (SvObjectIsInstanceOf(node_, QBContentSearch_getType())) {
        item->caption = SvStringCreate(gettext("Search"), NULL);
        item->iconRID = self->searchIconRID;
    } else if (SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        QBContentCategory node = (QBContentCategory) node_;
        SvString title = SvValueTryGetString((SvValue) QBContentCategoryGetAttribute(node, SVSTRING("shortTitle")));
        if (!title)
            title = QBContentCategoryGetTitle(node);

        item->caption = SVRETAIN(title);
        titleIconURI = SvValueTryGetString((SvValue)QBContentCategoryGetAttribute(node, SVSTRING("titleIconURI")));

        SvValue subcaptionV = (SvValue) QBContentCategoryGetAttribute(node, SVSTRING("subcaption"));
        SvString subcaption = SvValueTryGetString(subcaptionV);
        item->subcaption = SVTESTRETAIN(subcaption);

    } else if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType())) {
        SvDBRawObject leaf = (SvDBRawObject)node_;
        SvString title = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue(leaf, "name"));
        if (title) {
            item->caption = SVRETAIN(title);
        }
        titleIconURI = SvValueTryGetString((SvValue)SvDBRawObjectGetAttrValue(leaf, "titleIconURI"));
        SvValue subcaptionV = (SvValue) SvDBRawObjectGetAttrValue(leaf, "subcaption");
        SvString subcaption = SvValueTryGetString(subcaptionV);
        item->subcaption = SVTESTRETAIN(subcaption);
    } else if (SvObjectIsInstanceOf(node_, QBLoadingStub_getType())) {
        QBLoadingStub stub = (QBLoadingStub) node_;
        if (stub->isLoading)
            item->caption = SvStringCreate(gettext("Please wait, loading…"), NULL);
        else
            item->caption = SvStringCreate(gettext("No Items"), NULL);
    } else if (SvObjectIsInstanceOf(node_, QBContentStub_getType())) {
        item->caption = SvStringCreate(gettext("Please wait, loading…"), NULL);
    } else if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        titleIconURI = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("icon"));
        item->caption = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode)node_, SVSTRING("caption"));
        SVTESTRETAIN(item->caption);
    }

    item->iconURI.URI = SVTESTRETAIN(titleIconURI);
    item->iconURI.isStatic = true;

    if (!item->iconRID)
        item->icon = SVRETAIN(self->icon);

    SvWidget ret = NULL;

    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);

    SVRELEASE(item);
    return ret;
}

SvLocal void
QBSocialMediaCarouselMenuItemControllerSetItemState(SvGenericObject self_,
                                                    SvWidget item_,
                                                    XMBMenuState state,
                                                    bool isFocused)
{
    QBSocialMediaCarouselMenuItemController self = (QBSocialMediaCarouselMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}


SvLocal SvWidget
QBSocialMediaCarouselMenuItemControllerCreateSubMenu(SvGenericObject self_,
                                                     SvGenericObject node_,
                                                     SvGenericObject path,
                                                     SvApplication app)
{
    QBSocialMediaCarouselMenuItemController self = (QBSocialMediaCarouselMenuItemController) self_;

    if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType()))
        return NULL;
    if (SvObjectIsInstanceOf(node_, QBContentStub_getType()))
        return NULL;

    svSettingsPushComponent("SocialMediaInfo.settings");
    SvWidget w = XMBCarouselNew(app, "menu");
    SvWidget carousel = XMBCarouselGetCarouselWidget(w);

    for (int i = 0; i < QBVerticalCarouselGetSlotCount(carousel); i++) {
        QBVerticalCarouselSetContentAtIndex(carousel, i, SocialMediaInfoCreate(app, self->appGlobals->textRenderer));
        QBVerticalCarouselSetDataHandler(carousel, i, SocialMediaInfoSetObject);
    }
    svSettingsPopComponent();
    return w;
}


SvLocal SvType
QBSocialMediaCarouselMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSocialMediaCarouselMenuItemControllerDestroy
    };
    static SvType type = NULL;

    static const struct XMBMenuController_t menu_controller_methods = {
        .createSubMenu = QBSocialMediaCarouselMenuItemControllerCreateSubMenu,
    };

    static const struct XMBItemController_t controller_methods = {
        .createItem = QBSocialMediaCarouselMenuItemControllerCreateItem,
        .setItemState = QBSocialMediaCarouselMenuItemControllerSetItemState,
    };


    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSocialMediaCarouselMenuItemController",
                            sizeof(struct QBSocialMediaCarouselMenuItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            XMBMenuController_getInterface(), &menu_controller_methods,
                            NULL);
    }

    return type;
}



QBSocialMediaCarouselMenuItemController
QBSocialMediaCarouselMenuItemControllerCreate(AppGlobals appGlobals, SvGenericObject provider)
{
    QBSocialMediaCarouselMenuItemController itemControler = (QBSocialMediaCarouselMenuItemController) SvTypeAllocateInstance(
            QBSocialMediaCarouselMenuItemController_getType(), NULL);

    itemControler->appGlobals = appGlobals;
    itemControler->settingsCtx = svSettingsSaveContext();
    itemControler->provider = SVTESTRETAIN(provider);

    svSettingsPushComponent("VODMenu.settings");

    itemControler->icon = SVRETAIN(svSettingsGetBitmap("MenuItem", "icon"));
    itemControler->searchIconRID = svSettingsGetResourceID("MenuItem", "searchIcon");
    itemControler->iconRID = svSettingsGetResourceID("MenuItem", "icon");
    itemControler->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItems.inactiveFocus"))
        itemControler->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    itemControler->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    itemControler->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);

    svSettingsPopComponent();

    return itemControler;
}


QBSocialMediaCarouselMenuItemService
QBSocialMediaCarouselMenuCreate(AppGlobals appGlobals,
                                SvString serviceType,
                                SvString serviceID,
                                SvString serviceName,
                                bool searchable,
                                bool allVisible,
                                bool requiresAuthentication,
                                bool enableSlaveStaticCategories)
{
    QBSocialMediaCarouselMenuItemService itemInfo = NULL;
    itemInfo = (QBSocialMediaCarouselMenuItemService)
        SvTypeAllocateInstance(QBSocialMediaCarouselMenuItemService_getType(), NULL);

    if (itemInfo == NULL) {
        SvLogError("%s() : out of memory", __func__);
        return NULL;
    }

    QBContentTree tree = QBContentTreeCreate(serviceID, NULL);

    SvString url = QBMiddlewareManagerGetPrefixUrl(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvXMLRPCServerInfo serverInfo = QBMiddlewareManagerGetXMLRPCServerInfo(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString deviceID = QBMiddlewareManagerGetId(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);

    SvLogNotice("Registering Innov8on service: %15s (id=%s) server=%s",
                serviceName ? SvStringCString(serviceName) : "--",
                SvStringCString(serviceID), SvStringCString(url));

    Innov8onProviderParams params = NULL;
    SvGenericObject provider = NULL;
    params = Innov8onProviderParamsCreate(serverInfo, deviceID, 3, 10, 200, 200, 60,
                                          NULL, NULL, NULL, enableSlaveStaticCategories, NULL);
    provider = (SvGenericObject)
        Innov8onProviderCreate(tree, params, serviceID, NULL, searchable, allVisible, NULL);
    SVRELEASE(params);

    SvGenericObject itemController = (SvGenericObject) QBSocialMediaCarouselMenuItemControllerCreate(appGlobals, provider);
    SvGenericObject itemChoice = (SvGenericObject) QBSocialMediaCarouselMenuChoiceNew(appGlobals, serviceName);

    struct QBCarouselMenuInitParams_s initParams;
    memset(&initParams, 0, sizeof(initParams));

    initParams.serviceName = serviceName;
    initParams.serviceType = serviceType;
    initParams.serviceID = serviceID;
    initParams.serviceLayout = NULL;
    initParams.provider = provider;
    initParams.itemController = itemController;
    initParams.itemChoice = itemChoice;
    initParams.contentTree = (SvGenericObject) tree;

    QBCarouselMenuInitWithParams((QBCarouselMenuItemService) itemInfo, appGlobals, &initParams);

    ((QBCarouselMenuItemService) itemInfo)->clearOnFocusLost = true;

    SVRELEASE(provider);
    SVRELEASE(itemController);
    SVRELEASE(itemChoice);
    SVRELEASE(tree);

    itemInfo->requiresAuthentication = requiresAuthentication;

    if (requiresAuthentication) {
        if (appGlobals->authenticationService) {
            QBSocialMediaCarouselMenuItemServiceCurrentUserChanged((SvGenericObject) itemInfo, NULL);
            QBAuthenticationServiceAddListener(appGlobals->authenticationService,
                                               (SvGenericObject) itemInfo);
        } else {
            SvLogError("%s() : QBAuthenticationService doesn't exist", __func__);
            SVRELEASE(itemInfo);
            return NULL;
        }
    }

    QBContentProviderAddListener((QBContentProvider) provider, (SvObject) itemInfo);
    QBContentProviderStart((QBContentProvider) provider, appGlobals->scheduler);

    return itemInfo;
}

