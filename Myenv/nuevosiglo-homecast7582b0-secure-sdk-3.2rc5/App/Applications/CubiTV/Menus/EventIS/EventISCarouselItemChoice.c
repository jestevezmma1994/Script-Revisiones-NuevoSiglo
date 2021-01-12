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

#include "EventISCarouselItemChoice.h"

#include <QBSecureLogManager.h>
#include <libintl.h>
#include <QBConf.h>
#include <QBContentManager/QBContentCoupons.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <TraxisWebClient/TraxisWebVoDProvider.h>
#include <TraxisWebClient/TraxisWebVoDTransactions.h>
#include <TraxisWebClient/TraxisWebPosterServer.h>
#include <TraxisWebClient/TraxisWebCoreParser.h>
#include <QBOSK/QBOSKKey.h>
#include <QBOSK/QBOSK.h>
#include <QBSearch/QBSearchStatus.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/viewport.h>
#include <SWL/icon.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBListPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBOSKPane.h>
#include <Menus/menuchoice.h>
#include <Utils/authenticators.h>
#include <Utils/dbobject.h>
#include <Utils/value.h>
#include <Widgets/QBSimpleDialog.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/XMBCarousel.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <XMB2/XMBMenuBar.h>
#include <XMB2/XMBItemController.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <QBWidgets/QBGrid.h>
#include <Widgets/authDialog.h>
#include <Widgets/eventISMovieInfo.h>
#include <Widgets/confirmationDialog.h>
#include <Windows/mainmenu.h>
#include <Windows/channelscanning.h>
#include <Services/core/QBChannelScanningConfManager.h>
#include <Logic/PurchaseLogic.h>
#include <Middlewares/Traxis/traxis.h>
#include <iso_639_table.h>
#include <ctype.h>
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include "Windows/imagebrowser/QBDLNAImageBrowserController.h"
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <main.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBJSONUtils.h>

struct QBTraxisProductsItemController_t {
    struct SvObject_ super_;

    QBFrameConstructor* focus;
    QBXMBItemConstructor itemConstructor;
};

typedef struct QBTraxisProductsItemController_t *QBTraxisProductsItemController;

SvLocal SvWidget
QBTraxisProductsItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_,
                                         SvGenericObject path, SvApplication app,
                                         XMBMenuState initialState)
{
    QBTraxisProductsItemController self = (QBTraxisProductsItemController) self_;

    QBXMBItem item = QBXMBItemCreate();

    SvDBRawObject product = (SvDBRawObject) node_;
    SvWidget ret = NULL;
    item->focus = SVRETAIN(self->focus);

    item->caption = QBEventISMovieParseElement(product, SVSTRING("Name"), SVSTRING(""));
    ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);
    return ret;
}

SvLocal void
QBTraxisProductsItemControllerSetItemState(SvGenericObject self_, SvWidget item_,
                                           XMBMenuState state, bool isFocused)
{
    QBTraxisProductsItemController self = (QBTraxisProductsItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void
QBTraxisProductsItemControllerDestroy(void *self_)
{
    QBTraxisProductsItemController self = self_;
    SVRELEASE(self->focus);
    QBXMBItemConstructorDestroy(self->itemConstructor);
}

SvLocal SvType
QBTraxisProductsItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTraxisProductsItemControllerDestroy
    };
    static SvType type = NULL;
    static const struct XMBItemController_t controller_methods = {
        .createItem   = QBTraxisProductsItemControllerCreateItem,
        .setItemState = QBTraxisProductsItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTraxisProductsItemController",
                            sizeof(struct QBTraxisProductsItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            NULL);
    }

    return type;
}

typedef enum QBEventISCarouselOnButtonAction {
    QBEventISCarouselOnButtonActionUndefined = 0,
    QBEventISCarouselOnButtonActionPurchase,
    QBEventISCarouselOnButtonActionSideMenu,
    QBEventISCarouselOnButtonActionMax
} QBEventISCarouselOnButtonAction;

struct QBEventISCarouselMenuChoice_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    TraxisWebVoDProvider provider;

    QBEventISCarouselOnButtonAction onEnterButtonAction;
    QBEventISCarouselOnButtonAction onOptionButtonAction;
    QBEventISCarouselOnButtonAction selectedAction;

    struct QBEventisSideMenuSideMenu_t {
        QBContextMenu ctx;
    } sidemenu;

    SvWidget dialog;
    SvWidget parentalPopup;

    // needed for category tree traversal: when we have to wait for pin before entering "protectedCategoryPath"
    SvGenericObject protectedCategoryPath;

    // needed when we enter specific side-menu
    SvDBRawObject selectedProduct;
    SvDBRawObject selectedContent;
    SvDBRawObject selectedTitle;
    SvObject selectedTitlePath;
    SvObject lastSelectedPath;
    SvHashTable selectedCoupon;
    bool selectedFeature;
    SvString selectedOptionID; // name of side-menu option for "Play" variant ("Continue", "Play", "Play from Beginning")

    SvDBRawObject featureContent;     // refreshed from getOffer
    SvImmutableArray featureProducts; // refreshed from getOffer

    QBTraxisPlayContent playContent;
    QBProductPurchase purchase;

    bool switchPanes;

    SvString serviceName;
    struct {
        SvWidget viewport;
        SvWidget grid;
        SvWidget title;
        SvWidget description;
        SvWidget captionLanguages;
        SvWidget image;
    } extendedInfo;

    SvString itemNamesFilename;
    SvString rentalTermsMessage;

    TraxisWebVoDGetOfferTransaction getOfferTransaction;
    TraxisWebVoDWishListModificationTransaction getFavorites;
    SvWidget waitWidget;
    int basicPaneSettingsCtx;

    QBContentSearch contentSearch;
    SvGenericObject contentSearchPath;
    QBContentCategory searchRootCategory;
    SvGenericObject path;
};

SvLocal SvWidget
QBEventisSideMenuCreatePopup(AppGlobals appGlobals, const char *title, const char *message);

SvLocal bool
QBEventisContentNeedsAuthentication(QBEventISCarouselMenuChoice self, SvDBRawObject content);

SvLocal QBBasicPane
QBEventisSideMenuCreateOptionsPaneForCoupons(QBEventISCarouselMenuChoice self, SvImmutableArray coupons);

SvLocal void
QBEventisSideMenuOSKKeyTyped(void *self_, QBOSKPane pane, SvString input,
                             unsigned int layout, QBOSKKey key);

SvLocal bool
EventisCarouselShouldAddSearchToSideMenu(QBEventISCarouselMenuChoice self)
{
    const TraxisWebVoDProviderConfig* config = TraxisWebVoDProviderGetConfig(self->provider);

    return (self->lastSelectedPath != NULL) && config->enableSearchInSideMenu;
}

SvLocal bool
QBEventisSideMenuHasLastPosition(QBEventISCarouselMenuChoice self, SvDBRawObject movie)
{
    const char *URI  = SvDBRawObjectGetStringAttrValue(movie, "id", NULL);
    if (!URI)
        return false;

    SVAUTOSTRING(URIstr, URI);
    return QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, URIstr) != NULL;
}

SvLocal void
QBEventisSideMenuScanPopupCallback(void *self_, SvWidget w, SvString buttonTag, unsigned keyCode)
{
    QBEventISCarouselMenuChoice self = self_;
    if (!self->dialog)
        return;

    self->dialog = NULL;

    // push channel scanning context
    QBWindowContext channelscanning = QBChannelScanningContextCreate(self->appGlobals, NULL);
    QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
    QBChannelScanningConf conf = QBChannelScanningConfManagerGetConf(channelScanningConf,
                                                                     SVSTRING("automatic"));
    if (conf)
        QBChannelScanningContextLoadConf(channelscanning, conf);
    QBApplicationControllerPushContext(self->appGlobals->controller, channelscanning);
    SVRELEASE(channelscanning);
}

SvLocal bool
QBEventisSideMenuShowScanPopup(QBEventISCarouselMenuChoice self, char *title, char *msg)
{
    self->dialog = QBEventisSideMenuCreatePopup(self->appGlobals, title, msg);
    QBDialogRun(self->dialog, self, QBEventisSideMenuScanPopupCallback);
    return false;
}

SvLocal bool
QBEventisSideMenuCheckGroupId(QBEventISCarouselMenuChoice self)
{
    const char *groupId = QBConfigGet("EVENTIS_GROUP_ID");
    if (unlikely(!groupId)) {
        groupId = "1";
    }
    if (unlikely(!strcmp(groupId, "1"))) {
        return QBEventisSideMenuShowScanPopup(self, gettext("First time using VoD service?"),
                                              gettext("You need to perform manual channel "
                                              "scanning to start using VoD service."));
    } else if (unlikely(!strcmp(groupId, "0"))) {
        return QBEventisSideMenuShowScanPopup(self, gettext("VoD unavailable"),
                                              gettext("Unfortunately VoD service is not available."));
    }
    return true;
}

