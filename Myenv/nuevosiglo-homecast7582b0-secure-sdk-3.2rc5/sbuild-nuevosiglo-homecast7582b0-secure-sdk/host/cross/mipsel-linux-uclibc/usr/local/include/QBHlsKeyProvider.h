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

#ifndef Hls_KEY_REQUEST_INTERFACE_H
#define Hls_KEY_REQUEST_INTERFACE_H

/**
 * @file QBHlsKeyProvider.h
 * @brief Shared API for HLS key providers
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvData.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBHlsKeyProvider QBHlsKeyProvider: HLS key provider library
 * @ingroup MediaPlayer
 *
 * QBHlsKeyProvider defines an abstract an interface for components which implement key delivery for HLS content.
 * @{
 **/

/**
 * Create an instance of a generic (Pantos) implementation of the @ref QBHlsKeyProvider.
 * @param[in] httpEngine HTTP Client Engine
 * @return instance of generic plugin which implements QBHlsKeyProvider interface
 */
extern SvObject QBHlsKeyProviderCreateHlsPantosPlugin(SvHTTPClientEngine httpEngine);

/**
 * Get runtime type identification object representing QBHlsKeyProvider interface.
 * @return QBHlsKeyProvider interface object
 */
extern SvInterface QBHlsKeyProvider_getInterface(void);

/**
 * Get runtime type identification object representing QBHlsKeyRequest interface.
 * @return QBHlsKeyRequest interface object
 **/
extern SvInterface QBHlsKeyRequest_getInterface(void);

/**
 * Get runtime type identification object representing QBHlsKeyRequestListener interface.
 * @return QBHlsKeyRequestListener interface object
 **/
extern SvInterface QBHlsKeyRequestListener_getInterface(void);

/**
 * @brief It defines what error has occurred when trying to retrieve key from key server
 */
typedef enum {
    QBHlsKeyRequestError_NoError = 0,     //!< No error
    QBHlsKeyRequestError_NotInitialized,  //!< DRM has not been initialized
    QBHlsKeyRequestError_NoSubscription,  //!< Device is not subscribed
    QBHlsKeyRequestError_NoEntitled,      //!< No entitlement to the content
    QBHlsKeyRequestError_NoConnection,    //!< There is not connection to key server
    QBHlsKeyRequestError_BadCertificate,  //!< The certificate is not correct or it has expired
    QBHlsKeyRequestError_KeyNotObtained,  //!< Selected key could not be obtained
    QBHlsKeyRequestError_RequestCanceled, //!< The request has been canceled
    QBHlsKeyRequestError_FatalError,      //!< Other error has occurred.
    QBHlsKeyRequestError_eMax             //!< Enum max
} QBHlsKeyRequestError;

/**
 * QBHlsKeyProvider interface.
 * @brief It is used to create of key request and to perform them.
 */
typedef struct QBHlsKeyProvider_s {
    /**
     * @brief It creates request for a key.
     * @param[in] self_ provider handle
     * @param[in] keyUrl url to key
     * @return It returns instance of the object which implements @ref QBHlsKeyRequest
     */
    SvObject (*createRequest)(SvObject self_, SvURL keyUrl);

    /**
     * @brief It is used for scheduling previously created keys
     * @param[in] self_ provider handle
     * @param[in] keyRequest key request handle
     */
    void (*schedule)(SvObject self_, SvObject keyRequest);
} *QBHlsKeyProvider;

/**
 * QBHlsKeyRequest interface.
 * @brief Interface of key request
 */
typedef struct QBHlsKeyRequest_s {
    /**
     * @brief It sets listener for the request.
     * @param[in] self_ handle of key request
     * @param[in] listener instance of object which implements @ref QBHlsKeyRequestListener
     */
    void (*setListener)(SvObject self_, SvObject listener);

    /**
     * @brief It sets timeout for the request
     * @param[in] self_ handle of key request
     * @param[in] timeout timeout defined in seconds
     */
    void (*setTimeout)(SvObject self_, unsigned int timeout);

    /**
     * @brief It marks request as canceled. It won't be executed. The listener callbacks won't be called.
     * @param[in] self_ handle of key request
     */
    void (*cancel)(SvObject self_);
} *QBHlsKeyRequest;

/**
 * QBHlsKeyRequestListener interface.
 * @brief It has to be implemented by request listeners who want to receive HLS encryption keys.
 */
typedef struct QBHlsKeyRequestListener_s {
    /**
     * @brief It notifies listener that key is ready.
     * @param[in] self_ listener handler
     * @param[in] key key data
     */
    void (*keyReady)(SvObject self_, SvData key);

    /**
     * @brief It notifies listener that request has been canceled.
     * @param self handle of request
     */
    void (*canceled)(SvObject self);

    /**
     * @brief It notifies listener that error has occurred
     * @param[in] self_ handle of request
     * @param[in] error indicates what type of error has occurred
     */
    void (*errorOccured)(SvObject self_, QBHlsKeyRequestError error);
} *QBHlsKeyRequestListener;

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
