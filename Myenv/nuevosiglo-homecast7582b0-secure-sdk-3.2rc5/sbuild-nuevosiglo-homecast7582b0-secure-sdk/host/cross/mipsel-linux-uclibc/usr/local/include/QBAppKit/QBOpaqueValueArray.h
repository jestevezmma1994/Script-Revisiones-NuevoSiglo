/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_OPAQUE_VALUE_ARRAY_H_
#define QB_OPAQUE_VALUE_ARRAY_H_

/**
 * @file QBOpaqueValueArray.h Simple lightweight array class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBOpaqueValueArray lightweight array class
 * @ingroup QBAppKit
 * @{
 **/

/**
 * QBOpaqueValueArray class.
 * @class QBOpaqueValueArray
 * @extends SvObject
 **/
typedef struct QBOpaqueValueArray_ *QBOpaqueValueArray;

/**
 * Get runtime type identification object representing QBOpaqueValueArray class.
 *
 * @return array class
 **/
extern SvType
QBOpaqueValueArray_getType(void);

/**
 * Create a simple array.
 *
 * @param[in]  elementSize size of signle element. sizeof(typename *) for pointer types must be used
 * @param[in]  capacity    initial size of the array
 * @param[out] errorOut    error info
 * @return                 created array or @c NULL in case of error
 */
extern QBOpaqueValueArray
QBOpaqueValueArrayCreateWithCapacity(size_t elementSize,
                                     size_t capacity,
                                     SvErrorInfo *errorOut);

/**
 * Insert new element into array.
 *
 * Requires O(1) time when inserting into the end of the array, O(n) otherwise.
 *
 * @param[in] self      array handle
 * @param[in] position  0 based position of the new element. If position is beyound the end
 *                      of array then element is not inserted and error is returned.
 * @param[in] data      pointer to the new element to be inserted
 * @param[out] errorOut error info
 * @return              real position of the inserted element or -1 on error
 */
extern ssize_t
QBOpaqueValueArrayInsertAtIndex(QBOpaqueValueArray self, size_t position, void *data, SvErrorInfo *errorOut);


/**
 * Append new element at the end of the array.
 *
 * Requires O(1) time.
 *
 * @param[in]  self     array handle
 * @param[in]  data     pointer to the new element to be inserted
 * @param[out] errorOut error info
 * @return              real position of the inserted element or -1 on error
 */
extern ssize_t
QBOpaqueValueArrayAppend(QBOpaqueValueArray self, void *data, SvErrorInfo *errorOut);

/**
 * Insert new element at the beginning of the array.
 *
 * Requires O(n) time.
 *
 * @param[in] self      array handle
 * @param[in] data      pointer to the new element to be inserted
 * @param[out] errorOut error info
 * @return              real position of the inserted element or -1 on error
 */
static inline ssize_t
QBOpaqueValueArrayPrepend(QBOpaqueValueArray self, void *data, SvErrorInfo *errorOut)
{
    return QBOpaqueValueArrayInsertAtIndex(self, 0, data, errorOut);
}

/**
 * Remove element from the array.
 *
 * Requires O(1) time when removing from the end of the array, O(n) otherwise.
 *
 * @param[in] self      array handle
 * @param[in] position  position of the element to be removed. If position is beyound the end
 *                      of array then error is reported.
 * @param[out] errorOut error info
 * @return new size of the array or -1 on error
 */
extern ssize_t
QBOpaqueValueArrayRemoveAtIndex(QBOpaqueValueArray self, size_t position, SvErrorInfo *errorOut);

/**
 * Return element from the array.
 *
 * Requires O(1) time.
 *
 * @param[in] self     array handle
 * @param[in] position position of the element. If pos is beyound the end of array
 *                     then @c NULL is returned.
 * @return element at specified position or @c NULL on error.
 */
extern void *
QBOpaqueValueArrayGetObjectAtIndex(QBOpaqueValueArray self, size_t position);

/**
 * Return number of elements in the array.
 *
 * @param[in] self array handle
 * @return number of the elements in the array.
 */
extern size_t
QBOpaqueValueArrayGetCount(QBOpaqueValueArray self);

/**
 * Set resize step.
 *
 * When provided, internal buffer size is increased by given size when more space is required.
 * When not provided, internal buffer size is doubled when more space is required.
 *
 * Setting resize step may save some memory, but can increase complexity.
 *
 * Real additional memory allocated is equal to step * element size.
 *
 * @param[in] self array handle
 * @param[in] step increase step
 */
extern void
QBOpaqueValueArraySetResizeStep(QBOpaqueValueArray self, size_t step);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
