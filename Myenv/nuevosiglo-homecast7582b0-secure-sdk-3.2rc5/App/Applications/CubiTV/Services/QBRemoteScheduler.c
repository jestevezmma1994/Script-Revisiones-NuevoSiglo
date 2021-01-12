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

#include "QBRemoteScheduler.h"

#include <main.h>
#include <Services/core/playlistManager.h>
#include <Services/core/QBPushReceiver.h>
#include <QBPVRProvider.h>
#include <QBPVRProviderRequest.h>
#include <QBPVRRecording.h>
#include <QBPVRTypes.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <Services/core/QBMiddlewareManager.h>
#include <SvXMLRPCClient/SvXMLRPCClientListener.h>
#include <SvXMLRPCClient/SvXMLRPCDataReader.h>
#include <SvXMLRPCClient/SvXMLRPCRequest.h>
#include <SvXMLRPCClient/SvXMLRPCRequestsQueue.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvHTTPClient/SvHTTPRequest.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvLocalTime.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Logic/timeFormat.h>

#define INNOV8ON_REQUEST_VERSION "2"
#define REMOTE_SCHEDULER_ID "RS"

#define QB_REMOTE_SCHEDULER_DEBUG 0

#define log_error(fmt, ...) SvLogError(  COLBEG() "QBRemoteScheduler :: " fmt COLEND_COL(red), ##__VA_ARGS__)
#define log_debug(fmt, ...) do { if (QB_REMOTE_SCHEDULER_DEBUG) SvLogNotice( COLBEG() "QBRemoteScheduler :: " fmt COLEND_COL(blue), ##__VA_ARGS__); } while (0)

#define REC_META_KEY__SCHED_REMOTE_ID "sched:sched_id"

#define UPDATE_RECORDINGS_AFTER_MS  (10 * 1000)

struct QBRemoteScheduler_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    SvXMLRPCRequestsQueue xmlRPCReqQueue;

    SvArray recordingsToSend;

    SvFiber fiber;
    SvFiberTimer timer;
    SvScheduler scheduler;

    SvHashTable idToDesc;

    bool started;
};

SvLocal SvString
QBRemoteSchedulerGetCustomerId(QBRemoteScheduler self)
{
    if (!self->appGlobals->middlewareManager) {
        log_error("middlewareManager is not set");
        return NULL;
    }

    SvString customerId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    if (!customerId) {
        log_error("Failed to get customerId");
        return NULL;
    }

    return customerId;
}

SvLocal SvXMLRPCServerInfo
QBRemoteSchedulerGetServerInfo(QBRemoteScheduler self)
{
    if (!self->appGlobals->middlewareManager) {
        log_error("middlewareManager is not set");
        return NULL;
    }

    SvXMLRPCServerInfo serverInfo = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    if (!serverInfo) {
        log_error("Failed to get serverInfo");
        return NULL;
    }

    return serverInfo;
}

SvLocal bool
QBRemoteSchedulerHasNetwork(QBRemoteScheduler self)
{
    if (!self->appGlobals->middlewareManager) {
        log_error("middlewareManager is not set");
        return false;
    }

    return QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);
}

SvLocal SvHashTable
QBRemoteSchedulerPrepareBasicData(time_t startTime, time_t endTime, SvString channelId, SvString status)
{
    SvHashTable basicData = SvHashTableCreate(4, NULL);

    SvString startTimeStr = SvStringCreateWithFormat("%lld", (long long) startTime);
    SvHashTableInsert(basicData, (SvGenericObject) SVSTRING("start"), (SvGenericObject) startTimeStr);
    SVRELEASE(startTimeStr);

    SvString endTimeStr = SvStringCreateWithFormat("%lld", (long long) endTime);
    SvHashTableInsert(basicData, (SvGenericObject) SVSTRING("end"), (SvGenericObject) endTimeStr);
    SVRELEASE(endTimeStr);

    if (channelId)
        SvHashTableInsert(basicData, (SvGenericObject) SVSTRING("channelId"), (SvGenericObject) channelId);
    SvHashTableInsert(basicData, (SvGenericObject) SVSTRING("status"), (SvGenericObject) status);

    return basicData;
}

SvLocal SvString
QBRemoteSchedulerGenerateScheduleId(void)
{
    return SvStringCreateWithFormat("sched-%08x", rand());
}

