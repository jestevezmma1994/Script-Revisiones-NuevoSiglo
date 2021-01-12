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

#include <Logic/EPGLogic.h>

#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/event.h>
#include <QBConf.h>
#include <Windows/newtv.h>
#include <Windows/newtvguide.h>
#include <Windows/verticalEPG.h>
#include <Logic/NPvrLogic.h>
#include <Utils/appType.h>
#include <QBStringUtils.h>
#include <stringUtils.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBPVRProvider.h>
#include <Utils/value.h>
#include <NPvr/QBnPVRProvider.h>
#include <main.h>

#if SV_LOG_LEVEL > 0
    SV_DECL_INT_ENV_FUN_DEFAULT(env_log, 0, "EPGLogicLogLevel", "0:error, 1:debug");
    #define log_debug(fmt, ...) do { if (env_log() >= 1) SvLogNotice(COLBEG() "%s: " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...) do { if (env_log() >= 0) SvLogError(COLBEG() "%s: " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#else
    #define log_debug(fmt, ...)
    #define log_error(fmt, ...)
#endif

struct QBEPGLogic_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    bool started;                           // EPG logic state variable
    bool extendedInfo;
    bool startCatchupServiceEnabled;        // Catchup enable, set if this STB is entitled on MW for Catchup service
    bool catchupAvailabilityLimitEnabled;   // When set to true, catchup availability limit was received from middleware
    int catchupAvailabilityLimit;           // Limit of catchup availability in seconds
};

SvLocal void
QBEPGLogicServiceStatusChanged(SvObject self_, SvDBObject service, QBMWServiceMonitorEvent event)
{
    QBEPGLogic self = (QBEPGLogic) self_;
    SvValue val = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) service, "tag");
    SvString serviceTag = SvValueTryGetString(val);
    if (serviceTag && SvStringEqualToCString(serviceTag, "StartCatchup")) {
        if (event == QBMWServiceMonitor_serviceEnabled) {
            log_debug("Enable service '%s'", SvStringCString(serviceTag));
            self->startCatchupServiceEnabled = true;
        } else if (event == QBMWServiceMonitor_serviceDisabled) {
            log_debug("Disable service '%s'", SvStringCString(serviceTag));
            self->startCatchupServiceEnabled = false;
        }
    }
}

SvLocal void
QBEPGLogicConfigMonitorGotResponse(SvObject self_, SvHashTable results)
{
    QBEPGLogic self = (QBEPGLogic) self_;
    SvObject catchupAvailabilityVal = SvHashTableFind(results, (SvObject) SVSTRING("catchup_availability"));
    if (catchupAvailabilityVal && SvObjectIsInstanceOf(catchupAvailabilityVal, SvValue_getType()) && SvValueIsString((SvValue) catchupAvailabilityVal)) {
        SvString catchupAvailabilityStr = SvValueGetString((SvValue) catchupAvailabilityVal);
        SvTime backTimeLimit;

        if (QBStringToTimeDuration(SvStringCString(catchupAvailabilityStr), QBTimeDurationFormat_ISO8601_TIME, &backTimeLimit) <= 0) {
            SvLogWarning("%s(): invalid catchup availability range format", __func__);
            return;
        }

        int backLimit = -SvTimeGetSeconds(backTimeLimit);
        int forwardLimit = 0;
        SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                             SVSTRING("SvEPGManager"));
        SvEPGManagerGetTimeLimit(epgManager, NULL, &forwardLimit, NULL);
        SvEPGManagerSetTimeLimit(epgManager, backLimit, forwardLimit, NULL);
        self->catchupAvailabilityLimitEnabled = true;
        self->catchupAvailabilityLimit = backLimit;
    }
}

SvLocal void
QBEPGLogicConfigMonitorNoResponse(SvObject self_)
{
}

SvLocal SvType
QBEPGLogic_getType(void)
{
    static SvType type = NULL;

    static struct QBMWServiceMonitorListener_t innov8onServiceMonitorMethods = {
        .serviceStatusChanged = QBEPGLogicServiceStatusChanged,
    };

    static struct QBMWConfigMonitorListener_ configMonitorListenerMethods = {
        .gotResponse = QBEPGLogicConfigMonitorGotResponse,
        .noResponse  = QBEPGLogicConfigMonitorNoResponse
    };

    if (!type) {
        SvTypeCreateManaged("QBEPGLogic",
                            sizeof(struct QBEPGLogic_t),
                            SvObject_getType(), &type,
                            QBMWServiceMonitorListener_getInterface(), &innov8onServiceMonitorMethods,
                            QBMWConfigMonitorListener_getInterface(), &configMonitorListenerMethods,
                            NULL);
    }
    return type;
}

