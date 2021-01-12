/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBChannelListFilter.h"

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Services/core/JSONserializer.h>
#include <SvJSON/SvJSONParse.h>
#include <SvCore/SvCommonDefs.h>
#include <safeFiles.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>


#include <SvEPGDataLayer/SvEPGChannelListListener.h>
#include <SvEPGDataLayer/SvChannelMergerFilter.h>
#include <SvEPGDataLayer/SvChannelMerger.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 3, "QBChannelListFilterLogLevel", "")
#define log_debug(fmt, ...)     do { if (env_log_level() >= 4) SvLogNotice(COLBEG() "%s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
#define log_info(fmt, ...)      do { if (env_log_level() >= 3) SvLogNotice(COLBEG() "%s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
#define log_warning(fmt, ...)   do { if (env_log_level() >= 2) SvLogWarning(COLBEG() "%s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
#define log_error(fmt, ...)     do { if (env_log_level() >= 1) SvLogError(COLBEG()  "%s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)

struct QBChannelListFilter_ {
    struct SvObject_ super_;

    /// preferred plugin id
    unsigned int preferredPluginID;
    /// channels created by preffered plugin
    SvHashTable channels;
    /// flag indicatind if service have been started
    bool started;
    /// flag indicatind if QBChannelListFilter have been started
    bool isEnabled;
    /// JSON serializer service handle
    QBJSONSerializer JSONserializer;
    /// path to serialized channel list
    SvString pathToFile;
};

SvLocal void QBChannelListFilterChannelFound(SvObject self_, SvTVChannel tvChannel)
{
    QBChannelListFilter self = (QBChannelListFilter) self_;

    if (!self || !tvChannel) {
        log_error("null argument passed, self: %p, tvChannel: %p", self, tvChannel);
        return;
    }

    if (SvTVChannelGetPluginID(tvChannel) == self->preferredPluginID) {
        SvValue channelID = SvTVChannelGetID(tvChannel);
        SvString channelName = SvTVChannelGetName(tvChannel);
        if (channelID && SvValueIsString(channelID)) {
            SvString channelIDStr = SvValueGetString(channelID);
            SvHashTableInsert(self->channels, (SvObject) channelIDStr, (SvObject) channelName);
        } else {
            log_error("channel without ID");
        }
    }
}

SvLocal void QBChannelListFilterChannelLost(SvObject self_, SvTVChannel tvChannel)
{
    QBChannelListFilter self = (QBChannelListFilter) self_;

    if (!self || !tvChannel) {
        log_error("null argument passed, self: %p, tvChannel: %p", self, tvChannel);
        return;
    }

    if (SvTVChannelGetPluginID(tvChannel) == self->preferredPluginID) {
        SvValue channelID = SvTVChannelGetID(tvChannel);
        if (SvValueIsString(channelID)) {
            SvString channelIDStr = SvValueGetString(channelID);
            SvHashTableRemove(self->channels, (SvObject) channelIDStr);
        }
    }
}

SvLocal void QBChannelListFilterDestroy(void *self_)
{
    QBChannelListFilter self = (QBChannelListFilter) self_;
    SVRELEASE(self->channels);
    SVTESTRELEASE(self->JSONserializer);
    SVTESTRELEASE(self->pathToFile);
}

SvLocal void
QBChannelListFilterSave(QBChannelListFilter self)
{
    assert(self);
    log_debug("writing channels list to file: %s", SvStringCString(self->pathToFile));

    log_debug("channels count: %zu", SvHashTableCount(self->channels));
    if (!QBJSONSerializerAddJob(self->JSONserializer, (SvObject) self->channels, self->pathToFile))
        log_error("error while scheduling serialization of channels list (file: '%s')", SvStringCString(self->pathToFile));
}

