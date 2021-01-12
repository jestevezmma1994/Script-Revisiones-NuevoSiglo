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

#ifndef QB_RPC_ENDPOINT_H_
#define QB_RPC_ENDPOINT_H_

#include <QBRPC/QBRPCUUID.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

/**
 * QBRPCEndpoint class.
 **/
struct QBRPCEndpoint_t {
    struct SvObject_ super_;

    QBRPCUUID uuid;
};
typedef struct QBRPCEndpoint_t* QBRPCEndpoint;

/**
 * Get type instance of QBRPCEndpoint.
 *
 * @return               QBRPCEndpoint type instance
 **/
SvType
QBRPCEndpoint_getType(void);

/**
 * Create QBRPCEndpoint instance.
 * QBRPCEndpoint represents RPC connection handler.
 *
 * @param[in] uuid       endpoint uuid
 *
 * @return               @a self or @c NULL in case of error
 **/
QBRPCEndpoint
QBRPCEndpointCreate(QBRPCUUID uuid);

/**
 * Initialize QBRPCEndpoint.
 *
 * @param[in] self       endpoint handle
 * @param[in] uuid       endpoint uuid
 *
 * @return               @c 0 if successful, @c -1 in case of error
 **/
int
QBRPCEndpointInit(QBRPCEndpoint self, QBRPCUUID uuid);

/**
 * Get endpoint's uuid.
 *
 * @param[in] self       endpoint handle
 *
 * @return               endpoint uuid
 **/
QBRPCUUID
QBRPCEndpointGetUUID(QBRPCEndpoint self);


#endif // QB_RPC_ENDPOINT_H_
