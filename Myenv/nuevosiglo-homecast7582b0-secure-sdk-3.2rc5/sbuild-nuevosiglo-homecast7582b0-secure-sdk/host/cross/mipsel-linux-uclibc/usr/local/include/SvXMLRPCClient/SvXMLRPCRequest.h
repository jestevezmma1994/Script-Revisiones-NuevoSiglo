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

#ifndef SV_XMLRPC_REQUEST_H_
#define SV_XMLRPC_REQUEST_H_

/**
 * @file SvXMLRPCRequest.h
 * @brief XML-RPC request class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdarg.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvFoundation/SvHashTable.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvHTTPClient/SvHTTPRequest.h>
#include <SvXMLRPCClient/SvXMLRPCClientError.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvXMLRPCRequest XML-RPC request class
 * @ingroup SvXMLRPCClient
 * @{
 **/

/**
 * XML-RPC request class.
 * @class SvXMLRPCRequest
 * @extends SvObject
 **/
typedef struct SvXMLRPCRequest_ *SvXMLRPCRequest;

/**
 * XML-RPC request ID.
 **/
typedef unsigned int SvXMLRPCRequestID;

/**
 * XML-RPC request state.
 **/
typedef enum {
    /// Request has not yet been initialized
    SvXMLRPCRequestState_uninitialized = 0,
    /// Request has been initialized, but is not yet started
    SvXMLRPCRequestState_ready,
    /// Request has been started and it was already sent
    SvXMLRPCRequestState_sent,
    /// Request has received a response and is parsing it
    SvXMLRPCRequestState_parsing,
    /// Request has received a non-exception response
    SvXMLRPCRequestState_gotAnswer,
    /// Request has received a exception
    SvXMLRPCRequestState_gotException,
    /// Request has been cancelled
    SvXMLRPCRequestState_cancelled,
    /// There was some error in request processing either during sending
    /// or receiving response
    SvXMLRPCRequestState_error
} SvXMLRPCRequestState;


/**
 * Get runtime type identification object representing
 * type of XML-RPC request class.
 *
 * @return XML-RPC request class
 **/
extern SvType
SvXMLRPCRequest_getType(void);

/**
 * Initialize XML-RPC request object.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @param[in] serverInfo   XML-RPC server info handle
 * @param[in] methodName   XML-RPC method name
 * @param[in] arguments    method arguments
 * @param[out] errorOut    error info
 * @return                 @a self or @c NULL in case of error
 **/
extern SvXMLRPCRequest
SvXMLRPCRequestInit(SvXMLRPCRequest self,
                    SvXMLRPCServerInfo serverInfo,
                    SvString methodName,
                    SvImmutableArray arguments,
                    SvErrorInfo *errorOut);

/**
 * Create and initialize XML-RPC request object using parameters encoded in JSON format.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] serverInfo   XML-RPC server info handle
 * @param[in] method       XML-RPC method name
 * @param[out] errorOut    error info
 * @param[in] jsonTemplate method arguments in JSON format
 * @return                 created XML-RPC request object, @c NULL in case of error
 **/
extern SvXMLRPCRequest
SvXMLRPCRequestCreateWithJSONTemplate(SvXMLRPCServerInfo serverInfo,
                                      SvString method,
                                      SvErrorInfo *errorOut,
                                      const char *jsonTemplate,
                                      ...);

/**
 * Create and initialize XML-RPC request object using parameters encoded in JSON format.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] serverInfo   XML-RPC server info handle
 * @param[in] method       XML-RPC method name
 * @param[in] jsonTemplate method arguments in JSON format
 * @param[in] args         arguments for @a jsonTemplate
 * @param[out] errorOut    error info
 * @return                 created XML-RPC request object, @c NULL in case of error
 **/
extern SvXMLRPCRequest
SvXMLRPCRequestCreateWithJSONTemplateV(SvXMLRPCServerInfo serverInfo,
                                       SvString method,
                                       const char *jsonTemplate,
                                       va_list args,
                                       SvErrorInfo *errorOut);

