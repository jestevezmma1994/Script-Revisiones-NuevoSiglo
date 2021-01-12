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

#ifndef SV_HTTP_CLIENT_ERROR_H_
#define SV_HTTP_CLIENT_ERROR_H_

/**
 * @file SvHTTPClientError.h
 * @brief HTTP client error codes
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvHTTPClientError HTTP client error codes
 * @ingroup SvHTTPClient
 * @{
 *
 * Error codes reported by HTTP client library methods.
 **/


/**
 * HTTP client error domain.
 **/
#define SvHTTPClientErrorDomain "com.sentivision.SvHTTPClient"

/**
 * HTTP client error codes.
 **/
typedef enum {
   /** reserved value, not a valid error code */
   SvHTTPClientError_reserved = 0,
   /** transfer cancelled */
   SvHTTPClientError_cancelled,
   /** protocol level error */
   SvHTTPClientError_protocolError,
   /** server returned error code */
   SvHTTPClientError_httpError,
   /** other I/O error */
   SvHTTPClientError_ioError,
   /** @cond */
   SvHTTPClientError_last = SvHTTPClientError_ioError
   /** @endcond */
} SvHTTPClientError;

/**
 * @}
 **/


#endif
