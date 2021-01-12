/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CUBITV_COMMON_UTILS_H_
#define CUBITV_COMMON_UTILS_H_

/**
 * @file commonUtils.h Common utilities functions eq. value conversions.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvString.h>
#include <SvCore/SvCoreErrorDomain.h>

/**
 * Media file types
 **/
typedef enum {
    /** Unknown type */
    QBMediaFileType_unknown,
    /** A video file */
    QBMediaFileType_video,
    /** An audio file */
    QBMediaFileType_audio,
    /** A playlist file */
    QBMediaFileType_audioPlaylist,
    /** An IP stream */
    QBMediaFileType_IPstream
} QBMediaFileType;

/**
 * Get media-file type from a URI string.
 *
 * @param[in]   uri         The string to be interpreted.
 * @param[out]  errorOut    Error info.
 *
 * @retval      QBMediaFileType_audio
 *                          if URI ends with ".mp3"
 * @retval      QBMediaFileType_audioPlaylist
 *                          if URI ends with ".m3u"
 * @retval      QBMediaFileType_video
 *                          if URI ends with ".m3u8" or any value not mentioned
 *                          previously.
 */
extern QBMediaFileType QBAppKitURIToMediaFileType(const SvString uri, SvErrorInfo *errorOut);

#endif // CUBITV_COMMON_UTILS_H_
