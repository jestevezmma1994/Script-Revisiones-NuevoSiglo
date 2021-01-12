/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBConaxRetentionLimitMonitor.h"
#include "QBConaxURIManager.h"
#include "QBConaxPVRManager.h"
#include <SvFoundation/SvObject.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvEnv.h>
#include <QBICSmartcardURIData.h>
#include <QBICSmartcardPVRMetadata.h>
#include <fibers/c/fibers.h>
#include <QBCAS.h>
#include <QBRecordFS/file.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBTimeDateMonitor.h>

#include <SvPlayerManager/SvPlayerTask.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <QBViewport.h>
#include <main.h>

#include <stdbool.h>
#include <inttypes.h>
#include <time.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, "QBConaxRetentionLimitMonitorLogLevel", "");
SV_DECL_INT_ENV_FUN_DEFAULT(env_timeshift_limit_divisor, 0, "QBConaxRetentionLimitMonitorTimeshiftLimitDivisor", "");

#define log_error(fmt, ...) do { if (env_log_level() >= 1) { SvLogError(COLBEG() "%s : " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } } while (0)
#define log_state(fmt, ...) do { if (env_log_level() >= 2) { SvLogNotice(COLBEG() "%s : " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } } while (0)
#define log_info(fmt, ...) do { if (env_log_level() >= 3) { SvLogNotice(COLBEG() "%s : " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)
#define log_fun(fmt, ...) do { if (env_log_level() >= 4) { SvLogNotice(COLBEG() "%s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)

#define REC_META_KEY__LAST_TOUCH_TIME_UTC          "rec:last_touch_time_utc"          // time_t

typedef enum QBConaxRetentionLimitMonitorMode_e {
    QBConaxRetentionLimitMonitorMode_live,
    QBConaxRetentionLimitMonitorMode_timeshift,
    QBConaxRetentionLimitMonitorMode_pvr,
} QBConaxRetentionLimitMonitorMode;

struct QBConaxRetentionLimitMonitor_s {
    struct SvObject_ super_;

    bool isServiceStarted;

    AppGlobals appGlobals;

    SvFiber retentionLimitMonitorFiber;
    SvFiberTimer retentionLimitMonitorTimer;

    QBICSmartcardURIData currentURIData;
    bool contentBlocked;
    QBViewportBlockVideoReason videoBlockingReason;

    QBPVRRecording rec;

    bool isProperTime;
};

SvLocal
void QBConaxRetentionLimitMonitorBlockAV(QBConaxRetentionLimitMonitor self, const bool block)
{
    QBViewportBlockVideoReason newVideoBlockingReason = (self->isProperTime) ? QBViewportBlockVideoReason_contentExpired : QBViewportBlockVideoReason_contentExpirationPossible;

    if (self->contentBlocked == block && self->videoBlockingReason == newVideoBlockingReason) {
        return;
    }

    if (block) {
        if (self->isProperTime) {
            log_info("Content expired, blocking video and audio");
        } else {
            log_info("Content with expiration time but current time is unknown, blocking video and audio");
        }
    } else {
        log_info("Content valid, unblocking video and audio");
    }

    // setting current video blocking reason
    self->videoBlockingReason = newVideoBlockingReason;

    QBViewportBlockVideo(QBViewportGet(),
                         SVSTRING("QBConaxRetentionLimitMonitor"),
                         self->videoBlockingReason,
                         block);

    QBViewportBlockAudio(QBViewportGet(), block);
    self->contentBlocked = block;
}

SvLocal
SvTime QBConaxRetentionLimitMonitorCalculatePVRRetentionTime(QBConaxRetentionLimitMonitor self, QBPVRRecording rec, const SvPlayerTaskState playerState)
{
    time_t startTimeUTC = SvTimeGetSeconds(rec->startTime);
    time_t currentTimeUTC = SvTimeGetUTC();
    time_t lastTouchTime = 0;

    time_t referenceTime = currentTimeUTC;

    SvValue lastTouchTimeVal = (SvValue) QBPVRProviderGetRecordingProperty(self->appGlobals->pvrProvider,
                                                                           rec,
                                                                           SVSTRING(REC_META_KEY__LAST_TOUCH_TIME_UTC));
    if (lastTouchTimeVal) {
        lastTouchTime = SvValueGetInteger(lastTouchTimeVal);
    }

    if (lastTouchTime <= 0 || lastTouchTime < currentTimeUTC) {
        lastTouchTimeVal = SvValueCreateWithInteger(currentTimeUTC, NULL);
        QBPVRProviderSetRecordingProperty(self->appGlobals->pvrProvider,
                                          rec,
                                          SVSTRING(REC_META_KEY__LAST_TOUCH_TIME_UTC),
                                          (SvObject) lastTouchTimeVal);
        SVRELEASE(lastTouchTimeVal);
    }

    if (lastTouchTime > 0 && lastTouchTime > currentTimeUTC) {
        referenceTime = lastTouchTime;
    }

    log_info("Current position %f", playerState.currentPosition);
    log_info("Start time UTC = %d", (int) startTimeUTC);
    log_info("Reference time UTC = %d", (int) referenceTime);

    return SvTimeConstruct(referenceTime - (playerState.currentPosition + startTimeUTC), 0);
}

