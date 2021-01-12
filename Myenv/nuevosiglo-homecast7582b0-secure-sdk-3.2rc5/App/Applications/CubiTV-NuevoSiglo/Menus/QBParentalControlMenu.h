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

#ifndef QBPARENTALCONTROLMENU_H
#define QBPARENTALCONTROLMENU_H

/**
 * @file QBParentalControlMenu.h
 * @brief Menu of the parental control service.
 **/

#include <CUIT/Core/types.h>
#include <QBDataModel3/QBTreePathMap.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <Services/QBParentalControl/QBPCLockedTimes.h>
#include <SvFoundation/SvObject.h>
#include <main_decl.h>

typedef struct QBParentalControlMenuHandler_t* QBParentalControlMenuHandler;

void QBParentalControlMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals);

void QBParentalControlMenuUpdateLockedTimeTask(QBParentalControlMenuHandler self, QBParentalControlLockedTimeTask srcTask, QBActiveTreeNode node);

/**
 * Create menu node based on the LockedTimeTask object.
 *
 * @param[in] appGlobals    application global variables handle
 * @param[in] task          LockedTimeTask object handle
 * @return                  created menu node or @c NULL in case of error
 */
SvObject
QBParentalControlMenuCreateLockedTimeNode(AppGlobals appGlobals, QBParentalControlLockedTimeTask task);

void QBParentalControlMenuLockedTimeRemoveEmptyNode(AppGlobals appGlobals);

#endif
