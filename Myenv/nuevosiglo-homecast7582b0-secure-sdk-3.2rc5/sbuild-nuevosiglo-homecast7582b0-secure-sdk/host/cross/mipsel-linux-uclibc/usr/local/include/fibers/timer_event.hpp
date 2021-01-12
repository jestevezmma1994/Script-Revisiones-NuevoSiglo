/* Copyright (C) 2008  Michal Wojcikowski (deffer44@gmail.com)
 * For conditions of distribution and use, see copyright notice in fiber.h
 */

#ifndef FIBERS_TIMER_EVENT_H
#define FIBERS_TIMER_EVENT_H

#include "intrusive_queue.hpp"
#include "fiber.hpp"

#include "external/time.hpp"
#include "external/system_defs.hpp"

/////////////////////////////////
namespace fibers
/////////////////////////////////
{


  class TimerEvent
    : public Event
    , public intrusive::bin_heap_node_base<TimerEvent*>
  {
    friend class Scheduler;
    typedef Event  base_class;

    /// stuff for bin_heap_node_base.
  public:
    INL static bool less(const TimerEvent* lhs, const TimerEvent* rhs)
    {  return lhs->m_wake_time < rhs->m_wake_time;  };

    /// ------ constructors/destructors. ------
  public:
    explicit TimerEvent(Fiber & fiber);
    virtual ~TimerEvent(void);

    /// ------ member functions. ------
  public:
    void activate_at(FibersTime wake_time);
    void activate_after(FibersTime time_left) { activate_at(FibersTimeGet() + time_left); };

  protected:
    virtual void do_deactivate(void);

  public:
    virtual void print(int indent) const;

    /// ------ members variables. ------
  protected:
    FibersTime  m_wake_time;
  };


/////////////////////////////////
}; // (namespace fibers)
/////////////////////////////////

#endif // #ifndef FIBERS_TIMER_EVENT_H
