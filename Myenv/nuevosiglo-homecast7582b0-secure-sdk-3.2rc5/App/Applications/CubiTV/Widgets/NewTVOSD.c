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

#include "NewTVOSD.h"

#include <SvCore/SvCoreErrorDomain.h>
#include <libintl.h>
#include <SvFoundation/SvType.h>
#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <Services/core/QBTextRenderer.h>
#include <Logic/TVOSDInterface.h>
#include <settings.h>
#include <Widgets/teleosd.h>
#include <Widgets/osd.h>
#include <Widgets/extendedInfo.h>
#include <Windows/newtvInternal.h>
#include <SWL/clock.h>
#include <SWL/icon.h>
#include <ContextMenus/QBReminderEditorPane.h>
#include <Services/QBCASManager.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <QBAppKit/QBObserver.h>
#include <QBAppKit/QBObservable.h>
#include <QBSubsManager.h>
#include <QBOSD/QBOSD.h>
#include <QBOSD/QBOSDMain.h>
#include <QBOSD/QBOSDRecording.h>
#include <Widgets/OSD/QBOSDTSRecording.h>
#include <main.h>

struct QBNewTVOSD_ {
    struct QBOSDMain_ super_;

    unsigned int settingsCtx;

    SvApplication app;
    QBAdService adService;

    SvWidget osdTop, osdNotification, osdBottom;
    SvWidget tvOsd;
    SvWidget clock;
    SvWidget advert;
    SvTVContext tvContext;

    SvTVChannel channel;
    SvEPGEvent currentEvent, timeshiftedCurrentEvent;
    SvEPGEvent followingEvent, timeshiftedFollowingEvent;
    QBSubsManager subsManager;

    struct {
        QBContainerPane content;
        bool shown;
    } moreInfo;

    struct {
        QBContextMenu ctx;
        bool shown;
    } sidemenu;

    bool allowHide;
};

SvLocal bool NewTVOSDIsShown(SvGenericObject);
SvLocal void NewTVOSDShow(SvGenericObject, bool, bool force);
SvLocal void NewTVOSDHide(SvGenericObject, bool, bool force);
SvLocal void NewTVOSDLock(SvGenericObject);
SvLocal void NewTVOSDUnlock(SvGenericObject, bool force);
SvLocal bool NewTVOSDIsShowingMoreInfo(SvGenericObject);
SvLocal void NewTVOSDShowMoreInfo(SvGenericObject);
SvLocal void NewTVOSDShowLessInfo(SvGenericObject);
SvLocal void NewTVOSDSetChannel(SvGenericObject, SvTVChannel, bool);
SvLocal void NewTVOSDSetTVChannelName(SvGenericObject self, SvString name);
SvLocal void NewTVOSDSetTitle(SvGenericObject self, SvString title);
SvLocal void NewTVSetDescription(SvGenericObject self, SvString description);
SvLocal void NewTVOSDSetCurrentEvent(SvGenericObject, const SvEPGEvent);
SvLocal void NewTVOSDSetTimeshiftedCurrentEvent(SvGenericObject, const SvEPGEvent);
SvLocal void NewTVOSDSetFollowingEvent(SvGenericObject, const SvEPGEvent);
SvLocal void NewTVOSDSetTimeshiftedFollowingEvent(SvGenericObject, const SvEPGEvent);
SvLocal void NewTVOSDSetDataIntoEventFrame(SvGenericObject self, const SvGenericObject data, int frameIdx);
SvLocal void NewTVOSDSetSubsManager(SvGenericObject, const QBSubsManager);
SvLocal void NewTVOSDSetRecordingStatus(SvGenericObject, bool);
SvLocal void NewTVOSDUpdateFormat(SvGenericObject self, struct svdataformat* format);
SvLocal void NewTVOSDShowSideMenu(SvGenericObject self, SvGenericObject pane, SvString id);
SvLocal void NewTVOSDHideSideMenu(SvGenericObject self);
SvLocal bool NewTVOSDIsShowingSideMenu(SvGenericObject self);
SvLocal SvGenericObject NewTVOSDGetSideMenuContext(SvGenericObject self);
SvLocal void NewTVOSDSetChannelSelectionCallback(SvGenericObject self, QBTVChannelFrameCallback callback, void *callbackData);
SvLocal bool NewTVOSDHandleInputEvent(SvGenericObject self, SvInputEvent ie);
SvLocal void NewTVOSDAttachNotificationContent(SvObject self, SvWidget notificationContent, const char* notificationContentName);
SvLocal void NewTVOSDShowNotification(SvObject self, bool immediate);
SvLocal void NewTVOSDHideNotification(SvObject self, bool immediate);
SvLocal void NewTVOSDSetAdvert(SvObject self, SvRBBitmap advert);
SvLocal void NewTVOSDClearAdvert(SvObject self);
SvLocal void NewTVOSDSubsManagerUpdated(SvObject self, SvObject observerObject, SvObject arg);
SvLocal TVInfo *TVInfoFromTVContext(const SvTVContext tvContext);

