/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef AUDIO_TRACK_LOGIC_H_
#define AUDIO_TRACK_LOGIC_H_

/**
 * @file AudioTrackLogic.h Audio track logic class API
 * @brief Audio track logic API
 **/

#include <SvPlayerControllers/SvPlayerTaskControllers.h>
#include <SvPlayerControllers/SvPlayerAudioTrack.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <QBAppKit/QBAsyncService.h>
#include <main_decl.h>
#include <stdbool.h>

/**
 * @defgroup AudioTrackLogic Audio track logic class
 * @ingroup CubiTV_logic
 * @{
 **/

/**
 * Audio track logic class
 *
 * @class AudioTrackLogic AudioTrackLogic.h <Logic/AudioTrackLogic.h>
 * @extends SvObject
 *
 * This is an application logic class implementing functionality related to audio track logic handling.
 * An instance of this class can be created using @ref QBLogicFactory.
 **/
typedef struct AudioTrackLogic_ {
    struct SvObject_ super_;

    QBAsyncServiceState serviceState;

    SvPlayerAudioTrack preferredTrack;

    SvPlayerTaskControllers playerTaskControllers;
    SvPlayerTrackController audioTrackController;

    SvHashTable langMap;

} *AudioTrackLogic;

/**
 * @relates AudioTrackLogic
 *
 * Get runtime type identification object representing video output logic class.
 *
 * @return AudioTrackLogic type identification object
 */
SvType AudioTrackLogic_getType(void);

/**
 * Audio track logic virtual methods table.
 **/
typedef const struct AudioTrackLogicVTable_ {
    struct SvObjectVTable_ super_;

    // As chosen automatically, at the start of playback .
    bool (*audioTrackLogicSetPreferredAudioTrack)(AudioTrackLogic self, SvPlayerAudioTrack audioTrack);
    int (*selectAudioTrack)(AudioTrackLogic self);
} *AudioTrackLogicVTable;

/**
 * Create AudioTrackLogic class.
 *
 * @param[out] errorOut error information
 * @return audio track logic object
 */
AudioTrackLogic AudioTrackLogicCreate(SvErrorInfo *errorOut);

/**
 * Sets up audio and video track according to preferred track (if set) or selections criteria.
 *
 * @param[in] self AudioTrackLogic handle
 * @return true on success, otherwise false
 */
bool AudioTrackLogicSetupAudioTrack(AudioTrackLogic self);

/**
 * Select track directly, during playback.
 *
 * @param[in] self AudioTrackLogic handle
 * @param[in] audioTrack track to be set
 * @return true on success, otherwise false
 */
bool AudioTrackLogicSetAudioTrack(AudioTrackLogic self, SvPlayerAudioTrack audioTrack);

/**
 * Create array with audio tracks description.
 *
 * Single audio track is described by SvHashTable containing
 * "audioTrack" -> corresponding SvPlayerAudioTrack object
 * "caption"    -> caption string
 *
 * @param[in] self AudioTrackLogic handle
 * @return audio tracks description
 */
SvArray AudioTrackLogicComputeLangMenuItems(AudioTrackLogic self);

/**
 * Set preferred audio track.
 *
 * @param[in] self AudioTrackLogic handle
 * @param[in] audioTrack track to be set as preferred track
 */
static inline void AudioTrackLogicSetPreferredAudioTrack(AudioTrackLogic self, SvPlayerAudioTrack audioTrack)
{
    SvInvokeVirtual(AudioTrackLogic, self, audioTrackLogicSetPreferredAudioTrack, audioTrack);
}

/**
 * Set PlayerTask controllers.
 *
 * @param[in] self AudioTrackLogic handle
 * @param[in] controllers track controllers to be set
 */
void AudioTrackLogicSetPlayerTaskControllers(AudioTrackLogic self, SvPlayerTaskControllers controllers);

/**
 * Get associated PlayerTask controllers.
 *
 * @param[in] self AudioTrackLogic handle
 * @return PlayerTask controllers
 */
SvPlayerTaskControllers AudioTrackLogicGetPlayerTaskControllers(AudioTrackLogic self);

/**
 * @}
 **/

#endif
