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

#include "QBConaxFingerprint.h"
#include <QBICSmartcardFingerprint.h>
#include <QBSmartcard2Interface.h>
#include <QBCAS.h>

#include <fibers/c/fibers.h>
#include <CAGE/Text/SvTextRender.h>
#include <SvGfxHAL/SvGfxUtils.h>
#include <SvGfxHAL/SvGfxEngine.h>
#include <settings.h>
#include <QBApplicationController.h>
#include <QBGlobalWindowManager.h>
#include <main.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBTimeDateMonitor.h>
#include <Services/core/QBDualOutputHandler.h>
#include <QBConf.h>
#include <QBPlatformHAL/QBPlatformOutput.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>

#define log_debug(fmt, ...)  do { if (1) SvLogNotice(COLBEG() "%s:%d " fmt COLEND_COL(blue), __func__, __LINE__, ## __VA_ARGS__); } while (0)
#define log_info(fmt, ...)  do { if (0) SvLogNotice(COLBEG() "%s:%d " fmt COLEND_COL(green), __func__, __LINE__, ## __VA_ARGS__); } while (0)
#define log_error(fmt, ...)  do { if (1) SvLogError("%s:%d " fmt, __func__, __LINE__, ## __VA_ARGS__); } while (0)

static const Sv2DRect MINIMAL_FP_SIZE = { .width = 0, .height = 12 };

struct QBFingerprint_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    SvFiber fiber;
    SvFiberTimer timer;
    SvFiberTimer hideTimer;

    QBICSmartcardFingerprint *immediateFingerprint;
    QBICSmartcardFingerprint *scheduledFingerprint;

    struct {
        QBICSmartcardFingerprint *fingerprint;
        bool shown;
    } task;

    struct {
        int durationMs;
        uint64_t beforeShowFrameCounter;
        bool rendered;
    } taskState;

    struct QBFingerprintWidget_t {
        SvBitmap textBmp;
        SvFont font;
        int xOffset;
        int yOffset;
        unsigned int hd_handle;
        unsigned int sd_handle;
    } widget;

    bool isProperTime;
};

SvLocal void QBFingerprintSmartcardCallback(void *self_, const QBICSmartcardSessionDescription *sessionDesc, SvGenericObject obj);

static struct QBICSmartcardCallbacks_s conax_callbacks = {
    .fingerprint = QBFingerprintSmartcardCallback
};

SvLocal void QBFingerprintFillGfxObject(QBFingerprint self, struct SvGfxObject_s *gfxObject)
{
    gfxObject->src = self->widget.textBmp->hwSurface;
    gfxObject->color = COLOR(R(self->widget.textBmp->color), G(self->widget.textBmp->color), B(self->widget.textBmp->color), ALPHA_SOLID);
    gfxObject->blend = true;
    gfxObject->ldim = 0;
    gfxObject->dest_rect = Sv2DRectCreate(self->task.fingerprint->x, self->task.fingerprint->y, self->widget.textBmp->width, self->widget.textBmp->height);;
    gfxObject->src_rect = Sv2DRectCreate(0, 0, self->widget.textBmp->width, self->widget.textBmp->height);
}

