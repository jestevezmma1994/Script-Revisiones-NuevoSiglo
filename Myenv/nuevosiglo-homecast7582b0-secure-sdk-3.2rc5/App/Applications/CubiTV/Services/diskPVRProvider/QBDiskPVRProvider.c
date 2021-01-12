/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBDiskPVRProvider.h"
#include "QBDiskPVRDirectory.h"
#include "QBDiskPVRProviderInternal.h"
#include <QBPVRDirectoryInternal.h>

#include <main.h>
#include "QBDiskPVRDirectory.h"
#include "QBDiskPVRDirectoryManager.h"
#include "QBDiskPVRRecording.h"
#include <QBPVRDirectory.h>
#include <QBPVRProviderRequest.h>
#include <QBPVRProviderRequestInternal.h>
#include <QBPVRProviderVTable.h>
#include <QBPVRRecording.h>
#include <QBPVRDirectory.h>
#include <QBPVRTypes.h>
#include <QBPVRUtils.h>
#include <SvPlayerKit/SvContent.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <fibers/c/fibers.h>
#include <QBRecordFS/root.h>
#include <QBRecordFS/file.h>
#include <QBSchedDesc.h>
#include <QBDataModel3/QBSortedList2.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentCategoryInternal.h>
#include <QBContentManager/QBContentFilter.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPGDataLayer/SvEPGDataPlugin.h>
#include <SvEPGDataLayer/SvEPGDataRequest.h>
#include <SvEPGDataLayer/SvEPGRequestListener.h>
#include <QBAppKit/QBTimeDateMonitor.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBAsyncService.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvWeakList.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLocalTime.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvTime.h>

#include <TranslationMerger.h>

static const size_t MAX_MANUALS = 100;
static const size_t MIN_MB_FOR_RECORD = 50;
static const size_t MAX_RECORDINGS = 1000;

struct QBDiskPVRProvider_ {
    struct QBPVRProvider_ super_;

    SvImmutableArray dependencies;
    AppGlobals appGlobals;

    QBRecordFSRoot* storage;

    QBDiskPVRProviderParams params;
    QBContentTree tree;

    SvHashTable idToRecording;
    SvHashTable schedDescToRecording;
    QBDiskPVRDirectoryManager directoryManager;

    union {
        struct {
            QBContentCategory myRecordings;
            QBContentCategory schedule;
        } connectCategories;
        struct {
            QBContentCategory scheduled;
            QBContentCategory ongoing;
            QBContentCategory completed;
            QBContentCategory failed;
        } cubitvCategories;
    };

    SvWeakList listeners;
};

SvLocal SvArray
QBDiskPVRProviderGetRecordings(QBPVRProvider self_,
                               QBPVRProviderGetRecordingsParams params);

SvLocal QBPVRQuota
QBDiskPVRProviderGetCurrentQuota(QBPVRProvider self_);

SvLocal void
QBDiskPVRProviderNotifyRecordingAdded(QBDiskPVRProvider self,
                                      QBDiskPVRRecording rec)
{
    SvArray listenersArray = SvWeakListCreateElementsList(self->listeners, NULL);
    SvIterator it = SvArrayGetIterator(listenersArray);
    SvObject listener = NULL;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBPVRProviderListener, listener, recordingAdded,
                          (QBPVRRecording) rec);
    }
    SVRELEASE(listenersArray);
}

SvLocal void
QBDiskPVRProviderNotifyRecordingChanged(QBDiskPVRProvider self,
                                        QBDiskPVRRecording rec,
                                        QBDiskPVRRecording oldRec)
{
    SvArray listenersArray = SvWeakListCreateElementsList(self->listeners, NULL);
    SvIterator it = SvArrayGetIterator(listenersArray);
    SvObject listener = NULL;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBPVRProviderListener, listener, recordingChanged,
                          (QBPVRRecording) rec, (QBPVRRecording) oldRec);
    }
    SVRELEASE(listenersArray);
}

SvLocal void
QBDiskPVRProviderNotifyRecordingRemoved(QBDiskPVRProvider self,
                                        QBDiskPVRRecording rec)
{
    SvArray listenersArray = SvWeakListCreateElementsList(self->listeners, NULL);
    SvIterator it = SvArrayGetIterator(listenersArray);
    SvObject listener = NULL;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBPVRProviderListener, listener, recordingRemoved,
                          (QBPVRRecording) rec);
    }
    SVRELEASE(listenersArray);
}

SvLocal void
QBDiskPVRProviderNotifyRecordingRestricted(QBDiskPVRProvider self,
                                           QBDiskPVRRecording rec)
{
    SvArray listenersArray = SvWeakListCreateElementsList(self->listeners, NULL);
    SvIterator it = SvArrayGetIterator(listenersArray);
    SvObject listener = NULL;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBPVRProviderListener, listener, recordingRestricted,
                          (QBPVRRecording) rec);
    }
    SVRELEASE(listenersArray);
}

SvLocal void
QBDiskPVRProviderNotifyQuotaChanged(QBDiskPVRProvider self)
{
    SvArray listenersArray = SvWeakListCreateElementsList(self->listeners, NULL);
    SvIterator it = SvArrayGetIterator(listenersArray);
    SvObject listener = NULL;

    QBPVRQuota quota = QBDiskPVRProviderGetCurrentQuota((QBPVRProvider) self);
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBPVRProviderListener, listener, quotaChanged,
                          quota);
    }
    SVRELEASE(listenersArray);
}

SvHidden void
QBDiskPVRProviderNotifyDirectoryAdded(QBDiskPVRProvider self, QBPVRDirectory dir)
{
    if (!dir)
        return;
    SvArray listenersArray = SvWeakListCreateElementsList(self->listeners, NULL);
    SvIterator it = SvArrayGetIterator(listenersArray);
    SvObject listener = NULL;

    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBPVRProviderListener, listener, directoryAdded, dir);
    }
    SVRELEASE(listenersArray);
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType type,
                               QBPVRProviderDiskRequestStatus status,
                               SvObject params)
{
    QBPVRProviderRequest request = (QBPVRProviderRequest) SvTypeAllocateInstance(QBPVRProviderRequest_getType(), NULL);

    QBPVRProviderRequestInit(request, QBPVRProviderType_disk);
    request->type = type;
    request->disk.status = status;
    request->params = SVTESTRETAIN(params);

    return SVAUTORELEASE(request);
}

SvLocal void
QBDiskPVRProviderAddRecording(QBDiskPVRProvider self,
                              QBDiskPVRRecording rec)
{
    int addToDirectoryRet = -1;
    if (QBDiskPVRRecordingGetGroupId(rec))
        addToDirectoryRet = QBDiskPVRDirectoryManagerAddRecording(self->directoryManager, rec);

    if (self->params.super_.layout == QBPVRProviderLayout_CubiConnect) {
        QBSortedList2 list = NULL;
        if (((QBPVRRecording) rec)->state != QBPVRRecordingState_removed) {
            list = (QBSortedList2) QBContentCategoryGetItemsSource(self->connectCategories.schedule);
            QBSortedList2Insert(list, (SvObject) rec);
        }

        if (((QBPVRRecording) rec)->playable && addToDirectoryRet < 0) {
            list = (QBSortedList2) QBContentCategoryGetItemsSource(self->connectCategories.myRecordings);
            QBSortedList2Insert(list, (SvObject) rec);
        }
    } else if (addToDirectoryRet < 0) {
        QBSortedList2 list = NULL;
        if (((QBPVRRecording) rec)->playable &&
            ((QBPVRRecording) rec)->state != QBPVRRecordingState_active) {
            list = (QBSortedList2) QBContentCategoryGetItemsSource(self->cubitvCategories.completed);
            QBSortedList2Insert(list, (SvObject) rec);
        } else {
            switch (((QBPVRRecording) rec)->state) {
                case QBPVRRecordingState_scheduled:
                    list = (QBSortedList2) QBContentCategoryGetItemsSource(self->cubitvCategories.scheduled);
                    QBSortedList2Insert(list, (SvObject) rec);
                    break;
                case QBPVRRecordingState_active:
                    list = (QBSortedList2) QBContentCategoryGetItemsSource(self->cubitvCategories.ongoing);
                    QBSortedList2Insert(list, (SvObject) rec);
                    break;
                case QBPVRRecordingState_interrupted:
                case QBPVRRecordingState_stoppedManually:
                case QBPVRRecordingState_stoppedNoSpace:
                case QBPVRRecordingState_stoppedNoSource:
                case QBPVRRecordingState_failed:
                case QBPVRRecordingState_missed:
                    list = (QBSortedList2) QBContentCategoryGetItemsSource(self->cubitvCategories.failed);
                    QBSortedList2Insert(list, (SvObject) rec);
                    break;
                default:
                    break;
            }
        }
    }
}

