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

#include "QBPVROSD.h"

#include <libintl.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvType.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/icon.h>
#include <SWL/label.h>
#include <SWL/clock.h>
#include <Services/core/QBTextRenderer.h>
#include <Logic/TVOSDInterface.h>
#include <Logic/TVOSDPluginInterface.h>
#include <Windows/pvrplayerInternal.h>
#include <Widgets/osd.h>
#include <Widgets/QBTimeProgress.h>
#include <Widgets/QBTitle.h>
#include <Widgets/infoIcons.h>
#include <Windows/tv/QBTVChannelFrame.h>
#include <Widgets/OSD/QBPVROSDTSRecording.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBObserver.h>
#include <QBAppKit/QBObservable.h>
#include <QBSubsManager.h>
#include <math.h>
#include <Utils/QBEventUtils.h>
#include <QBOSD/QBOSD.h>
#include <QBOSD/QBOSDMain.h>
#include <QBOSD/QBOSDRecording.h>
#include <main.h>

struct QBPVROSD_ {
    struct QBOSDMain_ super_;

    SvWidget owner;
    SvApplication app;

    AppGlobals appGlobals;

    QBPVRPlayer player;

    SvWidget osdTop, osdBottom;
    SvWidget timeProgress;
    SvWidget title;
    SvWidget eventName;

    SvTVChannel channel;
    int channelFrameMinXOffset;
    int channelFrameSpacing;
    SvWidget channelFrame;
    SvWidget infoIcons;
    SvWidget cover;
    /** Plugin extends or change standard OSD functionality */
    SvObject plugin;

    int iconsSpacing;

    QBSubsManager subsManager;

    SvTimerId moveTimer;

    SvEPGEvent currentEvent;
    bool allowHide;
};

SvLocal bool QBPVROSDIsShown(SvGenericObject);
SvLocal void QBPVROSDShow(SvGenericObject, bool, bool force);
SvLocal void QBPVROSDHide(SvGenericObject, bool, bool force);
SvLocal void QBPVROSDUpdate(SvGenericObject);
SvLocal void QBPVROSDLock(SvGenericObject);
SvLocal void QBPVROSDUnlock(SvGenericObject, bool force);
SvLocal bool QBPVROSDIsShowingMoreInfo(SvGenericObject);
SvLocal void QBPVROSDShowMoreInfo(SvGenericObject);
SvLocal void QBPVROSDShowLessInfo(SvGenericObject);
SvLocal void QBPVROSDSetChannel(SvGenericObject, SvTVChannel, bool);
SvLocal void QBPVROSDSetTVChannelNumber(SvGenericObject, int, int, int);
SvLocal void QBPVROSDSetTVChannelName(SvGenericObject, SvString);
SvLocal void QBPVROSDSetTitle(SvGenericObject, SvString);
SvLocal void QBPVROSDSetCurrentEvent(SvGenericObject, const SvEPGEvent);
SvLocal void QBPVROSDSetTimeshiftedCurrentEvent(SvGenericObject, const SvEPGEvent);
SvLocal void QBPVROSDSetCaption(SvGenericObject, const SvString);
SvLocal void QBPVROSDSetFollowingEvent(SvGenericObject, const SvEPGEvent);
SvLocal void QBPVROSDSetTimeshiftedFollowingEvent(SvGenericObject, const SvEPGEvent);
SvLocal void QBPVROSDSetDataIntoEventFrame(SvGenericObject self, const SvGenericObject event, int frameIdx);
SvLocal void QBPVROSDSetSubsManager(SvGenericObject, const QBSubsManager);
SvLocal void QBPVROSDSetRecordingStatus(SvGenericObject, bool);
SvLocal void QBPVROSDSetPlayState(SvGenericObject, const SvPlayerTaskState*);
SvLocal void QBPVROSDSetPosition(SvGenericObject, double, double);
SvLocal void QBPVROSDSetEventSource(SvGenericObject, TVOSDEventSource, void *);
SvLocal void QBPVROSDCheckForUpdates(SvGenericObject, time_t, time_t, time_t);
SvLocal void QBPVROSDUpdateFormat(SvGenericObject, struct svdataformat*);
SvLocal void QBPVROSDSetCovervisibility(SvGenericObject self, bool visible);
SvLocal void QBPVROSDEnsureNonOverlapping(QBPVROSD);
SvLocal void QBPVROSDSetDescription(SvGenericObject self, SvString description);
SvLocal void QBPVROSDShowSideMenu(SvGenericObject self, SvGenericObject pane, SvString id);
SvLocal bool QBPVROSDIsShowingSideMenu(SvGenericObject self);
SvLocal SvGenericObject QBPVROSDGetSideMenuContext(SvGenericObject self);
SvLocal void QBPVROSDSetChannelSelectionCallback(SvGenericObject self, QBTVChannelFrameCallback callback, void *callbackData);
SvLocal bool QBPVROSDHandleInputEvent(SvGenericObject self, SvInputEvent ie);
SvLocal void QBPVROSDAttachNotificationContent(SvObject self, SvWidget notificationContent, const char* notificationContentName);
SvLocal void QBPVROSDShowNotification(SvObject self, bool immediate);
SvLocal void QBPVROSDHideNotification(SvObject self, bool immediate);
SvLocal void QBPVROSDSetAdvert(SvObject self, SvRBBitmap advert);
SvLocal void QBPVROSDClearAdvert(SvObject self);
SvLocal void QBPVROSDObjectUpdated(SvObject self, SvObject obsercerObject, SvObject arg);
SvLocal void QBPVROSDAttachOSD(QBOSDMain self, QBOSD osd, QBOSDMainAttachmentPosition position, int attachmentLevel, SvErrorInfo *errorOut);

