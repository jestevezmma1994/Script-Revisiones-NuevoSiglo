/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_OAUTH_PARAMS_H_
#define SV_OAUTH_PARAMS_H_

/**
 * @file SvOAuthParams.h
 * @brief HTTP client OAuth 1.0 parameters class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvOAuthParams HTTP client OAuth 1.0 parameters class
 * @ingroup SvHTTPClient
 * @{
 *
 * A container class grouping authorization parameters for OAuth 1.0 signed HTTP requests.
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * OAuth method
 **/
typedef enum {
    SvOAuthMethod_none = 0, ///< do not use signature
    SvOAuthMethod_HMAC, ///< use HMAC-SHA1 request signing method
    SvOAuthMethod_RSA, ///< use RSA signature
    SvOAuthMethod_PLAINTEXT, ///< use plain text signature (for testing only)
    SvOAuthMethod_max  ///< iteration delimiter
} SvOAuthMethod;

/**
 * HTTP client OAuth parameters class.
 * @class SvOAuthParams
 * @extends SvObject
 **/
typedef struct SvOAuthParams_ *SvOAuthParams;

/**
 * Get runtime type identification object representing
 * type of HTTP client OAuth 1.0 parameters class.
 *
 * @return HTTP client OAuth 1.0 parameters class
 **/
extern SvType
SvOAuthParams_getType(void);

/**
 * Initialize HTTP client OAuth 1.0 parameters object.
 *
 * @memberof SvOAuthParams
 *
 * @param[in] self              HTTP client OAuth 1.0 parameters handle
 * @param[in] method            OAuth 1.0 method
 * @param[in] consumerKey       consumer key
 * @param[in] consumerSecret    consumer secret
 * @param[in] accessToken       access token
 * @param[in] accessTokenSecret access token secret
 * @param[out] errorOut    error info
 * @return                 @a self or @c NULL in case of error
 **/
extern SvOAuthParams
SvOAuthParamsInit(SvOAuthParams self,
                  SvOAuthMethod method,
                  SvString consumerKey,
                  SvString consumerSecret,
                  SvString accessToken,
                  SvString accessTokenSecret,
                  SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
