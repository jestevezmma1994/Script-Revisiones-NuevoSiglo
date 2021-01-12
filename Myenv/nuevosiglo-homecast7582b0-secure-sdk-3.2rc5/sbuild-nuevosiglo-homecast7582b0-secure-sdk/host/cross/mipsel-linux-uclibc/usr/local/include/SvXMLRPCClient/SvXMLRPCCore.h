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

#ifndef SV_XMLRPC_CORE_H_
#define SV_XMLRPC_CORE_H_

/**
 * @file SvXMLRPCCore.h
 * @brief XML-RPC core
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvHTTPClient/SvHTTPClientEngine.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvXMLRPCCore XML-RPC core
 * @ingroup SvXMLRPCClient
 * @{
 **/

/**
 * Return a handle to the HTTP client engine that drives
 * XML-RPC requests.
 *
 * @return      handle to an HTTP client engine used by all XML-RPC requests
 **/
extern SvHTTPClientEngine
SvXMLRPCClientGetHTTPClientEngine(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
