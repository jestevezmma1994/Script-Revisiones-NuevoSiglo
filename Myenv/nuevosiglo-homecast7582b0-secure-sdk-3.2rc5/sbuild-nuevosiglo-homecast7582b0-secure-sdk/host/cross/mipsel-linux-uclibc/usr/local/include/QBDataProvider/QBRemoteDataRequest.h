/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_REMOTE_DATA_REQUEST_H_
#define QB_REMOTE_DATA_REQUEST_H_

/**
 * @file QBRemoteDataRequest.h
 * @brief Base class for remote data requests
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBDataProvider/QBRemoteSessionManager.h>
#include <SvHTTPClient/SvHTTPRequest.h>
#include <QBAppKit/QBObservable.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvFoundation/SvCoreTypes.h>
#include <fibers/c/fibers.h>
#include <stdlib.h>

/**
 * @defgroup QBRemoteDataRequest Remote data request
 * @ingroup QBDataProvider
 * @{
 **/

/**
 * Default chunk threshold.
 **/
#define DEFAULT_PARSER_CHUNK_THRESHOLD (64 * 1024) // 64 kiB

/**
 * Default environment variable used to enable tracing.
 **/
#define DEFAULT_TRACE_ENV ("QB_REMOTE_REQUEST_TRACE_FD")

/**
 * State of a remote data request or transaction.
 **/
typedef enum {
    /// reserved value for uninitialized requests
    QBRemoteDataRequestState_unknown = 0,
    /// request can be started
    QBRemoteDataRequestState_ready,
    /// request is in progress
    QBRemoteDataRequestState_inProgress,
    /// data transfer is finished, but has not been parsed yet
    QBRemoteDataRequestState_transferFinished,
    /// request finished successfully
    QBRemoteDataRequestState_finished,
    /// request has been cancelled
    QBRemoteDataRequestState_cancelled,
    /// request failed
    QBRemoteDataRequestState_failed
} QBRemoteDataRequestState;

/**
 * Convert QBRemoteDataRequestState to human readable representation.
 *
 * @param[in] self remote data request state
 * @return         C string containing human readable representation of
 *                 remote data request state when SV_LOG_LEVEL > 0
 *                 or empty C string ("") otherwise
 **/
const char *
QBRemoteDataRequestStateToCString(QBRemoteDataRequestState self);

/**
 * Error codes of a remote data request or transaction.
 *
 * @note This values are used only in Innov8onProviderTransaction and should be
 *       removed from this file in the future.
 **/
enum {
    /// no errors
    QBRemoteDataRequestStateErrorCode_noErrors = 0,
    /// requested price is different then offered one
    QBRemoteDataRequestStateErrorCode_invalidRequestedPrice = 65,
    /// not enough credits to finish transaction
    QBRemoteDataRequestStateErrorCode_notEnoughCredits = 93
};

/**
 * Status of a remote data request.
 *
 * @note Current layout of this struct is forced by some old style
 *       QBRemoteDataRequests implementations. It should be refactored
 *       in the future.
 **/
typedef struct QBRemoteDataRequestStatus_ {
    /// state of a remote data request or transaction
    QBRemoteDataRequestState state;
    /// remote data request error code
    int errorCode;
    /// remote request status message
    SvString message;
} QBRemoteDataRequestStatus;

/**
 * Remote data request.
 *
 * @class QBRemoteDataRequest QBRemoteDataRequest.h <QBRemoteProvider/QBRemoteDataRequest.h>
 * @extends QBObservable
 * @implements SvHTTPClientListener
 *
 * This class performs a single HTTP request to remote server.
 * This is base class with only minimal functionality implemented, extend it
 * for more sophisticated requests.
 *
 * Request result is available via QBRemoteDataRequestGetAnswer, default
 * implementation returns response text as SvString. Derived class can
 * implement QBRemoteDataRequestVTable::parseAnswer
 * and QBRemoteDataRequestVTable::getAnswer virtual functions to parse response
 * text to more specialized data types.
 *
 * By default SvHTTPRequest is initialized with following options:
 * | Parameter         | Value
 * |-------------------|--------
 * | max response size | 1MiB
 * | IO policy         | false
 * | user agent        | "CubiTV-Client"
 * | timeout           | 20s
 * SvHTTPRequest options can be customized by implementing
 * QBRemoteDataRequestVTable::setup.
 *
 * Request progress can be traced on chosen file descriptor using environment
 * variable provided in QBRemoteDataRequestInit. By default DEFAULT_TRACE_ENV
 * is used. Tracing works only when SV_LOG_LEVEL > 0.
 *
 * @warning There are lots of old style requests (every request based on
 *          SvXMLRPCRequest, TraxisWebClient requests, TiVoClient requests,
 *          and some others) deriving from this class but using it only as
 *          a common interface for remote requests. Those classes ignores all
 *          QBRemoteRequest built-in features implementing them on its own,
 *          often in very similar way. It should be fixed in future
 *          refacotrizations.
 **/