SvLocal void
QBDiskPVRProviderScheduleAdded(void *self_, QBSchedDesc *desc)
{
    QBDiskPVRProvider self = self_;
    if (SvHashTableFind(self->schedDescToRecording, (SvObject) desc)) {
        return;
    }

    QBDiskPVRRecording rec = QBDiskPVRRecordingCreate(desc, NULL);
    SvHashTableInsert(self->idToRecording, (SvObject) ((QBPVRRecording) rec)->id, (SvObject) rec);
    SvHashTableInsert(self->schedDescToRecording, (SvObject) desc, (SvObject) rec);

    QBDiskPVRProviderAddRecording(self, rec);
    QBDiskPVRProviderNotifyRecordingAdded(self, rec);
    SVRELEASE(rec);
    QBContentTreePropagateStaticItemsChange(self->tree, NULL);
}

SvLocal void
QBDiskPVRProviderRemoveRecording(QBDiskPVRProvider self,
                                 QBDiskPVRRecording rec)
{
    int removeFromDirectoryRet = -1;
    if (QBDiskPVRRecordingGetGroupId(rec))
        removeFromDirectoryRet = QBDiskPVRDirectoryManagerRemoveRecording(self->directoryManager, rec);

    if (self->params.super_.layout == QBPVRProviderLayout_CubiConnect) {
        QBSortedList2 list = (QBSortedList2) QBContentCategoryGetItemsSource(self->connectCategories.schedule);
        QBSortedList2Remove(list, (SvObject) rec);

        if (removeFromDirectoryRet < 0) {
            list = (QBSortedList2) QBContentCategoryGetItemsSource(self->connectCategories.myRecordings);
            QBSortedList2Remove(list, (SvObject) rec);
        }
    } else if (removeFromDirectoryRet < 0) {
        QBSortedList2 list = NULL;
        if (((QBPVRRecording) rec)->playable &&
            ((QBPVRRecording) rec)->state != QBPVRRecordingState_active) {
            list = (QBSortedList2) QBContentCategoryGetItemsSource(self->cubitvCategories.completed);
            QBSortedList2Remove(list, (SvObject) rec);
        } else {
            switch (((QBPVRRecording) rec)->state) {
                case QBPVRRecordingState_scheduled:
                    list = (QBSortedList2) QBContentCategoryGetItemsSource(self->cubitvCategories.scheduled);
                    QBSortedList2Remove(list, (SvObject) rec);
                    break;
                case QBPVRRecordingState_active:
                    list = (QBSortedList2) QBContentCategoryGetItemsSource(self->cubitvCategories.ongoing);
                    QBSortedList2Remove(list, (SvObject) rec);
                    break;
                case QBPVRRecordingState_interrupted:
                case QBPVRRecordingState_stoppedManually:
                case QBPVRRecordingState_stoppedNoSpace:
                case QBPVRRecordingState_stoppedNoSource:
                case QBPVRRecordingState_failed:
                case QBPVRRecordingState_missed:
                    list = (QBSortedList2) QBContentCategoryGetItemsSource(self->cubitvCategories.failed);
                    QBSortedList2Remove(list, (SvObject) rec);
                    break;
                default:
                    break;
            }
        }
    }
}

SvLocal void
QBDiskPVRProviderScheduleRemoved(void *self_, QBSchedDesc *desc)
{
    QBDiskPVRProvider self = self_;
    QBDiskPVRRecording rec = (QBDiskPVRRecording) SvHashTableFind(self->schedDescToRecording, (SvObject) desc);
    if (!rec) {
        SvLogError("%s(): schedDesc was removed but we don't have any recording for it.", __func__);
        return;
    }

    QBDiskPVRProviderRemoveRecording(self, rec);
    QBDiskPVRProviderNotifyRecordingRemoved(self, rec);
    SvHashTableRemove(self->idToRecording, (SvObject) ((QBPVRRecording) rec)->id);
    SvHashTableRemove(self->schedDescToRecording, (SvObject) desc);
    QBContentTreePropagateStaticItemsChange(self->tree, NULL);
}

SvLocal void
QBDiskPVRProviderScheduleUpdated(void *self_, QBSchedDesc *desc,
                                 QBSchedState oldState, SvLocalTime oldStart, SvLocalTime oldEnd)
{
    QBDiskPVRProvider self = self_;

    if (desc->state == QBSchedState_removed) {
        QBDiskPVRProviderScheduleRemoved(self_, desc);
        return;
    }

    QBDiskPVRRecording rec = SVTESTRETAIN(SvHashTableFind(self->schedDescToRecording, (SvObject) desc));
    if (!rec) {
        SvLogError("%s(): didn't find recording for given schedule description", __func__);
        return;
    }

    bool recordingChangesPosition = QBDiskPVRRecordingWillRecordingChangePosition(rec);

    if (recordingChangesPosition) {
        QBDiskPVRProviderRemoveRecording(self, rec);
    }

    QBDiskPVRRecording old = (QBDiskPVRRecording) SvObjectCopy((SvObject) rec, NULL);
    QBDiskPVRRecordingUpdate(rec);

    if (recordingChangesPosition) {
        QBDiskPVRProviderAddRecording(self, rec);
    }

    QBDiskPVRProviderNotifyRecordingChanged(self, rec, old);
    SVRELEASE(old);
    SVRELEASE(rec);
    QBContentTreePropagateStaticItemsChange(self->tree, NULL);
}

SvLocal void
QBDiskPVRProviderScheduleRestricted(void *self_, QBSchedDesc *desc)
{
    QBDiskPVRProvider self = self_;
    QBDiskPVRRecording rec = (QBDiskPVRRecording) SvHashTableFind(self->schedDescToRecording, (SvObject) desc);
    if (!rec) {
        SvLogError("%s(): schedDesc was restricted but we don't have any recording for it.", __func__);
        return;
    }

    QBDiskPVRProviderNotifyRecordingRestricted(self, rec);
}

static const struct QBSchedManagerInfoCallbacks_s schedManagerInfoMethods = {
    .added   = QBDiskPVRProviderScheduleAdded,
    .updated = QBDiskPVRProviderScheduleUpdated,
    .removed = QBDiskPVRProviderScheduleRemoved,
    .restriction = QBDiskPVRProviderScheduleRestricted,
};

SvLocal void
QBDiskPVRProviderDestroy(void *self_)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;

    QBSchedManagerDeinit();
    if (self->params.super_.layout == QBPVRProviderLayout_CubiConnect) {
        SVTESTRELEASE(self->connectCategories.myRecordings);
        SVTESTRELEASE(self->connectCategories.schedule);
    } else {
        SVTESTRELEASE(self->cubitvCategories.scheduled);
        SVTESTRELEASE(self->cubitvCategories.ongoing);
        SVTESTRELEASE(self->cubitvCategories.completed);
        SVTESTRELEASE(self->cubitvCategories.failed);
    }

    SVTESTRELEASE(self->idToRecording);
    SVTESTRELEASE(self->schedDescToRecording);
    SVTESTRELEASE(self->directoryManager);

    SVTESTRELEASE(self->tree);
    SVTESTRELEASE(self->listeners);
    SVTESTRELEASE(self->storage);

    SVTESTRELEASE(self->dependencies);
}

SvLocal void
QBDiskPVRProviderRootFSDataModified(void *self_)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    QBDiskPVRProviderNotifyQuotaChanged(self);
}

SvLocal void
QBDiskPVRProviderRootFSAdded(void *self_)
{
}

