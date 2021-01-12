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

#ifndef SV_HTTP_REQUEST_H_
#define SV_HTTP_REQUEST_H_

/**
 * @file SvHTTPRequest.h
 * @brief HTTP client request class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <sys/types.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvHashTable.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <SvHTTPClient/SvOAuthParams.h>

/**
 * @defgroup SvHTTPRequest HTTP client request class
 * @ingroup SvHTTPClient
 * @{
 *
 * HTTP client request class represents single request
 * to the HTTP or HTTPS server.
 *
 * @image html SvHTTPRequest.png
 **/


/**
 * HTTP client request class.
 * @class SvHTTPRequest
 * @extends SvObject
 **/
typedef struct SvHTTPRequest_ *SvHTTPRequest;

/**
 * HTTP client request ID.
 **/
typedef unsigned int SvHTTPRequestID;

/**
 * HTTP client request method.
 **/
typedef enum {
   /**
    * HTTP GET method, the most frequently used one;
    * should be specified also to download file using FTP protocol
    **/
   SvHTTPRequestMethod_GET,
   /// HTTP HEAD method, to get information about remote file
   SvHTTPRequestMethod_HEAD,
   /// HTTP PUT method, to upload file to remote server
   SvHTTPRequestMethod_PUT,
   /**
    * HTTP POST method, usually used to pass arguments to
    * a web service methods
    **/
   SvHTTPRequestMethod_POST,
   /// HTTP DELETE method, to remove file from remote server
   SvHTTPRequestMethod_DELETE
} SvHTTPRequestMethod;

/**
 * HTTP client request state.
 **/
typedef enum {
   /// request object allocated but not initialized
   SvHTTPRequestState_uninitialized = 0,
   /// request ready to be started
   SvHTTPRequestState_valid,
   /**
    * request delayed due to the limit of concurrent requests
    * that can be handled by the client engine
    **/
   SvHTTPRequestState_queued,
   /// sending HTTP headers, POST data or FTP request
   SvHTTPRequestState_sending,
   /// receiving HTTP headers or FTP response code
   SvHTTPRequestState_receivingHeaders,
   /// receiving actual data
   SvHTTPRequestState_receivingData,
   /// request explicitly suspended
   SvHTTPRequestState_suspended,
   /// request explicitly cancelled
   SvHTTPRequestState_cancelled,
   /// request finished successfully
   SvHTTPRequestState_finished,
   /// request failed
   SvHTTPRequestState_failed
} SvHTTPRequestState;

/**
 * HTTP client request errors.
 **/
typedef enum {
   /// no error (request did not failed)
   SvHTTPRequestError_none = 0,
   /// internal library error
   SvHTTPRequestError_internalError,
   /// invalid behaviour of HTTP client listener
   SvHTTPRequestError_listenerError,
   /// could not resolve host name
   SvHTTPRequestError_cantResolve,
   /// could not connect to host or proxy
   SvHTTPRequestError_connectFailed,
   /// download haven't been completed
   SvHTTPRequestError_partialFile,
   /// HTTP server returned error
   SvHTTPRequestError_HTTPError,
   /// operation timed out
   SvHTTPRequestError_timedOut,
   /// I/O error while sending data
   SvHTTPRequestError_sendFailed,
   /// I/O error while receiving data
   SvHTTPRequestError_recvFailed,
   /// remote resource not found
   SvHTTPRequestError_notFound,
   /// error in SSL/TLS handshake
   SvHTTPRequestError_SSLConnectFailed,
   /// local client certificate could not be loaded
   SvHTTPRequestError_SSLCertProblem,
   /// local CA certificates could not be loaded
   SvHTTPRequestError_SSLCACertProblem,
   /// could not use SSL/TLS cipher
   SvHTTPRequestError_cantUseSSLCipher,
   /// peer certificate could not be verified
   SvHTTPRequestError_cantVerifySSLPeer,
   /// HTTP protocol error, such as invalid HTTP header syntax
   SvHTTPRequestError_protocolError,
   /// size of downloaded file exceeds limit
   SvHTTPRequestError_fileTooBig,
   /// connection has been forcefully torn down
   SvHTTPRequestError_torndown,
   /// other unspecified error
   SvHTTPRequestError_unknownError
} SvHTTPRequestError;

/**
 * HTTP compression level.
 **/
typedef enum {
   /// advertise to server that we do not support compression
   SvHTTPRequestCompression_forbid = -1,
   /// default setting (forbid)
   SvHTTPRequestCompression_auto = 0,
   /**
    * advertise to server that we support compression,
    * let server decide whether to use compression
    **/
   SvHTTPRequestCompression_allow = 1,
   /**
    * require compression from HTTP server, fail when server
    * does not support compression
    **/
   SvHTTPRequestCompression_require
} SvHTTPRequestCompression;

/**
 * Get runtime type identification object representing
 * type of HTTP client request class.
 *
 * @return HTTP client request class
 **/
extern SvType
SvHTTPRequest_getType(void);

/**
 * Initialize HTTP client request object.
 *
 * @memberof SvHTTPRequest
 *
 * This method initializes HTTP client request object.
 * Properties of the object other than HTTP method and request URL
 * are set to default values and can be changed before calling
 * SvHTTPRequestStart().
 *
 * @param[in] self      HTTP client request handle
 * @param[in] engine    HTTP client engine (pass @c NULL to use default one)
 * @param[in] method    HTTP method
 * @param[in] URL       HTTP request URL
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvHTTPRequest
SvHTTPRequestInit(SvHTTPRequest self,
                  SvHTTPClientEngine engine,
                  SvHTTPRequestMethod method,
                  const char *URL,
                  SvErrorInfo *errorOut);

/**
 * Initialize HTTP client request object.
 *
 * This method is a different variant of SvHTTPRequestInit().
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] engine    HTTP client engine (pass @c NULL to use default one)
 * @param[in] method    HTTP method
 * @param[in] URL       HTTP request URL
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvHTTPRequest
SvHTTPRequestInitWithURL(SvHTTPRequest self,
                         SvHTTPClientEngine engine,
                         SvHTTPRequestMethod method,
                         SvURL URL,
                         SvErrorInfo *errorOut);

/**
 * Set URL of the HTTP client request.
 *
 * This method changes target URL. It can be used only before request was
 * actually sent, i.e. request's state must be either ::SvHTTPRequestState_valid
 * or ::SvHTTPRequestState_queued.
 *
 * @since 1.0.36
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] URL       HTTP request URL
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetURL(SvHTTPRequest self,
                    SvURL URL,
                    SvErrorInfo *errorOut);

/**
 * Enable/disable collecting HTTP headers.
 *
 * This method makes HTTP request collect all outgoing and incoming
 * HTTP headers. When the request is finished, you can get them using
 * SvHTTPRequestGetOutgoingHeaders() or SvHTTPRequestGetIncomingHeaders().
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] enable    @c true to enable HTTP headers logging
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestCollectHeaders(SvHTTPRequest self,
                            bool enable,
                            SvErrorInfo *errorOut);

/**
 * Get unique request ID of the HTTP client request.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @return              unique request ID
 **/
extern SvHTTPRequestID
SvHTTPRequestGetID(SvHTTPRequest self);

/**
 * Get current state of the HTTP client request.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @return              current state of the HTTP client request
 **/
extern SvHTTPRequestState
SvHTTPRequestGetState(SvHTTPRequest self);

/**
 * Client protocol types.
 **/
typedef enum {
   /// special marker used when protocol is unknown
   SvHTTPClientProto_unknown = 0,
   /// HTTP protocol, at least version 1.0
   SvHTTPClientProto_HTTP,
   /// HTTPS protocol (HTTP over SSL)
   SvHTTPClientProto_HTTPS,
   /// FTP protocol
   SvHTTPClientProto_FTP,
   /// SHOUTcast protocol, special version of HTTP 0.9 protocol
   SvHTTPClientProto_SHOUTcast,
} SvHTTPClientProto;

