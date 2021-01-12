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

#ifndef QBM3U8VARIANTSTREAM_H_
#define QBM3U8VARIANTSTREAM_H_

/**
 * @file QBM3U8VariantStream.h
 * @brief HLS Variant Stream API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
   Excerpt from the standard:
   A more complex presentation can be described by a Master Playlist.  A Master
   Playlist provides a set of Variant Streams, each of which describes a
   different version of the same content.

   A Variant Stream includes a Media Playlist that specifies media encoded at a
   particular bit rate, in a particular format, and at a particular resolution
   for media containing video.

   A Variant Stream can also specify a set of Renditions.  Renditions are
   alternate versions of the content, such as audio produced in different
   languages or video recorded from different camera angles.

   Clients should switch between different Variant Streams to adapt to network
   conditions.  Clients should choose Renditions based on user preferences.
 */

#include <QBIPStreamClient/HLS/M3U8/QBM3U8RenditionGroup.h>
#include "SvFoundation/SvType.h"
#include "SvFoundation/SvURI.h"
#include <stdint.h>

/**
 * @defgroup QBM3U8VariantStream HLS Variant Stream class
 * @ingroup QBM3U8Parser
 * @{
 **/

/**
 * Variant class.
 * @class QBM3U8VariantStream
 **/
typedef struct QBM3U8VariantStream_ *QBM3U8VariantStream;

/**
 * Get runtime type identification object representing
 * type of QBM3U8VariantStream class.
 *
 * @return Variant Stream class
 **/
extern SvType
QBM3U8VariantStream_getType(void);

/**
 * Get URI of specific Variant Stream
 *
 * @param[in] self Variant Stream to get from
 * @return         URI of specified Variant Stream
 **/
extern SvURI
QBM3U8VariantStreamGetURI(QBM3U8VariantStream self);

/**
 * Get Bandwidth of specific Variant Stream
 *
 * @param[in] self Variant Stream to get from
 * @return         Bandwidth of specified Variant Stream.
 **/
extern uint64_t
QBM3U8VariantStreamGetBandwidth(QBM3U8VariantStream self);

/**
 * Get Audio Rendition Group associated with specific Variant Steam
 *
 * @param[in] self Variant Stream to get from
 * @return         Audio Rendition Group associated with specified Variant Stream.
 **/
extern QBM3U8RenditionGroup
QBM3U8VariantStreamGetAudioRenditionGroup(QBM3U8VariantStream self);

/**
 * Get Audio Rendition Group associated with specific Variant Steam
 *
 * @param[in] self Variant Stream to get from
 * @return         Audio Rendition Group associated with specified Variant Stream.
 **/
extern QBM3U8RenditionGroup
QBM3U8VariantStreamGetVideoRenditionGroup(QBM3U8VariantStream self);

/**
 * Get Subtitles Rendition Group associated with specific Variant Stream
 *
 * @param[in] self Variant Stream to get from
 * @return         Subtitles Rendition Group associated with specific Variant Stream.
 **/
extern QBM3U8RenditionGroup
QBM3U8VariantStreamGetSubtitlesRenditionGroup(QBM3U8VariantStream self);

/**
 * Get Closed-Captions Rendition Group associated with specific Variant Stream
 *
 * @param[in] self Variant Stream to get from
 * @return         Closed-Captions Rendition Group associated with specific Variant Stream.
 **/
extern QBM3U8RenditionGroup
QBM3U8VariantStreamGetClosedCaptionsRenditionGroup(QBM3U8VariantStream self);

/**
 * @}
 **/

#endif // QBM3U8VARIANTSTREAM_H_
