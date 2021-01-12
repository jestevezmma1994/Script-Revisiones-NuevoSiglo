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

#include "trailerPreview.h"

#include <main.h>

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>

#include <QBViewport.h>

#include <SvPlayerManager/SvPlayerManager.h>
#include <SvPlayerManager/SvPlayerTask.h>
#include <SvPlayerManager/SvPlayerTaskListener.h>
#include <SvPlayerKit/SvContent.h>

#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <settings.h>
#include <SWL/label.h>
#include <SWL/QBFrame.h>
#include <QBWidgets/QBAsyncLabel.h>

#include <Services/core/GlobalPlayer.h>


typedef struct QBTrailerPreview_ *QBTrailerPreview;

struct QBTrailerPreview_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    SvPlayerTask playerTask;

    SvWidget w;

    SvWidget title;

    SvTimerId timer;

    SvString media;
    double delay;

    SvBitmap bg;
    SvWidget viewport;
    int viewportXOffset;
    int viewportYOffset;
    int border;
};

SvLocal void QBTrailerPreviewCancelPlayback(QBTrailerPreview self);

SvLocal void
QBTrailerPreviewReset(QBTrailerPreview self)
{
    QBTrailerPreviewCancelPlayback(self);
    svWidgetSetHidden(self->viewport, true);
    svWidgetSetBitmap(self->w, self->bg);
}
SvLocal void
QBTrailerPreviewPlaybackStateChanged(SvObject self_)
{
}

SvLocal void
QBTrailerPreviewPlaybackFailed(SvObject self_, SvErrorInfo errorInfo)
{
    QBTrailerPreview self = (QBTrailerPreview) self_;
    QBTrailerPreviewReset(self);
    SvErrorInfoWriteLogMessage(errorInfo);
    SvErrorInfoDestroy(errorInfo);
}

SvLocal void
QBTrailerPreviewGotPlaybackError(SvObject self_, SvErrorInfo errorInfo)
{
    QBTrailerPreview self = (QBTrailerPreview) self_;
    QBTrailerPreviewReset(self);
    SvErrorInfoWriteLogMessage(errorInfo);
    SvErrorInfoDestroy(errorInfo);
}

SvLocal void
QBTrailerPreviewPlaybackFinished(SvObject self_)
{
    QBTrailerPreview self = (QBTrailerPreview) self_;
    QBTrailerPreviewReset(self);
}

SvLocal void
QBTrailerPreviewGotPlayerEvent(SvObject self_, SvString name, void *arg)
{
}

SvLocal void
QBTrailerPreviewPlayerControlGranted(SvObject self_, SvObject controller)
{
}

SvLocal void
QBTrailerPreviewPlayerControlRevoked(SvObject self_, SvObject controller)
{
}

SvLocal void
QBTrailerPreviewDestroy(void *self_)
{
    QBTrailerPreview self = self_;

    SVTESTRELEASE(self->playerTask);
    SVTESTRELEASE(self->media);
    SVTESTRELEASE(self->bg);
    QBGlobalPlayerControllerRemoveController(self->appGlobals->playerController, (SvObject) self);
}

SvType
QBTrailerPreview_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTrailerPreviewDestroy
    };
    static SvType type = NULL;

    static const struct SvPlayerTaskListener_ playerTaskListenerMethods = {
        .stateChanged         = QBTrailerPreviewPlaybackStateChanged,
        .fatalErrorOccured    = QBTrailerPreviewPlaybackFailed,
        .nonFatalErrorOccured = QBTrailerPreviewGotPlaybackError,
        .playbackFinished     = QBTrailerPreviewPlaybackFinished,
        .handleEvent          = QBTrailerPreviewGotPlayerEvent
    };

    static const struct QBGlobalPlayerControllerListener_t playerControllerListener = {
        .controlGranted = QBTrailerPreviewPlayerControlGranted,
        .controlRevoked = QBTrailerPreviewPlayerControlRevoked,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTrailerPreview",
                            sizeof(struct QBTrailerPreview_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvPlayerTaskListener_getInterface(), &playerTaskListenerMethods,
                            QBGlobalPlayerControllerListener_getInterface(),
                            &playerControllerListener, NULL);
    }

    return type;
}

SvLocal void
QBTrailerPreviewTimerEventHandler(SvWidget w, SvTimerEvent ev)
{
    QBTrailerPreviewStartPlayback(w);
}

SvLocal void
QBTrailerPreviewCancelPlayback(QBTrailerPreview self)
{
    if (!self->playerTask)
        return;

    SvPlayerTaskClose(self->playerTask, NULL);
    SVRELEASE(self->playerTask);
    self->playerTask = NULL;

    QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_disabled, NULL);

    QBGlobalPlayerControllerRemoveController(self->appGlobals->playerController, (SvObject) self);
}

SvLocal void
QBTrailerPreviewClean(SvApplication app, void *prv)
{
    QBTrailerPreview self = prv;

    if (self->playerTask) {
        QBTrailerPreviewCancelPlayback(self);
    }

    SVRELEASE(self);
}

#define MAX_BUF 1024

