/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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
#ifndef QBINPUTAPP_H
#define QBINPUTAPP_H

#include <QBApp.h>
#include <QBInput/QBInputEvent.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvHashTable.h>
#include <stdint.h>

/**
 * @file QBInputApp.h
 * @brief QBInputApp class provides interface for user input aspects of an external app.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * QBInputApp class represents external graphical application with user input support,
 * launched by QBAppsManager.
 * It's deriviated from QBGfxApp and extends it by adding IInput RPC interface implementation.
 */
typedef struct QBInputApp_* QBInputApp;

/**
 * Get runtime type identification object representing QBInputApp class
 *
 * @return     object representing QBApp class
 */
SvType
QBInputApp_getType(void);

/**
 * @brief This function creates RPC interfaces implemented by QBInputApp and its ancestors
 *
 * @param     self          handle to QBInputApp interpreted as QBApp
 * @return    A hash table containing list of implemented interfaces
 */
SvHashTable
QBInputAppCreateImplementedInterfaces(QBApp self);


/**
 * @brief This function initializes fields of class
 *
 * @param     self_         QBInputApp handle
 * @param[in] implementedInterfaces hash table containing list of implemented interfaces
 * @param[in] params        QBApp initialization parameters
 * @return    true if function succeeded
 */
bool
QBInputAppInitAndStart(QBInputApp self_, SvHashTable implementedInterfaces, QBAppInitParams params);

/**
 * @brief This function handles input event.
 *
 * @param     self          QBInputApp handle
 * @param[in] inputEvent    input event to handle
 * @return    true if event was handled; false otherwise
 */
bool
QBInputAppHandleInputEvent(QBInputApp self, QBInputEvent inputEvent);

#endif
