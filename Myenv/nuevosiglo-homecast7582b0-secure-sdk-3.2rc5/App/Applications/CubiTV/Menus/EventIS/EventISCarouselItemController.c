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

#include "EventISCarouselItemController.h"

#include <libintl.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include "QBCarousel/QBVerticalCarousel.h"
#include "Windows/pvrplayer.h"
#include <TraxisWebClient/TraxisWebVoDProvider.h>
#include <QBContentManager/QBContentCoupons.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBContentManager/QBContentStub.h>
#include <Windows/mainmenu.h>
#include <SWL/QBFrame.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Menus/menuchoice.h>
#include <Widgets/XMBCarousel.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuController.h>
#include <XMB2/XMBVerticalMenu.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <Widgets/eventISMovieInfo.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <Utils/value.h>
#include <DataModels/loadingProxy.h>
#include <QBConf.h>
#include <Services/core/appState.h>
#include <main.h>
#include <QBStringUtils.h>
#include <Logic/timeFormat.h>
#include <QBContentManager/QBLoadingStub.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <SvJSON/SvJSONParse.h>

#include "EventISCarouselItemChoice.h"

struct QBEventISCarouselMenuItemController_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    QBXMBItemConstructor itemConstructor;
    QBXMBItemConstructor bigItemConstructor;
    unsigned int settingsCtx;

    SvGenericObject provider;

    SvRID iconRID, searchIconRID, couponIconRID;

    QBEventISMovieDynamicLogoProvider dynamicLogoProvider;
};

SvLocal void
QBEventISCarouselMenuItemControllerDestroy(void *self_)
{
    QBEventISCarouselMenuItemController self = self_;

    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVTESTRELEASE(self->dynamicLogoProvider);
    if (self->itemConstructor)
        QBXMBItemConstructorDestroy(self->itemConstructor);
    if (self->bigItemConstructor)
        QBXMBItemConstructorDestroy(self->bigItemConstructor);
    if (self->provider) {
        QBContentProviderStop((QBContentProvider) self->provider);
        SVRELEASE(self->provider);
    }
}

SvLocal SvString
QBEventISCarouselMenuItemControllerCreateCouponSubcaption(SvDBRawObject coupon)
{
    SvValue expirationDate_ = (SvValue)SvDBRawObjectGetAttrValue(coupon, "ExpirationDate");
    SvString dateStr = NULL;
    if (expirationDate_ && SvObjectIsInstanceOf((SvObject) expirationDate_, SvValue_getType()) && SvValueIsString(expirationDate_)) {
        SvString expirationDate = SvValueGetString(expirationDate_);
        if (SvStringLength(expirationDate) > 0) {
            SvTime value;
            if (QBStringToDateTime(SvStringCString(expirationDate), QBDateTimeFormat_ISO8601, false, &value) == (ssize_t) SvStringLength(expirationDate)) {
                struct tm localTime;
                if (SvTimeBreakDown(value, true, &localTime) < 0) {
                    // date/time outside of allowed range
                } else {
                    char buff[128];
                    if (QBTimeFormatGetCurrent()->traxisEntitlementDate)
                        strftime(buff, sizeof(buff), QBTimeFormatGetCurrent()->traxisEntitlementDate, &localTime);
                    else
                        strftime(buff, sizeof(buff), "%X %x", &localTime);
                    dateStr = SvStringCreate(buff, NULL);
                }
            }
        }
    }

    SvString subcaption = NULL;
    if (dateStr) {
        subcaption = SvStringCreateWithFormat(gettext("Expires at : %s"),
                                              dateStr ? SvStringCString(dateStr) : "");
        SVRELEASE(dateStr);
    } else {
        subcaption = SvStringCreateWithFormat(gettext("No time limits"));
    }

    return subcaption;
}