/**
 * Get data transfer protocol.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @return              data transfer protocol used by the request
 **/
extern SvHTTPClientProto
SvHTTPRequestGetProtocol(SvHTTPRequest self);

/**
 * Get parsed value of the Content-Length header.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @return              content length or @c -1 iff not available
 **/
extern int64_t
SvHTTPRequestGetContentLength(SvHTTPRequest self);

/**
 * Get parsed value of the Content-Type header.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @return              content type or @c NULL iff not available
 **/
extern SvString
SvHTTPRequestGetContentType(SvHTTPRequest self);

/**
 * Get value of an additional HTTP header.
 *
 * This method returns the value of an extra HTTP header that was
 * previously set using SvHTTPRequestSetHTTPHeader().
 *
 * @since 1.0.32
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self       HTTP client request handle
 * @param[in] headerName name of the HTTP header
 * @return               value of given HTTP header or @c NULL if not found
 **/
extern SvString
SvHTTPRequestGetHTTPHeader(SvHTTPRequest self,
                           SvString headerName);

/**
 * Set HTTP outgoing data.
 *
 * This method adds outgoing data to the HTTP client request.
 * It has to be called before SvHTTPRequestStart() for requests
 * using PUT and POST methods.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] contentType content type name for the outgoing data
 * @param[in] data      buffer with data to be sent
 * @param[in] length    number of bytes to be sent
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetOutgoingData(SvHTTPRequest self,
                             SvString contentType,
                             const uint8_t *data,
                             size_t length,
                             SvErrorInfo *errorOut);

/**
 * Set IPv4/IPv6 name resolver parameters.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] useIPv4   @c true to allow resolving names to IPv4 addresses
 * @param[in] useIPv6   @c true to allow resolving names to IPv6 addresses
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetResolverParams(SvHTTPRequest self,
                               bool useIPv4,
                               bool useIPv6,
                               SvErrorInfo *errorOut);

/**
 * Set HTTP user agent string.
 *
 * @memberof SvHTTPRequest
 *
 * This method sets the name of user agent that will be sent in HTTP
 * request as a value of 'User-Agent' HTTP header. If @a userAgent is
 * @c NULL, formerly set user agent string is removed. If request has no
 * user agent string, the default one from the HTTP client will be used.
 *
 * @param[in] self      HTTP client request handle
 * @param[in] userAgent user agent string (@c NULL to remove)
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetUserAgent(SvHTTPRequest self,
                          SvString userAgent,
                          SvErrorInfo *errorOut);

/**
 * Set HTTP proxy.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] proxy     proxy string in "address:port" format, @c NULL to remove
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetProxy(SvHTTPRequest self,
                      SvString proxy,
                      SvErrorInfo *errorOut);

/**
 * Set the value of additional HTTP header.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] headerName name of the HTTP header
 * @param[in] value     value of the HTTP header (@c NULL to remove)
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetHTTPHeader(SvHTTPRequest self,
                           SvString headerName, SvString value,
                           SvErrorInfo *errorOut);

/**
 * Set user name and password for HTTP/FTP authentication.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] userName  user name (@c NULL to disable)
 * @param[in] password  password
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetAuthorization(SvHTTPRequest self,
                              SvString userName, SvString password,
                              SvErrorInfo *errorOut);

/**
 * Set SSL parameters.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] sslParams SSL parameters handle
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetSSLParams(SvHTTPRequest self,
                          SvSSLParams sslParams,
                          SvErrorInfo *errorOut);

/**
 * Set OAuth 1.0 parameters.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self        HTTP client request handle
 * @param[in] oAuthParams OAuth 1.0 parameters handle
 * @param[out] errorOut   error info
 **/
extern void
SvHTTPRequestSetOAuthParams(SvHTTPRequest self,
                            SvOAuthParams oAuthParams,
                            SvErrorInfo *errorOut);

