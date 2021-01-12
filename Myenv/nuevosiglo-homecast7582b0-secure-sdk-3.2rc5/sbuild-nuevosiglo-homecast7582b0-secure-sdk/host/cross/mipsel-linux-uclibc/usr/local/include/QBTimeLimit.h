/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TIME_LIMIT_H
#define QB_TIME_LIMIT_H

#if defined QBTimeLimitEnable && SV_LOG_LEVEL > 0

#include <SvCore/SvTime.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvLog.h>

#include <sys/resource.h>
#include <stdbool.h>

/**
* RUSAGE_TYPE_STATS - it comes from Makefile it is set on:
  - RUSAGE_SELF when kernel version < 2.6.26
  - (1 == RUSAGE_THREAD) when kernel version >= 2.6.26
**/

#define QBTimeLimit_log(fmt, ...) SvLogNotice(fmt, ##__VA_ARGS__)
#define QBTimeDefaultTimeThreshold (50 * 000)

#ifdef QBTimeLimitUseParentheses
#  define QBTimeLimitStart_parenthesis  if (1) {
#  define QBTimeLimitEnd_parenthesis  };
#else
#  define QBTimeLimitStart_parenthesis
#  define QBTimeLimitEnd_parenthesis
#endif

#define CATT1(_a_, _b_) _a_##_b_
#define CATT(_a_, _b_)  CATT1(_a_, _b_)

#define QBTimeLimitBegin(_id_, _timeout_) \
  QBTimeLimitStart_parenthesis \
  int CATT(s_time_limit,_id_) = _timeout_; \
  \
  SvTime CATT(startt,_id_) = SvTimeGetZero(); \
  SvTimeRUsage CATT(ru1,_id_); \
  \
  SvTimeGetRUsageTimes(SvTimeRUsageType_thread, &CATT(ru1,_id_)); \
  CATT(startt,_id_) = SvTimeGet();

static const bool QBTimeLimit_force_print = false;

#define QBTimeLimitEnd_(_id_, _name_) \
  if (CATT(s_time_limit,_id_) > 0) { \
    SvTime startt = CATT(startt,_id_); \
    SvTimeRUsage ru1 = CATT(ru1,_id_); \
    SvTimeRUsage ru2; \
    SvTime endt = SvTimeGet(); \
    SvTimeGetRUsageTimes(SvTimeRUsageType_thread, &ru2); \
    int diff_us = SvTimeToUs(SvTimeSub(endt, startt)); \
    if (QBTimeLimit_force_print) \
      QBTimeLimit_log("[%s] : diff_us = %d", _name_, diff_us); \
    if (diff_us > CATT(s_time_limit,_id_)) { \
      int user_diff   = SvTimeToUs(SvTimeSub(ru2.userCPU, ru1.userCPU)); \
      int system_diff = SvTimeToUs(SvTimeSub(ru2.systemCPU, ru1.systemCPU)); \
      int we = user_diff + system_diff; \
      int others = diff_us - we; \
 \
      int color = COLOR_blue(); \
      if (we > CATT(s_time_limit,_id_)) \
        color = COLOR_red(); \
      QBTimeLimit_log(COLBEG() "TimeLimit : %6d us" \
                      "  (user = %6d  system = %6d  others = %6d)  [%s, %d] -> [%s]" COLEND() "\n", color, \
                      diff_us, user_diff, system_diff, others, \
                      __FUNCTION__, __LINE__, _name_); \
    }; \
  }; \
 QBTimeLimitEnd_parenthesis

#define QBTimeLimitEnd(_id_)  QBTimeLimitEnd_(_id_, #_id_)

#else // #if defined QBTimeLimitEnable && SV_LOG_LEVEL > 0

#define QBTimeLimitBegin(_id_, _timeout_)
#define QBTimeLimitEnd_(_id_, _name_)
#define QBTimeLimitEnd(_id_)

#endif // #if defined QBTimeLimitEnable && SV_LOG_LEVEL > 0

#endif // #ifndef QB_TIME_LIMIT_H
