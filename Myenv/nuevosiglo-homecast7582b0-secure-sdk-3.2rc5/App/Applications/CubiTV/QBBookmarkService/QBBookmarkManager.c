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

#include "QBBookmarkManager.h"
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManagerPlugin.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <fibers/c/fibers.h>

struct QBBookmarkManager_ {
    struct SvObject_ super_;

    SvHashTable plugins;
    SvHashTable idToBookmark;

    SvScheduler scheduler;
};

SvLocal void
QBBookmarkManagerDestroy(void *self_)
{
    QBBookmarkManager self = (QBBookmarkManager) self_;

    SVRELEASE(self->plugins);
    SVRELEASE(self->idToBookmark);
}

SvLocal SvType
QBBookmarkManager_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBBookmarkManagerDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBBookmarkManager",
                            sizeof(struct QBBookmarkManager_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBBookmarkManager
QBBookmarkManagerCreate(AppGlobals appGlobals)
{
    int err = -1;
    QBBookmarkManager self = NULL;
    SvHashTable plugins = NULL, bookmarks = NULL;

    if (!appGlobals) {
        SvLogError("%s: NULL argument passed: appGlobals", __func__);
        goto err;
    }

    plugins = SvHashTableCreate(3, NULL);
    if (unlikely(!plugins)) {
        SvLogError("%s: can't create SvHashTable", __func__);
        goto err;
    }

    bookmarks = SvHashTableCreate(60, NULL);
    if (unlikely(!plugins)) {
        SvLogError("%s: can't create SvHashTable", __func__);
        goto err;
    }

    self = (QBBookmarkManager) SvTypeAllocateInstance(QBBookmarkManager_getType(), NULL);
    if (unlikely(!self)) {
        SvLogError("%s: can't allocate memory for QBBookmarkManager", __func__);
        goto err;
    }

    self->plugins = plugins;
    self->idToBookmark = bookmarks;

    QBBookmarkRegisterJSONSerializers();

    err = 0;

err:
    if (err < 0) {
        SVTESTRELEASE(plugins);
        SVTESTRELEASE(bookmarks);
    }
    return self;
}

int
QBBookmarkManagerStart(QBBookmarkManager self,
                       SvScheduler scheduler)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    }

    SvIterator it = SvHashTableValuesIterator(self->plugins);
    SvObject plugin = NULL;
    while ((plugin = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBBookmarkManagerPlugin, plugin, start, scheduler);
    }

    self->scheduler = scheduler;

    return 0;
}

int
QBBookmarkManagerStop(QBBookmarkManager self)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    }

    SvIterator it = SvHashTableValuesIterator(self->plugins);
    SvObject plugin = NULL;
    while ((plugin = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBBookmarkManagerPlugin, plugin, stop);
    }

    return 0;
}

SvArray
QBBookmarkManagerCreateBookmarksArrayForContent(QBBookmarkManager self,
                                                SvString contentId)
{
    SvArray bookmarks = NULL;

    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return NULL;
    } else if (!contentId) {
        SvLogError("%s: NULL contentId passed", __func__);
        return NULL;
    }

    bookmarks = SvArrayCreate(NULL);
    QBBookmark bookmark;
    SvIterator it = SvHashTableValuesIterator(self->idToBookmark);
    while ((bookmark = (QBBookmark) SvIteratorGetNext(&it))) {
        if (!SvObjectEquals((SvObject) QBBookmarkGetContentId(bookmark), (SvObject) contentId))
            continue;

        SvArrayAddObject(bookmarks, (SvObject) bookmark);
    }

    return bookmarks;
}

QBBookmark
QBBookmarkManagerGetBookmarkByID(QBBookmarkManager self,
                                 SvString bookmarkId)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return NULL;
    } else if (!bookmarkId) {
        SvLogError("%s: NULL bookmarkId passed", __func__);
        return NULL;
    }

    return (QBBookmark) SvHashTableFind(self->idToBookmark, (SvObject) bookmarkId);
}

int
QBBookmarkManagerAddBookmark(QBBookmarkManager self,
                             QBBookmark bookmark)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    } else if (!bookmark) {
        SvLogError("%s: NULL bookmark passed", __func__);
        return -1;
    }

    SvHashTableInsert(self->idToBookmark, (SvObject) QBBookmarkGetBookmarkId(bookmark), (SvObject) bookmark);

    SvIterator it = SvHashTableValuesIterator(self->plugins);
    SvObject plugin = NULL;
    while ((plugin = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBBookmarkManagerPlugin, plugin, onAddBookmark, bookmark);
    }

    return 0;
}

