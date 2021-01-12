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

#ifndef QBM3U8MASTERPLAYLIST_H_
#define QBM3U8MASTERPLAYLIST_H_

/**
 * @file QBM3U8MasterPlaylist.h
 * @brief HLS Master Playlist API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/HLS/M3U8/QBM3U8MediaPlaylist.h>
#include <QBIPStreamClient/HLS/M3U8/QBM3U8VariantStream.h>
#include <QBIPStreamClient/HLS/M3U8/QBM3U8RenditionGroup.h>

/**
 * @defgroup QBM3U8MasterPlaylist HLS Master Playlist class
 * @ingroup QBM3U8Parser
 * @{
 **/

/**
 * Master Playlist class.
 * @class QBM3U8MasterPlaylist.
 **/
typedef struct QBM3U8MasterPlaylist_ *QBM3U8MasterPlaylist;

/**
 * Get runtime type identification object representing
 * type of Master Playlist class.
 *
 * @return Master Playlist class
 **/
extern SvType
QBM3U8MasterPlaylist_getType(void);

/**
 * Get total number of Variant Streams of a Master Playlist.
 *
 * @param[in] self master playlist to get from
 * @return         number of Variant Streams
 **/
extern size_t
QBM3U8MasterPlaylistGetVariantStreamsCount(QBM3U8MasterPlaylist self);

/**
 * Get Variant Stream at specified index of specified Master Playlist
 *
 * @param[in] self               Master Plalist to get from
 * @param[in] variantStreamIndex index of Variant Stream
 * @return                       specific Variant Stream
 **/
extern QBM3U8VariantStream
QBM3U8MasterPlaylistGetVariantStream(QBM3U8MasterPlaylist self, size_t variantStreamIndex);

/**
 * Get total number of Rendition Groups of a Master Playlist.
 *
 * @param[in] self Master Playlist to get from.
 * @return         number of Rendition Groups
 **/
extern size_t
QBM3U8MasterPlaylistGetRenditionGroupsCount(QBM3U8MasterPlaylist self);

/**
 * Get Rendition Group at specified index of specified Master Playlist
 *
 * @param[in] self                Master Plalist to get from
 * @param[in] renditionGroupIndex index of Rendition Group
 * @return                        specific Rendition Group
 **/
extern QBM3U8RenditionGroup
QBM3U8MasterPlaylistGetRenditionGroup(QBM3U8MasterPlaylist self, size_t renditionGroupIndex);

/**
 * @}
 **/

#endif // QBM3U8MASTERPLAYLIST_H_