SvLocal SvWidget
QBEventisSideMenuCreateProductPurchasingDialog(QBEventISCarouselMenuChoice self, SvDBRawObject product)
{
    SvGenericObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler,
                                                                   self->appGlobals->accessMgr,
                                                                   SVSTRING("VOD_AUTH"));
    SvValue productType = (SvValue) SvDBRawObjectGetAttrValue(product, "Type");
    SvString productTypeStr = SvValueGetString(productType);

    svSettingsPushComponent("AuthDialogVODAUTH.settings");
    SvWidget dialog = QBAuthDialogCreate(self->appGlobals, authenticator,
                                         gettext("Authentication required"), 0,
                                         true, NULL, NULL);
    svSettingsPopComponent();

    svSettingsPushComponent("Carousel_VOD.settings");
    SvApplication app = self->appGlobals->res;
    SvWidget infoWidget = svSettingsWidgetCreate(app, "purchaseInfo");
    SvWidget title = QBAsyncLabelNew(app, "purchaseInfo.title", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(infoWidget, title, "purchaseInfo.title", 1);
    SvWidget price = QBAsyncLabelNew(app, "purchaseInfo.price", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(infoWidget, price, "purchaseInfo.price", 1);
    SvString name = QBEventISMovieParseElement(product, SVSTRING("Name"), SVSTRING(""));
    SvString titleStr = SvStringCreateWithFormat(gettext("Title: %s"), SvStringCString(name));
    QBAsyncLabelSetText(title, titleStr);
    SVRELEASE(titleStr);
    SVRELEASE(name);

    if (!self->selectedCoupon) {
        SvString currency = QBEventISMovieParseElement(product, SVSTRING("Currency"), SVSTRING(""));
        int offerPrice = QBEventISMovieParseElementAsInteger(product, SVSTRING("OfferPrice"), 0) + 50;
        SvString priceStr = SvStringCreateWithFormat(gettext("Price: %i.%02i%s"), offerPrice / 10000,
                                                     (offerPrice / 100) % 100, SvStringCString(currency));
        SVRELEASE(currency);
        QBAsyncLabelSetText(price, priceStr);
        SVRELEASE(priceStr);
    } else {
        SvString couponName = (SvString) SvHashTableFind(self->selectedCoupon,
                                                         (SvGenericObject) SVSTRING("Name"));
        SvString couponStr = SvStringCreateWithFormat(gettext("Buying with %s"),
                                                      couponName ? SvStringCString(couponName) : "");
        QBAsyncLabelSetText(price, couponStr);
        SVRELEASE(couponStr);
    }

    if (SvStringEqualToCString(productTypeStr, "TVOD")) {
        // TVOD specific
        SvWidget rentalPeriod = QBAsyncLabelNew(app, "purchaseInfo.rentalPeriod",
                                                self->appGlobals->textRenderer);
        svSettingsWidgetAttach(infoWidget, rentalPeriod, "purchaseInfo.rentalPeriod", 1);
        int rp = QBEventISMovieParseElementAsInteger(product, SVSTRING("RentalPeriod"), 0);
        int hours = rp / 60;
        SvString rentalPeriodStr = SvStringCreateWithFormat(gettext("Rental period: %i %s"),
                                                            hours, ngettext("hour", "hours", hours));
        QBAsyncLabelSetText(rentalPeriod, rentalPeriodStr);
        SVRELEASE(rentalPeriodStr);
    } else if (SvStringEqualToCString(productTypeStr, "SVOD")) {
        // subscription specific
        SvWidget desc = QBAsyncLabelNew(app, "purchaseInfo.rentalPeriod", self->appGlobals->textRenderer);
        svSettingsWidgetAttach(infoWidget, desc, "purchaseInfo.rentalPeriod", 1);
        SvString descStr = QBEventISMovieParseElement(product, SVSTRING("Description"), SVSTRING(""));
        QBAsyncLabelSetText(desc, descStr);
        SVRELEASE(descStr);
    } else {
        SvLogWarning("%s product type '%s' not handled", __func__, productTypeStr->text);
    }
    svSettingsPopComponent();

    QBAuthDialogSetMessageWidget(dialog, infoWidget);

    return dialog;
}

SvLocal void
QBEventisSideMenuProductPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBEventISCarouselMenuChoice self = self_;
    self->dialog = NULL;
    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        SVTESTRELEASE(self->purchase);
        SvGenericObject transaction = (SvGenericObject)
                                          TraxisWebVoDPurchaseTransactionCreate(self->provider,
                                                                                self->selectedContent,
                                                                                self->selectedProduct, NULL);
        if (!transaction) {
            SvLogError("%s(): failed to create Traxis VOD purchase transaction", __func__);
            return;
        }
        if (self->selectedCoupon) {
            SvString couponId = (SvString) SvHashTableFind(self->selectedCoupon, (SvGenericObject) SVSTRING("id"));
            TraxisWebVoDPurchaseTransactionSetCoupon((TraxisWebVoDPurchaseTransaction) transaction, couponId);
        }
        SvGenericObject finalTransaction = QBPurchaseLogicNewTransaction(self->appGlobals->purchaseLogic,
                                                                         (SvGenericObject) self->selectedProduct,
                                                                         (SvGenericObject) transaction,
                                                                         (SvGenericObject) self->provider);
        self->purchase = QBProductPurchaseNew(finalTransaction, (SvGenericObject) self->selectedProduct, self->appGlobals);
        QBProductPurchaseSetListener(self->purchase, (SvGenericObject) self);
        SVRELEASE(transaction);
        SVRELEASE(finalTransaction);
        SVTESTRELEASE(self->selectedCoupon);
        self->selectedCoupon = NULL;
    }
}

SvLocal void
QBEventisSideMenuRentalTermsPopupCallback(void* self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBEventISCarouselMenuChoice self = self_;
    self->dialog = NULL;

    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        self->dialog = QBEventisSideMenuCreateProductPurchasingDialog(self, self->selectedProduct);
        QBDialogRun(self->dialog, self, QBEventisSideMenuProductPopupCallback);
    }
}

SvLocal void
QBEventisSideMenuProductAuthenticationNeededPopupCallback(void* self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBEventISCarouselMenuChoice self = self_;
    self->dialog = NULL;
    SvLogNotice("%s PIN popup exits with button '%s'", __func__, buttonTag ? SvStringCString(buttonTag) : "--");

    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        self->dialog = QBEventisSideMenuCreateProductPurchasingDialog(self, self->selectedProduct);
        QBDialogRun(self->dialog, self, QBEventisSideMenuProductPopupCallback);
    }
}

SvLocal void
QBEventisSideMenuProductErrorPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBEventISCarouselMenuChoice self = self_;
    self->dialog = NULL;
}

SvLocal void
QBEventisSideMenuShowPopupPurchaseImpossible(QBEventISCarouselMenuChoice self, SvDBRawObject product)
{
    svSettingsPushComponent("BorderedDialog.settings");
    QBDialogParameters params = {
        .app        = self->appGlobals->res,
        .controller = self->appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0
    };
    self->dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    QBDialogAddPanel(self->dialog, SVSTRING("content"), NULL, 1);
    SvString name = QBEventISMovieParseElement(product, SVSTRING("Name"), SVSTRING(""));
    QBDialogSetTitle(self->dialog, SvStringCString(name));
    SVRELEASE(name);
    SvString msg = QBEventISMovieParseElement(product, SVSTRING("Description"), NULL);
    if (!msg)
        msg = SvStringCreate(gettext("To subscribe to this product (Traxis SVOD), please call Customer Support."),
                             NULL);
    QBDialogAddLabel(self->dialog, SVSTRING("content"), SvStringCString(msg), SVSTRING("message"), 0);
    SVRELEASE(msg);
    QBDialogAddButton(self->dialog, SVSTRING("button"), gettext("OK"), 1);
    svSettingsPopComponent();
    QBDialogRun(self->dialog, self, QBEventisSideMenuProductErrorPopupCallback);
}

SvLocal int
QBEventisSideMenuTraxisGetPaneLevel(QBEventISCarouselMenuChoice self)
{
    int level = QBContextMenuGetCurrentPaneLevel(self->sidemenu.ctx);
    if (!self->switchPanes)
        level++;
    return level;
}

SvLocal void
QBEventisSideMenuTraxisPushPane(QBEventISCarouselMenuChoice self, SvGenericObject pane)
{
    if (self->switchPanes) {
        QBContextMenuSwitchPane(self->sidemenu.ctx, pane);
    } else {
        QBContextMenuPushPane(self->sidemenu.ctx, pane);
    }
    self->switchPanes = false;
}

SvLocal void
QBEventisSideMenuTraxisHidePane(QBEventISCarouselMenuChoice self)
{
    QBContextMenuHide(self->sidemenu.ctx, false);
    self->switchPanes = false;
}

SvLocal void
QBEventisSideMenuTraxisProductSelected_(QBEventISCarouselMenuChoice self)
{
    if (self->featureContent != self->selectedContent) {
        SVTESTRETAIN(self->featureContent);
        SVTESTRELEASE(self->selectedContent);
        self->selectedContent = self->featureContent;
    }
    SvDBRawObject product = self->selectedProduct;
    bool purchasePossible = true;
    SvValue productType = (SvValue) SvDBRawObjectGetAttrValue(product, "Type");
    if (!productType || !SvValueIsString(productType)) {
        SvLogError("%s no product type!", __func__);
        purchasePossible = false;
        goto check_finished;
    }
    SvString productTypeStr = SvValueGetString(productType);
    if (unlikely(!productTypeStr)) {
        SvLogError("%s product type not set!", __func__);
        purchasePossible = false;
        goto check_finished;
    }
    if (SvStringEqualToCString(productTypeStr, "SVOD")) {
        purchasePossible = false;
    } else if (SvStringEqualToCString(productTypeStr, "TVOD")) {
        SvValue testV = (SvValue) SvDBRawObjectGetAttrValue(product, "OfferPrice");
        if (!testV) {
            SvLogError("%s no OfferPrice for product!", __func__);
            purchasePossible = false;
            goto check_finished;
        }
    }

check_finished:
    if (!purchasePossible) {
        QBEventisSideMenuShowPopupPurchaseImpossible(self, product);
        QBEventisSideMenuTraxisHidePane(self);
    } else {
        if (self->rentalTermsMessage) {
            QBConfirmationDialogParams_t params = {
                .title                 = gettext("Rental Terms"),
                .message               = gettext(SvStringCString(self->rentalTermsMessage)),
                .local                 = true,
                .globalName            = NULL,
                .focusOK               = true,
                .labelOK               = gettext("Accept"),
                .isCancelButtonVisible = true,
                .labelCancel           = gettext("Decline")
            };

            svSettingsPushComponent("LongMessageDialog.settings");
            self->dialog = QBConfirmationDialogCreate(self->appGlobals->res, &params);
            svSettingsPopComponent();

            QBDialogRun(self->dialog, self, QBEventisSideMenuRentalTermsPopupCallback);
        } else {
            SvGenericObject authenticator = NULL;
            svSettingsPushComponent("ParentalControl.settings");

            if (QBEventisContentNeedsAuthentication(self, self->selectedTitle)) {
                authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, SVSTRING("PC"));
                svSettingsPushComponent("AuthDialogPC.settings");
                self->dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"), 0, true, NULL, NULL);
                svSettingsPopComponent();
                QBDialogRun(self->dialog, self, QBEventisSideMenuProductAuthenticationNeededPopupCallback);
            } else {
                self->dialog = QBEventisSideMenuCreateProductPurchasingDialog(self, self->selectedProduct);
                QBDialogRun(self->dialog, self, QBEventisSideMenuProductPopupCallback);
            }

            svSettingsPopComponent();
        }

        QBEventisSideMenuTraxisHidePane(self);
    }
}

SvLocal void
QBEventisSideMenuTraxisProductSelected(void* self_, QBListPane pane, SvGenericObject product, int pos)
{
    QBEventISCarouselMenuChoice self = self_;
    SVTESTRELEASE(self->selectedProduct);
    self->selectedProduct = SVRETAIN(product);

    SvImmutableArray coupons = (SvImmutableArray) SvDBRawObjectGetAttrValue((SvDBRawObject) product, "Coupons");
    if (coupons && SvImmutableArrayCount(coupons) > 0) {
        QBBasicPane options = QBEventisSideMenuCreateOptionsPaneForCoupons(self, coupons);
        QBEventisSideMenuTraxisPushPane(self, (SvGenericObject) options);
        SVRELEASE(options);
        return;
    }

    QBEventisSideMenuTraxisProductSelected_(self);
}