/**
 * Set desired download range.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] start     index of the first byte to be fetched
 * @param[in] length    number of bytes to be fetched
 *                      (use @c -1 to fetch entire file)
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetDownloadRange(SvHTTPRequest self,
                              off_t start, off_t length,
                              SvErrorInfo *errorOut);

/**
 * Set max file size.
 *
 * This method sets upper limit on downloaded data length.
 * When there is more data, @ref SvHTTPRequestError_fileTooBig
 * error will be reported.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] maxFileSize max size of file in bytes
 **/
extern void
SvHTTPRequestSetMaxFileSize(SvHTTPRequest self,
                            off_t maxFileSize);

/**
 * Set HTTP client request listener.
 *
 * This method installs a listener object for the HTTP client request.
 * Listener must implement @ref SvHTTPClientListener.
 *
 * @note Client request class holds only a weak reference to the listener.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] listener  HTTP client request listener handle
 * @param[in] readHeaders @c true to pass received HTTP headers to the listener
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetListener(SvHTTPRequest self,
                         SvGenericObject listener,
                         bool readHeaders,
                         SvErrorInfo *errorOut);

/**
 * Change I/O processing policy of the HTTP client request.
 *
 * This method changes the policy of I/O processing of the request.
 * By default requests have real-time I/O processing policy; such
 * policy is desired for requests that do not carry lots of data,
 * but should be processed as fast as possible. For typical data
 * requests, such as multimedia streaming, file downloads or RPC calls
 * that return lots of response data, real-time policy should
 * probably be explicitly turned off.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] realTime  @c true to enable real-time I/O processing,
 *                      @c false to disable
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetIOPolicy(SvHTTPRequest self,
                         bool realTime,
                         SvErrorInfo *errorOut);

/**
 * Request priorities.
 **/
enum {
   /// special priority value that means "start this request immediately"
   SvHTTPRequestPriority_preempt = -1,
   /// highest priority
   SvHTTPRequestPriority_max = 0,
   /// default priority
   SvHTTPRequestPriority_default = 7,
   /// lowest priority
   SvHTTPRequestPriority_min = INT_MAX
};

/**
 * Change priority of the HTTP client request.
 *
 * This method sets new priority of the request.
 * If the request is already processed, HTTP client engine
 * will be forced to reschedule its requests according to new
 * priorities.
 *
 * Priorities are generally in the range [@c 0; @c INT_MAX], @c 0 being
 * the highest priority. Setting priority to @ref SvHTTPRequestPriority_preempt
 * will cause HTTP client engine to immediately start processing this
 * request, possibly preempting (and thus delaying) some other requests.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] priority  new priority of the HTTP request
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetPriority(SvHTTPRequest self,
                         int priority,
                         SvErrorInfo *errorOut);

/**
 * Change HTTP client request timeout value.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] timeout   new timeout value in seconds
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetTimeout(SvHTTPRequest self,
                        unsigned int timeout,
                        SvErrorInfo *errorOut);

/**
 * Change HTTP max download speed value.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] maxSpeed  max download speed in bytes per second
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetMaxSpeed(SvHTTPRequest self,
                         unsigned long maxSpeed,
                         SvErrorInfo *errorOut);

/**
 * Change HTTP download buffer size.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] bufferSize HTTP download buffer size in bytes
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetBufferSize(SvHTTPRequest self,
                           size_t bufferSize,
                           SvErrorInfo *errorOut);

/**
 * Set requested HTTP compression level.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] level     requested compression level
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetCompression(SvHTTPRequest self,
                            SvHTTPRequestCompression level,
                            SvErrorInfo *errorOut);

/**
 * Change HTTP keep-alive behaviour.
 *
 * @since 1.0.32
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[in] forceConnect @c true to explicitly use new TCP connection
 *                      for this request, @c false to allow using cached
 *                      keep-alive connections (default)
 * @param[in] forbidReuse @c true to close the TCP connection used by this
 *                      request after it is finished, @c false to allow using
 *                      this connection for consecutive requests (default)
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSetKeepAliveOptions(SvHTTPRequest self,
                                 bool forceConnect,
                                 bool forbidReuse,
                                 SvErrorInfo *errorOut);

/**
 * Start asynchronous HTTP client request.
 *
 * This method starts the state machine of the HTTP request.
 * The state of this request can be polled using SvHTTPRequestGetState().
 * To read headers or data fetched from the HTTP server, HTTP listener
 * must be registered in the request - otherwise all fetched data will be
 * dropped.
 *
 * If this is an HTTPS request and its SSL parameters were not set (see
 * SvHTTPRequestSetSSLParams()), default secure SSL parameters will be
 * used as returned by SvSSLParamsCreateDefault().
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestStart(SvHTTPRequest self,
                   SvErrorInfo *errorOut);

/**
 * Suspend asynchronous HTTP client request.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestSuspend(SvHTTPRequest self,
                     SvErrorInfo *errorOut);

/**
 * Resume suspended asynchronous HTTP client request.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestResume(SvHTTPRequest self,
                    SvErrorInfo *errorOut);

/**
 * Cancel ongoing HTTP client request.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[out] errorOut error info
 **/
