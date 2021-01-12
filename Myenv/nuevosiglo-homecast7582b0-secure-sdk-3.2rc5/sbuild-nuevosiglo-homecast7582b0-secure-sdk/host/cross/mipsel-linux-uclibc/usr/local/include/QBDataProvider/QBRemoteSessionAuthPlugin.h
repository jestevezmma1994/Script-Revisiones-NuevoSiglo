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

#ifndef QB_REMOTE_SESSION_AUTH_PLUGIN_H_
#define QB_REMOTE_SESSION_AUTH_PLUGIN_H_

/**
 * @file QBRemoteSessionAuthPlugin.h Remote session authentication plugin class API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBDataProvider/QBRemoteDataRequest.h>
#include <QBDataProvider/QBDataProviderTypes.h>
#include <QBAppKit/QBObservable.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBRemoteSessionAuthPlugin Remote session authentication plugin class.
 * @ingroup QBDataProvider
 * @{
 **/

/**
 * QBRemoteSessionAuthPlugin class internals.
 **/
struct QBRemoteSessionAuthPlugin_ {
    /**
     * super object
     **/
    struct QBObservable_ super_;
};

/**
 * Remote session authentication plugin virtual methods table.
 **/
typedef const struct QBRemoteSessionAuthPluginVTable_ {
    /**
     * SvObject virtual methods table
     */
    struct SvObjectVTable_ super_;

    /**
     * Get current authentication status.
     *
     * @param[in] self   QBRemoteSessionAuthPlugin handle
     * @param[in] manager remote session manager handle
     * @return            authentication status
     **/
    QBRemoteSessionAuthStatus (*getStatus)(QBRemoteSessionAuthPlugin self,
                                           QBRemoteSessionManager manager);

    /**
     * Start authentication process.
     *
     * When authentication process is finished, QBRemoteSessionAuthPlugin
     * should notify listeners regardless of the outcome of the operation.
     * @see QBObservableNotifyObservers.
     *
     * @param[in]  self    QBRemoteSessionAuthPlugin handle
     * @param[in]  manager  remote session manager handle
     * @param[out] errorOut error info
     **/
    void (*authenticate)(QBRemoteSessionAuthPlugin self,
                         QBRemoteSessionManager manager,
                         SvErrorInfo *errorOut);

    /**
     * Prepare QBRemoteDataRequest to be sent to remote server.
     *
     * All necessary query params and HTTP headers should be set by this
     * method.
     *
     * @param[in]  self    QBRemoteSessionAuthPlugin handle
     * @param[in]  request  request to prepare
     * @param[out] errorOut error info
     **/
    void (*prepareRequest)(QBRemoteSessionAuthPlugin self,
                           QBRemoteDataRequest request,
                           SvErrorInfo *errorOut);

    /**
     * Stop all ongoing asynchronous tasks.
     *
     * @param[in]  self QBRemoteSessionAuthPlugin handle
     **/
    void (*stop)(QBRemoteSessionAuthPlugin self);
} *QBRemoteSessionAuthPluginVTable;

/**
 * Get runtime type identification object representing
 * type of QBRemoteSessionAuthPlugin class.
 *
 * @relates QBRemoteSessionAuthPlugin
 *
 * @return QBRemoteSessionAuthPlugin class
 **/
SvType
QBRemoteSessionAuthPlugin_getType(void);

/**
 * Get current authentication status.
 *
 * @public @memberof QBRemoteSessionAuthPlugin
 *
 * @param[in] self   QBRemoteSessionAuthPlugin handle
 * @param[in] manager remote session manager handle
 * @return            authentication status
 **/
static inline QBRemoteSessionAuthStatus
QBRemoteSessionAuthPluginGetStatus(QBRemoteSessionAuthPlugin self,
                                   QBRemoteSessionManager manager)
{
    return SvInvokeVirtual(QBRemoteSessionAuthPlugin, self, getStatus, manager);
}

/**
 * Start authentication process.
 *
 * @public @memberof QBRemoteSessionAuthPlugin
 *
 * When authentication process is finished, QBRemoteSessionAuthPlugin
 * should notify listeners regardless of the outcome of the operation
 * (@ref QBObservableNotifyObservers).
 *
 * @param[in]  self    QBRemoteSessionAuthPlugin handle
 * @param[in]  manager  remote session manager handle
 * @param[out] errorOut error info
 **/
static inline void
QBRemoteSessionAuthPluginAuthenticate(QBRemoteSessionAuthPlugin self,
                                      QBRemoteSessionManager manager,
                                      SvErrorInfo *errorOut)
{
    SvInvokeVirtual(QBRemoteSessionAuthPlugin, self, authenticate, manager, errorOut);
}

/**
 * Prepare QBRemoteDataRequest before send to middleware server.
 *
 * @public @memberof QBRemoteSessionAuthPlugin
 *
 * Set here all necessary query params and HTTP headers.
 *
 * @param[in]  self    QBRemoteSessionAuthPlugin handle
 * @param[in]  request  request to prepare
 * @param[out] errorOut error info
 **/
static inline void
QBRemoteSessionAuthPluginPrepareRequest(QBRemoteSessionAuthPlugin self,
                                        QBRemoteDataRequest request,
                                        SvErrorInfo *errorOut)
{
    SvInvokeVirtual(QBRemoteSessionAuthPlugin, self, prepareRequest, request, errorOut);
}

/**
 * Stop all ongoing asynchronous tasks.
 *
 * @public @memberof QBRemoteSessionAuthPlugin
 *
 * @param[in]  self QBRemoteSessionAuthPlugin handle
 **/
static inline void
QBRemoteSessionAuthPluginStop(QBRemoteSessionAuthPlugin self)
{
    SvInvokeVirtual(QBRemoteSessionAuthPlugin, self, stop);
}

/**
 * @}
 **/

#endif /* QB_REMOTE_SESSION_AUTH_PLUGIN_H_ */
