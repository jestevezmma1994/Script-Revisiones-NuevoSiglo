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

#include "eventISMovieInfo.h"

#include <libintl.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvHashTable.h>
#include <QBStringUtils.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/icon.h>
#include <SWL/fade.h>
#include <QBResourceManager/SvRBBitmap.h>
#include <Logic/timeFormat.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <TraxisWebClient/TraxisWebPosterServer.h>
#include <Windows/mainmenu.h>
#include <Menus/menuchoice.h>
#include <QBContentManager/QBContentCategory.h>
#include <main.h>
#include <QBPCRatings/QBPCRating.h>

struct  QBEventISMovieInfo_t {
    AppGlobals appGlobals;

    SvWidget name, price, parentalRating, purchaseInfo, thumbnail;
    SvWidget producer, director;
    SvWidget runtime;
    SvWidget genre, country, cast, entitlementEnd;

    SvWidget categoryContentImage;
    SvRID categoryTmpRID;

    struct {
        SvWidget cart;
        SvWidget parentalRating;
        SvWidget genre;
        SvWidget country;
        SvWidget director;
        SvWidget cast;
        SvWidget producer;
        SvWidget runtime;
        SvWidget calendar;
    } icons;

    SvWidget moviePage, categoryPage;

    SvDBRawObject metaData;

    SvApplication app;
    SvRID empty_bmp;

    SvString labelDefaultText;

    QBEventISMovieDynamicLogoProvider dynamicLogoProvider;
};

typedef struct QBEventISMovieInfo_t *QBEventISMovieInfo;


SvLocal void
QBEventISMovieDynamicLogoProviderDestroy(void *self_)
{
    QBEventISMovieDynamicLogoProvider self = self_;
    SVTESTRELEASE(self->prefix);
    SVTESTRELEASE(self->suffix);
}

SvType
QBEventISMovieDynamicLogoProvider_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBEventISMovieDynamicLogoProviderDestroy,
    };

    if (!type) {
        SvTypeCreateManaged("QBEventISMovieDynamicLogoProvider",
                            sizeof(struct QBEventISMovieDynamicLogoProvider_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

void
QBEventISMovieInfoSetDynamicLogoProvider(SvWidget w, QBEventISMovieDynamicLogoProvider provider)
{
    QBEventISMovieInfo prv = (QBEventISMovieInfo) w->prv;
    SVTESTRELEASE(prv->dynamicLogoProvider);
    prv->dynamicLogoProvider = SVTESTRETAIN(provider);
}

SvLocal void
QBEventISMovieInfoCleanup(SvApplication app, void* ptr)
{
    QBEventISMovieInfo prv = ptr;
    SVTESTRELEASE(prv->metaData);
    SVTESTRELEASE(prv->labelDefaultText);
    SVTESTRELEASE(prv->dynamicLogoProvider);
    free(ptr);
}

#define LABEL(widget, name, defaultText) do { \
        if (svSettingsIsWidgetDefined(name)) { \
            widget = QBAsyncLabelNew(app, name, appGlobals->textRenderer); \
            QBAsyncLabelSetText(widget, defaultText); \
            svSettingsWidgetAttach(moviePage, widget, name, 10); \
        } \
    } while(0)

#define ICON(icon, setting) do { \
        if (svSettingsIsWidgetDefined(setting)) { \
            icon = svSettingsWidgetCreate(app, setting); \
            svSettingsWidgetAttach(moviePage, icon, svWidgetGetName(icon), 0); \
        } \
    } while(0)

SvLocal void
QBEventISMovieInfoCreateContents(QBEventISMovieInfo prv, AppGlobals appGlobals, SvWidget moviePage)
{
    if (unlikely(!prv || !appGlobals || !moviePage)) {
        SvLogError("%s NULL argument passed prv=%p appGlobals=%p moviePage=%p", __func__, prv, appGlobals, moviePage);
        return;
    }
    SvApplication app = appGlobals->res;


    ICON(prv->icons.cart, "MoviePage.MovieInfo.CartIcon");
    ICON(prv->icons.parentalRating, "MoviePage.MovieInfo.RatingsIcon");
    ICON(prv->icons.genre, "MoviePage.MovieInfo.GenreIcon");
    ICON(prv->icons.country, "MoviePage.MovieInfo.CountryIcon");
    ICON(prv->icons.director, "MoviePage.MovieInfo.DirectorIcon");
    ICON(prv->icons.cast, "MoviePage.MovieInfo.CastIcon");
    ICON(prv->icons.producer, "MoviePage.MovieInfo.ProducerIcon");
    ICON(prv->icons.runtime, "MoviePage.MovieInfo.RuntimeIcon");
    ICON(prv->icons.calendar, "MoviePage.MovieInfo.CalendarIcon");

    LABEL(prv->price, "MoviePage.MovieInfo.Price", prv->labelDefaultText);
    LABEL(prv->parentalRating, "MoviePage.MovieInfo.Ratings", prv->labelDefaultText);
    LABEL(prv->purchaseInfo, "MoviePage.MovieInfo.PurchaseInfo", prv->labelDefaultText);
    LABEL(prv->producer, "MoviePage.MovieInfo.Producer", prv->labelDefaultText);
    LABEL(prv->director, "MoviePage.MovieInfo.Director", prv->labelDefaultText);
    LABEL(prv->runtime, "MoviePage.MovieInfo.Runtime", prv->labelDefaultText);
    LABEL(prv->genre, "MoviePage.MovieInfo.Genre", prv->labelDefaultText);
    LABEL(prv->country, "MoviePage.MovieInfo.Country", prv->labelDefaultText);
    LABEL(prv->cast, "MoviePage.MovieInfo.Cast", prv->labelDefaultText);
    LABEL(prv->entitlementEnd, "MoviePage.MovieInfo.EntitlementEnd", prv->labelDefaultText);
}

SvWidget
QBEventISMovieInfoCreateMoviePage(AppGlobals appGlobals)
{
    SvApplication app = appGlobals->res;
    QBEventISMovieInfo prv = calloc(1, sizeof(struct QBEventISMovieInfo_t));
    if (!prv)
        return NULL;


    SvWidget moviePage = svSettingsWidgetCreate(app, "MoviePage.Background");
    moviePage->prv = prv;
    moviePage->clean = QBEventISMovieInfoCleanup;

    prv->app = app;

    QBEventISMovieInfoCreateContents(prv, appGlobals, moviePage);

    return moviePage;
}

SvWidget
QBEventISMovieInfoCreate(AppGlobals appGlobals)
{
    SvApplication app = appGlobals->res;
    QBEventISMovieInfo prv = calloc(1, sizeof(struct QBEventISMovieInfo_t));
    if (!prv)
        return NULL;

    svSettingsPushComponent("Carousel_VOD.settings");
    if (svSettingsIsWidgetDefined("MoviePage.MovieInfo.Default")) {
        const char* val = svSettingsGetString("MoviePage.MovieInfo.Default", "labelText");
        if (val)
            prv->labelDefaultText = SvStringCreate(gettext(val), NULL);
    }
    svSettingsPopComponent();
    if (!prv->labelDefaultText)
        prv->labelDefaultText = SvStringCreate(gettext("N/A"), NULL);

    SvWidget box = svSettingsWidgetCreate(app, "Carousel.Item");

    SvWidget categoryPage = svSettingsWidgetCreate(app, "MoviePage.Background");
    svSettingsWidgetAttach(box, categoryPage, svWidgetGetName(categoryPage), 1);

    prv->categoryContentImage = svIconNew(app, "MoviePage.CategoryImage");
    prv->categoryTmpRID = svSettingsGetResourceID("MoviePage.CategoryImage", "bg");
    svIconSetBitmapFromRID(prv->categoryContentImage, 0, prv->categoryTmpRID);
    svSettingsWidgetAttach(categoryPage, prv->categoryContentImage, "MoviePage.CategoryImage", 1);

    SvWidget moviePage = svSettingsWidgetCreate(app, "MoviePage.Background");
    svSettingsWidgetAttach(box, moviePage, svWidgetGetName(moviePage), 1);
    box->prv = prv;
    box->clean = QBEventISMovieInfoCleanup;

    prv->appGlobals = appGlobals;
    prv->app = app;

    QBEventISMovieInfoCreateContents(prv, appGlobals, moviePage);
    if (svSettingsIsWidgetDefined("MoviePage.Image")) {
        prv->empty_bmp = svSettingsGetResourceID("MoviePage.Image", "bg");
        prv->thumbnail = svIconNew(app, "MoviePage.Image");
        svIconSetBitmapFromRID(prv->thumbnail, 0, prv->empty_bmp);
        svSettingsWidgetAttach(moviePage, prv->thumbnail, "MoviePage.Image", 3);
    }

    svWidgetSetFocusable(box, true);
    prv->moviePage = moviePage;
    prv->categoryPage = categoryPage;

    svWidgetSetHidden(prv->categoryPage, true);
    svWidgetSetHidden(prv->moviePage, false);

    return box;
}

bool
QBEventISMovieParseElementAsBoolean(SvDBRawObject metadata, SvString tag, bool defaultValue)
{
    if (!metadata) goto returnDefault;
    SvValue element_ = (SvValue) SvDBRawObjectGetAttrValue(metadata, SvStringCString(tag));
    if (!element_) goto returnDefault;

    if (SvValueIsBoolean(element_))
        return SvValueGetBoolean(element_);

returnDefault:
    return defaultValue;
}

int
QBEventISMovieParseElementAsInteger(SvDBRawObject metadata, SvString tag, int defaultValue)
{
    if (!metadata) goto returnDefault;
    SvValue element_ = (SvValue) SvDBRawObjectGetAttrValue(metadata, SvStringCString(tag));
    if (!element_) goto returnDefault;

    if (SvValueIsInteger(element_))
        return SvValueGetInteger(element_);

returnDefault:
    return defaultValue;
}

SvString
QBEventISMovieParseElement(SvDBRawObject metadata, SvString tag, SvString defaultValue)
{
    if (!metadata) goto returnDefault;
    SvValue element_ = (SvValue) SvDBRawObjectGetAttrValue(metadata, SvStringCString(tag));
    if (!element_) goto returnDefault;
    switch (SvValueGetType(element_)) {
        case SvValueType_string: {
            SvString element = SvValueGetString(element_);
            if (!element) goto returnDefault;
            return SVRETAIN(element);
        }
        case SvValueType_integer: {
            int element = SvValueGetInteger(element_);
            return SvStringCreateWithFormat("%i", element);
        }
        default:
            break;
    }
returnDefault:
    return SVTESTRETAIN(defaultValue);
}

SvString
QBEventISMovieParseElementList(SvDBRawObject metadata, SvString tag, SvString elementTag, SvString defaultValue, const size_t maxElements, bool multiLine)
{
    if (!metadata)
        goto returnDefault;

    SvHashTable elements = (SvHashTable) SvDBRawObjectGetAttrValue(metadata, SvStringCString(tag));
    if (!elements || !SvHashTableCount(elements))
        goto returnDefault;

    SvArray elementList = (SvArray) SvHashTableFind(elements, (SvGenericObject) elementTag);
    if (!elementList)
        goto returnDefault;

    ssize_t cnt = SvArrayCount(elementList);
    if (cnt <= 0)
        goto returnDefault;

    if (cnt > (ssize_t) maxElements)
        cnt = maxElements;

    SvString ret = NULL;
    ssize_t idx;
    for (idx = 0; idx < cnt; idx++) {
        SvString tmp = ret;
        SvString element = (SvString) SvValueGetString((SvValue) SvArrayObjectAtIndex(elementList, idx));
        if (ret)
            ret = SvStringCreateWithFormat(multiLine ? "%s\n%s" : "%s, %s", SvStringCString(ret), SvStringCString(element));
        else
            ret = SVRETAIN(element);
        SVTESTRELEASE(tmp);
    }
    if (SvArrayCount(elementList) > maxElements) {
        SvString tmp = ret;
        ret = SvStringCreateWithFormat("%s…", SvStringCString(ret));
        SVTESTRELEASE(tmp);
    }
    return ret;

returnDefault:
    return SVTESTRETAIN(defaultValue);
}

SvString
QBEventISMovieGetThumbnailURI(SvDBRawObject metaData)
{
    SvHashTable pictures = (SvHashTable) SvDBRawObjectGetAttrValue(metaData, "Pictures");
    if (!pictures)
        goto fail;
    SvArray picture = (SvArray) SvHashTableFind(pictures, (SvGenericObject) SVSTRING("Picture"));
    if (!picture || !SvArrayCount(picture))
        goto fail;
    SvHashTable image = (SvHashTable) SvArrayObjectAtIndex(picture, 0);
    if (!image)
        goto fail;
    SvValue uri_ = (SvValue) SvHashTableFind(image, (SvGenericObject) SVSTRING("Value"));
    if (!uri_)
        goto fail;
    SvString uri = SvValueGetString(uri_);
    if (!uri)
        goto fail;
    return uri;

fail:
    return NULL;
}

SvLocal void
QBEventISMovieSetThumbnail(SvWidget w)
{
    QBEventISMovieInfo prv = (QBEventISMovieInfo) w->prv;

    SvString uri = QBEventISMovieGetThumbnailURI(prv->metaData);
    SvString resizedImageURI = NULL;

    if (TraxisWebSessionManagerGetUsePosterServerInterface(prv->appGlobals->traxisWebSessionManager) && uri) {
        resizedImageURI = TraxisWebPosterServerPrepareURL(uri,
                                                          prv->thumbnail->width,
                                                          prv->thumbnail->height,
                                                          TraxisWebPosterServerImageMode_Box);
        uri = resizedImageURI;
    }

    if (!uri)
        goto fail;

    svIconSetBitmapFromURI(prv->thumbnail, 1, SvStringCString(uri));
    SVTESTRELEASE(resizedImageURI);

    svIconSwitch(prv->thumbnail, 1, 0, 0.0f);
    svWidgetSetHidden(prv->thumbnail, false);
    return;

fail:
    svWidgetSetHidden(prv->thumbnail, true);
}

SvLocal SvString
QBEventISMovieInfoCreateTitle(SvDBRawObject metadata)
{
    SvString name = QBEventISMovieParseElement(metadata, SVSTRING("Name"), SVSTRING(""));
    SvString productionDate = QBEventISMovieParseElement(metadata, SVSTRING("ProductionDate"), SVSTRING(""));
    SvString title = NULL;
    if (SvStringLength(productionDate)) {
        title = SvStringCreateWithFormat("%s (%s)", SvStringCString(name), SvStringCString(productionDate));
    } else {
        title = SVRETAIN(name);
    }
    SVRELEASE(name);
    SVRELEASE(productionDate);
    return title;
}

SvLocal void
QBEventISMovieInfoSetContentProviderLogo(QBEventISMovieInfo self, SvString contentProvider)
{
    if (self->dynamicLogoProvider) {
        SvString url = SvStringCreateWithFormat("%s%s%s",
                                                SvStringCString(self->dynamicLogoProvider->prefix),
                                                SvStringCString(contentProvider),
                                                SvStringCString(self->dynamicLogoProvider->suffix));
        SvWidget logo = svIconNew(self->app, "MoviePage.DynamicLogo");
        if (svIconSetBitmapFromURI(logo, 0, SvStringCString(url)) == SV_RID_INVALID)
            SvLogError("%s: Cannot attach provider logo", __func__);
        svSettingsWidgetAttach(self->moviePage, logo, "MoviePage.DynamicLogo", 1);
        svIconSwitch(logo, 0, 0, -1.0);
        SVRELEASE(url);
    } else {
        char *widgetName = NULL;
        asprintf(&widgetName, "MoviePage.Provider.%s", SvStringCString(contentProvider));
        if (svSettingsIsWidgetDefined(widgetName)) {
            SvWidget providerLogo = svSettingsWidgetCreate(self->app, widgetName);
            svSettingsWidgetAttach(self->moviePage, providerLogo, widgetName, 1);
        }
        free(widgetName);
    }
}

SvString
QBEventISMovieInfoCreatePCRatingString(SvDBRawObject metadata)
{
    SvString ratingStr = NULL;

    SvString pcRating = QBEventISMovieParseElement(metadata, SVSTRING("PCRating"), NULL);
    if (!pcRating)
        return NULL;

    const char *str = SvStringCString(pcRating);
    if (*str == '\0')
        goto fini;

    QBPCRating rating = QBPCRatingCreateFromString(pcRating, NULL);
    int id = QBPCRatingGetRatingId(rating, NULL);
    if (id > 0)
        ratingStr = SvStringCreateWithFormat(gettext("Minimum Age %d"), id);
    SVRELEASE(rating);

fini:
    SVRELEASE(pcRating);
    return ratingStr;
}

SvString
QBEventISMovieInfoCreateNotEntitledPriceString(SvDBRawObject metadata)
{
    int price = QBEventISMovieParseElementAsInteger(metadata, SVSTRING("ListPrice"), -1);
    const char *defaultCurrencyStr = svSettingsGetString("purchaseInfo", "defaultCurrency");
    SvString defaultCurrency = defaultCurrencyStr ? SvStringCreate(svSettingsGetString("purchaseInfo", "defaultCurrency"), NULL) : NULL;
    SvString currency = QBEventISMovieParseElement(metadata, SVSTRING("Currency"), defaultCurrency ? defaultCurrency : SVSTRING(" "));
    SVTESTRELEASE(defaultCurrency);
    SvString priceStr = NULL;
    if (price >= 0) {
        price = (price + 50) / 100;
        priceStr = SvStringCreateWithFormat("%i.%02i %s", price / 100, price % 100, SvStringCString(currency));
    } else {
        priceStr = SvStringCreate("", NULL);
    }
    SVTESTRELEASE(currency);
    return priceStr;
}

SvString
QBEventISMovieInfoCreateEntitledPriceString(SvDBRawObject metadata)
{
    SvString entitlementEnd = QBEventISMovieParseElement(metadata, SVSTRING("EntitlementEnd"), SVSTRING(""));
    SvString dateStr = NULL;
    if (SvStringLength(entitlementEnd) > 0) {
        SvTime value;
        if (QBStringToDateTime(SvStringCString(entitlementEnd), QBDateTimeFormat_ISO8601, false, &value) == (ssize_t) SvStringLength(entitlementEnd)) {
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
    SVRELEASE(entitlementEnd);
    return dateStr;
}

SvLocal void
QBEventISMovieInfoUpdate(SvWidget w)
{
    QBEventISMovieInfo prv = (QBEventISMovieInfo) w->prv;

    SvDBRawObject metadata = prv->metaData;

    if (prv->producer) {
        SvString producer = QBEventISMovieParseElementList(metadata, SVSTRING("Producers"), SVSTRING("Producer"), prv->labelDefaultText, 3, false);
        if (producer) {
            QBAsyncLabelSetText(prv->producer, producer);
            SVRELEASE(producer);
        } else {
            QBAsyncLabelSetText(prv->producer, prv->labelDefaultText);
        }
    }

    SvString pcRatingStr = QBEventISMovieInfoCreatePCRatingString(metadata);
    if (pcRatingStr) {
        QBAsyncLabelSetText(prv->parentalRating, pcRatingStr);
        SVRELEASE(pcRatingStr);
    } else {
        QBAsyncLabelSetText(prv->parentalRating, prv->labelDefaultText);
    }

    SvString contentProvider = QBEventISMovieParseElement(metadata, SVSTRING("ContentProvider"), prv->labelDefaultText);
    QBEventISMovieInfoSetContentProviderLogo(prv, contentProvider);
    SVRELEASE(contentProvider);

    SvString director = QBEventISMovieParseElementList(metadata, SVSTRING("Directors"), SVSTRING("Director"), prv->labelDefaultText, 1, false);
    if (director) {
        QBAsyncLabelSetText(prv->director, director);
        SVRELEASE(director);
    } else {
        QBAsyncLabelSetText(prv->director, prv->labelDefaultText);
    }

    SvString runtime = QBEventISMovieParseElement(metadata, SVSTRING("DisplayDuration"), prv->labelDefaultText);
    if (runtime) {
        QBAsyncLabelSetText(prv->runtime, runtime);
        SVRELEASE(runtime);
    } else {
        QBAsyncLabelSetText(prv->runtime, prv->labelDefaultText);
    }

    SvString country = QBEventISMovieParseElement(metadata, SVSTRING("ProductionLocation"), NULL);
    SvString date = QBEventISMovieParseElement(metadata, SVSTRING("ProductionDate"), NULL);
    SvString countryAndDate = NULL;
    if (country && date)
        countryAndDate = SvStringCreateWithFormat("%s, %s", SvStringCString(country), SvStringCString(date));
    else if (country)
        countryAndDate = SVRETAIN(country);
    else if (date)
        countryAndDate = SVRETAIN(date);
    else
        countryAndDate = SVRETAIN(prv->labelDefaultText);

    QBAsyncLabelSetText(prv->country, countryAndDate);
    SVTESTRELEASE(country);
    SVTESTRELEASE(date);
    SVRELEASE(countryAndDate);

    int maxLines = svSettingsGetInteger("MoviePage.MovieInfo.Cast", "maxLines", 1);
    SvString cast = QBEventISMovieParseElementList(metadata, SVSTRING("Actors"), SVSTRING("Actor"), prv->labelDefaultText, maxLines, true);
    if (cast) {
        QBAsyncLabelSetText(prv->cast, cast);
        SVRELEASE(cast);
    } else {
        QBAsyncLabelSetText(prv->cast, prv->labelDefaultText);
    }

    SvString genre = QBEventISMovieParseElementList(metadata, SVSTRING("AllGenres"), SVSTRING("AllGenre"), prv->labelDefaultText, 10, false);
    if (!genre)
        genre = QBEventISMovieParseElementList(metadata, SVSTRING("Genres"), SVSTRING("Genre"), prv->labelDefaultText, 3, false);
    if (genre) {
        QBAsyncLabelSetText(prv->genre, genre);
        SVRELEASE(genre);
    } else {
        QBAsyncLabelSetText(prv->genre, prv->labelDefaultText);
    }

    bool containsProducts = QBEventISMovieParseElementAsBoolean(metadata, SVSTRING("ContainsProducts"), false);
    bool isEntitled = QBEventISMovieParseElementAsBoolean(metadata, SVSTRING("IsFeatureEntitled"), false);

    if (isEntitled) {
        SvString dateStr = QBEventISMovieInfoCreateEntitledPriceString(metadata);
        SvString priceStr = SvStringCreate(gettext("Entitled"), NULL);
        if (prv->price) {
            QBAsyncLabelSetText(prv->price, dateStr ? : priceStr);
        }
        if (prv->purchaseInfo) {
            QBAsyncLabelSetText(prv->purchaseInfo, priceStr);
        }
        if (dateStr && prv->entitlementEnd) {
            QBAsyncLabelSetText(prv->entitlementEnd, dateStr);
        }
        SVTESTRELEASE(dateStr);
        SVRELEASE(priceStr);
    } else if (!containsProducts) {
        if (prv->price) {
            SvString infoString = SvStringCreate(gettext("Not Available"), NULL);
            QBAsyncLabelSetText(prv->price, infoString);
            SVRELEASE(infoString);
        }
    } else {
        SvString priceStr = QBEventISMovieInfoCreateNotEntitledPriceString(metadata);
        if (prv->price) {
            QBAsyncLabelSetText(prv->price, priceStr);
        }
        if (prv->purchaseInfo) {
            QBAsyncLabelSetText(prv->purchaseInfo, priceStr);
        }
        SVTESTRELEASE(priceStr);
    }

    if (prv->name) {
        SvString title = QBEventISMovieInfoCreateTitle(metadata);
        if (title) {
            QBAsyncLabelSetText(prv->name, title);
            SVRELEASE(title);
        } else {
            QBAsyncLabelSetText(prv->name, prv->labelDefaultText);
        }
    }

    if (prv->thumbnail) {
        if (prv->empty_bmp) {
            svIconSetBitmapFromRID(prv->thumbnail, 1, prv->empty_bmp);
        }
        QBEventISMovieSetThumbnail(w);
    }
}

void
QBEventISMovieInfoSetObject(SvWidget w, SvGenericObject object)
{
    QBEventISMovieInfo prv = (QBEventISMovieInfo) w->prv;

    if (!object && prv->categoryPage && prv->moviePage) {
        svWidgetSetHidden(prv->categoryPage, true);
        svWidgetSetHidden(prv->moviePage, false);
        return;
    }

    svSettingsPushComponent("Carousel_VOD.settings");
    if (SvObjectIsInstanceOf((SvObject) object, SvDBRawObject_getType())) {
        SvDBObject movie = (SvDBObject) object;
        SVTESTRELEASE(prv->metaData);
        prv->metaData = SVRETAIN(movie);
        QBEventISMovieInfoUpdate(w);
        if (prv->categoryPage) svWidgetSetHidden(prv->categoryPage, true);
        if (prv->moviePage) svWidgetSetHidden(prv->moviePage, false);
    } else if (SvObjectIsInstanceOf(object, QBContentCategory_getType())) {
        if (prv->categoryPage) svWidgetSetHidden(prv->categoryPage, false);
        if (prv->moviePage) svWidgetSetHidden(prv->moviePage, true);

        svIconSetBitmapFromRID(prv->categoryContentImage, 0, prv->categoryTmpRID);

        QBContentCategory category = (QBContentCategory) object;
        SvValue uriV = (SvValue) QBContentCategoryGetAttribute(category, SVSTRING("thumbnail"));
        if (uriV && SvObjectIsInstanceOf((SvObject) uriV, SvValue_getType()) && SvValueIsString(uriV)) {
            svIconSetBitmapFromURI(prv->categoryContentImage, 0, SvStringCString(SvValueGetString(uriV)));
            svIconSwitch(prv->categoryContentImage, 0, 0, -1.0);
        }
    }
    svSettingsPopComponent();
}