SvLocal QBActiveArray
QBEventisSideMenuCreateProductArray(SvImmutableArray allProducts,
                                    TraxisWebVoDProvider traxisProvider)
{
    size_t cnt = SvImmutableArrayCount(allProducts);
    SvArray filteredProducts = SvArrayCreateWithCapacity(cnt, NULL);

    const TraxisWebVoDProviderConfig* config = TraxisWebVoDProviderGetConfig(traxisProvider);

    size_t i;
    for (i = 0; i < cnt; ++i) {
        SvDBRawObject product = (SvDBRawObject) SvArrayAt(allProducts, i);
        SvValue productType = (SvValue) SvDBRawObjectGetAttrValue(product, "Type");
        if (!productType || !SvValueIsString(productType)) {
            SvLogError("%s : no product type!", __func__);
            continue;
        }
        SvString productTypeStr = SvValueGetString(productType);
        if (SvStringEqualToCString(productTypeStr, "TVOD")) {
            if (!config->ignoreTransactionProducts)
                SvArrayAddObject(filteredProducts, (SvGenericObject) product);
        } else if (SvStringEqualToCString(productTypeStr, "SVOD")) {
            if (!config->ignoreSubscriptionProducts)
                SvArrayAddObject(filteredProducts, (SvGenericObject) product);
        } else {
            SvLogError("%s : unsupported product type [%s]", __func__, SvStringCString(productTypeStr));
        }
    }

    SvIterator iter = SvImmutableArrayIterator(filteredProducts);
    QBActiveArray ret = QBActiveArrayCreateWithValues(&iter, NULL);
    SVRELEASE(filteredProducts);
    return ret;
}

SvLocal QBListPane
QBEventisSideMenuCreateProductsPane(QBEventISCarouselMenuChoice self)
{
    QBTraxisProductsItemController itemController = (QBTraxisProductsItemController) SvTypeAllocateInstance(QBTraxisProductsItemController_getType(), NULL);
    itemController->itemConstructor = QBXMBItemConstructorCreate("MenuItem", self->appGlobals->textRenderer);
    itemController->focus = QBFrameConstructorFromSM("NormalFocus");

    static struct QBListPaneCallbacks_t listCallbacks = {
        .selected = QBEventisSideMenuTraxisProductSelected,
    };
    QBListPane listPane = (QBListPane) SvTypeAllocateInstance(QBListPane_getType(), NULL);
    QBListPaneInit(listPane, self->appGlobals->res, NULL, (SvGenericObject) itemController, &listCallbacks,
                   self, self->sidemenu.ctx, QBEventisSideMenuTraxisGetPaneLevel(self), SVSTRING("ListPane"));
    SVRELEASE(itemController);

    return listPane;
}

SvLocal void
QBEventisSideMenuPurchase(QBEventISCarouselMenuChoice self)
{
    SvImmutableArray products = self->featureProducts;
    if (products) {
        size_t count = SvImmutableArrayCount(products);
        if (count <= 1) {
            SvDBRawObject product = (SvDBRawObject) SvImmutableArrayGetLastObject(products);
            if (product)
                QBEventisSideMenuTraxisProductSelected(self, NULL, (SvObject) product, 0);
        } else {
            svSettingsPushComponent("TraxisProductsList.settings");
            QBListPane productsPane = QBEventisSideMenuCreateProductsPane(self);

            QBActiveArray source = QBEventisSideMenuCreateProductArray(products, self->provider);
            QBListPaneSetSource(productsPane, (SvGenericObject) source);
            SVRELEASE(source);

            QBEventisSideMenuTraxisPushPane(self, (SvGenericObject) productsPane);
            SVRELEASE(productsPane);
            svSettingsPopComponent();
        }
    }
}

SvLocal void
QBEventisSideMenuPurchaseCallback(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;
    SVTESTRELEASE(self->selectedOptionID);
    self->selectedOptionID = SVRETAIN(id);
    self->selectedFeature = true;

    QBEventisSideMenuPurchase(self);
}

SvLocal void
QBEventisSideMenuFavoritesAction(QBEventISCarouselMenuChoice self, TraxisWebVoDWishListAction action)
{
    SvString name = QBEventISMovieParseElement(self->selectedTitle, SVSTRING("id"), SVSTRING(""));

    self->getFavorites = TraxisWebVoDWishListModificationTransactionCreate(self->provider,
                                                                           name, action,
                                                                           NULL);
    SVRELEASE(name);
    SvInvokeInterface(QBContentMgrTransaction, self->getFavorites, setListener,
                      (SvGenericObject) self, NULL);
    SvInvokeInterface(QBContentMgrTransaction, self->getFavorites, start, SvSchedulerGet());
}

SvLocal void
QBEventisSideMenuFavoritesActionCallback(void *self_, SvString id,
                                         QBBasicPane pane, QBBasicPaneItem item)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;
    SVTESTRELEASE(self->selectedOptionID);
    self->selectedOptionID = SVRETAIN(id);

    QBEventisSideMenuFavoritesAction(self, SvStringEqualToCString(id, "add to favorites") ?
                                     TraxisWebVoDWishListActionAdd : TraxisWebVoDWishListActionRemove);
}

SvLocal bool
QBEventisCreateSearchPane(QBEventISCarouselMenuChoice self, QBContentSearch search, SvGenericObject nodePath)
{
    SvString keyword;
    QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);

    SVTESTRELEASE(self->contentSearch);
    SVTESTRELEASE(self->contentSearchPath);
    self->contentSearch = SVRETAIN(search);
    self->contentSearchPath = SvObjectCopy(nodePath, NULL);
    svSettingsPushComponent("OSKPane.settings");
    SvErrorInfo error = NULL;
    QBOSKPaneInit(oskPane, self->appGlobals->scheduler, self->sidemenu.ctx, 1, SVSTRING("OSKPane"), QBEventisSideMenuOSKKeyTyped, self, &error);
    svSettingsPopComponent();
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        SVRELEASE(oskPane);
        return false;
    }
    keyword = QBContentSearchGetKeyword(search);
    if (keyword)
        QBOSKPaneSetRawInput(oskPane, SvStringCString(keyword));

    QBContextMenuShow(self->sidemenu.ctx);
    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) oskPane);
    SVRELEASE(oskPane);

    return true;
}

SvLocal void
QBEventisSideMenuSearchActionCallback(void *self_, SvString id,
                                      QBBasicPane pane, QBBasicPaneItem item)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;

    QBContentSearch sideMenuContentSearch = NULL;
    SvObject sideMenuContentSearchPath = NULL;
    SvObject searchNode, searchNodePath;

    SvObject parentPath = SvObjectCopy(self->lastSelectedPath, NULL);
    do {
        SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
        QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, parentPath, 0);
        do {
            searchNodePath = QBTreeIteratorGetCurrentNodePath(&iter);
            searchNode = QBTreeIteratorGetNextNode(&iter);
            if (searchNode && SvObjectIsInstanceOf(searchNode, QBContentSearch_getType()))  {
                sideMenuContentSearch = (QBContentSearch) searchNode;
                sideMenuContentSearchPath = searchNodePath;
                break;
            }
        } while (searchNode);
    } while (!sideMenuContentSearch && SvInvokeInterface(QBTreePath, parentPath, getLength) > 1);
    SVRELEASE(parentPath);

    QBEventisSideMenuTraxisHidePane(self);

    if (sideMenuContentSearch) {
        parentPath = SvObjectCopy(self->lastSelectedPath, NULL);
        SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
        SVTESTRELEASE(self->searchRootCategory);
        self->searchRootCategory = (QBContentCategory) SvInvokeInterface(QBTreeModel,
                                                                         self->appGlobals->menuTree,
                                                                         getNode, parentPath);
        SVRETAIN(self->searchRootCategory);
        QBEventisCreateSearchPane(self, sideMenuContentSearch, sideMenuContentSearchPath);
        SVRELEASE(parentPath);
    }
}

SvLocal void
QBEventisSideMenuPlay_(QBEventISCarouselMenuChoice self);

SvLocal void
QBEventisSideMenuCheckParentalControlPINCallback(void *ptr, SvWidget dlg, SvString ret, unsigned key)
{
    QBEventISCarouselMenuChoice sideMenu = ptr;
    if (!sideMenu->parentalPopup) {
        SVRELEASE(sideMenu);
        return;
    }

    sideMenu->parentalPopup = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        QBEventisSideMenuPlay_(sideMenu);
    }
}

SvLocal void
QBEventisSideMenuCheckParentalControlPIN(QBEventISCarouselMenuChoice self)
{
    svSettingsPushComponent("AuthDialogPC.settings");
    SvGenericObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler,
                                                                   self->appGlobals->accessMgr,
                                                                   SVSTRING("PC"));

    SvWidget dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"),
                                         gettext("Please enter parental control PIN to play choosen content"),
                                         true, NULL, NULL);

    svSettingsPopComponent();
    self->parentalPopup = dialog;
    QBDialogRun(dialog, self, QBEventisSideMenuCheckParentalControlPINCallback);
}

SvLocal void
QBEventisSideMenuPlay_(QBEventISCarouselMenuChoice self)
{
    SvDBRawObject movie = self->selectedTitle;

    QBTraxisPlayContent playContent = QBTraxisPlayContentNew(self->provider,
                                                             movie, self->appGlobals, self->selectedFeature);

    bool lastPosition = SvStringEqualToCString(self->selectedOptionID, "continue");
    QBTraxisToggleLastPosition(playContent, lastPosition);

    SVTESTRELEASE(self->playContent);
    self->playContent = playContent;
}

SvLocal SvString
QBEventISCarouselMenu_getTitleRating(SvDBRawObject movie)
{
    SvString rating = QBEventISMovieParseElement(movie, SVSTRING("PCRating"), NULL);
    return rating;
}

SvLocal bool
QBEventisContentNeedsAuthentication(QBEventISCarouselMenuChoice self, SvDBRawObject content)
{
    if (!content) {
        SvLogError("%s : NULL content passed", __func__);
        return false;
    }

    SvString parentalControl = QBEventISCarouselMenu_getTitleRating(content);
    bool checkNeeded = (parentalControl && !QBParentalControlRatingAllowed(self->appGlobals->pc, parentalControl));
    SVTESTRELEASE(parentalControl);
    if (checkNeeded)
        return true;
    return false;
}

SvLocal bool
QBEventisSideMenuIsPCCheckNeeded(QBEventISCarouselMenuChoice self,
                                 SvDBRawObject movie)
{
    bool checkNeeded = false;

    SvValue isAdult = (SvValue) SvDBRawObjectGetAttrValue(movie, "isAdult");
    if (isAdult && SvObjectIsInstanceOf((SvObject) isAdult, SvValue_getType()) && SvValueIsBoolean(isAdult) && SvValueGetBoolean(isAdult)) {
        checkNeeded = true;
    } else {
        SvString parentalControl = QBEventISCarouselMenu_getTitleRating(movie);
        checkNeeded = (parentalControl && !QBParentalControlRatingAllowed(self->appGlobals->pc,
                                                                          parentalControl));
        SVTESTRELEASE(parentalControl);
    }
    return checkNeeded;
}

SvLocal void
QBEventisSideMenuPlay(QBEventISCarouselMenuChoice self, bool checkPC)
{
    SvDBRawObject movie = self->selectedTitle;

    if (!QBEventisSideMenuCheckGroupId(self)) {
        SvLogWarning("%s(): can't play Traxis contents - invalid EVENTIS_GROUP_ID", __func__);
        return;
    }

    if (checkPC && QBEventisSideMenuIsPCCheckNeeded(self, movie)) {
        QBEventisSideMenuTraxisHidePane(self);
        QBEventisSideMenuCheckParentalControlPIN(self);
        return;
    }

    QBEventisSideMenuPlay_(self);
    QBEventisSideMenuTraxisHidePane(self);
}

