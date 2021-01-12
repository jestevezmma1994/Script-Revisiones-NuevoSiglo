/* Copyright (C) 2008  Michal Wojcikowski (deffer44@gmail.com)

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef FIBERS_FIBER_H
#define FIBERS_FIBER_H

#include "intrusive_list.hpp"
#include "intrusive_queue.hpp"
#include "event.hpp"
#include "external/time.hpp"
#include "external/rusage.hpp"

#include <SvCore/SvAllocator.h>

/////////////////////////////////
namespace fibers
/////////////////////////////////
{
  class Scheduler;

  // intrusive list tags.
  struct fibers_in_scheduler_tag {};


  typedef intrusive::list<Event*, events_in_fiber_tag>  event_list_t;


  class Fiber
      : public intrusive::bin_heap_node_base<Fiber*>
      , public intrusive::list_node<fibers_in_scheduler_tag>
      //, public boost::noncopyable
  {
    friend class Scheduler;
    friend class Event;

    /// stuff for bin_heap_node_base.
  public:
    INL static bool less(const Fiber* lhs, const Fiber* rhs)
    {  return (lhs->m_priority < rhs->m_priority) ||
              (lhs->m_priority == rhs->m_priority && (int)(lhs->m_generation - rhs->m_generation) < 0);  };

    /// ------ constructors/destructors. ------
  public:
    Fiber(Scheduler & scheduler, bool counted, const char* name);
  protected:
    virtual ~Fiber(void);
  private:
    Fiber(const Fiber & other); // non-copyable.
    Fiber& operator=(const Fiber & other); // non-copyable.

    /// ------ member functions - public api. ------
  public:
    void activate(int force_reschedule=0); // thread-safe
    void deactivate(void);
    FibersRUsage get_rusage(void) const;
    unsigned long long int get_steps_count(void) const;
    void set_memory_zone_name(const char* zone_name);
    void set_priority(int priority); // thread-safe

    void mark_persistent(bool persistent); // thread-safe

    void destroy(void);

  public:
    //void change_scheduler(Scheduler & scheduler); // not fully designed yet.

    /// ------ member functions - internal api. ------
  public:
    virtual void step_(const FibersTime start_time) = 0;

    INL Scheduler & get_scheduler_(void) const
    {  return *m_scheduler;  };

  protected:
    void lock(void);
    void unlock(void);
    void change_active_events(int diff);

  public:
    virtual void print(int indent) const;

    /// ------ members variables. ------
  public:
    char* const m_name;
  protected:
    Scheduler*  m_scheduler;
    bool  m_counted_in_scheduler;
  protected:
    bool  m_active;
    bool  m_active_wanted;
    int  m_priority;
    // based on scheduler counting each fiber inserted at the end of fiber queue.
    // it's here to ensure that sorting of fibers is stable (even when sorting algorithm is not).
    unsigned int  m_generation;

    // total CPU usage in step function
    FibersRUsage m_total_usage;
    // how many times step function was called
    unsigned long long int m_steps_count;

  private:
    event_list_t  m_event_lst;
    int m_active_event_cnt;
    // this fiber might be woken up from outside its scheduler.
    // used to suspend detection of "dead end" in scheduler.
    bool m_persistent;

  protected:
    // in step, so cannot be freely destroyed
    bool  m_in_step;
    // destroy at the end of step, please - used in destroy() when m_in_step is true.
    bool  m_autodestroy;

  private:
    char* m_memory_zone_name;
  protected:
    SvAllocatorZone  m_memory_zone;
  };


  typedef void (fiber_step_fun) (void* arg);
  Fiber* create_function_fiber(Scheduler & scheduler, const char* name, fiber_step_fun* fun, void* arg);


/////////////////////////////////
}; // (namespace fibers)
/////////////////////////////////

#endif // #ifndef FIBERS_FIBER_H