SvLocal const char *
QBRemoteSchedulerRecordingStateToString(QBPVRRecordingState state)
{
    switch (state) {
        case QBPVRRecordingState_scheduled:
            return "Scheduled";
        case QBPVRRecordingState_active:
            return "Active";
        case QBPVRRecordingState_completed:
        case QBPVRRecordingState_interrupted:
        case QBPVRRecordingState_stoppedManually:
        case QBPVRRecordingState_stoppedNoSpace:
        case QBPVRRecordingState_stoppedNoSource:
            return "Completed";
        case QBPVRRecordingState_missed:
            return "Missed";
        case QBPVRRecordingState_failed:
            return "Failed";
        case QBPVRRecordingState_removed :
            return "Removed";
        default:
            return "<unknown>";
    };
}

SvLocal SvHashTable
QBRemoteSchedulerCreateRecBasicData(QBRemoteScheduler self, QBPVRRecording recording)
{
    time_t startTime = (time_t) SvTimeGetSeconds(recording->startTime);
    time_t endTime = (time_t) SvTimeGetSeconds(recording->endTime);

    SvString state = SvStringCreate(QBRemoteSchedulerRecordingStateToString(recording->state), NULL);
    SvHashTable basicData = QBRemoteSchedulerPrepareBasicData(startTime, endTime, recording->channelId, state);
    SVRELEASE(state);

    SvValue scheduleIdVal = (SvValue) QBPVRProviderGetRecordingProperty(self->appGlobals->pvrProvider,
                                                                        recording,
                                                                        SVSTRING(REC_META_KEY__SCHED_REMOTE_ID));
    SvString scheduleId = scheduleIdVal ? SvValueGetString(scheduleIdVal) : NULL;
    if (!scheduleId) {
        scheduleId = QBRemoteSchedulerGenerateScheduleId();
        SvValue scheduleNewId = SvValueCreateWithString(scheduleId, NULL);
        QBPVRProviderSetRecordingProperty(self->appGlobals->pvrProvider,
                                          recording,
                                          SVSTRING(REC_META_KEY__SCHED_REMOTE_ID),
                                          (SvObject) scheduleNewId);
        SVRELEASE(scheduleNewId);

        SvHashTableInsert(basicData, (SvGenericObject) SVSTRING("scheduleId"), (SvGenericObject) scheduleId);
        SVRELEASE(scheduleId);
    } else {
        SvHashTableInsert(basicData, (SvGenericObject) SVSTRING("scheduleId"), (SvGenericObject) scheduleId);
    }

    return basicData;
}

SvLocal void
QBRemoteSchedulerSendUpdate(QBRemoteScheduler self, SvArray recordings, bool overwrite)
{
    SvString customerId = QBRemoteSchedulerGetCustomerId(self);
    SvXMLRPCServerInfo serverInfo = QBRemoteSchedulerGetServerInfo(self);
    bool hasNetwork = QBRemoteSchedulerHasNetwork(self);
    if (!customerId || !serverInfo || !hasNetwork) {
        SvLogWarning("%s cannot create request - server=%p, customerId=%p, hasNetwork=%s",
                     __func__, serverInfo, customerId, hasNetwork ? "yes" : "no");
        return;
    }

    SvArray recordingsUpdateData = SvArrayCreate(NULL);
    SvIterator it = SvArrayIterator(recordings);
    QBPVRRecording recording;
    while ((recording = (QBPVRRecording)SvIteratorGetNext(&it))) {
        SvHashTable recUpdateData = QBRemoteSchedulerCreateRecBasicData(self, recording);
        SvArrayAddObject(recordingsUpdateData, (SvGenericObject) recUpdateData);
        SVRELEASE(recUpdateData);
    }

    SvArray params = SvArrayCreateWithCapacity(3, NULL);

    SvArrayAddObject(params, (SvGenericObject) customerId);
    SvArrayAddObject(params, (SvGenericObject) recordingsUpdateData);

    SvValue overwriteV = SvValueCreateWithBoolean(overwrite, NULL);
    SvArrayAddObject(params, (SvGenericObject) overwriteV);
    SVRELEASE(overwriteV);

    SvXMLRPCRequest req = (SvXMLRPCRequest) SvTypeAllocateInstance(SvXMLRPCRequest_getType(), NULL);
    SvXMLRPCRequestInit(req, serverInfo, SVSTRING("stb.PvrUpdate"), (SvImmutableArray) params, NULL);
    SvXMLRPCRequestSetListener(req, (SvGenericObject) self, NULL);
    SVRELEASE(recordingsUpdateData);
    SVRELEASE(params);

    SvXMLRPCRequestsQueueAddToProcess(self->xmlRPCReqQueue, req);
    SVRELEASE(req);
}

SvLocal void
QBRemoteSchedulerUpdateRecordings(QBRemoteScheduler self)
{
    QBPVRProviderGetRecordingsParams params = {
        .includeRecordingsInDirectories = true,
        .playable = true,
    };
    params.inStates[QBPVRRecordingState_active] = true;
    params.inStates[QBPVRRecordingState_completed] = true;
    params.inStates[QBPVRRecordingState_scheduled] = true;

    SvArray recordings = QBPVRProviderGetRecordings(self->appGlobals->pvrProvider,
                                                    params);
    QBRemoteSchedulerSendUpdate(self, recordings, true);
    SVRELEASE(recordings);
}

SvLocal void
QBRemoteSchedulerAddRecordingToUpdate(QBRemoteScheduler self,
                                      QBPVRRecording recording)
{
    if (SvArrayContainsObject(self->recordingsToSend, (SvObject) recording))
        return;

    SvArrayAddObject(self->recordingsToSend, (SvObject) recording);
}

SvLocal void
QBRemoteSchedulerDestroy(void *self_)
{
    QBRemoteScheduler self = (QBRemoteScheduler) self_;

    if (self->fiber)
        SvFiberDestroy(self->fiber);

    SVRELEASE(self->xmlRPCReqQueue);
    SVRELEASE(self->recordingsToSend);
}

SvLocal int
QBRemoteSchedulerCreateScheduleResponse(QBRemoteScheduler self, SvString methodName, SvHashTable respData)
{
    SvString customerId = QBRemoteSchedulerGetCustomerId(self);
    SvXMLRPCServerInfo serverInfo = QBRemoteSchedulerGetServerInfo(self);
    bool hasNetwork = QBRemoteSchedulerHasNetwork(self);
    if (!customerId || !serverInfo || !hasNetwork)
        return -1;

    SvArray xmlRPCparams = SvArrayCreateWithCapacity(2, NULL);
    SvArrayAddObject(xmlRPCparams, (SvGenericObject) customerId);
    SvArrayAddObject(xmlRPCparams, (SvGenericObject) respData);

    SvXMLRPCRequest req = (SvXMLRPCRequest) SvTypeAllocateInstance(SvXMLRPCRequest_getType(), NULL);
    SvXMLRPCRequestInit(req, serverInfo, methodName, (SvImmutableArray) xmlRPCparams, NULL);
    SvXMLRPCRequestSetListener(req, (SvGenericObject) self, NULL);
    SVRELEASE(xmlRPCparams);

    SvXMLRPCRequestsQueueAddToProcess(self->xmlRPCReqQueue, req);
    SVRELEASE(req);

    return 0;
}

SvLocal void
QBRemoteSchedulerScheduleCancelSingleRec(QBRemoteScheduler self,
                                         time_t startTime,
                                         time_t endTime,
                                         SvTVChannel channel,
                                         SvString channelId)
{
    int res = -1;
    QBPVRProviderGetRecordingsParams params = {
        .inStates[QBPVRRecordingState_scheduled] = true,
    };

    SvArray results = QBPVRProviderFindRecordingsInTimeRange(self->appGlobals->pvrProvider,
                                                             channelId,
                                                             SvTimeConstruct(startTime, 0),
                                                             SvTimeConstruct(endTime, 0),
                                                             params);

    if (!results) {
        log_debug("No recordings for given time");
        goto fini;
    }

    // It has to be exactly one schedule for this request
    if (SvArrayCount(results) != 1) {
        log_debug("More than one schedule for given data: [cnt=%zu]", SvArrayCount(results));
        goto fini;
    }

    QBPVRRecording rec = (QBPVRRecording) SvArrayAt(results, 0);
    if (!rec->lock) {
        QBPVRProviderRequest request = QBPVRProviderDeleteRecording(self->appGlobals->pvrProvider, rec);
        if (QBPVRProviderDiskRequestGetStatus(request) == QBPVRProviderDiskRequestStatus_success)
            res = 0;
        SVRELEASE(request);
    }

fini:
    SVTESTRELEASE(results);
    SvString status = res == 0 ? SVSTRING("canceled") : SVSTRING("failed");
    SvHashTable recData = QBRemoteSchedulerPrepareBasicData(startTime, endTime, channelId, status);

    QBRemoteSchedulerCreateScheduleResponse(self, SVSTRING("stb.CancelRecording"), recData);
    SVRELEASE(recData);
}