SvLocal void
QBEventisSideMenuPlayFeature(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBEventISCarouselMenuChoice self = self_;
    SVTESTRELEASE(self->selectedOptionID);
    self->selectedOptionID = SVRETAIN(id);
    self->selectedFeature = true;

    QBEventisSideMenuPlay(self, true);
}

SvLocal void
QBEventisSideMenuPlayPreview(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBEventISCarouselMenuChoice self = self_;
    SVTESTRELEASE(self->selectedOptionID);
    self->selectedOptionID = SVRETAIN(id);
    self->selectedFeature = false;

    QBEventisSideMenuPlay(self, true);
}

SvLocal SvString
QBEventisSideMenuParseCaptionLanguages(SvGenericObject captionLanguages_,
                                       size_t maxElements, bool multiline)
{
    SvString ret = NULL;
    if (!captionLanguages_ || !SvObjectIsInstanceOf(captionLanguages_, SvHashTable_getType()))
        goto fini;

    SvGenericObject captionLanguage_ = SvHashTableFind((SvHashTable) captionLanguages_,
                                                       (SvObject) SVSTRING("CaptionLanguage"));
    if (!captionLanguage_ || !SvObjectIsInstanceOf(captionLanguage_, SvArray_getType()))
        goto fini;

    SvArray captionLanguage = (SvArray) captionLanguage_;

    ssize_t cnt = SvArrayCount(captionLanguage);
    if (cnt <= 0) {
        ret = SvStringCreate(gettext("Subtitles: -"), NULL);
        goto fini;
    }

    if (cnt > (ssize_t) maxElements)
        cnt = maxElements;


    SvStringBuffer subtitlesBuf = SvStringBufferCreate(NULL);
    SvStringBufferAppendCString(subtitlesBuf, "Subtitles:", NULL);

    for (ssize_t idx = 0; idx < cnt; idx++) {
        SvString element = (SvString) SvValueGetString((SvValue) SvArrayObjectAtIndex(captionLanguage, idx));
        const char *lang = iso639GetLangName(iso639_1to3(SvStringCString(element)));

        if (lang) {
            SvString language = SvStringCreate(dgettext("iso-codes", lang), NULL);

            if (multiline) {
                SvStringBufferAppendCString(subtitlesBuf, "\n", NULL);
            } else {
                if (idx == 0)
                    SvStringBufferAppendCString(subtitlesBuf, " ", NULL);
                else
                    SvStringBufferAppendCString(subtitlesBuf, ", ", NULL);
            }
            SvStringBufferAppendCString(subtitlesBuf, SvStringCString(language), NULL);
            SVRELEASE(language);
        }
    }
    if (SvArrayCount(captionLanguage) > maxElements) {
        SvStringBufferAppendCString(subtitlesBuf, "...", NULL);
    }
    ret = SvStringBufferCreateContentsString(subtitlesBuf, NULL);
    SVRELEASE(subtitlesBuf);
fini:
    return ret;
}

SvLocal void
QBEventisSideMenuExtendedInfoOnShow(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBEventISCarouselMenuChoice self = ptr;
    svSettingsPushComponent("carouselEventisExtendedInfo.settings");
    svSettingsWidgetAttach(frame, self->extendedInfo.viewport, "viewport", 0);
    svSettingsPopComponent();
    SvDBRawObject movie = self->selectedTitle;
    if (movie) {
        SvString description = NULL;
        SvGenericObject captionLanguages = NULL;
        SvString parsedCaptionLanguages = NULL;
        SvString title = NULL;
        SvString imageURI = NULL;
        title = QBEventISMovieParseElement(movie, SVSTRING("Name"), SVSTRING(""));
        captionLanguages = SvDBRawObjectGetAttrValue(movie, "CaptionLanguages");
        description = QBEventISMovieParseElement(movie, SVSTRING("Description"), SVSTRING(""));
        parsedCaptionLanguages = QBEventisSideMenuParseCaptionLanguages(captionLanguages, 10, false);
        imageURI = QBEventISMovieGetThumbnailURI(movie);
        SVTESTRETAIN(imageURI);

        if (TraxisWebSessionManagerGetUsePosterServerInterface(self->appGlobals->traxisWebSessionManager) && imageURI) {
            SvString resizedImageURI = TraxisWebPosterServerPrepareURL(imageURI,
                                                                       self->extendedInfo.image->width,
                                                                       self->extendedInfo.image->height,
                                                                       TraxisWebPosterServerImageMode_Box);
            SVRELEASE(imageURI);
            imageURI = resizedImageURI;
        }

        QBAsyncLabelSetText(self->extendedInfo.description, description);
        QBAsyncLabelSetText(self->extendedInfo.captionLanguages, parsedCaptionLanguages);
        QBAsyncLabelSetText(self->extendedInfo.title, title);
        if (imageURI) {
            svWidgetSetHidden(self->extendedInfo.image, false);
            unsigned int idx = (svIconGetBitmapsCount(self->extendedInfo.image) <= 0) ? 0 : 1;
            svIconSetBitmapFromURI(self->extendedInfo.image, idx, SvStringCString(imageURI));
            svIconSwitch(self->extendedInfo.image, idx, 0, 0.0);
        } else {
            svWidgetSetHidden(self->extendedInfo.image, true);
        }
        SVTESTRELEASE(title);
        SVTESTRELEASE(description);
        SVTESTRELEASE(imageURI);
        SVTESTRELEASE(parsedCaptionLanguages);
    } else {
        QBAsyncLabelSetText(self->extendedInfo.description, NULL);
        QBAsyncLabelSetText(self->extendedInfo.captionLanguages, NULL);
        QBAsyncLabelSetText(self->extendedInfo.title, NULL);
    }
    svViewPortSetContentsPosition(self->extendedInfo.viewport, 0, 0, true);
}

SvLocal void
QBEventisSideMenuExtendedInfoSetActive(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBEventISCarouselMenuChoice self = ptr;
    svWidgetSetFocus(self->extendedInfo.viewport);
}

SvLocal void
QBEventisSideMenuExtendedInfoOnHide(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBEventISCarouselMenuChoice self = ptr;
    svWidgetDetach(self->extendedInfo.viewport);
}

SvLocal void
QBEventisSideMenuClose(void *self_, QBContextMenu menu)
{
    QBEventISCarouselMenuChoice self = self_;
    SVTESTRELEASE(self->sidemenu.ctx);
    self->sidemenu.ctx = NULL;

    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBEventisSideMenuClose, self);
}

SvLocal void
QBEventisSideMenuSubscriptionInfoCallback(void *self_, SvWidget dialog,
                                          SvString buttonTag, unsigned keyCode)
{
    QBEventISCarouselMenuChoice self = self_;
    self->dialog = NULL;
}

SvLocal void
QBEventisSideMenuShowSubscriptionInfo(QBEventISCarouselMenuChoice self)
{
    QBConfirmationDialogParams_t params = {
            .title = gettext("Subscription Info"),
            .message = gettext("In order to get access to VoD service, please contact your sales representative"),
            .local = true,
            .globalName = NULL,
            .focusOK = true,
            .labelOK = gettext("OK"),
            .isCancelButtonVisible = false
    };

    svSettingsPushComponent("Dialog.settings");
    self->dialog = QBConfirmationDialogCreate(self->appGlobals->res, &params);
    svSettingsPopComponent();
    QBDialogRun(self->dialog, self, QBEventisSideMenuSubscriptionInfoCallback);
    QBEventisSideMenuTraxisHidePane(self);
}

/*****************************************************************************
 * Anonymous Browsing Pane Options  >
 *****************************************************************************/

SvLocal void
QBEventisAnonymousBrowsingSideMenuLoginCallback(void *self_, SvWidget dialog,
                                                SvString buttonTag, unsigned keyCode)
{
    QBEventISCarouselMenuChoice self = self_;
    self->dialog = NULL;
}

SvLocal void
QBEventisAnonymousBrowsingSideMenuPushLogInDialog(QBEventISCarouselMenuChoice self)
{
    svSettingsPushComponent("AuthDialogVODLogin.settings");
    SvGenericObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, SVSTRING("VOD_Login"));
    self->dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Log In"), gettext("In order to purchase products, please log in."), true, NULL, NULL);

    svSettingsPopComponent();
    QBDialogRun(self->dialog, self, QBEventisAnonymousBrowsingSideMenuLoginCallback);
}

SvLocal void
QBEventisAnonymousBrowsingSideMenuLogIn(void *self_, SvString id, QBBasicPane pane,
                                        QBBasicPaneItem item)
{
    QBEventISCarouselMenuChoice self = self_;

    QBEventisAnonymousBrowsingSideMenuPushLogInDialog(self);

    QBEventisSideMenuTraxisHidePane(self);
}

SvLocal void
QBEventisAnonymousBrowsingSideMenuShowSubscriptionInfo(void *self_, SvString id,
                                                       QBBasicPane pane, QBBasicPaneItem item)
{
    QBEventISCarouselMenuChoice self = self_;

    QBEventisSideMenuShowSubscriptionInfo(self);
}

SvLocal QBBasicPane
QBEventisAnonymousBrowsingCreateOptionPane(QBEventISCarouselMenuChoice self)
{
    svSettingsRestoreContext(self->basicPaneSettingsCtx);

    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer,
                    self->sidemenu.ctx, QBEventisSideMenuTraxisGetPaneLevel(self), SVSTRING("BasicPane"));

    QBBasicPaneLoadOptionsFromFile(options, self->itemNamesFilename);

    if (TraxisWebSessionManagerGetState(self->appGlobals->traxisWebSessionManager) == TraxisWebSessionState_active)
        QBBasicPaneAddOption(options, SVSTRING("log in"), NULL, QBEventisAnonymousBrowsingSideMenuLogIn, self);
    QBBasicPaneAddOption(options, SVSTRING("subscription info"), NULL, QBEventisAnonymousBrowsingSideMenuShowSubscriptionInfo, self);

    svSettingsPopComponent();

    static struct QBContainerPaneCallbacks_t moreInfoCallbacks = {
        .onShow    = QBEventisSideMenuExtendedInfoOnShow,
        .onHide    = QBEventisSideMenuExtendedInfoOnHide,
        .setActive = QBEventisSideMenuExtendedInfoSetActive,
    };
    svSettingsPushComponent("carouselEventisExtendedInfo.settings");
    QBBasicPaneAddOptionWithContainer(options, SVSTRING("more info"), NULL, SVSTRING("ExtendedInfo"), &moreInfoCallbacks, self);
    svSettingsPopComponent();

    return options;
}

/*****************************************************************************
 *  </ Anonymous Browsing Pane Options
 *****************************************************************************/

SvLocal SvWidget
QBEventisSideMenuCreatePopup(AppGlobals appGlobals, const char *title, const char *message)
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
QBEventisSideMenuPopupCallback(void *self_, SvWidget dialog, SvString buttonTag,
                               unsigned keyCode)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;
    self->dialog = NULL;
}

