/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_MW_CONFIG_HANDLER_H
#define QB_MW_CONFIG_HANDLER_H

/**
 * @file QBMWConfigHandlerPlugin.h
 * @brief Cubiware MW config handler.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 *
 **/

#include <main_decl.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBMWConfigHandler Cubiware MW Config Handler
 * @ingroup CubiTV_middleware_support
 * @{
 *
 * It is a listener of the QBMWConfigMonitor. It handles some generic
 * config settings that come from the Cubiware MW. It also passes
 * all config pairs (key, value) to it's plugins.
 *
 **/

/**
 * @brief QBMWConfigHandler type
 */
typedef struct QBMWConfigHandler_* QBMWConfigHandler;

/** Config state change */
typedef enum config_state_e {
    CONFIG_STATE_READ_CHANGE_NO_FIRSTBOOT, /**< Config has changed in a way that requires reboot without setting Firstboot flag*/
    CONFIG_STATE_READ_CHANGE,        /**< Config state changed */
    CONFIG_STATE_READ_NOCHANGE,      /**< Config state not changed */
    CONFIG_STATE_NOREAD              /**< Can't read config from middleware */
} config_state_t;

/**
 * Register new observer listener.
 *
 * @param[in] self      observer handle
 * @param[in] listener  handle to an object implementing
 *                      QBObserver interface
 **/
void
QBMWConfigHandlerAddListener(QBMWConfigHandler self, SvObject listener);

/**
 * Remove previously registered observer listener.
 *
 * @param[in] self      observer handle
 * @param[in] listener  handle to previously registered listener
 **/
void
QBMWConfigHandlerRemoveListener(QBMWConfigHandler self, SvObject listener);

/**
 * Create Cubiware MW config handler
 * @param[in] appGlobals        AppGlobals handle
 * @return                      created config handler or NULL
 */
QBMWConfigHandler
QBMWConfigHandlerCreate(AppGlobals appGlobals);

/**
 * Start Cubiware MW config handler
 * @param[in] self      handle
 */
void
QBMWConfigHandlerStart(QBMWConfigHandler self);

/**
 * Stop Cubiware MW config handler
 * @param[in] self      handle
 */
void
QBMWConfigHandlerStop(QBMWConfigHandler self);

/**
 * Add new plugin to the Cubiware MW config handler
 *
 * @param[in] self          handle
 * @param[in] newPlugin     plugin object
 * @param[in] pluginName    plugin name
 */
void
QBMWConfigHandlerAddPlugin(QBMWConfigHandler self, SvGenericObject newPlugin, SvString pluginName);

/**
 * Return plugin found by it's name
 * @param[in] self          handle
 * @param[in] pluginName    plugin name to be found
 * @return                  found plugin handle or NULL
 */
SvGenericObject
QBMWConfigHandlerGetPluginByName(QBMWConfigHandler self, SvString pluginName);

/**
 * @}
 **/

#endif // QB_MW_CONFIG_HANDLER_H
