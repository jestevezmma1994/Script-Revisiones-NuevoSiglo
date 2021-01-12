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

#include "QBConaxPVRManager.h"
#include "QBConaxRetentionLimitMonitor.h"
#include "QBConaxTimeshiftMonitor.h"

#include <main.h>
#include <Services/QBCASManager.h>
#include <QBApplicationController.h>
#include <QBContextSwitcher.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBDialog.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBTimeDateMonitor.h>
#include <Utils/authenticators.h>
#include <QBPVRProvider.h>
#include <QBPVRRecording.h>
#include <settings.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvEnv.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvType.h>
#include <fibers/c/fibers.h>

#include <QBSmartcard2Interface.h>
#include <QBICSmartcardCachedPIN.h>
#include <QBCAS.h>

#include <libintl.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBConaxPVRManager"
    SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, moduleName "LogLevel", "0:error, 1:warning, 2:state, 3:info, 4:deep, 5:debug, 6:function");
    #define log_fun(fmt, ...)   do { if(env_log_level() >= 6) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(green),  __func__, ##__VA_ARGS__); } while(0)
    #define log_debug(fmt, ...) do { if(env_log_level() >= 5) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(green),  __func__, ##__VA_ARGS__); } while(0)
    #define log_deep(fmt, ...)  do { if(env_log_level() >= 4) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(cyan),   __func__, ##__VA_ARGS__); } while(0)
    #define log_info(fmt, ...)  do { if(env_log_level() >= 3) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(cyan),   __func__, ##__VA_ARGS__); } while(0)
    #define log_state(fmt, ...) do { if(env_log_level() >= 2) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(blue),   __func__, ##__VA_ARGS__); } while(0)
    #define log_warn(fmt, ...)  do { if(env_log_level() >= 1) SvLogWarning(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ##__VA_ARGS__); } while(0)
    #define log_error(fmt, ...) do { if(env_log_level() >= 0) SvLogError(COLBEG()   moduleName " :: %s " fmt COLEND_COL(red),    __func__, ##__VA_ARGS__); } while(0)
#else
    #define log_fun(fmt, ...)
    #define log_debug(fmt, ...)
    #define log_deep(fmt, ...)
    #define log_info(fmt, ...)
    #define log_state(fmt, ...)
    #define log_warn(fmt, ...)
    #define log_error(fmt, ...)
#endif

#define STEP_INTERVAL_SEC 10

#define CAS_POPUP_BLOCKER_WINDOW_ID "conaxPopupManagerPINPopup"

struct QBConaxPVRManager_t {
    struct SvObject_ super;

    AppGlobals appGlobals;
    SvFiber fiber;
    SvFiberTimer timer;

    bool askForPIN;
    bool isDialogShown;
    bool isContextValid;
    SvWidget cardConfigurationDialog;

    QBPVRRecording rec;

    QBConaxRetentionLimitMonitor retentionLimitMonitor;
    QBConaxTimeshiftMonitor timeshiftMonitor;

    bool isProperTime;

    struct {
        SvArray recordings;
        SvIterator it;
    } cleanExpiredRecordingsTask;
};

SvLocal void QBConaxPVRManagerRemoveCardConfigurationDialog(QBConaxPVRManager self);

SvLocal void QBConaxPVRManagerCachedPinStatus(void* target, const QBICSmartcardSessionDescription *sessionDesc, SvObject obj)
{
    log_fun();
    QBConaxPVRManager self = (QBConaxPVRManager) target;
    QBICSmartcardCachedPIN cachedPin = (QBICSmartcardCachedPIN) obj;

    switch (cachedPin->status) {
        case QBICSmartcardCachedPINStatus_wrong:
        self->askForPIN = true;
        if (self->isContextValid) {
            SvFiberActivate(self->fiber);
        }
        break;
    default:
        self->askForPIN = false;
        break;
    }

    SVRELEASE(cachedPin);
}