SvLocal void QBFingerprintAddRemove(QBFingerprint self, bool add)
{
    int HDOutputIDGfx = -1;
    int SDOutputIDGfx = -1;

    int HDOutputID = QBPlatformFindOutput(QBOutputType_unknown, NULL, QBOutputCapability_HD, true, false);
    if (HDOutputID >= 0) {
        QBVideoOutputConfig config;
        int ret = QBPlatformGetOutputConfig(HDOutputID, &config);
        if (ret != 0) {
            log_error("Cannot get output config for outputID %d", HDOutputID);
            return;
        }
        if ((int) config.masterGfxID == HDOutputID) {
            HDOutputIDGfx = config.masterGfxID;
        }
    }

    int SDOutputID = QBPlatformFindOutput(QBOutputType_unknown, NULL, QBOutputCapability_SD, true, false);
    if (SDOutputID >= 0) {
        QBVideoOutputConfig config;
        int ret = QBPlatformGetOutputConfig(SDOutputID, &config);
        if (ret != 0) {
            log_error("Cannot get output config for outputID %d", SDOutputID);
            return;
        }
        if ((int) config.masterGfxID == SDOutputID) {
            SDOutputIDGfx = config.masterGfxID;
        }
    }

    if (add) {
        struct SvGfxObject_s object = { .object_id = 0 };
        QBFingerprintFillGfxObject(self, &object);

        if (HDOutputIDGfx >= 0) {
            SvGfxEngineAddImageOnVideo(HDOutputIDGfx, &object, &MINIMAL_FP_SIZE, SvGfxOnVideoQueueObjectType_ConaxFingerprint, &self->widget.hd_handle);
        }
        if (SDOutputIDGfx >= 0) {
            SvGfxEngineAddImageOnVideo(SDOutputIDGfx, &object, &MINIMAL_FP_SIZE, SvGfxOnVideoQueueObjectType_ConaxFingerprint, &self->widget.sd_handle);
        }
    } else {
        if (HDOutputIDGfx >= 0) {
            SvGfxEngineRemoveImageFromVideo(HDOutputIDGfx, self->widget.hd_handle);
        }
        if (SDOutputIDGfx >= 0) {
            SvGfxEngineRemoveImageFromVideo(SDOutputIDGfx, self->widget.sd_handle);
        }
    }
}

SvLocal void QBFingerprintDetach(QBFingerprint self)
{
    if (self->widget.textBmp) {
        QBFingerprintAddRemove(self, false);
        SVRELEASE(self->widget.textBmp);
        self->widget.textBmp = NULL;
    }
}

SvLocal void QBFingerprintHidePopup(QBFingerprint self)
{
    self->task.shown = false;
    QBFingerprintDetach(self);
}

SvLocal void QBFingerprint__dtor__(void *self_)
{
    QBFingerprint self = self_;

    if (self->task.fingerprint && self->task.shown)
        QBFingerprintHidePopup(self);
    SVTESTRELEASE(self->task.fingerprint);
    SVTESTRELEASE(self->immediateFingerprint);
    SVTESTRELEASE(self->scheduledFingerprint);
    SVRELEASE(self->widget.font);
    SVTESTRELEASE(self->widget.textBmp);

    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), removeConaxCallbacks, &conax_callbacks, self);
}

SvLocal void QBFingerprintAttach(QBFingerprint self)
{
    QBFingerprintAddRemove(self, true);
}

SvLocal void QBFingerprintPrepareLabel(QBFingerprint self);

SvLocal void QBFingerprintTimeUpdated(SvGenericObject self_, bool firstTime, QBTimeDateUpdateSource source)
{
    log_debug("TimeUpdated, firstTime: %d, source: %d", (int) firstTime, (int) source);
    QBFingerprint self = (QBFingerprint) self_;
    if (firstTime) {
        self->isProperTime = true;
        SvFiberActivate(self->fiber);
    }
}

SvLocal SvType QBFingerprint_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBFingerprint__dtor__
    };

    static SvType type = NULL;

    static const struct QBTimeDateMonitorListener_ timeDateListenerMethods = {
        .systemTimeSet = QBFingerprintTimeUpdated
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBFingerprint",
                            sizeof(struct QBFingerprint_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBTimeDateMonitorListener_getInterface(), &timeDateListenerMethods,
                            NULL);
    }
    return type;
}


SvLocal void QBFingerprintPrepareLabel(QBFingerprint self)
{
    int font_size = self->task.fingerprint->font_size;
    log_debug("font size(%d)", font_size);

    /// Also take into account that FP's size means height of a "0" digit,
    /// but SvFont will treat it as it pleases, so it might as well be the height of "Śy".
    /// We need to test the needed font_size, and use scaled size instead.
    Sv2DRect boundingBox;
    SvFontSetSize(self->widget.font, font_size, NULL);
    SvBitmap textBmp = SvBitmapCreateWithText("01AZ", self->widget.font, 0xffffffff, &boundingBox);
    SVRELEASE(textBmp);

    int new_font_size = font_size * font_size / boundingBox.y;
    SvFontSetSize(self->widget.font, new_font_size, NULL);
    textBmp = SvBitmapCreateWithText(SvStringCString(self->task.fingerprint->text),
                                     self->widget.font, 0xffffffff, &boundingBox);
    SvGfxBlitBitmap(textBmp, NULL);
    SvBitmapBlank(textBmp, NULL);

    SVTESTRELEASE(self->widget.textBmp);
    self->widget.textBmp = textBmp;
}

