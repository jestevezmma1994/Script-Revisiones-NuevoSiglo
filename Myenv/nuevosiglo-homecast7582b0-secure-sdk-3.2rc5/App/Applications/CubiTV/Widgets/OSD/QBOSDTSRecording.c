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

#include "QBOSDTSRecording.h"

#include <Logic/timeFormat.h>
#include <Utils/QBEventUtils.h>
#include <Widgets/QBRoundedLabel.h>
#include <QBDRMManager/QBDRMManagerListener.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <QBOSD/QBOSDRecording.h>
#include <settings.h>
#include <SWL/QBFrame.h>
#include <SWL/icon.h>
#include <SWL/label.h>
#include <SWL/gauge.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <libintl.h>

typedef enum {
   TV_MODE = 0,
   PLAY_MODE,
   PAUSE_MODE,
   PAUSE_NOT_AVAILABLE_MODE,
   FW_MODE,
   FW_NOT_AVAILABLE_MODE,
   RW_MODE
} QBOSDTSRecordingMode;

struct QBOSDTSRecording_ {
    struct QBOSDRecording_ super_;

    EventsLogic eventsLogic;
    QBLangPreferences langPreferences;

    SvWidget currentEventBg;
    SvWidget currentEventCaption, nextEventCaption;
    SvWidget currentEventStamp, nextEventStamp, endEventStamp;
    SvWidget arrow, arrowTS, timeLabelTS;
    SvWidget timeshiftCurrent, timeshiftNext;
    SvWidget timeshiftInformation;

    SvEPGEvent currentEvent;
    SvEPGEvent nextEvent;
    SvEPGEvent currentEventInRange;
    SvEPGEvent nextEventInRange;

    SvWidget statusIcon;
    SvWidget multiplier;
    int progressMargin;

    QBOSDTimeshiftEventSource eventSourceFun;
    void *eventSourceArg;

    int arrowOffsetPrevious;
    int arrowZeroPositionOffset;
    int iconHideDelay;

    SvFiber fiber;
    SvFiberTimer timer;

    QBOSDState state;
};

// Private API

SvLocal void
QBOSDTSRecordingSetStatus(QBOSDTSRecording self,
                          QBOSDTSRecordingMode mode,
                          int speed)
{
    static const char *const modeNames[] = {
        [TV_MODE]                  = "tv",
        [PLAY_MODE]                = "play",
        [PAUSE_MODE]               = "pause",
        [PAUSE_NOT_AVAILABLE_MODE] = "pauseNotAvailable",
        [FW_MODE]                  = "fw",
        [FW_NOT_AVAILABLE_MODE]    = "fwNotAvailable",
        [RW_MODE]                  = "rw"
    };

    if (mode == TV_MODE) {
        svWidgetSetHidden(self->timeLabelTS, true);
        svWidgetSetHidden(self->arrowTS, true);
    }

    SVAUTOSTRING(modeTag, modeNames[mode]);
    svIconSwitchByTag(self->statusIcon, modeTag, 0, 0.2);

    if ((mode ==  FW_MODE || mode == RW_MODE) && abs(speed) >= 2) {
        char *buf = NULL;
        asprintf(&buf, "X%d", abs(speed));
        svLabelSetText(self->multiplier, buf);
        free(buf);
    } else {
        svLabelSetText(self->multiplier, "");
    }
}

SvLocal int
QBOSDTSRecordingCalculateArrowOffset(QBOSDTSRecording self,
                                     time_t passedTime,
                                     time_t eventLength)
{
    return passedTime * (self->currentEventBg->width - 2 * self->progressMargin) / eventLength
        + self->progressMargin + self->arrowZeroPositionOffset;
}