static const struct QBICSmartcardCallbacks_s smartcardCallbacks = {
    .cached_pin_status = &QBConaxPVRManagerCachedPinStatus,
};

SvLocal void QBConaxPVRManagerSmartcardState(void *self_, QBCASSmartcardState state)
{
    log_fun();

    QBConaxPVRManager self = self_;

    if (state == QBCASSmartcardState_removed && self->isDialogShown) {
        QBConaxPVRManagerRemoveCardConfigurationDialog(self);
    }
}

static const struct QBCASCallbacks_s casCallbacks = {
    .smartcard_state = &QBConaxPVRManagerSmartcardState,
};

SvLocal void QBConaxPVRManagerStart(SvObject self_)
{
    log_fun();

    QBConaxPVRManager self = (QBConaxPVRManager) self_;
    QBConaxRetentionLimitMonitorStart(self->retentionLimitMonitor);
    QBConaxTimeshiftMonitorStart(self->timeshiftMonitor);
    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, &casCallbacks, self, "QBConaxPVRManager");
    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), addConaxCallbacks, &smartcardCallbacks, self_, "QBConaxPVRManager");
    QBApplicationControllerAddListener(self->appGlobals->controller, (SvObject) self);

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorAddListener(timeDateMonitor, (SvObject) self, NULL);

    SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(STEP_INTERVAL_SEC, 0));
}

SvLocal void QBConaxPVRManagerStop(SvObject self_)
{
    log_fun();

    QBConaxPVRManager self = (QBConaxPVRManager) self_;

    if (self->isDialogShown) {
        QBConaxPVRManagerRemoveCardConfigurationDialog(self);
    }

    QBConaxRetentionLimitMonitorStop(self->retentionLimitMonitor);
    QBConaxTimeshiftMonitorStop(self->timeshiftMonitor);
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    SVTESTRELEASE(self->cleanExpiredRecordingsTask.recordings);
    self->cleanExpiredRecordingsTask.recordings = NULL;
    SvInvokeInterface(QBCAS, QBCASGetInstance(), removeCallbacks, &casCallbacks, self);
    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), removeConaxCallbacks, &smartcardCallbacks, self_);
    QBApplicationControllerRemoveListener(self->appGlobals->controller, (SvObject) self);

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorRemoveListener(timeDateMonitor, (SvObject) self, NULL);
}

SvLocal void QBConaxPVRManagerRecordingAdded(SvObject self_, const char* caller)
{
    log_fun();

    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), checkCachedPin);
}

SvLocal void QBConaxPVRManagerPlaybackStarted(SvObject self_, QBPVRRecording recording)
{
    log_fun();

    QBConaxPVRManager self = (QBConaxPVRManager) self_;
    QBConaxRetentionLimitMonitorSetRecording(self->retentionLimitMonitor, recording);
    self->rec = recording;
}

SvLocal void QBConaxPVRManagerPlaybackStopped(SvObject self_, QBPVRRecording recording)
{
    log_fun();

    QBConaxPVRManager self = (QBConaxPVRManager) self_;
    QBConaxRetentionLimitMonitorSetRecording(self->retentionLimitMonitor, NULL);
    self->rec = NULL;
}

SvLocal void QBConaxPVRManagerContextSwitchStart(SvObject self_, QBWindowContext from, QBWindowContext to)
{
    log_fun();

    QBConaxPVRManager self = (QBConaxPVRManager) self_;

    self->isContextValid = false;
}

SvLocal void QBConaxPVRManagerContextSwitchEnd(SvObject self_, QBWindowContext from, QBWindowContext to)
{
    log_fun();

    QBConaxPVRManager self = (QBConaxPVRManager) self_;

    self->isContextValid = (to) ? true : false;

    if (self->askForPIN && self->isContextValid) {
        SvFiberActivate(self->fiber);
    }
}

SvLocal void QBConaxPVRManagerTimeUpdated(SvObject self_, bool firstTime, QBTimeDateUpdateSource source)
{
    log_fun();

    QBConaxPVRManager self = (QBConaxPVRManager) self_;

    log_info("TimeUpdated, firstTime: %d, source: %d", (int) firstTime, (int) source);

    if (firstTime) {
        self->isProperTime = true;
    }
}

