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

#include "reminder.h"

#include <libintl.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvWeakList.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvBinaryTree.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvJSON/SvJSONClassRegistry.h>
#include <SvJSON/SvJSONParse.h>
#include <SvJSON/SvJSONSerialize.h>
#include <QBStaticStorage.h>
#include <QBApplicationController.h>
#include <safeFiles.h>
#include <Logic/TVLogic.h>
#include <Services/core/playlistManager.h>
#include <SvEPGDataLayer/SvEPGChannelListListener.h>
#include <SvEPGDataLayer/SvEPGChannelListPlugin.h>
#include <Windows/newtv.h>
#include <Widgets/confirmationDialog.h>
#include <QBWidgets/QBDialog.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <QBAppKit/QBAsyncService.h>
#include <QBAppKit/QBInitializable.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBDataModel3/QBListModel.h>
#include <QBDataModel3/QBListModelListener.h>
#include <QBDataModel3/QBDataSource.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCommonDefs.h>
#include <main.h>


struct QBReminder_t {
    struct SvObject_ super_;
    SvBinaryTree byTime;
    SvHashTable byUID;
    SvString fileName;

    SvFiber fiber;
    SvFiberTimer timer;

    SvHashTable acceptedTasks;

    SvWeakList listeners;

    time_t remindTimeOffset;

    struct QBReminderRemindedTask {
        SvWidget widget;
        QBReminderTask task;
    } dialog;

    QBReminderTask key;

    QBAsyncServiceState state;
    SvImmutableArray dependencies;

    SvApplication app;
};

SvLocal void QBReminderChannelFound(SvObject self_, SvTVChannel channel);
SvLocal void QBReminderChannelLost(SvObject self_, SvTVChannel channel);
SvLocal void QBReminderChannelModified(SvObject self_, SvTVChannel channel);
SvLocal void QBReminderChannelListCompleted(SvObject self_, int pluginID);

SvLocal void QBReminderTask__dtor__(void *ptr)
{
    QBReminderTask self = ptr;
    SVTESTRELEASE(self->channelID);
}

SvType QBReminderTask_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBReminderTask__dtor__
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBReminderTask",
                            sizeof(struct QBReminderTask_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvInterface QBReminderListener_getInterface(void)
{
    static SvInterface interface = NULL;
    if (!interface) {
        SvInterfaceCreateManaged("QBReminderListener",
                                 sizeof(struct QBReminderListener_t),
                                 NULL, &interface,
                                 NULL);
    }

    return interface;
}

SvLocal void QBReminderDestroy(void *self_)
{
    QBReminder self = self_;
    SVRELEASE(self->dependencies);
    SVRELEASE(self->byUID);
    SVRELEASE(self->byTime);
    SVRELEASE(self->fileName);
    SVRELEASE(self->acceptedTasks);
    self->key->channelID = NULL;
    SVRELEASE(self->key);
    if(self->dialog.widget)
        QBDialogBreak(self->dialog.widget);
    SVTESTRELEASE(self->dialog.task);
    if (self->fiber)
        SvFiberDestroy(self->fiber);
    SVRELEASE(self->listeners);
}

SvLocal int QBReminderTaskCompare(void *prv, SvGenericObject objectA, SvGenericObject objectB)
{
    QBReminderTask first = (QBReminderTask) objectA, second = (QBReminderTask) objectB;
    if(abs((int64_t)first->startTime - (int64_t) second->startTime) < 20)
        return 0;
    if(first->startTime < second->startTime)
        return -1;
    else
        return 1;
}

SvLocal void QBReminderNotify(QBReminder self, bool isAdded, QBReminderTask task)
{
    SvIterator it = SvWeakListIterator(self->listeners);
    SvGenericObject listener;
    while ((listener = SvIteratorGetNext(&it))) {
        if (isAdded)
            SvInvokeInterface(QBReminderListener, listener, reminderAdded, task);
        else
            SvInvokeInterface(QBReminderListener, listener, reminderRemoved, task);
    }
}

SvLocal void QBReminderTaskSave(SvGenericObject helper_, SvGenericObject task_, SvHashTable desc, SvErrorInfo *errorOut)
{
    QBReminderTask self = (QBReminderTask) task_;
    SvValue startTime = SvValueCreateWithInteger(self->startTime, NULL);
    SvValue endTime = SvValueCreateWithInteger(self->endTime, NULL);

    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("startTime"), (SvGenericObject) startTime);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("endTime"), (SvGenericObject) endTime);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("channelID"), (SvGenericObject) self->channelID);

    SVRELEASE(startTime);
    SVRELEASE(endTime);
}

