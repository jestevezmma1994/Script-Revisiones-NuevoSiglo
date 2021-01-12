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

#ifndef SVPLAYERAUDIOTRACK_H
#define SVPLAYERAUDIOTRACK_H

/**
 * @file SvPlayerAudioTrack.h Audio track representation
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <dataformat/sv_data_format.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup SvPlayerAudioTrack SvPlayerAudioTrack class
 * @{
 * Audio track representation
 */

/**
 * SvPlayerAudioTrack class.
 * @class SvPlayerAudioTrack
 * @extends SvPlayerTrack
 */
typedef struct SvPlayerAudioTrack_* SvPlayerAudioTrack;

/**
 * Get track audio type.
 *
 * @param[in] self SvPlayerAudioTrack object
 * @return audio type
 */
QBAudioType
SvPlayerAudioTrackGetAudioType(SvPlayerAudioTrack self);

/**
 * Get track description.
 *
 * @memberof SvPlayerAudioTrack
 *
 * @param[in] self SvPlayerAudioTrack object
 * @return track description
 */
SvString
SvPlayerAudioTrackGetDescription(SvPlayerAudioTrack self);

/**
 * Get track language.
 *
 * @memberof SvPlayerAudioTrack
 *
 * @param[in] self SvPlayerAudioTrack object
 * @return track language
 */
SvString
SvPlayerAudioTrackGetLang(SvPlayerAudioTrack self);

/**
 * Get audio track codec.
 *
 * @param[in] self SvPlayerAudioTrack object
 * @return audio codec
 */
QBAudioCodec
SvPlayerAudioTrackGetAudioCodec(SvPlayerAudioTrack self);

/**
 * Check if track is multichannel .
 *
 * @param[in] self SvPlayerAudioTrack object
 * @return true if multi-channel track, otherwise false
 */
bool
SvPlayerAudioTrackIsMultiChannel(SvPlayerAudioTrack self);

/**
 * Get runtime type identification object representing SvPlayerAudioTrack class.
 *
 * @relates SvPlayerAudioTrack
 *
 * @return SvPlayerAudioTrack runtime type identification object
 **/
SvType
SvPlayerAudioTrack_getType(void);

/**
 * Create new Audio Track.
 *
 * @memberof SvPlayerAudioTrack
 *
 * @param[in] id unique ID of audio track
 * @param[in] audioFormat simple audio format
 * @param[out] errorOut error information
 * @return created SvPlayerAudioTrack object, or @c NULL in case of error
 */
SvPlayerAudioTrack
SvPlayerAudioTrackCreateFromFormat(unsigned int id, const struct sv_simple_audio_format *audioFormat, SvErrorInfo *errorOut);

/**
 * Create new Audio Track.
 *
 * @param[in] id unique ID of audio track
 * @param[in] audioType type of audio
 * @param[in] codec audio codec
 * @param[in] isMultiChannel true if track is multi-channel
 * @param[in] lang track language
 * @param[in] desc track description
 * @param[out] errorOut error information
 * @return created SvPlayerAudioTrack object, or @c NULL in case of error
 */
SvPlayerAudioTrack SvPlayerAudioTrackCreate(unsigned int id,
                                            QBAudioType audioType,
                                            QBAudioCodec codec,
                                            bool isMultiChannel,
                                            SvString lang,
                                            SvString desc,
                                            SvErrorInfo *errorOut);

/**
 * Register JSON serializer for audio track.
 */
void
SvPlayerAudioTrackRegisterJSONSerializers(void);

/**
 * @}
 */

#endif // SVPLAYERAUDIOTRACK_H
