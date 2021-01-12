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

#ifndef QB_IPC_LIGHT_SERVER_H_
#define QB_IPC_LIGHT_SERVER_H_

/**
 * @file QBIPCLightServer.h QBIPCServer API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <sys/time.h>

/**
 * @defgroup QBIPCLightServer QBIPCLightServer
 * @{
 **/


#define MAX_IPC_MSG_LENGTH 65535    /**< maximum lenght of a QBIPC message */
#define MAX_SOCKETS  10             /**< maximum number of sockets that can be registered */

/**
 * QBIPCLightServerMessage is a class that keeps an IPC
 * message and a socket file descriptor on which the message
 * was received.
 *
 * @class QBIPCLightServerMessage
 **/
typedef struct QBIPCLightServerMessage_ {
    char msg[MAX_IPC_MSG_LENGTH];   /**< body of a QBIPC message */
    int sourceSocketFd;             /**< source socket file descriptor of a QBIPC message */
} *QBIPCLightServerMessage;

/**
 * QBIPCLightServer receives incoming IPC messages on a QBIPCSocket.
 * After receiving an IPC message on the socket it triggers
 * registered callbacks.
 *
 * The QBIPCLightServer blocks its thread on a select function!
 *
 * @class QBIPCLightServer
 **/
typedef struct QBIPCLightServer_ *QBIPCLightServer;

/**
 * Creates a new instance of an QBIPCLightServer.
 *

 * @return a new instance of QBIPCLightServer
 **/
QBIPCLightServer
QBIPCLightServerCreate(void);

/**
 * Destroys an instance of a QBIPCLightServer.
 *
 * @param[in] self  handle to the instance of the QBIPCLightServer to be destroyed
 **/
void
QBIPCLightServerDestroy(QBIPCLightServer self);

/**
 * Registers a socket in a QBIPCLightServer.
 * The QBIPCLightServer will add its socket file descriptor to its listening pool.
 * This function should be called before QBIPCLightServerListen.
 *
 * @param[in] self      handle to the instance of the QBIPCLightServer
 * @param[in] path      path for a new QBIPCSocket (owned by QBIPCLightServer)
 * @return  file descriptor of created QBIPCSocket, -1 in case of error
 **/
int
QBIPCLightServerRegister(QBIPCLightServer self, const char *path);

/**
 * Registers a file descriptor of a custom (already setup) socket and callbacks in a QBIPCLightServer.
 * The QBIPCLightServer will add the socket file descriptor to its listening pool.
 * This function should be called before QBIPCLightServerListen.
 *
 * @param[in] self      handle to the instance of the QBIPCLightServer
 * @param[in] sockfd    a file descriptor of a socket
 * @return  file descriptor of created QBIPCSocket, -1 in case of error
 **/
int
QBIPCLightServerRegisterSocket(QBIPCLightServer self, int sockfd);

/**
 * Unregisters callbacks from a QBIPCLightServer.
 *
 * @param[in] self      handle to the instance of the QBIPCLightServer
 * @param[in] fd        a file descriptor of a socket to be unregistered
 **/
void
QBIPCLightServerUnregister(QBIPCLightServer self, int fd);

/**
 * Receive a single message from one of the sockets added by QBIPCLightServerRegister function calls.
 * This function shall be called in a loop for continuous message receiving from multiple sockets
 *
 * @param[in] self      handle to the instance of the QBIPCLightServer
 * @param[in] timeout   timeout for a message to come
 * @param[out] outMsg   received QBIPCLightServerMessage
 * @return  length of a body of the received QBIPC message, -1 in case of error
 **/
int
QBIPCLightServerReceive(QBIPCLightServer self,
                        struct timeval *timeout,
                        QBIPCLightServerMessage outMsg);

/**
 * @}
**/

#endif /* QB_IPC_SERVER_H_ */
