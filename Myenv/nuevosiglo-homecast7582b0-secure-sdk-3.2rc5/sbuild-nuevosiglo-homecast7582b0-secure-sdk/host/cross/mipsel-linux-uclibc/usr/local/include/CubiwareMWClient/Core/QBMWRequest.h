/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_WEB_REQUEST_H_
#define QB_WEB_REQUEST_H_

/**
 * @file QBMWRequest.h
 * @brief QBMWRequest class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CubiwareMWClient/Core/QBMWSessionManager.h>

#include <SvHTTPClient/SvHTTPRequest.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

#include <stdbool.h>
#include <stddef.h>

/**
 * @defgroup QBMWRequest QBMWRequest class
 * @ingroup QBMWClient
 * @{
 **/

/**
 * Cubiware middleware request class.
 *
 * @class QBMWRequest QBMWRequest.h <CubiwareMWClient/Core/QBMWRequest.h>
 * @extends QBRemoteDataRequest
 * @implements SvHTTPClientListener
 *
 * This class performs a single request to a Cubiware Middleware server.
 * Request's parameters are encoded in HTTP URL (following the REST
 * architecture), optionally also as additional data send with PUT/POST
 * command. Response is supposed to be in JSON format deserialized to
 * @ref SvHashTable.
 *
 * @note Use environment variable QB_MW_TRACE_FD to enable requests tracing.
 *
 **/
typedef struct QBMWRequest_ *QBMWRequest;


/**
 * Get runtime type identification object representing
 * type of QBMWRequest class.
 *
 * @relates QBMWRequest
 *
 * @return QBMWRequest class
 **/
extern SvType
QBMWRequest_getType(void);

/**
 * Create QBMWRequest instance.
 *
 * @memberof QBMWRequest
 *
 * @note In most cases it's much better to use more specialized constructors.
 *
 * @see QBMWRequestCreateSimple
 * @see QBMWRequestCreateAction
 * @see QBMWRequestCreateRelation
 *
 * @param[in]  manager  QBMWSession manager handle
 * @param[in]  method   HTTP request method
 * @param[in]  URL      request URL string
 * @param[out] errorOut error info
 * @return              created request, @c NULL in case of error
 **/
extern QBMWRequest
QBMWRequestCreate(QBMWSessionManager manager,
                  SvHTTPRequestMethod method,
                  SvString URL,
                  SvErrorInfo *errorOut);

/**
 * Create simple QBMWRequest to get single object.
 *
 * @memberof QBMWRequest
 *
 * @param[in]  manager    QBMWSession manager handle
 * @param[in]  objectType object type
 * @param[in]  objectID   object identifier (pass @c NULL to get all objects of given type)
 * @param[out] errorOut   error info
 * @return                created request, @c NULL in case of error
 **/
extern QBMWRequest
QBMWRequestCreateSimple(QBMWSessionManager manager,
                        SvString objectType,
                        SvString objectID,
                        SvErrorInfo *errorOut);

/**
 * Create QBMWRequest to perform an action on a single object.
 *
 * @memberof QBMWRequest
 *
 * @param[in]  manager    QBMWSession manager handle
 * @param[in]  service    name of the service
 * @param[in]  action     action to perform
 * @param[out] errorOut   error info
 * @return                created request, @c NULL in case of error
 **/
extern QBMWRequest
QBMWRequestCreateAction(QBMWSessionManager manager,
                        SvString service,
                        SvString action,
                        SvErrorInfo *errorOut);

/**
 * Create QBMWRequest for getting subset of objects connected
 * by some relation with base object (origin).
 *
 * @memberof QBMWRequest
 *
 * @param[in]  manager    QBMWSession manager handle
 * @param[in]  originType origin object type
 * @param[in]  originID   origin object ID
 * @param[in]  objectType origin object type
 * @param[in]  objectID   origin object ID (pass @c NULL to get all objects of given type)
 * @param[out] errorOut   error info
 * @return                created request, @c NULL in case of error
 **/
extern QBMWRequest
QBMWRequestCreateRelation(QBMWSessionManager manager,
                          SvString originType,
                          SvString originID,
                          SvString objectType,
                          SvString objectID,
                          SvErrorInfo *errorOut);

/**
 * Set query parameter for a HTTP request.
 *
 * @memberof QBMWRequest
 *
 * @param[in]  self       QBMWRequest handle
 * @param[in]  paramName  HTTP query parameter name
 * @param[in]  paramValue HTTP query parameter value
 * @param[out] errorOut   error info
 **/
extern void
QBMWRequestSetQueryParam(QBMWRequest self,
                         SvString paramName,
                         SvString paramValue,
                         SvErrorInfo *errorOut);
/**
 * Set HTTP header.
 *
 * @memberof QBMWRequest
 *
 * @param[in]  self        QBMWRequest handle
 * @param[in]  headerName  HTTP query parameter name
 * @param[in]  headerValue HTTP query parameter value
 * @param[out] errorOut    error info
 **/
extern void
QBMWRequestSetHTTPHeader(QBMWRequest self,
                         SvString headerName,
                         SvString headerValue,
                         SvErrorInfo *errorOut);

/**
 * Get full request URL string.
 *
 * @memberof QBMWRequest
 *
 * @param[in] self      QBMWRequest handle
 * @return              request URL string, @c NULL in case of error
 **/
extern SvString
QBMWRequestGetURL(QBMWRequest self);

/**
 * Get parsed JSON answer from the middleware server.
 *
 * @memberof QBMWRequest
 *
 * @param[in]  self     QBMWRequest handle
 * @param[out] errorOut error info
 * @return              handle to the web service answer,
 *                      @c NULL in case of error
 **/
extern SvHashTable
QBMWRequestGetAnswer(QBMWRequest self,
                     SvErrorInfo *errorOut);

/**
 * Get parsed JSON error reply from the middleware server.
 *
 * @memberof QBMWRequest
 *
 * @param[in]  self     QBMWRequest handle
 * @param[out] errorOut error info
 * @return              handle to the web service error reply,
 *                      @c NULL in case of error
 **/
extern SvHashTable
QBMWRequestGetErrorReply(QBMWRequest self,
                         SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
