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

#include "MStoreCarouselItemController.h"

#include <libintl.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <QBCarousel/QBVerticalCarousel.h>
#include <Windows/pvrplayer.h>
#include <QuadriCast-MStoreVoD/MStoreVoDProvider.h>
#include <QuadriCast-MStoreVoD/MStoreVoDOrder.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentStub.h>
#include <QBContentManager/QBContentProvider.h>
#include <Windows/mainmenu.h>
#include <SWL/QBFrame.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Menus/menuchoice.h>
#include <Widgets/XMBCarousel.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuController.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <Widgets/movieInfo.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <Utils/value.h>
#include <DataModels/loadingProxy.h>
#include <QBConf.h>
#include <Services/core/appState.h>
#include <Logic/MStoreLogic.h>
#include <main.h>
#include <QBContentManager/QBLoadingStub.h>
#include "MStoreCarouselItemChoice.h"

struct QBMStoreCarouselMenuItemController_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvBitmap icon;
    QBFrameConstructor* focus;
    QBXMBItemConstructor itemConstructor;
    unsigned int settingsCtx;

    SvGenericObject provider;

    SvRID iconRID, searchIconRID, directoryRID;
};

SvLocal void
QBMStoreCarouselMenuItemControllerDestroy(void *self_)
{
    QBMStoreCarouselMenuItemController self = self_;
    SVTESTRELEASE(self->icon);
    SVTESTRELEASE(self->focus);
    if (self->itemConstructor)
        QBXMBItemConstructorDestroy(self->itemConstructor);
    if (self->provider) {
        QBContentProviderStop((QBContentProvider) self->provider);
        SVRELEASE(self->provider);
    }

}

SvLocal SvWidget
QBMStoreCarouselMenuItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBMStoreCarouselMenuItemController self = (QBMStoreCarouselMenuItemController) self_;


    QBXMBItem item = QBXMBItemCreate();

    item->caption = SVSTRING("????");
    item->loadingRID = self->iconRID;
    SvString titleIconURI = NULL;

    if (SvObjectIsInstanceOf(node_, QBContentSearch_getType())) {
        item->caption = SvStringCreate(gettext("Search"), NULL);
        item->iconRID = self->searchIconRID;
    } else if (SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        QBContentCategory node = (QBContentCategory) node_;

        SvString name = QBContentCategoryGetName(node);
        if (name && SvStringEqualToCString(name, "LATEST_ARRIVALS")) {
            item->caption = SvStringCreate(gettext("Latest arrivals"), NULL);
        } else if (name && SvStringEqualToCString(name, "ORDER_HISTORY")) {
            item->caption = SvStringCreate(gettext("Order history"), NULL);
        } else {
            SvString title = SvValueTryGetString((SvValue) QBContentCategoryGetAttribute(node, SVSTRING("shortTitle")));
            if (!title)
                title = QBContentCategoryGetTitle(node);
            item->caption = SVRETAIN(title);
        }
        item->iconRID = self->directoryRID;
        titleIconURI = SvValueTryGetString((SvValue)QBContentCategoryGetAttribute(node, SVSTRING("titleIconURI")));

    } else if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType())) {
        SvDBRawObject leaf = (SvDBRawObject) node_;
        SvValue name_ = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) leaf, "name");
        if (name_) {
            SvString name = SvValueGetString(name_);
            item->caption = SVTESTRETAIN(name);
        }
    } else if (SvObjectIsInstanceOf(node_, QBContentStub_getType())) {
        item->caption = SvStringCreate(gettext("Please wait, loading…"), NULL);
    } else if (SvObjectIsInstanceOf(node_, QBLoadingStub_getType())) {
        QBLoadingStub stub = (QBLoadingStub) node_;
        if (stub->isLoading)
            item->caption = SvStringCreate(gettext("Please wait, loading…"), NULL);
        else
            item->caption = SvStringCreate(gettext("No Items"), NULL);
    } else if (SvObjectIsInstanceOf(node_, MStoreVoDOrder_getType())) {
        MStoreVoDOrder order = (MStoreVoDOrder) node_;
        if (order->title && order->id) {
            int orderId = SvValueIsInteger(order->id) ? SvValueGetInteger(order->id) : -1;
            item->caption = SvStringCreateWithFormat("#%d %s", orderId, SvStringCString(order->title));

            SvStringBuffer buffer = SvStringBufferCreate(NULL);
            /// Construct Local time
            struct tm start = { .tm_year = 0 };
            struct tm end   = { .tm_year = 0 };
            SvTimeBreakDown(SvTimeConstruct(order->authStart, 0), true, &start);
            SvTimeBreakDown(SvTimeConstruct(order->authEnd,   0), true, &end);
            SvStringBufferAppendFormatted(buffer, NULL, "%s %04d.%02d.%02d %s %04d.%02d.%02d",
                                          gettext("From"), start.tm_year + 1900, start.tm_mon + 1, start.tm_mday,
                                          gettext("to"),   end.tm_year   + 1900, end.tm_mon   + 1, end.tm_mday);

            item->subcaption = SvStringBufferCreateContentsString(buffer, NULL);
            SVRELEASE(buffer);
        }
    } else if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        item->caption = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("caption"));
        SVTESTRETAIN(item->caption);
    }

    item->iconURI.URI = SVTESTRETAIN(titleIconURI);

    SvWidget ret = NULL;

    item->focus = SVRETAIN(self->focus);
    ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);

    SVRELEASE(item);
    return ret;
}

