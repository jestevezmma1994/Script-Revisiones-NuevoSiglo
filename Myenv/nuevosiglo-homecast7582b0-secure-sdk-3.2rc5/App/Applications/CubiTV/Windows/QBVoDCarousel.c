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

#include "QBVoDCarousel.h"

#include <libintl.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/QBFrame.h>
#include <SWL/icon.h>
#include <SWL/clock.h>
#include <QBStringUtils.h>
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBListModel.h>
#include <QBDataModel3/QBListModelListener.h>
#include <QBDataModel3/QBListProxy.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBCarousel/QBVerticalCarousel.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <Windows/pvrplayer.h>
#include <Widgets/QBSimpleDialog.h>
#include <Widgets/QBTitle.h>
#include <QBWidgets/QBGrid.h>
#include <Widgets/authDialog.h>
#include <Widgets/infoIcons.h>
#include <Widgets/trailerPreview.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBVoDMenu.h>
#include <ContextMenus/QBInnov8onExtendedInfoPane.h>
#include <Middlewares/Innov8on/innov8onTransaction.h>
#include <SvDataBucket2/SvDBReference.h>
#include <Logic/AnyPlayerLogic.h>
#include <Utils/vod/QBListProxyNegativeFilter.h>
#include <Utils/authenticators.h>
#include <Utils/value.h>
#include <Utils/QBMovieUtils.h>
#include <Utils/QBVODUtils.h>
#include <player_hints/http_input.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentSeeAll.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentCategoryListener.h>
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include <SvPlayerKit/SvContent.h>
#include <main.h>
#include <QBPCRatings/QBPCUtils.h>
#include <Logic/GUILogic.h>
#include <Widgets/eventISMovieInfo.h>
#include <QBMenu/QBMenu.h>
#include <Menus/menuchoice.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <TraxisWebClient/TraxisWebVoDProvider.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGManager.h>

struct  QBMovieInfo_t {
    SvWidget        owner;
    SvWidget        content;

    struct {
        SvWidget purchaseInfo;
        SvWidget genre;
        SvWidget country;
        SvWidget director;
        SvWidget cast;
        SvWidget runtime;
        SvWidget ratings;
    } icons;

    SvWidget        purchaseInfo;
    SvWidget        director;
    SvWidget        runtime;
    SvWidget        genre;
    SvWidget        country;
    SvWidget        cast;
    SvWidget        ratings;

    unsigned int settingsCtx;

    SvApplication   app;

    unsigned int    ctxID;
    QBTextRenderer  textRenderer;
    const char     *dateFormat;

    SvGenericObject object;
};
typedef struct QBMovieInfo_t *QBMovieInfo;

SvLocal void
QBMovieInfoCreateSettingsLabel(QBMovieInfo self, SvWidget *label, const char* name, const char *value)
{
    if (unlikely(!label)) {
        return;
    }
    if (*label)
        return;

    char tmp[128];
    snprintf(tmp, 128, "%s.%s", name, value);

    if (!svSettingsIsWidgetDefined(tmp))
        return;

    *label = QBAsyncLabelNew(self->app, tmp, self->textRenderer);
    if (*label)
        svSettingsWidgetAttach(self->owner, *label, tmp, 10);
}

SvLocal void
QBMovieInfoCreateSettingsIcon(QBMovieInfo self, SvWidget *icon, const char* name, const char *value)
{
    if (!icon || *icon)
        return;

    char tmp[128];
    snprintf(tmp, 128, "%s.%s", name, value);
    if (svSettingsIsWidgetDefined(tmp)) {
        *icon = svSettingsWidgetCreate(self->app, tmp);
        svSettingsWidgetAttach(self->owner, *icon, tmp, 10);
    }
}

SvLocal void
QBMovieInfoCreateSettingsLabelAndIcon(QBMovieInfo self, SvWidget *label, SvWidget *icon, const char* name, const char *value)
{
    QBMovieInfoCreateSettingsLabel(self, label, name, value);
    char tmp[128];
    snprintf(tmp, 128, "%sIcon", value);
    QBMovieInfoCreateSettingsIcon(self, icon, name, tmp);
}

SvLocal void
QBMovieInfoUpdateSettingsIcon(QBMovieInfo self, SvWidget *icon, const char *name, const char *value)
{
    if (*icon) {
        svWidgetDestroy(*icon);
        *icon = NULL;
    }
    QBMovieInfoCreateSettingsIcon(self, icon, name, value);
}

SvLocal void
QBMovieInfoCleanup( SvApplication app, void* ptr )
{
    QBMovieInfo prv = (QBMovieInfo) ptr;
    SVTESTRELEASE(prv->object);
    free(ptr);
}

SvLocal SvWidget
QBMovieInfoCreate(SvApplication app, const char *widgetName,
                  QBTextRenderer renderer)
{
    QBMovieInfo prv = calloc(1, sizeof(struct QBMovieInfo_t));

    prv->settingsCtx = svSettingsSaveContext();
    prv->app                = app;
    prv->textRenderer       = renderer;

    SvWidget w              = svSettingsWidgetCreate(app, widgetName);
    w->prv                  = prv;
    prv->owner              = w;
    w->clean                = QBMovieInfoCleanup;
    svWidgetSetFocusable(w, true);

    prv->ctxID              = svSettingsSaveContext();

    char tmp[128];
    snprintf(tmp, 128, "%s.%s", svWidgetGetName(w), "Price");
    prv->dateFormat = svSettingsGetString(tmp, "expirationDateFormat");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->purchaseInfo, &prv->icons.purchaseInfo, svWidgetGetName(w), "Price");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->ratings, &prv->icons.ratings, svWidgetGetName(w), "Ratings");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->runtime, &prv->icons.runtime, svWidgetGetName(w), "Runtime");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->genre, &prv->icons.genre, svWidgetGetName(w), "Genre");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->country, &prv->icons.country, svWidgetGetName(w), "Country");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->cast, &prv->icons.cast, svWidgetGetName(w), "Cast");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->director, &prv->icons.director, svWidgetGetName(w), "Director");

    return w;
}

