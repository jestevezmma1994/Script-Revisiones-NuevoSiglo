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

#include <SvPlayerControllers/SvPlayerTrackController.h>
#include <SvPlayerControllers/SvPlayerTrackControllerListener.h>
#include <SvPlayerControllers/SvPlayerTaskControllersListener.h>
#include <Logic/AudioTrackLogic.h>
#include <Logic/QBLogicFactory.h>
#include <SvFoundation/SvString.h>
#include <SvCore/SvErrnoDomain.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <libintl.h>
#include <dataformat/sv_data_format.h>
#include <QBConf.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <SvJSON/SvJSONParse.h>
#include <iso_639_table.h>
#include <main.h>
#include <string.h> // memset
#include <errno.h>

#include <fibers/c/fibers.h>


SvLocal inline int
AudioTrackLogicSelectAudioTrack(AudioTrackLogic self)
{
    return SvInvokeVirtual(AudioTrackLogic, self, selectAudioTrack);
}

SvLocal bool mappingElementIsValid(SvHashTable elem)
{
    SvObject name = SvHashTableFind(elem, (SvObject) SVSTRING("name"));
    if (!name || !SvObjectIsInstanceOf(name, SvValue_getType()) || !SvValueIsString((SvValue) name)) {
        SvLogError("Audio lang map configuration: invalid mapping name");
        return false;
    }
    SvObject codes = SvHashTableFind(elem, (SvObject) SVSTRING("iso639codes"));

    if (!codes || !SvObjectIsInstanceOf(codes, SvArray_getType())) {
        return false;
    }

    SvIterator iter = SvArrayIterator((SvArray) codes);
    SvObject code;
    while ((code = SvIteratorGetNext(&iter))) {
        if (!SvObjectIsInstanceOf(code, SvValue_getType()) || !SvValueIsString((SvValue) code)) {
            return false;
        }
    }

    return true;
}

SvLocal void AudioTrackLogicLoadLangMap(AudioTrackLogic self, SvString path)
{
    char fileName[PATH_MAX];
    snprintf(fileName, PATH_MAX - 1, "%s%s", SvGetRuntimePrefix(), SvStringCString(path));

    SvErrorInfo error = NULL;
    SvObject root = SvJSONParseFile(fileName, false, &error);
    if (error) {
        if (SvErrorInfoEquals(error, SvErrnoDomain, ENOENT)) {
            SvErrorInfoDestroy(error);
            SvLogWarning("%s(): '%s' no such file", __func__, fileName);
        } else {
            SvErrorInfoPropagate(error, NULL);
        }
        return;
    }
    if (!root) {
        SvLogError("Audio lang map configuration: invalid format. Using defaults.");
        return;
    }

    if (!SvObjectIsInstanceOf(root, SvHashTable_getType())) {
        SvLogError("Audio lang map configuration: invalid format; elements not found. Using defaults.");
        SVRELEASE(root);
        return;
    }

    SvObject obj = SvHashTableFind((SvHashTable) root, (SvObject) SVSTRING("mapping"));

    if ((!obj) || (!SvObjectIsInstanceOf(obj, SvArray_getType()))) {
        SvLogError("Audio lang map configuration: invalid elements format. Using defaults.");
        SVRELEASE(root);
        return;
    }

    SvArray mappings = (SvArray) obj;
    SvIterator iter = SvArrayIterator(mappings);
    SvObject elem;

    while ((elem = SvIteratorGetNext(&iter))) {
        if (SvObjectIsInstanceOf(elem, SvHashTable_getType())) {
            if (!mappingElementIsValid((SvHashTable) elem)) {
                SvLogError("Audio lang map configuration: invalid entry. Using defaults.");
                SVRELEASE(root);
                return;
            }
        } else {
            SVRELEASE(root);
            return;
        }
    }

    self->langMap = SvHashTableCreate(3, NULL);

    iter = SvArrayIterator(mappings);
    while ((elem = SvIteratorGetNext(&iter))) {
        SvIterator codesIter = SvArrayIterator((SvArray) SvHashTableFind((SvHashTable) elem, (SvObject) SVSTRING("iso639codes")));
        SvObject codeElem;
        SvString name = SvValueGetString((SvValue) SvHashTableFind((SvHashTable) elem, (SvObject) SVSTRING("name")));
        while ((codeElem = SvIteratorGetNext(&codesIter))) {
            SvHashTableInsert(self->langMap, (SvObject) SvValueGetString((SvValue) codeElem), (SvObject) name);
        }
    }

    SVRELEASE(root);
}