SvLocal void
QBRemoteSchedulerScheduleSingleRec(QBRemoteScheduler self,
                                   time_t startTime,
                                   time_t endTime,
                                   SvTVChannel channel,
                                   SvString channelId,
                                   SvString scheduleId)
{
    SvString status;
    int result = -1;

    const QBPVRProviderCapabilities *providerCapabilities =
                QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider);
    if (!providerCapabilities->manual) {
        log_debug("Provider isn't capable of manual recordings");
        goto fini;
    }

    QBPVRRecordingSchedParams params = (QBPVRRecordingSchedParams)
              SvTypeAllocateInstance(QBPVRRecordingSchedParams_getType(), NULL);
    params->channel = SVRETAIN(channel);
    params->startTime = SvTimeConstruct(startTime, 0);
    params->stopTime = SvTimeConstruct(endTime, 0);
    params->type = QBPVRRecordingType_OTR;

    QBPVRProviderRequest request = QBPVRProviderScheduleRecording(self->appGlobals->pvrProvider, params);
    SVRELEASE(params);
    QBPVRRecording recording = QBPVRProviderRequestGetRecording(request);
    if (QBPVRProviderDiskRequestGetStatus(request) == QBPVRProviderDiskRequestStatus_success &&
        recording) {
        SvValue scheduleIdVal = SvValueCreateWithString(scheduleId, NULL);
        QBPVRProviderSetRecordingProperty(self->appGlobals->pvrProvider,
                                          recording,
                                          SVSTRING(REC_META_KEY__SCHED_REMOTE_ID),
                                          (SvObject) scheduleIdVal);
        SVRELEASE(scheduleIdVal);
        result = 0;
    }
    SVRELEASE(request);

fini:
    status = SvStringCreateWithFormat("%s", result == 0 ? "Scheduled" : "Failed");
    SvHashTable recData = QBRemoteSchedulerPrepareBasicData(startTime, endTime, channelId, status);
    SvHashTableInsert(recData, (SvGenericObject) SVSTRING("scheduleId"), (SvGenericObject) scheduleId);
    SVRELEASE(status);

    QBRemoteSchedulerCreateScheduleResponse(self, SVSTRING("stb.SetRecording"), recData);
    SVRELEASE(recData);
}

SvLocal void
QBRemoteSchedulerScheduleKeywordRec(QBRemoteScheduler self,
                                    SvString keyword,
                                    SvTVChannel channel,
                                    SvString channelId)
{
    SvHashTable recData;
    int result = -1;

    if (SvStringLength(keyword) < 2) {
        log_error("Scheduling failed: Keyword to short");
        goto fini;
    }

    const QBPVRProviderCapabilities *providerCapabilities =
                QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider);
    if (!providerCapabilities->keyword) {
        log_debug("Provider isn't capable of keyword recordings");
        goto fini;
    }

    QBPVRRecordingSchedParams params = (QBPVRRecordingSchedParams)
              SvTypeAllocateInstance(QBPVRRecordingSchedParams_getType(), NULL);
    params->channel = SVTESTRETAIN(channel);
    params->keyword = SVRETAIN(keyword);
    params->type = QBPVRRecordingType_keyword;

    QBPVRProviderRequest request = QBPVRProviderScheduleRecording(self->appGlobals->pvrProvider, params);
    SVRELEASE(params);
    if (QBPVRProviderDiskRequestGetStatus(request) == QBPVRProviderDiskRequestStatus_success)
        result = 0;
    SVRELEASE(request);

fini:
    recData = SvHashTableCreate(3, NULL);
    SvHashTableInsert(recData, (SvGenericObject) SVSTRING("status"), result == 0 ? (SvGenericObject) SVSTRING("Scheduled") : (SvGenericObject) SVSTRING("Failed"));
    SvHashTableInsert(recData, (SvGenericObject) SVSTRING("keyword"), (SvGenericObject) keyword);
    SvHashTableInsert(recData, (SvGenericObject) SVSTRING("channelId"), channelId ? (SvGenericObject) channelId : (SvGenericObject) SVSTRING(""));

    QBRemoteSchedulerCreateScheduleResponse(self, SVSTRING("stb.SetKeywordRecording"), recData);
    SVRELEASE(recData);
}

