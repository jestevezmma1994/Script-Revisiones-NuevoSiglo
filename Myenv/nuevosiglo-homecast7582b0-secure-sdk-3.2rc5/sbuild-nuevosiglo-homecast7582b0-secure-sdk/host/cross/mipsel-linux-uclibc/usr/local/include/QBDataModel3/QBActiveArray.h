/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_ACTIVE_ARRAY_H_
#define QB_ACTIVE_ARRAY_H_

/**
 * @file QBActiveArray.h Active array class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <unistd.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>
#include <QBDataModel3/QBListDataSource.h>
#include <SvFoundation/SvComparator.h>
#include <QBDataModel3/QBDataSource.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBActiveArray Active array class
 * @ingroup QBDataModel3
 * @{
 *
 * A simple container class implementing list data model.
 *
 * @image html QBActiveArray.png
 **/

/**
 * Active array class.
 * @class QBActiveArray
 * @extends QBListDataSource
 **/
typedef struct QBActiveArray_ {
    /// @internal base class
    struct QBListDataSource_ super_;

    /// @internal number of items in the array
    size_t count;
    /// @internal capacity of @a items array
    size_t capacity;
    /// @internal array of items
    SvObject *items;
} *QBActiveArray;


/**
 * Get runtime type identification object representing
 * type of active array class.
 *
 * @return active array class
 **/
extern SvType
QBActiveArray_getType(void);

/**
 * Initialize active array.
 *
 * @param[in] self          active array handle
 * @param[in] capacity      initial capacity
 * @param[out] errorOut     error info
 * @return                  initialized array, @c NULL in case of error
 **/
extern QBActiveArray
QBActiveArrayInit(QBActiveArray self,
                  size_t capacity,
                  SvErrorInfo *errorOut);

/**
 * Create new active array.
 *
 * @memberof QBActiveArray
 *
 * @param[in] capacity      initial capacity
 * @param[out] errorOut     error info
 * @return                  created array, @c NULL in case of error
 **/
extern QBActiveArray
QBActiveArrayCreate(size_t capacity,
                    SvErrorInfo *errorOut);

/**
 * Create new active array and fill it with values fetched by the @a iterator.
 *
 * @memberof QBActiveArray
 *
 * @param[in,out] iterator  iterator for fetching objects.
 * @param[out] errorOut     error info
 * @return                  created array, @c NULL in case of error
 **/
extern QBActiveArray
QBActiveArrayCreateWithValues(SvIterator *iterator,
                              SvErrorInfo *errorOut);

/**
 * Get number of elements in the array.
 *
 * @memberof QBActiveArray
 *
 * @param[in] self          active array handle
 * @return                  number of elements in the array
 **/
extern size_t
QBActiveArrayCount(QBActiveArray self);

/**
 * Get element at given index.
 *
 * @memberof QBActiveArray
 *
 * @param[in] self          active array handle
 * @param[in] idx           index in the array
 * @return                  handle to an object at @a idx,
 *                          @c NULL if unavailable
 **/
extern SvObject
QBActiveArrayObjectAtIndex(QBActiveArray self,
                           size_t idx);

/**
 * Set object at given index.
 *
 * @memberof QBActiveArray
 *
 * @param[in] self          active array handle
 * @param[in] idx           index in the array
 * @param[in] obj           handle to an object to be inserted at @a idx
 * @param[out] errorOut     error info
 **/
extern void
QBActiveArraySetObjectAtIndex(QBActiveArray self,
                              size_t idx,
                              SvObject obj,
                              SvErrorInfo *errorOut);

/**
 * Append object (add after the last one in the array.)
 *
 * @memberof QBActiveArray
 *
 * @param[in] self          active array handle
 * @param[in] obj           handle to an object to be appended to array
 * @param[out] errorOut     error info
 **/
extern void
QBActiveArrayAddObject(QBActiveArray self,
                       SvObject obj,
                       SvErrorInfo *errorOut);

