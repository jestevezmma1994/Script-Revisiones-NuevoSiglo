/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_RPC_INTERFACE_LOCK_H_
#define QB_RPC_INTERFACE_LOCK_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <common/QBRPCInterfaceCall.h>
#include <QBRPC/QBRPCMessage.h>
#include <time.h>


/**
 * Callback used to notify interface method caller that call response has been received.
 * Used in async call mode.
 *
 * @param[in] caller         handle to object that will handle response
 * @param[in] methodName     called method name
 * @param[in] status         call status
 * @param[in] message        response message
 *
 * @return                   @c 0 if handled, @c -1 otherwise
 *
 **/
typedef int (*QBRPCInterfaceLockResponseHandler)(SvObject caller, SvString methodName, QBRPCInterfaceCallStatus status, QBRPCMessage* message);

/**
 * QBRPCInterfaceLock class.
 * This class enables handling non-oneway interface calls.
 * Interface lock can be used to lock a method and wait for call response.
 **/
struct QBRPCInterfaceLock_t {
    struct SvObject_ super_;

    // lock id
    SvObject id;

    // lock time
    struct timespec lockTime;

    // time lock out after lockTimeoutMs passes
    long long lockTimeoutMs;

    // response message
    QBRPCMessage* message;

    // interface call's response status
    QBRPCInterfaceCallStatus status;

    // interface call's methodName
    SvString methodName;

    // interface call's response handler owner
    SvObject caller;

    // interface call's response handler
    QBRPCInterfaceLockResponseHandler responseHandler;
};
typedef struct QBRPCInterfaceLock_t* QBRPCInterfaceLock;


/**
 * Create rpc interface lock.
 *
 * @param[in] id                 interface lock id
 * @param[in] lockTimeoutMs      interface lock is valid for this period of time
 *
 * @return                       @a self, @c NULL in case of error
 **/
QBRPCInterfaceLock
QBRPCInterfaceLockCreate(SvObject id, long long lockTimeoutMs);

/**
 * Get rpc interface lock id.
 *
 * @param[in] self               interface lock handle
 *
 * @return                       interface lock id handle
 **/
SvObject
QBRPCInterfaceLockGetId(QBRPCInterfaceLock self);

/**
 * Get rpc interface lock time.
 *
 * @param[in] self               interface lock handle
 *
 * @return                       interface lock time
 **/
struct timespec
QBRPCInterfaceLockGetLockTime(QBRPCInterfaceLock self);

/**
 * Set rpc interface lock time.
 *
 * @param[in] self               interface lock handle
 **/
void
QBRPCInterfaceLockSetLockTime(QBRPCInterfaceLock self);

/**
 * Set rpc interface lock response message.
 *
 * @param[in] self               interface lock handle
 * @param[in] message            message handle
 **/
void
QBRPCInterfaceLockSetMessage(QBRPCInterfaceLock self, QBRPCMessage* message);

/**
 * Get rpc interface lock response message.
 *
 * @param[in] self               interface lock handle
 *
 * @return                       interface lock response message
 **/
QBRPCMessage*
QBRPCInterfaceLockGetMessage(QBRPCInterfaceLock self);

/**
 * Get rpc interface lock timeout status.
 *
 * @param[in] self               interface lock handle
 *
 * @return                       @c true if lock is timed out, @c false otherwise
 **/
bool
QBRPCInterfaceLockIsTimedOut(QBRPCInterfaceLock self);

/**
 * Set rpc interface lock call's response status.
 *
 * @param[in] self               interface lock handle
 * @param[in] status             status
 *
 * @return                       interface lock call response status
 **/
void
QBRPCInterfaceLockSetStatus(QBRPCInterfaceLock self, QBRPCInterfaceCallStatus status);

/**
 * Get rpc interface lock call's response status.
 *
 * @param[in] self               interface lock handle
 *
 * @return                       interface lock call response status
 **/
QBRPCInterfaceCallStatus
QBRPCInterfaceLockGetStatus(QBRPCInterfaceLock self);

/**
 * Set rpc interface lock called method name.
 *
 * @param[in] self               interface lock handle
 * @param[in] methodName         method name
 **/
void
QBRPCInterfaceLockSetMethodName(QBRPCInterfaceLock self, SvString methodName);

/**
 * Set rpc interface lock called method name.
 *
 * @param[in] self               interface lock handle
 * @param[in] caller             handle to an object that implements
 *                               async call's response handling interface.
 * @param[in] responseHandler    response handler callback
 *
 **/
void
QBRPCInterfaceLockSetResponseHandler(QBRPCInterfaceLock self, SvObject caller, QBRPCInterfaceLockResponseHandler responseHandler);

/**
 * Notify about response to call using response handler.
 *
 * @param[in] self               interface lock handle
 *
 * @return                       @c 0 if successful, @c -1 otherwise
 **/
int
QBRPCInterfaceLockNotify(QBRPCInterfaceLock self);

#endif // QB_RPC_INTERFACE_LOCK_H_