SvLocal QBBasicPane
QBEventisSideMenuCreateOptionsPane(QBEventISCarouselMenuChoice self)
{
    SvDBRawObject title = self->selectedTitle;

    svSettingsRestoreContext(self->basicPaneSettingsCtx);

    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer,
                    self->sidemenu.ctx, QBEventisSideMenuTraxisGetPaneLevel(self), SVSTRING("BasicPane"));

    QBBasicPaneLoadOptionsFromFile(options, self->itemNamesFilename);

    if (QBEventISMovieParseElementAsBoolean(title, SVSTRING("HasFeatures"), false)) {
        if (QBEventISMovieParseElementAsBoolean(title, SVSTRING("IsFeatureEntitled"), false)) {
            if (QBEventisSideMenuHasLastPosition(self, title)) {
                QBBasicPaneAddOption(options, SVSTRING("continue"), NULL, QBEventisSideMenuPlayFeature, self);
                QBBasicPaneAddOption(options, SVSTRING("play from the beginning"), NULL, QBEventisSideMenuPlayFeature, self);
            } else {
                QBBasicPaneAddOption(options, SVSTRING("play"), NULL, QBEventisSideMenuPlayFeature, self);
            }
        } else {
            SvString productTypeStr = QBEventISLogicGetTitleTypeFromProducts(self->appGlobals->eventISLogic,
                                                                             title, self->featureProducts);
            if (productTypeStr) {
                if (SvStringEqualToCString(productTypeStr, "TVOD")) {
                    QBBasicPaneAddOption(options, SVSTRING("buy"), NULL,
                                         QBEventisSideMenuPurchaseCallback, self);
                } else if (SvStringEqualToCString(productTypeStr, "SVOD")) {
                    QBBasicPaneAddOption(options, SVSTRING("subscription info"), NULL,
                                         QBEventisAnonymousBrowsingSideMenuShowSubscriptionInfo, self);
                }
            } else {
                QBBasicPaneAddOption(options, SVSTRING("buy"), NULL, QBEventisSideMenuPurchaseCallback, self);
            }
        }
    }

    if (QBEventISMovieParseElementAsBoolean(title, SVSTRING("HasPreviews"), false))
        QBBasicPaneAddOption(options, SVSTRING("preview"), NULL, QBEventisSideMenuPlayPreview, self);

    const TraxisWebVoDProviderConfig* config = TraxisWebVoDProviderGetConfig((TraxisWebVoDProvider) self->provider);
    if (title && config->enableFavorites) {
        if (QBEventISMovieParseElementAsBoolean(title, SVSTRING("OnWishList"), false)) {
            QBBasicPaneAddOption(options, SVSTRING("remove from favorites"), NULL,
                                 QBEventisSideMenuFavoritesActionCallback, self);
        } else {
            QBBasicPaneAddOption(options, SVSTRING("add to favorites"), NULL,
                                 QBEventisSideMenuFavoritesActionCallback, self);
        }
    }

    if (EventisCarouselShouldAddSearchToSideMenu(self))
        QBBasicPaneAddOption(options, SVSTRING("search"), NULL, QBEventisSideMenuSearchActionCallback, self);

    svSettingsPopComponent();

    static struct QBContainerPaneCallbacks_t moreInfoCallbacks = {
        .onShow    = QBEventisSideMenuExtendedInfoOnShow,
        .onHide    = QBEventisSideMenuExtendedInfoOnHide,
        .setActive = QBEventisSideMenuExtendedInfoSetActive,
    };
    if (title) {
        svSettingsPushComponent("carouselEventisExtendedInfo.settings");
        QBBasicPaneAddOptionWithContainer(options, SVSTRING("more info"), NULL, SVSTRING("ExtendedInfo"), &moreInfoCallbacks, self);
        svSettingsPopComponent();
    }

    return options;
}

SvLocal void
QBEventisSideMenuPurchaseWithCoupon(void *self_, SvString id, QBBasicPane pane,
                                    QBBasicPaneItem item)
{
    QBEventISCarouselMenuChoice self = self_;

    SvHashTable coupon = NULL;
    if (item->metadata) {
        coupon = (SvHashTable) SvHashTableFind(item->metadata, (SvGenericObject) SVSTRING("coupon"));
    }
    SVTESTRELEASE(self->selectedCoupon);
    self->selectedCoupon = SVTESTRETAIN(coupon);

    QBEventisSideMenuTraxisProductSelected_(self);
}

SvLocal QBBasicPane
QBEventisSideMenuCreateOptionsPaneForCoupons(QBEventISCarouselMenuChoice self,
                                             SvImmutableArray coupons)
{
    SvDBRawObject product = self->selectedProduct;

    svSettingsRestoreContext(self->basicPaneSettingsCtx);

    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer,
                    self->sidemenu.ctx, QBEventisSideMenuTraxisGetPaneLevel(self), SVSTRING("BasicPane"));

    // Add 1st option - 'Charge My Account'
    SvString caption = SvStringCreate(gettext("Charge my account"), NULL);

    SvString currency = QBEventISMovieParseElement(product, SVSTRING("Currency"), SVSTRING(""));
    int offerPrice = QBEventISMovieParseElementAsInteger(product, SVSTRING("OfferPrice"), 0);
    SvString subcaption = SvStringCreateWithFormat(gettext("%i.%02i %s"), offerPrice / 10000,
                                                   (offerPrice / 100) % 100, SvStringCString(currency));

    QBBasicPaneAddOptionWithSubcaption(options, NULL, caption, subcaption, QBEventisSideMenuPurchaseWithCoupon, self);
    SVRELEASE(caption);
    SVRELEASE(subcaption);
    SVRELEASE(currency);

    // Add additional options - coupons
    SvHashTable coupon = NULL;
    size_t idx;
    size_t count = SvImmutableArrayCount(coupons);
    for (idx = 0; idx < count; ++idx) {
        coupon = (SvHashTable) SvImmutableArrayObjectAtIndex(coupons, idx);
        if (!coupon)
            continue;
        SvHashTable couponMeta = SvHashTableCreate(4, NULL);

        SvString couponName = NULL;
        SvValue couponName_ = (SvValue) SvHashTableFind(coupon, (SvObject) SVSTRING("Name"));
        if (couponName_ && SvObjectIsInstanceOf((SvObject) couponName_, SvValue_getType()) && SvValueIsString(couponName_)) {
            couponName = SvValueGetString(couponName_);
            SvHashTableInsert(couponMeta, (SvObject) SVSTRING("Name"), (SvObject) couponName);
        }

        SvString couponID = NULL;
        SvValue couponID_ = (SvValue) SvHashTableFind(coupon, (SvObject) SVSTRING("id"));
        if (couponID_ && SvObjectIsInstanceOf((SvObject) couponID_, SvValue_getType()) && SvValueIsString(couponID_)) {
            couponID = SvValueGetString(couponID_);
            SvHashTableInsert(couponMeta, (SvObject) SVSTRING("id"), (SvObject) couponID);
        }

        SvString couponCaption = SvStringCreateWithFormat("\"%s\"", couponName ? SvStringCString(couponName) : "");
        SvString couponSubcaption = SvStringCreate(gettext("Use Coupon"), NULL);

        QBBasicPaneItem item = QBBasicPaneAddOptionWithSubcaption(options, NULL, couponCaption, couponSubcaption, QBEventisSideMenuPurchaseWithCoupon, self);
        SVRELEASE(couponCaption);
        SVRELEASE(couponSubcaption);

        SVTESTRELEASE(item->metadata);
        item->metadata = SvHashTableCreate(23, NULL);

        // Add coupon
        SvHashTableInsert(item->metadata, (SvGenericObject) SVSTRING("coupon"), (SvGenericObject) couponMeta);
        SVRELEASE(couponMeta);
    }

    svSettingsPopComponent();
    return options;
}

SvLocal void
QBEventisSideMenuPushMoreInfoPane(QBEventISCarouselMenuChoice self)
{
    static struct QBContainerPaneCallbacks_t moreInfoCallbacks = {
        .onShow = QBEventisSideMenuExtendedInfoOnShow,
        .onHide = QBEventisSideMenuExtendedInfoOnHide,
        .setActive = QBEventisSideMenuExtendedInfoSetActive,
    };
    svSettingsPushComponent("carouselEventisExtendedInfo.settings");
    QBContainerPane container = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    QBContainerPaneInit(container, self->appGlobals->res, self->sidemenu.ctx,
                        1, SVSTRING("ExtendedInfo"), &moreInfoCallbacks, self);
    QBEventisSideMenuTraxisPushPane(self, (SvGenericObject) container);
    QBContextMenuShow(self->sidemenu.ctx);
    SVRELEASE(container);
    svSettingsPopComponent();
}

