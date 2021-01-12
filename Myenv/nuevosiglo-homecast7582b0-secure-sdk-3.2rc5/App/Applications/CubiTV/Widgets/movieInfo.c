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

#include "movieInfo.h"

#include <libintl.h>
#include <QBResourceManager/SvRBBitmap.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/icon.h>
#include <QBStringUtils.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <QBContentManager/QBContentCategory.h>
#include <Menus/menuchoice.h>
#include <Utils/QBMovieUtils.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/XMBCarousel.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Windows/mainmenu.h>
#include <Windows/pvrplayer.h>
#include <QuadriCast-MStoreVoD/MStoreVoDOrder.h>
#include <SvDataBucket2/SvDBReference.h>

struct  QBMovieInfo_t {
    MovieInfoDisplayMode displayMode;
    SvWidget        owner;
    SvWidget        content;
    SvWidget        contentImage;

    SvWidget        thumbnail;
    SvWidget        description;

    struct {
        SvWidget cart;
        SvWidget genre;
        SvWidget country;
        SvWidget countryAndYear;
        SvWidget director;
        SvWidget cast;
        SvWidget ratings;
        SvWidget runtime;
    } icons;

    SvWidget        purchaseInfo;
    SvWidget        producer;
    SvWidget        director;
    SvWidget        screenplay;
    SvWidget        runtime;
    SvWidget        language;
    SvWidget        genre;
    SvWidget        country;
    SvWidget        countryAndYear;
    SvWidget        cast;
    SvWidget        ratings;

    SvApplication   app;

    unsigned int    ctxID;
    QBTextRenderer  textRenderer;
    SvRID           tmpBmpRID;
    SvRID           tmpCategoryBmpRID;

    SvGenericObject object;
};
typedef struct QBMovieInfo_t *QBMovieInfo;

typedef enum {
    QBMovieInfoPriceFormat_full,
    QBMovieInfoPriceFormat_skipDecimalPart,
    QBMovieInfoPriceFormat_unknown,
} QBMovieInfoPriceFormat;

SvLocal QBMovieInfoPriceFormat
QBMovieInfoPriceFormatFromString(const char *format)
{
    if (strcmp(format, "full") == 0)
        return QBMovieInfoPriceFormat_full;
    else if (strcmp(format, "skipDecimalPart") == 0)
        return QBMovieInfoPriceFormat_skipDecimalPart;
    else
        return QBMovieInfoPriceFormat_unknown;
}

SvLocal void
QBMovieInfoCleanup( SvApplication app, void* ptr )
{
    QBMovieInfo prv = (QBMovieInfo) ptr;
    SVTESTRELEASE(prv->object);
    free(ptr);
}