SvLocal SvWidget
QBEventISCarouselMenuItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBEventISCarouselMenuItemController self = (QBEventISCarouselMenuItemController) self_;


    QBXMBItem item = QBXMBItemCreate();
    QBXMBItemConstructor constructor = self->itemConstructor;

    item->caption = SVSTRING("????");
    item->loadingRID = self->iconRID;
    SvString titleIconURI = NULL;

    if (SvObjectIsInstanceOf(node_, QBContentSearch_getType())) {
        item->caption = SvStringCreate(gettext("Search"), NULL);
        item->iconRID = self->searchIconRID;
    } else if (SvObjectIsInstanceOf(node_, QBContentCoupons_getType())) {
        item->caption = SvStringCreate(gettext("My Coupons"), NULL);
        item->iconRID = self->couponIconRID;
    } else if (SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        QBContentCategory node = (QBContentCategory) node_;
        SvString title = SvValueTryGetString((SvValue) QBContentCategoryGetAttribute(node, SVSTRING("shortTitle")));
        if (!title)
            title = QBContentCategoryGetTitle(node);

        item->caption = SVRETAIN(title);
        titleIconURI = SvValueTryGetString((SvValue) QBContentCategoryGetAttribute(node, SVSTRING("titleIconURI")));
    } else if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType())) {
        SvDBRawObject leaf = (SvDBRawObject) node_;
        SvValue name_ = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) leaf, "Name");
        if (name_) {
            SvString name = SvValueGetString(name_);
            item->caption = SVTESTRETAIN(name);
        }

        SvString objType = SvDBObjectGetType((SvDBObject) node_);
        if (SvStringEqualToCString(objType, "traxis.coupon")) {
            item->subcaption = QBEventISCarouselMenuItemControllerCreateCouponSubcaption(leaf);

            if (self->couponIconRID)
                item->iconRID = self->couponIconRID;
            if (self->bigItemConstructor)
                constructor = self->bigItemConstructor;
        }
    } else if (SvObjectIsInstanceOf(node_, QBLoadingStub_getType()) || SvObjectIsInstanceOf(node_, QBContentStub_getType())) {
        bool isLoading = true;
        if (SvObjectIsInstanceOf(node_, QBLoadingStub_getType())) {
            isLoading = ((QBLoadingStub) node_)->isLoading;
        }
        if (isLoading)
            item->caption = SvStringCreate(gettext("Please wait, loading…"), NULL);
        else
            item->caption = SvStringCreate(gettext("No items"), NULL);
    } else if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        titleIconURI = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("icon"));
        item->caption = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode)node_, SVSTRING("caption"));
        SVTESTRETAIN(item->caption);
    }

    item->iconURI.URI = SVTESTRETAIN(titleIconURI);

    SvWidget ret = NULL;

    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    ret = QBXMBItemConstructorCreateItem(constructor, item, app, initialState);

    SVRELEASE(item);
    return ret;
}

SvLocal void
QBEventISCarouselMenuItemControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBEventISCarouselMenuItemController self = (QBEventISCarouselMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}


SvLocal SvWidget
QBEventISCarouselMenuItemControllerCreateSubMenu(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app)
{
    QBEventISCarouselMenuItemController self = (QBEventISCarouselMenuItemController) self_;

    if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType()))
        return NULL;
    if (SvObjectIsInstanceOf(node_, QBContentStub_getType()))
        return NULL;
    if (SvObjectIsInstanceOf(node_, QBLoadingStub_getType()))
        return NULL;

    if (SvObjectIsInstanceOf(node_, QBContentCoupons_getType())) {
        svSettingsPushComponent("PromoCouponsMenu.settings");
        SvWidget w = XMBVerticalMenuNew(app, "menu", NULL);

        if (!self->bigItemConstructor)
            self->bigItemConstructor = QBXMBItemConstructorCreate("MenuItem", self->appGlobals->textRenderer);
        svSettingsPopComponent();

        return w;
    }

    svSettingsPushComponent("Carousel_VOD.settings");

    SvWidget w = XMBCarouselNew(app, "menu");
    SvWidget carousel = XMBCarouselGetCarouselWidget(w);

    int i;
    for( i = 0; i < QBVerticalCarouselGetSlotCount(carousel); i++ ) {
        SvWidget movieInfo = QBEventISMovieInfoCreate(self->appGlobals);
        QBEventISMovieInfoSetDynamicLogoProvider(movieInfo, self->dynamicLogoProvider);
        QBVerticalCarouselSetContentAtIndex(carousel, i, movieInfo);
        QBVerticalCarouselSetDataHandler(carousel, i, QBEventISMovieInfoSetObject);
    }
    svSettingsPopComponent();
    return w;
}


SvLocal SvType
QBEventISCarouselMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBEventISCarouselMenuItemControllerDestroy
    };
    static SvType type = NULL;

    static const struct XMBMenuController_t menu_controller_methods = {
        .createSubMenu = QBEventISCarouselMenuItemControllerCreateSubMenu,
    };

    static const struct XMBItemController_t controller_methods = {
        .createItem = QBEventISCarouselMenuItemControllerCreateItem,
        .setItemState = QBEventISCarouselMenuItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBEventISCarouselMenuItemController",
                            sizeof(struct QBEventISCarouselMenuItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            XMBMenuController_getInterface(), &menu_controller_methods,
                            NULL);
    }

    return type;
}

