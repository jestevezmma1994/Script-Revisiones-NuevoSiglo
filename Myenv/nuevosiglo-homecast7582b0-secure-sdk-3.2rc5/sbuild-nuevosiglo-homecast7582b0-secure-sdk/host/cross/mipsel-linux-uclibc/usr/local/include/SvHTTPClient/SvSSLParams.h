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

#ifndef SV_SSL_PARAMS_H_
#define SV_SSL_PARAMS_H_

/**
 * @file SvSSLParams.h
 * @brief HTTP client SSL parameters class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvSSLParams HTTP client SSL parameters class
 * @ingroup SvHTTPClient
 * @{
 *
 * A container class grouping connection parameters for HTTPS requests.
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>


/**
 * SSL version
 **/
typedef enum {
   /// CURL default handshake, uses "the best" proto available
   SvSSLVersion_default = 0,
   /// SSL version 3
   SvSSLVersion_SSLv3,
   /// TLS version 1, backwards compatible with SSL version 3
   SvSSLVersion_TLSv1,
} SvSSLVersion;

/**
 * SSL key/certificate file format.
 **/
typedef enum {
   /// PEM format
   SvSSLFileFormat_PEM,
   /// ASN.1 DER format
   SvSSLFileFormat_DER,
   /// engine-specific format
   SvSSLFileFormat_ENG
} SvSSLFileFormat;


/**
 * HTTP client SSL parameters class.
 * @class SvSSLParams
 * @extends SvObject
 **/
typedef struct SvSSLParams_ *SvSSLParams;


/**
 * Get runtime type identification object representing
 * type of HTTP client SSL parameters class.
 *
 * @return HTTP client SSL parameters class
 **/
extern SvType
SvSSLParams_getType(void);

/**
 * Create HTTP client SSL parameters object with default values.
 *
 * @memberof SvSSLParams
 *
 * @param[out] errorOut    error info
 * @return                 created SSL params or @c NULL in case of error
 **/
extern SvSSLParams
SvSSLParamsCreateDefault(SvErrorInfo *errorOut);

/**
 * Initialize HTTP client SSL parameters object.
 *
 * This method initializes SSL paramers to default secure values:
 *  - client private key and certicate are loaded from @c /etc/certs/private,
 *  - peer (server) certificate is verified against default CA certificate
 *    file @c /etc/certs/public/cacert.pem,
 *  - peer (server) name verification is enabled.
 *
 * @memberof SvSSLParams
 *
 * @param[in] self         HTTP client SSL parameters handle
 * @param[in] version      SSL version
 * @param[out] errorOut    error info
 * @return                 @a self or @c NULL in case of error
 **/
extern SvSSLParams
SvSSLParamsInit(SvSSLParams self,
                SvSSLVersion version,
                SvErrorInfo *errorOut);

/**
 * Get requested SSL version.
 *
 * @memberof SvSSLParams
 *
 * @param[in] self         HTTP client SSL parameters handle
 * @param[out] errorOut    error info
 * @return                 SSL version
 **/
extern SvSSLVersion
SvSSLParamsGetSSLVersion(SvSSLParams self,
                         SvErrorInfo *errorOut);

/**
 * Set HTTP client SSL private key and certificate.
 *
 * @memberof SvSSLParams
 *
 * @param[in] self         HTTP client SSL parameters handle
 * @param[in] keyFilePath  path to SSL key file
 * @param[in] keyFormat    SSL key format
 * @param[in] certFilePath path to SSL certificate file
 * @param[in] certFormat   SSL certificate format
 * @param[out] errorOut    error info
 **/
extern void
SvSSLParamsSetClientAuth(SvSSLParams self,
                         const char *keyFilePath, SvSSLFileFormat keyFormat,
                         const char *certFilePath, SvSSLFileFormat certFormat,
                         SvErrorInfo *errorOut);

/**
 * Get HTTP client SSL private key and certificate.
 *
 * @memberof SvSSLParams
 *
 * @param[in] self         HTTP client SSL parameters handle
 * @param[out] keyFilePath path to SSL key file
 * @param[out] keyFormat   SSL key format
 * @param[out] certFilePath path to SSL certificate file
 * @param[out] certFormat  SSL certificate format
 * @param[out] errorOut    error info
 **/
extern void
SvSSLParamsGetClientAuth(SvSSLParams self,
                         const char **const keyFilePath, SvSSLFileFormat *keyFormat,
                         const char **const certFilePath, SvSSLFileFormat *certFormat,
                         SvErrorInfo *errorOut);

/**
 * Set HTTP server verification options.
 *
 * @memberof SvSSLParams
 *
 * @param[in] self         HTTP client SSL parameters handle
 * @param[in] verifyHost   @c true to enable host name verification
 *                         (requires @a verifyPeer to be also enabled)
 * @param[in] verifyPeer   @c true to enable peer verification
 * @param[in] caCertPath   path to SSL CA certificates stack
 * @param[out] errorOut    error info
 **/
extern void
SvSSLParamsSetVerificationOptions(SvSSLParams self,
                                  bool verifyHost,
                                  bool verifyPeer, const char *caCertPath,
                                  SvErrorInfo *errorOut);

/**
 * Get HTTP server verification options.
 *
 * @memberof SvSSLParams
 *
 * @param[in] self         HTTP client SSL parameters handle
 * @param[out] verifyHost  @c true if host name verification is enabled
 * @param[out] verifyPeer  @c true if peer verification is enabled
 * @param[out] caCertPath  path to SSL CA certificates stack,
 *                         valid only if @a verifyPeer is @c true
 * @param[out] errorOut    error info
 **/
extern void
SvSSLParamsGetVerificationOptions(SvSSLParams self,
                                  bool *verifyHost,
                                  bool *verifyPeer, const char **const caCertPath,
                                  SvErrorInfo *errorOut);

/**
 * Set path to Certificate Revocation List file.
 *
 * @note CRL will only be used when @a verifyPeer option has been set
 * using SvSSLParamsSetVerificationOptions().
 *
 * @since 1.0.37
 * @memberof SvSSLParams
 *
 * @param[in] self         HTTP client SSL parameters handle
 * @param[in] crlFilePath  path to CRL file in PEM format
 * @param[out] errorOut    error info
 **/
extern void
SvSSLParamsSetCRL(SvSSLParams self,
                  const char *crlFilePath,
                  SvErrorInfo *errorOut);

/**
 * Get path to Certificate Revocation List file.
 *
 * @since 1.0.37
 * @memberof SvSSLParams
 *
 * @param[in] self         HTTP client SSL parameters handle
 * @param[out] crlFilePath path to CRL file in PEM format, @c NULL if not set
 * @param[out] errorOut    error info
 **/
extern void
SvSSLParamsGetCRL(SvSSLParams self,
                  const char **const crlFilePath,
                  SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
