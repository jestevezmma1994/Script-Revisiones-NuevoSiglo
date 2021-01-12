/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBVoDMenu.h"

#include <QBSecureLogManager.h>
#include <libintl.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SvFoundation/SvWeakList.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <SvPlayerKit/SvContent.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <QBContentManager/Innov8onProviderTransaction.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBContentManager/Innov8onProvider.h>
#include <QBContentManager/Innov8onUtils.h>
#include <Services/QBParentalControl/QBParentalControl.h>
#include <player_hints/http_input.h>
#include <QBPlayerControllers/QBAnyPlayerController.h>
#include <Logic/AnyPlayerLogic.h>
#include <Windows/pvrplayer.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBInnov8onLoadablePane.h>
#include <ContextMenus/QBInnov8onExtendedInfoPane.h>
#include <Widgets/QBSimpleDialog.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <SvDataBucket2/SvDBReference.h>
#include <Middlewares/productPurchase.h>
#include <Middlewares/Innov8on/innov8onTransaction.h>
#include <Utils/contentCategoryUtils.h>
#include <Utils/QBMovieUtils.h>
#include <Utils/authenticators.h>
#include <Utils/value.h>
#include <Utils/dbobject.h>
#include <Utils/QBContentInfo.h>
#include <Utils/productUtils.h>
#include <Utils/QBVODUtils.h>
#include <main.h>
#include <QBPlayerControllers/QBAnyPlayerController.h>
#include <QBAppKit/Utils/commonUtils.h>
#include <QBJSONUtils.h>

