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

#ifndef QB_PLATFORM_TYPES_H_
#define QB_PLATFORM_TYPES_H_

/**
 * @file QBPlatformTypes.h Declarations of data types used in platform HAL API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <stdint.h>
#include <dataformat/audio.h>
#include <dataformat/sv_data_format.h>
#include <QBPlatformHAL/QBPlatformCEC.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformTypes Data types used in platform HAL API
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * TV system type.
 **/
typedef enum {
    /// reserved value
    QBTVSystem_unknown = 0,
    /// NTSC system
    QBTVSystem_NTSC,
    /// PAL system
    QBTVSystem_PAL
} QBTVSystem;

/**
 * GFX and video aspect ratio.
 **/
typedef enum {
    /// reserved value
    QBAspectRatio_unknown = 0,
    /// 16:9 aspect ratio
    QBAspectRatio_16x9,
    /// 4:3 aspect ratio
    QBAspectRatio_4x3
} QBAspectRatio;

/**
 * Video output standard.
 **/
typedef enum {
    /// special value that means "don't change"
    QBOutputStandard_current = -1,
    /// reserved
    QBOutputStandard_none = 0,
    /// 480 scanlines, 59.994 Hz, interlaced (NTSC)
    QBOutputStandard_480i59,
    /// 480 scanlines, 59.994 Hz, progressive
    QBOutputStandard_480p59,
    /// 576 scanlines, 50 Hz, interlaced (PAL)
    QBOutputStandard_576i50,
    /// 576 scanlines, 50 Hz, progressive
    QBOutputStandard_576p50,
    /// 720 scanlines, 50 Hz, progressive
    QBOutputStandard_720p50,
    /// 720 scanlines, 59.994 Hz, progressive
    QBOutputStandard_720p59,
    /// 1920x1080, 50 Hz, interlaced
    QBOutputStandard_1080i50,
    /// 1920x1080, 59.994 Hz, interlaced
    QBOutputStandard_1080i59,
    /// 1920x1080, 50 Hz, progressive
    QBOutputStandard_1080p50,
    /// 1920x1080, 59.994 Hz, progressive
    QBOutputStandard_1080p59,
    /// 1920x1080, 23.998 Hz, progressive
    QBOutputStandard_1080p24,
    /// 1920x1080, 25 Hz, progressive
    QBOutputStandard_1080p25,
    /// 1920x1080, 29.997 Hz, progressive
    QBOutputStandard_1080p30,
    /// 3840x2160, 24 Hz, progressive
    QBOutputStandard_2160p24,
    /// 3840x2160, 25 Hz, progressive
    QBOutputStandard_2160p25,
    /// 3840x2160, 30 Hz or 29.97 (not really certain), progressive
    QBOutputStandard_2160p30,
    /// 4096x2160, 24 Hz, progressive, Digital Cinema 4K DCI, this is not really a TV standard
    QBOutputStandard_4096x2160p24,
} QBOutputStandard;

/**
 * Video output capabilities.
 **/
typedef enum {
    /// HD output (must support at least 720p output standard)
    QBOutputCapability_HD = 0x0001,
    /// SD output (supports NTSC and/or PAL modes)
    QBOutputCapability_SD = 0x0002,
    /// HDCP is supported
    QBOutputCapability_HDCP = 0x0004,
    /// CGMS is supported
    QBOutputCapability_CGMS = 0x0008,
    /// CEC is supported
    QBOutputCapability_CEC = 0x0010,
    /// Macrovision is supported
    QBOutputCapability_Macrovision = 0x0020,
    /// Teletext via VBI is supported
    QBOutputCapability_VBITeletext  = 0x0040,
} QBOutputCapability;

/**
 * Video output type.
 **/
typedef enum {
    /// unknown output type
    QBOutputType_unknown = 0,
    /// HDMI output
    QBOutputType_HDMI,
    /// DVI output
    QBOutputType_DVI,
    /// YPbPr HD output (also known as Component Video)
    QBOutputType_YPbPr,
    /// CVBS SD output (also known as Composite Video)
    QBOutputType_CVBS,
    /// S-Video SD output (also known as S-VHS or Y/C)
    QBOutputType_SVideo,
    /// RGB component SD output (typically on SCART connector)
    QBOutputType_RGB,
    /// RF SD output
    QBOutputType_RF,
    /// other output type
    QBOutputType_other = 15
} QBOutputType;

