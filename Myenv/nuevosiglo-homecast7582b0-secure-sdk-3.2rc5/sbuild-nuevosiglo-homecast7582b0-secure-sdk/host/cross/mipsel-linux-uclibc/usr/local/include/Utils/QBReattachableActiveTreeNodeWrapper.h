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

#ifndef QB_REATTACHABLE_ACTIVE_TREE_NODE_WRAPPER_H_
#define QB_REATTACHABLE_ACTIVE_TREE_NODE_WRAPPER_H_

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBActiveTreeNode.h>

/**
 * @defgroup QBReattachableActiveTreeNodeWrapper QBReattachableActiveTreeNodeWrapper: Re-attachable QBActiveTreeNode wrapper.
 * @ingroup CubiTV_utils
 * @{
 *
 * Wrapper of QBActiveTreeNode in a tree that makes it easy to remove and re-add node.
 * Use it with caution, one tree level nodes should be handled by one controller,
 * it uses original indexes to insert nodes, so it's forbidden to insert random node when
 * some node is removed. This situation should be handled first by readding all nodes in
 * ascending order, and then removing unnecessary nodes.
 **/

/**
 * Reattachable QBActiveTreeNode wrapper.
 * @class QBReattachableActiveTreeNodeWrapper
 * @extends SvObject
 **/
typedef struct QBReattachableActiveTreeNodeWrapper_ * QBReattachableActiveTreeNodeWrapper;

/**
 * Create re-attachable QBActiveTreeNode wrapper.
 *
 * @memberof QBReattachableActiveTreeNodeWrapper
 *
 * This is a method that allocates and initializes QBReattachableActiveTreeNodeWrapper.
 *
 * @param[in] tree              tree handle
 * @param[in] nodeId            node id
 * @param[out] *errorOut        error info
 * @return                      created controller, @c NULL in case of error
 **/
QBReattachableActiveTreeNodeWrapper
QBReattachableActiveTreeNodeWrapperCreate(QBActiveTree tree,
                                          SvString nodeId,
                                          SvErrorInfo *errorOut);

/**
 * Remove reattachable active tree node wrapper from tree to which it belongs.
 *
 * @memberof QBReattachableActiveTreeNodeWrapper
 *
 * @param[in] self              reattachable active tree node wrapper handle
 **/
void
QBReattachableActiveTreeNodeWrapperRemove(QBReattachableActiveTreeNodeWrapper self);

/**
 * Insert reattachable active tree node wrapper as a child of
 * parent node of which it was child when it was created.
 *
 * @memberof QBReattachableActiveTreeNodeWrapper
 *
 * @param[in] self              reattachable active tree node wrapper handle
 **/
void
QBReattachableActiveTreeNodeWrapperInsert(QBReattachableActiveTreeNodeWrapper self);

/**
 * @}
 **/

#endif /* QB_REATTACHABLE_ACTIVE_TREE_NODE_WRAPPER_H_ */
