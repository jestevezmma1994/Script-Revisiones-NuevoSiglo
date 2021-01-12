/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TIMER_TASK_H_
#define QB_TIMER_TASK_H_

/**
 * @file QBTimerTask.h
 * @brief Timer task class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBTimerTask Timer task class
 * @ingroup QBAppKit
 * @{
 *
 * This abstract class represents an operation scheduled to be performed
 * at later time. For more information refer to @ref TimerManager.
 **/

/**
 * Timer task class.
 * @class QBTimerTask QBTimerTask.h <QBAppKit/QBTimerTask.h>
 * @extends SvObject
 * @implements QBRunnable
 **/
typedef struct QBTimerTask_ *QBTimerTask;

/**
 * QBTimerTask class internals.
 * @private
 **/
struct QBTimerTask_ {
    /// super class
    struct SvObject_ super;
};

/**
 * QBTimerTask virtual methods table.
 **/
typedef const struct QBTimerTaskVTable_ {
    ///< virtual methods of the base class
    struct SvObjectVTable_ super_;

    /**
     * Calculate time of the next execution.
     *
     * This method is called by timer manager just before QBRunnable_::run()
     * to determine how to schedule the next tick. Default implementation just
     * advances @a tickTime by @a period. You can provide your own version to
     * implement custom behaviour.
     *
     * @param[in] self      timer task handle
     * @param[in] tickTime  time of the current timer tick
     * @param[in] period    timer period in milliseconds as passed to QBTimerTaskSetup()
     * @return              time of the next tick, value of SvTimeGetZero()
     *                      to stop the timer after current timer tick
     **/
    SvTime (*getNextTickTime)(QBTimerTask self,
                              SvTime tickTime,
                              unsigned int period);

    /**
     * Stop (cancel) timer task if it is scheduled for execution.
     *
     * You can provide your own implementation of this method if you need
     * to perform some actions when timer task is stopped. Please remember
     * to call super class implementation from your own, this is necessary
     * to actually remove this scheduled task from the timer manager.
     *
     * @param[in] self      timer task handle
     **/
    void (*stop)(QBTimerTask self);
} *QBTimerTaskVTable;


/**
 * Get runtime type identification object representing QBTimerTask class.
 *
 * @relates QBTimerTask
 *
 * @return timer task class
 **/
extern SvType
QBTimerTask_getType(void);

/**
 * @copydoc QBTimerTaskVTable::stop()
 * @memberof QBTimerTask
 **/
static inline void
QBTimerTaskStop(QBTimerTask self)
{
    SvInvokeVirtual(QBTimerTask, self, stop);
}

/**
 * Schedule next execution of a timer task and (optionally) a tick period
 * for scheduling it later in a periodic manner.
 *
 * @memberof QBTimerTask
 *
 * @param[in] self      timer task handle
 * @param[in] nextTickTime time to schedule next execution at
 * @param[in] period    timer tick period in milliseconds,
 *                      @c 0 for one-shot execution
 * @param[out] errorOut error info
 **/
extern void
QBTimerTaskSetup(QBTimerTask self,
                 SvTime nextTickTime,
                 unsigned int period,
                 SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