SvLocal void
QBRemoteSchedulerScheduleRecurringRec(QBRemoteScheduler self,
                                      time_t startTime,
                                      time_t endTime,
                                      SvTVChannel channel,
                                      SvString channelId,
                                      SvString recurrenceStr)
{
    SvString status;
    int result = -1;

    const QBPVRProviderCapabilities *providerCapabilities =
                QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider);
    if (!providerCapabilities->repeated) {
        log_debug("Provider isn't capable of repeated recordings");
        goto err;
    }

    QBPVRRepeatedRecordingsMode recurrence = QBPVRRepeatedRecordingsModeFromName(SvStringCString(recurrenceStr));
    if (recurrence == QBPVRRepeatedRecordings_None) {
        log_debug("Invalid recurrence mode");
        goto err;
    }

    char buf1[256], buf2[256];
    struct tm local;
    SvTimeBreakDown(SvTimeConstruct(startTime, 0), true, &local);
    strftime(buf1, sizeof(buf1), QBTimeFormatGetCurrent()->manualRecordingName, &local);
    SvTimeBreakDown(SvTimeConstruct(endTime, 0), true, &local);
    strftime(buf2, sizeof(buf2), QBTimeFormatGetCurrent()->manualRecordingName, &local);
    SvString name = SvStringCreateWithFormat("%s - %s", buf1, buf2);

    QBPVRRecordingSchedParams params = (QBPVRRecordingSchedParams)
              SvTypeAllocateInstance(QBPVRRecordingSchedParams_getType(), NULL);
    params->name = SVRETAIN(name);
    params->channel = SVRETAIN(channel);
    params->repeatedInfo.start = SvTimeConstruct(startTime, 0);
    params->repeatedInfo.duration = difftime(endTime, startTime);
    params->repeatedInfo.mode = recurrence;
    params->type = QBPVRRecordingType_repeated;

    QBPVRProviderRequest request = QBPVRProviderScheduleRecording(self->appGlobals->pvrProvider, params);
    SVRELEASE(params);
    if (QBPVRProviderDiskRequestGetStatus(request) == QBPVRProviderDiskRequestStatus_success)
        result = 0;
    SVRELEASE(request);
    SVRELEASE(name);
err:
    status = SvStringCreateWithFormat("%s", result == 0 ? "Scheduled" : "Failed");
    SvHashTable recData = QBRemoteSchedulerPrepareBasicData(startTime, endTime, channelId, status);
    SVRELEASE(status);
    SvHashTableInsert(recData, (SvGenericObject) SVSTRING("recurrence"), (SvGenericObject) recurrenceStr);

    QBRemoteSchedulerCreateScheduleResponse(self, SVSTRING("stb.SetRecurringRecording"), recData);
    SVRELEASE(recData);
}

SvLocal SvTVChannel
QBRemoteSchedulerGetChannel(QBPlaylistManager playlistManager, SvValue channelIdV)
{
    SvObject playlist = QBPlaylistManagerGetById(playlistManager, SVSTRING("AllChannels"));
    if (!playlist) {
        log_error("no AllChannels playlist");
        return NULL;
    }

    return SvInvokeInterface(SvEPGChannelView, playlist, getByID, channelIdV);
}

