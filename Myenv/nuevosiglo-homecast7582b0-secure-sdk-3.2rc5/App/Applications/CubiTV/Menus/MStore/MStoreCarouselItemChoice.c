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

#include "MStoreCarouselItemChoice.h"

#include <libintl.h>
#include <QBConf.h>
#include <QBCarousel/QBCarousel.h>
#include <QBContentManager/QBContentTreePath.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentStub.h>
#include <QBContentManager/Innov8on/Innov8onProviderRequestPluginGetProducts.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBContentManager/QBContentArray.h>
#include <QBContentManager/QBRangeTree.h>
#include <QuadriCast-MStoreVoD/MStoreVoDProvider.h>
#include <QuadriCast-MStoreVoD/MStoreVoDOrder.h>
#include <QBOSK/QBOSKKey.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>
#include <SWL/viewport.h>
#include <SWL/icon.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBOSKPane.h>
#include <Menus/menuchoice.h>
#include <Utils/authenticators.h>
#include <Utils/value.h>
#include <Utils/contentCategoryUtils.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/XMBCarousel.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Widgets/QBSimpleDialog.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <XMB2/XMBMenuBar.h>
#include <XMB2/XMBItemController.h>
#include <QBWidgets/QBGrid.h>
#include <Widgets/authDialog.h>
#include <Widgets/eventISMovieInfo.h>
#include <Widgets/movieInfo.h>
#include <Windows/mainmenu.h>
#include <Windows/pvrplayer.h>
#include <Windows/imagebrowser.h>
#include <Windows/channelscanning.h>
#include <QBSmartCardMonitor.h>
#include <Logic/AnyPlayerLogic.h>
#include <Logic/MStoreLogic.h>
#include <player_hints/http_input.h>
#include <QBPlayerControllers/QBVoDAnyPlayerController.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <SvDataBucket2/SvDBReference.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <SvPlayerKit/SvViewRightDvbDRMInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <ctype.h>
#include <main.h>
#include <QBContentManager/Innov8onUtils.h>
#include <Middlewares/productPurchase.h>
#include <Middlewares/Innov8on/innov8onTransaction.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <QBContentManager/Innov8onProviderTransaction.h>

#include <Menus/Innov8on/innov8onCarouselItemController.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "MStoreCarouselItemChoice"
    SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, moduleName "Debug", "");
    #define log_debug(fmt, ...) do { if (env_log_level() >= 2) { SvLogNotice(COLBEG() moduleName ":: " fmt COLEND_COL(green), ## __VA_ARGS__); } } while (0)
#else
    #define log_debug(fmt, ...)
#endif

struct QBMStoreCarouselMenuChoice_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    SvObject provider;

    struct QBCarouselSideMenu_t {
        QBContextMenu ctx;
    } sidemenu;

    SvObject selectedProduct;
    QBProductPurchase purchase;
    int selectedOfferIndex;
    int price;

    SvObject selectedHistoryNode;

    QBContentSearch contentSearch;
    SvObject contentSearchPath;

    SvString smartcardId;

    SvWidget dialog;
    SvWidget parentalPopup;

    SvObject transaction;

    // needed for category tree traversal: when we have to wait for pin before entering "protectedCategoryPath"
    SvObject protectedCategoryPath;

    SvString serviceName;
    struct {
        unsigned int ctx;
        SvWidget viewport;
        SvWidget grid;
        SvWidget title;
        SvWidget description;
        SvWidget image;
    } extendedInfo;

    int settingsCtx;

    bool haveMWprovider;
    SvObject innov8onProvider;
    QBContentCategory category;
};

SvLocal void QBCarouselExtendedInfoOnShow(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBMStoreCarouselMenuChoice self = ptr;
    svSettingsRestoreContext(self->extendedInfo.ctx);
    svSettingsWidgetAttach(frame, self->extendedInfo.viewport, "viewport", 0);
    svSettingsPopComponent();
    SvDBRawObject movie = (SvDBRawObject) self->selectedProduct;
    if (movie) {
        SvValue titleVal = (SvValue) SvDBRawObjectGetAttrValue(movie, "name");
        SvString title = SvValueTryGetString(titleVal);
        SvValue descriptionVal = (SvValue) SvDBRawObjectGetAttrValue(movie, "extended_description");
        SvString description = SvValueTryGetString(descriptionVal);
        SvValue imageVal = (SvValue) SvDBRawObjectGetAttrValue(movie, "thumbnail");
        SvString imageURI = SvValueTryGetString(imageVal);

        QBAsyncLabelSetText(self->extendedInfo.description, description);
        QBAsyncLabelSetText(self->extendedInfo.title, title);
        if (imageURI) {
            svWidgetSetHidden(self->extendedInfo.image, false);
            unsigned int idx = (svIconGetBitmapsCount(self->extendedInfo.image) <= 0) ? 0 : 1;
            svIconSetBitmapFromURI(self->extendedInfo.image, idx, SvStringCString(imageURI));
            svIconSwitch(self->extendedInfo.image, idx, 0, 0.0);
        } else {
            svWidgetSetHidden(self->extendedInfo.image, true);
        }
    } else {
        QBAsyncLabelSetText(self->extendedInfo.description, NULL);
        QBAsyncLabelSetText(self->extendedInfo.title, NULL);
    }
    svViewPortSetContentsPosition(self->extendedInfo.viewport, 0, 0, true);
}

SvLocal void QBCarouselExtendedInfoSetActive(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBMStoreCarouselMenuChoice self = ptr;
    svWidgetSetFocus(self->extendedInfo.viewport);
}

SvLocal void QBCarouselExtendedInfoOnHide(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBMStoreCarouselMenuChoice self = ptr;
    svWidgetDetach(self->extendedInfo.viewport);
}

