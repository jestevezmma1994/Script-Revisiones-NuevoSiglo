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

#include "innov8onCarouselItemController.h"

#include <libintl.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <SWL/icon.h>
#include "QBCarousel/QBVerticalCarousel.h"
#include "Windows/pvrplayer.h"
#include <Innov8on.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentSeeAll.h>
#include <QBContentManager/QBContentManager.h>
#include <QBContentManager/QBContentStub.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBContentManager/QBLoadingStub.h>
#include <Windows/mainmenu.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/XMBCarousel.h>
#include <XMB2/XMBMenuController.h>
#include <XMB2/XMBItemController.h>
#include <SWL/QBFrame.h>
#include <Menus/menuchoice.h>
#include <Menus/carouselMenu.h>
#include <Menus/carouselMenuPrivate.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <Widgets/movieInfo.h>
#include <Widgets/extendedInfo.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Utils/value.h>
#include <SvDataBucket2/SvDBReference.h>
#include <DataModels/loadingProxy.h>
#include <Services/core/appState.h>
#include "innov8onCarouselItemChoice.h"
#include "main.h"


typedef struct QBInnov8onCarouselMenu_t {
    struct QBCarouselMenuItemService_ super_;
} *QBInnov8onCarouselMenu;

struct QBInnov8onCarouselMenuItemController_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvBitmap icon;
    QBFrameConstructor *focus;
    QBFrameConstructor *inactiveFocus;
    QBXMBItemConstructor itemConstructor;
    unsigned int settingsCtx;
    SvObject provider;

    SvRID iconRID, searchIconRID, seeAllIconRID;
};

struct QBInnov8onCarouselMenuItemService_t {
    struct SvObject_ super_;

    SvObject provider;
    SvObject itemController;
    SvObject itemChoice;
    QBContentTree tree;
    AppGlobals appGlobals;
};

SvLocal void
QBInnov8onCarouselMenuItemControllerDestroy(void *self_)
{
    QBInnov8onCarouselMenuItemController self = self_;

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

SvLocal void
QBInnov8onCarouselMenuItemControllerFillItemCaptionWithTranslatedCategoryTitle(QBInnov8onCarouselMenuItemController self, QBContentCategory node, QBXMBItem item)
{
    SvString title = QBContentCategoryGetTitle(node);
    if (title != NULL) {
        const char *translatedTitle = gettext(SvStringCString(title));
        item->caption = SvStringCreate(translatedTitle, NULL);
    }
}

SvLocal void
QBInnov8onCarouselMenuItemControllerFillItemUsingContentCategory(QBInnov8onCarouselMenuItemController self, QBContentCategory node, QBXMBItem item)
{
    SvString title = SvValueTryGetString((SvValue) QBContentCategoryGetAttribute(node, SVSTRING("shortTitle")));
    if (!title) {
        title = QBContentCategoryGetTitle(node);
    }
    item->caption = SVRETAIN(title);
    SvString titleIconURI = SvValueTryGetString((SvValue) QBContentCategoryGetAttribute(node, SVSTRING("titleIconURI")));;
    item->iconURI.URI = SVTESTRETAIN(titleIconURI);
}

SvLocal void
QBInnov8onCarouselMenuItemControllerFillItemUsingDBRawObject(QBInnov8onCarouselMenuItemController self, SvDBRawObject node, QBXMBItem item)
{
    SvString title = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue(node, "name"));
    if (title && !SvStringEqualToCString(title, "")) {
        item->caption = SVRETAIN(title);
    } else {
        SvDBReference parentRef = (SvDBReference) SvDBRawObjectGetAttrValue(node, "parent");
        if (parentRef) {
            SvDBRawObject parent = (SvDBRawObject) SvDBReferenceGetReferredObject((SvDBReference) parentRef);
            SvValue nameV = (SvValue) SvDBRawObjectGetAttrValue(parent, "name");
            if (nameV) {
                const char *name = SvValueGetStringAsCString(nameV, NULL);
                SvString caption = SvStringCreateWithFormat("%s %s", name, gettext("element"));
                item->caption = SVRETAIN(caption);
                SVRELEASE(caption);
            }
        }
    }
    SvString titleIconURI = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue(node, "titleIconURI"));
    item->iconURI.URI = SVTESTRETAIN(titleIconURI);
}

SvLocal void
QBInnov8onCarouselMenuItemControllerFillItemUsingLoadingStub(QBInnov8onCarouselMenuItemController self, QBLoadingStub node_, QBXMBItem item)
{
    QBLoadingStub stub = node_;
    if (stub->isLoading) {
        item->caption = SvStringCreate(gettext("Please wait, loading…"), NULL);
    } else {
        item->caption = SvStringCreate(gettext("No Items"), NULL);
    }
}

SvLocal void
QBInnov8onCarouselMenuItemControllerFillItemUsingActiveTreeNode(QBInnov8onCarouselMenuItemController self, QBActiveTreeNode node_, QBXMBItem item)
{
    SvString titleIconURI = (SvString) QBActiveTreeNodeGetAttribute(node_, SVSTRING("icon"));
    item->iconURI.URI = SVTESTRETAIN(titleIconURI);
    item->caption = (SvString) QBActiveTreeNodeGetAttribute(node_, SVSTRING("caption"));
    SVTESTRETAIN(item->caption);
}

SvLocal SvWidget
QBInnov8onCarouselMenuItemControllerCreateItem(SvObject self_, SvObject node_, SvObject path, SvApplication app, XMBMenuState initialState)
{
    QBInnov8onCarouselMenuItemController self = (QBInnov8onCarouselMenuItemController) self_;
    QBXMBItem item = QBXMBItemCreate();
    item->caption = SVSTRING("????");
    item->loadingRID = self->iconRID;
    item->iconURI.URI = NULL;
    if (SvObjectIsInstanceOf(node_, QBContentSearch_getType())) {
        QBInnov8onCarouselMenuItemControllerFillItemCaptionWithTranslatedCategoryTitle(self, (QBContentCategory) node_, item);
        item->iconRID = self->searchIconRID;
    } else if (SvObjectIsInstanceOf(node_, QBContentSeeAll_getType())) {
        QBInnov8onCarouselMenuItemControllerFillItemCaptionWithTranslatedCategoryTitle(self, (QBContentCategory) node_, item);
        item->iconRID = self->seeAllIconRID;
    } else if (SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        QBInnov8onCarouselMenuItemControllerFillItemUsingContentCategory(self, (QBContentCategory) node_, item);
    } else if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType())) {
        QBInnov8onCarouselMenuItemControllerFillItemUsingDBRawObject(self, (SvDBRawObject) node_, item);
    } else if (SvObjectIsInstanceOf(node_, QBLoadingStub_getType())) {
        QBInnov8onCarouselMenuItemControllerFillItemUsingLoadingStub(self, (QBLoadingStub) node_, item);
    } else if (SvObjectIsInstanceOf(node_, QBContentStub_getType())) {
        item->caption = SvStringCreate(gettext("Please wait, loading…"), NULL);
    } else if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        QBInnov8onCarouselMenuItemControllerFillItemUsingActiveTreeNode(self, (QBActiveTreeNode) node_, item);
    } else {
        SvLogWarning("Undefined type of node (%s)", node_ ? SvObjectGetTypeName(node_) : "NULL");
    }
    SvWidget ret = NULL;
    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);

    SVRELEASE(item);
    return ret;
}

SvLocal void
QBInnov8onCarouselMenuItemControllerSetItemState(SvObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBInnov8onCarouselMenuItemController self = (QBInnov8onCarouselMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBInnov8onCarouselMenuItemControllerVerticalCarouselSlotStateChangeHandler(SvWidget w, bool isInactive)
{
    SvWidget contentImage = MovieInfoGetContentIcon(w);
    if (contentImage)
        svIconSetState(contentImage, isInactive);
}

SvLocal SvWidget
QBInnov8onCarouselMenuItemControllerCreateSubMenu(SvObject self_, SvObject node_, SvObject path, SvApplication app)
{
    QBInnov8onCarouselMenuItemController self = (QBInnov8onCarouselMenuItemController) self_;
    MovieInfoDisplayMode displayMode = MovieInfoDisplayMode_rss;

    if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType()))
        return NULL;
    if (SvObjectIsInstanceOf(node_, QBContentStub_getType()))
        return NULL;

    if (SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        SvValue v = (SvValue) QBContentCategoryGetAttribute((QBContentCategory) node_, SVSTRING("displayMode"));
        if (v && SvValueIsString(v)) {
            if (SvStringEqualToCString(SvValueGetString(v), "image"))
                displayMode = MovieInfoDisplayMode_image;
        }
    } else if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        SvString explicitDisplayMode = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("displayMode"));
        if (explicitDisplayMode) {
            if (SvStringEqualToCString(explicitDisplayMode, "submenu")) {
                //submenu is handled in higher XMBMenuBar - default behaviour if this method returns NULL.
                return NULL;
            }
            if (SvStringEqualToCString(explicitDisplayMode, "image")) {
                displayMode = MovieInfoDisplayMode_image;
            }
        }
    }

    svSettingsRestoreContext(self->settingsCtx);

    SvWidget w = XMBCarouselNew(app, "menu");
    SvWidget carousel = XMBCarouselGetCarouselWidget(w);

    int i;
    for (i = 0; i < QBVerticalCarouselGetSlotCount(carousel); i++) {
        QBVerticalCarouselSetContentAtIndex(carousel, i, MovieInfoCreate(app, self->appGlobals->textRenderer, displayMode));
        QBVerticalCarouselSetDataHandler(carousel, i, MovieInfoSetObject);
    }
    QBVerticalCarouselSetSlotStateChangeHandler(carousel, QBInnov8onCarouselMenuItemControllerVerticalCarouselSlotStateChangeHandler);
    svSettingsPopComponent();

    return w;
}