SvLocal void
QBDiskPVRProviderRootFSRemoved(void *self_)
{
}

static QBRecordFSCallbacks recordFSCallbacks = {
    .rootFSDataModified = QBDiskPVRProviderRootFSDataModified,
    .rootFSAdded = QBDiskPVRProviderRootFSAdded,
    .rootFSRemoved = QBDiskPVRProviderRootFSRemoved,
};

SvLocal void
QBDiskPVRProviderContentProviderStart(QBContentProvider self_, SvScheduler scheduler)
{
    // this method should not be called, start is handled by async service start method
}

SvLocal void
QBDiskPVRProviderContentProviderStop(QBContentProvider self_)
{
    // this method should not be called, stop is handled by async service stop method
}

SvLocal void
QBDiskPVRProviderContentProviderRefresh(QBContentProvider self_, QBContentCategory category, size_t idx, bool force, bool updateSingleObject)
{
}

SvLocal void
QBDiskPVRProviderContentProviderClear(QBContentProvider self_)
{
}

SvLocal void
QBDiskPVRProviderContentProviderSetup(QBContentProvider self_)
{
}

SvLocal void
QBDiskPVRProviderContentProviderAddListener(QBContentProvider self_, SvObject listener)
{
}

SvLocal void
QBDiskPVRProviderContentProviderRemoveListener(QBContentProvider self_, SvObject listener)
{
}

SvLocal SvObject
QBDiskPVRProviderContentProviderGetManager(QBContentProvider self_)
{
    return NULL;
}

SvLocal void
QBDiskPVRProviderContentProviderSetLanguage(QBContentProvider self_, SvString lang)
{
    SvLogError("%s(): not implemented", __func__);
}

SvLocal void
QBDiskPVRProviderContentProviderSetFilter(QBContentProvider self, QBContentFilter filter)
{
    SvLogError("%s(): not implemented", __func__);
}

SvLocal QBContentTree
QBDiskPVRProviderGetTree(QBPVRProvider self_)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    return self->tree;
}

SvLocal QBPVRRecording
QBDiskPVRProviderGetRecordingById(QBPVRProvider self_, SvString recordingId)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    return (QBPVRRecording) SvHashTableFind(self->idToRecording, (SvObject) recordingId);
}

SvLocal int
QBDiskPVRProviderGetRecordingCurrentDuration(QBPVRProvider self_,
                                             QBPVRRecording rec_)
{
    QBDiskPVRRecording rec = (QBDiskPVRRecording) rec_;
    return QBDiskPVRRecordingGetDuration(rec);
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderUpdateRecording(QBPVRProvider self_,
                                 QBPVRRecording recording,
                                 QBPVRRecordingUpdateParams params)
{
    if (recording->type != QBPVRRecordingType_manual) {
        SvLogError("%s(): cannot update non time recordings", __func__);
        return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_updateRecording,
                                              QBPVRProviderDiskRequestStatus_failed,
                                              (SvObject) params);
    }

    if (recording->state != QBPVRRecordingState_scheduled) {
        SvLogError("%s(): cannot update not scheduled recordings", __func__);
        return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_updateRecording,
                                              QBPVRProviderDiskRequestStatus_failed,
                                              (SvObject) params);
    }

    if (params->updateRecovery) {
        SvLogError("%s(): cannot update recovery mode of recordings", __func__);
        return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_updateRecording,
                                              QBPVRProviderDiskRequestStatus_failed,
                                              (SvObject) params);
    }

    if (params->updateTime) {
        QBSchedParams schedParams;
        QBSchedParamsCopy(&schedParams, &(QBDiskPVRRecordingGetSchedDesc((QBDiskPVRRecording) recording)->params));
        schedParams.startTime = SvLocalTimeFromUTC(SvTimeGetSeconds(params->startTime));
        schedParams.stopTime = SvLocalTimeFromUTC(SvTimeGetSeconds(params->endTime));

        QBDiskPVRRecordingChangeStartEndTime((QBDiskPVRRecording) recording,
                                             params->startTime,
                                             params->endTime);
        return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_updateRecording,
                                              QBPVRProviderDiskRequestStatus_success,
                                              (SvObject) params);
    }

    SvLogError("%s(): nothing to update.", __func__);
    return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_updateRecording,
                                          QBPVRProviderDiskRequestStatus_success,
                                          (SvObject) params);
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderMoveRecording(QBPVRProvider self_,
                               QBPVRRecording recording,
                               QBPVRDirectory directory)
{
    QBDiskPVRRecordingSetGroupId((QBDiskPVRRecording) recording,
                                 directory ? QBContentCategoryGetId((QBContentCategory) directory) : NULL);
    return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_moveRecording,
                                          QBPVRProviderDiskRequestStatus_success,
                                          NULL);
}

QBDiskPVRProviderChannelCostClass
QBDiskPVRProviderGetPVRCostClassFromChannel(SvTVChannel ch)
{
    if (SvTVChannelGetAttribute(ch, SVSTRING("Radio")))
        return QBDiskPVRProviderChannelCostClass_radio;
    if (SvTVChannelGetAttribute(ch, SVSTRING("HD")))
        return QBDiskPVRProviderChannelCostClass_HDTV;
    return QBDiskPVRProviderChannelCostClass_SDTV;
}

SvLocal int
QBDiskPVRProviderCheckRecordingStorageConflicts(QBDiskPVRProvider self,
                                                QBPVRProviderConflictReason *conflictReason)
{
    if (!self->storage) {
        SvLogNotice("%s(): no storage attached", __func__);
        return 0;
    }

    QBRecordFSRootInfo info;
    memset(&info, 0, sizeof(info));
    QBRecordFSRootGetInfo(self->storage, &info);
    if (info.fileCnt >= self->params.limits.maxRecordingsCnt) {
        if (conflictReason)
            *conflictReason = QBPVRProviderConflictReason_tooManyRecordings;
        return -1;
    }
    if (info.bytesMax - info.bytesUsed < (int64_t) MIN_MB_FOR_RECORD * 1024 * 1024) {
        if (conflictReason)
            *conflictReason = QBPVRProviderConflictReason_lackOfSpace;
        return -1;
    }

    return 0;
}

SvLocal void
QBDiskPVRProviderPrepareSchedParamsFromRecordingSchedParams(QBSchedParams *schedParams,
                                                            QBPVRRecordingSchedParams params)
{
    QBSchedParamsInit(schedParams);
    schedParams->type = QBSchedType_record;

    schedParams->channelId = SVRETAIN(SvValueGetString(SvTVChannelGetID(params->channel)));
    schedParams->costType = (int) QBDiskPVRProviderGetPVRCostClassFromChannel(params->channel);
    schedParams->startMargin = params->startMargin;
    schedParams->stopMargin = params->endMargin;
    if (params->type == QBPVRRecordingType_manual ||
        params->type == QBPVRRecordingType_OTR) {
        schedParams->startTime = SvLocalTimeFromUTC(SvTimeGetSeconds(params->startTime));
        schedParams->stopTime = SvLocalTimeFromUTC(SvTimeGetSeconds(params->stopTime));
        schedParams->duration = params->duration;
        if (params->type == QBPVRRecordingType_OTR)
            schedParams->priority = QBDiskPVRPriority_OTR;
        else
            schedParams->priority = QBDiskPVRPriority_manual;
    } else {
        schedParams->event = SVRETAIN(params->event);
        schedParams->priority = QBDiskPVRPriority_event;
    }
}

