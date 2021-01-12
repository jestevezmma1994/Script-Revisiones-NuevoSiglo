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

#include "epgWatcher.h"

#include <SvEPGDataLayer/SvEPGChannelListListener.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvEPGDataLayer/SvEPGDataListener.h>
#include <SvEPGDataLayer/SvEPGDataWindow.h>
#include <SvEPGDataLayer/SvEPGIterator.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <QBDataModel3/QBListModelListener.h>
#include <SvDataBucket2/SvDBObject.h>
#include <QBAppKit/QBAsyncService.h>
#include <QBAppKit/QBInitializable.h>
#include <QBAppKit/QBPropertiesMap.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <fibers/c/fibers.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvWeakList.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvTime.h>

#include <stdbool.h>
#include <limits.h>

#define QBTimeLimitEnable
#define QBTimeThreshold (50*1000)
#include <QBTimeLimit.h>

#define EPG_UPDATE_DELAY_MS 500

SvInterface
QBEPGWatcherListener_getInterface(void)
{
    static SvInterface interface = NULL;
    SvErrorInfo error = NULL;

    if (!interface) {
        SvInterfaceCreateManaged("QBEPGWatcherListener",
                                 sizeof(struct QBEPGWatcherListener_t),
                                 NULL, &interface, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            abort();
        }
    }

    return interface;
}

struct QBEPGWatcher_ {
    struct SvObject_ super_;

    QBAsyncServiceState serviceState;
    SvImmutableArray dependencies;

    SvWeakList lists;
    SvEPGView master;
    SvEPGManager manager;
    SvFiber fiber;
    SvFiberTimer timer;
    SvHashTable current, next;
    int idx;
    SvScheduler scheduler;
    SvEPGDataWindow epgWindow;

    SvWeakList listeners;
};

SvLocal bool
QBEPGWatcherUpdateChannelEPG(QBEPGWatcher self, SvTVChannel channel)
{
    SvEPGIterator eit = SvEPGManagerCreateIterator(self->manager, SvDBObjectGetID((SvDBObject) channel), SvTimeNow());
    SvEPGEvent current = NULL, next = NULL;
    if(eit) {
        current = SvEPGIteratorGetNextEvent(eit);
        next = SvEPGIteratorGetNextEvent(eit);
        SVRELEASE(eit);
    }

    bool changed = false;
    SvEPGEvent prevCurrent = (SvEPGEvent) SvHashTableFind(self->current, (SvGenericObject) channel);
    if(prevCurrent != current) {
        changed = true;
        if(current)
            SvHashTableInsert(self->current, (SvGenericObject) channel, (SvGenericObject) current);
        else
            SvHashTableRemove(self->current, (SvGenericObject) channel);
    }

    SvEPGEvent prevNext = (SvEPGEvent) SvHashTableFind(self->next, (SvGenericObject) channel);
    if(prevNext != next) {
        changed = true;
        if(next)
            SvHashTableInsert(self->next, (SvGenericObject) channel, (SvGenericObject) next);
        else
            SvHashTableRemove(self->next, (SvGenericObject) channel);
    }

    return changed;
}

SvLocal void
QBEPGWatcherNotifyChannelEventsUpdate(QBEPGWatcher self, SvTVChannel channel, bool changed)
{
    if (changed) {
        SvIterator it = SvWeakListIterator(self->lists);
        SvEPGView list;
        while((list = (SvEPGView) SvIteratorGetNext(&it))) {
            SvEPGViewPropagateChannelsChange(list, SvInvokeInterface(SvEPGChannelView, list, getChannelIndex, channel), 1);
        }
    }

    SvEPGEvent current = (SvEPGEvent) SvHashTableFind(self->current, (SvGenericObject) channel);
    SvEPGEvent next = (SvEPGEvent) SvHashTableFind(self->next, (SvGenericObject) channel);

    SvIterator lit = SvWeakListIterator(self->listeners);
    SvGenericObject listener;
    while((listener = SvIteratorGetNext(&lit))) {
        if(changed)
            SvInvokeInterface(QBEPGWatcherListener, listener, eventsChanged, channel, current, next);
        else
            SvInvokeInterface(QBEPGWatcherListener, listener, eventsUnchanged, channel, current, next);
    }
}

SvLocal void
QBEPGWatcherEPGDataUpdated(SvGenericObject self_,
                                const SvTimeRange *timeRange,
                                SvValue channelID)
{
    QBEPGWatcher self = (QBEPGWatcher)self_;
    SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, self->master, getByID, channelID);
    if(!channel)
        return;
    if(!QBEPGWatcherUpdateChannelEPG(self, channel))
        return;
    QBEPGWatcherNotifyChannelEventsUpdate(self, channel, true);
}

