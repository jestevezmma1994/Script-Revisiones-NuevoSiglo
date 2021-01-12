/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_REMOTE_SESSION_MANAGER_H_
#define QB_REMOTE_SESSION_MANAGER_H_

/**
 * @file QBRemoteSessionManager.h Remote session manager class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBDataProvider/QBDataProviderTypes.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvSet.h>
#include <SvFoundation/SvDeque.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>

/**
 * @defgroup QBRemoteSessionManager Remote session manager
 * @ingroup QBDataProvider
 * @{
 **/

/**
 * Default reauthentication period (in seconds), used when next
 * authentication time is not known.
 */
#define QB_REMOTE_SESSION_DEFAULT_REAUTH_PERIOD (60 * 60)

/**
 * The time (in seconds) after the pending requests will be canceled.
 */
#define QB_REMOTE_SESSION_REQUEST_TIMEOUT (120)

/**
 * QBRemoteSessionManager class internals.
 **/
struct QBRemoteSessionManager_ {
    struct SvObject_ super_;                  ///< super class

    SvURL serverURL;                          ///< server URL
    SvSSLParams sslParams;                    ///< ssl params

    SvDeque pendingRequests;                  ///< pending requests
    size_t maxPendingRequestsCnt;             ///< max pending requests count

    SvSet ongoingRequests;                    ///< ongoing requests
    size_t maxOngoingRequestsCnt;             ///< max ongoing requests count

    SvSet processedRequests;                  ///< transfer finished but not parsed yet

    SvHTTPClientEngine clientEngine;          ///< http client engine
    size_t httpEngineRequestsLimit;           ///< http engine requests limit

    SvFiber fiber;                            ///< fiber
    SvFiberTimer authTimer;                   ///< authentication timer

    QBRemoteSessionAuthPlugin authPlugin;     ///< authentication plugin
    QBRemoteSessionAuthStatus lastAuthStatus; ///< last authentication status
    size_t failedAuthsCnt;                    ///< count of failed authentications

    bool started;                             ///< @c true if session manager
                                              ///< is started, @c false otherwise
    bool handshakeDone;                       ///< @c true if handshake is done,
                                              ///< @c false otherwise
};


/**
 * QBRemoteSessionManager virtual methods table.
 **/
typedef const struct QBRemoteSessionManagerVTable_ {
    /**
     * virtual methids of the base class
     **/
    struct SvObjectVTable_ super_;

    /**
     * Perform handshake.
     *
     * @private
     *
     * Do not call this function directly. It's called internally just after
     * session manager is started, but before authentication phase. Default
     * implementation does nothing, derived class can implement it to provide
     * custom handshake phase.
     *
     * @param[in] self_ QBRemoteSessionManager handle
     **/
    void (*performHandshake)(QBRemoteSessionManager self_);
} *QBRemoteSessionManagerVTable;

/**
 * Get runtime type identification object representing
 * type of QBRemoteSessionManager class.
 *
 * @relates QBRemoteSessionManager
 *
 * @return QBRemoteSessionManager class
 **/
SvType
QBRemoteSessionManager_getType(void);

/**
 * Initialize  QBRemoteSessionManager.
 *
 * @public @memberof QBRemoteSessionManager
 *
 * Use this function to initialize QBRemoteSessionManager inside derived
 * class constructor.
 *
 * @param[in]  self                    QBRemoteSessionManager instance
 * @param[in]  serverURL               server URL
 * @param[in]  authPlugin              authentication plugin, if @c NULL each
 *                                     request will be treated as permamently
 *                                     authenticated
 * @param[in]  maxPendingRequestsCnt   max size of pending requests queue
 * @param[in]  maxOngoingRequestsCnt   max count of ongoing requests
 * @param[in]  httpEngineRequestsLimit max count of requests being processed by
 *                                     HTTP engine
 * @param[out] errorOut                error info
 **/
void
QBRemoteSessionManagerInit(QBRemoteSessionManager self,
                           SvURL serverURL,
                           QBRemoteSessionAuthPlugin authPlugin,
                           size_t maxPendingRequestsCnt,
                           size_t maxOngoingRequestsCnt,
                           size_t httpEngineRequestsLimit,
                           SvErrorInfo *errorOut);

/**
 * Create an instance of QBRemoteSessionManager.
 *
 * @public @memberof QBRemoteSessionManager
 *
 * @param[in]  serverURL               server URL
 * @param[in]  authPlugin              authentication plugin, if @c NULL each
 *                                     request will be treated as permamently
 *                                     authenticated
 * @param[in]  maxPendingRequestsCnt   max size of pending requests queue
 * @param[in]  maxOngoingRequestsCnt   max count of ongoing requests
 * @param[in]  httpEngineRequestsLimit max count of requests being processed by
 *                                     HTTP engine
 * @param[out] errorOut                error info
 * @return                             QBRemoteSessionManager instance,
 *                                     @c NULL in case of error
 **/
QBRemoteSessionManager
QBRemoteSessionManagerCreate(SvURL serverURL,
                             QBRemoteSessionAuthPlugin authPlugin,
                             size_t maxPendingRequestsCnt,
                             size_t maxOngoingRequestsCnt,
                             size_t httpEngineRequestsLimit,
                             SvErrorInfo *errorOut);

/**
 * Start the session manager.
 *
 * @public @memberof QBRemoteSessionManager
 *
 * @param[in]  self      QBRemoteSessionManager handle
 * @param[in]  scheduler scheduler to be used for all asynchronous tasks
 * @param[out] errorOut  error info
 **/
void
QBRemoteSessionManagerStart(QBRemoteSessionManager self,
                            SvScheduler scheduler,
                            SvErrorInfo *errorOut);

/**
 * Stop the session manager.
 *
 * @public @memberof QBRemoteSessionManager
 *
 * @param[in]  self     QBRemoteSessionManager handle
 * @param[out] errorOut error info
 **/
void
QBRemoteSessionManagerStop(QBRemoteSessionManager self,
                           SvErrorInfo *errorOut);

/**
 * Set session ssl params.
 *
 * @public @memberof QBRemoteSessionManager
 *
 * @param[in]  self      QBRemoteSessionManager handle
 * @param[in]  sslParams SSL parameters
 * @param[out] errorOut  error info
 **/
void
QBRemoteSessionManagerSetSSLParams(QBRemoteSessionManager self,
                                   SvSSLParams sslParams,
                                   SvErrorInfo *errorOut);

/**
 * Get server URL.
 *
 * @public @memberof QBRemoteSessionManager
 *
 * @param[in] self      QBRemoteSessionManager handle
 * @return              server URL, @c NULL if unavailable
 **/
SvURL
QBRemoteSessionManagerGetServerURL(QBRemoteSessionManager self);

/**
 * Get session authentication status.
 *
 * @public @memberof QBRemoteSessionManager
 *
 * @param[in] self QBRemoteSessionManager handle
 * @return         authentication status
 **/
QBRemoteSessionAuthStatus
QBRemoteSessionManagerGetAuthStatus(QBRemoteSessionManager self);

/**
 * Notify session manager that handshake is finished.
 *
 * @protected @memberof QBRemoteSessionManager
 *
 * @param[in]  self     QBRemoteSessionManager handle
 * @param[out] errorOut error info
 **/
void
QBRemoteSessionManagerHandshakeFinished(QBRemoteSessionManager self,
                                        SvErrorInfo *errorOut);
/**
 * @}
 **/

#endif /* QB_REMOTE_SESSION_MANAGER_H_ */