SvLocal void
QBPVROSDDestroy(void *self_)
{
    QBPVROSD self = (QBPVROSD) self_;
    if (self->subsManager)
        QBObservableRemoveObserver((QBObservable) self->subsManager, (SvObject) self, NULL);
    SVTESTRELEASE(self->channel);
    SVTESTRELEASE(self->subsManager);
    SVTESTRELEASE(self->currentEvent);
    SVTESTRELEASE(self->plugin);
}

SvLocal void
QBPVROSDSetPlugin(SvObject self_, SvObject plugin)
{
    QBPVROSD self = (QBPVROSD) self_;
    if (!plugin || !SvObjectIsImplementationOf(plugin, TVOSDPlugin_getInterface())) {
        SvLogError("%s: passed incorrect plugin", __func__);
        return;
    }

    svAppTimerStop(self->app, self->moveTimer);
    self->plugin = plugin;
    svWidgetDestroy(self->cover);
    self->cover = NULL;
    SvInvokeInterface(TVOSDPlugin, self->plugin, start, self->owner);
}

SvLocal SvType
QBPVROSD_getType(void)
{
    static const struct QBOSDMainVTable_ vtable = {
        .super_          = {
            .super_      = {
                .destroy = QBPVROSDDestroy,
            }
        },
        .attachOSD       = QBPVROSDAttachOSD,
    };

    static struct TVOSD_ methods = {
        .isShown = QBPVROSDIsShown,
        .show = QBPVROSDShow,
        .hide = QBPVROSDHide,
        .update = QBPVROSDUpdate,
        .lock = QBPVROSDLock,
        .unlock = QBPVROSDUnlock,
        .isShowingMoreInfo = QBPVROSDIsShowingMoreInfo,
        .showMoreInfo = QBPVROSDShowMoreInfo,
        .showLessInfo = QBPVROSDShowLessInfo,
        .setTVChannel = QBPVROSDSetChannel,
        .setTVChannelNumber = QBPVROSDSetTVChannelNumber,
        .setTVChannelName = QBPVROSDSetTVChannelName,
        .setTitle = QBPVROSDSetTitle,
        .setDescription = QBPVROSDSetDescription,
        .setCurrentEvent = QBPVROSDSetCurrentEvent,
        .setTimeshiftedCurrentEvent = QBPVROSDSetTimeshiftedCurrentEvent,
        .setCurrentEventCaption = QBPVROSDSetCaption,
        .setFollowingEvent = QBPVROSDSetFollowingEvent,
        .setTimeshiftedFollowingEvent = QBPVROSDSetTimeshiftedFollowingEvent,
        .setDataIntoEventFrame = QBPVROSDSetDataIntoEventFrame,
        .setSubsManager = QBPVROSDSetSubsManager,
        .setRecordingStatus = QBPVROSDSetRecordingStatus,
        .setPlayState = QBPVROSDSetPlayState,
        .setPosition = QBPVROSDSetPosition,
        .setEventSource = QBPVROSDSetEventSource,
        .updateFormat = QBPVROSDUpdateFormat,
        .checkForUpdates = QBPVROSDCheckForUpdates,
        .setCoverVisibility = QBPVROSDSetCovervisibility,
        .showSideMenu = QBPVROSDShowSideMenu,
        .hideSideMenu = QBPVROSDShowLessInfo,
        .getSideMenuContext = QBPVROSDGetSideMenuContext,
        .isSideMenuShown = QBPVROSDIsShowingSideMenu,
        .setChannelSelectionCallback = QBPVROSDSetChannelSelectionCallback,
        .handleInputEvent = QBPVROSDHandleInputEvent,
        .attachNotificationContent = QBPVROSDAttachNotificationContent,
        .showNotification = QBPVROSDShowNotification,
        .hideNotification = QBPVROSDHideNotification,
        .setAdvert = QBPVROSDSetAdvert,
        .clearAdvert = QBPVROSDClearAdvert,
        .setPlugin = QBPVROSDSetPlugin
    };
    static const struct QBObserver_ observable = {
        .observedObjectUpdated = QBPVROSDObjectUpdated,
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBPVROSD", sizeof(struct QBPVROSD_),
                            QBOSDMain_getType(), &type,
                            QBOSDMain_getType(), &vtable,
                            TVOSD_getInterface(), &methods,
                            QBObserver_getInterface(), &observable,
                            NULL);
    }
    return type;
}

