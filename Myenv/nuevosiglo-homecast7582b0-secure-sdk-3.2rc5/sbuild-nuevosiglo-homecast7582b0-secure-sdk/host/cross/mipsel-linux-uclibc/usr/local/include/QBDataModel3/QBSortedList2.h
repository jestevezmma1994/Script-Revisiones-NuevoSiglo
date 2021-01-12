/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_SORTED_LIST2_H_
#define QB_SORTED_LIST2_H_

/**
 * @file QBSortedList2.h
 * @brief Sorted list 2 class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvComparator.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>
#include <SvCore/SvErrorInfo.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBSortedList2 Sorted list class
 * @ingroup QBDataModel3
 * @{
 *
 * An array that keeps elements in order, implementing @ref QBSearchableListModel.
 * If inserted object inherits from @ref QBObservable, QBSortedList will automatically
 * check if object is in the right position and move it if it's not on the
 * observedObjectUpdated notification, if object doesn't inherit from QBObservable this
 * behavior may be forced by calling QBSortedList2PropagateObjectsChange.
 **/

/**
 * Sorted list 2 class.
 * @class QBSortedList2
 * @extends QBListDataSource
 * @implements QBSearchableListModel
 * @implements QBObserver
 **/
typedef struct QBSortedList2_ *QBSortedList2;

/**
 * Create sorted list 2.
 *
 * @memberof QBSortedList2
 *
 * @param[in] compare                   compare function
 * @param[in] prv                       callers private data used in compare function
 * @param[out] errorOut                 error info
 * @return                              created sorted list, @c NULL in case of error
 **/
extern QBSortedList2
QBSortedList2CreateWithCompareFn(SvObjectCompareFn compare,
                                 void *prv,
                                 SvErrorInfo *errorOut);

/**
 * Get number of elements.
 *
 * @memberof QBSortedList2
 *
 * @param[in] self                      sorted list handle
 * @return                              number of elements in sorted list
 **/
extern size_t
QBSortedList2Count(QBSortedList2 self);

/**
 * Get object at index.
 *
 * @memberof QBSortedList2
 *
 * @param[in] self                      sorted list handle
 * @param[in] idx                       object index
 * @return                              object at index, @c NULL in case of error
 **/
extern SvObject
QBSortedList2ObjectAtIndex(QBSortedList2 self,
                           size_t idx);

/**
 * Insert object.
 *
 * @memberof QBSortedList2
 *
 * @param[in] self                      sorted list handle
 * @param[in] obj                       object
 **/
extern void
QBSortedList2Insert(QBSortedList2 self,
                    SvObject obj);

/**
 * Insert objects from iterator.
 *
 * @memberof QBSortedList2
 *
 * @param[in] self                      sorted list handle
 * @param[in] iter                      iterator for object for insertion
 **/
extern void
QBSortedList2InsertObjects(QBSortedList2 self,
                           SvIterator iter);

/**
 * Remove object.
 *
 * @memberof QBSortedList2
 *
 * @param[in] self                      sorted list handle
 * @param[in] obj                       object
 **/
extern void
QBSortedList2Remove(QBSortedList2 self,
                    SvObject obj);

/**
 * Remove all objects.
 *
 * @memberof QBSortedList2
 *
 * @param[in] self                      sorted list handle
 **/
extern void
QBSortedList2RemoveAllObjects(QBSortedList2 self);

/**
 * Get iterator.
 *
 * @memberof QBSortedList2
 *
 * @param[in] self                      sorted list handle
 * @return                              in order iterator for sorted list
 **/
extern SvIterator
QBSortedList2Iterator(QBSortedList2 self);

/**
 * Propagate objects change.
 *
 * @memberof QBSortedList2
 *
 * @brief This method will check if object is in the right position
 * and update it if it should be changed. There is no need to call this
 * if object inherits from QBObservable and will notify about it's change
 * correctly.
 *
 * @param[in] self                      sorted list handle
 * @param[in] obj                       object
 **/
extern void
QBSortedList2PropagateObjectsChange(QBSortedList2 self, SvObject obj);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QB_SORTED_LIST2_H_ */
