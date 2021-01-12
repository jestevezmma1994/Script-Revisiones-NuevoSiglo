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

#include "QBMovieInfo.h"

#include <libintl.h>
#include <settings.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/widget.h>
#include <SvDataBucket2/SvDBObject.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <SvDataBucket2/SvDBReference.h>
#include <QBStringUtils.h>
#include <Utils/QBMovieUtils.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <Windows/mainmenu.h>
#include <QBContentManager/QBContentCategory.h>
#include <Logic/VoDLogic.h>
#include <QBContentManager/Innov8onUtils.h>
#include <SvFoundation/SvStringBuffer.h>

/**
 * Type of rental duration format.
 **/
typedef enum {
    // show duration in minutes
    QBMovieInfoDurationFormatM  = 0,
    // show duration in hours
    QBMovieInfoDurationFormatH  = 1,
    // show duration in days
    QBMovieInfoDurationFormatD  = 2
} QBMovieInfoDurationFormat;

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
        SvWidget categoria; //AMERELES #2316 Categorización del título VoD
    } icons;

    SvWidget purchaseInfo; ///< price or rental period if the movie was rented
    SvWidget purchaseAnimation; ///< animation displayed if purchase data is being fetched (not yet known)
    SvWidget director; ///< movie director name
    SvWidget runtime; ///< movie runtime in minutes
    SvWidget genre; ///< genres a movie belongs to
    SvWidget country; ///< movie country of origin
    SvWidget cast; ///< list of cast memebers
    SvWidget ratings; ///< parental control ratings of a movie
    SvWidget categoria; //AMERELES #2316 Categorización del título VoD

    unsigned int settingsCtx;

    SvApplication   app;

    QBTextRenderer  textRenderer;
    const char     *dateFormat;
    int             castMaxLines;

    SvGenericObject object;

    const char     *castStringSeparator;

    QBMovieInfoDurationFormat durationFormat;

    bool isLoading; ///<flag telling if some info is not yet available, because data fetch is still in progress
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

SvWidget
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

    char tmp[128];
    snprintf(tmp, 128, "%s.%s", svWidgetGetName(w), "Price");
    prv->dateFormat = svSettingsGetString(tmp, "expirationDateFormat");
    prv->durationFormat = svSettingsGetInteger(tmp, "durationFormat", QBMovieInfoDurationFormatD);
    snprintf(tmp, 128, "%s.%s", svWidgetGetName(w), "Cast");
    prv->castMaxLines = svSettingsIsWidgetDefined(tmp) ? svSettingsGetInteger(tmp, "maxLines", 3) : 3;
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->purchaseInfo, &prv->icons.purchaseInfo, svWidgetGetName(w), "Price");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->ratings, &prv->icons.ratings, svWidgetGetName(w), "Ratings");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->runtime, &prv->icons.runtime, svWidgetGetName(w), "Runtime");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->genre, &prv->icons.genre, svWidgetGetName(w), "Genre");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->country, &prv->icons.country, svWidgetGetName(w), "Country");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->categoria, &prv->icons.categoria, svWidgetGetName(w), "Categoria"); //AMERELES #2316 Categorización del título VoD
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->cast, &prv->icons.cast, svWidgetGetName(w), "Cast");
    QBMovieInfoCreateSettingsLabelAndIcon(prv, &prv->director, &prv->icons.director, svWidgetGetName(w), "Director");

    prv->castStringSeparator = svSettingsGetString(svWidgetGetName(w), "castStringSeparator");

    return w;
}

