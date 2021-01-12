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

#ifndef SVPLAYERTASKTRACKCONTROLLER_H
#define SVPLAYERTASKTRACKCONTROLLER_H

/**
 * @file SvPlayerTrackController.h Player Track Controller
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBDecoder.h>
#include <dataformat/sv_data_format.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif


#define DISABLE_SUBS_TRACK    UINT_MAX      // special index value used to disable current subtitles track


/**
 * @defgroup SvPlayerTrackController SvPlayerTrackController class
 * @{
 * Player Track Controller
 */

/**
 * @class SvPlayerTrackController
 * @extends SvObject
 */
typedef struct SvPlayerTrackController_ *SvPlayerTrackController;

/**
 * Track Controller types.
 * @enum SvPlayerTrackControllerType
 */
typedef enum SvPlayerTrackControllerType_ {
    SvPlayerTrackControllerType_audio, //!< audio track controller type
    SvPlayerTrackControllerType_video, //!< video track controller type
    SvPlayerTrackControllerType_subs,  //!< subs track controller type
    SvPlayerTrackControllerType_cnt,   //!< types count
} SvPlayerTrackControllerType;

/**
 * Set tracks parsed from format and notify listeners.
 *
 * @memberof SvPlayerTrackController
 *
 * @param[in] self SvPlayerTrackController object
 * @param[in] format format to be parsed
 */
void
SvPlayerTrackControllerSetTracksFromFormat(SvPlayerTrackController self, const struct svdataformat *format);

/**
 * Set tracks and notify listeners.
 *
 * @memberof SvPlayerTrackController
 *
 * @param[in] self SvPlayerTrackController object
 * @param[in] tracks tracks to be set
 */
void
SvPlayerTrackControllerSetTracks(SvPlayerTrackController self, SvArray tracks);

/**
 * Add tracks (does not notify listeners).
 *
 * @memberof SvPlayerTrackController
 *
 * @param[in] self SvPlayerTrackController object
 * @param[in] tracks tracks to be added
 */
void
SvPlayerTrackControllerAddTracks(SvPlayerTrackController self, SvArray tracks);

/**
 * Get type of Player Track Controller.
 *
 * @memberof SvPlayerTrackController
 *
 * @param[in] self SvPlayerTrackController object
 * @return type of Player Track Controller
 */
SvPlayerTrackControllerType
SvPlayerTrackControllerGetTrackControllerType(SvPlayerTrackController self);

/**
 * Get available tracks
 *
 * @memberof SvPlayerTrackController
 *
 * @param[in] self SvPlayerTrackController object
 * @return array of available tracks
 */
SvArray
SvPlayerTrackControllerGetTracks(SvPlayerTrackController self);

/**
 * Get current track index
 *
 * @memberof SvPlayerTrackController
 *
 * @param[in] self SvPlayerTrackController object
 * @return index of current track
 */
unsigned int
SvPlayerTrackControllerGetCurrentTrackIndex(SvPlayerTrackController self);

/**
 * Set track by index.
 *
 * @memberof SvPlayerTrackController
 *
 * @param[in] self SvPlayerTrackController object
 * @param idx index of track
 * @return 0 on success, otherwise -1
 */
int
SvPlayerTrackControllerSetTrackByIndex(SvPlayerTrackController self, unsigned int idx);

/**
 * Register Player Track Controller listener.
 *
 * @memberof SvPlayerTrackController
 *
 * @param[in] self SvPlayerTrackController object
 * @param[in] listener object implementing SvPlayerTrackControllerListener interface
 * @param[out] errorOut error information
 */
void
SvPlayerTrackControllerAddListener(SvPlayerTrackController self, SvObject listener, SvErrorInfo *errorOut);

/**
 * Unregister Player Track Controller listener.
 *
 * @memberof SvPlayerTrackController
 *
 * @param[in] self SvPlayerTrackController object
 * @param[in] listener listener to unregister from Player Track Controller
 */
void
SvPlayerTrackControllerRemoveListener(SvPlayerTrackController self, SvObject listener);

/**
 * Set decoder.
 *
 * @memberof SvPlayerTrackController
 *
 * @param[in] self SvPlayerTrackController object
 * @param[in] decoder decoder to be set
 */
void
SvPlayerTrackControllerSetDecoder(SvPlayerTrackController self, QBDecoder *decoder);

/**
 * Get runtime type identification object representing SvPlayerTrackController class.
 *
 * @relates SvPlayerTrackController
 *
 * @return SvPlayerTrackController runtime type identification object
 **/
SvType
SvPlayerTrackController_getType(void);

/**
 * Create new Player Track Controller.
 *
 * @memberof SvPlayerTrackController
 *
 * @param[in] type type of track to control
 * @param[out] errorOut error information
 * @return created SvPlayerTrackController object
 */
SvPlayerTrackController
SvPlayerTrackControllerCreate(SvPlayerTrackControllerType type, SvErrorInfo *errorOut);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* SVPLAYERTASKTRACKCONTROLLER_H */
