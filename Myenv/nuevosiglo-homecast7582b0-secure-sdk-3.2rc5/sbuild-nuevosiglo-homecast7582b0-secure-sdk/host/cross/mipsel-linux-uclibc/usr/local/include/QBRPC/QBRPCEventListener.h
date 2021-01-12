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

#ifndef QB_RPC_EVENT_LISTENER_H_
#define QB_RPC_EVENT_LISTENER_H_

#include <QBRPC/QBRPCRemoteEndpoint.h>
#include <QBRPC/QBRPCRemoteObject.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>

/**
 * QBRPCEventListener class.
 **/
struct QBRPCEventListener_t {
    /**
     * Endpoint connected notification.
     *
     * @param[in] self_                listener handle
     * @param[in] remoteEndpoint       remote endpoint handle (new remote client)
     *
     **/
    void (*endpointConnected)(SvObject self_, QBRPCRemoteEndpoint remoteEndpoint);
    /**
     * Endpoint disconnected notification.
     *
     * @param[in] self_                listener handle
     * @param[in] remoteEndpoint       remote endpoint handle
     *
     **/
    void (*endpointDisconnected)(SvObject self_, QBRPCRemoteEndpoint remoteEndpoint);

    /**
     * Service found notification.
     *
     * @param[in] self_                listener handle
     * @param[in] remoteEndpoint       remote endpoint handle
     * @param[in] service              remote service handle
     * @param[in] interfaceList        remote service interface list
     *
     **/
    void (*serviceFound)(SvObject self_, QBRPCRemoteEndpoint remoteEndpoint, QBRPCRemoteObject service, SvHashTable interfaceList);

    /**
     * Service not found notification.
     *
     * @param[in] self_                listener handle
     * @param[in] remoteEndpoint       remote endpoint handle
     * @param[in] name                 service name
     *
     **/
    void (*serviceNotFound)(SvObject self_, QBRPCRemoteEndpoint remoteEndpoint, SvString name);

    /**
     * Resource received notification.
     *
     * @param[in] self_                listener handle
     * @param[in] remoteEndpoint       remote endpoint handle
     * @param[in] label                resource label
     * @param[in] descriptor           resource descriptor
     *
     **/
    void (*resourceReceived)(SvObject self_, QBRPCRemoteEndpoint remoteEndpoint, SvString label, int descriptor);
};
typedef struct QBRPCEventListener_t* QBRPCEventListener;

/**
 * Get QBRPCEventListener inteface instance.
 *
 * @return                             QBRPCEventListener interface instance
 *
 **/
SvInterface
QBRPCEventListener_getInterface(void);


#endif // QB_RPC_EVENT_LISTENER_H_
