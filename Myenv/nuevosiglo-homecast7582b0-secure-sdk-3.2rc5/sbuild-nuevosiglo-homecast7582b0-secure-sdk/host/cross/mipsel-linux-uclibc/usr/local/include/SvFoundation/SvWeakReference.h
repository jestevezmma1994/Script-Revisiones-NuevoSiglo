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

#ifndef SvFoundation_SvWeakReference_h
#define SvFoundation_SvWeakReference_h

/**
 * @file SvWeakReference.h Weak reference class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Reference queue class.
 * @class SvReferenceQueue
 * @extends SvObject
 * @ingroup SvReferenceQueue
 **/
typedef struct SvReferenceQueue_ *SvReferenceQueue;


/**
 * @defgroup SvWeakReference Weak reference class
 * @ingroup SvFoundationCoreClasses
 * @{
 **/

/**
 * Weak reference class.
 * @class SvWeakReference
 * @extends SvObject
 **/
typedef struct SvWeakReference_ *SvWeakReference;


/**
 * Get runtime type identification object representing SvWeakReference class.
 *
 * @return weak reference class
 **/
extern SvType
SvWeakReference_getType(void);

/**
 * Create new weak reference.
 *
 * @memberof SvWeakReference
 *
 * @param[in] object object to keep weak reference to
 * @param[out] errorOut error info
 * @return created weak reference or @c NULL if not set
 **/
extern SvWeakReference
SvWeakReferenceCreate(SvObject object,
                      SvErrorInfo *errorOut);

/**
 * Create new weak reference.
 *
 * @note This is a wrapper for SvWeakReferenceCreate().
 *
 * @memberof SvWeakReference
 *
 * @param[in] object object to keep weak reference to
 * @param[out] errorOut error info
 * @return created weak reference or @c NULL if not set
 **/
static inline SvWeakReference
SvWeakReferenceCreateWithObject(SvObject object,
                                SvErrorInfo *errorOut)
{
    return SvWeakReferenceCreate(object, errorOut);
}

/**
 * Create new weak reference to be used with a reference queue.
 *
 * @memberof SvWeakReference
 *
 * @param[in] object object to keep weak reference to
 * @param[in] queue handle to a queue to add weak reference to when
 *                  @a object is destroyed
 * @param[in] helper handle to a helper object, can be @c NULL
 * @param[out] errorOut error info
 * @return created weak reference or @c NULL if not set
 **/
extern SvWeakReference
SvWeakReferenceCreateWithObjectAndReferenceQueue(SvObject object,
                                                 SvReferenceQueue queue,
                                                 SvObject helper,
                                                 SvErrorInfo *errorOut);

/**
 * Get the referred object.
 *
 * @memberof SvWeakReference
 * @qb_allocator
 *
 * This method returns @c NULL if the referred object is no longer available
 * or a retained reference to the referred object otherwise.
 *
 * @param[in] self weak reference handle
 * @return handle to the referenced object or @c NULL if no longer available
 **/
extern SvObject
SvWeakReferenceTakeReferredObject(SvWeakReference self);

/**
 * Get the helper object from the weak reference.
 *
 * @memberof SvWeakReference
 *
 * @param[in] self weak reference handle
 * @return handle to the helper object or @c NULL if not set
 **/
extern SvObject
SvWeakReferenceGetHelper(SvWeakReference self);

/**
 * Clear the reference object.
 *
 * @memberof SvWeakReference
 *
 * Invoking this method will make it impossible to access the referred object
 * using this weak reference. The referred object will not be enqueued.
 *
 * If the reference is already cleared nothing happens.
 *
 * @param[in] self weak reference handle
 **/
extern void
SvWeakReferenceClear(SvWeakReference self);

/**
 * Enqueues the reference object to the queue it was created with.
 *
 * @memberof SvWeakReference
 *
 * If the reference object was created with a reference queue then calling
 * this method will enqueue it in that reference queue, unless it has been
 * already enqueued.
 *
 * @param[in] self weak reference handle
 * @return @c true if this method enqueued the reference in the queue
 **/
extern bool
SvWeakReferenceEnqueue(SvWeakReference self);

/**
 * Check if this reference object was enqueued with its associated reference
 * queue, if any.
 *
 * @memberof SvWeakReference
 *
 * @param[in] self weak reference handle
 * @return @c true if the reference was already enqueued
 **/
extern bool
SvWeakReferenceIsEnqueued(SvWeakReference self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
