/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#include <ContextMenus/QBContentSideMenu.h>

#include <QBSecureLogManager.h>
#include <libintl.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/widget.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <QBContentManager/QBContentProvider.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBContextMenuBase.h>
#include <ContextMenus/QBInnov8onExtendedInfoPane.h>
#include <ContextMenus/QBInnov8onLoadablePane.h>
#include "QBBookmarkService/QBBookmarkManager.h"
#include <QBPlayerControllers/QBAnyPlayerController.h>
#include <Middlewares/Innov8on/innov8onTransaction.h>
#include <QBContentManager/Innov8onUtils.h>
#include <Utils/authenticators.h>
#include <Utils/QBContentInfo.h>
#include <SvFoundation/SvObject.h>
#include <Utils/QBMovieUtils.h>
#include <Utils/QBVODUtils.h>
#include <Utils/productUtils.h>
#include <Utils/contentCategoryUtils.h>
#include <Middlewares/productPurchase.h>
#include <SvDataBucket2/SvDBReference.h>
#include <Widgets/QBSimpleDialog.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <Windows/pvrplayer.h>
#include <player_hints/http_input.h>
#include <settings.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBAppKit/Utils/commonUtils.h>
#include <main.h>
#include <QBJSONUtils.h>


struct QBContentSideMenu_t {
    struct QBContextMenu_t super_;

    void* owner;

    AppGlobals appGlobals;

    QBContentInfo contentInfo;

    QBInnov8onLoadablePane pane;

    bool entitled;
    bool initialized;

    QBProductPurchase purchase;

    SvWidget dialog;

    SvString selectedOptionID;

    int selectedOffer;
    struct {
        int price;
        bool isPriceRequired;
    } selectedPrice;

    struct {
        QBContentSideMenuPaneInitCallback paneInit;
    } callbacks;

    SvString optionFile;

    SvObject provider;
    SvString accessDomain;      ///< Determine domain for QBAccessManager.
    SvObject eventReporter;     ///< Event reporter used to send notification about occurred user activities
    QBContentSideMenuImmediateAction immediateAction; ///< action the user wants to perform as soon as possible. For example play content if it is purchased
    SvFiber postponeFiber;  ///< fiber for postponing immediate Play/Rent, to be executed outside of offer refresh callback
    bool refreshObject;     ///< flag indicationg if the data obtained from MW should be writen to the source object overwriting old values
};

SvLocal SvWidget QBContentSideMenuCreatePopup(AppGlobals appGlobals, const char *title, const char *message);
SvLocal void QBContentSideMenuPurchaseDisabled(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item);
SvLocal void QBContentSideMenuRefresh(QBContentSideMenu self, Innov8onProvider rootProvider, QBContentCategory activeTreePath, SvDBRawObject product);


SvLocal void
QBContentSideMenuProductPopupCallback(void *self_, SvWidget dialog, SvString buttonTag,
                                      unsigned keyCode)
{
    QBContentSideMenu self = self_;
    self->dialog = NULL;
    SvLogNotice("%s PIN popup exits with button '%s'", __func__,
                buttonTag ? SvStringCString(buttonTag) : "--");
    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        SVTESTRELEASE(self->purchase);
        SvGenericObject provider = QBContentInfoGetProvider(self->contentInfo);
        SvLogNotice("%s Start transaction, provider %p, offer %d", __func__, provider, self->selectedOffer);

        QBInnov8onPurchaseTransactionParams params = {
            .retCodeRequired = false,
            .isPriceRequired = self->selectedPrice.isPriceRequired,
            .price = self->selectedPrice.price
        };

        QBInnov8onPurchaseTransaction purchaseTransaction = QBInnov8onPurchaseTransactionCreate(provider, self->selectedOffer, params, NULL);
        SvGenericObject transaction = (SvGenericObject) QBInnov8onPurchaseTransactionCreateRequest(self->appGlobals, purchaseTransaction);

        SvGenericObject movie = (SvGenericObject) QBContentInfoGetMovie(self->contentInfo);
        SvGenericObject finalTransaction = QBPurchaseLogicNewTransaction(self->appGlobals->purchaseLogic,
                                                                         movie,
                                                                         transaction, provider);

        self->purchase = QBProductPurchaseNew(finalTransaction,
                                              movie,
                                              self->appGlobals);
        QBProductPurchaseSetListener(self->purchase, (SvGenericObject) self);
        SVRELEASE(purchaseTransaction);
        SVRELEASE(transaction);
        SVRELEASE(finalTransaction);
    }
}