SvLocal SvType
QBInnov8onCarouselMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBInnov8onCarouselMenuItemControllerDestroy
    };
    static SvType type = NULL;

    static const struct XMBMenuController_t menu_controller_methods = {
        .createSubMenu = QBInnov8onCarouselMenuItemControllerCreateSubMenu,
    };

    static const struct XMBItemController_t controller_methods = {
        .createItem   = QBInnov8onCarouselMenuItemControllerCreateItem,
        .setItemState = QBInnov8onCarouselMenuItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBInnov8onCarouselMenuItemController",
                            sizeof(struct QBInnov8onCarouselMenuItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            XMBMenuController_getInterface(), &menu_controller_methods,
                            NULL);
    }

    return type;
}

QBInnov8onCarouselMenuItemController
QBInnov8onCarouselMenuItemControllerCreate(AppGlobals appGlobals, SvObject provider)
{
    QBInnov8onCarouselMenuItemController itemControler = (QBInnov8onCarouselMenuItemController) SvTypeAllocateInstance(
        QBInnov8onCarouselMenuItemController_getType(), NULL);

    itemControler->settingsCtx = svSettingsSaveContext();

    svSettingsPushComponent("VODMenu.settings");

    itemControler->appGlobals = appGlobals;
    itemControler->provider = SVTESTRETAIN(provider);

    itemControler->icon = SVRETAIN(svSettingsGetBitmap("MenuItem", "icon"));
    itemControler->searchIconRID = svSettingsGetResourceID("MenuItem", "searchIcon");
    itemControler->seeAllIconRID = svSettingsGetResourceID("MenuItem", "seeAllIcon");
    itemControler->iconRID = svSettingsGetResourceID("MenuItem", "icon");
    itemControler->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        itemControler->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    itemControler->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);

    svSettingsPopComponent();

    return itemControler;
}

SvLocal void
QBInnov8onCarouselMenuUnmounted(QBCarouselMenuItemService self)
{
    QBInnov8onCarouselMenuChoiceStop(self->itemChoice);
}

SvLocal SvType
QBInnov8onCarouselMenu_getType(void)
{
    static const struct QBCarouselMenuItemServiceVTable_ serviceVTable = {
        .unmounted = QBInnov8onCarouselMenuUnmounted
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBInnov8onCarouselMenu",
                            sizeof(struct QBInnov8onCarouselMenu_t),
                            QBCarouselMenuItemService_getType(),
                            &type,
                            QBCarouselMenuItemService_getType(), &serviceVTable,
                            NULL);
    }

    return type;
}

QBCarouselMenuItemService
QBInnov8onCarouselMenuCreate(AppGlobals appGlobals,
                             SvString serviceType,
                             SvString serviceID,
                             SvString serviceName,
                             SvString serviceLayout,
                             SvDBRawObject service,
                             bool call2Action,
                             SvString externalUrl)
{
    SvObject tree = NULL;
    SvObject serverInfo = NULL;
    SvObject provider = QBMainMenuLogicCreateProvider(appGlobals->mainLogic, serviceType, serviceID, serviceName, serviceLayout, service, call2Action, externalUrl, &tree, &serverInfo);
    if (!provider) {
        SvLogError("%s: Can't create provider for service: %s, serviceID: %s", __func__, SvStringCString(serviceName), SvStringCString(serviceID));
        return NULL;
    }

    QBCarouselMenuPushSettingsComponent(serviceName, serviceType);

    SvObject itemController = (SvObject) QBInnov8onCarouselMenuItemControllerCreate(appGlobals, provider);

    SvObject itemChoice = (SvObject) QBInnov8onCarouselMenuChoiceNew(appGlobals, provider, (SvObject) tree, serviceID, serviceName,
                                                                     serviceType, serviceLayout, call2Action, serverInfo);
    SVTESTRELEASE(serverInfo);

    QBCarouselMenuItemService itemInfo = (QBCarouselMenuItemService) SvTypeAllocateInstance(QBInnov8onCarouselMenu_getType(), NULL);

    struct QBCarouselMenuInitParams_s initParams = {
        .serviceName    = serviceName,
        .serviceType    = serviceType,
        .serviceID      = serviceID,
        .serviceLayout  = serviceLayout,
        .provider       = provider,
        .itemController = itemController,
        .itemChoice     = itemChoice,
        .contentTree    = (SvObject) tree,
    };

    QBCarouselMenuInitWithParams((QBCarouselMenuItemService) itemInfo, appGlobals, &initParams);

    SVRELEASE(provider);
    SVRELEASE(itemController);
    SVRELEASE(itemChoice);
    SVRELEASE(tree);

    return itemInfo;
}