SvLocal void
QBCarouselOrderBySMSOnShow(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBMStoreCarouselMenuChoice self = ptr;
    svSettingsRestoreContext(self->extendedInfo.ctx);
    svSettingsWidgetAttach(frame, self->extendedInfo.viewport, "viewport", 0);
    svSettingsPopComponent();

    SvDBRawObject movie = (SvDBRawObject) self->selectedProduct;
    if (!movie) {
        QBAsyncLabelSetText(self->extendedInfo.title, NULL);
        QBAsyncLabelSetCText(self->extendedInfo.description,
                             gettext("There is a technical problem with this movie, please contact Customer Care"));
        goto out;
    }

    if (!self->smartcardId) {
        QBAsyncLabelSetText(self->extendedInfo.title, NULL);
        QBAsyncLabelSetCText(self->extendedInfo.description,
                             gettext("No SmartCard found. This movie can not be ordered"));
        goto out;
    }

    SvValue orderingIdVal = (SvValue) SvDBRawObjectGetAttrValue(movie, "orderingMovieID");
    if (orderingIdVal && SvObjectIsInstanceOf((SvObject) orderingIdVal, SvValue_getType()) && SvValueIsInteger(orderingIdVal)) {
        int orderingId = SvValueGetInteger(orderingIdVal);

        SvString title = SvStringCreateWithFormat("%d %s", orderingId, SvStringCString(self->smartcardId));
        SvValue descriptionVal = (SvValue) SvDBRawObjectGetAttrValue(movie, "orderingInstructions");
        SvString description = SvValueTryGetString(descriptionVal);

        QBAsyncLabelSetText(self->extendedInfo.title, title);
        QBAsyncLabelSetText(self->extendedInfo.description, description);
        SVRELEASE(title);
    } else {
        QBAsyncLabelSetText(self->extendedInfo.title, NULL);
        QBAsyncLabelSetCText(self->extendedInfo.description,
                             gettext("There is a technical problem with this movie, please contact Customer Care"));
        goto out;
    }

out:
    svWidgetSetHidden(self->extendedInfo.image, true);
    svViewPortSetContentsPosition(self->extendedInfo.viewport, 0, 0, true);
}

SvLocal void QBCarouselSideMenuClose(void *self_, QBContextMenu menu)
{
    QBMStoreCarouselMenuChoice self = self_;
    SVTESTRELEASE(self->sidemenu.ctx);
    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBCarouselSideMenuClose, self);
}

SvLocal void
QBCarouselSetControllerForContent(void *self_, QBAnyPlayerLogic anyPlayerLogic, SvDBRawObject movie, SvErrorInfo *errorOut)
{
    QBMStoreCarouselMenuChoice self = self_;
    SvObject controller = NULL;
    SvErrorInfo error = NULL;

    assert(self && anyPlayerLogic && movie);

    QBContentInfo contentInfo = (QBContentInfo) QBContentInfoCreate(self->appGlobals, NULL, NULL, (SvObject) self->selectedProduct, NULL, self->innov8onProvider, NULL);
    controller = QBAnyPlayerLogicCreateController(anyPlayerLogic, contentInfo);
    if (!controller) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "Controller for content could not be created");
    }

    QBAnyPlayerLogicSetController(anyPlayerLogic, controller);
    SVTESTRELEASE(contentInfo);
    SVTESTRELEASE(controller);
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal SvDRMInfo
QBCarouselDRMInfoCreate(QBMStoreCarouselMenuChoice self, SvDBRawObject movie)
{
    SvHashTable vmx = (SvHashTable) SvDBRawObjectGetAttrValue(movie, "vmx");

    if (vmx && SvObjectIsInstanceOf((SvObject) vmx, SvHashTable_getType())) {
        // Detected VMX encrypted content.

        SvValue storeInfoV = (SvValue) SvHashTableFind(vmx, (SvObject) SVSTRING("storeInfo"));
        SvString storeInfoPath = SvValueTryGetString(storeInfoV);

        SvValue globalInfoV = (SvValue) SvHashTableFind(vmx, (SvObject) SVSTRING("globalInfo"));
        SvString globalInfoPath = SvValueTryGetString(globalInfoV);

        if (storeInfoPath && globalInfoPath) {
            SvDRMInfo drmInfo = (SvDRMInfo) SvViewRightDvbDRMInfoCreateFromFiles(globalInfoPath, storeInfoPath, NULL);
            return drmInfo;
        } else {
            SvLogError("%s: VMX data not found", __func__);
        }
    }

    return NULL;
}

SvLocal void
QBCarouselMenuPlay(void *self_, SvString id, SvDBRawObject movie, bool needEmm)
{
    QBMStoreCarouselMenuChoice self = self_;

    assert(self && movie);

    SvString source = NULL;
    bool playTrailer = SvStringEqualToCString(id, "play trailer");

    if (playTrailer) {
        SvValue sourceV = (SvValue) SvDBRawObjectGetAttrValue(movie, "trailer");
        source = SvValueTryGetString(sourceV);
    } else {
        SvValue sourceV = (SvValue) SvDBRawObjectGetAttrValue(movie, "source");
        source = SvValueTryGetString(sourceV);
    }

    if (!source)
        return;

    QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->appGlobals);
    if (!playTrailer && needEmm)
        QBCarouselSetControllerForContent(self, anyPlayerLogic, movie, NULL);
    QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals, anyPlayerLogic);

    SVRELEASE(anyPlayerLogic);

    // Set title
    SvValue titleVal = (SvValue) SvDBRawObjectGetAttrValue(movie, "name");
    SvString title = SvValueTryGetString(titleVal);
    SvString serviceName = title ? SVRETAIN(title) : SvStringCreate(gettext("MStore"), NULL);
    QBPVRPlayerContextSetTitle(pvrPlayer, serviceName);
    SVRELEASE(serviceName);

    // Set cover
    SvString coverURI = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue(movie, "thumbnail"));
    if (coverURI)
        QBPVRPlayerContextSetCoverURI(pvrPlayer, coverURI);

    //QBPVRPlayerContextUseBookmarks(pvrPlayer, lastPosition);
    QBPVRPlayerContextSetVodMetaData(pvrPlayer, movie);

    SvContent c = SvContentCreateFromCString(SvStringCString(source), NULL);
    SvDRMInfo drmInfo = QBCarouselDRMInfoCreate(self, movie);
    if (drmInfo && !playTrailer) {
        SvContentSetDrmInfo(c, drmInfo);
    }
    SVTESTRELEASE(drmInfo);
    QBPVRPlayerContextSetContent(pvrPlayer, c);
    SVRELEASE(c);
    QBApplicationControllerPushContext(self->appGlobals->controller, pvrPlayer);
    SVRELEASE(pvrPlayer);
}

SvLocal void
QBCarouselSideMenuHide(QBMStoreCarouselMenuChoice self)
{
    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal void
QBMStoreCarouselOrderNowPlayCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBMStoreCarouselMenuChoice self = self_;

    assert(self);

    self->dialog = NULL;
    QBCarouselMenuPlay(self, SVSTRING("Play"), (SvDBRawObject) self->selectedProduct, true);
    SVTESTRELEASE(self->purchase);
    self->purchase = NULL;
    QBCarouselSideMenuHide(self);
}