SvLocal void
QBContentSideMenuPurchaseInnov8onOffer(QBContentSideMenu self, SvDBRawObject product,
                                       SvDBObject offer_)
{
    if (unlikely(!self || !offer_ || !product)) {
        return;
    }

    QBContentInfoSetMovie(self->contentInfo, (SvGenericObject) product);

    SvString errorMsg = NULL;

    SvDBRawObject offer = (SvDBRawObject) offer_;
    SvValue offerIdV = SvDBObjectGetID(offer_);
    if (offerIdV && QBInnov8onOfferIsAvailable(offer)) {
        AppGlobals appGlobals = self->appGlobals;
        int offerId = -1;
        if (SvValueIsInteger(offerIdV)) {
            offerId = SvValueGetInteger(offerIdV);
        } else if (SvValueIsString(offerIdV)) {
            offerId = atoi(SvStringCString(SvValueGetString(offerIdV)));
        }
        self->selectedOffer = offerId;

        SvValue priceRequiredVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) offer, "price_required");
        if (priceRequiredVal && SvObjectIsInstanceOf((SvObject) priceRequiredVal, SvValue_getType()) && SvValueIsInteger(priceRequiredVal))
            self->selectedPrice.isPriceRequired = (bool) SvValueGetInteger(priceRequiredVal);
        else
            self->selectedPrice.isPriceRequired = false;

        SvValue priceVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) offer, "price");
        if (priceVal && SvObjectIsInstanceOf((SvObject) priceVal, SvValue_getType()) && SvValueIsInteger(priceVal))
            self->selectedPrice.price = SvValueGetInteger(priceVal);
        else
            self->selectedPrice.price = 0;

        SvValue nameVal = (SvValue) SvDBRawObjectGetAttrValue(product, "name");
        SvString name = NULL;
        if (!nameVal) {
            nameVal = (SvValue) SvDBRawObjectGetAttrValue(product, "title");
        }
        if (nameVal && SvObjectIsInstanceOf((SvObject) nameVal, SvValue_getType()) && SvValueIsString(nameVal)) {
            name = SvValueGetString(nameVal);
        }
        if (self->selectedPrice.price == 0) {
            SvLogNotice("%s Purchasing free offer id=%d, product name='%s'. PIN is NOT required", __func__, offerId, name ? SvStringCString(name) : "--");
            QBContentSideMenuProductPopupCallback(self, NULL, SVSTRING("OK-button"), 0);
            return;
        }

        SvString domain = NULL;
        const char *forcedDomain = getenv("INNOV8ON_VOD_AUTH");
        if (forcedDomain) {
            domain = SvStringCreate(forcedDomain, NULL);
        } else {
            domain = SvStringCreate("VOD_AUTH", NULL);
        }
        SvGenericObject authenticator = NULL;
        authenticator = QBAuthenticateViaAccessManager(appGlobals->scheduler,
                                                       appGlobals->accessMgr,
                                                       domain);
        SVTESTRELEASE(domain);
        svSettingsPushComponent("AuthenticationDialog.settings");
        self->dialog = QBAuthDialogCreate(appGlobals, authenticator,
                                          gettext("Authentication required"),
                                          0, true, NULL, NULL);
        svSettingsPopComponent();

        svSettingsPushComponent("Carousel_VOD.settings");
        SvApplication app = appGlobals->res;
        SvWidget w = svSettingsWidgetCreate(app, "purchaseInfo");
        SvWidget title = QBAsyncLabelNew(app, "purchaseInfo.title", appGlobals->textRenderer);
        svSettingsWidgetAttach(w, title, "purchaseInfo.title", 1);
        SvWidget price = QBAsyncLabelNew(app, "purchaseInfo.price", appGlobals->textRenderer);
        svSettingsWidgetAttach(w, price, "purchaseInfo.price", 1);

        SvString titleStr = SvStringCreate(gettext("If you wish to purchase this asset,\nplease enter your Purchase Pin"), NULL);
        QBAsyncLabelSetText(title, titleStr);
        SVRELEASE(titleStr);

        SvValue currencyV = (SvValue) SvDBRawObjectGetAttrValue(offer, "currency");
        SvString currency = NULL;
        if (likely(currencyV && SvObjectIsInstanceOf((SvGenericObject) currencyV, SvObject_getType())
                   && SvValueIsString(currencyV))) {
            currency = SvValueGetString(currencyV);
        } else {
            currency = SVSTRING("");
        }

        SvString priceStr = SvStringCreateWithFormat(gettext("Price: %i.%02i %s"), self->selectedPrice.price / 100,
                                                     self->selectedPrice.price % 100, SvStringCString(currency));
        QBAsyncLabelSetText(price, priceStr);
        SVRELEASE(priceStr);

        QBAuthDialogSetMessageWidget(self->dialog, w);
        svSettingsPopComponent();

        SvLogNotice("%s Purchasing offer id=%d, product name='%s' price='%d %s'. PIN is required (domain=%s)",
                    __func__, offerId, name ? SvStringCString(name) : "--", self->selectedPrice.price,
                    currency ? SvStringCString(currency) : "--", domain ? SvStringCString(domain) : "--");
    } else {
        SvString titleStr = NULL;
        SvValue nameVal = (SvValue) SvDBRawObjectGetAttrValue(product, "name");
        if (!nameVal) {
            nameVal = (SvValue) SvDBRawObjectGetAttrValue(product, "title");
        }
        if (nameVal && SvValueIsString(nameVal)) {
            SvString name = SvValueGetString(nameVal);
            titleStr = SvStringCreateWithFormat(gettext("Title: %s"), SvStringCString(name));
        } else {
            titleStr = SvStringCreateEmpty(NULL);
        }
        errorMsg = SvStringCreateWithFormat("%s", gettext("The offer You are trying to access is a part of a package.\nPlease contact operator for further instructions."));
        self->dialog = QBContentSideMenuCreatePopup(self->appGlobals, SvStringCString(titleStr), SvStringCString(errorMsg));
        SVRELEASE(titleStr);
    }


    if (self->dialog) {
        QBDialogRun(self->dialog, self, QBContentSideMenuProductPopupCallback);
        if (errorMsg) {
            char* escapedMsg = QBStringCreateJSONEscapedString(SvStringCString(errorMsg));
            QBSecureLogEvent("ContentSideMenu", "ErrorShown.ContextMenus.Access_Denied", "JSON:{\"description\":\"%s\"}", escapedMsg);
            free(escapedMsg);
        }
    }
    SVTESTRELEASE(errorMsg);
    QBContextMenuHide((QBContextMenu) self, false);
}

