/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCONFIGURATIONMENULOGIC_H_
#define QBCONFIGURATIONMENULOGIC_H_

/**
 * @file QBConfigurationMenuLogic.h QBConfigurationMenu Logic API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>
#include <SvFoundation/SvString.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBActiveTree.h>
#include <main_decl.h>

/**
 * @defgroup QBConfigurationMenuLogic QBConfigurationMenu logic
 * @ingroup CubiTV_logic
 * @{
 **/

typedef struct QBConfigurationMenuLogic_* QBConfigurationMenuLogic;

/**
 * Creates configuration menu instance
 *
 * @param[in] appGlobals appGlobals handle
 * @return newly created configuration menu instance
 */
QBConfigurationMenuLogic QBConfigurationMenuLogicCreate(AppGlobals appGlobals) __attribute__((weak));

/**
 * Starts Configuration menu logic
 *
 * @param[in] self QBConfigurationMenuLogic instance
 */
void QBConfigurationMenuLogicStart(QBConfigurationMenuLogic self) __attribute__((weak));

/**
 * Stops Configuration menu logic
 *
 * @param[in] self QBConfigurationMenuLogic instance
 */
void QBConfigurationMenuLogicStop(QBConfigurationMenuLogic self) __attribute__((weak));

/**
 * Triggers action from configuration menu.
 *
 * @param[in] self QBConfigurationMenuLogic instance
 * @param[in] action action name
 * @param[in] node node associated to action
 * @return @c true if action is handled, @c false otherwise
 */
bool QBConfigurationMenuLogicActionTrigger(QBConfigurationMenuLogic self, SvString action, QBActiveTreeNode node) __attribute__((weak));

/**
 * Function adjusts menu tree according to logic specific e.g. can remove some nodes
 *
 * @param[in] self QBConfigurationMenuLogic instance
 * @param[in] menu menu to be adjusted
 */
void QBConfigurationMenuLogicAdjustMenuTree(QBConfigurationMenuLogic self, QBActiveTree menu) __attribute__((weak));

/** @} */

#endif /* QBCONFIGURATIONMENULOGIC_H_ */
