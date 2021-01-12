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

#ifndef QB_LOCAL_WINDOW_H_
#define QB_LOCAL_WINDOW_H_

/**
 * @file QBLocalWindowManager.h QBLocalWindowManager class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBApplicationControllerTypes.h>
#include <QBWindowContext.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvObject.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/QBWindowInternal.h>
#include <CUIT/Core/app.h>

/**
 * @defgroup QBLocalWindowManager Local window manager class
 * @ingroup QBApplicationControllerLibrary
 * @{
 *
 * A class that manages local windows and implements focus navigation between them.
 *
 * Local windows intention is to manage popups/sidemenus/etc for a single context,
 * that is, they are 'local' for that context. They shouldn't be shown on any
 * other one, nor should survive its destruction. Basically, if functionality
 * of a context requires a popup, it should be a local one. If a popup should be
 * show independantly of the context, then it should be a Global Window.
 *
 * Local windows type is either 'OnTop', 'NonFocusable', 'Focusable'.
 * Windows of type 'OnTop' is always on top. If there are more than one 'OnTop' widget,
 * window added as a last one will be shown on top.
 * If a window is 'NonFocusable' it is shown, but it will never get focus.
 * All other types are focusable.
 *
 * Intended for internal use, shouldn't be called by application layer.
 **/


/**
 * Create new local window manager instance.
 *
 * @param[in] app   CUIT application handle
 * @param[in] fm    focus manager handle
 * @return a new local window manager instance
 **/
QBLocalWindowManager QBLocalWindowManagerCreate(SvApplication app, QBFocusManager fm);

/**
 * Adds a new local window, it will be shown immediately if possible.
 * If it is not, then it will be queued and shown when possible.
 *
 * All local windows are automatically removed upon context change.
 * @param[in] lm        local window manager instance
 * @param[in] local     window to add
 * @param[in] ctx       current window context
 **/
void QBLocalWindowManagerAddWindow(QBLocalWindowManager lm, QBLocalWindow local, QBWindowContext ctx);

/**
 * Instantly removes a window from screen.
 *
 * @param[in] lm        local window manager instance
 * @param[in] local     window to remove
 * @param[in] ctx       current window context
 **/
void QBLocalWindowManagerRemoveWindow(QBLocalWindowManager lm, QBLocalWindow local, QBWindowContext ctx);

/**
 * Stop local window manager.
 *
 * @param[in] self local window manager handle
 **/
void QBLocalWindowManagerStop(QBLocalWindowManager self);

/**
 * Notify local windows that application is reinitialized.
 *
 * @note This method will only notify local windows,
 * that implement the @ref QBReinitializable.
 *
 * @param[in] self local window manager handle
 * @param[in] requestors list of requestors as SvString
 *
 **/
void QBLocalWindowManagerReinitialize(QBLocalWindowManager self, SvArray requestors);

/**
 * @}
 **/


/**
 * @defgroup QBLocalWindow Local window class
 * @ingroup QBApplicationControllerLibrary
 * @{
 **/

/**
 * Available local window types.
 **/
typedef enum QBLocalWindowType_e {
    QBLocalWindowTypeFocusable = 0, //!< by default window is focusable
    QBLocalWindowTypeNonFocusable,  //!< window that can't be focused
    QBLocalWindowTypeOnTop,         //!< focusable window always on top
    QBLocalWindowTypeMax            //!< count of local window types
} QBLocalWindowType;

/**
 * An abstract base class for local window implementations.
 **/
struct QBLocalWindow_t {
    struct QBWindow_ super_; //!< super class
    SvWidget window;    //!< window that will be shown
    QBLocalWindowType type;      //!< type of local window
};

/**
 * Get runtime type identification object describing local window class.
 * @return Local window runtime type identification object
 */
SvType QBLocalWindow_getType(void);

/**
 * Initialize local window instance.
 * @param[in] self      instance to initialize
 * @param[in] window    window that will be shown
 * @param[in] type      type of local window
 */
void QBLocalWindowInit(QBLocalWindow self, SvWindow window, QBLocalWindowType type);

/**
 * Remove all local windows.
 * @param[in] lm        local window manager instance
 * @param[in] ctx       on which QBWindowContext to remove the windows
 */
void QBLocalWindowManagerRemoveAll(QBLocalWindowManager lm, QBWindowContext ctx);

/**
 * Returns the count of shown local windows.
 * @param[in] lm        local window manager instance
 * @return count of shown local windows
 **/
int QBLocalWindowManagerShownWindowsCount(QBLocalWindowManager lm);

/**
 * @}
 **/

#endif