SvLocal void
QBRemoteSchedulerPushDataReceived(SvGenericObject self_,
                                  SvString type,
                                  SvGenericObject data_)
{
    QBRemoteScheduler self = (QBRemoteScheduler) self_;

    SvHashTable data = (SvHashTable) data_;
    if (!type || !SvStringEqualToCString(type, REMOTE_SCHEDULER_ID))
        return;

    SvValue cmdVal = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("command"));
    if (!cmdVal || !SvValueIsString(cmdVal))
        return;

    log_debug("Remote schedule command received");
    SvString cmd = SvValueGetString(cmdVal);
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    if (SvStringEqualToCString(cmd, "set_recording") || SvStringEqualToCString(cmd, "set_recurring_recording") || SvStringEqualToCString(cmd, "cancel_recording")) {
        SvHashTable args = (SvHashTable) SvHashTableFind(data, (SvObject) SVSTRING("args"));
        if (!args || !SvObjectIsInstanceOf((SvObject) args, SvHashTable_getType())) {
            log_error("error parsing [args]");
            return;
        }

        SvValue startTimeV = (SvValue) SvHashTableFind(args, (SvObject) SVSTRING("start"));
        if (!startTimeV || !SvObjectIsInstanceOf((SvObject) startTimeV, SvValue_getType()) || !SvValueIsString(startTimeV)) {
            log_error("error parsing [startTime]");
            return;
        }
        time_t startTime = (time_t) strtoll(SvStringCString(SvValueGetString(startTimeV)), NULL, 10);

        SvValue endTimeV = (SvValue) SvHashTableFind(args, (SvObject) SVSTRING("end"));
        if (!endTimeV || !SvObjectIsInstanceOf((SvObject) endTimeV, SvValue_getType()) || !SvValueIsString(endTimeV)) {
            log_error("error parsing [endTime]");
            return;
        }
        time_t endTime = (time_t) strtoll(SvStringCString(SvValueGetString(endTimeV)), NULL, 10);

        SvValue channelIdV = (SvValue) SvHashTableFind(args, (SvObject) SVSTRING("channelId"));
        if (!channelIdV || !SvObjectIsInstanceOf((SvObject) channelIdV, SvValue_getType()) || !SvValueIsString(channelIdV)) {
            log_error("error parsing [channelId]");
            return;
        }

        SvString channelId = SvValueGetString(channelIdV);
        SvTVChannel channel = QBRemoteSchedulerGetChannel(playlists, channelIdV);
        if (!channel) {
            log_error("didn't find channel with channelID: %s", SvStringCString(channelId));
            return;
        }

        if (SvStringEqualToCString(cmd, "set_recurring_recording")) {
            SvValue recurrenceV = (SvValue) SvHashTableFind(args, (SvObject) SVSTRING("recurrence"));
            if (!recurrenceV || !SvObjectIsInstanceOf((SvObject) recurrenceV, SvValue_getType()) || !SvValueIsString(recurrenceV)) {
                log_error("error parsing [recurrence]");
                return;
            }
            SvString recurrence = SvValueGetString(recurrenceV);
            log_debug("Command set_recurring_recording received");
            QBRemoteSchedulerScheduleRecurringRec(self, startTime, endTime, channel, channelId, recurrence);
        } else if (SvStringEqualToCString(cmd, "cancel_recording")) {
            log_debug("Command cancel_recording received");
            QBRemoteSchedulerScheduleCancelSingleRec(self, startTime, endTime, channel, channelId);
        } else {
            SvString scheduleId;
            SvValue scheduleIdV = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("scheduleId"));
            if (scheduleIdV) {
                if (!SvObjectIsInstanceOf((SvObject) scheduleIdV, SvValue_getType()) || !SvValueIsString(scheduleIdV)) {
                    log_error("error parsing [scheduleId]");
                    return;
                }
                scheduleId = SvValueGetString(scheduleIdV);
            } else {
                scheduleId = QBRemoteSchedulerGenerateScheduleId();
            }
            log_debug("Command set_recording received");
            QBRemoteSchedulerScheduleSingleRec(self, startTime, endTime, channel, channelId, scheduleId);
        }
    } else if (SvStringEqualToCString(cmd, "set_keyword_recording")) {
        SvHashTable args = (SvHashTable) SvHashTableFind(data, (SvObject) SVSTRING("args"));
        if (!args || !SvObjectIsInstanceOf((SvObject) args, SvHashTable_getType()))
            return;

        SvValue keywordV = (SvValue) SvHashTableFind(args, (SvObject) SVSTRING("keyword"));
        if (!keywordV || !SvObjectIsInstanceOf((SvObject) keywordV, SvValue_getType()) || !SvValueIsString(keywordV))
            return;
        SvString keyword = SvValueGetString(keywordV);

        SvValue channelIdV = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("channelId"));
        SvString channelId = NULL;
        if (channelIdV) {
            if (!SvObjectIsInstanceOf((SvObject) channelIdV, SvValue_getType()) || !SvValueIsString(channelIdV))
                return;
            channelId = SvValueGetString(channelIdV);
        }

        SvTVChannel channel = QBRemoteSchedulerGetChannel(playlists, channelIdV);
        if (!channel) {
            log_error("didn't find channel with channelID: %s", SvStringCString(channelId));
            return;
        }

        log_debug("Command set_keyword_recording received");
        QBRemoteSchedulerScheduleKeywordRec(self, keyword, channel, channelId);
    } else if (SvStringEqualToCString(cmd, "pvr_update")) {
        QBRemoteSchedulerUpdateRecordings(self);
    } else {
        SvLogError("Unsupported command");
        return;
    }
}

SvLocal void
QBRemoteSchedulerPushStatusChanged(SvObject self_,
                                   QBPushReceiverStatus status)
{
}

SvLocal void
QBRemoteSchedulerXMLRPCRequestSetup(SvGenericObject self_,
                                    SvXMLRPCRequest req)
{
    SvHTTPRequest httpReq = SvXMLRPCRequestGetHTTPRequest(req, NULL);
    SvHTTPRequestSetHTTPHeader(httpReq, SVSTRING("X-CW-Api-Version"), SVSTRING(INNOV8ON_REQUEST_VERSION), NULL);
}

