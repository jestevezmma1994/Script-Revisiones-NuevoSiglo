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


#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

/**
 * @file playlistManager.h Playlist manager class API
 **/

#include <SvFoundation/SvString.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPGDataLayer/Views/SvEPGView.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>


/**
 * Playlist manager class.
 *
 * @class QBPlaylistManager
 * @extends SvObject
 * @implements QBAsyncService
 * @implements QBInitializable
 * @implements QBConfigListener
 *
 * @note This class is intended to be used in cooperation with QBServiceRegistry,
 *       use "QBPlaylistManager" key to get it from service registry.
 *
 * @see QBServiceRegistryGetService
 */
typedef struct QBPlaylistManager_t* QBPlaylistManager;

typedef SvObject (*QBPlaylistManagerLoadFunc)(void *self_, SvString id, SvString name, SvString type, SvString filename);

/**
 * Types of notifications
 **/
typedef enum QBPlaylistChangeType_ {
    QBPlaylist_CategoryAdded,      ///< new category have been added
    QBPlaylist_CategoryRemoved,    ///< category have been removed
    QBPlaylist_CategoryRenamed     ///< category name have been changed
} QBPlaylistChangeType;

/**
 * QBPlaylistChangeParams class.
 *
 * Parameter passed to observers when service status have been changed.
 **/
typedef struct QBPlaylistChangeParams_ {
    struct SvObject_ super_;

    QBPlaylistChangeType type;    /// type of action
    SvObject channelList;         /// channel list handle
    SvString channelListID;       /// channel list id
    SvString channelListName;     /// channel list name
    size_t position;              /// channel list position
} *QBPlaylistChangeParams;

/**
 * Creates new instance of service notification params.
 *
 * @param [in] type         type of notification
 * @param [in] ID           channel list id
 * @param [in] name         channel list name
 * @param [in] channelList  channel list handle
 * @param [in] position     channel list position
 * @return                  Handle to QBPlaylistChangeParams
 */
QBPlaylistChangeParams
QBPlaylistChangeParamsCreate(QBPlaylistChangeType type, SvString ID, SvString name, SvObject channelList, const size_t position);


/**
 * Create QBPlaylistManager service.
 *
 * @param[in] confDir           directory path for QBPlaylistManager files
 * @return                      created volume service instance, @c NULL if failed
 **/
QBPlaylistManager QBPlaylistManagerCreate(SvString confDir);

/**
 * @relates QBPlaylistManager
 *
 * Get runtime type identification object representing playlist manager class.
 *
 * @return QBPlaylistManager type identification object
 */
SvType QBPlaylistManager_getType(void);

/**
 * Add playlist.
 *
 * @param[in] self                      QBPlaylistManager handle
 * @param[in] playlist                  playlist
 * @param[in] id                        playlist id
 * @param[in] name                      playlist name
 * @param[in] type                      playlist type
 * @param[in] registerToEPGManager      @c true if playlist should be registered to EPG manager as listener
 **/
void QBPlaylistManagerAdd(QBPlaylistManager self,
                          SvObject playlist,
                          SvString id,
                          SvString name,
                          SvString type,
                          bool registerToEPGManager);

/**
 * Removes playlist
 *
 * @param [in] self         QBPlaylistManager handle
 * @param [in] id           playlist id
 * @return playlist @a id on success, @c NULL if playlist was not registered in manager
 */
SvString QBPlaylistManagerRemove(QBPlaylistManager self, SvString id);

void QBPlaylistManagerRename(QBPlaylistManager self, SvString oldName, SvString newName);

SvObject QBPlaylistManagerCreatePlaylist(QBPlaylistManager self, SvString name, SvString type);

/**
 * Set sort id for playlist with a given id. Type of playlist have to be 'CATEGORY'.
 *
 * @param [in] self         QBPlaylistManager handle
 * @param [in] id           playlist id
 * @param [in] sortID       new sorting ID
 * @param [out] errorOut    error info
 */
void QBPlaylistManagerSetPlaylistSortId(QBPlaylistManager self, SvString id, int sortID, SvErrorInfo *errorOut);

void QBPlaylistManagerSetListNumberingById(QBPlaylistManager self, SvString id, SvEPGViewChannelNumbering numbering);

