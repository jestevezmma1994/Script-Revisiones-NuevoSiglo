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

#ifndef QB_VIEWRIGHT_WEB_H_
#define QB_VIEWRIGHT_WEB_H_

/**
 * @file QBViewRightWeb.h ViewRightWeb API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvData.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBViewRightWeb QBViewRightWeb plugin
 * @ingroup DRM
 * @{
 **/

/**
 * @enum QBViewRightWebState_e
 * @brief It defines state of ViewRightWeb plugin.
 */
enum QBViewRightWebState_e {
    QBViewRightWebState_Created,                //!< Created
    QBViewRightWebState_InitializationProcess,  //!< Initialization process is pending
    QBViewRightWebState_ConnectedAndProvisioned,//!< Established connection with vcas server and passed successfully authentication
    QBViewRightWebState_Stopped,                //!< Plugin has been stopped - it is termination state.
    QBViewRightWebState_NotConnected,           //!< Connection to vcas server cannot be established.
    QBViewRightWebState_BadCertificate,         //!< Connection to vcas server cannot be established because of the bad certificate.
    QBViewRightWebState_NotProvisioned,         //!< STB device cannot be provisioned
    QBViewRightWebState_Error,                  //!< Other critical error has occurred.
    QBViewRightWebState_InitializationError,    //!< Critical issue during the initialization.
    QBViewRightWebState_eMax                    //!< QBViewRightWebState_eMax
};

/**
 * @typedef QBViewRightWebState
 */
typedef enum QBViewRightWebState_e QBViewRightWebState;

/**
 * @typedef QBViewRightWebListener
 * @brief Typedef of struct QBViewRightWebListener_s. Interface of QBViewRightWebListener. Object which implements that interface is notified about every change state of QBViewRightWeb
 */
typedef struct QBViewRightWebListener_s {
    /**
     * @brief Method notifies listener about changes in state of QBViewRightWeb, and gives errorCode if any error occurs.
     * @param self_ handle of listener
     * @param newState new state of QBViewRightWeb
     * @param errorCode its value is equal to error code returned by third party library "Verimatrix ViewRightWeb"
     */
    void (*stateChanged)(SvObject self_, QBViewRightWebState newState, int errorCode);
}* QBViewRightWebListener;

/**
 * @brief Gets SvInterface instance of QBViewRightWebListener listener.
 * @return instance of SvInterface
 */
SvInterface QBViewRightWebListener_getInterface(void);

/**
 * @typedef QBViewRightWeb
 */
typedef struct QBViewRightWeb_s * QBViewRightWeb;

/**
 * @brief It gets global instance of QBViewRightWeb. It can be called after QBViewRightWebInit(), in other case it will return NULL
 * @return Instance of QBViewRightWeb if QBViewRightWeb has been initialized or NULL if not
 */
QBViewRightWeb QBViewRightWebGetInstance(void);

/**
 * @brief Method creates object which implements QBHlsKeyProvider interface.
 * @return Instance of object which implements QBHlsKeyProvider interface.
 */
SvObject QBViewRightWebCreateHlsKeyProviderPlugin(void);

/**
 * @brief Method creates global instance of ViewRightWeb plugin and launches it.
 * @param vcasServerURL URL to VCASServer
 * @param storagePath path to directory where ViewRightWeb library will store its data
 */
void QBViewRightWebInit(SvURL vcasServerURL, SvString storagePath);

/**
 * @brief It sets listener, which has to implement QBViewRightWebListener interface
 * @param self handle of QBViewRightWeb
 * @param listener instance of object which implements QBViewRightWebListener interface
 */
void QBViewRightWebSetListener(QBViewRightWeb self, SvObject listener);

/**
 * @brief Method stops ViewRightWeb plugin, and free its resources.
 */
void QBViewRightWebDeinit(void);

/**
 * @brief It gives information if device has been successfully provisioned.
 * @param self handler of QBViewRightWeb instance
 * @return It returns true if device was provisioned
 */
bool QBViewRightWebGetIsDeviceProvisioned(QBViewRightWeb self);

/**
 * @brief It gets information of snapshot state of QBViewRight plugin.
 * @param self handle of QBViewRightWeb instance
 * @param[out] errorCode if current state is equal to error then errorCode will be filled with errorCode returned by ViewRightLibrary
 * @return QBViewRightWebState enum which identifies snapshot state of QBViewRightWeb
 */
QBViewRightWebState QBViewRightWebGetState(QBViewRightWeb self, int *errorCode);

/**
 * @brief It gets unique id of STB. During initialization phase it could not be ready (Create and InitializationProcess states).
 * @param self handle of QBViewRightWeb instance
 * @return unique id of STB or null in case of error or if it is not ready.
 */
SvData QBViewRightWebGetUniqueIdentifier(QBViewRightWeb self);

/**
 * @}
 **/
#ifdef __cplusplus
}
#endif

#endif // QB_VIEWRIGHT_WEB_H_
