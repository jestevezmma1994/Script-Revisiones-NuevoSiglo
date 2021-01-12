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

#ifndef SvFoundation_SvImmutableArray_h
#define SvFoundation_SvImmutableArray_h

/**
 * @file SvImmutableArray.h Immutable array class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdarg.h>
#include <unistd.h> // for ssize_t
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvComparator.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvImmutableArray Immutable array class
 * @ingroup SvFoundationCollections
 * @{
 **/

/**
 * Immutable array class.
 * @class SvImmutableArray
 * @since 1.10
 * @extends SvObject
 **/
typedef struct SvImmutableArray_ *SvImmutableArray;


/**
 * Get runtime type identification object representing SvImmutableArray class.
 *
 * @return immutable array class
 **/
extern SvType
SvImmutableArray_getType(void);

/**
 * Create an immutable array with given objects.
 *
 * @memberof SvImmutableArray
 * @since 1.13.2
 *
 * @param[in] contents array of objects
 * @param[in] count number of objects in @a contents array
 * @param[out] errorOut error info
 * @return new immutable array containing given @a contents,
 *         @c NULL in case of error
 **/
extern SvImmutableArray
SvImmutableArrayCreate(const SvObject *contents,
                       size_t count,
                       SvErrorInfo *errorOut);

/**
 * Create an immutable array with given objects.
 *
 * This method creates an instance of SvImmutableArray filled with objects
 * passed as arguments. Types of arguments are specified as a @a typeSpec
 * string, with each character specifying the type of a single argument,
 * as follows:
 *   - 'i' means 'int' and results in SvValue of ::SvValueType_integer kind
 *     in the array,
 *   - 'd' means 'double' and maps to SvValue of ::SvValueType_double kind,
 *   - '?' means 'bool' and maps to SvValue of ::SvValueType_boolean kind,
 *   - '*' takes no argument and maps to SvValue of ::SvValueType_null kind,
 *   - 's' means 'const char *' and maps to SvString,
 *   - '\@' means any SvObject.
 *
 * Additionally a type specification may be prefixed with 'v' to force
 * using SvValue for this argument. It is especially useful for arguments
 * described with 's', for 'i', 'd', '?' and '*' it is implied, for '\@'
 * it is only allowed for SvString objects.
 * Spaces and horizontal tabulations in the string are ignored.
 *
 * It can be very helpful for:
 *   - writing class serializers,
 *   - constructing lists of parameters for Remote Procedure Calls,
 *   - constructing documents in JSON format.
 * It is also used internally in SvAutoIterator().
 *
 * @memberof SvImmutableArray
 * @since 1.10
 *
 * @param[in] typeSpec a string specifying types of arguments
 * @param[out] errorOut error info
 * @return new immutable array or @c NULL in case of error
 **/
extern SvImmutableArray
SvImmutableArrayCreateWithTypedValues(const char *typeSpec,
                                      SvErrorInfo *errorOut,
                                      ...);

/**
 * Create an immutable array with given objects.
 *
 * This method is another version of SvImmutableArrayCreateWithTypedValues().
 *
 * @memberof SvImmutableArray
 * @since 1.10
 *
 * @param[in] typeSpec a string specifying types of arguments
 * @param[in] args arguments
 * @param[out] errorOut error info
 * @return new immutable array or @c NULL in case of error
 **/
extern SvImmutableArray
SvImmutableArrayCreateWithTypedValuesV(const char *typeSpec,
                                       va_list args,
                                       SvErrorInfo *errorOut);

/**
 * Check if an array contains given object.
 *
 * @note Objects are compared using SvObjectEquals().
 *
 * @memberof SvImmutableArray
 * @since 1.10
 *
 * @param[in] self array handle
 * @param[in] object compared object
 * @return @c true if any object in the array is SvObjectEquals() to the given object, @c false otherwise
 **/
extern bool
SvImmutableArrayContainsObject(SvImmutableArray self,
                               SvObject object);