#define log_debug(fmt, ...) \
    do { if (0) { SvLogNotice(COLBEG() "%s() :: " fmt COLEND_COL(blue), __func__, ##__VA_ARGS__); } } while (0)


struct QBVoDMenu_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBContextMenu ctxMenu;

    SvWidget parentalPopup;
    SvWidget dialog;

    SvObject provider;
    SvObject path;
    SvDBRawObject product;
    QBProductPurchase purchase;
    bool entitled;

    SvString selectedOptionID;
    SvString serviceId;
    SvString serviceName;

    SvObject transaction;

    int selectedOffer;
    int settingsCtx;

    struct {
        int price;
        bool isPriceRequired;
    } selectedPrice;

    QBInnov8onLoadablePane pane;
    SvString accessDomain;      ///< Determine domain for QBAccessManager.
};


SvLocal void
QBVoDMenuHide(QBVoDMenu self)
{
    if (self->ctxMenu)
        QBContextMenuHide(self->ctxMenu, false);
}

SvLocal void
QBVoDMenuPushExtendedInfo(QBVoDMenu self, int level)
{
    QBInnov8onExtendedInfoPane moreInfo =
        QBInnov8onExtendedInfoPaneCreateFromSettings("carouselExtendedInfo.settings",
                                                     self->appGlobals, self->ctxMenu,
                                                     SVSTRING("ExtendedInfoPane"),
                                                     level, (SvObject) self->product);
    QBContextMenuPushPane(self->ctxMenu, (SvObject) moreInfo);
    SVRELEASE(moreInfo);
}

SvLocal void
QBVoDMenuMoreInfo(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBVoDMenu self = self_;

    int level = QBBasicPaneGetLevel(pane);
    QBVoDMenuPushExtendedInfo(self, level + 1);

    const SvString type = QBMovieUtilsGetAttr(self->product, "type", NULL);
    if (type && SvStringEqualToCString(type, "vod")) {
        QBVoDUtilsLogMovieDetails(self->product);
    }
}

// Purchase functions

SvLocal void
QBVoDMenuProductPopupCallback(void *self_,
                              SvWidget dialog,
                              SvString buttonTag,
                              unsigned keyCode)
{
    QBVoDMenu self = self_;
    self->dialog = NULL;
    SvLogNotice("%s PIN popup exits with button '%s'", __func__,
                buttonTag ? SvStringCString(buttonTag) : "--");
    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        SVTESTRELEASE(self->purchase);
        SvLogNotice("%s Start transaction", __func__);

        QBInnov8onPurchaseTransactionParams params = {
            .retCodeRequired = Innov8onProviderGetNeedSuccessCode(self->provider),
            .isPriceRequired = self->selectedPrice.isPriceRequired,
            .price = self->selectedPrice.price
        };
        QBInnov8onPurchaseTransaction purchaseTransaction = QBInnov8onPurchaseTransactionCreate(self->provider, self->selectedOffer, params, NULL);

        SvObject transaction = (SvObject) QBInnov8onPurchaseTransactionCreateRequest(self->appGlobals, purchaseTransaction);
        SVTESTRELEASE(self->transaction);
        self->transaction = transaction;

        SvObject finalTransaction = QBPurchaseLogicNewTransaction(self->appGlobals->purchaseLogic,
                                                                  (SvObject) self->product,
                                                                  transaction, self->provider);

        self->purchase = QBProductPurchaseNew(finalTransaction,
                                              (SvObject) self->product,
                                              self->appGlobals);
        QBProductPurchaseSetListener(self->purchase, (SvObject) self);
        SVRELEASE(finalTransaction);
        SVRELEASE(purchaseTransaction);
    }
}

SvLocal void
QBVoDMenuPurchaseInnov8onOffer(QBVoDMenu self, SvDBRawObject product, SvDBObject offer_)
{
    if (unlikely(!self || !offer_ || !product)) {
        return;
    }

    SvDBRawObject offer = (SvDBRawObject) offer_;
    SvValue offerIdV = SvDBObjectGetID(offer_);
    if (offerIdV) {
        AppGlobals appGlobals = self->appGlobals;
        int offerId = -1;
        if (SvValueIsInteger(offerIdV)) {
            offerId = SvValueGetInteger(offerIdV);
        } else if (SvValueIsString(offerIdV)) {
            offerId = atoi(SvStringCString(SvValueGetString(offerIdV)));
        }
        self->selectedOffer = offerId;

        SvString domain = NULL;
        const char *forcedDomain = getenv("INNOV8ON_VOD_AUTH");
        if (forcedDomain) {
            domain = SvStringCreate(forcedDomain, NULL);
        } else {
            domain = SvStringCreate("VOD_AUTH", NULL);
        }
        SvObject authenticator = NULL;
        authenticator = QBAuthenticateViaAccessManager(appGlobals->scheduler,
                                                       appGlobals->accessMgr,
                                                       domain);
        SVTESTRELEASE(domain);
        svSettingsPushComponent("ParentalControl.settings");
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

        SvString name = SvValueGetString((SvValue) SvDBRawObjectGetAttrValue(product, "name"));
        SvString titleStr = SvStringCreateWithFormat(gettext("Title: %s"), SvStringCString(name));
        QBAsyncLabelSetText(title, titleStr);
        SVRELEASE(titleStr);

        SvValue currencyV = (SvValue) SvDBRawObjectGetAttrValue(offer, "currency");
        SvString currency = NULL;
        if (likely(currencyV && SvObjectIsInstanceOf((SvObject) currencyV, SvObject_getType())
                   && SvValueIsString(currencyV))) {
            currency = SvValueGetString(currencyV);
        } else {
            currency = SVSTRING("");
        }

        SvValue priceRequiredVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) offer, "price_required");
        if (priceRequiredVal && SvObjectIsInstanceOf((SvObject) priceRequiredVal, SvValue_getType())
            && SvValueIsInteger(priceRequiredVal))
            self->selectedPrice.isPriceRequired = (bool) SvValueGetInteger(priceRequiredVal);
        else
            self->selectedPrice.isPriceRequired = false;

        SvValue priceVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) offer, "price");
        if (priceVal && SvObjectIsInstanceOf((SvObject) priceVal, SvValue_getType())
            && SvValueIsInteger(priceVal))
            self->selectedPrice.price = SvValueGetInteger(priceVal);
        else
            self->selectedPrice.price = 0;

        SvString priceStr = SvStringCreateWithFormat(gettext("Price: %i.%02i %s"), self->selectedPrice.price / 100,
                                                     self->selectedPrice.price % 100, SvStringCString(currency));

        QBAsyncLabelSetText(price, priceStr);
        SVRELEASE(priceStr);

        QBAuthDialogSetMessageWidget(self->dialog, w);
        svSettingsPopComponent();

        SvLogNotice("%s Purchasing offer id=%d, product name='%s' price='%d %s'. PIN is required (domain=%s)",
                    __func__, offerId, name ? SvStringCString(name) : "--", self->selectedPrice.price,
                    currency ? SvStringCString(currency) : "--", domain ? SvStringCString(domain) : "--");

        QBDialogRun(self->dialog, self, QBVoDMenuProductPopupCallback);
        QBVoDMenuHide(self);
    }
}