SvLocal bool
AudioTrackLogicIsMultiChannelModeActive(AudioTrackLogic self)
{
    const char *multiChannelModeStr = QBConfigGet("MULTICHANNELAUDIO");
    return (multiChannelModeStr && !strcmp(multiChannelModeStr, "YES"));
}

SvLocal SvString
AudioTrackLogicGetName(SvObject self_)
{
    return SVSTRING("AudioTrackLogic");
}

SvLocal SvImmutableArray
AudioTrackLogicGetDependencies(SvObject self_)
{
    return NULL;
}

SvLocal QBAsyncServiceState
AudioTrackLogicGetState(SvObject self_)
{
    AudioTrackLogic self = (AudioTrackLogic) self_;
    return self->serviceState;
}


SvLocal void
AudioTrackLogicStart(SvObject self_, SvScheduler scheduler, SvErrorInfo *errorOut)
{
    AudioTrackLogic self = (AudioTrackLogic) self_;
    self->serviceState = QBAsyncServiceState_running;
}

SvLocal void
AudioTrackLogicStop(SvObject self_, SvErrorInfo *errorOut)
{
    AudioTrackLogic self = (AudioTrackLogic) self_;
    self->serviceState = QBAsyncServiceState_idle;
}

void
AudioTrackLogicSetPlayerTaskControllers(AudioTrackLogic self, SvPlayerTaskControllers controllers)
{
    // remove old listener and release old controllers
    if (self->audioTrackController) {
        SvPlayerTrackControllerRemoveListener(self->audioTrackController, (SvObject) self);
        SVRELEASE(self->audioTrackController);
        self->audioTrackController = NULL;
    }

    if (self->playerTaskControllers) {
        SvPlayerTaskControllersRemoveListener(self->playerTaskControllers, (SvObject) self);
        SVRELEASE(self->playerTaskControllers);
        self->playerTaskControllers = NULL;
    }

    // set new controllers and add listeners to new controllers objects
    if (controllers) {
        // player task controller
        self->playerTaskControllers = SVRETAIN(controllers);
        SvPlayerTaskControllersAddListener(self->playerTaskControllers, (SvObject) self, NULL);

        // audio controller
        SvPlayerTrackController currentAudioTrackController =
            SvPlayerTaskControllersGetTrackController(self->playerTaskControllers, SvPlayerTrackControllerType_audio);

        if (currentAudioTrackController) {
            self->audioTrackController = SVRETAIN(currentAudioTrackController);
            SvPlayerTrackControllerAddListener(self->audioTrackController, (SvObject) self, NULL);
        }
   }
}

SvPlayerTaskControllers AudioTrackLogicGetPlayerTaskControllers(AudioTrackLogic self)
{
    return self->playerTaskControllers;
}

SvLocal void
AudioTrackLogicDestroy(void *self_)
{
    AudioTrackLogic self = self_;

    // remove listeners
    if (self->audioTrackController) {
        SvPlayerTrackControllerRemoveListener(self->audioTrackController, (SvObject) self);
    }

    if (self->playerTaskControllers) {
        SvPlayerTaskControllersRemoveListener(self->playerTaskControllers, (SvObject) self);
    }

    // release all objects
    SVTESTRELEASE(self->preferredTrack);
    SVTESTRELEASE(self->audioTrackController);
    SVTESTRELEASE(self->playerTaskControllers);
    SVTESTRELEASE(self->langMap);
}

SvLocal bool
AudioTrackLogicSetPreferredAudioTrack_(AudioTrackLogic self, SvPlayerAudioTrack audioTrack)
{
    // Accept the preference
    return AudioTrackLogicSetAudioTrack(self, audioTrack);
}

