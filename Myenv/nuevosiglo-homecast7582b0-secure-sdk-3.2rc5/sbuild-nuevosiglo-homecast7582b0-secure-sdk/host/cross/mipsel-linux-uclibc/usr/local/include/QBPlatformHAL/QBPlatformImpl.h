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

#ifndef QB_PLATFORM_IMPL_H_
#define QB_PLATFORM_IMPL_H_

/**
 * @file QBPlatformImpl.h Structure describing platform-specific HAL implementation
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <QBPlatformHAL-Gfx.h>
#include <QBPlatformHAL/QBPlatformInit.h>
#include <QBPlatformHAL/QBPlatformCEC.h>
#include <QBPlatformHAL/QBPlatformVBI.h>
#include <QBPlatformHAL/QBPlatformHDMIOutput.h>
#include <QBPlatformHAL/QBPlatformTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformImpl Private interface of the platform-specific HAL implementation
 * @ingroup QBPlatformHAL
 * @{
 **/


// this file should only be used by platform-specific implementation of platform HAL!


/**
 * Max number of video outputs that can be defined.
 **/
#define QBVideoOutputs__max__ 8

/**
 * Max number of audio outputs that can be defined.
 **/
#define QBAudioOutputs__max__ 8


/**
 * Descriptor of the platform HAL implementation.
 **/
