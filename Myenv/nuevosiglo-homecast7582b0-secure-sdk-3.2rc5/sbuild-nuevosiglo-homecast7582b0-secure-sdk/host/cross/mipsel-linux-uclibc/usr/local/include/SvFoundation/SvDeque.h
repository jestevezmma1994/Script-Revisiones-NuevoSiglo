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

#ifndef SvFoundation_SvDeque_h
#define SvFoundation_SvDeque_h

/**
 * @file SvDeque.h Double-ended queue class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvDeque Double-ended queue class
 * @ingroup SvFoundationCollections
 * @since 1.11.3
 * @{
 **/

/**
 * Double-ended queue class.
 * @class SvDeque
 * @extends SvObject
 **/
typedef struct SvDeque_ *SvDeque;


/**
 * Get runtime type identification object representing SvDeque class.
 *
 * @relates SvDeque
 *
 * @return double-ended queue class
 **/
extern SvType
SvDeque_getType(void);

/**
 * Create a double-ended queue without reserving initial capacity.
 *
 * @memberof SvDeque
 *
 * @param[out] errorOut error info
 * @return              new queue, @c NULL in case of error
 **/
extern SvDeque
SvDequeCreate(SvErrorInfo *errorOut);

/**
 * Create a double-ended queue with reserved initial capacity.
 *
 * @memberof SvDeque
 *
 * @param[in] capacity  number of objects to reserve space for
 * @param[out] errorOut error info
 * @return              new queue, @c NULL in case of error
 **/
extern SvDeque
SvDequeCreateWithCapacity(size_t capacity,
                          SvErrorInfo *errorOut);

/**
 * Create a double-ended queue with given objects.
 *
 * @memberof SvDeque
 *
 * This method creates an double-ended queue filled with objects
 * retrieved using the @a iterator.
 *
 * @param[in,out] iterator iterator for fetching objects
 * @param[out] errorOut    error info
 * @return                 new queue filled with objects from @a iterator,
 *                         @c NULL in case of error
 **/
extern SvDeque
SvDequeCreateWithValues(SvIterator *iterator,
                        SvErrorInfo *errorOut);

/**
 * Initialize double-ended queue object.
 *
 * @memberof SvDeque
 *
 * @param[in] self         double-ended queue handle
 * @param[in] capacity     number of objects to reserve space for
 * @param[out] errorOut    error info
 * @return                 @a self, @c NULL in case of error
 **/
extern SvDeque
SvDequeInit(SvDeque self,
            size_t capacity,
            SvErrorInfo *errorOut);

/**
 * Get the number of objects in the queue.
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 * @return              number of objects in the queue
 **/
extern size_t
SvDequeGetCount(SvDeque self);

/** @cond */
static inline size_t
SvDequeCount(SvDeque self)
{
    return SvDequeGetCount(self);
}
/** @endcond */

/**
 * Check if queue is empty.
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 * @return              @c true if queue is empty
 **/
static inline bool
SvDequeIsEmpty(SvDeque self)
{
    return SvDequeGetCount(self) == 0;
}

/**
 * Get the capacity of the queue.
 *
 * This method returns the number of objects the queue can hold.
 * Queues grow and shrink dynamically when needed, so this value
 * changes when objects are added or removed.
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 * @return              current capacity of the queue
 **/
extern size_t
SvDequeGetCapacity(SvDeque self);

/**
 * Get object at the given index.
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 * @param[in] idx       index of the object to get
 * @return              handle to the object at @a idx,
 *                      @c NULL if @a idx is invalid
 **/
extern SvObject
SvDequeGetObjectAtIndex(SvDeque self,
                        size_t idx);

/** @cond */
static inline SvObject
SvDequeObjectAtIndex(SvDeque self,
                     size_t idx)
{
    return SvDequeGetObjectAtIndex(self, idx);
}

static inline SvObject
SvDequeAt(SvDeque self,
          size_t idx)
{
    return SvDequeGetObjectAtIndex(self, idx);
}
/** @endcond */