SvLocal void
NewTVOSDDestroy(void *self_)
{
    QBNewTVOSD self = (QBNewTVOSD) self_;

    SVTESTRELEASE(self->channel);
    SVTESTRELEASE(self->currentEvent);
    SVTESTRELEASE(self->timeshiftedCurrentEvent);
    SVTESTRELEASE(self->followingEvent);
    SVTESTRELEASE(self->timeshiftedFollowingEvent);
    SVTESTRELEASE(self->subsManager);

    SVRELEASE(self->moreInfo.content);
    SVRELEASE(self->sidemenu.ctx);

    SVTESTRELEASE(self->adService);
}

// TVOSD interface mathods (empty)

SvLocal void
NewTVOSDCheckForUpdates(SvGenericObject self, time_t currentTime, time_t timeshiftStart, time_t timeshiftEnd)
{
}

SvLocal void
NewTVOSDSetEventSource(SvGenericObject self, TVOSDEventSource func, void *userdata)
{
}

SvLocal void
NewTVOSDSetPlayState(SvGenericObject self, const SvPlayerTaskState* state)
{
}

SvLocal void
NewTVOSDUpdate(SvGenericObject self)
{
}

SvLocal void
NewTVOSDSetTVChannelNumber(SvGenericObject self, int number, int digits, int digitsMax)
{
}

SvLocal void
NewTVOSDSetCaption(SvGenericObject self, const SvString caption)
{
}

SvLocal void
NewTVOSDSetPosition(SvGenericObject self, double position, double end)
{
}

SvLocal void
NewTVOSDSetCoverVisibility(SvGenericObject self, bool visible)
{
}

// QBOSDMain virtual methods

SvLocal void
NewTVOSDAttachOSD(QBOSDMain self,
                  QBOSD osd,
                  QBOSDMainAttachmentPosition position,
                  int attachmentLevel,
                  SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!osd) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed [osd = %p]", osd);
        goto out;
    } else if ((int) position < 0 || position >= QBOSDMainAttachmentPosition_cnt) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "QBOSDMainAttachmentPosition out of range");
        goto out;
    }

    SvWidget osdWidget = NULL;
    switch (position) {
        case QBOSDMainAttachmentPosition_top:
            osdWidget = ((QBNewTVOSD) self)->osdTop;
            break;
        case QBOSDMainAttachmentPosition_bottom:
            osdWidget = ((QBNewTVOSD) self)->osdBottom;
            break;
        default:
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                      "underlying OSD widget not set");
            goto out;
    }

    QBOSDHandlerType type = QBOSDGetType(osd);
    SvString id = QBOSDGetId(osd);

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
QBNewTVOSDSetPlugin(SvObject self, SvObject product)
{
}