SvLocal void
QBEventisSideMenuDefaultActionChosen(QBEventISCarouselMenuChoice self, bool accessGranted)
{
    SvGenericObject possibleOptions = QBBasicPaneParseOptionsFile(self->appGlobals->res,
                                                                  self->itemNamesFilename);

    SVTESTRELEASE(self->selectedOptionID);
    self->selectedOptionID = NULL;

    if (accessGranted) {
        SvDBRawObject title = self->selectedTitle;
        if (QBEventISMovieParseElementAsBoolean(title, SVSTRING("HasFeatures"), false)) {
            if (QBEventISMovieParseElementAsBoolean(title, SVSTRING("IsFeatureEntitled"), false)) {
                if (QBEventisSideMenuHasLastPosition(self, title)) {
                    if (QBBasicPaneFindItemIdxById(possibleOptions, SVSTRING("continue")) >= 0) {
                        self->selectedFeature = true;
                        self->selectedOptionID = SVSTRING("continue");
                        QBEventisSideMenuPlay(self, true);
                    } else if (QBBasicPaneFindItemIdxById(possibleOptions, SVSTRING("play from the beginning")) >= 0) {
                        self->selectedFeature = true;
                        self->selectedOptionID = SVSTRING("play from the beginning");
                        QBEventisSideMenuPlay(self, true);
                    }
                } else if (QBBasicPaneFindItemIdxById(possibleOptions, SVSTRING("play")) >= 0) {
                    self->selectedFeature = true;
                    self->selectedOptionID = SVSTRING("play");
                    QBEventisSideMenuPlay(self, true);
                }
            } else {
                SvString productTypeStr = QBEventISLogicGetTitleTypeFromProducts(self->appGlobals->eventISLogic,
                                                                                 title, self->featureProducts);
                if (productTypeStr) {
                    if (SvStringEqualToCString(productTypeStr, "TVOD") &&
                        QBBasicPaneFindItemIdxById(possibleOptions, SVSTRING("buy")) >= 0) {
                        self->selectedFeature = true;
                        self->selectedOptionID = SVSTRING("buy");
                        QBEventisSideMenuPurchase(self);
                    } else if (SvStringEqualToCString(productTypeStr, "SVOD") &&
                               QBBasicPaneFindItemIdxById(possibleOptions,
                                                          SVSTRING("subscription info")) >= 0) {
                        self->selectedOptionID = SVSTRING("subscription info");
                        QBEventisSideMenuShowSubscriptionInfo(self);
                    }
                } else if (QBBasicPaneFindItemIdxById(possibleOptions, SVSTRING("buy")) >= 0) {
                        self->selectedFeature = true;
                        self->selectedOptionID = SVSTRING("buy");
                        QBEventisSideMenuPurchase(self);
                }
            }
        }

        if (self->selectedOptionID) {
            SVTESTRELEASE(possibleOptions);
            return;
        }

        if (QBEventISMovieParseElementAsBoolean(title, SVSTRING("HasPreviews"), false) &&
            QBBasicPaneFindItemIdxById(possibleOptions, SVSTRING("preview")) >= 0) {
            self->selectedFeature = false;
            self->selectedOptionID = SVSTRING("preview");
            QBEventisSideMenuPlay(self, true);
        } else if (QBBasicPaneFindItemIdxById(possibleOptions, SVSTRING("more info")) >= 0) {
            self->selectedOptionID = SVSTRING("more info");
            QBEventisSideMenuPushMoreInfoPane(self);
        } else {
            const TraxisWebVoDProviderConfig* config = TraxisWebVoDProviderGetConfig((TraxisWebVoDProvider) self->provider);
            if (config->enableFavorites) {
                if (QBEventISMovieParseElementAsBoolean(title, SVSTRING("OnWishList"), false) &&
                    QBBasicPaneFindItemIdxById(possibleOptions, SVSTRING("remove from favorites")) >= 0) {
                    self->selectedOptionID = SVSTRING("remove from favorites");
                    QBEventisSideMenuFavoritesAction(self, TraxisWebVoDWishListActionRemove);
                } else if (QBBasicPaneFindItemIdxById(possibleOptions, SVSTRING("add to favorites")) >= 0) {
                    self->selectedOptionID = SVSTRING("add to favorites");
                    QBEventisSideMenuFavoritesAction(self, TraxisWebVoDWishListActionAdd);
                }
            }
        }
    } else if (TraxisWebSessionManagerGetState(self->appGlobals->traxisWebSessionManager) == TraxisWebSessionState_active
               && QBBasicPaneFindItemIdxById(possibleOptions, SVSTRING("log in")) >= 0) {
        self->selectedOptionID = SVSTRING("log in");
        QBEventisAnonymousBrowsingSideMenuPushLogInDialog(self);
    } else if (QBBasicPaneFindItemIdxById(possibleOptions, SVSTRING("subscription info")) >= 0) {
        self->selectedOptionID = SVSTRING("subscription info");
        QBEventisSideMenuShowSubscriptionInfo(self);
    } else if (QBBasicPaneFindItemIdxById(possibleOptions, SVSTRING("more info")) >= 0) {
        self->selectedOptionID = SVSTRING("more info");
        QBEventisSideMenuPushMoreInfoPane(self);
    }
    SVTESTRELEASE(possibleOptions);
}

SvLocal void
QBEventisSideMenuHandleGetOffer(QBEventISCarouselMenuChoice self,
                                TraxisWebVoDGetOfferTransaction transaction)
{
    SvDBRawObject featureContent = TraxisWebVoDGetOfferTransactionGetFeatureContent(transaction, NULL);
    if (featureContent != self->featureContent) {
        SVTESTRETAIN(featureContent);
        SVTESTRELEASE(self->featureContent);
        self->featureContent = featureContent;
    }

    SvImmutableArray featureProducts = TraxisWebVoDGetOfferTransactionGetProducts(transaction, NULL);
    if (featureProducts != self->featureProducts) {
        SVTESTRETAIN(featureProducts);
        SVTESTRELEASE(self->featureProducts);
        self->featureProducts = featureProducts;
    }

    /// TODO: merge content+products into selectedTitle, to get fresh entitlement status
    /// ...

    /// There's waitPane pushed already, so we need to remember this to switch panes later
    self->switchPanes = true;
    if (self->selectedAction == QBEventISCarouselOnButtonActionSideMenu ||
        self->selectedAction == QBEventISCarouselOnButtonActionUndefined) {
        QBBasicPane options = QBEventisSideMenuCreateOptionsPane(self);
        QBEventisSideMenuTraxisPushPane(self, (SvGenericObject) options);
        SVRELEASE(options);
    } else if (self->selectedAction == QBEventISCarouselOnButtonActionPurchase) {
        QBEventisSideMenuDefaultActionChosen(self, true);
    }
}

SvLocal void
QBEventisSideMenuHandlePurchase(QBEventISCarouselMenuChoice self)
{
    QBEventisSideMenuPlay(self, false); // no pin check needed now
}

SvLocal void
QBEventisSideMenuHideCtxMenuAndShowPopup(QBEventISCarouselMenuChoice self,
                                         const char *title, const char *msg)
{
    if (self->dialog) {
        QBDialogBreak(self->dialog);
    }

    QBEventisSideMenuTraxisHidePane(self);

    self->dialog = QBEventisSideMenuCreatePopup(self->appGlobals, title, msg);
    QBDialogRun(self->dialog, self, QBEventisSideMenuPopupCallback);
}

SvLocal void
QBEventisSideMenuHandleServerFailure(QBEventISCarouselMenuChoice self,
                                     const QBContentMgrTransactionStatus *const status)
{
    const char* message = gettext("An error has occurred. Please contact your service provider.\n");
    SvString errorMsg = SvStringCreateWithFormat("%s[%s]", message,
                                                 status->status.message ? SvStringCString(status->status.message) : "Communication error");
    QBEventisSideMenuHideCtxMenuAndShowPopup(self, gettext("A problem occurred"), SvStringCString(errorMsg));
    char* escapedMsg = QBStringCreateJSONEscapedString(status->status.message ? SvStringCString(status->status.message) : "Communication error");
    char *escapedErrorMsg = QBStringCreateJSONEscapedString(message);
    QBSecureLogEvent("QBEventisSideMenu", "ErrorShown.Menus.EventisServerFailure", "JSON:{\"description\":\"%s[%s]\"}",
                     escapedErrorMsg,
                     escapedMsg);
    free(escapedMsg);
    free(escapedErrorMsg);
    SVRELEASE(errorMsg);
}

SvLocal void
QBEventisSideMenuPurchaseStatusChanged(SvGenericObject self_,
                                       SvGenericObject transaction_,
                                       const QBContentMgrTransactionStatus *const status)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;
    if (status->status.state == QBRemoteDataRequestState_finished) {
        SvLogNotice("%s Purchase succeeded.", __func__);
        QBEventisSideMenuHandlePurchase(self);
        SVTESTRELEASE(self->purchase);
        self->purchase = NULL;
    } else if (status->status.state == QBRemoteDataRequestState_failed) {
        SvLogNotice("%s Purchase failed.", __func__);
        QBEventisSideMenuHandleServerFailure(self, status);
        SVTESTRELEASE(self->purchase);
        self->purchase = NULL;
    } else if (status->status.state == QBRemoteDataRequestState_cancelled) {
        SvLogNotice("%s Purchase cancelled", __func__);
        SVTESTRELEASE(self->purchase);
        self->purchase = NULL;
    }
}

SvLocal void
QBEventisSideMenuGetOfferStatusChanged(SvGenericObject self_,
                                       SvGenericObject transaction_,
                                       const QBContentMgrTransactionStatus *const status)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;
    if (status->status.state == QBRemoteDataRequestState_finished) {
        SvLogNotice("%s GetOffer succeeded.", __func__);
        QBEventisSideMenuHandleGetOffer(self, self->getOfferTransaction);
        SVTESTRELEASE(self->getOfferTransaction);
        self->getOfferTransaction = NULL;
    } else if (status->status.state == QBRemoteDataRequestState_failed) {
        SvLogNotice("%s GetOffer failed.", __func__);
        QBEventisSideMenuHandleServerFailure(self, status);
        SVTESTRELEASE(self->getOfferTransaction);
        self->getOfferTransaction = NULL;
    } else if (status->status.state == QBRemoteDataRequestState_cancelled) {
        SvLogNotice("%s GetOffer cancelled", __func__);
        SVTESTRELEASE(self->getOfferTransaction);
        self->getOfferTransaction = NULL;
    }
}


SvLocal void
QBEventisSideMenuHandleFavoritesStatusChanged(QBEventISCarouselMenuChoice self)
{
    TraxisWebVoDProviderReloadFavorites(self->provider);
    /**
     * If we change WishList in "Favorites" category we need to refresh it immediately to see changes.
     **/
    SvGenericObject pathCopy = SvObjectCopy(self->selectedTitlePath, NULL);
    SvInvokeInterface(QBTreePath, pathCopy, truncate, -1);
    QBContentCategory category = (QBContentCategory) SvInvokeInterface(QBTreeModel,
                                                                       self->appGlobals->menuTree,
                                                                       getNode, pathCopy);
    if (TraxisWebVoDProviderIsFavoritesCategory(self->provider, category)) {
        QBContentProviderRefresh((QBContentProvider) self->provider, category, 0, true, false);
    }
    SVRELEASE(pathCopy);
}

SvLocal void
QBEventisSideMenuFavoritesStatusChanged(SvGenericObject self_,
                                        SvGenericObject transaction_,
                                        const QBContentMgrTransactionStatus *const status)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;
    if (status->status.state == QBRemoteDataRequestState_finished) {
        SvLogNotice("%s succeded.", __func__);
        SVTESTRELEASE(self->getFavorites);
        self->getFavorites = NULL;
        QBEventisSideMenuHandleFavoritesStatusChanged(self);
        QBEventisSideMenuHideCtxMenuAndShowPopup(self, gettext("Action ends with success."), NULL);
    } else if (status->status.state == QBRemoteDataRequestState_failed) {
        SvLogNotice("%s failed.", __func__);
        SVTESTRELEASE(self->getFavorites);
        self->getFavorites = NULL;
        QBEventisSideMenuHideCtxMenuAndShowPopup(self, gettext("Action failed! Please try later."), NULL);
    } else if (status->status.state == QBRemoteDataRequestState_cancelled) {
        SvLogNotice("%s cancelled.", __func__);
        SVTESTRELEASE(self->getFavorites);
        self->getFavorites = NULL;
    }
}

SvLocal void
QBEventisSideMenuTransactionStatusChanged(SvGenericObject self_,
                                          SvGenericObject transaction_,
                                          const QBContentMgrTransactionStatus *const status)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;
    if (transaction_ == (SvGenericObject) self->purchase) {
        QBEventisSideMenuPurchaseStatusChanged(self_, transaction_, status);
    } else if (transaction_ == (SvGenericObject) self->getOfferTransaction) {
        QBEventisSideMenuGetOfferStatusChanged(self_, transaction_, status);
    } else if (transaction_ == (SvGenericObject) self->getFavorites) {
        QBEventisSideMenuFavoritesStatusChanged(self_, transaction_, status);
    }
}

