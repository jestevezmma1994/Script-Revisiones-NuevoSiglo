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

#ifndef QB_CONTEXT_SWITCHER_H_
#define QB_CONTEXT_SWITCHER_H_

/**
 * @file QBContextSwitcher.h QBContextSwitcher interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvWeakList.h>
#include <QBApplicationControllerTypes.h>
#include <QBWindowContext.h>

/**
 * @defgroup QBContextSwitcher Context switcher interface
 * @ingroup QBApplicationControllerLibrary
 * @{
 *
 * An interface for classes that implement top-level windows switching.
 *
 * Implementations of the context switcher interface switch between two top-level
 * windows, represented by their contexts, usually employing some nice visual effects.
 * They need to be written extremely carefully, as there are many corner cases
 * (see the source code of the @ref QBContextSwitcherStandard for reference).
 **/

/**
 * QBContextSwitcher interface.
 **/
typedef struct QBContextSwitcher_t {
    /**
     * Start switch between two top-level windows, from the one represented
     * by @a ctx1 to the one represented by @a ctx2.
     *
     * @param[in] self_     context switcher handle
     * @param[in] ctx1      context of the first window
     * @param[in] ctx2      context of the second window
     * @param[in] ctxPushed @c true when context is pushed on stack, @c false othwerwise
     **/
    void (*performSwitch)(SvObject self_, QBWindowContext ctx1, QBWindowContext ctx2, bool ctxPushed);

    /**
     * Immediately finish current switch process.
     *
     * @param[in] self_     context switcher handle
     **/
    void (*finish)(SvObject self_);

    /**
     * Set a set of listeners for the switcher.
     *
     * @param[in] self_     context switcher handle
     * @param[in] listeners a list of listeners to be used by the switcher
     **/
    void (*setListeners)(SvObject self_, SvWeakList listeners);

    /**
     *
     * Query if there are any planned or current context switches
     *
     * @param [in] self_    constxt switcher handle
     * @return true iff a context switch is in progres or there are any planned context switces
     */
    bool (*isFinished)(SvObject self_);

    /**
     * Query if given context is currently visible
     *
     * @param [in] self_    context switcher handle
     * @param [in] ctx      context to check
     * @return true if given context is currently visible
     */
    bool (*isVisible)(SvObject self_, QBWindowContext ctx);
} *QBContextSwitcher;

/**
 * Get runtime type identification object representing QBContextSwitcher interface.
 * @return Context switcher interface identification object
 **/
SvInterface QBContextSwitcher_getInterface(void);


/**
 * QBContextSwitcherListener interface.
 **/
typedef struct QBContextSwitcherListener_t {
    /**
     * Notifies that windows switch has started.
     *
     * @param[in] self_     listener handle
     * @param[in] from      context of the window from which we are switching
     * @param[in] to        context of the window to which we are switching
     **/
    void (*started)(SvObject self_, QBWindowContext from, QBWindowContext to);

    /**
     * Notifies that windows switch has finished.
     *
     * @param[in] self_     listener handle
     * @param[in] from      context of the window from which we were switching
     * @param[in] to        context of the window to which we were switching
     **/
    void (*ended)(SvObject self_, QBWindowContext from, QBWindowContext to);
} *QBContextSwitcherListener;

/**
 * Get runtime type identification object representing QBContextSwitcherListener interface.
 * @return Context switcher listener interface identification object
 **/
SvInterface QBContextSwitcherListener_getInterface(void);


/**
 * @}
 **/

#endif
