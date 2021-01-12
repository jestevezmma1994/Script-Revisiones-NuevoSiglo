/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBAudioBackgroundPlayer.h"
#include <assert.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvData.h>
#include <Logic/AnyPlayerLogic.h>
#include <QBPlayerControllers/QBAnyPlayerController.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <SvPlayerManager/SvPlayerTask.h>
#include <SvPlayerManager/SvPlayerTaskListener.h>
#include <QBM3uParser.h>
#include "main.h"

#define MAX_M3U_SIZE 10 * 1024

struct QBAudioBackgroundPlaybackService_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvArray musicPlaylist;
    size_t playlistPos;
    SvPlayerTask playerTask;
};

SvLocal void QBAudioBackgroundPlaybackServiceStop(QBAudioBackgroundPlaybackService self)
{
    if (self->playerTask) {
        SvPlayerTaskClose(self->playerTask, NULL);
        SVRELEASE(self->playerTask);
        self->playerTask = NULL;
        QBGlobalPlayerControllerRemoveController(self->appGlobals->playerController, (SvGenericObject) self);
    }
}

SvLocal void QBAudioBackgroundPlaybackServiceStart(QBAudioBackgroundPlaybackService self)
{
    struct SvPlayerTaskParams_s params = {
        .content = NULL,
    };

    if (!self->musicPlaylist)
        return;
    params.content = (SvContent) SvArrayGetObjectAtIndex(self->musicPlaylist, self->playlistPos);
    if (!params.content)
        return;
    params.standalone = true;
    params.delegate = (SvObject) self;
    //FIXME: dummy view port should be created here?
    QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_disabled, NULL);
    params.viewport = QBViewportGet();
    params.speed = 1.0;
    params.position = 0.0;
    params.max_memory_use = 0;

    if (self->playerTask)
        QBAudioBackgroundPlaybackServiceStop(self);

    if (QBGlobalPlayerControllerPushController(self->appGlobals->playerController, (SvGenericObject) self) != 0)
        return;

    self->playerTask = SvPlayerManagerCreatePlayerTask(SvPlayerManagerGetInstance(), &params, NULL);
    if (self->playerTask)
        SvPlayerTaskPlay(self->playerTask, 1.0, 0.0, NULL);
    else
        QBGlobalPlayerControllerRemoveController(self->appGlobals->playerController, (SvGenericObject) self);
}

SvLocal void QBAudioBackgroundPlaybackServicePlaybackStateChanged(SvObject self_)
{
}

SvLocal void QBAudioBackgroundPlaybackServiceFatalErrorOccured(SvObject self_, SvErrorInfo errorOut)
{
    QBAudioBackgroundPlaybackService self = (QBAudioBackgroundPlaybackService) self_;
    QBAudioBackgroundPlaybackServiceStop(self);
}

SvLocal void QBAudioBackgroundPlaybackServiceNonFatalErrorOccured(SvObject self_, SvErrorInfo errorOut)
{
    QBAudioBackgroundPlaybackService self = (QBAudioBackgroundPlaybackService) self_;
    QBAudioBackgroundPlaybackServiceStop(self);
}

SvLocal void QBAudioBackgroundPlaybackServicePlaybackFinished(SvObject self_)
{
    QBAudioBackgroundPlaybackService self = (QBAudioBackgroundPlaybackService) self_;

    QBAudioBackgroundPlaybackServiceStop(self);
    self->playlistPos++;
    if (self->playlistPos < SvArrayGetCount(self->musicPlaylist))
        QBAudioBackgroundPlaybackServiceStart(self);
}

SvLocal void QBAudioBackgroundPlaybackServiceHandlePlayerEvent(SvObject self_, SvString name, void *arg)
{
}

SvLocal void QBAudioBackgroundPlaybackServiceDestroy(void *self_)
{
    QBAudioBackgroundPlaybackService self = (QBAudioBackgroundPlaybackService) self_;
    QBAudioBackgroundPlaybackServiceStop(self);
    SVTESTRELEASE(self->musicPlaylist);
}

SvLocal void QBAudioBackgroundPlaybackServiceControlGranted(SvObject self_, SvObject controller)
{
}

SvLocal void QBAudioBackgroundPlaybackServiceControlRevoked(SvObject self_, SvObject controller)
{
    QBAudioBackgroundPlaybackService self = (QBAudioBackgroundPlaybackService) self_;
    if (controller == self_)
        QBAudioBackgroundPlaybackServiceStop(self);
}

