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

#ifndef QB_RPC_REMOTE_OBJECT_H_
#define QB_RPC_REMOTE_OBJECT_H_

#include <QBRPC/QBRPCObject.h>
#include <QBRPC/QBRPCRemoteEndpoint.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBRPC/QBRPCMessage.h>


/**
 * QBRPCRemoteObject class.
 * QBRPCRemoteObject represents remote rpc object.
 **/
struct QBRPCRemoteObject_t {
    struct QBRPCObject_t super_;

    // object id
    SvString id;

    // remote endpoint handle
    QBRPCRemoteEndpoint remoteEndpoint;
};
typedef struct QBRPCRemoteObject_t* QBRPCRemoteObject;

/**
 * QBRPCRemoteEndpoint virtual methods table
 **/
typedef const struct QBRPCRemoteObjectVTable_ {
    /**
     * SvObject virtual methods table
     **/
    struct SvObjectVTable_ super_;

    /**
     * Interface method handler.
     * Tip: message's read buffer offset is already set to first
     * method's param name.
     *
     * @param[in] handler            an object that implements called method
     * @param[in] methodName         method name
     * @param[in] message            message
     *
     * @return                       @a 0 in case of unhandled method, @c positive value in case of handled method
     *
     **/
    unsigned int (*methodHandler)(SvObject self_, QBRPCObject handler_, SvString methodName, QBRPCMessage* message);
} *QBRPCRemoteObjectVTable;

/**
 * Get instance of QBRPCRemoteObject type.
 *
 * @return                 QBRPCRemoteObject type instance
 *
 **/
SvType
QBRPCRemoteObject_getType(void);

/**
 * Create remote rpc object.
 *
 * @param[in] id                   remote rpc object id
 * @param[in] remoteEndpoint       remote endpoint handle
 *
 * @return                         @a self, @c NULL in case of error
 **/
QBRPCRemoteObject
QBRPCRemoteObjectCreate(SvString id, QBRPCRemoteEndpoint remoteEndpoint);

/**
 * Initialize remote rpc object.
 *
 * @param[in] self                 remote rpc object handle
 * @param[in] id                   remote rpc object id
 * @param[in] remoteEndpoint       remote endpoint handle
 *
 * @return                         @c 0 if successful, @c -1 in case of error
 **/
int
QBRPCRemoteObjectInit(QBRPCRemoteObject self, SvString id, QBRPCRemoteEndpoint remoteEndpoint);

/**
 * Get remote rpc object id.
 *
 * @param[in] self            remote rpc object handle
 *
 * @return                    remote rpc object id
 **/
SvString
QBRPCRemoteObjectGetId(QBRPCRemoteObject self);

/**
 * Get remote rpc object's remote endpoint.
 *
 * @param[in] self            remote rpc object handle
 *
 * @return                    remote endpoint handle
 **/
QBRPCRemoteEndpoint
QBRPCRemoteObjectGetRemoteEndpoint(QBRPCRemoteObject self);


#endif // QB_RPC_REMOTE_OBJECT_H_