SvLocal void
QBOSDTSRecordingUpdateTimeshiftGauge(QBOSDTSRecording self, time_t timeshiftStart, time_t timeshiftEnd)
{
    if (timeshiftStart == 0 || !self->currentEvent) {
        svWidgetSetHidden(self->timeshiftCurrent, true);
        svWidgetSetHidden(self->timeshiftNext, true);
        return;
    }

    time_t upperBound = self->currentEvent->endTime;
    time_t lowerBound = self->currentEvent->startTime;
    time_t tss = (timeshiftStart < lowerBound) ? lowerBound : timeshiftStart;
    time_t tse = (timeshiftEnd > upperBound) ? upperBound : timeshiftEnd;

    upperBound -= lowerBound;
    tss -= lowerBound;
    tse -= lowerBound;
    lowerBound = 0;

    svGaugeSetBounds(self->timeshiftCurrent, lowerBound, upperBound);
    svGaugeSetInterval(self->timeshiftCurrent, tss, tse);
    svWidgetSetHidden(self->timeshiftCurrent, false);

    if (self->nextEvent) {
        upperBound = self->nextEvent->endTime;
        lowerBound = self->nextEvent->startTime;
        tss = (timeshiftStart < lowerBound) ? lowerBound : timeshiftStart;
        tse = (timeshiftEnd > upperBound) ? upperBound : timeshiftEnd;

        upperBound -= lowerBound;
        tss -= lowerBound;
        tse -= lowerBound;
        lowerBound = 0;

        svGaugeSetBounds(self->timeshiftNext, lowerBound, upperBound);
        svGaugeSetInterval(self->timeshiftNext, tss, tse);
        svWidgetSetHidden(self->timeshiftNext, false);
    } else {
        svWidgetSetHidden(self->timeshiftNext, true);
    }
}

SvLocal void
QBOSDTSRecordingUpdateArrow(QBOSDTSRecording self,
                            time_t currentTime,
                            bool isTimeshiftEnabled)
{
    // update arrows visibility
    SvTimeRange timeRange;
    SvTimeRangeInit(&timeRange, 0, 0);
    if (self->currentEvent) {
        SvEPGEventGetTimeRange(self->currentEvent, &timeRange, NULL);
    }

    if (SvTimeRangeContainsTimePoint(&timeRange, currentTime)) {
        svWidgetSetHidden(self->arrow, isTimeshiftEnabled);
        svWidgetSetHidden(self->arrowTS, !isTimeshiftEnabled);
        svWidgetSetHidden(self->timeLabelTS, !isTimeshiftEnabled);
    } else {
        svWidgetSetHidden(self->arrow, true);
        svWidgetSetHidden(self->arrowTS, true);
        svWidgetSetHidden(self->timeLabelTS, true);
        self->arrow->off_x = 0;
        self->arrowTS->off_x = 0;
        self->timeLabelTS->off_x = 0;
        return; // return if we don't have self->currentEvent or currentTime is not in self->currentEvent time range
    }

    // calculate arrows offset
    unsigned currentEventLength = (unsigned) SvEPGEventGetLength(self->currentEvent);
    int passedTime = (unsigned) currentTime - (unsigned) self->currentEvent->startTime;
    int arrowOffset = QBOSDTSRecordingCalculateArrowOffset(self, passedTime, currentEventLength);

    // check range of arrowOffset
    if (arrowOffset < self->arrowZeroPositionOffset) {
        arrowOffset = self->arrowZeroPositionOffset;
    } else if (arrowOffset >= (self->currentEventBg->width - self->progressMargin + self->arrowZeroPositionOffset)) {
        arrowOffset = self->currentEventBg->width - self->progressMargin + self->arrowZeroPositionOffset - 1;
    }

    // update arrows offset
    self->arrow->off_x = arrowOffset - self->arrow->width / 2;
    self->arrowTS->off_x = arrowOffset - self->arrowTS->width / 2;
    self->timeLabelTS->off_x = arrowOffset;

    // prepare timeshift text label time
    char buf[256];
    struct tm brokenDownTime;
    SvTimeBreakDown(SvTimeConstruct(currentTime, 0), true, &brokenDownTime);
    strftime(buf, sizeof(buf), QBTimeFormatGetCurrent()->tvOsdTime, &brokenDownTime);
    SvString currentTimeString = SvStringCreate(buf, NULL);
    QBRoundedLabelSetText(self->timeLabelTS, currentTimeString);
    SVRELEASE(currentTimeString);

    // force render if offsets changed
    if (self->arrowOffsetPrevious != arrowOffset && isTimeshiftEnabled) {
        svWidgetForceRender(self->arrowTS);
        svWidgetForceRender(self->timeLabelTS);
        self->arrowOffsetPrevious = arrowOffset;
    }
}

