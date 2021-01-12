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

#include "pvrAgent.h"

#include <main.h>
#include <QBPVRRecording.h>
#include <Logic/PVRLogic.h>
#include <Menus/menuchoice.h>
#include <Windows/mainmenu.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <fibers/c/fibers.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGManager.h>

#define log_debug(fmt, ...)  do { if(0) SvLogNotice(COLBEG() "PVRAgent :: " fmt  COLEND_COL(blue), ##__VA_ARGS__); } while (0)

struct QBPVRAgent_s {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvEPGManager epgManager;

    SvHashTable revMap; /// PVRRecording -> QBActiveArray

    QBActiveArray completedRecordings;
    QBActiveArray onGoingRecordings;
    QBActiveArray scheduledRecordings;
    QBActiveArray failedRecordings;
    QBActiveArray removedRecordings;

    SvHashTable trackedEvents; //QBPVRProviderRequest -> SvEPGEvent

};

SvLocal size_t
find_object_in_array(QBActiveArray array,
                     const QBPVRRecording rec)
{
    return QBActiveArrayIndexOfObjectIdenticalTo(array, (SvGenericObject) rec, NULL);
}

SvLocal bool
has_correct_place_in_array(QBActiveArray array,
                           const QBPVRRecording rec)
{
    bool found = false;
    size_t i;
    size_t cnt = QBActiveArrayCount(array);

    for (i = 0; i < cnt; i++) {
        const QBPVRRecording rec2 = (const QBPVRRecording) QBActiveArrayObjectAtIndex(array, i);
        if (!found) {
            if (rec == rec2) {
                found = true;
                continue;
            }

            if (rec->state != QBPVRRecordingState_scheduled) {
                if (SvTimeCmp(rec->startTime, rec2->startTime) > 0) {
                    return false;
                }
            } else {
                if (SvTimeCmp(rec->startTime, rec2->startTime) < 0) {
                    return false;
                }
            }
        } else {
            if (rec->state != QBPVRRecordingState_scheduled) {
                if (SvTimeCmp(rec->startTime, rec2->startTime) < 0) {
                    return false;
                }
            } else {
                if (SvTimeCmp(rec->startTime, rec2->startTime) > 0) {
                    return false;
                }
            }
        }
    }

    return true;
}

SvLocal ssize_t
find_correct_place_in_array(QBActiveArray array,
                            const QBPVRRecording rec)
{
    ssize_t i;
    ssize_t cnt = QBActiveArrayCount(array);

    for (i = 0; i < cnt; i++) {
        const QBPVRRecording rec2 = (const QBPVRRecording) QBActiveArrayObjectAtIndex(array, i);

        if (rec->state != QBPVRRecordingState_scheduled) {
            if (SvTimeCmp(rec->startTime, rec2->startTime) >= 0) {
                return i;
            }
        } else {
            if (SvTimeCmp(rec->startTime, rec2->startTime) <= 0) {
                return i;
            }
        }
    }
    return -1;
}


SvLocal QBActiveArray
QBPVRAgentGetArrayFromState(QBPVRAgent self, QBPVRRecordingState state)
{
    if (state == QBPVRRecordingState_active)
        return self->onGoingRecordings;

    if (state == QBPVRRecordingState_completed)
        return self->completedRecordings;

    if (state == QBPVRRecordingState_scheduled)
        return self->scheduledRecordings;

    if (state == QBPVRRecordingState_failed)
        return self->failedRecordings;

    if (state == QBPVRRecordingState_removed)
        return self->removedRecordings;

    return NULL;
}

