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
#ifndef QBCALL2ACTION_TREE_NODE_H_
#define QBCALL2ACTION_TREE_NODE_H_

/**
 * @file QBCall2ActionTreeNode.h Call2ActionTreeNode class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvHashTable.h>
#include <stdbool.h>

/**
 * @defgroup QBCall2ActionTreeNode Call2ActionTreeNode class
 * @ingroup QBCall2ActionProvider
 * @{
 **/

/** QBCall2ActionTreeNodeType defines flow that is expected from gui */
typedef enum QBCall2ActionTreeNodeType {
    QBCall2ActionTreeNodeTypeInvalid = -1, /**<  incorrect action type */
    QBCall2ActionTreeNodeTypeNormal, /**<  action contains sub-actions, gui shows another level of actions */
    QBCall2ActionTreeNodeTypeMultichoice, /**<  no sub-actions, gui shows the same level of actions */
    QBCall2ActionTreeNodeTypeFinal, /**<  no sub-actions, gui hide all levels of actions, after response*/
    QBCall2ActionTreeNodeTypeFinalAsynchronous /**<  no sub-actions, gui hide all levels of actions immediately*/
} QBCall2ActionTreeNodeType;

/**
 * Call2ActionTreeNode class.
 * @class QBCall2ActionTreeNode
 * @extends SvObject
 **/
typedef struct QBCall2ActionTreeNode_ * QBCall2ActionTreeNode;

/**
 * Get runtime type identification object representing QBCall2ActionTreeNode class.
 * @public @memberof QBCall2ActionProvider
 *
 * @return QBCall2ActionTreeNode class
 */
extern SvType
QBCall2ActionTreeNode_getType(void);

/**
 * Create QBCall2ActionTreeNode
 * @public @memberof QBCall2ActionProvider
 *
 * @param[in] object    DataBucket object
 * @param[out] errorOut error info handle
 * @return              @c QBCall2ActionTreeNode object, if creation ends with success, @c NULL, otherwise
 */
QBCall2ActionTreeNode
QBCall2ActionTreeNodeCreate(SvDBRawObject object, SvErrorInfo * errorOut);

/**
 * Get node id.
 * @public @memberof QBCall2ActionProvider
 *
 * @note SvDBRawObject can't exist without id, QBCall2ActionTreeNode also.
 *
 * @param[in] self      node handle
 * @param[out] errorOut error info handle
 * @return              @c id handle if self argument isn't NULL, @c NULL otherwise
 */
extern SvValue
QBCall2ActionTreeNodeGetId(QBCall2ActionTreeNode self, SvErrorInfo * errorOut);

/**
 * Get value from attribute.
 * @public @memberof QBCall2ActionProvider
 *
 * @param[in] self      node handle
 * @param[in] key       key name
 * @param[out] errorOut error info handle
 * @return              @c value handle if node contains attribute, @c NULL otherwise
 */
extern SvValue
QBCall2ActionTreeNodeGetAttrValue(QBCall2ActionTreeNode self, const char * key, SvErrorInfo * errorOut);

/**
 * Get SvDBRawObject from node
 * @public @memberof QBCall2ActionProvider
 *
 * @note QBCall2ActionTreeNode can't exist without SvDBRawObject.
 *
 * @param[in] self      node handle
 * @param[out] errorOut error indo handle
 * @return              @c SvDBRawObject object if self argument isn't NULL, @c NULL otherwise
 */
extern SvDBRawObject
QBCall2ActionTreeNodeGetObject(QBCall2ActionTreeNode self, SvErrorInfo * errorOut);

/**
 * Get action type from node
 * @public @memberof QBCall2ActionProvider
 *
 * @note QBCall2ActionTreeNode can't be created without valid 'type' attribute.
 *
 * @param[in] self      node handle
 * @param[out] errorOut error info handle
 * @return              @c node type value if self argument isn't NULL, @c NULL otherwise
 */
extern QBCall2ActionTreeNodeType
QBCall2ActionTreeNodeGetType(QBCall2ActionTreeNode self, SvErrorInfo * errorOut);

