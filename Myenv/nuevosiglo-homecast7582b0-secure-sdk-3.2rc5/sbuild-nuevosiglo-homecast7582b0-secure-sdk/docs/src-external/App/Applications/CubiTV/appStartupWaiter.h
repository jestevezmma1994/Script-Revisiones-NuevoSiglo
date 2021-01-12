/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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


#ifndef QBAPPSTARTUPWAITER_H_
#define QBAPPSTARTUPWAITER_H_

/**
 * @file appStartupWaiter.h
 * @brief CubiTV startup waiter context
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 *
 **/

#include <QBApplicationController.h>
#include <main_decl.h>
#include <stdbool.h>

/**
 * @defgroup AppStartupWaiter CubiTV startup waiter context
 * @ingroup CubiTV_windows
 *
 * Aplication at startup has to load not only local resources, which can be
 * accessed immediately, but also resources that need to be obtained from
 * the outside (some middleware server or DVB network).
 *
 * In order to not keep the user in the dark we need to show this screen
 * that will inform him what is happening.
 *
 * The dependency on external data is highly project specific, thus this
 * context merely implements the widgets but does not implement the waiting logic.
 * It is implemented by a plugin that is provided on start, the context
 * uses QBAppStartupWaiterPlugin interface to obtain current status.
 **/

/**
 * QBAppStartupWaiter plugin interface
 */
struct QBAppStartupWaiterPlugin_ {
    /**
     * Check if plugin is still working. This callback is called in every QBAppStartupWaiter fiber step.
     *
     * @param[in]  self_        handle to object implementing plugin interface
     * @return                  @c true if plugin has finished its job, @c false otherwise
     */
    bool (*hasFinished)(SvGenericObject self_);

    /**
     * Requests the plugin to start working, only after calling this function hasError and hasFinished
     * may be called.
     *
     * @param[in] self_         handle to object implementing plugin interface
     **/
    void (*start)(SvGenericObject self_);

    /**
     * Requests that the plugin stops working.
     *
     * @param[in] self_         handle to object implementing plugin interface
     **/
    void (*stop)(SvGenericObject self_);
};
typedef struct QBAppStartupWaiterPlugin_ *QBAppStartupWaiterPlugin;

/**
 * QBAppStartupWaiter window context opaque structure
 * */
typedef struct QBAppStartupWaiterContext_t* QBAppStartupWaiterContext;

/**
 * Get runtime type identification object representing
 * QBAppStartupWaiterPlugin interface.
 *
 * @return QBAppStartupWaiterPlugin interface object
 */
SvInterface
QBAppStartupWaiterPlugin_getInterface(void);

/**
 * Create QBAppStartupWaiter window context object
 *
 * @param[in] appGlobals CubiTV globals
 * @return               QBAppStartupWaiter window context object
 */
QBWindowContext
QBAppStartupWaiterContextCreate(AppGlobals appGlobals);

/**
 * Add waiter plugin to QBAppStartupWaiter window context object
 *
 * @param[in] self pointer to QBAppStartupWaiter window context object
 * @param[in] plugin plugin object (which must implement QBAppStartupWaiterPlugin interface) to be added
 */
void
QBAppStartupWaiterContextAddPlugin(QBAppStartupWaiterContext self, SvGenericObject plugin);

/**
 * @}
 **/

#endif