SvLocal void
QBVoDMenuOfferCallback(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBVoDMenu self = (QBVoDMenu) self_;
    SvArray offers = (SvArray) SvDBRawObjectGetAttrValue(self->product, "offers");
    if (!offers || !SvObjectIsInstanceOf((SvObject) offers, SvArray_getType()))
        goto fini;

    SvIterator it = SvArrayIterator(offers);
    SvDBReference offerRef = NULL;
    while ((offerRef = (SvDBReference) SvIteratorGetNext(&it))) {
        SvDBRawObject offer = (SvDBRawObject) SvDBReferenceGetReferredObject(offerRef);
        if (!offer)
            continue;

        SvValue nameV = (SvValue) SvDBRawObjectGetAttrValue(offer, "name");
        SvString name = SvValueTryGetString(nameV);
        if (!name)
            continue;

        if (SvObjectEquals((SvObject) name, (SvObject) id)) {
            QBVoDMenuPurchaseInnov8onOffer(self, self->product, (SvDBObject) offer);
            break;
        }
    }

fini:
    QBVoDMenuHide(self);
}

SvLocal void
QBVoDMenuPurchaseDisabled(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBVoDMenu self = self_;
    QBInnov8onLoadablePaneShowConnectionErrorPopup(self->pane, self->appGlobals);
}

SvLocal void
QBVoDMenuPurchase(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBVoDMenu self = self_;
    SVRETAIN(id);
    SVTESTRELEASE(self->selectedOptionID);
    self->selectedOptionID = id;
    if (self->product) {
        SvArray offers = (SvArray) SvDBRawObjectGetAttrValue(self->product, "offers");
        if (SvArrayCount(offers) == 1) {
            SvDBReference offerRef = (SvDBReference) SvArrayObjectAtIndex(offers, 0);
            QBVoDMenuPurchaseInnov8onOffer(self, self->product,
                                           SvDBReferenceGetReferredObject(offerRef));
            QBVoDMenuHide(self);
        } else {
            SvLogNotice("%s more than 1 offers to choose - pushing next sidemenu", __func__);
            // show offers to choose from
            svSettingsPushComponent("BasicPane.settings");
            AppGlobals appGlobals = self->appGlobals;
            int level = QBBasicPaneGetLevel(pane);
            QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
            QBBasicPaneInit(options, appGlobals->res, appGlobals->scheduler,
                            appGlobals->textRenderer, self->ctxMenu, level + 1,
                            SVSTRING("BasicPane"));
            SvIterator it = SvArrayIterator(offers);
            SvObject offerRef = NULL;
            while ((offerRef = SvIteratorGetNext(&it))) {
                SvDBRawObject offer = (SvDBRawObject) SvDBReferenceGetReferredObject((SvDBReference) offerRef);
                SvValue nameV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) offer, "name");
                SvString name = SvValueTryGetString(nameV);
                if (name)
                    QBBasicPaneAddOption(options, name, name, QBVoDMenuOfferCallback, self);
            }
            svSettingsPopComponent();
            QBContextMenuPushPane(self->ctxMenu, (SvObject) options);
            SVRELEASE(options);
        }
    }
}

// Playback functions

SvLocal void
QBVoDMenuPlayContent(QBVoDMenu self, SvDBRawObject movie, bool authenticated)
{
    if (!movie)
        return;

    bool lastPosition = SvStringEqualToCString(self->selectedOptionID, "Continue");
    bool playTrailer = SvStringEqualToCString(self->selectedOptionID, "Play Trailer");

    const char* attributeName = playTrailer ? "trailer" : "source";
    const char* URIstr = SvDBRawObjectGetStringAttrValue(movie, attributeName, NULL);
    if (!URIstr)
        return;

    const char* proxyURI = SvDBRawObjectGetStringAttrValue(movie, "proxyURI", NULL);

    SvString URI = SvStringCreate(URIstr, NULL);
    const char *coverURI = NULL;

    QBMediaFileType type = QBAppKitURIToMediaFileType(URI, NULL);
    SvContent c = NULL;
    if (URI)
        SvLogNotice("URI = %s", SvStringCString(URI));

    if (type != QBMediaFileType_audioPlaylist) {
        c = SvContentCreateFromCString(SvStringCString(URI), NULL);
        SvString title = SvValueGetString((SvValue) SvDBRawObjectGetAttrValue(movie, "name"));
        if (title) {
            SvContentMetaDataSetStringProperty(SvContentGetMetaData(c),
                                               SVSTRING(SV_PLAYER_META__TITLE),
                                               title);
        }
        coverURI = SvDBRawObjectGetStringAttrValue(movie, "thumbnail", NULL);
    }

    if (proxyURI) {
        SvString str = SvStringCreate(proxyURI, NULL);
        SvContentMetaDataSetStringProperty(SvContentGetHints(c),
                                           SVSTRING(PLAYER_HINT__HTTP_INPUT_PROXY), str);
        SVRELEASE(str);
    }

    SvString productId = NULL;
    SvValue productIdV = SvDBObjectGetID((SvDBObject) movie);
    if (productIdV && SvValueIsString(productIdV))
        productId = SvValueGetString(productIdV);

    QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->appGlobals);
    const char *contentType = SvDBRawObjectGetStringAttrValue(movie, "type", "");
    if (!strcmp(contentType, "internet_radio"))
        QBAnyPlayerLogicToggleChannelChange(anyPlayerLogic, false);
    if (productId && !playTrailer && self->appGlobals->bookmarkManager) {
        QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, productId);
        if (!bookmark)
            bookmark = QBBookmarkManagerSetLastPositionBookmark(self->appGlobals->bookmarkManager, productId, 0.0, QBBookmarkType_Generic);

        if (bookmark)
            QBAnyPlayerLogicSetLastPositionBookmark(anyPlayerLogic, bookmark);
    }

    QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals,
                                                         anyPlayerLogic);

    QBAnyPlayerLogicSetupPlaybackStateController(anyPlayerLogic);
    if (!playTrailer) {
        QBContentInfo contentInfo = (QBContentInfo) QBContentInfoCreate(self->appGlobals, self->serviceId, self->serviceName, (SvObject) movie, NULL, self->provider, NULL);
        SvObject controller = QBAnyPlayerLogicCreateController(anyPlayerLogic, contentInfo);
        SvInvokeInterface(QBAnyPlayerController, controller, setContent, (SvObject) c);
        QBAnyPlayerLogicSetController(anyPlayerLogic, controller);
        SVRELEASE(contentInfo);
        SVRELEASE(controller);
    }
    QBPVRPlayerContextSetTitle(pvrPlayer, self->serviceName);
    QBPVRPlayerContextUseBookmarks(pvrPlayer, lastPosition);
    QBPVRPlayerContextSetVodMetaData(pvrPlayer, movie);
    QBPVRPlayerContextSetAccessDomain(pvrPlayer, self->accessDomain);
    if (authenticated)
        QBPVRPlayerContextSetAuthenticated(pvrPlayer);

    if (coverURI) {
        SvString uri = SvStringCreate(coverURI, NULL);
        QBPVRPlayerContextSetCoverURI(pvrPlayer, uri);
        SVRELEASE(uri);
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
        case QBMediaFileType_IPstream:
            QBPVRPlayerContextSetContent(pvrPlayer, c);
            break;
        case QBMediaFileType_unknown:
            /** QBMediaFileType_unknown it could be e.g. internet radio */
            QBPVRPlayerContextSetContent(pvrPlayer, c);
            break;
        default:
            break;
    }
    SVRELEASE(URI);
    SVTESTRELEASE(c);
    QBApplicationControllerPushContext(self->appGlobals->controller, pvrPlayer);
    SVRELEASE(pvrPlayer);
}

SvLocal void
QBVoDMenuCheckParentalControlPINCallback(void *ptr, SvWidget dlg, SvString ret, unsigned key)
{
    QBVoDMenu self = ptr;
    if (!self->parentalPopup) {
        SVRELEASE(self);
        return;
    }

    self->parentalPopup = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        QBVoDMenuPlayContent(self, self->product, true);
    }
}

SvLocal void
QBVoDMenuCheckParentalControlPIN(QBVoDMenu self)
{
    svSettingsPushComponent("Carousel_VOD.settings");
    AppGlobals appGlobals = self->appGlobals;
    SvObject authenticator = NULL;
    authenticator = QBAuthenticateViaAccessManager(appGlobals->scheduler,
                                                   appGlobals->accessMgr,
                                                   self->accessDomain);
    SvWidget master = NULL;
    SvWidget dialog = QBAuthDialogCreate(appGlobals, authenticator,
                                         gettext("Authentication required"),
                                         0, true, NULL, &master);

    self->parentalPopup = dialog;
    QBDialogRun(dialog, self, QBVoDMenuCheckParentalControlPINCallback);
    svSettingsPopComponent();
}

