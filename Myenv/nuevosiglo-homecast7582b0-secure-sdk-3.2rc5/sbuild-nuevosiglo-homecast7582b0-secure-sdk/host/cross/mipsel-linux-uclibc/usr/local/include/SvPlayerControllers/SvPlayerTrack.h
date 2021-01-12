/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SVPLAYERTRACK_H
#define SVPLAYERTRACK_H

/**
 * @file SvPlayerTrack.h Player's track representation
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup SvPlayerTrack SvPlayerTrack class
 * @{
 * Player's track representation
 */

/**
 * SvPlayerTrack abstract class.
 * @class SvPlayerTrack
 * @extends SvObject
 */
typedef struct SvPlayerTrack_ {
    struct SvObject_ super_;    /**< super type */

    unsigned int id;            /**< id of track */
} *SvPlayerTrack;

/**
 * Get runtime type identification object representing SvPlayerTrack class.
 *
 * @relates SvPlayerTrack
 *
 * @return SvPlayerTrack runtime type identification object
 **/
SvType
SvPlayerTrack_getType(void);

/**
 * Get id of track.
 *
 * @param[in] self SvPlayerTrack object
 * @return id of track
 */
unsigned int
SvPlayerTrackGetId(SvPlayerTrack self);

/**
 * @}
 */

#endif // SVPLAYERTRACK_H