SvLocal void
QBEventisSideMenuWaitOnShow(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBEventISCarouselMenuChoice self = ptr;
    svSettingsRestoreContext(self->basicPaneSettingsCtx);
    svSettingsWidgetAttach(frame, self->waitWidget, "WaitWidget", 1);
    svSettingsPopComponent();
}

SvLocal void
QBEventisSideMenuWaitOnHide(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBEventISCarouselMenuChoice self = ptr;
    svWidgetDestroy(self->waitWidget);
    self->waitWidget = NULL;

    if (self->getOfferTransaction) {
        SvInvokeInterface(QBContentMgrTransaction, self->getOfferTransaction, cancel);
        SVTESTRELEASE(self->getOfferTransaction);
        self->getOfferTransaction = NULL;
    }
}

SvLocal void
QBEventisSideMenuWaitSetActive(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBEventISCarouselMenuChoice self = ptr;
    svWidgetSetFocus(self->waitWidget);
}

SvLocal bool
QBEventisSideMenuIsObjectCoupon(QBEventISCarouselMenuChoice self, SvGenericObject object)
{
    if (SvObjectIsInstanceOf(object, SvDBRawObject_getType())) {
        SvString objType = SvDBObjectGetType((SvDBObject) object);
        if (SvStringEqualToCString(objType, "traxis.coupon"))
            return true;
    }
    return false;
}

SvLocal void
QBEventisTitleWithoutProductsPopupCallback(void *self_, SvWidget dialog,
                                           SvString buttonTag, unsigned keyCode)
{
    QBEventISCarouselMenuChoice self = self_;
    self->dialog = NULL;
}

SvLocal bool
QBEventisSideMenuProductHasEntitlement(QBEventISCarouselMenuChoice self)
{
    bool containsProducts = QBEventISMovieParseElementAsBoolean(self->selectedTitle, SVSTRING("ContainsProducts"), false);
    bool isEntitled = QBEventISMovieParseElementAsBoolean(self->selectedTitle, SVSTRING("IsFeatureEntitled"), false);
    if (!isEntitled && !containsProducts) {
        self->dialog = QBEventisSideMenuCreatePopup(self->appGlobals, gettext("The movie is currently not available."), NULL);
        QBDialogRun(self->dialog, self, QBEventisTitleWithoutProductsPopupCallback);
        return false;
    }
    return true;
}

SvLocal void
QBEventisSideMenuExecuteGetOffer(QBEventISCarouselMenuChoice self)
{
    self->getOfferTransaction = TraxisWebVoDGetOfferTransactionCreate(self->provider, self->selectedTitle, NULL);
    SvInvokeInterface(QBContentMgrTransaction, self->getOfferTransaction, setListener, (SvGenericObject) self, NULL);
    SvInvokeInterface(QBContentMgrTransaction, self->getOfferTransaction, start, SvSchedulerGet());
}

SvLocal void
QBEventisSideMenuItemSelected(QBEventISCarouselMenuChoice self, SvGenericObject path,
                              SvGenericObject node, QBEventISCarouselOnButtonAction action)
{
    if (self->lastSelectedPath != path) {
        SVTESTRELEASE(self->lastSelectedPath);
        self->lastSelectedPath = SVRETAIN(path);
    }

    if (QBEventisSideMenuIsObjectCoupon(self, node))
        return;

    if (!SvObjectIsInstanceOf(node, SvDBRawObject_getType()))
        return;

    self->selectedAction = action;
    if ((SvGenericObject) self->selectedTitle != node) {
        SVTESTRELEASE(self->selectedTitle);
        self->selectedTitle = SVRETAIN(node);
    }

    if (self->selectedTitlePath != path) {
        SVTESTRELEASE(self->selectedTitlePath);
        self->selectedTitlePath = SVRETAIN(path);
    }

    if (!QBEventisSideMenuProductHasEntitlement(self))
        return;

    svSettingsPushComponent("BasicPane.settings");
    self->basicPaneSettingsCtx = svSettingsSaveContext();

    QBAccessCode status = QBAccessManagerCheckAccess(self->appGlobals->accessMgr, SVSTRING("VOD_Login"));
    if (status == QBAccess_granted) {
        int level = 1;
        QBContainerPane waitPane = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
        static struct QBContainerPaneCallbacks_t callbacks = {
            .onShow = QBEventisSideMenuWaitOnShow,
            .onHide = QBEventisSideMenuWaitOnHide,
            .setActive = QBEventisSideMenuWaitSetActive,
        };
        QBContainerPaneInit(waitPane, self->appGlobals->res, self->sidemenu.ctx, level,
                            SVSTRING("WaitPane"), &callbacks, self);
        self->waitWidget = svSettingsWidgetCreate(self->appGlobals->res, "WaitWidget");

        QBContextMenuShow(self->sidemenu.ctx);
        QBEventisSideMenuTraxisPushPane(self, (SvGenericObject) waitPane);
        SVRELEASE(waitPane);
        QBEventisSideMenuExecuteGetOffer(self);
    } else if (self->selectedAction == QBEventISCarouselOnButtonActionSideMenu ||
               self->selectedAction == QBEventISCarouselOnButtonActionUndefined) {
        QBBasicPane options = QBEventisAnonymousBrowsingCreateOptionPane(self);
        QBContextMenuShow(self->sidemenu.ctx);
        QBEventisSideMenuTraxisPushPane(self, (SvGenericObject) options);
        SVRELEASE(options);
    } else if (self->selectedAction == QBEventISCarouselOnButtonActionPurchase) {
        QBEventisSideMenuDefaultActionChosen(self, false);
    }
    svSettingsPopComponent();
}

SvLocal void
QBEventisShowSearchPane(QBEventISCarouselMenuChoice self, SvObject path)
{
    SVTESTRELEASE(self->selectedTitle);
    self->selectedTitle = NULL;
    SVTESTRELEASE(self->lastSelectedPath);
    self->lastSelectedPath = SVRETAIN(path);

    svSettingsPushComponent("BasicPane.settings");
    self->basicPaneSettingsCtx = svSettingsSaveContext();
    svSettingsPopComponent();

    QBContextMenuShow(self->sidemenu.ctx);
    QBBasicPane options = QBEventisSideMenuCreateOptionsPane(self);
    QBEventisSideMenuTraxisPushPane(self, (SvGenericObject) options);
    SVRELEASE(options);
}

SvLocal void
QBEventisSideMenuContextChosen(SvGenericObject self_, SvGenericObject nodePath_)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;

    if (self->dialog)
        return;

    if (self->onOptionButtonAction == QBEventISCarouselOnButtonActionUndefined)
        return;

    SvGenericObject node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath_);

    const TraxisWebVoDProviderConfig* config = TraxisWebVoDProviderGetConfig(self->provider);
    if (!SvObjectIsInstanceOf(node, SvDBRawObject_getType()) && config->enableSearchInSideMenu) {
        QBEventisShowSearchPane(self, nodePath_);
        return;
    }

    QBEventisSideMenuItemSelected(self, nodePath_, node, self->onOptionButtonAction);
}

SvLocal void
QBEventisCarouselMenuChoosen(SvGenericObject self_, SvGenericObject node,
                             SvGenericObject nodePath_, int position)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;

    QBEventisSideMenuItemSelected(self, nodePath_, node, self->onEnterButtonAction);
}

SvLocal bool
QBEventisSideMenuIsSearchFailed(QBEventISCarouselMenuChoice self,
                                QBSearchStatus *status, SvString *errorMsg)
{
    bool res = false;

    const char *tmpMsg = NULL;
    switch (status->code) {
        case TraxisWebVoDSearchProviderError_BadQuery: {
            tmpMsg = gettext("The query was not ok, either the resource was not known or the requested resource id was not "
                             "found or query parameters are unknown.");
            res = true;
            break;
        }
        case TraxisWebVoDSearchProviderError_UnauthorizedAccess: {
            tmpMsg = gettext("The access was not authorized (e.g. trying to update Cpe with a different "
                             "customer id) or the supplied CpeId, CustomerId or ProfileId was incorrect.");
            res = true;
            break;
        }
        case TraxisWebVoDSearchProviderError_ResourceNotFound: {
            tmpMsg = gettext("The requested resource id was not found.");
            res = true;
            break;
        }
        case TraxisWebVoDSearchProviderError_AllowedSizeExceeded: {
            tmpMsg = gettext("The custom data exceeds the allowed configured size.");
            res = true;
            break;
        }
        case TraxisWebVoDSearchProviderError_InternalServerError: {
            tmpMsg = gettext("Internal server error");
            res = true;
            break;
        }
        default: {
            if (status->code >= 400) {
                res = true;
            }
            break;
        }
    }

    if (errorMsg && res) {
        if (tmpMsg)
            *errorMsg = SvStringCreateWithFormat("%s[%s]", gettext("An error has occurred.\n"), tmpMsg);
        else {
            *errorMsg = SvStringCreateWithFormat("%s[%s]", gettext("An error has occurred.\n"), status->message ? status->message : gettext("Search Error"));
        }
    }

    return res;
}

SvLocal void
QBEventisSideMenuSearchStatusUpdate(SvGenericObject self_, QBSearchStatus status)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;
    SvString errorMsg = NULL;
    if (!QBEventisSideMenuIsSearchFailed(self, &status, &errorMsg))
        return;

    QBEventisSideMenuHideCtxMenuAndShowPopup(self, gettext("A problem occurred"), SvStringCString(errorMsg));
    SVRELEASE(errorMsg);

    TraxisWebVoDProviderSetSearchCallback(self->provider, NULL, NULL);
}

SvLocal void
QBEventisSideMenuOSKKeyTyped(void *self_, QBOSKPane pane, SvString input,
                             unsigned int layout, QBOSKKey key)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;

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

            TraxisWebVoDProviderSetSearchCallback(self->provider, (void *) self,
                                                  QBEventisSideMenuSearchStatusUpdate);
            QBContentSearchStartNewSearchWithCategory(self->contentSearch, keyword, self->appGlobals->scheduler, self->searchRootCategory);
            SVRELEASE(keyword);

            if (self->contentSearchPath) {
                QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, self->contentSearchPath, 0);
                SvGenericObject destPath = QBTreeIteratorGetCurrentNodePath(&iter);
                if (destPath) {
                    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
                    SvInvokeInterface(QBMenu, menuBar->prv, setPosition, destPath, NULL);
                }
            }

            QBEventisSideMenuTraxisHidePane(self);

            SVTESTRELEASE(self->contentSearch);
            SVTESTRELEASE(self->contentSearchPath);
            self->contentSearch = NULL;
            self->contentSearchPath = NULL;
        }
    }
}

SvLocal void
QBEventisSideMenuCheckCategoryParentalControlPINCallback(void *self_, SvWidget dlg,
                                                         SvString ret, unsigned key)
{
    QBEventISCarouselMenuChoice self = self_;

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
            SvGenericObject destPath = QBTreeIteratorGetCurrentNodePath(&iter);
            SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
            SvInvokeInterface(QBMenu, menuBar->prv, setPosition, destPath, NULL);
        }
    }

    SVRELEASE(self->protectedCategoryPath);
    self->protectedCategoryPath = NULL;
}