SvLocal void
QBContentSideMenuOfferCallback(void *self_, SvString id, QBBasicPane pane,
                               QBBasicPaneItem item)
{
    QBContentSideMenu self = (QBContentSideMenu) self_;
    SvDBRawObject movie = (SvDBRawObject) QBContentInfoGetMovie(self->contentInfo);
    SvArray offers = (SvArray) SvDBRawObjectGetAttrValue(movie, "offers");
    if (!offers) {
        goto fini;
    }

    SvIterator it = SvArrayIterator(offers);
    SvDBReference offerRef = NULL;
    while ((offerRef = (SvDBReference) SvIteratorGetNext(&it))) {
        SvDBRawObject offer = (SvDBRawObject) SvDBReferenceGetReferredObject(offerRef);
        if (!offer) {
            continue;
        }
        SvValue nameV = (SvValue) SvDBRawObjectGetAttrValue(offer, "name");
        if (!nameV || !SvObjectIsInstanceOf((SvGenericObject) nameV, SvValue_getType())
            || !SvValueIsString(nameV)) {
            continue;
        }
        SvString name = SvValueGetString(nameV);
        if (SvObjectEquals((SvGenericObject) name, (SvGenericObject) id)) {
            QBContentSideMenuPurchaseInnov8onOffer(self, movie,
                                                   (SvDBObject) offer);
            break;
        }
    }
fini:
    QBContextMenuHide((QBContextMenu) self, false);
}

SvLocal void
QBContentSideMenuPurchase(void *self_, SvString id, QBBasicPane pane,
                          QBBasicPaneItem item)
{
    QBContentSideMenu self = self_;
    SVTESTRELEASE(self->selectedOptionID);
    self->selectedOptionID = SVRETAIN(id);
    SvDBRawObject movie = (SvDBRawObject) QBContentInfoGetMovie(self->contentInfo);
    if (movie) {
        SvArray offers = (SvArray) SvDBRawObjectGetAttrValue(movie, "offers");
        if (SvArrayCount(offers) == 1) {
            SvDBReference offerRef = (SvDBReference) SvArrayObjectAtIndex(offers, 0);
            QBContentSideMenuPurchaseInnov8onOffer(self, movie,
                                                   SvDBReferenceGetReferredObject(offerRef));
            QBContextMenuHide((QBContextMenu) self, false);
        } else {
            SvLogNotice("%s more than 1 offers to choose - pushing next sidemenu", __func__);
            // show offers to choose from
            svSettingsPushComponent("BasicPane.settings");
            AppGlobals appGlobals = self->appGlobals;
            QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
            QBBasicPaneInit(options, appGlobals->res, appGlobals->scheduler,
                            appGlobals->textRenderer, (QBContextMenu) self, 2,
                            SVSTRING("BasicPane"));
            SvIterator it = SvArrayIterator(offers);
            SvGenericObject offerRef = NULL;
            while ((offerRef = SvIteratorGetNext(&it))) {
                SvDBRawObject offer = (SvDBRawObject) SvDBReferenceGetReferredObject((SvDBReference) offerRef);
                SvValue nameV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) offer, "name");
                if (nameV && SvObjectIsInstanceOf((SvGenericObject) nameV, SvValue_getType())
                    && SvValueIsString(nameV)) {
                    SvString name = SvValueGetString(nameV);
                    QBBasicPaneAddOption(options, name, name,
                                         QBContentSideMenuOfferCallback, self);
                }
            }
            svSettingsPopComponent();
            QBContextMenuPushPane((QBContextMenu) self, (SvGenericObject) options);
            QBContextMenuShow((QBContextMenu) self);
            SVRELEASE(options);
        }
    }
}

SvLocal void
QBContentSideMenuPurchaseDisabled(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBContentSideMenu self = (QBContentSideMenu) self_;
    QBInnov8onLoadablePaneShowConnectionErrorPopup(self->pane, self->appGlobals);
}

SvLocal void
QBContentSideMenuMoreInfo(void *self_, SvString id, QBBasicPane pane,
                          QBBasicPaneItem item)
{
    QBContentSideMenu self = self_;

    if (!self)
        return;

    int level = QBBasicPaneGetLevel(pane);
    SvObject movie = (SvObject) QBContentInfoGetMovie(self->contentInfo);

    if (movie && SvObjectIsInstanceOf((SvObject) movie, SvDBRawObject_getType())) {
        const SvString type = QBMovieUtilsGetAttr((SvDBRawObject) movie, "type", NULL);
        if (type && SvStringEqualToCString(type, "vod")) {
            QBVoDUtilsLogMovieDetails((SvDBRawObject) movie);
        }
    }

    QBInnov8onExtendedInfoPane moreInfo =
        QBInnov8onExtendedInfoPaneCreateFromSettings("carouselExtendedInfo.settings",
                                                     self->appGlobals,
                                                     (QBContextMenu) self,
                                                     SVSTRING("ExtendedInfoPane"),
                                                     level + 1,
                                                     movie);
    QBContextMenuPushPane((QBContextMenu) self, (SvGenericObject) moreInfo);
    SVRELEASE(moreInfo);
}

SvLocal void
QBContentSideMenuExit(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBContentSideMenu self = self_;
    //Pop the PVRplayer context
    QBApplicationControllerPopContext(self->appGlobals->controller);
}

SvLocal void
QBContentSideMenuCloseContent(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBContentSideMenu self = self_;
    //Pop the PVRplayer context
    QBApplicationControllerPopContext(self->appGlobals->controller);
    //Pop the QBVoDMovieDetails context
    QBApplicationControllerPopContext(self->appGlobals->controller);
}