SvLocal SvType QBAudioBackgroundPlaybackService_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAudioBackgroundPlaybackServiceDestroy
    };

    static const struct SvPlayerTaskListener_ playerTaskListenerMethods = {
        .stateChanged         = QBAudioBackgroundPlaybackServicePlaybackStateChanged,
        .fatalErrorOccured    = QBAudioBackgroundPlaybackServiceFatalErrorOccured,
        .nonFatalErrorOccured = QBAudioBackgroundPlaybackServiceNonFatalErrorOccured,
        .playbackFinished     = QBAudioBackgroundPlaybackServicePlaybackFinished,
        .handleEvent          = QBAudioBackgroundPlaybackServiceHandlePlayerEvent
    };

    static const struct QBGlobalPlayerControllerListener_t playerControllerMethods = {
        .controlGranted = QBAudioBackgroundPlaybackServiceControlGranted,
        .controlRevoked = QBAudioBackgroundPlaybackServiceControlRevoked,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBAudioBackgroundPlaybackService",
                            sizeof(struct QBAudioBackgroundPlaybackService_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvPlayerTaskListener_getInterface(), &playerTaskListenerMethods,
                            QBGlobalPlayerControllerListener_getInterface(), &playerControllerMethods,
                            NULL);
    }
    return type;
}

QBAudioBackgroundPlaybackService
QBAudioBackgroundPlaybackServiceCreate(AppGlobals appGlobals)
{
    QBAudioBackgroundPlaybackService self = (QBAudioBackgroundPlaybackService) SvTypeAllocateInstance(QBAudioBackgroundPlaybackService_getType(), NULL);
    if (!self)
        return NULL;
    self->appGlobals = appGlobals;
    return self;
}

SvLocal void QBAudioBackgroundPlaybackServicePlaylistGotItem(void *self_, QBM3uParser parser, QBM3uItem item)
{
    QBAudioBackgroundPlaybackService self = (QBAudioBackgroundPlaybackService) self_;

    if (!item || !item->track)
        return;
    SvContent content = SvContentCreateFromCString(SvStringCString(item->track), NULL);
    if (content) {
        SvArrayAddObject(self->musicPlaylist, (SvObject) content);
        SVRELEASE(content);
    }
}

void
QBAudioBackgroundPlaybackServicePlayM3uPlaylist(QBAudioBackgroundPlaybackService self, SvString URI)
{
    assert(self);

    QBAudioBackgroundPlaybackServiceStop(self);

    SVTESTRELEASE(self->musicPlaylist);
    self->musicPlaylist = SvArrayCreate(NULL);
    if (!self->musicPlaylist)
        return;

    QBM3uParser parser = QBM3uParserCreateWithFile(URI, 1024, (MAX_M3U_SIZE), &QBAudioBackgroundPlaybackServicePlaylistGotItem, NULL, NULL, self);
    if (!parser)
        return;
    QBM3uParserRun(parser);
    SVRELEASE(parser);
    if (SvArrayGetCount(self->musicPlaylist) > 0) {
        self->playlistPos = 0;
        QBAudioBackgroundPlaybackServiceStart(self);
    }
}

void
QBAudioBackgroundPlaybackServicePlayPlaylist(QBAudioBackgroundPlaybackService self, SvArray playlist)
{
    assert(self);

    QBAudioBackgroundPlaybackServiceStop(self);

    SVTESTRELEASE(self->musicPlaylist);
    self->musicPlaylist = SVTESTRETAIN(playlist);
    if (self->musicPlaylist && SvArrayGetCount(self->musicPlaylist) > 0) {
        self->playlistPos = 0;
        QBAudioBackgroundPlaybackServiceStart(self);
    }
}

void
QBAudioBackgroundPlaybackServicePlayAudio(QBAudioBackgroundPlaybackService self, SvString URI)
{
    assert(self);

    QBAudioBackgroundPlaybackServiceStop(self);

    SVTESTRELEASE(self->musicPlaylist);
    self->musicPlaylist = SvArrayCreate(NULL);
    if (!self->musicPlaylist)
        return;
    SvContent content = SvContentCreateFromCString(SvStringGetCString(URI), NULL);
    if (content) {
        self->playlistPos = 0;
        SvArrayAddObject(self->musicPlaylist, (SvObject) content);
        SVRELEASE(content);
        QBAudioBackgroundPlaybackServiceStart(self);
    }
}

void
QBAudioBackgroundPlaybackServiceStopPlayback(QBAudioBackgroundPlaybackService self)
{
    assert(self);
    QBAudioBackgroundPlaybackServiceStop(self);
}

bool
QBAudioBackgroundPlaybackServiceIsPlaying(QBAudioBackgroundPlaybackService self)
{
    assert(self);
    return self->playerTask != NULL;
}
