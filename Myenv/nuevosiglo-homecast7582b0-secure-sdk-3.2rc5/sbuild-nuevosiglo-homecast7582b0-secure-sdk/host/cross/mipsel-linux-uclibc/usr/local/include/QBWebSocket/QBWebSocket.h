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

#ifndef QB_WEB_SOCKET_H_
#define QB_WEB_SOCKET_H_

/**
 * @file QBWebSocket.h WebSocket class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBWebSocketLibrary QBWebSocket: WebSocket client library
 * @ingroup Networking
 *
 * QBWebSocket is client library implementing WebSocket protocol as defined
 * by <a href="http://tools.ietf.org/html/rfc6455">RFC6455</a>,
 * with interface similar to
 * <a href="http://www.w3c.org/TR/websockets">W3C WebSocket JavaScript API</a>.
 **/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <fibers/c/fibers.h>
#include <QBWebSocket/QBWebSocketMessage.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURL.h>
#include <SvHTTPClient/SvSSLParams.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBWebSocket WebSocket class
 * @ingroup QBWebSocketLibrary
 * @{
 *
 * QBWebSocket class implements single WebSocket.
 **/

/**
 * WebSocket connection state.
 **/
typedef enum {
    /// special marker used to signal that state is unknown
    QBWebSocketState_unknown = -1,
    /// state of socket before QBWebSocketConnect()
    QBWebSocketState_initial = 0,
    /// socket is connecting to the server
    QBWebSocketState_connecting = 1,
    /// connection is opened
    QBWebSocketState_open = 2,
    /// socket is performing close handshake
    QBWebSocketState_closing = 3,
    /// connection is closed
    QBWebSocketState_closed = 4
} QBWebSocketState;

/**
 * WebSocket error code.
 **/
typedef enum {
    /// special marker used to signal state with no error
    QBWebSocketError_notset             = 0x0000,
    /// can't resolve host name
    QBWebSocketError_resolveName        = 0x0100,
    /// socket() function failed
    QBWebSocketError_socket             = 0x0201,
    /// connect() function failed
    QBWebSocketError_connect            = 0x0202,
    /// setsockopt() function failed
    QBWebSocketError_setSocket          = 0x0301,
    /// can't create SSL context
    QBWebSocketError_sslContext         = 0x0302,
    /// can't use certificate file
    QBWebSocketError_sslCertFile        = 0x0303,
    /// unknown SSL file format
    QBWebSocketError_sslFormat          = 0x0304,
    /// can't use key file
    QBWebSocketError_sslKey             = 0x0305,
    /// private key and certificate are inconsistent
    QBWebSocketError_sslKeyAndCert      = 0x0306,
    /// can't read CA list
    QBWebSocketError_sslCA              = 0x0307,
    /// hostname verification failed
    QBWebSocketError_sslHost            = 0x0308,
    /// can't get peer certificate
    QBWebSocketError_sslPeerCert        = 0x0309,
    /// SSL connect() function failed
    QBWebSocketError_sslConnect         = 0x030A,
    /// failed to create SSL struct
    QBWebSocketError_sslStruct          = 0x030B,
    /// can't bind SSL context
    QBWebSocketError_sslBind            = 0x030C,
    /// SSL connection timeout
    QBWebSocketError_sslTimeout         = 0x030D,
    /// SSL unexpected notification
    QBWebSocketError_sslNotification    = 0x030E,
    /// socket is closed
    QBWebSocketError_socketClosed       = 0x0400,
} QBWebSocketError;

/**
 * WebSocket class.
 * @class QBWebSocket
 * @extends SvObject
 **/
typedef struct QBWebSocket_ *QBWebSocket;

/**
 * Return the runtime type identification object
 * of the QBWebSocket class.
 *
 * @return WebSocket class
 **/
extern SvType
QBWebSocket_getType(void);

/**
 * Create new WebSocket.
 *
 * @memberof QBWebSocket
 *
 * @param[in] maxMessageLength max length of received message, in bytes
 * @param[in] outgoingBufferSize size of the buffer for outgoing data, in bytes
 * @param[out] errorOut error info
 * @return              created WebSocket, @c NULL in case of error
 **/
extern QBWebSocket
QBWebSocketCreate(size_t maxMessageLength,
                  size_t outgoingBufferSize,
                  SvErrorInfo *errorOut);

/**
 * Get connection state of a WebSocket.
 *
 * @memberof QBWebSocket
 *
 * @param[in] self      WebSocket handle
 * @return              connection state
 **/
extern QBWebSocketState
QBWebSocketGetState(QBWebSocket self);

/**
 * Get error state of a WebSocket.
 *
 * @memberof QBWebSocket
 *
 * @param[in] self      WebSocket handle
 * @return              error state
 **/
extern QBWebSocketError
QBWebSocketGetError(QBWebSocket self);

/**
 * Set list of supported protocols to announce to server.
 *
 * @note This method must be called before QBWebSocketConnect().
 *
 * @memberof QBWebSocket
 *
 * @param[in] self      WebSocket handle
 * @param[in] protocols array of protocol names as SvString
 * @param[out] errorOut error info
 **/
extern void
QBWebSocketSetProtocols(QBWebSocket self,
                        SvArray protocols,
                        SvErrorInfo *errorOut);

/**
 * Setup how often to send PING packets to server.
 *
 * @memberof QBWebSocket
 *
 * @param[in] self      WebSocket handle
 * @param[in] period    delay between consecutive PING packets in seconds,
 *                      pass @c 0 to disable sending PING packets
 * @param[out] errorOut error info
 **/
extern void
QBWebSocketSetPingPeriod(QBWebSocket self,
                         unsigned int period,
                         SvErrorInfo *errorOut);

/**
 * Add or remove custom header to send in WebSocket handshake.
 *
 * @note This method must be called before QBWebSocketConnect().
 *
 * @memberof QBWebSocket
 *
 * @param[in] self      WebSocket handle
 * @param[in] name      name of the custom header
 * @param[in] value     value of the custom header, @c NULL to
 *                      remove currently set header with this @a name
 * @param[out] errorOut error info
 **/
extern void
QBWebSocketSetCustomHeader(QBWebSocket self,
                           SvString name,
                           SvString value,
                           SvErrorInfo *errorOut);

/**
 * Register WebSocket listener.
 *
 * @note This method must be called before QBWebSocketConnect().
 *
 * @memberof QBWebSocket
 *
 * @param[in] self      WebSocket handle
 * @param[in] listener  listener handle (must implement @ref QBWebSocketListener),
 *                      @c NULL to remove currently registered listener
 * @param[out] errorOut error info
 **/
extern void
QBWebSocketSetListener(QBWebSocket self,
                       SvObject listener,
                       SvErrorInfo *errorOut);

/**
 * Open connection to a server.
 *
 * @memberof QBWebSocket
 *
 * @param[in] self      WebSocket handle
 * @param[in] scheduler scheduler to use by this WebSocket
 * @param[in] serverURL URL to connect to, must be 'ws://' (plain-text)
 *                      or 'wss://' (secure SSL/TLS communication)
 * @param[in] sslParams SSL/TLS parameters, required when using SSL/TLS,
 *                      otherwise ignored
 * @param[in] originURL origin URL to pass to server
 * @param[out] errorOut error info
 **/
extern void
QBWebSocketConnect(QBWebSocket self,
                   SvScheduler scheduler,
                   SvURL serverURL,
                   SvSSLParams sslParams,
                   SvURL originURL,
                   SvErrorInfo *errorOut);

/**
 * Get protocol selected by server.
 *
 * @note This method is valid only when connection is opened.
 *
 * @memberof QBWebSocket
 *
 * @param[in] self      WebSocket handle
 * @return              protocol name, empty if there is no protocol
 *                      selected, @c NULL in case of error
 **/
extern SvString
QBWebSocketGetProtocol(QBWebSocket self);

/**
 * Send binary message to the server.
 *
 * @note This method is valid only when connection is opened.
 *
 * @memberof QBWebSocket
 *
 * @param[in] self      WebSocket handle
 * @param[in] data      contents of the message to send
 * @param[in] length    length of @a data in bytes
 * @param[out] errorOut error info
 **/
extern void
QBWebSocketSendBinaryMessage(QBWebSocket self,
                             const uint8_t *data,
                             size_t length,
                             SvErrorInfo *errorOut);

/**
 * Send text message to the server.
 *
 * @note This method is valid only when connection is opened.
 *
 * @memberof QBWebSocket
 *
 * @param[in] self      WebSocket handle
 * @param[in] text      contents of the message to send,
 *                      must be valid UTF-8 text
 * @param[in] length    length of @a text in bytes,
 *                      pass @c -1 to send entire NULL-terminated string
 * @param[out] errorOut error info
 **/
extern void
QBWebSocketSendTextMessage(QBWebSocket self,
                           const char *text,
                           ssize_t length,
                           SvErrorInfo *errorOut);

/**
 * Close connection to the server.
 *
 * @memberof QBWebSocket
 *
 * @param[in] self      WebSocket handle
 * @param[in] sendReason @c true to send connection close code
 *                      and reason to server
 * @param[in] code      connection close code to pass to server,
 *                      sent only of @a sendReason is @c true
 * @param[in] reason    connection close reason to pass to server,
 *                      sent only of @a sendReason is @c true
 * @param[out] errorOut error info
 **/
extern void
QBWebSocketDisconnect(QBWebSocket self,
                      bool sendReason,
                      unsigned int code,
                      const char *reason,
                      SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
