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

#include "dlnaCarouselItemController.h"
#include <libintl.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <QBCarousel/QBVerticalCarousel.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBDLNAProviderParams.h>
#include <QBContentManager/QBDLNAProvider.h>
#include <Windows/mainmenu.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <SWL/QBFrame.h>
#include <Menus/menuchoice.h>
#include <Widgets/XMBCarousel.h>
#include <XMB2/XMBMenuController.h>
#include <XMB2/XMBItemController.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <QBContentManager/QBContentStub.h>
#include <QBContentManager/QBContentProvider.h>
#include <Widgets/movieInfo.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Utils/value.h>
#include <DataModels/loadingProxy.h>
#include <Services/core/appState.h>
#include <QBContentManager/QBLoadingStub.h>
#include "dlnaCarouselItemChoice.h"
#include "main.h"


struct QBDLNACarouselMenuItemController_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    QBXMBItemConstructor itemConstructor;
    unsigned int settingsCtx;

    SvGenericObject provider;

    SvRID iconRID, searchIconRID;
};

SvLocal void QBDLNACarouselMenuItemControllerDestroy(void *self_)
{
    QBDLNACarouselMenuItemController self = self_;
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
QBDLNACarouselMenuItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBDLNACarouselMenuItemController self = (QBDLNACarouselMenuItemController) self_;

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

        svSettingsPushComponent("Carousel_VOD.settings");
        SvString type = SvValueTryGetString((SvValue) QBContentCategoryGetAttribute(node, SVSTRING("type")));
        if (SvStringEqualToCString(type, "DLNADevice"))
            item->icon = svSettingsGetBitmap("MoviePage.ServerIcon", "bg");
        else if (SvStringEqualToCString(type, "videoContainer"))
            item->icon = svSettingsGetBitmap("MoviePage.VideoCategoryIcon", "bg");
        else if (SvStringEqualToCString(type, "audioContainer"))
            item->icon = svSettingsGetBitmap("MoviePage.AudioCategoryIcon", "bg");
        else if (SvStringEqualToCString(type, "imageContainer"))
            item->icon = svSettingsGetBitmap("MoviePage.PhotoCategoryIcon", "bg");
        else if (SvStringEqualToCString(type, "playlistContainer"))
            item->icon = svSettingsGetBitmap("MoviePage.PlaylistCategoryIcon", "bg");
        else if (SvStringEqualToCString(type, "container"))
            item->icon = svSettingsGetBitmap("MoviePage.CategoryIcon", "bg");

        svSettingsPopComponent();
        SVRETAIN(item->icon);
    } else if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType())) {
        SvDBRawObject leaf = (SvDBRawObject) node_;
        svSettingsPushComponent("Carousel_VOD.settings");
        SvString title = NULL;
        SvString type = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue(leaf, "type"));
        bool internalName = false;
        if (SvStringEqualToCString(type, "video"))
            item->icon = svSettingsGetBitmap("MoviePage.VideoContentIcon", "bg");
        else if (SvStringEqualToCString(type, "audio"))
            item->icon = svSettingsGetBitmap("MoviePage.AudioContentIcon", "bg");
        else if (SvStringEqualToCString(type, "image"))
            item->icon = svSettingsGetBitmap("MoviePage.PhotoContentIcon", "bg");
        else if (SvStringEqualToCString(type, "playlist"))
            item->icon = svSettingsGetBitmap("MoviePage.PlaylistContentIcon", "bg");
        else if (SvStringEqualToCString(type, "empty")) {
            item->icon = svSettingsGetBitmap("MoviePage.EmptyIcon", "bg");
            item->caption = SvStringCreate(gettext("Empty folder"), NULL);
            internalName = true;
        } else if (SvStringEqualToCString(type, "noresponse")) {
            item->icon = svSettingsGetBitmap("MoviePage.NoResponseIcon", "bg");
            item->caption = SvStringCreate(gettext("Device is not responding"), NULL);
            internalName = true;
        } else if (SvStringEqualToCString(type, "unbrowsable")) {
            item->icon = svSettingsGetBitmap("MoviePage.UnbrowsableIcon", "bg");
            item->caption = SvStringCreate(gettext("Error occurred"), NULL);
            internalName = true;
        } else if (SvStringEqualToCString(type, "accessdenied")) {
            item->icon = svSettingsGetBitmap("MoviePage.UnbrowsableIcon", "bg");
            item->caption = SvStringCreate(gettext("Access denied"), NULL);
            internalName = true;
        }

        if (!internalName) {
            title = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue(leaf, "title"));
            if (title) {
                item->caption = SVRETAIN(title);
            }
            SvString extension = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue(leaf, "extension"));
            if (extension) {
                item->subcaption = SVRETAIN(extension);
            }
        }

        svSettingsPopComponent();
        SVRETAIN(item->icon);
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

    SvWidget ret = NULL;

    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);

    SVRELEASE(item);
    return ret;
}

