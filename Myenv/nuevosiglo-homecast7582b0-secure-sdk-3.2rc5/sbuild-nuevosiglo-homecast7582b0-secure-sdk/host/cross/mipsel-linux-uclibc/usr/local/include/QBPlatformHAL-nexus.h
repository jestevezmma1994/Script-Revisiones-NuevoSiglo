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

/* SMP/HAL/QBPlatformHAL-nexus/QBPlatformHAL-nexus.h */

#ifndef QB_PLATFORM_HAL_NEXUS_H_
#define QB_PLATFORM_HAL_NEXUS_H_

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_adj.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_output.h"
#include "nexus_audio_decoder.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_hdmi_output_hdcp.h"
#include "nexus_core_utils.h"
#include "bstd.h"

#include <dataformat/audio.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <CAGE/Core/Sv2DRect.h>
#include <CAGE/Core/SvColor.h>

extern NEXUS_DisplayHandle bcmImplDisplayHandle[16];
extern NEXUS_VideoWindowHandle bcmImplVideoWindowHandle[16];

// this file should not be used by any component other than NEXUS HAL modules!


/**
 * Identifiers for video outputs on this platform.
 **/
enum {
    BCMOutput_CVBS = 0,
    BCMOutput_HDMI = 1,
    BCMOutput_YPbPr = 2,
    BCMOutput_YC = 3,
    BCMOutput_RGB = 4,
    BCMOutput_RF = 5,
    BCMOutputs__count__ // must always be the last one
};

/**
 * Identifiers for audio outputs on this platform.
 **/
enum {
    BCMAudioOutput_analog = 0,
    BCMAudioOutput_SPDIF = 1,
    BCMAudioOutput_HDMI = 2,
    BCMAudioOutput_RFM = 3,
    BCMAudioOutputs__count__ // must always be the last one
};

/**
 * Setup GFX viewport.
 *
 * @param[in] outputID     ID of the output to configure viewport for
 * @param[in] mode         desired viewport mode
 * @param[in] outputRect   output rectangle, used only
 *                         if @a mode == QBViewportMode::QBViewportMode_windowed
 * @param[in] inputRect    input rectangle, @c NULL to keep current value
 * @return                 @c 0 on success, @c -1 in case of error
 **/
extern int BCMSetupGFXViewport(unsigned int outputID,
                               QBViewportMode mode,
                               const Sv2DRect *outputRect,
                               const Sv2DRect *inputRect);

/**
 * Get GFX viewport.
 *
 * @param[in] outputID     ID of the output to get viewport for
 * @param[out] mode        viewport mode
 *                         (can be @c NULL if not needed)
 * @param[out] outputRect  output rectangle, relevant only
 *                         if @a mode == QBViewportMode::QBViewportMode_windowed
 *                         (can be @c NULL if not needed)
 * @param[out] inputRect   input rectangle
 *                         (can be @c NULL if not needed)
 * @return                 @c 0 on success, @c -1 in case of error
 **/
extern int BCMGetGFXViewport(unsigned int outputID,
                             QBViewportMode *mode,
                             Sv2DRect *outputRect,
                             Sv2DRect *inputRect);

/**
 * Print available memory (Nexus and kernel)
 **/
extern void BCMPrintMem();

/**
 * Fill out the platformImpl.outputs static structure
 * with supported video output formats.
 * When SurfaceCompositor is being used also instantiate it here.
 **/
extern void BCMOutputsInit(void);

/**
 * Deinstantiate SurfaceCompositor
 **/
extern void BCMOutputsDeinit(void);

/**
 * Fill out the platformImpl.audioOutputs static structure
 * with supported audio outputs
 **/
extern void BCMAudioOutputsInit(void);

extern NEXUS_DisplayHandle BCMGetDisplayHandle(int output);
extern int BCMGetMasterOutput(int output);
extern int BCMApplyDeinterlace(int output, bool enable);

extern NEXUS_StcChannelHandle BCMGetStcChannel(void);

extern NEXUS_AudioCodec BCMAudioCodecToNexus(QBAudioCodec codec);
extern QBAudioCodec BCMAudioCodecFromNexus(NEXUS_AudioCodec codec);

extern QBOutputStandard BCMNexusVideoFormatToQBOutputStandard(NEXUS_VideoFormat formatId);

/**
 * Setup Analog Video Protections on given output
 *
 * @param[in] outputID                 ID of the output to configure viewport for
 * @param[in] cgmsEnabled              indicates if CGMS-A protection should be enabled
 * @param[in] cgmsValue                CGMS-A protections bits
 * @param[in] macrovision_settings     Macrovision protection configuration @see QBContentProtectionMacrovisionSettings
 * @return @c 0 on success @c -1 on failure
 */
extern int BCMSetupAnalogVideoOutputProtection(unsigned int output, bool cgmsEnabled, unsigned int cgmsValue, const QBContentProtectionMacrovisionSettings *macrovision_settings);


/**
 * Translate SvColorSpace to NEXUS and return NEXUS pixel format.
 * @param[in] csp           host color space
 * @param[out] hwCSP        destination NEXUS color space
 * @return                  NEXUS pixel format (default: NEXUS_PixelFormat_eA8_R8_G8_B8)
 */
extern NEXUS_PixelFormat BCMSvColorSpaceToNexusPixelFormat(SvColorSpace csp, SvColorSpace *hwCSP);

/**
 * Translate qb volume to NEXUS volume
 * @param[in] vol       volume from application
 * @param[in] max       max volume used by application
 * @param[in] type        linear value or in decibels
 * @return                attenuation in db used to control volume in NEXUS
 */
extern int BCMQBVolumeToNexus(const int vol, const int max, const NEXUS_AudioVolumeType type);

/**
 * Reads memory size from sysfs.
 *
 * @param[out] memorySize  memory size in MiB
 * @return                @c 0 on success, @c -1 in case of error
 **/
extern int BCMUtilGetMemorySizeFromSys(unsigned int *memorySize);

/**
 * Reads model ID from sysfs.
 *
 * @param[out] modelId     model ID
 * @return                @c 0 on success, @c -1 in case of error
 **/
extern int BCMUtilGetModelIdFromSys(unsigned int *modelId);

/**
 * Checks if given audio codec is supported
 *
 * @param[in] codecType: decoding or passthrough
 * @param[in] codec: audio codec
 *
 * @return true if supported, false otherwise
 **/
extern bool BCMAudioCodecIsSupported(QBAudioCodecType codecType, QBAudioCodec codec);

/**
 * Check if given video codec is supported
 *
 * @param[in] codec: video codec
 *
 * @return true if supported, false otherwise
 **/
extern bool BCMVideoCodecIsSupported(NEXUS_VideoCodec codec);

/**
 * Check if board uses internal RF Output.
 *
 * @return          @c true in case if board uses RF internal Output, @c false otherwise
 **/
extern bool BCMRFOutputUsesInternalRFM(void);
#endif
