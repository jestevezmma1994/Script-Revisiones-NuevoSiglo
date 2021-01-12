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

#ifndef QB_RPC_REMOTE_ENDPOINT_H_
#define QB_RPC_REMOTE_ENDPOINT_H_

#include <QBRPC/QBRPCEndpoint.h>
#include <QBRPC/QBRPCLocalEndpoint.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBRPC/QBRPCMessage.h>
#include <stdint.h>


/**
 * QBRPCRemoteEndpoint class.
 * QBRPCRemoteEndpoint represents remote RPC connection handler.
 **/
struct QBRPCRemoteEndpoint_t {
    struct QBRPCEndpoint_t super_;

    // local endpoint handle
    QBRPCLocalEndpoint localEndpoint;

    // connection handle
    SvObject connection;

    // time, when last received monitor message in seconds since 00:00 hours, Jan1, 1970 UTC
    SvTime lastMonitorTime;

    // timeout, after which endpoint is considered disconnected, in seconds
    uint32_t connectionTimeout;

    //polling interval in seconds
    uint32_t monitorInterval;
};
typedef struct QBRPCRemoteEndpoint_t* QBRPCRemoteEndpoint;

/**
 * Get type instance of QBRPCRemoteEndpoint.
 *
 * @return               QBRPCRemoteEndpoint type instance
 **/
SvType
QBRPCRemoteEndpoint_getType(void);

/**
 * QBRPCRemoteEndpoint virtual methods table
 **/
typedef const struct QBRPCRemoteEndpointVTable_ {
    /**
     * SvObject virtual methods table
     **/
    struct SvObjectVTable_ super_;

    /**
     * Service query.
     *
     * @param[in] self               remote endpoint handle
     * @param[in] name               service name
     * @param[in] interfaceName      interface name
     *
     * @return                       @c 0 if successful, @c -1 in case of error
     **/
    int (*queryService)(QBRPCRemoteEndpoint self, SvString name, SvString interfaceName);

    /**
     * Add resource.
     *
     * @param[in] self        remote endpoint handle
     * @param[in] label       resource label
     * @param[in] fd          resource (file) descriptor
     *
     * @return                @c 0 if successful, @c -1 in case of error
     **/
    int (*enqueueResource)(QBRPCRemoteEndpoint self, SvString label, int fd);

    /**
     * Remove resource.
     *
     * @param[in] self        remote endpoint handle
     * @param[in] label       resource label
     *
     * @return                @c 0 if successful, @c -1 in case of error
     **/
    int (*dequeueResource)(QBRPCRemoteEndpoint self, SvString label);

    /**
     * Send rpc message.
     *
     * @param[in] self        remote endpoint handle
     * @param[in] message     message handle
     *
     * @return                @c 0 if successful, @c -1 in case of error
     **/
    int (*sendMessage)(QBRPCRemoteEndpoint self, QBRPCMessage* message);

    /**
     * Set up connection monitoring.
     *
     * @param[in] self        remote endpoint handle
     * @param[in] timeout_    timeout before discarding endpoint, in seconds
     * @param[in] interval_   how frequently endpoint is supposed to send signals, in seconds
     *
     * @return                @c 0 if successful, @c -1 in case of error
     **/
    int (*setConnectionMonitoring)(QBRPCRemoteEndpoint self, uint32_t timeout_, uint32_t interval_);
} *QBRPCRemoteEndpointVTable;


/**
 * Create remote endpoint.
 *
 * @param[in] localEndpoint     local endpoint handle
 * @param[in] connection        connection handle
 *
 * @return                      remote endpoint handle, @c NULL in case of error
 **/
QBRPCRemoteEndpoint
QBRPCRemoteEndpointCreate(QBRPCLocalEndpoint localEndpoint, SvObject connection);

/**
 * Initialize remote endpoint.
 *
 * @param[in] self              remote endpoint handle
 * @param[in] localEndpoint     local endpoint handle
 * @param[in] connection        connection handle
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
int
QBRPCRemoteEndpointInit(QBRPCRemoteEndpoint self, QBRPCLocalEndpoint localEndpoint, SvObject connection);

/**
 * Get local endpoint handle.
 *
 * @param[in] self              remote endpoint handle
 *
 * @return                      local endpoint handle
 **/
QBRPCLocalEndpoint
QBRPCRemoteEndpointGetLocalEndpoint(QBRPCRemoteEndpoint self);

/**
 * Get connection handle.
 *
 * @param[in] self              remote endpoint handle
 *
 * @return                      connection handle
 **/
SvObject
QBRPCRemoteEndpointGetConnection(QBRPCRemoteEndpoint self);

/**
 * @brief Function turns on/off monitoring connection with this remote endpoint
 * @param[in] self QBRPCRemoteEndpoint handle
 * @param[in] timeout_ time, after which connection is considered disconnected
 * @param[in] interval_ polling interval
 * @return    @c 0 if successful, @c -1 in case of error
 */
int
QBRPCRemoteEndpointSetConnectionMonitoring(QBRPCRemoteEndpoint self, uint32_t timeout_, uint32_t interval_);

/**
 * @brief Update time when endpoint was last checked
 * @param[in] self QBRPCRemoteEndpoint handle
 * @param[in] newTime updated last check time
 */
void
QBRPCRemoteEndpointSetLastMonitorTime(QBRPCRemoteEndpoint self, SvTime newTime);

/**
 * @brief Check if remote endpoint hasn't disconnected
 * @param[in] self QBRPCRemoteEndpoint handle
 * @return true if endpoint is disconnected,
 * false if timeout hasn't been exceeded or connection is not monitored
 */
bool
QBRPCRemoteEndpointCheckConnectionTimeout(QBRPCRemoteEndpoint self);
#endif // QB_RPC_REMOTE_ENDPOINT_H_