SvLocal SvString
QBEventISCarouselMenu_getCategoryRating(QBContentCategory category)
{
    SvString rating = NULL;

    SvValue isAdult = (SvValue) QBContentCategoryGetAttribute(category, SVSTRING("isAdult"));
    SvValue PCRating = (SvValue) QBContentCategoryGetAttribute(category, SVSTRING("PCRating"));

    if (isAdult && SvObjectIsInstanceOf((SvObject) isAdult, SvValue_getType()) && SvValueIsBoolean(isAdult)) {
        if (SvValueGetBoolean(isAdult)) {
            rating = SVSTRING("[AGE=18]");
            return rating;
        }
    }
    if (PCRating && SvObjectIsInstanceOf((SvObject) PCRating, SvValue_getType()) && SvValueIsString(PCRating)) {
        rating = SvValueGetString(PCRating);
    }

    return rating;
}

SvLocal bool
QBEventisSideMenuCheckCategoryParentalRating(QBEventISCarouselMenuChoice self,
                                             QBContentCategory category,
                                             SvGenericObject path)
{
    SvString rating = QBEventISCarouselMenu_getCategoryRating(category);
    if (!rating || QBParentalControlRatingAllowed(self->appGlobals->pc, rating))
        return false;

    // remember path to this node to use it in dialog's callback
    SVTESTRELEASE(self->protectedCategoryPath);
    self->protectedCategoryPath = SVRETAIN(path);

    svSettingsPushComponent("AuthDialogPCMENU.settings");
    SvGenericObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, SVSTRING("PC_MENU"));
    SvWidget dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"), gettext("Please enter parental control PIN to enter this category"), true, NULL, NULL);
    self->parentalPopup = dialog;
    QBDialogRun(dialog, self, QBEventisSideMenuCheckCategoryParentalControlPINCallback);

    return true;
}

SvLocal void
QBEventisSideMenuLoginToCategoryCallback(void *self_, SvWidget dialog,
                                         SvString buttonTag, unsigned keyCode)
{
    QBEventISCarouselMenuChoice self = self_;

    self->dialog = NULL;
    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, self->path, 0);
        if (QBTreeIteratorGetNodesCount(&iter) > 0) {
            SvGenericObject destPath = QBTreeIteratorGetCurrentNodePath(&iter);
            SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
            SvInvokeInterface(QBMenu, menuBar->prv, setPosition, destPath, NULL);
        }
    }
}

SvLocal bool
QBEventisSideMenuVerifyCategoryAccess(QBEventISCarouselMenuChoice self, SvGenericObject nodePath)
{
    if (TraxisWebSessionManagerGetState(self->appGlobals->traxisWebSessionManager) != TraxisWebSessionState_active)
        return false;

    if (QBAccessManagerCheckAccess(self->appGlobals->accessMgr, SVSTRING("VOD_Login")) == QBAccess_granted)
        return true;

    svSettingsPushComponent("AuthDialogVODLogin.settings");
    SvGenericObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler,
                                                                   self->appGlobals->accessMgr,
                                                                   SVSTRING("VOD_Login"));
    self->dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Log In"),
                                      gettext("In order to browse this category, please log in."),
                                      true, NULL, NULL);

    svSettingsPopComponent();

    SVTESTRELEASE(self->path);
    self->path = SVTESTRETAIN(nodePath);

    QBDialogRun(self->dialog, self, QBEventisSideMenuLoginToCategoryCallback);
    return false;
}

SvLocal bool
QBEventisSideMenuNodeSelected(SvGenericObject self_, SvGenericObject node_,
                              SvGenericObject nodePath)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice) self_;

    if (SvObjectIsInstanceOf(node_, QBContentSearch_getType())) {
        SVTESTRELEASE(self->searchRootCategory);
        self->searchRootCategory = NULL;
        return QBEventisCreateSearchPane(self, (QBContentSearch) node_, nodePath);
    } else if (SvObjectIsInstanceOf(node_, QBContentCoupons_getType())) {
        return !QBEventisSideMenuVerifyCategoryAccess(self, nodePath);
    } else if (SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        if (SvStringEqualToCString(QBContentCategoryGetName((QBContentCategory) node_), "My Rentals")) {
            TraxisWebVoDProviderReloadCategory(self->provider, (QBContentCategory) node_);
            return !QBEventisSideMenuVerifyCategoryAccess(self, nodePath);
        } else if (TraxisWebVoDProviderIsFavoritesCategory(self->provider, (QBContentCategory) node_)) {
            return !QBEventisSideMenuVerifyCategoryAccess(self, nodePath);
        } else {
            return QBEventisSideMenuCheckCategoryParentalRating(self, (QBContentCategory) node_, nodePath);
        }
    }

    return false;
}

SvLocal void
QBEventISCarouselMenuChoiceParseOnButtonActions(QBEventISCarouselMenuChoice self,
                                                const QBItemChoiceConfig *config)
{
    if (!config || !self)
        return;

    if (config->onEnterButton) {
        if (SvStringEqualToCString(config->onEnterButton, "PurchaseAction")) {
            self->onEnterButtonAction = QBEventISCarouselOnButtonActionPurchase;
        } else if (SvStringEqualToCString(config->onEnterButton, "SideMenuAction")) {
            self->onEnterButtonAction = QBEventISCarouselOnButtonActionSideMenu;
        }
    } else {
        self->onEnterButtonAction = QBEventISCarouselOnButtonActionUndefined;
    }
    if (config->onOptionButton) {
        if (SvStringEqualToCString(config->onOptionButton, "PurchaseAction")) {
            self->onOptionButtonAction = QBEventISCarouselOnButtonActionPurchase;
        } else if (SvStringEqualToCString(config->onOptionButton, "SideMenuAction")) {
            self->onOptionButtonAction = QBEventISCarouselOnButtonActionSideMenu;
        }
    } else {
        self->onOptionButtonAction = QBEventISCarouselOnButtonActionUndefined;
    }
}

SvLocal void
QBEventISCarouselMenuChoiceDestroy(void *self_)
{
    QBEventISCarouselMenuChoice self = self_;

    if (self->sidemenu.ctx) {
        QBContextMenuSetCallbacks(self->sidemenu.ctx, NULL, NULL);
        QBEventisSideMenuTraxisHidePane(self);
        SVRELEASE(self->sidemenu.ctx);
    }

    if (self->dialog)
        QBDialogBreak(self->dialog);

    svWidgetDestroy(self->waitWidget);
    self->waitWidget = NULL;

    SVTESTRELEASE(self->protectedCategoryPath);

    if (self->playContent) {
        QBTraxisPlayContentCancel(self->playContent);
        SVRELEASE(self->playContent);
    }

    if (self->purchase) {
        QBProductPurchaseCancel(self->purchase);
        SVRELEASE(self->purchase);
    }
    if (self->getOfferTransaction) {
        SvInvokeInterface(QBContentMgrTransaction, self->getOfferTransaction, cancel);
        SVTESTRELEASE(self->getOfferTransaction);
        self->getOfferTransaction = NULL;
    }

    if (self->getFavorites) {
        SvInvokeInterface(QBContentMgrTransaction, self->getFavorites, cancel);
        SVTESTRELEASE(self->getFavorites);
        self->getFavorites = NULL;
    }

    SVTESTRELEASE(self->selectedTitle);
    SVTESTRELEASE(self->selectedTitlePath);
    SVTESTRELEASE(self->selectedContent);
    SVTESTRELEASE(self->selectedProduct);
    SVTESTRELEASE(self->selectedCoupon);
    SVTESTRELEASE(self->selectedOptionID);

    SVRELEASE(self->itemNamesFilename);
    SVTESTRELEASE(self->rentalTermsMessage);

    SVTESTRELEASE(self->featureContent);
    SVTESTRELEASE(self->featureProducts);

    SVTESTRELEASE(self->provider);

    SVTESTRELEASE(self->contentSearch);
    SVTESTRELEASE(self->contentSearchPath);
    SVTESTRELEASE(self->searchRootCategory);
    SVTESTRELEASE(self->lastSelectedPath);
    SVTESTRELEASE(self->path);

    SVTESTRELEASE(self->serviceName);

    svWidgetDestroy(self->extendedInfo.viewport);
}

SvLocal SvType
QBEventISCarouselMenuChoice_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBEventISCarouselMenuChoiceDestroy
    };

    static const struct QBMenuContextChoice_t contextMethods = {
        .contextChoosen = QBEventisSideMenuContextChosen,
    };

    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBEventisCarouselMenuChoosen,
    };
    static const struct QBContentMgrTransactionListener_ productPurchaseStatus = {
        .statusChanged = QBEventisSideMenuTransactionStatusChanged,
    };

    static const struct QBMenuEventHandler_ selectedMethods = {
        .nodeSelected = QBEventisSideMenuNodeSelected,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBEventISCarouselMenuChoice",
                            sizeof(struct QBEventISCarouselMenuChoice_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuContextChoice_getInterface(), &contextMethods,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBMenuEventHandler_getInterface(), &selectedMethods,
                            QBContentMgrTransactionListener_getInterface(), &productPurchaseStatus,
                            NULL);
    }

    return type;
}

QBEventISCarouselMenuChoice
QBEventISCarouselMenuChoiceNew(AppGlobals appGlobals, SvGenericObject vodProvider, SvString serviceName,
                               SvString optionsNamesFilename, const QBItemChoiceConfig *config)
{
    QBEventISCarouselMenuChoice self = (QBEventISCarouselMenuChoice)
                                            SvTypeAllocateInstance(QBEventISCarouselMenuChoice_getType(), NULL);

    self->provider = (TraxisWebVoDProvider) SVTESTRETAIN(vodProvider);
    self->appGlobals = appGlobals;
    self->serviceName = SVRETAIN(serviceName);
    self->itemNamesFilename = SVRETAIN(optionsNamesFilename);
    QBEventISCarouselMenuChoiceParseOnButtonActions(self, config);

    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBEventisSideMenuClose, self);

    svSettingsPushComponent("carouselEventisExtendedInfo.settings");
    const char *rentalTermsMessage = svSettingsGetString("messages", "rentalTermsMessage");
    if (rentalTermsMessage) {
        self->rentalTermsMessage = SvStringCreate(rentalTermsMessage, NULL);
    }
    self->extendedInfo.grid = QBGridCreate(appGlobals->res, "grid");
    self->extendedInfo.viewport = svViewPortNew(appGlobals->res, "viewport");
    svViewPortSetContents(self->extendedInfo.viewport, self->extendedInfo.grid);
    self->extendedInfo.title = QBGridAddAsyncLabel(self->extendedInfo.grid, "title", appGlobals->textRenderer);
    self->extendedInfo.description = QBGridAddAsyncLabel(self->extendedInfo.grid, "description", appGlobals->textRenderer);
    self->extendedInfo.image = QBGridAddIcon(self->extendedInfo.grid, "image");
    self->extendedInfo.captionLanguages = QBGridAddAsyncLabel(self->extendedInfo.grid, "captionLanguages", appGlobals->textRenderer);
    svSettingsPopComponent();

    return self;
}
