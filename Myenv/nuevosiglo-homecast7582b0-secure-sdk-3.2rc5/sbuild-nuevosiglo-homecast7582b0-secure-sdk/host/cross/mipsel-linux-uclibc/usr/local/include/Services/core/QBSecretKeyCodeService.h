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

#ifndef QB_SECRET_KEY_CODE_SERVICE_H_
#define QB_SECRET_KEY_CODE_SERVICE_H_

/**
 * @file QBSecretKeyCodeService.h Secret Key Code Service class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBApplicationController.h>
#include <QBInput/QBInputEvent.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvData.h>

/**
* @defgroup QBSecretKeyCodeService Secret Key Code Service
* @ingroup CubiTV_services
* @{
*
* This service checks if pressed keys match to any secret key codes.
**/

/**
* This service checks if pressed keys match to any secret key codes.
*
* @class QBSecretKeyCodeService
*
* This service allows to inform its listeners if user pressed previously
* defined secret key code. It might be used to unlock secret menus or services.
**/
typedef struct QBSecretKeyCodeService_ *QBSecretKeyCodeService;

/**
* This data type is inherits from SvData type just to reimplement its 'hash' and 'equals' virtual functions.
*
* @class QBSecretKeyCodeData
**/
typedef struct QBSecretKeyCodeData_* QBSecretKeyCodeData;

struct QBSecretKeyCodeServiceListener_ {
    /**
     * This function is invoked when key code is matched
     *
     * @param[in] self object implementing QBSecretKeyCodeService interface
     * @param[in] code secret code
     **/
    void (*keyCodeMatch)(SvObject self_, QBSecretKeyCodeData code);
};

/**
* This data type defines an interface for QBSecretKeyCodeService listeners.
*
* @class QBSecretKeyCodeServiceListener
**/
typedef struct QBSecretKeyCodeServiceListener_* QBSecretKeyCodeServiceListener;

/**
 * Create interface for the secret key code listeners.
 *
 * @return created interface for the listeners, @c NULL in case of error
 **/
SvInterface QBSecretKeyCodeServiceListener_getInterface(void);

/**
 * Create secret key code data.
 * @param[in] bytes     data buffer which will be copied to created data object
 * @param[in] length    size of @a bytes buffer
 * @param[out] errorOut error info
 * @return              created QBSecretKeyCodeData object, @c NULL in case of error
 **/
QBSecretKeyCodeData QBSecretKeyCodeDataCreate(const char *bytes, size_t length, SvErrorInfo *errorOut);

/**
 * Add a listener that will be notified if secret code is entered.
 *
 * @param[in] self      QBSecretKeyCodeService handle
 * @param[in] listener  object implementing QBSecretKeyCodeService interface
 * @param[in] code      secret code
 * @return true if no error and false if error occurs
 **/
bool QBSecretKeyCodeServiceAddListener(QBSecretKeyCodeService self, SvObject listener, QBSecretKeyCodeData code);

/**
 * Remove a listener previously added by QBSecretKeyCodeServiceAddListener from
 * active listeners list.
 *
 * @param[in] self             QBSecretKeyCodeService handle
 * @param[in] listenerToRemove listener which will be removed from listeners list
 * @param[in] code             secret code
 * @return true if no error and false if error occurs
 **/
bool QBSecretKeyCodeServiceRemoveListener(QBSecretKeyCodeService self, SvObject listenerToRemove, QBSecretKeyCodeData code);

/**
 * Create secret key code service.
 * @param[in] appController QBApplicationController object
 * @return                  returns created QBSecretKeyCodeService, @c NULL in case of error
 **/
QBSecretKeyCodeService QBSecretKeyCodeServiceCreate(QBApplicationController appController);

/**
 * Start secret key code service.
 *
 * @param[in] self        QBSecretKeyCodeService object
 **/
void QBSecretKeyCodeServiceStart(QBSecretKeyCodeService self);

/**
 * Stop secret key code service.
 *
 * @param[in] self        QBSecretKeyCodeService object
 **/
void QBSecretKeyCodeServiceStop(QBSecretKeyCodeService self);

/**
* @}
**/

#endif /* QB_SECRET_KEY_CODE_SERVICE_H_ */