SvLocal void
QBCarouselMenuPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBMStoreCarouselMenuChoice self = self_;
    if (self)
        self->dialog = NULL;
}

SvLocal SvWidget QBMStoreCarouselCreatePopup(AppGlobals appGlobals, const char *title, const char *message)
{
    svSettingsPushComponent("Carousel_VOD.settings");

    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0
    };

    SvWidget dialog = QBSimpleDialogCreate(&params, title, message);
    svSettingsPopComponent();

    return dialog;
}

SvLocal bool
QBMStoreCarouselNeedsEmm(QBMStoreCarouselMenuChoice self, SvArray orders)
{
    assert(self && orders);

    SvValue orderIdV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) self->selectedProduct, "orderingMovieID");
    if (!orderIdV || !SvValueIsInteger(orderIdV)) {
        SvLogWarning("%s: Value of wrong type passed!", __func__);
        return false;
    }

    SvIterator it = SvArrayIterator(orders);
    MStoreVoDOrder order = NULL;
    while ((order = (MStoreVoDOrder) SvIteratorGetNext(&it))) {
        // if SmartCard already have event with the same id as selected product and if it is not outdated, then it means we also have EMM
        if (SvObjectEquals((SvObject) orderIdV, (SvObject) order->id) && SvTimeNow() - order->authEnd < 0)
            return false;
    }
    // no matching event was found on SmartCard, we need to have new EMM delivered from MW
    return true;
}

SvLocal void
QBMStoreCarouselEventsListCallback(void *self_, SvArray orders, SvString msg)
{
    QBMStoreCarouselMenuChoice self = (QBMStoreCarouselMenuChoice) self_;

    bool needEmm = true;
    if (orders)
        needEmm = QBMStoreCarouselNeedsEmm(self, orders);

    if (self->dialog) {
        QBDialogBreak(self->dialog);
        self->dialog = NULL;
    }

    QBCarouselMenuPlay(self, SVSTRING("Play"), (SvDBRawObject) self->selectedProduct, needEmm);
}

SvLocal void
QBCarouselSideMenuPlayCallback(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBMStoreCarouselMenuChoice self = self_;

    assert(self);

    if (self->dialog)
        QBDialogBreak(self->dialog);

    QBCarouselSideMenuHide(self);

    svSettingsPushComponent("Carousel_VOD.settings");
    self->dialog = QBAnimationDialogCreate(self->appGlobals->res, self->appGlobals->controller);
    svSettingsPopComponent();
    if (self->dialog)
        QBDialogRun(self->dialog, self, QBCarouselMenuPopupCallback);

    QBMStoreLogicScheduleOrderHistory(self->appGlobals->mstoreLogic, QBMStoreCarouselEventsListCallback, self);
}

SvLocal void
QBCarouselSideMenuPlayTrailerCallback(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBMStoreCarouselMenuChoice self = self_;
    assert(self && id);

    QBCarouselMenuPlay(self, id, (SvDBRawObject) self->selectedProduct, false);
}


SvLocal void
QBCarouselProductPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBMStoreCarouselMenuChoice self = self_;
    self->dialog = NULL;
    log_debug("%s PIN popup exits with button '%s'", __func__, buttonTag ? SvStringCString(buttonTag) : "--");
    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        SVTESTRELEASE(self->purchase);

        // we assume that in MStore every product requires price
        // return code is also required
        QBInnov8onPurchaseTransactionParams params = {
            .retCodeRequired = true,
            .isPriceRequired = true,
            .price = self->price
        };
        QBInnov8onPurchaseTransaction purchaseTransaction = QBInnov8onPurchaseTransactionCreate(self->provider, self->selectedOfferIndex, params, NULL);

        SvObject transaction = (SvObject) QBInnov8onPurchaseTransactionCreateRequest(self->appGlobals, purchaseTransaction);
        SVTESTRELEASE(self->transaction);
        self->transaction = transaction;

        SvObject finalTransaction = QBPurchaseLogicNewTransaction(self->appGlobals->purchaseLogic,
                                                                  self->selectedProduct,
                                                                  (SvObject) transaction,
                                                                  self->provider);

        self->purchase = QBProductPurchaseNew(finalTransaction,
                                              (SvObject) self->selectedProduct,
                                              self->appGlobals);

        QBProductPurchaseSetListener(self->purchase, (SvObject) self);
        SVRELEASE(finalTransaction);
        SVRELEASE(purchaseTransaction);
    }
}

SvLocal void
QBCarouselSetOfferID(QBMStoreCarouselMenuChoice self, SvValue offerIdV)
{
    assert(self && offerIdV);

    int offerId = -1;
    if (SvValueIsInteger(offerIdV)) {
        offerId = SvValueGetInteger(offerIdV);
    } else if (SvValueIsString(offerIdV)) {
        offerId = atoi(SvStringCString(SvValueGetString(offerIdV)));
    }
    self->selectedOfferIndex = offerId;
}

SvLocal void
QBCarouselCreateAndAttachTitleWidget(QBMStoreCarouselMenuChoice self, SvDBRawObject product, SvWidget w)
{
    assert(self && product);

    SvApplication app = self->appGlobals->res;
    SvWidget title = QBAsyncLabelNew(app, "purchaseInfo.title", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(w, title, "purchaseInfo.title", 1);
    SvString name = SvValueGetString((SvValue) SvDBRawObjectGetAttrValue(product, "name"));
    SvString titleStr = SvStringCreateWithFormat(gettext("Title: %s"), SvStringCString(name));
    if (titleStr)
        QBAsyncLabelSetText(title, titleStr);

    SVRELEASE(titleStr);
}

SvLocal void
QBCarouselCreateAndAttachPriceWidget(QBMStoreCarouselMenuChoice self, SvDBObject offer, SvWidget w)
{
    assert(self && offer);

    SvApplication app = self->appGlobals->res;

    SvWidget price = QBAsyncLabelNew(app, "purchaseInfo.price", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(w, price, "purchaseInfo.price", 1);

    SvValue currencyV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) offer, "currency");
    SvString currency = NULL;
    if (likely(currencyV && SvObjectIsInstanceOf((SvObject) currencyV, SvObject_getType()) && SvValueIsInteger(currencyV))) {
        currency = SvValueGetString(currencyV);
    } else {
        currency = SVSTRING("");
    }

    SvValue priceVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) offer, "price");

    if (priceVal && SvObjectIsInstanceOf((SvObject) priceVal, SvValue_getType()) && SvValueIsInteger(priceVal))
        self->price = SvValueGetInteger(priceVal);
    else
        self->price = 0;

    SvString priceStr = SvStringCreateWithFormat(gettext("Price: %i.%02i %s"), self->price / 100, self->price % 100, SvStringCString(currency));
    QBAsyncLabelSetText(price, priceStr);
    SVRELEASE(priceStr);
}