SvLocal void
NewTVOSDHandlePeerEvent(SvObject self_, QBPeerEvent event_, SvObject sender)
{
    QBNewTVOSD self = (QBNewTVOSD) self_;

    if (SvObjectIsInstanceOf((SvObject) event_, SvOSDStateChangedEvent_getType()) && sender == (SvObject) self->osdBottom->prv) {
        SvOSDStateChangedEvent event = (SvOSDStateChangedEvent) event_;
        TVInfo *tvInfo = TVInfoFromTVContext(self->tvContext);
        if (!self->adService)
            return;
        if (event->OSDIsVisible) {
            SvWidget ad = QBAdServiceGetAd(self->adService, NULL);
            QBAdWindowSetAd(tvInfo->adWindow, ad);
            QBAdWindowShow(tvInfo->adWindow);
        } else {
            QBAdWindowHide(tvInfo->adWindow);
        }
        return;
    }
}

SvLocal SvType
QBNewTVOSD_getType(void)
{
    static const struct QBOSDMainVTable_ vtable = {
        .super_      = {
            .super_  = {
                .destroy = NewTVOSDDestroy,
            }
        },
        .attachOSD   = NewTVOSDAttachOSD,
    };

    static struct TVOSD_ methods = {
        .attachNotificationContent = NewTVOSDAttachNotificationContent,
        .checkForUpdates = NewTVOSDCheckForUpdates,
        .clearAdvert = NewTVOSDClearAdvert,
        .getSideMenuContext = NewTVOSDGetSideMenuContext,
        .handleInputEvent = NewTVOSDHandleInputEvent,
        .hide = NewTVOSDHide,
        .hideNotification = NewTVOSDHideNotification,
        .hideSideMenu = NewTVOSDHideSideMenu,
        .isShowingMoreInfo = NewTVOSDIsShowingMoreInfo,
        .isShown = NewTVOSDIsShown,
        .isSideMenuShown = NewTVOSDIsShowingSideMenu,
        .lock = NewTVOSDLock,
        .setAdvert = NewTVOSDSetAdvert,
        .setChannelSelectionCallback = NewTVOSDSetChannelSelectionCallback,
        .setCoverVisibility = NewTVOSDSetCoverVisibility,
        .setCurrentEvent = NewTVOSDSetCurrentEvent,
        .setCurrentEventCaption = NewTVOSDSetCaption,
        .setDataIntoEventFrame = NewTVOSDSetDataIntoEventFrame,
        .setDescription = NewTVSetDescription,
        .setEventSource = NewTVOSDSetEventSource,
        .setFollowingEvent = NewTVOSDSetFollowingEvent,
        .setPlayState = NewTVOSDSetPlayState,
        .setPlugin = QBNewTVOSDSetPlugin,
        .setPosition = NewTVOSDSetPosition,
        .setRecordingStatus = NewTVOSDSetRecordingStatus,
        .setSubsManager = NewTVOSDSetSubsManager,
        .setTVChannel = NewTVOSDSetChannel,
        .setTVChannelName = NewTVOSDSetTVChannelName,
        .setTVChannelNumber = NewTVOSDSetTVChannelNumber,
        .setTimeshiftedCurrentEvent = NewTVOSDSetTimeshiftedCurrentEvent,
        .setTimeshiftedFollowingEvent = NewTVOSDSetTimeshiftedFollowingEvent,
        .setTitle = NewTVOSDSetTitle,
        .show = NewTVOSDShow,
        .showLessInfo = NewTVOSDShowLessInfo,
        .showMoreInfo = NewTVOSDShowMoreInfo,
        .showNotification = NewTVOSDShowNotification,
        .showSideMenu = NewTVOSDShowSideMenu,
        .unlock = NewTVOSDUnlock,
        .update = NewTVOSDUpdate,
        .updateFormat = NewTVOSDUpdateFormat,
    };

    static const struct QBObserver_ observableMethods = {
        .observedObjectUpdated = NewTVOSDSubsManagerUpdated,
    };

    static const struct QBPeerEventReceiver_ eventReceiverMethods = {
        .handleEvent = NewTVOSDHandlePeerEvent
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNewTVOSD", sizeof(struct QBNewTVOSD_),
                            QBOSDMain_getType(), &type,
                            QBOSDMain_getType(), &vtable,
                            TVOSD_getInterface(), &methods,
                            QBObserver_getInterface(), &observableMethods,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            NULL);
    }

    return type;
}