SvLocal QBEventISMovieDynamicLogoProvider
QBEventISCarouselMenuItemControllerCreateDynamicLogoProvider(void)
{
    QBEventISMovieDynamicLogoProvider provider = NULL;
    SvValue prefix = NULL, suffix = NULL;

    SvString dynamicLogo = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/VODProviderLogo.json", SvGetRuntimePrefix());
    SvObject json = SvJSONParseFile(SvStringCString(dynamicLogo), false, NULL);
    SVRELEASE(dynamicLogo);
    if (!json)
        return NULL;

    if (!SvObjectIsInstanceOf(json, SvHashTable_getType()))
        goto ret;

    prefix = (SvValue) SvHashTableFind((SvHashTable)json, (SvObject)SVSTRING("prefix"));
    suffix = (SvValue) SvHashTableFind((SvHashTable)json, (SvObject)SVSTRING("suffix"));

    if (!prefix || !suffix || !SvValueIsString(prefix) || !SvValueIsString(suffix) || SvStringGetLength(SvValueGetString(prefix)) == 0)
        goto ret;

    provider = (QBEventISMovieDynamicLogoProvider)SvTypeAllocateInstance(QBEventISMovieDynamicLogoProvider_getType(), NULL);
    provider->prefix = SVRETAIN(SvValueGetString(prefix));
    provider->suffix = SVRETAIN(SvValueGetString(suffix));

  ret:
    SVRELEASE(json);
    return provider;
}

QBEventISCarouselMenuItemController
QBEventISCarouselMenuItemControllerCreate(AppGlobals appGlobals, SvGenericObject provider)
{
    QBEventISCarouselMenuItemController itemController = (QBEventISCarouselMenuItemController) SvTypeAllocateInstance(
            QBEventISCarouselMenuItemController_getType(), NULL);

    itemController->appGlobals = appGlobals;
    itemController->settingsCtx = svSettingsSaveContext();
    itemController->provider = SVTESTRETAIN(provider);

    svSettingsPushComponent("VODMenu.settings");

    itemController->searchIconRID = svSettingsGetResourceID("MenuItem", "searchIcon");
    itemController->couponIconRID = svSettingsGetResourceID("MenuItem", "couponIcon");
    itemController->iconRID = svSettingsGetResourceID("MenuItem", "icon");
    itemController->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        itemController->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    itemController->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);
    itemController->bigItemConstructor = NULL;
    itemController->dynamicLogoProvider = QBEventISCarouselMenuItemControllerCreateDynamicLogoProvider();

    svSettingsPopComponent();

    return itemController;
}

static bool getBoolean(SvHashTable attrs, SvString key)
{
    SvValue val = (SvValue) SvHashTableFind(attrs, (SvObject) key);
    if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType())) {
        if (SvValueIsBoolean(val))
            return SvValueGetBoolean(val);
        else if (SvValueIsInteger(val))
            return SvValueGetInteger(val) != 0;
    }
    return false;
}

static SvString getString(SvHashTable attrs, SvString key)
{
    SvGenericObject val = SvHashTableFind(attrs, (SvGenericObject) key);
    if (val && SvObjectIsInstanceOf(val, SvString_getType()))
        return (SvString) val;
    return NULL;
}

SvLocal void
getUnsignedInt(SvHashTable attrs, SvString key, unsigned int *ret, unsigned int min, unsigned int max)
{
    SvValue val = (SvValue) SvHashTableFind(attrs, (SvObject) key);
    if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsInteger(val)) {
        int tmp = SvValueGetInteger(val);
        if (tmp < 0)
            return;
        if ((unsigned) tmp >= min && (unsigned) tmp <= max) {
            *ret = tmp;
        }
    }
}

static TraxisWebVoDFilterState getTraxisWebVoDFilterState(SvHashTable attrs, SvString key)
{
    SvValue val = (SvValue) SvHashTableFind(attrs, (SvObject) key);
    if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType())) {
        if (SvValueIsBoolean(val))
            return SvValueGetBoolean(val) ? TraxisWebVoDFilterTrue : TraxisWebVoDFilterFalse;
        else if (SvValueIsInteger(val))
            return (SvValueGetInteger(val) != 0) ? TraxisWebVoDFilterTrue : TraxisWebVoDFilterFalse;
    }
    return TraxisWebVoDFilterDisabled;
}

