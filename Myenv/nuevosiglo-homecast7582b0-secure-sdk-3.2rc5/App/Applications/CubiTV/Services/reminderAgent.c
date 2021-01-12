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


#include "reminderAgent.h"
#include <SvEPGDataLayer/SvEPGManager.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvIterator.h>
#include <QBDataModel3/QBSortedList.h>
#include <main.h>

struct QBReminderAgent_ {
    struct SvObject_ super_;
    QBSortedList tasks;
    AppGlobals appGlobals;
};

SvLocal void QBReminderAgent__dtor__(void *self_)
{
    QBReminderAgent self = self_;
    SVRELEASE(self->tasks);
}

SvLocal void QBReminderAgentPropagateChange(QBReminderAgent self, QBReminderTask task)
{
    SvTimeRange range = {
        .startTime = task->startTime,
        .endTime = task->endTime
    };
    SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                         SVSTRING("SvEPGManager"));
    SvEPGManagerPropagateEventsChange(epgManager, task->channelID, &range, NULL);
}

SvLocal void QBReminderAgentReminderAdded(SvGenericObject self_, QBReminderTask task)
{
    QBReminderAgent self = (QBReminderAgent) self_;
    QBSortedListInsert(self->tasks, (SvGenericObject) task);
    QBReminderAgentPropagateChange(self, task);
}

SvLocal void QBreminderAgentReminderRemoved(SvGenericObject self_, QBReminderTask task)
{
    QBReminderAgent self = (QBReminderAgent) self_;
    QBSortedListRemove(self->tasks, (SvGenericObject) task);
    QBReminderAgentPropagateChange(self, task);
}

SvLocal SvType QBReminderAgent_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBReminderAgent__dtor__
    };
    static SvType type = NULL;
    static const struct QBReminderListener_t reminderListener = {
        .reminderAdded = QBReminderAgentReminderAdded,
        .reminderRemoved = QBreminderAgentReminderRemoved,
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBReminderAgent",
                            sizeof(struct QBReminderAgent_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBReminderListener_getInterface(), &reminderListener,
                           NULL);
    }

    return type;
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

QBReminderAgent QBReminderAgentCreate(AppGlobals appGlobals)
{
    QBReminderAgent self = (QBReminderAgent) SvTypeAllocateInstance(QBReminderAgent_getType(), NULL);
    self->tasks = QBSortedListCreateWithCompareFn(QBReminderTaskCompare, NULL, NULL);
    self->appGlobals = appGlobals;

    return self;
}

SvGenericObject QBReminderAgentGetTasks(QBReminderAgent self)
{
    return (SvGenericObject) self->tasks;
}

void QBReminderAgentStart(QBReminderAgent self)
{
    SvIterator it = QBReminderTaskIterator(self->appGlobals->reminder);
    QBSortedListInsertObjects(self->tasks, it);
    QBReminderAddListener(self->appGlobals->reminder, (SvGenericObject) self);
}

void QBReminderAgentStop(QBReminderAgent self)
{
    QBReminderRemoveListener(self->appGlobals->reminder, (SvGenericObject) self);
}