SvLocal void
QBMStoreCarouselMenuItemControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBMStoreCarouselMenuItemController self = (QBMStoreCarouselMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}


SvLocal SvWidget
QBMStoreCarouselMenuItemControllerCreateSubMenu(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app)
{
    QBMStoreCarouselMenuItemController self = (QBMStoreCarouselMenuItemController) self_;

    if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType()))
        return NULL;
    else if (SvObjectIsInstanceOf(node_, QBContentStub_getType()))
        return NULL;
    else if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return NULL;

    svSettingsPushComponent("Carousel_PVOD.settings");

    SvWidget w = XMBCarouselNew(app, "menu");
    SvWidget carousel = XMBCarouselGetCarouselWidget(w);

    int slotCount = QBVerticalCarouselGetSlotCount(carousel);
    for (int i = 0; i < slotCount; i++ ) {
        QBVerticalCarouselSetContentAtIndex(carousel, i, MovieInfoCreate(app, self->appGlobals->textRenderer, MovieInfoDisplayMode_rss));
        QBVerticalCarouselSetDataHandler(carousel, i, MovieInfoSetObject);
    }
    svSettingsPopComponent();
    return w;
}

SvLocal SvType
QBMStoreCarouselMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMStoreCarouselMenuItemControllerDestroy
    };
    static SvType type = NULL;

    static const struct XMBMenuController_t menu_controller_methods = {
        .createSubMenu = QBMStoreCarouselMenuItemControllerCreateSubMenu,
    };

    static const struct XMBItemController_t controller_methods = {
        .createItem = QBMStoreCarouselMenuItemControllerCreateItem,
        .setItemState = QBMStoreCarouselMenuItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMStoreCarouselMenuItemController",
                            sizeof(struct QBMStoreCarouselMenuItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            XMBMenuController_getInterface(), &menu_controller_methods,
                            NULL);
    }

    return type;
}