SvLocal void
QBCarouselPurchaseMStoreOffer(QBMStoreCarouselMenuChoice self, SvDBObject offer)
{
    assert(self && offer);

    SvValue offerIdV = SvDBObjectGetID(offer);
    if (offerIdV && QBInnov8onOfferIsAvailable((SvDBRawObject) offer)) {
        QBCarouselSetOfferID(self, offerIdV);

        SvString domain = NULL;
        const char *forcedDomain = getenv("INNOV8ON_VOD_AUTH");
        if (forcedDomain) {
            domain = SvStringCreate(forcedDomain, NULL);
        } else {
            domain = SvStringCreate("VOD_AUTH", NULL);
        }
        SvObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, domain);
        SVTESTRELEASE(domain);
        svSettingsRestoreContext(self->settingsCtx);
        self->dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"), 0, true, NULL, NULL);
        svSettingsPopComponent();

        svSettingsRestoreContext(self->settingsCtx);
        SvApplication app = self->appGlobals->res;
        SvWidget w = svSettingsWidgetCreate(app, "purchaseInfo");

        QBCarouselCreateAndAttachTitleWidget(self, (SvDBRawObject) self->selectedProduct, w);
        QBCarouselCreateAndAttachPriceWidget(self, offer, w);

        QBAuthDialogSetMessageWidget(self->dialog, w);
        svSettingsPopComponent();
    } else {
        SvString name = SvValueGetString((SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) self->selectedProduct, "name"));
        SvString titleStr = SvStringCreateWithFormat(gettext("Title: %s"), SvStringCString(name));
        self->dialog = QBMStoreCarouselCreatePopup(self->appGlobals, SvStringCString(titleStr),
                                                   gettext("The offer You are trying to access is a part of a package.\nPlease contact operator for further instructions."));
        SVRELEASE(titleStr);
    }

    QBDialogRun(self->dialog, self, QBCarouselProductPopupCallback);
    QBCarouselSideMenuHide(self);
}

SvLocal SvDBRawObject
QBMStoreCarouselFindOfferToPurchase(QBMStoreCarouselMenuChoice self, SvArray offers, SvString id)
{
    assert(self && offers && id);

    SvIterator it = SvArrayIterator(offers);
    SvDBReference offerRef = NULL;
    SvDBRawObject offer = NULL;
    while ((offerRef = (SvDBReference) SvIteratorGetNext(&it))) {
        offer = (SvDBRawObject) SvDBReferenceGetReferredObject(offerRef);
        if (!offer) {
            continue;
        }
        SvValue nameV = (SvValue) SvDBRawObjectGetAttrValue(offer, "name");
        if (!nameV || !SvObjectIsInstanceOf((SvObject) nameV, SvValue_getType()) || !SvValueIsString(nameV)) {
            continue;
        }
        SvString name = SvValueGetString(nameV);
        if (SvObjectEquals((SvObject) name, (SvObject) id)) {
            return offer;
            break;
        }
    }

    return offer;
}

SvLocal void
QBCarouselSideMenuOfferCallback(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBMStoreCarouselMenuChoice self = (QBMStoreCarouselMenuChoice) self_;
    SvArray offers = (SvArray) SvDBRawObjectGetAttrValue((SvDBRawObject) self->selectedProduct, "offers");
    if (!offers) {
        goto fini;
    }

    SvDBRawObject offer = QBMStoreCarouselFindOfferToPurchase(self, offers, id);

    if (offer)
        QBCarouselPurchaseMStoreOffer(self, (SvDBObject) offer);

fini:
    QBCarouselSideMenuHide(self);
}

SvLocal void
QBMStoreCarouselPrepareAndPushOffersPane(QBMStoreCarouselMenuChoice self, SvArray offers)
{
    // shows offers to choose from
    svSettingsRestoreContext(self->settingsCtx);
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer,
                    self->sidemenu.ctx, 2, SVSTRING("BasicPane"));
    SvIterator it = SvArrayIterator(offers);
    SvObject offerRef = NULL;
    while ((offerRef = SvIteratorGetNext(&it))) {
        SvDBRawObject offer = (SvDBRawObject) SvDBReferenceGetReferredObject((SvDBReference) offerRef);
        SvValue nameV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) offer, "name");
        if (nameV && SvObjectIsInstanceOf((SvObject) nameV, SvValue_getType()) && SvValueIsString(nameV)) {
            SvString name = SvValueGetString(nameV);
            QBBasicPaneAddOption(options, name, name, QBCarouselSideMenuOfferCallback, self);
        }
    }
    svSettingsPopComponent();
    QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) options);
    QBContextMenuShow(self->sidemenu.ctx);
    SVRELEASE(options);
}

SvLocal void
QBMStoreCarouselOrderNowCallback(void *self_,
                                 SvString id,
                                 QBBasicPane pane,
                                 QBBasicPaneItem item)
{
    QBMStoreCarouselMenuChoice self = self_;

    if (!self->selectedProduct && !SvObjectIsInstanceOf(self->selectedProduct, SvDBRawObject_getType()))
        return;

    SvArray offers = (SvArray) SvDBRawObjectGetAttrValue((SvDBRawObject) self->selectedProduct, "offers");

    if (SvArrayCount(offers) == 1) {
        SvDBReference offerRef = (SvDBReference) SvArrayObjectAtIndex(offers, 0);
        QBCarouselPurchaseMStoreOffer(self, SvDBReferenceGetReferredObject(offerRef));
        QBCarouselSideMenuHide(self);
    } else {
        log_debug("%s more than 1 offers to choose - pushing next sidemenu", __func__);
        QBMStoreCarouselPrepareAndPushOffersPane(self, offers);
    }
}

SvLocal void
QBMStoreCarouselUpdateSelectedItem(QBMStoreCarouselMenuChoice self, SvObject recievedProduct, SvErrorInfo *errorOut)
{
    assert(self && self->selectedProduct && recievedProduct);


    SvArray offersList = (SvArray) SvDBRawObjectGetAttrValue((SvDBRawObject) recievedProduct, "offers");
    SvErrorInfo error = NULL;

    if (!offersList || SvArrayCount(offersList) == 0) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidArgument, error,
                                           "QBMStoreCarouselUpdateSelectedItem failed. Recieved empty offers list.");
        goto err;
    }

    // We need to fill MStore product with offers and other data received from MW
    SvArray offers = SvArrayCreate(NULL);
    SvDBReference offerRef = NULL;
    SvIterator it = SvArrayIterator(offersList);
    SvDataBucket storage = SvDataBucketCreate(NULL);

    while ((offerRef = (SvDBReference) SvIteratorGetNext(&it))) {
        SvObject offer = (SvObject) SvDBReferenceGetReferredObject(offerRef);
        if (!offer)
            continue;
        SvArrayAddObject(offers, offer);
    }

    SvDBRawObjectSetAttrValue((SvDBRawObject) self->selectedProduct, "offers", (SvObject) offersList);
    SvArrayAddObject(offers, self->selectedProduct);
    SvDataBucketMerge(storage, offers, NULL);

    SvValue drmV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) recievedProduct, "drm");
    if (drmV && SvObjectIsInstanceOf((SvObject) drmV, SvValue_getType()))
        SvDBRawObjectAddAttribute((SvDBRawObject) self->selectedProduct, SVSTRING("drm"), (SvObject) drmV);

    SvValue entitlementV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) recievedProduct, "expires_at");
    if (entitlementV && SvObjectIsInstanceOf((SvObject) entitlementV, SvValue_getType()))
        SvDBRawObjectAddAttribute((SvDBRawObject) self->selectedProduct, SVSTRING("expires_at"), (SvObject) entitlementV);

    SVRELEASE(offers);
    SVRELEASE(storage);

err:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBMStoreCarouselSetupPane(void* self_, QBInnov8onLoadablePane pane, SvObject product)
{
    QBMStoreCarouselMenuChoice self = self_;

    assert(self && pane);

    // Update selected product with data recieved from MW
    if (self->category) {
        SvObject itemsSource = QBContentCategoryGetItemsSource(self->category);
        SvObject recievedProduct = SvInvokeInterface(QBListModel, itemsSource, getObject, 0);
        if (recievedProduct)
            QBMStoreCarouselUpdateSelectedItem(self, recievedProduct, NULL);
    }

    SvString option = NULL;
    bool entitled = QBInnov8onProductIsEntitled((SvDBRawObject) self->selectedProduct);
    log_debug("%s: Selected product is %s", __func__, entitled ? "entitled" : "not entitled");

    CubiwareMWCustomerInfoStatus customerStatus = CubiwareMWCustomerInfoGetStatus(self->appGlobals->customerInfoMonitor);

    if (customerStatus == CubiwareMWCustomerInfoStatus_ok && self->haveMWprovider && !entitled) {
        option = SvStringCreate(gettext("Order Now"), NULL);
        QBInnov8onLoadablePaneAddOption(pane, SVSTRING("order_now"), option, QBMStoreCarouselOrderNowCallback, self);
        SVRELEASE(option);
    }

    if (QBMStoreLogicHasOrderBySMS(self->appGlobals->mstoreLogic)) {
        option = SvStringCreate(gettext("Order by SMS"), NULL);
        static struct QBContainerPaneCallbacks_t orderBySMSCallbacks = {
            .onShow    = QBCarouselOrderBySMSOnShow,
            .onHide    = QBCarouselExtendedInfoOnHide,
            .setActive = QBCarouselExtendedInfoSetActive,
        };
        svSettingsRestoreContext(self->extendedInfo.ctx);
        QBInnov8onLoadablePaneAddOptionWithContainer(pane, SVSTRING("order_by_sms"), option, SVSTRING("ExtendedInfo"), &orderBySMSCallbacks, self);
        svSettingsPopComponent();
        SVRELEASE(option);
    }

    option = SvStringCreate(gettext("Play"), NULL);
    QBInnov8onLoadablePaneAddOption(pane, SVSTRING("play"), option, QBCarouselSideMenuPlayCallback, self);
    SVRELEASE(option);

    SvValue trailerVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) self->selectedProduct, "trailer");
    SvString trailer = SvValueTryGetString(trailerVal);
    if (trailer) {
        option = SvStringCreate(gettext("Play Trailer"), NULL);
        QBInnov8onLoadablePaneAddOption(pane, SVSTRING("play trailer"), option,
                                        QBCarouselSideMenuPlayTrailerCallback, self);
        SVRELEASE(option);
    }

    option = SvStringCreate(gettext("More info"), NULL);
    static struct QBContainerPaneCallbacks_t moreInfoCallbacks = {
        .onShow = QBCarouselExtendedInfoOnShow,
        .onHide = QBCarouselExtendedInfoOnHide,
        .setActive = QBCarouselExtendedInfoSetActive,
    };
    svSettingsRestoreContext(self->extendedInfo.ctx);
    QBInnov8onLoadablePaneAddOptionWithContainer(pane, SVSTRING("more info"), option, SVSTRING("ExtendedInfo"), &moreInfoCallbacks, self);
    svSettingsPopComponent();
    SVRELEASE(option);

    QBInnov8onLoadablePaneSetLoadingState(pane, true);
    QBInnov8onLoadablePaneOnLoadFinished(pane);
}

SvLocal void
QBMStoreCarouselSideMenuInnov8onCallback(void *self_, QBInnov8onLoadablePane pane, SvObject product)
{
    QBMStoreCarouselMenuChoice self = self_;

    assert(self && pane);

    QBMStoreCarouselSetupPane(self, pane, product);
}

