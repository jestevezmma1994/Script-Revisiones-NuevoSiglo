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

#ifndef QB_APPLICATION_CONTROLLER_H_
#define QB_APPLICATION_CONTROLLER_H_

#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <QBAppKit/QBAsyncService.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/types.h>
#include <QBInput/QBInputEvent.h>
#include <QBInput/QBInputQueue.h>

#include <QBApplicationControllerTypes.h>
#include <QBGlobalWindowManager.h>
#include <QBLocalWindowManager.h>
#include <QBFocusManager.h>
#include <QBContextSwitcher.h>
#include <QBWindowContext.h>
#include <QBWindowContextInputEventHandler.h>
#include <stdbool.h>

/**
 * @file QBApplicationController.h QBApplicationController library API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBApplicationControllerLibrary QBApplicationController: application controller library
 * @ingroup GUI
 * @{
 *
 * QBApplicationController library is used by the GUI applications
 * to manage the stack of @ref CUIT windows.
 *
 * There are three types of windows:
 * - top-level windows (represented by the @ref QBWindowContext): used to display a single application
 *   screen, there can be only one such window on the screen,
 * - local windows (represented by the @ref QBLocalWindow): used for transient windows like pop-up
 *   notifications that are created and owned by current top-level window,
 * - global windows (represented by the @ref QBGlobalWindow): used for any windows like pop-ups or notifications,
 *   that concern the whole application.
 *
 * Implementations of top-level windows must inherit from the @ref QBWindowContext and implement
 * its virtual methods. Contexts of top-level windows are kept on a stack, the window
 * represented by a context being on the top of the stack is displayed.
 * To manage this stack, use methods QBApplicationControllerPushContext(), QBApplicationControllerPopContext(),
 * QBApplicationControllerSwitchToRoot(), or their extended versions.
 *
 * Generally, contexts should be created when pushed onto stack and destroyed when popped.
 * If some context should live longer, it should be registered in the controller using
 * QBApplicationControllerAddPersistentContext() method.
 *
 * Whenever applications does a huge change in appearance (for example: change language, screen resolution
 * or GUI theme/layout), QBApplicationControllerReinitializeWindows() method should be used: it will notify
 * all existing contexts about the change and request to redraw their windows.
 *
 * Local and global windows have dedicated manager classes: the @ref QBLocalWindowManager and the @ref QBGlobalWindowManager.
 * Application controller serves only as a proxy for adding and removing local and global windows.
 **/

/**
 * @defgroup QBApplicationController Application controller class
 * @ingroup QBApplicationControllerLibrary
 * @{
 *
 * A class implementing the core of application controller library.
 **/

/**
 * Application controller parameters.
 **/
typedef struct QBApplicationControllerParams_t {
    SvApplication app;                      ///< CUIT application handle
    SvScheduler scheduler;                  ///< a handle to the scheduler that drives GUI thread
    SvObject switcher;                      ///< default context switcher (a handle to any object implementing @ref QBContextSwitcher)
    QBGlobalWindowManager globalManager;    ///< a handle to the global window manager
    QBLocalWindowManager localManager;      ///< a handle to the local window manager
    QBFocusManager focusManager;            ///< a handle to the focus manager
    SvArray windowTypesToRemember;          ///< window types which should be remembered in history
    QBInputQueue inputQueue;                ///< input events queue
} *QBApplicationControllerParams;

struct QBApplicationControllerInputListener_ {
    /**
     * This function is invoked when key is pressed
     *
     * @param[in] self object implementing QBSecretKeyCodeService interface
     * @param[in] code key input event
     **/
    void (*keyPressed)(SvObject self_, const QBInputEvent *ev);
};

/**
* This data type defines an interface for QBApplicationController input listeners.
*
* @class QBApplicationControllerInputListener
**/
typedef struct QBApplicationControllerInputListener_* QBApplicationControllerInputListener;

/**
 * Global input handler type.
 *
 **/
typedef bool (*QBApplicationControllerInputEventHandler)(void* param, const QBInputEvent* ev);

/**
 * Create new QBApplicationController instance.
 *
 * @param[in] params controller parameters
 * @return created application controller or @c NULL in case of error
 **/
QBApplicationController QBApplicationControllerCreate(QBApplicationControllerParams params);

/**
 * Set default context switcher.
 *
 * @param[in] self application controller handle
 * @param[in] switcher handle to an object implementing @ref QBContextSwitcher
 **/
void QBApplicationControllerSetSwitcher(QBApplicationController self, SvObject switcher);

/**
 * Stop asynchronous tasks of the application controller.
 *
 * This method finishes all background tasks managed by the application
 * controller. It allows for safe destruction of contexts, local and global
 * windows.  It should be called before QBApplicationControllerDestroy().
 *
 * @param[in] self      application controller handle
 * @param[out] errorOut error info
 **/