SvWidget
QBTrailerPreviewNew(AppGlobals appGlobals, const char *widgetName)
{
    SvWidget w, tmp;
    char buf[MAX_BUF];
    QBTrailerPreview self = (QBTrailerPreview) SvTypeAllocateInstance(QBTrailerPreview_getType(), NULL);

    self->appGlobals = appGlobals;

    w = svSettingsWidgetCreate(appGlobals->res, widgetName);
    w->prv = self;
    w->clean = QBTrailerPreviewClean;
    svWidgetSetTimerEventHandler(w, QBTrailerPreviewTimerEventHandler);

    self->w = w;

    SvBitmap bg = svSettingsGetBitmap(widgetName, "bg");
    self->bg = SVTESTRETAIN(bg);
    svWidgetSetBitmap(w, self->bg);

    self->border = svSettingsGetInteger(widgetName, "border", 0);

    self->viewport = svWidgetCreateViewport(appGlobals->res,
                                            w->width - 2 * self->border,
                                            w->height - 2 * self->border);
    self->viewportXOffset = self->viewportYOffset = self->border;

    svWidgetAttach(w, self->viewport, self->viewportXOffset, self->viewportYOffset, 1);
    svWidgetSetHidden(self->viewport, true);

    snprintf(buf, MAX_BUF, "%s.Frame", widgetName);
    tmp = QBFrameCreateFromSM(appGlobals->res, buf);
    if (tmp)
        svSettingsWidgetAttach(w, tmp, buf, 2);

    snprintf(buf, MAX_BUF, "%s.Title", widgetName);
    self->title = QBAsyncLabelNew(appGlobals->res, buf, appGlobals->textRenderer);
    if (self->title)
        svSettingsWidgetAttach(w, self->title, buf, 3);

    self->delay = svSettingsGetDouble(widgetName, "delay", 1.0);

    self->media = NULL;

    return w;
}

extern int
QBTrailerPreviewSetTitle(SvWidget w, SvString title)
{
    QBTrailerPreview self = w->prv;
    QBAsyncLabelSetText(self->title, title);
    return 0;
}

int
QBTrailerPreviewSetMedia(SvWidget w, SvString url)
{
    QBTrailerPreview self = w->prv;

    SVTESTRETAIN(url);
    SVTESTRELEASE(self->media);
    self->media = url;

    if (self->timer) {
        svAppTimerStop(w->app, self->timer);
        self->timer = 0;
    }
    if (self->playerTask) {
        QBTrailerPreviewCancelPlayback(self);
    }

    svWidgetSetBitmap(w, self->bg);

    if (url && self->delay >= 0.0)
        self->timer = svAppTimerStart(w->app, w, self->delay, 1);

    return 0;
}

extern void
QBTrailerPreviewStartPlayback(SvWidget w)
{
    QBTrailerPreview self = w->prv;

    if (!self->media)
        return;
    if (self->playerTask) {
        SvPlayerTaskPlay(self->playerTask, 1.0, -1.0, NULL);
        return;
    }

    SvContent content = SvContentCreateFromCString(SvStringCString(self->media), NULL);

    struct SvPlayerTaskParams_s params = {
        .content        = content,
        .standalone     = true,
        .delegate       = (SvObject) self,
        .viewport       = QBViewportGet(),
        .speed          = 1.0,
        .position       = 0.0,
        .max_memory_use = 0,
    };

    QBGlobalPlayerControllerPushController(self->appGlobals->playerController, (SvObject) self);

    Sv2DRect rect = { .x = self->border, .y = self->border };
    SvWidget tmp = w;
    while (tmp) {
        rect.x += tmp->off_x;
        rect.y += tmp->off_y;
        tmp = svWidgetGetParent(tmp);
    }
    rect.width = w->width - 2 * self->border;
    rect.height = w->height - 2 * self->border;

    self->playerTask = SvPlayerManagerCreatePlayerTask(SvPlayerManagerGetInstance(), &params, NULL);
    if (self->playerTask) {
        svWidgetSetHidden(self->viewport, false);
        if (QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_windowed, &rect) == 0)
            SvPlayerTaskPlay(self->playerTask, 1.0, -1.0, NULL);
        svWidgetSetBitmap(w, NULL);
    } else {
        QBGlobalPlayerControllerRemoveController(self->appGlobals->playerController, (SvGenericObject) self);
        SvLogError("%s() : can't create player task", __func__);
    }

    SVRELEASE(content);
}

extern void
QBTrailerPreviewPausePlayback(SvWidget w)
{
    QBTrailerPreview self = w->prv;
    if (!self->playerTask)
        return;
    SvPlayerTaskPlay(self->playerTask, 0.0, -1.0, NULL);
}

extern void
QBTrailerPreviewPlayPausePlayback(SvWidget w)
{
    QBTrailerPreview self = w->prv;
    if (!self->playerTask) {
        QBTrailerPreviewStartPlayback(w);
        return;
    }
    SvPlayerTaskState state = SvPlayerTaskGetState(self->playerTask);
    if (state.wantedSpeed == 1.0) {
        QBTrailerPreviewPausePlayback(w);
    } else {
        QBTrailerPreviewStartPlayback(w);
    }
}

extern bool
QBTrailerPreviewIsPlaying(SvWidget w)
{
    QBTrailerPreview self = w->prv;
    if (self->playerTask)
        return true;
    return false;
}

extern double
QBTrailerPreviewGetCurrentPosition(SvWidget w)
{
    QBTrailerPreview self = w->prv;
    if (!self->playerTask)
        return 0.0;
    SvPlayerTaskState state = SvPlayerTaskGetState(self->playerTask);
    return state.currentPosition;
}