SvLocal bool
QBMStoreCarouselItemChoiceCreateMWProvider(QBMStoreCarouselMenuChoice self, CubiwareMWCustomerInfoStatus customerStatus)
{
    assert(self);

    SVTESTRELEASE(self->innov8onProvider);
    self->innov8onProvider = NULL;

    SvArray mounts = QBProvidersControllerServiceGetServices(self->appGlobals->providersController, SVSTRING("VOD"),
                                                             QBProvidersControllerService_INNOV8ON);

    if (!mounts || SvArrayCount(mounts) == 0)
        return false;

    bool found = false;
    const char* serviceName = "VOD";
    SvIterator it = SvArrayIterator(mounts);
    QBCarouselMenuItemService menu;

    while ((menu = (QBCarouselMenuItemService) SvIteratorGetNext(&it))) {
        if (SvStringEqualToCString(QBCarouselMenuGetServiceName(menu), serviceName)) {
            found = true;
            break;
        }
    }

    if (!found) {
        SvLogWarning("%s: Did not find service [%s]", __func__, serviceName);
        return false;
    }

    SvValue categoryIdVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) self->selectedProduct, "name");
    SvString categoryId = SvValueTryGetString(categoryIdVal);
    QBContentTree tree = QBContentTreeCreate(categoryId, NULL);

    SvXMLRPCServerInfo serverRpcInfo = (SvXMLRPCServerInfo) QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager,
                                                                                                   QBMiddlewareManagerType_Innov8on);
    SvString deviceID = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager,
                                                 QBMiddlewareManagerType_Innov8on);

    SvObject plugin = Innov8onProviderRequestPluginGetProductsCreate();
    Innov8onProviderParams params = Innov8onProviderParamsCreate(serverRpcInfo, deviceID, 3, 1, 1, 1, 60, NULL, NULL, plugin, false, NULL);
    self->innov8onProvider = (SvObject) Innov8onProviderCreate(tree, params, QBCarouselMenuGetServiceID(menu), NULL, true, false, NULL);
    SVRELEASE(plugin);
    SVRELEASE(params);

    if (!self->innov8onProvider) {
        SVRELEASE(tree);
        return false;
    }

    QBContentCategory category = QBContentTreeGetRootCategory(tree);
    SVTESTRELEASE(self->category);
    self->category = SVRETAIN(category);

    SvArray dataSourceArray = SvArrayCreateWithCapacity(1, NULL);
    SvArrayInsertObjectAtIndex(dataSourceArray, self->selectedProduct, 0);

    QBRangeTree rangeTree = QBRangeTreeCreate();
    QBRangeTreeAddRange(rangeTree, 0, dataSourceArray);
    QBContentCategorySetItemsSource(self->category, (SvObject) rangeTree);
    QBRangeTreeSetRangeSize(rangeTree, 0);

    SvInvokeVirtual(QBContentProvider, (QBContentProvider) self->innov8onProvider, start, self->appGlobals->scheduler);

    SVRELEASE(tree);
    SVRELEASE(rangeTree);
    SVRELEASE(dataSourceArray);

    return true;
}

SvLocal void QBCarouselMenuPaneInit(SvObject self_, SvObject path_)
{
    QBMStoreCarouselMenuChoice self = (QBMStoreCarouselMenuChoice) self_;

    svSettingsPushComponent("LoadableBasicPane.settings");
    QBInnov8onLoadablePane pane = (QBInnov8onLoadablePane) SvTypeAllocateInstance(QBInnov8onLoadablePane_getType(), NULL);
    QBInnov8onLoadablePaneInit(pane, self->sidemenu.ctx, self->appGlobals);

    SVTESTRELEASE(self->innov8onProvider);
    self->innov8onProvider = NULL;
    self->haveMWprovider = false;

    CubiwareMWCustomerInfoStatus customerStatus = CubiwareMWCustomerInfoGetStatus(self->appGlobals->customerInfoMonitor);
    log_debug("%s(): customer status: %s", __func__,
              customerStatus == CubiwareMWCustomerInfoStatus_ok ? "CubiwareMWCustomerInfoStatus_ok" :
              customerStatus == CubiwareMWCustomerInfoStatus_error ? "CubiwareMWCustomerInfoStatus_error" : "CubiwareMWCustomerInfoStatus_unknown");

    if (customerStatus == CubiwareMWCustomerInfoStatus_ok) {
        self->haveMWprovider = QBMStoreCarouselItemChoiceCreateMWProvider(self, customerStatus);

        if (self->haveMWprovider) {
            struct QBInnov8onLoadablePaneRefreshParams_ params = {
                .product  = (SvObject) self->selectedProduct,
                .provider = self->innov8onProvider,
                .category = (SvObject) self->category
            };

            QBInnov8onLoadablePaneSetCallback(pane, QBMStoreCarouselSideMenuInnov8onCallback, self);
            QBInnov8onLoadablePaneSetRefreshParams(pane, &params);
        }
    }

    QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) pane);
    QBContextMenuShow(self->sidemenu.ctx);

    if (!self->haveMWprovider)
        QBMStoreCarouselSetupPane(self, pane, self->selectedProduct);

    SVRELEASE(pane);
    svSettingsPopComponent();
}

SvLocal void
QBCarouselMenuChoosen(SvObject self_, SvObject node, SvObject nodePath_, int position)
{
    QBMStoreCarouselMenuChoice self = (QBMStoreCarouselMenuChoice) self_;

    if (!SvObjectIsInstanceOf(node, SvDBRawObject_getType())) {
        SvLogInfo("%s Menu node chosen is not SvDBRawObject.", __func__);
        return;
    }
    SVTESTRELEASE(self->selectedProduct);
    self->selectedProduct = SVRETAIN(node);
    QBCarouselMenuPaneInit((SvObject) self, nodePath_);
}

SvLocal void
QBCarouselMenuOSKKeyTyped(void *self_, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBMStoreCarouselMenuChoice self = (QBMStoreCarouselMenuChoice) self_;

    if (key->type == QBOSKKeyType_enter) {
        if (input && SvStringLength(input) > 0) {
            const char* begin = SvStringCString(input);
            const char* end = begin + SvStringLength(input) - 1;
            while (*begin != '\0' && isspace(*begin))
                begin++;
            if (begin > end)
                return;
            while (end > begin && isspace(*end))
                end--;
            char* final = calloc(end - begin + 2, sizeof(char));
            strncpy(final, begin, end - begin + 1);
            SvString keyword = SvStringCreate(final, NULL);
            free(final);

            QBContentSearchStartNewSearch(self->contentSearch, keyword, self->appGlobals->scheduler);
            SVRELEASE(keyword);

            // Enable for VOD2
            //if (QBCarouselMenuPushContext(self, self->contentSearchPath, true))
            //    return;

            SvObject path = self->contentSearchPath;
            QBTreeIterator tit = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, path, 0);

            path = QBTreeIteratorGetCurrentNodePath(&tit);

            if (path) {
                SvWidget menuBar = SvInvokeInterface(QBMainMenu, (SvObject) self->appGlobals->main, getMenu);
                SvInvokeInterface(QBMenu, menuBar->prv, setPosition, path, NULL);
            }

            QBContextMenuHide(self->sidemenu.ctx, false);

            SVTESTRELEASE(self->contentSearch);
            SVTESTRELEASE(self->contentSearchPath);
            self->contentSearch = NULL;
            self->contentSearchPath = NULL;
        }
    }
}

