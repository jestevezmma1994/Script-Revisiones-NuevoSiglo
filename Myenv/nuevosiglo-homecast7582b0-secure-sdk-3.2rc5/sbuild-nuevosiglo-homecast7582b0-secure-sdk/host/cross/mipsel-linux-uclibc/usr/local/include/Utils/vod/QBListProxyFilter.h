/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_LIST_PROXY_FILTER_H_
#define QB_LIST_PROXY_FILTER_H_

/**
 * @file QBListProxyFilter.h List filter class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBDataModel3/QBListProxy.h>
#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBListProxyFilter List filter class
 * @ingroup QBDataModel3
 * @{
 *
 * A class providing @ref QBListModel, that wraps @ref QBListProxy and
 * filers it's nodes to contain only a specific node and a search node.
 **/

/**
 * List filter class.
 * @class QBListProxyFilter
 * @extends QBListDataSource
 **/
typedef struct QBListProxyFilter_ *QBListProxyFilter;

/**
 * Get runtime type identification object representing
 * type of list filter class.
 *
 * @return list filter class
 **/
extern SvType
QBListProxyFilter_getType(void);

/**
 * Create new list filter.
 *
 * @memberof QBListProxyFilter
 *
 * @param[in] list          handle to a @ref QBListProxy object that will be filtered
 * @param[in] filterObject  object that is to be present in the filtered list if present in list
 * @param[in] searchType    type of search node that will be present in the filtered list if present in the list
 * @param[out] errorOut     error info
 * @return                  created filter, @c NULL in case of error
 **/
extern QBListProxyFilter
QBListProxyFilterCreate(QBListProxy list,
                        SvObject filterObject,
                        SvType searchType,
                        SvErrorInfo *errorOut);

/**
 * Get node at given index.
 *
 * @memberof QBListProxyFilter
 *
 * @param[in] self      list filter handle
 * @param[in] idx       index of the node to retrieve
 * @param[out] path     path to the node if found (can be @c NULL)
 * @return              handle to a tree node at @a idx, @c NULL if not found
 **/
extern SvObject
QBListProxyFilterGetTreeNode(QBListProxyFilter self,
                             size_t idx,
                             SvObject *path);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
