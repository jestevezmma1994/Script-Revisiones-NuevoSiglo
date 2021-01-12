/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_TIME_H_
#define SV_TIME_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>


#ifdef __cplusplus
extern "C" {
#endif


struct SvTimeStructure {
    int64_t us;
};
typedef struct SvTimeStructure  SvTime;

// SvTimeRUsage types
// meaning of them like in getrusage (see man)
typedef enum {
    SvTimeRUsageType_children = -1,
    SvTimeRUsageType_self,
    SvTimeRUsageType_thread
} SvTimeRUsageType;

typedef struct {
    SvTime userCPU; /* user CPU time used - like in getrusage */
    SvTime systemCPU; /* system CPU time used - like in getrusage */
    SvTimeRUsageType type;
} SvTimeRUsage;

static inline SvTime
SvTimeGetZero(void)
{
    SvTime result;
    result.us = 0;
    return result;
}

static inline SvTime
SvTimeConstruct(int sec, int usec)
{
    SvTime t;
    t.us = (int64_t) sec * 1000 * 1000 + usec;
    return t;
}



#ifdef MODULE

#include <linux/ktime.h>

static inline SvTime
SvTimeGetCurrentTime(void)
{
    struct timespec  temp;
    ktime_get_real_ts(&temp);
    return SvTimeConstruct(temp.tv_sec, temp.tv_nsec / 1000);
}

#else

#include <time.h>
#include <sys/time.h>
#include <unistd.h> // defines _POSIX_TIMERS and _POSIX_MONOTONIC_CLOCK

extern void
SvTimeSetCurrentTimeOffset(int64_t offset);

extern int
SvTimeSetCurrentTime(time_t now);

extern time_t
SvTimeNow(void);

static inline time_t
SvTimeGetUTC(void)
{
    return SvTimeNow();
}

extern SvTime
SvTimeGetCurrentTime(void);

extern SvTime
SvTimeFromBrokenDownTime(const struct tm *t, unsigned int us, bool inLocalTime);

extern int
SvTimeBreakDown(SvTime value, bool inLocalTime, struct tm *res);

extern int
SvTimeGetCurrentTimeZoneOffset(void);

extern void
SvTimeGetRUsageTimes(SvTimeRUsageType type, SvTimeRUsage *times);

extern SvTime
SvTimeGetTotalRUsage(SvTimeRUsageType type);

#endif


static inline SvTime
SvTimeGet(void)
{
    SvTime result;
#if !defined(_POSIX_TIMERS) || _POSIX_TIMERS <= 0
    result = SvTimeGetCurrentTime();
#else
    struct timespec temp;
    clock_gettime(CLOCK_MONOTONIC, &temp);
    result.us = (int64_t) temp.tv_sec * 1000 * 1000 + temp.tv_nsec / 1000;
#endif
    return result;
}

static inline SvTime
SvTimeAdd(SvTime value1, SvTime value2)
{
    SvTime result;
    result.us = value1.us + value2.us;
    return result;
}

static inline SvTime
SvTimeSub(SvTime value1, SvTime value2)
{
    SvTime result;
    result.us = value1.us - value2.us;
    return result;
}

static inline SvTime
SvTimeFromMilliseconds(int64_t ms)
{
    SvTime result;
    result.us = ms * (int64_t)1000;
    return result;
}

#define SvTimeFromMs(val) SvTimeFromMilliseconds((val))

static inline int
SvTimeToMilliseconds(SvTime value)
{
    return (int)(value.us / 1000);
}
#define SvTimeToMs(val) SvTimeToMilliseconds((val))

static inline int64_t
SvTimeToMilliseconds64(SvTime value)
{
    return value.us / 1000;
}
#define SvTimeToMs64(val) SvTimeToMilliseconds64((val))

static inline SvTime
SvTimeFromMicroseconds(int64_t us)
{
    SvTime result;
    result.us = us;
    return result;
}

#define SvTimeFromUs(val) SvTimeFromMicroseconds((val))

static inline SvTime
SvTimeFromTimeval(const struct timeval tv)
{
    return SvTimeConstruct(tv.tv_sec, tv.tv_usec);
}

static inline int64_t
SvTimeToMicroseconds(SvTime value)
{
    return value.us;
}

#define SvTimeToUs(val) SvTimeToMicroseconds((val))

static inline int
SvTimeGetSeconds(SvTime value)
{
    return (int)(value.us / (1000*1000));
}

static inline int
SvTimeGetMicroseconds(SvTime value)
{
    return (int)(value.us % (1000*1000));
}

static inline int64_t
SvTimeToMicroseconds64(SvTime value)
{
    return value.us;
}

#define SvTimeToUs64(val) SvTimeToMicroseconds64((val))


/* a <  b  ->  -1
   a == b  ->   0
   a >  b  ->   1
 */
static inline int
SvTimeCmp(SvTime a, SvTime b)
{
    int64_t us_a = SvTimeToMicroseconds64(a);
    int64_t us_b = SvTimeToMicroseconds64(b);
    if (us_a < us_b)
        return -1;
    if (us_a > us_b)
        return 1;
    return 0;
}

/** (_freq_expr_) <- interval in miliseconds (zero for immediate failure).
    The condition will succeed once per given interval.
 */
#define TIMED_CONDITION( _freq_expr_ ) ({ \
    int success = 0; \
    if ((_freq_expr_)) \
    { \
      int freq_ms = (_freq_expr_); \
      static SvTime last_print; \
      static int has_last_print = 0; \
      if (!has_last_print) { \
        has_last_print = 1; \
        last_print = SvTimeGet(); \
      } else { \
        SvTime now = SvTimeGet(); \
        int diff_ms = SvTimeToMs(SvTimeSub(now, last_print)); \
        if (diff_ms > freq_ms) { \
          last_print = now; \
          success = diff_ms; \
        }; \
      }; \
    }; \
    success; \
  })


#ifdef __cplusplus
}
#endif

#endif
