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

#include "QBDiskPVRRecording.h"

#include "QBDiskPVRProviderInternal.h"
#include <QBPVRRecording.h>
#include <QBPVRRecordingInternal.h>
#include <QBPVRTypes.h>
#include <QBRecordFS/file.h>
#include <QBRecordFS/RecMetaKeys.h>
#include <QBSchedManager.h>
#include <QBTunerTypes.h>
#include <SvPlayerKit/SvContentMetaData.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvAtomic.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLocalTime.h>
#include <SvCore/SvLog.h>

#define REC_META_KEY__LOCK                      "app:lock"            // bool
#define REC_META_KEY__ALREADY_VIEWED            "app:already_viewed"  // bool

struct QBDiskPVRRecording_ {
    struct QBPVRRecording_  super_;           ///< super type
    QBSchedDesc *desc;                        ///< sched manager recording description

    SvString groupId;                         ///< group id from recording meta, by this recording is mapped to directory
    bool hasFile;                             ///< does recording have file
};

SvLocal SvType
QBDiskPVRRecording_getType(void);

SvLocal
void QBDiskPVRRecordingDestroy(void* self_)
{
    QBDiskPVRRecording self = (QBDiskPVRRecording) self_;

    SVTESTRELEASE(self->desc);
    SVTESTRELEASE(self->groupId);
}

SvLocal void *
QBDiskPVRRecordingCopy(void *self_, SvErrorInfo *errorOut)
{
    QBDiskPVRRecording self = self_;
    QBDiskPVRRecording out = (QBDiskPVRRecording) SvTypeAllocateInstance(QBDiskPVRRecording_getType(), NULL);

    QBPVRRecordingCopy((QBPVRRecording) out, (QBPVRRecording) self);
    out->desc = SVTESTRETAIN(self->desc);
    out->groupId = SVTESTRETAIN(self->groupId);

    return out;
}

