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

#ifndef QBPLAYLISTCONTROLLER_H_
#define QBPLAYLISTCONTROLLER_H_

#include <SvDataBucket2/SvDBRawObject.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <fibers/c/fibers.h>
#include <CubiwareMWClient/Controllers/Player/QBMWProductUsageController.h>
#include <Services/core/QBMiddlewareManager.h>

typedef struct QBPlaylistController_* QBPlaylistController;

/**
 * Create playlist controller for AnyPlayer.
 *
 * @param[in] scheduler scheduler handle
 * @param[in] middlewareManager Middleware Manager handle
 * @param[in] qbMWProductUsageController Middleware Product Usage Controller handle
 * @param[in] listener AnyPlayerController listener object
 * @param[in] serviceId service for which controller is created.
 * @param[in] product product to handle
 * @param[in] server MW server info
 * @return created plugin or NULL in case of error
 */
QBPlaylistController
QBPlaylistControllerCreate(SvScheduler scheduler,
                           QBMiddlewareManager middlewareManager,
                           QBMWProductUsageController qbMWProductUsageController,
                           SvObject listener,
                           SvString serviceId,
                           SvDBRawObject product,
                           SvXMLRPCServerInfo server);

/**
 * Add new plugin to the controller.
 *
 * @param[in] self self object
 * @param[in] plugin plugin to be added
 */
void QBPlaylistControllerAddPlugin(QBPlaylistController self, SvObject plugin);

/**
 * Remove plugin from the controller.
 *
 * @param[in] self self object
 * @param[in] plugin plugin to be removed
 */
void QBPlaylistControllerRemovePlugin(QBPlaylistController self, SvObject plugin);

/**
 * Get runtime identifier of QBPlaylistController.
 *
 * @return QBPlaylistController type
 */
SvType QBPlaylistController_getType(void);

#endif