SvLocal int
QBDiskPVRProviderCheckRecordingConcurrencyConflicts(QBDiskPVRProvider self,
                                                    QBPVRRecordingSchedParams params,
                                                    SvArray treatAsRemoved,
                                                    SvArray *conflicts,
                                                    int *timeWithoutConflicts,
                                                    QBPVRProviderConflictReason *conflictReason)
{
    QBSchedParams schedParams;
    QBDiskPVRProviderPrepareSchedParamsFromRecordingSchedParams(&schedParams, params);

    SvArray schedTreatAsRemoved = NULL;
    if (treatAsRemoved) {
        schedTreatAsRemoved = SvArrayCreate(NULL);

        SvIterator it = SvArrayGetIterator(treatAsRemoved);
        QBDiskPVRRecording rec = NULL;
        while ((rec = (QBDiskPVRRecording) SvIteratorGetNext(&it))) {
            const QBSchedDesc *schedDesc = QBDiskPVRRecordingGetSchedDesc(rec);
            SvArrayAddObject(schedTreatAsRemoved, (SvObject) schedDesc);
        }
    }
    SvArray schedConflictsOut = NULL;
    QBSchedManagerConflictReason schedConflictReasonOut;
    int ret = QBSchedManagerTestConflicts(&schedParams,
                                          schedTreatAsRemoved,
                                          &schedConflictsOut,
                                          &schedConflictReasonOut);

    if (ret != 0) {
        if (conflictReason) {
            switch (schedConflictReasonOut) {
                case QBSchedManagerConflictReason_driveTooSlow:
                    *conflictReason = QBPVRProviderConflictReason_driveTooSlow;
                    break;
                case QBSchedManagerConflictReason_tooManyRecordingSessions:
                    *conflictReason = QBPVRProviderConflictReason_tooManySimultaneousRecordings;
                    break;
                default:
                    *conflictReason = QBPVRProviderConflictReason_conflicts;
                    break;
            }
        }

        if (timeWithoutConflicts)
            *timeWithoutConflicts = QBSchedManagerTestDuration(&schedParams, schedTreatAsRemoved);

        if (conflicts && schedConflictsOut) {
            *conflicts = SvArrayCreate(NULL);
            SvIterator it = SvArrayGetIterator(schedConflictsOut);
            const QBSchedDesc *schedDesc = NULL;
            while ((schedDesc = (QBSchedDesc *) SvIteratorGetNext(&it))) {
                QBDiskPVRRecording rec = (QBDiskPVRRecording) SvHashTableFind(self->schedDescToRecording, (SvObject) schedDesc);
                if (!rec) {
                    SvLogError("%s(): didn't find recording for given schedule description", __func__);
                    continue;
                }
                SvArrayAddObject(*conflicts, (SvObject) rec);
            }
        }
    }

    QBSchedParamsDestroy(&schedParams);
    SVTESTRELEASE(schedTreatAsRemoved);
    SVTESTRELEASE(schedConflictsOut);
    return ret;
}

SvLocal int
QBDiskPVRProviderCheckRecordingConflicts(QBPVRProvider self_,
                                         QBPVRRecordingSchedParams params,
                                         SvArray treatAsRemoved,
                                         SvArray *conflicts,
                                         int *timeWithoutConflicts,
                                         QBPVRProviderConflictReason *conflictReason)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;

    int ret = QBDiskPVRProviderCheckRecordingStorageConflicts(self, conflictReason);
    if (ret < 0)
        return ret;

    if (QBSchedManagerGetScheduledCount() > MAX_MANUALS) {
        if (conflictReason)
            *conflictReason = QBPVRProviderConflictReason_tooManySimultaneousRecordings;
        return -1;
    }

    if (params->type != QBPVRRecordingType_event &&
        params->type != QBPVRRecordingType_manual &&
        params->type != QBPVRRecordingType_OTR)
        return 0;

    return QBDiskPVRProviderCheckRecordingConcurrencyConflicts(self, params, treatAsRemoved,
                                                               conflicts, timeWithoutConflicts,
                                                               conflictReason);
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderScheduleRecording(QBPVRProvider self_, QBPVRRecordingSchedParams params)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    if (params->type == QBPVRRecordingType_event ||
        params->type == QBPVRRecordingType_manual ||
        params->type == QBPVRRecordingType_OTR) {
        if (!params->channel) {
            SvLogError("%s(): cannot create event or time recording without channel.", __func__);
            return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_scheduleRecording,
                                                  QBPVRProviderDiskRequestStatus_failed,
                                                  (SvObject) params);
        }

        if (params->type == QBPVRRecordingType_event && !params->event) {
            SvLogError("%s(): event missing for event recording type", __func__);
            return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_scheduleRecording,
                                                  QBPVRProviderDiskRequestStatus_failed,
                                                  (SvObject) params);
        }

        QBSchedParams schedParams;
        QBDiskPVRProviderPrepareSchedParamsFromRecordingSchedParams(&schedParams, params);

        QBSchedDesc *descOut;
        int ret = QBSchedManagerAdd(&schedParams, &descOut);
        QBSchedParamsDestroy(&schedParams);
        QBPVRProviderRequest request = QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_scheduleRecording,
                                                                      (ret == 0 ? QBPVRProviderDiskRequestStatus_success :
                                                                                  QBPVRProviderDiskRequestStatus_failed),
                                                                      (SvObject) params);
        if (ret == 0 && descOut) {
            QBDiskPVRRecording rec = QBDiskPVRRecordingCreate(descOut, NULL);
            SvHashTableInsert(self->idToRecording, (SvObject) ((QBPVRRecording) rec)->id, (SvObject) rec);
            SvHashTableInsert(self->schedDescToRecording, (SvObject) descOut, (SvObject) rec);
            QBDiskPVRProviderAddRecording(self, rec);
            QBDiskPVRProviderNotifyRecordingAdded(self, rec);

            request->recording = (QBPVRRecording) rec;
            QBContentTreePropagateStaticItemsChange(self->tree, NULL);
        }
        return request;
    } else if (params->type == QBPVRRecordingType_keyword ||
               params->type == QBPVRRecordingType_repeated) {
        QBDiskPVRDirectory dir =
                QBDiskPVRDirectoryManagerCreateDirectoryFromRecordingSchedParams(self->directoryManager,
                                                                                 params,
                                                                                 NULL);
        QBPVRProviderRequest request = QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_scheduleRecording,
                                                                      (dir ? QBPVRProviderDiskRequestStatus_success :
                                                                             QBPVRProviderDiskRequestStatus_failed),
                                                                      (SvObject) params);
        request->directory = (QBPVRDirectory) dir;
        return request;
    } else {
        SvLogError("%s(): not supported", __func__);
        return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_scheduleRecording,
                                              QBPVRProviderDiskRequestStatus_failed,
                                              (SvObject) params);
    }
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderStopRecording(QBPVRProvider self_, QBPVRRecording recording)
{
    if (recording->state != QBPVRRecordingState_active) {
        SvLogError("%s(): cannot stop non active recordings", __func__);
        return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_stopRecording,
                                              QBPVRProviderDiskRequestStatus_failed,
                                              NULL);
    }

    if (recording->directory) {
        QBDiskPVRDirectory dir = (QBDiskPVRDirectory) SvWeakReferenceTakeReferredObject(recording->directory);
        if (((QBPVRDirectory) dir)->type == QBPVRDirectoryType_keyword)
            QBDiskPVRKeywordDirectoryBlacklistRecording(dir, (QBDiskPVRRecording) recording);
        SVRELEASE(dir);
    }
    QBDiskPVRRecordingStop((QBDiskPVRRecording) recording);
    return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_stopRecording,
                                          QBPVRProviderDiskRequestStatus_success,
                                          NULL);
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderDeleteRecording(QBPVRProvider self_, QBPVRRecording recording)
{
    if (recording->lock) {
        SvLogError("%s(): cannot delete locked recording", __func__);
        return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_deleteRecording,
                                              QBPVRProviderDiskRequestStatus_failed,
                                              NULL);
    }

    if (recording->directory) {
        QBDiskPVRDirectory dir = (QBDiskPVRDirectory) SvWeakReferenceTakeReferredObject(recording->directory);
        if (((QBPVRDirectory) dir)->type == QBPVRDirectoryType_keyword)
            QBDiskPVRKeywordDirectoryBlacklistRecording(dir, (QBDiskPVRRecording) recording);
        SVRELEASE(dir);
    }
    QBDiskPVRRecordingDelete((QBDiskPVRRecording) recording);

    return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_deleteRecording,
                                          QBPVRProviderDiskRequestStatus_success,
                                          NULL);
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderDeleteAllInState(QBPVRProvider self_,
                                  QBPVRProviderDeleteAllInStateParams params)
{
    QBPVRProviderGetRecordingsParams recordingParams = {
        .includeRecordingsInDirectories = true,
    };
    memcpy(recordingParams.inStates, params.inStates, sizeof(params.inStates));

    SvArray recordings =  QBDiskPVRProviderGetRecordings(self_, recordingParams);
    if (!recordings) {
        return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_deleteAllRecordingsInState,
                                              QBPVRProviderDiskRequestStatus_failed,
                                              NULL);
    }

    SvIterator it = SvArrayGetIterator(recordings);
    QBPVRRecording rec = NULL;
    while ((rec = (QBPVRRecording) SvIteratorGetNext(&it))) {
        if (!rec->lock)
            QBDiskPVRProviderDeleteRecording(self_, rec);
    }
    SVRELEASE(recordings);
    return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_deleteAllRecordingsInState,
                                          QBPVRProviderDiskRequestStatus_success,
                                          NULL);
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderDeleteAllFromCategory(QBPVRProvider self_, QBContentCategory category)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    if (!category) {
        SvLogError("%s(): NULL category passed", __func__);
        return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_deleteAllRecordingsFromCategory,
                                              QBPVRProviderDiskRequestStatus_failed,
                                              NULL);
    }

    if (SvObjectIsInstanceOf((SvObject) category, QBPVRDirectory_getType())) {
        struct QBPVRDirectoryDeleteParams_ deleteDirParams = {
            .keepContents = false,
        };
        QBDiskPVRDirectoryManagerDeleteAllFromDirectory(self->directoryManager,
                                                        (QBDiskPVRDirectory) category,
                                                        &deleteDirParams);
    }

    QBSortedList2 list = (QBSortedList2) QBContentCategoryGetItemsSource(category);
    SvIterator iter = QBSortedList2Iterator(list);

    SvArray objTemp = SvArrayCreate(NULL);
    SvObject obj = NULL;
    while ((obj = SvIteratorGetNext(&iter))) {
        SvArrayAddObject(objTemp, (SvObject) obj);
    }

    static struct QBPVRDirectoryDeleteParams_ deleteDirParams = {
        .keepContents = false,
    };
    iter = SvArrayIterator(objTemp);
    while ((obj = SvIteratorGetNext(&iter))) {
        if (SvObjectIsInstanceOf(obj, QBPVRRecording_getType())) {
            if (!((QBPVRRecording) obj)->lock)
                QBDiskPVRProviderDeleteRecording(self_, (QBPVRRecording) obj);
        } else if (SvObjectIsInstanceOf(obj, QBPVRDirectory_getType())) {
            QBDiskPVRDirectoryManagerDeleteDirectory(self->directoryManager,
                                                     (QBDiskPVRDirectory) obj,
                                                     &deleteDirParams);
        }
    }
    SVRELEASE(objTemp);

    return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_deleteAllRecordingsFromCategory,
                                          QBPVRProviderDiskRequestStatus_success,
                                          NULL);
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderLockRecording(QBPVRProvider self_, QBPVRRecording recording, bool lock)
{
    QBDiskPVRRecordingSetLock((QBDiskPVRRecording) recording, lock);
    return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_lockRecording,
                                          QBPVRProviderDiskRequestStatus_success,
                                          NULL);
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderMarkRecordingViewed(QBPVRProvider self_, QBPVRRecording recording)
{
    QBDiskPVRRecordingSetAsAlreadyViewed((QBDiskPVRRecording) recording);
    return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_markRecordingViewed,
                                          QBPVRProviderDiskRequestStatus_success,
                                          NULL);
}