SvLocal SvString
create_human_readable_string_from_rental_period(int minutes, QBMovieInfoDurationFormat format)
{
    SvString niceString = NULL;
    if (minutes > 0) {
        if (minutes < 60 || format == QBMovieInfoDurationFormatM) {
            niceString = SvStringCreateWithFormat(gettext("(%dmin)"), minutes);
        } else if (minutes < 1440 || format == QBMovieInfoDurationFormatH) { // 24h
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
        return NULL;

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
QBMovieInfoFillProductWidget(QBMovieInfo self, SvDBRawObject metaData)
{
    SvString str;
    if (self->purchaseInfo) {
        SvString expirationDateStr = QBMovieUtilsGetAttr(metaData, "expires_at", NULL);
        if (expirationDateStr  && SvStringGetLength(expirationDateStr) > 0) {
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
                if (self->purchaseAnimation) {
                    svWidgetDestroy(self->purchaseAnimation);
                    self->purchaseAnimation = NULL;
                }
                QBAsyncLabelSetCText(self->purchaseInfo, buff);
            }
        } else {
            SvArray offers = (SvArray) SvDBRawObjectGetAttrValue(metaData, "offers");
            if (offers && SvObjectIsInstanceOf((SvObject) offers, SvArray_getType()) && SvArrayCount(offers) == 1) {
                SvDBRawObject offer = (SvDBRawObject) SvDBReferenceGetReferredObject((SvDBReference) SvArrayObjectAtIndex(offers, 0));
                if (offer && QBInnov8onOfferIsAvailable(offer)) {
                    SvValue val = (SvValue) SvDBRawObjectGetAttrValue(offer, "promotional");
                    bool isPromotion = false;
                    if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsInteger(val))
                        isPromotion = SvValueGetInteger(val);

                    if (isPromotion)
                        QBMovieInfoUpdateSettingsIcon(self, &self->icons.purchaseInfo, svWidgetGetName(self->owner), "PricePromoIcon");
                    else
                        QBMovieInfoUpdateSettingsIcon(self, &self->icons.purchaseInfo, svWidgetGetName(self->owner), "PriceIcon");


                    SvValue currencyV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject)offer, "currency");
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
                        if (rentalPeriodV && SvObjectIsInstanceOf((SvObject) rentalPeriodV, SvValue_getType()) && SvValueIsInteger(rentalPeriodV)) {
                            int rentalPeriod = SvValueGetInteger(rentalPeriodV);
                            rentalPerdiodStr = create_human_readable_string_from_rental_period(rentalPeriod, self->durationFormat);
                        }
                        if (self->purchaseAnimation) {
                            svWidgetDestroy(self->purchaseAnimation);
                            self->purchaseAnimation = NULL;
                        }
                        if (offerPrice > 0) {
                            SvString labelStr = SvStringCreateWithFormat("%i.%02i %s %s", offerPrice / 100, offerPrice % 100, SvStringCString(currency), rentalPerdiodStr ? SvStringCString(rentalPerdiodStr) : "");
                            QBAsyncLabelSetCText(self->purchaseInfo, SvStringCString(labelStr));
                            SVRELEASE(labelStr);
                        } else {
                            SvString labelStr = SvStringCreate(gettext("FREE"), NULL);
                            QBAsyncLabelSetText(self->purchaseInfo, labelStr);
                            SVRELEASE(labelStr);
                        }
                        SVTESTRELEASE(rentalPerdiodStr);
                    }
                } else if (offer) {
                    SvString labelStr = SvStringCreate(gettext("Free with Subscription"), NULL);
                    QBAsyncLabelSetText(self->purchaseInfo, labelStr);
                    SVRELEASE(labelStr);
                }
            } else {
                if (self->isLoading) { //the purchase info is not known yet (data fetch in progress) - display loading animation in this case
                    char tmp[128];
                    snprintf(tmp, 128, "%s.%s", svWidgetGetName(self->owner), "Animation");
                    if (!self->purchaseAnimation) {
                        self->purchaseAnimation = QBWaitAnimationCreate(self->app, tmp);
                    }
                    svSettingsWidgetAttach(self->owner, self->purchaseAnimation, tmp, 10);
                }
            }
        }
    }

    if (self->ratings) {
        SvString ratings = QBMovieUtilsGetAttr(metaData, "ratings", NULL);
        // BEGIN AMERELES Agregar Información de Lenguaje a contenidos de VoD3
        SvStringBuffer buff = SvStringBufferCreate(NULL);
        if (ratings) {
            SvString parsedRating = QBMovieInfoParseRating(SvStringCString(ratings));
            if (parsedRating) {
                //QBAsyncLabelSetCText(self->ratings, SvStringCString(parsedRating));
                SvStringBufferAppendCString(buff, SvStringCString(parsedRating) , NULL);
                SVRELEASE(parsedRating);
            }
        } else {
            SvString message = QBVODLogicCreateNoRatingMessage();
            if (message) {
                //QBAsyncLabelSetCText(self->ratings, SvStringCString(message));
                SvStringBufferAppendCString(buff, SvStringCString(message) , NULL);
                SVRELEASE(message);
            }
        }
        SvString extended_description = QBMovieUtilsGetAttr(metaData, "extended_description", SVSTRING(""));
        
        char str_char[1024];
        char *ptr = NULL;
        strncpy(str_char, SvStringCString(extended_description), 1023);
        strtok_r (str_char, "|", &ptr);
        SvString language;
        if (ptr && strcmp(ptr, "") != 0)
        {
            language = SvStringCreate(ptr, NULL);
        }
        else if (str_char[0] == '|') // Si es de tipo "|"+"lenguaje" porque strtok_r ignora token al inicio y fin
        {
            language = SvStringCreate(str_char + 1, NULL);
        }
        else
        {
            language = SvStringCreate("", NULL);
        }
        
        if (!SvStringEqualToCString(SvStringBufferCreateContentsString(buff, NULL), "") && !SvStringEqualToCString(language, ""))
        {
            SvStringBufferAppendCString(buff, " | ", NULL);
        }
        
        SvStringBufferAppendCString(buff, SvStringCString(language), NULL);
        SvString parsedRating_language = SvStringBufferCreateContentsString(buff, NULL);
        QBAsyncLabelSetCText(self->ratings, SvStringCString(parsedRating_language));
        
        SVTESTRELEASE(buff);
        SVTESTRELEASE(language);
        SVTESTRELEASE(parsedRating_language);
        // END AMERELES Agregar Información de Lenguaje a contenidos de VoD3
    }

    if (self->director) {
        SvString director = QBMovieUtilsGetAttr(metaData, "director", NULL);
        if (director)
            QBAsyncLabelSetText(self->director, director);
    }
    if (self->genre) {
        SvString genres = QBMovieUtilsGetAttr(metaData, "genres", NULL);
        if (genres)
            QBAsyncLabelSetText(self->genre, genres);
    }
    if (self->country) {
        // BEGIN AMERELES, add year to VoD info, as year-country
        SvString year = QBMovieUtilsCreateStrFromAttrInteger(metaData, "year", "%i", SVSTRING(""));;// la ultima parte SVSTRING("") hace que no pueda devolver null el QBMovieUtilsGetAttr
        SvString country = QBMovieUtilsGetAttr(metaData, "country", SVSTRING("")); // la ultima parte SVSTRING("") hace que no pueda devolver null el QBMovieUtilsGetAttr

        // AMERELES BEGIN #2316 Categorización del título VoD
        // Campo country ahora es de la forma "country"|"categoría del contenido"
        char str2[128];
        char *ptr = NULL;
        strcpy(str2, SvStringCString(country));
        strtok_r (str2, "|", &ptr);
        
        if (str2[0] == '|') // Si el primer caracter es '|' strtok_r no separa en str="" y ptr="tipo_cat" sino que queda str con todo y ptr=""
        {
            country = SvStringCreate("", NULL);
        }
        else
        {
            country = SvStringCreate(str2, NULL);
        }
        // AMERELES END #2316 Categorización del título VoD
      
        SvStringBuffer buff = SvStringBufferCreate(NULL);
        SvStringBufferAppendCString(buff, SvStringCString(year) , NULL);
        if (!SvStringEqualToCString(year, "") && !SvStringEqualToCString(country, ""))
        {
            SvStringBufferAppendCString(buff, " | ", NULL);
        }

        SvStringBufferAppendCString(buff, SvStringCString(country), NULL);
        SvString year_country;
        year_country = SvStringBufferCreateContentsString(buff, NULL);
        QBAsyncLabelSetCText(self->country, SvStringCString(year_country));
        // END AMERELES, add year to VoD info, as year-country


       // if (country)
       //     QBAsyncLabelSetText(self->country, country);
    }

    // AMERELES BEGIN #2316 Categorización del título VoD
    // Campo country ahora es de la forma "country"|"categoría del contenido"

    if (self->categoria)
    {
        SvString country = QBMovieUtilsGetAttr(metaData, "country", SVSTRING(""));
        char str3[128];
        char *ptr = NULL;
        strcpy(str3, SvStringCString(country));
        strtok_r (str3, "|", &ptr);
        
        if (str3[0] == '|') // Si el primer caracter es '|' strtok_r no separa en str="" y ptr="tipo_cat" sino que queda str con todo y ptr=""
        {
            QBAsyncLabelSetCText(self->categoria, str3 + 1);
        }
        else if (ptr)
        {
            QBAsyncLabelSetCText(self->categoria, ptr);
        }
    }
    // AMERELES END #2316 Categorización del título VoD

    if (self->cast) {
        SvString castString = QBMovieUtilsCreateFormattedStrFromAttrArray(metaData, "cast_array", self->castMaxLines, self->castStringSeparator);
        if (castString) {
            QBAsyncLabelSetText(self->cast, castString);
            SVRELEASE(castString);
        } else {
            SvString cast = QBMovieUtilsGetAttr(metaData, "cast", NULL);
            if (cast)
                QBAsyncLabelSetText(self->cast, cast);
        }
    }

    if (self->runtime) {
        str = QBMovieUtilsCreateStrFromAttrInteger(metaData, "runtime", "%i min", SVSTRING(""));
        QBAsyncLabelSetCText(self->runtime, SvStringCString(str));
        SVRELEASE(str);
    }
}