SvLocal SvGenericObject QBReminderTaskRestore(SvGenericObject helper_, SvString className, SvHashTable desc, SvErrorInfo *errorOut)
{
    SvValue startTime = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("startTime"));
    SvValue endTime = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("endTime"));
    SvValue channelID = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("channelID"));

    if (!startTime || !SvObjectIsInstanceOf((SvObject) startTime, SvValue_getType()) || !SvValueIsInteger(startTime))
        return NULL;
    if (!endTime || !SvObjectIsInstanceOf((SvObject) endTime, SvValue_getType()) || !SvValueIsInteger(endTime))
        return NULL;
    if (!channelID || !SvObjectIsInstanceOf((SvObject) channelID, SvValue_getType()) || !SvValueIsString(channelID))
        return NULL;

    QBReminderTask self = (QBReminderTask) SvTypeAllocateInstance(QBReminderTask_getType(), NULL);

    self->startTime = SvValueGetInteger(startTime);
    self->endTime = SvValueGetInteger(endTime);
    self->channelID = SVRETAIN(channelID);

    return (SvGenericObject) self;
}


SvLocal void QBReminderSaveTasks(QBReminder self)
{
    SvIterator it = SvBinaryTreeIterator(self->byTime);
    SvArray tasks = SvArrayCreateWithValues(&it, NULL);
    SvLogNotice("Saving %zd tasks from %zd", SvArrayCount(tasks), SvBinaryTreeNodesCount(self->byTime));
    SvString data = SvJSONDataToString((SvGenericObject) tasks, true, NULL);
    QBStaticStorageWriteFile(self->fileName, data);
    SVRELEASE(data);
    SVRELEASE(tasks);
}

SvLocal void QBReminderLoadTasks(QBReminder self)
{
    char *buffer;
    QBFileToBuffer(SvStringCString(self->fileName), &buffer);
    if(!buffer) {
        SvLogNotice("%s() : no reminders found in %s", __func__, SvStringCString(self->fileName));
        return;
    }
    SvArray tasks = (SvArray) SvJSONParseString(buffer, true, NULL);
    free(buffer);
    if (!tasks) {
        SvLogError("%s() : cannot parse %s, JSON is corrupted", __func__, SvStringCString(self->fileName));
        return;
    }

    if (!SvObjectIsInstanceOf((SvObject) tasks, SvArray_getType())) {
        SvLogError("%s() : cannot parse %s, JSON does not contain an array", __func__, SvStringCString(self->fileName));
        SVRELEASE(tasks);
        return;
    }


    SvBinaryTreeRemoveAllObjects(self->byTime);
    SvHashTableRemoveAllObjects(self->byUID);
    SvHashTableRemoveAllObjects(self->acceptedTasks);

    SvIterator it = SvArrayIterator(tasks);
    int cnt = SvArrayCount(tasks);
    QBReminderTask task;
    int i = 0;
    while ((task = (QBReminderTask) SvIteratorGetNext(&it))) {
        i++;
        if (!SvObjectIsInstanceOf((SvObject) task, QBReminderTask_getType()))
            continue;

        SvLogNotice("%s() : loading reminder %d out of %d", __func__, i, cnt);

        SvBinaryTreeInsert(self->byTime, (SvGenericObject) task);
        SvBinaryTree channelTree = (SvBinaryTree) SvHashTableFind(self->byUID, (SvGenericObject) task->channelID);
        if(!channelTree) {
            channelTree = SvBinaryTreeCreateWithCompareFn(QBReminderTaskCompare, NULL, NULL);
            SvHashTableInsert(self->byUID, (SvGenericObject) task->channelID, (SvGenericObject) channelTree);
            SVRELEASE(channelTree);
        }
        SvBinaryTreeInsert(channelTree, (SvGenericObject) task);
        QBReminderNotify(self, true, task);
    }

    SVTESTRELEASE(tasks);
}

SvLocal QBReminderTask QBReminderTaskNew(SvTVChannel channel, SvEPGEvent event)
{
    QBReminderTask self = (QBReminderTask) SvTypeAllocateInstance(QBReminderTask_getType(), NULL);
    self->startTime = event->startTime;
    self->endTime = event->endTime;
    SvDBObject obj = (SvDBObject) channel;
    self->channelID = SVRETAIN(obj->id);

    return self;
}