SvLocal void
QBNewTVOSDOnSideMenuClose(void *self_, QBContextMenu menu)
{
    QBNewTVOSD prv = (QBNewTVOSD) self_;
    prv->sidemenu.shown = false;
    prv->moreInfo.shown = false;
    NewTVOSDUnlock((SvObject) prv, false);
}

SvLocal void
QBNewTVOSDOnSideMenuShow(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBNewTVOSD prv = (QBNewTVOSD) ptr;
    TVInfo *tvInfo = TVInfoFromTVContext(prv->tvContext);
    svSettingsPushComponent("TVExtendedInfo.settings");
    svSettingsWidgetAttach(frame, tvInfo->extendedInfo,"ExtendedInfo", 1);
    svSettingsPopComponent();
}

SvLocal void
QBNewTVOSDSetSideMenuActive(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBNewTVOSD prv = (QBNewTVOSD) ptr;
    TVInfo *tvInfo = TVInfoFromTVContext(prv->tvContext);
    svWidgetSetFocus(tvInfo->extendedInfo);
    QBExtendedInfoResetPosition(tvInfo->extendedInfo);
}
SvLocal void
QBNewTVOSDOnSideMenuHide(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBNewTVOSD prv = (QBNewTVOSD) ptr;
    TVInfo *tvInfo = TVInfoFromTVContext(prv->tvContext);
    svWidgetDetach(tvInfo->extendedInfo);
}

SvLocal bool
QBNewTVOSDSideMenuGlobalInputEventHandler(void *self_, SvGenericObject src, SvInputEvent e)
{
    if (e->ch == QBKEY_UP || e->ch == QBKEY_DOWN) {
        return true;
    }
    return false;
}

QBNewTVOSD
QBNewTVOSDCreate(const QBNewTVOSDParams *params)
{
    SvWidget w = svSettingsWidgetCreate(params->app, params->widgetName);
    QBNewTVOSD self = (QBNewTVOSD) SvTypeAllocateInstance(QBNewTVOSD_getType(), NULL);


    SvString id = SvStringCreate(params->widgetName, NULL);
    QBOSDMainInit((QBOSDMain) self, w, id, NULL);
    SVRELEASE(id);

    self->settingsCtx = svSettingsSaveContext();
    self->app = params->app;
    self->adService = SVTESTRETAIN(params->adService);
    self->tvContext = (SvTVContext) params->tvContext;
    self->subsManager = SVRETAIN(self->tvContext->subsManager);
    QBObservableAddObserver((QBObservable) self->subsManager, (SvObject) self, NULL);

    QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));

    self->osdTop = svOSDNew(params->app, "OSD");
    svOSDAttachToParent(self->osdTop, w, 10);
    QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, SvOSDStateChangedEvent_getType(), self->osdTop->prv, NULL);

    self->osdBottom = svOSDNew(params->app, "OSDTS");
    svOSDAttachToParent(self->osdBottom, w, 10);
    QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, SvOSDStateChangedEvent_getType(), self->osdBottom->prv, NULL);

    self->tvOsd = svTVOSDNew(params->app, "TVOSD", params->textRenderer,
                             params->playlists, params->viewport,
                             params->eventsLogic, params->tvLogic, params->pc);
    svWidgetAttach(self->osdTop, self->tvOsd, 0, 0, 1);

    self->clock = svClockNew(params->app, "clock");
    svSettingsWidgetAttach(self->osdBottom, self->clock, svWidgetGetName(self->clock), 1);

    self->sidemenu.ctx = QBContextMenuCreateFromSettings("TVContextMenu.settings",
                                                         params->controller,
                                                         params->app,
                                                         SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBNewTVOSDOnSideMenuClose, self);
    self->moreInfo.content = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);

    static struct QBContainerPaneCallbacks_t moreInfoCallbacks = {
            .onShow = QBNewTVOSDOnSideMenuShow,
            .onHide = QBNewTVOSDOnSideMenuHide,
            .setActive = QBNewTVOSDSetSideMenuActive
    };
    svSettingsPushComponent("TVExtendedInfo.settings");
    QBContainerPaneInit(self->moreInfo.content, params->app,
                        self->sidemenu.ctx, 1, SVSTRING("ExtendedInfo"),
                        &moreInfoCallbacks, self);
    svSettingsPopComponent();

    QBContextMenuSetInputEventHandler(self->sidemenu.ctx, NULL, QBNewTVOSDSideMenuGlobalInputEventHandler);

    return self;
}

