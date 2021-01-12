/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_LIST_DATA_SOURCE_H_
#define QB_LIST_DATA_SOURCE_H_

/**
 * @file QBListDataSource.h Abstract list data source class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBDataModel3/QBDataSource.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBListDataSource Abstract list data source class
 * @ingroup QBDataModel3
 * @{
 *
 * An abstract class that can serve as a base for other
 * classes that implement @ref QBListModel.
 *
 * @image html QBListDataSource.png
 **/

/**
 * List data source class.
 * @class QBListDataSource
 * @extends QBDataSource
 **/
typedef struct QBListDataSource_ {
    /// base class
    struct QBDataSource_ super_;
} *QBListDataSource;


/**
 * Get runtime type identification object representing
 * type of abstract list data source class.
 *
 * @return abstract list data source class
 **/
extern SvType
QBListDataSource_getType(void);

/**
 * Initialize list data source object.
 *
 * @memberof QBListDataSource
 *
 * @param[in] self      list data source handle
 * @param[out] errorOut error info
 * @return              @a self, @c NULL in case of error
 **/
extern QBListDataSource
QBListDataSourceInit(QBListDataSource self,
                     SvErrorInfo *errorOut);

/**
 * Send QBListModelListener::itemsAdded() or QBListModelListener2::itemsAdded()
 * notification to all registered listeners.
 *
 * @memberof QBListDataSource
 *
 * @param[in] self      list data source handle
 * @param[in] first     index of the first added object
 * @param[in] count     number of added objects
 **/
extern void
QBListDataSourceNotifyItemsAdded(QBListDataSource self,
                                 size_t first,
                                 size_t count);

/**
 * Send QBListModelListener::itemsRemoved() or QBListModelListener2::itemsRemoved()
 * notification to all registered listeners.
 *
 * @memberof QBListDataSource
 *
 * @param[in] self      list data source handle
 * @param[in] first     index of the first removed object
 * @param[in] count     number of removed objects
 **/
extern void
QBListDataSourceNotifyItemsRemoved(QBListDataSource self,
                                   size_t first,
                                   size_t count);

/**
 * Send QBListModelListener::itemsChanged() or QBListModelListener2::itemsChanged()
 * notification to all registered listeners.
 *
 * @memberof QBListDataSource
 *
 * @param[in] self      list data source handle
 * @param[in] first     index of the first modified object
 * @param[in] count     number of modified objects
 **/
extern void
QBListDataSourceNotifyItemsChanged(QBListDataSource self,
                                   size_t first,
                                   size_t count);

/**
 * Send QBListModelListener::itemsReordered() or QBListModelListener2::itemsReordered()
 * notification to all registered listeners.
 *
 * @memberof QBListDataSource
 *
 * @param[in] self      list data source handle
 * @param[in] first     index of the first object in the reordered range
 * @param[in] count     number of objects in the reordered range
 **/
extern void
QBListDataSourceNotifyItemsReordered(QBListDataSource self,
                                     size_t first,
                                     size_t count);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