SvLocal void
QBDiskPVRProviderAddListener(QBPVRProvider self_, SvObject listener)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;

    if (!listener) {
        SvLogError("%s: NULL listener passed", __func__);
        return;
    }
    if (!SvObjectIsImplementationOf(listener, QBPVRProviderListener_getInterface())) {
        SvLogError("%s: interface QBPVRProviderListener is not implemented by object", __func__);
        return;
    }

    SvWeakListPushBack(self->listeners, listener, NULL);
}

SvLocal void
QBDiskPVRProviderRemoveListener(QBPVRProvider self_, SvObject listener)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;

    if (!listener) {
        SvLogError("%s: NULL listener passed", __func__);
        return;
    }

    SvWeakListRemoveObject(self->listeners, listener);
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderCreateDirectory(QBPVRProvider self_,
                                 QBPVRDirectoryCreateParams params)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;

    QBDiskPVRDirectory dir =
            QBDiskPVRDirectoryManagerCreateDirectoryFromParams(self->directoryManager,
                                                               params,
                                                               NULL);

    QBPVRProviderRequest request =
            QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_createDirectory,
                                           dir ? QBPVRProviderDiskRequestStatus_success :
                                                 QBPVRProviderDiskRequestStatus_failed,
                                           (SvObject) params);
    request->directory = (QBPVRDirectory) dir;
    return request;
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderDeleteDirectory(QBPVRProvider self_,
                                 QBPVRDirectory directory,
                                 QBPVRDirectoryDeleteParams params)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;

    QBDiskPVRDirectoryManagerDeleteDirectory(self->directoryManager, (QBDiskPVRDirectory) directory, params);
    return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_deleteDirectory,
                                          QBPVRProviderDiskRequestStatus_success,
                                          (SvObject) params);
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderUpdateDirectory(QBPVRProvider self_,
                                 QBPVRDirectory directory,
                                 QBPVRDirectoryUpdateParams params)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    QBDiskPVRDirectoryManagerUpdateDirectory(self->directoryManager,
                                             (QBDiskPVRDirectory) directory,
                                             params);
    return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_updateDirectory,
                                          QBPVRProviderDiskRequestStatus_success,
                                          (SvObject) params);
}

SvLocal SvArray
QBDiskPVRProviderCreateDirectoriesList(QBPVRProvider self_)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    return SVRETAIN(QBDiskPVRDirectoryManagerGetDirectoriesList(self->directoryManager));
}

SvLocal bool
QBDiskPVRProviderCheckDirectoryLimit(QBPVRProvider self_, QBPVRDirectoryType type)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    return QBDiskPVRDirectoryManagerIsDirectoryLimitExceeded(self->directoryManager, type);
}

SvLocal bool
QBDiskPVRProviderCheckScheduledLimit(QBPVRProvider self)
{
    return (QBSchedManagerGetScheduledCount() > MAX_MANUALS);
}

SvLocal QBPVRProviderRequest
QBDiskPVRProviderStopDirectory(QBPVRProvider self_, QBPVRDirectory directory)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    if (directory->type == QBPVRDirectoryType_normal)
        return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_stopSchedule,
                                              QBPVRProviderDiskRequestStatus_failed,
                                              NULL);


    QBDiskPVRDirectoryManagerStopDirectory(self->directoryManager,
                                           (QBDiskPVRDirectory) directory);
    return QBDiskPVRProviderCreateRequest(QBPVRProviderRequestType_stopSchedule,
                                          QBPVRProviderDiskRequestStatus_success,
                                          NULL);
}

static const struct QBPVRProviderCapabilities_ capabilities = {
        .type = QBPVRProviderType_disk,
        .ongoingRecs = true,
        .recordingsCustomizable = true,
        .premiereFeature = false,
        .setMarginsFeature = false,
        .recoveryFeature = false,
        .canStopOngoing = true,
        .manual = true,
        .event = true,
        .series = false,
        .keyword = true,
        .repeated = true,
        .directories = true,
        .directoryQuota = true,
        .locks = true,
        .viewed = true,
        .recoveryModeOfRecording = false,
        .quotaUnits = QBPVRQuotaUnits_bytes,
};

SvLocal const QBPVRProviderCapabilities *
QBDiskPVRProviderGetCapabilities(QBPVRProvider self_)
{
    return &capabilities;
}

SvLocal QBPVRRecording
QBDiskPVRProviderGetCurrentRecordingForChannel(QBPVRProvider self_, SvTVChannel channel)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    SvArray recordings = NULL;
    QBSchedManagerFindActive(SvValueGetString(SvTVChannelGetID(channel)), &recordings);
    if (recordings) {
        QBPVRRecording ret = NULL;
        SvIterator it = SvArrayGetIterator(recordings);
        SvObject desc = SvIteratorGetNext(&it);
        QBPVRRecording rec = (QBPVRRecording) SvHashTableFind(self->schedDescToRecording, (SvObject) desc);
        if (rec && rec->state == QBPVRRecordingState_active) {
            ret = rec;
        }
        SVRELEASE(recordings);
        return ret;
    }
    return NULL;
}

