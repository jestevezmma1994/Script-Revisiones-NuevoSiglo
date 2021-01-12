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

#ifndef QBM3U8MEDIAPLAYLIST_H_
#define QBM3U8MEDIAPLAYLIST_H_

/**
 * @file QBM3U8MediaPlaylist.h
 * @brief HLS Media Playlist API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/HLS/M3U8/QBM3U8MediaSegment.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvURI.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup QBM3U8MediaPlaylist HLS Media Playlist class
 * @ingroup QBM3U8Parser
 * @{
 **/

/**
 * Media Playlist class.
 * @class QBM3U8MediaPlaylist.
 **/
typedef struct QBM3U8MediaPlaylist_ *QBM3U8MediaPlaylist;

/**
 * Get runtime type identification object representing
 * type of Media Playlist class.
 *
 * @return Media Playlist class
 **/
extern SvType
QBM3U8MediaPlaylist_getType(void);

/**
 * Get maximum Media Segment duration.
 *
 * @param[in] self - media playlist to get from
 * @return         - target duration
 */
extern uint64_t
QBM3U8MediaPlaylistGetTargetDuration(QBM3U8MediaPlaylist self);

/**
 * Get Media Sequence Number of the first media segment that appears in a Playlist file.
 *
 * @param[in] self - media playlist to get from
 * @return         - Media Sequence Number
 */
extern uint64_t
QBM3U8MediaPlaylistGetMediaSequence(QBM3U8MediaPlaylist self);

/**
 * Get Discontinuity Media Sequence Number (used to synchronize between different
 * Renditions of the same Variant Stream or different Variant Streams).
 *
 * @param[in] self - media playlist to get from
 * @return         - Discontinuity Media Sequence Number
 */
extern uint64_t
QBM3U8MediaPlaylistGetMediaDiscontinuitySequence(QBM3U8MediaPlaylist self);

/**
 * Indicates if no more Media Sements wil be added to the Media Playlist file.
 *
 * @param[in] self     - media playlist to get from
 * @return    @c true  - media segments can be added
 *            @c false - media segments cannot be added
 */
extern bool
QBM3U8MediaPlaylistHasEndlist(QBM3U8MediaPlaylist self);

/**
 * Media Playlist type mutability information about Media Playlist file.
 */
typedef enum {
    QBM3U8MediaPlaylistType_undefined, ///< The Playlist Type was not specified, that means that
                                       ///< playlist can be updated eg. live playlist MAY be updated
                                       ///< to remove Media Segments in order that they appeared.
    QBM3U8MediaPlaylistType_event,     ///< Media Segments can only be added to the end of the Media Playlist.
    QBM3U8MediaPlaylistType_vod        ///< Media Playlist cannot change.
} QBM3U8MediaPlaylistType;

/**
 * Get mutability information about Media Playlist file.
 *
 * @param[in] self - media playlist to get from
 * @return         - QBM3U8MediaPlaylistType
 */
extern QBM3U8MediaPlaylistType
QBM3U8MediaPlaylistGetPlaylistType(QBM3U8MediaPlaylist self);

/**
 * Get info if the Media Playlist contains only IFrames.
 *
 * @param[in] self     - media playlist to get from
 * @return    @c true  - IFrames only media playlist
 *            @c false - normal media playlist
 */
extern bool
QBM3U8MediaPlaylistIsIFramesOnly(QBM3U8MediaPlaylist self);

/**
 * Get Media Segment at given index.
 *
 * @param[in] self  - media playlist to get from
 * @param[in] index - index of Media Segment to get
 * @return          - media segment at given index
 */
extern QBM3U8MediaSegment
QBM3U8MediaPlaylistGetMediaSegment(QBM3U8MediaPlaylist self, size_t index);

/**
 * @}
 **/

#endif // QBM3U8MEDIAPLAYLIST_H_
