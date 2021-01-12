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

#ifndef QB_ACTIVE_TREE_NODE_H_
#define QB_ACTIVE_TREE_NODE_H_

/**
 * @file QBActiveTreeNode.h Active tree node class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvIterator.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBActiveTreeNode Active tree node class
 * @ingroup QBDataModel3
 * @{
 *
 * A class representing single node of a tree implemented by the @ref QBActiveTree.
 **/

/**
 * Active tree node class.
 * @class QBActiveTreeNode
 * @extends SvObject
 **/
typedef struct QBActiveTreeNode_ *QBActiveTreeNode;

/**
 * Active tree node class with private data.
 * @class QBActiveTreeExtendedNode
 * @extends QBActiveTreeNode
 **/
typedef struct QBActiveTreeExtendedNode_ * QBActiveTreeExtendedNode;

/**
 * Get runtime type identification object representing
 * type of active tree node class.
 *
 * @return active tree node class
**/
extern SvType
QBActiveTreeNode_getType(void);

/**
 * Get runtime type identification object representing
 * type of active tree extended node class.
 *
 * @return active tree extended node class
**/
extern SvType
QBActiveTreeExtendedNode_getType(void);

/**
 * Initialize active tree node instance.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @param[in] nodeID    node ID or @c NULL
 * @param[in] parent    parent node handle (can be @c NULL)
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBActiveTreeNode
QBActiveTreeNodeInit(QBActiveTreeNode self,
                     SvString nodeID,
                     QBActiveTreeNode parent,
                     SvErrorInfo *errorOut);

/**
 * Create new active tree node instance.
 *
 * @note This is a convenience wrapper that calls SvTypeAllocateInstance()
 *       and QBActiveTreeNodeInit().
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] nodeID    node ID or @c NULL
 * @param[in] parent    parent node handle (can be @c NULL)
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBActiveTreeNode
QBActiveTreeNodeCreate(SvString nodeID,
                       QBActiveTreeNode parent,
                       SvErrorInfo *errorOut);

/**
 * Create new active tree extended node instance.
 *
 * @memberof QBActiveTreeExtendedNode
 *
 * @param[in] data      node private data
 * @param[in] nodeID    node ID or @c NULL
 * @param[in] parent    parent node handle (can be @c NULL)
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBActiveTreeExtendedNode
QBActiveTreeExtendedNodeCreate(SvObject data,
                               SvString nodeID,
                               QBActiveTreeNode parent,
                               SvErrorInfo *errorOut);

/**
 * Get private data associated with this node
 *
 * @memberof QBActiveTreeExtendedNode
 *
 * @param[in] self      active tree extended node handle
 * @return              data passed while creating or initializing node
 **/
extern SvObject
QBActiveTreeExtendedNodeGetData(QBActiveTreeExtendedNode self);

/**
 * Creates active tree path. The path is a string where nodes are represented by IDs and separated by '|' sign.
 * Example of output: "NPvr|npvr-server-status"
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @return              created tree path, @c NULL in case of error
 **/
extern SvString
QBActiveTreeCreateStringPath(QBActiveTreeNode self);

/**
 * Get active tree node ID.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @return              node ID, @c NULL if unavailable
 **/
extern SvString
QBActiveTreeNodeGetID(QBActiveTreeNode self);

/**
 * Get parent node of a active tree node.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @return              parent node handle, @c NULL in case of error
 **/
extern QBActiveTreeNode
QBActiveTreeNodeGetParentNode(QBActiveTreeNode self);

/**
 * Check if active tree node has child nodes.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @return              @c true if node has child nodes, @c false otherwise
 **/
extern bool
QBActiveTreeNodeHasChildNodes(QBActiveTreeNode self);

/**
 * Get number of child nodes.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @return              number of child nodes, @c -1 in case of error
 **/
extern ssize_t
QBActiveTreeNodeGetChildNodesCount(QBActiveTreeNode self);

/**
 * Get index of a child node.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @param[in] childNode child node handle
 * @return              index of @a childNode inside @a self,
 *                      @c -1 if @a childNode not found
 **/
extern ssize_t
QBActiveTreeNodeGetChildNodeIndex(QBActiveTreeNode self,
                                  QBActiveTreeNode childNode);

/**
 * Find child node with given node ID.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @param[in] childID   requested child node ID
 * @return              a handle to a child node with @a childID,
 *                      @c NULL if not found
 **/
extern QBActiveTreeNode
QBActiveTreeNodeFindChildByID(QBActiveTreeNode self,
                              SvString childID);

/**
 * Get child node at given index.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self  active tree node handle
 * @param[in] idx   requested child node index
 * @return          a handle to a child node at @a idx,
 *                  @c NULL if @a idx is invalid
 **/
extern QBActiveTreeNode
QBActiveTreeNodeGetChildByIndex(QBActiveTreeNode self,
                                size_t idx);

/**
 * Find child node.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @param[in] childNode child node handle
 * @return              @a childNode or @c NULL if not found
 **/
extern QBActiveTreeNode
QBActiveTreeNodeFindChild(QBActiveTreeNode self,
                          QBActiveTreeNode childNode);

/**
 * Get iterator over node's child nodes.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @return              iterator over child nodes
 **/
extern SvIterator
QBActiveTreeNodeChildNodesIterator(QBActiveTreeNode self);

/**
 * Get node's attribute value.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @param[in] name      attribute name
 * @return              attribute value, @c NULL if attribute
 *                      with @a name has not been found
 **/
extern SvObject
QBActiveTreeNodeGetAttribute(QBActiveTreeNode self,
                             SvString name);

/**
 * Set node's attribute value.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @param[in] name      attribute name
 * @param[in] value     attribute value, @c NULL to remove attribute
 **/
extern void
QBActiveTreeNodeSetAttribute(QBActiveTreeNode self,
                             SvString name,
                             SvObject value);

/**
 * Get iterator over names of node's attributes.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @return              iterator over names of attributes
 **/
extern SvIterator
QBActiveTreeNodeAttributesIterator(QBActiveTreeNode self);

/**
 * Add child node.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @param[in] childNode new child node handle
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreeNodeAddChildNode(QBActiveTreeNode self,
                             QBActiveTreeNode childNode,
                             SvErrorInfo *errorOut);

/**
 * Add child node registering new sub tree in the active tree.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @param[in] subTreeRoot handle to the root node of the subtree to be added
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreeNodeAddSubTree(QBActiveTreeNode self,
                           QBActiveTreeNode subTreeRoot,
                           SvErrorInfo *errorOut);

/**
 * Add child node at specified position.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @param[in] childNode new child node handle
 * @param[in] idx       position to add child node at
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreeNodeInsertChildNode(QBActiveTreeNode self,
                                QBActiveTreeNode childNode,
                                size_t idx,
                                SvErrorInfo *errorOut);

/**
 * Remove child node of the current node.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @param[in] childNode handle to a child node to remove
 **/
extern void
QBActiveTreeNodeRemoveChildNode(QBActiveTreeNode self,
                                QBActiveTreeNode childNode);

/**
 * Remove child node deregistering it from the active tree.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @param[in] subTreeRoot handle to the root node of the subtree to be removed
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreeNodeRemoveSubTree(QBActiveTreeNode self,
                              QBActiveTreeNode subTreeRoot,
                              SvErrorInfo *errorOut);

/**
 * Remove all child nodes deregistering them from the active tree.
 *
 * @memberof QBActiveTreeNode
 *
 * @param[in] self      active tree node handle
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreeNodeRemoveSubTrees(QBActiveTreeNode self,
                               SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
