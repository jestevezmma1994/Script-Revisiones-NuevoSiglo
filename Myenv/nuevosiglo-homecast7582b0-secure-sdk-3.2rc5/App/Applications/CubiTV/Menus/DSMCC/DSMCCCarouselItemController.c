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

#include "DSMCCCarouselItemChoice.h"
#include "DSMCCCarouselItemController.h"

#include <main.h>
#include <DataModels/loadingProxy.h>
#include <Menus/carouselMenu.h>
#include <Widgets/movieInfo.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/XMBCarousel.h>
#include <QBCarousel/QBVerticalCarousel.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuController.h>
#include <QBDSMCC/QBDSMCCFeedProvider.h>
#include <QBDSMCC/QBDSMCCMonitor.h>
#include <QBContentManager/QBContentTree.h>
#include <settings.h>
#include <SvFoundation/SvType.h>
#include <libintl.h>
#include <QBContentManager/QBLoadingStub.h>


#if SV_LOG_LEVEL > 0
    #define moduleName "QBDSMCCCarouselItemController"
    #define log_warning(fmt, ...) do { SvLogWarning(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...) do { SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#else
    #define log_warning(fmt, ...)
    #define log_error(fmt, ...)
#endif


struct QBDSMCCCarouselItemController_s {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    QBFrameDescription* focus;
    QBFrameDescription* inactiveFocus;

    QBXMBItemConstructor itemConstructor;
};

SvLocal SvType QBDSMCCCarouselItemController_getType(void);

QBDSMCCCarouselItemController QBDSMCCCarouselItemControllerCreate(AppGlobals appGlobals)
{
    if (!appGlobals) {
        log_error("got NULL appGlobals");
        return NULL;
    }

    QBDSMCCCarouselItemController self = NULL;
    QBFrameDescription *focus = NULL;
    QBFrameDescription *inactiveFocus = NULL;
    QBXMBItemConstructor itemConstructor = NULL;

    // create members

    //svSettingsPushComponent("VODMenu.settings");
    focus = QBFrameDescriptionCreateFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        inactiveFocus = QBFrameDescriptionCreateFromSM("MenuItem.inactiveFocus");
    itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);
    //svSettingsPopComponent();

    if (!focus) {
        log_error("could not get focus frame description");
        goto fini;
    }
    if (!itemConstructor) {
        log_error("could not create item constructor");
        goto fini;
    }

    // create self

    self = (QBDSMCCCarouselItemController) SvTypeAllocateInstance(QBDSMCCCarouselItemController_getType(), NULL);

    // attach members

    self->appGlobals = appGlobals;

    self->focus = SVRETAIN(focus);
    self->inactiveFocus = SVTESTRETAIN(inactiveFocus);
    self->itemConstructor = SVRETAIN(itemConstructor);

fini:
    SVTESTRELEASE(focus);
    SVTESTRELEASE(inactiveFocus);
    SVTESTRELEASE(itemConstructor);
    return self;
}

SvLocal void QBDSMCCCarouselItemControllerDestroy(void *self_)
{
    QBDSMCCCarouselItemController self = self_;

    SVRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);

    QBXMBItemConstructorDestroy(self->itemConstructor);
}

SvLocal SvWidget QBDSMCCCarouselItemControllerCreateItem(SvObject self_, SvObject node_, SvObject path,
                                                         SvApplication app, XMBMenuState initialState);

SvLocal void QBDSMCCCarouselItemControllerSetItemState(SvObject self_, SvWidget item, XMBMenuState menuState,
                                                       bool isFocused);

SvLocal SvWidget QBDSMCCCarouselItemControllerCreateSubMenu(SvObject self_, SvObject node, SvObject path,
                                                            SvApplication app);

SvLocal SvType QBDSMCCCarouselItemController_getType(void)
{
    static const struct SvObjectVTable_ vtable = {
        .destroy = QBDSMCCCarouselItemControllerDestroy
    };

    static const struct XMBItemController_t xmbItemControllerMethods = {
        .createItem   = QBDSMCCCarouselItemControllerCreateItem,
        .setItemState = QBDSMCCCarouselItemControllerSetItemState
    };

    static const struct XMBMenuController_t xmbMenuControllerMethods = {
        .createSubMenu = QBDSMCCCarouselItemControllerCreateSubMenu
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDSMCCCarouselItemController",
                            sizeof(struct QBDSMCCCarouselItemController_s),
                            SvObject_getType(), &type,
                            SvObject_getType(), &vtable,
                            XMBItemController_getInterface(), &xmbItemControllerMethods,
                            XMBMenuController_getInterface(), &xmbMenuControllerMethods,
                            NULL);
    }

    return type;
}

