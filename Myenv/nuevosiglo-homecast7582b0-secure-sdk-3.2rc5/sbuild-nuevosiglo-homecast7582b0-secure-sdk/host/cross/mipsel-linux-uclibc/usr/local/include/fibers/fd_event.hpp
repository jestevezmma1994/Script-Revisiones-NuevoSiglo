/* Copyright (C) 2008  Michal Wojcikowski (deffer44@gmail.com)
 * For conditions of distribution and use, see copyright notice in fiber.h
 */

#ifndef FIBERS_FD_EVENT_H
#define FIBERS_FD_EVENT_H

#include "fiber.hpp"

/////////////////////////////////
namespace fibers
/////////////////////////////////
{

  class FdMonitor;
  struct fd_events_in_fd_monitor_tag;


  class FdEvent
    : public Event
    , public intrusive::list_node<fd_events_in_fd_monitor_tag>
  {
    typedef Event  base_class;
    friend class FdMonitor;
    friend class FdEventHT;

    /// ------ constructors/destructors. ------
  public:
    FdEvent(Fiber & fiber, int fd);
    virtual ~FdEvent(void);

    /// ------ member functions. ------
  public:
    void arm(bool writing, bool real_time);
    //void mark_exhausted(void); // mark that reading/writing operation has exhausted fd.

  protected:
    virtual void do_deactivate(void);

  public:
    virtual void print(int indent) const;

    /// ------ members variables. ------
  private:
    /// valid only when event is armed.
    /// depending on how event is armed (rt or not), it might point to a different fdmonitor.
    FdMonitor* m_fd_monitor;

    int  m_fd;
    bool  m_writing;
    //bool  m_exhausted;
  };


/////////////////////////////////
}; // (namespace fibers)
/////////////////////////////////

#endif // #ifndef FIBERS_FD_EVENT_H