/**
 * Get XML-RPC method name.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @return                 XML-RPC method name, @c NULL in case of error
 **/
extern SvString
SvXMLRPCRequestGetMethodName(SvXMLRPCRequest self);

/**
 * Set XML-RPC request timeout.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @param[in] timeout      timeout value in seconds, @c 0 for no timeout
 * @param[out] errorOut    error info
 **/
extern void
SvXMLRPCRequestSetTimeout(SvXMLRPCRequest self,
                          unsigned int timeout,
                          SvErrorInfo *errorOut);

/**
 * Set XML-RPC data listener.
 *
 * This method installs data listener object for the XML-RPC response.
 * Listener must implement @ref SvXMLRPCDataListener.
 * When data listener is not set when XML-RPC request starts,
 * an instance of SvXMLRPCDataReader class is created to be used as one.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @param[in] listener     XML-RPC data listener handle
 * @param[out] errorOut    error info
 **/
extern void
SvXMLRPCRequestSetDataListener(SvXMLRPCRequest self,
                               SvObject listener,
                               SvErrorInfo *errorOut);

/**
 * Get XML-RPC data listener.
 *
 * This method returns a handle to the XML-RPC data listener object.
 * This can be a custom listener object installed by
 * SvXMLRPCRequestSetDataListener() or a default one.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @param[out] errorOut    error info
 * @return                 XML-RPC data listener handle
 **/
extern SvObject
SvXMLRPCRequestGetDataListener(SvXMLRPCRequest self,
                               SvErrorInfo *errorOut);

/**
 * Set XML-RPC client listener.
 *
 * This method installs a listener object for the XML-RPC response.
 * Listener must implement @ref SvXMLRPCClientListener.
 *
 * @warning XML-RPC request keeps hard reference to a listener installed using
 * this method. It is a deprecated practice, so this method should not be used
 * in new code; use SvXMLRPCRequestAddListener() instead. Mixing both methods
 * of registering listeners is considered dangerous, so this method will fail
 * if SvXMLRPCRequestAddListener() was used before.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @param[in] listener     XML-RPC listener handle, pass @c NULL
 *                         to remove existing listener
 * @param[out] errorOut    error info
 **/
extern void
SvXMLRPCRequestSetListener(SvXMLRPCRequest self,
                           SvObject listener,
                           SvErrorInfo *errorOut);

/**
 * Register new XML-RPC client listener.
 *
 * This method registers a listener object for the XML-RPC response.
 * Listener must implement @ref SvXMLRPCClientListener.
 *
 * @memberof SvXMLRPCRequest
 * @since 1.1.5
 *
 * @param[in] self         XML-RPC request handle
 * @param[in] listener     XML-RPC listener handle
 * @param[out] errorOut    error info
 **/
extern void
SvXMLRPCRequestAddListener(SvXMLRPCRequest self,
                           SvObject listener,
                           SvErrorInfo *errorOut);

/**
 * Unregister XML-RPC client listener.
 *
 * This method unregisters a listener object that have been previously
 * registered using SvXMLRPCRequestAddListener().
 *
 * @memberof SvXMLRPCRequest
 * @since 1.1.5
 *
 * @param[in] self         XML-RPC request handle
 * @param[in] listener     XML-RPC listener handle
 * @param[out] errorOut    error info
 **/
extern void
SvXMLRPCRequestRemoveListener(SvXMLRPCRequest self,
                              SvObject listener,
                              SvErrorInfo *errorOut);

/**
 * Get unique request ID.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @return                 unique request ID
 **/
extern SvXMLRPCRequestID
SvXMLRPCRequestGetID(SvXMLRPCRequest self);

/**
 * Get request state.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @return                 request state
 **/
extern SvXMLRPCRequestState
SvXMLRPCRequestGetState(SvXMLRPCRequest self);

/**
 * Set HTTP client engine to be used by XML-RPC call.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @param[in] engine       HTTP client engine handle
 * @param[out] errorOut    error info
 **/