SvLocal void
QBPVROSDAttachOSD(QBOSDMain self, QBOSD osd, QBOSDMainAttachmentPosition position, int attachmentLevel, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self || !osd) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed [self = %p, osd = %p]",
                                  self, osd);
        goto out;
    } else if (position >= QBOSDMainAttachmentPosition_cnt) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "QBOSDMainAttachmentPosition out of range");
        goto out;
    }

    SvWidget osdWidget = NULL;
    switch (position) {
        case QBOSDMainAttachmentPosition_top:
            osdWidget = ((QBPVROSD) self)->osdTop;
            break;
        case QBOSDMainAttachmentPosition_bottom:
            osdWidget = ((QBPVROSD) self)->osdBottom;
            break;
        default:
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                      "underlying OSD widget not set");
            goto out;
    }

    QBOSDHandlerType type = QBOSDGetType(osd);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain,
                                           SvCoreError_invalidState,
                                           error,
                                           "QBOSDGetType call failed");
        goto out;
    }

    SvString id = QBOSDGetId(osd);
    if (!id) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain,
                                           SvCoreError_invalidState,
                                           error,
                                           "QBOSDGetId call failed");
        goto out;
    }

    if (self->osdTypesToHandlers[type]) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "handler [%s] for type [%d] already defined",
                                  SvStringCString(id), type);
        goto out;
    }

    SvWidget w = QBOSDTakeWidget(osd, &error);
    if (!w) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain,
                                           SvCoreError_invalidState,
                                           error,
                                           "QBOSDTake Widget call failed");
        goto out;
    }

    if (svSettingsWidgetAttach(osdWidget, w, SvStringCString(id), attachmentLevel) == 0) {
        svWidgetDestroy(w);
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "can't attach widget [%s]", SvStringCString(id));
        goto out;
    }

    self->osdTypesToHandlers[type] = SVRETAIN(osd);

out:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBPVROSDTimerEventHandler(SvWidget w, SvTimerEvent te)
{
    QBPVROSD prv = (QBPVROSD) w->prv;

    if (prv->cover && te->id == prv->moveTimer) {
        prv->cover->off_x = 2 * ((int)(0.025 * w->app->width) + (int) ((double)(0.45 * w->app->width - prv->cover->width * 0.5) * (rand() / (RAND_MAX + 1.0))));
        prv->cover->off_y = 2 * ((int) (0.025 * w->app->height) + (int) ((double)(0.45 * w->app->height - prv->cover->height * 0.5) * (rand() / (RAND_MAX + 1.0))));
        prv->moveTimer = svAppTimerStart(w->app, w, 10.0, 1);
        svWidgetForceRender(prv->cover);

        return;
    }
}

SvLocal void
OSDTitleUpdateFrameOffset(void *self_, SvWidget title)
{
    QBPVROSD prv = (QBPVROSD) self_;
    QBPVROSDEnsureNonOverlapping(prv);
}

QBPVROSD
QBPVROSDCreate(SvApplication app, AppGlobals appGlobals, void *userdata)
{
    QBPVRPlayer player = (QBPVRPlayer) userdata;
    SvWidget wgt = svWidgetCreate(app, player->window->width, player->window->height);
    QBPVROSD prv = (QBPVROSD) SvTypeAllocateInstance(QBPVROSD_getType(), NULL);
    QBOSDMainInit((QBOSDMain) prv, wgt, SVSTRING("OSDPVR"), NULL);

    prv->owner = wgt;
    prv->app = app;
    prv->appGlobals = appGlobals;
    prv->player = player;
    svWidgetSetTimerEventHandler(wgt, QBPVROSDTimerEventHandler);

    prv->osdTop = svOSDNew(app, "OSDTop");
    svOSDAttachToParent(prv->osdTop, wgt, 10);

    prv->osdBottom = svOSDNew(app, "OSDBottom");
    svOSDAttachToParent(prv->osdBottom, wgt, 10);

    SvWidget wClock = svClockNew(app, "clock");
    svSettingsWidgetAttach(prv->osdBottom, wClock, svWidgetGetName(wClock), 1);

    if (svSettingsIsWidgetDefined("customerLogo")) {
        SvWidget logo = svSettingsWidgetCreate(app, "customerLogo");
        svSettingsWidgetAttach(prv->osdTop, logo, svWidgetGetName(logo), 1);
    }

    prv->timeProgress = QBTimeProgressCreate(app, "progress", appGlobals->textRenderer);
    svSettingsWidgetAttach(prv->osdBottom, prv->timeProgress, "progress", 10);

    prv->title = QBTitleNew(app, "title", appGlobals->textRenderer);
    svSettingsWidgetAttach(prv->osdTop, prv->title, "title", 10);

    prv->channelFrameMinXOffset = svSettingsGetInteger("channelFrame", "xOffset", 0);
    prv->channelFrameSpacing = svSettingsGetInteger("channelFrame", "spacing", 0);

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    prv->channelFrame = QBTVChannelFrameNew(app, "channelFrame",
                                            appGlobals->textRenderer,
                                            playlists,
                                            appGlobals->tvLogic);

    QBTitleSetCallback(prv->title, OSDTitleUpdateFrameOffset, prv);

    svSettingsWidgetAttach(prv->osdTop, prv->channelFrame, "channelFrame", 10);
    svWidgetSetHidden(prv->channelFrame, true);

    if (player->ctx->rec || player->ctx->npvrRec || player->ctx->cutvEvent) {
        SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                             SVSTRING("SvEPGManager"));
        prv->infoIcons = QBInfoIconsCreate(app, "infoIcons",
                                           appGlobals->eventsLogic,
                                           epgManager,
                                           appGlobals->pc);
        svSettingsWidgetAttach(prv->osdTop, prv->infoIcons, "infoIcons", 10);
        prv->eventName = QBAsyncLabelNew(app, "eventName", appGlobals->textRenderer);
        svSettingsWidgetAttach(prv->osdBottom, prv->eventName, "eventName", 15);
    }

    if (player->ctx->title) {
        SvString titleStr = SvStringCreate(gettext(SvStringCString(player->ctx->title)), NULL);
        QBTitleSetText(prv->title, titleStr, 0);
        SVTESTRELEASE(titleStr);
    }

    prv->cover = svIconNew(app, "cover");
    if (prv->cover) {
        svSettingsWidgetAttach(wgt, prv->cover, "cover", 5);
        SvRID defaultBitmapRID = svSettingsGetResourceID("cover", "defaultBitmap");
        if (defaultBitmapRID != SV_RID_INVALID)
            svIconSetBitmapFromRID(prv->cover, 0, defaultBitmapRID);
        if (player->ctx->coverURI) {
            if (defaultBitmapRID == SV_RID_INVALID) {
                svIconSetBitmapFromURI(prv->cover, 0, SvStringCString(player->ctx->coverURI));
            } else {
                svIconSetBitmapFromURI(prv->cover, 1, SvStringCString(player->ctx->coverURI));
                svIconSwitch(prv->cover, 1, 0, 0.0);
            }
        }
        svWidgetSetHidden(prv->cover, true);
    }

    prv->moveTimer = svAppTimerStart(app, wgt, 10.0, 1);

    QBPVROSDEnsureNonOverlapping(prv);

    return prv;
}

