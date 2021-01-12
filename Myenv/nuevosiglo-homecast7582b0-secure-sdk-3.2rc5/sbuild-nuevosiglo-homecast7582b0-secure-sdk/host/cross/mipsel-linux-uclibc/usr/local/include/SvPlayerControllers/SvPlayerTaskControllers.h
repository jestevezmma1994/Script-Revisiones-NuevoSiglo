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

#ifndef SVPLAYERTASKCONTROLLERS_H
#define SVPLAYERTASKCONTROLLERS_H

/**
 * @file SvPlayerTaskController.h Player Task track Controller
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvPlayerControllers/SvPlayerTrackController.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBDecoder.h>

/**
 * @defgroup SvPlayerTaskController SvPlayerTaskController class
 * @{
 * Player Task Track Controller
 */
typedef struct SvPlayerTaskControllers_ *SvPlayerTaskControllers;

/**
 * Get track controller of given type.
 *
 * @memberof SvPlayerTaskControllers
 *
 * @param[in] self SvPlayerTaskController object
 * @param[in] type type of controller to be returned
 * @return track controller of given type, or @c NULL if not set
 */
SvPlayerTrackController
SvPlayerTaskControllersGetTrackController(SvPlayerTaskControllers self, SvPlayerTrackControllerType type);

/**
 * Add listener.
 *
 * @param[in] self SvPlayerTaskControllers object
 * @param[in] listener object implementing SvPlayerTaskControllersListener interface
 * @param[out] errorOut error information
 */
void
SvPlayerTaskControllersAddListener(SvPlayerTaskControllers self, SvObject listener, SvErrorInfo *errorOut);

/**
 * Remove listener.
 *
 * @memberof SvPlayerTaskControllers
 *
 * @param[in] self SvPlayerTaskControllers object
 * @param[in] listener listener to unregister from Player Task Controllers
 */
void
SvPlayerTaskControllersRemoveListener(SvPlayerTaskControllers self, SvObject listener);

/**
 * Add format update listener.
 *
 * @param[in] self SvPlayerTaskControllers object
 * @param[in] listener object implementing SvPlayerTaskControllersFormatUpdateListener interface
 * @param[out] errorOut error information
 */
void
SvPlayerTaskControllersAddFormatUpdateListener(SvPlayerTaskControllers self, SvObject listener, SvErrorInfo *errorOut);

/**
 * Remove format update listener.
 *
 * @memberof SvPlayerTaskControllers
 *
 * @param[in] self SvPlayerTaskControllers object
 * @param[in] listener format update listener to unregister from Player Task Controllers
 */
void
SvPlayerTaskControllersRemoveFormatUpdateListener(SvPlayerTaskControllers self, SvObject listener);

/**
 * Set track controller.
 *
 * @memberof SvPlayerTaskControllers
 *
 * @param[in] self SvPlayerTaskController object
 * @param[in] controller track controller to be set
 */
void
SvPlayerTaskControllersSetTrackController(SvPlayerTaskControllers self, SvPlayerTrackController controller);

/**
 * Set decoder to all track decoders associated in Player Task Controllers.
 *
 * @memberof SvPlayerTaskControllers
 *
 * @param[in] self SvPlayerTaskController object
 * @param[in] decoder decoder to be set
 */
void
SvPlayerTaskControllersSetDecoder(SvPlayerTaskControllers self, QBDecoder *decoder);

/**
 * Get decoder of tracks associated in Player Task Controllers.
 *
 * @memberof SvPlayerTaskControllers
 *
 * @param[in] self SvPlayerTaskController object
 * @return decoder of tracks associated in Player Task Controllers
 */
QBDecoder *SvPlayerTaskControllersGetDecoder(SvPlayerTaskControllers self);

/**
 * Get runtime type identification object representing SvPlayerTaskController class.
 *
 * @relates SvPlayerTaskController
 *
 * @return SvPlayerTaskController runtime type identification object
 **/
SvType
SvPlayerTaskControllers_getType(void);

/**
 * Create new SvPlayerTaskControllers object.
 *
 * @memberof SvPlayerTaskControllers
 *
 * @param[out] errorOut error information
 * @return created SvPlayerTrackController object
 */
SvPlayerTaskControllers
SvPlayerTaskControllersCreate(SvErrorInfo *errorOut);

/**
 * @}
 */

#endif