typedef struct QBRemoteDataRequest_ *QBRemoteDataRequest;

/**
 * QBRemoteDataRequest class internals.
 *
 * @warning Do not access this fields directly.
 **/
struct QBRemoteDataRequest_ {
    struct QBObservable_ super_;      ///< super class

    QBOutputStream traceStream;       ///< trace stream, used only when
                                      ///< SV_LOG_LEVEL > 0
    SvString traceEnv;                ///< name of the environment variable
                                      ///< used to enable tracing
    unsigned int ID;                  ///< request id

    SvHTTPRequestMethod method;       ///< HTTP request method
    SvHTTPRequest req;                ///< HTTP request handle
    QBRemoteDataRequestStatus status; ///< request status
    SvTime startTime;                 ///< request start time
    SvWeakReference manager;          ///< weak reference to
                                      ///< QBRemoteSessionManager
    SvString requestURL;              ///< request URL

    SvFiber fiber;                    ///< fiber used for request answer parsing

    size_t parserChunkThreshold;      ///< data chunk size for stream parser
                                      ///< (@c 0 for unlimited)
    bool lastChunkReceived;           ///< last data chunk received flag

    struct {
        size_t length;                ///< internal buffer length
        size_t used;                  ///< used data count
        size_t processedDataLength;   ///< processed data length
        uint8_t *data;                ///< received data
    } buffer;                         ///< internal buffer
};

/**
 * QBRemoteDataRequest virtual methods table.
 */
typedef const struct QBRemoteDataRequestVTable_ {
    /**
     * SvObject virtual methods table
     */
    struct SvObjectVTable_ super_;

    /**
     * Get request status.
     *
     * @param[in] self      remote data request handle
     * @return              request status
     **/
    QBRemoteDataRequestStatus (*getStatus)(QBRemoteDataRequest self);

    /**
     * Start the request.
     *
     * @param[in]  self      remote data request handle
     * @param[in]  scheduler handle to a scheduler to be used by the request
     **/
    void (*start)(QBRemoteDataRequest self,
                  SvScheduler scheduler);

    /**
     * Cancel the request.
     *
     * @param[in] self      remote data request handle
     **/
    void (*cancel)(QBRemoteDataRequest self);

    /**
     * Setup the request.
     *
     * @private
     *
     * Do not call this function directly. It's called internally just before
     * request is send. Default implementation does nothing. Derived class
     * can implement this function to setup SvHTTPRequest on its own.
     *
     * If error info is set, request will fail.
     *
     * @param[in]  self      remote data request handle
     * @param[in]  req       http request
     * @param[out] errorInfo error info
     **/
    void (*setup)(QBRemoteDataRequest self,
                  SvHTTPRequest req,
                  SvErrorInfo *errorOut);

    /**
     * Handle incoming header.
     *
     * @private
     *
     * Do not call this function directly. It's called internally when new
     * header arrives. Default implementation does nothing. Derived class
     * can implement this function to to handle incoming headers on its own.
     *
     * If error info is set, request will fail.
     *
     * @param[in]  self        remote data request handle
     * @param[in]  request     http request
     * @param[in]  headerName  header name
     * @param[in]  headerValue header value
     * @param[out] errorInfo   error info
     **/
    void (*headerReceived)(QBRemoteDataRequest self,
                           SvHTTPRequest request,
                           const char *headerName,
                           const char *headerValue,
                           SvErrorInfo *errorOut);

    /**
     * Parse the answer.
     *
     * @private
     *
     * Do not call this function directly. It's called internally when request
     * is finished. Default implementation does nothing, because response text
     * is not parsed.
     *
     * Use QBRemoteDataRequestGetAnswer to get parsed answer.
     *
     * @param[in]  self      remote data request handle
     * @param[in]  offset    data offset
     * @param[in]  data      data to parse
     * @param[in]  length    data length
     * @param[in]  lastChunk @c true if this is the last data chunk to be parsed,
     *                       @c false otherwise
     * @param[out] errorOut  error info
     **/
    void (*parseAnswer)(QBRemoteDataRequest self,
                        off_t offset,
                        const uint8_t *data,
                        size_t length,
                        bool lastChunk,
                        SvErrorInfo *errorOut);

    /**
     * Take parsed answer.
     *
     * Use this function to retreive answer from parser. Default
     * implementation does not parse incoming data, so this function simply
     * return SvString containing response text.
     *
     * @warning This function is intended to return retained object, which
     *          should be released when no longer needed.
     *
     * @param[in]  self     remote data request handle
     * @param[out] errorOut error info
     * @return              handle to the remote service answer,
     *                      @c NULL in case of error
     **/
    SvObject (*takeAnswer)(QBRemoteDataRequest self,
                           SvErrorInfo *errorOut);
} *QBRemoteDataRequestVTable;

