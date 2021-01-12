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
#ifndef QBCALL2ACTION_TREE_PATH_H_
#define QBCALL2ACTION_TREE_PATH_H_

/**
 * @file QBCall2ActionTreePath.h Call2ActionTreePath class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBCall2ActionClient/QBCall2ActionTreeNode.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvType.h>

/**
 * @defgroup QBCall2ActionTreePath Call2ActionTreePath class
 * @ingroup QBCall2ActionProvider
 * @{
 **/

/**
 * Call2ActionTreePath class
 * @class QBCall2ActionTreePath
 * @implements QBTreePath
 * @extends SvObject
 **/
typedef struct QBCall2ActionTreePath_ * QBCall2ActionTreePath;

/**
 * Get runtime type identification object representing QBCall2ActionTreePath class.
 *
 * @return call2ActionTreePath class
 **/
extern SvType
QBCall2ActionTreePath_getType(void);

/**
 * Create QBCall2ActionTreePath.
 *
 * @param[in] node      node handle
 * @param[out] errorOut error info handle
 * @return              @c path handle if creation ends with success, @c NULL otherwise
 **/
QBCall2ActionTreePath
QBCall2ActionTreePathCreate(QBCall2ActionTreeNode node, SvErrorInfo * errorOut);

/**
 * Get id from QBCall2ActionTreePath.
 *
 * @note Path leads to a node in the tree. Method returns id of this node.
 *
 * @param[in] self  path handle
 * @return          @c id value if path isn't NULL, @c NULL otherwise
 **/
SvValue
QBCall2ActionTreePathGetID(QBCall2ActionTreePath self);

/**
 * @}
 **/

#endif
