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

#ifndef QB_JSON_REQUEST_H_
#define QB_JSON_REQUEST_H_

/**
 * @file QBJSONRequest.h
 * @brief QBJSONRequest class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvHTTPClient/SvHTTPRequest.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <QBDataProvider/QBRemoteDataRequest.h>
#include <QBDataProvider/QBDataProviderTypes.h>
#include <SvJSON/SvJSONDataBuilder.h>
#include <SvJSON/SvJSONStreamParser.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <QBOutputStream.h>
#include <fibers/c/fibers.h>

/**
 * @defgroup QBJSONRequest JSON request
 * @ingroup QBDataProvider
 * @{
 **/

/**
 * JSON request class.
 *
 * @class QBJSONRequest QBJSONRequest.h <QBDataProvider/QBJSONRequest.h>
 * @extends QBRemoteDataRequest
 *
 * This class extends QBRemoteDataRequest adding support for parsing request
 * response from JSON format. All data requests expecting JSON response should
 * derive from this class.
 **/
typedef struct QBJSONRequest_ *QBJSONRequest;

/**
 * QBJSONRequest class internals.
 *
 * @warning Do not access this fields directly.
 **/
struct QBJSONRequest_ {
    struct QBRemoteDataRequest_ super_; ///< super class

    SvJSONDataBuilder builder;          ///< JSON data builder
    SvJSONStreamParser parser;          ///< JSON parser
};

/**
 * Get runtime type identification object representing QBJSONRequest class.
 *
 * @relates QBJSONRequest
 *
 * @return QBJSONRequest runtime type identification object
 **/
SvType
QBJSONRequest_getType(void);

/**
 * Initialize QBJSONRequest.
 *
 * @public @memberof QBJSONRequest
 *
 * Use this function to initialize QBJSONRequest inside derived
 * class constructors.
 *
 * @param[in]  self                 QBJSONRequest handle
 * @param[in]  manager              session manager
 * @param[in]  method               HTTP request method
 * @param[in]  URL                  request URL
 * @param[in]  traceEnv             environment variable name used for request
 *                                  tracing
 * @param[in]  parserChunkThreshold data chunk size for steram parser, data
 *                                  is parsed when the threshold is exceeded
 *                                  (@c 0 for unlimited)
 * @param[out] errorOut             error info
 **/
void
QBJSONRequestInit(QBJSONRequest self,
                  QBRemoteSessionManager manager,
                  SvHTTPRequestMethod method,
                  SvString URL,
                  SvString traceEnv,
                  size_t parserChunkThreshold,
                  SvErrorInfo *errorOut);

/**
 * Create an instance of QBJSONRequest.
 *
 * @public @memberof QBJSONRequest
 *
 * QBJSONRequest is only a tiny wrapper for QBDataRequest and is intended
 * to be base class for more sophisticated requests, so treat this function
 * only as a demo or an example.
 *
 * @param[in]  manager              session manager
 * @param[in]  URL                  request URL
 * @param[out] errorOut             error info
 * @return                          QBJSONRequest instance,
 *                                  @c NULL in case of error
 **/
QBJSONRequest
QBJSONRequestCreate(QBRemoteSessionManager manager,
                    SvString URL,
                    SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QB_JSON_REQUEST_H_ */