SvLocal SvString
create_human_readable_string_from_rental_period(int minutes)
{
    SvString niceString = NULL;
    if (minutes > 0) {
        if (minutes < 60) {
            niceString = SvStringCreateWithFormat(gettext("(%dmin)"), minutes);

        } else if (minutes < 1440) { // 24h
            int minutesLeft = minutes % 60;
            if (minutesLeft) {
                niceString = SvStringCreateWithFormat(gettext("(%dh %dmin)"), minutes / 60, minutesLeft);
            } else {
                int hours = minutes / 60;
                niceString = SvStringCreateWithFormat(ngettext("(%d hour)", "(%d hours)", hours), hours);
            }

        } else { // days
            int days = minutes / 1440;
            int minutesLeft = minutes % 1440;
            int hoursLeft = minutesLeft / 60;
            if (hoursLeft) {
                niceString = SvStringCreateWithFormat(ngettext("(%d day %dh)", "(%d days %dh)", days), days, hoursLeft);
            } else {
                niceString = SvStringCreateWithFormat(ngettext("(%d day)", "(%d days)", days), days);
            }
        }
    }

    return niceString;
}

SvLocal SvString QBMovieInfoParseRating(const char* sourceString) {

    if (!sourceString || sourceString[0] == '\0')
        return SVSTRING("");

    char* newString = calloc(strlen(sourceString)-1, sizeof(char));

    unsigned int newStringIndex = 0;
    for (unsigned int i=1; i < strlen(sourceString)-1; i++) {
        if (sourceString[i] == ']' && sourceString[i+1] == '[') {
            i++;
            newString[newStringIndex++] = ',';
            newString[newStringIndex] = ' ';
        } else if (sourceString[i] == '=') {
            newString[newStringIndex] = ' ';
        } else {
            newString[newStringIndex] = sourceString[i];
        }
        newStringIndex++;
    }
    newString[newStringIndex] = '\0';

    SvString ret = SvStringCreate(newString, NULL);
    free(newString);

    return ret;
}

SvLocal void
QBMovieInfoFillProductWidgetInnov8on(QBMovieInfo self, SvDBRawObject metaData)
{
    if (self->purchaseInfo) {
        SvString expirationDateStr = QBMovieUtilsGetAttr(metaData, "expires_at", NULL);
        if (expirationDateStr && SvStringGetLength(expirationDateStr) > 0) {
            QBMovieInfoUpdateSettingsIcon(self, &self->icons.purchaseInfo, svWidgetGetName(self->owner), "PriceIcon");
            SvTime expirationDate;
            if (QBStringToDateTime(SvStringCString(expirationDateStr), QBDateTimeFormat_ISO8601, false, &expirationDate) == (ssize_t) SvStringGetLength(expirationDateStr)) {
                struct tm localTime;
                if (SvTimeBreakDown(expirationDate, true, &localTime) < 0) {
                      //Ups, we are out of t_time type range -> sizeof(time_t) == 4
                      const char *endOfTheTime = "2038-01-01T00:00:00Z";
                      QBStringToDateTime(endOfTheTime, QBDateTimeFormat_ISO8601, false, &expirationDate);
                      SvTimeBreakDown(expirationDate, true, &localTime);
                }
                char buff[128];
                strftime(buff, sizeof(buff), self->dateFormat, &localTime);
                QBAsyncLabelSetCText(self->purchaseInfo, buff);
            }
        } else {
            SvArray offers = (SvArray) SvDBRawObjectGetAttrValue(metaData, "offers");
            if (offers && SvObjectIsInstanceOf((SvObject) offers, SvArray_getType()) && SvArrayCount(offers) == 1) {
                SvDBRawObject offer = (SvDBRawObject) SvDBReferenceGetReferredObject((SvDBReference)SvArrayObjectAtIndex(offers, 0));
                if (offer) {
                    SvValue val = (SvValue) SvDBRawObjectGetAttrValue(offer, "promotional");
                    bool isPromotion = false;
                    if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsInteger(val))
                        isPromotion = SvValueGetInteger(val);

                    if (isPromotion)
                        QBMovieInfoUpdateSettingsIcon(self, &self->icons.purchaseInfo, svWidgetGetName(self->owner), "PricePromoIcon");
                    else
                        QBMovieInfoUpdateSettingsIcon(self, &self->icons.purchaseInfo, svWidgetGetName(self->owner), "PriceIcon");

                    SvValue currencyV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) offer, "currency");
                    SvString currency = NULL;
                    if (likely(currencyV && SvObjectIsInstanceOf((SvObject) currencyV, SvObject_getType()) && SvValueIsString(currencyV))) {
                        currency = SvValueGetString(currencyV);
                    } else {
                        currency = SVSTRING("");
                    }
                    SvValue offerPriceV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject)offer, "price");
                    if (offerPriceV) {
                        int offerPrice = SvValueGetInteger(offerPriceV);
                        SvString rentalPerdiodStr = NULL;
                        SvValue rentalPeriodV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject)offer, "rental_period");
                        if (rentalPeriodV) {
                            int rentalPeriod = SvValueGetInteger(rentalPeriodV);
                            rentalPerdiodStr = create_human_readable_string_from_rental_period(rentalPeriod);
                        }
                        SvString labelStr = SvStringCreateWithFormat("%i.%02i %s %s", offerPrice/100, offerPrice%100, SvStringCString(currency), rentalPerdiodStr ? SvStringCString(rentalPerdiodStr) : "");
                        QBAsyncLabelSetCText(self->purchaseInfo, SvStringCString(labelStr));
                        SVTESTRELEASE(rentalPerdiodStr);
                        SVRELEASE(labelStr);
                    }
                }
            }
        }
    }

    if (self->ratings) {
        SvString ratings = QBMovieUtilsGetAttr(metaData, "ratings", NULL);
        if (ratings) {
            SvString parsedRating = QBMovieInfoParseRating(SvStringCString(ratings));
            QBAsyncLabelSetText(self->ratings, parsedRating);
            SVRELEASE(parsedRating);
        }
    }

    if (self->director) {
        SvString director  = QBMovieUtilsGetAttr(metaData, "director", NULL);
        if (director)
            QBAsyncLabelSetText(self->director, director);
    }
    if (self->genre) {
        SvString genres = QBMovieUtilsGetAttr(metaData, "genres", NULL);
        if (genres)
            QBAsyncLabelSetText(self->genre, genres);
    }
    if (self->country) {
        SvString country = QBMovieUtilsGetAttr(metaData, "country", NULL);
        if (country)
            QBAsyncLabelSetText(self->country, country);
    }
    if (self->cast) {
        SvString cast = QBMovieUtilsCreateStrWithMaxLinesFromAttrArray(metaData, "cast_array", 3);
        if (cast) {
            QBAsyncLabelSetText(self->cast, cast);
            SVRELEASE(cast);
        } else {
            cast = QBMovieUtilsGetAttr(metaData, "cast", NULL);
            if (cast)
                QBAsyncLabelSetText(self->cast, cast);
        }
    }

    if (self->runtime) {
        SvString runtime = QBMovieUtilsCreateStrFromAttrInteger(metaData, "runtime", "%i min", NULL);
        QBAsyncLabelSetText(self->runtime, runtime);
        SVRELEASE(runtime);
    }
}

