/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_LOG_WINDOW_MANAGER_H
#define QB_LOG_WINDOW_MANAGER_H

/**
 * @file QBLogWindowManager.h Log window manager class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h> // AppGlobals
#include <QBApplicationControllerTypes.h> // QBGlobalWindow

/**
 * @defgroup QBLogWindowManager Log window manager class
 * @ingroup CubiTV_services
 * @{
 **/

typedef struct QBLogWindowManager_s* QBLogWindowManager;

/**
 * Create log window manager.
 *
 * @param[in] appGlobals    appGlobals handle
 * @return                  created log window manager, @c NULL in case of error
 **/
QBGlobalWindow QBLogWindowManagerCreate(AppGlobals appGlobals);

/**
 * Start log window manager.
 * @param[in] self          handler to QBLogWindowManager
 */
void QBLogWindowManagerStart(QBGlobalWindow self);

/**
 * Stop log window manager.
 * @param[in] self          handler to QBLogWindowManager
 */
void QBLogWindowManagerStop(QBGlobalWindow self);

/**
 * Add a log message to QBLogWindow. The message will be displayed immediately.
 * @param[in] self_         QBLogWindowManager handle
 * @param[in] text          message to log
 */
void QBLogWindowManagerAddLog(QBGlobalWindow self_, char* text);

/**
 * @}
 **/

#endif