SvLocal void
QBContentSideMenuPlayContent(QBContentSideMenu self, SvDBRawObject movie, bool authenticated)
{
    if (!movie)
        return;

    bool lastPosition = self->selectedOptionID && SvStringEqualToCString(self->selectedOptionID, "Continue");
    bool playTrailer = self->selectedOptionID && SvStringEqualToCString(self->selectedOptionID, "Play Trailer");

    const char* attributeName = NULL;
    if (playTrailer)
        attributeName = "trailer";
    else
        attributeName = "source";

    SvString URI = QBMovieUtilsGetAttr(movie, attributeName, NULL);
    if (!URI)
        return;

    SvString proxyURI = QBMovieUtilsGetAttr(movie, "proxyURI", NULL);

    SvString coverURI = NULL;

    QBMediaFileType type = QBAppKitURIToMediaFileType(URI, NULL);

    SvContent c = NULL;
    if (URI)
        SvLogNotice("URI = %s", SvStringCString(URI));

    if (type != QBMediaFileType_audioPlaylist) {
        c = SvContentCreateFromCString(SvStringCString(URI), NULL);
        SvValue titleVal = (SvValue) SvDBRawObjectGetAttrValue(movie, "name");
        if (!titleVal) {
            titleVal = (SvValue) SvDBRawObjectGetAttrValue(movie, "title");
        }
        if (titleVal && SvObjectIsInstanceOf((SvObject) titleVal, SvValue_getType()) && SvValueIsString(titleVal)) {
            SvString title = SvValueGetString(titleVal);
            SvContentMetaDataSetStringProperty(SvContentGetMetaData(c),
                                               SVSTRING(SV_PLAYER_META__TITLE),
                                               title);
        }
        coverURI = QBMovieUtilsGetAttr(movie, "thumbnail", NULL);
    }

    if (proxyURI) {
        SvContentMetaDataSetStringProperty(SvContentGetHints(c),
                                           SVSTRING(PLAYER_HINT__HTTP_INPUT_PROXY),
                                           proxyURI);
    }

    QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->appGlobals);
    SvValue propertyKey = SvValueCreateWithString(SVSTRING("VoD"), NULL);
    SvValue propertyValue = SvValueCreateWithString(SVSTRING("Innov8on"), NULL);
    QBAnyPlayerLogicSetProperty(anyPlayerLogic, propertyKey, propertyValue);
    SVRELEASE(propertyKey);
    SVRELEASE(propertyValue);
    if (URI && !playTrailer)
        QBAnyPlayerLogicSetVodId(anyPlayerLogic, URI);

    QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals,
                                                         anyPlayerLogic);

    QBAnyPlayerLogicSetupPlaybackStateController(anyPlayerLogic);
    if (!playTrailer) {
        SvGenericObject controller = QBAnyPlayerLogicCreateController(anyPlayerLogic, self->contentInfo);
        SvInvokeInterface(QBAnyPlayerController, controller, setContent, (SvGenericObject) c);
        QBAnyPlayerLogicSetController(anyPlayerLogic, controller);
        SVRELEASE(controller);
        QBPVRPlayerContextSetContentData(pvrPlayer, NULL);
    } else {
        QBPVRPlayerContextSetContentData(pvrPlayer, (SvGenericObject) self->contentInfo);
    }

    if (self->appGlobals->bookmarkManager) {
        SvValue productIdV = SvDBObjectGetID((SvDBObject) movie);
        if (productIdV && SvValueIsString(productIdV)) {
            SvString productId = SvValueGetString(productIdV);
            QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, productId);
            if (!lastPosition) {
                if (bookmark)
                    QBBookmarkManagerRemoveBookmark(self->appGlobals->bookmarkManager, bookmark);

                bookmark = QBBookmarkManagerSetLastPositionBookmark(self->appGlobals->bookmarkManager, productId, 0.0, QBBookmarkType_Generic);
            }

            if (bookmark) {
                QBAnyPlayerLogicSetLastPositionBookmark(anyPlayerLogic, bookmark);
            }
        }
    }

    QBPVRPlayerContextSetTitle(pvrPlayer, QBContentInfoGetServiceName(self->contentInfo));
    QBPVRPlayerContextUseBookmarks(pvrPlayer, lastPosition);
    QBPVRPlayerContextSetVodMetaData(pvrPlayer, movie);
    QBPVRPlayerContextSetAccessDomain(pvrPlayer, SVSTRING("PC_VOD"));
    if (authenticated)
        QBPVRPlayerContextSetAuthenticated(pvrPlayer);

    if (coverURI) {
        QBPVRPlayerContextSetCoverURI(pvrPlayer, coverURI);
    }

    SvValue playerFeaturesV = NULL;
    playerFeaturesV = (SvValue) SvDBRawObjectGetAttrValue(movie, "playerFeatures");
    if (playerFeaturesV) {
        const char *playerFeatures = SvStringCString(SvValueGetString(playerFeaturesV));
        while (playerFeatures && *playerFeatures) {
            if (!strncmp(playerFeatures, "disableAll", strlen("disableAll"))) {
                playerFeatures += strlen("disableAll");
                QBAnyPlayerLogicToggleAll(anyPlayerLogic, false);
            } else if (!strncmp(playerFeatures, "loop", strlen("loop"))) {
                playerFeatures += strlen("loop");
                QBAnyPlayerLogicToggleLoop(anyPlayerLogic, true);
            } else
                break;

            if (*playerFeatures)
                playerFeatures++;   //skip ','
        }
    }
    SVRELEASE(anyPlayerLogic);

    switch (type) {
        case QBMediaFileType_audio:
            QBPVRPlayerContextSetMusic(pvrPlayer, c);
            break;
        case QBMediaFileType_video:
            QBPVRPlayerContextSetContent(pvrPlayer, c);
            break;
        case QBMediaFileType_audioPlaylist:
            QBPVRPlayerContextSetMusicPlaylist(pvrPlayer, URI);
            break;
        case QBMediaFileType_unknown:
            /** TODO: Add handling of QBMediaFileType_unknown case */
            QBPVRPlayerContextSetContent(pvrPlayer, c);
            break;
        default:
            break;
    }
    SVTESTRELEASE(c);

    //If context type is already PVRPlayer, remove it to avoid stacking multiple PVRPlayer contexts (breaks "back" etc. functionality)
    SvObject currentContext = (SvObject) QBApplicationControllerGetCurrentContext(self->appGlobals->controller);
    if (SvObjectIsInstanceOf(currentContext, QBPVRPlayerContext_getType())) {
        QBApplicationControllerPopContext(self->appGlobals->controller);
    }

    if (self->eventReporter) {
        if (playTrailer) {
            SvInvokeInterface(QBMWClientEventReporter, self->eventReporter, sendEvent, self->appGlobals->scheduler, QBMWClientEventReporter_playTrailer, NULL);
        } else {
            SvInvokeInterface(QBMWClientEventReporter, self->eventReporter, sendEvent, self->appGlobals->scheduler, QBMWClientEventReporter_play, NULL);
            SvInvokeInterface(QBMWClientEventReporter, self->eventReporter, sendEvent, self->appGlobals->scheduler, QBMWClientEventReporter_addToWatched, NULL);
        }
    }

    QBApplicationControllerPushContext(self->appGlobals->controller, pvrPlayer);
    SVRELEASE(pvrPlayer);
}