static inline void QBApplicationControllerStop(QBApplicationController self, SvErrorInfo *errorOut)
{
    SvInvokeInterface(QBAsyncService, self, stop, errorOut);
}

/**
 * Get contexts stack.
 *
 * @param[in] self application controller handle
 * @return context history array or @c NULL in case of error
 */
SvArray QBApplicationControllerGetContextStack(QBApplicationController self);

/**
 * Remove all contexts from stack. Similar to sequence of QBApplicationControllerTakeContext (does not switch current context)
 *
 * @param[in] self application controller handle
 */
void QBApplicationControllerCleanContextStack(QBApplicationController self);
/**
 * Add new context on top of current one and switch to it.
 *
 * This method switches between current window context and @a ctx,
 * pushing @a ctx on top of the stack.
 *
 * @note In the future when @a ctx is popped from the stack the same switcher
 * will be used as the one used originally to switch to @a ctx.
 *
 * @param[in] self application controller handle
 * @param[in] ctx context to be pushed on top of the stack
 **/
void QBApplicationControllerPushContext(QBApplicationController self, QBWindowContext ctx);

/**
 * Add new context on top of current one and switch to it.
 *
 * This method is another version of QBApplicationControllerPushContext(),
 * but it allows to explicitly specify context switcher to be used for
 * transition from current context to @c ctx and back.
 *
 * @param[in] self application controller handle
 * @param[in] ctx context to be pushed on top of the stack
 * @param[in] switcher handle to an object implementing @ref QBContextSwitcher
 **/
void QBApplicationControllerPushContextWithSwitcher(QBApplicationController self, QBWindowContext ctx, SvObject switcher);

/**
 * Remove current context and switch to previous one.
 *
 * @param[in] self application controller handle
 **/
void QBApplicationControllerPopContext(QBApplicationController self);

/**
 * Remove current context and switch to previous one.
 *
 * This method is another version of QBApplicationControllerPopContext(),
 * but it allows to explicitly specify context switcher to be used for
 * transition from current context to previous one.
 *
 * @param[in] self application controller handle
 * @param[in] switcher handle to an object implementing @ref QBContextSwitcher
 **/
void QBApplicationControllerPopContextWithSwitcher(QBApplicationController self, SvObject switcher);

/**
 * Flushes context history and switches current context to the first one.
 * @param[in] self application controller handle
 */
void QBApplicationControllerSwitchToRoot(QBApplicationController self);

/**
 * Similar to push, but doesn't automaticaly switch. Immediately after calling a sequence of
 * QBApplicationControllerPutContext(), QBApplicationControllerPutContextWithSwitcher(),
 * QBApplicationControllerTakeContext() a call to one of QBApplicationControllerPushContext(),
 * QBApplicationControllerPushContextWithSwitcher(), QBApplicationControllerPopContext(),
 * QBApplicationControllerPopContextWithSwitcher() must be called.
 * @param[in] self application controller handle
 * @param[in] ctx context to be put
 */
void QBApplicationControllerPutContext(QBApplicationController self, QBWindowContext ctx);

/**
 * Allows for additional context switcher
 * @param[in] self application controller handle
 * @param[in] ctx context to be put
 * @param[in] switcher which switcher should be used to pop this context
 */
void QBApplicationControllerPutContextWithSwitcher(QBApplicationController self, QBWindowContext ctx, SvObject switcher);

/**
 * Variant of QBApplicationControllerPopContext() that doesn't automaticaly switch.
 *
 * @see QBApplicationControllerPopContext()
 * @param[in] self application controller handle
 */
void QBApplicationControllerTakeContext(QBApplicationController self);

/**
 * Replaces bottom context with a new one.
 *
 * @param[in] self application controller handle
 * @param[in] ctx context to be put on bottom
 */
void QBApplicationControllerSwitchBottom(QBApplicationController self, QBWindowContext ctx);

/**
 * Allows for additionaly specifing context switcher
 * @see QBApplicationControllerSwitchBottom()
 *
 * @param[in] self application controller handle
 * @param[in] ctx context to be put on bottom
 * @param[in] switcher handle to an object implementing @ref QBContextSwitcher
 */
void QBApplicationControllerSwitchBottomWithSwitcher(QBApplicationController self, QBWindowContext ctx, SvObject switcher);

/**
 * Replaces current context with a new one.
 *
 * @param[in] self application controller handle
 * @param[in] ctx context to be shown
 */
void QBApplicationControllerSwitchTop(QBApplicationController self, QBWindowContext ctx);

/**
 * Allows for additionaly specifing context switcher
 * @see QBApplicationControllerSwitchTop()
 * @param[in] self application controller handle
 * @param[in] ctx context to be shown
 * @param[in] switcher handle to an object implementing @ref QBContextSwitcher
 */