SvLocal bool
QBEPGLogicIsRecordShown(QBEPGLogic self,
                                     QBWindowContext epg_)
{
    if (!self->appGlobals->pvrProvider) {
        return false;
    }

    SvEPGEvent event = NULL;
    svNewTVGuideGetActiveEvent(epg_, &event);

    if (!event)
        return false;

    QBPVRRecording ret = NULL;
    QBPVRProviderGetRecordingsParams params = {
        .playable = false,
    };
    params.inStates[QBPVRRecordingState_scheduled] = true;
    params.inStates[QBPVRRecordingState_active] = true;

    SvArray results = QBPVRProviderFindRecordingsInTimeRange(self->appGlobals->pvrProvider,
                                                         SvValueGetString(event->channelID),
                                                         SvTimeConstruct(event->startTime, 0),
                                                         SvTimeConstruct(event->endTime, 0),
                                                         params);
    if (results && SvArrayCount(results) > 0) {
        SvIterator it = SvArrayIterator(results);
        QBPVRRecording recording;
        while((recording = (QBPVRRecording) SvIteratorGetNext(&it))) {
            if (recording->event) {
                SvValue evId1 = SvEPGEventCreateID(event);
                SvValue evId2 = SvEPGEventCreateID(recording->event);
                if (!evId1 || !evId2) {
                    SVTESTRELEASE(evId1);
                    SVTESTRELEASE(evId2);
                    break;
                }
                bool equalEvents = SvObjectEquals((SvObject) evId1, (SvObject) evId2);
                SVRELEASE(evId1);
                SVRELEASE(evId2);

                if (equalEvents) {
                    ret = recording;
                    break;
                }
            } else if (SvObjectEquals((SvObject) recording->channelId, (SvObject) SvValueGetString(event->channelID)) &&
                       SvTimeCmp(SvTimeConstruct(event->startTime, 0), recording->endTime) < 0 &&
                       SvTimeCmp(recording->startTime, SvTimeConstruct(event->endTime, 0)) < 0 ) {
                ret = recording;
                break;
            }
        }
    }
    SVTESTRELEASE(results);

    return ret != NULL;
}

QBEPGLogic
QBEPGLogicNew(AppGlobals appGlobals)
{
    QBEPGLogic self = (QBEPGLogic) SvTypeAllocateInstance(QBEPGLogic_getType(), NULL);
    self->appGlobals = appGlobals;

    return self;
}

SvLocal bool
QBEPGLogicTryScheduleNPvrRecordingForEvent(QBEPGLogic self)
{
    QBWindowContext epg = self->appGlobals->newTVGuide;
    SvEPGEvent event = NULL;
    bool isSideMenuShown = false;
    bool isScheduled = false;

    if (self->appGlobals->nPVRProvider == NULL) {
        goto fini;
    }

    QBnPVRRecordingType scheduledRecordingType = QBnPVRRecordingType_none;
    svNewTVGuideGetActiveEvent(epg, &event);
    if (event) {
        scheduledRecordingType = QBNPvrLogicTryScheduleRecordingForEvent(self->appGlobals, event, &isScheduled);
        if ((scheduledRecordingType != QBnPVRRecordingType_none) && !svNewTVGuideIsSideMenuShown(epg)) {
            if (scheduledRecordingType == QBnPVRRecordingType_keyword) {
                isSideMenuShown = true;
                svNewTVGuideShowSideMenu(epg, SVSTRING(""), NewTVGuideSideMenuType_NPvrKeyword);
            } else if (scheduledRecordingType == QBnPVRRecordingType_series) {
                isSideMenuShown = true;
                svNewTVGuideShowSideMenu(epg, SVSTRING(""), NewTVGuideSideMenuType_NPvrSeries);
            }
        }
    }

fini:
    return isSideMenuShown || isScheduled;
}