SvWidget
MovieInfoCreate(SvApplication app, QBTextRenderer textRenderer, MovieInfoDisplayMode displayMode)
{
    QBMovieInfo prv = calloc(1, sizeof(struct QBMovieInfo_t));

    prv->app                = app;
    prv->textRenderer       = textRenderer;
    prv->displayMode        = displayMode;
    switch (displayMode) {
    case MovieInfoDisplayMode_image:
        prv->tmpBmpRID = svSettingsGetResourceID("MoviePage.PhotoContentImage", "bg");
        break;
    case MovieInfoDisplayMode_audio:
        prv->tmpBmpRID = svSettingsGetResourceID("MoviePage.AudioContentImage", "bg");
        break;
    case MovieInfoDisplayMode_video:
        prv->tmpBmpRID = svSettingsGetResourceID("MoviePage.VideoContentImage", "bg");
        break;
    case MovieInfoDisplayMode_playlist:
        prv->tmpBmpRID = svSettingsGetResourceID("MoviePage.PlaylistContentImage", "bg");
        break;
    case MovieInfoDisplayMode_emptyContainer:
        prv->tmpBmpRID = svSettingsGetResourceID("MoviePage.EmptyCategoryImage", "bg");
        break;
    case MovieInfoDisplayMode_unableToBrowseContainer:
        prv->tmpBmpRID = svSettingsGetResourceID("MoviePage.EmptyCategoryImage", "bg");
        break;
    case MovieInfoDisplayMode_noresponseContainer:
        prv->tmpBmpRID = svSettingsGetResourceID("MoviePage.EmptyCategoryImage", "bg");
        break;
    case MovieInfoDisplayMode_container:
        prv->tmpCategoryBmpRID = svSettingsGetResourceID("MoviePage.CategoryImage", "bg");
        break;
    default:
        prv->tmpBmpRID          = svSettingsGetResourceID("MoviePage.ContentImage", "bg");
        prv->tmpCategoryBmpRID  = svSettingsGetResourceID("MoviePage.CategoryImage", "bg");
        break;
    }

    prv->content            = QBFrameCreateFromSM(app, "MoviePage.Background");

    SvWidget w              = svWidgetCreate(app, prv->content->width, prv->content->height);
    w->prv                  = prv;
    prv->owner              = w;
    w->clean                = QBMovieInfoCleanup;

    svWidgetAttach(prv->owner, prv->content, 0, 0, 3);

    prv->ctxID              = svSettingsSaveContext();

    svWidgetSetFocusable(w, false);
    return w;
}

SvWidget
MovieInfoGetContentIcon(SvWidget w)
{
    QBMovieInfo prv = (QBMovieInfo) w->prv;

    return prv->contentImage;
}

SvLocal void
QBMovieInfoSetLabelText(SvWidget label, const char* str)
{
    if (!label) {
        return;
    }
    SvString s = SvStringCreate(str, NULL);
    QBAsyncLabelSetText(label, s);
    SVRELEASE(s);
}

SvLocal void
QBMovieInfoCreateSettingsLabel(QBMovieInfo self, SvWidget *label, const char* setting)
{
    if (unlikely(!label)) {
        return;
    }
    if (*label)
        return;
    if (!svSettingsIsWidgetDefined(setting)) {
        return;
    }
    *label = QBAsyncLabelNew(self->app, setting, self->textRenderer);
    if (*label)
        svSettingsWidgetAttach(self->content, *label, setting, 10);
}

SvLocal void
QBMovieInfoCreateSettingsIcon(QBMovieInfo self, SvWidget *icon, const char *setting)
{
    if (!*icon && svSettingsIsWidgetDefined(setting)) {
        *icon = svSettingsWidgetCreate(self->app, setting);
        const char* iconName = svWidgetGetName(*icon);
        svSettingsWidgetAttach(self->content, *icon, iconName, 0);
    }
}

SvLocal void
QBMovieInfoCreateContentImage(QBMovieInfo self, const char* settings)
{
    if (!self->contentImage) {
        self->contentImage  = svIconNew(self->app, settings);
        self->tmpBmpRID = svSettingsGetResourceID(settings, "bg");
        svSettingsWidgetAttach(self->owner, self->contentImage, settings, 5);
    }

    svIconSetBitmapFromRID(self->contentImage, 0, self->tmpBmpRID);
}

SvLocal void
QBMovieInfoSetThumbnail(QBMovieInfo self, const char *URI)
{
    if (unlikely(!URI)) {
        return;
    }
    svIconSetBitmapFromURI(self->contentImage, 0, URI);
    svIconSwitch(self->contentImage, 0, 0, -1.0);
}

SvLocal void
QBMovieInfoCreateProductDescWidget(QBMovieInfo self)
{
    QBMovieInfoCreateSettingsLabel(self, &self->description, "MoviePage.Description");
}

SvLocal void
QBMovieInfoFillProductDescWidget(QBMovieInfo self, SvDBRawObject metaData, char const * desc)
{
    QBMovieUtilsSetThumbnailIntoIcon(self->contentImage, (SvObject)metaData, "covers", "cover_name", "grid", 0);
    if (self->description)
        QBMovieInfoSetLabelText(self->description, desc);
}

SvLocal void
QBMovieInfoUpdateSettingsIcon(QBMovieInfo self, SvWidget *icon, const char* settingsName)
{
    if (*icon) {
        svWidgetDetach(*icon);
        svWidgetDestroy(*icon);
        *icon = NULL;
    }
    QBMovieInfoCreateSettingsIcon(self, icon, settingsName);
}

SvLocal void
QBMovieInfoCreateProductWidget(QBMovieInfo self)
{
    QBMovieInfoCreateSettingsIcon(self, &self->icons.cart, "MoviePage.MovieInfo.CartIcon");
    QBMovieInfoCreateSettingsIcon(self, &self->icons.genre, "MoviePage.MovieInfo.GenreIcon");
    QBMovieInfoCreateSettingsIcon(self, &self->icons.country, "MoviePage.MovieInfo.CountryIcon");
    QBMovieInfoCreateSettingsIcon(self, &self->icons.countryAndYear, "MoviePage.MovieInfo.CountryAndYearIcon");
    QBMovieInfoCreateSettingsIcon(self, &self->icons.director, "MoviePage.MovieInfo.DirectorIcon");
    QBMovieInfoCreateSettingsIcon(self, &self->icons.cast, "MoviePage.MovieInfo.CastIcon");
    QBMovieInfoCreateSettingsIcon(self, &self->icons.runtime, "MoviePage.MovieInfo.RuntimeIcon");
    QBMovieInfoCreateSettingsIcon(self, &self->icons.ratings, "MoviePage.MovieInfo.RatingsIcon");

    QBMovieInfoCreateSettingsLabel(self, &self->purchaseInfo, "MoviePage.MovieInfo.Price");
    QBMovieInfoCreateSettingsLabel(self, &self->ratings, "MoviePage.MovieInfo.Ratings");
    QBMovieInfoCreateSettingsLabel(self, &self->runtime, "MoviePage.MovieInfo.Runtime");
    QBMovieInfoCreateSettingsLabel(self, &self->genre, "MoviePage.MovieInfo.Genre");
    QBMovieInfoCreateSettingsLabel(self, &self->country, "MoviePage.MovieInfo.Country");
    QBMovieInfoCreateSettingsLabel(self, &self->countryAndYear, "MoviePage.MovieInfo.CountryAndYear");
    QBMovieInfoCreateSettingsLabel(self, &self->cast, "MoviePage.MovieInfo.Cast");
    QBMovieInfoCreateSettingsLabel(self, &self->director, "MoviePage.MovieInfo.Director");
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
    bool stripRatingName = true;
    //If there are at least two ratings, then don't strip rating name
    char *tmp = strchr(sourceString, '[');
    if (tmp && strchr(tmp + 1, '['))
        stripRatingName = false;

    unsigned int newStringIndex = 0;
    bool afterRatingName = false;
    for (unsigned int i=1; i < strlen(sourceString)-1; i++) {
        if (sourceString[i] == ']' && sourceString[i+1] == '[') {
            i++;
            newString[newStringIndex++] = ',';
            newString[newStringIndex++] = ' ';
            afterRatingName = false;
        } else if (sourceString[i] == '=') {
            if (!stripRatingName)
                newString[newStringIndex++] = ' ';
            afterRatingName = true;
        } else {
            if (!stripRatingName || afterRatingName)
                newString[newStringIndex++] = sourceString[i];
        }
    }
    newString[newStringIndex] = '\0';

    SvString ret = SvStringCreate(newString, NULL);
    free(newString);

    return ret;
}

SvLocal void
QBMovieInfoFillProductWidget(QBMovieInfo self, SvDBRawObject metaData)
{
    svSettingsPushComponent("Carousel_VOD.settings");
    if (self->purchaseInfo) {
        SvString expirationDateStr = QBMovieUtilsGetAttr(metaData, "expires_at", NULL);
        if (expirationDateStr && SvStringGetLength(expirationDateStr) > 0) {
            QBMovieInfoUpdateSettingsIcon(self, &self->icons.cart, "MoviePage.MovieInfo.CartIcon");
            SvTime expirationDate;
            if (QBStringToDateTime(SvStringCString(expirationDateStr), QBDateTimeFormat_ISO8601, false, &expirationDate) == (ssize_t) SvStringGetLength(expirationDateStr)) {
                struct tm localTime;
                if (SvTimeBreakDown(expirationDate, true, &localTime) < 0) {
                      //Ups, we are out of t_time type range -> sizeof(time_t) == 4
                      const char*endOfTheTime = "2038-01-01T00:00:00Z";
                      QBStringToDateTime(endOfTheTime, QBDateTimeFormat_ISO8601, false, &expirationDate);
                      SvTimeBreakDown(expirationDate, true, &localTime);
                }
                const char* dateFormat = svSettingsGetString("MoviePage.MovieInfo.Price", "expirationDateFormat");
                char buff[128];
                strftime(buff, sizeof(buff), dateFormat, &localTime);
                QBMovieInfoSetLabelText(self->purchaseInfo, buff);
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
                        QBMovieInfoUpdateSettingsIcon(self, &self->icons.cart, "MoviePage.MovieInfo.CartPromoIcon");
                    else
                        QBMovieInfoUpdateSettingsIcon(self, &self->icons.cart, "MoviePage.MovieInfo.CartIcon");

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
                        SvString rentalPeriodStr = NULL;
                        SvValue rentalPeriodV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject)offer, "rental_period");
                        if (rentalPeriodV) {
                            int rentalPeriod = SvValueGetInteger(rentalPeriodV);
                            rentalPeriodStr = create_human_readable_string_from_rental_period(rentalPeriod);
                        } else {
                            SvValue availabilityEndDateV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject)offer, "availabilityEndDate");
                            if (availabilityEndDateV) {
                                const char *availabilityEndDate = SvValueGetStringAsCString(availabilityEndDateV, NULL);
                                SvTime availabilityEndDateTime;
                                if (QBStringToDateTime(availabilityEndDate, QBDateTimeFormat_ISO8601, false, &availabilityEndDateTime) == (ssize_t) strlen(availabilityEndDate)) {
                                    struct tm localTime;
                                    if (SvTimeBreakDown(availabilityEndDateTime, true, &localTime) < 0) {
                                        //Ups, we are out of t_time type range -> sizeof(time_t) == 4
                                        const char*endOfTheTime = "2038-01-01T00:00:00Z";
                                        QBStringToDateTime(endOfTheTime, QBDateTimeFormat_ISO8601, false, &availabilityEndDateTime);
                                        SvTimeBreakDown(availabilityEndDateTime, true, &localTime);
                                    }
                                    const char* dateFormat = svSettingsGetString("MoviePage.MovieInfo.Price", "expirationDateFormat");
                                    char buff[128];
                                    strftime(buff, sizeof(buff), dateFormat, &localTime);
                                    rentalPeriodStr = SvStringCreate(buff, NULL);
                                }
                            }
                        }
                        const char* priceFormat = svSettingsGetString("MoviePage.MovieInfo.Price", "priceFormat");
                        SvString labelStr = NULL;
                        if (priceFormat && QBMovieInfoPriceFormatFromString(priceFormat) == QBMovieInfoPriceFormat_skipDecimalPart) {
                            labelStr = SvStringCreateWithFormat("%i %s %s", offerPrice/100, SvStringCString(currency),
                                                                rentalPeriodStr ? SvStringCString(rentalPeriodStr) : "");
                        } else {
                            labelStr = SvStringCreateWithFormat("%i.%02i %s %s", offerPrice/100, offerPrice%100,
                                                                SvStringCString(currency), rentalPeriodStr ? SvStringCString(rentalPeriodStr) : "");
                        }
                        QBMovieInfoSetLabelText(self->purchaseInfo, SvStringCString(labelStr));
                        SVTESTRELEASE(rentalPeriodStr);
                        SVRELEASE(labelStr);
                    }
                }
            } else
                QBMovieInfoUpdateSettingsIcon(self, &self->icons.cart, "MoviePage.MovieInfo.CartIcon");
        }
    }

    SvString ratings = QBMovieUtilsGetAttr(metaData, "ratings", NULL);
    if (ratings) {
        SvString parsedRating = QBMovieInfoParseRating(SvStringCString(ratings));
        QBMovieInfoSetLabelText(self->ratings, gettext(SvStringCString(parsedRating)));
        SVRELEASE(parsedRating);
    }

    SvString director = QBMovieUtilsGetAttr(metaData, "director", NULL);
    if (director)
        QBMovieInfoSetLabelText(self->director, SvStringCString(director));

    SvString genres = QBMovieUtilsGetAttr(metaData, "genres", NULL);
    if (genres)
        QBMovieInfoSetLabelText(self->genre,  SvStringCString(genres));

    SvString country = QBMovieUtilsGetAttr(metaData, "country", NULL);
    if (country)
        QBMovieInfoSetLabelText(self->country, SvStringCString(country));
    QBMovieUtilsSetThumbnailIntoIcon(self->contentImage, (SvObject)metaData, "covers", "cover_name", "grid", 0);
    int maxLines = svSettingsGetInteger("MoviePage.MovieInfo.Cast", "maxLines", 1);
    SvString cast = QBMovieUtilsCreateStrWithMaxLinesFromAttrArray(metaData, "cast_array", maxLines);
    if (cast) {
        QBMovieInfoSetLabelText(self->cast, SvStringCString(cast));
        SVRELEASE(cast);
    } else {
        cast = QBMovieUtilsGetAttr(metaData, "cast", NULL);
        if (cast)
            QBMovieInfoSetLabelText(self->cast, SvStringCString(cast));
    }

    if (self->countryAndYear) {
        SvValue date = (SvValue) SvDBRawObjectGetAttrValue(metaData, "productionDate");
        if (!date) {
            date = (SvValue) SvDBRawObjectGetAttrValue(metaData, "year");
        }
        if (date && SvObjectIsInstanceOf((SvObject) date, SvValue_getType()) && SvValueIsInteger(date)) {
            SvString tmp = SvStringCreateWithFormat("%s %d",
                                                    SvStringCString(QBMovieUtilsGetAttr(metaData, "country", SVSTRING(""))),
                                                    SvValueGetInteger(date));
            QBMovieInfoSetLabelText(self->countryAndYear, SvStringCString(tmp));
            SVRELEASE(tmp);
        }
    }

    SvString str = QBMovieUtilsCreateStrFromAttrInteger(metaData, "runtime", gettext("%i min"), SVSTRING(""));
    QBMovieInfoSetLabelText(self->runtime, SvStringCString(str));
    SVRELEASE(str);

    svSettingsPopComponent();
}

SvLocal void
QBMovieInfoSetupImageWidget(QBMovieInfo self, SvDBRawObject object, const char* imageType)
{
    svSettingsPushComponent("Carousel_WEBTV.settings");
    QBMovieInfoCreateContentImage(self, "MoviePage.ContentImage");
    QBMovieUtilsSetThumbnailIntoIcon(self->contentImage, (SvObject) object, "covers", "cover_name", imageType , 0);
    svSettingsPopComponent();
}

SvLocal void
QBMovieInfoCreateCategoryWidget(QBMovieInfo self, QBContentCategory category)
{
    svSettingsPushComponent("Carousel_WEBTV.settings");
    QBMovieInfoCreateContentImage(self, "MoviePage.ContentImage");

    svIconSetBitmapFromRID(self->contentImage, 0, self->tmpCategoryBmpRID);

    SvValue uriV = (SvValue) QBContentCategoryGetAttribute(category, SVSTRING("thumbnail"));
    if (unlikely(!uriV)) {
        uriV = (SvValue) QBContentCategoryGetAttribute(category, SVSTRING("contentURI"));
    }
    if (uriV && SvObjectIsInstanceOf((SvObject) uriV, SvValue_getType()) && SvValueIsString(uriV))
        QBMovieInfoSetThumbnail(self, SvStringCString(SvValueGetString(uriV)));

    svSettingsPopComponent();
}

SvLocal void
QBMovieInfoSetupTextWidget(QBMovieInfo self, SvDBRawObject object)
{
    svSettingsPushComponent("Carousel_WEBTV.settings");
    QBMovieInfoCreateSettingsLabel(self, &self->description, "MoviePage.FullText");
    if (self->description) {
        SvString description = QBMovieUtilsGetAttr(object, "description", NULL);
        if (description)
            QBMovieInfoSetLabelText(self->description, SvStringCString(description));
    }
    svSettingsPopComponent();
}

SvLocal void
QBMovieInfoSetupTextImageWidget(QBMovieInfo self, SvDBRawObject object)
{
    svSettingsPushComponent("Carousel_WEBTV.settings");
    QBMovieInfoCreateContentImage(self, "MoviePage.Image");
    QBMovieInfoCreateProductDescWidget(self);
    SvString description = QBMovieUtilsGetAttr(object, "description", NULL);
    if (description)
        QBMovieInfoFillProductDescWidget(self, object, SvStringCString(description));
    svSettingsPopComponent();
}

SvLocal void
QBMovieInfoSetupCarousel(QBMovieInfo self, SvDBRawObject object)
{
    const char *widgetName = NULL;
    bool isHugeImage = true;
    if (self->displayMode == MovieInfoDisplayMode_image) {
        widgetName = "MoviePage.PhotoContentImage";
    } else if (self->displayMode == MovieInfoDisplayMode_radio) {
        widgetName = "MoviePage.AudioContentImage";
    } else if (self->displayMode == MovieInfoDisplayMode_audio) {
        widgetName = "MoviePage.AudioContentImage";
    } else if (self->displayMode == MovieInfoDisplayMode_video) {
        widgetName = "MoviePage.VideoContentImage";
    } else if (self->displayMode == MovieInfoDisplayMode_playlist) {
        widgetName = "MoviePage.PlaylistContentImage";
    } else if (self->displayMode == MovieInfoDisplayMode_emptyContainer) {
        widgetName = "MoviePage.EmptyCategoryImage";
    } else if (self->displayMode == MovieInfoDisplayMode_unableToBrowseContainer) {
        widgetName = "MoviePage.UnbrowsableCategoryImage";
    } else if (self->displayMode == MovieInfoDisplayMode_noresponseContainer) {
        widgetName = "MoviePage.NoResponseCategoryImage";
    } else if (self->displayMode == MovieInfoDisplayMode_container) {
        widgetName = "MoviePage.CategoryImage";
    } else {
        isHugeImage = false;
    }

    if (isHugeImage) {
        svSettingsPushComponent("Carousel_WEBTV.settings");
        QBMovieInfoCreateContentImage(self, widgetName);
        SvString thumbnail = QBMovieUtilsGetAttr(object, "thumbnail", NULL);
        if (thumbnail)
            QBMovieInfoSetThumbnail(self, SvStringCString(thumbnail));
        svSettingsPopComponent();
    } else if (self->displayMode == MovieInfoDisplayMode_rss) {
        svSettingsPushComponent("Carousel_WEBTV.settings");
        QBMovieInfoCreateContentImage(self, "MoviePage.Image");
        SvString description = QBMovieUtilsGetAttr(object, "description", NULL);
        QBMovieInfoCreateProductDescWidget(self);
        if (description)
            QBMovieInfoFillProductDescWidget(self, object, SvStringCString(description));
        svSettingsPopComponent();
    } else {
        svSettingsPushComponent("Carousel_VOD.settings");
        QBMovieInfoCreateContentImage(self, "MoviePage.Image");
        QBMovieInfoCreateProductWidget(self);
        QBMovieInfoFillProductWidget(self, object);
        svSettingsPopComponent();
    }
}