SvLocal void
QBMovieInfoFillProductWidgetTraxis(QBMovieInfo self, SvDBRawObject metaData)
{
    if (self->purchaseInfo) {
        bool containsProducts = QBEventISMovieParseElementAsBoolean(metaData, SVSTRING("ContainsProducts"), false);
        bool isEntitled = QBEventISMovieParseElementAsBoolean(metaData, SVSTRING("IsFeatureEntitled"), false);

        QBMovieInfoUpdateSettingsIcon(self, &self->icons.purchaseInfo, svWidgetGetName(self->owner), "PriceIcon");
        if (isEntitled) {
            SvString entitled = QBEventISMovieInfoCreateEntitledPriceString(metaData);
            if (!entitled)
                entitled = SvStringCreate(gettext("Entitled"), NULL);
            QBAsyncLabelSetText(self->purchaseInfo, entitled);
            SVRELEASE(entitled);
        } else if (!containsProducts) {
            SvString infoString = SvStringCreate(gettext("Not Available"), NULL);
            QBAsyncLabelSetText(self->purchaseInfo, infoString);
            SVRELEASE(infoString);
        } else {
            SvString priceStr = QBEventISMovieInfoCreateNotEntitledPriceString(metaData);
            QBAsyncLabelSetText(self->purchaseInfo, priceStr);
            SVTESTRELEASE(priceStr);
        }
    }

    if (self->ratings) {
        SvString parsedRating = QBEventISMovieInfoCreatePCRatingString(metaData);
        if (parsedRating) {
            QBAsyncLabelSetText(self->ratings, parsedRating);
            SVRELEASE(parsedRating);
        }
    }
    if (self->director) {
        SvString director = QBEventISMovieParseElementList(metaData, SVSTRING("Directors"), SVSTRING("Director"), NULL, 1, false);
        if (director) {
            QBAsyncLabelSetText(self->director, director);
            SVRELEASE(director);
        }
    }
    if (self->genre) {
        SvString genres = QBEventISMovieParseElementList(metaData, SVSTRING("AllGenres"), SVSTRING("AllGenre"), NULL, 1, false);
        if (genres) {
            QBAsyncLabelSetText(self->genre, genres);
            SVRELEASE(genres);
        }
    }
    if (self->country) {
        SvString country = NULL;
        SvString countryStr = QBEventISMovieParseElement(metaData, SVSTRING("ProductionLocation"), NULL);
        SvString dateStr = QBEventISMovieParseElement(metaData, SVSTRING("ProductionDate"), NULL);
        if (countryStr && dateStr) {
            country = SvStringCreateWithFormat("%s, %s", SvStringCString(countryStr), SvStringCString(dateStr));
        } else if (countryStr) {
            country = SVRETAIN(countryStr);
        } else if (dateStr) {
            country = SVRETAIN(dateStr);
        }
        QBAsyncLabelSetText(self->country, country);
        SVTESTRELEASE(countryStr);
        SVTESTRELEASE(dateStr);
        SVTESTRELEASE(country);
    }
    if (self->cast) {
        SvString cast = QBEventISMovieParseElementList(metaData, SVSTRING("Actors"), SVSTRING("Actor"), NULL, 3, true);
        if (cast) {
            QBAsyncLabelSetText(self->cast, cast);
            SVRELEASE(cast);
        }
    }
    if (self->runtime) {
        SvString runtime = QBMovieUtilsGetAttr(metaData, "DisplayDuration", NULL);
            QBAsyncLabelSetText(self->runtime, runtime);
    }
}

SvLocal void
QBMovieInfoClearWidgets(QBMovieInfo self)
{
    if (self->purchaseInfo)
        QBAsyncLabelSetText(self->purchaseInfo, NULL);
    if (self->ratings)
        QBAsyncLabelSetText(self->ratings, NULL);
    if (self->runtime)
        QBAsyncLabelSetText(self->runtime, NULL);
    if (self->genre)
        QBAsyncLabelSetText(self->genre, NULL);
    if (self->country)
        QBAsyncLabelSetText(self->country, NULL);
    if (self->cast)
        QBAsyncLabelSetText(self->cast, NULL);
    if (self->director)
        QBAsyncLabelSetText(self->director, NULL);
    if (self->icons.purchaseInfo)
        QBMovieInfoUpdateSettingsIcon(self, &self->icons.purchaseInfo, svWidgetGetName(self->owner), "Price");
}

struct QBVoDCarouselContext_ {
    struct QBWindowContext_t super_;

    AppGlobals appGlobals;
    SvGenericObject path;
    SvGenericObject provider;
    SvGenericObject dataSource;
    SvString serviceId;
    SvString serviceName;
    QBContentCategory category;
    QBContentCategory parentCategory;
    SvString categoryType;
    bool pcProtectedCategory;
    int currentPosition;
    bool invalidateAuthentication;  //indicates if while destroying context authentication should by invalidated
};

struct QBVoDCarouselInfo_ {
    struct SvObject_ super_;

    QBVoDCarouselContext ctx;