SvLocal SvType
QBDiskPVRRecording_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDiskPVRRecordingDestroy,
        .copy = QBDiskPVRRecordingCopy,
    };

    if (!type) {
        SvTypeCreateManaged("QBDiskPVRRecording",
                            sizeof(struct QBDiskPVRRecording_),
                            QBPVRRecording_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal SvString
QBDiskPVRRecordingGenerateID(void)
{
    static volatile long int prevID = 0;
    return SvStringCreateWithFormat("DISK PVR %ld", SvAtomicIncrement(&prevID) + 1);
}

SvLocal QBPVRRecordingState
QBDiskPVRRecordingGetState(QBSchedDesc *desc) {
    switch (desc->state) {
        case QBSchedState_scheduled:
            return QBPVRRecordingState_scheduled;
        case QBSchedState_active:
            return QBPVRRecordingState_active;
        case QBSchedState_completed: {
            SvString state = NULL, reason = NULL;
            SvContentMetaDataGetStringProperty(desc->params.meta, SVSTRING(REC_META_KEY__STATE), &state);
            if (SvStringEqualToCString(state, REC_META_STATE__UNCOMPLETED)) {
                return QBPVRRecordingState_interrupted;
            } else if (SvStringEqualToCString(state, REC_META_STATE__STOPPED)) {
                SvContentMetaDataGetStringProperty(desc->params.meta, SVSTRING(REC_META_KEY__STOP_REASON), &reason);
                if (SvStringEqualToCString(reason, REC_META_STOP_REASON__MANUAL))
                    return QBPVRRecordingState_stoppedManually;
                else if (SvStringEqualToCString(reason, REC_META_STOP_REASON__NO_SPACE))
                    return QBPVRRecordingState_stoppedNoSpace;
                else if (SvStringEqualToCString(reason, REC_META_STOP_REASON__SOURCE))
                    return QBPVRRecordingState_stoppedNoSource;
            }
            return QBPVRRecordingState_completed;
        }
        case QBSchedState_missed:
            return QBPVRRecordingState_missed;
        case QBSchedState_failed:
            return QBPVRRecordingState_failed;
        case QBSchedState_removed:
            return QBPVRRecordingState_removed;
    }
    return QBPVRRecordingState_unknown;
}

SvLocal bool
QBDiskPVRRecordingGetLock(QBSchedDesc *desc)
{
    bool lock = false;
    SvContentMetaDataGetBooleanProperty(desc->params.meta, SVSTRING(REC_META_KEY__LOCK), &lock);
    return lock;
}

SvLocal bool
QBDiskPVRRecordingGetViewed(QBSchedDesc *desc)
{
    bool wasViewed = false;
    SvContentMetaDataGetBooleanProperty(desc->params.meta, SVSTRING(REC_META_KEY__ALREADY_VIEWED), &wasViewed);
    return wasViewed;
}

SvLocal bool
QBDiskPVRRecordingGetPlayable(QBSchedDesc *desc)
{
    if (desc->state != QBSchedState_completed &&
        desc->state != QBSchedState_active)
        return false;

    if (!desc->rec.file) {
        SvLogError("%s: no file connected with recording", __func__);
        return false;
    }

    return true;
}

SvLocal bool
QBDiskPVRRecordingGetIsAdult(QBSchedDesc *desc)
{
    SvValue isAdultChannel = desc->extracted.channelIsAdult;
    return isAdultChannel && SvValueGetBoolean(isAdultChannel);
}

SvLocal SvString
QBDiskPVRRecordingGetGroupIdFromSchedDesc(QBSchedDesc *desc)
{
    SvString groupId = NULL;
    SvContentMetaDataGetStringProperty(desc->params.meta, SVSTRING(REC_META_KEY__GROUP_ID), &groupId);
    return groupId;
}

QBDiskPVRRecording
QBDiskPVRRecordingCreate(QBSchedDesc *desc, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!desc) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL desc passed");
        goto err1;
    }

    QBDiskPVRRecording self = (QBDiskPVRRecording)
            SvTypeAllocateInstance(QBDiskPVRRecording_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error,
                                           "can't allocate QBDiskPVRRecording");
        goto err1;
    }

    if (QBPVRRecordingInit((QBPVRRecording) self,
                           &error) < 0) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBPVRRecordingInit() failed.");
        goto err2;
    }

    self->desc = SVRETAIN(desc);
    self->super_.id = QBDiskPVRRecordingGenerateID();
    self->super_.event = SVTESTRETAIN(self->desc->params.event);
    self->super_.epgEvents = SVTESTRETAIN(self->desc->epgEvents);
    self->super_.recoveryOption = QBPVRRecordingSpaceRecovery_DeleteOldest;
    switch ((QBDiskPVRPriority) desc->params.priority) {
        case QBDiskPVRPriority_keyword:
            self->super_.type = QBPVRRecordingType_keyword;
            break;
        case QBDiskPVRPriority_repeat:
            self->super_.type = QBPVRRecordingType_repeated;
            break;
        case QBDiskPVRPriority_event:
            self->super_.type = QBPVRRecordingType_event;
            break;
        case QBDiskPVRPriority_OTR:
            self->super_.type = QBPVRRecordingType_OTR;
            break;
        case QBDiskPVRPriority_manual:
            self->super_.type = QBPVRRecordingType_manual;
            break;
    }
    QBDiskPVRRecordingUpdate(self);

    return self;
err2:
    SVRELEASE(self);
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

void
QBDiskPVRRecordingUpdate(QBDiskPVRRecording self)
{
    self->super_.lock = QBDiskPVRRecordingGetLock(self->desc);
    self->super_.viewed = QBDiskPVRRecordingGetViewed(self->desc);

    SVTESTRELEASE(self->super_.channelId);
    self->super_.channelId = SVTESTRETAIN(self->desc->params.channelId);

    SVTESTRELEASE(self->super_.channelName);
    self->super_.channelName = SVTESTRETAIN(self->desc->extracted.channelName);

    self->super_.startTime = SvTimeConstruct(self->desc->extracted.startTime.utc, 0);
    self->super_.endTime = SvTimeConstruct(self->desc->extracted.stopTime.utc, 0);
    self->super_.expirationTime = SvTimeConstruct(self->desc->extracted.expirationTime.utc, 0);

    self->super_.state = QBDiskPVRRecordingGetState(self->desc);

    self->super_.playable = QBDiskPVRRecordingGetPlayable(self->desc);
    self->super_.isAdult = QBDiskPVRRecordingGetIsAdult(self->desc);

    SVTESTRELEASE(self->groupId);
    self->groupId = SVTESTRETAIN(QBDiskPVRRecordingGetGroupIdFromSchedDesc(self->desc));

    self->hasFile = self->desc->rec.file ? true : false;

    QBObservableSetChangedAndNotify((QBObservable) self, NULL, NULL);
}

const QBSchedDesc *
QBDiskPVRRecordingGetSchedDesc(QBDiskPVRRecording self)
{
    return self->desc;
}

SvString
QBDiskPVRRecordingGetGroupId(QBDiskPVRRecording self)
{
    return self->groupId;
}