int
QBBookmarkManagerRemoveBookmark(QBBookmarkManager self,
                                QBBookmark bookmark)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    } else if (!bookmark) {
        SvLogError("%s: NULL bookmark passed", __func__);
        return -1;
    }

    SVRETAIN(bookmark);
    SvHashTableRemove(self->idToBookmark, (SvObject) QBBookmarkGetBookmarkId(bookmark));

    SvIterator it = SvHashTableValuesIterator(self->plugins);
    SvObject plugin = NULL;
    while ((plugin = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBBookmarkManagerPlugin, plugin, onRemoveBookmark, bookmark);
    }

    SVRELEASE(bookmark);

    return 0;
}

int
QBBookmarkManagerUpdateBookmark(QBBookmarkManager self,
                                QBBookmark bookmark)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    } else if (!bookmark) {
        SvLogError("%s: NULL bookmark passed", __func__);
        return -1;
    }

    SvIterator it = SvHashTableValuesIterator(self->plugins);
    SvObject plugin = NULL;
    while ((plugin = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBBookmarkManagerPlugin, plugin, onChangeBookmark, bookmark);
    }

    return 0;
}

int
QBBookmarkManagerAddPlugin(QBBookmarkManager self,
                           SvObject plugin,
                           QBBookmarkType *handledTypes,
                           size_t typesCnt)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    } else if (!plugin || !SvObjectIsImplementationOf(plugin, QBBookmarkManagerPlugin_getInterface())) {
        SvLogError("%s: invalid plugin passed", __func__);
        return -1;
    }

    SvHashTableInsert(self->plugins, (SvObject) plugin, (SvObject) plugin);

    return SvInvokeInterface(QBBookmarkManagerPlugin, plugin, setup, self->idToBookmark, handledTypes, typesCnt);
}

int
QBBookmarkManagerRemovePlugin(QBBookmarkManager self,
                              SvObject plugin)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    } else if (!plugin) {
        SvLogError("%s: NULL plugin passed", __func__);
        return -1;
    }

    if (!SvHashTableFind(self->plugins, (SvObject) plugin)) {
        SvLogError("%s: plugin not found", __func__);
        return -1;
    }

    SvInvokeInterface(QBBookmarkManagerPlugin, plugin, stop);
    SvHashTableRemove(self->plugins, (SvObject) plugin);

    return 0;
}

int
QBBookmarkManagerRestartPlugin(QBBookmarkManager self,
                               SvObject plugin)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    } else if (!plugin) {
        SvLogError("%s: NULL plugin passed", __func__);
        return -1;
    } else if (!SvHashTableFind(self->plugins, (SvObject) plugin)) {
        SvLogError("%s: plugin not found", __func__);
        return -1;
    }

    SvInvokeInterface(QBBookmarkManagerPlugin, plugin, stop);
    return SvInvokeInterface(QBBookmarkManagerPlugin, plugin, start, self->scheduler);
}

QBBookmark
QBBookmarkManagerGetLastPositionBookmark(QBBookmarkManager self,
                                         SvString contentId)
{
    QBBookmark bookmark = NULL;
    SvArray contentBookmarks = NULL;

    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return NULL;
    } else if (!contentId) {
        SvLogError("%s: NULL contentId passed", __func__);
        return NULL;
    }

    // First try to find last position bookmark for this content
    contentBookmarks = QBBookmarkManagerCreateBookmarksArrayForContent(self, contentId);
    if (!contentBookmarks)
        return NULL;
    else if (SvArrayCount(contentBookmarks) == 0) {
        SVRELEASE(contentBookmarks);
        return NULL;
    }

    SvIterator it = SvArrayIterator(contentBookmarks);
    while ((bookmark = (QBBookmark) SvIteratorGetNext(&it))) {
        SvObject lastPosAttr = QBBookmarkGetParameter(bookmark, (SvObject) SVSTRING("LAST_POSITION"));

        if(lastPosAttr) {
            SVTESTRELEASE(contentBookmarks);
            return bookmark;
        }
    }

    SVTESTRELEASE(contentBookmarks);
    return NULL;
}

QBBookmark
QBBookmarkManagerSetLastPositionBookmark(QBBookmarkManager self,
                                         SvString contentId,
                                         double position,
                                         QBBookmarkType type)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return NULL;
    } else if (!contentId) {
        SvLogError("%s: NULL contentId passed", __func__);
        return NULL;
    }

    QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self, contentId);
    if (bookmark) {
        QBBookmarkUpdatePosition(bookmark, position);
    } else {
        bookmark = QBBookmarkCreate(NULL, contentId, position, type);
        if (!bookmark)
            return NULL;

        QBBookmarkAddParameter(bookmark, (SvObject) SVSTRING("LAST_POSITION"), (SvObject) SVSTRING("LAST_POSITION"));
        QBBookmarkManagerAddBookmark(self, bookmark);
        SVRELEASE(bookmark);
    }

    return bookmark;
}

