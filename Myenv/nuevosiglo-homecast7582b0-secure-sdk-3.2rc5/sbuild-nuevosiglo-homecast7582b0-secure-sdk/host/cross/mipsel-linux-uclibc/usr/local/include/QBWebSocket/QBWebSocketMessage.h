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

#ifndef QB_WEB_SOCKET_MESSAGE_H_
#define QB_WEB_SOCKET_MESSAGE_H_

/**
 * @file QBWebSocketMessage.h WebSocket message class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvData.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBWebSocketMessage WebSocket message class
 * @ingroup QBWebSocketLibrary
 * @{
 *
 * QBWebSocketMessage class represents an abstract message
 * received from a WebSocket.
 *
 * @image html QBWebSocketMessage.png
 **/

/**
 * WebSocket message class.
 * @class QBWebSocketMessage
 * @extends SvObject
 **/
typedef struct QBWebSocketMessage_ *QBWebSocketMessage;

/**
 * Return the runtime type identification object
 * of the QBWebSocketMessage class.
 *
 * @return WebSocket message class
 **/
extern SvType
QBWebSocketMessage_getType(void);

/**
 * @}
 **/


/**
 * @defgroup QBWebSocketBinaryMessage WebSocket binary message class
 * @ingroup QBWebSocketLibrary
 * @{
 *
 * QBWebSocketBinaryMessage class represents a binary message
 * received from a WebSocket.
 **/

/**
 * WebSocket binary message class.
 * @class QBWebSocketBinaryMessage
 * @extends QBWebSocketMessage
 **/
typedef struct QBWebSocketBinaryMessage_ *QBWebSocketBinaryMessage;

/**
 * Return the runtime type identification object
 * of the QBWebSocketBinaryMessage class.
 *
 * @return WebSocket binary message class
 **/
extern SvType
QBWebSocketBinaryMessage_getType(void);

/**
 * Get contents of a binary message.
 *
 * @memberof QBWebSocketBinaryMessage
 *
 * @param[in] self      WebSocket binary message handle
 * @return              message contents
 **/
extern SvData
QBWebSocketBinaryMessageGetData(QBWebSocketBinaryMessage self);

/**
 * @}
 **/


/**
 * @defgroup QBWebSocketTextMessage WebSocket text message class
 * @ingroup QBWebSocketLibrary
 * @{
 *
 * QBWebSocketTextMessage class represents a text message
 * received from a WebSocket.
 **/

/**
 * WebSocket text message class.
 * @class QBWebSocketTextMessage
 * @extends QBWebSocketMessage
 **/
typedef struct QBWebSocketTextMessage_ *QBWebSocketTextMessage;

/**
 * Return the runtime type identification object
 * of the QBWebSocketTextMessage class.
 *
 * @return WebSocket text message class
 **/
extern SvType
QBWebSocketTextMessage_getType(void);

/**
 * Get contents of a text message.
 *
 * @memberof QBWebSocketTextMessage
 *
 * @param[in] self      WebSocket text message handle
 * @return              message contents
 **/
extern SvString
QBWebSocketTextMessageGetText(QBWebSocketTextMessage self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