SvLocal void
QBDLNACarouselMenuItemControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBDLNACarouselMenuItemController self = (QBDLNACarouselMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}


SvLocal SvWidget
QBDLNACarouselMenuItemControllerCreateSubMenu(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app)
{
    QBDLNACarouselMenuItemController self = (QBDLNACarouselMenuItemController) self_;
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
            if (SvStringEqualToCString(explicitDisplayMode, "image")) {
                displayMode = MovieInfoDisplayMode_image;
            }
        }
    }

    svSettingsPushComponent("Carousel_WEBTV.settings");

    SvWidget w = XMBCarouselNew(app, "menu");
    SvWidget carousel = XMBCarouselGetCarouselWidget(w);

    int i;
    for( i = 0; i < QBVerticalCarouselGetSlotCount(carousel); i++ ) {
        QBVerticalCarouselSetContentAtIndex(carousel, i, MovieInfoCreate(app, self->appGlobals->textRenderer, displayMode));
        QBVerticalCarouselSetDataHandler(carousel, i, MovieInfoSetObject);
    }
    svSettingsPopComponent();
    return w;
}


SvLocal SvType
QBDLNACarouselMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDLNACarouselMenuItemControllerDestroy
    };
    static SvType type = NULL;

    static const struct XMBMenuController_t menu_controller_methods = {
        .createSubMenu = QBDLNACarouselMenuItemControllerCreateSubMenu,
    };

    static const struct XMBItemController_t controller_methods = {
        .createItem = QBDLNACarouselMenuItemControllerCreateItem,
        .setItemState = QBDLNACarouselMenuItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDLNACarouselMenuItemController",
                            sizeof(struct QBDLNACarouselMenuItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            XMBMenuController_getInterface(), &menu_controller_methods,
                            NULL);
    }

    return type;
}



SvLocal QBDLNACarouselMenuItemController
QBDLNACarouselMenuItemControllerCreate(AppGlobals appGlobals, SvGenericObject provider)
{
    QBDLNACarouselMenuItemController itemControler = (QBDLNACarouselMenuItemController) SvTypeAllocateInstance(
            QBDLNACarouselMenuItemController_getType(), NULL);

    itemControler->appGlobals = appGlobals;
    itemControler->settingsCtx = svSettingsSaveContext();
    itemControler->provider = SVTESTRETAIN(provider);

    itemControler->searchIconRID = svSettingsGetResourceID("MenuItem", "searchIcon");
    itemControler->iconRID = svSettingsGetResourceID("MenuItem", "icon");
    itemControler->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        itemControler->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    itemControler->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);


    return itemControler;
}


QBCarouselMenuItemService
QBDLNACarouselMenuCreate(AppGlobals appGlobals)
{
    QBCarouselMenuItemService itemInfo = (QBCarouselMenuItemService) SvTypeAllocateInstance(QBCarouselMenuItemService_getType(), NULL);

    QBContentTree tree = QBContentTreeCreate(SVSTRING("DLNA"), NULL);

    QBDLNAProviderParams params = QBDLNAProviderParamsCreate(3, 20, 20, 200, 60, true, NULL);
    SvGenericObject provider = (SvGenericObject) QBDLNAProviderCreate(tree, params, SVSTRING("DLNAService"), NULL);
    SVRELEASE(params);

    svSettingsPushComponent("DLNAMenu.settings");
    SvGenericObject itemController = (SvGenericObject) QBDLNACarouselMenuItemControllerCreate(appGlobals, provider);
    SvGenericObject itemChoice = (SvGenericObject) QBDLNACarouselMenuChoiceNew(appGlobals, SVSTRING("DLNA"));

    struct QBCarouselMenuInitParams_s initParams = {
        .serviceName = SVSTRING("DLNA"),
        .serviceType = SVSTRING("DLNA"),
        .serviceID =  SVSTRING("DLNA"),
        .serviceLayout = NULL,
        .provider = provider,
        .itemController = itemController,
        .itemChoice = itemChoice,
        .contentTree = (SvGenericObject) tree,
    };

    QBCarouselMenuInitWithParams((QBCarouselMenuItemService) itemInfo, appGlobals, &initParams);

    svSettingsPopComponent();

    SVRELEASE(provider);
    SVRELEASE(itemController);
    SVRELEASE(itemChoice);
    SVRELEASE(tree);

    return itemInfo;
}