SvLocal void
AudioTrackLogicControllersUpdated(SvObject self_)
{
    AudioTrackLogic self = (AudioTrackLogic) self_;

    // update audio track controller
    SvPlayerTrackController currentAudioTrackController =
        SvPlayerTaskControllersGetTrackController(self->playerTaskControllers, SvPlayerTrackControllerType_audio);

    if (self->audioTrackController != currentAudioTrackController) {
        // release previous one and set new audio controller
        if (self->audioTrackController) {
            SvPlayerTrackControllerRemoveListener(self->audioTrackController, (SvObject) self);
            SVRELEASE(self->audioTrackController);
            self->audioTrackController = NULL;
        }

        if (currentAudioTrackController) {
            self->audioTrackController = SVRETAIN(currentAudioTrackController);
            SvPlayerTrackControllerAddListener(self->audioTrackController, (SvObject) self, NULL);
        }
    }
}

SvLocal void
AudioTrackLogicTracksUpdated(SvObject self_, SvPlayerTrackController controller)
{
    AudioTrackLogic self = (AudioTrackLogic) self_;

    AudioTrackLogicSetupAudioTrack(self);
}

SvLocal void
AudioTrackLogicCurrentTrackChanged(SvObject self, SvPlayerTrackController controller, unsigned int idx)
{
}

SvLocal int
AudioTrackLogicSelectAudioTrack_(AudioTrackLogic self);

SvType
AudioTrackLogic_getType(void)
{
    static SvType type = NULL;

    static const struct AudioTrackLogicVTable_ logicVTable = {
        .super_                                = {
            .destroy                           = AudioTrackLogicDestroy
        },
        .audioTrackLogicSetPreferredAudioTrack = AudioTrackLogicSetPreferredAudioTrack_,
        .selectAudioTrack                      = AudioTrackLogicSelectAudioTrack_,
    };

    static struct QBAsyncService_ asyncServiceMethods = {
        .getName         = AudioTrackLogicGetName,
        .getDependencies = AudioTrackLogicGetDependencies,
        .getState        = AudioTrackLogicGetState,
        .start           = AudioTrackLogicStart,
        .stop            = AudioTrackLogicStop
    };

    static const struct SvPlayerTaskControllersListener_ taskControllersListenerMethods = {
        .controllersUpdated = AudioTrackLogicControllersUpdated,
    };

    static const struct SvPlayerTrackControllerListener_ trackControllerListenerMethods = {
        .tracksUpdated       = AudioTrackLogicTracksUpdated,
        .currentTrackChanged = AudioTrackLogicCurrentTrackChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateVirtual("AudioTrackLogic",
                            sizeof(struct AudioTrackLogic_),
                            SvObject_getType(),
                            sizeof(logicVTable),
                            &logicVTable,
                            &type,
                            QBAsyncService_getInterface(), &asyncServiceMethods,
                            SvPlayerTaskControllersListener_getInterface(), &taskControllersListenerMethods,
                            SvPlayerTrackControllerListener_getInterface(), &trackControllerListenerMethods,
                            NULL);
    }

    return type;
}

AudioTrackLogic
AudioTrackLogicCreate(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    AudioTrackLogic self = NULL;

    SvType logicClass =
        QBLogicFactoryFindImplementationOf(QBLogicFactoryGetInstance(), AudioTrackLogic_getType(), &error);
    if (!logicClass) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidArgument, error,
                                           "unable to find audio track logic");
        goto out;
    }

    self = (AudioTrackLogic) SvTypeAllocateInstance(logicClass, &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "unable to allocate %s", SvTypeGetName(logicClass));
        goto out;
    }

    self->preferredTrack = NULL;
    AudioTrackLogicLoadLangMap(self, SVSTRING("/usr/local/share/CubiTV/Configurations/audioLangMap"));

out:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

bool AudioTrackLogicSetupAudioTrack(AudioTrackLogic self)
{
    if (unlikely(!self->playerTaskControllers)) {
        SvLogError("unable to select audio track, no SvPlayerTask controller");
        return false;
    }

    if (self->audioTrackController) {
        int trackIndex = AudioTrackLogicSelectAudioTrack(self);
        SvPlayerTrackControllerSetTrackByIndex(self->audioTrackController, trackIndex);
    }

    return true;
}

bool AudioTrackLogicSetAudioTrack(AudioTrackLogic self, SvPlayerAudioTrack audioTrack)
{
    SVTESTRELEASE(self->preferredTrack);
    self->preferredTrack = SVTESTRETAIN(audioTrack);
    return AudioTrackLogicSetupAudioTrack(self);
}

