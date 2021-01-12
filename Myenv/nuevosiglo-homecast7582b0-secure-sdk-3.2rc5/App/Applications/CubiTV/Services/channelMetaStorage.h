/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CHANNELMETASTORAGE_H_
#define CHANNELMETASTORAGE_H_

#include <main_decl.h>
#include <SvFoundation/SvString.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>

/**
 * @file channelMetaStorage.h Channel Meta Storage class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * QBChannelMetaStorage class.
 * @class QBChannelMetaStorage
 **/
typedef struct QBChannelMetaStorage_ QBChannelMetaStorage;

/**
 * Create Channel Meta Storage
 *
 * @param[in] merger        channel merger handle
 * @param[in] filename      file to meta data storage
 * @return                  new QBChannelMetaStorage object
 */
QBChannelMetaStorage* QBChannelMetaStorageCreate(SvObject merger, SvString filename);

/**
 * Add a writable to file metakey for given plugin ID
 *
 * @param[in] self          QBChannelMetaStorage handle
 * @param[in] key           key name
 * @param[in] pluginID      channel pluginID
 */
void QBChannelMetaStorageAddKeyForPlugin(QBChannelMetaStorage* self, SvString key, int pluginID);

/**
 * Restore writable meta keys to given channel
 *
 * @param[in] self          QBChannelMetaStorage handle
 * @param[in] channel       channel object
 */
void QBChannelMetaStorageRestoreChannel(QBChannelMetaStorage* self, SvTVChannel channel);

/**
 * Start of Channel Meta Storage
 *
 * @param[in] self          QBChannelMetaStorage handle
 */
void QBChannelMetaStorageStart(QBChannelMetaStorage* self);

/**
 * Remove all writable meta keys from all channels
 *
 * @param[in] self          QBChannelMetaStorage handle
 */
void QBChannelMetaStorageForgetEverything(QBChannelMetaStorage* self);

#endif
