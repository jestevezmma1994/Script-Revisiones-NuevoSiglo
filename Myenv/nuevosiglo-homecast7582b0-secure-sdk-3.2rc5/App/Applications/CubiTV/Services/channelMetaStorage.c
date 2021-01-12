/*****************************************************************************
** TiVo Poland Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2019 TiVo Poland Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by TiVo Poland Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** TiVo Poland Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include "channelMetaStorage.h"
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvHashTable.h>
#include <SvJSON/SvJSONParse.h>
#include <SvEPGDataLayer/SvChannelMerger.h>
#include <SvEPGDataLayer/SvEPGExtendedChannelListListener.h>
#include <Services/core/JSONserializer.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <safeFiles.h>
#include <main.h>

struct QBChannelMetaStorage_
{
    struct SvObject_ super_;

    /// key -> (SvValue) channelID, value -> (SvHashTable) keys
    /// keys: key -> (SvString) attributeName, value -> (SvObject) attributeValue
    SvHashTable channelidToKeys;
    /// key -> (SvValue) pluginID, value -> (SvHashTable) keys
    /// keys: key -> (SvString) atrributeName, value -> (SvString) atrributeName
    SvHashTable pluginToKeys;
    /// key -> (SvTVChannel) channel, value -> (SvTVChannel) channel
    SvHashTable channels;

    bool isStarted;
    SvObject channelMerger;

    SvString filename;
};

SvLocal void QBChannelMetaStorage__dtor__(void* self_)
{
    QBChannelMetaStorage* self = self_;
    SVRELEASE(self->channelidToKeys);
    SVRELEASE(self->filename);
    SVRELEASE(self->pluginToKeys);
    SVRELEASE(self->channels);
    SVRELEASE(self->channelMerger);
}

SvLocal void QBChannelMetaStoragePruneAndSave(QBChannelMetaStorage* self)
{
    SvHashTable keysToSerialize = SvHashTableCreate(101, NULL);

    SvIterator channelIt = SvHashTableKeysIterator(self->channels);
    SvTVChannel channel;
    while ((channel = (SvTVChannel) SvIteratorGetNext(&channelIt))) {
        SvValue channelID = SvTVChannelGetID(channel);
        SvHashTable keys = (SvHashTable) SvHashTableFind(self->channelidToKeys, (SvObject) channelID);
        if (!keys)
            continue;

        SVAUTOINTVALUE(pluginIDValue, channel->pluginID);
        SvHashTable writableKeys = (SvHashTable) SvHashTableFind(self->pluginToKeys, (SvObject) pluginIDValue);
        if (!writableKeys) {
            SvHashTableRemove(self->channelidToKeys, (SvObject) channelID);
            continue;
        }

        SvIterator keysIt = SvHashTableKeysIterator(keys);
        SvString key;
        while ((key = (SvString) SvIteratorGetNext(&keysIt))) {
            if (!SvHashTableFind(writableKeys, (SvObject) key))
                SvHashTableRemove(keys, (SvObject) key);
        }
        if (SvHashTableCount(keys))
            SvHashTableInsert(keysToSerialize, (SvObject) SvValueGetString(channelID), (SvObject) keys);
        else
            SvHashTableRemove(self->channelidToKeys, (SvObject) channelID);
    }

    QBJSONSerializer JSONserializer = (QBJSONSerializer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBJSONSerializer"));
    QBJSONSerializerAddJob(JSONserializer, (SvObject) keysToSerialize, self->filename);
    SVRELEASE(keysToSerialize);
}

SvLocal void QBChannelMetaStorageChannelLost(SvObject self_, SvTVChannel channel)
{
    QBChannelMetaStorage* self = (QBChannelMetaStorage*) self_;
    SvHashTableRemove(self->channels, (SvObject) channel);

    SvValue id = SvTVChannelGetID(channel);
    if (SvHashTableFind(self->channelidToKeys, (SvObject) id)) {
        SvHashTableRemove(self->channelidToKeys, (SvObject) id);
        QBChannelMetaStoragePruneAndSave(self);
    }
}

SvLocal void QBChannelMetaStorageChannelListCompleted(SvObject self_, int pluginID)
{
}

SvLocal bool QBChannelMetaStorageGetWritableKeysIteratorForPlugin(QBChannelMetaStorage* self, int pluginID, SvIterator* writableKeysIt)
{
    SVAUTOINTVALUE(pluginIDValue, pluginID);
    SvHashTable writableKeys = (SvHashTable) SvHashTableFind(self->pluginToKeys, (SvObject) pluginIDValue);
    if (writableKeys) {
        *writableKeysIt = SvHashTableGetKeysIterator(writableKeys);
        return true;
    }
    return false;
}

SvLocal void QBChannelMetaStorageChannelModified(SvObject self_, SvTVChannel channel)
{
    QBChannelMetaStorage* self = (QBChannelMetaStorage*) self_;

    SvHashTableInsert(self->channels, (SvObject) channel, (SvObject) channel);

    SvIterator writableKeysIt;
    if (!QBChannelMetaStorageGetWritableKeysIteratorForPlugin(self, channel->pluginID, &writableKeysIt)) {
        return;
    }

    bool didChange = false;
    SvValue channelID = SvTVChannelGetID(channel);
    SvHashTable keys = (SvHashTable) SvHashTableFind(self->channelidToKeys, (SvObject) channelID);
    SvString writableKey;
    while ((writableKey = (SvString) SvIteratorGetNext(&writableKeysIt))) {
        SvObject oldVal = keys ? SvHashTableFind(keys, (SvObject) writableKey) : NULL;
        SvObject attrVal = SvTVChannelGetAttribute(channel, writableKey);

        if (SvObjectEquals(attrVal, oldVal))
            continue;

        didChange = true;
        if (oldVal && keys)
            SvHashTableRemove(keys, (SvObject) writableKey);

        if (attrVal) {
            if (!keys) {
                keys = SvHashTableCreate(23, NULL);
                SvHashTableInsert(self->channelidToKeys, (SvObject) channelID, (SvObject) keys);
                SVRELEASE(keys);
            }
            SvHashTableInsert(keys, (SvObject) writableKey, attrVal);
        }
    }

    if (didChange)
        QBChannelMetaStoragePruneAndSave(self);
}

SvLocal void QBChannelMetaStorageChannelReplaced(SvObject self_, SvTVChannel oldChannel, SvTVChannel newChannel)
{
    QBChannelMetaStorageChannelModified(self_, newChannel);
}

SvLocal SvType QBChannelMetaStorage_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBChannelMetaStorage__dtor__
    };
    static SvType type = NULL;
    if (!type) {
        static struct SvEPGExtendedChannelListListener_ mergerListenerMethods = {
            .super_                   = {
                .channelFound         = QBChannelMetaStorageChannelModified,
                .channelModified      = QBChannelMetaStorageChannelModified,
                .channelLost          = QBChannelMetaStorageChannelLost,
                .channelListCompleted = QBChannelMetaStorageChannelListCompleted
            },
            .channelReplaced          = QBChannelMetaStorageChannelReplaced
        };
        SvTypeCreateManaged("QBChannelMetaStorage",
                            sizeof(struct QBChannelMetaStorage_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvEPGExtendedChannelListListener_getInterface(), &mergerListenerMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBChannelMetaStorageLoad(QBChannelMetaStorage* self)
{
    SvData metaStorageData = SvDataCreateFromFile(SvStringCString(self->filename), 4096, NULL);

    if (!metaStorageData)
        return;

    char* buffer = SvDataGetBytes(metaStorageData);
    SvHashTable channelIdToKeys = NULL;

    if (!buffer)
        goto fini;

    channelIdToKeys = (SvHashTable) SvJSONParseData(buffer, SvDataGetLength(metaStorageData), true, NULL);
    if (!channelIdToKeys || !SvObjectIsInstanceOf((SvObject) channelIdToKeys, SvHashTable_getType()))
        goto fini;

    SvIterator it = SvHashTableKeysIterator(channelIdToKeys);
    SvString id;
    while ((id = (SvString) SvIteratorGetNext(&it))) {
        if (!SvObjectIsInstanceOf((SvObject) id, SvString_getType()))
            continue;
        SvHashTable keyVals = (SvHashTable) SvHashTableFind(channelIdToKeys, (SvObject) id);
        if (!SvObjectIsInstanceOf((SvObject) keyVals, SvHashTable_getType()))
            continue;
        SvHashTable keys = SvHashTableCreate(23, NULL);
        SvValue idVal = SvValueCreateWithString(id, NULL);
        SvHashTableInsert(self->channelidToKeys, (SvObject) idVal, (SvObject) keys);
        SVRELEASE(idVal);

        SvIterator it2 = SvHashTableKeysIterator(keyVals);
        SvString keyVal;
        while ((keyVal = (SvString) SvIteratorGetNext(&it2))) {
            if (!SvObjectIsInstanceOf((SvObject) keyVal, SvString_getType()))
                continue;
            SvHashTableInsert(keys, (SvObject) keyVal, SvHashTableFind(keyVals, (SvObject) keyVal));
        }
        SVRELEASE(keys);
    }

fini:
    SVTESTRELEASE(metaStorageData);
    SVTESTRELEASE(channelIdToKeys);
}

QBChannelMetaStorage* QBChannelMetaStorageCreate(SvObject merger, SvString filename)
{
    QBChannelMetaStorage* self = (QBChannelMetaStorage*) SvTypeAllocateInstance(QBChannelMetaStorage_getType(), NULL);
    self->channelMerger = SVRETAIN(merger);
    self->filename = SVRETAIN(filename);

    self->channels = SvHashTableCreate(199, NULL);
    self->channelidToKeys = SvHashTableCreate(199, NULL);
    self->pluginToKeys = SvHashTableCreate(5, NULL);

    self->isStarted = false;

    return self;
}

void QBChannelMetaStorageAddKeyForPlugin(QBChannelMetaStorage* self, SvString key, int pluginID)
{
    SvValue pluginIDValue = SvValueCreateWithInteger(pluginID, NULL);
    SvHashTable keys = (SvHashTable) SvHashTableFind(self->pluginToKeys, (SvObject) pluginIDValue);
    if (!keys) {
        keys = SvHashTableCreate(5, NULL);
        SvHashTableInsert(self->pluginToKeys, (SvObject) pluginIDValue, (SvObject) keys);
        SVRELEASE(keys);
    }
    SVRELEASE(pluginIDValue);

    if (!SvHashTableFind(keys, (SvObject) key)) {
        SvHashTableInsert(keys, (SvObject) key, (SvObject) key);
    }
}

void QBChannelMetaStorageRestoreChannel(QBChannelMetaStorage* self, SvTVChannel channel)
{
    SvValue id = SvTVChannelGetID(channel);
    SvHashTable channelKeys = (SvHashTable) SvHashTableFind(self->channelidToKeys, (SvObject) id);

    SvIterator writableKeysIt;
    if (!QBChannelMetaStorageGetWritableKeysIteratorForPlugin(self, channel->pluginID, &writableKeysIt)) {
        return;
    }

    SvString writableKey;
    while ((writableKey = (SvString) SvIteratorGetNext(&writableKeysIt))) {
        SvObject value = channelKeys ? SvHashTableFind(channelKeys, (SvObject) writableKey) : NULL;
        if (value)
            SvTVChannelSetAttribute(channel, writableKey, value);
    }
}

void QBChannelMetaStorageStart(QBChannelMetaStorage* self)
{
    QBChannelMetaStorageLoad(self);

    SvInvokeInterface(SvChannelMerger, self->channelMerger, addListener, (SvObject) self, NULL);

    self->isStarted = true;
}

void QBChannelMetaStorageForgetEverything(QBChannelMetaStorage* self)
{
    SvIterator channelIt = SvHashTableKeysIterator(self->channels);
    SvTVChannel channel;
    while ((channel = (SvTVChannel) SvIteratorGetNext(&channelIt))) {
        bool changed = false;

        SvIterator writableKeysIt;
        if (!QBChannelMetaStorageGetWritableKeysIteratorForPlugin(self, channel->pluginID, &writableKeysIt)) {
            continue;
        }

        SvString writableKey;
        while ((writableKey = (SvString) SvIteratorGetNext(&writableKeysIt))) {
            if (SvTVChannelGetAttribute(channel, writableKey)) {
                SvTVChannelRemoveAttribute(channel, writableKey);
                changed = true;
            }
        }

        if (changed) {
            SvInvokeInterface(SvChannelMerger, self->channelMerger, propagateChannelChange, channel, NULL);

            //we will receive channel changed and save the changes, so there is no need to save right now
        }
    }
}
