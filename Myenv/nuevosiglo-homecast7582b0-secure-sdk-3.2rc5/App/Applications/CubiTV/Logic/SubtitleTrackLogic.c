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

#include <Logic/SubtitleTrackLogic.h>
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <QBConf.h>

#include <QBDvbSubsManager.h>
#include <QBTeletextSubtitlesManager.h>
#include <SvPlayerControllers/SvPlayerSubsTrackTS.h>
#include <QBScte27Subs.h>
#include <QBClosedCaptionSubsManager.h>

#include <SvPlayerControllers/SvPlayerTrackControllerListener.h>
#include <SvPlayerControllers/SvPlayerTaskControllersListener.h>

#include <QBAppKit/QBAsyncService.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <stdbool.h>

struct SubtitleTrackLogic_ {
    struct SvObject_ super_;

    QBAsyncServiceState serviceState;
    SvImmutableArray serviceDeps;

    SvPlayerTrackController subtitlesTrackController;

    SvPlayerTaskControllers playerTaskControllers;

    SvString preferredLang;
};


SvLocal SvType SubtitleTrackLogic_getType(void);
SvLocal SvPlayerSubsTrackTStype SubtitleTrackLogicGetTypeFromTrackTS(QBSubsTrack track);

// QBAsyncService related stuff
SvLocal SvString
SubtitleTrackLogicGetName(SvObject self_)
{
    return SVSTRING("SubtitleTrackLogic");
}

SvLocal SvImmutableArray
SubtitleTrackLogicGetDependencies(SvObject self_)
{
    return ((SubtitleTrackLogic) self_)->serviceDeps;
}

SvLocal QBAsyncServiceState
SubtitleTrackLogicGetState(SvObject self_)
{
    SubtitleTrackLogic self = (SubtitleTrackLogic) self_;
    return self->serviceState;
}