/**
 * PAL system
 */
typedef enum QBPALSystem_ {
    /// default PAL G system
    QBPALSystem_default,
    /// PAL N system
    QBPALSystem_N,
    /// PAL CN system
    QBPALSystem_CN,
    /// PAL M system
    QBPALSystem_M,
    /// PAL B system
    QBPALSystem_B,
    /// PAL B1 system
    QBPALSystem_B1,
    /// PAL D system
    QBPALSystem_D,
    /// PAL D1 system
    QBPALSystem_D1,
    /// PAL DK1 system
    QBPALSystem_DK1,
    /// PAL DK2 system
    QBPALSystem_DK2,
    /// PAL DK3 system
    QBPALSystem_DK3,
    /// PAL G system
    QBPALSystem_G,
    /// PAL H system
    QBPALSystem_H,
    /// PAL K system
    QBPALSystem_K,
    /// PAL I system
    QBPALSystem_I,
    /// PAL 60Hz system
    QBPALSystem_60Hz,
} QBPALSystem;

/**
 * Output priority for settings
 **/
typedef enum {
    QBPlatformOutputPriority_unknown = 0,   /**< for backward compatibility, this is not prioritize */
    //Lowest priority
    QBPlatformOutputPriority_platform ,     /**< platform level priority - lowest */
    QBPlatformOutputPriority_application,   /**< application level priority */
    //Highest priority
    QBPlatformOutputPriority_content,       /**< content priority - highest */
    QBPlatformOutputPriority_max,           /**< max for counting */
} QBPlatformOutputPriority;

/**
 * HDCP protection status.
 **/
typedef enum {
    /// HDCP protection disabled
    QBHDCPProtection_disabled,
    /// HDCP protection initializing
    QBHDCPProtection_initializing,
    /// HDCP protection enabled
    QBHDCPProtection_enabled,

} QBHDCPProtectionStatus;

typedef enum {
    ///> No error or not recognized
    QBHDCPErrorCode_none,
    ///> HDCP Rx BKsv Error"
    QBHDCPErrorCode_eRxBksvError,
    ///> HDCP Rx BKsv/Keyset Revoked
    QBHDCPErrorCode_eRxBksvRevoked,
    ///> HDCP I2C Read Error
    QBHDCPErrorCode_eRxBksvI2cReadError,
    ///> HDCP Tx Aksv Error
    QBHDCPErrorCode_eTxAksvError,
    ///> HDCP I2C Write Error
    QBHDCPErrorCode_eTxAksvI2cWriteError,
    ///> HDCP Receiver Authentication Failure
    QBHDCPErrorCode_eReceiverAuthenticationError,
    ///> HDCP Repeater Authentication Failure
    QBHDCPErrorCode_eRepeaterAuthenticationError,
    ///> HDCP Repeater MAX Downstram Devices Exceeded
    QBHDCPErrorCode_eRxDevicesExceeded,
    ///> HDCP Repeater MAX Downstram Levels Exceeded
    QBHDCPErrorCode_eRepeaterDepthExceeded,
    ///> Timeout waiting for Repeater
    QBHDCPErrorCode_eRepeaterFifoNotReady,
    ///> HDCP Repeater Device count 0
    QBHDCPErrorCode_eRepeaterDeviceCount0,
    ///> HDCP Repeater Link Failure
    QBHDCPErrorCode_eRepeaterLinkFailure,
    ///> HDCP Ri Integrity Check Failure
    QBHDCPErrorCode_eLinkRiFailure,
    ///> HDCP Pj Integrity Check Failure
    QBHDCPErrorCode_eLinkPjFailure,
    ///> Video configuration issue - fifo underflow
    QBHDCPErrorCode_eFifoUnderflow,
    ///> Video configuration issue - fifo overflow
    QBHDCPErrorCode_eFifoOverflow,
    ///> Multiple Authentication Request...
    QBHDCPErrorCode_eMultipleAnRequest,
} QBHDCPErrorCode;