SvLocal void
QBChannelListFilterLoad(QBChannelListFilter self)
{
    assert(self);

    SvHashTable channels = (SvHashTable) SvJSONParseFile(SvStringGetCString(self->pathToFile), false, NULL);
    if (!channels || !SvObjectIsInstanceOf((SvObject) channels, SvHashTable_getType())) {
        log_error("incorrect channels list: %p", channels);
        SVTESTRELEASE(channels);
        return;
    }

    SVRELEASE(self->channels);
    self->channels = channels;
    log_info("Loaded %zu channels from file.", SvHashTableCount(self->channels));

    return;
}

SvLocal bool QBChannelListFilterUpdateState(SvObject self_, unsigned int pluginID)
{
    log_debug("");
    QBChannelListFilter self = (QBChannelListFilter) self_;

    assert(self);

    if (!self->started) {
        log_warning("Updating state of disabled filter.");
        return false;
    }

    if (self->preferredPluginID != pluginID)
        return self->isEnabled;

    if (SvHashTableGetCount(self->channels) > 0) {
        self->isEnabled = true;
    } else {
        self->isEnabled = false;
    }

    QBChannelListFilterSave(self);

    return self->isEnabled;
}

void QBChannelListFilterStart(QBChannelListFilter self, unsigned int pluginID, SvString pathToFile)
{
    log_debug("");
    assert(self);

    if (self->started) {
        log_warning("plugin already started, ignoring preferred plugin ID: %u", pluginID);
        return;
    }

    if (self->pathToFile) {
        log_error("null pathToFile passed");
        return;
    }

    QBJSONSerializer JSONserializer = (QBJSONSerializer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBJSONSerializer"));
    if (!JSONserializer) {
        log_error("NULL JSONserializer!");
        return;
    }

    self->JSONserializer = SVRETAIN(JSONserializer);
    self->pathToFile = SVRETAIN(pathToFile);

    QBChannelListFilterLoad(self);

    self->preferredPluginID = pluginID;
    self->started = true;

    if (SvHashTableCount(self->channels) > 0) {
        log_debug("update state...");
        QBChannelListFilterUpdateState((SvObject) self, self->preferredPluginID);
    }
}

void QBChannelListFilterStop(QBChannelListFilter self)
{
    log_debug("");
    assert(self);

    if (!self->started)
        return;

    self->started = false;
}

SvLocal bool QBChannelListFilterIsEnabled(SvObject self_)
{
    QBChannelListFilter self = (QBChannelListFilter) self_;
    return self->isEnabled;
}

SvLocal bool QBChannelListFilterIsChannelAllowed(SvObject self_, SvValue channelID)
{
    QBChannelListFilter self = (QBChannelListFilter) self_;

    if (!self) {
        log_error("null self passed");
        return true;
    }

    if (!self->started || !self->isEnabled)
        return true;

    if (SvValueIsString(channelID)) {
        SvString channelIDStr = SvValueGetString(channelID);
        if (SvHashTableFind(self->channels, (SvObject) channelIDStr))
            return true;
    }

    return false;
}

SvLocal SvType QBChannelListFilter_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBChannelListFilterDestroy
    };

    static const struct SvChannelMergerFilter_ methods = {
        .channelFound = QBChannelListFilterChannelFound,
        .channelLost  = QBChannelListFilterChannelLost,
        .updateState  = QBChannelListFilterUpdateState,
        .isEnabled    = QBChannelListFilterIsEnabled,
        .isAllowed    = QBChannelListFilterIsChannelAllowed
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBChannelListFilter",
                            sizeof(struct QBChannelListFilter_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvChannelMergerFilter_getInterface(), &methods,
                            NULL);
    }

    return type;
}

QBChannelListFilter QBChannelListFilterCreate(void)
{
    QBChannelListFilter self = (QBChannelListFilter) SvTypeAllocateInstance(QBChannelListFilter_getType(), NULL);
    if (!self) {
        log_error("can't create QBChannelListFilter");
        return self;
    }

    self->channels = SvHashTableCreate(97, NULL);

    return self;
}
