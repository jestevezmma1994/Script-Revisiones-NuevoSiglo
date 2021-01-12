/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TREE_DATA_SOURCE_H_
#define QB_TREE_DATA_SOURCE_H_

/**
 * @file QBTreeDataSource.h Abstract tree data source class
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
 * @defgroup QBTreeDataSource Abstract tree data source class
 * @ingroup QBDataModel3
 * @{
 *
 * An abstract class that can serve as a base for other
 * classes that implement @ref QBTreeModel.
 *
 * @image html QBTreeDataSource.png
 **/

/**
 * Tree data source class.
 * @class QBTreeDataSource
 * @extends QBDataSource
 **/
typedef struct QBTreeDataSource_ {
    /// base class
    struct QBDataSource_ super_;
} *QBTreeDataSource;


/**
 * Get runtime type identification object representing
 * type of abstract tree data source class.
 *
 * @return abstract tree data source class
 **/
extern SvType
QBTreeDataSource_getType(void);

/**
 * Initialize tree data source object.
 *
 * @memberof QBTreeDataSource
 *
 * @param[in] self      tree data source handle
 * @param[out] errorOut error info
 * @return              @a self, @c NULL in case of error
 **/
extern QBTreeDataSource
QBTreeDataSourceInit(QBTreeDataSource self,
                     SvErrorInfo *errorOut);

/**
 * Send QBTreeModelListener::nodesAdded() notification to all registered listeners.
 *
 * @memberof QBTreeDataSource
 *
 * @param[in] self      tree data source handle
 * @param[in] path      path of the parent node of newly added nodes
 * @param[in] first     index of the first added node
 * @param[in] count     number of added nodes
 **/
extern void
QBTreeDataSourceNotifyNodesAdded(QBTreeDataSource self,
                                 SvObject path,
                                 size_t first,
                                 size_t count);

/**
 * Send QBTreeModelListener::nodesRemoved() notification to all registered listeners.
 *
 * @memberof QBTreeDataSource
 *
 * @param[in] self      tree data source handle
 * @param[in] path      path of the parent node of removed nodes
 * @param[in] first     index of the first removed node
 * @param[in] count     number of removed nodes
 **/
extern void
QBTreeDataSourceNotifyNodesRemoved(QBTreeDataSource self,
                                   SvObject path,
                                   size_t first,
                                   size_t count);

/**
 * Send QBTreeModelListener::nodesChanged() notification to all registered listeners.
 *
 * @memberof QBTreeDataSource
 *
 * @param[in] self      tree data source handle
 * @param[in] path      path of the parent node of changed nodes
 * @param[in] first     index of the first modified node
 * @param[in] count     number of modified nodes
 **/
extern void
QBTreeDataSourceNotifyNodesChanged(QBTreeDataSource self,
                                   SvObject path,
                                   size_t first,
                                   size_t count);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