SvLocal void QBReminderRemoveTask(QBReminder self, QBReminderTask task)
{
    SvLogNotice("Removing a task");
    if(self->dialog.task == task)
        QBDialogBreak(self->dialog.widget);

    SVRETAIN(task);
    SvBinaryTreeRemove(self->byTime, (SvGenericObject) task);
    SvBinaryTree channelTree = (SvBinaryTree) SvHashTableFind(self->byUID, (SvGenericObject) task->channelID);

    if (channelTree) {
        SvBinaryTreeRemove(channelTree, (SvGenericObject) task);
        if (SvBinaryTreeNodesCount(channelTree) == 0) {
            SvHashTableRemove(self->byUID, (SvGenericObject) task->channelID);
        }
    }
    QBReminderNotify(self, false, task);
    SVRELEASE(task);
    QBReminderSaveTasks(self);
}

SvLocal void QBReminderPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBReminder self = self_;
    QBReminderTask task = self->dialog.task;
    if(!self->dialog.widget)
        return;

    self->dialog.task = NULL;
    self->dialog.widget = NULL;

    if (buttonTag && task) {
        if (SvStringEqualToCString(buttonTag, "OK-button"))
        {
            SvHashTableInsert(self->acceptedTasks, (SvGenericObject) task, (SvGenericObject) task);
        }
        else if (SvStringEqualToCString(buttonTag, "cancel-button"))
        {
            QBReminderRemoveTask(self, task);
            QBReminderSaveTasks(self);
        }
    }

    SVTESTRELEASE(task);
    SVRELEASE(self);
}

SvLocal void QBReminderShowPopup(QBReminder self, QBReminderTask task)
{
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject playlist = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
    SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, playlist, getByID, task->channelID);
    if(!channel)
        return;

    char* msg;
    asprintf(&msg, "%s %s. %s %s.", gettext("You will be soon zapped to"), SvStringCString(channel->name),
                                   gettext("Press Cancel to stay on current channel or press OK to go to"), SvStringCString(channel->name));

    QBConfirmationDialogParams_t params =
    {
            .title = gettext("Reminder"),
            .message = msg,
            .local = false,
            .globalName = SVSTRING("QBReminder"),
            .focusOK = true,
            .isCancelButtonVisible = true
    };

    self->dialog.widget = QBConfirmationDialogCreate(self->app, &params);
    free(msg);
    self->dialog.task = SVRETAIN(task);
    QBDialogRun(self->dialog.widget, SVRETAIN(self), QBReminderPopupCallback);
}

SvLocal void QBReminderRemind(QBReminder self, QBReminderTask task)
{
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBApplicationController controller =
        (QBApplicationController) QBServiceRegistryGetService(registry, SVSTRING("QBApplicationController"));
    QBApplicationControllerSwitchToRoot(controller);
    /// Switch to "on" permanently.

    QBStandbyStateData state = QBStandbyStateDataCreate(QBStandbyState_on, (SvObject) self, false, SVSTRING("reminder"));
    QBStandbyAgent standbyAgent = (QBStandbyAgent) QBGlobalStorageGetItem(globalStorage, QBStandbyAgent_getType(), NULL);
    QBStandbyAgentSetWantedState(standbyAgent, state);
    SVRELEASE(state);

    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject playlist = QBPlaylistManagerGetCurrent(playlists);
    SvTVChannel channel = NULL;

    if (playlist)
        channel = SvInvokeInterface(SvEPGChannelView, playlist, getByID, task->channelID);
    QBTVLogic tvLogic = (QBTVLogic) QBGlobalStorageGetItem(globalStorage,
                                                           QBTVLogic_getType(),
                                                           NULL);
    if (channel)
        QBTVLogicPlayChannel(tvLogic, channel, SVSTRING("QBReminder"));


    if (!playlist || !channel) {
        channel = NULL;
        playlist = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
        if (playlist)
            channel = SvInvokeInterface(SvEPGChannelView, playlist, getByID, task->channelID);
        if (channel)
            QBTVLogicPlayChannelFromPlaylist(tvLogic, SVSTRING("TVChannels"), channel, SVSTRING("QBReminder"));
    }

    if(task == self->dialog.task) {
        QBDialogBreak(self->dialog.widget);
    }
}

