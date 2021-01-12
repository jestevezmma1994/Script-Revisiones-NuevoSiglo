/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef TRAXIS_WEB_AUTH_PLUGIN_H_
#define TRAXIS_WEB_AUTH_PLUGIN_H_

/**
 * @file TraxisWebAuthPlugin.h
 * @brief Traxis.Web authentication plugin
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <TraxisWebClient/TraxisWebAuthStatus.h>
#include <TraxisWebClient/TraxisWebAuthRequest.h>
#include <TraxisWebClient/TraxisWebSessionManager.h>


/**
 * @defgroup TraxisWebAuthPlugin Traxis.Web authentication plugin interface
 * @ingroup TraxisWebClient
 * @{
 **/

/**
 * Traxis.Web authentication plugin interface.
 **/
typedef const struct TraxisWebAuthPlugin_ {
    /**
     * Get current authentication status.
     *
     * @param[in] self_     handle to an object implementing @ref TraxisWebAuthPlugin
     * @param[in] manager   Traxis.Web session manager handle
     * @return              authentication status
     **/
    TraxisWebAuthStatus (*getStatus)(SvObject self_,
                                     TraxisWebSessionManager manager);

    /**
     * Start authentication process.
     *
     * @param[in] self_     handle to an object implementing @ref TraxisWebAuthPlugin
     * @param[in] manager   Traxis.Web session manager handle
     * @param[in] errorOut  error info
     * @return              authentication status
     **/
    TraxisWebAuthRequest (*authenticate)(SvObject self_,
                                         TraxisWebSessionManager manager,
                                         SvErrorInfo *errorOut);

    /**
     * Start the plugin and all subordinate tasks
     *
     * @param[in] self_     handle to an object implementing @ref TraxisWebAuthPlugin
     * @param[in] manager   Traxis.Web session manager handle
     **/
    void (*start)(SvObject self_,
                  TraxisWebSessionManager manager);

    /**
     * Stop all ongoing asynchronous tasks.
     *
     * @param[in] self_     handle to an object implementing @ref TraxisWebAuthPlugin
     **/
    void (*stop)(SvObject self_);
} *TraxisWebAuthPlugin;

/**
 * Get runtime type identification object representing
 * TraxisWebAuthPlugin interface.
 *
 * @return TraxisWebAuthPlugin interface object
 **/
extern SvInterface
TraxisWebAuthPlugin_getInterface(void);

/**
 * @}
 **/


/**
 * @defgroup TraxisWebDummyAuthPlugin Traxis.Web dummy authentication plugin
 * @ingroup TraxisWebClient
 * @{
 **/

/**
 * Traxis.Web dummy authentication plugin class.
 * @class TraxisWebDummyAuthPlugin
 * @extends SvObject
 **/
typedef struct TraxisWebDummyAuthPlugin_ *TraxisWebDummyAuthPlugin;

/**
 * Create dummy Traxis.Web authentication plugin.
 *
 * @memberof TraxisWebDummyAuthPlugin
 *
 * @param[out] errorOut error info
 * @return              created plugin, @c NULL in case of error
 **/
extern TraxisWebDummyAuthPlugin
TraxisWebDummyAuthPluginCreate(SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