SvLocal void QBFingerprintShowPopup(QBFingerprint self)
{
    self->task.shown = true;
    QBFingerprintPrepareLabel(self);
    QBFingerprintAttach(self);
}

#define  FIRST_FRAME_POLL_MS  5

#define SECONDS_IN_DAY (24 * 60 * 60)
#define MILISECONDS_IN_DAY (SECONDS_IN_DAY * 1000)

SvLocal int64_t QBFingerprintComputeStartTime(QBICSmartcardFingerprint *fingerprint)
{
    int64_t startTime = 0;

    // valid time could be calculated only for scheduled fingerprint
    if (fingerprint->start_time >= 0 && fingerprint->validReceivedTime) {
        int64_t us_per_day = (int64_t) 1000 * MILISECONDS_IN_DAY;

        // difference between start time and received time
        int64_t startToReceiveDiff = ((int64_t) fingerprint->start_time * 1000) - (fingerprint->receivingTime % us_per_day);
        startTime = fingerprint->receivingTime + ((us_per_day + startToReceiveDiff) % us_per_day);
    }

    return startTime;
}

SvLocal bool QBFingerprintCheckFingerprint(QBFingerprint self, QBICSmartcardFingerprint *fingerprint, int *leftDuration)
{
    if ((self->task.fingerprint->start_time < 0)) {
        // immediate fingerprint is always valid
        *leftDuration = fingerprint->duration;
        return true;
    }

    if (!self->isProperTime) {
        // we need to wait for a proper time for scheduler fingerprints
        return false;
    }

    SvTime now = SvTimeGetCurrentTime();
    int32_t durationMs = self->task.fingerprint->duration;

    if (self->task.fingerprint->validReceivedTime) {
        int64_t startTime = QBFingerprintComputeStartTime(self->task.fingerprint);
        int64_t currentTime = SvTimeToMicroseconds64(now);

        struct tm tmp;
        SvTimeBreakDown(SvTimeFromMicroseconds(startTime), false, &tmp);
        char startTimeBuffer[64] = { 0 };
        strftime(startTimeBuffer, sizeof(startTimeBuffer), "%Y/%m/%d %H:%M:%S", &tmp);

        if (startTime > currentTime) {
            // we are still before showing time
            return false;
        }

        if (currentTime >= startTime + durationMs * 1000) {
            // this is an old fingerprint, mark as invalid
            log_debug("Current fingerprint is too old, start Time (%s) UTC time, duration (%d) ms", startTimeBuffer, durationMs);
            SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), markFingerprint, self->task.fingerprint);
            SVTESTRELEASE(self->task.fingerprint);
            self->task.fingerprint = NULL;
            SVRELEASE(self->scheduledFingerprint);
            self->scheduledFingerprint = NULL;
            return false;
        }

        // we need to show it now, calculate left duration
        *leftDuration = durationMs - (currentTime - startTime) / 1000;
        log_info("current fingerprint start Time (%s) UTC time, original duration (%d) ms, left duration (%d) ms", startTimeBuffer, durationMs, *leftDuration);
    } else {
        // this part is from old QBConax2 api, it is here for compatibility only, could be removed if all modules will support fingerprint receiving time
        struct tm tm;
        SvTimeBreakDown(now, false, &tm);
        tm.tm_sec = 0;
        tm.tm_min = 0;
        tm.tm_hour = 0;
        SvTime midnight = SvTimeFromBrokenDownTime(&tm, 0, false);

        int32_t nowMs = SvTimeToMs(SvTimeSub(now, midnight));
        int32_t startMs = self->task.fingerprint->start_time;

        if (startMs > nowMs) {
            // we are still before showing time
            return false;
        }

        // check if showing time is not in the past
        if (nowMs >= startMs + durationMs) {
            log_info("nowMs (%d) >= startMs (%d) + duration (%d) = (%d)", nowMs, startMs, durationMs, startMs + durationMs);
            SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), markFingerprint, self->task.fingerprint);
            SVTESTRELEASE(self->task.fingerprint);
            self->task.fingerprint = NULL;
            // immediate fingerprint is released here intentionally, in old api we use only one slot for scheduled and immediate fingerprints
            SVRELEASE(self->immediateFingerprint);
            self->immediateFingerprint = NULL;
            return false;
        }

        *leftDuration = durationMs - (nowMs - startMs);
    }

    return true;
}