SvLocal inline void
insertErrorMessage(QBContentCategory category, SvString msg)
{
    QBActiveArray items = (QBActiveArray) QBContentCategoryGetItemsSource(category);
    QBActiveArrayRemoveAllObjects(items, NULL);
    QBActiveTreeNode errorMsg = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBActiveTreeNodeSetAttribute(errorMsg, SVSTRING("caption"), (SvObject) msg);
    QBActiveArrayAddObject(items, (SvObject) errorMsg, NULL);
    SVRELEASE(errorMsg);
}

SvLocal void
QBCarouselMenuMyRentalsCallback(void *self_, SvArray orders, SvString msg)
{
    QBMStoreCarouselMenuChoice self = self_;

    if (!self->selectedHistoryNode)
        return;

    if (orders)
        MStoreVoDProviderSetMyRentals((MStoreVoDProvider) self->provider, orders, NULL);
    else if (msg)
        insertErrorMessage((QBContentCategory) self->selectedHistoryNode, msg);
}

SvLocal void
QBCarouselMenuOrderHistoryCallback(void *self_, SvArray orders, SvString msg)
{
    QBMStoreCarouselMenuChoice self = self_;

    if (orders)
        MStoreVoDProviderSetOrderHistory((MStoreVoDProvider) self->provider, orders, NULL);
    else if (msg)
        insertErrorMessage((QBContentCategory) self->selectedHistoryNode, msg);
}

SvLocal void QBMStoreCarouselMenuCheckCategoryParentalControlPINCallback(void *self_, SvWidget dlg, SvString ret, unsigned key)
{
    QBMStoreCarouselMenuChoice self = self_;

    if (!self->parentalPopup || !self->protectedCategoryPath) {
        SVTESTRELEASE(self->protectedCategoryPath);
        self->protectedCategoryPath = NULL;
        self->parentalPopup = NULL;
        return;
    }

    self->parentalPopup = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        // enter protected category
        QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, self->protectedCategoryPath, 0);
        if (QBTreeIteratorGetNodesCount(&iter) > 0) {
            SvObject destPath = QBTreeIteratorGetCurrentNodePath(&iter);
            SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
            SvInvokeInterface(QBMenu, menuBar->prv, setPosition, destPath, NULL);
        }
    }

    SVRELEASE(self->protectedCategoryPath);
    self->protectedCategoryPath = NULL;
}

SvLocal bool QBMStoreCarouselMenuIsCategoryAdult(QBContentCategory category)
{
    SvValue isAdult = (SvValue) QBContentCategoryGetAttribute(category, SVSTRING("isAdult"));

    if (isAdult && SvObjectIsInstanceOf((SvObject) isAdult, SvValue_getType()) && SvValueIsBoolean(isAdult)) {
        if (SvValueGetBoolean(isAdult))
            return true;
    }

    return false;
}

SvLocal bool QBMStoreCarouselMenuCheckCategoryParentalRating(QBMStoreCarouselMenuChoice self, QBContentCategory category, SvObject path)
{
    if (!QBMStoreCarouselMenuIsCategoryAdult(category))
        return false;

    // remember path to this node to use it in dialog's callback
    SVTESTRELEASE(self->protectedCategoryPath);
    self->protectedCategoryPath = SVRETAIN(path);

    svSettingsPushComponent("Carousel_VOD.settings");
    SvObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, SVSTRING("PC"));
    SvWidget master = NULL;
    SvWidget dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"), gettext("Please enter parental control PIN to enter this category"), true, NULL, &master);
    self->parentalPopup = dialog;
    QBDialogRun(dialog, self, QBMStoreCarouselMenuCheckCategoryParentalControlPINCallback);
    svSettingsPopComponent();

    return true;
}

SvLocal bool
QBCarouselMenuNodeSelected(SvObject self_, SvObject node_, SvObject nodePath)
{
    QBMStoreCarouselMenuChoice self = (QBMStoreCarouselMenuChoice) self_;

    if (SvObjectIsInstanceOf(node_, QBContentSearch_getType())) {
        SvString keyword;
        QBContentSearch search = (QBContentSearch) node_;
        QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);

        SVRETAIN(search);
        SVTESTRELEASE(self->contentSearch);
        self->contentSearch = search;
        SVTESTRELEASE(self->contentSearchPath);
        self->contentSearchPath = SvObjectCopy(nodePath, NULL);

        svSettingsRestoreContext(self->settingsCtx);
        SvErrorInfo error = NULL;
        QBOSKPaneInit(oskPane, self->appGlobals->scheduler, self->sidemenu.ctx, 1, SVSTRING("OSKPane"), QBCarouselMenuOSKKeyTyped, self, &error);
        svSettingsPopComponent();
        if (!error) {
            keyword = QBContentSearchGetKeyword(search);
            if (keyword)
                QBOSKPaneSetRawInput(oskPane, SvStringCString(keyword));

            QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) oskPane);
            QBContextMenuShow(self->sidemenu.ctx);
            SVRELEASE(oskPane);

            return true;
        } else {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            SVRELEASE(oskPane);

            return false;
        }
    } else if (SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        QBContentCategory category = (QBContentCategory) node_;
        SvString name = QBContentCategoryGetName(category);
        if (SvStringEqualToCString(name, "MY_RENTALS")) {
            QBMStoreLogicScheduleOrderHistory(self->appGlobals->mstoreLogic, QBCarouselMenuMyRentalsCallback, self);
            SVTESTRETAIN(node_);
            SVTESTRELEASE(self->selectedHistoryNode);
            self->selectedHistoryNode = node_;
            MStoreVoDProviderSetMyRentals((MStoreVoDProvider) self->provider, NULL, NULL);
        } else if (SvStringEqualToCString(name, "ORDER_HISTORY")) {
            QBMStoreLogicScheduleOrderHistory(self->appGlobals->mstoreLogic, QBCarouselMenuOrderHistoryCallback, self);
            SVTESTRETAIN(node_);
            SVTESTRELEASE(self->selectedHistoryNode);
            self->selectedHistoryNode = node_;
            MStoreVoDProviderSetOrderHistory((MStoreVoDProvider) self->provider, NULL, NULL);
        } else {
            return QBMStoreCarouselMenuCheckCategoryParentalRating(self, (QBContentCategory) node_, nodePath);
        }
    }

    return false;
}

