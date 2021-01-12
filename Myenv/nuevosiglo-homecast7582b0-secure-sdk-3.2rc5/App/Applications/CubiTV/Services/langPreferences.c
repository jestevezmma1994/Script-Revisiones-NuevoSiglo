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


#include <dataformat/sv_data_format.h>
#include <QBConf.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Logic/SubtitleTrackLogic.h>
#include <string.h>
#include <safeFiles.h>
#include "langPreferences.h"
#include "main.h"

#include <QBDvbSubsManager.h>
#include <QBExtSubsManager.h>
#include <QBClosedCaptionSubsManager.h>
#include <SvJSON/SvJSONParse.h>
#include <Windows/newtv.h>
#include <iso_639_table.h>

#define MAX_SUBTITLES_COUNT 32

#define log_debug(fmt, ...)  if (1) SvLogNotice(COLBEG() " %s:%d " fmt  COLEND_COL(green), __func__,__LINE__,##__VA_ARGS__)

typedef enum {
    QBLangPreferencesMode_subtitlesOverClosedCaptions = 0,
    QBLangPreferencesMode_closedCaptionsOverSubs
} QBLangPreferencesMode;

typedef enum {
    QBClosedCaptioningPreference_CC708_Over_CC608 = 0,
    QBClosedCaptioningPreference_CC608_Over_CC708
} QBClosedCaptioningPreference;

struct QBLangPreferences_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvArray epgLang;
    QBLangPreferencesEPGLang defaultEPGLang;

    SvWeakList subsManagers;
    QBLangPreferencesMode mode;
    QBClosedCaptioningPreference ccPreference;
    SvHashTable ccConfig;
};


SvLocal void QBLangPreferences__dtor__(void *self_)
{
    QBLangPreferences self = self_;
    SVRELEASE(self->epgLang);
    SVRELEASE(self->subsManagers);
    SVTESTRELEASE(self->ccConfig);
}

SvLocal SvString
getMenuLang(void)
{
    const char *menuLang1 = QBConfigGet("LANG");
    if (!menuLang1 || strlen(menuLang1) < 2) {
        return NULL;
    }

    char subLang[3] = { menuLang1[0], menuLang1[1], '\0' };
    const char *menuLang3 = iso639_1to3(subLang);
    if (!menuLang3) {
        return NULL;
    }

    return SvStringCreate(menuLang3, NULL);
}

SvLocal void addLang(SvArray langs, const char *clang)
{
    if (!langs || !clang) {
        return;
    }

    SvString lang = SvStringCreate(clang, NULL);
    SvArrayAddObject(langs, (SvGenericObject)lang);
    SVRELEASE(lang);
}

SvLocal void
QBLangPreferencesUpdateEPGLang(QBLangPreferences self)
{
    SVTESTRELEASE(self->epgLang);
    self->epgLang = SvArrayCreate(NULL);
    SvString menuLang = getMenuLang();
    if (menuLang && QBLangPreferencesEPGLang_menuLang == self->defaultEPGLang) {
        SvArrayAddObject(self->epgLang, (SvGenericObject) menuLang);
    }
    addLang(self->epgLang, QBConfigGet("SUBTITLESLANG"));
    addLang(self->epgLang, QBConfigGet("SUBTITLESLANGSECONDARY"));
    if (menuLang && !SvArrayContainsObject(self->epgLang, (SvGenericObject) menuLang)) {
        SvArrayAddObject(self->epgLang, (SvGenericObject) menuLang);
    }
    SVTESTRELEASE(menuLang);
}

SvLocal void
QBLangPreferencesConfigChanged(SvGenericObject self_, const char *key, const char *value)
{
    QBLangPreferences self = (QBLangPreferences) self_;
    QBLangPreferencesUpdateEPGLang(self);

    bool subsChanged = !key || !strcmp(key, "SUBTITLESLANG") || !strcmp(key, "SUBTITLESLANGSECONDARY") || !strcmp(key, "HARDOFHEARING") || !strcmp(key, "CLOSED_CAPTION_CHANNEL_MODE");

    if (subsChanged) {
        QBLangPreferencesSetPreferedSubtitlesTrack(self, NULL);
    }
}

SvType QBLangPreferences_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBLangPreferences__dtor__
    };
    static SvType type = NULL;
    static const struct QBConfigListener_t config_methods = {
        .changed = QBLangPreferencesConfigChanged
    };
    if (!type) {
        SvTypeCreateManaged("QBLangPreferences",
                            sizeof(struct QBLangPreferences_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBConfigListener_getInterface(), &config_methods,
                            NULL);
    }
    return type;
}

