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

#ifndef QB_LIST_PROXY_H_
#define QB_LIST_PROXY_H_

/**
 * @file QBListProxy.h List proxy class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBListProxy List proxy class
 * @ingroup QBDataModel3
 * @{
 *
 * A class providing @ref QBListModel, that wraps a tree model
 * mapping one level of the tree to a list.
 *
 * @image html QBListProxy.png
 **/

/**
 * List proxy class.
 * @class QBListProxy
 * @extends QBListDataSource
 **/
typedef struct QBListProxy_ *QBListProxy;


/**
 * Get runtime type identification object representing
 * type of list proxy class.
 *
 * @return list proxy class
 **/
extern SvType
QBListProxy_getType(void);

/**
 * Create new list proxy.
 *
 * @memberof QBListProxy
 *
 * @param[in] tree          handle to an object implementing @ref QBTreeModel
 * @param[in] path          path to the parent node whose child nodes are
 *                          to be presented as a list
 * @param[out] errorOut     error info
 * @return                  created proxy, @c NULL in case of error
 **/
extern QBListProxy
QBListProxyCreate(SvObject tree,
                  SvObject path,
                  SvErrorInfo *errorOut);

/**
 * Create new list proxy with static items list.
 *
 * @param[in] tree          handle to an object implementing QBTreeModel
 * @param[in] path          path to the parent node whose child nodes are
 *                          to be presented as a list
 * @param[in] staticItems   handle to an object implementing QBListProxy
 * @param[out] errorOut     error info
 * @return                  created proxy, @c NULL in case of error
 **/
extern QBListProxy
QBListProxyCreateWithStaticItems(SvObject tree,
                                 SvObject path,
                                 SvObject staticItems,
                                 SvErrorInfo *errorOut);

/**
 * Get node at given index.
 *
 * @memberof QBListProxy
 *
 * @param[in] self      list proxy handle
 * @param[in] idx       index of the node to retrieve
 * @param[out] path     path to the node if found (can be @c NULL)
 * @return              handle to a tree node at @a idx, @c NULL if not found
 **/
extern SvObject
QBListProxyGetTreeNode(QBListProxy self,
                       size_t idx,
                       SvObject *path);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
