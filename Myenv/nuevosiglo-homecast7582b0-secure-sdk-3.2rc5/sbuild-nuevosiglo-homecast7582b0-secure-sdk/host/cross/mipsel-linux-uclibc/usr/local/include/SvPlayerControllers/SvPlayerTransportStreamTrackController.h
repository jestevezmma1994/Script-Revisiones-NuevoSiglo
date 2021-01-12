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

#ifndef SVPLAYERTRANSPORTSTREAMTRACKCONTROLLER_H
#define SVPLAYERTRANSPORTSTREAMTRACKCONTROLLER_H

/**
 * @file SvPlayerTransportStreamTrackController.h TS track controller
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvPlayerControllers/SvPlayerTrackController.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup SvPlayerTransportStreamTrackController SvPlayerTransportStreamTrackController class
 * @{
 * TS track controller
 */

/**
 * SvPlayerTransportStreamTrackController class.
 * @class SvPlayerTransportStreamTrackController
 * @extends SvPlayerTrackController
 */
typedef struct SvPlayerTransportStreamTrackController_ *SvPlayerTransportStreamTrackController;

/**
 * Get runtime type identification object representing SvPlayerTransportStreamTrackController class.
 *
 * @relates SvPlayerTransportStreamTrackController
 *
 * @return SvPlayerTransportStreamTrackController runtime type identification object
 **/
SvType
SvPlayerTransportStreamTrackController_getType(void);

/**
 * Create new TS track controller.
 *
 * @param[in] type type of track controller
 * @param[out] errorOut error information
 * @return created SvPlayerTransportStreamTrackController object
 */
SvPlayerTransportStreamTrackController
SvPlayerTransportStreamTrackControllerCreate(SvPlayerTrackControllerType type, SvErrorInfo *errorOut);

/**
 * @}
 */

#endif // SVPLAYERTRANSPORTSTREAMTRACKCONTROLLER_H