SvLocal void QBReminderStep(void *self_)
{
    QBReminder self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    time_t now = SvTimeNow();

    if(!self->dialog.widget) {
        QBReminderTask toRemind = NULL, next;
        SvIterator it = SvBinaryTreeIterator(self->byTime);
        while((next = (QBReminderTask) SvIteratorGetNext(&it))) {
            if(next->startTime <= now + self->remindTimeOffset) {
                if(next->endTime > now && !SvHashTableFind(self->acceptedTasks, (SvGenericObject) next)) {
                    toRemind = next;
                    break;
                }
            } else
                break;
        }

        if(toRemind)
            QBReminderShowPopup(self, toRemind);
    }


    //Find last task that started before now but isn't finished yet
    QBReminderTask toRun = NULL, next;
    bool anyRemoved = false;
    while((next = (QBReminderTask) SvBinaryTreeObjectAtIndex(self->byTime, 0))) {
        if(next->endTime < now) {
            QBReminderRemoveTask(self, next);
            anyRemoved = true;
        } else if(next->startTime > now) {
            break;
        } else {
            SVTESTRELEASE(toRun);
            toRun = SVRETAIN(next);
            QBReminderRemoveTask(self, next);
            anyRemoved = true;
        }
    }

    if(anyRemoved)
        QBReminderSaveTasks(self);

    if(toRun) {
        QBReminderRemind(self, toRun);
        SVRELEASE(toRun);
    }


    if(SvHashTableCount(self->byUID) != 0)
        SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(1,0));
}

SvLocal SvArray QBReminderFindConflicts(QBReminder self, QBReminderTask task)
{
    SvArray conflicts = NULL;
    SvIterator it = SvBinaryTreeLowerBound(self->byTime, (SvGenericObject) task);
    SvGenericObject conflict;

    while((conflict = SvIteratorGetNext(&it)) && QBReminderTaskCompare(NULL, (SvGenericObject) task, conflict) == 0) {
        if(!conflicts)
            conflicts = SvArrayCreate(NULL);
        SvArrayAddObject(conflicts, conflict);
    }

    return conflicts;
}

SvArray QBReminderResolveConflicts(QBReminder self, SvTVChannel channel, SvEPGEvent event)
{
    QBReminderTask task = QBReminderTaskNew(channel, event);
    SvArray conflicts = QBReminderFindConflicts(self, task);
    SVRELEASE(task);

    if(!conflicts)
        return NULL;

    SvIterator it = SvArrayIterator(conflicts);
    while((task = (QBReminderTask) SvIteratorGetNext(&it))) {
        QBReminderRemoveTask(self, task);
    }
    return conflicts;
}

int QBReminderAdd(QBReminder self, SvTVChannel channel, SvEPGEvent event, SvArray* conflicts)
{
    QBReminderTask task = QBReminderTaskNew(channel, event);
    if(event->startTime <= SvTimeNow())
        return 0;

    SvLogNotice("Adding new event to remind");
    SvDBObject obj = (SvDBObject) channel;
    *conflicts = QBReminderFindConflicts(self, task);
    if(*conflicts) {
        SVRELEASE(task);
        return SvArrayCount(*conflicts);
    }

    SvLogNotice("All ok, I'll add it now");

    SvBinaryTreeInsert(self->byTime, (SvGenericObject) task);
    SvBinaryTree channelTree = (SvBinaryTree) SvHashTableFind(self->byUID, (SvGenericObject) obj->id);
    if(!channelTree) {
        channelTree = SvBinaryTreeCreateWithCompareFn(QBReminderTaskCompare, NULL, NULL);
        SvHashTableInsert(self->byUID, (SvGenericObject) obj->id, (SvGenericObject) channelTree);
        SVRELEASE(channelTree);
    }

    SvBinaryTreeInsert(channelTree, (SvGenericObject) task);
    QBReminderNotify(self, true, task);

    SVRELEASE(task);

    SvFiberActivate(self->fiber);

    QBReminderSaveTasks(self);

    return 0;
}

bool QBReminderIsScheduledFor(QBReminder self, SvTVChannel channel, SvEPGEvent event)
{
    SvDBObject obj = (SvDBObject) channel;
    SvBinaryTree channelTree = (SvBinaryTree) SvHashTableFind(self->byUID, (SvGenericObject) obj->id);
    if(!channelTree)
        return false;

    self->key->startTime = event->startTime;
    self->key->endTime = event->endTime;
    self->key->channelID = obj->id;

    ssize_t idx = SvBinaryTreeGetObjectIndex(channelTree, (SvGenericObject) self->key);
    return idx >= 0;
}