SvLocal void
QBDiskPVRProviderGetRecordingsWithParams(QBDiskPVRProvider self,
                                         SvArray recordings,
                                         SvArray recordingsOut,
                                         QBPVRProviderGetRecordingsParams params)
{
    SvIterator it = SvArrayGetIterator(recordings);
    SvObject desc = NULL;

    if (SvHashTableGetCount(self->schedDescToRecording) == 0 && SvArrayGetCount(recordings) > 0) {
        SvLogError("%s(): recordings not present or not processed yet", __func__);
        return;
    }

    while ((desc = SvIteratorGetNext(&it))) {
        QBDiskPVRRecording rec = (QBDiskPVRRecording) SvHashTableFind(self->schedDescToRecording, desc);
        if (!rec) {
            SvLogError("%s(): didn't find recording for given schedule description", __func__);
            continue;
        }

        bool include = false;
        if (params.playable && ((QBPVRRecording) rec)->playable)
            include = true;
        if (params.inStates[((QBPVRRecording) rec)->state]) {
            if (((QBPVRRecording) rec)->directory) {
                if (params.includeRecordingsInDirectories) {
                    include = true;
                }
            } else {
                include = true;
            }
        }
        if (include)
            SvArrayAddObject(recordingsOut, (SvObject) rec);
    }
}

SvLocal SvArray
QBDiskPVRProviderFindRecordingsInTimeRange(QBPVRProvider self_, SvString channelId,
                                           SvTime start, SvTime end,
                                           QBPVRProviderGetRecordingsParams params)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;

    SvArray recordingsOut = SvArrayCreate(NULL);
    SvArray recordings = NULL;
    QBSchedManagerFind(channelId,
                       SvLocalTimeFromUTC(SvTimeGetSeconds(start)),
                       SvLocalTimeFromUTC(SvTimeGetSeconds(end)), &recordings, true);
    if (!recordings)
        goto out;

    QBDiskPVRProviderGetRecordingsWithParams(self,
                                             recordings,
                                             recordingsOut,
                                             params);
    SVRELEASE(recordings);
out:
    return recordingsOut;
}

SvLocal SvArray
QBDiskPVRProviderGetRecordings(QBPVRProvider self_,
                               QBPVRProviderGetRecordingsParams params)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;

    SvArray recordingsOut = SvArrayCreate(NULL);
    SvArray recordings = QBSchedManagerGetAll();
    if (!recordings)
        goto out;

    QBDiskPVRProviderGetRecordingsWithParams(self,
                                             recordings,
                                             recordingsOut,
                                             params);
    SVRELEASE(recordings);
out:
    return recordingsOut;
}

SvLocal bool
QBDiskPVRProviderCanRecord(QBPVRProvider self_)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    return ((QBPVRProvider) self)->state == QBAsyncServiceState_running &&
           (self->storage != NULL);
}

SvLocal bool
QBDiskPVRProviderIsEventRecordable(QBPVRProvider self_, SvEPGEvent event)
{
    return QBDiskPVRProviderCanRecord(self_);
}

SvLocal bool
QBDiskPVRProviderCanScheduleEventRecording(QBPVRProvider self_, SvEPGEvent event)
{
    if (!QBDiskPVRProviderCanRecord(self_))
        return false;

    time_t now = SvTimeNow();
    if (event->endTime <= now)
        return false;

    SvArray recordings = NULL;
    QBSchedManagerFind(SvValueGetString(event->channelID),
                       SvLocalTimeFromUTC(event->startTime),
                       SvLocalTimeFromUTC(event->endTime), &recordings, false);
    SVTESTRELEASE(recordings);
    return recordings == NULL;
}

SvLocal SvString
QBDiskPVRProviderCreateRecordingFailureReasonString(QBPVRProvider self_, QBPVRRecording recording)
{
    SvLogError("%s(): not implemented", __func__);
    return NULL;
}

SvLocal void
QBDiskPVRProviderSetRecordingProperty(QBPVRProvider self_,
                                      QBPVRRecording recording,
                                      SvString key,
                                      SvObject value)
{
    QBDiskPVRRecordingSetRecordingProperty((QBDiskPVRRecording) recording,
                                           key,
                                           value);
}

SvLocal SvObject
QBDiskPVRProviderGetRecordingProperty(QBPVRProvider self_,
                                      QBPVRRecording recording,
                                      SvString key)
{
    return QBDiskPVRRecordingGetRecordingProperty((QBDiskPVRRecording) recording,
                                                  key);
}

SvLocal SvContent
QBDiskPVRProviderCreateContentForRecording(QBPVRProvider self_,
                                           QBPVRRecording recording)
{
    QBDiskPVRRecording rec = (QBDiskPVRRecording) recording;
    return QBDiskPVRRecordingCreateContent(rec);
}

SvLocal QBPVRQuota
QBDiskPVRProviderGetCurrentQuota(QBPVRProvider self_)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    QBRecordFSRootInfo info = {
        .bytesMax = -1,
        .bytesUsed = -1,
    };

    if (((QBPVRProvider) self)->state == QBAsyncServiceState_running &&
        self->storage)
        QBRecordFSRootGetInfo(self->storage, &info);

    QBPVRQuota quota = {
        .limit = info.bytesMax,
        .used = info.bytesUsed,
    };
    return quota;
}

SvLocal SvArray
QBDiskPVRProviderCreateActiveDirectoriesListForEvent(QBPVRProvider self_, SvEPGEvent event)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    SvArray ret = SvArrayCreate(NULL);

    SvArray dirs = QBDiskPVRDirectoryManagerGetDirectoriesList(self->directoryManager);
    SvIterator it = SvArrayGetIterator(dirs);
    QBDiskPVRDirectory dir;

    while ((dir = (QBDiskPVRDirectory) SvIteratorGetNext(&it))) {
        if (QBDiskPVRDirectoryMatchKeyword(dir, event) ||
            QBDiskPVRDirectoryMatchRepeated(dir, event))
            SvArrayAddObject(ret, (SvObject)dir);
    }

    return ret;
}


SvLocal SvImmutableArray
QBDiskPVRProviderGetDependencies(SvObject self_)
{
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;
    return self->dependencies;
}

SvLocal void
QBDiskPVRProviderStart(SvObject self_,
                       SvScheduler scheduler,
                       SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
        goto out;
    }
    if (!scheduler) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL scheduler passed");
        goto out;
    }
    if (((QBPVRProvider) self)->state == QBAsyncServiceState_running) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBDiskPVRProvider already started");
        goto out;
    }

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorAddListener(timeDateMonitor, (SvObject) self, NULL);
    if (self->appGlobals->epgPlugin.DVB.data)
        SvInvokeInterface(SvEPGDataPlugin, self->appGlobals->epgPlugin.DVB.data, addListener, (SvObject) self, NULL);
    if (self->appGlobals->epgPlugin.IP.data)
        SvInvokeInterface(SvEPGDataPlugin, self->appGlobals->epgPlugin.IP.data, addListener, (SvObject) self, NULL);

    QBSchedManagerRegisterInfo(self_, &schedManagerInfoMethods, SVSTRING("QBDiskPVRProvider"));
    QBSchedManagerStart();

    QBRecordFSSetCallbacks(&recordFSCallbacks, self);

    QBDiskPVRDirectoryManagerStart(self->directoryManager, NULL);

    long int interfaceID = SvInterfaceGetID(QBAsyncService_getInterface());
    const struct QBAsyncService_ *superObjectMethods =
        SvTypeGetVTable(QBPVRProvider_getType(), interfaceID);
    superObjectMethods->start(self_, scheduler, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "superType->start() failed");
        goto out;
    }
out:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBDiskPVRProviderStop(SvObject self_,
                      SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBDiskPVRProvider self = (QBDiskPVRProvider) self_;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
        goto out;
    }
    if (((QBPVRProvider) self)->state != QBAsyncServiceState_running) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBDiskPVRProvider not running");
        goto out;
    }

    QBDiskPVRDirectoryManagerStop(self->directoryManager, NULL);

    QBRecordFSUnsetCallbacks(&recordFSCallbacks);

    QBSchedManagerStop();
    QBSchedManagerUnregisterInfo(self_, &schedManagerInfoMethods);

    if (self->appGlobals->epgPlugin.DVB.data)
        SvInvokeInterface(SvEPGDataPlugin, self->appGlobals->epgPlugin.DVB.data, removeListener, (SvObject) self, NULL);
    if (self->appGlobals->epgPlugin.IP.data)
        SvInvokeInterface(SvEPGDataPlugin, self->appGlobals->epgPlugin.IP.data, removeListener, (SvObject) self, NULL);

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorRemoveListener(timeDateMonitor, (SvObject) self, NULL);

    long int interfaceID = SvInterfaceGetID(QBAsyncService_getInterface());
    const struct QBAsyncService_ *superObjectMethods =
        SvTypeGetVTable(QBPVRProvider_getType(), interfaceID);
    superObjectMethods->stop(self_, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "superType->stop() failed");
        goto out;
    }

out:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal
void QBDiskPVRProviderTimeSet(SvObject self_, bool firstTime, QBTimeDateUpdateSource source)
{
    QBSchedManagerTimeDateRulesUpdated();
}

SvLocal void
QBDiskPVRProviderDataRequestStateChanged(SvObject self_,
                                         SvEPGDataRequest req)
{
}

SvLocal void
QBDiskPVRProviderDataReceived(SvObject self_,
                              SvObject plugin,
                              SvEPGDataRequest req,
                              SvValue channelID,
                              SvArray events)
{
    size_t cnt = SvArrayCount(events);
    for (size_t i = 0; i < cnt; ++i) {
        SvEPGEvent event = (SvEPGEvent) SvArrayAt(events, i);
        QBSchedManagerEventUpdated(event);
    }
}

SvLocal SvType
QBDiskPVRProvider_getType(void)
{
    static SvType type = NULL;

    static const struct QBPVRProviderVTable_ virtualMethods = {
        .super_ = {
            .super_ = {
                .destroy = QBDiskPVRProviderDestroy,
            },
            .start = QBDiskPVRProviderContentProviderStart,
            .stop = QBDiskPVRProviderContentProviderStop,
            .refresh = QBDiskPVRProviderContentProviderRefresh,
            .clear = QBDiskPVRProviderContentProviderClear,
            .setup = QBDiskPVRProviderContentProviderSetup,
            .addListener = QBDiskPVRProviderContentProviderAddListener,
            .removeListener = QBDiskPVRProviderContentProviderRemoveListener,
            .getManager = QBDiskPVRProviderContentProviderGetManager,
            .setLanguage = QBDiskPVRProviderContentProviderSetLanguage,
            .setFilter = QBDiskPVRProviderContentProviderSetFilter
        },
        .getTree = QBDiskPVRProviderGetTree,
        .getRecordingById = QBDiskPVRProviderGetRecordingById,
        .getRecordingCurrentDuration = QBDiskPVRProviderGetRecordingCurrentDuration,
        .updateRecording = QBDiskPVRProviderUpdateRecording,
        .moveRecording = QBDiskPVRProviderMoveRecording,
        .checkRecordingConflicts = QBDiskPVRProviderCheckRecordingConflicts,
        .scheduleRecording = QBDiskPVRProviderScheduleRecording,
        .stopRecording = QBDiskPVRProviderStopRecording,
        .deleteRecording = QBDiskPVRProviderDeleteRecording,
        .deleteAllInState = QBDiskPVRProviderDeleteAllInState,
        .deleteAllFromCategory = QBDiskPVRProviderDeleteAllFromCategory,
        .lockRecording = QBDiskPVRProviderLockRecording,
        .markRecordingViewed = QBDiskPVRProviderMarkRecordingViewed,
        .addListener = QBDiskPVRProviderAddListener,
        .removeListener = QBDiskPVRProviderRemoveListener,
        .createDirectory = QBDiskPVRProviderCreateDirectory,
        .deleteDirectory = QBDiskPVRProviderDeleteDirectory,
        .updateDirectory = QBDiskPVRProviderUpdateDirectory,
        .listDirectories = QBDiskPVRProviderCreateDirectoriesList,
        .checkDirectoryLimit = QBDiskPVRProviderCheckDirectoryLimit,
        .checkScheduledLimit = QBDiskPVRProviderCheckScheduledLimit,
        .stopDirectory = QBDiskPVRProviderStopDirectory,
        .getCapabilities = QBDiskPVRProviderGetCapabilities,
        .getCurrentRecordingForChannel = QBDiskPVRProviderGetCurrentRecordingForChannel,
        .findRecordingsInTimeRange = QBDiskPVRProviderFindRecordingsInTimeRange,
        .getRecordings = QBDiskPVRProviderGetRecordings,
        .canRecord = QBDiskPVRProviderCanRecord,
        .isEventRecordable = QBDiskPVRProviderIsEventRecordable,
        .canScheduleEventRecording = QBDiskPVRProviderCanScheduleEventRecording,
        .createRecordingFailureReasonString = QBDiskPVRProviderCreateRecordingFailureReasonString,
        .setRecordingProperty = QBDiskPVRProviderSetRecordingProperty,
        .getRecordingProperty = QBDiskPVRProviderGetRecordingProperty,
        .createContentForRecording = QBDiskPVRProviderCreateContentForRecording,
        .getCurrentQuota = QBDiskPVRProviderGetCurrentQuota,
        .listActiveDirectoriesForEvent = QBDiskPVRProviderCreateActiveDirectoriesListForEvent,
    };

    static const struct QBAsyncService_ asyncServiceMethods = {
        .getDependencies = QBDiskPVRProviderGetDependencies,
        .start = QBDiskPVRProviderStart,
        .stop = QBDiskPVRProviderStop,
    };

    static const struct QBTimeDateMonitorListener_ timeDateListenerMethods = {
        .systemTimeSet = QBDiskPVRProviderTimeSet
    };

    static const struct SvEPGRequestListener_ requestListenerMethods = {
        .requestStateChanged = QBDiskPVRProviderDataRequestStateChanged,
        .dataReceived = QBDiskPVRProviderDataReceived,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDiskPVRProvider",
                            sizeof(struct QBDiskPVRProvider_),
                            QBPVRProvider_getType(),
                            &type,
                            QBPVRProvider_getType(), &virtualMethods,
                            QBAsyncService_getInterface(), &asyncServiceMethods,
                            QBTimeDateMonitorListener_getInterface(), &timeDateListenerMethods,
                            SvEPGRequestListener_getInterface(), &requestListenerMethods,
                            NULL);
    }

    return type;
}

SvLocal int
QBDiskPVRProviderCreateConnectLayoutCategories(QBDiskPVRProvider self,
                                               QBContentCategory localRoot,
                                               SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    SvString categoryName = SvStringCreate(gettext_noop("My Recordings"), NULL);
    self->connectCategories.myRecordings =
        QBPVRProviderCreateStaticCategory(SVSTRING(MY_RECORDINGS_CATEGORY_ID), categoryName,
                                          localRoot, (SvObject) self,
                                          self->params.super_.addSearchToStaticCategories,
                                          self->params.super_.searchProviderTest,
                                          &error);
    SVRELEASE(categoryName);
    if (!self->connectCategories.myRecordings) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBPVRProviderCreateStaticCategory() failed");
        goto err;
    }
    QBContentCategoryAddStaticItem(localRoot, (SvObject) self->connectCategories.myRecordings);

    if (!(self->directoryManager = QBDiskPVRDirectoryManagerCreate(self,
                                                                   self->connectCategories.myRecordings,
                                                                   self->params.super_.addSearchToDirectories,
                                                                   self->params.super_.searchProviderTest,
                                                                   self->params.limits,
                                                                   &error))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBDiskPVRDirectoryManagerCreate() failed");
        goto err;
    }

    categoryName = SvStringCreate(gettext_noop("Schedule"), NULL);
    self->connectCategories.schedule =
        QBPVRProviderCreateStaticCategory(SVSTRING(SCHEDULE_CATEGORY_ID), categoryName,
                                          localRoot, (SvObject) self,
                                          self->params.super_.addSearchToStaticCategories,
                                          self->params.super_.searchProviderTest,
                                          &error);
    SVRELEASE(categoryName);
    if (!self->connectCategories.schedule) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBPVRProviderCreateStaticCategory() failed");
        goto err;
    }
    QBContentCategoryAddStaticItem(localRoot, (SvObject) self->connectCategories.schedule);
    return 0;
