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

#ifndef SV_XMLRPC_SERVER_INFO_H_
#define SV_XMLRPC_SERVER_INFO_H_

/**
 * @file SvXMLRPCServerInfo.h
 * @brief XML-RPC server info class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURL.h>
#include <SvHTTPClient/SvSSLParams.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvXMLRPCServerInfo XML-RPC server info class
 * @ingroup SvXMLRPCClient
 * @{
 **/

/**
 * XML-RPC server info class.
 * @class SvXMLRPCServerInfo
 * @extends SvObject
 **/
typedef struct SvXMLRPCServerInfo_ *SvXMLRPCServerInfo;


/**
 * Get runtime type identification object representing
 * type of XML-RPC server info class.
 *
 * @return XML-RPC server info class
 **/
extern SvType
SvXMLRPCServerInfo_getType(void);

/**
 * Initialize XML-RPC server info object.
 *
 * @memberof SvXMLRPCServerInfo
 *
 * @param[in] self      XML-RPC server info handle
 * @param[in] baseURL   XML-RPC server URL
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvXMLRPCServerInfo
SvXMLRPCServerInfoInit(SvXMLRPCServerInfo self,
                       SvString baseURL,
                       SvErrorInfo *errorOut);

/**
 * Set XML-RPC server authentication information.
 *
 * @memberof SvXMLRPCServerInfo
 *
 * @param[in] self      XML-RPC server info handle
 * @param[in] userName  user name for HTTP Basic authentication
 * @param[in] password  password for HTTP Basic authentication
 * @param[out] errorOut error info
 **/
extern void
SvXMLRPCServerInfoSetAuthInfo(SvXMLRPCServerInfo self,
                              SvString userName,
                              SvString password,
                              SvErrorInfo *errorOut);

/**
 * Set SSL parameters.
 *
 * @memberof SvXMLRPCServerInfo
 *
 * @param[in] self      XML-RPC server info handle
 * @param[in] sslParams SSL parameters
 * @param[out] errorOut error info
 **/
extern void
SvXMLRPCServerInfoSetSSLParams(SvXMLRPCServerInfo self,
                               SvSSLParams sslParams,
                               SvErrorInfo *errorOut);

/**
 * Get base URL.
 *
 * @memberof SvXMLRPCServerInfo
 *
 * @param[in] self      XML-RPC server info handle
 * @param[out] errorOut error info
 * @return              base URL
 */
extern SvURL
SvXMLRPCServerInfoGetBaseURL(SvXMLRPCServerInfo self,
                             SvErrorInfo *errorOut);

/**
 * Set base URL.
 *
 * @memberof SvXMLRPCServerInfo
 *
 * @param[in] self      XML-RPC server info handle
 * @param[in] baseURL   XML-RPC server URL
 * @param[out] errorOut error info
 */
extern void
SvXMLRPCServerInfoSetBaseURL(SvXMLRPCServerInfo self,
                             SvString baseURL,
                             SvErrorInfo *errorOut);
/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
