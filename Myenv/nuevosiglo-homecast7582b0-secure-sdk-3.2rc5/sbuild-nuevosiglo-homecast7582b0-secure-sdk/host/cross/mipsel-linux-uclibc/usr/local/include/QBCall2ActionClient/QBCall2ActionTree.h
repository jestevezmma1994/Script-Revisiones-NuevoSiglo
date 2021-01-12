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
#ifndef QBCALL2ACTION_TREE_H_
#define QBCALL2ACTION_TREE_H_

/**
 * @file QBCall2ActionTree.h Call2ActionTree class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBCall2ActionClient/QBCall2ActionTreeNode.h>
#include <QBCall2ActionClient/QBCall2ActionTreePath.h>
#include <SvCore/SvErrorInfo.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvValue.h>

/**
 * @defgroup QBCall2ActionTree Call2ActionTree class
 * @ingroup QBCall2ActionProvider
 * @{
 **/

/**
 * Call2ActionTree class.
 * @class QBCall2ActionTree
 * @extends SvObject
 **/
typedef struct QBCall2ActionTree_ * QBCall2ActionTree;

/**
 * Create QBCall2ActionTree
 *
 * @param[in] rootId    name of root id
 * @param[out] errorOut error info handle
 * @return              @c tree object if creation ends with success, @c NULL otherwise
 **/
QBCall2ActionTree
QBCall2ActionTreeCreate(SvString rootId, SvErrorInfo * errorOut);

/**
 * Remove node from QBCall2ActionTree
 *
 * @param[in] self      tree handle
 * @param[in] node      node that going to be removed from tree
 * @param[out] errorOut error info handle
 * @return              @c true if node was removed with success, @c false otherwise
 **/
bool
QBCall2ActionTreeRemoveNode(QBCall2ActionTree self, QBCall2ActionTreeNode node, SvErrorInfo * errorOut);

/**
 * Get root node from QBCall2ActionTree
 *
 * @note QBCall2ActionTree can't exist without root node.
 *
 * @param[in] self      tree handle
 * @param[out] errorOut error info handle
 * @return              @c root node if tree wasn't NULL, @c NULL otherwise
 **/
extern QBCall2ActionTreeNode
QBCall2ActionTreeGetRootNode(QBCall2ActionTree self, SvErrorInfo * errorOut);

/**
 * Get QBCall2ActionTree iterator that implements QBTreeIterator interface.
 *
 * @note Iterator iterates through 'node' children starting with child at 'position'.
 *
 * @param[in] self      tree handle
 * @param[in] node      node handle
 * @param[in] position  start position
 * @return              @c iterator handle
 **/
QBTreeIterator
QBCall2ActionTreeIterator(QBCall2ActionTree self, QBCall2ActionTreeNode node, size_t position);

/**
 * Find node in QBCall2ActionTree by path.
 *
 * @param[in] self      tree handle
 * @param[in] path      path handle
 * @param[out] errorOut error info handle
 * @return              @c node if there is node at given path, @c NULL otherwise
 **/
extern QBCall2ActionTreeNode
QBCall2ActionTreeFindNodeByPath(QBCall2ActionTree self, QBCall2ActionTreePath path, SvErrorInfo * errorOut);

/**
 * Find node in QBCall2ActionTree by node id.
 *
 * @param[in] self      tree handle
 * @param[in] id        node id
 * @param[in, out] path path handle
 * @param[out] errorOut error info handle
 * @return              @c node if there is node with given id, @c NULL otherwise
 **/
extern QBCall2ActionTreeNode
QBCall2ActionTreeFindNodeById(QBCall2ActionTree self, SvValue id, QBCall2ActionTreePath * path, SvErrorInfo * errorOut);

/**
 * Add node to QBCall2ActionTree.
 *
 * @param[in] self      tree handle
 * @param[in] object    SvDBRawObject handle
 * @param[in] parentId  parent id
 * @param[out] errorOut error info handle
 **/
void
QBCall2ActionTreeAddObject(QBCall2ActionTree self, SvDBRawObject object, SvValue parentId, SvErrorInfo * errorOut);

/**
 * @}
 **/

#endif