SvLocal void
QBRemoteSchedulerXMLRPCRequestStateChanged(SvGenericObject self_,
                                           SvXMLRPCRequest req,
                                           SvXMLRPCRequestState state)
{
    QBRemoteScheduler self = (QBRemoteScheduler) self_;

    if (state == SvXMLRPCRequestState_gotAnswer) {
        SvXMLRPCDataReader reader = (SvXMLRPCDataReader) SvXMLRPCRequestGetDataListener(req, NULL);
        SvValue answer = (SvValue) SvXMLRPCDataReaderGetDataItem(reader, 0, NULL);
        if (SvObjectIsInstanceOf((SvObject) answer, SvValue_getType()) && SvValueIsString(answer)) {
            SvString result = (SvString)SvValueGetString(answer);
            log_debug("MW response: %s", SvStringCString(result));
        } else {
            log_error("Invalid MW answer type");
        }
    } else if (state == SvXMLRPCRequestState_gotException) {
        SvString desc;
        int code;
        SvXMLRPCRequestGetException(req, &code, &desc, NULL);
        log_error("Exception: %d: %s", code, SvStringCString(desc));
    } else if (state != SvXMLRPCRequestState_error) {
        return;
    }

    SvXMLRPCRequestsQueueFinishRequest(self->xmlRPCReqQueue);
    SvXMLRPCRequestsQueueProcessNext(self->xmlRPCReqQueue);
}

SvLocal void
QBRemoteSchedulerRecordingAdded(SvObject self_,
                                QBPVRRecording recording)
{
    QBRemoteScheduler self = (QBRemoteScheduler) self_;
    if (recording->state != QBPVRRecordingState_scheduled &&
        recording->state != QBPVRRecordingState_active &&
        !recording->playable)
        return;

    QBRemoteSchedulerAddRecordingToUpdate(self, recording);
    SvFiberTimerActivateAfter(self->timer, SvTimeFromMilliseconds(UPDATE_RECORDINGS_AFTER_MS));
}

SvLocal void
QBRemoteSchedulerRecordingChanged(SvObject self_,
                                  QBPVRRecording recording,
                                  QBPVRRecording oldRecording)
{
    QBRemoteScheduler self = (QBRemoteScheduler) self_;
    if (oldRecording->state != QBPVRRecordingState_scheduled &&
        oldRecording->state != QBPVRRecordingState_active)
        return;

    if (oldRecording->state == recording->state) {
        if (SvTimeCmp(oldRecording->startTime, recording->startTime) == 0 && SvTimeCmp(oldRecording->endTime, recording->endTime) == 0)
            return;
    }

    QBRemoteSchedulerAddRecordingToUpdate(self, recording);
    SvFiberTimerActivateAfter(self->timer, SvTimeFromMilliseconds(UPDATE_RECORDINGS_AFTER_MS));
}

SvLocal void
QBRemoteSchedulerRecordingRemoved(SvObject self_,
                                  QBPVRRecording recording)
{
}

SvLocal void
QBRemoteSchedulerRecordingRestricted(SvObject self_,
                                     QBPVRRecording rec)
{
}

SvLocal void
QBRemoteSchedulerQuotaChanged(SvObject self, QBPVRQuota quota)
{
}

SvLocal void
QBRemoteSchedulerDirectoryAdded(SvObject self_, QBPVRDirectory dir)
{
}

SvLocal SvType
QBRemoteScheduler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRemoteSchedulerDestroy
    };
    static SvType type = NULL;

    static const struct QBPushReceiverListener_t pushReceiverMethods = {
        .dataReceived = QBRemoteSchedulerPushDataReceived,
        .statusChanged = QBRemoteSchedulerPushStatusChanged
    };

    static const struct SvXMLRPCClientListener_t xmlrpcMethods = {
        .stateChanged = QBRemoteSchedulerXMLRPCRequestStateChanged,
        .setup = QBRemoteSchedulerXMLRPCRequestSetup,
    };

    static const struct QBPVRProviderListener_ pvrRecordingMethods = {
        .recordingAdded = QBRemoteSchedulerRecordingAdded,
        .recordingChanged = QBRemoteSchedulerRecordingChanged,
        .recordingRemoved = QBRemoteSchedulerRecordingRemoved,
        .recordingRestricted = QBRemoteSchedulerRecordingRestricted,
        .quotaChanged = QBRemoteSchedulerQuotaChanged,
        .directoryAdded = QBRemoteSchedulerDirectoryAdded,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRemoteScheduler",
                            sizeof(struct QBRemoteScheduler_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPushReceiverListener_getInterface(), &pushReceiverMethods,
                            SvXMLRPCClientListener_getInterface(), &xmlrpcMethods,
                            QBPVRProviderListener_getInterface(), &pvrRecordingMethods,
                            NULL);
    }

    return type;
}