QBMStoreCarouselMenuItemController
QBMStoreCarouselMenuItemControllerCreate(AppGlobals appGlobals, SvGenericObject provider)
{
    QBMStoreCarouselMenuItemController itemControler = (QBMStoreCarouselMenuItemController) SvTypeAllocateInstance(
            QBMStoreCarouselMenuItemController_getType(), NULL);

    itemControler->appGlobals = appGlobals;
    itemControler->settingsCtx = svSettingsSaveContext();
    itemControler->provider = SVTESTRETAIN(provider);

    svSettingsRestoreContext(itemControler->settingsCtx);

    itemControler->icon = SVRETAIN(svSettingsGetBitmap("MenuItem", "icon"));
    itemControler->searchIconRID = svSettingsGetResourceID("MenuItem", "searchIcon");
    itemControler->iconRID = svSettingsGetResourceID("MenuItem", "icon");
    itemControler->directoryRID = svSettingsGetResourceID("MenuItem", "directoryIcon");
    itemControler->focus = QBFrameConstructorFromSM("MenuItem.focus");
    itemControler->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);

    svSettingsPopComponent();

    return itemControler;
}

QBCarouselMenuItemService
QBMStoreCarouselMenuCreate(AppGlobals appGlobals, SvString mountPoint, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBCarouselMenuItemService itemInfo = NULL;
    SvGenericObject itemController = NULL;
    SvGenericObject itemChoice = NULL;
    MStoreVoDProvider provider = NULL;
    QBContentTree tree = NULL;
    SvString langStr = NULL;

    if (!mountPoint) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "mountPoint is NULL");
        goto fini;
    }

    if (!(tree = QBContentTreeCreate(SVSTRING("VOD"), &error))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "QBContentTreeCreate() call failed.");
        goto fini;
    }

    struct MStoreVoDProviderParams params =  {
        .mountPoint           = mountPoint,
        .tree                 = tree,
        .createSearch         = QBMStoreLogicHasSearch(appGlobals->mstoreLogic),
        .createLatestArrivals = QBMStoreLogicHasLatestArrivals(appGlobals->mstoreLogic),
        .createOrderHistory   = QBMStoreLogicHasOrderHistory(appGlobals->mstoreLogic),
        .createMyRentals      = QBMStoreLogicHasMyRentals(appGlobals->mstoreLogic),
    };

    if (!(provider = MStoreVoDProviderCreate(&params, &error))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "MStoreVoDProviderCreate() call failed.");
        goto fini;
    }

    const char *lang = QBConfigGet("LANG");
    langStr = SvStringCreateWithCStringAndLength(lang, 2, NULL);
    MStoreVoDProviderSetLanguage(provider, langStr, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "MStoreVoDProviderSetLanguage() call failed.");
        goto fini;
    }

    QBCarouselMenuPushSettingsComponent(SVSTRING("VOD"), SVSTRING("VOD"));

    SvLogNotice("Registering MStore service: %s", "VOD");
    itemController = (SvGenericObject)
        QBMStoreCarouselMenuItemControllerCreate(appGlobals, (SvGenericObject) provider);
    itemChoice = (SvGenericObject)
        QBMStoreCarouselMenuChoiceNew(appGlobals, (SvGenericObject) provider, SVSTRING("VOD"));

    svSettingsPopComponent();

    itemInfo = (QBCarouselMenuItemService) SvTypeAllocateInstance(QBCarouselMenuItemService_getType(), &error);
    if (!itemInfo) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "Can't allocate QBCarouselMenuItemService instance.");
        goto fini;
    }

    struct QBCarouselMenuInitParams_s initParams = {
        .serviceName = SVSTRING("VOD"),
        .serviceType = SVSTRING("VOD"),
        .serviceID =  SVSTRING("VOD"),
        .serviceLayout = NULL,
        .provider = (SvObject) provider,
        .itemController = itemController,
        .itemChoice = itemChoice,
        .contentTree = (SvObject) tree,
    };

    QBCarouselMenuInitWithParams((QBCarouselMenuItemService) itemInfo, appGlobals, &initParams);

fini:
    SVTESTRELEASE(provider);
    SVTESTRELEASE(itemController);
    SVTESTRELEASE(itemChoice);
    SVTESTRELEASE(tree);
    SVTESTRELEASE(langStr);

    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : itemInfo;
}