    SvWidget title;
    SvWidget movieTitle;
    SvWidget description;
    SvWidget infoIcons;
    SvWidget movieInfo;
    SvWidget carousel;
    SvWidget clock;
    SvWidget trailerPreview;

    QBVoDMenu vodMenu;
    SvWidget noResultsPopup;
    SvWidget dialog;

    SvTimerId timer;

    SvDBRawObject activeProduct;

    double setObjectDelay;
    bool widgetsVisibility;
};
typedef struct QBVoDCarouselInfo_ *QBVoDCarouselInfo;

struct QBVoDCarouselSlot_t {
    QBAccessManager accessMgr;
    QBParentalControlLogic pcLogic;
    SvWidget box;
    SvWidget icon;
    unsigned int settingsContext;
    char *widgetName;
    SvGenericObject object;
};
typedef struct QBVoDCarouselSlot_t *QBVoDCarouselSlot;

SvLocal SvWidget
QBVoDCarouselCreatePopup(AppGlobals appGlobals, const char *title,
                             const char *message);

SvLocal bool
QBVoDCarouselInfoIsTraxis(QBVoDCarouselInfo self)
{
    return SvObjectIsInstanceOf(self->ctx->provider, TraxisWebVoDProvider_getType());
}

SvLocal void
QBMovieInfoFillProductWidget(QBVoDCarouselInfo self, SvDBRawObject metaData)
{
    SvWidget w = self->movieInfo;
    QBMovieInfo prv = (QBMovieInfo) w->prv;

    if (QBVoDCarouselInfoIsTraxis(self))
        QBMovieInfoFillProductWidgetTraxis(prv, metaData);
    else
        QBMovieInfoFillProductWidgetInnov8on(prv, metaData);
}

SvLocal void
QBMovieInfoSetObject(QBVoDCarouselInfo self, SvObject object)
{
    SvWidget w = self->movieInfo;
    QBMovieInfo prv = (QBMovieInfo) w->prv;

    SVTESTRETAIN(object);
    SVTESTRELEASE(prv->object);
    prv->object = object;

    svSettingsRestoreContext(prv->settingsCtx);

    if (!object) {
        QBMovieInfoClearWidgets(prv);
        goto fini;
    } else {
        if (SvObjectIsInstanceOf((SvObject) object, SvDBRawObject_getType())) {
            QBMovieInfoFillProductWidget(self, (SvDBRawObject) object);
        }
    }
fini:
    svSettingsPopComponent();
}

SvLocal void
QBVoDCarouselSetWidgetsVisibility(QBVoDCarouselInfo self, bool hide)
{
    if (self->widgetsVisibility == hide)
        return;

    self->widgetsVisibility = hide;

    if (self->title) {
        svWidgetSetHidden(self->title, hide);
    }
    if (self->movieTitle) {
        svWidgetSetHidden(self->movieTitle, hide);
    }
    if (self->description) {
        svWidgetSetHidden(self->description, hide);
    }
    if (self->infoIcons) {
        svWidgetSetHidden(self->infoIcons, hide);
    }
    if (self->carousel) {
        svWidgetSetHidden(self->carousel, hide);
    }
    if (self->trailerPreview) {
        svWidgetSetHidden(self->trailerPreview, hide);
    }
    if (self->movieInfo) {
        svWidgetSetHidden(self->movieInfo, hide);
    }
}

SvLocal void
QBVoDCarouselSearchNoResultDialogCallback(void *self_, SvWidget dialog, SvString buttonTag,
                           unsigned keyCode)
{
    QBVoDCarouselInfo self = (QBVoDCarouselInfo) self_;
    self->noResultsPopup = NULL;
    if(buttonTag && SvStringEqualToCString(buttonTag, "button")) {
        QBApplicationControllerPopContext(self->ctx->appGlobals->controller);
    }
}

SvLocal void
QBVoDCarouselSetupNoResultDialog(QBVoDCarouselInfo self)
{
    if (self->noResultsPopup)
        return;

    if (self->dialog)
        QBDialogBreak(self->dialog);

    self->noResultsPopup = QBVoDCarouselCreatePopup(self->ctx->appGlobals, gettext("Information"), gettext("No results found"));
    QBDialogRun(self->noResultsPopup, self, QBVoDCarouselSearchNoResultDialogCallback);
}

SvLocal bool
QBVoDCarouselShouldBeBlocked(QBAccessManager accessMgr, QBParentalControlLogic pcLogic, SvObject product)
{
    if (QBAccessManagerGetAuthenticationStatus(accessMgr, SVSTRING("PC_VOD"), NULL) == QBAuthStatus_OK)
        return false;

    if (!QBParentalControlLogicAdultIsBlocked(pcLogic))
        return false;

    return QBVoDUtilsIsAdult((SvObject) product);
}

SvLocal SvWidget
QBVoDCarouselCreatePopup(AppGlobals appGlobals, const char *title,
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
QBVoDCarouselFillInfo(QBVoDCarouselInfo self)
{
    SvValue val = NULL;

    if (!QBVoDCarouselShouldBeBlocked(self->ctx->appGlobals->accessMgr, self->ctx->appGlobals->parentalControlLogic, (SvObject) self->activeProduct)) {
        val = (SvValue) SvDBRawObjectGetAttrValue(self->activeProduct, "name");
        if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsString(val)) {
            QBAsyncLabelSetText(self->movieTitle, SvValueGetString(val));
            QBTrailerPreviewSetTitle(self->trailerPreview, SvValueGetString(val));
        }

        val = (SvValue) SvDBRawObjectGetAttrValue(self->activeProduct, "trailer");
        if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsString(val)) {
            QBTrailerPreviewSetMedia(self->trailerPreview, SvValueGetString(val));
        }

        val = (SvValue) SvDBRawObjectGetAttrValue(self->activeProduct, "description");
        if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsString(val)) {
            QBAsyncLabelSetText(self->description, SvValueGetString(val));
        }

        val = (SvValue) SvDBRawObjectGetAttrValue(self->activeProduct, "high_definition");
        if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsInteger(val)) {
            QBInfoIconsSetState(self->infoIcons, QBInfoIcons_HD, SvValueGetInteger(val));
        }
        val = (SvValue) SvDBRawObjectGetAttrValue(self->activeProduct, "ratings");
        if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsString(val)) {
            SvArray activeProductRatings = QBPCUtilsCreateRatingsFromString(SvValueGetString(val));
            QBPCRating lowestRating = (QBPCRating) SvArrayAt(activeProductRatings, 0);
            QBInfoIconsSetByRating(self->infoIcons, lowestRating);
            SVTESTRELEASE(activeProductRatings);
        }
        QBInfoIconsSetByMovie(self->infoIcons, self->activeProduct);

        svWidgetSetHidden(self->movieInfo, false);
        QBMovieInfoSetObject(self, (SvObject) self->activeProduct);
    } else {
        QBAsyncLabelSetText(self->movieTitle, QBParentalControlGetBlockedMovieTitle(self->ctx->appGlobals->pc));
        QBAsyncLabelSetText(self->description, QBParentalControlGetBlockedMovieDescription(self->ctx->appGlobals->pc));
        QBInfoIconsClear(self->infoIcons);
        QBMovieInfoSetObject(self, NULL);
        svWidgetSetHidden(self->movieInfo, true);
    }
}

SvLocal void
QBVoDCarouselCheckParentalControlPINCallbackShowSideMenu(void *ptr, SvWidget dlg,
                                                             SvString ret, unsigned key)
{
    QBVoDCarouselInfo self = ptr;

    if (!ret || !SvStringEqualToCString(ret, "OK-button"))
        return;

    QBVoDCarouselFillInfo(self);
    QBVerticalCarouselUpdateItems(self->carousel);
}

SvLocal void
QBVoDCarouselCheckParentalControlPIN(QBVoDCarouselInfo self)
{
    svSettingsPushComponent("ParentalControl.settings");
    AppGlobals appGlobals = self->ctx->appGlobals;
    SvGenericObject authenticator = NULL;
    authenticator = QBAuthenticateViaAccessManager(appGlobals->scheduler,
                                                   appGlobals->accessMgr,
                                                   SVSTRING("PC_VOD"));
    SvWidget master = NULL;
    SvWidget dialog = QBAuthDialogCreate(appGlobals, authenticator,
                                         gettext("Authentication required"),
                                         0, true, NULL, &master);

    QBDialogRun(dialog, self, QBVoDCarouselCheckParentalControlPINCallbackShowSideMenu);
    svSettingsPopComponent();
}

SvLocal void
QBVoDCarouselProductChoosen(QBVoDCarouselInfo self, SvDBRawObject movie)
{
    if (QBVoDCarouselInfoIsTraxis(self)) {
        SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->ctx->appGlobals->main, getMenu);
        SvObject handler = SvInvokeInterface(QBMenu, menuBar->prv, getEventHandlerForPath, self->ctx->path, NULL);
        if (SvObjectIsImplementationOf(handler, QBMenuChoice_getInterface()))
            SvInvokeInterface(QBMenuChoice, handler, choosen, (SvObject) movie, self->ctx->path, self->ctx->currentPosition);
    } else {
        SvValue typeV = (SvValue) SvDBRawObjectGetAttrValue(movie, "type");
        SvString type = SvValueTryGetString(typeV);
        if (type && (SvStringEqualToCString(type, "vod") || SvStringEqualToCString(type, "catchup_event"))) {
            if (QBVoDCarouselShouldBeBlocked(self->ctx->appGlobals->accessMgr, self->ctx->appGlobals->parentalControlLogic, (SvObject) self->activeProduct)) {
                QBVoDCarouselCheckParentalControlPIN(self);
                return;
            }
            QBVoDMenuStart(self->vodMenu, movie, self->ctx->path);
        }
    }
}

SvLocal void
QBVoDCarouselCategoryStateChanged(SvGenericObject self_, QBContentCategory category, QBContentCategoryLoadingState previousState, QBContentCategoryLoadingState currentState)
{
    QBVoDCarouselInfo self = (QBVoDCarouselInfo) self_;

    if (currentState == QBContentCategoryLoadingState_idle) { // information from middleware was received
        if (self->dialog) {
            QBDialogBreak(self->dialog);
            self->dialog = NULL;
        }
        if ((QBContentCategoryGetLength(category) - QBContentCategoryGetStaticCount(category)) == 0) {
            QBVoDCarouselSetupNoResultDialog(self);
        } else {
            QBVoDCarouselSetWidgetsVisibility(self, false);
        }
    }
}

SvLocal void
QBVoDCarouselClearAll(QBVoDCarouselInfo self)
{
    QBAsyncLabelSetText(self->movieTitle, NULL);
    QBAsyncLabelSetText(self->description, NULL);
    QBMovieInfoSetObject(self, NULL);
    svWidgetSetHidden(self->movieInfo, true);
    QBTrailerPreviewSetTitle(self->trailerPreview, NULL);
    QBTrailerPreviewSetMedia(self->trailerPreview, NULL);
    QBInfoIconsClear(self->infoIcons);
}

SvLocal void
QBVoDCarouselStartTimer(SvWidget w)
{
    QBVoDCarouselInfo prv = w->prv;

    if (prv->timer) {
        svAppTimerStop(w->app, prv->timer);
        prv->timer = 0;
    }

    prv->timer = svAppTimerStart(w->app, w, prv->setObjectDelay, 1);
}

SvLocal void
QBVoDCarouselSlotClean(SvApplication app, void *self_)
{
    QBVoDCarouselSlot self = self_;
    SVTESTRELEASE(self->object);
    SVRELEASE(self->accessMgr);
    SVRELEASE(self->pcLogic);
    free(self->widgetName);
    free(self);
}