typedef struct QBPlatformImpl_ {
    /**
     * Initialization marker.
     **/
    bool initialized;

    /**
     * Lock that guards access to this structure.
     *
     * @note All functions called by QBPlatformHAL via pointers
     * defined below are called with this lock held.
     **/
    pthread_rwlock_t lock;

    /**
     * Number of video outputs available.
     **/
    unsigned int outputsCount;

    /**
     * Configuration of all video outputs.
     **/
    struct QBVideoOutputConfig_t outputs[QBVideoOutputs__max__];

    /**
     * Number of audio outputs available.
     **/
    unsigned int audioOutputsCount;

    /**
     * Configuration of all audio outputs.
     **/
    struct QBAudioOutputConfig_t audioOutputs[QBAudioOutputs__max__];


    /**
     * Initialize all hardware components used by platform HAL.
     *
     * @param[in] config initialization cofiguration
     *
     * @return              @c 0 on success, @c -1 in case of error
     **/
    int (*initialize)(const QBPlatformInitConfig *config);

    /**
     * Terminate platform HAL modules and free all used resources.
     **/
    void (*cleanup)(void);

    /**
     * Get Gfx engine implementation
     *
     * @return              Gfx engine implementation handle
     **/
    const QBGfxEngineImplementation* (*getGfxEngineImplementation)(void);

    /**
     * Initialize video output.
     *
     * This function initializes all hardware modules needed to generate
     * usable video signal on given @a output. Actual video mode is
     * selected to closely match requested one. After initialization
     * output remains disabled, i.e. there is no video signal on the
     * physical output port.
     *
     * @param[in] output    video output ID
     * @param[in,out] config on input: desired video output configuration,
     *                      on output: actual output configuration
     * @return              @c 0 on success, @c -1 in case of error
     **/
    int (*openOutput)(unsigned int output, QBVideoOutputConfig *config);

    /**
     * Close video output.
     *
     * This function deinitializes hardware modules that take part
     * in generating video signal on given @a output.
     *
     * @param[in] output    video output ID
     * @return              @c 0 on success, @c -1 in case of error
     **/
    int (*closeOutput)(unsigned int output);

    /**
     * Set video mode of a video output.
     *
     * This function changes video mode and aspect ratio of given @a output.
     * Output has to be already initialized.
     *
     * @param[in] output    video output ID
     * @param[in,out] config on input: desired video mode and aspect ratio,
     *                      on output: actual video mode and aspect ratio
     * @return              @c 0 on success, @c -1 in case of error
     **/
    int (*setOutputVideoMode)(unsigned int output, QBVideoOutputConfig *config);

     /**
     * Set aspect ratio of a video output.
     *
     * This function changes aspect ratio of given @a output.
     * Output has to be already initialized.
     *
     * @param[in] output    video output ID
     * @param[in,out] config on input: desired aspect ratio,
     *                      on output: actual aspect ratio
     * @return              @c 0 on success, @c -1 in case of error
     **/
    int (*setOutputAspectRatio)(unsigned int output, QBVideoOutputConfig *config);

    /**
    * Set protection of a video output.
    *
    * This function changes protection of given @a output.
    * Output has to be already initialized.
    *
    * @param[in] output    video output ID
    * @param[in,out] config on input: desired protection,
    *                      on output: actual protection
    * @return              @c 0 on success, @c -1 in case of error
    **/
    int (*setOutputProtection)(unsigned int output, QBVideoOutputConfig *config);

    /**
     * Enable video output.
     *
     * This function configures video output so that video signal
     * appears on the physical output port. Output has to be
     * already initialized.
     *
     * @param[in] output    video output ID
     * @return              @c 0 on success, @c -1 in case of error
     **/
    int (*enableOutput)(unsigned int output);

    /**
     * Disable video output.
     *
     * This function configures video output so that video signal
     * is not emitted on the physical output port.
     *
     * @param[in] output    video output ID
     * @return              @c 0 on success, @c -1 in case of error
     **/
    int (*disableOutput)(unsigned int output);

    /**
     * Check if given @a mode is supported by video output.
     *
     * @param[in] output  video output ID, starting from @c 0
     * @param[in] mode      video mode to check
     * @param[out] isSupported @c true if @a mode is supported, @c false otherwise
     *                      (valid only when function succeeds)
     * @return              @c 0 on success, @c -1 in case of error
     **/
    int (*isModeSupported)(unsigned int output, QBOutputStandard mode, bool *isSupported);

    /**
     * Get last reset reason.
     *
     * @return              @c reason of last system reset
     **/
    QBPlatformResetReason (*getResetReason)(void);

    /**
     * Radio Frequency Output (RFOutput) Methods
     **/
    struct {
        /**
         * Get list of channel numbers supported by RF output.
         *
         * @param[in] output    video output ID of the RF output
         * @param[out] channels array filled with channel numbers
         * @param[in] maxChannels number of elements in @a channels array
         * @return              number of supported channels (can be higher
         *                      than @a maxChannels), @c -1 in case of error
         **/
        ssize_t (*getSupportedChannels)(unsigned int output,
                                        unsigned int *channels,
                                        size_t maxChannels);

        /**
         * Set channel number used by RF output.
         *
         * @param[in] output    video output ID of the RF output
         * @param[in] channel   channel number
         * @return              @c 0 on success, @c -1 in case of error
         **/
        int (*setChannelNumber)(unsigned int output,
                                unsigned int channel);
        /**
         * Get list of channel numbers supported by RF output.
         *
         * @param[in] output    video output ID of the RF output
         * @param[out] current  channel number
         * @return              @c 0 on success, @c -1 in case of error
         **/
         int  (*getChannelNumber)(unsigned int output,
                                  unsigned int *channel);
        /**
         * Check if setChannelNumber function can be used to set RF modulator output channel.
         *
         * @param[in]  output                video output ID of the RF output
         * @param[out] setChannelAllowed     true if RF channel QBPlatformRFOutputSetChannelNumber can be used
         * @return                           @c 0 on success, @c -1 in case of error
         **/
         int (*setChannelNumberIsAllowed)(unsigned int output, bool * setChannelAllowed);
    } RFOutputMethods; ///< groups all rf output related methods

    /**
     * Audio Output Methods
     **/
    struct {
        /**
         * Mute or unmute audio output.
         *
         * @param[in] output    audio output ID
         * @param[in] mute      @c true to mute, @c false to unmute
         * @return              @c 0 on success, @c -1 in case of error
         **/
        int (*muteAudioOutput)(unsigned int output, bool mute);

        /**
         * Set audio output volume.
         *
         * @param[in] output    audio output ID
         * @param[in] volume    request audio output volume
         * @return              @c 0 on success, @c -1 in case of error
         **/
        int (*setAudioOutputVolume)(unsigned int output, unsigned int volume);
    } audioOutputMethods; ///< groups all audio output related methods

    struct {
        /**
         * Get size of available memory.
         *
         * @return              @c size in bytes
         */
        size_t (*getAvailableMemory)(void);
    } memoryMethods;  ///< groups all memory related methods

    /**
     * Print debugging statistics.
     *
     * @param[in] reason   short message to prepend to outputted information
     *                     to describe the reason for printing statistics
     **/
    void (*debug)(const char* reason);

    /**
     * CEC Methods, Consumer Electronics Control is a part of the HDMI interface specification
     **/
    struct {
        /**
         * Send CEC message over output.
         *
         * @param[in] outputID  video output ID
         * @param[in] msg       CEC message to send
         * @return              @c 0 on success, @c SV_ERR_AGAIN if cannot send because there are pending messages,
         *                      -1 in case of other error
         **/
        int (*sendMessage)(unsigned int outputID, const QBCECMessage msg);

        /**
         * Set new CEC device physical address.
         *
         * @param[in] outputID        video output ID
         * @param[in] physicalAddress new physical address (16-bit host-endian)
         * @return              @c 0 on success, @c -1 in case of error
         **/
        int (*setPhysicalAddress)(unsigned int outputID, uint16_t physicalAddress);

        /**
         * Enable or disable CEC.
         *
         * @param[in] outputID    video output ID
         * @param[in] enable      @c true to enable, @c false to disable
         * @return              @c 0 on success, @c -1 in case of error
         **/
        int (*enable)(unsigned int output, bool enable);
    } cecMethods;

    /**
     * VBI Teletext methods
     **/
    struct {
        /**
         * Enable or disable teletext through VBI on output outputID.
         * @param[in] outputID  video output ID
         * @param[in] enabled   true iff teletext should be enabled on outputID
         * @return              @c 0 on success, @c -1 in case of error
         **/
        int (*setupTeletext)(unsigned outputID, bool enabled);

        /**
         * Request displaying teletext through VBI. This function is non blocking
         * but may consume only a part of provided lines.
         * @param[in] outputID  video output ID
         * @param[in] lines     an array of teletext lines to be displayed
         * @param[in] linesCnt  the sie of lines array
         * @param[out] processedLinesCnt if function returns success, then this is the count
         *                          of lines from lines array that were accepted. Non accepted lines
         *                          have to be resent later.
         * @return              @c 0 on success, @c -1 in case of error
         **/
        int (*showTeletextLines)(unsigned outputID, QBPlatformVBITeletextLine* lines, int linesCnt, int *processedLinesCnt);
    } vbiTeletextMethods;

    /**
     * One Time Programing methods
     **/
    struct {
        /**
         * Check if the underlaying platform has OTP fuse map
         * @return @c true if the underlaying platform has OTP fuse map (and thus QBPlatformOTP API is functional),
         * @c false otherwise
         **/
        bool (*hasFuseMap)(void);

        /**
         * Burn OTP security fuses.
         * @return @c 0 on success, @c -1 in case of error
         **/
        int (*setSecurityFuses)(void);

        /**
         * Check status of OTP security fuses.
         * @return      @c 0 if OTP flash area is unlocked,
         *              @c 1 if it is locked,
         *              @c -1 in case of an error occurred
         **/
        int (*isSecured)(void);
    } otpMethods;
} QBPlatformImpl;


