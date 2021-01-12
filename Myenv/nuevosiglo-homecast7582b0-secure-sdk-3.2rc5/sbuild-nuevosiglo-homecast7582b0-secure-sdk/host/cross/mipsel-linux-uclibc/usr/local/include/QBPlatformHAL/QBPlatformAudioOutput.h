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

#ifndef QB_PLATFORM_AUDIO_OUTPUT_H_
#define QB_PLATFORM_AUDIO_OUTPUT_H_

/**
 * @file QBPlatformAudioOutput.h Audio outputs control API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <dataformat/audio.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBPlatformHAL/QBPlatformEvent.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformAudioOutput Audio outputs control
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Get number of available audio outputs.
 *
 * @return                  number of audio outputs, @c -1 in case of error
 **/
extern int
QBPlatformGetAudioOutputsCount(void);

/**
 * Find audio output by type and/or name.
 *
 * @param[in] type         requested audio output type, pass QBAudioOutputType_unknown
 *                         to search only by @a name
 * @param[in] name         requested audio output name, pass @c NULL to search
 *                         only by @a type
 * @return                 ID of the matching output, @c -1 if not found
 **/
extern int
QBPlatformFindAudioOutput(QBAudioOutputType type,
                          const char *name);

/**
 * Get current configuration of the audio output.
 *
 * This function returns current configuration of one of audio outputs.
 * Audio outputs are identified by consecutive numbers, starting with @c 0.
 *
 * @param[in] outputID  audio output ID, starting from @c 0
 * @param[out] cfg      current configuration of audio output identified by
 *                      @a outputID
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformGetAudioOutputConfig(unsigned int outputID,
                               QBAudioOutputConfig *cfg);

/**
 * Mute or unmute audio output.
 *
 * @param[in] outputID  audio output ID, starting from @c 0
 * @param[in] mute      @c true to mute, @c false to unmute
 * @param[in] priority  mute priority to prevent (when muted)
 *                      unmuting from other priority levels
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformMuteAudioOutput(unsigned int outputID,
                          bool mute,
                          QBPlatformOutputPriority priority);

/**
 * Change volume on audio output.
 *
 * @param[in] outputID  audio output ID, starting from @c 0
 * @param[in] volume    new requested volume level, must fit in output-specific
 *                      range (see ::QBAudioOutputConfig)
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformSetAudioOutputVolume(unsigned int outputID,
                               unsigned int volume);

/**
 * Change latency on audio output.
 *
 * @param[in] outputID  audio output ID, starting from @c 0
 * @param[in] latency   new requested audio latency in milliseconds
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformSetAudioOutputLatency(unsigned int outputID,
                                unsigned int latency);

/**
 * Get output format settings for specified audio output(s).
 *
 * This function returns output format settings for specified audio
 * outputs. If there is more output formats defined than @a maxFormats,
 * returned value will reflect the real number of available output
 * formats. If @a codec is specified (is not QBAudioCodec::QBAudioCodec_unknown),
 * function guarantees that there will be output format returned
 * for each output.
 *
 * @note Returned output formats are always ordered by output ID.
 *
 * @param[in] outputID      audio output ID, pass @c -1 to get preferences
 *                          for all audio outputs
 * @param[in] codec         codec of the source audio stream, QBAudioCodec::QBAudioCodec_unknown
 *                          to get preferences for all codecs
 * @param[in] multiChannel  @c true if the source audio stream is multi channel,
 *                          @c false for mono/stereo
 * @param[out] outputFormats table of output formats for specified audio
 *                          output(s) and/or input codec, ordered by output ID
 * @param[in] maxFormats    length of @a outputFormats array
 * @return                  number of output format settings available for
 *                          given parameters (can be >@a formatsCnt),
 *                          @c -1 in case of error
 **/
extern int
QBPlatformGetAudioOutputPreferences(int outputID,
                                    QBAudioCodec codec,
                                    bool multiChannel,
                                    QBAudioOutputFormat *outputFormats,
                                    unsigned int maxFormats);

/**
 * Set output format preferences for specified audio output.
 *
 * @param[in] outputID      audio output ID
 * @param[in] outputFormats table of output formats for different input codecs
 * @param[in] formatsCnt    length of @a outputFormats array
 * @return                  @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformSetAudioOutputPreferences(unsigned int outputID,
                                    const QBAudioOutputFormat *outputFormats,
                                    unsigned int formatsCnt);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
