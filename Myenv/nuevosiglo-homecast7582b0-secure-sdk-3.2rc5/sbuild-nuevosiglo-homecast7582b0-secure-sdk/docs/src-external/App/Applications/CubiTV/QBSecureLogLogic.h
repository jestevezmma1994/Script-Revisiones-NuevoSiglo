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

#ifndef QB_SECURE_LOG_LOGIC_H_
#define QB_SECURE_LOG_LOGIC_H_

/**
 * @file QBSecureLogLogic.h Secure Logger Logic
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBNetManager.h>
#include <QBNetworkMonitor.h>
#include <QBSmartCardMonitor.h>
#include <Services/core/QBMiddlewareManager.h>
#include <QBApplicationController.h>
#include <Services/standbyAgent/QBStandbyAgentService.h>
#include <Services/QBAppVersion.h>

/**
* @defgroup  QBSecureLogLogic Secure Logger Logic
* @ingroup CubiTV_logic
* @{
*
* This logic is responsible for logic of Secure Logger.
**/
typedef struct QBSecureLogLogic_* QBSecureLogLogic;

/**
 * SecureLog logic's parameters.
 **/
typedef struct QBSecureLogLogicParams_ {
    QBNetManager net_manager;              ///< QBNetManager service object
    QBSmartCardMonitor smartCardMonitor;   ///< QBSmartCardMonitor service object
    QBMiddlewareManager middlewareManager; ///< QBMiddlewareManager service object
    QBAppVersion appVersion;               ///< QBAppVersion service object
    QBStandbyAgent standbyAgent;           ///< QBStandbyAgent service object
    QBApplicationController controller;    ///< QBApplicationController service object
    QBNetworkMonitor networkMonitor;       ///< QBNetworkMonitor service object
} *QBSecureLogLogicParams;

/**
 * Start log manager.
 *
 * @return QBSecureLogLogic object
 **/
extern QBSecureLogLogic QBSecureLogLogicCreate(void) __attribute__((weak));

/**
 * Starts SecureLog's logic.
 *
 * @param[in] self   QBSecureLogLogic service object
 * @param[in] params QBSecureLogLogicParams object with parameters
 **/
extern void QBSecureLogLogicStart(QBSecureLogLogic self, QBSecureLogLogicParams params) __attribute__((weak));

/**
 * Stops SecureLog's logic.
 *
 * @param[in] self QBSecureLogLogic object
 **/
extern void QBSecureLogLogicStop(QBSecureLogLogic self) __attribute__((weak));

/**
 * Creates custom hello message content for SecureLog event.
 *
 * @param[in] self_     QBSecureLogLogic object
 * @return              created custom content for hello message
 **/
extern SvString QBSecureLogLogicCreatHelloMsgContent(SvObject self_) __attribute__((weak));

/**
 * Creates custom header for SecureLog event.
 *
 * @param[in] self_     QBSecureLogLogic object
 * @param[in] eventType type of logged event
 * @return              created custom header
 **/
extern SvString QBSecureLogLogicCreateCustomHeader(SvObject self_, SvString eventType) __attribute__((weak));

/**
* @}
**/

#endif /* QB_SECURE_LOG_LOGIC_H_ */