SvLocal bool
QBContentSideMenuCheckParentalControl(QBContentSideMenu self, SvDBRawObject movie)
{
    SvString parentalControl = QBMovieUtilsGetAttr(movie, "ratings", NULL);
    bool allowed = false;

    if (!parentalControl || QBParentalControlRatingAllowed(self->appGlobals->pc, parentalControl))
        allowed = true;

    return allowed;
}

SvLocal void
QBContentSideMenuCheckParentalControlPINCallbackPlayback(void *ptr, SvWidget dlg,
                                                         SvString ret, unsigned key)
{
    QBContentSideMenu self = ptr;

    self->dialog = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        SvDBRawObject movie = (SvDBRawObject) QBContentInfoGetMovie(self->contentInfo);
        QBContentSideMenuPlayContent(self, movie, true);
    }
}

SvLocal void
QBContentSideMenuCheckParentalControlPIN(QBContentSideMenu self)
{
    svSettingsPushComponent("QBVoDMovieDetails.settings");
    AppGlobals appGlobals = self->appGlobals;
    SvObject authenticator = NULL;
    authenticator = QBAuthenticateViaAccessManager(appGlobals->scheduler,
                                                   appGlobals->accessMgr,
                                                   self->accessDomain);

    SvWidget master = NULL;
    SvWidget dialog = QBAuthDialogCreate(appGlobals, authenticator,
                                         gettext("Authentication required"),
                                         0, true, NULL, &master);

    self->dialog = dialog;
    QBDialogRun(dialog, self, QBContentSideMenuCheckParentalControlPINCallbackPlayback);
    svSettingsPopComponent();
}

SvLocal void
QBContentSideMenuPlay_(QBContentSideMenu self, SvDBRawObject movie)
{
    AppGlobals appGlobals = self->appGlobals;
    int authStat = QBAccessManagerCheckAuthentication(appGlobals->accessMgr, self->accessDomain, NULL);

    if (QBContentSideMenuCheckParentalControl(self, movie) || authStat == QBAuthStatus_OK) {
        QBContentSideMenuPlayContent(self, movie, authStat == QBAuthStatus_OK);
    } else {
        QBContentInfoSetMovie(self->contentInfo, (SvGenericObject) movie);
        QBContentSideMenuCheckParentalControlPIN(self);
    }
}

SvLocal void
QBContentSideMenuPlay(void *self_, SvString id, QBBasicPane pane,
                      QBBasicPaneItem item)
{
    QBContentSideMenu self = self_;
    SVTESTRELEASE(self->selectedOptionID);
    self->selectedOptionID = SVRETAIN(id);
    SvDBRawObject movie = (SvDBRawObject) QBContentInfoGetMovie(self->contentInfo);
    QBContentSideMenuPlay_(self, movie);
    QBContextMenuHide((QBContextMenu) self, false);
}

SvLocal bool
QBContentSideMenuHasLastPosition(QBContentSideMenu self, SvDBRawObject movie)
{
    if (!self->appGlobals->bookmarkManager) {
        return false;
    }

    SvString productId = NULL;
    SvValue productIdV = SvDBObjectGetID((SvDBObject) movie);
    if (productIdV && SvValueIsString(productIdV)) {
        productId = SvValueGetString(productIdV);
        QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, productId);
        if (bookmark) {
            return true;
        }
    }

    return false;
}

SvLocal bool
supports_bookmarks(SvDBRawObject product)
{
    if (product && SvObjectIsInstanceOf((SvGenericObject) product, SvDBRawObject_getType())) {
        SvString type = QBMovieUtilsGetAttr((SvDBRawObject) product, "type", NULL);
        return SvObjectEquals((SvGenericObject) type, (SvGenericObject) SVSTRING("vod")) || SvObjectEquals((SvGenericObject) type, (SvGenericObject) SVSTRING("content"));
    }
    return false;
}

SvLocal void
QBContentSideMenuSetupPane(QBContentSideMenu self, SvDBRawObject product, QBBasicPane pane)
{
    if (!self || !product || !pane) {
        SvLogWarning("%s(): NULL parameter passed! (self=%p, product=%p, pane=%p)",
                     __func__, self, product, pane);
        return;
    }

    if (supports_bookmarks(product)) {
        SvString playOption = NULL;
        if (QBContentSideMenuHasLastPosition(self, product)) {
            SvString option = SvStringCreate(gettext("Continue"), NULL);
            QBBasicPaneAddOption(pane, SVSTRING("Continue"), option,
                                 QBContentSideMenuPlay, self);
            SVRELEASE(option);
            playOption = SvStringCreate(gettext("Play from the beginning"), NULL);
            QBBasicPaneAddOption(pane, SVSTRING("playFromTheBeginning"), playOption,
                                 QBContentSideMenuPlay, self);
        } else {
            playOption = SvStringCreate(gettext("Play"), NULL);
            QBBasicPaneAddOption(pane, SVSTRING("play"), playOption,
                                 QBContentSideMenuPlay, self);
        }
        SVRELEASE(playOption);
    } else {
        SvString option = SvStringCreate(gettext("Play"), NULL);
        QBBasicPaneAddOption(pane, SVSTRING("play"), option,
                             QBContentSideMenuPlay, self);
        SVRELEASE(option);
    }
}

SvLocal void
QBContentSideMenuInnov8onCallback(void  *self_, QBInnov8onLoadablePane pane, SvGenericObject product)
{
    QBContentSideMenu self = self_;
    if (unlikely(!self || !pane)) {
        SvLogError("%s NULL argument passed", __func__);
        return;
    }

    if (self->owner && self->callbacks.paneInit)
        self->callbacks.paneInit(self->owner, self, pane);
    else
        QBContentSideMenuInitDefault(self, (SvDBRawObject) product);

    bool entitled = false;
    if (product) {
        entitled = QBInnov8onProductIsEntitled((SvDBRawObject) product);
        if (self->entitled != entitled) {
            SvGenericObject provider = QBContentInfoGetProvider(self->contentInfo);
            QBContentSideMenuRefresh(self, (Innov8onProvider) provider, QBContentInfoGetCategory(self->contentInfo),
                                     (SvDBRawObject) QBContentInfoGetMovie(self->contentInfo));
        }
    }

    self->entitled = entitled;

    QBInnov8onLoadablePaneOnLoadFinished(pane);
}

SvLocal void
QBContentSideMenuPushLoadablePane(QBContentSideMenu self)
{
    if (unlikely(!self)) {
        SvLogError("%s NULL argument passed", __func__);
        return;
    }
    svSettingsPushComponent("LoadableBasicPane.settings");
    QBInnov8onLoadablePane options = (QBInnov8onLoadablePane) SvTypeAllocateInstance(QBInnov8onLoadablePane_getType(), NULL);
    QBInnov8onLoadablePaneInit(options, (QBContextMenu) self, self->appGlobals);
    QBInnov8onLoadablePaneSetRefreshObject(options, self->refreshObject);

    self->pane = options;

    SvDBRawObject product = (SvDBRawObject) QBContentInfoGetMovie(self->contentInfo);
    self->entitled = QBInnov8onProductIsEntitled(product);
    QBContentCategory category = (QBContentCategory) QBContentInfoGetCategory(self->contentInfo);
    SVTESTRELEASE(self->provider);
    self->provider = category ? QBContentCategoryTakeProvider(category) : NULL;
    struct QBInnov8onLoadablePaneRefreshParams_ params = {
        .product  = (SvObject) product,
        .provider = self->provider,
        .category = (SvObject) category
    };
    QBInnov8onLoadablePaneSetRefreshParams(options, &params);
    QBInnov8onLoadablePaneSetCallback(options, QBContentSideMenuInnov8onCallback, self);

    if (self->provider)
        QBContentProviderStart((QBContentProvider) self->provider, self->appGlobals->res->scheduler);

    QBContextMenuPushPane((QBContextMenu) self, (SvGenericObject) options);
    QBContextMenuBaseShow((QBContextMenu) self);
    SVRELEASE(options);
    svSettingsPopComponent();
}

SvLocal SvWidget
QBContentSideMenuCreatePopup(AppGlobals appGlobals, const char *title,
                             const char *message)
{
    svSettingsPushComponent("Carousel_VOD.settings");
    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0,
    };
    SvWidget dialog = QBSimpleDialogCreate(&params, title, message);
    svSettingsPopComponent();
    return dialog;
}

SvLocal void
QBContentSideMenuPopupCallback(void *self_, SvWidget dialog, SvString buttonTag,
                               unsigned keyCode)
{
    QBContentSideMenu self = (QBContentSideMenu) self_;
    self->dialog = NULL;
}

SvLocal void
QBContentSideMenuRefresh(QBContentSideMenu self, Innov8onProvider rootProvider,
                         QBContentCategory category, SvDBRawObject product)
{
    if (!self || !rootProvider || !category) {
        SvLogError("%s() failed: NULL argument passed. self=%p rootProvider=%p category=%p",
                   __func__, self, rootProvider, category);
        return;
    } else if (!SvObjectIsInstanceOf((SvGenericObject) rootProvider, Innov8onProvider_getType())) {
        SvLogError("%s() failed. rootProvider=%p is of type %s,"
                   " but is expected to be of type Innov8onProvider", __func__,
                   rootProvider, SvObjectGetTypeName((SvGenericObject) rootProvider));
        return;
    }

    int i = QBContentCategoryUtilsGetProductIdxInCategory(category, (SvGenericObject) product);
    if (i < 0) {
        SvLogError("%s() failed: There's no such product=%p at given path.", __func__, product);
        return;
    }
    QBContentProvider provider = (QBContentProvider) QBContentCategoryTakeProvider(category);
    QBContentProviderRefresh(provider ? provider : (QBContentProvider) rootProvider, category, (size_t) i, false, true);
    SVTESTRELEASE(provider);
}

SvLocal void
QBContentSideMenuPurchaseStatusChanged(SvGenericObject self_,
                                       SvGenericObject transaction_,
                                       const QBContentMgrTransactionStatus *const status)
{
    QBContentSideMenu self = (QBContentSideMenu) self_;
    if (status->status.state == QBRemoteDataRequestState_finished) {
        SvLogNotice("%s Purchase succeeded. Play content.", __func__);
        SvDBRawObject movie = (SvDBRawObject) QBContentInfoGetMovie(self->contentInfo);

        if (movie && SvObjectIsInstanceOf((SvObject) movie, SvDBRawObject_getType())) {
            const SvString type = QBMovieUtilsGetAttr((SvDBRawObject) movie, "type", NULL);
            if (type && SvStringEqualToCString(type, "vod")) {
                QBVoDUtilsLogMoviePurchase(movie, self->selectedOffer);
            }
        }

        if (self->eventReporter) {
            SvInvokeInterface(QBMWClientEventReporter, self->eventReporter, sendEvent, self->appGlobals->scheduler, QBMWClientEventReporter_purchase, NULL);
        }
        QBContentSideMenuPlay_(self, QBProductPurchaseGetProduct(self->purchase));
        SVTESTRELEASE(self->purchase);
        self->purchase = NULL;
        SvGenericObject provider = QBContentInfoGetProvider(self->contentInfo);
        QBContentSideMenuRefresh(self, (Innov8onProvider) provider,
                                 QBContentInfoGetCategory(self->contentInfo), movie);
    } else if (status->status.state == QBRemoteDataRequestState_failed) {
        SvLogNotice("%s Purchase failed.", __func__);
        if (self->dialog) {
            QBDialogBreak(self->dialog);
        }

        SvString errorMsg = NULL;

        if (status->status.errorCode == QBRemoteDataRequestStateErrorCode_notEnoughCredits) {
            errorMsg = SvStringCreateWithFormat("%s", gettext("Insufficient credit to purchase event."));
        } else {
            const char *msg1 = gettext("An error has occurred."
                                       " Please contact your service provider.\n");
            const char *msg2 = status->status.message
                               ? SvStringCString(status->status.message) : gettext("Communication error");
            errorMsg = SvStringCreateWithFormat("%s[%s]", msg1, msg2);
        }
        self->dialog = QBContentSideMenuCreatePopup(self->appGlobals,
                                                    gettext("A problem occurred"),
                                                    SvStringCString(errorMsg));
        QBDialogRun(self->dialog, self, QBContentSideMenuPopupCallback);
        char* escapedMsg = QBStringCreateJSONEscapedString(SvStringCString(errorMsg));
        SvString data = QBProductUtilsCreateEventData(QBProductPurchaseGetProduct(self->purchase), NULL);
        QBSecureLogEvent("ContentSideMenu", "ErrorShown.ContextMenus.Purchase_Failed", "JSON:{\"description\":\"%s\"%s}",
                         escapedMsg, data ? SvStringCString(data) : "");
        SVTESTRELEASE(data);
        free(escapedMsg);
        SVTESTRELEASE(self->purchase);
        self->purchase = NULL;
        SVRELEASE(errorMsg);
    } else if (status->status.state == QBRemoteDataRequestState_cancelled) {
        SvLogNotice("%s Purchase cancelled", __func__);
        SVTESTRELEASE(self->purchase);
        self->purchase = NULL;
    }
}

SvLocal void
QBContentSideMenuClose(void *self_, QBContextMenu menu)
{
}

SvLocal
void QBContentSideMenuShow(QBContextMenu self_)
{
    QBContentSideMenu self = (QBContentSideMenu) self_;
    if (!self->contentInfo)
        return;
    QBContentSideMenuPushLoadablePane(self);
}

void
QBContentSideMenuSetImmediateAction(QBContentSideMenu self, QBContentSideMenuImmediateAction action)
{
    self->immediateAction = action;
}

SvLocal void
QBContentSideMenuDestroy(void *self_)
{
    QBContentSideMenu self = (QBContentSideMenu) self_;
    if (self->dialog) {
        QBDialogBreak(self->dialog);
    }
    SVTESTRELEASE(self->purchase);
    SVTESTRELEASE(self->selectedOptionID);
    SVTESTRELEASE(self->contentInfo);
    SVRELEASE(self->optionFile);
    if (self->provider) {
        QBContentProviderStop((QBContentProvider) self->provider);
        SVRELEASE(self->provider);
    }
    SVRELEASE(self->accessDomain);
    SVTESTRELEASE(self->eventReporter);
    SvFiberDestroy(self->postponeFiber);
}

SvLocal SvType
QBContentSideMenu_getType(void)
{
    static SvType type = NULL;

    static const struct QBContentMgrTransactionListener_ productPurchaseStatus = {
        .statusChanged = QBContentSideMenuPurchaseStatusChanged,
    };

    static const struct QBContextMenuVTable_ virtualMethods = {
        .super_          = {
            .super_      = {
                .destroy = QBContentSideMenuDestroy
            },
        },
        .show            = QBContentSideMenuShow
    };

    if (!type) {
        SvTypeCreateManaged("QBContentSideMenu",
                            sizeof(struct QBContentSideMenu_t),
                            QBContextMenu_getType(),
                            &type,
                            QBContextMenu_getType(), &virtualMethods,
                            QBContentMgrTransactionListener_getInterface(), &productPurchaseStatus,
                            NULL);
    }

    return type;
}

SvLocal void
QBContentSideMenuPostponedActionStep(void* self_)
{
    QBContentSideMenu self = self_;
    SvFiberDeactivate(self->postponeFiber);
    if (self->immediateAction == QBContentSideMenuImmediateAction_rent) {
        QBContentSideMenuPurchase(self, SVSTRING("rent"), NULL, NULL);
    } else if (self->immediateAction == QBContentSideMenuImmediateAction_play) {
        QBContentSideMenuPlay(self, SVSTRING("play"), NULL, NULL);
    }
    self->immediateAction = QBContentSideMenuImmediateAction_none;
}

QBContextMenu
QBContentSideMenuCreate(AppGlobals appGlobals, SvString optionFile, SvString accessDomain)
{
    QBContentSideMenu self = (QBContentSideMenu) SvTypeAllocateInstance(QBContentSideMenu_getType(), NULL);
    if (!self)
        return NULL;

    self->appGlobals = appGlobals;
    svSettingsPushComponent("ContextMenu.settings");
    QBContextMenuInit(&self->super_,
                      self->appGlobals->controller,
                      self->appGlobals->res,
                      SVSTRING("SideMenu"));
    svSettingsPopComponent();
    QBContextMenuSetCallbacks((QBContextMenu) self, QBContentSideMenuClose, self);

    self->owner = NULL;
    self->contentInfo = NULL;
    self->dialog = NULL;
    self->pane = NULL;
    self->purchase = NULL;
    self->selectedOptionID = NULL;
    self->optionFile = SVRETAIN(optionFile);
    self->immediateAction = QBContentSideMenuImmediateAction_none;
    self->postponeFiber = SvFiberCreate(appGlobals->scheduler, NULL, "QBContentSideMenuPostponedActionFiber", QBContentSideMenuPostponedActionStep, self);
    self->refreshObject = false;

    if (accessDomain)
        self->accessDomain = SVRETAIN(accessDomain);
    else
        self->accessDomain = SvStringCreate("PC_MENU", NULL);

    return (QBContextMenu) self;
}