SvLocal void
QBCarouselMenuSmartcardStatusChanged(SvObject self_)
{
    QBMStoreCarouselMenuChoice self = (QBMStoreCarouselMenuChoice) self_;

    SvString tmp = SVTESTRETAIN(QBSmartCardMonitorGetSmartcardIdWithCheckSum(self->appGlobals->smartCardMonitor));
    SVTESTRELEASE(self->smartcardId);
    self->smartcardId = tmp;

    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal void
QBMStoreCarouselMenuChoiceDestroy(void *self_)
{
    QBMStoreCarouselMenuChoice self = self_;

    if (self->sidemenu.ctx) {
        QBContextMenuSetCallbacks(self->sidemenu.ctx, NULL, NULL);
        QBContextMenuHide(self->sidemenu.ctx, false);
        SVRELEASE(self->sidemenu.ctx);
    }

    if (self->dialog)
        QBDialogBreak(self->dialog);

    SVTESTRELEASE(self->protectedCategoryPath);
    SVTESTRELEASE(self->selectedProduct);
    SVTESTRELEASE(self->selectedHistoryNode);
    SVTESTRELEASE(self->purchase);
    SVTESTRELEASE(self->transaction);

    SVRELEASE(self->provider);
    SVRELEASE(self->serviceName);

    SVTESTRELEASE(self->smartcardId);
    SVTESTRELEASE(self->innov8onProvider);
    SVTESTRELEASE(self->category);

    svWidgetDestroy(self->extendedInfo.viewport);
}

SvLocal void
QBCarouselMenuPurchaseStatusChanged(SvObject self_,
                                    SvObject transaction_,
                                    const QBContentMgrTransactionStatus *const status)
{
    QBMStoreCarouselMenuChoice self = (QBMStoreCarouselMenuChoice) self_;

    if (!self || !status)
        return;

    SvValue answerCode = NULL;
    SvString answerMsg = NULL;

    if (status->status.state == QBRemoteDataRequestState_finished) {
        log_debug("%s: Purchase succeeded.", __func__);
        SvObject answer = Innov8onProviderTransactionGetAnswer((Innov8onProviderTransaction) self->transaction, NULL);
        if (answer && SvObjectGetType(answer) == SvHashTable_getType())
            answerCode = (SvValue) SvHashTableFind((SvHashTable) answer, (SvObject) SVSTRING("success_code"));

        if (answerCode && SvValueIsInteger(answerCode))
            answerMsg = Innov8onProviderCreateMessageFromErrorCode(SvValueGetInteger(answerCode));

        if (!answerMsg) {
            SvLogError("%s: Empty answer message", __func__);
            return;
        }

        self->dialog = QBMStoreCarouselCreatePopup(self->appGlobals, gettext("Operation Successful"), SvStringCString(answerMsg));
        QBDialogRun(self->dialog, self, QBMStoreCarouselOrderNowPlayCallback);
    } else if (status->status.state == QBRemoteDataRequestState_failed) {
        log_debug("%s: Purchase failed.", __func__);
        answerMsg = Innov8onProviderCreateMessageFromErrorCode(status->status.errorCode);
        self->dialog = QBMStoreCarouselCreatePopup(self->appGlobals,
                                                   gettext("A problem occurred"),
                                                   SvStringCString(answerMsg));

        QBDialogRun(self->dialog, self, QBCarouselMenuPopupCallback);
        SVTESTRELEASE(self->purchase);
        self->purchase = NULL;
    } else if (status->status.state == QBRemoteDataRequestState_cancelled) {
        if (self->dialog) {
            QBDialogBreak(self->dialog);
            self->dialog = NULL;
        }

        log_debug("%s Purchase cancelled", __func__);
        SVTESTRELEASE(self->purchase);
        self->purchase = NULL;
    }

    SVTESTRELEASE(answerMsg);
}

SvLocal SvType
QBMStoreCarouselMenuChoice_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMStoreCarouselMenuChoiceDestroy
    };
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBCarouselMenuChoosen
    };

    static const struct QBMenuEventHandler_ selectedMethods = {
        .nodeSelected = QBCarouselMenuNodeSelected,
    };

    static const struct QBSmartCardMonitorListener_t smartcardMonitorMethods = {
        .statusChanged = QBCarouselMenuSmartcardStatusChanged
    };

    static const struct QBContentMgrTransactionListener_ productPurchaseStatus = {
        .statusChanged = QBCarouselMenuPurchaseStatusChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMStoreCarouselMenuChoice",
                            sizeof(struct QBMStoreCarouselMenuChoice_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBMenuEventHandler_getInterface(), &selectedMethods,
                            QBSmartCardMonitorListener_getInterface(), &smartcardMonitorMethods,
                            QBContentMgrTransactionListener_getInterface(), &productPurchaseStatus,
                            NULL);
    }

    return type;
}

QBMStoreCarouselMenuChoice
QBMStoreCarouselMenuChoiceNew(AppGlobals appGlobals, SvObject vodProvider, SvString serviceName)
{
    QBMStoreCarouselMenuChoice self = (QBMStoreCarouselMenuChoice) SvTypeAllocateInstance(QBMStoreCarouselMenuChoice_getType(), NULL);

    self->provider = SVRETAIN(vodProvider);
    self->appGlobals = appGlobals;
    self->serviceName = SVRETAIN(serviceName);
    self->settingsCtx = svSettingsSaveContext();

    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBCarouselSideMenuClose, self);

    svSettingsPushComponent("carouselMStoreExtendedInfo.settings");
    self->extendedInfo.ctx = svSettingsSaveContext();
    self->extendedInfo.grid = QBGridCreate(appGlobals->res, "grid");
    self->extendedInfo.viewport = svViewPortNew(appGlobals->res, "viewport");
    svViewPortSetContents(self->extendedInfo.viewport, self->extendedInfo.grid);
    self->extendedInfo.title = QBGridAddAsyncLabel(self->extendedInfo.grid, "title", appGlobals->textRenderer);
    self->extendedInfo.description = QBGridAddAsyncLabel(self->extendedInfo.grid, "description", appGlobals->textRenderer);
    self->extendedInfo.image = QBGridAddIcon(self->extendedInfo.grid, "image");
    svSettingsPopComponent();

    self->smartcardId = SVTESTRETAIN(QBSmartCardMonitorGetSmartcardIdWithCheckSum(self->appGlobals->smartCardMonitor));
    QBSmartCardMonitorAddListener(self->appGlobals->smartCardMonitor, (SvObject) self);

    return self;
}