SvLocal void
QBPVROSDEnsureNonOverlapping(QBPVROSD self)
{
    int titleSpan = self->title->off_x + self->title->width + self->channelFrameSpacing;
    if (titleSpan >= self->channelFrameMinXOffset)
        self->channelFrame->off_x = titleSpan;
    else
        self->channelFrame->off_x = self->channelFrameMinXOffset;
}

SvLocal bool QBPVROSDIsShown(SvGenericObject self)
{
    QBPVROSD prv = (QBPVROSD) self;

    return (svOSDIsVisible(prv->osdTop) || svOSDIsVisible(prv->osdBottom));
}

SvLocal void QBPVROSDShow(SvGenericObject self, bool immediate, bool force)
{
    QBPVROSD prv = (QBPVROSD) self;

    if (force)
        svOSDShow(prv->osdTop, immediate);
    svOSDShow(prv->osdBottom, immediate);
}

SvLocal void QBPVROSDHide(SvGenericObject self, bool immediate, bool force)
{
    QBPVROSD prv = (QBPVROSD) self;

    svOSDHide(prv->osdTop, immediate);
    if (force || prv->allowHide)
        svOSDHide(prv->osdBottom, immediate);
}

SvLocal void QBPVROSDUpdate(SvGenericObject self)
{
    QBPVROSD prv = (QBPVROSD) self;
    struct svdataformat* format = QBViewportGetOutputFormat(QBViewportGet());

    if (prv->infoIcons) {
        QBInfoIconsSetByTVChannel(prv->infoIcons, prv->channel);
        QBInfoIconsSetByViewport(prv->infoIcons, QBViewportGet());
        QBInfoIconsSetByFormat(prv->infoIcons, format);
        QBInfoIconsSetBySubsManager(prv->infoIcons, prv->subsManager);
    }
}

SvLocal void QBPVROSDLock(SvGenericObject self)
{
    QBPVROSD prv = (QBPVROSD) self;

    svOSDLock(prv->osdTop);
    svOSDLock(prv->osdBottom);
}

SvLocal void QBPVROSDUnlock(SvGenericObject self, bool force)
{
    QBPVROSD prv = (QBPVROSD) self;

    svOSDUnlock(prv->osdTop);
    if (force || prv->allowHide)
        svOSDUnlock(prv->osdBottom);
}

SvLocal bool QBPVROSDIsShowingMoreInfo(SvGenericObject self)
{
    QBPVROSD prv = (QBPVROSD) self;
    QBPVRPlayer player = prv->player;

    return player->sidemenu.shown;
}

SvLocal void QBPVROSDShowMoreInfo(SvGenericObject self)
{
    QBPVROSD prv = (QBPVROSD) self;
    QBPVRPlayer player = prv->player;

    player->sidemenu.shown = true;
    QBContextMenuPushPane(player->sidemenu.ctx, (SvGenericObject) player->sidemenu.content);
    QBContextMenuShow(player->sidemenu.ctx);
}

SvLocal void QBPVROSDShowLessInfo(SvGenericObject self)
{
    QBPVROSD prv = (QBPVROSD) self;
    QBPVRPlayer player = prv->player;

    if (player->sidemenu.shown)
        QBContextMenuHide(player->sidemenu.ctx, false);
}

SvLocal void QBPVROSDShowSideMenu(SvGenericObject self, SvGenericObject pane, SvString id)
{
    QBPVROSDShowMoreInfo(self);
}

SvLocal bool QBPVROSDIsShowingSideMenu(SvGenericObject self)
{
    QBPVROSD prv = (QBPVROSD) self;
    QBPVRPlayer player = prv->player;

    return player->sidemenu.shown;
}

SvLocal void QBPVROSDSetChannelSelectionCallback(SvGenericObject self, QBTVChannelFrameCallback callback, void *callbackData)
{
}

SvLocal bool QBPVROSDHandleInputEvent(SvGenericObject self, SvInputEvent ie)
{
    return false;
}

SvLocal SvGenericObject QBPVROSDGetSideMenuContext(SvGenericObject self)
{
    QBPVROSD prv = (QBPVROSD) self;
    QBPVRPlayer player = prv->player;

    return (SvGenericObject) player->sidemenu.ctx;
}

SvLocal void QBPVROSDSetChannel(SvGenericObject self, SvTVChannel channel, bool isFavorite)
{
    QBPVROSD prv = (QBPVROSD) self;

    if (channel != prv->channel) {
        SVTESTRELEASE(prv->channel);
        prv->channel = SVTESTRETAIN(channel);
    }
}

SvLocal void QBPVROSDSetTVChannelNumber(SvGenericObject self, int number, int digits, int digitsMax)
{
}

SvLocal void QBPVROSDSetTVChannelName(SvGenericObject self, SvString name)
{
    QBPVROSD prv = (QBPVROSD) self;

    svWidgetSetHidden(prv->channelFrame, false);
    QBTVChannelFrameSetChannelName(prv->channelFrame, name);
}

SvLocal void QBPVROSDSetTitle(SvGenericObject self, SvString title)
{
    QBPVROSD prv = (QBPVROSD) self;

    QBTitleSetText(prv->title, title, 1);
}

SvLocal void QBPVROSDSetCurrentEvent(SvGenericObject self, const SvEPGEvent event)
{
    QBPVROSD prv = (QBPVROSD) self;

    if (event != prv->currentEvent) {
        SVTESTRELEASE(prv->currentEvent);
        prv->currentEvent = SVTESTRETAIN(event);
    }

    QBAsyncLabelSetText(prv->eventName, QBEventUtilsGetTitleFromEvent(prv->appGlobals->eventsLogic, prv->appGlobals->langPreferences, event));
    if (prv->infoIcons)
        QBInfoIconsSetByEvent(prv->infoIcons, event);
}

SvLocal void QBPVROSDSetTimeshiftedCurrentEvent(SvGenericObject self, const SvEPGEvent event)
{
}

SvLocal void QBPVROSDSetCaption(SvGenericObject self, const SvString caption)
{
}

SvLocal void QBPVROSDSetSubsManager(SvGenericObject self, const QBSubsManager subsManager)
{
    QBPVROSD prv = (QBPVROSD) self;

    if (prv->subsManager == subsManager)
        return;

    if (prv->subsManager) {
        QBObservableRemoveObserver((QBObservable) prv->subsManager, (SvObject) self, NULL);
        SVRELEASE(prv->subsManager);
    }

    QBObservableAddObserver((QBObservable) subsManager, (SvObject) self, NULL);
    prv->subsManager = SVTESTRETAIN(subsManager);
}

SvLocal void QBPVROSDSetFollowingEvent(SvGenericObject self, const SvEPGEvent followingEvent)
{
}

SvLocal void QBPVROSDSetTimeshiftedFollowingEvent(SvGenericObject self, const SvEPGEvent event)
{
}

SvLocal void QBPVROSDSetDataIntoEventFrame(SvGenericObject self, const SvGenericObject event, int frameIdx)
{
    if (SvObjectIsInstanceOf(event, SvEPGEvent_getType())) {
        if (frameIdx == 0) {
            QBPVROSDSetCurrentEvent(self, (SvEPGEvent) event);
        } else if (frameIdx == 1) {
            QBPVROSDSetFollowingEvent(self, (SvEPGEvent) event);
        }
    }
}

