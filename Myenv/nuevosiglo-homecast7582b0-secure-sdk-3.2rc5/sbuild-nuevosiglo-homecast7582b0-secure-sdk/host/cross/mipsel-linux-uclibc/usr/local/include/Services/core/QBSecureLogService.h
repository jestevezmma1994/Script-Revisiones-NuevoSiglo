/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBSECURELOGSERVICE_H_
#define QBSECURELOGSERVICE_H_

/**
 * @file QBSecureLogService.h Secure Logger Service class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
* @defgroup  QBSecureLogService Secure Logger Service
* @ingroup CubiTV_services
* @{
*
* This service is responsible for logging various events and sending them out to the remote server.
**/
typedef struct QBSecureLogService_ *QBSecureLogService;

/**
 * Creates Secure Log Manager object.
 *
 * @param [in] pushReceiver QBPushReceiver object
 * @param [in] secureLogLogic QBSecureLogLogic object
 * @param [in] confPath to configuration file
 * @return new Secure Log Manager instance
 **/
QBSecureLogService QBSecureLogServiceCreate(QBPushReceiver pushReceiver,
                                            SvObject secureLogLogic,
                                            SvString confPath);

/**
 * Start Secure Log Manager service.
 *
 * @param [in] self object
 * @param [in] scheduler SvScheduler object
 * @param[out] errorOut error info
 **/
void QBSecureLogServiceStart(QBSecureLogService self,
                             SvScheduler scheduler,
                             SvErrorInfo *errorOut);

/**
 * Stop Secure Log Manager service.
 *
 * @param [in] self object
 * @param[out] errorOut error info
 **/
void QBSecureLogServiceStop(QBSecureLogService self, SvErrorInfo *errorOut);

/**
 * @}
 **/
#endif /* QBSECURELOGSERVICE_H_ */