SvLocal void
QBOSDTSRecordingSetAvailabilityLimit(QBOSDRecording self_,
                                     SvTime availabilityTime,
                                     bool isAvailable,
                                     SvErrorInfo *errorOut);

SvLocal SvWidget
QBOSDTSRecordingWidgetCreate(QBOSDTSRecording self,
                             const char *widgetName,
                             SvApplication app,
                             QBTextRenderer renderer,
                             QBDRMManager drmManager,
                             SvErrorInfo *errorOut)
{
    SvWidget w = NULL;
    SvErrorInfo error = NULL;

    char *buf = strdup(widgetName); // strdup slightly simplifies error handling
    if (!(w = svSettingsWidgetCreate(app, buf)))
        goto err;
    free(buf);

    asprintf(&buf, "%s.currentTimeStamp", widgetName);
    if (!(self->currentEventStamp = svLabelNewFromSM(app, buf))
        || svSettingsWidgetAttach(w, self->currentEventStamp, buf, 1) == 0)
        goto err;
    free(buf);

    asprintf(&buf, "%s.nextTimeStamp", widgetName);
    if (!(self->nextEventStamp = svLabelNewFromSM(app, buf))
        || svSettingsWidgetAttach(w, self->nextEventStamp, buf, 1) == 0)
        goto err;
    free(buf);

    asprintf(&buf, "%s.endTimeStamp", widgetName);
    if (!(self->endEventStamp = svLabelNewFromSM(app, buf))
        ||svSettingsWidgetAttach(w, self->endEventStamp, buf, 1) == 0)
        goto err;
    free(buf);

    SvWidget eventsArea = NULL;
    asprintf(&buf, "%s.progressArea", widgetName);
    if (!(eventsArea = svSettingsWidgetCreate(app, buf))
        || svSettingsWidgetAttach(w, eventsArea, buf, 8) == 0)
        goto err;
    free(buf);

    SvWidget eventsViewport = NULL;
    asprintf(&buf, "%s.viewport", widgetName);
    if (!(eventsViewport = svSettingsWidgetCreate(app, buf))
        || svSettingsWidgetAttach(eventsArea, eventsViewport, buf, 5) == 0)
        goto err;
    free(buf);

    asprintf(&buf, "%s.currentEvent", widgetName);
    if (!(self->currentEventBg = QBFrameCreateFromSM(app, buf)))
        goto err;
    svSettingsWidgetAttach(eventsViewport, self->currentEventBg, buf, 1);
    free(buf);

    SvWidget nextEventBg = NULL;
    asprintf(&buf, "%s.nextEvent", widgetName);
    if (!(nextEventBg = QBFrameCreateFromSM(app, buf)))
        goto err;
    svSettingsWidgetAttach(eventsViewport, nextEventBg, buf, 1);
    free(buf);

    asprintf(&buf, "%s.currentEventCaption", widgetName);
    if (!(self->currentEventCaption = QBAsyncLabelNew(app, buf, renderer))
        || svSettingsWidgetAttach(eventsArea, self->currentEventCaption, buf, 10) == 0)
        goto err;
    free(buf);

    asprintf(&buf, "%s.nextEventCaption", widgetName);
    if (!(self->nextEventCaption = QBAsyncLabelNew(app, buf, renderer))
        || svSettingsWidgetAttach(eventsArea, self->nextEventCaption, buf, 10) == 0)
        goto err;
    free(buf);

    asprintf(&buf, "%s.timeshiftCurrent", widgetName);
    if (!(self->timeshiftCurrent = svGaugeNewFromSM(app, buf, SV_GAUGE_HORIZONTAL, SV_GAUGE_STYLE_BASIC, 0, 1, 0))
        || svSettingsWidgetAttach(eventsArea, self->timeshiftCurrent, buf, 9) == 0)
        goto err;
    free(buf);

    asprintf(&buf, "%s.timeshiftNext", widgetName);
    if (!(self->timeshiftNext = svGaugeNewFromSM(app, buf, SV_GAUGE_HORIZONTAL, SV_GAUGE_STYLE_BASIC, 0, 1, 0))
        || svSettingsWidgetAttach(eventsArea, self->timeshiftNext, buf, 9) == 0)
        goto err;
    free(buf);

    asprintf(&buf, "%s.timeMark", widgetName);
    if (!(self->arrow = svSettingsWidgetCreate(app, buf))
        || svSettingsWidgetAttach(eventsArea, self->arrow, buf, 20) == 0)
        goto err;
    free(buf);

    asprintf(&buf, "%s.timeLabelTS", widgetName);
    if (!(self->timeLabelTS = QBRoundedLabelNew(app, buf))
        || svSettingsWidgetAttach(eventsArea, self->timeLabelTS, buf, 25) == 0)
        goto err;
    free(buf);

    asprintf(&buf, "%s.timeMarkTS", widgetName);
    if (!(self->arrowTS = svSettingsWidgetCreate(app, buf))
        || svSettingsWidgetAttach(eventsArea, self->arrowTS, buf, 20) == 0)
        goto err;
    free(buf);

    asprintf(&buf, "%s.status", widgetName);
    self->progressMargin = svSettingsGetInteger(buf, "margin", 0);
    if (!(self->statusIcon = svIconNew(app, buf))
        || svSettingsWidgetAttach(w, self->statusIcon, buf, 1) == 0)
        goto err;
    self->iconHideDelay = svSettingsGetInteger(buf, "iconHideDelay", 2);
    free(buf);

    asprintf(&buf, "%s.multiplier", widgetName);
    if (!(self->multiplier = svLabelNewFromSM(app, buf))
        || svSettingsWidgetAttach(w, self->multiplier, buf, 1) == 0)
        goto err;
    free(buf);

    asprintf(&buf, "%s.timeshiftInformation", widgetName);
    if (!(self->timeshiftInformation = QBAsyncLabelNew(app, buf, renderer))
        || svSettingsWidgetAttach(w, self->timeshiftInformation, buf, 1) == 0)
        goto err;
    free(buf);

    const QBDRMManagerData* data = QBDRMManagerGetDRMManagerData(drmManager);
    QBOSDTSRecordingSetAvailabilityLimit((QBOSDRecording) self, data->availabilityTime, data->isAvailable, &error);

    self->arrowZeroPositionOffset = eventsViewport->off_x;
    svWidgetSetHidden(self->arrow, true);
    svWidgetSetHidden(self->arrowTS, true);
    svWidgetSetHidden(self->timeLabelTS, true);

    return w;

err:
    error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                              "Can't create or attach widget [%s].", buf);
    free(buf);
    svWidgetDestroy(w);
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

// QBOSDRecording virtual methods

SvLocal void
QBOSDTSRecordingSetEventSource(QBOSDRecording self_,
                               QBOSDTimeshiftEventSource source,
                               void *userData,
                               SvErrorInfo *errorOut)
{
    QBOSDTSRecording self = (QBOSDTSRecording) self_;
    SvErrorInfo error = NULL;

    if (!source || !userData) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed [source = %p, userData = %p]",
                                  source, userData);
        goto out;
    }

    self->eventSourceFun = source;
    self->eventSourceArg = userData;

out:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBOSDTSRecordingUpdateEvents(QBOSDRecording self_,
                             SvErrorInfo *errorOut)
{
    QBOSDTSRecording self = (QBOSDTSRecording) self_;

    char buf[256] = { 0 };
    struct tm tmp = { .tm_sec = 0 };

    if (self->currentEvent) {
        SvTimeBreakDown(SvTimeConstruct(self->currentEvent->startTime, 0), true, &tmp);
        strftime(buf, sizeof(buf), QBTimeFormatGetCurrent()->tvOsdTime, &tmp);
        svLabelSetText(self->currentEventStamp, buf);
        SvString title = QBEventUtilsGetTitleFromEvent(self->eventsLogic,
                                                       self->langPreferences,
                                                       self->currentEvent);
        QBAsyncLabelSetText(self->currentEventCaption, title);
    } else {
        svLabelSetText(self->currentEventStamp,"");
        QBAsyncLabelSetText(self->currentEventCaption, SVSTRING(""));
    }

    if (self->nextEvent) {
        SvTimeBreakDown(SvTimeConstruct(self->nextEvent->startTime, 0), true, &tmp);
        strftime(buf, sizeof(buf), QBTimeFormatGetCurrent()->tvOsdTime, &tmp);
        svLabelSetText(self->nextEventStamp, buf);
        SvTimeBreakDown(SvTimeConstruct(self->nextEvent->endTime, 0), true, &tmp);
        strftime(buf, sizeof(buf), QBTimeFormatGetCurrent()->tvOsdTime, &tmp);
        svLabelSetText(self->endEventStamp, buf);
        SvString title = QBEventUtilsGetTitleFromEvent(self->eventsLogic,
                                                       self->langPreferences,
                                                       self->nextEvent);
        QBAsyncLabelSetText(self->nextEventCaption, title);
    } else {
        svLabelSetText(self->nextEventStamp, "");
        svLabelSetText(self->endEventStamp, "");
        QBAsyncLabelSetText(self->nextEventCaption, SVSTRING(""));
    }
}