SvLocal void
QBPVRAgentRecordingStateChanged(QBPVRAgent self, QBPVRRecording rec, QBActiveArray oldArray, QBActiveArray newArray)
{
    log_debug("self->onGoingRecordings %p", self->onGoingRecordings);
    log_debug("self->completedRecordings %p", self->completedRecordings);
    log_debug("self->scheduledRecordings %p", self->scheduledRecordings);
    log_debug("self->failedRecordings %p", self->failedRecordings);
    log_debug("self->removedRecordings %p", self->removedRecordings);

    log_debug("oldArray %p , newArray %p", oldArray, newArray);

    size_t oldPos = -1;
    if (oldArray) {
        oldPos = find_object_in_array(oldArray, rec);
    }

    if (oldArray == newArray) {
        if (!oldArray) {
            return;
        }

        if (has_correct_place_in_array(oldArray, rec)) {
            log_debug("rec in place - just update");
            QBActiveArrayPropagateObjectsChange(oldArray, oldPos, 1, NULL);
            return;
        }
    }

    if (oldArray) {
        log_debug("remove from old dir");
        SvHashTableRemove(self->revMap, (SvGenericObject)rec);
        QBActiveArrayRemoveObjectAtIndex(oldArray, oldPos, NULL);
    }

    if (newArray) {
        log_debug("new folder");

        ssize_t newPos = find_correct_place_in_array(newArray, rec);
        if (newPos >= 0) {
            QBActiveArrayInsertObjectAtIndex(newArray, newPos, (SvGenericObject)rec, NULL);
        } else {
            QBActiveArrayAddObject(newArray, (SvGenericObject)rec, NULL);
        }

        SvHashTableInsert(self->revMap, (SvGenericObject)rec, (SvGenericObject)newArray);
    }
}

SvLocal void
QBPVRAgentRecordingRemoved(SvGenericObject self_, QBPVRRecording rec)
{
    QBPVRAgent self = (QBPVRAgent) self_;

    QBActiveArray oldArray = (QBActiveArray) SvHashTableFind(self->revMap, (SvGenericObject)rec);

    if (QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->recoveryFeature &&
        (QBActiveArrayIndexOfObjectIdenticalTo(self->removedRecordings, (SvGenericObject) rec, NULL) == -1) &&
        (rec->deleteReason == QBPVRRecordingDeleteReason_DeleteOldest || rec->deleteReason == QBPVRRecordingDeleteReason_Manual)) {
        QBPVRAgentRecordingStateChanged(self, rec, NULL, self->removedRecordings);
    } else {
        QBPVRAgentRecordingStateChanged(self, rec, oldArray, NULL);
    }
}

SvLocal void
QBPVRAgentRecordingRestricted(SvObject self_,
                              QBPVRRecording rec)
{
}

SvLocal void
QBPVRAgentRecordingChanged(SvGenericObject self_, QBPVRRecording rec, QBPVRRecording oldRecording)
{
    QBPVRAgent self = (QBPVRAgent) self_;

    QBActiveArray oldArray = (QBActiveArray) SvHashTableFind(self->revMap, (SvGenericObject)rec);
    QBActiveArray newArray = QBPVRAgentGetArrayFromState(self, rec->state);
    QBPVRAgentRecordingStateChanged(self, rec, oldArray, newArray);
}

SvLocal void
QBPVRAgentRecordingAdded(SvGenericObject self_, QBPVRRecording rec)
{
    QBPVRAgent self = (QBPVRAgent) self_;

    QBActiveArray newArray = QBPVRAgentGetArrayFromState(self, rec->state);
    QBPVRAgentRecordingStateChanged(self, rec, NULL, newArray);
}

SvLocal void
QBPVRAgentQuotaChanged(SvObject self, QBPVRQuota quota)
{
}

SvLocal void
QBPVRAgentDirectoryAdded(SvObject self_, QBPVRDirectory dir)
{
}

SvLocal void
QBPVRAgentRequestStateChanged(SvGenericObject self_, QBPVRProviderRequest request)
{
    QBPVRAgent self = (QBPVRAgent) self_;

    if (QBPVRProviderNetworkRequestGetException(request) == QBPVRProviderNetworkRequestException_eventNotFound) {
        SvTimeRange timeRange;
        SvEPGEvent trackedEvent = (SvEPGEvent) SvHashTableFind(self->trackedEvents, (SvGenericObject)request);
        SvTimeRangeInit(&timeRange, trackedEvent->startTime, trackedEvent->endTime);
        SvEPGDataWindow epgWindow = SvEPGManagerCreateDataWindow(self->epgManager, NULL);
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        SvGenericObject master = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
        SvEPGDataWindowSetChannelView(epgWindow, (SvGenericObject) master, NULL);
        SvEPGDataWindowSetChannelsRange(epgWindow, 0, 0, NULL);
        SvEPGDataWindowSetTimeRange(epgWindow, timeRange, NULL);
        SvEPGDataWindowInvalidateEvents(epgWindow, NULL);
        SVRELEASE(epgWindow);
    }

    SvHashTableRemove(self->trackedEvents, (SvGenericObject)request);
    QBPVRProviderRequestRemoveListener(request, (SvGenericObject) self);
}

