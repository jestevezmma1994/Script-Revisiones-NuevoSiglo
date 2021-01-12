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

#ifndef QB_SHMEM_QUEUE_ERROR_H_
#define QB_SHMEM_QUEUE_ERROR_H_

/**
 * @file QBSHMemQueueError.h
 * @brief Shared memory queue class error codes
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @ingroup QBSHMemQueue
 * @{
 **/

/**
 * Shared memory queue error domain.
 **/
#define QBSHMemQueueErrorDomain "com.cubiware.QBSHMemQueue"

/**
 * Shared memory queue error codes.
 **/
typedef enum {
   /** @cond */
   QBSHMemQueueError_OK = 0,
   /** @endcond */
   /** queue structure is malformed */
   QBSHMemQueueError_invalidQueue,
   /** can't open queue in that direction, it is in use */
   QBSHMemQueueError_alreadyOpened,
   /** can't write, message won't fit in queue */
   QBSHMemQueueError_msgTooLong,
   /** can't write, queue is full */
   QBSHMemQueueError_full,
   /** can't read, queue is empty */
   QBSHMemQueueError_empty,
   /** @cond */
   QBSHMemQueueError_last = QBSHMemQueueError_empty
   /** @endcond */
} QBSHMemQueueError;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
