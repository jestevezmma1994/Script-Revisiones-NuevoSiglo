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

#ifndef SV_XMLRPC_REQUESTS_QUEUE_H_
#define SV_XMLRPC_REQUESTS_QUEUE_H_

/**
 * @file SvXMLRPCRequestsQueue.h
 * @brief XML-RPC requests queue class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvXMLRPCClient/SvXMLRPCRequest.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvXMLRPCRequestsQueue XML-RPC requests queue class
 * @ingroup SvXMLRPCClient
 * @{
 **/

/**
 * XML-RPC requests queue class.
 * @class SvXMLRPCRequestsQueue
 * @extends SvObject
 **/
typedef struct SvXMLRPCRequestsQueue_ *SvXMLRPCRequestsQueue;


/**
 * Get runtime type identification object representing
 * type of XML-RPC requests queue class.
 *
 * @return XML-RPC requests queue class
 **/
extern SvType
SvXMLRPCRequestsQueue_getType(void);

/**
 * Create XML-RPC requests queue.
 *
 * @memberof SvXMLRPCRequestsQueue
 *
 * @return                 created XML-RPC requests queue, @c NULL in case of error
 **/
extern SvXMLRPCRequestsQueue
SvXMLRPCRequestsQueueCreate(void);

/**
 * Get current request from the queue.
 *
 * @memberof SvXMLRPCRequestsQueue
 *
 * @param[in] self         XML-RPC requests queue handle
 * @return                 currently processed request, @c NULL if queue is empty
 **/
extern SvXMLRPCRequest
SvXMLRPCRequestsQueuePeekCurrentRequest(SvXMLRPCRequestsQueue self);

/**
 * Add XML-RPC request to the queue and start it if possible.
 *
 * @memberof SvXMLRPCRequestsQueue
 *
 * @param[in] self         XML-RPC requests queue handle
 * @param[in] req          XML-RPC request to be added
 * @return                 number of requests added, less than @c 0 in case of error
 **/
extern int
SvXMLRPCRequestsQueueAddToProcess(SvXMLRPCRequestsQueue self,
                                  SvXMLRPCRequest req);

/**
 * Start next XML-RPC request.
 *
 * @memberof SvXMLRPCRequestsQueue
 *
 * @param[in] self         XML-RPC requests queue handle
 * @return                 number of started requests, less than @c 0 in case of error
 **/
extern int
SvXMLRPCRequestsQueueProcessNext(SvXMLRPCRequestsQueue self);

/**
 * Notify the queue that the last request has finished.
 *
 * @memberof SvXMLRPCRequestsQueue
 *
 * @param[in] self        XML-RPC requests queue handle
 * @return                @c 0 on success, less than @c 0 in case of error
 **/
extern int
SvXMLRPCRequestsQueueFinishRequest(SvXMLRPCRequestsQueue self);

/**
 * Cancel single XML-RPC request.
 *
 * @memberof SvXMLRPCRequestsQueue
 *
 * @param[in] self         XML-RPC requests queue handle
 * @param[in] req          XML-RPC request to cancel
 * @return                 number of cancelled requests, less than @c 0 in case of error
 **/
extern int
SvXMLRPCRequestsQueueCancelRequest(SvXMLRPCRequestsQueue self,
                                   SvXMLRPCRequest req);

/**
 * Cancel all XML-RPC requests in the queue.
 *
 * @memberof SvXMLRPCRequestsQueue
 *
 * @param[in] self         XML-RPC requests queue handle
 * @return                 number of cancelled requests, less than @c 0 in case of error
 **/
extern int
SvXMLRPCRequestsQueueCancelAllRequests(SvXMLRPCRequestsQueue self);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
