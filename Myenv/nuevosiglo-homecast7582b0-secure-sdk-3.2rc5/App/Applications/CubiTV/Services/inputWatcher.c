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

#include "inputWatcher.h"
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvBinaryTree.h>
#include <QBInput/QBInputService.h>

#define INPUT_WATCHER_STEP_PERIOD_MS    250

/**
 * after DEAD_LISTENERS_REMOVAL_PERIOD_MS ms will be removed dead listeners
 **/
#define DEAD_LISTENERS_REMOVAL_PERIOD_MS    (INPUT_WATCHER_STEP_PERIOD_MS * 1000)


struct QBInputWatcherListenerTag_t {
    struct SvObject_ super_;
    SvWeakReference listener;
    time_t delay;
    bool notified;
};
typedef struct QBInputWatcherListenerTag_t* QBInputWatcherListenerTag;

struct QBInputWatcher_t {
    struct SvObject_ super_;
    SvBinaryTree tags;
    SvHashTable listenerToTag;
    SvScheduler scheduler;

    SvFiber fiber;
    SvFiberTimer timer;
    SvFiberTimer poll;
    SvTime lastChange;
};

SvInterface QBInputWatcherListener_getInterface(void)
{
    static SvInterface interface = NULL;
    if (!interface) {
        SvInterfaceCreateManaged("QBInputWatcherListener", sizeof(struct QBInputWatcherListener_t),
                                 NULL, &interface, NULL);
    }

    return interface;
}

SvLocal void QBInputWatcherListenerTag__dtor__(void *self_)
{
    QBInputWatcherListenerTag self = self_;
    SVRELEASE(self->listener);
}

SvLocal SvType QBInputWatcherListenerTag_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBInputWatcherListenerTag__dtor__
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBInputWatcherListenerTag",
                            sizeof(struct QBInputWatcherListenerTag_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal int QBInputWatcherListenerTagSort(void *unused, SvGenericObject objA, SvGenericObject objB)
{
    if (objA == objB)
        return 0;

    QBInputWatcherListenerTag listener1 = (QBInputWatcherListenerTag) objA;
    QBInputWatcherListenerTag listener2 = (QBInputWatcherListenerTag) objB;
    if (listener1->delay < listener2->delay)
        return -1;
    else if (listener1->delay > listener2->delay)
        return 1;
    else if (listener1 < listener2)
        return -1;
    else
        return 1;
}

SvLocal void QBInputWatcherPrepareForNotifications(QBInputWatcher self, QBInputWatcherListenerTag listener)
{
    if (!self->fiber)
        return;

    SvIterator it;
    if (!listener)
        it = SvBinaryTreeIterator(self->tags);
    else
        it = SvBinaryTreeLowerBound(self->tags, (SvGenericObject) listener);

    while ((listener = (QBInputWatcherListenerTag) SvIteratorGetNext(&it))) {
        if (!listener->notified)
            break;
    }
    if (!listener)
        return;

    SvTime lastEventTime = QBInputServiceGetLastEventTime(NULL);
    int64_t diff = (int64_t) listener->delay * 1000 - (int64_t) (SvTimeToMs(SvTimeSub(SvTimeGet(), lastEventTime)));


    if (diff < 0)
        SvFiberActivate(self->fiber);
    else
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(diff));
}

SvLocal void QBInputWatcher__dtor__(void *self_)
{
    QBInputWatcher self = self_;
    SVRELEASE(self->tags);
    if (self->fiber)
        SvFiberDestroy(self->fiber);
}

