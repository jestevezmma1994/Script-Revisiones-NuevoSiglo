/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_LOG_QUEUE_H_
#define SV_LOG_QUEUE_H_

#include <SvCore/SvTime.h>


#ifdef __cplusplus
extern "C" {
#endif


struct SvLogQueueElement_s {
    char* message;
    SvTime time;
};
typedef struct SvLogQueueElement_s  SvLogQueueElement;


#if SV_LOG_LEVEL > 0

/**
 * Add a log message to queue.
 * @param[in] format        like in printf
 */
extern void
__attribute__ ((format (printf, 1, 2)))
SvLogQueuePush(const char* format, ...);

# else

static inline void
__attribute__ ((unused))
__attribute__ ((format (printf, 1, 2)))
SvLogQueuePush(const char* format, ...) {}

#endif

// Convenience alias
#define SvLogPush  SvLogQueuePush


/**
 * Pop a log message from queue.
 * @param[out]  elem element to fill if it exists
 * @returns     @c true if @a elem was filled (@c false if the log queue was empty)
 */
extern bool SvLogQueuePop(SvLogQueueElement* elem);

/**
 * Multi pop. Set q (queue) parameter.
 * @param[out]  q an array for the messages. First message is the oldest (FIFO order).
 * @return  amount of elements filled in @a q
 */
extern int SvLogQueueMultiPop(SvLogQueueElement* q, int maxCnt);


#ifdef __cplusplus
}
#endif

#endif