QBRemoteScheduler
QBRemoteSchedulerCreate(AppGlobals appGlobals,
                        int64_t recordingsUpdateDelayMs)
{
    QBRemoteScheduler self = NULL;

    SvXMLRPCRequestsQueue reqQueue = NULL;
    SvArray recordingsToSend = NULL;

    int retval = -1;

    if (!appGlobals) {
        log_error("NULL argument passed: appGlobals");
        goto err;
    }

    reqQueue = SvXMLRPCRequestsQueueCreate();
    if (!reqQueue) {
        log_error("Can't create requests queue");
        goto err;
    }

    recordingsToSend = SvArrayCreate(NULL);
    if (!recordingsToSend) {
        log_error("Can't create SvArray: recordingsToSend");
        goto err;
    }

    self = (QBRemoteScheduler) SvTypeAllocateInstance(QBRemoteScheduler_getType(), NULL);
    if (unlikely(!self)) {
        log_error("can't create QBRemoteScheduler");
        goto err;
    }

    self->xmlRPCReqQueue = reqQueue;
    self->recordingsToSend = recordingsToSend;
    self->appGlobals = appGlobals;

    retval = 0;

err:
    if (retval < 0) {
        SVTESTRELEASE(reqQueue);
        SVTESTRELEASE(recordingsToSend);
    }

    return self;
}

SvLocal void
QBRemoteSchedulerStep(void *self_)
{
    QBRemoteScheduler self = (QBRemoteScheduler) self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    QBRemoteSchedulerSendUpdate(self, self->recordingsToSend, false);
    SvArrayRemoveAllObjects(self->recordingsToSend);
}

int
QBRemoteSchedulerStart(QBRemoteScheduler self,
                       SvScheduler scheduler)
{
    SvErrorInfo error = NULL;

    if (!self) {
        log_error("NULL self passed");
        return -1;
    }

    if (self->started) {
        log_error("Remote scheduler already started");
        return -1;
    }

    if (!self->appGlobals->pvrProvider) {
        log_error("Invalid app state, no pvr provider.");
        return -1;
    }

    const QBPVRProviderCapabilities *providerCapabilities =
                QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider);

    if (providerCapabilities->type != QBPVRProviderType_disk) {
        log_error("PVR provider isn't disk provider");
        return -1;
    }

    if (!self->appGlobals->pushReceiver) {
        log_error("pushReceiver doesn't exist");
        return -1;
    }

    QBPushReceiverAddListener(self->appGlobals->pushReceiver, (SvGenericObject) self,
                              SVSTRING(REMOTE_SCHEDULER_ID), &error);
    if (error) {
        log_error("Can't register pushReceiver listener");
        SvErrorInfoDestroy(error);
        return -1;
    }

    QBPVRProviderAddListener(self->appGlobals->pvrProvider, (SvObject) self);

    self->fiber = SvFiberCreate(scheduler, NULL, "QBRemoteScheduler", QBRemoteSchedulerStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    self->scheduler = scheduler;

    self->started = true;
    return 0;
}

int
QBRemoteSchedulerStop(QBRemoteScheduler self)
{
    SvErrorInfo error = NULL;

    if (!self) {
        log_error("NULL self passed");
        return -1;
    }

    if (!self->started) {
        log_error("Remote scheduler wasn't started");
        return -1;
    }

    SvXMLRPCRequestsQueueCancelAllRequests(self->xmlRPCReqQueue);

    QBPVRProviderRemoveListener(self->appGlobals->pvrProvider, (SvObject) self);

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->timer = NULL;
    }

    if (self->recordingsToSend)
        SvArrayRemoveAllObjects(self->recordingsToSend);

    if (self->appGlobals->pushReceiver)
        return -1;

    QBPushReceiverRemoveListener(self->appGlobals->pushReceiver, (SvGenericObject) self,
                                 &error);
    if (error) {
        log_error("Can't remove pushReceiver listener");
        SvErrorInfoDestroy(error);
        return -1;
    }

    self->started = false;
    return 0;
}

