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

#ifndef SV_LOCAL_TIME_H_
#define SV_LOCAL_TIME_H_

#include <time.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct SvLocalTime_s  SvLocalTime;
struct SvLocalTime_s
{
  time_t  utc;
  int  offset;
};

static inline SvLocalTime  SvLocalTimeCreate(time_t utc, int offset)
{
  SvLocalTime t;
  t.utc = utc;
  t.offset = offset;
  return t;
}

static inline int  SvLocalTimeToInt(SvLocalTime t)
{
  return t.utc + t.offset;
}

static inline bool  SvLocalTimeIsValid(SvLocalTime t)
{
  return t.utc > 0;
}

static inline SvLocalTime  SvLocalTimeAdd(SvLocalTime t, int duration)
{
  t.utc += duration;
  return t;
}

static inline int SvLocalTimeDiff(SvLocalTime t1, SvLocalTime t2)
{
  return (t1.utc - t2.utc) + (t1.offset - t2.offset);
}

static inline int  SvLocalTimeCmp(SvLocalTime t1, SvLocalTime t2)
{
  return SvLocalTimeToInt(t1) - SvLocalTimeToInt(t2);
}


extern SvLocalTime SvLocalTimeFromUTC(time_t utc);
extern SvLocalTime SvLocalTimeFromLocal(const struct tm* local);

extern struct tm SvLocalTimeToLocal(SvLocalTime t);


#ifdef __cplusplus
}
#endif

#endif
