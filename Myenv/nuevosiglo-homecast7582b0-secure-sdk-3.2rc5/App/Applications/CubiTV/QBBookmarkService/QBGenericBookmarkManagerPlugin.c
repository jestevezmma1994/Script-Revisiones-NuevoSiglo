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

#include "QBGenericBookmarkManagerPlugin.h"
#include <QBBookmarkService/QBBookmarkManagerPlugin.h>
#include <QBBookmarkService/QBBookmark.h>
#include <Services/core/JSONserializer.h>
#include <SvJSON/SvJSONParse.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvBinaryTree.h>
#include <safeFiles.h>
#include <fibers/c/fibers.h>
#include <main_decl.h>
#include <main.h>

// Possible optimization: Add information (to bookmark object) when bookmark was modified/used for the last time
//       Remove bookmarks that weren't modified/used for specified period of time
//       Now when bookmark storage is full, every time when we add new bookmark, we have to skim whole table
//       for the oldest bookmark

#define MAX_BOOKMARKS_CNT 100

struct QBGenericBookmarkManagerPlugin_ {
    struct SvObject_ super_;

    SvHashTable bookmarksStorage;
    SvString filePath;

    QBBookmarkType *handledTypes;
    size_t handledTypesCnt;
};

SvLocal int
QBGenericBookmarkManagerPluginCompareTimestamps(void *prv, SvObject objectA, SvObject objectB)
{
    int timeA = QBBookmarkGetTimestamp((QBBookmark) objectA);
    int timeB = QBBookmarkGetTimestamp((QBBookmark) objectB);

    if (timeA < timeB)
        return -1;
    else if (timeA > timeB)
        return 1;
    else
        return 0;
}

SvLocal void
QBGenericBookmarkManagerPluginRemoveOldBookmarks(QBGenericBookmarkManagerPlugin self, SvBinaryTree bookmarks, size_t numberOfBookmarksToRemove, QBBookmark lastBookmark)
{
    SvObject bookmark;

    SvString lastBookmarkId = QBBookmarkGetBookmarkId(lastBookmark);

    int i = 0;
    while (numberOfBookmarksToRemove > 0) {
        bookmark = SvBinaryTreeObjectAtIndex(bookmarks, i);
        SvString bookmarkId = QBBookmarkGetBookmarkId((QBBookmark) bookmark);
        if (SvObjectEquals((SvObject) bookmarkId, (SvObject) lastBookmarkId)) {
            i++;
        } else {
            SvHashTableRemove(self->bookmarksStorage, (SvObject) bookmarkId);
            SvBinaryTreeRemove(bookmarks, (SvObject) bookmark);
            numberOfBookmarksToRemove--;
        }
    }
}