QBLangPreferences QBLangPreferencesCreate(AppGlobals appGlobals)
{
    QBLangPreferences self = (QBLangPreferences)SvTypeAllocateInstance(QBLangPreferences_getType(), NULL);

    self->appGlobals = appGlobals;
    self->subsManagers = SvWeakListCreate(NULL);
    self->mode = QBLangPreferencesMode_subtitlesOverClosedCaptions;
    self->ccPreference = QBClosedCaptioningPreference_CC708_Over_CC608;
    self->defaultEPGLang = QBLangPreferencesEPGLang_default;

    QBLangPreferencesConfigChanged((SvGenericObject)self, NULL, NULL);
    QBConfigAddListener((SvGenericObject) self, "LANG");
    QBConfigAddListener((SvGenericObject) self, "SUBTITLESLANG");
    QBConfigAddListener((SvGenericObject) self, "SUBTITLESLANGSECONDARY");
    QBConfigAddListener((SvGenericObject) self, "HARDOFHEARING");
    QBConfigAddListener((SvGenericObject) self, "CLOSED_CAPTION_CHANNEL_MODE");

    return self;
}

void QBLangPreferencesLoadClosedCaptioningConf(QBLangPreferences self, SvString config)
{
    char *buffer;
    QBFileToBuffer(SvStringCString(config), &buffer);
    if (buffer) {
        self->ccConfig = (SvHashTable) SvJSONParseString(buffer, false, NULL);
        free(buffer);
    }

    if (!(self->ccConfig && SvObjectIsInstanceOf((SvObject) self->ccConfig, SvHashTable_getType()))) {
        SvLogError("%s(): Closed Captioning configuration file '%s' has invalid format", __func__, SvStringCString(config));
        SVTESTRELEASE(self->ccConfig);
        self->ccConfig = NULL;
        return;
    }

    if (self->ccConfig) {
        SvValue preferenceV = (SvValue) SvHashTableFind(self->ccConfig, (SvGenericObject)SVSTRING("preference"));
        if (preferenceV) {
            if (!(SvObjectIsInstanceOf((SvObject) preferenceV, SvValue_getType()) && SvValueIsString(preferenceV))) {
                SvLogError("%s(): invalid format of Closed Captioning configuration file '%s': preference must be a string", __func__, SvStringCString(config));
                return;
            }
            SvString preference = SvValueGetString(preferenceV);
            if (SvStringEqualToCString(preference, "closed captions")) {
                self->mode = QBLangPreferencesMode_closedCaptionsOverSubs;
            }
        }
        SvValue ccPreferenceV = (SvValue) SvHashTableFind(self->ccConfig, (SvGenericObject) SVSTRING("cc_preference"));
        if (ccPreferenceV) {
            if (!(SvObjectIsInstanceOf((SvObject) ccPreferenceV, SvValue_getType()) && SvValueIsString(ccPreferenceV))) {
                SvLogError("%s(): invalid format of Closed Captioning configuration file '%s': cc_preference must be a string",
                        __func__, SvStringCString(config));
                return;
            }
            SvString preference = SvValueGetString(ccPreferenceV);
            if (SvStringEqualToCString(preference, "CC-608")) {
                self->ccPreference = QBClosedCaptioningPreference_CC608_Over_CC708;
            }
        }
    }
}

SvGenericObject QBLangPreferencesGetClosedCaptioningParam(QBLangPreferences self, SvString param)
{
    SvGenericObject ret = NULL;
    if (self->ccConfig) {
        ret = SvHashTableFind(self->ccConfig, (SvGenericObject)param);
    }
    return ret;
}

static bool isHardOfHearing(QBSubsTrack track)
{
    if (SvObjectIsInstanceOf((SvObject) track, QBDvbSubsTrack_getType())) {
        return QBDvbSubsTrackisHardOfHearing((QBDvbSubsTrack) track);
    }
    return false;
}

typedef struct {
    char defLang1[4], defLang2[4], defLang3[4];
    bool hardOfHearing;
} SubtitleCmpData;