/**
 * Video output configuration.
 **/
typedef struct QBVideoOutputConfig_t {
    /// descriptive output name
    const char *name;

    /// output capabilities (binary mask)
    unsigned int capabilities;
    /// output type
    QBOutputType type;
    /// ID of the master output (points to itself in master outputs)
    unsigned int masterID;
    /// ID of the gfx master output (it's gfx frame buffer is used at current output)
    unsigned int masterGfxID;
    /// output width, read only, set by mode
    unsigned int width;
    /// output height, read only, set by mode
    unsigned int height;

    /// output mode (number of scanlines, refresh rate)
    QBOutputStandard mode;
    /// output PAL system (only for PAL output standard)
    QBPALSystem PALSystem;
    /// keeping mode cfg for all priorities, read only
    QBOutputStandard modePriorityCfg[QBPlatformOutputPriority_max];

    /// aspect ratio of the connected output device
    QBAspectRatio aspectRatio;
    /// keeping aspectRatio cfg for all priorities, read only
    QBAspectRatio aspectRatioPriorityCfg[QBPlatformOutputPriority_max];
    /// @c if true, video on this output should be disable

    bool mute;
    /// keeping mute cfg for all priorities, read only
    bool mutePriorityCfg[QBPlatformOutputPriority_max];

    /// @c true if output is enabled
    bool enabled;

    /// @c true if output is powered off
    bool powerOff;
    /// keeping powerOff cfg for all priorities, read only
    bool powerOffPriorityCfg[QBPlatformOutputPriority_max];

    /// required output content protection
    struct sv_content_protection contentProtection;
    /// content protection configuration requests of all priorities, used only if this is master output
    struct sv_content_protection contentProtectionPriorityCfg[QBPlatformOutputPriority_max];

    /// @c true if output is active, read-only
    bool active;
    /// HDCP protection status, read only
    QBHDCPProtectionStatus HDCPStatus;

    /// current output CEC status
    struct QBCECStatus_ CECStatus;

    /// @c true if VBI Teletext in active
    bool VBITeletextActive;

    /// HDCP error code, if exists
    QBHDCPErrorCode HDCPErrorCode;

    /// @cond
    uint32_t reserved[2];
    /// @endcond
} QBVideoOutputConfig;

/**
 * Viewport mode.
 **/
typedef enum {
    /// viewport is invisible
    QBViewportMode_disabled = 0,
    /// viewport is automatically configured to take entire output
    QBViewportMode_fullScreen,
    /**
     * viewport is displayed in some explicitly specified rectangle
     * of the output
     **/
    QBViewportMode_windowed
} QBViewportMode;

/**
 * Video content display mode.
 **/
typedef enum {
    /// reserved value
    QBContentDisplayMode_unknown = 0,
    /**
     * platform specific "zoom" mode, implemented only on some platforms
     * (use only for test purposes)
     **/
    QBContentDisplayMode_zoom,
    /**
     * letterbox mode: Fit image into viewport while keeping the aspect ratio.
     * Black areas on the sides of image may appear.
     **/
    QBContentDisplayMode_letterBox,
    /**
     * pan-scan mode: fill entire viewport while keeping the aspect
     * ratio, for the price of losing some part of video frame
     **/
    QBContentDisplayMode_panScan,
    /**
     * platform specific "combined" mode, implemented only on some platforms
     * (use only for test purposes)
     **/
    QBContentDisplayMode_combined,
    /**
     * stretched mode: fill entire viewport and don't keep aspect ratio.
     **/
    QBContentDisplayMode_stretched,
    /// kept for backward compatibility
    QBContentDisplayMode_full = QBContentDisplayMode_stretched,
} QBContentDisplayMode;

/**
 * DVB tuner type.
 **/
typedef enum {
    /// reserved value
    QBTunerType_unknown = 0,
    /// DVB-T or DVB-T2 (terrestrial) tuner
    QBTunerType_ter,
    /// DVB-S or DVB-S2 (satellite) tuner
    QBTunerType_sat,
    /// DVB-C (cable) tuner
    QBTunerType_cab,
    /// number of tuner types + 1
    QBTunerType_cnt
} QBTunerType;