SvLocal void
QBOSDTSRecordingUpdateTimes(QBOSDRecording self_,
                            SvTVChannel channel,
                            time_t currentTime,
                            time_t timeshiftStart,
                            time_t timeshiftEnd,
                            SvErrorInfo *errorOut)
{
    QBOSDTSRecording self = (QBOSDTSRecording) self_;

    SvEPGEvent currentEvent = NULL;
    SvEPGEvent nextEvent = NULL;
    SvValue channelID = NULL;

    bool isTimeshiftEnabled = timeshiftStart > 0;

    if (channel && self->eventSourceFun) {
        self->eventSourceFun(self->eventSourceArg, channel, currentTime, &currentEvent, &nextEvent);
        channelID = SvTVChannelGetID(channel);
    }

    SvTimeRange timeRange;
    SvTimeRangeInit(&timeRange, 0, 0);
    if (self->currentEventInRange)
        SvEPGEventGetTimeRange(self->currentEventInRange, &timeRange, NULL);

    if (currentEvent) {
        if (self->currentEventInRange != currentEvent) {
            SVTESTRELEASE(self->currentEventInRange);
            self->currentEventInRange = SVRETAIN(currentEvent);
        }

        if (self->nextEventInRange != nextEvent) {
            SVTESTRELEASE(self->nextEventInRange);
            self->nextEventInRange = SVTESTRETAIN(nextEvent);
        }
    } else if (SvTimeRangeContainsTimePoint(&timeRange, currentTime)
               && SvObjectEquals((SvObject) channelID, (SvObject) self->currentEventInRange->channelID)) {
        currentEvent = self->currentEventInRange;
        nextEvent = self->nextEventInRange;
    } else {
        SVTESTRELEASE(self->currentEventInRange);
        self->currentEventInRange = NULL;

        SVTESTRELEASE(self->nextEventInRange);
        self->nextEventInRange = NULL;
    }

    if (currentEvent != self->currentEvent) {
        SVTESTRELEASE(self->currentEvent);

        //Zero length events are uninteresting
        if (currentEvent && (SvEPGEventGetLength(currentEvent) == 0
                             || currentEvent->startTime > SvTimeNow())) {
            currentEvent = NULL;
        }

        self->currentEvent = SVTESTRETAIN(currentEvent);
        QBOSDTSRecordingUpdateEvents((QBOSDRecording) self, NULL);
    }

    if (nextEvent != self->nextEvent) {
        SVTESTRELEASE(self->nextEvent);

        //Zero length events are uninteresting
        if (nextEvent && SvEPGEventGetLength(nextEvent) == 0) {
            nextEvent = NULL;
        }

        self->nextEvent = SVTESTRETAIN(nextEvent);
        QBOSDTSRecordingUpdateEvents((QBOSDRecording) self, NULL);
    }

    if (isTimeshiftEnabled)
        QBOSDTSRecordingUpdateArrow(self, currentTime, true);
    else
        QBOSDTSRecordingUpdateArrow(self, currentTime, false);

    QBOSDTSRecordingUpdateTimeshiftGauge(self, timeshiftStart, timeshiftEnd);
}

