/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_SHMEM_QUEUE_H_
#define QB_SHMEM_QUEUE_H_

/**
 * @file QBSHMemQueue.h
 * @brief Shared memory queue class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdint.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBSHMemQueue Shared memory queue class
 * @ingroup QBSHMem
 * @{
 **/

/**
 * Shared memory queue class.
 * @class QBSHMemQueue
 * @extends SvObject
 **/
typedef struct QBSHMemQueue_ *QBSHMemQueue;

/**
 * Open mode (direction).
 **/
typedef enum {
   /// undefined direction used as an error marker
   QBSHMemQueueDirection_none = 0,
   /// queue opened for reading
   QBSHMemQueueDirection_read,
   /// queue opened for writing
   QBSHMemQueueDirection_write
} QBSHMemQueueDirection;


/**
 * Get runtime type identification object representing
 * shared memory queue class.
 *
 * @return shared memory queue class
 **/
extern SvType
QBSHMemQueue_getType(void);

/**
 * Initialize shared memory queue object.
 *
 * @memberof QBSHMemQueue
 *
 * @param[in] self      shared memory queue object handle
 * @param[in] segment   shared memory segment: a handle to an
 *                      object implementing @ref QBMemSegment
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBSHMemQueue
QBSHMemQueueInit(QBSHMemQueue self,
                 SvObject segment,
                 SvErrorInfo *errorOut);

/**
 * Open a shared memory queue for reading or writing.
 *
 * @memberof QBSHMemQueue
 *
 * @param[in] self      shared memory queue object handle
 * @param[in] direction open direction (read or write)
 * @param[out] errorOut error info
 **/
extern void
QBSHMemQueueOpen(QBSHMemQueue self,
                 QBSHMemQueueDirection direction,
                 SvErrorInfo *errorOut);

/**
 * Get open mode of the shared memory queue.
 *
 * @memberof QBSHMemQueue
 *
 * @param[in] self      shared memory queue object handle
 * @return              endpoint direction,
 *                      #QBSHMemQueueDirection_none in case of error
 **/
extern QBSHMemQueueDirection
QBSHMemQueueGetDirection(QBSHMemQueue self);


/**
 * The type of queue message tag.
 **/
typedef uint8_t QBSHMemQueueTag;

/**
 * Prepare next queue message to be sent and return the pointer
 * to the data space of this message.
 *
 * @memberof QBSHMemQueue
 *
 * @param[in] self      shared memory queue object handle
 * @param[in] tag       message tag
 * @param[in] length    message data length in bytes (can be @c 0)
 * @param[out] errorOut error info
 * @return              pointer to the data space, @c NULL on error
 **/
extern void *
QBSHMemQueueGetNextSlot(QBSHMemQueue self,
                        QBSHMemQueueTag tag,
                        size_t length,
                        SvErrorInfo *errorOut);

/**
 * Commit message written to the slot, that have been allocated by
 * QBSHMemQueueGetNextSlot().
 *
 * @memberof QBSHMemQueue
 *
 * @param[in] self      shared memory queue object handle
 **/
extern void
QBSHMemQueueCommitWrite(QBSHMemQueue self);

/**
 * Get the pointer to the data space of the next message in the queue.
 *
 * @memberof QBSHMemQueue
 *
 * @param[in] self      shared memory queue object handle
 * @param[out] tag      message tag (you can pass @c NULL to ignore it)
 * @param[out] length   message data length in bytes (can be @c 0)
 * @param[out] errorOut error info
 * @return              pointer to the data space, @c NULL if the queue
 *                      is empty or in case of error
 **/
extern void *
QBSHMemQueueGetNextMsg(QBSHMemQueue self,
                       QBSHMemQueueTag *tag,
                       size_t *length,
                       SvErrorInfo *errorOut);

/**
 * Confirm that message returned by QBSHMemQueueGetNextMsg()
 * has been read and can be removed from the queue.
 *
 * @memberof QBSHMemQueue
 *
 * @param[in] self      shared memory queue object handle
 **/
extern void
QBSHMemQueueCommitRead(QBSHMemQueue self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
