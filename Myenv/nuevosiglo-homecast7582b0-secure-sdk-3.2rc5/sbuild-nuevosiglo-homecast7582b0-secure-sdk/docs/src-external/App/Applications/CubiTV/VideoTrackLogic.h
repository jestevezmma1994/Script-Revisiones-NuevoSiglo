/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2017 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef VIDEO_TRACK_LOGIC_H_
#define VIDEO_TRACK_LOGIC_H_

/**
 * @file VideoTrackLogic.h Video track logic class API
 * @brief Video track logic API
 **/

#include <SvPlayerControllers/SvPlayerTaskControllers.h>
#include <dataformat/sv_data_format.h>
#include <QBAppKit/QBAsyncService.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <stdbool.h>
#include <main_decl.h>

/**
 * @defgroup VideoTrackLogic Video track logic class
 * @ingroup CubiTV_logic
 * @{
 **/

/**
 * Video track logic class
 *
 * @class VideoTrackLogic VideoTrackLogic.h <Logic/VideoTrackLogic.h>
 * @extends SvObject
 *
 * This is an application logic class implementing functionality related to video track logic handling.
 * An instance of this class can be created using @ref QBLogicFactory.
 **/
typedef struct VideoTrackLogic_ {
    struct SvObject_ super_;

    QBAsyncServiceState serviceState;

    SvPlayerTaskControllers playerTaskControllers;
    SvPlayerTrackController videoTrackController;

} *VideoTrackLogic;

/**
 * @relates VideoTrackLogic
 *
 * Get runtime type identification object representing video output logic class.
 *
 * @return VideoTrackLogic type identification object
 */
SvType VideoTrackLogic_getType(void);

/**
 * Video track logic virtual methods table.
 **/
typedef const struct VideoTrackLogicVTable_ {
    struct SvObjectVTable_ super_;
} *VideoTrackLogicVTable;

/**
 * Create VideoTrackLogic class.
 *
 * @param[out] errorOut error information
 * @return video track logic object
 */
VideoTrackLogic VideoTrackLogicCreate(SvErrorInfo *errorOut);

/**
 * Sets up video track according to preferred track (if set) or selections criteria.
 *
 * @param[in] self Video TrackLogic handle
 * @return true on success, otherwise false
 */
bool VideoTrackLogicSetupVideoTrack(VideoTrackLogic self);

/**
 * Set PlayerTask controllers.
 *
 * @param[in] self VideoTrackLogic handle
 * @param[in] controllers track controllers to be set
 */
void VideoTrackLogicSetPlayerTaskControllers(VideoTrackLogic self, SvPlayerTaskControllers controllers);

/**
 * Get associated PlayerTask controllers.
 *
 * @param[in] self VideoTrackLogic handle
 * @return PlayerTask controllers
 */
SvPlayerTaskControllers VideoTrackLogicGetPlayerTaskControllers(VideoTrackLogic self);

/**
 * @}
 **/

#endif