SvLocal
SvTime QBConaxRetentionLimitMonitorCalculateTimeshiftRetentionTime(QBConaxRetentionLimitMonitor self, const SvPlayerTaskState playerState)
{
    log_info("Current position %f", playerState.currentPosition);
    log_info("Range end = %f", playerState.timeshift.range_end);

    return SvTimeConstruct(playerState.timeshift.range_end - playerState.currentPosition, 0);
}

SvLocal
QBConaxRetentionLimitMonitorMode QBConaxRetentionLimitMonitorGetMode(QBConaxRetentionLimitMonitor self, const SvPlayerTaskState state)
{
    if (self->rec) {
        return QBConaxRetentionLimitMonitorMode_pvr;
    }

    if (state.timeshift.recording && state.timeshift.enabled) {
        return QBConaxRetentionLimitMonitorMode_timeshift;
    }

    return QBConaxRetentionLimitMonitorMode_live;
}

SvLocal
double QBConaxRetentionLimitMonitorGetNextValidPosition(QBConaxRetentionLimitMonitor self,
                                                        QBPVRRecording recording,
                                                        double currentPosition,
                                                        SvTime retentionTime,
                                                        bool isProperTime)
{
    log_fun();

    SvObject obj = QBPVRProviderGetRecordingProperty(self->appGlobals->pvrProvider,
                                                     recording,
                                                     SVSTRING(REC_META_KEY__CONAX_METADATA));

    if (!obj || !SvObjectIsInstanceOf(obj, QBICSmartcardPVRMetadata_getType())) {
        log_error("No conax data in metadata");
        return 0.0;
    }

    return QBICSmartcardPVRMetadataGetFirstValidContentPeriod((QBICSmartcardPVRMetadata) obj, currentPosition, retentionTime, isProperTime);
}

