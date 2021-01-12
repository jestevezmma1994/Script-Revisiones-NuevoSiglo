/* Copyright (C) 2008  Michal Wojcikowski (deffer44@gmail.com)
 * For conditions of distribution and use, see copyright notice in fiber.h
 */

#ifndef FIBERS_EXTERNAL_LOCKS_H
#define FIBERS_EXTERNAL_LOCKS_H

#include "system_defs.hpp"
#include "log.hpp"
#include "time.hpp"

#include <cstring>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

struct FibersRecursiveLock
{
  pthread_mutex_t  mutex;

  FibersRecursiveLock(void) {
    pthread_mutexattr_t attrs;
    pthread_mutexattr_init(&attrs);
    pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&this->mutex, &attrs);
    pthread_mutexattr_destroy(&attrs);
  };

  ~FibersRecursiveLock(void) {
    pthread_mutex_destroy(&this->mutex);
  };

  void lock(void)   { pthread_mutex_lock(&this->mutex); };
  void unlock(void) { pthread_mutex_unlock(&this->mutex); };
};


struct FibersCond
{
  FibersCond(void)
  {
    m_waiting = 0;
    int fds[2];
    int res = pipe(fds);
    if (res < 0) {
      perror("pipe");
      abort();
    };
    //fibers_warn("MyCond() : opened=(%d,%d)", fds[0], fds[1]);

    fd_rd = fds[0];
    fd_wr = fds[1];
  };

  ~FibersCond(void)
  {
    close(fd_rd);
    close(fd_wr);
    //fibers_warn("MyCond() : closed=(%d,%d)", fd_rd, fd_wr);
  };

  void timed_wait(struct FibersRecursiveLock & lock, int64_t us)
  {
    if (us > 0)
      us += 100; // HACK: since select usually returns a little early.
    m_waiting = 1;
    lock.unlock();
    ///
    fd_set  set;
    FD_ZERO(&set);
    FD_SET(fd_rd, &set);
    struct timeval* tv_ptr = NULL;
    struct timeval tv;
    if (us >= 0) {
      tv.tv_sec = (int)(us / (1000*1000));
      tv.tv_usec = (int)(us % (1000*1000));
      tv_ptr = &tv;
    };
    //FibersTime before = FibersTimeGet();
    errno = 0;
    int res = select(fd_rd+1, &set, NULL, NULL, tv_ptr);
    if (res > 0) {
      // someone wrote a single byte to fd. drain it out.
//       {
//         FibersTime after = FibersTimeGet();
//         int64_t us_diff = to_us(after - before);
//         fibers_warn("MyCond::timed_wait(us=%lld) : select(fd=%d) -> %d (after %lld us)", us, res, fd_rd, us_diff);
//       };
      errno = 0;
      char c;
      res = read(fd_rd, &c, 1);
      if (res <= 0) {
        fibers_error("MyCond::timed_wait(us=%lld) : read(fd=%d, len=1) -> %d [%d, %s]", (long long) us, fd_rd, res, errno, strerror(errno));
      } else {
        //fibers_warn("MyCond::timed_wait(us=%lld) : read(fd=%d, len=1) -> %d", us, fd_rd, res);
      };
    } else if (res == 0) {
#if 0
      FibersTime after = FibersTimeGet();
      int64_t us_diff = to_us(after - before);
      int64_t late_us = us_diff - us;
      if (late_us > 3000 || late_us < -2000) {
        fibers_warn("MyCond::timed_wait(us=%lld) : select(fd=%d) -> timeout (after %lld us) (late=%lld us)", us, fd_rd, us_diff, late_us);
      } else {
        fibers_log("MyCond::timed_wait(us=%lld) : select(fd=%d) -> timeout (after %lld us)", us, fd_rd, us_diff);
      };
#endif
    } else {
      if (errno != EINTR)
        fibers_error("MyCond::timed_wait(us=%lld) : select(fd=%d) -> %d [%d, %s]", (long long) us, fd_rd, res, errno, strerror(errno));
    };
    ///
    lock.lock();
    m_waiting = 0;
  };

  void wait(struct FibersRecursiveLock & lock)
  {
    timed_wait(lock, -1);
  };

  void signal(struct FibersRecursiveLock & lock)
  {
    if (!m_waiting)
      return;

    (void)lock;

    char c = 0;
    write(fd_wr, &c, 1);
    m_waiting = 0; // un-mark immediately to protect against multiple signal() calls for one waiter.
  };

public:
  int fd_rd;
  int fd_wr;

  bool m_waiting;
};


#endif // #ifndef FIBERS_EXTERNAL_LOCKS_H
