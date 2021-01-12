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

#ifndef QB_ACTIVE_TREE_H_
#define QB_ACTIVE_TREE_H_

/**
 * @file QBActiveTree.h Active tree class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvImmutableArray.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreeIterator.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBActiveTree Active tree class
 * @ingroup QBDataModel3
 * @{
 *
 * A simple container class implementing tree data model.
 *
 * @image html QBActiveTree.png
 **/

/**
 * Active tree class.
 * @class QBActiveTree
 * @extends SvObject
 **/
typedef struct QBActiveTree_ *QBActiveTree;


/**
 * Get runtime type identification object representing
 * type of active tree class.
 *
 * @return active tree class
 **/
extern SvType
QBActiveTree_getType(void);

/**
 * Initialize active tree instance.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @param[in] rootNode  root node handle
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBActiveTree
QBActiveTreeInit(QBActiveTree self,
                 QBActiveTreeNode rootNode,
                 SvErrorInfo *errorOut);

/**
 * Create active tree from XML tree definition file.
 *
 * @memberof QBActiveTree
 *
 * @param[in] filePath  path to a file containing active tree definition
 * @param[out] errorOut error info
 * @return              newly created active tree, @c NULL in case of error
 **/
extern QBActiveTree
QBActiveTreeCreateFromFile(const char *filePath,
                           SvErrorInfo *errorOut);

/**
 * Get root node of a tree.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @return              root node handle, @c NULL in case of error
 **/
extern QBActiveTreeNode
QBActiveTreeGetRootNode(QBActiveTree self);

/**
 * Find tree node with given ID.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @param[in] nodeID    ID of the tree node to find
 * @param[out] path     path to the node if found (can be @c NULL)
 * @return              handle to a tree node with @a nodeID,
 *                      @c NULL if not found
 **/
extern QBActiveTreeNode
QBActiveTreeFindNodeByID(QBActiveTree self,
                         SvString nodeID,
                         SvObject *path);

/**
 * Find tree node with given ID.
 *
 * @memberof QBActiveTree
 * @since 3.2.5
 *
 * @param[in] self      active tree handle
 * @param[in] nodeID    ID of the tree node to find
 * @return              handle to a tree node with @a nodeID,
 *                      @c NULL if not found
 **/
extern QBActiveTreeNode
QBActiveTreeFindNode(QBActiveTree self,
                     SvString nodeID);

/**
 * Create path to given tree node.
 *
 * @memberof QBActiveTree
 * @since 3.2.5
 *
 * @param[in] self      active tree handle
 * @param[in] nodeID    ID of the target tree node
 * @return              created path to a tree node with @a nodeID,
 *                      @c NULL if not found
 **/
extern SvObject
QBActiveTreeCreateNodePath(QBActiveTree self,
                           SvString nodeID);

/**
 * Find tree node at given path.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @param[in] path      path to the node to find
 * @return              handle to a tree node at @a path,
 *                      @c NULL if not found
 **/
extern QBActiveTreeNode
QBActiveTreeFindNodeByPath(QBActiveTree self,
                           SvObject path);

/**
 * Propagate QBTreeModelListener::nodesChanged() notification
 * for child nodes of a node at given @a path.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @param[in] path      handle to a tree path, @c NULL for root node
 * @param[in] first     index of the first modified node
 * @param[in] count     number of modified nodes
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreePropagateNodesChange(QBActiveTree self,
                                 SvObject path,
                                 size_t first, size_t count,
                                 SvErrorInfo *errorOut);

/**
 * Propagate QBTreeModelListener::nodesChanged() notification
 * for a given @a node.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @param[in] node      handle to a node to notify about
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreePropagateNodeChange(QBActiveTree self,
                                QBActiveTreeNode node,
                                SvErrorInfo *errorOut);

/**
 * Propagate QBTreeModelListener::nodesChanged notification
 * for a sub tree on a given @a path.
 *
 * @param[in] self      active tree handle
 * @param[in] path      path to a sub tree to notify about
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreePropagateSubTreeChange(QBActiveTree self,
                                   SvObject path,
                                   SvErrorInfo *errorOut);

/**
 * Add subtree to the active tree.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @param[in] parent    handle to a node that will become a parent of @a subTreeRoot
 * @param[in] subTreeRoot handle to the root node of the subtree to be added
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreeAddSubTree(QBActiveTree self,
                       QBActiveTreeNode parent,
                       QBActiveTreeNode subTreeRoot,
                       SvErrorInfo *errorOut);

/**
 * Add subtree to the active tree.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @param[in] parent    handle to a node that will become a parent of @a subTreeRoot
 * @param[in] subTreeRoot handle to the root node of the subtree to be added
 * @param[in] idx       index at which to insert @a subTreeRoot
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreeInsertSubTree(QBActiveTree self,
                          QBActiveTreeNode parent,
                          QBActiveTreeNode subTreeRoot,
                          size_t idx,
                          SvErrorInfo *errorOut);

/**
 * Remove subtree from the active tree.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @param[in] subTreeRoot handle to the root node of the subtree to be removed
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreeRemoveSubTree(QBActiveTree self,
                          QBActiveTreeNode subTreeRoot,
                          SvErrorInfo *errorOut);

/**
 * Remove all child nodes of a given sub-tree root.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @param[in] subTreeRoot handle to the parent node of the nodes to be removed
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreeRemoveNodes(QBActiveTree self,
                        QBActiveTreeNode subTreeRoot,
                        SvErrorInfo *errorOut);

/**
 * Mount new subtree to the active tree.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @param[in] subTree   subtree handle
 * @param[in] path      tree path to mount @a subTree at
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreeMountSubTree(QBActiveTree self,
                         SvObject subTree,
                         SvObject path,
                         SvErrorInfo *errorOut);

/**
 * Unmount previously mounted subtree.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @param[in] subTree   subtree handle
 * @param[out] errorOut error info
 **/
extern void
QBActiveTreeUmountSubTree(QBActiveTree self,
                          SvObject subTree,
                          SvErrorInfo *errorOut);

/**
 * List all mounted subtrees.
 *
 * @memberof QBActiveTree
 *
 * @param[in] self      active tree handle
 * @return              array of mounted subtrees, @c NULL in case of error
 **/
extern SvImmutableArray
QBActiveTreeGetMountedSubTrees(QBActiveTree self);

/**
 * Get mounted subtree at given path.
 *
 * @param[in] self      active tree handle
 * @param[in] path      path to mount point
 * @return              mounted subtree, @c NULL in case of error or inexistence
 **/
extern SvObject
QBActiveTreeGetMountedSubTree(QBActiveTree self, SvObject path);

/**
 * Create post order tree iterator.
 *
 * @param[in] self      active tree handle
 * @return              post order tree iterator
 **/
extern QBTreeIterator
QBActiveTreeCreatePostOrderTreeIterator(QBActiveTree self);

/**
 * Create pre order tree iterator.
 *
 * @param[in] self      active tree handle
 * @return              pre order tree iterator
 **/
extern QBTreeIterator
QBActiveTreeCreatePreOrderTreeIterator(QBActiveTree self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
