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

#ifndef SvFoundation_SvWeakList_h
#define SvFoundation_SvWeakList_h

/**
 * @file SvWeakList.h Weak references list class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvWeakList Weak references list class
 * @ingroup SvFoundationCollections
 * @since 1.11.12
 * @{
 **/


/**
 * Weak references list class.
 * @class SvWeakList
 * @extends SvObject
 **/
typedef struct SvWeakList_ *SvWeakList;


/**
 * Return the run-time representation of the SvWeakList class type.
 *
 * @return weak references list class
 **/
extern SvType
SvWeakList_getType(void);

/**
 * Create a list.
 *
 * @memberof SvWeakList
 *
 * @param[out] errorOut error info
 * @return              new list, @c NULL in case of error
 **/
extern SvWeakList
SvWeakListCreate(SvErrorInfo *errorOut);

/**
 * Create a list with given objects.
 *
 * This method creates a list of weak references to objects
 * retrieved using the @a iterator.
 *
 * @memberof SvWeakList
 *
 * @param[in,out] iterator iterator for fetching objects
 * @param[out] errorOut    error info
 * @return                 new list filled with weak references to objects
 *                         from @a iterator, @c NULL in case of error
 **/
extern SvWeakList
SvWeakListCreateWithValues(SvIterator *iterator,
                           SvErrorInfo *errorOut);

/**
 * Initialize list object.
 *
 * @memberof SvWeakList
 *
 * @param[in] self         list handle
 * @param[out] errorOut    error info
 * @return                 @a self, @c NULL in case of error
 **/
extern SvWeakList
SvWeakListInit(SvWeakList self,
               SvErrorInfo *errorOut);

/**
 * Get the number of objects in the list.
 *
 * @memberof SvWeakList
 *
 * @param[in] self      list handle
 * @return              number of objects in the list
 **/
extern size_t
SvWeakListGetCount(SvWeakList self);

/** @cond */
static inline size_t
SvWeakListCount(SvWeakList self)
{
    return SvWeakListGetCount(self);
}
/** @endcond */

/**
 * Check if list is empty.
 *
 * @memberof SvWeakList
 *
 * @param[in] self      list handle
 * @return              @c true if list is empty
 **/
static inline bool
SvWeakListIsEmpty(SvWeakList self)
{
    return SvWeakListCount(self) == 0;
}

/**
 * Check if list contains given object.
 *
 * This method checks if @a object is present in the list.
 * Objects are compared using the equality operator (by address), comparing
 * with SvObjectEquals() has a race condition.
 *
 * @memberof SvWeakList
 * @since 1.12.12
 *
 * @param[in] self      list handle
 * @param[in] object    object to search for
 * @return              @c true if list contains @a object, @c false otherwise
 **/
extern bool
SvWeakListContainsObject(SvWeakList self,
                         SvObject object);

/**
 * Create an array with all objects in the list.
 *
 * @memberof SvWeakList
 * @qb_allocator
 *
 * @param[in] self      list handle
 * @param[out] errorOut error info
 * @return array of all objects in the list
 **/
extern SvArray
SvWeakListCreateElementsList(SvWeakList self,
                             SvErrorInfo *errorOut);

/**
 * Remove the first occurence of an object from the list.
 *
 * @memberof SvWeakList
 *
 * @param[in] self      list handle
 * @param[in] object    handle to an object to remove
 * @return              handle to an object removed from the list,
 *                      @c NULL if not found
 **/
extern SvObject
SvWeakListRemoveObject(SvWeakList self,
                       SvObject object);

/**
 * Remove all objects from the list.
 *
 * @memberof SvWeakList
 *
 * @param[in] self      list handle
 **/
extern void
SvWeakListRemoveAllObjects(SvWeakList self);

/**
 * Add new object as the first element of the list.
 *
 * This method adds new object at the front of the list.
 * Request can fail only if it is unable to allocate storage
 * for this new object.
 *
 * @memberof SvWeakList
 *
 * @param[in] self      list handle
 * @param[in] obj       new object to be added at the front of the list
 * @param[out] errorOut error info
 **/
extern void
SvWeakListPushFront(SvWeakList self,
                    SvObject obj,
                    SvErrorInfo *errorOut);

/**
 * Add new object as the last element of the list.
 *
 * This method adds new object at the end of the list.
 * Request can fail only if it is unable to allocate storage
 * for this new object.
 *
 * @memberof SvWeakList
 *
 * @param[in] self      list handle
 * @param[in] obj       new object to be added at the end of the list
 * @param[out] errorOut error info
 **/
extern void
SvWeakListPushBack(SvWeakList self,
                   SvObject obj,
                   SvErrorInfo *errorOut);

/**
 * Remove and return first object from the list.
 *
 * This method removes the first object from the list (if it exists)
 * and returns it.
 *
 * @memberof SvWeakList
 * @qb_allocator
 *
 * @param[in] self      list handle
 * @return              handle to an object removed from the list,
 *                      (must be released by caller), @c NULL if list is empty
 **/
extern SvObject
SvWeakListTakeFirst(SvWeakList self);

/**
 * Remove and return last object from the list.
 *
 * This method removes the last object from the list (if it exists)
 * and returns it.
 *
 * @memberof SvWeakList
 * @qb_allocator
 *
 * @param[in] self      list handle
 * @return              handle to an object removed from the list
 *                      (must be released by caller), @c NULL if list is empty
 **/
extern SvObject
SvWeakListTakeLast(SvWeakList self);

/**
 * Get an iterator for iterating over a list in normal order
 * (from the first object to the last).
 *
 * @deprecated This method is not thread-safe and have been deprecated.
 * Rewrite your code to use SvWeakListCreateElementsList() and iterate
 * over returned array.
 *
 * @memberof SvWeakList
 *
 * @param[in] self      list handle
 * @return              the iterator
 **/
extern SvIterator
SvWeakListGetIterator(SvWeakList self);

/** @cond */
static inline SvIterator
SvWeakListIterator(SvWeakList self)
{
    return SvWeakListGetIterator(self);
}
/** @endcond */

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