bool
QBDiskPVRRecordingHasFile(QBDiskPVRRecording self)
{
    return self->hasFile;
}

void
QBDiskPVRRecordingSetLock(QBDiskPVRRecording self, bool lock)
{
    if (lock)
        SvContentMetaDataSetBooleanProperty(self->desc->params.meta, SVSTRING(REC_META_KEY__LOCK), true);
    else
        SvContentMetaDataDelete(self->desc->params.meta, SVSTRING(REC_META_KEY__LOCK));
    QBSchedManagerPropagateChange(self->desc);
}

void
QBDiskPVRRecordingSetAsAlreadyViewed(QBDiskPVRRecording self)
{
    bool wasViewed = false;
    SvContentMetaDataGetBooleanProperty(self->desc->params.meta, SVSTRING(REC_META_KEY__ALREADY_VIEWED), &wasViewed);
    if (!wasViewed) {
        SvContentMetaDataSetBooleanProperty(self->desc->params.meta, SVSTRING(REC_META_KEY__ALREADY_VIEWED), true);
        QBSchedManagerPropagateChange(self->desc);
    }
}

void
QBDiskPVRRecordingSetGroupId(QBDiskPVRRecording self, SvString groupId)
{
    if (groupId)
        SvContentMetaDataSetStringProperty(self->desc->params.meta, SVSTRING(REC_META_KEY__GROUP_ID), groupId);
    else
        SvContentMetaDataDelete(self->desc->params.meta, SVSTRING(REC_META_KEY__GROUP_ID));
    QBSchedManagerPropagateChange(self->desc);
}

void
QBDiskPVRRecordingSetRecordingProperty(QBDiskPVRRecording self,
                                       SvString key,
                                       SvObject value)
{
    if (value) {
        SvContentMetaDataSetObjectProperty(self->desc->params.meta, key, value);
    } else {
        SvContentMetaDataDelete(self->desc->params.meta, key);
    }
    QBSchedManagerPropagateChange(self->desc);
}

SvObject
QBDiskPVRRecordingGetRecordingProperty(QBDiskPVRRecording self,
                                       SvString key)
{
    SvObject value = NULL;
    SvContentMetaDataGetObjectProperty(self->desc->params.meta, key, &value);
    return value;
}

void
QBDiskPVRRecordingChangeStartEndTime(QBDiskPVRRecording self,
                                     SvTime startTime,
                                     SvTime endTime)
{
    QBSchedManagerChange(self->desc,
                         SvLocalTimeFromUTC(SvTimeGetSeconds(startTime)),
                         SvLocalTimeFromUTC(SvTimeGetSeconds(endTime)));
}

void
QBDiskPVRRecordingStop(QBDiskPVRRecording self)
{
    QBSchedManagerStopRecording(self->desc);
}

void
QBDiskPVRRecordingDelete(QBDiskPVRRecording self)
{
    QBSchedManagerDeleteRecording(self->desc);
}

int64_t
QBDiskPVRRecordingGetFileSize(QBDiskPVRRecording self)
{
    if (!self->desc->rec.file)
        return -1;

    QBRecordFSFileInfo info;
    QBRecordFSFileGetInfo(self->desc->rec.file, &info);
    return info.size;
}

int
QBDiskPVRRecordingGetDuration(QBDiskPVRRecording self)
{
    if (!self->desc->rec.file)
        return 0;

    QBRecordFSFileInfo info;
    QBRecordFSFileGetInfo(self->desc->rec.file, &info);
    return (int) info.duration;
}

const struct QBTunerMuxId *
QBDiskPVRRecordingGetMuxId(QBDiskPVRRecording self)
{
    return &self->desc->extracted.tunerParams.mux_id;
}

SvContent
QBDiskPVRRecordingCreateContent(QBDiskPVRRecording self)
{
    return self->desc->rec.file ? QBRecordFSCreateContent(self->desc->rec.file) : NULL;
}

bool
QBDiskPVRRecordingWillRecordingChangePosition(QBDiskPVRRecording self)
{
    QBPVRRecordingState newState = QBDiskPVRRecordingGetState(self->desc);
    bool newPlayable = QBDiskPVRRecordingGetPlayable(self->desc);
    if ((self->super_.state != newState) ||
        (self->super_.playable != newPlayable))
        return true;

    SvString newGroupId = QBDiskPVRRecordingGetGroupIdFromSchedDesc(self->desc);
    if (!(SvObjectEquals((SvObject) self->groupId, (SvObject) newGroupId)))
        return true;

    return false;
}