void QBApplicationControllerSwitchTopWithSwitcher(QBApplicationController self, QBWindowContext ctx, SvObject switcher);

/**
 * Adds a persistent context.
 * @param[in] self application controller handle
 * @param[in] ctx persistent context
 */
void QBApplicationControllerAddPersistentContext(QBApplicationController self, QBWindowContext ctx);

/**
 * Removes a persistent context.
 * @param[in] self application controller handle
 * @param[in] ctx persistent context
 */
void QBApplicationControllerRemovePersistentContext(QBApplicationController self, QBWindowContext ctx);

/**
 * Retrives an persistent context with name @a name.
 * @param[in] self application controller handle
 * @param[in] name persistent context name
 * @return found persistent context or @c NULL if context wasn't found
 */
QBWindowContext QBApplicationControllerGetPersistentContext(QBApplicationController self, SvString name);

/**
 * Returns current context.
 * @param[in] self QBApplicationController handle
 * @return current context
 */
QBWindowContext QBApplicationControllerGetCurrentContext(QBApplicationController self);

/**
 * Using supplied QBLocalWindowManager adds @a ctx as a local window.
 * @param[in] self application controller handle
 * @param[in] ctx local window to be added
 */
void QBApplicationControllerAddLocalWindow(QBApplicationController self, QBLocalWindow ctx);

/**
 * Removes supplied local window.
 * @param[in] self application controller handle
 * @param[in] ctx local window to be removed
 */
void QBApplicationControllerRemoveLocalWindow(QBApplicationController self, QBLocalWindow ctx);

/**
 * Using supplied QBGlobalWindowManager adds @a ctx as a global window.
 * @param[in] self application controller handle
 * @param[in] ctx global window to be added
 * @return @c true window was added
 */
bool QBApplicationControllerAddGlobalWindow(QBApplicationController self, QBGlobalWindow ctx);

/**
 * Removes supplied global window.
 * @param[in] self application controller handle
 * @param[in] ctx global window to be removed
 */
void QBApplicationControllerRemoveGlobalWindow(QBApplicationController self, QBGlobalWindow ctx);
/**
 * Queries whether it is possible to add a window with name @a name.
 * @param[in] self application controller handle
 * @param[in] name global window name
 * @return @c true if window can be added to current context
 */
bool QBApplicationControllerCanAddGlobalWindowWithName(QBApplicationController self, SvString name);

/**
 * Whenever a huge change in application happens (resolution/language change) this functions
 * should be called update to new settings all visible windows and all presistent windows.
 * @param[in] self application controller handle
 * @param[in] requestors list of requestors as SvString
 */
void QBApplicationControllerReinitializeWindows(QBApplicationController self, SvArray requestors);

/**
 * Queries whether windows are reinitializing.
 * @see QBApplicationControllerReinitializeWindows()
 * @param[in] self application controller handle
 * @return @c true iff self is still reinitializing windows
 */
bool QBApplicationControllerReinitializingWindows(QBApplicationController self);

/**
 * Queries if there is any context switch in progress.
 * @param[in] self application controller handle
 * @return true iff there is any context switch in progress
 */
bool QBApplicationControllerIsSwitchInProgress(QBApplicationController self);

/**
 * Adds a listener, which will be notified about all context changes.
 * @param[in] self application controller handle
 * @param[in] obj  listener handle implementing QBApplicationControllerListener interface
 */
void QBApplicationControllerAddListener(QBApplicationController self, SvObject obj);

/**
 * Removes listener @see QBApplicationControllerAddListener()
 * @param[in] self application controller handle
 * @param[in] obj  listener handle implementing QBApplicationControllerListener interface
 */
void QBApplicationControllerRemoveListener(QBApplicationController self, SvObject obj);

/**
 * Adds a input listener, which will be notified about received input.
 * @param[in] self      application controller handle
 * @param[in] listener  listener handle implementing QBApplicationControllerInputListener interface
 **/
void QBApplicationControllerAddInputListener(QBApplicationController self, SvObject listener);

/**
 * Removes input listener @see QBApplicationControllerAddInputListener()
 * @param[in] self      application controller handle
 * @param[in] listener  listener handle implementing QBApplicationControllerInputListener interface
 **/
void QBApplicationControllerRemoveInputListener(QBApplicationController self, SvObject listener);

/**
* Get runtime type identification object representing
* QBApplicationControllerInputListener interface.
*
* @return QBApplicationControllerInputListener interface object
**/
SvInterface QBApplicationControllerInputListener_getInterface(void);

/**
 * Register global input handler.
 * @param[in] self      QBApplicationController handle
 * @param[in] handler   global input handler
 * @param[in] param     global input handler parameter
 */
void QBApplicationControllerRegisterInputHandler(QBApplicationController self, QBApplicationControllerInputEventHandler handler, void* param);

/**
 * @}
 * @}
 **/

#endif
