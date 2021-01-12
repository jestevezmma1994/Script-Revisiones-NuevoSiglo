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

#include "Widgets/infoIcons.h"

#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/icon.h>
#include <dataformat/sv_data_format.h>
#include <Utils/value.h>
#include <Utils/QBEventUtils.h>
#include <QBPCRatings/QBPCRating.h>
#include <QBPCRatings/QBPCUtils.h>
#include <SvSettingsManager/SvSettingsParamsArray.h>
#include <SvCore/SvEnv.h>

// Default icon useful when we have event without any information about ratings.
// First icon supposed to be 'not rated'
#define QB_INFO_ICONS_DEFAULT_RATING 0

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "QBInfoIconsLogLevel", "");

#define log_error(fmt, ...)   do { if (env_log_level() >= 0) SvLogError(COLBEG() "[%s] " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#define log_warning(fmt, ...) do { if (env_log_level() >= 0) SvLogWarning(COLBEG() "[%s] " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
#define log_state(fmt, ...)   do { if (env_log_level() >= 1) SvLogNotice(COLBEG() "[%s] " fmt COLEND_COL(cyan), __func__, ## __VA_ARGS__); } while (0)
#define log_debug(fmt, ...)   do { if (env_log_level() >= 2) SvLogNotice("[%s] " fmt, __func__, ## __VA_ARGS__); } while (0)

static const char *iconsName[] = {
        [QBInfoIcons_AC3] = "dolby",
        [QBInfoIcons_16_9] = "aspect16x9",
        [QBInfoIcons_Parental] = "parental",
        [QBInfoIcons_Subtitles] = "subtitles",
        [QBInfoIcons_HD] = "HD",
        [QBInfoIcons_Radio] = "radio",
        [QBInfoIcons_Audio] = "audio",
        [QBInfoIcons_Rating] = "rating",
        [QBInfoIcons_Teletext] = "teletext",
        [QBInfoIcons_Recording] = "recording",
        [QBInfoIcons_Premium] = "premium",
};

typedef enum{
    IconDisabled,
    IconEnabled
}IconState;

struct QBInfoIconsConstructor_t {
    struct SvObject_ super_;

    SvArray bitmaps[QBInfoIcons_Count];
    int off_x[QBInfoIcons_Count], off_y[QBInfoIcons_Count];
    double duration;
    int width, height;

    SvApplication app;
    EventsLogic eventsLogic;
    SvEPGManager epgManager;
    QBParentalControl pc;
};

SvLocal void
QBInfoIconsConstructorLoadIcon(QBInfoIconsConstructor self, const char *widgetName, QBInfoIconsSelected icon)
{
    if (!self) {
        log_error("null self passed");
        return;
    }

    log_debug("key: %s Selected icon: %d", widgetName, icon);

    SvSettingsParamsArray bitmaps = svSettingsGetArray(widgetName, "bitmaps");
    if (bitmaps) {
        if (SvSettingsParamsArrayGetElementType(bitmaps) != SvSettingsParamType_bitmap) {
            log_error("invalid array of bitmaps");
            return;
        }
        self->bitmaps[icon] = SVRETAIN(bitmaps);
    }
}

SvLocal void
QBInfoIconsConstructor__dtor__(void *self_)
{
    QBInfoIconsConstructor self = self_;
    int i;
    for (i = 0; i < QBInfoIcons_Count; i++) {
        SVTESTRELEASE(self->bitmaps[i]);
    }

    SVRELEASE(self->eventsLogic);
    SVRELEASE(self->epgManager);
    SVRELEASE(self->pc);
}