static const char *getPrettyCodecName(QBAudioCodec codec)
{
    static const char *names[] = {
        [QBAudioCodec_aac] = "AAC",
        [QBAudioCodec_heaac] = "HE-AAC",
        [QBAudioCodec_ac3] = "DD",
        [QBAudioCodec_eac3] = "DD+",
        [QBAudioCodec_dts] = "DTS"
    };
    if (codec < sizeof(names) / sizeof(names[0]))
        return names[codec];
    return NULL;
}

SvArray AudioTrackLogicComputeLangMenuItems(AudioTrackLogic self)
{
    SvArray languages = SvArrayCreate(NULL);
    char captionBuffer[127];

    if (unlikely(!self->playerTaskControllers)) {
        SvLogError("unable to compute language menu items, no SvPlayerTask controller");
        return languages;
    }

    if (!self->audioTrackController)
        return languages;

    SvArray tracks = SvPlayerTrackControllerGetTracks(self->audioTrackController);

    for (size_t i = 0; i < SvArrayGetCount(tracks); i++) {
        SvPlayerAudioTrack track = (SvPlayerAudioTrack) SvArrayGetObjectAtIndex(tracks, i);

        QBAudioType type = SvPlayerAudioTrackGetAudioType(track);
        if (type != QBAudioType_unknown &&
            type != QBAudioType_effects &&
            type != QBAudioType_hearing_imp &&
            type != QBAudioType_visually_imp)
            continue;

        size_t len;

        // create new node and fill its attributes
        SvHashTable audioNode = SvHashTableCreate(7, NULL);
        SvHashTableInsert(audioNode, (SvObject) SVSTRING("audioTrack"), (SvObject) track);

        SvString lang = SvPlayerAudioTrackGetLang(track);
        const char *translatedLang = NULL;
        if (self->langMap && lang) {
            SvString mappedLang = (SvString) SvHashTableFind(self->langMap, (SvObject) lang);
            translatedLang = mappedLang ? gettext(SvStringCString(mappedLang)) : NULL;
        }
        if (!translatedLang) {
            const char *origLang = lang > 0 ? iso639GetLangName(SvStringGetCString(lang)) : NULL;
            translatedLang = origLang ? dgettext("iso-codes", origLang) : NULL;
        }
        if (!translatedLang)
            len = snprintf(captionBuffer, sizeof(captionBuffer), gettext("Track %d"), (int) i + 1);
        else
            len = snprintf(captionBuffer, sizeof(captionBuffer), "%s", gettext(translatedLang));
        if (type == QBAudioType_hearing_imp)
            len += snprintf(captionBuffer + len, sizeof(captionBuffer) - len, " %s", gettext("(hard of hearing)"));
        else if (type == QBAudioType_visually_imp)
            len += snprintf(captionBuffer + len, sizeof(captionBuffer) - len, " %s", gettext("(visually impaired)"));
        const char *codecName = getPrettyCodecName(SvPlayerAudioTrackGetAudioCodec(track));
        if (codecName) {
            len += snprintf(captionBuffer + len, sizeof(captionBuffer) - len, " %s", codecName);
            if (SvPlayerAudioTrackIsMultiChannel(track))
                len += snprintf(captionBuffer + len, sizeof(captionBuffer) - len, " 5.1");
        }
        SvString caption = SvStringCreateWithCStringAndLength(captionBuffer, len, NULL);
        SvHashTableInsert(audioNode, (SvObject) SVSTRING("caption"), (SvObject) caption);
        SVRELEASE(caption);

        SvArrayAddObject(languages, (SvObject) audioNode);
        SVRELEASE(audioNode);
    }

    return languages;
}

static int filterTracksByLanguage(SvPlayerAudioTrack *selectedTracks, size_t totalCnt, const char *lang)
{
    size_t cnt = 0;
    for (size_t i = 0; i < totalCnt; i++) {
        SvString trackLang = SvPlayerAudioTrackGetLang(selectedTracks[i]);
        if (!trackLang)
            continue;
        if (iso639TerminologicalCompare(SvStringGetCString(trackLang), lang) == 0)
            selectedTracks[cnt++] = selectedTracks[i];
    }
    return cnt;
}

SvLocal int
AudioTrackLogicSelectAudioTrack_(AudioTrackLogic self)
{
    if (unlikely(!self->playerTaskControllers)) {
        SvLogError("unable to select audio track, no SvPlayerTask controller");
        return false;
    }

    if (!self->audioTrackController)
        return -1;

    SvArray allTracks = SvPlayerTrackControllerGetTracks(self->audioTrackController);
    size_t i, j;

    // first: check current preference
    if (self->preferredTrack) {
        int trackIdx = SvArrayIndexOfObject(allTracks, (SvObject) self->preferredTrack);
        if (trackIdx >= 0)
            return trackIdx;
    }

    size_t totalCnt = SvArrayGetCount(allTracks);
    if (totalCnt == 0)
        return -1;
    else if (totalCnt == 1)
        return 0;

    SvPlayerAudioTrack selectedTracks[totalCnt];
    size_t cnt = 0;

    for (i = 0; i < totalCnt; ++i)
        selectedTracks[i] = (SvPlayerAudioTrack) SvArrayGetObjectAtIndex(allTracks, i);

    // top priority criteria: track type
    QBAudioType trackType = QBAudioType_unknown;
    const char *audioTrackTypeStr = QBConfigGet("AUDIOTRACKTYPE");
    if (audioTrackTypeStr && !strcasecmp(audioTrackTypeStr, "hard-of-hearing"))
        trackType = QBAudioType_hearing_imp;
    else if (audioTrackTypeStr && !strcasecmp(audioTrackTypeStr, "visually-impaired"))
        trackType = QBAudioType_visually_imp;

    cnt = 0;
    if (trackType == QBAudioType_hearing_imp || trackType == QBAudioType_visually_imp) {
        for (i = 0; i < totalCnt; ++i) {
            QBAudioType t = SvPlayerAudioTrackGetAudioType(selectedTracks[i]);
            if (t == trackType)
                selectedTracks[cnt++] = selectedTracks[i];
        }
    } else {
        for (i = 0; i < totalCnt; ++i) {
            QBAudioType t = SvPlayerAudioTrackGetAudioType(selectedTracks[i]);
            if (t != QBAudioType_hearing_imp && t != QBAudioType_visually_imp)
                selectedTracks[cnt++] = selectedTracks[i];
        }
    }

    if (cnt == 1)
        return SvArrayIndexOfObjectIdenticalTo(allTracks, (SvObject) selectedTracks[0]);
    else if (cnt > 1)
        totalCnt = cnt;

    // next criteria: track language
    const char *languages[2];
    languages[0] = QBConfigGet("AUDIOLANG");
    languages[1] = QBConfigGet("AUDIOLANGSECONDARY");
    for (i = 0; i < 2; i++) {
        if (!languages[i])
            continue;
        cnt = filterTracksByLanguage(selectedTracks, totalCnt, languages[i]);
        if (cnt == 1) {
            return SvArrayIndexOfObjectIdenticalTo(allTracks, (SvObject) selectedTracks[0]);
        } else if (cnt > 1) {
            totalCnt = cnt;
            break;
        }
    }

    // next criteria: select multi-channel or non-multi-channel
    bool multiChannelMode = AudioTrackLogicIsMultiChannelModeActive(self);
    cnt = 0;
    for (i = 0; i < totalCnt; i++) {
        bool isMultiChannel = SvPlayerAudioTrackIsMultiChannel(selectedTracks[i]);
        if (multiChannelMode == isMultiChannel)
            selectedTracks[cnt++] = selectedTracks[i];
    }
    if (cnt == 1)
        return SvArrayIndexOfObjectIdenticalTo(allTracks, (SvObject) selectedTracks[0]);
    else if (cnt > 1)
        totalCnt = cnt;

    // last criteria: select the most advanced codec first
    static const QBAudioCodec codecs[] = {
        QBAudioCodec_heaac, QBAudioCodec_aac, QBAudioCodec_eac3,
        QBAudioCodec_dts,   QBAudioCodec_ac3, QBAudioCodec_mpeg
    };
    for (i = 0; i < (int) (sizeof(codecs) / sizeof(codecs[0])); i++) {
        for (j = 0; j < totalCnt; j++) {
            SvPlayerAudioTrack track = selectedTracks[j];
            if (SvPlayerAudioTrackGetAudioCodec(track) == codecs[i])
                return SvArrayIndexOfObjectIdenticalTo(allTracks, (SvObject) selectedTracks[j]);
        }
    }

    // all tracks are exactly the same, just select the first one
    return SvArrayIndexOfObjectIdenticalTo(allTracks, (SvObject) selectedTracks[0]);
}
