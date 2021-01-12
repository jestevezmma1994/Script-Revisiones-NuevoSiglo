/* This file does not belong to the "fibers" library.
 * It is a link to external toolchain and may be freely changed or removed.
 */

#ifndef FIBERS_EXTERNAL_TIME_H
#define FIBERS_EXTERNAL_TIME_H

#include "system_defs.hpp"

#include <SvCore/SvTime.h>

typedef SvTime FibersTime;

static INL FibersTime FibersTimeGet(void) {
  return SvTimeGet();
};
static INL FibersTime FibersTimeZero(void) {
  return SvTimeGetZero();
};
static INL FibersTime FibersTimeFromMs(int ms) {
  return SvTimeFromMilliseconds(ms);
};
static INL FibersTime FibersTimeConstruct(int sec, int usec) {
  return SvTimeConstruct(sec, usec);
};
static INL FibersTime FibersTimeFromTimeval(const struct timeval tv) {
  return SvTimeConstruct(tv.tv_sec, tv.tv_usec);
};

static INL FibersTime operator+(const FibersTime & a, const FibersTime & b) {
  return SvTimeAdd(a, b);
};
static INL FibersTime operator-(const FibersTime & a, const FibersTime & b) {
  return SvTimeSub(a, b);
};

static INL int64_t  to_ms(const FibersTime & t) {
  return SvTimeToMicroseconds64(t) / 1000;
};
static INL int64_t  to_us(const FibersTime & t) {
  return SvTimeToMicroseconds64(t);
};

static INL bool  operator<(const FibersTime & a, const FibersTime & b) {
  return to_us(a) < to_us(b);
};
static INL bool  operator>(const FibersTime & a, const FibersTime & b) {
  return to_us(a) > to_us(b);
};
static INL bool  operator<=(const FibersTime & a, const FibersTime & b) {
  return to_us(a) <= to_us(b);
};
static INL bool  operator==(const FibersTime & a, const FibersTime & b) {
  return to_us(a) == to_us(b);
};
static INL bool  operator!=(const FibersTime & a, const FibersTime & b) {
  return to_us(a) != to_us(b);
};

#endif // #ifndef FIBERS_EXTERNAL_TIME_H