SvLocal bool NewTVOSDIsShown(SvGenericObject self)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    return (svOSDIsVisible(prv->osdTop) || svOSDIsVisible(prv->osdBottom));
}

SvLocal void NewTVOSDShow(SvGenericObject self, bool immediate, bool force)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    svOSDShow(prv->osdTop, immediate);
    svTVOsdSetInfo(prv->tvOsd, prv->channel, prv->currentEvent, prv->subsManager);
    svOSDShow(prv->osdBottom, immediate);
}

SvLocal void NewTVOSDHide(SvGenericObject self, bool immediate, bool force)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    svOSDHide(prv->osdTop, immediate);
    if (force || prv->allowHide)
        svOSDHide(prv->osdBottom, immediate);
}

SvLocal void NewTVOSDLock(SvGenericObject self)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    svOSDLock(prv->osdTop);
    if (prv->osdNotification)
        svOSDLock(prv->osdNotification);
    svOSDLock(prv->osdBottom);
}

SvLocal void NewTVOSDUnlock(SvGenericObject self, bool force)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    svOSDUnlock(prv->osdTop);
    if (prv->osdNotification)
        svOSDUnlock(prv->osdNotification);
    if (force || prv->allowHide)
        svOSDUnlock(prv->osdBottom);
}

SvLocal TVInfo *TVInfoFromTVContext(const SvTVContext tvContext)
{
    if (!tvContext || !tvContext->super_.window)
        return NULL;
    return tvContext->super_.window->prv;
}

SvLocal bool NewTVOSDIsShowingMoreInfo(SvGenericObject self)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    return prv->moreInfo.shown;
}

SvLocal void NewTVOSDShowMoreInfo(SvGenericObject self)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    if (!prv->currentEvent && !prv->followingEvent) {
        return;
    }

    SvTVContext ctx = prv->tvContext;
    TVInfo *tvInfo = TVInfoFromTVContext(ctx);

    prv->sidemenu.shown = true;
    prv->moreInfo.shown = true;
    SvArray events = SvArrayCreate(NULL);
    if (prv->currentEvent) {
        SvArrayAddObject(events, (SvGenericObject) prv->currentEvent);
    }
    if (prv->followingEvent) {
        SvArrayAddObject(events, (SvGenericObject) prv->followingEvent);
    }
    QBExtendedInfoSetEvents(tvInfo->extendedInfo, events);
    SVRELEASE(events);

    QBContextMenuPushPane(prv->sidemenu.ctx, (SvGenericObject) prv->moreInfo.content);
    QBContextMenuShow(prv->sidemenu.ctx);
}

SvLocal void NewTVOSDShowLessInfo(SvGenericObject self)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    if (prv->moreInfo.shown) {
        QBContextMenuHide(prv->sidemenu.ctx, false);
    }
}

SvLocal void NewTVOSDShowSideMenu(SvGenericObject self, SvGenericObject pane, SvString id)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    if (pane) {
        QBContextMenuPushPane(prv->sidemenu.ctx, (SvGenericObject) pane);
        if (SvObjectIsInstanceOf(pane, QBReminderEditorPane_getType())) {
            SvString posId = id ? id : SVSTRING("");
            QBReminderEditorPaneSetPosition((QBReminderEditorPane) pane, posId, true);
        }
    }
    prv->sidemenu.shown = true;
    QBContextMenuShow(prv->sidemenu.ctx);
}

SvLocal void NewTVOSDHideSideMenu(SvGenericObject self)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    if (prv->sidemenu.shown) {
        QBContextMenuHide(prv->sidemenu.ctx, false);
    }
}

SvLocal bool NewTVOSDIsShowingSideMenu(SvGenericObject self)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    return prv->sidemenu.shown;
}

