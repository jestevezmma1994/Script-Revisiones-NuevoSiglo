/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

/* SMP/SvPlayerKit/Headers/SvPlayerKit/SvTimeRange.h */

#ifndef SV_EPG_TIME_RANGE_H_
#define SV_EPG_TIME_RANGE_H_

/**
 * @file SvTimeRange.h
 * @brief EPG Time Range Class API.
 **/

/**
 * @defgroup SvEPGTimeRange EPG time range class
 * @ingroup SvPlayerKit
 * @{
 **/

#include <time.h>
#include <stdbool.h>


/**
 * EPG time range class.
 **/
typedef struct SvTimeRange_ {
   time_t startTime;
   time_t endTime;
} SvTimeRange;


/**
 * Initialize time range instance.
 *
 * @param[in] self      EPG time range handle
 * @param[in] startTime initial start time
 * @param[in] endTime   initial end time
 * @return              @a self
 **/
static inline SvTimeRange *
SvTimeRangeInit(SvTimeRange *const self,
                time_t startTime,
                time_t endTime)
{
   self->startTime = startTime;
   self->endTime = endTime;
   return self;
}

/**
 * Create time range.
 *
 * @param[in] startTime initial start time
 * @param[in] endTime   initial end time
 * @return              time range
 **/
static inline SvTimeRange
SvTimeRangeConstruct(time_t startTime,
                     time_t endTime)
{
   SvTimeRange range;
   SvTimeRangeInit(&range, startTime, endTime);
   return range;
}

/**
 * Get length (duration) of the time range.
 *
 * @param[in] self      EPG time range handle
 * @return              range length in seconds
 **/
static inline unsigned int
SvTimeRangeSpan(const SvTimeRange *const self)
{
   return (unsigned int)(self->endTime - self->startTime);
}

/**
 * Add other time range to the time range.
 *
 * @param[in] self      EPG time range handle
 * @param[in] startTime start time of the other range
 * @param[in] endTime   end time of the other range
 **/
extern void
SvTimeRangeExtend(SvTimeRange *const self,
                  time_t startTime,
                  time_t endTime);

/**
 * Check if a time ranges overlaps other one.
 *
 * @param[in] self      EPG time range handle
 * @param[in] other     other time range handle
 * @return              true if @a self overlaps the other range
 **/
extern bool
SvTimeRangeOverlaps(const SvTimeRange *const self,
                    const SvTimeRange *const other);

/**
 * Check if a time ranges overlaps other one.
 *
 * @param[in] startTime1  first range start time
 * @param[in] endTime1    first range start time
 * @param[in] startTime2  second range end time
 * @param[in] endTime2    second range end time
 * @return              true if first and second time range overlap
 **/
extern bool
SvTimeRangeOverlapsEx(time_t startTime1, time_t endTime1,
                      time_t startTime2, time_t endTime2);

/**
 * Check if a time range contains in other one.
 *
 * @param[in] self      EPG time range handle
 * @param[in] other     other time range handle
 * @return              true if @a self contains the other range
 **/
static inline bool
SvTimeRangeContains(const SvTimeRange *const self,
                    const SvTimeRange *const other)
{
   return (other->startTime >= self->startTime)
           && (other->endTime <= self->endTime);
}

/**
- * Check if a time range contains time point.
- *
- * @param[in] self      EPG time range handle
- * @param[in] timePoint time point
- * @return              true if timePoint is within the range <startTime, endTime) of @a self
- **/
static inline bool
SvTimeRangeContainsTimePoint(const SvTimeRange *const self, time_t timePoint)
{
    return (timePoint >= self->startTime) && (timePoint < self->endTime);
}

/**
 * Add two time ranges.
 *
 * @param[in,out] self  EPG time range (sum)
 * @param[in] other     EPG time range
 **/
static inline void
SvTimeRangeAdd(SvTimeRange *const self,
               const SvTimeRange *const other)
{
   if (other->startTime < self->startTime)
      self->startTime = other->startTime;
   if (other->endTime > self->endTime)
      self->endTime = other->endTime;
}

/**
 * Subtract a time range from other one.
 *
 * @param[in,out] self  EPG time range (subtraction minuend)
 * @param[in] other     EPG time range (subtraction subtrahend)
 **/
static inline void
SvTimeRangeSubtract(SvTimeRange *const self,
                    const SvTimeRange *const other)
{
   if (self->endTime  <= other->startTime ||
       other->endTime <= self->startTime) {
      return;
   }

   if (other->startTime <= self->startTime) {
      if (other->endTime < self->endTime)
         self->startTime = other->endTime;
      else
         self->endTime = self->startTime; // contained completely by "other"
   } else if (other->startTime < self->endTime) {
      if (other->endTime >= self->endTime) {
         self->endTime = other->startTime;
      } else {
         // "other" is contained completely - nothing to do
      }
   }
}

/**
 * Compute intersection of a time range with other one.
 *
 * @param[in,out] self  first time range, will contain intersection
 *                      value on return
 * @param[in] other     second time range to intersect @a self with
 **/
static inline void
SvTimeRangeIntersect(SvTimeRange *const self,
                     const SvTimeRange *const other)
{
   if (self->startTime <= other->startTime) {
      if (other->startTime < self->endTime) {
         self->startTime = other->startTime;
         if (other->endTime < self->endTime)
            self->endTime = other->endTime;
      } else {
         // intersection is empty
         self->endTime = self->startTime;
      }
   } else { // other->startTime < self->startTime
      if (self->startTime < other->endTime) {
         if (other->endTime < self->endTime)
            self->endTime = other->endTime;
      } else {
         // intersection is empty
         self->endTime = self->startTime;
      }
   }
}

/**
 * @}
 **/


#endif