SvLocal void
QBVoDCarouselSlotSetObject(SvWidget w, SvGenericObject object)
{
    QBVoDCarouselSlot self = w->prv;
    if (!SvObjectEquals(self->object, object) || !self->icon) {
        if (self->icon)
            svWidgetDestroy(self->icon);
        SVTESTRELEASE(self->object);
        self->object = SVTESTRETAIN(self->object);
        svSettingsRestoreContext(self->settingsContext);
        self->icon = svIconNew(w->app, self->widgetName);
        svSettingsPopComponent();
        svWidgetAttach(self->box, self->icon, 0, 0, 1);
        self->box->width = self->icon->width;
        self->box->height = self->icon->height;
    }

    if (SvObjectIsInstanceOf(object, SvDBRawObject_getType())) {
        const char *url = NULL;

        SvValue val = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) object, "thumbnail"); //innov8on
        if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsString(val)) {
            url = SvValueGetStringAsCString(val, NULL);
        }
        if (!url) {
            SvString str = QBEventISMovieGetThumbnailURI((SvDBRawObject) object); //traxis
            if (str)
                url = SvStringGetCString(str);
        }

        if (url) {
            svIconSetBitmapFromURI(self->icon, 1, url);
            if (QBVoDCarouselShouldBeBlocked(self->accessMgr, self->pcLogic, object)) {
                svSettingsRestoreContext(self->settingsContext);
                svIconSetBitmap(self->icon, 2, svSettingsGetBitmap(self->widgetName, "bgLocked"));
                svSettingsPopComponent();
                svIconSwitch(self->icon, 2, 0, 0.0);
                return;
            }
            if (svIconIsBitmapAvailable(self->icon, 1)) {
                svIconSwitch(self->icon, 1, 0, 0.0);
            } else {
                svIconSwitch(self->icon, 1, 0, -1.0);
            }
            return;
        }
    }

    svIconSwitch(self->icon, 0, 0, 0.0);
}

SvLocal SvWidget
QBVoDCarouselSlotCreate(QBAccessManager accessMgr, QBParentalControlLogic pcLogic, SvApplication app, const char *widgetName)
{
    QBVoDCarouselSlot self = calloc(1, sizeof(struct QBVoDCarouselSlot_t));
    self->accessMgr = SVRETAIN(accessMgr);
    self->pcLogic = SVRETAIN(pcLogic);
    self->box = svWidgetCreateBitmap(app, 0, 0, NULL);
    self->box->prv = self;
    self->box->clean = QBVoDCarouselSlotClean;
    self->settingsContext = svSettingsSaveContext();
    self->widgetName = strdup(widgetName);

    QBVoDCarouselSlotSetObject(self->box, NULL);

    return self->box;
}


SvLocal void
QBVoDCarouselUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e)
{
    QBVoDCarouselInfo prv = w->prv;

    if (e->code == SV_EVENT_QB_CAROUSEL_NOTIFICATION) {
        QBVerticalCarouselNotification vcn = e->prv;
        if (vcn->type == QB_CAROUSEL_NOTIFICATION_ACTIVE_ELEMENT) {
            prv->ctx->currentPosition = vcn->dataIndex;
            QBVoDCarouselStartTimer(w);

            if (vcn->dataObject) {
                QBVoDCarouselSetWidgetsVisibility(prv, false);
                if (SvObjectIsInstanceOf(vcn->dataObject, SvDBRawObject_getType())) {
                    if (prv->dialog) {
                        QBDialogBreak(prv->dialog);
                        prv->dialog = NULL;
                    }
                    SVRETAIN(vcn->dataObject);
                    SVTESTRELEASE(prv->activeProduct);
                    prv->activeProduct = (SvDBRawObject) vcn->dataObject;
                } else {
                    SVTESTRELEASE(prv->activeProduct);
                    prv->activeProduct = NULL;
                }
            }
            QBVoDCarouselClearAll(prv);
            return;
        } else if (vcn->type == QB_CAROUSEL_NOTIFICATION_SELECTED_ELEMENT) {
            prv->ctx->currentPosition = vcn->dataIndex;
            if (vcn->dataObject && SvObjectIsInstanceOf(vcn->dataObject, SvDBRawObject_getType())) {
                QBVoDCarouselProductChoosen(prv, (SvDBRawObject) vcn->dataObject);
                return;
            }
        }
    }
}

SvLocal void
QBVoDCarouselTimerEventHandler(SvWidget w, SvTimerEvent e)
{
    QBVoDCarouselInfo prv = w->prv;

    if (e->id == prv->timer) {
        prv->timer = 0;
        if (!QBVerticalCarouselIsScrolling(prv->carousel)) {
            if (prv->activeProduct)
                QBVoDCarouselFillInfo(prv);
        } else {
            QBVoDCarouselStartTimer(w);
        }

        return;
    }
}

SvLocal void
QBVoDCarouselClean(SvApplication app, void *prv_)
{
    QBVoDCarouselInfo self = prv_;
    SVRELEASE(self);
}

SvLocal void
QBVoDCarouselItemsNoop(SvGenericObject self_, size_t idx, size_t count)
{
}

SvLocal void
QBVoDCarouselItemsChanged(SvGenericObject self_, size_t idx, size_t count)
{
    QBVoDCarouselInfo self = (QBVoDCarouselInfo) self_;

    if (self->ctx->currentPosition < 0)
        return;

    size_t pos = self->ctx->currentPosition;
    if (pos < idx || pos >= idx + count)
        return;

    SVTESTRELEASE(self->activeProduct);
    self->activeProduct = NULL;
    SvGenericObject newProduct = SvInvokeInterface(QBListModel, self->ctx->dataSource, getObject, pos);
    if (!SvObjectIsInstanceOf(newProduct, SvDBRawObject_getType()))
        return;
    self->activeProduct = SVRETAIN(newProduct);

    if (self->timer)
        return;

    QBVoDCarouselFillInfo(self);
}

SvLocal void
QBVoDCarouselInfo__dtor__(void *self_)
{
    QBVoDCarouselInfo self = self_;

    SVTESTRELEASE(self->vodMenu);
    if (self->noResultsPopup)
        QBDialogBreak(self->noResultsPopup);
    if (self->dialog)
        QBDialogBreak(self->dialog);

    SVTESTRELEASE(self->activeProduct);
    if (self->ctx->invalidateAuthentication)
        QBAccessManagerInvalidateAuthentication(self->ctx->appGlobals->accessMgr, SVSTRING("PC_VOD"), NULL);
}

SvLocal void
QBVoDCarouselInfoServiceNotificationNoop(SvObject self_, SvString serviceId)
{
}

