/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBPCLOCKEDTIMES_H
#define QBPCLOCKEDTIMES_H

/**
 * @file QBPCLockedTimes.h
 * @brief Parental control locked times service.
 **/

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvArray.h>
#include <stdbool.h>

#define QBPC_LT_MAX_TASKS 10

typedef enum {
    QBParentalControlLockedTime_sundays = 0,
    QBParentalControlLockedTime_mondays,
    QBParentalControlLockedTime_tuesdays,
    QBParentalControlLockedTime_wednesdays,
    QBParentalControlLockedTime_thursdays,
    QBParentalControlLockedTime_fridays,
    QBParentalControlLockedTime_saturdays,
    QBParentalControlLockedTime_weekdays,
    QBParentalControlLockedTime_weekends,
    QBParentalControlLockedTime_alldays,
    QBParentalControlLockedTime_end
} QBParentalControlLockedTime;

typedef struct QBParentalControlLockedTimeTask_t* QBParentalControlLockedTimeTask;
struct QBParentalControlLockedTimeTask_t {
    struct SvObject_ super_;
    int startTime;
    int endTime;
    QBParentalControlLockedTime option;
    bool state;
};

struct QBParentalControl_;

SvArray
QBParentalControlGetLockedTimeTasks(struct QBParentalControl_ *pc);

int
QBParentalControlGetLockedTimeTasksCount(struct QBParentalControl_ *pc);

const char*
QBParentalControlLimitedTimeDayToString(QBParentalControlLockedTime option);

QBParentalControlLockedTimeTask
QBParentalControlLockedTimeTaskNew(struct QBParentalControl_ *pc, int start, int end, bool state, QBParentalControlLockedTime option);

void
QBParentalControlLockedTimeRemoveTask(struct QBParentalControl_ *self, QBParentalControlLockedTimeTask task);

/**
 * Check if in QBParentalControl exists task with the same parameters as the task given in second parameter.
 *
 * @param[in] pc pointer to QBParentalControl
 * @param[in] task to search
 * @return @c true when task exists or @c false in other case
 **/
bool
QBParentalControlIsLockedTimeTaskExist(struct QBParentalControl_ *pc, QBParentalControlLockedTimeTask task);

void
QBParentalControlAddAndSaveLockedTimeTask(struct QBParentalControl_ *pc, QBParentalControlLockedTimeTask task);

#endif // QBPCLOCKEDTIMES_H
