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

#ifndef SVPLAYERCONTROLLERS_SVPLAYERCONTROLLERINTERNAL_H_
#define SVPLAYERCONTROLLERS_SVPLAYERCONTROLLERINTERNAL_H_

/**
 * @file SvPlayerControllerInternal.h Internals of Player Track Controller
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvPlayerControllers/SvPlayerTrackController.h>
#include <QBDecoder.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvWeakList.h>

/**
 * @defgroup SvPlayerControllerInternal SvPlayerTrackController internals
 * @{
 * Internals of Plater Track Controller
 **/

/**
 * Player Track Controller class.
 * @class SvPlayerTrackController
 * @extends SvObject
 */
struct SvPlayerTrackController_ {
    /** super class */
    struct SvObject_ super_;

    /** type of controlled class */
    SvPlayerTrackControllerType type;

    /** list of available tracks */
    SvArray tracks;

    /** current track */
    unsigned int currentTrackIdx;

    /** decoder */
    QBDecoder *decoder;

    /** decoder interface */
    struct QBDecoderTrackControllerIface decoderIface;

    /** list of controller listeners */
    SvWeakList listeners;

};

/**
 * Initialize previously allocated Player Track Controller.
 *
 * @memberof SvPlayerTrackController
 *
 * @param[in] self allocated SvPlayerTrackController
 * @param[in] type type of track to control
 * @param[out] errorOut error information
 */
void
SvPlayerTrackControllerInit(SvPlayerTrackController self, SvPlayerTrackControllerType type, SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* SVPLAYERCONTROLLERS_SVPLAYERCONTROLLERINTERNAL_H_ */