SvLocal bool
QBVoDMenuCheckParentalControl(QBVoDMenu self, SvDBRawObject object)
{
    SvString ratings = QBMovieUtilsGetAttr(object, "ratings", NULL);
    if (!ratings || QBParentalControlRatingAllowed(self->appGlobals->pc, ratings))
        return false;

    return true;
}

SvLocal void
QBVoDMenuPlay_(QBVoDMenu self)
{
    QBAuthStatus authStat = QBAccessManagerCheckAuthentication(self->appGlobals->accessMgr, self->accessDomain, NULL);

    if (authStat == QBAuthStatus_OK || !QBVoDMenuCheckParentalControl(self, self->product))
        QBVoDMenuPlayContent(self, self->product, authStat == QBAuthStatus_OK);
    else
        QBVoDMenuCheckParentalControlPIN(self);

    QBVoDMenuHide(self);
}

SvLocal void
QBVoDMenuPlay(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBVoDMenu self = self_;
    SVRETAIN(id);
    SVTESTRELEASE(self->selectedOptionID);
    self->selectedOptionID = id;
    QBVoDMenuPlay_(self);
}

SvLocal bool
QBVoDMenuHasLastPosition(QBVoDMenu self, SvDBRawObject movie)
{
    SvValue productIdV = SvDBObjectGetID((SvDBObject) movie);
    if (!productIdV || !SvValueIsString(productIdV))
        return false;

    SvString productId = SvValueGetString(productIdV);
    return QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, productId) != NULL;
}

SvLocal bool
supports_bookmarks(SvDBRawObject product)
{
    if (product && SvObjectIsInstanceOf((SvObject) product, SvDBRawObject_getType())) {
        const char *type = SvDBRawObjectGetStringAttrValue((SvDBRawObject) product, "type", "");
        return !strcmp(type, "vod") || !strcmp(type, "catchup_event");
    }
    return false;
}

SvLocal void
QBVoDMenuSetupPane(QBVoDMenu self, SvDBRawObject product, QBBasicPane pane)
{
    if (!self || !product || !pane) {
        SvLogWarning("%s(): NULL parameter passed! (self=%p, product=%p, pane=%p)",
                     __func__, self, product, pane);
        return;
    }

    if (supports_bookmarks(product)) {
        SvString playOption = NULL;
        if (QBVoDMenuHasLastPosition(self, product)) {
            SvString option = SvStringCreate(gettext("Continue"), NULL);
            QBBasicPaneAddOption(pane, SVSTRING("Continue"), option, QBVoDMenuPlay, self);
            SVRELEASE(option);
            playOption = SvStringCreate(gettext("Play from the beginning"), NULL);
        } else {
            playOption = SvStringCreate(gettext("Play"), NULL);
        }
        QBBasicPaneAddOption(pane, SVSTRING("play"), playOption, QBVoDMenuPlay, self);
        SVRELEASE(playOption);
    } else {
        SvString option = SvStringCreate(gettext("Play"), NULL);
        QBBasicPaneAddOption(pane, SVSTRING("Play"), option, QBVoDMenuPlay, self);
        SVRELEASE(option);
    }
}

SvLocal void
QBVoDMenuRefresh(QBVoDMenu self,
                 Innov8onProvider rootProvider,
                 SvObject activeTreePath,
                 SvDBRawObject product)
{
    if (!self || !rootProvider || !activeTreePath) {
        SvLogError("%s() failed: NULL argument passed. self=%p rootProvider=%p"
                   " activeTreePath=%p", __func__, self, rootProvider, activeTreePath);
        return;
    } else if (!SvObjectIsInstanceOf((SvObject) rootProvider, Innov8onProvider_getType())) {
        SvLogError("%s() failed. rootProvider=%p is of type %s,"
                   " but is expected to be of type Innov8onProvider", __func__,
                   rootProvider, SvObjectGetTypeName((SvObject) rootProvider));
        return;
    } else if (!SvObjectIsImplementationOf(activeTreePath, QBTreePath_getInterface())) {
        SvLogError("%s() failed. activeTreePath=%p doesn't implemente interface QBTreePath",
                   __func__, activeTreePath);
        return;
    }

    QBContentCategory category = QBContentCategoryUtilsFindCategory((SvObject) self->appGlobals->menuTree, activeTreePath);
    if (!category) {
        SvLogError("%s() failed: ActiveTreePath=%p should point to QBContentCategory", __func__, activeTreePath);
        return;
    }
    int i = QBContentCategoryUtilsGetProductIdxInCategory(category, (SvObject) product);
    if (i < 0) {
        SvLogError("%s() failed: There's no such product=%p at given path.", __func__, product);
        return;
    }
    SvObject provider = QBContentCategoryTakeProvider(category);
    if (!provider)
        provider = (SvObject) SVRETAIN(rootProvider);
    QBContentProviderRefresh((QBContentProvider) provider, category, (size_t) i, false, true);
    SVTESTRELEASE(provider);
}

