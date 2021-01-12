/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_XMLRPC_CLIENT_ERROR_H_
#define SV_XMLRPC_CLIENT_ERROR_H_

/**
 * @file SvXMLRPCClientError.h
 * @brief XML-RPC client error codes
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvXMLRPCClientError XML-RPC client error codes
 * @ingroup SvXMLRPCClient
 * @{
 **/

/**
 * XML-RPC client error domain.
 **/
#define SvXMLRPCClientErrorDomain "com.cubiware.SvXMLRPCClient"

/**
 * XML-RPC client error codes.
 **/
typedef enum {
   /** reserved value, not a valid error code */
   SvXMLRPCClientError_reserved = 0,
   /** internal library error */
   SvXMLRPCClientError_internalError,
   /** XML-RPC request failed on HTTP level */
   SvXMLRPCClientError_httpError,
   /** received invalid XML-RPC response headers */
   SvXMLRPCClientError_invalidResponseHeaders,
   /** received invalid XML-RPC response body */
   SvXMLRPCClientError_invalidResponseBody,
   /** marker for counting error codes */
   SvXMLRPCClientError_last = SvXMLRPCClientError_invalidResponseBody
} SvXMLRPCClientError;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