SvLocal SvType QBInputWatcher_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBInputWatcher__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBInputWatcher",
                            sizeof(struct QBInputWatcher_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
QBInputWatcherAddDeadListenersToTrash(QBInputWatcher self, SvArray *trash)
{
    QBInputWatcherListenerTag listener;
    SvGenericObject obj = NULL;
    SvIterator it = SvBinaryTreeIterator(self->tags);

    while ((listener = (QBInputWatcherListenerTag) SvIteratorGetNext(&it))) {
        if (!(obj = SvWeakReferenceTakeReferredObject(listener->listener))) {
            if (!*trash)
                *trash = SvArrayCreate(NULL);
            SvArrayAddObject(*trash, (SvGenericObject) listener);
        } else {
            SVRELEASE(obj);
        }
    }
}

SvLocal void QBInputWatcherStep(void *self_)
{
    static int loopCnt = 0;
    QBInputWatcher self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    SvFiberEventDeactivate(self->poll);

    SvTime lastEventTime = QBInputServiceGetLastEventTime(NULL);

    if (SvTimeCmp(lastEventTime, self->lastChange) > 0) {
        SvIterator it = SvBinaryTreeIterator(self->tags);
        QBInputWatcherListenerTag listener;
        while ((listener = (QBInputWatcherListenerTag) SvIteratorGetNext(&it))) {
            listener->notified = false;
        }
    }

    self->lastChange = lastEventTime;

    SvArray trash = NULL;
    QBInputWatcherListenerTag listener;
    SvIterator it = SvBinaryTreeIterator(self->tags);
    uint64_t diff = (SvTimeToMs(SvTimeSub(SvTimeGet(), lastEventTime)));

    while ((listener = (QBInputWatcherListenerTag) SvIteratorGetNext(&it))) {
        if (listener->notified)
            continue;
        if (((uint64_t) listener->delay) * 1000 <= diff) {
            SvGenericObject listenerObj = SvWeakReferenceTakeReferredObject(listener->listener);
            if (!listenerObj) {
                if (!trash)
                    trash = SvArrayCreate(NULL);

                SvArrayAddObject(trash, (SvGenericObject) listener);
                continue;
            }
            listener->notified = true;
            SvInvokeInterface(QBInputWatcherListener, listenerObj, tick, self);
            SVRELEASE(listenerObj);
        } else
            break;
    }

    QBInputWatcherPrepareForNotifications(self, listener);

    ++loopCnt;
    if (loopCnt == DEAD_LISTENERS_REMOVAL_PERIOD_MS / INPUT_WATCHER_STEP_PERIOD_MS) {
        QBInputWatcherAddDeadListenersToTrash(self, &trash);
        loopCnt = 0;
    }

    if (trash) {
        it = SvArrayIterator(trash);
        while ((listener = (QBInputWatcherListenerTag) SvIteratorGetNext(&it))) {
            SvBinaryTreeRemove(self->tags, (SvGenericObject) listener);
        }
        SVRELEASE(trash);
    }

    SvFiberTimerActivateAfter(self->poll, SvTimeFromMs(INPUT_WATCHER_STEP_PERIOD_MS));
}

QBInputWatcher QBInputWatcherCreate(SvScheduler scheduler)
{
    QBInputWatcher self = (QBInputWatcher) SvTypeAllocateInstance(QBInputWatcher_getType(), NULL);
    self->tags = SvBinaryTreeCreateWithCompareFn(QBInputWatcherListenerTagSort, NULL, NULL);
    self->scheduler = scheduler;
    self->lastChange = SvTimeGet();

    return self;
}

void QBInputWatcherAddListener(QBInputWatcher self, SvGenericObject listenerObj, time_t delay)
{
    QBInputWatcherListenerTag listener = (QBInputWatcherListenerTag) SvTypeAllocateInstance(QBInputWatcherListenerTag_getType(), NULL);

    listener->listener = SvWeakReferenceCreateWithObject(listenerObj, NULL);
    listener->delay = delay;

    SvIterator it = SvBinaryTreeIterator(self->tags);
    QBInputWatcherListenerTag prevListener;
    while ((prevListener = (QBInputWatcherListenerTag) SvIteratorGetNext(&it))) {
        SvGenericObject prevListenerObj = SvWeakReferenceTakeReferredObject(prevListener->listener);
        SVTESTRELEASE(prevListenerObj);
        if (listenerObj == prevListenerObj) {
            SvBinaryTreeRemove(self->tags, (SvGenericObject) prevListener);
            break;
        }
    }

    SvBinaryTreeInsert(self->tags, (SvGenericObject) listener);
    SVRELEASE(listener);

    QBInputWatcherPrepareForNotifications(self, NULL);
}

void QBInputWatcherAddListenerWithRelativeDelay(QBInputWatcher self, SvGenericObject listenerObj, time_t delay)
{
    SvTime lastEventTime = QBInputServiceGetLastEventTime(NULL);
    int64_t diff = (int64_t) (SvTimeToMs(SvTimeSub(SvTimeGet(), lastEventTime)));
    QBInputWatcherAddListener(self, listenerObj, delay + diff / 1000);
}

void QBInputWatcherRemoveListener(QBInputWatcher self, SvGenericObject listenerObj)
{
    SvIterator it = SvBinaryTreeIterator(self->tags);
    QBInputWatcherListenerTag prevListener;
    while ((prevListener = (QBInputWatcherListenerTag) SvIteratorGetNext(&it))) {
        SvGenericObject prevListenerObj = SvWeakReferenceTakeReferredObject(prevListener->listener);
        SVTESTRELEASE(prevListenerObj);
        if (listenerObj == prevListenerObj) {
            SvBinaryTreeRemove(self->tags, (SvGenericObject) prevListener);
            break;
        }
    }
}

void QBInputWatcherStart(QBInputWatcher self)
{
    self->fiber = SvFiberCreate(self->scheduler, NULL, "QBInputWatcher", QBInputWatcherStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    self->poll = SvFiberTimerCreate(self->fiber);
    QBInputWatcherPrepareForNotifications(self, NULL);
}

void QBInputWatcherStop(QBInputWatcher self)
{
    SvFiberDestroy(self->fiber);
    self->fiber = NULL;
}
