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

#ifndef QB_PUSH_RECEIVER_H_
#define QB_PUSH_RECEIVER_H_

/**
 * @file QBPushReceiver.h Push Receiver class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBWebSocket/QBWebSocket.h>

#include <fibers/c/fibers.h>

#include <Services/core/QBMiddlewareManager.h>
#include <SvHTTPClient/SvSSLParams.h>

/**
 * @defgroup QBPushReceiver Push Receiver Service
 * @ingroup CubiTV_services
 * @{
 *
 * This service is responsible for connection with push server.
 **/

/**
 * This service is responsible for connection with push server.
 *
 * @class QBPushReceiver
 *
 * This service lets its listeners to received data from push server.
 * It also allows to trace push server status changes.
 **/
typedef struct QBPushReceiver_ *QBPushReceiver;

/**
 * Notifications sent from PushReceiver to listeners
 **/
typedef enum {
    /// special marker used to signal that state is unknown
    QBPushReceiver_unknown,
    /// socket is successfully connected with the push server
    QBPushReceiver_connected,
    /// socket is not connected with the push server
    QBPushReceiver_disconnected,
} QBPushReceiverStatus;

struct QBPushReceiverListener_t {
    /**
     * This function is invoked when data is received from server
     *
     * @param[in] self_ object implementing QBPushReceiver interface
     * @param[in] type  type of received data
     * @param[in] data  data received from push server
     **/
    void (*dataReceived)(SvObject self_,
                         SvString type,
                         SvObject data);
    /**
     * Status change handler
     *
     * @param[in] self_  WebSocket handle
     * @param[in] status Status of Push Connectivity
     **/
    void (*statusChanged)(SvObject self_,
                          QBPushReceiverStatus status);
};

/**
 * This data type defines an interface for QBPushReceiver listeners.
 **/
typedef const struct QBPushReceiverListener_t *QBPushReceiverListener;

/**
 * Create interface for the Push Receiver listeners.
 *
 * @return created interface for the listeners, @c NULL in case of error
 **/
SvInterface QBPushReceiverListener_getInterface(void);

/**
 * Returns last error set by WebSocket.
 *
 * @param[in] self QBPushReceiver handle
 * @return         Connection error code
 *
 **/
QBWebSocketError QBPushReceiverGetLastError(QBPushReceiver self);

/**
 * Add listener to the QBPushReceiver service.
 *
 * @param[in] self         QBPushReceiver handle
 * @param[in] listener     object implementing QBPushReceiver interface
 * @param[in] listenerType type of listener
 * @param[out] errorOut    error info
 **/
void
QBPushReceiverAddListener(QBPushReceiver self,
                          SvObject listener,
                          SvString listenerType,
                          SvErrorInfo *errorOut);

/**
 * Remove listener previously added by QBPushReceiverAddListener from
 * active listeners list.
 *
 * @param[in] self      QBPushReceiver handle
 * @param[in] listener  listener which will be removed from listeners list
 * @param[out] errorOut error info
 **/
void
QBPushReceiverRemoveListener(QBPushReceiver self,
                             SvObject listener,
                             SvErrorInfo *errorOut);

/**
 * Creates Push Receiver object.
 *
 * @param [in] middlewareManager QBMiddlewareManager handle
 * @param [in] sslParams         SvSSLParams handle
 * @param [in] refreshTimeSec    time between retries
 * @param[out] errorOut          error info
 * @return                       new Push Receiver instance
 **/
QBPushReceiver
QBPushReceiverCreate(QBMiddlewareManager middlewareManager,
                     SvSSLParams sslParams,
                     int refreshTimeSec,
                     SvErrorInfo *errorOut);

/**
 * Start Push Receiver service.
 *
 * @param [in] self      QBPushReceiver handle
 * @param [in] scheduler SvScheduler handle
 * @param[out] errorOut  error info
 * @return               0 on success, -1 otherwise
 **/
int
QBPushReceiverStart(QBPushReceiver self,
                    SvScheduler scheduler,
                    SvErrorInfo *errorOut);

/**
 * Stop Push Receiver service.
 *
 * @param [in] self     QBPushReceiver handle
 * @param[out] errorOut error info
 * @return              0 on success, -1 otherwise
 **/
int
QBPushReceiverStop(QBPushReceiver self,
                   SvErrorInfo *errorOut);

/**
 * @}
 **/
#endif /* QB_PUSH_RECEIVER_H_ */
