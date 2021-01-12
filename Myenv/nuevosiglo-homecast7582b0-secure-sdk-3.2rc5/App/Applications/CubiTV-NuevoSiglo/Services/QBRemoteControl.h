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

#ifndef QB_REMOTE_CONTROL_H_
#define QB_REMOTE_CONTROL_H_

/**
 * @file QBRemoteControl.h
 * @brief Remote Control Service for handling control from mobile app
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBPlayerControllers/QBPlaybackStateController.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <fibers/c/fibers.h>
#include <main_decl.h>

/**
 * @defgroup QBRemoteControl Remote Control Service
 * @ingroup CubiTV_services
 * @{
 **/

/**
 * QBRemoteControl Service class.
 * @class QBRemoteControl
 * @extends SvObject
 **/
typedef struct QBRemoteControl_ *QBRemoteControl;

/**
 * Create QBRemoteControl service.
 *
 * @memberof QBRemoteControl
 *
 * @param[in] appGlobals global data handle
 * @param[out] errorOut error info
 * @return created QBRemoteControl service, @c NULL in case of error
 **/
QBRemoteControl
QBRemoteControlCreate(AppGlobals appGlobals,
                      SvErrorInfo *errorOut);

/**
 * Create QBPlaybackStateController - a class for handling sending state to MW
 *
 * @memberof QBRemoteControl
 *
 * @param[in] self QBRemoteControl handle
 * @param[out] errorOut error info
 * @return created QBPlaybackStateController, @c NULL in case of error
 **/
QBPlaybackStateController
QBRemoteControlCreatePlaybackStateController(QBRemoteControl self,
                                             SvErrorInfo *errorOut);

/**
 * Start QBRemoteControl service.
 *
 * @memberof QBRemoteControl
 *
 * @param[in] self QBRemoteControl handle
 * @param[out] errorOut error info
 * @return 0 if no error and -1 if error occurs
 **/
int
QBRemoteControlStart(QBRemoteControl self,
                     SvErrorInfo *errorOut);

/**
 * Stop QBRemoteControl service.
 *
 * @memberof QBRemoteControl
 *
 * @param[in] self QBRemoteControl handle
 * @param[out] errorOut error info
 * @return 0 if no error and -1 if error occurs
 **/
int
QBRemoteControlStop(QBRemoteControl self,
                    SvErrorInfo *errorOut);

/**
 * Add a listener that will be notified if a command not handled internally will
 * be received by remote control
 *
 * @memberof QBRemoteControl
 *
 * @param[in] self QBRemoteControl handle
 * @param[in] listener object implementing QBPushReceiverListener interface
 * @param[out] errorOut error info
 * @return 0 if no error and -1 if error occurs
 **/
int
QBRemoteControlAddListener(QBRemoteControl self,
                           SvGenericObject listener,
                           SvErrorInfo *errorOut);

/**
 * Remove a listener previously added by QBRemoteControlAddListener from
 * active listeners list
 *
 * @memberof QBRemoteControl
 *
 * @param[in] self QBRemoteControl handle
 * @param[in] listener object to be removed from listeners list
 * @param[out] errorOut error info
 * @return 0 if no error and -1 if error occurs
 **/
int
QBRemoteControlRemoveListener(QBRemoteControl self,
                              SvGenericObject listener,
                              SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QB_REMOTE_CONTROL_H_ */