void QBPlaylistManagerSetCurrent(QBPlaylistManager self, SvString id);

SvObject QBPlaylistManagerGetCurrent(QBPlaylistManager self);

int QBPlaylistManagerGetNumberOfChannel(QBPlaylistManager self, SvTVChannel channel);

int QBPlaylistManagerGetNumberOfChannelInPlaylist(QBPlaylistManager self, SvObject playlist, SvTVChannel channel);

/**
 * Gets number of channels in given playlist
 *
 * @param[in] self      playlist manager
 * @param[in] playlist  playlist to get channels count of
 *
 * @return count of channels in playlist, -1 on error
 */
int QBPlaylistManagerGetChannelsCount(QBPlaylistManager self, SvObject playlist);

/**
 * Gets QBPlayList by name from Favorite playlists.
 *
 * @param[in] self playlist manager
 * @param[in] name name of playlist to get
 *
 * @return list in case of success, NULL otherwise
 */
SvObject QBPlaylistManagerGet(QBPlaylistManager self, SvString name);

SvObject QBPlaylistManagerGetById(QBPlaylistManager self, SvString id);

SvArray QBPlaylistManagerGetPlaylistNames(QBPlaylistManager self, SvString type);

/**
 * Checks if playlist with given name and different id exists in the same group (e.g. in Favorites)
 *
 * @details It can be used for checking if playlist with @a existingPlaylistId id can be renamed to @a name
 *
 * @param[in] self                  playlist manager handle
 * @param[in] name                  name to check if used (can't be @c NULL)
 * @param[in] existingPlaylistId    id of existing playlist (can't be @c NULL)
 * @param[out] errorOut             error info
 *
 * @return @c true if exists, @c false otherwise
 */
bool QBPlaylistManagerSimilarPlaylistExists(QBPlaylistManager self, SvString name, SvString existingPlaylistId, SvErrorInfo *errorOut);

SvArray QBPlaylistManagerGetPlaylistIds(QBPlaylistManager self, SvString type);

SvArray QBPlaylistManagerGetPlaylists(QBPlaylistManager self, SvString type);

void QBPlaylistManagerSave(QBPlaylistManager self);

void QBPlaylistManagerLoad(QBPlaylistManager self);

SvString QBPlaylistManagerGetCurrentName(QBPlaylistManager self);

SvString QBPlaylistManagerGetCurrentId(QBPlaylistManager self);

SvString QBPlaylistManagerGetPlaylistName(QBPlaylistManager self, SvObject list_);

SvString QBPlaylistManagerGetPlaylistId(QBPlaylistManager self, SvObject list_);

SvString QBPlaylistManagerGetCurrentType(QBPlaylistManager self);

SvString QBPlaylistManagerGetPlaylistType(QBPlaylistManager self, SvObject playlist);

int QBPlaylistManagerGetGlobalNumberOfChannel(QBPlaylistManager playlists, SvTVChannel channel);

void QBPlaylistManagerAddGlobalPlaylist(QBPlaylistManager self, SvString id);

void QBPlaylistManagerSetLoadFunc(QBPlaylistManager self, QBPlaylistManagerLoadFunc load, void *loadData);

int QBPlaylistManagerGetListCountForType(QBPlaylistManager self, SvString type);

void QBPlaylistManagerSortPlaylistIds(QBPlaylistManager self, SvArray ids);

void QBPlaylistManagerSortPlaylists(QBPlaylistManager self, SvArray playlists);

/**
 * Load playlists from configuration file.
 *
 * @param[in] self                       QBPlaylistManager handle
 * @param[in] path                       path to configuration file
 * @param[in] playlistManagerInterpreter configuration file interpreter
 * @param[out] errorOut                  error info
 **/
void QBPlaylistManagerLoadStaticPlaylists(QBPlaylistManager self, SvString path, SvObject playlistManagerInterpreter, SvErrorInfo *errorOut);

SvObject QBPlaylistManagerGetPlaylistMetaData(QBPlaylistManager self, SvObject playlist, SvObject key);

void QBPlaylistManagerSetPlaylistMetaData(QBPlaylistManager self, SvObject playlist, SvObject key, SvObject value);

#endif