SvLocal void
QBVoDMenuInnov8onCallback(void *self_,
                          QBInnov8onLoadablePane pane,
                          SvObject product)
{
    QBVoDMenu self = self_;

    if (unlikely(!self || !pane)) {
        SvLogError("%s NULL argument passed", __func__);
        return;
    }

    bool entitled = false;
    if (product) {
        entitled = QBInnov8onProductIsEntitled((SvDBRawObject) product);
        if (entitled) {
            QBVoDMenuSetupPane(self, self->product,
                               QBInnov8onLoadablePaneGetBasicPane(pane));
        } else {
            SvString option = SvStringCreate(gettext("Rent"), NULL);
            QBInnov8onLoadablePaneAddOption(pane, SVSTRING("rent"), option,
                                            QBVoDMenuPurchase, self);
            SVRELEASE(option);
        }
    } else {
        QBBasicPaneItem item = NULL;
        SvString option = SvStringCreate(gettext("Rent"), NULL);
        item = QBInnov8onLoadablePaneAddOption(pane, SVSTRING("rentDisabled"),
                                               option, QBVoDMenuPurchaseDisabled, self);
        SVRELEASE(option);
        item->itemDisabled = true;
        QBInnov8onLoadablePaneOptionPropagateObjectChange(pane, item);
    }

    const char *trailer = SvDBRawObjectGetStringAttrValue(self->product, "trailer", NULL);
    if (trailer && *trailer) {
        SvString option = SvStringCreate(gettext("Play Trailer"), NULL);
        QBInnov8onLoadablePaneAddOption(pane, SVSTRING("Play Trailer"), option,
                                        QBVoDMenuPlay, self);
        SVRELEASE(option);
    }

    SvString moreInfo = SvStringCreate(gettext("More info"), NULL);
    QBInnov8onLoadablePaneAddOption(pane, SVSTRING("more info"), moreInfo,
                                    QBVoDMenuMoreInfo, self);
    SVRELEASE(moreInfo);

    self->entitled = entitled;

    QBInnov8onLoadablePaneOnLoadFinished(pane);
}

SvLocal SvWidget
QBVoDMenuCreatePopup(AppGlobals appGlobals, const char *title, const char *message)
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
QBVoDMenuPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBVoDMenu self = (QBVoDMenu) self_;
    self->dialog = NULL;
}

SvLocal void
QBVoDMenuSideMenuClose(void *self_, QBContextMenu menu)
{
    QBVoDMenu self = self_;
    if (self->ctxMenu) {
        QBContextMenuHide(self->ctxMenu, false);
    }
}

void
QBVoDMenuStart(QBVoDMenu self, SvDBRawObject product, SvObject path)
{
    SVRETAIN(product);
    SVTESTRELEASE(self->product);
    self->product = product;

    SVRETAIN(path);
    SVTESTRELEASE(self->path);
    self->path = path;

    if (self->ctxMenu) {
        QBContextMenuSetCallbacks(self->ctxMenu, NULL, NULL);
        QBContextMenuHide(self->ctxMenu, true);
        SVRELEASE(self->ctxMenu);
    }

    self->ctxMenu = QBContextMenuCreateFromSettings("ContextMenu.settings",
                                                    self->appGlobals->controller,
                                                    self->appGlobals->res,
                                                    SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->ctxMenu, QBVoDMenuSideMenuClose, self);

    const char *type = SvDBRawObjectGetStringAttrValue(product, "type", "");
    if (!strcmp(type, "vod")) {
        svSettingsPushComponent("LoadableBasicPane.settings");
        QBInnov8onLoadablePane pane = (QBInnov8onLoadablePane)
                                      SvTypeAllocateInstance(QBInnov8onLoadablePane_getType(), NULL);
        self->pane = pane;
        QBInnov8onLoadablePaneInit(pane, self->ctxMenu, self->appGlobals);
        svSettingsPopComponent();

        QBContentCategory category = QBContentCategoryUtilsFindCategory((SvObject) self->appGlobals->menuTree, path);
        SvObject provider = QBContentCategoryTakeProvider(category);
        self->entitled = QBInnov8onProductIsEntitled(product);
        struct QBInnov8onLoadablePaneRefreshParams_ params = {
            .product  = (SvObject) self->product,
            .provider = provider,
            .category = (SvObject) category
        };
        QBInnov8onLoadablePaneSetRefreshParams(pane, &params);
        QBInnov8onLoadablePaneSetCallback(pane, QBVoDMenuInnov8onCallback, self);

        QBContextMenuPushPane(self->ctxMenu, (SvObject) pane);
        QBContextMenuShow(self->ctxMenu);

        SVTESTRELEASE(provider);
        SVRELEASE(pane);
    } else {
        svSettingsPushComponent("BasicPane.settings");
        QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
        QBBasicPaneInit(options, self->appGlobals->res,
                        self->appGlobals->scheduler,
                        self->appGlobals->textRenderer,
                        self->ctxMenu, 1, SVSTRING("BasicPane"));
        svSettingsPopComponent();

        const char* URIstr = SvDBRawObjectGetStringAttrValue(product, "mediaURI", "");
        const char* source = SvDBRawObjectGetStringAttrValue(product, "source", "");
        if (*URIstr || *source) {
            QBVoDMenuSetupPane(self, product, options);
            const char *trailer = SvDBRawObjectGetStringAttrValue(product, "trailer", NULL);
            if (trailer && *trailer) {
                SvString option = SvStringCreate(gettext("Play Trailer"), NULL);
                QBBasicPaneAddOption(options, SVSTRING("Play Trailer"), option,
                                     QBVoDMenuPlay, self);
                SVRELEASE(option);
            }
        }

        if (QBBasicPaneGetOptionsCount(options) > 0) {
            SvString option = SvStringCreate(gettext("More info"), NULL);
            QBBasicPaneAddOption(options, SVSTRING("more info"), option,
                                 QBVoDMenuMoreInfo, self);
            SVRELEASE(option);
            QBContextMenuPushPane(self->ctxMenu, (SvObject) options);
        } else {
            QBVoDMenuPushExtendedInfo(self, 1);
        }
        QBContextMenuShow(self->ctxMenu);
        SVRELEASE(options);
    }
}

SvLocal void
QBVoDMenuRentSuccessfulPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBVoDMenu self = (QBVoDMenu) self_;
    self->dialog = NULL;

    if (self->purchase) {
        QBVoDMenuPlay_(self);
        SVTESTRELEASE(self->purchase);
        self->purchase = NULL;
        QBVoDMenuRefresh(self, (Innov8onProvider) self->provider,
                         self->path, self->product);
    }
}

SvLocal SvString
QBVoDMenuCreateSuccessMessage(QBVoDMenu self, Innov8onProviderTransaction transaction)
{
    if (!self) {
        SvLogError("%s: NULL self passed!", __func__);
        return NULL;
    }

    SvString answerMsg = NULL;
    SvValue answerCode = NULL;

    SvObject answer = Innov8onProviderTransactionGetAnswer(transaction, NULL);
    if (answer && SvObjectIsInstanceOf((SvObject) answer, SvHashTable_getType()))
        answerCode = (SvValue) SvHashTableFind((SvHashTable) answer, (SvGenericObject) SVSTRING("success_code"));
    if (answerCode && SvValueIsInteger(answerCode))
        answerMsg = Innov8onProviderCreateMessageFromErrorCode(SvValueGetInteger(answerCode));

    return answerMsg;
}

// QBContentMgrTransactionListener virtual methods

SvLocal void
QBVoDMenuPurchaseStatusChanged(SvObject self_,
                               SvObject transaction_,
                               const QBContentMgrTransactionStatus *const status)
{
    QBVoDMenu self = (QBVoDMenu) self_;
    if (status->status.state == QBRemoteDataRequestState_finished) {
        SvLogNotice("%s Purchase succeeded. Play content.", __func__);

        const SvString type = QBMovieUtilsGetAttr(self->product, "type", NULL);
        if (type && SvStringEqualToCString(type, "vod")) {
            QBVoDUtilsLogMoviePurchase(self->product, self->selectedOffer);
        }

        SvString successMsg = QBVoDMenuCreateSuccessMessage(self, (Innov8onProviderTransaction) self->transaction);
        if (successMsg) {
            self->dialog = QBVoDMenuCreatePopup(self->appGlobals, gettext("Operation Successful"), SvStringCString(successMsg));
            QBDialogRun(self->dialog, self, QBVoDMenuRentSuccessfulPopupCallback);
            SVRELEASE(successMsg);
        } else {
            QBVoDMenuPlay_(self);
            SVTESTRELEASE(self->purchase);
            self->purchase = NULL;
            QBVoDMenuRefresh(self, (Innov8onProvider) self->provider,
                             self->path, self->product);
        }
    } else if (status->status.state == QBRemoteDataRequestState_failed) {
        SvLogNotice("%s Purchase failed.", __func__);
        if (self->dialog)
            QBDialogBreak(self->dialog);

        const char *msg1 = gettext("An error has occurred."
                                   " Please contact your service provider.\n");
        const char *msg2 = SvStringCString(Innov8onProviderCreateMessageFromErrorCode(status->status.errorCode));
        SvString errorMsg = SvStringCreateWithFormat("%s[%s]", msg1, msg2);
        self->dialog = QBVoDMenuCreatePopup(self->appGlobals,
                                            gettext("A problem occurred"),
                                            SvStringCString(errorMsg));
        QBDialogRun(self->dialog, self, QBVoDMenuPopupCallback);
        char* escapedMsg = QBStringCreateJSONEscapedString(SvStringCString(errorMsg));

        SvString data = QBProductUtilsCreateEventData(self->product, NULL);
        QBSecureLogEvent("VoDMenu", "ErrorShown.ContextMenus.VodPurchaseFailed", "JSON:{\"description\":\"%s\"%s}",
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
QBVoDMenu__dtor__(void *self_)
{
    QBVoDMenu self = self_;

    if (self->ctxMenu) {
        QBContextMenuSetCallbacks(self->ctxMenu, NULL, NULL);
        QBContextMenuHide(self->ctxMenu, true);
        SVRELEASE(self->ctxMenu);
    }

    if (self->dialog)
        QBDialogBreak(self->dialog);

    if (self->parentalPopup)
        QBDialogBreak(self->parentalPopup);

    if (self->purchase) {
        QBProductPurchaseCancel(self->purchase);
        SVRELEASE(self->purchase);
    }

    SVRELEASE(self->provider);
    SVRELEASE(self->serviceId);
    SVRELEASE(self->serviceName);

    SVTESTRELEASE(self->transaction);
    SVTESTRELEASE(self->selectedOptionID);
    SVTESTRELEASE(self->product);
    SVTESTRELEASE(self->path);
    SVRELEASE(self->accessDomain);
}

SvType
QBVoDMenu_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDMenu__dtor__
    };
    static SvType type = NULL;

    static const struct QBContentMgrTransactionListener_ productPurchaseStatus = {
        .statusChanged = QBVoDMenuPurchaseStatusChanged,
    };

    if (!type) {
        SvTypeCreateManaged("QBVoDMenu",
                            sizeof(struct QBVoDMenu_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContentMgrTransactionListener_getInterface(), &productPurchaseStatus,
                            NULL);
    }

    return type;
}

void
QBVoDMenuInit(QBVoDMenu self,
              AppGlobals appGlobals,
              SvObject provider,
              SvString serviceId,
              SvString serviceName)
{
    self->settingsCtx = svSettingsSaveContext();
    self->appGlobals = appGlobals;
    self->provider = SVRETAIN(provider);
    self->serviceId = SVRETAIN(serviceId);
    self->serviceName = SVRETAIN(serviceName);
}

QBVoDMenu
QBVoDMenuCreate(AppGlobals appGlobals,
                SvObject provider,
                SvString serviceId,
                SvString serviceName,
                SvString accessDomain)
{
    QBVoDMenu self = (QBVoDMenu) SvTypeAllocateInstance(QBVoDMenu_getType(), NULL);
    QBVoDMenuInit(self, appGlobals, provider, serviceId, serviceName);

    if (accessDomain)
        self->accessDomain = SVRETAIN(accessDomain);
    else
        self->accessDomain = SvStringCreate("PC_MENU", NULL);

    return self;
}

void
QBVoDMenuStop(QBVoDMenu self)
{
    if (self->ctxMenu) {
        // because stop is 'emergency' situation we disable callback and hide side menu without effect
        QBContextMenuSetCallbacks(self->ctxMenu, NULL, NULL);
        QBContextMenuHide(self->ctxMenu, true);
    }
}
