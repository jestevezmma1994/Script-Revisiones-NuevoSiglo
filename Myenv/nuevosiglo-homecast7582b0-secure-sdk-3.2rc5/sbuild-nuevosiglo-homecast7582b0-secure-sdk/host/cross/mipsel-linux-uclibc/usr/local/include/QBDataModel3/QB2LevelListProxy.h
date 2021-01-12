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

#ifndef QB_2_LEVEL_LIST_PROXY_H_
#define QB_2_LEVEL_LIST_PROXY_H_

/**
 * @file QB2LevelListProxy.h Two level list proxy class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QB2LevelListProxy List proxy class
 * @ingroup QBDataModel3
 * @{
 *
 * A class providing @ref QBListModel, that wraps a tree model
 * mapping two levels of the tree to a list in the depth first order
 * (first first level node then its children,
 * second first level node then its children
 * and so on).
 **/

/**
 * Two level list proxy class.
 * @class QB2LevelListProxy
 * @extends QBListDataSource
 **/
typedef struct QB2LevelListProxy_ *QB2LevelListProxy;

/**
 * Create new two level list proxy.
 *
 * @memberof QB2LevelListProxy
 *
 * @param[in] tree          handle to an object implementing @ref QBTreeModel
 * @param[in] path          path to the parent node whose child nodes and their child nodes are
 *                          to be presented as a list
 * @param[out] errorOut     error info
 * @return                  created proxy, @c NULL in case of error
 **/
extern QB2LevelListProxy
QB2LevelListProxyCreate(SvObject tree,
                        SvObject path,
                        SvErrorInfo *errorOut);

/**
 * Get proxy output index of first level node with given index.
 *
 * @memberof QB2LevelListProxy
 *
 * @param[in] self          QB2LevelListProxy handle
 * @param[in] idx           first level node index
 * @param[out] errorOut     error info
 * @return                  proxy output index of first level node with given index,
 *                          @c 0 in case of error
 **/
size_t
QB2LevelListProxyGetProxyIndexOfFirstLevelNode(QB2LevelListProxy self,
                                               size_t idx,
                                               SvErrorInfo *errorOut);

/**
 * Get number of first level nodes.
 *
 * @memberof QB2LevelListProxy
 *
 * @param[in]  self         QB2LevelListProxy handle
 * @return                  number of first level nodes
 **/
size_t
QB2LevelListProxyGetFirstLevelNodesCount(QB2LevelListProxy self);

/**
 * Get first level node at given index.
 *
 * @memberof QB2LevelListProxy
 *
 * @param[in]  self         QB2LevelListProxy handle
 * @param[in]  index        desired first level node index
 * @return                  first level node at given index
 **/
SvObject
QB2LevelListProxyGetFirstLevelNode(QB2LevelListProxy self, size_t index);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif /* QB_2_LEVEL_LIST_PROXY_H_ */