/**
 * Get level of node from tree
 * @public @memberof QBCall2ActionProvider
 *
 * @param[in] self      node handle
 * @param[out] errorOut error info handle
 * @return              @c level value if self argument isn't NULL, @c -1 otherwise
 */
size_t
QBCall2ActionTreeNodeGetLevel(QBCall2ActionTreeNode self, SvErrorInfo * errorOut);

/**
 * Get children nodes count from node
 * @public @memberof QBCall2ActionProvider
 *
 * @param[in] self      node handle
 * @param[out] errorOut error info handle
 * @return              @c children count if self argument isn't NULL, @c -1 otherwise
 */
size_t
QBCall2ActionTreeNodeChildrenCount(QBCall2ActionTreeNode self, SvErrorInfo * errorOut);

/**
 * Get children nodes table.
 * @public @memberof QBCall2ActionProvider
 *
 * Children nodes are holding in hash table in paires <nodeId, node>
 *
 * @param[in] self      node handle
 * @param[out] errorOut error info handle
 * @return              @c hash table if there's any child, @c NULL otherwise
 */
SvHashTable
QBCall2ActionTreeNodeGetChildren(QBCall2ActionTreeNode self, SvErrorInfo * errorOut);

/**
 * Get parent node from node
 * @public @memberof QBCall2ActionProvider
 *
 * @param[in] self      node handle
 * @param[out] errorOut error info handle
 * @return              @c parent node if there's parent, @c NULL otherwise
 */
QBCall2ActionTreeNode
QBCall2ActionTreeNodeGetParent(QBCall2ActionTreeNode self, SvErrorInfo * errorOut);

/**
 * Add node as a child.
 * @public @memberof QBCall2ActionProvider
 *
 * @param[in] self      node handle
 * @param[in] node      node that going to be the child
 * @param[out] errorOut error info handle
 * @return              @c true if node was added with success, @c false otherwise
 */
bool
QBCall2ActionTreeNodeAddChild(QBCall2ActionTreeNode self, QBCall2ActionTreeNode node, SvErrorInfo * errorOut);

/**
 * Remove node from children nodes.
 * @public @memberof QBCall2ActionProvider
 *
 * @param[in] self      node handle
 * @param[in] node      node that going to be removed
 * @param[out] errorOut error info handle
 * @return              @c true if node was removed with success, @c false otherwise
 */
bool
QBCall2ActionTreeNodeRemoveChild(QBCall2ActionTreeNode self, QBCall2ActionTreeNode node, SvErrorInfo * errorOut);

/**
 * Remove all children.
 * @public @memberof QBCall2ActionProvider
 *
 * @param[in] self      node handle
 * @param[out] errorOut error info handle
 * @return              @c true if nodes were removed with success, @c false otherwise
 */
bool
QBCall2ActionTreeNodeRemoveAllChildren(QBCall2ActionTreeNode self, SvErrorInfo * errorOut);

/**
 * Remove nodes array from children nodes.
 * @public @memberof QBCall2ActionProvider
 *
 * @note Function return count of nodes that were removed with success.
 * If error occures, function end and return count of successfully removed nodes.
 *
 * @param[in] self      node handle
 * @param[in] array     nodes array
 * @param[out] errorOut error info handle
 * @return              @c removed nodes count, @c -1 if node didn't have children or parameters were empty
 */
int
QBCall2ActionTreeNodeRemoveChildren(QBCall2ActionTreeNode self, SvArray array, SvErrorInfo * errorOut);

/**
 * Add nodes array as node children.
 * @public @memberof QBCall2ActionProvider
 *
 * @note Function return count of nodes that were added with success.
 * If error occures, function end and return count of successfully added nodes.
 *
 * @param[in] self      node handle
 * @param[in] array     nodes array
 * @param[out] errorOut error info handle
 * @return              @c added nodes count, @c -1 if parameters were empty
 */
int
QBCall2ActionTreeNodeAddChildren(QBCall2ActionTreeNode self, SvArray array, SvErrorInfo * errorOut);

/**
 * @}
 **/

#endif
