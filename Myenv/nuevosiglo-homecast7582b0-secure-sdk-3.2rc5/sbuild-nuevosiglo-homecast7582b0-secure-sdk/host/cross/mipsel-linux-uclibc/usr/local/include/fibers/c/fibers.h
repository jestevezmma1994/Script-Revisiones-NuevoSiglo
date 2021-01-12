/* Copyright (C) 2008  Michal Wojcikowski (deffer44@gmail.com)
 * For conditions of distribution and use, see copyright notice in fiber.h
 */

#ifndef EXTERNC_FIBERS_H
#define EXTERNC_FIBERS_H

#include <pthread.h>
#include <SvCore/SvTime.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @file fibers.h Fibers library API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup Fibers Fibers: cooperative multitasking
 * @ingroup BaseLibraries
 * @{
 *
 * Fibers library provides an implementation of cooperative multitasking environment.
 *
 * Three basic objects are used:
 * - schedulers
 * - fibers
 * - events
 *
 * A scheduler is used to manage tasks in a single thread (like POSIX threads),
 * fibers allow tasks to run and events are used to wake up fibers when specific
 * events happen.
 *
 * Scheduler runs all active fibers in a round robin fashion. Fibers have also
 * a priority assigned and until there exists a active fiber with higher priority
 * fibers with lower priority will not run.
 *
 * When a fiber is run it executes a user-defined function. This function
 * is expected to run in short time (a few miliseconds at most) and exit.
 * This behaviour is not automatically enforced, the task must be nice
 * and behave accordingly (thus 'cooperative'). This means that if a
 * task wants to do much work, it has to be able to split the work into
 * small chunks and process them one-by-one.
 *
 * This is a major difference between regular threads and fibers, usually
 * threads are just functions that do heavy work and the thread exits
 * when the function exits.
 * Fibers require a different approach in which the function is called
 * often, does little work and returns, to be called again soon.
 *
 * In order to ease this task of splitting work a event mechanism
 * is provided. There are a few kinds of events:
 * - a file descriptor (FD) is available to write
 * - a file descriptor is available to read
 * - a specified amount of time has passed
 * A fiber may request to be activated when any of above events occurs.
 *
 * \note For example, when a fiber wants to read from a socket, it should
 * create it in non blocking mode, read as much as it can and
 * then request to be activated when this socket will have more data
 * to read. This way it will not block on reads, but will do reads
 * whenever possible.
 *
 * It is important to understand when a fiber is active, how to change
 * this state and when it is automatically changed.
 * A fiber is always created in inactive state. It may be activated
 * either by manually calling SvFiberActivate() or automatically
 * by an event. When events are activated they also automatically activate
 * the fiber (just as if SvFiberActivate() would be called).
 * The fiber stays active until SvFiberDeactivate() is
 * called and all its events are inactive. In order to stop a fiber
 * from executing its task all its events must be deactivated
 * and the fiber itself must be deactivated.
 * A event is activated when a condition is met and stays active
 * even if the condition is no longer met. In order to deactive it
 * SvFiberEventDeactivate() must be called, there is no other way
 * of deactiving it.
 *
 * It is important to note that a scheduler runs only a single fiber at a time.
 * This means that thread synchronization is not needed between fibers from a scheduler.
 * Of course there may be multiple threads, each with its own scheduler, resulting
 * in multiple fibers running in parallel. Careful design should
 * allow for great reduction in thread synchronization code anyway.
 **/


/**
 * @defgroup SvScheduler Fiber scheduler class
 * @{
 **/

/**
 * Scheduler class.
 *
 * It is used to manage fibers, allowing to create and run them, and
 * providies facilities for events.
 * It runs in a single thread and most of its functions are not thread safe.
 *
 * Runs a single fiber at a time. Scheduler divides all active fibers into
 * round-robin queues based on their priority.
 * Queues are run only if they are not empty and all queues with higher priority
 * are empty.
 *
 * Scheduler keeps a counter modified by SvSchedulerRetain() and
 * SvSchedulerRelease(). If scheduler loops in non-infinite mode
 * then it will not exit until this counter is nonzero. (@see SvSchedulerLoop())
 *
 * Additionally there are two kinds of schedulers to be defined:
 * - main, this is the first created scheduler
 * - looping, this is the scheduler that works in 'current' thread
 *
 * @note This is not an SvObject.
 **/
typedef struct SvScheduler_s*  SvScheduler;

/**
 * Create new fiber scheduler.
 *
 * @return new created fiber scheduler handle
 **/
extern SvScheduler SvSchedulerCreate(void);

/**
 * Destroy fiber scheduler.
 *
 * @param[in] sch fiber scheduler handle
 **/
extern void SvSchedulerDestroy(SvScheduler sch);

/**
 * Prints using SvLog* various statistics about fibers and events.
 * Useful for debugging.
 * @param[in] sch   scheduler handle
 **/
void SvSchedulerPrint(SvScheduler sch);

/**
 * Scheduler will run all active fibers. If infinite is
 * set to false it will exit when there are no active
 * fibers and events, no events are armed and scheduler retain count
 * is 0. Otherwise if infinite is set to true it will simply sleep
 * until an event occurs or a fiber is activated (even
 * from an another thread).
 *
 * @param[in] sch       scheduler handle
 * @param[in] infinite  iff true scheduler will sleep if nothing to do, otherwise it will return
 **/
void SvSchedulerLoop(SvScheduler sch, int infinite);

/**
 * Scheduler will exit its loop as soon as possible
 * (it will wait until current fiber exits its task), even
 * if scheduler was run in infinite mode or fibers are active.
 *
 * @param[in] sch       scheduler handle
 **/
void SvSchedulerBreakLoop(SvScheduler sch);


/**
 * Increases scheduler retain count by one. @see SvSchedulerLoop()
 * @param[in] sch       scheduler handle
 **/
void SvSchedulerRetain(SvScheduler sch);
/**
 * Decreases scheduler retain count by one. @see SvSchedulerLoop()
 * @param[in] sch       scheduler handle
 **/
void SvSchedulerRelease(SvScheduler sch);

/**
 * Fiber scheduler statistics.
 **/
typedef struct SvSchedulerStats_s {
  /// number of currently active fibers
  unsigned int active_fiber_cnt;
  /// number of timer events
  unsigned int timer_cnt;

  /// number of scheduler loops executed
  unsigned long long int loop_cnt;
  /// number of fiber steps executed so far
  unsigned long long int step_cnt;
} SvSchedulerStats;

/**
 * Get fiber scheduler statistics.
 *
 * @param[in] sch fiber scheduler handle
 * @return statistics of the scheduler
 **/
extern SvSchedulerStats SvSchedulerGetStats(SvScheduler sch);

/**
 * Scheduler will automatically call SvSchedulerPrint() periodically.
 * The period is contolled by SvSchedulerPrintFibersLimit env variable.
 * @param[in] sch       scheduler handle
 **/
extern void SvSchedulerEnableLoopPrintFibers(SvScheduler sch);

/**
 * If enabled, prints last few run fibers.
 * It is enabled by setting the SvSchedulerPrintFibers env variable.
 * @param[in] sch       scheduler handle
 **/
extern void SvSchedulerPrintLogFibersQueue(SvScheduler sch);

// ------------ SvScheduler (global) --------------

/**
 * Creates the main scheduler running in main thread.
 **/
void SvSchedulerCreateMain(void);

/**
 * If in current thread a scheduler is looping, then it is returned.
 * Otherwise the main scheduler is returned.
 *
 * @return scheduler handle
 **/
SvScheduler SvSchedulerGet(void); // main/looping scheduler
/**
 * Destroys the main scheduler.
 **/
void SvSchedulerCleanup(void); // main scheduler

/**
 * Tells whether the fiber is no longer allowed to continue execution.
 * Fibers may poll this value in order to cooperatively
 * exit their tasks.
 *
 * Can be called only while a fiber task is executed.
 * @return 1 iff fiber is no longer allowed to continue execution
 **/
int SvFiberTimePassed(void); // looping scheduler
/**
 * Tells for how many more microseconds the fiber may run.
 * @return microseconds left for fiber execution
 **/
int SvFiberTimeLeft(void); // looping scheduler

/**
 * Used to obtain the count of how many fiber steps have been run.
 * @return how many fiber steps were done
 **/
static inline unsigned long long int
SvSchedulerGetStepCnt(void)
{
  SvSchedulerStats stats = SvSchedulerGetStats(SvSchedulerGet());
  return stats.step_cnt;
}

/** @} */


/**
 * @defgroup SvFiberGroup Fiber group class
 * @{
 **/

/**
 * SvFiberGroup class.
 *
 * @note This is not an SvObject.
 **/
typedef struct SvFiberGroup_s* SvFiberGroup;

/**
 * Create new fiber group.
 *
 * @param[in] name      descriptive name for the fiber group
 * @return new created fiber group handle
 **/
extern SvFiberGroup SvFiberGroupCreate(const char* name);

/**
 * Destroy fiber group.
 *
 * @param[in] group     fiber group handle
 **/
extern void SvFiberGroupDestroy(SvFiberGroup group);

/** @} */


/**
 * @defgroup SvFiber Fiber class
 * @{
 **/

/**
 * Fiber step function type.
 *
 * @param[in] arg       opaque pointer to fiber's private data
 **/
typedef void (SvFiberStepFun)(void* arg);

/**
 * SvFiber class.
 *
 * @note This is not an SvObject.
 **/
typedef struct SvFiber_s* SvFiber;

/**
 * Create new fiber.
 *
 * \note User may destroy the fiber while fun is executing
 *
 * @param[in] scheduler handle to a scheduler this fiber will be scheduled by
 * @param[in] group     group to add this fiber to, @c NULL for none
 * @param[in] name      descriptive name for the fiber. It can contain
 *                      only alphanumeric characters, dots, colons and underscores.
 *                      Name can not contain any white spaces. If you have multiple
 *                      fibers in the same module it's good idea to use dots,
 *                      to show that they are connected. For example:
 *                      SvEPGManager.DynamicRangeScavenger,
 *                      SvEPGManager.RequestUpdateStep
 *                      If new name or fiber is generated each time when we create
 *                      fiber (what is not good idea) you have to use
 *                      SvFiberSetMemoryZoneName().
 * @param[in]  fun      fiber step function
 * @param[in]  arg      opaque pointer to fiber's private data
 * @return new created fiber handle
 **/
extern SvFiber SvFiberCreate(SvScheduler scheduler, SvFiberGroup group, const char* name, SvFiberStepFun* fun, void* arg);

/**
 * Destroy fiber. It will also automatically destroy all events for this fiber.
 *
 * @param[in] fiber fiber handle
 **/
extern void SvFiberDestroy(SvFiber fiber);

/**
 * Activate fiber.
 *
 * @note This method is thread-safe.
 *
 * @param[in] fiber     fiber handle
 **/
extern void SvFiberActivate(SvFiber fiber);

/**
 * Activate fiber, but request that current fiber stops as
 * soon as possible.
 *
 * @param[in] fiber     fiber handle
 **/
void SvFiberActivateReschedule(SvFiber fiber); // thread-safe

/**
 * Deactivate fiber.
 *
 * @param[in] fiber     fiber handle
 **/
void SvFiberDeactivate(SvFiber fiber);

/**
 * Get total CPU time usage for fiber step function.
 *
 * @param[in] fiber     fiber handle
 * @return              CPU time usage in the step function
 **/
extern SvTimeRUsage SvFiberGetRUsage(SvFiber fiber);

/**
 * Get number of times step function was called.
 *
 * @param[in] fiber     fiber handle
 * @return              number of fiber steps performed
 **/
extern unsigned long long int SvFiberGetStepsCount(SvFiber fiber);

/**
 * Sets the SvAllocator zone name.
 *
 * Fibers cooperate with SvAllocator zones and by default create zones that
 * are named by default with the fiber's name. This function is used to
 * override the default.
 *
 * @param[in] fiber     fiber handle
 * @param[in] name      new zone name. It can contain only alphanumeric
 *                      characters, dots, colons and underscores. Name
 *                      can not contain any white spaces. If you have few
 *                      fibers in the same module it's good idea to use dots,
 *                      to show that they are connected. For example:
 *                      SvEPGManager.DynamicRangeScavenger,
 *                      SvEPGManager.RequestUpdateStep
 **/
void SvFiberSetMemoryZoneName(SvFiber fiber, const char* name);

/**
 * Sets fiber priority. The higher the more important the fiber is.
 *
 * @param[in] fiber     fiber handle
 * @param[in] priority  new fiber priority
 **/
void SvFiberSetPriority(SvFiber fiber, int priority); // thread-safe

/**
 * If presistent is true then SvSchedulerRetain() will be called on the underlying scheduler.
 * Otherwise SvSchedulerRelease() will be called on it.
 *
 * @param[in] fiber     fiber handle
 * @param[in] persistent iff true scheduler will be retained, otherwise released.
 **/
void SvFiberMarkPersistent(SvFiber fiber, int persistent); // thread-safe

/** @} */


/**
 * @defgroup SvFiberEvent Fiber event class
 * @{
 **/

/**
 * SvFiberEvent class.
 *
 * @note This is not an SvObject.
 **/
typedef struct SvFiberEvent_s SvFiberEvent;

/**
 * Deactive fiber event.
 *
 * @param[in] event fiber event handle
 **/
extern void
SvFiberEventDeactivate(SvFiberEvent* event);

/**
 * Check if fiber event is inactive.
 *
 * @param[in] event fiber event handle
 * @return @c true if event is inactive, @c false otherwise
 **/
extern bool
SvFiberEventIsInactive(SvFiberEvent *event);

/**
 * Check if fiber event is active.
 *
 * @param[in] event fiber event handle
 * @return @c true if event is active, @c false otherwise
 **/
extern bool
SvFiberEventIsActive(SvFiberEvent *event);

/**
 * Check if fiber event is armed.
 *
 * @param[in] event fiber event handle
 * @return @c true if event is armed, @c false otherwise
 **/
extern bool
SvFiberEventIsArmed(SvFiberEvent* event);

/**
 * Set name of a fiber event.
 *
 * @param[in] event fiber event handle
 * @param[in] name new event name
 **/
extern void
SvFiberEventSetName(SvFiberEvent* event, const char* name);

// ------------ SvEvent - base class : internal API --------------
/// @cond

struct _sv_fiber_event_base_ { int _n_; };

extern void _sv_fiber_event_base_destroy_(struct _sv_fiber_event_base_* ev);
extern void _sv_fiber_event_base_deactivate_(struct _sv_fiber_event_base_* ev);
extern int _sv_fiber_event_base_is_inactive_(struct _sv_fiber_event_base_* ev);
extern int _sv_fiber_event_base_is_active_(struct _sv_fiber_event_base_* ev);
extern int _sv_fiber_event_base_is_armed_(struct _sv_fiber_event_base_* ev);
extern void _sv_fiber_event_base_set_name_(struct _sv_fiber_event_base_* ev, const char* name);

/// @endcond

/**
 * Destroys the event.
 * @param[in] ev      event handle
 **/
#define SvFiberEventDestroy(ev)     _sv_fiber_event_base_destroy_( &(ev)->_base_class_ )

/**
 * Deactivates the event.
 * @param[in] ev      event handle
 **/
#define SvFiberEventDeactivate(ev)  _sv_fiber_event_base_deactivate_( &(ev)->_base_class_ )

/**
 * Queries whether the event is active.
 *
 * @param[in] ev      event handle
 * @return true iff event is active
 **/
#define SvFiberEventIsInactive(ev)    _sv_fiber_event_base_is_inactive_( &(ev)->_base_class_ )

/**
 * Queries whether the event is active.
 *
 * @param[in] ev      event handle
 * @return true iff event is active
 **/
#define SvFiberEventIsActive(ev)    _sv_fiber_event_base_is_active_( &(ev)->_base_class_ )

/**
 * Queries whether the event is armed (inactive, but
 * waiting for the condition to be met).
 *
 * @param[in] ev      event handle
 * @return true iff event is armed
 **/
#define SvFiberEventIsArmed(ev)    _sv_fiber_event_base_is_armed_( &(ev)->_base_class_ )

/**
 * Sets the event's name.
 *
 * @param[in] ev      event handle
 * @param[in] name    new event name
 **/
#define SvFiberEventSetName(ev, name)    _sv_fiber_event_base_set_name_( &(ev)->_base_class_, name )



// ------------ SvFiberTimer --------------

/**
 * Fiber timer event class.
 * @extends SvFiberEvent
 * @note This is not an SvObject.
 **/
typedef struct SvFiberTimer_s*  SvFiberTimer;
/// @cond
struct SvFiberTimer_s {
  struct _sv_fiber_event_base_  _base_class_;
};
/// @endcond

/**
 * Creates a new fiber event that will be activated
 * when a timer passes.
 *
 * @param[in] fiber     fiber handle which should be activated when event is activated
 * @return new instance of fiber timer event
 **/
SvFiberTimer SvFiberTimerCreate(SvFiber fiber);

/**
 * Requests that the event is activated when SvTimeGet() is greater or
 * equal to specified value. If the event was armed or active it will
 * ignore the previous request and enter armed state.
 *
 * @param[in] ev        event handle
 * @param[in] wake_time specifies time at which the event should become active.
 **/
void SvFiberTimerActivateAt(SvFiberTimer ev, SvTime wake_time);

/**
 * Requests that the event is activated after requested time passes.
 * If the event was armed or active it will ignore the previous request
 * and enter armed state.
 *
 * @param[in] ev        event handle
 * @param[in] time_left after this time the event will activate
 **/
void SvFiberTimerActivateAfter(SvFiberTimer ev, SvTime time_left);


// ------------ SvFiberFd --------------

/**
 * Fiber file descriptor event class.
 * @extends SvFiberEvent
 * @note This is not an SvObject.
 **/
typedef struct SvFiberFd_s*  SvFiberFd;
/// @cond
struct SvFiberFd_s {
  struct _sv_fiber_event_base_  _base_class_;
};
/// @endcond

/**
 * Creates a new fiber event that will be activated when
 * write xor read from a fd is possible in nonblocking mode.
 *
 * @param[in] fiber     fiber handle which should be activated when event is activated
 * @param[in] fd        file descriptor handle
 * @return new instance of fiber fd event
 **/
SvFiberFd SvFiberFdCreate(SvFiber fiber, int fd);

/**
 * Requests that the event is activated when data can be read xor written
 * to/form fd. If the event was armed or active it will ignore previous
 * request and enter armed state.
 *
 * @param[in] ev        event handle
 * @param[in] writing    iff true then event is awoken when writes are possible
 *                      otherwise it is awoken when reads are possible
 **/
void SvFiberFdArm(SvFiberFd ev, int writing);

/**
 * Similar to SvFiberFdArm(), but the condition is checked much more often.
 * Due to the fact this is more CPU intensive, it is recommended that this
 * function is used rarely.
 *
 * @param[in] ev        event handle
 * @param[in] writing    iff true then event is awoken when writes are possible
 *                      otherwise it is awoken when reads are possible
 **/
void SvFiberFdArmRT(SvFiberFd ev, int writing);

/** @} */


/**
 * @defgroup SvFiberCmd Fiber command class
 * @{
 **/

/**
 * Fiber command class.
 *
 * It is a one-shot fiber, that will execute exactly once (even if destroyed).
 *
 * @note This is not an SvObject.
 **/
typedef struct SvFiberCmd_s* SvFiberCmd;

typedef void (SvFiberCmdFun) (void* arg, int is_done);

/**
 * Creates a new fiber command instance.
 *
 * @param[in] ms        after what time should the function be executed
 * @param[in] arg       opaque handler of data for fun
 * @param[in] fun       command to execure
 * @param[in] priority  priority of the fiber
 * @return fiber command instance
 **/
SvFiberCmd SvFiberCmdCreate(int ms, void* arg, SvFiberCmdFun* fun, int priority);

/**
 * Destroys the fiber command instance, but the command will be run anyway.
 *
 * @param[in] cmd       fiber command handle
 **/
void SvFiberCmdDestroy(SvFiberCmd cmd);

/** @} */


/**
 * @defgroup SvBackgroundScheduler Background scheduler class
 * @{
 **/

/**
 * Background scheduler class.
 *
 * It is a utility class that manages a thread and runs an internal SvScheduler in it.
 * It is used to ease thread management.
 *
 * @note This is not an SvObject.
 **/
typedef struct SvBackgroundScheduler_s* SvBackgroundScheduler;

/**
 * Create new background scheduler.
 *
 * @param[in] name  name of the scheduler
 * @return          created background scheduler, @c NULL in case of error
 **/
SvBackgroundScheduler SvBackgroundSchedulerCreate(const char* name);

/**
 * Create new background scheduler and detach its thread.
 *
 * This is the old variant of SvBackgroundSchedulerCreate(). This function will
 * be removed ASAP, so it should NEVER be used in new code.
 *
 * @param[in] name      name of the scheduler
 * @param[out] out      handle to background scheduler
 * @return              handle to internal scheduler
 **/
SvScheduler SvBackgroundSchedulerCreateDetached(const char* name, SvBackgroundScheduler* out);

/**
 * Getter for internal scheduler.
 *
 * @param[in] bsch      background scheduler handle
 * @return handle to internal scheduler
 **/
SvScheduler SvBackgroundSchedulerGetScheduler(SvBackgroundScheduler bsch);

/**
 * Getter for background scheduler's thread handle.
 *
 * @param[in] bsch      background scheduler handle
 * @return thread handle
 **/
pthread_t SvBackgroundSchedulerGetThread(SvBackgroundScheduler bsch);

/**
 * Destroy background scheduler.
 *
 * This method joins the thread used by background scheduler and destroys it.
 *
 * @param[in] bsch      background scheduler handle
 **/
void SvBackgroundSchedulerDestroy(SvBackgroundScheduler bsch);

/** @} */

/** @} */


#ifdef __cplusplus
}
#endif

#endif // #ifndef EXTERNC_FIBERS_H