/**
 * Insert object at given index.
 *
 * This method inserts new object to the array. Unlike
 * QBActiveArraySetObjectAtIndex() this method won't replace
 * object that previously occupied @a idx, but move all objects
 * at indices >= @a idx to make place for new object.
 *
 * @memberof QBActiveArray
 *
 * @param[in] self          active array handle
 * @param[in] idx           requested index of @a obj in the array
 * @param[in] obj           handle to an object to be added to array
 * @param[out] errorOut     error info
 **/
extern void
QBActiveArrayInsertObjectAtIndex(QBActiveArray self,
                                 size_t idx,
                                 SvObject obj,
                                 SvErrorInfo *errorOut);

/**
 * Remove object at given index.
 *
 * This method removes an object from the array.
 * Objects at indices >= @a idx are moved to fill the gap.
 *
 * @memberof QBActiveArray
 *
 * @param[in] self          active array handle
 * @param[in] idx           index of object to be removed
 * @param[out] errorOut     error info
 **/
extern void
QBActiveArrayRemoveObjectAtIndex(QBActiveArray self,
                                 size_t idx,
                                 SvErrorInfo *errorOut);

/**
 * Remove all objects.
 *
 * @memberof QBActiveArray
 *
 * @param[in] self          active array handle
 * @param[out] errorOut     error info
 **/
extern void
QBActiveArrayRemoveAllObjects(QBActiveArray self,
                              SvErrorInfo *errorOut);

/**
 * Move object to another position.
 *
 * @memberof QBActiveArray
 *
 * @param[in] self          active array handle
 * @param[in] srcIdx        original index of the object to be moved
 * @param[in] destIdx       destination index of the object to be moved
 * @param[out] errorOut     error info
 **/
extern void
QBActiveArrayMoveObject(QBActiveArray self,
                        size_t srcIdx, size_t destIdx,
                        SvErrorInfo *errorOut);

/**
 * Propagate QBListModelListener::itemsChanged() notification
 * for given range of objects in the array.
 *
 * @memberof QBActiveArray
 *
 * @param[in] self          active array handle
 * @param[in] first         index of the first object in the range
 * @param[in] count         number of objects in the range
 * @param[out] errorOut     error info
 **/
extern void
QBActiveArrayPropagateObjectsChange(QBActiveArray self,
                                    size_t first, size_t count,
                                    SvErrorInfo *errorOut);

/**
 * Get iterator over entire array.
 *
 * @memberof QBActiveArray
 *
 * @param[in] self          active array handle
 * @return                  iterator over all objects in array
 **/
extern SvIterator
QBActiveArrayIterator(QBActiveArray self);

/**
 * Compute the index of the first object equal to the given object.
 *
 * @note Objects are compared using SvObjectEquals().
 *
 * @memberof QBActiveArray
 *
 * @param[in]  self         active array handle
 * @param[in]  object       compared object
 * @param[out] errorOut     error info
 * @return                  index of the object in the array
 *                          or @c -1 if it is not in the array
 **/
extern ssize_t
QBActiveArrayIndexOfObject(QBActiveArray self,
                           SvObject object,
                           SvErrorInfo *errorOut);

/**
 * Compute the index of the first object equal to the given object.
 *
 * @note Objects are compared using the equality operator (by address.)
 *
 * @memberof QBActiveArray
 *
 * @param[in]  self         active array handle
 * @param[in]  object       compared object
 * @param[out] errorOut     error info
 * @return                  index of the object in the array
 *                          or @c -1 if it is not in the array
 **/
extern ssize_t
QBActiveArrayIndexOfObjectIdenticalTo(QBActiveArray self,
                                      SvObject object,
                                      SvErrorInfo *errorOut);

/**
 * Compute the index of the lower bound element (first element
 * in range [@a minIdx, @a maxIdx) which does not compare less
 * than @a object).
 *
 * @param[in] self          active array handle
 * @param[in] minIdx        index of first element
 * @param[in] maxIdx        index of element after last element
 * @param[in] object        compared object
 * @param[in] compare       function for comparing objects
 * @param[in] prv           pointer to @a compare function's private data
 * @return                  index of first object in the array
 *                          which does not compare less than @a object
 **/
extern size_t
QBActiveArrayLowerBound(QBActiveArray self,
                        size_t minIdx, size_t maxIdx,
                        SvObject object,
                        SvObjectCompareFn compare,
                        void *prv);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