SvLocal void
QBPVRAgentDestroy(void *self_)
{
    QBPVRAgent self = (QBPVRAgent) self_;

    SVRELEASE(self->completedRecordings);
    SVRELEASE(self->onGoingRecordings);
    SVRELEASE(self->scheduledRecordings);
    SVRELEASE(self->failedRecordings);
    SVTESTRELEASE(self->removedRecordings);

    SVRELEASE(self->revMap);
    SVRELEASE(self->trackedEvents);
    SVTESTRELEASE(self->epgManager);
}

SvLocal SvType
QBPVRAgent_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBPVRAgentDestroy
    };
    static SvType type = NULL;

    static const struct QBPVRProviderListener_ pvrProviderListenerMethods = {
        .recordingAdded = QBPVRAgentRecordingAdded,
        .recordingChanged = QBPVRAgentRecordingChanged,
        .recordingRemoved = QBPVRAgentRecordingRemoved,
        .recordingRestricted = QBPVRAgentRecordingRestricted,
        .quotaChanged = QBPVRAgentQuotaChanged,
        .directoryAdded = QBPVRAgentDirectoryAdded,
    };

    static const struct QBPVRProviderRequestListener_ PVRRequestMethods = {
        .stateChanged = QBPVRAgentRequestStateChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBPVRAgent",
                            sizeof(struct QBPVRAgent_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPVRProviderListener_getInterface(), &pvrProviderListenerMethods,
                            QBPVRProviderRequestListener_getInterface(), &PVRRequestMethods,
                            NULL);
    }
    return type;
}

QBPVRAgent
QBPVRAgentCreate(AppGlobals appGlobals)
{
    if (!appGlobals->pvrProvider) {
        SvLogError("QBPVRAgentCreate() : pvr provider not created!");
        return NULL;
    };

    QBPVRAgent self = (QBPVRAgent) SvTypeAllocateInstance(QBPVRAgent_getType(), NULL);

    self->appGlobals = appGlobals;

    self->completedRecordings = QBActiveArrayCreate(97, NULL);
    self->onGoingRecordings = QBActiveArrayCreate(11, NULL);
    self->scheduledRecordings = QBActiveArrayCreate(97, NULL);
    self->failedRecordings = QBActiveArrayCreate(97, NULL);
    if (QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->recoveryFeature)
        self->removedRecordings = QBActiveArrayCreate(97, NULL);

    self->revMap = SvHashTableCreate(97, NULL);
    self->trackedEvents = SvHashTableCreate(11, NULL);

    return self;
}


QBActiveArray
QBPVRAgentGetCompletedRecordingsList(QBPVRAgent self)
{
    return self->completedRecordings;
}

QBActiveArray
QBPVRAgentGetScheduledRecordingsList(QBPVRAgent self)
{
    return self->scheduledRecordings;
}

QBActiveArray
QBPVRAgentGetOnGoingRecordingsList(QBPVRAgent self)
{
    return self->onGoingRecordings;
}

QBActiveArray
QBPVRAgentGetFailedRecordingsList(QBPVRAgent self)
{
    return self->failedRecordings;
}

QBActiveArray
QBPVRAgentGetRemovedRecordingsList(QBPVRAgent self)
{
    return self->removedRecordings;
}

void
QBPVRAgentStart(QBPVRAgent self)
{
    if (!self->appGlobals->pvrProvider) {
        SvLogError("%s(): no PVR provider", __func__);
        return;
    }

    QBPVRProviderAddListener(self->appGlobals->pvrProvider, (SvObject) self);
    self->epgManager = SVRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));
}

void
QBPVRAgentStop(QBPVRAgent self)
{
    if (!self->appGlobals->pvrProvider) {
        SvLogError("%s(): no PVR provider", __func__);
        return;
    }

    QBPVRProviderRemoveListener(self->appGlobals->pvrProvider, (SvObject) self);
}

extern void
QBPVRAgentTrackRequest(QBPVRAgent self, QBPVRProviderRequest request, SvEPGEvent event)
{
    if (!self || !request || !event) {
        return;
    }

    SvHashTableInsert(self->trackedEvents, (SvGenericObject)request, (SvGenericObject)event);
    QBPVRProviderRequestAddListener(request, (SvGenericObject)self);
}