SvLocal void
QBMovieInfoSetupWidget(QBMovieInfo self, SvDBRawObject object)
{
    SvString presentationScheme = QBMovieUtilsGetAttr(object, "presentation_scheme", NULL);
    if (presentationScheme) {
        if (SvStringEqualToCString(presentationScheme, "TEXTIMAGE")) {
            SvString thumbnail = QBMovieUtilsGetAttr(object, "thumbnail", NULL);
            SvString description = QBMovieUtilsGetAttr(object, "description", NULL);
            if (!thumbnail) {
                QBMovieInfoSetupTextWidget(self, object);
            } else if (!description) {
                QBMovieInfoSetupImageWidget(self, object, "movie_details");
            } else {
                QBMovieInfoSetupTextImageWidget(self, object);
            }
        } else if (SvStringEqualToCString(presentationScheme, "IMAGE")) {
            QBMovieInfoSetupImageWidget(self, object, "grid");
        } else if (SvStringEqualToCString(presentationScheme, "TEXT")) {
            QBMovieInfoSetupTextWidget(self, object);
        } else {
            SvLogError("%s unrecognized presentationScheme: '%s'", __func__, SvStringCString(presentationScheme));
        }
    } else {
        // image mode is enforced, otherwise try to guess display mode by product type
        if (self->displayMode != MovieInfoDisplayMode_image) {
            // guess display mode by product type
            SvString type = QBMovieUtilsGetAttr(object, "type", SVSTRING("rss"));
            if (SvStringEqualToCString(type, "vod")) {
                self->displayMode = MovieInfoDisplayMode_vod;
            } else if (SvStringEqualToCString(type, "internet_radio")) {
                self->displayMode = MovieInfoDisplayMode_radio;
            } else if (SvStringEqualToCString(type, "video")) {
                self->displayMode = MovieInfoDisplayMode_video;
            } else if (SvStringEqualToCString(type, "audio")) {
                self->displayMode = MovieInfoDisplayMode_audio;
            } else if (SvStringEqualToCString(type, "image")) {
                self->displayMode = MovieInfoDisplayMode_image;
            } else if (SvStringEqualToCString(type, "playlist")) {
                self->displayMode = MovieInfoDisplayMode_playlist;
            } else if (SvStringEqualToCString(type, "empty")) {
                self->displayMode = MovieInfoDisplayMode_emptyContainer;
            } else if (SvStringEqualToCString(type, "unbrowsable") || SvStringEqualToCString(type, "accessdenied")) {
                self->displayMode = MovieInfoDisplayMode_unableToBrowseContainer;
            } else if (SvStringEqualToCString(type, "noresponse")) {
                self->displayMode = MovieInfoDisplayMode_noresponseContainer;
            } else {
                self->displayMode = MovieInfoDisplayMode_rss;
            }
        }
        QBMovieInfoSetupCarousel(self, object);
    }
}

SvLocal void QBMovieInfoSetupExpiredWidget(QBMovieInfo self) {
    svSettingsPushComponent("Carousel_VOD.settings");
    QBMovieInfoCreateContentImage(self, "MoviePage.ExpiredIcon");

    QBMovieInfoCreateSettingsLabel(self, &self->description,
                                   "MoviePage.ExpiredText");
    if (self->description)
        QBMovieInfoSetLabelText(self->description, gettext("Renting Period Expired."));
    svSettingsPopComponent();
}

