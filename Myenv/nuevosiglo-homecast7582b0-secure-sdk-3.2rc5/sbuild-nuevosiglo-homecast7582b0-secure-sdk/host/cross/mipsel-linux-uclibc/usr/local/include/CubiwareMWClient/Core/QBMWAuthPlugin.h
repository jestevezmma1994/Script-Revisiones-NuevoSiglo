/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_WEB_AUTH_PLUGIN_H_
#define QB_WEB_AUTH_PLUGIN_H_

/**
 * @file QBMWAuthPlugin.h
 * @brief Cubiware Middleware authentication plugin
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


#include <CubiwareMWClient/Core/QBMWAuthStatus.h>
#include <CubiwareMWClient/Core/QBMWSessionManager.h>
#include <CubiwareMWClient/Core/QBMWRequest.h>

#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>

#include <stdbool.h>

/**
 * @defgroup QBMWAuthPlugin QB Middleware authentication plugin interface
 * @ingroup QBMWClient
 * @{
 **/

/**
 * Cubiware Middleware authentication plugin interface.
 *
 * @interface QBMWAuthPlugin QBMWAuthPlugin.h <CubiwareMWClient/Core/QBMWAuthPlugin.h>
 *
 * @see QBMWSessionManagerCreate
 **/
typedef const struct QBMWAuthPlugin_ {
    /**
     * Get current authentication status.
     *
     * @memberof QBMWAuthPlugin
     *
     * @param[in] self_     handle to an object implementing @ref QBMWAuthPlugin
     * @param[in] manager   QB Middleware session manager handle
     * @return              authentication status
     **/
    QBMWAuthStatus (*getStatus)(SvObject self_,
                                QBMWSessionManager manager);

    /**
     * Start authentication process.
     *
     * @memberof QBMWAuthPlugin
     *
     * When authentication process is finished, object implementing
     * QBMWAuthPlugin interface should notify listeners regardless
     * of the outcome of the operation (@ref QBMWAuthPluginListener::authStatusChanged)
     *
     * @param[in]  self_     handle to an object implementing @ref QBMWAuthPlugin
     * @param[in]  manager   QB Middleware session manager handle
     * @param[out] errorOut  error info
     **/
    void (*authenticate)(SvObject self_,
                         QBMWSessionManager manager,
                         SvErrorInfo *errorOut);

    /**
     * Prepare @ref QBMWRequest before send to middleware server.
     *
     * @memberof QBMWAuthPlugin
     *
     * Set here all necessary query params and HTTP headers.
     *
     * @param[in]  self_    handle to an object implementing @ref QBMWAuthPlugin
     * @param[in]  request  request to prepare
     * @param[out] errorOut error info
     **/
    void (*prepareRequest)(SvObject self_,
                           QBMWRequest request,
                           SvErrorInfo *errorOut);

    /**
     * Stop all ongoing asynchronous tasks.
     *
     * @memberof QBMWAuthPlugin
     *
     * @param[in] self_     handle to an object implementing @ref QBMWAuthPlugin
     **/
    void (*stop)(SvObject self_);

    /**
     * Add listener to authentication plugin.
     *
     * @memberof QBMWAuthPlugin
     *
     * @see QBMWAuthPluginListener
     *
     * @param[in]  self_    handle to an object implementing @ref QBMWAuthPlugin
     * @param[in]  listener handle to an object implementing @ref QBMWAuthPluginListener
     * @param[out] errorOut error ingo
     */
    void (*addListener)(SvObject self_,
                        SvObject listener,
                        SvErrorInfo *errorOut);

    /**
     * Remove listener from authentication plugin.
     * @see QBMWAuthPluginListener
     *
     * @param[in]  self_    handle to an object implementing @ref QBMWAuthPlugin
     * @param[in]  listener handle to an object implementing @ref QBMWAuthPluginListener
     * @param[out] errorOut error ingo
     */
    void (*removeListener)(SvObject self_,
                           SvObject listener,
                           SvErrorInfo *errorOut);
} *QBMWAuthPlugin;

/**
 * Get runtime type identification object representing
 * QBMWAuthPlugin interface.
 *
 * @relates QBMWAuthPlugin
 *
 * @return QBMWAuthPlugin interface object
 **/
extern SvInterface
QBMWAuthPlugin_getInterface(void);

/**
 * @}
 **/

/**
 * @defgroup QBMWAuthPluginListener QB Middleware authentication plugin listener interface
 * @ingroup QBMWAuthPlugin
 * @{
 **/

/**
 * Cubiware Middleware authentication plugin listener interface.
 *
 * @interface QBMWAuthPluginListener QBMWAuthPlugin.h <CubiwareMWClient/Core/QBMWAuthPlugin.h>
 *
 * Register object implementing this interface using @ref QBMWAuthPlugin::addListener.
 *
 * @see QBMWAuthPlugin
 **/
typedef const struct QBMWAuthPluginListener_ {
    /**
     * Notify about finished authentication.
     *
     * @memberof QBMWAuthPluginListener
     *
     * Use @ref QBMWAuthPlugin::getStatus to check the result.
     *
     * @param[in] self_     handle to an object implementing @ref QBMWAuthPluginListener
     **/
    void (*authStatusChanged)(SvObject self_);

} *QBMWAuthPluginListener;

/**
 * Get runtime type identification object representing
 * QBMWAuthPluginListener interface.
 *
 * @relates QBMWAuthPluginListener
 *
 * @return QBMWAuthPluginListener interface object
 **/
extern SvInterface
QBMWAuthPluginListener_getInterface(void);
/**
 * @}
 **/

/**
 * @defgroup QBMWDummyAuthPlugin Cubiware Middleware dummy authentication plugin
 * @ingroup QBMWClient
 * @{
 **/

/**
 * Cubiware Middleware dummy authentication plugin class.
 *
 * @class QBMWDummyAuthPlugin QBMWAuthPlugin.h <CubiwareMWClient/Core/QBMWAuthPlugin.h>
 * @extends SvObject
 * @implements QBMWAuthPlugin
 *
 * This QBMWAuthPlugin implementation is ued if no other authentication plugin
 * is provided to session manager.
 * @see QBMWSessionManagerCreate
 **/
typedef struct QBMWDummyAuthPlugin_ *QBMWDummyAuthPlugin;

/**
 * Create dummy Cubiware Middleware authentication plugin.
 *
 * @memberof QBMWDummyAuthPlugin
 *
 * @param[out] errorOut error info
 * @return              created plugin, @c NULL in case of error
 **/
extern QBMWDummyAuthPlugin
QBMWDummyAuthPluginCreate(SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
