/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SVPLAYERSUBSTRACK_TS_H
#define SVPLAYERSUBSTRACK_TS_H

/**
 * @file SvPlayerSubsTrackTS.h Subtitles track representation for subtitles being encapsulated in TS
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvPlayerControllers/SvPlayerSubsTrack.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <dataformat/sv_data_format.h>
#include <stdbool.h>

/**
 * @defgroup SvPlayerSubsTrack SvPlayerSubsTrackTS class
 * @{
 * Subtitles track representation for subtitles types being encapsulated in TS
 */

/// Subtitles standards supported by @c SvPlayerSubsTrackTS class
typedef enum SvPlayerSubsTrackTStype_ {
    SvPlayerSubsTrackTStype_unknown,
    SvPlayerSubsTrackTStype_dvb,      ///< DVB subtitles, as described by sv_subtitle_service structure, an equivalent type to svsubtitlestype_dvb
    SvPlayerSubsTrackTStype_ttx,      ///< teletext subtitles, as described by sv_teletext_service structure, an equivalent type to svsubtitlestype_ttx
    SvPlayerSubsTrackTStype_scte27,   ///< SCT-27 subtitles, as described by sv_scte27_stream structure, an equivalent type to svsubtitlestype_scte27
    SvPlayerSubsTrackTStype_cc,       ///< Closed Captions, as described by sv_closed_caption_subs_service structure, an equivalent type to svsubtitlestype_cc
    SvPlayerSubsTrackTStype_count
} SvPlayerSubsTrackTStype;

/**
 * SvPlayerSubsTrackTS class.
 * @class SvPlayerSubsTrackTS
 * @extends SvPlayerSubsTrack
 */
typedef struct SvPlayerSubsTrackTS_ *SvPlayerSubsTrackTS;

/**
 * Get type of track.
 *
 * @memberof SvPlayerSubsTrackTS
 *
 * @param[in] self SvPlayerSubsTrackTS object
 * @return type of track
 */
SvPlayerSubsTrackTStype
SvPlayerSubsTrackTSGetTypeOfTrack(SvPlayerSubsTrackTS self);

/**
 * Get track language.
 *
 * This function is just a wrapper around GetLang() function from base class provided here for your convenience
 *
 * @memberof SvPlayerSubsTrackTS
 *
 * @param[in] self SvPlayerSubsTrackTS object
 * @return track language
 */
static inline SvString
SvPlayerSubsTrackTSGetLang(SvPlayerSubsTrackTS self)
{
    return SvPlayerSubsTrackGetLang((SvPlayerSubsTrack) self);
}

/**
 * Set track language.
 * This function is just a wrapper around SetLang() function from base class provided here for your convenience
 *
 * @memberof SvPlayerSubsTrackTS
 *
 * @param[in] self SvPlayerSubsTrackTS object
 * @param[in] lang track language to be set
 */
static inline void
SvPlayerSubsTrackTSSetLang(SvPlayerSubsTrackTS self, SvString lang)
{
    SvPlayerSubsTrackSetLang((SvPlayerSubsTrack) self, lang);
}

/**
 * Get PID on which the track is transmitted.
 *
 * @memberof SvPlayerSubsTrackTS
 *
 * @param[in] self SvPlayerSubsTrackTS object
 * @return PID of track
 */
int
SvPlayerSubsTrackTSGetPID(SvPlayerSubsTrackTS self);

/**
 * Get subtitle track type.
 *
 * @memberof SvPlayerSubsTrackTS
 *
 * @param[in] self SvPlayerSubsTrackTS object
 * @return type of track
 */
QBSubtitleType
SvPlayerSubsTrackTSGetSubtitleType(SvPlayerSubsTrackTS self);

/**
 * Check if this track is the only service being transmitted on its PID
 *
 * @memberof SvPlayerSubsTrackTS
 *
 * @param[in] self SvPlayerSubsTrackTS object
 * @return Exclusive PID flag
 */
bool
SvPlayerSubsTrackTSIsPIDexclusive(SvPlayerSubsTrackTS self);

/**
 * Get runtime type identification object representing SvPlayerSubsTrackTS class.
 *
 * @relates SvPlayerSubsTrackTS
 *
 * @return SvPlayerSubsTrackTS runtime type identification object
 **/
SvType
SvPlayerSubsTrackTS_getType(void);

/**
 * Create new TS Subtitles Track.
 *
 * @memberof SvPlayerSubsTrackTS
 *
 * @param[in] subs pointer to structure describing subtitles service; its exact meaning is defined by @c subsType
 * @param[in] subsType depicts exact type of parameter passed by @c subs
 * @param[in] pid PID in TS which carries this subtitles track
 * @param[in] isExclusive is this track the only service being transmitted on its PID
 * @param[out] errorOut error information
 * @return created SvPlayerSubsTrack object, or @c NULL in case of error
 */
SvPlayerSubsTrackTS
SvPlayerSubsTrackTSCreate(const void *subs, enum svsubtitlestype subsType, int pid, bool isExclusive, SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* SVPLAYERSUBSTRACK_TS_H */
