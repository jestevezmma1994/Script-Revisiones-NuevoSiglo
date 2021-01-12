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
** Cubiware Sp. z o.o. to ararray an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_CONTENT_ARRAY_H_
#define QB_CONTENT_ARRAY_H_

/**
 * @file QBContentArray.h
 * @brief Content array class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>

/**
 * @defgroup QBContentArray content array class
 * @ingroup QBContentManagerCore
 * @{
 **/

/**
 * Content array class.
 * @class QBContentArray QBContentArray.h <QBContentManager/QBContentArray.h>
 * @extends QBDataSource
 * @implements QBSearchableListModel
 **/
typedef struct QBContentArray_ *QBContentArray;

/**
 * Create content array.
 *
 * @memberof QBContentArray
 *
 * @return                              QBContentArray, @c NULL in case of error
 */
extern QBContentArray QBContentArrayCreate(void);

/**
 * Set items and set length to number of objects passed.
 *
 * @memberof QBContentArray
 *
 * This method will set as array content values that are passed.
 * If content array was already filled, previous content will be released
 * and swapped to the new content.
 *
 * @param[in] self                    QBContentArray handle
 * @param[in] objects                 objects
 **/
extern void QBContentArraySetItems(QBContentArray self, SvArray objects);

/**
 * Clear content array.
 *
 * @memberof QBContentArray
 *
 * @param[in] self                    QBContentArray handle
 **/
extern void QBContentArrayClear(QBContentArray self);

/**
 * Set object at index to given object and notify appropriately.
 *
 * @memberof QBContentArray
 *
 * If index is above current length this method also sets length so that the
 * array will fit the new object.
 *
 * @param[in] self                    QBContentArray handle
 * @param[in] index                   index
 * @param[in] object                  object
 **/
extern void QBContentArrayItemChanged(QBContentArray self, size_t index, SvObject object);

/**
 * Merge items.
 *
 * @memberof QBContentArray
 *
 * Exchange objects starting from the given index in case they are not equal or did change flag is
 * set for this object.
 *
 * @param[in] self                    QBContentArray handle
 * @param[in] didChange               flag specifying if object at given index should be exchanged
 *                                    by the new object from the objects in case it is is equal
 * @param[in] first                   first
 * @param[in] objects                 objects
 **/
extern void QBContentArrayMergeItems(QBContentArray self, bool *didChange, size_t first, SvArray objects);

/**
 * Set content array length.
 *
 * @memberof QBContentArray
 *
 * @param[in] self                    QBContentArray handle
 * @param[in] length                  length
 **/
extern void QBContentArraySetLength(QBContentArray self, size_t length);

/**
 * @}
 **/

#endif
