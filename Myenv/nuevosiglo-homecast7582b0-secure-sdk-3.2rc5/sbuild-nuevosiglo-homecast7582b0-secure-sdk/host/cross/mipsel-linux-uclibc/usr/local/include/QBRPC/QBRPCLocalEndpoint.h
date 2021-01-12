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

#ifndef QB_RPC_LOCAL_ENDPOINT_H_
#define QB_RPC_LOCAL_ENDPOINT_H_

#include <QBRPC/QBRPCObject.h>
#include <QBRPC/QBRPCUUID.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>


/**
 * QBRPCLocalEndpoint class.
 * QBRPCLocalEndpoint represents local RPC connection handler.
 **/
typedef struct QBRPCLocalEndpoint_t* QBRPCLocalEndpoint;


/**
 * Create local endpoint.
 *
 * @param[in] uuid               uuid
 * @param[in] socketPath         socket path
 *
 * @return                       @a self, @c NULL in case of error
 **/
QBRPCLocalEndpoint
QBRPCLocalEndpointCreate(QBRPCUUID uuid, SvString socketPath);

/**
 * Create local endpoint without opening listening socket.
 *
 * @param[in] uuid               uuid
 *
 * @return                       @a self, @c NULL in case of error
 **/
QBRPCLocalEndpoint
QBRPCLocalEndpointCreateWithoutSocket(QBRPCUUID uuid);

/**
 * Get value of own UUID.
 *
 * @param[in] self               local endpoint handle
 *
 * @return                       uuid
 **/
QBRPCUUID
QBRPCLocalEndpointGetUUID(QBRPCLocalEndpoint self);

/**
 * Add endpoint listener.
 *
 * @param[in] self               local endpoint handle
 * @param[in] listener           listener handle
 *
 **/
void
QBRPCLocalEndpointAddListener(QBRPCLocalEndpoint self, SvObject listener);

/**
 * Remove endpoint listener.
 *
 * @param[in] self               local endpoint handle
 * @param[in] listener           listener handle
 *
 **/
void
QBRPCLocalEndpointRemoveListener(QBRPCLocalEndpoint self, SvObject listener);

/**
 * Start endpoint's main thread.
 *
 * @param[in] self               local endpoint handle
 *
 **/
void
QBRPCLocalEndpointStart(QBRPCLocalEndpoint self);

/**
 * Stop endpoint's main thread.
 *
 * @param[in] self               local endpoint handle
 *
 **/
void
QBRPCLocalEndpointStop(QBRPCLocalEndpoint self);

/**
 * Stop endpoint's main thread, but wait until all outgoing messages are sent.
 */
void
QBRPCLocalEndpointStopGently(QBRPCLocalEndpoint self);

/**
 * Process endpoint events.
 *
 * @param[in] self               local endpoint handle
 *
 **/
void
QBRPCLocalEndpointProcessEvents(QBRPCLocalEndpoint self);

/**
 * Register a service.
 *
 * @param[in] self               local endpoint handle
 * @param[in] name               service name
 * @param[in] service            service handle
 *
 **/
void
QBRPCLocalEndpointRegisterService(QBRPCLocalEndpoint self, SvString name, QBRPCObject service);

/**
 * Unregister a service.
 *
 * @param[in] self               local endpoint handle
 * @param[in] name               service name
 *
 **/
void
QBRPCLocalEndpointUnregisterService(QBRPCLocalEndpoint self, SvString name);

/**
 * Connect to remote endpoint.
 *
 * @param[in] self               local endpoint handle
 * @param[in] uuid               remote endpoint's uuid
 * @param[in] socketPath         path to socket
 *
 **/
void
QBRPCLocalEndpointConnect(QBRPCLocalEndpoint self, QBRPCUUID uuid, SvString socketPath);

/**
 * Get remote endpoints container.
 *
 * @param[in] self               local endpoint handle
 *
 * @return                       vector containing remote endpoints
 *
 **/
SvArray
QBRPCLocalEndpointGetRemoteEndpoints(QBRPCLocalEndpoint self);

/**
 * Get handle to last message source.
 *
 * @param[in] self               local endpoint handle
 *
 * @return                       remote endpoint handle
 *
 **/
SvObject
QBRPCLocalEndpointGetLastMessageSource(QBRPCLocalEndpoint self);

/**
 * @brief Accept / refuse connections from remote endpoints
 * for a given period of time
 * @param self - LocalEndpoint handle
 * @param state - new state (true - accepting)
 * @param timeWindow - time, after which LocalEndpoint will
 * stop accepting incoming connections. 0 - connections will
 * be accpted for unlimited amount of time.
 */
void
QBRPCLocalEndpointAcceptNewConnections(QBRPCLocalEndpoint self, bool state, SvTime timeWindow);

/**
 * @brief Function deletes disconnected remote endpoints and their services
 * @param self - LocalEndpoint handle
 */
void
QBRPCLocalEndpointCleanup(QBRPCLocalEndpoint self);

/**
 * @brief Function checks if there are any messages to be sent to remote endpoints.
 *
 * @param self      QBRPCLocalEndpoint handle
 * @return          true if there are any messages, false otherwise
 */
bool
QBRPCLocalEndpointAreThereMessagesToSend(QBRPCLocalEndpoint self);

/**
 * QBRPCLocalEndpointListener class.
 **/
struct QBRPCLocalEndpointListener_t {
    /**
     * New event to process notification.
     *
     * @param[in] self_                listener handle
     **/
    void (*unhandledEvents)(SvObject self_);
};
typedef struct QBRPCLocalEndpointListener_t* QBRPCLocalEndpointListener;

/**
 * Get QBRPCLocalEndpointListener inteface instance.
 *
 * @return      QBRPCLocalEndpointListener interface instance
 **/
SvInterface
QBRPCLocalEndpointListener_getInterface(void);
#endif // QB_RPC_LOCAL_ENDPOINT_H_