static int SubtitleCmp(SubtitleCmpData *data, QBSubsTrack s1, QBSubsTrack s2)
{
#define FI (1)
#define SE (-1)
    if(s1 == s2)
        return 0;

    /// Tweak: only allow HoH tracks, when it is enabled by the user
    if (!data->hardOfHearing) {
        if (isHardOfHearing(s1))
            return SE;
        if (isHardOfHearing(s2))
            return FI;
    }

    if (!s1->langCode && !s2->langCode)
        return 0;

    if (!s1->langCode)
        return SE;

    if (!s2->langCode)
        return FI;

    if (iso639TerminologicalCompare(SvStringCString(s1->langCode), SvStringCString(s2->langCode))) {
        if (!iso639TerminologicalCompare(data->defLang1, SvStringCString(s1->langCode))) {
            return FI;
        }
        if (!iso639TerminologicalCompare(data->defLang1, SvStringCString(s2->langCode))) {
            return SE;
        }
        if (!iso639TerminologicalCompare(data->defLang2, SvStringCString(s1->langCode))) {
            return FI;
        }
        if (!iso639TerminologicalCompare(data->defLang2, SvStringCString(s2->langCode))) {
            return SE;
        }
        if (!iso639TerminologicalCompare(data->defLang3, SvStringCString(s1->langCode))) {
            return FI;
        }
        if (!iso639TerminologicalCompare(data->defLang3, SvStringCString(s2->langCode))) {
            return SE;
        }
        return 0;
    }

    if (SvObjectGetType((SvObject) s1) != SvObjectGetType((SvObject) s2)) {
        if (SvObjectIsInstanceOf((SvObject) s1, QBDvbSubsTrack_getType()))
            return FI;
        if (SvObjectIsInstanceOf((SvObject) s2, QBDvbSubsTrack_getType()))
            return SE;
        return 0;
    }

    bool h1 = isHardOfHearing(s1);
    bool h2 = isHardOfHearing(s2);
    if (h1 != h2) {
        if (data->hardOfHearing == h1)
            return FI;
        if (data->hardOfHearing == h2)
            return SE;
        return 0;
    }
    return 0;
}

static void setLang(char lang[4], const char *pref)
{
    if (!pref){
        lang[0] = 0;
        return;
    }
    strncpy(lang, pref, 4);
    lang[3] = 0;
}

void QBLangPreferencesAddPreferredSubtitlesTrackListener(QBLangPreferences self, QBSubsManager manager)
{
    SvWeakListPushFront(self->subsManagers, (SvGenericObject)manager, NULL);
}

// If there are any external subtitles then return one of them, else return currentBest
SvLocal QBSubsTrack QBLangPreferencesFindExternalSubtitlesTrack(QBLangPreferences self, QBSubsManager manager, QBSubsTrack currentBest)
{
    SvIterator tracksIter = QBActiveArrayIterator(QBSubsManagerGetAllTracks(manager));

    QBSubsTrack currentTrack = NULL;
    while ((currentTrack = (QBSubsTrack) SvIteratorGetNext(&tracksIter))) {
        if (SvObjectIsInstanceOf((SvObject) currentTrack, QBExtSubsTrack_getType())) {
            return currentTrack;
        }
    }

    return currentBest;
}

