/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_FOCUS_MANAGER_H_
#define QB_FOCUS_MANAGER_H_

/**
 * @file QBFocusManager.h QBFocusManager class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <QBApplicationControllerTypes.h>
#include <CUIT/Core/app.h>
#include <stdbool.h>

/**
 * @defgroup QBFocusManager Focus manager class
 * @ingroup QBApplicationControllerLibrary
 * @{
 *
 * A utility class that decides which window should be focused.
 * Each (local/global) window manager selects a single window
 * that should have focus and then this class selects the
 * appropriate one for focus. If no window is available
 * then current context will be focused.
 *
 * This functionality is internal and shouldn't be used by
 * application layer.
 **/

/**
 * Create new focus manager.
 *
 * @param[in] app CUIT application handle
 * @return created focus manager or @c NULL in case of error
 **/
QBFocusManager QBFocusManagerCreate(SvApplication app);

/**
 * Informs which local window has been selected to have focus out of
 * all local windows.
 * @param[in] self      focus manager handle
 * @param[in] local     local window handle
 **/
void QBFocusManagerSetLocalWindow(QBFocusManager self, QBLocalWindow local);

/**
 * Returns local window that has focus
 * @param[in] self      focus manager handle
 * @return              QBLocalWindow handle
 **/
QBLocalWindow QBFocusManagerGetLocalWindow(QBFocusManager self);

/**
 * Returns global window that has focus
 * @param[in] self      focus manager handle
 * @return              QBGlobalWindow handle
 **/
QBGlobalWindow QBFocusManagerGetGlobalWindow(QBFocusManager self);

/**
 * Informs that the local window is no longer to be focused.
 * @param[in] self      focus manager handle
 * @param[in] local     local window handle
 **/
void QBFocusManagerRemoveLocalWindow(QBFocusManager self, QBLocalWindow local);

/**
 * Informs which global window has been selected to have focus out of
 * all global windows.
 * @param[in] self      focus manager handle
 * @param[in] global    global window handle
 **/
void QBFocusManagerSetGlobalWindow(QBFocusManager self, QBGlobalWindow global);

/**
 * Informs if global window has been selected to have focus
 * @param[in] self      focus manager handle
 * @return              true if the global window was focused
 **/
bool QBFocusManagerIsGlobalWindowFocused(QBFocusManager self);

/**
 * Informs that the global window is no longer to be focused.
 * @param[in] self      focus manager handle
 * @param[in] global    global window handle
 **/
void QBFocusManagerRemoveGlobalWindow(QBFocusManager self, QBGlobalWindow global);

/**
 * @}
 **/

#endif
