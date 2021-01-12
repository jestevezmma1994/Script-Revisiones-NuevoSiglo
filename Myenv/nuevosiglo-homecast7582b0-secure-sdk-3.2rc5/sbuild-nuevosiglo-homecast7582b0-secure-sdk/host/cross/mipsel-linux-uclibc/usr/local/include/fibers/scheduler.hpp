/* Copyright (C) 2008  Michal Wojcikowski (deffer44@gmail.com)
 * For conditions of distribution and use, see copyright notice in fiber.h
 */

#ifndef FIBERS_SCHEDULER_H
#define FIBERS_SCHEDULER_H

/////////////////////////////////
namespace fibers
/////////////////////////////////
{

    //----------------------
    // Scheduler
    //----------------------

    class Scheduler;


    Scheduler* create_scheduler(void);
    void destroy_scheduler(Scheduler* sch);
    void scheduler_print(const Scheduler* sch);

    void scheduler_loop(Scheduler* sch, bool infinite);
    void scheduler_break_loop(Scheduler* sch);

    void scheduler_change_persistency(Scheduler* sch, int diff);

    int scheduler_time_left_us(const Scheduler* sch);
    bool scheduler_time_passed(const Scheduler* sch);

    struct scheduler_stats
    {
      unsigned int active_fiber_cnt;
      unsigned int timer_cnt;

      unsigned long long int loop_cnt;
      unsigned long long int step_cnt;
    };

    scheduler_stats scheduler_get_stats(const Scheduler* sch);
    void scheduler_enable_profiling(Scheduler* sch);
    void scheduler_enable_loop_print_fibers(Scheduler* sch);

#if SV_LOG_LEVEL > 0
    void scheduler_print_log_fibers_queue(Scheduler *sch);
#else
    static inline void __attribute__ ((unused)) scheduler_print_log_fibers_queue(Scheduler *shc) {}
#endif

/////////////////////////////////
}; // (namespace fibers)
/////////////////////////////////

#endif // #ifndef FIBERS_SCHEDULER_H
