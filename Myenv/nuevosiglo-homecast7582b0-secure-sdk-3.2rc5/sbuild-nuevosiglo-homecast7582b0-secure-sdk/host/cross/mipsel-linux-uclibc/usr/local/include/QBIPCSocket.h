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

#ifndef QB_IPC_SOCKET_H_
#define QB_IPC_SOCKET_H_

/**
 * @file QBIPCSocket.h IPC Socket API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>

/**
 * @defgroup QBIPCSocket QBIPCSocket
 * @{
 **/

/**
 * QBIPCSocket is used for both sending and receiving IPC messages.
 *
 * @class QBIPCSocket
 **/
typedef struct QBIPCSocket_ *QBIPCSocket;

/**
 * Creates a new instance of an QBIPCSocket.
 *
 * @param[in] path  path for a new QBIPCSocket
 * @return  a new instance of QBIPCSocket
 **/
QBIPCSocket
QBIPCSocketCreate(const char* path);

/**
 * Creates a new instance of an QBIPCSocket with a predefined socked file descriptor.
 *
 * @param[in] sockfd  predefined socked file descriptor
 * @return  a new instance of QBIPCSocket
 **/
QBIPCSocket
QBIPCSocketCreateWithSockFd(int sockfd);

/**
 * Destroys an instance of an QBIPCSocket.
 *
 * @param[in] self  handle to the instance of the QBIPCSocket to be destroyed
 **/
void
QBIPCSocketDestroy(QBIPCSocket self);

/**
 * Binds an instance of an QBIPCSocket.
 *
 * @param[in] self  handle to the instance of the QBIPCSocket
 * @return  zero on success. On error, -1 is returned, and errno is set appropriately.
 **/
int
QBIPCSocketBind(QBIPCSocket self);

/**
 * Unlinks an instance of an QBIPCSocket.
 * Use the function when setting up a QBIPCServer only.
 *
 * @param[in] self  handle to the instance of the QBIPCSocket
 * @return  zero on success. On error, -1 is returned, and errno is set appropriately.
 **/
int
QBIPCSocketUnlink(QBIPCSocket self);

/**
 * Receives a data from an instance of an QBIPCSocket.
 * Use the function when stopping a QBIPCServer only.
 *
 * @param[in] self      handle to the instance of the QBIPCSocket
 * @param[out] recvBuf  buffer to be filled in with data (data is not \0 escaped!)
 * @param[in] maxLen    maximum number of bytes that can be filled in the recvBuf
 * @return  number of bytes filled in the recvBuf, -1 in case of error
 **/
int
QBIPCSocketRecvMessage(QBIPCSocket self, char *recvBuf, const size_t maxLen);

/**
 * Sends a data to an instance of an QBIPCSocket.
 * This function uses select blocking function.
 *
 * @param[in] self          handle to the instance of the QBIPCSocket
 * @param[out] buf          buffer to be filled in with data (data is not \0 escaped!)
 * @param[in] len           number of bytes to be sent
 * @param[in] timeoutSec    timeout for the socket to be ready for sending
 * @return  number of bytes filled in the recvBuf, -1 in case of error
 **/
int
QBIPCSocketSend(QBIPCSocket self, const void* buf, int len, int timeoutSec);

/**
 * Gets a file descriptor of a QBIPCSocket.
 *
 * @param[in] self  handle to the instance of the QBIPCSocket
 * @return  file descriptor of a QBIPCSocket
 **/
int
QBIPCSocketGetFileDescriptor(QBIPCSocket self);

/**
 * @}
**/

#endif /* QB_IPC_SOCKET_H_ */