err:
    SvErrorInfoPropagate(error, errorOut);
    return -1;
}

SvLocal int
QBDiskPVRProviderCreateCubiTVLayoutCategories(QBDiskPVRProvider self,
                                              QBContentCategory localRoot,
                                              SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    // dataSource for main category
    QBSortedList2 sortedList = NULL;
    if (!(sortedList = QBSortedList2CreateWithCompareFn(
            QBPVRProviderCompareRecordingsAndDirectories, NULL, &error))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBSortedList2CreateWithCompareFn() failed");
        goto err;
    }
    QBContentCategorySetItemsSource(localRoot, (SvObject) sortedList);
    SVRELEASE(sortedList);

    if (!(self->directoryManager = QBDiskPVRDirectoryManagerCreate(self,
                                                                   localRoot,
                                                                   self->params.super_.addSearchToDirectories,
                                                                   self->params.super_.searchProviderTest,
                                                                   self->params.limits,
                                                                   &error))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBDiskPVRDirectoryManagerCreate() failed");
        goto err;
    }

    SvString categoryName = SvStringCreate(gettext_noop("Scheduled Recordings"), NULL);
    self->cubitvCategories.scheduled =
        QBPVRProviderCreateStaticCategory(SVSTRING(SCHEDULED_CATEGORY_ID), categoryName,
                                          localRoot, (SvObject) self,
                                          self->params.super_.addSearchToStaticCategories,
                                          self->params.super_.searchProviderTest,
                                          &error);
    SVRELEASE(categoryName);
    if (!self->cubitvCategories.scheduled) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBPVRProviderCreateStaticCategory() failed");
        goto err;
    }
    QBContentCategoryAddStaticItem(localRoot, (SvObject) self->cubitvCategories.scheduled);

    categoryName = SvStringCreate(gettext_noop("Ongoing Recordings"), NULL);
    self->cubitvCategories.ongoing =
        QBPVRProviderCreateStaticCategory(SVSTRING(ONGOING_CATEGORY_ID), categoryName,
                                          localRoot, (SvObject) self,
                                          self->params.super_.addSearchToStaticCategories,
                                          self->params.super_.searchProviderTest,
                                          &error);
    SVRELEASE(categoryName);
    if (!self->cubitvCategories.ongoing) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBPVRProviderCreateStaticCategory() failed");
        goto err;
    }
    QBContentCategoryAddStaticItem(localRoot, (SvObject) self->cubitvCategories.ongoing);

    categoryName = SvStringCreate(gettext_noop("Completed Recordings"), NULL);
    self->cubitvCategories.completed =
        QBPVRProviderCreateStaticCategory(SVSTRING(COMPLETED_CATEGORY_ID), categoryName,
                                          localRoot, (SvObject) self,
                                          self->params.super_.addSearchToStaticCategories,
                                          self->params.super_.searchProviderTest,
                                          &error);
    SVRELEASE(categoryName);
    if (!self->cubitvCategories.completed) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBPVRProviderCreateStaticCategory() failed");
        goto err;
    }
    QBContentCategoryAddStaticItem(localRoot, (SvObject) self->cubitvCategories.completed);

    categoryName = SvStringCreate(gettext_noop("Failed Recordings"), NULL);
    self->cubitvCategories.failed =
        QBPVRProviderCreateStaticCategory(SVSTRING(FAILED_CATEGORY_ID), categoryName,
                                          localRoot, (SvObject) self,
                                          self->params.super_.addSearchToStaticCategories,
                                          self->params.super_.searchProviderTest,
                                          &error);
    SVRELEASE(categoryName);
    if (!self->cubitvCategories.failed) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBPVRProviderCreateStaticCategory() failed");
        goto err;
    }
    QBContentCategoryAddStaticItem(localRoot, (SvObject) self->cubitvCategories.failed);

    return 0;
err:
    SvErrorInfoPropagate(error, errorOut);
    return -1;
}

QBDiskPVRProvider
QBDiskPVRProviderCreate(AppGlobals appGlobals,
                        SvScheduler scheduler,
                        QBDiskPVRProviderParams params,
                        SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!appGlobals) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL appGlobals passed");
        goto err1;
    }
    if (!scheduler) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL scheduler passed");
        goto err1;
    }

    QBDiskPVRProvider self = (QBDiskPVRProvider)
            SvTypeAllocateInstance(QBDiskPVRProvider_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error,
                                           "can't allocate QBDiskPVRProvider");
        goto err1;
    }


    if (!(self->tree = QBContentTreeCreate(SVSTRING("DISK_PVR"), &error))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBContentTreeCreate() failed");
        goto err2;
    }

    if (!(self->idToRecording = SvHashTableCreate(157, &error))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "SvHashTableCreate() failed");
        goto err2;
    }

    if (!(self->schedDescToRecording = SvHashTableCreate(157, &error))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "SvHashTableCreate() failed");
        goto err2;
    }

    self->dependencies = SvImmutableArrayCreateWithTypedValues("ss", &error, "QBJSONSerializer", "QBEPGWatcher");
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvImmutableArrayCreateWithTypedValues() failed");
        goto err2;
    }

    QBContentCategory localRoot = QBContentTreeGetRootCategory(self->tree);

    self->params = params;
    if (self->params.limits.maxRecordingsCnt < 0) {
        self->params.limits.maxRecordingsCnt = MAX_RECORDINGS;
    }
    self->appGlobals = appGlobals;

    if (!(self->listeners = SvWeakListCreate(&error))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "SvWeakListCreate() failed");
        goto err2;
    }

    if (self->params.super_.layout == QBPVRProviderLayout_CubiConnect) {
        if (QBDiskPVRProviderCreateConnectLayoutCategories(self, localRoot, &error) < 0)
            goto err2;
    } else {
        if (QBDiskPVRProviderCreateCubiTVLayoutCategories(self, localRoot, &error) < 0)
            goto err2;
    }

    QBSchedManagerInit(scheduler);
    QBSchedManagerSetTunerLimit(params.tunerCnt);

    return self;
err2:
    SVRELEASE(self);
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

void
QBDiskPVRProviderSetStorage(QBDiskPVRProvider self, QBRecordFSRoot* root)
{
    SVTESTRELEASE(self->storage);
    self->storage = SVTESTRETAIN(root);

    QBSchedManagerSetStorage(root);

    QBDiskPVRProviderNotifyQuotaChanged(self);
}

void
QBDiskPVRProviderRegisterAppCallbacks(QBDiskPVRProvider self,
                                      void *target,
                                      const QBDiskPVRProviderAppCallbacks* callbacks)
{
    QBSchedManagerRegisterApp(target, callbacks);
}

void
QBDiskPVRProviderUnreserveTuner(QBDiskPVRProvider self,
                                struct QBTunerParams *tunerParams)
{
    QBSchedManagerUnReserveTuner(tunerParams);
}

void
QBDiskPVRProviderSetRecordingsLimit(QBDiskPVRProvider self, size_t totalCost)
{
    QBSchedManagerSetRecCostLimit(totalCost);
}

void
QBDiskPVRProviderSetRecordingSessionsLimit(QBDiskPVRProvider self, size_t cnt)
{
    QBSchedManagerSetRecSessionsLimit(cnt);
}

const struct QBTunerMuxId *
QBDiskPVRProviderGetRecordingsMuxId(QBDiskPVRProvider self, QBPVRRecording rec)
{
    return QBDiskPVRRecordingGetMuxId((QBDiskPVRRecording) rec);
}