SvLocal void NewTVOSDSetChannelSelectionCallback(SvGenericObject self, QBTVChannelFrameCallback callback, void *callbackData)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    svTVOSDSetChannelFrameCallback(prv->tvOsd, callback, callbackData);
}

SvLocal bool NewTVOSDHandleInputEvent(SvGenericObject self, SvInputEvent ie)
{
    QBNewTVOSD prv = (QBNewTVOSD) self;
    return svTVOSDInputEventHandler(prv->tvOsd, ie->fullInfo);
}

SvLocal SvGenericObject NewTVOSDGetSideMenuContext(SvGenericObject self)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    return (SvGenericObject) prv->sidemenu.ctx;
}

SvLocal void NewTVOSDSetChannel(SvGenericObject self, SvTVChannel channel, bool isFavorite)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    SVTESTRELEASE(prv->channel);
    prv->channel = SVTESTRETAIN(channel);
    svTVOsdSetTVChannel(prv->tvOsd, channel, isFavorite);
}

SvLocal void NewTVOSDSetTVChannelName(SvGenericObject self, SvString name)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    svTVOsdSetChannelName(prv->tvOsd, name);
}

SvLocal void NewTVOSDSetTitle(SvGenericObject self, SvString title)
{
}

SvLocal void NewTVSetDescription(SvGenericObject self, SvString description)
{
}

SvLocal void NewTVOSDSetCurrentEvent(SvGenericObject self, const SvEPGEvent event)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    bool hasChanged = event != prv->currentEvent;
    SVTESTRELEASE(prv->currentEvent);
    prv->currentEvent = SVTESTRETAIN(event);
    if (hasChanged) {
        SvWidget tvOSD = prv->tvOsd;
        svTVOsdSetInfoIconsByEvent(tvOSD, prv->currentEvent);
    }
}

SvLocal void NewTVOSDSetTimeshiftedCurrentEvent(SvGenericObject self, const SvEPGEvent event)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    SVTESTRELEASE(prv->timeshiftedCurrentEvent);
    prv->timeshiftedCurrentEvent = SVTESTRETAIN(event);
}

SvLocal void NewTVOSDSetSubsManager(SvGenericObject self, const QBSubsManager subsManager)
{
    QBNewTVOSD prv = (QBNewTVOSD) self;

    if (prv->subsManager == subsManager)
        return;

    if (prv->subsManager) {
        QBObservableRemoveObserver((QBObservable) prv->subsManager, (SvObject) self, NULL);
        SVRELEASE(prv->subsManager);
    }

    prv->subsManager = SVRETAIN(subsManager);
    QBObservableAddObserver((QBObservable) prv->subsManager, (SvObject) self, NULL);
}

SvLocal void NewTVOSDUpdateFormat(SvGenericObject self, struct svdataformat* format)
{
    QBNewTVOSD prv = (QBNewTVOSD) self;
    SvTVOsdSetInfoIconsByFormat(prv->tvOsd, format);
}

SvLocal void NewTVOSDSetFollowingEvent(SvGenericObject self, const SvEPGEvent followingEvent)
{
    QBNewTVOSD prv = (QBNewTVOSD) self;

    SVTESTRELEASE(prv->followingEvent);
    prv->followingEvent = SVTESTRETAIN(followingEvent);
}

SvLocal void NewTVOSDSetTimeshiftedFollowingEvent(SvGenericObject self, const SvEPGEvent event)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;

    SVTESTRELEASE(prv->timeshiftedFollowingEvent);
    prv->timeshiftedFollowingEvent = SVTESTRETAIN(event);
}

SvLocal void NewTVOSDSetDataIntoEventFrame(SvGenericObject self, const SvGenericObject data, int frameIdx) {
    if (!data || SvObjectIsInstanceOf(data, SvEPGEvent_getType())) {
        if (frameIdx == 0) {
            NewTVOSDSetCurrentEvent(self, (SvEPGEvent) data);
        } else if (frameIdx == 1) {
            NewTVOSDSetFollowingEvent(self, (SvEPGEvent) data);
        }
    }
}

