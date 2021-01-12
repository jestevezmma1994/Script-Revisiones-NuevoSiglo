/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBVODANYPLAYERCONTROLLER_H_
#define QBVODANYPLAYERCONTROLLER_H_

/**
 * @file    QBVoDAnyPlayerController.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief   VOD any player controller API
 **/

#include <fibers/c/fibers.h>
#include <Services/core/QBMiddlewareManager.h>
#include <CubiwareMWClient/Controllers/Player/QBMWProductUsageController.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup    QBVoDAnyPlayerController VOD any player controller class
 * @ingroup     CubiTV_controllers
 * @{
 **/

/**
 * VOD any player controller class.
 **/
typedef struct QBVoDAnyPlayerController_* QBVoDAnyPlayerController;

/**
 * Create VOD any player controller.
 *
 * @param[in] scheduler                        scheduler handle
 * @param[in] middlewareManager             Middleware Manager handle
 * @param[in] qbMWProductUsageController     Middleware Product Usage Controller handle
 * @param[in] listener      handle to an object implementing QBAnyPlayerControllerListener
 * @return                  new VOD any player controller instance or @c NULL
 *                          in case of error
 **/
QBVoDAnyPlayerController QBVoDAnyPlayerControllerCreate(SvScheduler scheduler,
                                                        QBMiddlewareManager middlewareManager,
                                                        QBMWProductUsageController qbMWProductUsageController,
                                                        SvObject listener);

/**
 * Set product that will be used for playback.
 *
 * @param[in] self VOD any player controller handle
 * @param[in] product product handle
 */
void QBVoDAnyPlayerControllerSetProduct(QBVoDAnyPlayerController self, SvObject product);

/**
 * Add new plugin to player controller.
 *
 * @param[in] self      VOD any player controller handle
 * @param[in] newPlugin plugin handle to be added
 */
void QBVoDAnyPlayerControllerAddPlugin(QBVoDAnyPlayerController self, SvObject newPlugin);

/**
 * Get runtime type identification object representing QBVoDAnyPlayerController class.
 *
 * @return VOD any player controller class
 **/
SvType QBVoDAnyPlayerController_getType(void);

/**
 * @}
 **/
#endif /* QBVODANYPLAYERCONTROLLER_H_ */