SvLocal void
QBOSDTSRecordingScheduleIconHide(QBOSDTSRecording self)
{
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(self->iconHideDelay, 0));
}

SvLocal void
QBOSDTSRecordingUpdatePlaybackState(QBOSDRecording self_,
                                    const SvPlayerTaskState *state,
                                    double wantedSpeed,
                                    SvErrorInfo *errorOut)
{
    QBOSDTSRecording self = (QBOSDTSRecording) self_;

    // TV_MODE is the default value when state is NULL
    QBOSDTSRecordingMode mode = TV_MODE;
    int speed = 0;

    if (state) {
        speed = state->wantedSpeed;
        if (state->timeshift.enabled && state->restrictions.isFFSpeedLimited && state->restrictions.maxFFSpeed < wantedSpeed) {
            mode = FW_NOT_AVAILABLE_MODE;
            QBOSDTSRecordingScheduleIconHide(self);
        }

        if (wantedSpeed == 0 && state->restrictions.pausingDisallowed) {
            mode = PAUSE_NOT_AVAILABLE_MODE;
            QBOSDTSRecordingScheduleIconHide(self);
        }


        if (state->currentSpeed == state->wantedSpeed && state->wantedSpeed == wantedSpeed && SvFiberEventIsArmed(self->timer)) {
            // It's confirmation from player that state was changed.
            return;
        }

        if (mode == TV_MODE && state->timeshift.enabled) {
            SvFiberDeactivate(self->fiber);
            SvFiberEventDeactivate(self->timer);
            if (state->wantedSpeed < 0) {
                mode = RW_MODE;
            } else if (state->wantedSpeed > 1) {
                mode = FW_MODE;
            } else if (state->wantedSpeed == 0) {
                mode = PAUSE_MODE;
            } else {
                mode = PLAY_MODE;
            }
        }
    }

    self->state = (QBOSDState) {
        .lock      = mode==PAUSE_MODE || mode==RW_MODE || mode==FW_MODE,
        .show      = true,
        .forceShow = false,
        .allowHide = state ? state->wantedSpeed == 1.0 : true
    };

    QBOSDTSRecordingSetStatus(self, mode, speed);

    QBObservableSetChanged((QBObservable) self, NULL);
    QBObservableNotifyObservers((QBObservable) self, NULL, NULL);
}

SvLocal void
QBOSDTSRecordingSetAvailabilityLimit(QBOSDRecording self_,
                                     SvTime availabilityTime,
                                     bool isAvailable,
                                     SvErrorInfo *errorOut)
{
    QBOSDTSRecording self = (QBOSDTSRecording) self_;
    int availabilitySeconds = SvTimeGetSeconds(availabilityTime);
    if (!isAvailable) {
        // XXX: Timeshift disallowed - this label is temporary solution
        SvString label = SvStringCreate(gettext("TSH disallowed"), NULL);
        QBAsyncLabelSetText(self->timeshiftInformation, label);
        SVRELEASE(label);
    } else if (availabilitySeconds > 0) {
        int hour = availabilitySeconds / 3600;
        int min = availabilitySeconds / 60 % 60;
        SvString label = SvStringCreateWithFormat(gettext("T: %dh%02d'"), hour, min);
        QBAsyncLabelSetText(self->timeshiftInformation, label);
        SVRELEASE(label);
    } else {
        QBAsyncLabelSetText(self->timeshiftInformation, SVSTRING(""));
    }
}

// QBOSD virtual methods

SvLocal QBOSDState
QBOSDTSRecordingGetState(QBOSD self_,
                         SvErrorInfo *errorOut)
{
    QBOSDTSRecording self = (QBOSDTSRecording) self_;

    return self->state;
}

// SvObject virtual methods