SvLocal void QBFingerprintShowIfReady(QBFingerprint self)
{
    int leftDuration = self->task.fingerprint->duration;

    if (!QBFingerprintCheckFingerprint(self, self->task.fingerprint, &leftDuration)) {
        return;
    }
    //SvLogNotice("immediate = %d, nowMs=%d, start_time=%d, durationMs=%d", (self->task.fingerprint->start_time < 0) ? "true" : "false", nowMs, startMs, durationMs);

    log_info("left duration (%d) ms, original (%d) ms", leftDuration, self->task.fingerprint->duration);

    if (leftDuration) {
        QBFingerprintShowPopup(self);

#if 0
        /// with very short duration, the fp might show/hide in-between frames
        /// yes, we are checking if the fp is rendered on canvas, but it might still be rejected by the framebuffer blitter,
        ///   and we can't check for that from this level
        if (durationMs < 40)
            durationMs = 40;
#endif

        self->taskState.durationMs = leftDuration;
        self->taskState.beforeShowFrameCounter = self->appGlobals->res->rendered_frames;
        self->taskState.rendered = false; // not gone through the renderer yet

        svAppForceRender(self->appGlobals->res);

        /// start with waiting for next frame to be rendered
        SvFiberTimerActivateAfter(self->hideTimer, SvTimeFromMs(FIRST_FRAME_POLL_MS));
    }
}

SvLocal bool QBFingerprintShouldReplaceCurrentFingerprint(QBFingerprint self, QBICSmartcardFingerprint *newFingerprint)
{
    if (self->task.fingerprint && newFingerprint->validReceivedTime) {
        bool newIsImmediate = (newFingerprint->start_time < 0);
        bool oldIsImmediate = (self->task.fingerprint->start_time < 0);
        bool oldHasHigherPriority = self->task.fingerprint->withPriority && !newFingerprint->withPriority;
        // don't replace immediate fingerprint by a scheduled one or by a lower priority fingerprint
        if (oldIsImmediate && ((!newIsImmediate) || oldHasHigherPriority)) {
            return false;
        }
    }

    return true;
}

SvLocal void QBFingerprintSmartcardCallback(void *self_, const QBICSmartcardSessionDescription *sessionDesc, SvGenericObject obj)
{
    QBFingerprint self = self_;

    // we are interested in all fingerprint from playback session and from non-session (sessionId == -1)
    if (sessionDesc->sessionId != -1 && sessionDesc->sessionType != QBCASSessionType_playback) {
        return;
    }

    if (!SvObjectIsInstanceOf(obj, QBICSmartcardFingerprint_getType()))
        return;

    QBICSmartcardFingerprint *newFingerprint = (QBICSmartcardFingerprint *) obj;

    // there are two slots for fingerprints (for scheduled and immediate)
    // scheduled fingerprint should be checked after displaying immediate one, below scenario should be handled properly:
    // 1. scheduled fingerprint received with display time after 1h
    // 2. after 10s there is an immediate fingerprint, should be shown immediately
    // 3. after 1h from point 1 we should show scheduled fingerprint
    QBICSmartcardFingerprint *nextFingerprint = NULL;

    // we will treat fingerprints without validReceivedTime as a immediate fingerprint for compatibility with QBConax API less than 2.05
    if (newFingerprint->validReceivedTime && newFingerprint->start_time >= 0) {
        SVTESTRELEASE(self->scheduledFingerprint);
        self->scheduledFingerprint = SVRETAIN(newFingerprint);

        nextFingerprint = self->scheduledFingerprint;
    } else {
        SVTESTRELEASE(self->immediateFingerprint);
        self->immediateFingerprint = SVRETAIN(newFingerprint);

        nextFingerprint = self->immediateFingerprint;
    }

    // remove current fingerprint from display, we should not replace immediate fingerprint with scheduled one
    if (QBFingerprintShouldReplaceCurrentFingerprint(self, newFingerprint)) {
        if (self->task.fingerprint) {
            if (self->task.shown) {
                SvFiberEventDeactivate(self->hideTimer);
                QBFingerprintHidePopup(self);
            }
        }

        SVTESTRELEASE(self->task.fingerprint);
        self->task.fingerprint = SVRETAIN(nextFingerprint);
    }

    SvFiberActivate(self->fiber);
}

