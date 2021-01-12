/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_INPUT_QUEUE_H_
#define QB_INPUT_QUEUE_H_

/**
 * @file QBInputQueue.h Input queue class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>
#include <SvCore/SvAtomic.h>
#include <QBInput/QBInputEvent.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBInputQueue Input queue class
 * @ingroup QBInputCore
 * @{
 **/

/**
 * Input queue class.
 *
 * This class implements simple circular array of input events.
 *
 * @note QBInputQueue is not an SvObject!
 **/
typedef struct QBInputQueue_ {
   /// length of circular array @a slots
   long int slotsCnt;
   /// index of the first used slot
   volatile long int firstUsed;
   /// index of the first free slot
   volatile long int firstFree;
   /// circular array of input events
   QBInputEvent slots[0];
} *QBInputQueue;


/**
 * Allocate input queue.
 *
 * This method allocates new input events queue.
 * It can be safely freed by SvAllocatorDeallocate().
 *
 * @param[in] length queue length
 * @return           newly allocated input events queue,
 *                   @c NULL if not enough memory was available
 **/
extern QBInputQueue QBInputQueueCreate(unsigned int length);

/**
 * Check if queue is empty.
 *
 * @param[in] q      input events queue
 * @return           @c true if @a q is empty, @c false otherwise
 **/
static inline bool QBInputQueueIsEmpty(const QBInputQueue q)
{
   return q->firstUsed == q->firstFree;
}

/**
 * Check if queue is full.
 *
 * @param[in] q      input events queue
 * @return           @c true if @a q is empty, @c false otherwise
 **/
static inline bool QBInputQueueIsFull(const QBInputQueue q)
{
   long int nextFree = (q->firstFree + 1) % q->slotsCnt;
   return q->firstUsed == nextFree;
}

/**
 * Return number of events in the queue.
 *
 * @param[in] q      input events queue
 * @return           number of events in @a q
 **/
static inline unsigned int QBInputQueueGetUsedCount(const QBInputQueue q)
{
   return (unsigned int)(q->firstFree + q->slotsCnt - q->firstUsed) % q->slotsCnt;
}

/**
 * Return number of free slots in the queue.
 *
 * @param[in] q      input events queue
 * @return           number of free slots in @a q
 **/
static inline unsigned int QBInputQueueGetFreeCount(const QBInputQueue q)
{
   return (unsigned int)(q->firstUsed + q->slotsCnt - 1 - q->firstFree) % q->slotsCnt;
}

/**
 * Peek first event from the queue.
 *
 * This method reads first event from the queue @a q.
 * It should not be called simultaneously from different threads!
 *
 * @param[in] q      input events queue
 * @return           first event, invalid if @a q is empty
 **/
static inline QBInputEvent QBInputQueuePeek(QBInputQueue q)
{
   if (QBInputQueueGetUsedCount(q) > 0)
      return q->slots[q->firstUsed];

   return QBInputEventGetInvalid();
}

/**
 * Remove and return first event from the queue.
 *
 * This method reads first event from the queue @a q.
 * It should not be called simultaneously from different threads!
 *
 * @param[in] q      input events queue
 * @return           first event, invalid if @a q is empty
 **/
static inline QBInputEvent QBInputQueueGet(QBInputQueue q)
{
   QBInputEvent ev;
   register long int idx, nextIndex;

   do {
      idx = q->firstUsed;
      if (idx == q->firstFree) {
         // queue is empty
         ev = QBInputEventGetInvalid();
         break;
      } else {
         ev = q->slots[idx];
         if ((nextIndex = idx + 1) == q->slotsCnt)
            nextIndex = 0;
      }
   } while (SvCAS(&(q->firstUsed), idx, nextIndex) != idx);

   return ev;
}

/**
 * Add event to the end of the queue.
 *
 * This method adds event @a ev to the queue @a q. It should
 * not be called simultaneously from different threads!
 *
 * @param[in] q      input events queue
 * @param[in] ev     input event
 * @return           @c true on success, @c false if @a q is full
 **/
static inline bool QBInputQueuePut(QBInputQueue q, QBInputEvent ev)
{
   register long int idx, nextIndex;

   idx = q->firstFree;
   if ((nextIndex = idx + 1) == q->slotsCnt)
      nextIndex = 0;
   if (nextIndex == q->firstUsed) {
      // queue is full
      return false;
   }

   q->slots[idx] = ev;
   q->firstFree = nextIndex;
   return true;
}

/**
 * Move one event from one queue to another.
 *
 * @param[in] dest   input events queue to write to
 * @param[in] src    input events queue to read from
 * @return           @c true on success, @c false if @a src empty or @a dest full
 **/
static inline bool QBInputQueueMoveEvent(QBInputQueue dest, QBInputQueue src)
{
   if (QBInputQueueIsEmpty(src) || QBInputQueueIsFull(dest))
      return false;
   QBInputEvent ev = QBInputQueueGet(src);
   if (ev.type == QBInputEventType_invalid)
      return false;
   return QBInputQueuePut(dest, ev);
}


/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