/**
 * Get the first object in the queue.
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 * @return              handle to the first object in the queue,
 *                      @c NULL if queue is empty
 **/
extern SvObject
SvDequeGetFirstObject(SvDeque self);

/** @cond */
static inline SvObject
SvDequeFront(SvDeque self)
{
    return SvDequeGetFirstObject(self);
}
/** @endcond */

/**
 * Get the last object in the queue.
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 * @return              handle to the last object in the queue,
 *                      @c NULL if queue is empty
 **/
extern SvObject
SvDequeGetLastObject(SvDeque self);

/** @cond */
static inline SvObject
SvDequeBack(SvDeque self)
{
    return SvDequeGetLastObject(self);
}
/** @endcond */

/**
 * Change queue capacity.
 *
 * This method reserves the space needed to store at least the given number
 * of objects. The exact capacity is implementation dependent.
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 * @param[in] capacity  requested new queue capacity
 * @param[out] errorOut error info
 **/
extern void
SvDequeResize(SvDeque self,
              size_t capacity,
              SvErrorInfo *errorOut);

/**
 * Remove all objects from the queue.
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 **/
extern void
SvDequeRemoveAllObjects(SvDeque self);

/**
 * Find and remove given object from the queue.
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 * @param[in] obj       object to be removed
 * @return              @a obj if found, @c NULL if not found
 **/
extern SvObject
SvDequeRemove(SvDeque self,
              SvObject obj);

/**
 * Add new object at the front of the queue.
 *
 * This method adds new object at the front of the queue.
 * Request can fail only if it is unable to allocate storage
 * for this new object.
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 * @param[in] obj       new object to be added at the front of the queue
 * @param[out] errorOut error info
 **/
extern void
SvDequePushFront(SvDeque self,
                 SvObject obj,
                 SvErrorInfo *errorOut);

/**
 * Add new object at the end of the queue.
 *
 * This method adds new object at the end of the queue.
 * Request can fail only if it is unable to allocate storage
 * for this new object.
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 * @param[in] obj       new object to be added at the end of the queue
 * @param[out] errorOut error info
 **/
extern void
SvDequePushBack(SvDeque self,
                SvObject obj,
                SvErrorInfo *errorOut);

/**
 * Remove and return the object from the front of the queue.
 *
 * This method removes the first object in the queue (if it exists)
 * and returns it.
 *
 * @memberof SvDeque
 * @qb_allocator
 *
 * @param[in] self      double-ended queue handle
 * @return              handle to an object removed from the front
 *                      of the queue, @c NULL if queue is empty
 **/
extern SvObject
SvDequeTakeFront(SvDeque self);

/**
 * Remove and return the object from the end of the queue.
 *
 * This method removes the last object in the queue (if it exists)
 * and returns it.
 *
 * @memberof SvDeque
 * @qb_allocator
 *
 * @param[in] self      double-ended queue handle
 * @return              handle to an object removed from the end
 *                      of the queue, @c NULL if queue is empty
 **/
extern SvObject
SvDequeTakeBack(SvDeque self);

/**
 * Get an iterator for iterating over a queue in normal order
 * (from the first object to the last.)
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 * @return              the iterator
 **/
extern SvIterator
SvDequeGetIterator(SvDeque self);

/** @cond */
static inline SvIterator
SvDequeIterator(SvDeque self)
{
    return SvDequeGetIterator(self);
}
/** @endcond */

/**
 * Get an iterator for iterating over a queue in reversed order
 * (from the last object to the first.)
 *
 * @memberof SvDeque
 *
 * @param[in] self      double-ended queue handle
 * @return              the reverse iterator
 **/
extern SvIterator
SvDequeGetReverseIterator(SvDeque self);

/** @cond */
static inline SvIterator
SvDequeReverseIterator(SvDeque self)
{
    return SvDequeGetReverseIterator(self);
}
/** @endcond */

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
