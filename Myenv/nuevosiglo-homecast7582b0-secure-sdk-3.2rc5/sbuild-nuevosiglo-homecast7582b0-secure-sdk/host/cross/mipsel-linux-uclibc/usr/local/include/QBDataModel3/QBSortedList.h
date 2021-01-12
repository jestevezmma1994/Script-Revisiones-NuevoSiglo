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

#ifndef QB_SORTED_LIST_H_
#define QB_SORTED_LIST_H_

/**
 * @file QBSortedList.h Sorted list class
 **/

#include <stddef.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvComparator.h>
#include <SvFoundation/SvIterator.h>
#include <QBDataModel3/QBListModel.h>
#include <QBDataModel3/QBListModelListener.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBSortedList Sorted list class
 * @ingroup QBDataModel3
 * @{
 *
 * An array that keeps elements in order, implementing @ref QBSearchableListModel.
 **/

/**
 * Sorted list class.
 * @class QBSortedList
 * @extends QBListDataSource
 **/
typedef struct QBSortedList_ *QBSortedList;


extern QBSortedList
QBSortedListCreateWithCompareFn(SvObjectCompareFn compare,
                                void *prv,
                                SvErrorInfo *errorOut);

extern size_t
QBSortedListCount(QBSortedList self);

extern SvObject
QBSortedListObjectAtIndex(QBSortedList self,
                          size_t idx);

extern void
QBSortedListInsert(QBSortedList self,
                   SvObject obj);

extern void
QBSortedListInsertObjects(QBSortedList self,
                          SvIterator it);

extern bool
QBSortedListReplaceObject(QBSortedList self,
                          SvObject oldObj,
                          SvObject newObj);

extern void
QBSortedListRemove(QBSortedList self,
                   SvObject obj);

extern void
QBSortedListRemoveAllObjects(QBSortedList self);

extern SvIterator
QBSortedListIterator(QBSortedList self);

extern SvIterator
QBSortedListLowerBound(QBSortedList self,
                       SvObject key);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
