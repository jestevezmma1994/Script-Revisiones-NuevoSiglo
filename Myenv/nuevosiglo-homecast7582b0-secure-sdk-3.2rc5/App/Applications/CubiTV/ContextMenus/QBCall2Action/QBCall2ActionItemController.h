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

#ifndef QBCALL2ACTION_ITEM_CONTROLLER_H_
#define QBCALL2ACTION_ITEM_CONTROLLER_H_

/**
 * @file QBCall2ActionItemController.h Call2ActionItemController class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>
#include <QBCall2ActionClient/QBCall2ActionTree.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>

/**
 * @defgroup QBCall2ActionItemController Call2ActionItemController class
 * @ingroup QBCall2ActionContext
 * @{
 **/

/**
 * Call2ActionItemController is a default item controller for Call2ActionPane.
 * It's set at the root level of Call2ActionTree. There's posibility to set external item controller for any path.
 **/

/**
 * Create item controller
 *
 * @param[in] tree          QBCall2ActionTree handle
 * @param[in] appGlobals    appGlobals handle
 * @param[in] widgetName    paramter group name in settings
 * @return                  @c QBCall2ActionItemController instance, if creation end with success, @c NULL, otherwise
 **/
SvObject
QBCall2ActionItemControllerCreate(QBCall2ActionTree tree, AppGlobals appGlobals, SvString widgetName);

/**
 * @}
 **/

#endif /* QBCALL2ACTION_ITEM_CONTROLLER_H_ */
