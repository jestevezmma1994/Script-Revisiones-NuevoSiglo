/* This file does not belong to the "fibers" library.
 * It is a link to external toolchain and may be freely changed or removed.
 */

#ifndef FIBERS_EXTERNAL_LOG_H
#define FIBERS_EXTERNAL_LOG_H

#include <SvCore/SvLog.h>

#define fibers_log(fmt, ...)   SvLogNotice(fmt, ##__VA_ARGS__)
#define fibers_warn(fmt, ...) SvLogWarning(COLBEG() fmt COLEND_COL(yellow), ##__VA_ARGS__)
#define fibers_error(fmt, ...) SvLogError(COLBEG() fmt COLEND_COL(red), ##__VA_ARGS__)

#define lld  long long int

#endif // #ifndef FIBERS_EXTERNAL_LOG_H