void QBReminderRemoveByTask(QBReminder self, QBReminderTask task)
{
    SvValue channelID = (SvValue) task->channelID;
    SvBinaryTree channelTree = (SvBinaryTree) SvHashTableFind(self->byUID, (SvGenericObject) channelID);
    if(!channelTree)
        return;

    SvIterator it = SvBinaryTreeLowerBound(channelTree, (SvGenericObject) task);
    SvGenericObject other = SvIteratorGetNext(&it);
    if(other && QBReminderTaskCompare(NULL, (SvGenericObject) task, other) == 0) {
        QBReminderRemoveTask(self, (QBReminderTask) other);
        QBReminderSaveTasks(self);
    }

    if(SvHashTableCount(self->byUID) != 0)
        SvFiberActivate(self->fiber);
}

void QBReminderRemove(QBReminder self, SvTVChannel channel, SvEPGEvent event)
{
    QBReminderTask key = QBReminderTaskNew(channel, event);
    QBReminderRemoveTask(self, key);
    SVRELEASE(key);
}

void QBReminderRemoveAll(QBReminder self)
{
    while(SvBinaryTreeNodesCount(self->byTime)) {
        QBReminderTask task = (QBReminderTask)SvBinaryTreeObjectAtIndex(self->byTime, 0);
        QBReminderRemoveTask(self, task);
    }
    QBReminderSaveTasks(self);
}

// methods of QBAsyncService interface

SvLocal SvString
QBReminderGetName(SvObject self_)
{
    return SVSTRING("QBReminder");
}

SvLocal SvImmutableArray
QBReminderGetDependencies(SvObject self_)
{
    QBReminder self = (QBReminder) self_;
    return self->dependencies;
}

SvLocal QBAsyncServiceState
QBReminderGetState(SvObject self_)
{
    QBReminder self = (QBReminder) self_;
    return self->state;
}

SvLocal void
QBReminderStart_(SvObject self_,
                  SvScheduler scheduler,
                  SvErrorInfo *errorOut)
{
    QBReminder self = (QBReminder) self_;
    SvErrorInfo error = NULL;

    if (self->state != QBAsyncServiceState_idle) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBReminder is not in idle state");
        goto fini;
    }

    self->fiber = SvFiberCreate(scheduler, NULL, "QBReminder", QBReminderStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    DVBEPGPlugin channelListPlugin = (DVBEPGPlugin) QBGlobalStorageGetItem(globalStorage,
                                                                           DVBEPGPlugin_getType(),
                                                                           NULL);
    if (channelListPlugin)
        SvInvokeInterface(SvEPGChannelListPlugin, channelListPlugin, addListener, (SvGenericObject) self, false, NULL);
    QBReminderLoadTasks(self);
    if(SvHashTableCount(self->byUID) != 0)
        SvFiberActivate(self->fiber);
    self->state = QBAsyncServiceState_running;

fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBReminderStop_(SvObject self_,
                SvErrorInfo *errorOut)
{
    QBReminder self = (QBReminder) self_;
    SvErrorInfo error = NULL;

    if (self->state != QBAsyncServiceState_running) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBReminder is not in running state");
        goto fini;
    }
    self->state = QBAsyncServiceState_stopping;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    SvFiberDestroy(self->fiber);
    self->fiber = NULL;
    if(self->dialog.widget)
        QBDialogBreak(self->dialog.widget);

    self->state = QBAsyncServiceState_idle;

fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvType QBReminder_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBReminderDestroy
    };

    static const struct SvEPGChannelListListener_ dvbChannelListListenerMethods = {
        .channelFound = QBReminderChannelFound,
        .channelLost = QBReminderChannelLost,
        .channelModified = QBReminderChannelModified,
        .channelListCompleted = QBReminderChannelListCompleted
    };

    static const struct QBAsyncService_ serviceMethods = {
        .getName         = QBReminderGetName,
        .getDependencies = QBReminderGetDependencies,
        .getState        = QBReminderGetState,
        .start           = QBReminderStart_,
        .stop            = QBReminderStop_
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBReminder",
                            sizeof(struct QBReminder_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvEPGChannelListListener_getInterface(), &dvbChannelListListenerMethods,
                            QBAsyncService_getInterface(), &serviceMethods,
                            NULL);
    }
    return type;
}