SvLocal void QBConaxPVRManagerDestroy(void *self_)
{
    log_fun();

    QBConaxPVRManager self = (QBConaxPVRManager) self_;
    QBConaxPVRManagerStop(self_);
    SvFiberDestroy(self->fiber);
    SVRELEASE(self->retentionLimitMonitor);
    SVRELEASE(self->timeshiftMonitor);
}

SvLocal SvType QBConaxPVRManager_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBConaxPVRManagerDestroy
    };

    static const struct QBCASPVRManager_ pvrMethods = {
        .recordingAdded     = &QBConaxPVRManagerRecordingAdded,
        .start              = &QBConaxPVRManagerStart,
        .stop               = &QBConaxPVRManagerStop
    };

    static const struct QBCASPVRPlaybackMonitor_ pvrPlaybackMethods = {
        .playbackStarted = &QBConaxPVRManagerPlaybackStarted,
        .playbackStopped = &QBConaxPVRManagerPlaybackStopped
    };

    static const struct QBContextSwitcherListener_t switcherMethods = {
        .started = QBConaxPVRManagerContextSwitchStart,
        .ended = QBConaxPVRManagerContextSwitchEnd
    };

    static const struct QBTimeDateMonitorListener_ timeDateMonitorListenerMethods = {
        .systemTimeSet = QBConaxPVRManagerTimeUpdated
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBConaxPVRManager",
                            sizeof(struct QBConaxPVRManager_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBCASPVRManager_getInterface(), &pvrMethods,
                            QBCASPVRPlaybackMonitor_getInterface(), &pvrPlaybackMethods,
                            QBContextSwitcherListener_getInterface(), &switcherMethods,
                            QBTimeDateMonitorListener_getInterface(), &timeDateMonitorListenerMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBConaxPVRManagerDialogCallback(void *ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    log_fun();

    QBConaxPVRManager self = (QBConaxPVRManager) ptr;

    if (buttonTag) {
        self->askForPIN = false;
    }

    self->isDialogShown = false;
    self->cardConfigurationDialog = NULL;

    if (self->appGlobals->casPopupVisabilityManager) {
        QBCASPopupVisibilityManagerUnblock(self->appGlobals->casPopupVisabilityManager, NULL, SVSTRING(CAS_POPUP_BLOCKER_WINDOW_ID));
    }
}


SvLocal void QBConaxPVRManagerCleanExpiredRecordingsTask(QBConaxPVRManager self)
{
    log_fun();

    if (!self->cleanExpiredRecordingsTask.recordings) {
        // Finding all completed recordings
        QBPVRProviderGetRecordingsParams params = {
            .includeRecordingsInDirectories = true,
        };
        params.inStates[QBPVRRecordingState_completed] = true;
        params.inStates[QBPVRRecordingState_interrupted] = true;
        params.inStates[QBPVRRecordingState_stoppedManually] = true;
        params.inStates[QBPVRRecordingState_stoppedNoSpace] = true;
        params.inStates[QBPVRRecordingState_stoppedNoSource] = true;
        params.inStates[QBPVRRecordingState_failed] = true;

        // TODO: This method may exceeded fiber step time.
        // This method returs retained SvArray
        SvArray recs = QBPVRProviderGetRecordings(self->appGlobals->pvrProvider,
                                                params);
        if (!recs)
            return;

        self->cleanExpiredRecordingsTask.recordings = recs;
        self->cleanExpiredRecordingsTask.it = SvArrayGetIterator(self->cleanExpiredRecordingsTask.recordings);
        log_debug("Found (%zu) completed recordings", SvArrayCount(recs));
    }

    SvObject obj = NULL;
    bool timePassed = false;

    // Check at least one recording.
    while ((obj = SvIteratorGetNext(&self->cleanExpiredRecordingsTask.it)) != NULL && !timePassed) {

        timePassed = SvFiberTimePassed();
        QBPVRRecording rec = (QBPVRRecording) obj;

        if (self->rec && SvObjectEquals((SvObject) rec->id, (SvObject) self->rec->id)) {
            continue;
        }

        if (rec->expirationTime.us <= 0) {
            continue;
        }

        SvTime now = SvTimeGetCurrentTime();
        if (self->isProperTime && SvTimeCmp(rec->expirationTime, now) < 0 && rec->state != QBPVRRecordingState_removed) {
            QBPVRProviderDeleteRecording(self->appGlobals->pvrProvider, rec);
            log_state("Removed expired recording id = %s", SvStringGetCString(rec->id));
        }
    }

    if (timePassed) {
        log_debug("Fiber time passed. Reschedule it.");
        SvFiberActivate(self->fiber);
        return;
    }

    SVRELEASE(self->cleanExpiredRecordingsTask.recordings);
    self->cleanExpiredRecordingsTask.recordings = NULL;
}

SvLocal void QBConaxPVRManagerShowCardConfigurationDialog(QBConaxPVRManager self)
{
    log_fun();

    SvObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, SVSTRING("ConaxCachedPIN"));
    svSettingsPushComponent("ConaxPVRManager.settings");
    const char *message = gettext("Card configuration changed - please enter PIN");
    self->cardConfigurationDialog = QBAuthDialogCreate(self->appGlobals, authenticator, NULL, message, true, SVSTRING("ConaxPopupManagerPINPopup"), NULL);

    QBDialogRun(self->cardConfigurationDialog, self, QBConaxPVRManagerDialogCallback);
    svSettingsPopComponent();

    self->isDialogShown = true;

    if (self->appGlobals->casPopupVisabilityManager) {
        QBCASPopupVisibilityManagerBlock(self->appGlobals->casPopupVisabilityManager, NULL, SVSTRING(CAS_POPUP_BLOCKER_WINDOW_ID));
    }

    log_state("Shown: 'Card configuration' dialog.");
}

SvLocal void QBConaxPVRManagerRemoveCardConfigurationDialog(QBConaxPVRManager self)
{
    log_fun();

    QBDialogBreak(self->cardConfigurationDialog);
    self->askForPIN = false;
    log_state("Removed: 'Card configuration' dialog.");
}

SvLocal void QBConaxPVRManagerStep(void *self_)
{
    log_fun();

    QBConaxPVRManager self = (QBConaxPVRManager) self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    if (self->askForPIN == true && self->isDialogShown == false) {
        QBConaxPVRManagerShowCardConfigurationDialog(self);
    }

    QBConaxPVRManagerCleanExpiredRecordingsTask(self);

    SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(STEP_INTERVAL_SEC, 0));
}

void QBConaxPVRManagerSetURIData(QBConaxPVRManager self, QBICSmartcardURIData uriData)
{
    log_fun();

    QBConaxRetentionLimitMonitorSetURIData(self->retentionLimitMonitor, uriData);
}

QBConaxPVRManager QBConaxPVRManagerCreate(AppGlobals appGlobals)
{
    log_fun();

    if (!SvObjectIsImplementationOf(QBCASGetInstance(), QBSmartcard2Interface_getInterface())) {
        log_error("Conax PVR Manager should be only created for Conax CAS type");
        return NULL;
    }

    QBConaxPVRManager self = (QBConaxPVRManager) SvTypeAllocateInstance(QBConaxPVRManager_getType(), NULL);
    self->appGlobals = appGlobals;
    self->fiber = SvFiberCreate(appGlobals->scheduler, NULL, "QBConaxPVRManagerStep", QBConaxPVRManagerStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    self->retentionLimitMonitor = QBConaxRetentionLimitMonitorCreate(appGlobals);
    self->timeshiftMonitor = QBConaxTimeshiftMonitorCreate(appGlobals->scheduler);

    return self;
}