SvLocal void
SubtitleTrackLogicStart(SvObject self_, SvScheduler scheduler, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    SubtitleTrackLogic self = (SubtitleTrackLogic) self_;
    self->serviceState = QBAsyncServiceState_running;

    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
SubtitleTrackLogicStop(SvObject self_, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    SubtitleTrackLogic self = (SubtitleTrackLogic) self_;
    self->serviceState = QBAsyncServiceState_idle;

    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
SubtitleTrackLogicRemoveTaskControllersListener(SubtitleTrackLogic self)
{
    if (self->playerTaskControllers) {
        if (self->subtitlesTrackController) {
            SvPlayerTrackControllerRemoveListener(self->subtitlesTrackController, (SvObject) self);
        }
        SvPlayerTaskControllersRemoveListener(self->playerTaskControllers, (SvObject) self);
    }
}

SvLocal void
SubtitleTrackLogicAddTaskControllersListener(SubtitleTrackLogic self)
{
    if (self->playerTaskControllers) {
        if (self->subtitlesTrackController) {
            SvPlayerTrackControllerAddListener(self->subtitlesTrackController, (SvObject) self, NULL);
        }
        SvPlayerTaskControllersAddListener(self->playerTaskControllers, (SvObject) self, NULL);
    }
}

SvLocal void
SubtitleTrackLogicDestroy(void *self_)
{
    SubtitleTrackLogic self = self_;
    SVTESTRELEASE(self->preferredLang);
    SVRELEASE(self->serviceDeps);

    SubtitleTrackLogicRemoveTaskControllersListener(self);
    SVTESTRELEASE(self->subtitlesTrackController);
    SVTESTRELEASE(self->playerTaskControllers);
}


// SvPlayerTaskControllersListener methods

SvLocal void
SubtitleTrackLogicControllersUpdated(SvObject self_)
{
    SubtitleTrackLogic self = (SubtitleTrackLogic) self_;

    SvPlayerTrackController currentSubsTrackController =
        SvPlayerTaskControllersGetTrackController(self->playerTaskControllers, SvPlayerTrackControllerType_subs);
    if (self->subtitlesTrackController != currentSubsTrackController) {
        SubtitleTrackLogicRemoveTaskControllersListener(self);
        SVTESTRELEASE(self->subtitlesTrackController);

        self->subtitlesTrackController = SVTESTRETAIN(currentSubsTrackController);
        SubtitleTrackLogicAddTaskControllersListener(self);
    }
}

// SvPlayerTrackControllerListener methods

SvLocal void
SubtitleTrackLogicTracksUpdated(SvObject self, SvPlayerTrackController controller)
{
    assert(SvObjectIsInstanceOf((SvObject) self, SubtitleTrackLogic_getType()));

    SubtitleTrackLogic this = (SubtitleTrackLogic) self;
    QBSubsTrack track = QBSubsManagerGetCurrentTrack((QBSubsManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                 SVSTRING("QBSubsManager")));
    if (likely(track)) {
        SubtitleTrackLogicSubsTrackChanged(this, track);
    }
}

SvLocal void
SubtitleTrackLogicCurrentTrackChanged(SvObject self_, SvPlayerTrackController controller, unsigned int idx)
{
}

SvLocal SvType
SubtitleTrackLogic_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = SubtitleTrackLogicDestroy
    };

    static const struct QBAsyncService_ asyncServiceMethods = {
        .getName         = SubtitleTrackLogicGetName,
        .getDependencies = SubtitleTrackLogicGetDependencies,
        .getState        = SubtitleTrackLogicGetState,
        .start           = SubtitleTrackLogicStart,
        .stop            = SubtitleTrackLogicStop
    };
    static const struct SvPlayerTaskControllersListener_ taskControllersListenerMethods = {
        .controllersUpdated = SubtitleTrackLogicControllersUpdated,
    };
    static const struct SvPlayerTrackControllerListener_ trackMethods = {
        .tracksUpdated       = SubtitleTrackLogicTracksUpdated,
        .currentTrackChanged = SubtitleTrackLogicCurrentTrackChanged,
    };

    static SvType t = NULL;
    if (!t) {
        t = SvTypeCreateManaged("SubtitleTrackLogic",
                                sizeof(struct SubtitleTrackLogic_),
                                SvObject_getType(),
                                &t,
                                SvObject_getType(), &objectVTable,
                                QBAsyncService_getInterface(), &asyncServiceMethods,
                                SvPlayerTaskControllersListener_getInterface(), &taskControllersListenerMethods,
                                SvPlayerTrackControllerListener_getInterface(), &trackMethods,
                                NULL);
    }
    return t;
}

SubtitleTrackLogic SubtitleTrackLogicCreate(AppGlobals appGlobals)
{
    SubtitleTrackLogic self = (SubtitleTrackLogic) SvTypeAllocateInstance(SubtitleTrackLogic_getType(), NULL);

    self->preferredLang = NULL;
    self->playerTaskControllers = NULL;
    self->serviceDeps = SvImmutableArrayCreateWithTypedValues("s", NULL, "QBSubsManager");

    return self;
}

void SubtitleTrackLogicSetPlayerTaskControllers(SubtitleTrackLogic self, SvPlayerTaskControllers controllers)
{
    // remove old listener first
    SubtitleTrackLogicRemoveTaskControllersListener(self);
    SVTESTRELEASE(self->playerTaskControllers);
    self->playerTaskControllers = NULL;
    SVTESTRELEASE(self->subtitlesTrackController);
    self->subtitlesTrackController = NULL;

    self->playerTaskControllers = SVTESTRETAIN(controllers);
    if (self->playerTaskControllers) {
        SvPlayerTrackController currentSubsTrackController =
            SvPlayerTaskControllersGetTrackController(self->playerTaskControllers, SvPlayerTrackControllerType_subs);
        self->subtitlesTrackController = SVTESTRETAIN(currentSubsTrackController);
        SubtitleTrackLogicAddTaskControllersListener(self);
    }
}