SvLocal
void QBConaxRetentionLimitMonitorStep(void *self_)
{
    log_fun();
    QBConaxRetentionLimitMonitor self = (QBConaxRetentionLimitMonitor) self_;

    // Deactivate fiber and timer event
    SvFiberDeactivate(self->retentionLimitMonitorFiber);
    SvFiberEventDeactivate(self->retentionLimitMonitorTimer);

    SvPlayerManager man = SvPlayerManagerGetInstance();
    SvPlayerTask playerTask = SvPlayerManagerGetPlayerTask(man, -1);

    if (!playerTask) {
        log_info("Player task is null. No restrictions will be applied.");
        QBConaxRetentionLimitMonitorBlockAV(self, false);
        return;
    }

    // Check if URI Data is present. If not apply no restrictions.
    if (!self->currentURIData) {
        log_info("There is no URI data. No restrictions will be applied.");
        QBConaxRetentionLimitMonitorBlockAV(self, false);
        return;
    }

    // Check if there are any restrictions.
    QBICSmartcardURIData uriData = self->currentURIData;
    if (uriData->usageRulesInformation.emiCopyControl == QBICSmartcardURIDataEMICopyControl_copyFreely) {
        log_info("No restrictions. Unblocking audio/video");
        QBConaxRetentionLimitMonitorBlockAV(self, false);
        return;
    }

    // Get player task state.
    SvPlayerTaskState state = SvPlayerTaskGetState(playerTask);
    SvTime retentionTime = SvTimeGetZero();
    SvTime retentionTimeLimit = SvTimeGetZero();

    // Check if we are in pvr, timeshift or live.
    QBConaxRetentionLimitMonitorMode mode = QBConaxRetentionLimitMonitorGetMode(self, state);

    switch (mode) {
        case QBConaxRetentionLimitMonitorMode_timeshift:
            // First check EMI flags for CopyOnce. CopyOnce means that content has got long term restrictions, but there is no timeshift restrictions.
            if (uriData->usageRulesInformation.emiCopyControl == QBICSmartcardURIDataEMICopyControl_copyOnce) {
                log_info("No timeshift restrictions. Unblocking audio/video");
                QBConaxRetentionLimitMonitorBlockAV(self, false);
                return;
            }

            // It is sanity check. EMI flag in live content shall never be equal to CopyNoMore (in case of conax 6.0 with enhanced pvr).
            // See Conax (6.0) Specification point 6.3.3.2
            if (uriData->usageRulesInformation.emiCopyControl == QBICSmartcardURIDataEMICopyControl_copyNoMore) {
                log_error("Received unexpected EMI flag (CopyNoMore) in timeshift."
                          "According to conax specification this should never happen."
                          "Video and audio will be blocked.");
                QBConaxRetentionLimitMonitorBlockAV(self, true);
                return;
            }

            // Check if there is any retention limit.
            if (uriData->usageRulesInformation.timeshiftRetentionLimit == QBICSmartcardURIDataTimeshiftRetentionLimit_noLimit) {
                log_info("No timeshift restrictions. Unblocking audio/video");
                QBConaxRetentionLimitMonitorBlockAV(self, false);
                return;
            }

            // Calculate current retention time. In other words, calculate how old is content which is currently played.
            retentionTime = QBConaxRetentionLimitMonitorCalculateTimeshiftRetentionTime(self, state);
            // Calculate retention limit.
            retentionTimeLimit = QBICSmartcardURIDataTimeshiftRetentionLimitToTime(self->currentURIData->usageRulesInformation.timeshiftRetentionLimit);

            if (env_timeshift_limit_divisor()) {
                retentionTimeLimit.us /= env_timeshift_limit_divisor();
            }

            break;
        case QBConaxRetentionLimitMonitorMode_pvr:
            // It is sanity check. EMI flag in pvr stored content shall never be equal to copyNever (in case of conax 6.0 with enhanced pvr).
            // This situation is possible in case of retention PVR. Anyway audio and video shall be blocked.
            if (uriData->usageRulesInformation.emiCopyControl == QBICSmartcardURIDataEMICopyControl_copyNever) {
                log_info("This content shouldn't be stored on disk. Blocking audio/video");
                QBConaxRetentionLimitMonitorBlockAV(self, true);
                return;
            }

            // Check if there is any retention limit.
            if (uriData->usageRulesInformation.longTermRetentionLimit == QBICSmartcardURIDataLongTermRetentionLimit_noLimit) {
                log_info("No long term restrictions. Unblocking audio/video");
                QBConaxRetentionLimitMonitorBlockAV(self, false);
                return;
            }

            if (self->isProperTime) {
                // Calculate current retention time. In other words, calculate how old is content which is currently played.
                retentionTime = QBConaxRetentionLimitMonitorCalculatePVRRetentionTime(self, self->rec, state);
            }

            // jump to first valid content period
            playerTask = SvPlayerManagerGetPlayerTask(SvPlayerManagerGetInstance(), state.playback_session_id);

            double firstValidPeriod = QBConaxRetentionLimitMonitorGetNextValidPosition(self, self->rec, state.currentPosition, retentionTime, self->isProperTime);
            log_info("First valid content period (%f sec)", firstValidPeriod);

            if (state.currentSpeed == 1.0) {
                // we cannot jump directly to first valid period because when we want to jump to given position we are not sure that we will be exactly in a proper
                //  offset, normally jump is done 0.5-0.6 sec before requested position
                if ((state.currentPosition + 1.0) < firstValidPeriod) {
                    log_state("jump to next valid content period (%f), current position (%f)", firstValidPeriod, state.currentPosition);
                    // we should block AV now, it will be unblocked after jumping
                    QBConaxRetentionLimitMonitorBlockAV(self, true);
                    SvPlayerTaskPlay(playerTask, 1.0, firstValidPeriod, NULL);
                    return;
                } else if (firstValidPeriod < 0) {
                    // there are no more valid period in playback
                    QBConaxRetentionLimitMonitorBlockAV(self, true);

                    // TODO: now we don't have an ability to jump to end of the stream, state.duration and self->schedDesc->params.duration are equal to 0
                    //  there is possibility to use below code, but we are not sure that between start and stop time there wasn't any time update
                    //  int duration = SvLocalTimeDiff(self->schedDesc->extracted.stopTime, self->schedDesc->extracted.startTime);
                    //  SvPlayerTaskPlay(playerTask, 1.0, duration, NULL);
                    //  return;
                }
            }

            // Calculate retention limit
            retentionTimeLimit = QBICSmartcardURIDataLongTermRetentionLimitToTime(self->currentURIData->usageRulesInformation.longTermRetentionLimit);
            break;
        case QBConaxRetentionLimitMonitorMode_live:
            log_info("We are in live playback. No restrictions will be applied.");
            QBConaxRetentionLimitMonitorBlockAV(self, false);
            // We have to poll player to check whether we are still in live mode.
            goto fini;
            break;
        default:
            log_error("Unknown mode.");
            return;
    }

    log_info("Retention time = %d sec", SvTimeGetSeconds(retentionTime));
    log_info("Retention limit time = %d sec", SvTimeGetSeconds(retentionTimeLimit));

    bool shouldBlockAV = true;
    if (self->isProperTime && (SvTimeCmp(retentionTimeLimit, retentionTime) > 0)) {
        shouldBlockAV = false;
    }

    QBConaxRetentionLimitMonitorBlockAV(self, shouldBlockAV);

fini:
    SvFiberTimerActivateAfter(self->retentionLimitMonitorTimer, SvTimeConstruct(1, 0));
}