SvLocal void
QBVoDCarouselInfoServiceRemoved(SvObject self_, SvString serviceId)
{
    QBVoDCarouselInfo self = (QBVoDCarouselInfo) self_;
    if (SvStringEqualToCString(self->ctx->serviceId, SvStringCString(serviceId))) {
        QBApplicationControllerSwitchToRoot(self->ctx->appGlobals->controller);
        QBApplicationControllerPushContext(self->ctx->appGlobals->controller, self->ctx->appGlobals->main);
    }
}

SvLocal SvType
QBVoDCarouselInfo_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDCarouselInfo__dtor__
    };
    static SvType type = NULL;

    static const struct QBContentCategoryListener_ categoryListenerMethods = {
        .loadingStateChanged = QBVoDCarouselCategoryStateChanged,
    };

    static const struct QBProvidersControllerServiceListener_t providersControllerServiceMethods = {
        .serviceAdded    = QBVoDCarouselInfoServiceNotificationNoop,
        .serviceRemoved  = QBVoDCarouselInfoServiceRemoved,
        .serviceModified = QBVoDCarouselInfoServiceNotificationNoop
    };

    static const struct QBListModelListener_t listModelMethods = {
        .itemsAdded = QBVoDCarouselItemsNoop,
        .itemsRemoved = QBVoDCarouselItemsNoop,
        .itemsReordered = QBVoDCarouselItemsNoop,
        .itemsChanged = QBVoDCarouselItemsChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDCarouselInfo",
                            sizeof(struct QBVoDCarouselInfo_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContentCategoryListener_getInterface(), &categoryListenerMethods,
                            QBListModelListener_getInterface(), &listModelMethods,
                            QBProvidersControllerServiceListener_getInterface(), &providersControllerServiceMethods,
                            NULL);
    }

    return type;
}

SvLocal void
QBVoDCarouselProgressDialogCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBVoDCarouselInfo self = self_;
    self->dialog = NULL;

    if (buttonTag && SvStringEqualToCString(buttonTag, "cancel-button")) {
        QBApplicationControllerPopContext(self->ctx->appGlobals->controller);
    }
}

SvLocal void
QBVoDCarouselSetupProgressDialog(QBVoDCarouselInfo self)
{
    AppGlobals appGlobals = self->ctx->appGlobals;
    SvApplication app = appGlobals->res;
    if (!svSettingsIsWidgetDefined("Dialog")) {
        return;
    }
    QBDialogParameters params = {
        .app        = app,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0,
    };
    SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    if (!dialog) {
        return;
    }
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    QBDialogSetTitle(dialog, gettext(svSettingsGetString("Popup", "title")));
    QBDialogAddButton(dialog, SVSTRING("cancel-button"), gettext("Cancel"), 1);

    SvWidget animation = QBWaitAnimationCreate(app, "Popup.Animation");
    if (animation) {
        QBDialogAddWidget(dialog, SVSTRING("content"), animation, SVSTRING("animation"), 3, "Dialog.Animation");
    }

    self->dialog = dialog;
    QBDialogRun(dialog, self, QBVoDCarouselProgressDialogCallback);
}


SvLocal void
QBVoDCarouselContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBVoDCarouselContext ctx = (QBVoDCarouselContext) self_;

    svSettingsPushComponent("VoDCarousel.settings");
    SvWidget window = QBGUILogicCreateBackgroundWidget(ctx->appGlobals->guiLogic, "Window", NULL);
    svWidgetSetName(window, "QBVoDCarousel");

    QBVoDCarouselInfo prv = (QBVoDCarouselInfo) SvTypeAllocateInstance(
        QBVoDCarouselInfo_getType(), NULL);
    prv->ctx = ctx;

    prv->title = QBTitleNew(app, "Title", ctx->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, prv->title, "Title", 1);
    QBTitleSetText(prv->title, ctx->serviceName, 0);
    SvString categoryName = NULL;
    if (SvObjectIsInstanceOf((SvObject) ctx->category, QBContentSeeAll_getType())) {
        if (ctx->parentCategory)
            categoryName = SvStringCreateWithFormat("%s %s", gettext("See all from"), SvStringCString(QBContentCategoryGetName(ctx->parentCategory)));
        else
            categoryName = SvStringCreate(gettext("See all"), NULL);
    } else {
        categoryName = SVRETAIN(QBContentCategoryGetName((QBContentCategory) ctx->category));
    }
    QBTitleSetText(prv->title, categoryName, 1);
    SVRELEASE(categoryName);

    prv->movieTitle = QBAsyncLabelNew(app, "MovieTitle", ctx->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, prv->movieTitle, svWidgetGetName(prv->movieTitle), 1);

    prv->description = QBAsyncLabelNew(app, "Description", ctx->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, prv->description, svWidgetGetName(prv->description), 1);

    SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                         SVSTRING("SvEPGManager"));
    prv->infoIcons = QBInfoIconsCreate(app, "InfoIcons",
                                       ctx->appGlobals->eventsLogic,
                                       epgManager,
                                       ctx->appGlobals->pc);
    svSettingsWidgetAttach(window, prv->infoIcons, "InfoIcons", 1);

    prv->movieInfo = QBMovieInfoCreate(app, "MovieInfo", ctx->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, prv->movieInfo, svWidgetGetName(prv->movieInfo), 1);

    prv->carousel = QBVerticalCarouselNewFromSM(app, "Carousel");
    QBVerticalCarouselSetNotificationTarget(prv->carousel, svWidgetGetId(window));
    svSettingsWidgetAttach(window, prv->carousel, svWidgetGetName(prv->carousel), 1);

    prv->widgetsVisibility = false;
    if ((QBContentCategoryGetLength(ctx->category) - QBContentCategoryGetStaticCount(ctx->category)) == 0) {
        QBVoDCarouselSetWidgetsVisibility(prv, true);
        if (QBContentCategoryGetLoadingState(ctx->category) != QBContentCategoryLoadingState_idle) {
            QBVoDCarouselSetupProgressDialog(prv);
        } else {
            QBVoDCarouselSetupNoResultDialog(prv);
        }
    }

    QBContentCategoryAddListener(ctx->category, (SvGenericObject) prv, NULL);
    QBProvidersControllerServiceAddListener(ctx->appGlobals->providersController, (SvGenericObject) prv, QBProvidersControllerService_INNOV8ON);

    int count = QBVerticalCarouselGetSlotCount(prv->carousel);
    for (int i = 0; i < count; ++i) {
        QBVerticalCarouselSetContentAtIndex(prv->carousel, i, QBVoDCarouselSlotCreate(ctx->appGlobals->accessMgr, ctx->appGlobals->parentalControlLogic, app, "Carousel.Slot"));
        QBVerticalCarouselSetDataHandler(prv->carousel, i, QBVoDCarouselSlotSetObject);
    }
    QBVerticalCarouselSetFocusWidget(prv->carousel, QBFrameCreateFromSM(app, "Carousel.Focus"));
    QBVerticalCarouselConnectToSource(prv->carousel, ctx->dataSource, NULL);
    QBVerticalCarouselSetActive(prv->carousel, ctx->currentPosition);
    SvInvokeInterface(QBDataModel, ctx->dataSource, addListener, (SvGenericObject) prv, NULL);

    prv->clock = svClockNew(app, "Clock");
    svSettingsWidgetAttach(window, prv->clock, svWidgetGetName(prv->clock), 1);

    prv->trailerPreview = QBTrailerPreviewNew(ctx->appGlobals, "TrailerPreview");
    svSettingsWidgetAttach(window, prv->trailerPreview, "TrailerPreview", 1);

    prv->setObjectDelay = svSettingsGetDouble("Carousel", "setObjectDelay", 0.5);

    svWidgetSetFocus(prv->carousel);

    window->prv = prv;
    window->clean = QBVoDCarouselClean;
    svWidgetSetUserEventHandler(window, QBVoDCarouselUserEventHandler);
    svWidgetSetTimerEventHandler(window, QBVoDCarouselTimerEventHandler);

    svSettingsPopComponent();

    if (!QBVoDCarouselInfoIsTraxis(prv)) {
        prv->vodMenu = QBVoDMenuCreate(ctx->appGlobals, ctx->provider,
                                       ctx->serviceId, ctx->serviceName, SVSTRING("PC_VOD"));
    }

    ctx->super_.window = window;

    prv->timer = 0;
    prv->activeProduct = NULL;
}

SvLocal void
QBVoDCarouselContextDestroyWindow(QBWindowContext self_)
{
    QBVoDCarouselContext self = (QBVoDCarouselContext) self_;

    QBProvidersControllerServiceRemoveListener(self->appGlobals->providersController, self->super_.window->prv, QBProvidersControllerService_INNOV8ON);
    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal void
QBVoDCarouselContextDestroy(void *self_)
{
    QBVoDCarouselContext self = self_;
    SVRELEASE(self->path);
    SVRELEASE(self->provider);
    SVRELEASE(self->dataSource);
    SVRELEASE(self->serviceId);
    SVRELEASE(self->serviceName);
    SVTESTRELEASE(self->category);
    SVTESTRELEASE(self->parentCategory);
    SVTESTRELEASE(self->categoryType);
}

SvType
QBVoDCarouselContext_getType(void)
{
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_        = {
            .destroy   = QBVoDCarouselContextDestroy
        },
        .createWindow  = QBVoDCarouselContextCreateWindow,
        .destroyWindow = QBVoDCarouselContextDestroyWindow
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDCarouselContext",
                            sizeof(struct QBVoDCarouselContext_),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            NULL);
    }

    return type;
}

bool
QBVoDCarouselContextIsParentalControlProtected(QBVoDCarouselContext self)
{
    return self->pcProtectedCategory;
}

extern QBWindowContext
QBVoDCarouselContextCreate(AppGlobals appGlobals, SvGenericObject path,
                           SvGenericObject provider, SvString serviceId,
                           SvString serviceName, bool pcProtectedCategory,
                           bool invalidateAuthentication)
{
    QBVoDCarouselContext self = (QBVoDCarouselContext) SvTypeAllocateInstance(
                                                       QBVoDCarouselContext_getType(), NULL);

    self->appGlobals = appGlobals;
    self->path = SVRETAIN(path);

    QBListProxy listProxy = QBListProxyCreate((SvObject) appGlobals->menuTree, path, NULL);
    SvObject dataSource = (SvObject) QBListProxyNegativeFilterCreate(listProxy, QBContentSearch_getType(), NULL);
    SVRELEASE(listProxy);
    self->dataSource = dataSource;

    self->provider = SVRETAIN(provider);
    self->serviceId = SVRETAIN(serviceId);
    self->serviceName = SVRETAIN(serviceName);
    self->currentPosition = 0;
    self->categoryType = NULL;
    self->invalidateAuthentication = invalidateAuthentication;

    SvGenericObject node = SvInvokeInterface(QBTreeModel, appGlobals->menuTree, getNode, path);
    if (node && SvObjectIsInstanceOf(node, QBContentCategory_getType())) {
        self->category = SVTESTRETAIN(node);
        SvGenericObject parentPath = SvObjectCopy(path, NULL);
        SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
        SvGenericObject parentNode = SvInvokeInterface(QBTreeModel, appGlobals->menuTree, getNode, parentPath);
        if (SvObjectIsInstanceOf(parentNode, QBContentCategory_getType()))
            self->parentCategory = SVRETAIN(parentNode);
        SVRELEASE(parentPath);

        if (QBVODUtilsCategoryShouldBeBlocked(appGlobals, (QBContentCategory) node)) {
            self->pcProtectedCategory = true;
        }

        SvValue catTypeVal = (SvValue) QBContentCategoryGetAttribute((QBContentCategory) node, SVSTRING("categoryType"));

        if (catTypeVal && SvObjectIsInstanceOf((SvObject) catTypeVal, SvValue_getType()) && SvValueIsString(catTypeVal)) {
            self->categoryType = SVRETAIN(SvValueGetString(catTypeVal));
        }

        if (SvStringEqualToCString(self->serviceName, "VOD")) {
            QBVoDUtilsLogMovieCategory(QBContentCategoryGetId(self->category), QBContentCategoryGetName(self->category), QBContentCategoryGetLevel(self->category));
        }
    }

    return (QBWindowContext) self;
}