SvLocal void
QBGenericBookmarkManagerPluginSaveFile(QBGenericBookmarkManagerPlugin self, QBBookmark lastBookmark)
{
    QBBookmark bookmark;
    SvIterator it = SvHashTableValuesIterator(self->bookmarksStorage);
    SvBinaryTree bookmarksTree = SvBinaryTreeCreateWithCompareFn(QBGenericBookmarkManagerPluginCompareTimestamps, NULL, NULL);
    while ((bookmark = (QBBookmark) SvIteratorGetNext(&it))) {
        if (QBBookmarkManagerPluginIsTypeHandled(self->handledTypes, self->handledTypesCnt, QBBookmarkGetType(bookmark)))
            SvBinaryTreeInsert(bookmarksTree, (SvObject) bookmark);
    }

    // if limit is exhausted remove the oldest entries
    size_t bookmarksCnt = SvBinaryTreeNodesCount(bookmarksTree);
    if (bookmarksCnt > MAX_BOOKMARKS_CNT)
        QBGenericBookmarkManagerPluginRemoveOldBookmarks(self, bookmarksTree, bookmarksCnt - MAX_BOOKMARKS_CNT, lastBookmark);

    it = SvBinaryTreeIterator(bookmarksTree);
    SvArray bookmarks = SvArrayCreateWithValues(&it, NULL);
    QBJSONSerializer JSONSerializer = (QBJSONSerializer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBJSONSerializer"));
    QBJSONSerializerAddJob(JSONSerializer, (SvObject) bookmarks, self->filePath);
    SVRELEASE(bookmarks);
    SVRELEASE(bookmarksTree);
}

SvLocal int
QBGenericBookmarkManagerPluginStart(SvObject self_,
                                    SvScheduler scheduler)
{
    int retval = 0;
    QBGenericBookmarkManagerPlugin self = (QBGenericBookmarkManagerPlugin) self_;

    char *buffer = NULL;
    SvObject temp = NULL;

    QBFileToBuffer(SvStringCString(self->filePath), &buffer);
    if (!buffer) {
        SvLogWarning("%s(): failed to read bookmarks storage file", __func__);
        return -1;
    }

    SvHashTable pluginBookmarks = SvHashTableCreate(30, NULL);

    temp = SvJSONParseString(buffer, true, NULL);
    if (!temp) {
        SvLogError("%s(): bookmarks storage file is not a valid JSON!", __func__);
        retval = -1;
        goto fini;
    } else if (!SvObjectIsInstanceOf(temp, SvArray_getType())) {
        SvLogError("%s(): bookmarks storage file is not a JSON array!", __func__);
        retval = -1;
        goto fini;
    }

    QBBookmark bookmark;
    SvIterator it = SvArrayIterator((SvArray) temp);
    while ((bookmark = (QBBookmark) SvIteratorGetNext(&it))) {
        if (!SvObjectIsInstanceOf((SvObject) bookmark, QBBookmark_getType())) {
            SvLogError("%s(): bookmarks storage file contains something other than QBBookmark", __func__);
            retval = -1;
            break;
        }

        if (QBBookmarkManagerPluginIsTypeHandled(self->handledTypes, self->handledTypesCnt, QBBookmarkGetType(bookmark)))
            SvHashTableInsert(pluginBookmarks, (SvObject) QBBookmarkGetBookmarkId(bookmark), (SvObject) bookmark);
    }

    if (retval == 0) {
        SvIterator itKeys = SvHashTableKeysIterator(pluginBookmarks);
        SvIterator itVals = SvHashTableValuesIterator(pluginBookmarks);
        SvHashTableInsertObjects(self->bookmarksStorage, &itKeys, &itVals);
    } else {
        SvHashTableRemoveAllObjects(pluginBookmarks);
    }

fini:
    free(buffer);
    SVTESTRELEASE(temp);
    SVTESTRELEASE(pluginBookmarks);
    return retval;
}

SvLocal int
QBGenericBookmarkManagerPluginStop(SvObject self_)
{
    return 0;
}

SvLocal int
QBGenericBookmarkManagerPluginSetup(SvObject self_,
                                    SvHashTable storage,
                                    QBBookmarkType *handledTypes,
                                    size_t typesCnt)
{
    QBGenericBookmarkManagerPlugin self = (QBGenericBookmarkManagerPlugin) self_;

    if (!storage) {
        SvLogError("%s: NULL argument passed", __func__);
        return -1;
    }

    if (typesCnt > 0) {
        self->handledTypes = calloc(typesCnt, sizeof(QBBookmarkType));
        if (unlikely(!self->handledTypes)) {
            SvLogError("%s: can't allocate memory for array of types handled by generic bookmark manager plugin", __func__);
            return -1;
        }

        memcpy(self->handledTypes, handledTypes, typesCnt * sizeof(QBBookmarkType));
        self->handledTypesCnt = typesCnt;
    }

    self->bookmarksStorage = storage;

    return 0;
}

SvLocal int
QBGenericBookmarkManagerPluginOnChangeBookmark(SvObject self_,
                                               QBBookmark bookmark)
{
    QBGenericBookmarkManagerPlugin self = (QBGenericBookmarkManagerPlugin) self_;

    if (!QBBookmarkManagerPluginIsTypeHandled(self->handledTypes, self->handledTypesCnt, QBBookmarkGetType(bookmark)))
        return 0;

    QBGenericBookmarkManagerPluginSaveFile(self, bookmark);
    return 0;
}

SvLocal int
QBGenericBookmarkManagerPluginOnAddBookmark(SvObject self_,
                                            QBBookmark bookmark)
{
    return QBGenericBookmarkManagerPluginOnChangeBookmark(self_, bookmark);
}

SvLocal int
QBGenericBookmarkManagerPluginOnRemoveBookmark(SvObject self_,
                                               QBBookmark bookmark)
{
    return QBGenericBookmarkManagerPluginOnChangeBookmark(self_, bookmark);
}

SvLocal void
QBGenericBookmarkManagerPluginDestroy(void *self_)
{
    QBGenericBookmarkManagerPlugin self = (QBGenericBookmarkManagerPlugin) self_;

    if (self->handledTypesCnt > 0)
        free(self->handledTypes);

    SVTESTRELEASE(self->filePath);
}

SvType
QBGenericBookmarkManagerPlugin_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBGenericBookmarkManagerPluginDestroy
    };
    static SvType type = NULL;

    static const struct QBBookmarkManagerPlugin_t BMMethods = {
            .start = QBGenericBookmarkManagerPluginStart,
            .stop = QBGenericBookmarkManagerPluginStop,
            .setup = QBGenericBookmarkManagerPluginSetup,
            .onAddBookmark = QBGenericBookmarkManagerPluginOnAddBookmark,
            .onRemoveBookmark = QBGenericBookmarkManagerPluginOnRemoveBookmark,
            .onChangeBookmark = QBGenericBookmarkManagerPluginOnChangeBookmark
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBGenericBookmarkManagerPlugin",
                            sizeof(struct QBGenericBookmarkManagerPlugin_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBBookmarkManagerPlugin_getInterface(), &BMMethods,
                NULL);
    }

    return type;
}

QBGenericBookmarkManagerPlugin
QBGenericBookmarkManagerPluginInit(QBGenericBookmarkManagerPlugin self,
                                   SvString filePath)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return NULL;
    } else if (!filePath) {
        SvLogError("%s: NULL filePath passed", __func__);
        return NULL;
    }

    self->filePath = SVRETAIN(filePath);

    return self;
}