SvLocal void QBConaxRetentionLimitMonitorDestroy(void *self_)
{
    log_fun();
    QBConaxRetentionLimitMonitor self = (QBConaxRetentionLimitMonitor) self_;

    if (self->isServiceStarted)
        QBConaxRetentionLimitMonitorStop(self);

    SvFiberEventDestroy(self->retentionLimitMonitorTimer);
    SvFiberDestroy(self->retentionLimitMonitorFiber);

    SVTESTRELEASE(self->currentURIData);
    SVTESTRELEASE(self->rec);
}

SvLocal void QBConaxRetentionLimitTimeUpdated(SvObject self_, bool firstTime, QBTimeDateUpdateSource source)
{
    log_fun();
    QBConaxRetentionLimitMonitor self = (QBConaxRetentionLimitMonitor) self_;

    log_info("TimeUpdated, firstTime: %d, source: %d", (int) firstTime, (int) source);

    if (firstTime) {
        self->isProperTime = true;
        SvFiberActivate(self->retentionLimitMonitorFiber);
    }
}

SvLocal SvType QBConaxRetentionLimitMonitor_getType(void)
{
    log_fun();
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBConaxRetentionLimitMonitorDestroy
    };

    static const struct QBTimeDateMonitorListener_ timeDateMonitorListenerMethods = {
        .systemTimeSet = QBConaxRetentionLimitTimeUpdated
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBConaxRetentionLimitMonitor",
                            sizeof(struct QBConaxRetentionLimitMonitor_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBTimeDateMonitorListener_getInterface(), &timeDateMonitorListenerMethods,
                            NULL);
    }

    return type;
}

void QBConaxRetentionLimitMonitorStart(QBConaxRetentionLimitMonitor self)
{
    log_fun();
    if (self->isServiceStarted) {
        log_error("Service has been already started.");
        return;
    }

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorAddListener(timeDateMonitor, (SvObject) self, NULL);

    self->isServiceStarted = true;
}

void QBConaxRetentionLimitMonitorStop(QBConaxRetentionLimitMonitor self)
{
    log_fun();
    if (!self->isServiceStarted) {
        log_error("Service hasn't been started.");
        return;
    }

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorRemoveListener(timeDateMonitor, (SvObject) self, NULL);

    SvFiberDeactivate(self->retentionLimitMonitorFiber);
    SvFiberEventDeactivate(self->retentionLimitMonitorTimer);

    self->isServiceStarted = false;
}

QBConaxRetentionLimitMonitor QBConaxRetentionLimitMonitorCreate(AppGlobals appGlobals)
{
    log_fun();
    QBConaxRetentionLimitMonitor self = (QBConaxRetentionLimitMonitor) SvTypeAllocateInstance(QBConaxRetentionLimitMonitor_getType(), NULL);

    self->isServiceStarted = false;
    self->appGlobals = appGlobals;
    self->retentionLimitMonitorFiber = SvFiberCreate(appGlobals->scheduler, NULL, "QBConaxRetentionLimitMonitorStep",
                                                     QBConaxRetentionLimitMonitorStep, self);
    self->retentionLimitMonitorTimer = SvFiberTimerCreate(self->retentionLimitMonitorFiber);

    return self;
}

void QBConaxRetentionLimitMonitorSetURIData(QBConaxRetentionLimitMonitor self, QBICSmartcardURIData uriData)
{
    log_fun();
    SVTESTRELEASE(self->currentURIData);
    self->currentURIData = SVRETAIN(uriData);
    SvFiberActivate(self->retentionLimitMonitorFiber);
}

void QBConaxRetentionLimitMonitorSetRecording(QBConaxRetentionLimitMonitor self, QBPVRRecording rec)
{
    SVTESTRELEASE(self->rec);
    self->rec = SVTESTRETAIN(rec);
}
