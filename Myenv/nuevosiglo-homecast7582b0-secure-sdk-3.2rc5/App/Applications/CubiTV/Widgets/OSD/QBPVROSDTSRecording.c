/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBPVROSDTSRecording.h"

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvTime.h>
#include <Windows/pvrplayerInternal.h>
#include <QBOSD/QBOSDRecording.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/icon.h>
#include <SWL/label.h>
#include <math.h>

struct QBPVROSDTSRecording_ {
    struct QBOSDRecording_ super_;

    SvWidget statusIcon;
    SvWidget multiplier;

    int iconHideDelay;

    SvFiber fiber;
    SvFiberTimer timer;

    QBOSDState state;
};

// Private API

SvLocal SvWidget
QBPVROSDTSRecordingWidgetCreate(QBPVROSDTSRecording self,
                                const char *widgetName,
                                SvApplication app,
                                SvErrorInfo *errorOut)
{
    SvWidget w = NULL;
    SvErrorInfo error = NULL;
    char *buf = strdup(widgetName);

    w = svSettingsWidgetCreate(app, buf);
    if (!w)
        goto err;
    free(buf);

    asprintf(&buf, "%s.playState", widgetName);
    self->statusIcon = svIconNew(app, buf);

    SvBitmap bmp = svSettingsGetBitmap(buf, "tv");
    svIconSetBitmap(self->statusIcon, QBPlayState_TV_MODE, bmp);

    bmp = svSettingsGetBitmap(buf, "play");
    svIconSetBitmap(self->statusIcon, QBPlayState_PLAY_MODE, bmp);

    bmp = svSettingsGetBitmap(buf, "pause");
    svIconSetBitmap(self->statusIcon, QBPlayState_PAUSE_MODE, bmp);

    bmp = svSettingsGetBitmap(buf, "pauseNotAvailable");
    svIconSetBitmap(self->statusIcon, QBPlayState_PAUSE_NOT_AVAILABLE_MODE, bmp);

    bmp = svSettingsGetBitmap(buf, "fw");
    svIconSetBitmap(self->statusIcon, QBPlayState_FW_MODE, bmp);

    bmp = svSettingsGetBitmap(buf, "fwNotAvailable");
    svIconSetBitmap(self->statusIcon, QBPlayState_FW_NOT_AVAILABLE_MODE, bmp);

    bmp = svSettingsGetBitmap(buf, "rw");
    svIconSetBitmap(self->statusIcon, QBPlayState_RW_MODE, bmp);


    svSettingsWidgetAttach(w, self->statusIcon, buf, 10);
    svIconSwitch(self->statusIcon, QBPlayState_PLAY_MODE, QBPlayState_TV_MODE, -1);

    self->iconHideDelay = svSettingsGetInteger(buf, "iconHideDelay", 2);

    free(buf);

    asprintf(&buf, "%s.multiplier", widgetName);

    if (svSettingsIsWidgetDefined(buf)) {
        self->multiplier = svLabelNewFromSM(app, buf);
        svSettingsWidgetAttach(w, self->multiplier, buf, 10);
    }
    free(buf);

    return w;

err:
    error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                              "Can't create or attach widget [%s].", buf);
    free(buf);
    svWidgetDestroy(w);
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

// QBPVROSDRecording virtual methods

SvLocal void
QBPVROSDTSRecordingSetEventSource(QBOSDRecording self_,
                                  QBOSDTimeshiftEventSource source,
                                  void *userData,
                                  SvErrorInfo *errorOut)
{
}

SvLocal void
QBPVROSDTSRecordingUpdateEvents(QBOSDRecording self_,
                                SvErrorInfo *errorOut)
{
}

SvLocal void
QBPVROSDTSRecordingUpdateTimes(QBOSDRecording self_,
                               SvTVChannel channel,
                               time_t currentTime,
                               time_t timeshiftStart,
                               time_t timeshiftEnd,
                               SvErrorInfo *errorOut)
{
}

SvLocal void
QBPVROSDTSRecordingScheduler(QBPVROSDTSRecording self)
{
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(self->iconHideDelay, 0));
}

