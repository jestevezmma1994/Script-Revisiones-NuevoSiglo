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

#ifndef QB_GLOBAL_MANAGER_H_
#define QB_GLOBAL_MANAGER_H_

/**
 * @file QBGlobalWindowManager.h QBGlobalWindowManager class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/QBWindowInternal.h>
#include <CUIT/Core/widget.h>
#include <QBApplicationControllerTypes.h>
#include <QBWindowContext.h>

/**
 * @defgroup QBGlobalWindowManager Global window manager class
 * @ingroup QBApplicationControllerLibrary
 * @{
 *
 * A class that manages global windows and implements focus navigation between them.
 *
 * Global windows intention is to manage popups/sidemenus/etc that should be shown
 * independently of which context is on screen. They are still on screen even after
 * a context switch.
 *
 * Global windows have a name, which is used for whitelisting global windows on a
 * per context basis. It is possible to disallow showing certain global windows
 * on selected contexts. The window may be added to global window manager, but it will
 * not be shown until application switches to a context that allows this window.
 *
 * Intended for internal use, shouldn't be called by application layer.
 **/

/**
 * Create new global window manager instance.
 *
 * @param[in] app   CUIT application handle
 * @param[in] name  name in settings which specifies which windows are visible on which screens
 * @param[in] fm    focus manager handle
 * @return a new global window manager instance
 **/
QBGlobalWindowManager QBGlobalWindowManagerCreate(SvApplication app, const char *name, QBFocusManager fm);

/**
 * Destroy global window manager instance.
 *
 * @param[in] self global window manager handle
 **/
void QBGlobalWindowManagerDestroy(QBGlobalWindowManager self);

/**
 * Stop global window manager.
 *
 * @param[in] self global window manager handle
 **/
void QBGlobalWindowManagerStop(QBGlobalWindowManager self);

/**
 * Adds a new global window, it will be shown immediately if possible.
 * If it is not, then it will be queued and shown when possible.
 *
 * @param[in] self      global window manager instance
 * @param[in] global    window to add
 * @param[in] ctx       current window context
 * @return true iff window has been shown, otherwise is added to set of windows to be added
 **/
bool QBGlobalWindowManagerAddWindow(QBGlobalWindowManager self, QBGlobalWindow global, QBWindowContext ctx);

/**
 * Instantly removes a window from screen.
 *
 * @param[in] self      global window manager instance
 * @param[in] global    window to remove
 * @param[in] ctx       current window context
 **/
void QBGlobalWindowManagerRemoveWindow(QBGlobalWindowManager self, QBGlobalWindow global, QBWindowContext ctx);

/**
 * Queries whether a global window can be added to a selected (current) window context.
 *
 * @param[in] gm        global window manager instance
 * @param[in] globalName window name
 * @param[in] ctx       window context
 * @return true iff window can be added to context
 **/
bool QBGlobalWindowManagerCanAddWindowWithName(QBGlobalWindowManager gm, SvString globalName, QBWindowContext ctx);

/**
 * Notify global windows that application is reinitialized.
 *
 * @note This method will only notify global windows,
 * that implement the @ref QBReinitializable.
 *
 * @param[in] self global window manager handle
 * @param[in] requestors list of requestors as SvString
 *
 **/
void QBGlobalWindowManagerReinitialize(QBGlobalWindowManager self, SvArray requestors);

/**
 * @}
 **/


/**
 * @defgroup QBGlobalWindow Global window class
 * @ingroup QBApplicationControllerLibrary
 * @{
 *
 * An abstract base class for global window implementations.
 **/

struct QBGlobalWindow_t {
    struct QBWindow_ super_;        //!< super class
    SvWidget window;                //!< widget that will be placed on screen
    SvString name;                  //!< name of the widget
    bool focusable;                 //!< @c true if window is focusable and wants focus
};

/**
 * Get runtime type identification object representing global window class.
 * @return Global window runtime type identification object
 */
SvType QBGlobalWindow_getType(void);

/**
 * Initialize global window instance.
 *
 * @param[in] self global window handle
 * @param[in] window window to be shown
 * @param[in] name name of the widget
 **/
void QBGlobalWindowInit(QBGlobalWindow self, SvWindow window, SvString name);

/**
 * Default handler for window context change
 * @param[in] self global window manager handle
 * @param[in] ctx new window context
 **/
void QBGlobalWindowManagerContextChanged(QBGlobalWindowManager self, QBWindowContext ctx);

/**
 * @}
 **/


#endif
