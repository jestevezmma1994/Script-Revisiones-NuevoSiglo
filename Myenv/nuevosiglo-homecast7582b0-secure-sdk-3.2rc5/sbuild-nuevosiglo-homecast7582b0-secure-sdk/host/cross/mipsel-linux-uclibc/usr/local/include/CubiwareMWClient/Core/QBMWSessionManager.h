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

#ifndef QB_WEB_SESSION_MANAGER_H_
#define QB_WEB_SESSION_MANAGER_H_

/**
 * @file QBMWSessionManager.h
 * @brief Cubiware middleware session manager class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CubiwareMWClient/Core/QBMWAuthStatus.h>

#include <SvHTTPClient/SvSSLParams.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>

#include <stddef.h>
#include <stdbool.h>

/**
 * @defgroup QBMWSessionManager Cubiware middleware session manager class
 * @ingroup QBMWClient
 * @{
 **/

enum {
    /**
     * Default reauthentication period (in seconds), used when next
     * authentication time is not known.
     */
    QBMWSESSION_DEFAULT_REAUTH_PERIOD = 60*60,
    /**
     * The time (in seconds) after which the pending requests will be canceled.
     */
    QBMWSESSION_REQUEST_TIMEOUT = 120,
};

/**
 * Cubiware middleware session manager class.
 *
 * @class QBMWSessionManager QBMWSessionManager.h <CubiwareMWClient/Core/QBMWSessionManager.h>
 * @extends SvObject
 * @implements QBMWAuthPluginListener
 *
 * This class creates and maintains middleware server session. Session
 * management is done using authentication plugin (an object implementing
 * @ref QBMWAuthPlugin interface).
 *
 * It also supports sending requests to the server. Each request
 * (@ref QBMWRequest) is authenticated before sending. If session is
 * not established, all new requests are added to pending requests list.
 * As soon as the session is established, all (except outstanding,
 * @ref QBMWSESSION_REQUEST_TIMEOUT) requests are immidiately sent to
 * the server.
 *
 **/
typedef struct QBMWSessionManager_ *QBMWSessionManager;

/**
 * Get runtime type identification object representing
 * type of middleware session manager class.
 *
 * @relates QBMWSessionManager
 *
 * @return QBMWSessionManager class
 **/
extern SvType
QBMWSessionManager_getType(void);

/**
 * Create QBMWSession manager.
 *
 * @memberof QBMWSessionManager
 *
 * @see QBMWAuthPlugin
 * @see QBMWDummyAuthPlugin
 *
 * @param[in]  serverURL  middleware server URL
 * @param[in]  authPlugin authentication Plugin, if @c NULL, QBMWDummyAuthPlugin will be used
 * @param[out] errorOut   error info
 * @return                created session manager, @c NULL in case of error
 **/
extern QBMWSessionManager
QBMWSessionManagerCreate(SvURL serverURL,
                         SvObject authPlugin,
                         SvErrorInfo *errorOut);

/**
 * Get authentication status.
 *
 * @memberof QBMWSessionManager
 *
 * @param[in] self      QBMWSessionManager handle
 * @return              authentication status
 **/
extern QBMWAuthStatus
QBMWSessionManagerGetAuthStatus(QBMWSessionManager self);

/**
 * Get server URL.
 *
 * @memberof QBMWSessionManager
 *
 * @param[in] self      QBMWSession manager handle
 * @return              server URL, @c NULL if unavailable
 **/
extern SvURL
QBMWSessionManagerGetServerURL(QBMWSessionManager self);

/**
 * Set SSL parameters for HTTPS communication with middleware server.
 *
 * @memberof QBMWSessionManager
 *
 * @param[in]  self      QBMWSession manager handle
 * @param[in]  sslParams SSL parameters
 * @param[out] errorOut  error info
 **/
extern void
QBMWSessionManagerSetSSLParams(QBMWSessionManager self,
                               SvSSLParams sslParams,
                               SvErrorInfo *errorOut);

/**
 * Start the session.
 *
 * @memberof QBMWSessionManager
 *
 * @param[in] self      QBMWSessionManager handle
 * @param[in] scheduler scheduler to be used for entire QBMWSessionManager
 * @param[out] errorOut error info
 **/
extern void
QBMWSessionManagerStart(QBMWSessionManager self,
                        SvScheduler scheduler,
                        SvErrorInfo *errorOut);

/**
 * Stop the session.
 *
 * @note This method will cancel all ongoing requests.
 *
 * @memberof QBMWSessionManager
 *
 * @param[in] self      QBMWSessionManager handle
 * @param[out] errorOut error info
 **/
extern void
QBMWSessionManagerStop(QBMWSessionManager self,
                       SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif
