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


#ifndef QBREMINDER_H_
#define QBREMINDER_H_

#include <main_decl.h>

#include <fibers/c/fibers.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvInterface.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <CUIT/Core/types.h>

#include <stdbool.h>
#include <time.h>

struct QBReminderTask_t {
    struct SvObject_ super_;
    time_t startTime;
    time_t endTime;
    SvValue channelID;
};
typedef struct QBReminderTask_t* QBReminderTask;

struct QBReminderListener_t {
    void (*reminderAdded)(SvGenericObject self_, QBReminderTask task);
    void (*reminderRemoved)(SvGenericObject self_, QBReminderTask task);
};
typedef struct QBReminderListener_t* QBReminderListener;

typedef struct QBReminder_t* QBReminder;

SvType QBReminderTask_getType(void);

/**
 * Get runtime type identification object representing QBReminder type.
 *
 * @return QBReminder runtime type identification object
**/
SvType QBReminder_getType(void);
SvInterface QBReminderListener_getInterface(void);

/**
 * Create QBReminder object.
 *
 * @param[in] app                      CUIT application handle
 * @param[in] fileName                 file to save QBReminder tasks
 * @param[in] remindTimeOffset         offset time for reminds
 * @param[out] errorOut                error info
 * @return QBReminder object
**/
QBReminder QBReminderCreate(SvApplication app, SvString fileName, time_t remindTimeOffset, SvErrorInfo *errorOut);
int QBReminderAdd(QBReminder self, SvTVChannel channel, SvEPGEvent event, SvArray* conflicts);
bool QBReminderIsScheduledFor(QBReminder self, SvTVChannel channel, SvEPGEvent event);
void QBReminderRemove(QBReminder self, SvTVChannel channel, SvEPGEvent event);
void QBReminderRemoveByTask(QBReminder self, QBReminderTask task);
void QBReminderRemoveAll(QBReminder self);
SvArray QBReminderResolveConflicts(QBReminder self, SvTVChannel channel, SvEPGEvent event);
SvIterator QBReminderTaskIterator(QBReminder self);
void QBReminderAddListener(QBReminder self, SvGenericObject listener);
void QBReminderRemoveListener(QBReminder self, SvGenericObject listener);
size_t QBReminderGetTasksCount(QBReminder self);

#endif