/**
 * Audio output type.
 **/
typedef enum {
    /// reserved value
    QBAudioOutputType_unknown = 0,
    /// analog audio output (usually on RCA and/or SCART connector)
    QBAudioOutputType_analog,
    /// optical or electrical S/PDIF output
    QBAudioOutputType_SPDIF,
    /// HDMI audio output
    QBAudioOutputType_HDMI,
    /// other type of audio output
    QBAudioOutputType_other
} QBAudioOutputType;

/**
 * Audio output configuration.
 **/
typedef struct QBAudioOutputConfig_t {
    /// descriptive output name
    const char *name;

    /// output type
    QBAudioOutputType type;

    /// @c true if muting is supported for this output
    bool mutingControlSupported;
    /// @c true read-only, true if output is muted
    bool muted;
    /// mute state for specific priority
    bool mutePriority[QBPlatformOutputPriority_max];
    /// @c true if volume control is supported for this output
    bool volumeControlSupported;
    /// current volume level
    unsigned int volume;
    /// max volume level
    unsigned int maxVolume;

    /// @c true if output supports latency control
    bool latencyControlSupported;
    /// output latency in milliseconds
    unsigned int latency;

    /// @cond
    uint32_t reserved[3];
    /// @endcond
} QBAudioOutputConfig;

/**
 * Audio output format preference.
 **/
typedef struct QBAudioOutputFormat_ {
    /// audio output ID
    unsigned int outputID;

    /// input stream format
    struct {
        /// input stream audio codec
        QBAudioCodec codec;
        /**
         * @c true if audio stream carries more than 2 audio channels
         * (is not mono nor stereo)
         **/
        bool multiChannel;
    } input;

    /// output stream format
    struct {
        /// output stream audio codec
        QBAudioCodec codec;
        /**
         * @c true if audio stream carries more than 2 audio channels
         * (is not mono nor stereo)
         **/
        bool multiChannel;
    } output;
} QBAudioOutputFormat;

/**
 * Front panel display capability flags.
 **/
enum {
    /**
     * supports text display (as an opposite to a 7-segment display
     * that can only display digits)
     **/
    QBFrontPanelCapability_text = 0x0001,
    /// supports icon display (for example: HD, 1080i, CVBS icons)
    QBFrontPanelCapability_icons = 0x0002,
    /// supports clock display in standby mode
    QBFrontPanelCapability_clock = 0x0004,
};

/**
 * Standby mode flags
 **/
enum {
    /// supports passive standby
    QBStandbyCapability_passive = 0x0001,
};

/**
 * Front panel capabilities.
 **/
typedef struct QBFrontPanelCapabilities_ {
    /// capabilities, bitwise mask of QBFrontPanelCapability_* values
    unsigned int capabilities;
    /// number of characters visible on the display at a time
    unsigned int visibleCharacters;
    /**
     * total number of characters (for example: in scrolling mode)
     * that can be handled by the display
     **/
    unsigned int totalCharacters;

    /// @cond
    uint32_t reserved[1];
    /// @endcond
} QBFrontPanelCapabilities;

/**
 * HDCP Authorization status.
 **/
typedef enum {
    /// HDCP authorization error
    HDCPAuth_error,
    /// HDCP initializing
    HDCPAuth_initializing,
    /// HDCP authorization successful
    HDCPAuth_success,
    /// HDCP authorization disabled
    HDCPAuth_disabled
} HDCPAuthStatus;

/**
* Reset reasons.
**/
typedef enum {
    /// reset initiated by software
    QBPlatformResetReason_software = 0,
    /// first system boot after powering on
    QBPlatformResetReason_powerOn,
    /// reset initiated by watchdog
    QBPlatformResetReason_watchdogTimer,
    /// reset due to undervoltage
    QBPlatformResetReason_undervoltage,
    /// reset due to overvoltage
    QBPlatformResetReason_overvoltage,
    /// reset due to excessive temperature
    QBPlatformResetReason_overtemp,
    /// reset due to wakeup from standby
    QBPlatformResetReason_wakeup,
    /// unknown reset reason
    QBPlatformResetReason_unknown,
} QBPlatformResetReason;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
