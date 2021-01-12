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

#ifndef SvFoundation_SvArray_h
#define SvFoundation_SvArray_h

/**
 * @file SvArray.h Array class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <unistd.h> // for ssize_t
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvFoundation/SvComparator.h>
#include <SvFoundation/SvIterator.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvArray Array class
 * @ingroup SvFoundationCollections
 * @{
 **/

/**
 * Array class.
 * @class SvArray
 * @extends SvImmutableArray
 **/
typedef struct SvImmutableArray_ *SvArray;


/**
 * Get runtime type identification object representing SvArray class.
 *
 * @return array class
 **/
extern SvType
SvArray_getType(void);

/**
 * Create a mutable array.
 *
 * @memberof SvArray
 *
 * @param[out] errorOut error info
 * @return new empty array, @c NULL in case of error
 **/
extern SvArray
SvArrayCreate(SvErrorInfo *errorOut);

/**
 * Create a mutable array with given initial capacity.
 *
 * @memberof SvArray
 *
 * @param[in] capacity requested capacity
 * @param[out] errorOut error info
 * @return new empty array of requested capacity, @c NULL in case of error
 **/
extern SvArray
SvArrayCreateWithCapacity(size_t capacity,
                          SvErrorInfo *errorOut);

/**
 * Create a mutable array with object fetched by the @a iterator.
 *
 * @memberof SvArray
 *
 * @param[in,out] iterator iterator for fetching objects
 * @param[out] errorOut error info
 * @return new array filled with objects from @a iterator, @c NULL in case of error
 **/
extern SvArray
SvArrayCreateWithValues(SvIterator *iterator,
                        SvErrorInfo *errorOut);

/**
 * Check if an array contains given object.
 *
 * @note Objects are compared using SvObjectEquals().
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] object compared object.
 * @return @c true if any object in the array is SvObjectEquals()
 *         to the given object, @c false otherwise
 **/
static inline bool
SvArrayContainsObject(SvArray self,
                      SvObject object)
{
    return SvImmutableArrayContainsObject((SvImmutableArray) self, object);
}

/**
 * Get the number of objects in the array.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @return number of objects in the array
 **/
static inline size_t
SvArrayGetCount(SvArray self)
{
    return SvImmutableArrayGetCount((SvImmutableArray) self);
}

/** @cond */
static inline size_t
SvArrayCount(SvArray self)
{
    return SvArrayGetCount(self);
}
/** @endcond */

/**
 * Check if array is empty.
 *
 * @memberof SvArray
 *
 * @param[in] self      array handle
 * @return              @c true if array is empty
 **/
static inline bool
SvArrayIsEmpty(SvArray self)
{
    return SvArrayGetCount(self) == 0;
}

/**
 * Get the capacity of the array.
 *
 * This method returns the number of objects an array can hold
 * without resizing its internal storage.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @return capacity of the array
 **/
extern size_t
SvArrayGetCapacity(SvArray self);

/**
 * Find first object equal to the given object.
 *
 * @note Objects are compared using SvObjectEquals().
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] object compared object
 * @return index of @a object in the array or @c -1 if it is not in the array
 **/
static inline ssize_t
SvArrayFindObject(SvArray self,
                  SvObject object)
{
    return SvImmutableArrayFindObject((SvImmutableArray) self, object);
}

/** @cond */
static inline ssize_t
SvArrayIndexOfObject(SvArray self,
                     SvObject object)
{
    return SvArrayFindObject(self, object);
}
/** @endcond */

/**
 * Find first object equal to the given object.
 *
 * @note Objects are compared using the equality operator (by address).
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] object compared object
 * @return index of @a object in the array or @c -1 if it is not in the array
 **/
static inline ssize_t
SvArrayFindObjectIdenticalTo(SvArray self,
                             SvObject object)
{
    return SvImmutableArrayFindObjectIdenticalTo((SvImmutableArray) self, object);
}

/** @cond */
static inline ssize_t
SvArrayIndexOfObjectIdenticalTo(SvArray self,
                                SvObject object)
{
    return SvArrayFindObjectIdenticalTo(self, object);
}
/** @endcond */

/**
 * Compute the index of first object that is not less than given object.
 *
 * @note Objects are compared using given comparator function.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] object handle to the object to search for
 * @param[in] comparator comparator function
 * @param[in] ptr private data for comparator
 * @return index of the first object not less than given object or @c -1 if all are less
 **/
static inline ssize_t
SvArrayBinarySearch(SvArray self,
                    SvObject object,
                    SvObjectCompareFn comparator,
                    void *ptr)
{
    return SvImmutableArrayBinarySearch((SvImmutableArray) self, object, comparator, ptr);
}

/**
 * Get object at the given index.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] idx index of the object
 * @return handle to an object at the given index or @c NULL if index is invalid
 **/
static inline SvObject
SvArrayGetObjectAtIndex(SvArray self,
                        size_t idx)
{
    return SvImmutableArrayGetObjectAtIndex((SvImmutableArray) self, idx);
}

/** @cond */
static inline SvObject
SvArrayObjectAtIndex(SvArray self,
                     size_t idx)
{
    return SvArrayGetObjectAtIndex(self, idx);
}

static inline SvObject
SvArrayAt(SvArray self,
          size_t idx)
{
    return SvArrayGetObjectAtIndex(self, idx);
}
/** @endcond */

/**
 * Get object at the highest index.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @return an object at the highest index or @c NULL if the array is empty
 **/
static inline SvObject
SvArrayGetLastObject(SvArray self)
{
    return SvImmutableArrayGetLastObject((SvImmutableArray) self);
}

