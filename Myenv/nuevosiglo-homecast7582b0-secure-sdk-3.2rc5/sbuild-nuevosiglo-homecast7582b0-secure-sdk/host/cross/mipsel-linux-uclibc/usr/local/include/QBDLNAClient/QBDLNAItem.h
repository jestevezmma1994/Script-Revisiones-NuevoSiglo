/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

/* SMP/QBDLNAClient/QBDLNAItem.h */

#ifndef QBDLNACLIENT_ITEM_INTERNAL_H_
#define QBDLNACLIENT_ITEM_INTERNAL_H_

/**
 * @file QBDLNAItem.h
 * @brief DLNA item (category or product) structures and interface
 **/

/**
 * @defgroup QBDLNAItem DLNA item (category or product) structures and interface
 * @ingroup QBDLNAClient
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

/**
 * Type of DLNA item - casual container
 */
#define QB_UPNP_CONTAINER                 "object.container"
/**
 * Type of DLNA item - music album container
 */
#define QB_UPNP_CONTAINER_MUSIC_ALBUM     "object.container.album.musicAlbum"
/**
 * Type of DLNA item - video album container
 */
#define QB_UPNP_CONTAINER_VIDEO_ALBUM     "object.container.album.videoAlbum"
/**
 * Type of DLNA item - photo album container
 */
#define QB_UPNP_CONTAINER_PHOTO_ALBUM     "object.container.album.photoAlbum"
/**
 * Type of DLNA item - storage folder
 */
#define QB_UPNP_CONTAINER_STORAGE_FOLDER  "object.container.storageFolder"
/**
 * Type of DLNA item - storage volume
 */
#define QB_UPNP_CONTAINER_STORAGE_VOLUME  "object.container.storageVolume"
/**
 * Type of DLNA item - storage system
 */
#define QB_UPNP_CONTAINER_STORAGE_SYSTEM  "object.container.storageSystem"
/**
 * Type of DLNA item - playlist container
 */
#define QB_UPNP_CONTAINER_PLAYLIST        "object.container.playlistContainer"
/**
 * Type of DLNA item - music genre container
 */
#define QB_UPNP_CONTAINER_MUSICGENRE      "object.container.genre.musicGenre"
/**
 * Type of DLNA item - movie genre container
 */
#define QB_UPNP_CONTAINER_MOVIEGENRE      "object.container.genre.movieGenre"
/**
 * Type of DLNA item - music artist container
 */
#define QB_UPNP_CONTAINER_MUSICARTIST     "object.container.person.musicArtist"
/**
 * Type of DLNA item - movie actor container
 */
#define QB_UPNP_CONTAINER_MOVIEACTOR      "object.container.person.movieActor"
/**
 * Type of DLNA item - image
 */
#define QB_UPNP_ITEM_IMAGE                "object.item.imageItem"
/**
 * Type of DLNA item - photo
 */
#define QB_UPNP_ITEM_PHOTO                "object.item.imageItem.photo"
/**
 * Type of DLNA item - music track
 */
#define QB_UPNP_ITEM_MISICTRACK           "object.item.audioItem.musicTrack"
/**
 * Type of DLNA item - video
 */
#define QB_UPNP_ITEM_VIDEO                "object.item.videoItem"
/**
 * Type of DLNA item - movie
 */
#define QB_UPNP_ITEM_MOVIE                "object.item.videoItem.movie"
/**
 * Type of DLNA item - music video clip
 */
#define QB_UPNP_ITEM_MOVIECLIP            "object.item.videoItem.musicVideoClip"
/**
 * Type of DLNA item - playlist
 */
#define QB_UPNP_ITEM_PLAYLIST             "object.item.playlistItem"
/**
 * Type of DLNA item - video broadcast
 */
#define QB_UPNP_ITEM_VIDEOBROADCAST       "object.item.videoItem.videoBroadcast"
/**
 * Type of DLNA item - audio broadcast
 */
#define QB_UPNP_ITEM_AUDIOBROADCAST       "object.item.audioItem.audioBroadcast"


/**
 * Mapped DLNA item class
 */
typedef enum {
  QB_DLNA_ITEM_CLASS_AUDIO,
  QB_DLNA_ITEM_CLASS_VIDEO,
  QB_DLNA_ITEM_CLASS_PLAYLIST,
  QB_DLNA_ITEM_CLASS_PHOTO,
  QB_DLNA_ITEM_CLASS_VIDEOBROADCAST,
  QB_DLNA_ITEM_CLASS_AUDIOBROADCAST,
  QB_DLNA_ITEM_CLASS_NOENTRIES,
  QB_DLNA_ITEM_CLASS_UNBROWSABLE,
  QB_DLNA_ITEM_CLASS_NORESPONSE,
  QB_DLNA_ITEM_CLASS_ACCESS_DENIED,
  QB_DLNA_ITEM_CLASS_A_CNT,
  QB_DLNA_ITEM_CLASS_V_CNT,
  QB_DLNA_ITEM_CLASS_P_CNT,
  QB_DLNA_ITEM_CLASS_L_CNT,
  QB_DLNA_ITEM_CLASS_FOLDER,
  QB_DLNA_ITEM_CLASS_UNKNOWN,
} QBDLNAItemClass;

/**
 * DLNA item class
 */
struct QBDLNAItem_t {
    struct SvObject_ super_;
    SvString                id;
    SvString                pid;
    SvString                title;
    SvString                creator;
    SvString                contentURL;
    SvString                thumbnailURL;
    SvString                genre;
    SvString                album;
    SvString                date;
    int                     childCount;
    QBDLNAItemClass         dlnaClass;
};

typedef struct QBDLNAItem_t *QBDLNAItem;

/**
 * Get runtime type identification object representing
 * type of DLNA item class.
 *
 * @return DLNA item class
 **/
SvType QBDLNAItem_getType(void);

/**
 * Create DLNA info item
 *
 * @param[in] id            item's identifier
 * @param[in] dlnaClass     item's class
 * @param[out] errorOut     error info
 * @return                  @a self or @c NULL in case of error
 **/
QBDLNAItem QBDLNAItemCreateInfoItem(SvString id,
                                    QBDLNAItemClass dlnaClass,
                                    SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif
