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

#ifndef QB_PLATFORM_EDID_H_
#define QB_PLATFORM_EDID_H_

/**
 * @file QBPlatformEDID.h Data structure describing EDID information
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <dataformat/audio.h>
#include <QBPlatformHAL/QBPlatformTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformEDID EDID data handling
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Audio samples size.
 **/
enum {
    /// 16 bit audio samples
    QBEDIDAudioSampleSize_16 = (1 << 0),
    /// 20 bit audio samples
    QBEDIDAudioSampleSize_20 = (1 << 1),
    /// 24 bit audio samples
    QBEDIDAudioSampleSize_24 = (1 << 2)
};

/**
 * Audio sampling rate.
 **/
enum {
    /// 32 kHz sampling rate
    QBEDIDAudioSampleRate_32kHz = (1 << 0),
    /// 44 kHz sampling rate
    QBEDIDAudioSampleRate_44kHz = (1 << 1),
    /// 48 kHz sampling rate
    QBEDIDAudioSampleRate_48kHz = (1 << 2),
    /// 88 kHz sampling rate
    QBEDIDAudioSampleRate_88kHz = (1 << 3),
    /// 96 kHz sampling rate
    QBEDIDAudioSampleRate_96kHz = (1 << 4),
    /// 176 kHz sampling rate
    QBEDIDAudioSampleRate_176kHz = (1 << 5),
    /// 196 kHz sampling rate
    QBEDIDAudioSampleRate_192kHz = (1 << 6)
};

/**
 * Audio format description.
 **/
typedef struct {
    /// audio codec used
    QBAudioCodec codec;
    /// number of audio channels
    unsigned int numberOfChannels;
    /// mask of supported sample sizes, valid only for LPCM codec
    unsigned int bitsPerSampleMask;
    /// mask of supported sampling rates
    unsigned int samplingRatesMask;
    /// max bitrate of the audio stream, invalid for LPCM codec
    unsigned int maxBitRate;
} QBEDIDAudioFormat;

/**
 * Video format description.
 **/
typedef struct {
    /// video mode, ::QBOutputStandard_none for unknown modes
    QBOutputStandard mode;
    /// raw mode identifier as defined by CEA/EIA-861E
    uint32_t rawModeID;
    /// @c true if this video mode is the native mode of the display device
    bool isNative;
} QBEDIDVideoFormat;

/**
 * EDID information block describing display device.
 **/
typedef struct QBEDIDBlock_ {
    /// EDID structure version (major version number)
    uint8_t version;
    /// EDID structure revision (minor version number)
    uint8_t revision;

    // general information about display device
    /// 3-letter device manufacturer code
    char manufacturer[4];
    /// device model identifier (manufacturer specific)
    uint16_t modelID;
    /// date of production
    struct {
        /// week number, @c 1 to @c 53
        uint16_t week;
        /// year of production
        uint16_t year;
    } manufactureDate;

    // basic display parameters
    /// @c 1 for digital display device, @c 0 for analog one
    uint8_t isDigital;
    /**
     * physical image dimensions,
     * undefined in any of the values is @c 0 (for example: projector)
     *
     * @note Some manufacturers don't provide actual dimensions in EDID
     * block, just any values that keep correct aspect ratio of the
     * image, for example: QBEDIDBlock_::width == 16 and QBEDIDBlock_::height == 9.
     **/
    struct {
        /// max image width in centimetres
        uint8_t width;
        /// max image height in centimetres
        uint8_t height;
    } maxImageSize;
    /// display gamma factor
    uint8_t gamma;

    /// number of supported video formats
    uint32_t videoFormatsCnt;
    /// an array of supported video formats
    QBEDIDVideoFormat *videoFormats;

    /// number of supported audio formats
    uint32_t audioFormatsCnt;
    /// an array of supported audio formats
    QBEDIDAudioFormat *audioFormats;
} QBEDIDBlock;


/**
 * Allocate EDID block with space for specified number
 * of video and audio formats.
 *
 * @param[in] videoFormatsCnt number of video formats to reserve space for
 * @param[in] audioFormatsCnt number of audio formats to reserve space for
 * @return              new allocated EDID block, @c NULL in case of error
 **/
extern QBEDIDBlock *
QBPlatformHDMIAllocEDIDBlock(unsigned int videoFormatsCnt,
                             unsigned int audioFormatsCnt);

/**
 * Copy EDID block.
 *
 * @param[in] EDID      source EDID block
 * @return              copy of the @a EDID block, @c NULL in case of error
 **/
extern QBEDIDBlock *
QBPlatformHDMICopyEDIDBlock(const QBEDIDBlock *EDID);

/**
 * Check if given video mode is supported.
 *
 * @param[in] EDID      EDID block
 * @param[in] mode      video mode
 * @return              @c true if @a mode is supported, @c false otherwise
 **/
extern bool
QBPlatformEDIDIsVideoModeSupported(const QBEDIDBlock *EDID,
                                   QBOutputStandard mode);

/**
 * Check if given audio format is supported.
 *
 * @param[in] EDID      EDID block
 * @param[in] codec     audio codec
 * @param[in] multiChannel @c true if multi-channel (at least 5.1) is required
 * @return              @c true if format is supported, @c false otherwise
 **/
extern bool
QBPlatformEDIDIsAudioFormatSupported(const QBEDIDBlock *EDID,
                                     QBAudioCodec codec,
                                     bool multiChannel);

/**
 * Remove repeating entries in QBEDIDBlock::videoFormats and QBEDIDBlock::audioFormats.
 *
 * @param[in] EDID      EDID block
 **/
extern void
QBPlatformEDIDMakeUnique(QBEDIDBlock *EDID);

/**
 * Print information from EDID block to the @ref SvLog.
 *
 * @param[in] EDID      EDID block
 * @param[in] prefix    prefix of each printed line
 **/
extern void
QBPlatformEDIDPrint(const QBEDIDBlock *EDID, const char *prefix);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