SvLocal void
QBMovieInfoClearWidgets(QBMovieInfo self)
{
    if (self->purchaseAnimation) {
        svWidgetDestroy(self->purchaseAnimation);
        self->purchaseAnimation = NULL;
    }
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
    if (self->categoria)
        QBAsyncLabelSetText(self->categoria, NULL);
    if (self->cast)
        QBAsyncLabelSetText(self->cast, NULL);
    if (self->director)
        QBAsyncLabelSetText(self->director, NULL);
    if (self->icons.purchaseInfo)
        QBMovieInfoUpdateSettingsIcon(self, &self->icons.purchaseInfo, svWidgetGetName(self->owner), "Price");
}

void
QBMovieInfoSetObjectLoadingState(SvWidget w, bool isLoading)
{
    QBMovieInfo self = (QBMovieInfo) w->prv;
    self->isLoading = isLoading;
    if (!self->isLoading && self->purchaseAnimation) {
        svWidgetDestroy(self->purchaseAnimation);
        self->purchaseAnimation = NULL;
    }
}

void
QBMovieInfoSetObject(SvWidget w, SvGenericObject object)
{
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
            QBMovieInfoFillProductWidget(prv, (SvDBRawObject) object);
        } else if (SvObjectIsInstanceOf(object, QBContentCategory_getType())) {
        } else {
            //any other type received
        }
    }
fini:
    svSettingsPopComponent();
}