QBCarouselMenuItemService
QBEventISCarouselMenuCreate(AppGlobals appGlobals, SvString catalogName, SvHashTable attrs)
{
    if (!appGlobals->traxisWebSessionManager) {
        SvLogError("%s(): Traxis.Web session manager not initialized!", __func__);
        return NULL;
    }

    SvString caption = (SvString) SvHashTableFind(attrs, (SvGenericObject) SVSTRING("TraxisWebCatalogCaption"));
    if (!caption)
        caption = SVSTRING("VOD");
    QBContentTree tree = QBContentTreeCreate(caption, NULL);

    unsigned int maxCachedAssets = 0, maxSearchResults = 0, searchCategoryDepth = 0;

    getUnsignedInt(attrs, SVSTRING("TraxisWebMaxCachedAssets"), &maxCachedAssets, 0, INT_MAX);
    getUnsignedInt(attrs, SVSTRING("TraxisWebMaxSearchResults"), &maxSearchResults, 0, INT_MAX);
    getUnsignedInt(attrs, SVSTRING("TraxisWebSearchInEachCategoryDepth"), &searchCategoryDepth, 0, INT_MAX);

    bool enableMyRentals            = getBoolean(attrs, SVSTRING("TraxisWebEnableMyRentals"));
    bool enableCoupons              = getBoolean(attrs, SVSTRING("TraxisWebEnableCoupons"));
    bool enableSearch               = getBoolean(attrs, SVSTRING("TraxisWebEnableSearch"));
    bool enableSearchInEachCategory = getBoolean(attrs, SVSTRING("TraxisWebEnableSearchInEachCategory"));
    bool enableSearchInSideMenu     = getBoolean(attrs, SVSTRING("TraxisWebEnableSearchInSideMenu"));
    bool enableFavorites            = getBoolean(attrs, SVSTRING("TraxisWebEnableFavorites"));
    bool ingnoreTVOD                = getBoolean(attrs, SVSTRING("TraxisWebIgnoreTransactionProducts"));
    bool ingnoreSVOD                = getBoolean(attrs, SVSTRING("TraxisWebIgnoreSubscriptionProducts"));
    bool ignoreTVODOnFavorites      = getBoolean(attrs, SVSTRING("TraxisWebIgnoreTransactionProductsOnFavorites"));
    bool ignoreSVODOnFavorites      = getBoolean(attrs, SVSTRING("TraxisWebIgnoreSubscriptionProductsOnFavorites"));
    bool ignoreFVODOnFavorites      = getBoolean(attrs, SVSTRING("TraxisWebIgnoreFreeProductsOnFavorites"));

    TraxisWebVoDProviderFilterConfig filterConfig = {
        .previewTitlesFilter             = getTraxisWebVoDFilterState(attrs, SVSTRING("TraxisWebPreviewTitlesFilter")),
        .viewableTitlesFilter            = getTraxisWebVoDFilterState(attrs, SVSTRING("TraxisWebViewableTitlesFilter")),
        .visibleTitlesFilter             = getTraxisWebVoDFilterState(attrs, SVSTRING("TraxisWebVisibleTitlesFilter")),
        .featureTitlesFilter             = getTraxisWebVoDFilterState(attrs, SVSTRING("TraxisWebFeatureTitlesFilter"))
    };

    TraxisWebVoDProviderFilterConfig searchFilterConfig = {
        .previewTitlesFilter             = getTraxisWebVoDFilterState(attrs, SVSTRING("TraxisWebPreviewTitlesFilterOnSearch")),
        .viewableTitlesFilter            = getTraxisWebVoDFilterState(attrs, SVSTRING("TraxisWebViewableTitlesFilterOnSearch")),
        .visibleTitlesFilter             = getTraxisWebVoDFilterState(attrs, SVSTRING("TraxisWebVisibleTitlesFilterOnSearch")),
        .featureTitlesFilter             = getTraxisWebVoDFilterState(attrs, SVSTRING("TraxisWebFeatureTitlesFilterOnSearch"))
    };

    TraxisWebVoDProviderFilterConfig myRentalsFilterConfig = {
        .previewTitlesFilter             = getTraxisWebVoDFilterState(attrs, SVSTRING("TraxisWebPreviewTitlesFilterOnMyRentals")),
        .viewableTitlesFilter            = getTraxisWebVoDFilterState(attrs, SVSTRING("TraxisWebViewableTitlesFilterOnMyRentals")),
        .visibleTitlesFilter             = getTraxisWebVoDFilterState(attrs, SVSTRING("TraxisWebVisibleTitlesFilterOnMyRentals")),
        .featureTitlesFilter             = getTraxisWebVoDFilterState(attrs, SVSTRING("TraxisWebFeatureTitlesFilterOnMyRentals"))
    };

    const char *categoriesSortOrder = NULL;
    bool sortCategoriesDesc = false;
    SvString categoriesOrderStr = (SvString) SvHashTableFind(attrs, (SvObject) SVSTRING("TraxisWebCategoriesSortOrder"));
    if (categoriesOrderStr && SvObjectIsInstanceOf((SvObject) categoriesOrderStr, SvString_getType())) {
        if (SvStringCString(categoriesOrderStr)[0] == '/') {
            categoriesSortOrder = SvStringCString(categoriesOrderStr) + 1;
            sortCategoriesDesc = true;
        } else {
            categoriesSortOrder = SvStringCString(categoriesOrderStr);
        }
    }

    const char *titlesSortOrder = NULL;
    bool sortTitlesDesc = false;
    SvString titlesOrderStr = (SvString) SvHashTableFind(attrs, (SvObject) SVSTRING("TraxisWebTitlesSortOrder"));
    if (titlesOrderStr && SvObjectIsInstanceOf((SvObject) titlesOrderStr, SvString_getType())) {
        if (SvStringCString(titlesOrderStr)[0] == '/') {
            titlesSortOrder = SvStringCString(titlesOrderStr) + 1;
            sortTitlesDesc = true;
        } else {
            titlesSortOrder = SvStringCString(titlesOrderStr);
        }
    }

    TraxisWebVoDProviderConfig config = {
        .minRefreshPeriod = 1800,
        .enableMyRentals = enableMyRentals,
        .enableCoupons = enableCoupons,
        .enableSearch = enableSearch,
        .enableSearchInEachCategory = enableSearchInEachCategory,
        .enableSearchInSideMenu = enableSearchInSideMenu,
        .enableFavorites = enableFavorites,
        .ignoreTransactionProducts = ingnoreTVOD,
        .ignoreSubscriptionProducts = ingnoreSVOD,
        .ignoreTransactionProductsOnFavorites = ignoreTVODOnFavorites,
        .ignoreSubscriptionProductsOnFavorites = ignoreSVODOnFavorites,
        .ignoreFreeProductsOnFavorites = ignoreFVODOnFavorites,
        .filterConfig = filterConfig,
        .searchFilterConfig = searchFilterConfig,
        .myRentalsFilterConfig = myRentalsFilterConfig,
        .maxCachedAssets = maxCachedAssets,
        .maxSearchResults = maxSearchResults,
        .searchInEachCategoryDepth = searchCategoryDepth,
    };

    const QBItemChoiceConfig choiceConfig = {
        .onEnterButton  = getString(attrs, SVSTRING("QBItemChoiceActionOnEnterButton")),
        .onOptionButton = getString(attrs, SVSTRING("QBItemChoiceActionOnOptionButton"))
    };

    TraxisWebVoDProvider provider = TraxisWebVoDProviderCreate(appGlobals->traxisWebSessionManager, tree, catalogName, &config, NULL);
    TraxisWebVoDProviderSetSortOrder(provider, categoriesSortOrder, sortCategoriesDesc, titlesSortOrder, sortTitlesDesc, NULL);

    SvLogNotice("Registering Traxis service: %s (catalog = '%s')", "VOD", catalogName ? SvStringCString(catalogName) : "-");
    SvGenericObject itemController = (SvGenericObject) QBEventISCarouselMenuItemControllerCreate(appGlobals, (SvGenericObject) provider);
    SvGenericObject itemChoice = (SvGenericObject) QBEventISCarouselMenuChoiceNew(appGlobals, (SvGenericObject) provider, SVSTRING("VOD"), SVSTRING("EventISCarouselItemChoiceOptions.json"), &choiceConfig);

    QBCarouselMenuItemService itemInfo = QBCarouselMenuCreate(appGlobals, SVSTRING("VOD"), SVSTRING("VOD"), SVSTRING("VOD"), NULL, (SvGenericObject) provider, itemController, itemChoice, NULL, (SvGenericObject) tree);

    SVRELEASE(provider);
    SVRELEASE(itemController);
    SVRELEASE(itemChoice);
    SVRELEASE(tree);

    return itemInfo;
}

void
QBEventISCarouselMenuForceReload(SvGenericObject provider)
{
    TraxisWebVoDProviderForceReloadEverything((TraxisWebVoDProvider) provider);
}