SvLocal QBSubsTrack QBLangPreferencesFindBestSubtitlesTrack(QBLangPreferences self, QBSubsManager manager, QBSubsTrack currentBest)
{
    QBActiveArray tracks = QBSubsManagerGetAllTracks(manager);

    SubtitleCmpData cmpData;
    SubtitleTrackLogic subtitleTrackLogic =
        (SubtitleTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SubtitleTrackLogic"));
    SvString preferred = SubtitleTrackLogicGetPreferredLang(subtitleTrackLogic);
    if (preferred)
        setLang(cmpData.defLang1, SvStringCString(preferred));
    else
        setLang(cmpData.defLang1, NULL);

    setLang(cmpData.defLang2, QBConfigGet("SUBTITLESLANG"));
    setLang(cmpData.defLang3, QBConfigGet("SUBTITLESLANGSECONDARY"));

    const char *hardOfHearing_ = QBConfigGet("HARDOFHEARING");
    cmpData.hardOfHearing = hardOfHearing_ && !strcmp("1", hardOfHearing_);

    /// find max
    QBSubsTrack bestTrack = currentBest;
    int i;
    for (i=0; i<(int)QBActiveArrayCount(tracks); i++) {
        QBSubsTrack track = (QBSubsTrack)QBActiveArrayObjectAtIndex(tracks, i);
        if (SubtitleCmp(&cmpData, bestTrack, track) < 0)
            bestTrack = track;
    }
    return bestTrack;
}

SvLocal QBSubsTrack QBLangPreferencesFindManualBestClosedCaptionTrack(QBLangPreferences self, QBSubsManager manager, QBSubsTrack currentBest)
{
    const char *ccChannelPri = QBConfigGet("CLOSED_CAPTION_CHANNEL_PRI");
    const char *ccChannelSec = QBConfigGet("CLOSED_CAPTION_CHANNEL_SEC");
    const char *ccPriLang = QBConfigGet("CLOSED_CAPTION_LANG_PRI");
    const char *ccSecLang = QBConfigGet("CLOSED_CAPTION_LANG_PRI");

    if (!ccChannelPri || !ccChannelSec || !ccPriLang || !ccSecLang) {
        SvLogError("%s: QBConfigGet returned NULL",__func__);
        return currentBest;
    }

    QBActiveArray ccTracks = QBSubsManagerGetClosedCaptionsTracks(manager);
    SvIterator itTrack = QBActiveArrayIterator(ccTracks);
    QBSubsTrack track = NULL;
    QBSubsTrack track608Pri = NULL;
    QBSubsTrack track608Sec = NULL;
    QBSubsTrack track708Pri = NULL;
    QBSubsTrack track708Sec = NULL;

    while ((track = (QBSubsTrack) SvIteratorGetNext(&itTrack))) {
        if (SvObjectIsInstanceOf((SvObject) track, QBDTVCCTrack_getType())) {
            if (!strcmp(SvStringCString(track->langCode), ccPriLang)) {
                track708Pri = track;
            } else if (!strcmp(SvStringCString(track->langCode), ccSecLang)) {
                track708Sec = track;
            }
        }
        if (SvObjectIsInstanceOf((SvObject) track, QBAnalogCCTrack_getType())) {
            if (!strcmp(QBAnalogCCTrackGetChannelName((SvGenericObject) track), ccChannelPri)) {
                track608Pri = track;
            } else if (!strcmp(QBAnalogCCTrackGetChannelName((SvGenericObject) track), ccChannelSec)) {
                track608Sec = track;
            }
        }
    }

    if (self->ccPreference == QBClosedCaptioningPreference_CC708_Over_CC608) {
        if (track708Pri)
            return track708Pri;
        if (track708Sec)
            return track708Sec;
        if (track608Pri)
            return track608Pri;
        if (track608Sec)
            return track608Sec;
    } else if (self->ccPreference == QBClosedCaptioningPreference_CC608_Over_CC708) {
        if (track608Pri)
            return track608Pri;
        if (track608Sec)
            return track608Sec;
        if (track708Pri)
            return track708Pri;
        if (track708Sec)
            return track708Sec;
    }

    return currentBest;
}

SvLocal QBSubsTrack QBLangPreferencesFindAutoBestClosedCaptionTrack(QBLangPreferences self, QBSubsManager manager, QBSubsTrack currentBest)
{
    QBSubsTrack bestTrack = currentBest;

    const char *ccPreferredChannel = SubtitleTrackLogicCCGetPreferredChannel();
    const char *ccPreferredLang = SubtitleTrackLogicCCGetPreferredLang();
    const char *ccPriLang = QBConfigGet("CLOSED_CAPTION_LANG_PRI");

    if (!ccPreferredChannel || !ccPreferredLang || !ccPriLang) {
        SvLogError("%s One of configuration is NULL",__func__);
        return currentBest;
    }

    QBActiveArray ccTracks = QBSubsManagerGetClosedCaptionsTracks(manager);
    SvIterator itTrack = QBActiveArrayIterator(ccTracks);
    QBSubsTrack track = NULL;
    QBSubsTrack track608CC1 = NULL;
    QBSubsTrack track608CC3 = NULL;
    QBSubsTrack track708Menu = NULL;
    QBSubsTrack track708Pri = NULL;

    while ((track = (QBSubsTrack) SvIteratorGetNext(&itTrack))) {
        if (SvObjectIsInstanceOf((SvObject) track, QBDTVCCTrack_getType())) {
            if (!strcmp(SvStringCString(track->langCode), ccPreferredLang)) {
                track708Menu = track;
            } else if (!strcmp(SvStringCString(track->langCode), ccPriLang)) {
                track708Pri = track;
            }
        }
        if (SvObjectIsInstanceOf((SvObject) track, QBAnalogCCTrack_getType())) {
            if (!strcmp(QBAnalogCCTrackGetChannelName((SvGenericObject) track), "CC1")) {
                track608CC1 = track;
            } else if (!strcmp(QBAnalogCCTrackGetChannelName((SvGenericObject) track), "CC3")) {
                track608CC3 = track;
            }
        }
    }

    if (self->ccPreference == QBClosedCaptioningPreference_CC708_Over_CC608) {
        if (track708Menu)
            return track708Menu;
        if (track708Pri)
            return track708Pri;
        if (!strcmp(ccPreferredChannel, "CC1")) {
            if (track608CC1)
                return track608CC1;
            if (track608CC3)
                return track608CC3;
        } else {
            if (track608CC3)
                return track608CC3;
            if (track608CC1)
                return track608CC1;
        }
    } else if (self->ccPreference == QBClosedCaptioningPreference_CC608_Over_CC708) {
        if (!strcmp(ccPreferredChannel, "CC1")) {
            if (track608CC1)
                return track608CC1;
            if (track608CC3)
                return track608CC3;
        } else {
            if (track608CC3)
                return track608CC3;
            if (track608CC1)
                return track608CC1;
        }
        if (track708Menu)
            return track708Menu;
        if (track708Pri)
            return track708Pri;
    }

    return bestTrack;
}

SvLocal QBSubsTrack QBLangPreferencesFindBestClosedCaptionTrack(QBLangPreferences self, QBSubsManager manager, QBSubsTrack currentBest)
{
    QBSubsTrack bestTrack = currentBest;
    const char * mode = QBConfigGet("CLOSED_CAPTION_CHANNEL_MODE");
    if (mode == NULL)
        return currentBest;

    if (strcmp(mode, "Automatic") == 0) {
        bestTrack = QBLangPreferencesFindAutoBestClosedCaptionTrack(self, manager, currentBest);
    } else if (strcmp(mode, "Manual") == 0) {
        bestTrack = QBLangPreferencesFindManualBestClosedCaptionTrack(self, manager, currentBest);
    }
    return bestTrack;
}

SvLocal void QBLangPreferencesSetPreferedSubtitlesTrack_(void *target, QBSubsManager manager)
{
    QBLangPreferences self = (QBLangPreferences) target;

    QBSubsTrack bestTrack = QBSubsManagerGetNullTrack(manager);

    switch (self->mode) {
    case QBLangPreferencesMode_closedCaptionsOverSubs:
        bestTrack = QBLangPreferencesFindBestClosedCaptionTrack(self, manager, bestTrack);
        if (bestTrack == QBSubsManagerGetNullTrack(manager)) {
            bestTrack = QBLangPreferencesFindBestSubtitlesTrack(self, manager, bestTrack);
        }
        break;
    case QBLangPreferencesMode_subtitlesOverClosedCaptions:
    default:
        bestTrack = QBLangPreferencesFindBestSubtitlesTrack(self, manager, bestTrack);
        if (bestTrack == QBSubsManagerGetNullTrack(manager)) {
            bestTrack = QBLangPreferencesFindBestClosedCaptionTrack(self, manager, bestTrack);
        }
        break;
    }

    if (bestTrack == QBSubsManagerGetNullTrack(manager)) {
        bestTrack = QBLangPreferencesFindExternalSubtitlesTrack(self, manager, bestTrack);
    }

    QBSubsManagerSetCurrentTrack(manager, bestTrack);
}

void QBLangPreferencesSetPreferedSubtitlesTrack(void *target, QBSubsManager manager)
{
    QBLangPreferences self = (QBLangPreferences) target;

    if (manager)
        QBLangPreferencesSetPreferedSubtitlesTrack_(self, manager);
    else {
        SvIterator it = SvWeakListIterator(self->subsManagers);
        QBSubsManager subsManager;
        while((subsManager = (QBSubsManager)SvIteratorGetNext(&it))) {
            QBLangPreferencesSetPreferedSubtitlesTrack_(self, subsManager);
        }
    }
}

SvString
QBLangPreferencesGetTitleFromEvent(QBLangPreferences self, SvEPGEvent ev)
{
    SvEPGEventDesc desc = QBLangPreferencesGetDescFromEvent(self, ev);
    return desc && desc->title ? desc->title : SVSTRING("");
}

SvEPGEventDesc
QBLangPreferencesGetDescFromEvent_(SvGenericObject self_, SvEPGEvent event)
{
    return QBLangPreferencesGetDescFromEvent((QBLangPreferences) self_, event);
}

SvEPGEventDesc
QBLangPreferencesGetDescFromEvent(QBLangPreferences self, SvEPGEvent event)
{
    if (!self || !event)
        return NULL;
    SvEPGEventDesc desc = NULL;

    if (SvArrayCount(self->epgLang) > 0) {
        SvString langCode = NULL;
        SvIterator it = SvArrayIterator(self->epgLang);
        while ((langCode = (SvString) SvIteratorGetNext(&it))) {
            desc = SvEPGEventGetDescription(event, langCode);
            if (desc)
                return desc;
        }
    }

    // Use any lang
    desc = SvEPGEventGetAnyDescription(event);
    return desc;
}

void
QBLangPreferencesSetDefaultEPGLang(QBLangPreferences self, QBLangPreferencesEPGLang defaultEPGLang)
{
    if (self->defaultEPGLang == defaultEPGLang) {
        return;
    }
    self->defaultEPGLang = defaultEPGLang;
    QBLangPreferencesUpdateEPGLang(self);
}
