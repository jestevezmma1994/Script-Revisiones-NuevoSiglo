/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBRecordingBookmarkManagerPlugin.h"

#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManagerPlugin.h>
#include <QBPVRRecording.h>
#include <QBPVRTypes.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>


// TODO: Add support for bookmarks other than LAST_POSITION.
//       When this is done, bookmarkId has to be saved in file
//       and contentIdToBookmarkId can be removed.

struct QBRecordingBookmarkManagerPlugin_ {
    struct SvObject_ super_;
    QBPVRProvider provider;

    SvHashTable bookmarksStorage;

    SvHashTable contentIdToBookmarkId;

    QBBookmarkType *handledTypes;
    size_t handledTypesCnt;
};

#define APP_META_KEY__LAST_POSITION  "app:last_pos"

SvLocal void
QBRecordingBookmarkManagerPluginRecordingAdded(SvObject self_,
                                               QBPVRRecording rec)
{
    QBRecordingBookmarkManagerPlugin self = (QBRecordingBookmarkManagerPlugin) self_;
    if (!rec->playable)
        return;

    if (SvHashTableFind(self->contentIdToBookmarkId, (SvObject) rec->id)) {
        return;
    }

    SvValue val = (SvValue) QBPVRProviderGetRecordingProperty(self->provider,
                                                              rec,
                                                              SVSTRING(APP_META_KEY__LAST_POSITION));
    if (!val)
        return;
    double last_pos = SvValueGetDouble(val);

    QBBookmark bookmark = QBBookmarkCreate(NULL, rec->id, last_pos, QBBookmarkType_Recording);
    QBBookmarkAddParameter(bookmark, (SvObject) SVSTRING("LAST_POSITION"), (SvObject) SVSTRING("LAST_POSITION"));
    SvHashTableInsert(self->bookmarksStorage, (SvObject) QBBookmarkGetBookmarkId(bookmark), (SvObject) bookmark);
    SvHashTableInsert(self->contentIdToBookmarkId, (SvObject) rec->id, (SvObject) QBBookmarkGetBookmarkId(bookmark));
    SVRELEASE(bookmark);
}

SvLocal void
QBRecordingBookmarkManagerPluginRecordingRemoved(SvObject self_,
                                                 QBPVRRecording rec)
{
    QBRecordingBookmarkManagerPlugin self = (QBRecordingBookmarkManagerPlugin) self_;
    if (!rec->playable)
        return;

    SvString bookmarkId = (SvString) SvHashTableFind(self->contentIdToBookmarkId, (SvObject) rec->id);
    if (bookmarkId) {
        SvHashTableRemove(self->bookmarksStorage, (SvObject) bookmarkId);
        SvHashTableRemove(self->contentIdToBookmarkId, (SvObject) rec->id);
    }
}

SvLocal void
QBRecordingBookmarkManagerPluginRecordingChanged(SvObject self_,
                                                 QBPVRRecording rec,
                                                 QBPVRRecording oldRecording)
{
}

SvLocal void
QBRecordingBookmarkManagerPluginRecordingRestricted(SvObject self_,
                                                    QBPVRRecording rec)
{
}

SvLocal void
QBRecordingBookmarkManagerPluginQuotaChanged(SvObject self, QBPVRQuota quota)
{
}

SvLocal void
QBRecordingBookmarkManagerPluginDirectoryAdded(SvObject self_, QBPVRDirectory dir)
{
}

SvLocal SvArray
QBRecordingBookmarkManagerPluginCreateRecordingsArray(QBRecordingBookmarkManagerPlugin self)
{
    QBPVRProviderGetRecordingsParams params = {
        .includeRecordingsInDirectories = true,
        .playable = true,
    };

    return QBPVRProviderGetRecordings(self->provider,
                                      params);
}