void
QBContentSideMenuSetEventReporter(QBContentSideMenu self, SvObject eventReporter)
{
    if (!self || !eventReporter) {
        SvLogError("%s null argument passed (self: %p, eventReporter: %p)", __func__, self, eventReporter);
        return;
    }
    SVTESTRELEASE(self->eventReporter);
    self->eventReporter = SVRETAIN(eventReporter);
}

void
QBContentSideMenuSetRefreshObject(QBContentSideMenu self, bool refreshObject)
{
    self->refreshObject = refreshObject;
    if (self->pane) {
        QBInnov8onLoadablePaneSetRefreshObject(self->pane, self->refreshObject);
    }
}

void
QBContentSideMenuInitDefault(QBContentSideMenu self, SvDBRawObject product)
{
    QBBasicPane basicPane = QBInnov8onLoadablePaneGetBasicPane(self->pane);

    QBBasicPaneLoadOptionsFromFile(basicPane, self->optionFile);

    bool playAvailable = false;
    bool rentAvailable = false;

    if (product) {
        if (QBInnov8onProductIsEntitled((SvDBRawObject) product)) {
            SvDBRawObject movie = (SvDBRawObject) QBContentInfoGetMovie(self->contentInfo);
            QBContentSideMenuSetupPane(self, movie, basicPane);
            playAvailable = true;
        } else {
            bool isFree = false;
            SvDBRawObject movie = (SvDBRawObject) QBContentInfoGetMovie(self->contentInfo);
            SvArray offers = (SvArray) SvDBRawObjectGetAttrValue(movie, "offers");
            if (SvArrayCount(offers) == 1) {
                SvDBReference offerRef = (SvDBReference) SvArrayObjectAtIndex(offers, 0);
                SvDBRawObject offer = (SvDBRawObject) SvDBReferenceGetReferredObject(offerRef);
                SvValue priceVal = (SvValue) SvDBRawObjectGetAttrValue(offer, "price");
                if (QBInnov8onOfferIsAvailable(offer) &&
                    priceVal &&
                    SvObjectIsInstanceOf((SvGenericObject) priceVal, SvValue_getType()) &&
                    SvValueIsInteger(priceVal) &&
                    SvValueGetInteger(priceVal) == 0) {
                    isFree = true;
                }
            }
            SvString option = SvStringCreate(isFree ? gettext("Play") : gettext("Rent"), NULL);
            QBInnov8onLoadablePaneAddOption(self->pane, SVSTRING("rent"), option,
                                            QBContentSideMenuPurchase, self);
            SVRELEASE(option);
            rentAvailable = true;
        }
    } else {
        QBBasicPaneItem item = NULL;
        SvString option = SvStringCreate(gettext("Rent"), NULL);
        item = QBInnov8onLoadablePaneAddOption(self->pane, SVSTRING("rentDisabled"),
                                               option, QBContentSideMenuPurchaseDisabled, self);
        SVRELEASE(option);
        item->itemDisabled = true;
        QBInnov8onLoadablePaneOptionPropagateObjectChange(self->pane, item);
    }

    SvString trailer = QBMovieUtilsGetAttr(product, "trailer", NULL);
    if (trailer && SvStringGetLength(trailer) > 0) {
        SvString option = SvStringCreate(gettext("Play Trailer"), NULL);
        QBInnov8onLoadablePaneAddOption(self->pane, SVSTRING("Play Trailer"), option, QBContentSideMenuPlay, self);
        SVRELEASE(option);
    }

    SvString moreInfo = SvStringCreate(gettext("More info"), NULL);
    QBInnov8onLoadablePaneAddOption(self->pane, SVSTRING("more info"), moreInfo,
                                    QBContentSideMenuMoreInfo, self);
    SVRELEASE(moreInfo);
    SvString sExit = SvStringCreate(gettext("Exit"), NULL);
    QBInnov8onLoadablePaneAddOption(self->pane, SVSTRING("Exit"), sExit,
                                    QBContentSideMenuExit, self);
    SVRELEASE(sExit);
    SvString exitHome = SvStringCreate(gettext("Exit to Home Page"), NULL);
    QBInnov8onLoadablePaneAddOption(self->pane, SVSTRING("exit home"), exitHome,
                                    QBContentSideMenuCloseContent, self);
    SVRELEASE(exitHome);

    if (rentAvailable && (self->immediateAction == QBContentSideMenuImmediateAction_rent || self->immediateAction == QBContentSideMenuImmediateAction_rentOrPlay)) {
        self->immediateAction = QBContentSideMenuImmediateAction_rent;
        SvFiberActivate(self->postponeFiber);
    } else if (playAvailable && (self->immediateAction == QBContentSideMenuImmediateAction_play || self->immediateAction == QBContentSideMenuImmediateAction_rentOrPlay)) {
        self->immediateAction = QBContentSideMenuImmediateAction_play;
        SvFiberActivate(self->postponeFiber);
    }
}

void
QBContentSideMenuSetCallbacks(QBContentSideMenu self,
                              void* owner,
                              QBContentSideMenuPaneInitCallback paneInit)
{
    if (!self)
        return;

    if (owner)
        self->owner = owner;
    if (paneInit)
        self->callbacks.paneInit = paneInit;
}

void
QBContentSideMenuSetContentInfo(QBContentSideMenu self, SvGenericObject contentInfo_)
{
    if (!self)
        return;
    SVTESTRELEASE(self->contentInfo);
    if (contentInfo_) {
        self->contentInfo = (QBContentInfo) contentInfo_;
        SVRETAIN(contentInfo_);
    }
}
