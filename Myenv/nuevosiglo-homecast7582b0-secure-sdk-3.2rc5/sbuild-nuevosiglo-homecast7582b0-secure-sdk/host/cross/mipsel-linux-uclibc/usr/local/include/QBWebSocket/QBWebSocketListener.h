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

#ifndef QB_WEB_SOCKET_LISTENER_H_
#define QB_WEB_SOCKET_LISTENER_H_

/**
 * @file QBWebSocketListener.h WebSocket listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBWebSocket/QBWebSocket.h>
#include <QBWebSocket/QBWebSocketMessage.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBWebSocketListener WebSocket listener interface
 * @ingroup QBWebSocketLibrary
 * @{
 *
 * QBWebSocketListener is an interface for objects that want
 * to receive notifications from a WebSocket.
 **/

/**
 * QBWebSocketListener interface.
 **/
typedef const struct QBWebSocketListener_ {
    /**
     * Method called when socket establishes connection.
     *
     * @param[in] self      listener handle
     * @param[in] socket    WebSocket handle
     **/
    void (*connected)(SvObject self,
                      QBWebSocket socket);

    /**
     * Method called when socket received message.
     *
     * @param[in] self      listener handle
     * @param[in] socket    WebSocket handle
     * @param[in] message   WebSocket message handle
     **/
    void (*messageReceived)(SvObject self,
                            QBWebSocket socket,
                            QBWebSocketMessage message);

    /**
     * Method called when connection is closed.
     *
     * @param[in] self      listener handle
     * @param[in] socket    WebSocket handle
     * @param[in] wasClean  @c true if connection was cleanly closed
     * @param[in] code      connection close code provided by server,
     *                      @c valid only if @a wasClean is @c true
     * @param[in] reason    close reason given by server (can be @c NULL),
     *                      @c valid only if @a wasClean is @c true
     **/
    void (*closed)(SvObject self,
                   QBWebSocket socket,
                   bool wasClean,
                   unsigned int code,
                   SvString reason);
} *QBWebSocketListener;


/**
 * Get runtime type identification object representing
 * interface of WebSocket listener.
 *
 * @return WebSocket listener interface
 **/
extern SvInterface
QBWebSocketListener_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
