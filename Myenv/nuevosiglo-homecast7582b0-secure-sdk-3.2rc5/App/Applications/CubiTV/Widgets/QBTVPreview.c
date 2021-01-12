/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBTVPreview.h"
#include <SWL/QBFrame.h>
#include <Windows/newtv.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <CUIT/Core/widget.h>
#include <settings.h>
#include <main.h>

struct QBTVPreview_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    QBPlaylistCursor cursor;

    int previewBorder;

    SvWidget preview;
    SvWidget channelNameLabel;
};
typedef struct QBTVPreview_t* QBTVPreview;

SvLocal void
QBTVPreviewSetChannelPreviewInfo(QBTVPreview self, SvGenericObject playlist, SvTVChannel ch)
{
    if (!self->channelNameLabel) {
        return;
    }

    if (ch == NULL) {
        QBAsyncLabelSetCText(self->channelNameLabel, "");
        return;
    }

    int idx = SvInvokeInterface(SvEPGChannelView, playlist, getChannelNumber, ch);
    char* buf = NULL;
    asprintf(&buf, "%03d %s", idx, SvStringCString(ch->name));
    QBAsyncLabelSetCText(self->channelNameLabel, buf);
    free(buf);
}

SvLocal void
QBTVPreviewSetOverlay(SvWidget self)
{
    QBTVPreview ctx = self->prv;
    Sv2DRect rect = Sv2DRectCreate(self->off_x + ctx->previewBorder,
                                   self->off_y + ctx->previewBorder,
                                   self->width - 2 * ctx->previewBorder,
                                   self->height - 2 * ctx->previewBorder);

    SvTVContextSetOverlay(ctx->appGlobals->newTV, &rect);
}

SvLocal void
QBTVPreviewChannelUpdated(SvGenericObject self_, QBPlaylistCursor cursor)
{
    QBTVPreview self = (QBTVPreview) self_;
    if (!self->cursor->channelID)
        return;

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject playlist = QBPlaylistManagerGetById(playlists, self->cursor->playlistID);
    SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, playlist, getByID, self->cursor->channelID);

    QBTVPreviewSetOverlay(self->preview);
    QBTVPreviewSetChannelPreviewInfo(self, playlist, channel);
}

SvLocal void
QBTVPreview__dtor__(void *self_)
{
    QBTVPreview self = self_;
    SVRELEASE(self->cursor);
    self->cursor = NULL;
}

SvLocal SvType
QBTVPreview_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTVPreview__dtor__
    };
    static SvType type = NULL;
    static const struct QBPlaylistCursorListener_t cursorsMethods = {
        .updated = QBTVPreviewChannelUpdated
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTVPreview",
                            sizeof(struct QBTVPreview_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPlaylistCursorListener_getInterface(), &cursorsMethods,
                            NULL);
    }

    return type;
}

SvLocal void
QBTVPreviewClean(SvApplication app, void* ptr)
{
    QBTVPreview prv = (QBTVPreview) ptr;
    QBPlaylistCursorRemoveListener(prv->cursor, (SvGenericObject) prv);
    SVRELEASE(prv);
}

SvWidget
QBTVPreviewNew(AppGlobals appGlobals, const char *widgetName, QBPlaylistCursor cursor)
{
    if (!cursor) {
        return NULL;
    }

    SvApplication app = appGlobals->res;
    SvWidget w = svSettingsWidgetCreate(app, widgetName);
    w->off_x = svSettingsGetInteger(widgetName, "xOffset", INT_MIN);
    w->off_y = svSettingsGetInteger(widgetName, "yOffset", INT_MIN);

    char* previewFrameStr = NULL;
    char* channelNameStr = NULL;
    char* ChannelNameBackgroundStr = NULL;

    asprintf(&previewFrameStr, "%s.PreviewFrame", widgetName);
    asprintf(&channelNameStr, "%s.ChannelName", widgetName);
    asprintf(&ChannelNameBackgroundStr, "%s.ChannelNameBackground", widgetName);

    QBTVPreview prv = (QBTVPreview) SvTypeAllocateInstance(QBTVPreview_getType(), NULL);

    prv->preview = w;
    prv->cursor = SVRETAIN(cursor);

    prv->appGlobals = appGlobals;

    prv->previewBorder = svSettingsGetInteger(widgetName, "border", 1);

    if ((prv->preview->width - 3 * prv->previewBorder) <= 0 ||
        (prv->preview->height - 3 * prv->previewBorder) <= 0) {
        prv->previewBorder = 10;
    }

    SvWidget viewportWidget = svWidgetCreateViewport(app,
                                                     prv->preview->width - 2 * prv->previewBorder,
                                                     prv->preview->height - 2 * prv->previewBorder);

    svWidgetAttach(prv->preview, viewportWidget, prv->previewBorder, prv->previewBorder, 1);

    if (svSettingsIsWidgetDefined(previewFrameStr)) {
        SvWidget frame = QBFrameCreateFromSM(app, previewFrameStr);
        if (frame) {
            svSettingsWidgetAttach(prv->preview, frame, svWidgetGetName(frame), 2);
        }
    }

    if (svSettingsIsWidgetDefined(ChannelNameBackgroundStr)) {
        SvWidget gradientBackground = QBFrameCreateFromSM(app, ChannelNameBackgroundStr);
        if (gradientBackground) {
            svSettingsWidgetAttach(prv->preview, gradientBackground, svWidgetGetName(gradientBackground), 2);
        }
    }

    if (svSettingsIsWidgetDefined(channelNameStr)) {
        prv->channelNameLabel = QBAsyncLabelNew(app, channelNameStr, prv->appGlobals->textRenderer);
        if (prv->channelNameLabel) {
            svSettingsWidgetAttach(prv->preview, prv->channelNameLabel, channelNameStr, 2);
        }
    }

    free(channelNameStr);
    free(previewFrameStr);
    free(ChannelNameBackgroundStr);

    w->prv = prv;
    w->clean = QBTVPreviewClean;

    QBTVPreviewChannelUpdated((SvGenericObject) prv, cursor);
    QBPlaylistCursorAddListener(cursor, (SvGenericObject) prv);

    return w;
}