SvLocal int
QBRecordingBookmarkManagerPluginStart(SvObject self_,
                                      SvScheduler scheduler)
{
    QBRecordingBookmarkManagerPlugin self = (QBRecordingBookmarkManagerPlugin) self_;

    SvArray recordings = QBRecordingBookmarkManagerPluginCreateRecordingsArray(self);
    SvIterator it = SvArrayIterator(recordings);
    QBPVRRecording rec = NULL;
    while ((rec = (QBPVRRecording) SvIteratorGetNext(&it))) {
        SvValue val = (SvValue) QBPVRProviderGetRecordingProperty(self->provider,
                                                                  rec,
                                                                  SVSTRING(APP_META_KEY__LAST_POSITION));
        if (!val)
            continue;

        double last_pos = SvValueGetDouble(val);

        QBBookmark bookmark = QBBookmarkCreate(NULL, rec->id, last_pos, QBBookmarkType_Recording);
        QBBookmarkAddParameter(bookmark, (SvObject) SVSTRING("LAST_POSITION"), (SvObject) SVSTRING("LAST_POSITION"));

        SvHashTableInsert(self->contentIdToBookmarkId, (SvObject) rec->id, (SvObject) QBBookmarkGetBookmarkId(bookmark));
        SvHashTableInsert(self->bookmarksStorage, (SvObject) QBBookmarkGetBookmarkId(bookmark), (SvObject) bookmark);
        SVRELEASE(bookmark);
    }
    SVRELEASE(recordings);

    QBPVRProviderAddListener(self->provider, (SvObject) self);
    return 0;
}

SvLocal int
QBRecordingBookmarkManagerPluginStop(SvObject self_)
{
    QBRecordingBookmarkManagerPlugin self = (QBRecordingBookmarkManagerPlugin) self_;
    QBPVRProviderRemoveListener(self->provider, (SvObject) self);
    return 0;
}

SvLocal int
QBRecordingBookmarkManagerPluginSetup(SvObject self_,
                                      SvHashTable storage,
                                      QBBookmarkType *handledTypes,
                                      size_t typesCnt)
{
    QBRecordingBookmarkManagerPlugin self = (QBRecordingBookmarkManagerPlugin) self_;

    if (typesCnt > 0) {
        self->handledTypes = calloc(typesCnt, sizeof(QBBookmarkType));
        if (unlikely(!self->handledTypes)) {
            SvLogError("%s: Can't allocate memory for array of types handled by recording bookmark manager plugin", __func__);
            return -1;
        }

        memcpy(self->handledTypes, handledTypes, typesCnt * sizeof(QBBookmarkType));
        self->handledTypesCnt = typesCnt;
    }

    self->bookmarksStorage = storage;

    return 0;
}

static void QBRecordingBookmarkManagerPluginSaveFile(QBRecordingBookmarkManagerPlugin self,
                                                     QBPVRRecording rec,
                                                     const double *new)
{
    double old = -1.0;
    SvValue oldVal = (SvValue) QBPVRProviderGetRecordingProperty(self->provider,
                                                                 rec,
                                                                 SVSTRING(APP_META_KEY__LAST_POSITION));
    if (oldVal) {
        old = SvValueGetDouble(oldVal);
    }

    if (oldVal && new && *new == old)
        return;

    SvValue newVal = NULL;
    if (new)
        newVal = SvValueCreateWithDouble(*new, NULL);
    QBPVRProviderSetRecordingProperty(self->provider,
                                      rec,
                                      SVSTRING(APP_META_KEY__LAST_POSITION),
                                      (SvObject) newVal);
    SVTESTRELEASE(newVal);
}

SvLocal int
QBRecordingBookmarkManagerPluginOnChangeBookmark_(SvObject self_,
                                                  QBBookmark bookmark,
                                                  double *position)
{
    QBRecordingBookmarkManagerPlugin self = (QBRecordingBookmarkManagerPlugin) self_;

    if (!QBBookmarkManagerPluginIsTypeHandled(self->handledTypes, self->handledTypesCnt, QBBookmarkGetType(bookmark)))
        return 0;

    QBPVRRecording recording = QBPVRProviderGetRecordingById(self->provider, QBBookmarkGetContentId(bookmark));
    if (!recording)
        return -1;

    // Set new position
    QBRecordingBookmarkManagerPluginSaveFile(self, recording, position);

    return 0;
}