/**
 * @relates QBRemoteDataRequest
 *
 * Get runtime type identification object representing
 * remote data request type.
 *
 * @return remote data request type identification object
 **/
extern SvType
QBRemoteDataRequest_getType(void);

/**
 * Initialize the request.
 *
 * @public @memberof QBRemoteDataRequest
 *
 * Use this functiont to initialize QBRemoteDataRequest inside derived
 * class constructors.
 *
 * @param[in]  self                 QBRemoteDataRequest handle
 * @param[in]  manager              session manager
 * @param[in]  method               HTTP request method
 * @param[in]  URL                  request URL
 * @param[in]  traceEnv             environment variable name used for request
 *                                  tracing, @c NULL for DEFAULT_TRACE_ENV
 * @param[in]  parserChunkThreshold data chunk size for steram parser, data
 *                                  is parsed when the threshold is exceeded
 *                                  (@c 0 for unlimited)
 * @param[out] errorOut             error info
 **/
void
QBRemoteDataRequestInit(QBRemoteDataRequest self,
                        QBRemoteSessionManager manager,
                        SvHTTPRequestMethod method,
                        SvString URL,
                        SvString traceEnv,
                        size_t parserChunkThreshold,
                        SvErrorInfo *errorOut);

/**
 * Create QBRemoteDataRequest instance.
 *
 * @public @memberof QBRemoteDataRequest
 *
 * QBRemoteDataRequest is intended to be base class for more sophisticated
 * requests and default implementation of it's virtual functions are as simply
 * as possible, so treat this function only as a demo or an example.
 *
 * @param[in]  manager  session manager
 * @param[in]  URL      request URL
 * @param[out] errorOut error info
 * @return              newly created request,
 *                      @c NULL in case of error
 **/
QBRemoteDataRequest
QBRemoteDataRequestCreate(QBRemoteSessionManager manager,
                          SvString URL,
                          SvErrorInfo *errorOut);

/**
 * Get request status.
 *
 * @public @memberof QBRemoteDataRequest
 *
 * @param[in] self      remote data request handle
 * @return              request status
 **/
static inline QBRemoteDataRequestStatus
QBRemoteDataRequestGetStatus(QBRemoteDataRequest self)
{
    return SvInvokeVirtual(QBRemoteDataRequest, self, getStatus);
}

/**
 * Start request.
 *
 * @public @memberof QBRemoteDataRequest
 *
 * @param[in] self      remote data request handle
 * @param[in] scheduler handle to a scheduler to be used by the request
 **/
static inline void
QBRemoteDataRequestStart(QBRemoteDataRequest self,
                         SvScheduler scheduler)
{
    SvInvokeVirtual(QBRemoteDataRequest, self, start, scheduler);
}

/**
 * Cancel request.
 *
 * @public @memberof QBRemoteDataRequest
 *
 * @param[in] self      remote data request handle
 **/
static inline void
QBRemoteDataRequestCancel(QBRemoteDataRequest self)
{
    SvInvokeVirtual(QBRemoteDataRequest, self, cancel);
}

/**
 * Get parsed answer.
 *
 * @public @memberof QBRemoteDataRequest
 *
 * Default implementation does not parse incoming data, it just returns
 * response text as SvString.
 *
 * @warning This function is intended to return retained object, which
 *          should be released when no longer needed.
 *
 * @param[in]  self     remote data request handle
 * @param[out] errorOut error info
 * @return              handle to the remote service answer,
 *                      @c NULL in case of error
 **/
static inline SvObject
QBRemoteDataRequestTakeAnswer(QBRemoteDataRequest self,
                             SvErrorInfo *errorOut)
{
    return SvInvokeVirtual(QBRemoteDataRequest, self, takeAnswer, errorOut);
}

/**
 * Get full request URL string.
 *
 * @public @memberof QBRemoteDataRequest
 *
 * @param[in] self      QBMWRequest handle
 * @return              request URL string, @c NULL in case of error
 **/
SvString
QBRemoteDataRequestGetURL(QBRemoteDataRequest self);
/**
 * @}
 **/

#endif