SvLocal void QBFingerprintStep(void *ptr)
{
    QBFingerprint self = ptr;
    SvFiberDeactivate(self->fiber);

    if (SvFiberEventIsActive(self->hideTimer)) {
        /// not sure if it's already gone through the renderer?
        if (!self->taskState.rendered) {
            if (self->appGlobals->res->rendered_frames == self->taskState.beforeShowFrameCounter) {
                /// series of short waits until a frame is actually shown for the first time
                SvFiberTimerActivateAfter(self->hideTimer, SvTimeFromMs(FIRST_FRAME_POLL_MS));
                return;
            };
            self->taskState.rendered = true;

            /// now wait the nominal duration of the fp
            SvFiberTimerActivateAfter(self->hideTimer, SvTimeFromMs(self->taskState.durationMs));
            return;
        };

        SvFiberEventDeactivate(self->hideTimer);
        QBFingerprintHidePopup(self);
        // inform conax module that fingerprint was shown (then fingerprint without priority could be loaded)
        SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), markFingerprint, self->task.fingerprint);

        if (SvObjectEquals((SvObject) self->task.fingerprint, (SvObject) self->scheduledFingerprint)) {
            SVRELEASE(self->scheduledFingerprint);
            self->scheduledFingerprint = NULL;
            SVTESTRELEASE(self->task.fingerprint);
            self->task.fingerprint = NULL;
        } else {
            // load scheduled if present
            SVTESTRELEASE(self->task.fingerprint);
            self->task.fingerprint = SVTESTRETAIN(self->scheduledFingerprint);
        }
        return;
    };

    SvFiberEventDeactivate(self->timer);

    if (self->task.fingerprint && !self->task.shown)
        QBFingerprintShowIfReady(self);

    // wait one second and check if we could show current scheduled fingerprint
    if (self->task.fingerprint)
        SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(1, 0));
}

QBFingerprint QBFingerprintCreate(AppGlobals appGlobals)
{
    if (!SvObjectIsImplementationOf(QBCASGetInstance(), QBSmartcard2Interface_getInterface())) {
        log_error("Conax Fingerprint should be only created for Conax CAS type");
        return NULL;
    }

    QBFingerprint self = (QBFingerprint) SvTypeAllocateInstance(QBFingerprint_getType(), NULL);

    self->appGlobals = appGlobals;

    svSettingsPushComponent("Fingerprint.settings");


    self->widget.font = svSettingsCreateFont("Fingerprint", "font");
    self->widget.xOffset = svSettingsGetInteger("Fingerprint", "xOffset", 0);
    self->widget.yOffset = svSettingsGetInteger("Fingerprint", "yOffset", 0);

    svSettingsPopComponent();

    return self;
}

void QBFingerprintStart(QBFingerprint self)
{
    if (!self) {
        SvLogError("%s :: NULL argument passed", __func__);
        return;
    } else if (self->fiber) {
        SvLogError("%s :: Module is running", __func__);
        return;
    } else {
        self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "fingerprint", QBFingerprintStep, self);
        self->timer = SvFiberTimerCreate(self->fiber);
        self->hideTimer = SvFiberTimerCreate(self->fiber);
        SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(1, 0));
    }

    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), addConaxCallbacks, &conax_callbacks, self, "QBConaxFingerprint");

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorAddListener(timeDateMonitor, (SvObject) self, NULL);
}

void QBFingerprintStop(QBFingerprint self)
{
    SvFiberDestroy(self->fiber);

    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), removeConaxCallbacks, &conax_callbacks, self);

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorRemoveListener(timeDateMonitor, (SvObject) self, NULL);
}
