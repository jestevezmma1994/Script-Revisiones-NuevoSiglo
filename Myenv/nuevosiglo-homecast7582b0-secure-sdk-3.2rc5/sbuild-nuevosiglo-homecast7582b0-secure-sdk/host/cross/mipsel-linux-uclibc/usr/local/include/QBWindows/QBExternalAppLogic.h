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

#ifndef QB_EXTERNAL_APP_LOGIC_H
#define QB_EXTERNAL_APP_LOGIC_H

#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBWindowContext.h>
#include <QBInputApp.h>

/**
 * @file QBExternalAppLogic.h
 * @brief QBExternalAppLogic handles input events delivery for external applications
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup External Application logic
 * @ingroup CubiTV_logic
 * @{
 **/

/**
 *QBExternalAppLogic class handles input events for external applications
 */
typedef struct QBExternalAppLogic_ *QBExternalAppLogic;

/**
 * Creates external application logic instance
 *
 * @param[in] ctx           QBExternalApp context handle
 *
 * @return                  QBExternalAppLogic object, NULL if object creation failed
 */
QBExternalAppLogic
QBExternalAppLogicCreate(QBWindowContext ctx) __attribute__((weak));

/**
 * External application logic input event handler
 *
 * @param[in] self          QBExternalAppLogic handle
 * @param[in] app           QBInputApp handle
 * @param[in] ev            input event handle
 *
 * @return                  @c false if event was not handled, true otherwise
 */
bool
QBExternalAppLogicInputEventHandler(QBExternalAppLogic self, QBInputApp app, SvInputEvent ev) __attribute__((weak));

/**
 * @}
 **/
#endif // QB_EXTERNAL_APP_LOGIC_H
