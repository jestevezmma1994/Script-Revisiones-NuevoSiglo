/* Copyright (C) 2008  Michal Wojcikowski (deffer44@gmail.com)
 * For conditions of distribution and use, see copyright notice in fiber.h
 */

#ifndef FIBERS_EVENT_H
#define FIBERS_EVENT_H

#include "intrusive_list.hpp"

#include "external/system_defs.hpp"

/////////////////////////////////
namespace fibers
/////////////////////////////////
{

  class Scheduler;
  class Fiber;


  // intrusive list tags.
  struct events_in_fiber_tag;


  enum EventState {
    EventState_inactive = 0,
    EventState_armed = 1,
    EventState_active = 2,
  };


  class Event
    : public intrusive::list_node<events_in_fiber_tag>
    //, public boost::noncopyable
  {
    friend class Scheduler;

    /// ------ constructors/destructors. ------
  public:
    explicit Event(Fiber & fiber);
    virtual ~Event(void);
  private:
    Event(const Event & other); // non-copyable.
    Event& operator=(const Event & other); // non-copyable.

    /// ------ member functions - public api. ------
  public:
    bool is_inactive() const
    {  return m_state == EventState_inactive;  }
    INL bool is_active(void) const
    {  return m_state == EventState_active;  };
    INL bool is_armed(void) const
    {  return m_state == EventState_armed;  };
    INL void deactivate(void)
    {  do_deactivate();  };
    void set_name(const char* name);
  protected:
    virtual void do_deactivate(void);

  public:
    virtual void print(int indent) const = 0;

    /// ------ member functions - internal api. ------
  protected:
    void lock(void);  //  {  m_fiber.lock();    };
    void unlock(void);//  {  m_fiber.unlock();  };

    void activate_(void);

    /// ------ members variables. ------
  protected:
    Fiber &  m_fiber;
    EventState  m_state;
    char* m_name; /// may be null
  };


/////////////////////////////////
}; // (namespace fibers)
/////////////////////////////////

#endif // #ifndef FIBERS_EVENT_H