extern void
SvHTTPRequestCancel(SvHTTPRequest self,
                    SvErrorInfo *errorOut);

/**
 * Get final URL after all redirections.
 *
 * @since 1.0.38
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[out] errorOut error info
 * @return              URL after redirections, @c NULL in case of error
 **/
extern SvURL
SvHTTPRequestGetEffectiveURL(SvHTTPRequest self,
                             SvErrorInfo *errorOut);

/**
 * Get the original URL the request is connecting to.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self     HTTP client request handle
 * @return             original URL
 **/
extern SvURL
SvHTTPRequestGetURL(SvHTTPRequest self);


/**
 * Get error code of failed HTTP client request.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[out] errorOut error info
 * @return              error code
 **/
extern SvHTTPRequestError
SvHTTPRequestGetErrorCode(SvHTTPRequest self,
                          SvErrorInfo *errorOut);

/**
 * Get error code returned by HTTP server.
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[out] errorOut error info
 * @return              error code, @c 0 if unavailable
 **/
extern unsigned int
SvHTTPRequestGetHTTPErrorCode(SvHTTPRequest self,
                              SvErrorInfo *errorOut);

/**
 * Get HTTP headers sent by HTTP client request.
 *
 * @note Collecting HTTP headers (see SvHTTPRequestCollectHeaders())
 * must be enabled to use this method.
 *
 * @since 1.0.35
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[out] errorOut error info
 * @return              all sent HTTP headers
 **/
extern SvHashTable
SvHTTPRequestGetOutgoingHeaders(SvHTTPRequest self,
                                SvErrorInfo *errorOut);

/**
 * Get HTTP headers received by HTTP client request.
 *
 * @note Collecting HTTP headers (see SvHTTPRequestCollectHeaders())
 * must be enabled to use this method.
 *
 * @since 1.0.35
 *
 * @memberof SvHTTPRequest
 *
 * @param[in] self      HTTP client request handle
 * @param[out] errorOut error info
 * @return              all received HTTP headers
 **/
extern SvHashTable
SvHTTPRequestGetIncomingHeaders(SvHTTPRequest self,
                                SvErrorInfo *errorOut);

/**
 * Get name of the HTTP method.
 *
 * @since 1.0.33
 *
 * @param[in] method    HTTP method
 * @return              printable name of the @a method
 **/
extern const char *
SvHTTPRequestGetMethodName(SvHTTPRequestMethod method);

/**
 * Get printable name of the HTTP request error code.
 *
 * @since 1.0.33
 *
 * @param[in] errorCode HTTP request error code
 * @return              printable name of the @a errorCode
 **/
extern const char *
SvHTTPRequestGetErrorName(SvHTTPRequestError errorCode);

/**
 * @}
 **/


#endif
