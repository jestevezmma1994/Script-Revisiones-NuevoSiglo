/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PLAYREADY_INTERFACE_H
#define QB_PLAYREADY_INTERFACE_H

/**
 * @file QBPlayReadyInterface.h
 * @brief Common interface for different MS playready implementations
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBPlayReadyInterface QBPlayReadyInterface.h
 * @ingroup QBPlayReady
 * @{
 */

#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <QBAppKit/QBFuture.h>
#include <SvPlayerKit/SvDrmSystemInfo.h>
#include <SvPlayerKit/SvBuf.h>
#include <SvHTTPClient/SvSSLParams.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * QBPlayReady interface
 **/
typedef struct QBPlayReadyInterface_ {
    /**
    * Opens new PlayReady session.
    *
    * @param[in] self QBPlayReady handle
    * @param[out] errorOut error info
    * @return new session id
    */
    int (*openSession)(SvObject self, SvErrorInfo* errorOut);

    /**
    * Close PlayReady session.
    *
    * @param[in] self QBPlayReady handle
    * @param[in] sessionId id of the session to close
    * @param[out] errorOut error info
    */
    void (*closeSession)(SvObject self, int sessionId, SvErrorInfo* errorOut);

    /**
    * Basing on given drmSystemInfo it can perform individualization or/and license acquisition
    * if any of this steps is needed.
    *
    * @param[in] self QBPlayReady handle
    * @param[in] sessionId id of the current session
    * @param[in] drmSystemInfo PlayReady DRM info sent with the content
    * @param[out] errorOut error info
    * @return returns QBFuture with QBSharedContentProtection
    */
    QBFuture (*prepareForPlayback)(SvObject self, int sessionId, SvDrmSystemInfo drmSystemInfo, SvErrorInfo* errorOut);

    /**
    * Given encryption info will be used to decrypt subsequent samples.
    *
    * @param[in] self QBPlayReady handle
    * @param[in] sessionId id of the current session
    * @param[in] sampleEncryptionInfo encrytpion info that will be used to decrypt samples
    * @param[out] errorOut error info
    */
    void (*setSampleEncryptionInfo)(SvObject self, int sessionId, SvSampleEncryptionInfo sampleEncryptionInfo, SvErrorInfo* errorOut);

    /**
    * Decrypts sample inplace using given sampleEncryptionInfo.
    * This method is blocking.
    *
    * @param[in] self QBPlayReady handle
    * @param[in] sessionId id of the current session
    * @param[in] sample sample data
    * @param[out] errorOut error info
    */
    void (*decryptSample)(SvObject self, int sessionId, SvBuf sample, SvErrorInfo* errorOut);

    /**
     * Set additional GET parameter(s) that will be appended to PlayReady licence request url.
     *
     * @param[in] self QBPlayReady handle
     * @param[in] licenceParams GET parameters
     * @param[out] errorOut error info
     */
    void (*setLicenceRequestParams)(SvObject self, SvString licenceRequestParams, SvErrorInfo* errorOut);

    /**
     * Set custom data for PlayReady licence acquisition.
     *
     * @param[in] self QBPlayReady handle
     * @param[in] customData licence acquisition custom data
     * @param[out] errorOut error info
     */
    void (*setLicenceRequestCustomData)(SvObject self, SvString customData, SvErrorInfo* errorOut);

    /**
     * Set SSL params for PlayReady licence acquisition.
     *
     * @param[in] self QBPlayReady handle
     * @param[in] sslParams licence acquisition SSL params
     * @param[out] errorOut error info
     */
    void (*setLicenceRequestSSLParams)(SvObject self, SvSSLParams sslParams, SvErrorInfo* errorOut);

} *QBPlayReadyInterface;

/**
 * QBPlayReadyInterface_getInterface creates QBPlayReadyInterface object
 * @return interface object
 */
SvInterface QBPlayReadyInterface_getInterface(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif // QB_PLAYREADY_INTERFACE_H