/**
 * Get descriptor of the platform HAL implementation.
 *
 * @return descriptor of the platform HAL implementation
 **/
extern QBPlatformImpl *
QBPlatformGetImplementation(void);

/**
 * Lock platform HAL implementation descriptor for reading.
 *
 * @param[in] platformImpl descriptor of the platform HAL implementation
 **/
static inline void
QBPlatformRDLock(QBPlatformImpl *platformImpl)
{
    pthread_rwlock_rdlock(&(platformImpl->lock));
}

/**
 * Lock platform HAL implementation descriptor for writing.
 *
 * @param[in] platformImpl descriptor of the platform HAL implementation
 **/
static inline void
QBPlatformWRLock(QBPlatformImpl *platformImpl)
{
    pthread_rwlock_wrlock(&(platformImpl->lock));
}

/**
 * Unlock platform HAL implementation descriptor.
 *
 * @param[in] platformImpl descriptor of the platform HAL implementation
 **/
static inline void
QBPlatformUnlock(QBPlatformImpl *platformImpl)
{
    pthread_rwlock_unlock(&(platformImpl->lock));
}

/**
 * Update value of the HDCP status flag in video output configuration.
 *
 * @param[in] outputID  video output ID of the HDMI output
 * @param[in] HDCPStatus status of HDCP protection
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformSetHDCPStatus(unsigned int outputID,
                        QBHDCPProtectionStatus HDCPStatus);

/**
 * Update value of the HDCP status flag in video output configuration
 * with optional QBHDCPErrorCode.
 *
 * @param[in] outputID  video output ID of the HDMI output
 * @param[in] HDCPStatus status of HDCP protection
 * @param[in] errorCode  optional error code
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformSetHDCPStatusWithErrorCode(unsigned int outputID,
                                     QBHDCPProtectionStatus HDCPStatus,
                                     QBHDCPErrorCode errorCode);

/**
 * Get HDCP status.
 *
 * @param[in] outputID     video output ID of the HDMI output
 * @return HDCPAuthStatus  HDCP authorization status
 **/
extern HDCPAuthStatus
QBPlatformGetHDCPStatus(unsigned int outputID);

/**
 * Get HDCP error code.
 *
 * @param[in] outputID     video output ID of the HDMI output
 * @return QBHDCPErrorCode HDCP error code
 **/
extern QBHDCPErrorCode
QBPlatformGetHDCPErrorCode(unsigned int outputID);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