SvLocal void
MovieInfoClearWidgets(QBMovieInfo self)
{
    svWidgetDestroy(self->content);
    self->content = NULL;
    svWidgetDestroy(self->contentImage);
    self->contentImage = NULL;
    self->purchaseInfo = NULL;
    self->thumbnail = NULL;
    self->producer = NULL;
    self->director = NULL;
    self->screenplay = NULL;
    self->runtime = NULL;
    self->language = NULL;
    self->genre = NULL;
    self->country = NULL;
    self->countryAndYear = NULL;
    self->cast = NULL;
    self->description = NULL;
    self->ratings = NULL;

    self->icons.genre = NULL;
    self->icons.cart = NULL;
    self->icons.director = NULL;
    self->icons.cast = NULL;
    self->icons.ratings = NULL;
    self->icons.runtime = NULL;
    self->icons.country = NULL;
    self->icons.countryAndYear = NULL;

    self->content = QBFrameCreateFromSM(self->app, "MoviePage.Background");
    svWidgetAttach(self->owner, self->content, 0, 0, 3);
}

SvLocal bool
ObjectsCompatible(SvGenericObject objA, SvGenericObject objB)
{
    if (!objA && !objB)
        return true;
    if (!objA || !objB)
        return false;
    if (SvObjectGetType(objA) == SvObjectGetType(objB)) {
        if (SvObjectIsInstanceOf(objA, QBContentCategory_getType())) {
            return true;
        } else if (SvObjectIsInstanceOf((SvObject) objA, SvDBRawObject_getType())) {
            SvString typeA = QBMovieUtilsGetAttr((SvDBRawObject) objA, "type", NULL);
            SvString typeB = QBMovieUtilsGetAttr((SvDBRawObject) objB, "type", NULL);
            if (typeA && typeB && SvObjectEquals((SvObject) typeA, (SvObject) typeB)) {
                // not compatible if only one has thumbnail
                SvString thumbA = QBMovieUtilsGetAttr((SvDBRawObject)objA, "thumbnail", NULL);
                SvString thumbB = QBMovieUtilsGetAttr((SvDBRawObject)objB, "thumbnail", NULL);
                if ((thumbA == NULL) ^ (thumbB == NULL)) {
                    return false;
                }

                // not compatible if only one had description
                SvString descA = QBMovieUtilsGetAttr((SvDBRawObject)objA, "description", NULL);
                SvString descB = QBMovieUtilsGetAttr((SvDBRawObject)objB, "description", NULL);
                if ((descA == NULL) ^ (descB == NULL)) {
                    return false;
                }

                SvString schemeA = QBMovieUtilsGetAttr((SvDBRawObject) objA, "presentation_scheme", NULL);
                SvString schemeB = QBMovieUtilsGetAttr((SvDBRawObject) objB, "presentation_scheme", NULL);
                if (schemeA && schemeB && SvObjectEquals((SvObject) schemeA, (SvObject) schemeB))
                    return true;
            }
        }
    }

    return false;
}


void
MovieInfoSetObject(SvWidget w, SvGenericObject object)
{
    QBMovieInfo prv = (QBMovieInfo) w->prv;

    svSettingsRestoreContext(prv->ctxID);

    if (!ObjectsCompatible(prv->object, object)) {
        MovieInfoClearWidgets(prv);
    }

    SVTESTRETAIN(object);
    SVTESTRELEASE(prv->object);
    prv->object = object;

    if(!object) {
        svSettingsPopComponent();
        return;
    } else {
        if (SvObjectIsInstanceOf((SvObject) object, SvDBRawObject_getType())) {
            QBMovieInfoSetupWidget(prv, (SvDBRawObject) object);
        } else if (SvObjectIsInstanceOf(object, QBContentCategory_getType())) {
            QBContentCategory category = (QBContentCategory) object;
            QBMovieInfoCreateCategoryWidget(prv, category);
        } else if (SvObjectIsInstanceOf(object, MStoreVoDOrder_getType())) {
            QBMovieInfoSetupExpiredWidget(prv);
        } else {
            //any other type received
        }
    }

    svSettingsPopComponent();
}

