/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_PLAYER_CONFIG_H_
#define SV_PLAYER_CONFIG_H_

/**
 * @file SvPlayerConfig.h SvPlayerConfig class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


#include <SvPlayerKit/QBSmoothStreaming/QBSmoothStreamingDownloaderConf.h>
#include <SvFoundation/SvHashTable.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvPlayerConfig Player configuration class
 * @ingroup SvPlayerKit
 * @{
 **/

/**
 * @brief Indicates how player should handle errors in the stream.
 *
 * @note Effect of every option may vary between platforms.
 */
typedef enum SvPlaybackErrorHandlingPolicy_e {
    SvPlaybackErrorHandlingPolicy_none,            //!< Show every frame, even if it is corrupted.
    SvPlaybackErrorHandlingPolicy_partial,         //!< Show every frame, but try to fix it if it's broken
    SvPlaybackErrorHandlingPolicy_dropBrokenFrames,//!< Do not show detected corrupted frames.
    SvPlaybackErrorHandlingPolicy_heuristic,       //!< Use platform specific heuristic algorithm to detect broken fragments.
    SvPlaybackErrorHandlingPolicy_max,             //!< count of possible enum values
} SvPlayerErrorHandlingPolicy;

/**
 * @brief Indicates how STC clock should be set up.
 *
 * @note Effect of every option may vary between platforms. It is implmented only on selected platforms.
 *
 */
typedef enum SvPlayerConfigAVSyncMode_e {
    SvPlayerConfigAVSyncMode_manualPCR,            //!< Player will parse PCRs and set STC clock according to them - default mode used as a fallback
    SvPlayerConfigAVSyncMode_autoPTS,              //!< Audio and video decoder will set STC automatically based on PTS
    SvPlayerConfigAVSyncMode_autoPCR,              //!< Audio and video decoder will set STC automatically based on PCR
    SvPlayerConfigAVSyncMode_max                   //!< count of possible enum values
} SvPlayerConfigAVSyncMode;

/**
 * @brief Informs about the player.
 */
typedef enum SvPlayerConfigContentType_e {
    SvPlayerConfigContentType_default,                   //!< Player is no defined.
    SvPlayerConfigContentType_smoothStreaming_live,      //!< Smooth streaming for live content.
    SvPlayerConfigContentType_smoothStreaming_onDemand   //!< Smooth streaming for content playing on demand.
} SvPlayerConfigContentType;

/**
 * @class SvPlayerConfig
 * @brief Container for player configuration.
 */
typedef struct SvPlayerConfig_s *SvPlayerConfig;

/**
 * Create SvPlayerConfig instance with default settings.
 * @return instance of SvPlayerConfig with default settings.
 */
SvPlayerConfig SvPlayerConfigCreate(void);

/**
 * Create SvPlayerConfig instance with specific settings.
 *
 * @param[in] settings parsed from proper config file.
 * @return instance of SvPlayerConfig with proper settings.
 */
SvPlayerConfig SvPlayerConfigCreateFromSettings(SvHashTable settings);

/**
 * Get error handling policy.
 * @param[in] self instance of SvPlayerConfig
 * @return error handling policy
 */
SvPlayerErrorHandlingPolicy SvPlayerConfigGetErrorHandlingPolicy(SvPlayerConfig self);

/**
 * Get sync mode for dvb live content.
 * @param[in] self instance of SvPlayerConfig
 * @return audio video sync mode @link SvPlaybackErrorHandlingPolicy_e @endlink
 */
SvPlayerConfigAVSyncMode SvPlayerConfigGetDvbLiveAVSyncMode(SvPlayerConfig self);

/**
 * Get PID filtering for IP streams setting
 * @param[in] self instance of SvPlayerConfig
 * @return if Pid Filtering for IP streams is enabled
 */
bool SvPlayerConfigUsePidFilteringForIPStream(SvPlayerConfig self);

/**
 * Check if player should wait for correct IFrame before starting playback.
 *
 * @param[in] self SvPlayerConfig instance
 * @return @c true if player should wait for correct IFrame, @c false otherwise or in case of error
 */
bool SvPlayerConfigShouldWaitForCorrectIFrame(SvPlayerConfig self);

/**
 * Check if player should decode IP stream data before first ECM processed.
 *
 * @param[in] self SvPlayerConfig instance
 * @return @c true if player should decode IP stream data before first ECM processed, @c false otherwise
 */
bool SvPlayerConfigShouldDecodeIPStreamBeforeFirstECMProcessed(SvPlayerConfig self);

/**
 * Get EIT Present/Following Actual table using policy.
 * @param[in] self instance of SvPlayerConfig
 * @return if use Present/Following Actual table
 */
bool SvPlayerConfigUseEITPresentFollowingActual(SvPlayerConfig self);


/**
 * Get EIT Present/Following Other table using policy.
 * @param[in] self instance of SvPlayerConfig
 * @return if use Present/Following Other table
 */
bool SvPlayerConfigUseEITPresentFollowingOther(SvPlayerConfig self);

/**
 * Get size of buffer for player, which is specified by type.
 * @param[in] self instance of SvPlayerConfig, if NULL, method returns '-1'
 * @param[in] type of player, if out of range, method returns '-1'
 * @return size of buffer, in bytes
 */
int SvPlayerConfigGetBufferSize(SvPlayerConfig self, SvPlayerConfigContentType type);

/**
 * Get amount of time, which should be buffered, before playing videos on demand and NPvR.
 * @param[in] self instance of SvPlayerConfig, if NULL, method returns '-1'
 * @return number of seconds which should be buffered
 */
int SvPlayerConfigGetPrefillBufferSec(SvPlayerConfig self);

/**
 * Get numbers of bytes, which should be buffered, before playing videos on demand and NPvR.
 * @param[in] self instance of SvPlayerConfig, if NULL, method returns '-1'
 * @return number of bytes which should be buffered
 */
int SvPlayerConfigGetPrefillBufferBytes(SvPlayerConfig self);

/**
 * Get buffer size to be used by curl when playing MPEG-DASH content.
 *
 * @param[in] self  instance of SvPlayerConfig
 * @return          buffer size to be used by curl
 **/
int SvPlayerConfigGetDASHCurlBufferSize(SvPlayerConfig self);

/**
 * Smooth streaming. Get config for Smooth Streaming.
 * @param[in] self instance of SvPlayerConfig
 * @return    SmoothStreaming config class
 */
QBSmoothStreamingDownloaderConf SvPlayerConfigGetSmoothStreamingSettings(SvPlayerConfig self);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif //SV_PLAYER_CONFIG_