SvLocal int
QBRecordingBookmarkManagerPluginOnChangeBookmark(SvObject self_,
                                                 QBBookmark bookmark)
{
    // Set new position
    double pos = QBBookmarkGetPosition(bookmark);
    if (pos < 0)
        return -1;

    return QBRecordingBookmarkManagerPluginOnChangeBookmark_(self_, bookmark, &pos);
}


SvLocal int
QBRecordingBookmarkManagerPluginOnAddBookmark(SvObject self_,
                                              QBBookmark bookmark)
{
    QBRecordingBookmarkManagerPlugin self = (QBRecordingBookmarkManagerPlugin) self_;

    SvHashTableInsert(self->contentIdToBookmarkId, (SvObject) QBBookmarkGetContentId(bookmark), (SvObject) QBBookmarkGetBookmarkId(bookmark));
    return QBRecordingBookmarkManagerPluginOnChangeBookmark(self_, bookmark);
}

SvLocal int
QBRecordingBookmarkManagerPluginOnRemoveBookmark(SvObject self_,
                                                 QBBookmark bookmark)
{
    QBRecordingBookmarkManagerPlugin self = (QBRecordingBookmarkManagerPlugin) self_;

    SvHashTableRemove(self->contentIdToBookmarkId, (SvObject) QBBookmarkGetContentId(bookmark));
    return QBRecordingBookmarkManagerPluginOnChangeBookmark_(self_, bookmark, NULL);
}

SvLocal void
QBRecordingBookmarkManagerPluginDestroy(void *self_)
{
    QBRecordingBookmarkManagerPlugin self = (QBRecordingBookmarkManagerPlugin) self_;

    if (self->handledTypesCnt > 0)
        free(self->handledTypes);

    SVTESTRELEASE(self->contentIdToBookmarkId);
}

SvType
QBRecordingBookmarkManagerPlugin_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRecordingBookmarkManagerPluginDestroy
    };
    static SvType type = NULL;

    static const struct QBBookmarkManagerPlugin_t BMMethods = {
            .start = QBRecordingBookmarkManagerPluginStart,
            .stop = QBRecordingBookmarkManagerPluginStop,
            .setup = QBRecordingBookmarkManagerPluginSetup,
            .onAddBookmark = QBRecordingBookmarkManagerPluginOnAddBookmark,
            .onRemoveBookmark = QBRecordingBookmarkManagerPluginOnRemoveBookmark,
            .onChangeBookmark = QBRecordingBookmarkManagerPluginOnChangeBookmark
    };

    static const struct QBPVRProviderListener_ pvrProviderListenerMethods = {
        .recordingAdded = QBRecordingBookmarkManagerPluginRecordingAdded,
        .recordingChanged = QBRecordingBookmarkManagerPluginRecordingChanged,
        .recordingRemoved = QBRecordingBookmarkManagerPluginRecordingRemoved,
        .recordingRestricted = QBRecordingBookmarkManagerPluginRecordingRestricted,
        .quotaChanged = QBRecordingBookmarkManagerPluginQuotaChanged,
        .directoryAdded = QBRecordingBookmarkManagerPluginDirectoryAdded,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRecordingBookmarkManagerPlugin",
                            sizeof(struct QBRecordingBookmarkManagerPlugin_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBBookmarkManagerPlugin_getInterface(), &BMMethods,
                QBPVRProviderListener_getInterface(), &pvrProviderListenerMethods,
                NULL);
    }

    return type;
}

int
QBRecordingBookmarkManagerPluginInit(QBRecordingBookmarkManagerPlugin self, QBPVRProvider provider)
{
    if (!self || !provider)
        return -1;

    self->provider = provider;
    self->contentIdToBookmarkId = SvHashTableCreate(70, NULL);
    return 0;
}
