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

#ifndef QBCECSERVICE_H_
#define QBCECSERVICE_H_

/**
 * @file cecService.h
 * @brief QBCEC manipulation service, specific with configuration for Multimedia Polska
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>
#include <stdbool.h>
#include <CUIT/Core/types.h>
#include <QBInput/QBInputEvent.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBCECService handler class
 * @ingroup CubiTV_services
 * @{
 *
 * Requirements for CEC are following:
 * CEC should be enabled or disabled through menu (item "TV control" in user interface). Following command to be supported:
 * 1) Turning on / off TV - send command when pressing RCU's power button. When going to passive or active standby STB sends
 *                          standby function to TV. When waking up STB wakes up TV.
 * 2) Volume up / down -    always follow RCU's keys, set STB's volume to 100%. Other outputs won't receive any change of volume;
 *                          if HDMI-based TV managing is enabled only a TV connected via HDMI will change the volume level.
 *                          A cubiware's volume widget will not be shown on screen.
 *
 **/

/**
 * QBCECService class.
 * @class QBCecService
 * @extends SvObject
 **/
typedef struct QBCecService_* QBCecService;

/**
 * Create QBCecService
 *
 * @memberof QBCecService
 *
 * @param[in] appGlobals    application globals
 * @return created QBCECService, @c NULL in case of error
 **/
QBCecService QBCecServiceCreate(AppGlobals appGlobals);

/**
 * Start service. This function sets tap on changes of "CEC" enabled/disabled value, configurable in the XMB menu.
 *
 * @param self  @ref QBCecService handle
 */
void QBCecServiceStart(QBCecService self);

/**
 * Stop service. This function removes tap on changes of "CEC" enabled/disabled value, configurable in the XMB menu.
 *
 * @param self @ref QBCecService handle
 */
void QBCecServiceStop(QBCecService self);

/**
 * Input Event Handler - parse input (RCU's buttons) whether a user pressed Volume Up or Volume Down.
 * If so and CEC is enabled, then perform an action and consume (return non-zero value) the input.
 *
 * @param self @ref QBCecService handle
 * @param e input event
 * @return value whether input was consumed or not
 */
bool QBCecServiceInputEventHandler(QBCecService self, const QBInputEvent* e);

/**
 * Wrapper to broadcast active source. This functions checks whether CEC is enabled.
 *
 * @param self @ref QBCecService handle
 */
void QBCecServiceOneTouchPlay(QBCecService self);

/**
 * Wrapper to broadcast standby. This functions checks whether CEC is enabled.
 *
 * @param self  @ref QBCecService handle
 */
void QBCecServiceBroadcastStandby(QBCecService self);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