SvLocal SvWidget QBDSMCCCarouselItemControllerCreateItem(SvObject self_, SvObject node_, SvObject path,
                                                         SvApplication app, XMBMenuState initialState)
{
    QBDSMCCCarouselItemController self = (QBDSMCCCarouselItemController) self_;
    SvWidget widget = NULL;

    QBXMBItem item = QBXMBItemCreate();

    if (SvObjectIsInstanceOf(node_, SvString_getType())) {
        item->caption = SVRETAIN(node_);
    } else if (SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        QBContentCategory node = (QBContentCategory) node_;
        item->caption = SVRETAIN(QBContentCategoryGetTitle(node));
    } else if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType())) {
        SvDBRawObject leaf = (SvDBRawObject) node_;
        item->caption = SVRETAIN(SvValueGetString((SvValue) SvDBRawObjectGetAttrValue(leaf, "title")));
    } else if (SvObjectIsInstanceOf(node_, QBLoadingStub_getType())) {
        item->caption = SvStringCreate(gettext("No Items"), NULL);
    } else {
        item->caption = SVSTRING("????");
    }

    svSettingsPushComponent("Carousel_VOD.settings");
    item->icon = SVTESTRETAIN(svSettingsGetBitmap("MoviePage.VideoContentIcon", "bg"));
    svSettingsPopComponent();

    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);

    widget = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);

    SVRELEASE(item);
    return widget;
}

SvLocal void QBDSMCCCarouselItemControllerSetItemState(SvObject self_, SvWidget item, XMBMenuState menuState,
                                                       bool isFocused)
{
    QBDSMCCCarouselItemController self = (QBDSMCCCarouselItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item, menuState, isFocused);
}

SvLocal SvWidget QBDSMCCCarouselItemControllerCreateSubMenu(SvObject self_, SvObject node, SvObject path,
                                                            SvApplication app)
{
    QBDSMCCCarouselItemController self = (QBDSMCCCarouselItemController) self_;

    if (!SvObjectIsInstanceOf(node, QBContentCategory_getType()))
        return NULL;

    svSettingsPushComponent("Carousel_WEBTV.settings");

    SvWidget w = XMBCarouselNew(app, "menu");
    SvWidget carousel = XMBCarouselGetCarouselWidget(w);

    for (int i = 0; i < QBVerticalCarouselGetSlotCount(carousel); ++i) {
        QBVerticalCarouselSetContentAtIndex(carousel, i, MovieInfoCreate(app, self->appGlobals->textRenderer, MovieInfoDisplayMode_image));
        QBVerticalCarouselSetDataHandler(carousel, i, MovieInfoSetObject);
    }
    svSettingsPopComponent();
    return w;
}


QBCarouselMenuItemService QBDSMCCCarouselMenuCreate(AppGlobals appGlobals, SvString serviceId)
{
    QBCarouselMenuItemService itemInfo = NULL;

    QBContentTree contentTree = NULL;
    SvObject dataProvider = NULL;
    SvObject itemController = NULL;
    SvObject itemChoice = NULL;

    svSettingsPushComponent("Carousel_DSMCC.settings");

    contentTree = QBContentTreeCreate(serviceId, NULL);
    if (!contentTree) {
        SvLogError("%s(): could not create content tree", __func__);
        goto fini;
    }

    QBDSMCCClient dsmccClient = QBDSMCCMonitorGetClient(appGlobals->dsmccMonitor);
    if (!dsmccClient) {
        SvLogError("%s(): could not get DSM-CC client", __func__);
        goto fini;
    }
    dataProvider = (SvObject) QBDSMCCFeedProviderCreate(dsmccClient, contentTree);
    if (!dataProvider) {
        SvLogError("%s(): creating data provider failed", __func__);
        goto fini;
    }

    itemController = (SvObject) QBDSMCCCarouselItemControllerCreate(appGlobals);
    if (!itemController) {
        SvLogError("%s(): creating item controller failed", __func__);
        goto fini;
    }

    itemChoice = (SvObject) QBDSMCCCarouselMenuChoiceCreate(appGlobals, serviceId);
    if (!itemChoice) {
        SvLogError("%s(): creating item choice failed", __func__);
        goto fini;
    }

    itemInfo = (QBCarouselMenuItemService) SvTypeAllocateInstance(QBCarouselMenuItemService_getType(), NULL);
    if (!itemInfo) {
        SvLogError("%s(): creating item info failed", __func__);
        goto fini;
    }

    struct QBCarouselMenuInitParams_s carouselMenuInitParams = {
        .serviceName    = SVSTRING("DSMCC"),
        .serviceType    = SVSTRING("DSMCC"),
        .serviceID      = serviceId,
        .serviceLayout  = NULL,
        .provider       = dataProvider,
        .itemController = itemController,
        .itemChoice     = itemChoice,
        .contentTree    = (SvObject) contentTree,
    };

    QBCarouselMenuInitWithParams(itemInfo, appGlobals, &carouselMenuInitParams);

fini:
    SVTESTRELEASE(contentTree);
    SVTESTRELEASE(dataProvider);
    SVTESTRELEASE(itemController);
    SVTESTRELEASE(itemChoice);

    svSettingsPopComponent();

    return itemInfo;
}
