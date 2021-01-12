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

#ifndef SVPLAYERSUBSTRACK_ES_H
#define SVPLAYERSUBSTRACK_ES_H

/**
 * @file SvPlayerSubsTrackES.h Subtitles track representation for subtitles being encapsulated in ES
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvPlayerControllers/SvPlayerSubsTrack.h>
#include <dataformat/sv_data_format.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup SvPlayerSubsTrack SvPlayerSubsTrackES class
 * @{
 * Subtitles track representation for subtitles types being encapsulated in ES
 */

/**
 * SvPlayerSubsTrackES class.
 * @class SvPlayerSubsTrackES
 * @extends SvPlayerSubsTrack
 */
typedef struct SvPlayerSubsTrackES_ *SvPlayerSubsTrackES;

/**
 * Get track description.
 *
 * @memberof SvPlayerSubsTrack
 *
 * @param[in] self SvPlayerSubsTrack object
 * @return track description
 */
SvString
SvPlayerSubsTrackESGetDescription(SvPlayerSubsTrackES self);

/**
 * Get track language.
 * This function is just a wrapper around GetLang() function from base class provided here for your convenience
 *
 * @memberof SvPlayerSubsTrack
 *
 * @param[in] self SvPlayerSubsTrack object
 * @return track language
 */
static inline SvString
SvPlayerSubsTrackESGetLang(SvPlayerSubsTrackES self)
{
    return SvPlayerSubsTrackGetLang((SvPlayerSubsTrack) self);
}

/**
 * Set track language.
 * This function is just a wrapper around SetLang() function from base class provided here for your convenience
 *
 * @memberof SvPlayerSubsTrackES
 *
 * @param[in] self SvPlayerSubsTrackES object
 * @param[in] lang track language to be set
 */
static inline void
SvPlayerSubsTrackESSetLang(SvPlayerSubsTrackES self, SvString lang)
{
    SvPlayerSubsTrackSetLang((SvPlayerSubsTrack) self, lang);
}

/**
 * Get runtime type identification object representing SvPlayerSubsTrackES class.
 *
 * @relates SvPlayerSubsTrack
 *
 * @return SvPlayerSubsTrackES runtime type identification object
 **/
SvType
SvPlayerSubsTrackES_getType(void);

/**
 * Create new ES Subtitles Track.
 *
 * @memberof SvPlayerSubsTrackES
 *
 * @param[in] subs pointer to structure describing subtitles service
 * @param[out] errorOut error information
 * @return created SvPlayerSubsTrackES object, or @c NULL in case of error
 */
SvPlayerSubsTrackES
SvPlayerSubsTrackESCreate(const struct sv_txt_subs_stream *subs, SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* SVPLAYERSUBSTRACK_ES_H */