SvLocal SvType
QBInfoIconsConstructor_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBInfoIconsConstructor__dtor__
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBInfoIconsConstructor",
                            sizeof(struct QBInfoIconsConstructor_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

QBInfoIconsConstructor
QBInfoIconsConstructorCreate(SvApplication app,
                             const char *widgetName,
                             EventsLogic eventsLogic,
                             SvEPGManager epgManager,
                             QBParentalControl pc)
{
    QBInfoIconsConstructor self = (QBInfoIconsConstructor) SvTypeAllocateInstance(QBInfoIconsConstructor_getType(), NULL);
    int i;

    const size_t nameSize = strlen(widgetName) + 16;
    char name[nameSize];
    for (i = 0; i < QBInfoIcons_Count; i++) {
        snprintf(name, nameSize, "%s.%s", widgetName, iconsName[i]);

        if (!svSettingsIsWidgetDefined(name))
            continue;
        QBInfoIconsConstructorLoadIcon(self, name, i);
        self->off_x[i] = svSettingsGetInteger(name, "xOffset", 0);
        self->off_y[i] = svSettingsGetInteger(name, "yOffset", 0);
    }
    self->width = svSettingsGetInteger(widgetName, "width", 0);
    self->height = svSettingsGetInteger(widgetName, "height", 0);
    self->duration = svSettingsGetDouble(widgetName, "fadeDuration", 0.0);

    self->app = app;
    self->eventsLogic = SVRETAIN(eventsLogic);
    self->epgManager = SVRETAIN(epgManager);
    self->pc = SVRETAIN(pc);

    return self;
}

typedef struct QBInfoIcons_t *QBInfoIcons;
struct QBInfoIcons_t {
    SvWidget icons[QBInfoIcons_Count];
    double duration;

    SvValue channelId;

    EventsLogic eventsLogic;
    SvEPGManager epgManager;
    QBParentalControl pc;
};

SvLocal QBPCRating QBInfoIconsCreateMovieRating(QBInfoIcons self, SvDBRawObject movie)
{
    if (!movie)
        return NULL;

    SvValue ratingV = (SvValue) SvDBRawObjectGetAttrValue(movie, "ratings");
    if (!ratingV)
        ratingV = (SvValue) SvDBRawObjectGetAttrValue(movie, "PCRating");
    SvString ratingString = SvValueTryGetString(ratingV);
    if (!ratingString)
        return NULL;

    SvArray movieRatings = QBPCUtilsCreateRatingsFromString(ratingString);
    if (!movieRatings) {
        return NULL;
    }
    QBPCRating lowestMovieRating = SVTESTRETAIN((QBPCRating) SvArrayAt(movieRatings, 0));
    SVRELEASE(movieRatings);

    return lowestMovieRating;
}

SvLocal void
QBInfoIconsClean(SvApplication app, void *self_)
{
    QBInfoIcons self = (QBInfoIcons) self_;
    SVTESTRELEASE(self->channelId);

    SVRELEASE(self->eventsLogic);
    SVRELEASE(self->epgManager);
    SVRELEASE(self->pc);

    free(self_);
}

SvWidget
QBInfoIconsCreateFromConstructor(QBInfoIconsConstructor constructor)
{
    log_state("");
    QBInfoIcons prv = calloc(1, sizeof(*prv));
    SvWidget w = svWidgetCreateBitmap(constructor->app, constructor->width, constructor->height, NULL);

    w->prv = prv;
    w->clean = QBInfoIconsClean;
    prv->eventsLogic = SVRETAIN(constructor->eventsLogic);
    prv->epgManager = SVRETAIN(constructor->epgManager);
    prv->pc = SVRETAIN(constructor->pc);

    for (unsigned int i = 0; i < QBInfoIcons_Count; i++) {
        if (constructor->bitmaps[i] > 0) {
            int count = SvSettingsParamsArrayGetCount((SvSettingsParamsArray) constructor->bitmaps[i]);
            log_debug("array of bitmaps nr: %d count: %d", i, count);
            if (count <= 0)
                continue;
            SvBitmap bmp[count];
            unsigned int width = 0, height = 0;
            for (int j = 0; j < count; j++) {
                bmp[j] = (SvBitmap) SvSettingsParamsArrayGetBitmap((SvSettingsParamsArray) constructor->bitmaps[i], j, NULL);
                if (bmp[j]->width > width)
                    width = bmp[j]->width;
                if (bmp[j]->height > height)
                    height = bmp[j]->height;
            }
            prv->icons[i] = svIconNewWithSize(constructor->app, width, height, false);
            for (int j = 0; j < count; j++) {
                svIconSetBitmap(prv->icons[i], j, bmp[j]);
                SvString key = SvSettingsParamsArrayGetKey((SvSettingsParamsArray) constructor->bitmaps[i], j, NULL);
                if (i == QBInfoIcons_Rating) {
                    SvString normalizedKey = QBPCUtilsCreateCanonicalRatingName(key);
                    if (normalizedKey) {
                        log_debug("Settings rating name: %s Normalized: %s", SvStringCString(key), SvStringCString(normalizedKey));
                        SvIconSetBitmapTag(prv->icons[i], j, normalizedKey);
                        SVRELEASE(normalizedKey);
                    } else {
                        log_warning("Using unknown rating name: %s", SvStringCString(key));
                        SvIconSetBitmapTag(prv->icons[i], j, key);
                    }
                } else {
                    SvIconSetBitmapTag(prv->icons[i], j, key);
                }
            }
            svWidgetAttach(w, prv->icons[i], constructor->off_x[i], constructor->off_y[i], 10);
        }
    }
    prv->duration = constructor->duration;

    return w;
}

SvWidget
QBInfoIconsCreate(SvApplication app,
                  const char *widgetName,
                  EventsLogic eventsLogic,
                  SvEPGManager epgManager,
                  QBParentalControl pc)
{
    QBInfoIconsConstructor c = QBInfoIconsConstructorCreate(app, widgetName,
                                                            eventsLogic,
                                                            epgManager, pc);
    SvWidget w = QBInfoIconsCreateFromConstructor(c);
    SVRELEASE(c);
    return w;
}

void QBInfoIconsSetState(SvWidget w, QBInfoIconsSelected icon, int state)
{
    QBInfoIcons prv = (QBInfoIcons) w->prv;
    if (!prv->icons[icon] || svIconGetIndex(prv->icons[icon]) == state)
        return;

    svIconSwitch(prv->icons[icon], state, 0, prv->duration);
}

void QBInfoIconsSetStateByTag(SvWidget w, QBInfoIconsSelected icon, SvString tag)
{
    QBInfoIcons prv = (QBInfoIcons) w->prv;
    if (!prv->icons[icon] || !tag)
        return;

    if (svIconSwitchByTag(prv->icons[icon], tag, 0, prv->duration)) {
        log_warning("Can't set icon with tag '%s'.", SvStringCString(tag));
    }
}

void QBInfoIconsClear(SvWidget w)
{
    int i;
    for (i = 0; i < QBInfoIcons_Count; i++)
        QBInfoIconsSetState(w, i, 0);
}

void QBInfoIconsSetByViewport(SvWidget w, QBViewport viewport){
    if(viewport) {
        QBViewportVideoInfo vi;
        int res = QBViewportGetVideoInfo(viewport, &vi);
        if(!res) {
            QBInfoIconsSetState(w, QBInfoIcons_HD, vi.height > 625); // more then PAL
            QBInfoIconsSetState(w, QBInfoIcons_16_9, vi.aspectRatio == QBAspectRatio_16x9);
        } else {
            QBInfoIconsSetState(w, QBInfoIcons_16_9, 0);
        }
    }
}

void QBInfoIconsSetBySubsManager(SvWidget w, QBSubsManager subsManager){
    QBInfoIconsSetState(w, QBInfoIcons_Subtitles, subsManager && QBActiveArrayCount(QBSubsManagerGetAllTracks(subsManager))>1);
}

void QBInfoIconsSetByFormat(SvWidget w, struct svdataformat* format){
    if(format){
        int ac3=0, i=0;
        for(i=0; i<format->audio_cnt; i++)
            ac3 |= (format->audio[i].codec == QBAudioCodec_ac3 || format->audio[i].codec == QBAudioCodec_eac3);
        QBInfoIconsSetState(w, QBInfoIcons_AC3, ac3);
        QBInfoIconsSetState(w, QBInfoIcons_Audio, format->audio_cnt>1);
        QBInfoIconsSetState(w, QBInfoIcons_Teletext, format->ts.teletext_cnt>0);
    }else{
        QBInfoIconsSetState(w, QBInfoIcons_AC3, 0);
        QBInfoIconsSetState(w, QBInfoIcons_Subtitles, 0);
        QBInfoIconsSetState(w, QBInfoIcons_Audio, 0);
        QBInfoIconsSetState(w, QBInfoIcons_Teletext, 0);
    }
}

void QBInfoIconsSetByTVChannel(SvWidget w, SvTVChannel channel)
{
    QBInfoIcons prv = (QBInfoIcons) w->prv;

    if (!channel) {
        QBInfoIconsSetState(w, QBInfoIcons_Parental, false);
        QBInfoIconsSetState(w, QBInfoIcons_HD, false);
        return;
    }

    QBInfoIconsSetState(w, QBInfoIcons_Parental, QBParentalControlChannelIsBlocked(prv->pc, channel));

    if (!SvObjectEquals((SvObject) prv->channelId, (SvObject) SvTVChannelGetID(channel))) {
        SVTESTRELEASE(prv->channelId);
        prv->channelId = SVRETAIN(SvTVChannelGetID(channel));
        SvValue attrHD = channel ? (SvValue) SvTVChannelGetAttribute(channel, SVSTRING("HD")) : NULL;
        bool HD = attrHD && SvObjectIsInstanceOf((SvObject) attrHD, SvValue_getType()) && SvValueGetBoolean(attrHD);
        QBInfoIconsSetState(w, QBInfoIcons_HD, HD);

        SvValue isPremiumVal = channel ? (SvValue) SvTVChannelGetAttribute(channel, SVSTRING("isPremium")) : NULL;
        bool isPremium = isPremiumVal && SvObjectIsInstanceOf((SvObject) isPremiumVal, SvValue_getType()) && SvValueGetBoolean(isPremiumVal);
        QBInfoIconsSetState(w, QBInfoIcons_Premium, isPremium);
    }
}

SvLocal SvString
QBInfoIconsCreateEventRatingName(EventsLogic eventsLogic, SvEPGManager epgManager, SvEPGEvent event)
{
    if (!eventsLogic || !epgManager || !event) {
        return 0;
    }

    QBPCList eventRating = QBEventUtilsCreateEventRating(eventsLogic, epgManager, event);

    if (!eventRating) {
        return 0;
    }

    SvArray eventRatingsListByStandard = NULL;

    for (QBPCRatingStandard i = QBPCRatingStandard_FCC; i < QBPCRatingStandard_MAX; i++) {
        if (!QBPCListIsStandardCleared(eventRating, i, NULL)) {
            eventRatingsListByStandard = QBPCListCreateLockedRatingsListByStandard(eventRating, i, NULL);
            break;
        }
    }

    SvString retVal = NULL;
    if (eventRatingsListByStandard) {
        QBPCRating rating = (QBPCRating) SvArrayAt(eventRatingsListByStandard, 0);
        if (rating) {
            retVal = QBPCRatingCreateRatingWithStandardName(rating);
            log_debug("Rating name: %s", SvStringCString(retVal));
        }
        SVRELEASE(eventRatingsListByStandard);
    }

    SVRELEASE(eventRating);
    return retVal;
}

void QBInfoIconsSetByEvent(SvWidget w, SvEPGEvent event)
{
    QBInfoIcons prv = (QBInfoIcons) w->prv;
    if (event) {
        SvString tag = QBInfoIconsCreateEventRatingName(prv->eventsLogic, prv->epgManager, event);
        if (tag) {
            QBInfoIconsSetStateByTag(w, QBInfoIcons_Rating, tag);
            SVRELEASE(tag);
        } else {
            // Set default icon when we have event without any information about ratings.
            QBInfoIconsSetState(w, QBInfoIcons_Rating, QB_INFO_ICONS_DEFAULT_RATING);
        }
    } else {
        QBInfoIconsSetState(w, QBInfoIcons_Rating, QB_INFO_ICONS_DEFAULT_RATING);
    }
}

void QBInfoIconsSetByMovie(SvWidget w, SvDBRawObject movie)
{
    QBInfoIcons prv = (QBInfoIcons) w->prv;
    QBPCRating rating = QBInfoIconsCreateMovieRating(prv, movie);
    QBInfoIconsSetByRating(w, rating);
    SVTESTRELEASE(rating);

    SvValue hdV = (SvValue) SvDBRawObjectGetAttrValue(movie, "high_definition");
    if (hdV && SvObjectIsInstanceOf((SvObject) hdV, SvValue_getType()) && SvValueIsInteger(hdV)) {
        if (SvValueGetInteger(hdV))
            QBInfoIconsSetState(w, QBInfoIcons_HD, 1);
        else
            QBInfoIconsSetState(w, QBInfoIcons_HD, 0);
    } else {
        QBInfoIconsSetState(w, QBInfoIcons_HD, 0);
    }
}

void QBInfoIconsSetByRating(SvWidget w, QBPCRating rating)
{
    SvString ratingName = QBPCRatingCreateRatingWithStandardName(rating);
    if (ratingName) {
        log_debug("Rating name: %s", SvStringCString(ratingName));
        QBInfoIconsSetStateByTag(w, QBInfoIcons_Rating, ratingName);
        SVRELEASE(ratingName);
    } else {
        log_debug("Rating name: %s", SvStringCString(ratingName));
        QBInfoIconsSetState(w, QBInfoIcons_Rating, QB_INFO_ICONS_DEFAULT_RATING);
    }
}
