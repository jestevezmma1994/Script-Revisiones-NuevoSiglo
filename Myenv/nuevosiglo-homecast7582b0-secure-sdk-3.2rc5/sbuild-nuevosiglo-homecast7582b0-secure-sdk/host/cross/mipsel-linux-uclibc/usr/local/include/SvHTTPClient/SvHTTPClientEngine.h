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

#ifndef SV_HTTP_CLIENT_ENGINE_H_
#define SV_HTTP_CLIENT_ENGINE_H_

/**
 * @file SvHTTPClientEngine.h
 * @brief HTTP client engine class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <SvHTTPClient/SvHTTPFileCache.h>


/**
 * @defgroup SvHTTPClientEngine HTTP client engine class
 * @ingroup SvHTTPClient
 * @{
 *
 * Actual implementation of a pool of HTTP requests.
 **/


/**
 * HTTP client engine class.
 * @class SvHTTPClientEngine
 * @extends SvObject
 **/
typedef struct SvHTTPClientEngine_ *SvHTTPClientEngine;


/**
 * Get runtime type identification object representing
 * type of HTTP client engine class.
 *
 * @return HTTP client engine class
 **/
extern SvType
SvHTTPClientEngine_getType(void);

/**
 * Initialize HTTP client engine object with explicit scheduler instance.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self         HTTP client engine handle
 * @param[in] scheduler    handle to the scheduler to be used
 * @param[in] maxRequests  max number of concurrently processed requests
 * @param[out] errorOut    error info
 * @return                 @a self or @c NULL in case of error
 **/
extern SvHTTPClientEngine
SvHTTPClientEngineInitWithScheduler(SvHTTPClientEngine self,
                                    SvScheduler scheduler,
                                    unsigned int maxRequests,
                                    SvErrorInfo *errorOut);

/**
 * Initialize HTTP client engine object with default scheduler.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self         HTTP client engine handle
 * @param[in] maxRequests  max number of concurrently processed requests
 * @param[out] errorOut    error info
 * @return                 @a self or @c NULL in case of error
 **/
extern SvHTTPClientEngine
SvHTTPClientEngineInit(SvHTTPClientEngine self,
                       unsigned int maxRequests,
                       SvErrorInfo *errorOut);

/**
 * Get default HTTP client engine instance.
 *
 * This function returns the default HTTP client engine instance.
 * If it does not exist yet, this function tries to create one.
 *
 * @param[out] errorOut    error info
 * @return                 default HTTP client engine instance
 *                         or @c NULL in case of error
 **/
extern SvHTTPClientEngine
SvHTTPClientEngineGetDefaultInstance(SvErrorInfo *errorOut);

/**
 * Set name of the HTTP client engine instance.
 *
 * This function sets the (optional) name of the HTTP client engine
 * instance. This name is not really needed for it to work, but
 * can be useful to recognise different engine instances running
 * in parallel while debugging.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self         HTTP client engine handle
 * @param[in] name         descriptive name
 **/
extern void
SvHTTPClientEngineSetName(SvHTTPClientEngine self,
                          SvString name);

/**
 * Set support for cookies.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self          HTTP client engine handle
 * @param[in] enableCookies @c true to enable cookies, @c false to disable
 * @param[out] errorOut     error info
 **/
extern void
SvHTTPClientEngineSetupCookies(SvHTTPClientEngine self,
                               bool enableCookies,
                               SvErrorInfo *errorOut);

/**
 * Get name of the HTTP client engine instance.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self         HTTP client engine handle
 * @return                 name of the HTTP client engine instance
 *                         or @c NULL in case of error
 **/
extern SvString
SvHTTPClientEngineGetName(SvHTTPClientEngine self);

/**
 * Get file cache object bound to the HTTP client engine.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self         HTTP client engine handle
 * @return                 HTTP client file cache handle
 *                         or @c NULL in case of error
 **/
extern SvHTTPFileCache
SvHTTPClientEngineGetFileCache(SvHTTPClientEngine self);

/**
 * Set default IPv4/IPv6 name resolver parameters for HTTP client engine.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self         HTTP client engine handle
 * @param[in] useIPv4      @c true to allow resolving names to IPv4 addresses
 * @param[in] useIPv6      @c true to allow resolving names to IPv6 addresses
 * @param[out] errorOut    error info
 **/
extern void
SvHTTPClientEngineSetResolverParams(SvHTTPClientEngine self,
                                    bool useIPv4,
                                    bool useIPv6,
                                    SvErrorInfo *errorOut);

/**
 * Set default SSL parameters for HTTP client engine.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self         HTTP client engine handle
 * @param[in] sslParams    SSL parameters handle
 * @param[out] errorOut    error info
 **/
extern void
SvHTTPClientEngineSetSSLParams(SvHTTPClientEngine self,
                               SvSSLParams sslParams,
                               SvErrorInfo *errorOut);

/**
 * Set default HTTP file cache for HTTP client engine.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self         HTTP client engine handle
 * @param[in] fileCache    HTTP cache handle
 * @param[out] errorOut    error info
 **/
extern void
SvHTTPClientEngineSetFileCache(SvHTTPClientEngine self,
                               SvHTTPFileCache fileCache,
                               SvErrorInfo *errorOut);

/**
 * Set default user agent name for HTTP client engine.
 *
 * This method sets the default user agent name that will be sent
 * in HTTP requests as a value of 'User-Agent' HTTP header.
 * This default value can be overriden per request using
 * SvHTTPRequestSetUserAgent().
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self         HTTP client engine handle
 * @param[in] userAgent    user agent name
 * @param[out] errorOut    error info
 **/
extern void
SvHTTPClientEngineSetUserAgent(SvHTTPClientEngine self,
                               SvString userAgent,
                               SvErrorInfo *errorOut);

/**
 * Set default value of additional HTTP header.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self      HTTP client engine handle
 * @param[in] headerName name of the HTTP header
 * @param[in] value     value of the HTTP header (@c NULL to remove)
 * @param[out] errorOut error info
 **/
extern void
SvHTTPClientEngineSetHTTPHeader(SvHTTPClientEngine self,
                                SvString headerName, SvString value,
                                SvErrorInfo *errorOut);

/**
 * Set http pipelining.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self      HTTP client engine handle
 * @param[in] enable    @c true to set http pipelining
 * @param[out] errorOut error info
 **/
extern void
SvHTTPClientEngineSetPipelining(SvHTTPClientEngine self,
                                bool enable,
                                SvErrorInfo *errorOut);

/**
 * Check weather http pipelining is enabled.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self         HTTP client engine handle
 * @return                 @c true if http pipelining is enabled,
 *                         otherwise @c false
 **/
extern bool
SvHTTPClientEngineGetPipelining(SvHTTPClientEngine self);

/**
 * Get handle to a scheduler driving HTTP client engine instance.
 *
 * @memberof SvHTTPClientEngine
 *
 * @param[in] self         HTTP client engine object handle
 * @return                 scheduler handle, @c NULL in case of error
 **/
extern SvScheduler
SvHTTPClientEngineGetScheduler(SvHTTPClientEngine self);

/**
 * @}
 **/


#endif