bool
QBEPGLogicHandleInputEvent(QBEPGLogic self, SvInputEvent ev)
{
    QBWindowContext epg = self->appGlobals->newTVGuide;

    bool isRecordOn = false;
    if (ev->ch == QBKEY_REC) {
        isRecordOn = QBEPGLogicIsRecordShown(self, epg);
        if (isRecordOn) {
            return true;
        }
        QBAppPVRDiskState diskState = QBAppStateServiceGetPVRDiskState(self->appGlobals->appState);
        if (diskState == QBPVRDiskState_pvr_present || diskState == QBPVRDiskState_pvr_ts_present) {
            svNewTVGuideShowRecord(epg);
            return true;
        }

        bool isScheduled = QBEPGLogicTryScheduleNPvrRecordingForEvent(self);
        if (isScheduled) {
            return true;
        }
    } else if (ev->ch == QBKEY_STOP) {
        if (!(isRecordOn = QBEPGLogicIsRecordShown(self, epg)))
            return true;
    } else if (ev->ch == QBKEY_INFO) {
        if (!svNewTVGuideIsSideMenuShown(epg)) {
            self->extendedInfo = true;
            svNewTVGuideShowExtendedInfo(epg);
        } else if (self->extendedInfo) {
            self->extendedInfo = false;
            svNewTVGuideHideSideMenu(epg);
        }
        return true;
    }

    if (ev->ch == QBKEY_FUNCTION || ev->ch == QBKEY_ENTER || isRecordOn) {
        if (svNewTVGuideIsSideMenuShown(epg))
            svNewTVGuideHideSideMenu(epg);
        else
        {
            // AMERELES CatchUp ReminderEditorPanel seleccionar el primero de la lista
            //svNewTVGuideShowSideMenu(epg, SVSTRING(""), NewTVGuideSideMenuType_Full);
            svNewTVGuideShowSideMenuOnIndex(epg, 0, NewTVGuideSideMenuType_Full);
        }
        return true;
    }

    return false;
}

void
QBEPGLogicDefaultViewSet(AppGlobals appGlobals, QBWindowContext ctx)
{
    if(ctx == appGlobals->newTVGuide)
        QBConfigSet("EPGVIEW", "grid");
    else
        QBConfigSet("EPGVIEW", "vertical");

    QBConfigSave();
}

bool
QBEPGLogicUpdateDefaultTimeLimitsForViews(QBEPGLogic self, int *backTimeLimit, int *forwardTimeLimit)
{
    if (!self || !backTimeLimit || !forwardTimeLimit) {
        goto fini;
    }

    if (!self->startCatchupServiceEnabled) {
        //block back in time if StartCatchup service is disabled
        *backTimeLimit = 0;
        return true;
    }

    if (self->catchupAvailabilityLimitEnabled) {
        //set back in time limit to value received from stb.GetConfiguration in field catchup_availability
        *backTimeLimit = self->catchupAvailabilityLimit;
        return true;
    }

fini:
    return false;
}

void
QBEPGLogicHandleManualChannelSelection(QBEPGLogic self, SvTVChannel channel)
{
    if (!self || !channel) {
        SvLogError("%s(): NULL arguments passed", __func__);
        return;
    }
    svNewTVGuideSetChannel(self->appGlobals->newTVGuide, channel);
}

void
QBEPGLogicStart(QBEPGLogic self)
{
    if (!self) {
        SvLogError("%s(): NULL self passed", __func__);
        return;
    }
    if (self->started) {
        SvLogError("%s(): EPG logic already started", __func__);
        return;
    }

    if (QBAppTypeIsIP() || QBAppTypeIsHybrid()) {
        if (self->appGlobals->serviceMonitor) {
            QBMWServiceMonitorAddListener(self->appGlobals->serviceMonitor, (SvObject) self);
        }
        QBMWConfigMonitorAddListener(self->appGlobals->qbMWConfigMonitor, (SvObject) self);
    }

    self->started = true;
}

void
QBEPGLogicStop(QBEPGLogic self)
{
    if (!self) {
        SvLogError("%s(): NULL self passed", __func__);
        return;
    }
    if (!self->started) {
        SvLogError("%s(): EPG logic not started", __func__);
        return;
    }

    if (self->appGlobals->serviceMonitor) {
        QBMWServiceMonitorRemoveListener(self->appGlobals->serviceMonitor, (SvObject) self);
    }
    QBMWConfigMonitorRemoveListener(self->appGlobals->qbMWConfigMonitor, (SvObject) self);

    self->started = false;
}

void
QBEPGLogicPostWidgetsCreate(QBEPGLogic self)
{
}

void
QBEPGLogicPreWidgetsDestroy(QBEPGLogic self)
{
}

SvString
QBEPGLogicGetChannelName(QBEPGLogic self, SvTVChannel channel)
{
    return NULL;
}

time_t
QBEPGLogicGetCatchupAvailabilityLimitSecond(QBEPGLogic self)
{
    if (!self->catchupAvailabilityLimitEnabled) {
       return 0;
    }
    time_t backlimit = -self->catchupAvailabilityLimit;
    return backlimit;
}

bool QBEPGLogicShouldEventBeHighlighted(QBEPGLogic self, SvEPGEvent event)
{
    if (!event || !self) {
        SvLogError("%s(): NULL arguments passed", __func__);
        return false;
    }

    bool eventCanBeRecorded = self->appGlobals->nPVRProvider && SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, isEventRecordable, event);

    return eventCanBeRecorded;
}