/**
 * Get the number of objects in the array.
 *
 * @memberof SvImmutableArray
 * @since 1.10
 *
 * @param[in] self array handle
 * @return number of objects in the array
 **/
extern size_t
SvImmutableArrayGetCount(SvImmutableArray self);

/** @cond */
static inline size_t
SvImmutableArrayCount(SvImmutableArray self)
{
    return SvImmutableArrayGetCount(self);
}
/** @endcond */

/**
 * Find first object equal to the given object.
 *
 * @note Objects are compared using SvObjectEquals().
 *
 * @memberof SvImmutableArray
 * @since 1.10
 *
 * @param[in] self array handle
 * @param[in] object the compared object
 * @return index of the object in the array or @c -1 if it is not in the array
 **/
extern ssize_t
SvImmutableArrayFindObject(SvImmutableArray self,
                           SvObject object);

/**
 * Find first object equal to the given object.
 *
 * @note Objects are compared using the equality operator (by address).
 *
 * @memberof SvImmutableArray
 * @since 1.10
 *
 * @param[in] self array handle
 * @param[in] object the compared object
 * @return index of the object in the array or @c -1 if it is not in the array
 **/
extern ssize_t
SvImmutableArrayFindObjectIdenticalTo(SvImmutableArray self,
                                      SvObject object);

/**
 * Compute the index of first object that is not less than given object.
 *
 * @note Objects are compared using given comparator function.
 *
 * @memberof SvImmutableArray
 *
 * @param[in] self array handle
 * @param[in] object handle to an object to be searched for
 * @param[in] comparator comparator function
 * @param[in] ptr private data for comparator
 * @return index of the first object not less than given object or @c -1 if all are less
 **/
extern ssize_t
SvImmutableArrayBinarySearch(SvImmutableArray self,
                             SvObject object,
                             SvObjectCompareFn comparator,
                             void *ptr);

/**
 * Get object at given index.
 *
 * @memberof SvImmutableArray
 * @since 1.10
 *
 * @param[in] self array handle
 * @param[in] idx the index of the object
 * @return an object at the given index or @c NULL if the index is not valid
 **/
extern SvObject
SvImmutableArrayGetObjectAtIndex(SvImmutableArray self,
                                 size_t idx);

/** @cond */
static inline SvObject
SvImmutableArrayObjectAtIndex(SvImmutableArray self,
                              size_t idx)
{
    return SvImmutableArrayGetObjectAtIndex(self, idx);
}
/** @endcond */

/**
 * Get object at the highest index.
 *
 * @memberof SvImmutableArray
 * @since 1.10
 *
 * @param[in] self array handle
 * @return an object at the highest index or @c NULL if the array is empty
 **/
extern SvObject
SvImmutableArrayGetLastObject(SvImmutableArray self);

/**
 * Get an iterator for iterating over an array in the normal order
 * (from the first element, at index @c 0, to the last element.)
 *
 * @memberof SvImmutableArray
 * @since 1.10
 *
 * @param[in] self array handle
 * @return the iterator
 **/
extern SvIterator
SvImmutableArrayGetIterator(SvImmutableArray self);

/** @cond */
static inline SvIterator
SvImmutableArrayIterator(SvImmutableArray self)
{
    return SvImmutableArrayGetIterator(self);
}
/** @endcond */

/**
 * Get an iterator for iterating over an array in the normal order
 * pointing initially at element at given index.
 *
 * @memberof SvImmutableArray
 * @since 1.12
 *
 * @param[in] self array handle
 * @param[in] idx index of the first element to set iterator to
 * @return the iterator
 **/
extern SvIterator
SvImmutableArrayGetIteratorAtIndex(SvImmutableArray self,
                                   size_t idx);

/**
 * Get an iterator for iterating over an array in the reversed order
 * (from the last element to the first element.)
 *
 * @memberof SvImmutableArray
 * @since 1.10
 *
 * @param[in] self array handle
 * @return the reverse iterator
 **/
extern SvIterator
SvImmutableArrayGetReverseIterator(SvImmutableArray self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
