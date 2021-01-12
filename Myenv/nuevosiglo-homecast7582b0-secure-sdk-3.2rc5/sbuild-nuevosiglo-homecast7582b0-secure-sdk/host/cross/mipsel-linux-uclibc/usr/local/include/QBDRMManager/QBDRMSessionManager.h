/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DRM_SESSION_MANAGER_H_
#define QB_DRM_SESSION_MANAGER_H_

/**
 * @file QBDRMSessionManager.h DRM Session Manager interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBDRMInitializationData.h"
#include "QBDRMSystemInfo.h"

#include <dataformat/QBSharedContentProtection.h>
#include <SvPlayerKit/SvDrmSystemInfo.h>
#include <SvPlayerKit/SvBuf.h>
#include <QBAppKit/QBFuture.h>
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>

/**
 * DRM Session Manager interface class.
 * @class QBDRMSessionManager
 **/
typedef struct QBDRMSessionManager_ {
    /**
     * Check if a DRM Session Manager implementation can handle given initialization data
     *
     * @param[in] self_                 handle to object implementing the interface
     * @param[in] initializationData    DRM initialization data
     * @param[out] errorOut             error info
     * @return @c true if the initialization data can be handled, @c false otherwise
     **/
    bool (*canHandle)(SvObject self_,
                      QBDRMInitializationData initializationData,
                      SvErrorInfo *errorOut);
    /**
     * Start a DRM session (asynchronous function)
     *
     * @param[in] self_                 handle to object implementing the interface
     * @param[in] sessionId             ID of a DRM session to be started
     * @param[in] initializationData    DRM initialization data
     * @param[out] errorOut             error info
     * @return @c future class instance handle, which will be ready when session is established,
     *         @c NULL in case of error otherwise
     **/
    QBFuture (*startSession)(SvObject self_,
                             int sessionId,
                             QBDRMInitializationData initializationData,
                             SvErrorInfo *errorOut);

    /**
     * Stop a DRM session.
     *
     * @param[in] self_                 handle to object implementing the interface
     * @param[in] sessionId             ID of a DRM session to be stopeed
     * @param[out] errorOut             error info
     * @return @c 0 on success, @c -1 otherwise
     **/
    int (*stopSession)(SvObject self_,
                       int sessionId,
                       SvErrorInfo *errorOut);

    /**
     * Prepare a DRM key (asynchronous function).
     *
     * @param[in] self_                 handle to object implementing the interface
     * @param[in] sessionId             ID of a DRM session
     * @param[in] drmSystemInfo         DRM system info instance handle
     * @param[out] errorOut             error info
     * @return future class instance handle in which the prepared key will be stored
     **/
    QBFuture (*prepareKey)(SvObject self_,
                           int sessionId,
                           QBDRMSystemInfo drmSystemInfo,
                           SvErrorInfo *errorOut);

    /**
     * Get shared content protection.
     *
     * @param[in] self_                 handle to object implementing the interface
     * @param[in] sessionId             ID of a DRM session
     * @param[out] errorOut             error info
     * @return shared content protection handle
     */
    QBSharedContentProtection (*getContentProtection)(SvObject self_,
                                                      int sessionId,
                                                      SvErrorInfo *errorOut);

    /**
     * Set DRM sample encryption information of a given session.
     *
     * @param[in] self_                 handle to object implementing the interface
     * @param[in] sessionId             ID of a DRM session
     * @param[in] sampleEncryptionInfo  DRM sample encryption information to be set
     * @param[in] errorOut              error info
     */
    void (*setSampleEncryptionInfo)(SvObject self_,
                                    int sessionId,
                                    SvSampleEncryptionInfo sampleEncryptionInfo,
                                    SvErrorInfo *errorOut);

    /**
     * Decrypt given sample.
     *
     * @param[in] self_                 handle to object implementing the interface
     * @param[in] sessionId             ID of a DRM session
     * @param[in] sample                sample to be decrypted
     * @param[in] errorOut              error info
     * @return future class instance handle in which the decrypted sample be stored
     */
    QBFuture (*decryptSample)(SvObject self_,
                              int sessionId,
                              SvBuf sample,
                              SvErrorInfo *errorOut);
} *QBDRMSessionManager;

/**
 * DRM system state
 **/
typedef enum {
    QBDRMSystemState_sessionStopped     /**< session is stopped */
} QBDRMSystemState;

/**
 * Latens DRM system error
 **/
typedef enum {
    QBLatensDRMSystemError_unknown   /**< unknown Latens ERM error */
} QBLatensDRMSystemError;

/**
 * DRM system error
 **/
typedef union {
  QBLatensDRMSystemError latensError;       /**< Latens system errors */
} QBDRMSystemError;

/**
 * DRM System listener interface class.
 * @class QBDRMSystemListener
 **/
typedef struct QBDRMSystemListener_ {
    /**
     * Handle change of a state of a DRM system
     *
     * @param[in] self_                 handle to object implementing the interface
     * @param[in] state                 current state of the DRM system
     * @return @c 0 on success, @c -1 otherwise
     **/
    void (*stateChanged)(SvObject self_, int sessionId, QBDRMSystemState state);

    /**
     * Handle error coming from a DRM system
     *
     * @param[in] self_                 handle to object implementing the interface
     * @param[in] error                 DRM system error
     * @return @c 0 on success, @c -1 otherwise
     **/
    void (*errorOccured)(SvObject self_, int sessionId, QBDRMSystemError error);
} *QBDRMSystemListener;

/**
 * Get runtime type identification object representing
 * QBDRMSessionManager interface.
 *
 * @return QBDRMSessionManager interface object
 **/
SvInterface
QBDRMSessionManager_getInterface(void);

/**
 * Get runtime type identification object representing
 * QBDRMSystemListener interface.
 *
 * @return QBDRMSystemListener interface object
 **/
SvInterface
QBDRMSystemListener_getInterface(void);


#endif /* QB_DRM_SESSION_MANAGER_H_ */
