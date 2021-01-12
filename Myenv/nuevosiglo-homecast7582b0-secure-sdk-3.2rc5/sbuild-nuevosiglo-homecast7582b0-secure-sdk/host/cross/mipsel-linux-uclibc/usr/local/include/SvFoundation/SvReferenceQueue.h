/*******************************************************************************
 ** Sentivision K.K. Software License Version 1.1
 **
 ** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
 **
 ** Any rights which are not expressly granted in this License are entirely and
 ** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
 ** modify, translate, reverse engineer, decompile, disassemble, or create
 ** derivative works based on this Software. You may not make access to this
 ** Software available to others in connection with a service bureau, application
 ** service provider, or similar business, or make any other use of this Software
 ** without express written permission from Sentivision K.K.

 ** Any User wishing to make use of this Software must contact Sentivision K.K.
 ** to arrange an appropriate license. Use of the Software includes, but is not
 ** limited to:
 ** (1) integrating or incorporating all or part of the code into a product
 **     for sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 *******************************************************************************/

#ifndef SvFoundation_SvReferenceQueue_h
#define SvFoundation_SvReferenceQueue_h

/**
 * @file SvReferenceQueue.h Reference queue class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvWeakReference.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvReferenceQueue Reference queue class
 * @ingroup SvFoundationCoreClasses
 * @{
 **/

/**
 * Get runtime type identification object representing SvReferenceQueue class.
 *
 * @return reference queue class
 **/
extern SvType
SvReferenceQueue_getType(void);

/**
 * Create a reference queue.
 *
 * @memberof SvReferenceQueue
 *
 * @param[out] errorOut error info
 * @return created reference queue, @c NULL in case of error
 **/
extern SvReferenceQueue
SvReferenceQueueCreate(SvErrorInfo *errorOut);

/**
 * Poll the queue to see if an enqueued reference is available.
 *
 * This method checks if an enqueued reference is available; if so,
 * it is immediately removed and returned.
 *
 * @memberof SvReferenceQueue
 * @qb_allocator
 *
 * @param[in] self reference queue handle
 * @return enqueued reference or @c NULL if queue is empty
 **/
extern SvWeakReference
SvReferenceQueuePoll(SvReferenceQueue self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