SvLocal void
QBOSDTSRecordingDestroy(void *self_)
{
    QBOSDTSRecording self = self_;

    SVRELEASE(self->eventsLogic);
    SVRELEASE(self->langPreferences);

    SVTESTRELEASE(self->currentEvent);
    SVTESTRELEASE(self->nextEvent);
    SVTESTRELEASE(self->currentEventInRange);
    SVTESTRELEASE(self->nextEventInRange);
    SvFiberDestroy(self->fiber);
}

// Public API

SvType
QBOSDTSRecording_getType(void)
{
    static SvType type = NULL;

    static const struct QBOSDRecordingVTable_ vtable = {
        .super_               = {
            .super_           = {
                .destroy      = QBOSDTSRecordingDestroy,
            },
            .getState         = QBOSDTSRecordingGetState
        },
        .setEventSource       = QBOSDTSRecordingSetEventSource,
        .updateTimes          = QBOSDTSRecordingUpdateTimes,
        .updateEvents         = QBOSDTSRecordingUpdateEvents,
        .updatePlaybackState  = QBOSDTSRecordingUpdatePlaybackState,
        .setAvailabilityLimit = QBOSDTSRecordingSetAvailabilityLimit,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBOSDTSRecording",
                            sizeof(struct QBOSDTSRecording_),
                            QBOSDRecording_getType(), &type,
                            QBOSDRecording_getType(), &vtable,
                            NULL);
    }

    return type;
}

SvLocal void
QBOSDTSRecordingStep(void *self_)
{
    QBOSDTSRecording self = (QBOSDTSRecording) self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    SvPlayerManager man = SvPlayerManagerGetInstance();
    SvPlayerTask playerTask = SvPlayerManagerGetPlayerTask(man, -1);
    if (!playerTask) {
        SvLogError("%s() - cannot get playerTask", __func__);
        return;
    }
    SvPlayerTaskState state = SvPlayerTaskGetState(playerTask);
    QBOSDTSRecordingUpdatePlaybackState((QBOSDRecording) self, &state, state.wantedSpeed, NULL);
}

QBOSDTSRecording
QBOSDTSRecordingCreate(SvApplication app,
                       const char *widgetName,
                       SvScheduler scheduler,
                       QBTextRenderer renderer,
                       EventsLogic eventsLogic,
                       QBLangPreferences langPreferences,
                       QBDRMManager drmManager,
                       SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBOSDTSRecording self = NULL;

    if (!app || !widgetName || !renderer || !eventsLogic || !langPreferences || !drmManager) {
        error = SvErrorInfoCreate(SvCoreErrorDomain,
                                  SvCoreError_invalidArgument,
                                  "NULL argument passed : [\n"
                                  "\tapp             = %p,\n"
                                  "\twidgetName      = %s,\n"
                                  "\trenderer        = %p,\n"
                                  "\teventsLogic     = %p,\n"
                                  "\tlangPreferences = %p\n]"
                                  "\tDRMManager      = %p\n]",
                                  app, widgetName, renderer,
                                  eventsLogic, langPreferences, drmManager);
        goto out;
    }

    self = (QBOSDTSRecording) SvTypeAllocateInstance(QBOSDTSRecording_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain,
                                           SvCoreError_noMemory,
                                           error,
                                           "unable to allocate QBOSDTSRecording instance");
        goto out;
    }

    SvWidget w = QBOSDTSRecordingWidgetCreate(self, widgetName, app, renderer, drmManager, &error);
    if (!w) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain,
                                           SvCoreError_invalidState,
                                           error,
                                           "can't create timeshift OSD widget");
        goto out;
    }

    self->eventsLogic = SVRETAIN(eventsLogic);
    self->langPreferences = SVRETAIN(langPreferences);

    self->fiber = SvFiberCreate(scheduler, NULL, "QBOSDTSRecording", QBOSDTSRecordingStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);

    SvString id = SvStringCreate(widgetName, NULL);
    QBOSDRecordingInit((QBOSDRecording) self, w, id, &error);
    SVRELEASE(id);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error, "QBOSDRecording init failed");
        goto out;
    }

out:
    SvErrorInfoPropagate(error, errorOut);
    if (error) {
        SVTESTRELEASE(self);
        return NULL;
    } else
        return self;
}