void SubtitleTrackLogicSetPreferredLang(SubtitleTrackLogic self, SvString lang)
{
    SVTESTRETAIN(lang);
    SVTESTRELEASE(self->preferredLang);
    self->preferredLang = lang;
}

SvString SubtitleTrackLogicGetPreferredLang(SubtitleTrackLogic self)
{
    return self->preferredLang;
}

const char *SubtitleTrackLogicCCGetPreferredChannel(void)
{
    return "CC1";
}

const char *SubtitleTrackLogicCCGetPreferredLang(void)
{
    return QBConfigGet("CLOSED_CAPTION_LANG_PRI");
}


SvLocal int SubtitleTrackLogicFindTrack(SubtitleTrackLogic self, QBSubsTrack track)
{
    if (!self->subtitlesTrackController) {
        SvLogError("%s(): Unable to select subtitle track, we don't have subtitle controller", __func__);
        return -1;
    }

    SvArray tracks = SvPlayerTrackControllerGetTracks(self->subtitlesTrackController);
    if (unlikely(!tracks)) {
        SvLogError("unable to select subtitle track, subtitle controller does't have any tracks");
        return -1;
    }

    unsigned subsTrackId = track->id;
    SvPlayerSubsTrack currTrack;
    SvIterator it = SvArrayGetIterator(tracks);
    unsigned result = -1;
    unsigned i = 0;

    while ((currTrack = (SvPlayerSubsTrack) SvIteratorGetNext(&it))) {
        if (((SvPlayerTrack) currTrack)->id == subsTrackId) {
            result = i;
            break;
        }

        ++i;
    }

    return result;
}

SvLocal SvPlayerSubsTrackTStype SubtitleTrackLogicGetTypeFromTrackTS(QBSubsTrack track)
{
    if (SvObjectIsInstanceOf((SvObject) track, QBDvbSubsTrack_getType())) {
        return SvPlayerSubsTrackTStype_dvb;
    } else if (SvObjectIsInstanceOf((SvObject) track, QBTeletextSubtitlesTrack_getType())) {
        return SvPlayerSubsTrackTStype_ttx;
    } else if (SvObjectIsInstanceOf((SvObject) track, QBScte27SubsTrack_getType())) {
        return SvPlayerSubsTrackTStype_scte27;
    } else if (SvObjectIsInstanceOf((SvObject) track, QBAnalogCCTrack_getType()) || SvObjectIsInstanceOf((SvObject) track, QBDTVCCTrack_getType())) {
        return SvPlayerSubsTrackTStype_cc;
    } else {
        return SvPlayerSubsTrackTStype_unknown;
    }
}


void SubtitleTrackLogicSubsTrackChanged(SubtitleTrackLogic self, QBSubsTrack track)
{
    SvPlayerSubsTrackTStype type = SubtitleTrackLogicGetTypeFromTrackTS(track);
    int idx;

    if (type == SvPlayerSubsTrackTStype_unknown) {
        if (SvObjectIsInstanceOf((SvObject) track, QBSubsTrack_getType()) && SvStringEqualToCString(track->langCode, "off")) {
            idx = DISABLE_SUBS_TRACK;
        } else {
            SvLogError("%s() invalid parameters: subs type unknown and lang %s", __func__, SvStringGetCString(track->langCode));
            return;
        }
    } else {
        idx = SubtitleTrackLogicFindTrack(self, track);
        if (idx == -1) {
            idx = DISABLE_SUBS_TRACK;
        }
    }

    if (!self->playerTaskControllers) {
        SvLogError("%s(): no player task controllers structure to act on!", __func__);
        return;
    }

    if (unlikely(!self->subtitlesTrackController)) {
        SvLogError("%s() no subtitle controller to act on!", __func__);
        return;
    }

    SvPlayerTrackControllerSetTrackByIndex(self->subtitlesTrackController, idx);
}

SvPlayerTaskControllers SubtitleTrackLogicGetPlayerTaskControllers(SubtitleTrackLogic self)
{
    return self->playerTaskControllers;
}