/** @cond */
static inline SvObject
SvArrayLastObject(SvArray self)
{
    return SvArrayGetLastObject(self);
}
/** @endcond */

/**
 * Get object at the lowest index.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @return an object at the lowest index or @c NULL if the array is empty
 **/
static inline SvObject
SvArrayGetFirstObject(SvArray self)
{
    return SvImmutableArrayGetObjectAtIndex((SvImmutableArray) self, 0);
}

/**
 * Add an object to the end of the array.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] object handle to an object to be added to the array
 **/
extern void
SvArrayAddObject(SvArray self,
                 SvObject object);

/**
 * Add multiple objects fetched by the @a iterator to the end of the array.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in,out] iterator iterator for fetching objects
 **/
extern void
SvArrayAddObjects(SvArray self,
                  SvIterator *iterator);

/**
 * Insert an object to a specific position in the array.
 *
 * This method adds new object to the array.
 * The valid ranges of index are from @c 0 to the SvArrayGetCount() inclusive.
 * It is possible to append an element to an array this way.
 *
 * If the position is already used then the array is re-arranged and all
 * objects after that index are moved towards the end of the array. The
 * capacity of the array is adjusted.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] object handle to the object to be added to the array
 * @param[in] idx the position of the object after the insert operation
 **/
extern void
SvArrayInsertObjectAtIndex(SvArray self,
                           SvObject object,
                           size_t idx);

/**
 * Remove all objects from the array, trimming the internal storage.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 **/
extern void
SvArrayRemoveAllObjects(SvArray self);

/**
 * Remove an object at the highest index in the array.
 *
 * This method is a wrapper for SvArrayRemoveObjectAtIndex() with functionality
 * equivalent to 'SvArrayRemoveObjectAtIndex(array, SvArrayGetCount(array) - 1)'.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 **/
extern void
SvArrayRemoveLastObject(SvArray self);

/**
 * Remove the first occurance of an object.
 *
 * @note Objects are compared using SvObjectEquals().
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] obj the object to remove
 * @return @a obj if found, @c NULL if not found
 **/
extern SvObject
SvArrayRemoveObject(SvArray self,
                    SvObject obj);

/**
 * Remove an object at given index.
 *
 * This method removes a single object at given index; if the object is not
 * at the end of the array, all objects at higher indices are shifted towards
 * the begining of the array.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] idx position of the object
 * @return @a idx or @c -1 when @a idx is invalid
 **/
extern ssize_t
SvArrayRemoveObjectAtIndex(SvArray self,
                           size_t idx);

/**
 * Remove the first occurance of an object.
 *
 * @note Objects are compared using the comparison operator (by address.)
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] object compared object
 * @return index of removed object or @c -1 when not found
 **/
extern ssize_t
SvArrayRemoveObjectIdenticalTo(SvArray self,
                               SvObject object);

/**
 * Replace the object at given index with another object.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] idx position of the object
 * @param[in] object replacement object
 * @return @a idx or @c -1 when @a idx is invalid
 **/
extern ssize_t
SvArrayReplaceObjectAtIndexWithObject(SvArray self,
                                      size_t idx,
                                      SvObject object);

/**
 * Exchange the position of two objects in the array.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] indexA position of the first object
 * @param[in] indexB position of the second object
 * @return @c 0 on success or @c -1 when one of indices is invalid
 **/
extern ssize_t
SvArrayExchangeObjectAtIndexWithObjectAtIndex(SvArray self,
                                              size_t indexA,
                                              size_t indexB);

/**
 * Sort array using a comparator object.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] comparator handle to an object implementing @ref SvComparator
 **/
extern void
SvArraySort(SvArray self,
            SvObject comparator);

/**
 * Sort array using a comparing function.
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @param[in] compare function for comparing objects
 * @param[in] prv opaque pointer to @a compare function's private data
 **/
extern void
SvArraySortWithCompareFn(SvArray self,
                         SvObjectCompareFn compare,
                         void *prv);

/**
 * Get an iterator for iterating over an array in the normal order
 * (from the first element, at index @c 0, to the last element.)
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @return the iterator
 **/
static inline SvIterator
SvArrayGetIterator(SvArray self)
{
    return SvImmutableArrayGetIterator((SvImmutableArray) self);
}

/** @cond */
static inline SvIterator
SvArrayIterator(SvArray self)
{
    return SvArrayGetIterator(self);
}
/** @endcond */

/**
 * Get an iterator for iterating over an array in the normal order
 * pointing initially at element at given index.
 *
 * @memberof SvArray
 * @since 1.12
 *
 * @param[in] self array handle
 * @param[in] idx index of the first element to set iterator to
 * @return the iterator
 **/
static inline SvIterator
SvArrayGetIteratorAtIndex(SvArray self,
                          size_t idx)
{
    return SvImmutableArrayGetIteratorAtIndex((SvImmutableArray) self, idx);
}

/** @cond */
static inline SvIterator
SvArrayIteratorAtIndex(SvArray self,
                       size_t idx)
{
    return SvArrayGetIteratorAtIndex(self, idx);
}
/** @endcond */

/**
 * Get an iterator for iterating over an array in the reversed order
 * (from the last element to the first element.)
 *
 * @memberof SvArray
 *
 * @param[in] self array handle
 * @return the reverse iterator
 **/
static inline SvIterator
SvArrayGetReverseIterator(SvArray self)
{
    return SvImmutableArrayGetReverseIterator((SvImmutableArray) self);
}

/** @cond */
static inline SvIterator
SvArrayReverseIterator(SvArray self)
{
    return SvArrayGetReverseIterator(self);
}
/** @endcond */

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