QBReminder QBReminderCreate(SvApplication app, SvString fileName, time_t remindTimeOffset, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBReminder self = NULL;

    if(!app || !fileName) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Wrong params app=%p, fileName=%p", app, fileName);
        goto fini;
    }
    self = (QBReminder) SvTypeAllocateInstance(QBReminder_getType(), &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "Can't allocate Reminder class");
        goto fini;
    }

    self->app = app;
    self->byUID = SvHashTableCreate(31, NULL);
    self->dependencies = SvImmutableArrayCreateWithTypedValues("ss", &error,
                                                               "QBPlaylistManager",
                                                               "QBApplicationController");
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "Can't create ImmutableArray");
        goto fini;
    }

    self->byTime = SvBinaryTreeCreateWithCompareFn(QBReminderTaskCompare, NULL, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "Can't create Binary Tree");
        goto fini;
    }

    self->acceptedTasks = SvHashTableCreate(31, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "Can't create hash table");
        goto fini;
    }

    self->fileName = SVRETAIN(fileName);
    self->remindTimeOffset = remindTimeOffset;

    self->key = (QBReminderTask) SvTypeAllocateInstance(QBReminderTask_getType(), &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "Can't allocate Reminder task");
        goto fini;
    }

    self->listeners = SvWeakListCreate(&error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "Can't create weak list");
        goto fini;
    }
    SvJSONClass helper = SvJSONClassCreate(QBReminderTaskSave, QBReminderTaskRestore, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "Can't create JSONClass");
        goto fini;
    }
    SvJSONRegisterClassHelper(SVSTRING("QBReminderTask"), (SvGenericObject) helper, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "Can't register JSONClass helper");
        goto fini2;
    }
fini2:
    SVRELEASE(helper);
fini:
    SvErrorInfoPropagate(error, errorOut);
    if (error)
        SVTESTRELEASE(self);
    return error ? NULL : self;
}

SvIterator QBReminderTaskIterator(QBReminder self)
{
    return SvBinaryTreeIterator(self->byTime);
}

void QBReminderAddListener(QBReminder self, SvGenericObject listener)
{
    SvWeakListPushFront(self->listeners, listener, NULL);
}

void QBReminderRemoveListener(QBReminder self, SvGenericObject listener)
{
    SvWeakListRemoveObject(self->listeners, listener);
}

size_t QBReminderGetTasksCount(QBReminder self)
{
    return SvBinaryTreeNodesCount(self->byTime);
}

// SvEPGChannelListListener methods

SvLocal void QBReminderChannelFound(SvObject self_, SvTVChannel channel)
{
}

SvLocal void QBReminderChannelLost(SvObject self_, SvTVChannel channel)
{
    QBReminder self = (QBReminder) self_;
    SvValue channelID = ((SvDBObject) channel)->id;

    if(!SvHashTableFind(self->byUID, (SvObject) channelID))
        return;

    SvHashTableRemove(self->byUID, (SvObject) channelID);
    SvString channelIDString = SvValueGetString(channelID);

    SvArray channelReminders = NULL;
    SvIterator it = SvBinaryTreeIterator(self->byTime);
    SvObject task;

    while((task = SvIteratorGetNext(&it))) {
        SvString str = SvValueGetString(((QBReminderTask) task)->channelID);
        if (!SvObjectEquals((SvObject) channelIDString, (SvObject) str))
            continue;

        if(!channelReminders)
            channelReminders = SvArrayCreate(NULL);
        SvArrayAddObject(channelReminders, task);
    }

    if (channelReminders) {
        SvIterator channelReminderIterator = SvArrayIterator(channelReminders);
        while((task = SvIteratorGetNext(&channelReminderIterator))) {
            if(self->dialog.task == (QBReminderTask) task)
                QBDialogBreak(self->dialog.widget);
            SvBinaryTreeRemove(self->byTime, (SvObject) task);
            QBReminderNotify(self, false, (QBReminderTask) task);
        }
        SVRELEASE(channelReminders);
    }
    QBReminderSaveTasks(self);
}

SvLocal void QBReminderChannelModified(SvObject self_, SvTVChannel channel)
{
}

SvLocal void QBReminderChannelListCompleted(SvObject self_, int pluginID)
{
}
