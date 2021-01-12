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

#ifndef QB_IPC_SERVER_H_
#define QB_IPC_SERVER_H_

/**
 * @file QBIPCServer.h IPC Server API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <fibers/c/fibers.h>

/**
 * @defgroup QBIPCServer IPC Server
 * @{
 **/

/**
 * QBIPCServer receives incoming IPC messages on a QBIPCSocket.
 * After receiving an IPC message on the socket it triggers
 * registered callbacks.
 *
 * @class QBIPCServer
 **/
typedef struct QBIPCServer_ *QBIPCServer;

/**
 * Callbacks that will be trigerred by a QBIPCServer after
 * it receives an IPC message on its QBIPCSocket.
 *
 * @class QBIPCServerCallbacks_
 **/
struct QBIPCServerCallbacks_ {
    void (*message)(void* target, QBIPCServer ipc, const void* buf, int len);
    void (*error)(void* target, QBIPCServer ipc, int err);
};

/**
 * Creates a new instance of an QBIPCServer.
 *
 * @param[in] path  path for a new QBIPCSocket (owned by QBIPCServer)
 * @return a new instance of QBIPCServer
 **/
QBIPCServer
QBIPCServerCreate(const char* path);

/**
 * Destroys an instance of a QBIPCServer.
 *
 * @param[in] self  handle to the instance of the QBIPCServer to be destroyed
 **/
void
QBIPCServerDestroy(QBIPCServer self);

/**
 * Registers callbacks in a QBIPCServer.
 *
 * @param[in] self      handle to the instance of the QBIPCServer
 * @param[in] callbacks callbacks to be called after the IPC server receives an IPC message
 * @param[in] target    callbacks' owner
 **/
void
QBIPCServerRegister(QBIPCServer self, const struct QBIPCServerCallbacks_ *callbacks, void *target);

/**
 * Unregisters callbacks from a QBIPCServer.
 *
 * @param[in] self      handle to the instance of the QBIPCServer
 * @param[in] callbacks callbacks to be called after the IPC server receives an IPC message
 * @param[in] target    callbacks' owner
 **/
void
QBIPCServerUnregister(QBIPCServer self, const struct QBIPCServerCallbacks_ *callbacks, void *target);

/**
 * Starts a QBIPCServer.
 *
 * @param[in] self      handle to the instance of the QBIPCServer
 * @param[in] scheduler instance of a SvScheduler
 * @param[in] name      name of the internal fiber of the QBIPCServer
 **/
void
QBIPCServerStart(QBIPCServer self, SvScheduler scheduler, const char *name);

/**
 * Stops a QBIPCServer.
 *
 * @param[in] self      handle to the instance of the QBIPCServer
 **/
void
QBIPCServerStop(QBIPCServer self);

/**
 * @}
**/

#endif /* QB_IPC_SERVER_H_ */