SvLocal void
QBPVROSDTSRecordingUpdatePlaybackState(QBOSDRecording self_,
                                       const SvPlayerTaskState *state,
                                       double wantedSpeed,
                                       SvErrorInfo *errorOut)
{
    QBPVROSDTSRecording self = (QBPVROSDTSRecording) self_;
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
        goto out;
    }

    QBPlayState mode = QBPlayState_PAUSE_MODE;

    if (state) {

        mode = QBPlayState_PLAY_MODE;
        if (state->restrictions.isFFSpeedLimited && state->restrictions.maxFFSpeed < wantedSpeed) {
            mode = QBPlayState_FW_NOT_AVAILABLE_MODE;
            QBPVROSDTSRecordingScheduler(self);
        } else if (wantedSpeed == 0 && state->restrictions.pausingDisallowed) {
            mode = QBPlayState_PAUSE_NOT_AVAILABLE_MODE;
            QBPVROSDTSRecordingScheduler(self);
        }

        if (state->currentSpeed == state->wantedSpeed && state->wantedSpeed == wantedSpeed && SvFiberEventIsArmed(self->timer)) {
            // It's confirmation from player that state was changed.
            return;
        }

        if (mode == QBPlayState_PLAY_MODE) {
            SvFiberDeactivate(self->fiber);
            SvFiberEventDeactivate(self->timer);
            if (state->wantedSpeed < 0) {
                mode = QBPlayState_RW_MODE;
            } else if (state->wantedSpeed > 1) {
                mode = QBPlayState_FW_MODE;
            } else if (state->wantedSpeed == 0) {
                mode = QBPlayState_PAUSE_MODE;
            }
        }

        if (self->multiplier) {
            if (state->wantedSpeed != 1 && state->wantedSpeed != 0) {
                char buf[12];
                snprintf(buf, sizeof(buf), "X%i", (int) fabs(state->wantedSpeed));
                svLabelSetText(self->multiplier, buf);
            } else {
                svLabelSetText(self->multiplier, "");
            }
        }

        self->state = (QBOSDState) {
            .lock = false,
            .show = false,
            .forceShow = false,
            .allowHide = state ? state->wantedSpeed == 1.0 : true
        };
        QBObservableSetChanged((QBObservable) self, &error);
        QBObservableNotifyObservers((QBObservable) self, NULL, &error);
    }

    svIconSwitch(self->statusIcon, mode, QBPlayState_TV_MODE, -1);

out:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBPVROSDTSRecordingSetAvailabilityLimit(QBOSDRecording self_,
                                        SvTime retentionTime,
                                        bool isAvailable,
                                        SvErrorInfo *errorOut)
{
}

// QBOSD virtual methods

SvLocal QBOSDState
QBPVROSDTSRecordingGetState(QBOSD self_,
                            SvErrorInfo *errorOut)
{
    QBPVROSDTSRecording self = (QBPVROSDTSRecording) self_;
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
        goto err;
    }

    return self->state;

err:
    SvErrorInfoPropagate(error, errorOut);
    return (QBOSDState) { .show = false };
}

// SvObject virtual methods

SvLocal void
QBPVROSDTSRecordingDestroy(void *self_)
{
    QBPVROSDTSRecording self = (QBPVROSDTSRecording) self_;
    SvFiberDestroy(self->fiber);
}

// Public API

SvType
QBPVROSDTSRecording_getType(void)
{
    static SvType type = NULL;

    static const struct QBOSDRecordingVTable_ vtable = {
        .super_               = {
            .super_           = {
                .destroy      = QBPVROSDTSRecordingDestroy,
            },
            .getState         = QBPVROSDTSRecordingGetState
        },
        .setEventSource       = QBPVROSDTSRecordingSetEventSource,
        .updateTimes          = QBPVROSDTSRecordingUpdateTimes,
        .updateEvents         = QBPVROSDTSRecordingUpdateEvents,
        .updatePlaybackState  = QBPVROSDTSRecordingUpdatePlaybackState,
        .setAvailabilityLimit = QBPVROSDTSRecordingSetAvailabilityLimit,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBPVROSDTSRecording",
                            sizeof(struct QBPVROSDTSRecording_),
                            QBOSDRecording_getType(), &type,
                            QBOSDRecording_getType(), &vtable,
                            NULL);
    }

    return type;
}

SvLocal void
QBPVROSDTSRecordingStep(void *self_)
{
    QBPVROSDTSRecording self = (QBPVROSDTSRecording) self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    SvPlayerManager man = SvPlayerManagerGetInstance();
    SvPlayerTask playerTask = SvPlayerManagerGetPlayerTask(man, -1);
    if (!playerTask) {
        SvLogError("%s() - cannot get playerTask", __func__);
        return;
    }
    SvPlayerTaskState state = SvPlayerTaskGetState(playerTask);
    QBPVROSDTSRecordingUpdatePlaybackState((QBOSDRecording) self, &state, state.wantedSpeed, NULL);
}

QBPVROSDTSRecording
QBPVROSDTSRecordingCreate(SvApplication app,
                          SvScheduler scheduler,
                          SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBPVROSDTSRecording self = NULL;

    if (!app) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL app passed");
        goto out;
    }

    self = (QBPVROSDTSRecording) SvTypeAllocateInstance(QBPVROSDTSRecording_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain,
                                           SvCoreError_noMemory,
                                           error,
                                           "unable to allocate QBOSDTSRecording instance");
        goto out;
    }

    SvWidget w = QBPVROSDTSRecordingWidgetCreate(self, "OSDPVR", app, &error);
    if (!w) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain,
                                           SvCoreError_invalidState,
                                           error,
                                           "can't create timeshift OSD widget");
        goto out;
    }

    QBOSDRecordingInit((QBOSDRecording) self, w, SVSTRING("OSDPVR"), &error);

    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error, "QBOSDRecording init failed");
        goto out;
    }

    self->fiber = SvFiberCreate(scheduler, NULL, "QBPVRTSRecording", QBPVROSDTSRecordingStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);

out:
    SvErrorInfoPropagate(error, errorOut);
    if (error) {
        SVTESTRELEASE(self);
        return NULL;
    } else
        return self;
}