extern void
SvXMLRPCRequestSetHTTPClientEngine(SvXMLRPCRequest self,
                                   SvHTTPClientEngine engine,
                                   SvErrorInfo *errorOut);

/**
 * Start asynchronous XML-RPC call.
 *
 * This method starts the state machine of the XML-RPC call.
 * The state of this request can be polled using SvXMLRPCRequestGetState()
 * or monitored asynchronously via the listener interface.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @param[out] errorOut    error info
 **/
extern void
SvXMLRPCRequestStart(SvXMLRPCRequest self,
                     SvErrorInfo *errorOut);

/**
 * Get HTTP request object that serves as a transport for XML-RPC call.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @param[out] errorOut    error info
 * @return                 HTTP request handle
 **/
extern SvHTTPRequest
SvXMLRPCRequestGetHTTPRequest(SvXMLRPCRequest self,
                              SvErrorInfo *errorOut);

/**
 * Cancel previously started XML-RPC call.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @param[out] errorOut    error info
 **/
extern void
SvXMLRPCRequestCancel(SvXMLRPCRequest self,
                      SvErrorInfo *errorOut);

/**
 * Get exception.
 *
 * This method returns the XML-RPC exception code and description.
 * Result is valid only when request object is in
 * @ref SvXMLRPCRequestState_gotException state.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self         XML-RPC request handle
 * @param[out] faultCode   XML-RPC fault code
 * @param[out] faultString XML-RPC fault string
 * @param[out] errorOut    error info
 **/
extern void
SvXMLRPCRequestGetException(SvXMLRPCRequest self,
                            int *faultCode,
                            SvString *faultString,
                            SvErrorInfo *errorOut);

/**
 * Get textual description of a request.
 *
 * This method returns a string describing XML-RPC request.
 * It can be used for debugging purposes.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self          XML-RPC request handle
 * @return                  textual description of the request
 **/
extern SvString
SvXMLRPCRequestGetDescription(SvXMLRPCRequest self);

/**
 * Get error code of failed XML-RPC request.
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self      XML-RPC request handle
 * @param[out] errorOut error info
 * @return              error code
 **/
extern SvXMLRPCClientError
SvXMLRPCRequestGetErrorCode(SvXMLRPCRequest self,
                            SvErrorInfo *errorOut);

/**
 * Get printable name of the XML-RPC request error code.
 *
 * @since 1.1.6
 *
 * @param[in] errorCode XML-RPC request error code
 * @return              printable name of the @a errorCode
 **/
extern const char *
SvXMLRPCRequestGetErrorName(SvXMLRPCClientError errorCode);

/**
 * Get error codes from HTTPRequest inside XMLRPCRequest
 *
 * @memberof SvXMLRPCRequest
 *
 * @param[in] self               XML-RPC request handle
 * @param[out] httpErrorCode     HTTP error code
 * @param[out] requestErrorCode  HTTPRequest error code
 * @param[out] errorOut          error info
 **/
void
SvXMLRPCRequestGetHTTPRequestStatus(SvXMLRPCRequest self,
                                    int *httpErrorCode,
                                    SvHTTPRequestError *requestErrorCode,
                                    SvErrorInfo *errorOut);

/**
 * Create JSON log from XMLRPC request data
 * example: {result:"gotException",request:"sample",httpErrorCode:200,httpRequestErrorCode:0,exceptionCode:2,exceptionDescr:"sample",errorCode:3}
 *
 * @param[in] request       XMLRCP request handle
 * @param[in] extraMsgFmt   additional message format specification, it is escaped JSON message with comma at the beggining ex: ",\"example\": 23,\"example2\":\"sample\""
 * @param[in] ...           arguments for format specification
 * @return                  secure log JSON message generated from request data
 */
SvString
SvXMLRPCRequestCreateJSONLog(SvXMLRPCRequest request,
                             const char *extraMsgFmt, ...);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
