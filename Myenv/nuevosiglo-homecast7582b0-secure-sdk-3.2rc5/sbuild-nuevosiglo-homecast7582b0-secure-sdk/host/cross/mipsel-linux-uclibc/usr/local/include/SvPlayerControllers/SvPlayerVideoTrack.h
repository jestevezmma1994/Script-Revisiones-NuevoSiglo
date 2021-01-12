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

#ifndef SVPLAYERVIDEOTRACK_H
#define SVPLAYERVIDEOTRACK_H

/**
 * @file SvPlayerAudioTrack.h Video track representation
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <dataformat/sv_data_format.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup SvPlayerVideoTrack SvPlayerVideoTrack class
 * @{
 * Video track representation
 */

/**
 * SvPlayerVideoTrack class.
 * @class SvPlayerVideoTrack
 * @extends SvPlayerTrack
 */
typedef struct SvPlayerVideoTrack_* SvPlayerVideoTrack;

/**
 * Get AVC level.
 *
 * @memberof SvPlayerVideoTrack
 *
 * @param[in] self SvPlayerVideoTrack object
 * @return AVC level
 */
int
SvPlayerVideoTrackGetAVCLevel(SvPlayerVideoTrack self);

/**
 * Get track description.
 *
 * @memberof SvPlayerVideoTrack
 *
 * @param[in] self SvPlayerVideoTrack object
 * @return track description
 */
SvString
SvPlayerVideoTrackGetDescription(SvPlayerVideoTrack self);

/**
 *
 * @param self
 * @return
 */
SvString
SvPlayerVideoTrackGetCodec(SvPlayerVideoTrack self);

/**
 * Get runtime type identification object representing SvPlayerVideoTrack class.
 *
 * @relates SvPlayerVideoTrack
 *
 * @return SvPlayerVideoTrack runtime type identification object
 **/
SvType
SvPlayerVideoTrack_getType(void);

/**
 * Create new Video Track from format.
 *
 * @memberof SvPlayerVideoTrack
 *
 * @param[in] videoFormat simple video format
 * @param[out] errorOut error information
 * @return created SvPlayerSubsTrack object, or @c NULL in case of error
 */
SvPlayerVideoTrack
SvPlayerVideoTrackCreateFromFormat(const struct sv_simple_video_format* videoFormat, SvErrorInfo *errorOut);

/**
 * Create new Video Track from format.
 *
 * @memberof SvPlayerVideoTrack
 *
 * @param[in] codec video codec
 * @param[in] desc track description
 * @param[in] avcLevel AVC level
 * @param[out] errorOut error information
 * @return created SvPlayerSubsTrack object, or @c NULL in case of error
 */
SvPlayerVideoTrack
SvPlayerVideoTrackCreate(SvString codec, SvString desc, int avcLevel, SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* SVPLAYERVIDEOTRACK_H */