SvLocal void QBEPGWatcherStep(void *self_)
{
    QBEPGWatcher self = self_;

    QBTimeLimitBegin(timeEPGWatcher, QBTimeThreshold);

    SvTimeRange timeRange;
    time_t t = SvTimeNow();
    SvTimeRangeInit(&timeRange, t-120, t+60*10); // [-2min, +10min]
    SvEPGDataWindowSetTimeRange(self->epgWindow, timeRange, NULL);
    QBTimeLimitEnd_(timeEPGWatcher, "EPGWatcher.DataWindowSetTimeRange");

    SvEPGDataWindowSetChannelsRange(self->epgWindow, 0, 0, NULL);
    QBTimeLimitEnd_(timeEPGWatcher, "EPGWatcher.SetChannelsRange");

    SvFiberDeactivate(self->fiber);
    int count = SvInvokeInterface(SvEPGChannelView, self->master, getCount);
    int lag = 0;
    for(;self->idx < count && lag < 3; self->idx++, lag++) {
        SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, self->master, getByIndex, self->idx);

        QBEPGWatcherNotifyChannelEventsUpdate(self, channel, QBEPGWatcherUpdateChannelEPG(self, channel));
    }
    if(self->idx >= count)
        self->idx = 0;

    QBTimeLimitEnd_(timeEPGWatcher, "EPGWatcher.NotifyChannelEventsUpdate");

    SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(1,0));
}

void QBEPGWatcherRemoveList(QBEPGWatcher self, SvEPGView list)
{
    SvWeakListRemoveObject(self->lists, (SvGenericObject) list);
}

void QBEPGWatcherAddList(QBEPGWatcher self, SvEPGView list)
{
    SvWeakListPushBack(self->lists, (SvGenericObject) list, NULL);
}

SvLocal void QBEPGWatcher__dtor__(void *self_)
{
    QBEPGWatcher self = self_;
    SVTESTRELEASE(self->dependencies);
    if (self->fiber)
        SvFiberDestroy(self->fiber);
    SVTESTRELEASE(self->lists);
    SVTESTRELEASE(self->manager);
    SVTESTRELEASE(self->master);
    SVTESTRELEASE(self->current);
    SVTESTRELEASE(self->next);
    SVTESTRELEASE(self->epgWindow);
    SVTESTRELEASE(self->listeners);
}

SvLocal void
QBEPGWatcherStubFunction(SvGenericObject self_,
                         SvTVChannel channel)
{
}

SvLocal void
QBEPGWatcherChannelListCompletedStubFunction(SvGenericObject self_, int pluginID)
{
}

SvLocal SvString
QBEPGWatcherGetName(SvObject self_)
{
    return SVSTRING("QBEPGWatcher");
}

SvLocal SvImmutableArray
QBEPGWatcherGetDependencies(SvObject self_)
{
    QBEPGWatcher self = (QBEPGWatcher) self_;
    return self->dependencies;
}

SvLocal QBAsyncServiceState
QBEPGWatcherGetState(SvObject self_)
{
    QBEPGWatcher self = (QBEPGWatcher) self_;
    return self->serviceState;
}

SvLocal void
QBEPGWatcherStart(SvObject self_, SvScheduler scheduler, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBEPGWatcher self = (QBEPGWatcher) self_;

    if (self->serviceState == QBAsyncServiceState_running) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBEPGWatcher service is already running");
        goto out;
    }

    self->scheduler = scheduler;

    self->manager = SVTESTRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                             SVSTRING("SvEPGManager")));
    if (!self->manager) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBServiceRegistryGetService(\"SvEPGManager\") failed");
        goto out;
    }

    self->epgWindow = SvEPGManagerCreateDataWindow(self->manager, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvEPGManagerCreateDataWindow() failed");
        goto out;
    }

    SvEPGDataWindowSetChannelView(self->epgWindow, (SvObject) self->master, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvEPGDataWindowSetChannelView() failed");
        goto out;
    }

    SvEPGDataWindowAddListener(self->epgWindow, (SvGenericObject) self, NULL);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvEPGDataWindowAddListener() failed");
        goto out;
    }

    SvEPGManagerUpdateParams params = SvEPGManagerCreateUpdateParameters(self->manager);
    params->updateDelayMs = EPG_UPDATE_DELAY_MS;
    params->maxChannels = INT_MAX;
    SvEPGManagerSetUpdatesStrategyForClient(self->manager, (SvObject) self->epgWindow, params);
    SVRELEASE(params);

    self->fiber = SvFiberCreate(self->scheduler, NULL, "EPGWatcher", QBEPGWatcherStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    SvFiberSetPriority(self->fiber, 5);
    SvFiberActivate(self->fiber);

    self->serviceState = QBAsyncServiceState_running;
out:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBEPGWatcherStop(SvObject self_, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBEPGWatcher self = (QBEPGWatcher) self_;

    if (self->serviceState != QBAsyncServiceState_running) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBEPGWatcher service is not running");
        goto out;
    }

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
    }

    SVRELEASE(self->manager);
    self->manager = NULL;

    SvEPGDataWindowRemoveListener(self->epgWindow, (SvObject) self, NULL);
    SVRELEASE(self->epgWindow);
    self->epgWindow = NULL;

    self->serviceState = QBAsyncServiceState_idle;
out:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal SvObject
QBEPGWatcherInit(SvObject self_,
                 QBPropertiesMap properties,
                 SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBEPGWatcher self = (QBEPGWatcher) self_;

    if (!properties) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL properties passed");
        goto out;
    }

    SvEPGView master = (SvEPGView)
        QBPropertiesMapGetProperty(properties, SVSTRING("master"));
    if (unlikely(!master)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "no master in properties passed");
        goto out;
    }

    self->dependencies = SvImmutableArrayCreateWithTypedValues("s", &error, "SvEPGManager");
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvImmutableArrayCreateWithTypedValues() failed");
        goto out;
    }

    self->lists = SvWeakListCreate(&error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvWeakListCreate() failed");
        goto out;
    }

    self->master = SVRETAIN(master);

    self->current = SvHashTableCreate(7, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvHashTableCreate() failed");
        goto out;
    }

    self->next = SvHashTableCreate(7, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvHashTableCreate() failed");
        goto out;
    }

    QBEPGWatcherAddList(self, self->master);

    self->listeners = SvWeakListCreate(&error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvWeakListCreate() failed");
        goto out;
    }

out:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self_;
}

SvType
QBEPGWatcher_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBEPGWatcher__dtor__
    };

    static const struct SvEPGDataListener_ dlMethods = {
        .dataUpdated = QBEPGWatcherEPGDataUpdated
    };

    static const struct SvEPGChannelListListener_ listMethods = {
        .channelFound         = QBEPGWatcherStubFunction,
        .channelLost          = QBEPGWatcherStubFunction,
        .channelModified      = QBEPGWatcherStubFunction,
        .channelListCompleted = QBEPGWatcherChannelListCompletedStubFunction
    };

    static struct QBAsyncService_ asyncServiceMethods = {
        .getName         = QBEPGWatcherGetName,
        .getDependencies = QBEPGWatcherGetDependencies,
        .getState        = QBEPGWatcherGetState,
        .start           = QBEPGWatcherStart,
        .stop            = QBEPGWatcherStop
    };

    static const struct QBInitializable_ initializableMethods = {
        .init = QBEPGWatcherInit,
    };

    static SvType myType = NULL;

    if (!myType) {
        SvTypeCreateManaged("QBEPGWatcher",
                            sizeof(struct QBEPGWatcher_),
                            SvObject_getType(),
                            &myType,
                            SvObject_getType(), &objectVTable,
                            SvEPGChannelListListener_getInterface(), &listMethods,
                            SvEPGDataListener_getInterface(), &dlMethods,
                            QBAsyncService_getInterface(), &asyncServiceMethods,
                            QBInitializable_getInterface(), &initializableMethods,
                            NULL);
    }

    return myType;
}

void QBEPGWatcherEnable(QBEPGWatcher self)
{
    if (self->serviceState != QBAsyncServiceState_running) {
        SvLogError("%s(): QBEPGWatcher not running", __func__);
        return;
    }

    if (self->fiber) {
        SvLogError("%s(): QBEPGWatcher already enabled", __func__);
        return;
    }

    self->fiber = SvFiberCreate(self->scheduler, NULL, "EPGWatcher", QBEPGWatcherStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    SvFiberSetPriority(self->fiber, 5);
    SvFiberActivate(self->fiber);
}

void QBEPGWatcherDisable(QBEPGWatcher self)
{
    if (self->serviceState != QBAsyncServiceState_running) {
        SvLogError("%s(): QBEPGWatcher not running", __func__);
        return;
    }

    if (!self->fiber) {
        SvLogError("%s(): QBEPGWatcher already disabled", __func__);
        return;
    }

    SvFiberDestroy(self->fiber);
    self->fiber = NULL;
}

void QBEPGWatcherAddListener(QBEPGWatcher self, SvGenericObject listener)
{
    SvWeakListPushFront(self->listeners, listener, NULL);

    int idx, count = SvInvokeInterface(SvEPGChannelView, self->master, getCount);
    for(idx=0; idx < count; idx++) {
        SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, self->master, getByIndex, idx);
        SvEPGEvent current = (SvEPGEvent) SvHashTableFind(self->current, (SvGenericObject) channel);
        SvEPGEvent next = (SvEPGEvent) SvHashTableFind(self->next, (SvGenericObject) channel);
        if(current || next)
            SvInvokeInterface(QBEPGWatcherListener, listener, eventsChanged, channel, current, next);
    }
}

void QBEPGWatcherRemoveListener(QBEPGWatcher self, SvObject listener)
{
    SvWeakListRemoveObject(self->listeners, listener);
}