SvLocal void QBPVROSDSetRecordingStatus(SvGenericObject self, bool isRecording)
{
}

SvLocal void QBPVROSDSetPlayState(SvGenericObject self, const SvPlayerTaskState* state)
{
}

SvLocal void QBPVROSDSetPosition(SvGenericObject self, double position, double end)
{
    QBPVROSD prv = (QBPVROSD) self;

    svWidgetSetHidden(prv->timeProgress, !prv->player->ctx->canSeek);
    if (prv->player->ctx->canSeek) {
        if (end >= 0)
            QBTimeProgressSetMaxTime(prv->timeProgress, end);
        if (position >= 0)
            QBTimeProgressSetCurrentTime(prv->timeProgress, position);
    }
}

SvLocal void QBPVROSDSetEventSource(SvGenericObject self, TVOSDEventSource func, void *userdata)
{
}

SvLocal void QBPVROSDUpdateFormat(SvGenericObject self, struct svdataformat* format)
{
    QBPVROSD prv = (QBPVROSD) self;
    if (prv->infoIcons)
        QBInfoIconsSetByFormat(prv->infoIcons, format);
}

SvLocal void QBPVROSDCheckForUpdates(SvGenericObject self, time_t currentTime, time_t timeshiftStart, time_t timeshiftEnd)
{
    QBPVROSD prv = (QBPVROSD) self;

    if (timeshiftStart == 0 || !prv->currentEvent) {
        QBTimeProgressHideAvailableContent(prv->timeProgress);
        return;
    }
    time_t upperBound, lowerBound;
    time_t tss, tse;
    upperBound = prv->currentEvent->endTime;
    lowerBound = prv->currentEvent->startTime;
    tss = (timeshiftStart < lowerBound) ? lowerBound : timeshiftStart;
    tse = (timeshiftEnd > upperBound) ? upperBound : timeshiftEnd;

    upperBound -= lowerBound;
    tss -= lowerBound;
    tse -= lowerBound;

    QBTimeProgressSetAvailableContent(prv->timeProgress, tss, tse);
    QBTimeProgressShowAvailableContent(prv->timeProgress);
}

SvLocal void QBPVROSDSetCovervisibility(SvGenericObject self, bool visible)
{
    QBPVROSD prv = (QBPVROSD) self;
    if (prv->cover)
        svWidgetSetHidden(prv->cover, !visible);
}

SvLocal void QBPVROSDSetDescription(SvGenericObject self, SvString description)
{
}

SvLocal void QBPVROSDAttachNotificationContent(SvObject self, SvWidget notificationContent, const char* notificationContentName)
{
}

SvLocal void QBPVROSDShowNotification(SvObject self, bool immediate)
{
}

SvLocal void QBPVROSDHideNotification(SvObject self, bool immediate)
{
}

SvLocal void QBPVROSDSetAdvert(SvObject self, SvRBBitmap advert)
{
}

SvLocal void QBPVROSDClearAdvert(SvObject self)
{
}

SvLocal void QBPVROSDObjectUpdated(SvObject self, SvObject observedObject, SvObject arg)
{
    QBPVROSD prv = (QBPVROSD) self;
    QBPVROSDTSRecording osd = (QBPVROSDTSRecording) QBOSDMainGetHandler((QBOSDMain) prv, QBOSDHandlerType_recording, NULL);
    if (prv->subsManager == (QBSubsManager) observedObject) {
        QBSubsManager subsManager = (QBSubsManager) observedObject;
        if (prv->infoIcons)
            QBInfoIconsSetBySubsManager(prv->infoIcons, subsManager);
    } else if (osd && osd == (QBPVROSDTSRecording) observedObject
               && SvObjectIsInstanceOf(observedObject, QBPVROSDTSRecording_getType())) {
        QBOSDState state = QBOSDGetState((QBOSD) osd, NULL);
        prv->allowHide = state.allowHide;
    }
}