SvLocal void NewTVOSDSetRecordingStatus(SvGenericObject self, bool isRecording)
{
    QBNewTVOSD prv = (QBNewTVOSD)self;
    TVInfo *tvInfo = TVInfoFromTVContext(prv->tvContext);

    bool statusChanged = false;

    if (tvInfo->isRecording && !isRecording) {
        tvInfo->isRecording = false;
        statusChanged = true;
    } else if (!tvInfo->isRecording && isRecording) {
        tvInfo->isRecording = true;
        statusChanged = true;
    }

    if (statusChanged) {
        svTVOsdSetRecordingStatus(prv->tvOsd, isRecording);
        NewTVOSDShow(self, false, true);
    }
}

SvLocal void NewTVOSDAttachNotificationContent(SvObject self_, SvWidget notificationContent, const char *notificationContentName)
{
    QBNewTVOSD self = (QBNewTVOSD) self_;

    if (self->osdNotification) {
        svWidgetDestroy(self->osdNotification);
        self->osdNotification = NULL;
    }

    if (!notificationContent) {
        return;
    }

    svSettingsRestoreContext(self->settingsCtx);

    const char* const notificationWidgetName = "OSD.Notification";
    self->osdNotification = svOSDNew(self->app, notificationWidgetName);
    svOSDAttachToParent(self->osdNotification, self->osdTop, 0);

    QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
    QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, SvOSDStateChangedEvent_getType(), self->osdTop->prv, NULL);

    svSettingsPopComponent();

    svSettingsWidgetAttach(self->osdNotification, notificationContent, notificationContentName, 0);
}

SvLocal void NewTVOSDShowNotification(SvObject self, bool immediate)
{
    QBNewTVOSD prv = (QBNewTVOSD) self;
    if (prv->osdNotification) {
        svOSDShow(prv->osdNotification, immediate);
    }
}

SvLocal void NewTVOSDHideNotification(SvObject self, bool immediate)
{
    QBNewTVOSD prv = (QBNewTVOSD) self;
    if (prv->osdNotification) {
        svOSDHide(prv->osdNotification, immediate);
    }
}

SvLocal void NewTVOSDSetAdvert(SvObject self, SvRBBitmap advertBitmap)
{
    QBNewTVOSD prv = (QBNewTVOSD) self;

    if (!advertBitmap) {
        NewTVOSDClearAdvert(self);
        return;
    }

    svSettingsRestoreContext(prv->settingsCtx);

    const char* const advertName = "OSDTS.Advert";
    if (svSettingsIsWidgetDefined(advertName)) {
        if (!prv->advert) {
            prv->advert = svIconNew(prv->app, advertName);
            svSettingsWidgetAttach(prv->osdBottom, prv->advert, advertName, 10);
        }
        svIconSetBitmapFromRID(prv->advert, 0, SvRBObjectGetID((SvRBObject) advertBitmap));
    }

    svSettingsPopComponent();
}

SvLocal void NewTVOSDClearAdvert(SvObject self)
{
    QBNewTVOSD prv = (QBNewTVOSD) self;
    if (prv->advert) {
        svWidgetDestroy(prv->advert);
        prv->advert = NULL;
    }
}

SvLocal void
NewTVOSDSubsManagerUpdated(SvObject self_, SvObject observedObject, SvObject arg)
{
    QBNewTVOSD self = (QBNewTVOSD) self_;

    QBOSDTSRecording osd = (QBOSDTSRecording)
        QBOSDMainGetHandler((QBOSDMain) self, QBOSDHandlerType_recording, NULL);

    if (self->subsManager == (QBSubsManager) observedObject) {
        QBSubsManager subsManager = (QBSubsManager) observedObject;
        svTVOsdSetInfoIconsBySubsManager(self->tvOsd, subsManager);
    } else if (osd && osd == (QBOSDTSRecording) observedObject && SvObjectIsInstanceOf(observedObject, QBOSDTSRecording_getType())) {
        QBOSDState state = QBOSDGetState((QBOSD) osd, NULL);
        svOSDShow(self->osdBottom, state.forceShow);
        if (state.lock)
            svOSDLock(self->osdBottom);
        else
            svOSDUnlock(self->osdBottom);
        self->allowHide = state.allowHide;
    }
}
