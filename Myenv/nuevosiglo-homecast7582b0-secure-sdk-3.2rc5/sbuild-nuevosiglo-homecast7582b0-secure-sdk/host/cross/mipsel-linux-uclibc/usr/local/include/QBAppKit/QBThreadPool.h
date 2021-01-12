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

#ifndef QB_THREAD_POOL_H_
#define QB_THREAD_POOL_H_

/**
 * @file QBThreadPool.h
 * @brief Thread pool class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <QBAppKit/QBAsyncService.h>
#include <QBAppKit/QBFuture.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBThreadPool Thread pool class
 * @ingroup QBAppKit
 * @{
 *
 * @link QBThreadPool @endlink provides an easy to use abstraction layer
 * for delegating tasks to worker threads.
 *
 * Thread pools should be used by components that perform some CPU-intensive
 * tasks. Moving them to separate threads running with lower priority reduces
 * the risk of stalling main GUI thread and helps utilize the performance
 * of modern multi-core CPUs.
 *
 * Using thread pool is very simple: implement @ref QBRunnable in your
 * worker class and pass an instance of the worker to QBThreadPoolAddTask().
 * If you want to know when the task is done, implement an @ref QBObserver
 * and use QBObservableAddObserver() on the returned future.
 *
 * There is a thread pool instance available for other components, you can
 * access it using QBServiceRegistryGetService() method with well-known name
 * "ThreadPool". If your component wants to delegate more than a single
 * occasional task, it should create a thread pool for its own use.
 **/

/**
 * Thread pool class.
 * @class QBThreadPool
 * @extends QBObservable
 * @implements QBAsyncService
 **/
typedef struct QBThreadPool_ *QBThreadPool;


/**
 * Create a thread pool.
 *
 * @memberof QBThreadPool
 *
 * @param[in] name       pool name, used for debugging
 * @param[in] memoryZoneName name for the memory zone, optional
 *                       (created automatically when @c NULL is passed)
 * @param[in] minNumThreads number of threads created immediately by QBThreadPoolStart()
 *                       and maintained until QBThreadPoolStop() is called,
 *                       value in range [@c 1;@a maxNumThreads]
 * @param[in] maxNumThreads max number of threads that can be running when
 *                       pool is very busy, in range [@a minNumThreads;@c 4]
 * @param[out] errorOut  error info
 * @return               created thread pool, @c NULL in case of error
 **/
extern QBThreadPool
QBThreadPoolCreate(const char *name,
                   const char *memoryZoneName,
                   unsigned int minNumThreads,
                   unsigned int maxNumThreads,
                   SvErrorInfo *errorOut);

/**
 * Check if thread pool was started.
 *
 * @memberof QBThreadPool
 *
 * @param[in] self      thread pool handle
 * @return              @c true if thread pool was started, @c false otherwise
 **/
static inline bool
QBThreadPoolIsRunning(QBThreadPool self)
{
    QBAsyncServiceState state = SvInvokeInterface(QBAsyncService, self, getState);
    return state == QBAsyncServiceState_starting || state == QBAsyncServiceState_running || state == QBAsyncServiceState_stopping;
}

/**
 * Start a thread pool.
 *
 * This method creates worker threads and starts scheduling waiting workers.
 *
 * @memberof QBThreadPool
 *
 * @param[in] self      thread pool handle
 * @param[in] scheduler scheduler to be used for delivering notifications
 * @param[out] errorOut error info
 **/
static inline void
QBThreadPoolStart(QBThreadPool self,
                  SvScheduler scheduler,
                  SvErrorInfo *errorOut)
{
    SvInvokeInterface(QBAsyncService, self, start, scheduler, errorOut);
}

/**
 * Stop thread pool.
 *
 * This method will ask all worker threads to stop their work and finish,
 * and then wait for all threads to exit.
 *
 * @memberof QBThreadPool
 *
 * @param[in] self      thread pool handle
 * @param[out] errorOut error info
 **/
static inline void
QBThreadPoolStop(QBThreadPool self,
                 SvErrorInfo *errorOut)
{
    SvInvokeInterface(QBAsyncService, self, stop, errorOut);
}

/**
 * Schedule execution of a task.
 *
 * This method enqueues a task to be executed by one of the threads
 * of a thread pool. If @a condition is specified, but not yet completed,
 * execution of a task is suspended until @a condition is met. When
 * @a condition is cancelled, task won't be executed at all.
 *
 * Thread pool creates a future representing the state and result
 * of a scheduled task and guarantees that it'll exist until all
 * notifications are delivered to registered observers.
 * This future will change its state to completed when task execution
 * is finished, its value will be set to the task itself.
 * When @a condition is cancelled, returned future will also be cancelled.
 *
 * Caller can also signal that it is not interested anymore with task's
 * result by cancelling the returned future. It may result in task not
 * being executed at all if it was still waiting in the queue.
 *
 * @memberof QBThreadPool
 *
 * @param[in] self      thread pool handle
 * @param[in] task      handle to an object implementing @ref QBRunnable
 * @param[in] condition optional handle to a future that must be completed
 *                      for @a task to be executed; cancelling @a condition
 *                      will automatically cancel @a task
 * @param[out] errorOut error info
 * @return              a future representing state and result of @a task,
 *                      @c NULL in case of error
 **/
extern QBFuture
QBThreadPoolSchedule(QBThreadPool self,
                     SvObject task,
                     QBFuture condition,
                     SvErrorInfo *errorOut);

/**
 * Statistics collected by the thread pool.
 **/
typedef struct {
    /// number of threads created
    unsigned int threadsCreated;
    /// number of threads joined
    unsigned int threadsJoined;
    /// max number of threads that were running in parallel at any time
    unsigned int maxRunningThreads;
} QBThreadPoolStats;

/**
 * Get statistics from the thread pool.
 *
 * This method returns statistics collected since QBThreadPoolStart().
 *
 * @memberof QBThreadPool
 *
 * @param[in] self      thread pool handle
 * @param[out] stats    statistics filled by this method
 * @param[out] errorOut error info
 **/
extern void
QBThreadPoolGetStats(QBThreadPool self,
                     QBThreadPoolStats *const stats,
                     SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
