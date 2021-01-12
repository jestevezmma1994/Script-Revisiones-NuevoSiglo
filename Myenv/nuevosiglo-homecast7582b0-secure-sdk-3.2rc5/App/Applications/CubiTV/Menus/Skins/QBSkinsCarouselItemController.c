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

#include "QBSkinsCarouselItemController.h"

#include <libintl.h>
#include <QBConf.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <QBCarousel/QBVerticalCarousel.h>
#include <QBContentManager/QBContentStub.h>
#include <Windows/mainmenu.h>
#include <Menus/menuchoice.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/XMBCarousel.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuController.h>
#include <Widgets/movieInfo.h>
#include <Utils/value.h>
#include <DataModels/loadingProxy.h>
#include <Services/core/appState.h>
#include <Services/QBSkinManager.h>
#include <main.h>
#include <QBContentManager/QBLoadingStub.h>


struct QBSkinsCarouselMenuItemController_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvBitmap icon;
    SvBitmap tickMarkOn;
    SvBitmap tickMarkOff;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    QBXMBItemConstructor itemConstructor;
    unsigned int settingsCtx;
    const char *skinName;

    SvRID iconRID, searchIconRID;
};

SvLocal void QBSkinsCarouselMenuItemControllerDestroy(void *self_)
{
    QBSkinsCarouselMenuItemController self = self_;
    SVTESTRELEASE(self->icon);
    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVRELEASE(self->tickMarkOn);
    SVRELEASE(self->tickMarkOff);
    if (self->itemConstructor)
        QBXMBItemConstructorDestroy(self->itemConstructor);
}

SvLocal SvWidget
QBSkinsCarouselMenuItemControllerCreateItem(SvGenericObject self_,
                                            SvGenericObject node_,
                                            SvGenericObject path,
                                            SvApplication app,
                                            XMBMenuState initialState)
{
    QBSkinsCarouselMenuItemController self = (QBSkinsCarouselMenuItemController) self_;

    QBXMBItem item = QBXMBItemCreate();

    item->caption = SVSTRING("????");
    item->loadingRID = self->iconRID;

    if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType())) {
        SvDBRawObject leaf = (SvDBRawObject)node_;
        SvString title = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue(leaf, "name"));
        if (title)
            item->caption = SVRETAIN(title);
    } else if (SvObjectIsInstanceOf(node_, SvString_getType())) {
        item->caption = SVRETAIN(node_);
    } else if (SvObjectIsInstanceOf(node_, QBLoadingStub_getType())) {
        QBLoadingStub stub = (QBLoadingStub) node_;
        if (stub->isLoading)
            item->caption = SvStringCreate(gettext("Please wait, loading…"), NULL);
        else
            item->caption = SvStringCreate(gettext("No Items"), NULL);
    } else if (SvObjectIsInstanceOf(node_, QBContentStub_getType())) {
        item->caption = SvStringCreate(gettext("Please wait, loading…"), NULL);
    } else if (SvObjectIsInstanceOf(node_, QBSkinMWError_getType())) {
        item->caption = SVRETAIN(QBSkinMWErrorGetMessage((QBSkinMWError) node_));
        item->disabled = true;
    }

    if (self->skinName && item->caption && SvStringEqualToCString(item->caption, self->skinName))
        item->icon = SVRETAIN(self->tickMarkOn);
    else
        item->icon = SVRETAIN(self->tickMarkOff);

    SvWidget ret = NULL;

    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);

    SVRELEASE(item);
    return ret;
}

SvLocal void
QBSkinsCarouselMenuItemControllerSetItemState(SvGenericObject self_,
                                              SvWidget item_,
                                              XMBMenuState state,
                                              bool isFocused)
{
    QBSkinsCarouselMenuItemController self = (QBSkinsCarouselMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal SvWidget
QBSkinsCarouselMenuItemControllerCreateSubMenu(SvGenericObject self_,
                                               SvGenericObject node_,
                                               SvGenericObject path,
                                               SvApplication app)
{
    QBSkinsCarouselMenuItemController self = (QBSkinsCarouselMenuItemController) self_;

    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        svSettingsPushComponent("Carousel_WEBTV.settings");
        MovieInfoDisplayMode displayMode = MovieInfoDisplayMode_image;

        SvWidget w = XMBCarouselNew(app, "menu");
        SvWidget carousel = XMBCarouselGetCarouselWidget(w);

        for (int i = 0; i < QBVerticalCarouselGetSlotCount(carousel); i++ ) {
            QBVerticalCarouselSetContentAtIndex(carousel, i, MovieInfoCreate(app, self->appGlobals->textRenderer, displayMode));
            QBVerticalCarouselSetDataHandler(carousel, i, MovieInfoSetObject);
        }

        svSettingsPopComponent();

        return w;
    }

    return NULL;
}

SvLocal SvType
QBSkinsCarouselMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSkinsCarouselMenuItemControllerDestroy
    };
    static SvType type = NULL;

    static const struct XMBMenuController_t menu_controller_methods = {
        .createSubMenu = QBSkinsCarouselMenuItemControllerCreateSubMenu,
    };

    static const struct XMBItemController_t controller_methods = {
        .createItem = QBSkinsCarouselMenuItemControllerCreateItem,
        .setItemState = QBSkinsCarouselMenuItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSkinsCarouselMenuItemController",
                            sizeof(struct QBSkinsCarouselMenuItemController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            XMBMenuController_getInterface(), &menu_controller_methods,
                            NULL);
    }

    return type;
}

QBSkinsCarouselMenuItemController
QBSkinsCarouselMenuItemControllerCreate(AppGlobals appGlobals)
{
    QBSkinsCarouselMenuItemController itemControler = (QBSkinsCarouselMenuItemController)
        SvTypeAllocateInstance(QBSkinsCarouselMenuItemController_getType(), NULL);

    itemControler->appGlobals = appGlobals;
    itemControler->settingsCtx = svSettingsSaveContext();

    svSettingsPushComponent("Configuration.settings");

    itemControler->icon = SVRETAIN(svSettingsGetBitmap("MenuItem", "icon"));
    itemControler->searchIconRID = svSettingsGetResourceID("MenuItem", "searchIcon");
    itemControler->iconRID = svSettingsGetResourceID("MenuItem", "icon");
    itemControler->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        itemControler->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    itemControler->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);
    itemControler->tickMarkOn = SVRETAIN(svSettingsGetBitmap("MenuItem", "tickMarkOn"));
    itemControler->tickMarkOff = SVRETAIN(svSettingsGetBitmap("MenuItem", "tickMarkOff"));

    itemControler->skinName = QBConfigGet("SKIN.NAME");

    svSettingsPopComponent();

    return itemControler;
}

