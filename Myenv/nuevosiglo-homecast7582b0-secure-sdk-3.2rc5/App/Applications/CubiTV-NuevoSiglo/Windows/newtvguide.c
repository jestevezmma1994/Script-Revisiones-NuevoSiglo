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

#include "newtvguide.h"

#include <libintl.h>
#include <SvCore/SvTime.h>
#include <QBInput/QBInputCodes.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/clock.h>
#include <SWL/icon.h>
#include <SvEPG/SvEPGGrid.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvStringBuffer.h>
#include <SvFoundation/SvArray.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <QBAppKit/QBPeerEvent.h>
#include <QBAppKit/QBTimeDateMonitor.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <ctype.h>
#include <error_codes.h>
#include <QBPlayerControllers/utils/content.h>
#include <main.h>
#include <QBOSK/QBOSKKey.h>
#include <QBSearch/QBSearchEngine.h>
#include <Windows/newtv.h>
#include <Windows/verticalEPG.h>
#include <Windows/tv/QBTVChannelFrame.h>
#include <Windows/searchResults.h>
#include <Widgets/infoIcons.h>
#include <Widgets/QBTitle.h>
#include <Widgets/tvEPGOverlay.h>
#include <Widgets/customerLogo.h>
#include <Widgets/QBEPGDisplayer.h>
#include <Widgets/QBTVPreview.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBReminderEditorPane.h>
#include <ContextMenus/QBExtendedInfoPane.h>
#include <Services/reminder.h>
#include <Services/core/appState.h>
#include <Utils/QBEventUtils.h>
#include <Logic/timeFormat.h>
#include <Logic/TVLogic.h>
#include <Logic/EventsLogic.h>
#include <CAGE/Core/Sv2DRect.h>
#include <Utils/recordingUtils.h>
#include <TranslationMerger.h>
#include <QBPCRatings/QBPCUtils.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBDSMCC/QBDSMCCAdsRotationService.h>
#include <SvCore/SvCommonDefs.h>
#include <Utils/dsmccUtils.h>
#include <QBConf.h>
#include <SvEPGDataLayer/SvEPGManager.h>

typedef struct EPGContext_t *EPGContext;
struct EPGContext_t {
   struct QBWindowContext_t super_;

   unsigned int settingsCtx;

   AppGlobals appGlobals;
   SvTVChannel activeChannel;

   SvEPGEvent activeEvent;
   SvEPGManager manager;
   SvGenericObject view;
   SvEPGDataWindow epgWindow;

   time_t closeTime;

   QBDSMCCAdsRotationService adsRotor;
};

typedef struct EPGInfo_t *EPGInfo;
struct EPGInfo_t {
    EPGContext ctx;
    SvWidget grid;
    SvWidget title;
    SvWidget logo;
    SvWidget logoLabel;
    SvWidget preview;
    SvWidget descriptionTitle;
    SvWidget description;
    SvWidget icons;
    SvWidget clock;
    SvWidget infoLabel;
    SvWidget channelFrame;
    SvWidget adPreview;
    SvWidget adBottom;
    int previewBorder;

    struct {
       QBContextMenu ctx;
       bool shown;
    } sidemenu;
};

SvLocal void SvNewTVGuideChannelSelect(SvWidget w, SvTVChannel ch);

SvLocal void
EPGWindowManualChannelSelection(void *prv, SvTVChannel channel)
{
    EPGInfo info = prv;

    //By Active in EPG we mean focused
    SvEPGGridSetActiveChannel(info->grid, SvDBObjectGetID((SvDBObject) channel));
}

SvLocal bool
EPGWindowInputEventHandler(SvWidget w, SvInputEvent ev)
{
    EPGInfo self = w->prv;

    if (QBTVChannelFrameInputEventHandler(self->channelFrame, &ev->fullInfo))
        return true;

    if (QBEPGLogicHandleInputEvent(self->ctx->appGlobals->epgLogic, ev))
        return true;

    AppGlobals appGlobals = self->ctx->appGlobals;
    QBAppPVRDiskState diskState = QBAppStateServiceGetPVRDiskState(appGlobals->appState);

    if (ev->ch == QBKEY_EPG) {
        time_t activeTime = SvEPGGridGetTimePoint(self->grid);
        SvTVChannel channel = SvEPGGridGetActiveChannel(self->grid);

        QBApplicationControllerSwitchTop(appGlobals->controller, appGlobals->verticalEPG);
        QBVerticalEPGSetFocus(appGlobals->verticalEPG, channel, activeTime);
        return true;
    } else if (ev->ch == QBKEY_STOP) {
        if (diskState == QBPVRDiskState_pvr_present || diskState == QBPVRDiskState_pvr_ts_present) {
            svNewTVGuideShowSideMenu((QBWindowContext) self->ctx, SVSTRING(""), NewTVGuideSideMenuType_Stop); //Show appropriate side menu
            return true;
        }
    } else if (ev->ch == QBKEY_BACK) { // AMERELES Fix en la navegación del menú de la derecha
        if (svNewTVGuideIsSideMenuShown((QBWindowContext) self->ctx)) {
            svNewTVGuideHideSideMenu((QBWindowContext) self->ctx);
            return true;
        }
    }

    return svWidgetInputEventHandler(w, ev);
}

SvLocal SvString SvEPGEventGetPCRatingString(AppGlobals appGlobals, SvEPGEvent event)
{
    return EventsLogicPCRatingToString(appGlobals->eventsLogic, event->PC);
}

SvLocal
SvString SvEPGEventGetTitle(SvEPGEvent event, AppGlobals appGlobals, bool showPCRating)
{
    SvString title = NULL;

    if (event) {
        SvEPGEventDesc desc =  QBEventUtilsGetDescFromEvent(appGlobals->eventsLogic, appGlobals->langPreferences, event);

        if (desc && desc->title) {
            SvStringBuffer buf = SvStringBufferCreate(NULL);
            SvStringBufferAppendFormatted(buf, NULL, "%s", SvStringCString(desc->title));

            if (showPCRating) {
                SvString ratingStr = SvEPGEventGetPCRatingString(appGlobals, event);
                if (ratingStr) {
                    SvStringBufferAppendFormatted(buf, NULL, " (%s)", SvStringCString(ratingStr));
                    SVRELEASE(ratingStr);
                }
            }
            title = SvStringBufferCreateContentsString(buf, NULL);
            SVRELEASE(buf);
        }
    }
    return title;
}

SvString SvEPGEventGetDurationTime(SvEPGEvent event, const char *timeFormat)
{
    SvString str = NULL;

    if (event) {
        struct tm t1, t2;
        char buff1[128], buff2[128];
        SvTimeBreakDown(SvTimeConstruct(event->startTime, 0), true, &t1);
        SvTimeBreakDown(SvTimeConstruct(event->endTime, 0), true, &t2);
        strftime(buff1, sizeof(buff1), (timeFormat) ? timeFormat : QBTimeFormatGetCurrent()->gridEPGTime, &t1);
        strftime(buff2, sizeof(buff2), (timeFormat) ? timeFormat : QBTimeFormatGetCurrent()->gridEPGTime, &t2);
        str = SvStringCreateWithFormat("%s - %s\n", buff1, buff2);
    }
    return str;
}

SvLocal void EPGWindowUpdateEvent(EPGInfo self, SvTVChannel channel, SvEPGEvent event)
{
    SvString title = SvEPGEventGetTitle(event, self->ctx->appGlobals, true);
    SvString durationTime = SvEPGEventGetDurationTime(event, QBTimeFormatGetCurrent()->gridEPGTime);

    SvString desc = NULL;

    // set description
    if (event) {
        SvStringBuffer buf = SvStringBufferCreate(NULL);
        const char *category = SvEPGEventGetCategoryName(event);
        if (category)
            SvStringBufferAppendFormatted(buf, NULL, "%s\n", gettext(category));
        AppGlobals appGlobals = self->ctx->appGlobals;
        SvEPGEventDesc ed = QBEventUtilsGetDescFromEvent(appGlobals->eventsLogic, appGlobals->langPreferences, event);
        SvString descStr = NULL;

        if (ed)
            descStr = ed->shortDescription ? ed->shortDescription : ed->description;
        if (descStr)
            SvStringBufferAppendFormatted(buf, NULL, "%s", SvStringCString(descStr));
        desc = SvStringBufferCreateContentsString(buf, NULL);
        SVRELEASE(buf);
    }

    SvStringBuffer descriptionBuff = SvStringBufferCreate(NULL);
    if (title)
        SvStringBufferAppendFormatted(descriptionBuff, NULL, "%s ", SvStringCString(title));
    if (durationTime)
        SvStringBufferAppendFormatted(descriptionBuff, NULL, "%s", SvStringCString(durationTime));

    if (self->descriptionTitle) {
        SvString titleAndTime = SvStringBufferCreateContentsString(descriptionBuff, NULL);
        QBAsyncLabelSetText(self->descriptionTitle, titleAndTime);
        SVRELEASE(titleAndTime);

        QBAsyncLabelSetText(self->description, desc);
    } else {
        if (desc)
            SvStringBufferAppendFormatted(descriptionBuff, NULL, "\n%s", SvStringCString(desc));

        SvString fullDesc = SvStringBufferCreateContentsString(descriptionBuff, NULL);

        QBAsyncLabelSetText(self->description, fullDesc);
        SVRELEASE(fullDesc);
    }

    SVRELEASE(descriptionBuff);
    SVTESTRELEASE(desc);
    SVTESTRELEASE(durationTime);
    SVTESTRELEASE(title);
    if (self->icons)
        QBInfoIconsSetByEvent(self->icons, event);
}

SvLocal void EPGContextHandlePeerEvent(SvObject self_, QBPeerEvent event_, SvObject sender)
{
    EPGContext self = (EPGContext) self_;

    if (SvObjectIsInstanceOf((SvObject) event_, SvEPGGridFocusChangedEvent_getType())) {
        SvEPGGridFocusChangedEvent event = (SvEPGGridFocusChangedEvent) event_;

        SVTESTRETAIN(event->activeEvent);
        SVTESTRELEASE(self->activeEvent);
        self->activeEvent = event->activeEvent;

        if (self->super_.window) {
            EPGInfo info = self->super_.window->prv;
            QBTVChannelFrameSetChannel(info->channelFrame, event->activeChannel);
            EPGWindowUpdateEvent(info, event->activeChannel, event->activeEvent);
        }
    }
}

SvLocal void EPGOnSideMenuClose(void *self_, QBContextMenu menu)
{
    EPGInfo self = self_;
    self->sidemenu.shown = false;
}

SvLocal void EPGContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    EPGContext self = (EPGContext) self_;
    EPGInfo prv = (EPGInfo) calloc(1, sizeof(struct EPGInfo_t));
    const char* widgetName = "EPGWidget";

    QBEPGLogicDefaultViewSet(self->appGlobals, self_);

    SvTVChannel channel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
    bool channelIsRadio = (channel && SvTVChannelGetAttribute(channel, SVSTRING("Radio"))) ? true : false;

    prv->ctx = self;
    if (channelIsRadio) {
        svSettingsPushComponent("RadioGuide.settings");
    } else {
        svSettingsPushComponent("TVguide.settings");
    }
    self->settingsCtx = svSettingsSaveContext();
    SvGenericObject displayer = QBEPGDisplayerCreate(self->appGlobals, widgetName);
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject current = QBPlaylistManagerGetCurrent(playlists);

    if (current != (SvGenericObject) self->view) {
        SVTESTRELEASE(self->view);
        self->view = SVTESTRETAIN(current);
        if (self->view) {
            SvEPGDataWindowSetChannelView(self->epgWindow, (SvGenericObject) self->view, NULL);
        }
    }

    SvWindow window = QBGUILogicCreateBackgroundWidget(self->appGlobals->guiLogic, "EPGWindow", NULL);
    self->super_.window = window;
    svWidgetSetName(window, "EPGWindow");

    if (svSettingsGetBoolean("EPGWindow", "useTransparency", false)) {
        int transparency = 100;
        QBConfigGetInteger("MENU_TRANSPARENCY", &transparency);
        window->alpha = ALPHA_SOLID - (uint8_t)((double)ALPHA_SOLID*((double)transparency/100.0));
        if (window->alpha != ALPHA_SOLID)
            SvTVContextSetOverlay(prv->ctx->appGlobals->newTV, NULL);
        else
            SvTVContextDisableOverlay(prv->ctx->appGlobals->newTV);
    }

    window->prv = prv;
    svWidgetSetInputEventHandler(window, EPGWindowInputEventHandler);

    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(self->appGlobals->initLogic);
    if (logoWidgetName && svSettingsIsWidgetDefined(SvStringGetCString(logoWidgetName)))
        prv->logo = QBCustomerLogoAttach(self->super_.window, logoWidgetName, 1);

    if (svSettingsIsWidgetDefined("Title")) {
        if (!(prv->title = QBTitleNew(app, "Title", self->appGlobals->textRenderer)))
            goto init_error;
        if (!svSettingsWidgetAttach(window, prv->title, "Title", 1))
            goto init_error;
    }

    if (svSettingsIsWidgetDefined("TVPreview")) {
        SvString playlistId = QBPlaylistManagerGetCurrentId(playlists);
        QBPlaylistCursor cursor = QBPlaylistCursorsGet(self->appGlobals->playlistCursors, SVSTRING("TV"), playlistId);
        if (!(prv->preview = QBTVPreviewNew(self->appGlobals, "TVPreview", cursor)))
            goto init_error;
        if (svSettingsWidgetAttach(window, prv->preview, "TVPreview", 1) == 0)
            goto init_error;
    }

    if (svSettingsIsWidgetDefined("LogoLabel")) {
       if ((prv->logoLabel = svSettingsWidgetCreate(app, "LogoLabel"))) {
          if (svSettingsWidgetAttach(window, prv->logoLabel, "LogoLabel", 100) == 0)
             goto init_error;
       }
    }

    if (svSettingsIsWidgetDefined("EPGWindow.description.title")) {
        prv->descriptionTitle = QBAsyncLabelNew(app, "EPGWindow.description.title", self->appGlobals->textRenderer);
        svSettingsWidgetAttach(window, prv->descriptionTitle, svWidgetGetName(prv->descriptionTitle), 0);
    }

    prv->description = QBAsyncLabelNew(app, "EPGWindow.description", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, prv->description, svWidgetGetName(prv->description), 0);

    if (svSettingsIsWidgetDefined("icons")) {
        SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                             SVSTRING("SvEPGManager"));
        prv->icons = QBInfoIconsCreate(app, "icons",
                                       self->appGlobals->eventsLogic,
                                       epgManager,
                                       self->appGlobals->pc);
        if (prv->icons)
            svSettingsWidgetAttach(window, prv->icons, "icons", 0);
    }

    prv->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenuEPG.settings", self->appGlobals->controller, app, SVSTRING("SideMenuEPG"));
    QBContextMenuSetCallbacks(prv->sidemenu.ctx, EPGOnSideMenuClose, prv);

    prv->clock = svClockNew(app, "clock");
    svSettingsWidgetAttach(window, prv->clock, svWidgetGetName(prv->clock), 1);

    if ((prv->infoLabel = QBAsyncLabelNew(app, "EPGWindow.info", self->appGlobals->textRenderer)))
        svSettingsWidgetAttach(window, prv->infoLabel, svWidgetGetName(prv->infoLabel), 1);

    if (prv->title) {
        SvString listName = QBPlaylistManagerGetCurrentName(playlists);
        if (!listName || SvStringEqualToCString(QBPlaylistManagerGetCurrentType(playlists), "TV")) {
            listName = SvStringCreate(gettext("EPG"), NULL);
        } else {
            SVRETAIN(listName);
        }

        QBTitleSetText(prv->title, listName, 0);
        SVTESTRELEASE(listName);
    }
    int timeScale = svSettingsGetInteger("EPGWindow.EPGGrid", "timeScale", 180);
    if (!(prv->grid =
        SvEPGGridNew(app, "EPGWindow.EPGGrid", NULL,
                     (SvEPGTextRenderer) QBServiceRegistryGetService(registry, SVSTRING("SvEPGTextRenderer")),
                     timeScale, displayer, NULL)))
       goto init_error;
    SvEPGGridSetDescFunc(prv->grid, QBEventUtilsGetDescFromEvent_, prv->ctx->appGlobals);
    // TODO CUB8-463
    SvEPGGridSetFormatId(prv->grid, QBTimeFormatGetCurrent()->gridEPGDate);
    if (self->view) {
        SvGenericObject overlay = tvEPGOverlayCreate(self->appGlobals, "EPGWindow.EPGGrid.StripeHeader", "EPGWindow.EPGHeader", "EPGWindow.EPGGrid.StripeHeader", (SvGenericObject) self->view);
        SvEPGGridSetOverlay(prv->grid, overlay, NULL);
        SVRELEASE(overlay);

        SvEPGGridSetDataWindow(prv->grid, self->epgWindow, NULL);
    }

    QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
    QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, SvEPGGridFocusChangedEvent_getType(), prv->grid->prv, NULL);

    svSettingsWidgetAttach(window, prv->grid, "EPGWindow.EPGGrid", 1);
    int leftTimeLimit = svSettingsGetInteger("EPGWindow.EPGGrid", "leftTimeLimit", 30 * 3600 * 24);
    int rightTimeLimit = svSettingsGetInteger("EPGWindow.EPGGrid", "rightTimeLimit", 30 * 3600 * 24);
    QBEPGLogicUpdateDefaultTimeLimitsForViews(self->appGlobals->epgLogic, &leftTimeLimit, &rightTimeLimit);
    SvEPGGridSetTimeLimit(prv->grid, leftTimeLimit, rightTimeLimit, NULL);

    prv->channelFrame = QBTVChannelFrameNew(app, "ChannelFrame",
                                            self->appGlobals->textRenderer,
                                            playlists,
                                            self->appGlobals->tvLogic);
    QBTVChannelFrameSetCallback(prv->channelFrame, EPGWindowManualChannelSelection, prv);

    int offsetY = svSettingsGetInteger("ChannelFrame", "yOffset", 0);
    int offsetX = prv->title ? (prv->title->width + prv->title->off_x) : svSettingsGetInteger("ChannelFrame", "xOffset", 170);
    svWidgetAttach(window, prv->channelFrame, offsetX, offsetY, 10);

    svSettingsPopComponent();

    time_t startTime = self->activeEvent ? self->activeEvent->startTime : SvTimeNow();
    SvEPGGridSetActivePosition(prv->grid, startTime, NULL);
    SvEPGGridSetActiveChannel(prv->grid, self->activeEvent ? self->activeEvent->channelID : SvDBObjectGetID((SvDBObject) channel));
    SvNewTVGuideChannelSelect(self->super_.window, channel);
    QBTVChannelFrameSetChannel(prv->channelFrame, channel);
    self->activeChannel = channel;

    if (prv->preview) {
        SvWidget subs = svWidgetCreateBitmap(app, prv->preview->width, prv->preview->height, NULL);
        svWidgetAttach(prv->preview, subs, 0, 0, 10);
        SvTVContextAttachSubtitle(self->appGlobals->newTV, subs);
    }

    if (self->appGlobals->dsmccMonitor) {
        QBContentProviderAddListener((QBContentProvider) self->appGlobals->adsDSMCCDataProvider, (SvObject) self);
        QBContentProviderStart((QBContentProvider) self->appGlobals->adsDSMCCDataProvider, self->appGlobals->scheduler);
        QBDSMCCAdsRotationServiceStart(self->adsRotor, self->appGlobals->scheduler);
    }

    QBEPGLogicPostWidgetsCreate(self->appGlobals->epgLogic);
    SVTESTRELEASE(displayer);
    return;

init_error:
    SvLogError("Error in TVGuide initialization");
    svSettingsPopComponent();
    SVTESTRELEASE(displayer);
}

SvLocal void EPGContextDestroyWindow(QBWindowContext self_)
{
    EPGContext self = (EPGContext) self_;
    QBEPGLogicPreWidgetsDestroy(self->appGlobals->epgLogic);
    self->activeChannel = NULL;
    SvTVContextDisableOverlay(self->appGlobals->newTV);
    SvTVContextDetachSubtitle(self->appGlobals->newTV);
    if (self->appGlobals->dsmccMonitor) {
        QBDSMCCAdsRotationServiceStop(self->adsRotor);
        QBDSMCCAdsRotationServiceClearContent(self->adsRotor);
        QBContentProviderRemoveListener((QBContentProvider) self->appGlobals->adsDSMCCDataProvider, (SvObject) self);
        QBContentProviderStop((QBContentProvider) self->appGlobals->adsDSMCCDataProvider);
    }
    EPGInfo prv = self->super_.window->prv;
    SVRELEASE(prv->sidemenu.ctx);
    self->closeTime = SvTimeNow();
    svWidgetDestroy(prv->icons);
    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal void EPGContextReinitializeWindow(QBWindowContext self_, SvArray itemList)
{
    EPGContext self = (EPGContext) self_;
    if (!self->super_.window) {
        return;
    }
    EPGInfo prv = (EPGInfo) self->super_.window->prv;
    if (!itemList || SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("CustomerLogo"))) {
        SvString logoWidgetName = QBInitLogicGetLogoWidgetName(self->appGlobals->initLogic);
        if (logoWidgetName)
            prv->logo = QBCustomerLogoReplace(prv->logo, self->super_.window, logoWidgetName, 1);
    }
    if (!itemList || SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("QBParentalControl"))) {
        EPGWindowUpdateEvent(prv, self->activeChannel, self->activeEvent);
    }

    SvEPGGridReinitialize(prv->grid);
}

SvLocal void
EPGContextDestroy(void *self_)
{
   EPGContext self = self_;
   SVTESTRELEASE(self->manager);
   SVTESTRELEASE(self->epgWindow);
   SVTESTRELEASE(self->view);

   SVTESTRELEASE(self->activeEvent);

   SVTESTRELEASE(self->adsRotor);
}

SvLocal void EPGContextTimeUpdated(SvGenericObject self_, bool firstTime, QBTimeDateUpdateSource source);

SvLocal void EPGContextDSMCCObjectReceived(SvObject self_, SvString serviceID)
{
    EPGContext self = (EPGContext) self_;
    if (SvStringEqualToCString(serviceID, "adverts")) {
        SvHashTable data = QBDSMCCDataProviderGetObject(self->appGlobals->adsDSMCCDataProvider);
        QBDSMCCAdsRotationServiceSetContent(self->adsRotor, data);
    }
}

SvLocal void EPGContextSetAds(SvObject epg_, SvHashTable images)
{
    EPGContext epg = (EPGContext) epg_;
    SvWindow window = epg->super_.window;
    EPGInfo self = window->prv;

    DSMCCUtilsEPGSetAds(epg->appGlobals->res, window, epg->settingsCtx, &self->adPreview, &self->adBottom, images);
}

SvType EPGContext_getType(void)
{
    static SvType type = NULL;
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_             = {
            .destroy        = EPGContextDestroy
        },
        .reinitializeWindow = EPGContextReinitializeWindow,
        .createWindow       = EPGContextCreateWindow,
        .destroyWindow      = EPGContextDestroyWindow
    };
    static const struct QBPeerEventReceiver_ eventReceiverMethods = {
        .handleEvent = EPGContextHandlePeerEvent
    };
    static const struct QBTimeDateMonitorListener_ timeDateListenerMethods = {
        .systemTimeSet = EPGContextTimeUpdated
    };
    static const struct QBDSMCCDataProviderListener_ dsmccDataProviderListenerMethods = {
        .objectReceived = EPGContextDSMCCObjectReceived
    };
    static const struct QBDSMCCAdsRotationServiceListener_ dsmccAdsRotationServiceListenerMethods = {
        .setAds = EPGContextSetAds
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("EPGContext",
                            sizeof(struct EPGContext_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            QBTimeDateMonitorListener_getInterface(), &timeDateListenerMethods,
                            QBDSMCCDataProviderListener_getInterface(), &dsmccDataProviderListenerMethods,
                            QBDSMCCAdsRotationServiceListener_getInterface(), &dsmccAdsRotationServiceListenerMethods,
                            NULL);
    }

    return type;
}

SvLocal void SvNewTVGuideChannelSelect(SvWidget w, SvTVChannel ch)
{
    EPGInfo self = w->prv;
    if (!ch)
        return;

    if (ch == self->ctx->activeChannel)
        return;

    QBTVLogicPlayChannel(self->ctx->appGlobals->tvLogic, ch, SVSTRING("TVGuide"));
    self->ctx->activeChannel = ch;
}

QBWindowContext svNewTVGuideContextCreate(AppGlobals appGlobals)
{
   EPGContext self;

   self = (EPGContext) SvTypeAllocateInstance(EPGContext_getType(), NULL);

   if (!self)
      return NULL;

   self->appGlobals = appGlobals;

   self->activeChannel = NULL;
   self->activeEvent = NULL;

   self->manager = SVRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));
   QBServiceRegistry registry = QBServiceRegistryGetInstance();
   QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
   self->view = SVTESTRETAIN(QBPlaylistManagerGetCurrent(playlists));
   SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                        SVSTRING("SvEPGManager"));
   self->epgWindow = SvEPGManagerCreateDataWindow(epgManager, NULL);

   SvEPGManagerUpdateParams params = SvEPGManagerCreateUpdateParameters(epgManager);
   SvEPGManagerSetUpdatesStrategyForClient(epgManager, (SvObject)self->epgWindow, params);
   SVRELEASE(params);

   if(self->view)
       SvEPGDataWindowSetChannelView(self->epgWindow, (SvGenericObject) self->view, NULL);

   QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                       SVSTRING("QBTimeDateMonitor"));
   QBTimeDateMonitorAddListener(timeDateMonitor, (SvObject) self, NULL);

   if (appGlobals->dsmccMonitor) {
      self->adsRotor = QBDSMCCAdsRotationServiceCreate();
      QBDSMCCAdsRotationServiceAddListener(self->adsRotor, (SvObject) self);
   }

   return (QBWindowContext) self;
}

void svNewTVGuideShowExtendedInfo(QBWindowContext epg_)
{
    EPGContext epg = (EPGContext) epg_;
    if(!epg->super_.window)
        return;
    EPGInfo self = epg->super_.window->prv;
    SvEPGEvent event = epg->activeEvent;
    if(!self->sidemenu.shown && event) {
        self->sidemenu.shown = true;
        SvArray events = SvArrayCreate(NULL);
        SvArrayAddObject(events, (SvGenericObject)event);
        QBExtendedInfoPane pane = QBExtendedInfoPaneCreateFromSettings("ReminderEditorPane.settings", epg->appGlobals, self->sidemenu.ctx, SVSTRING("ExtendedInfo"), 1, events);
        SVRELEASE(events);
        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) pane);
        QBContextMenuShow(self->sidemenu.ctx);
        SVRELEASE(pane);
    }
}

SvLocal void
svNewTVGuideSearchKeyTyped(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    EPGInfo self = ptr;

    if (key->type == QBOSKKeyType_enter && SvStringLength(input)) {
        QBWindowContext searchResults = QBSearchResultsContextCreate(self->ctx->appGlobals);

        SvArray agents = SvArrayCreate(NULL);
        SvString playlistId = SvTVChannelGetAttribute(self->ctx->activeChannel, SVSTRING("TV")) ? SVSTRING("TVChannels") : SVSTRING("RadioChannels");
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        SvObject view = QBPlaylistManagerGetById(playlists, playlistId);
        SvObject agent = QBInitLogicCreateSearchAgent(self->ctx->appGlobals->initLogic,
                                                      EPGManSearchAgentMode_titleOnly, 0, view);

        SvArrayAddObject(agents, agent);
        SVRELEASE(agent);

        SvObject search = QBSearchEngineCreate(agents, input, NULL);
        SVRELEASE(agents);

        if (SvArrayCount(QBSearchEngineGetKeywords(search))) {
            QBSearchResultsSetDataSource(searchResults, search);
            QBApplicationControllerPushContext(self->ctx->appGlobals->controller, searchResults);
        }

        SVRELEASE(searchResults);
        SVRELEASE(search);
    }
}

SvLocal void
svNewTVGuideCreateAndPushSearchPane(EPGContext epg)
{
    EPGInfo self = epg->super_.window->prv;
    //Prepare OSK
    svSettingsPushComponent("OSKPane.settings");
    QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
    SvErrorInfo error = NULL;
    QBOSKPaneInit(oskPane, self->ctx->appGlobals->scheduler, self->sidemenu.ctx, 1, SVSTRING("OSKPane"),
                  svNewTVGuideSearchKeyTyped, self, &error);
    svSettingsPopComponent();
    if (!error) {
        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) oskPane);
        QBContextMenuShow(self->sidemenu.ctx);
    } else {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVRELEASE(oskPane);
}

SvLocal void svNewTVGuideShowSideMenu_(EPGContext epg, SvString id, size_t idx, NewTVGuideSideMenuType type, bool useID)
{
    if (!epg->super_.window) {
        return;
    }

    EPGInfo self = epg->super_.window->prv;

    if (self->sidemenu.shown) {
        return;
    }

    SvTVChannel channel = SvEPGGridGetActiveChannel(self->grid);

    if (!channel) {
        return;
    }

    QBReminderEditorPane pane = NULL;
    const char *allowedOptions = NULL;

    switch (type) {
        case NewTVGuideSideMenuType_NPvrSeries:
            pane = QBReminderEditorPaneCreateAndPushWithNPvrFromSettings("ReminderEditorPane.settings", epg->appGlobals, self->sidemenu.ctx, SVSTRING("ReminderEditorWithNPvr"), channel, epg->activeEvent, QBReminderEditorPaneType_Series);
            break;
        case NewTVGuideSideMenuType_NPvrKeyword:
            pane = QBReminderEditorPaneCreateAndPushWithNPvrFromSettings("ReminderEditorPane.settings", epg->appGlobals, self->sidemenu.ctx, SVSTRING("ReminderEditorWithNPvr"), channel, epg->activeEvent, QBReminderEditorPaneType_Keyword);
            break;
        case NewTVGuideSideMenuType_Stop:
            allowedOptions = "EPGReminderEditorStopPane.json";
            break;
        case NewTVGuideSideMenuType_SearchOnly:
            svNewTVGuideCreateAndPushSearchPane(epg);
            break;
        case NewTVGuideSideMenuType_Full:
        default:
            allowedOptions = "newtvguideEPGReminderEditorPane.json";
            break;
    }

    if (pane || !allowedOptions) {
        self->sidemenu.shown = true;
        SVTESTRELEASE(pane);
        return;
    }

    pane = QBReminderEditorPaneCreateFromSettings("ReminderEditorPane.settings", allowedOptions, epg->appGlobals, self->sidemenu.ctx, SVSTRING("ReminderEditor"), channel, epg->activeEvent);

    if (!pane) {
        return;
    }

    self->sidemenu.shown = true;
    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) pane);
    QBContextMenuShow(self->sidemenu.ctx);

    if (useID) {
        QBReminderEditorPaneSetPosition(pane, id, true);
    } else {
        QBReminderEditorPaneSetPositionByIndex(pane, idx, true);
    }

    SVRELEASE(pane);
}

void svNewTVGuideShowSideMenu(QBWindowContext epg_, SvString id, NewTVGuideSideMenuType type)
{
    EPGContext epg = (EPGContext) epg_;
    svNewTVGuideShowSideMenu_(epg, id, 0, type, true);
}

void svNewTVGuideShowSideMenuOnIndex(QBWindowContext epg_, size_t idx, NewTVGuideSideMenuType type)
{
    EPGContext epg = (EPGContext) epg_;
    svNewTVGuideShowSideMenu_(epg, NULL, idx, type, false);
}

void svNewTVGuideShowRecord(QBWindowContext epg_)
{
    EPGContext epg = (EPGContext) epg_;
    if(!epg->super_.window)
        return;
    EPGInfo self = epg->super_.window->prv;
    SvTVChannel channel = SvEPGGridGetActiveChannel(self->grid);
    if(!self->sidemenu.shown && channel && epg->activeEvent) {
        QBReminderEditorPane pane = QBReminderEditorPaneCreateFromSettings("ReminderEditorPane.settings", "EPGReminderEditorPane.json", epg->appGlobals, self->sidemenu.ctx, SVSTRING("ReminderEditor"), channel, epg->activeEvent);
        if (!pane)
            return;
        self->sidemenu.shown = true;
        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) pane);
        QBContextMenuShow(self->sidemenu.ctx);
        QBReminderEditorPaneSetPosition(pane, SVSTRING("record"), true);
        QBReminderEditorPaneRecord(pane);
        SVRELEASE(pane);
    }
}


bool svNewTVGuideStopRecording(QBWindowContext epg_)
{
    EPGContext epg = (EPGContext) epg_;
    SvEPGEvent event = NULL;
    svNewTVGuideGetActiveEvent(epg_, &event);
    if (!event)
        return false;

    return (QBRecordingUtilsStopRecordingForEvent(epg->appGlobals->pvrProvider, event));
}


bool svNewTVGuideIsSideMenuShown(QBWindowContext epg_)
{
    EPGContext epg = (EPGContext) epg_;
    if(!epg->super_.window)
        return false;
    EPGInfo self = epg->super_.window->prv;
    return self->sidemenu.shown;
}

void svNewTVGuideHideSideMenu(QBWindowContext epg_)
{
    EPGContext epg = (EPGContext) epg_;
    if(!epg->super_.window)
        return;
    EPGInfo self = epg->super_.window->prv;
    if(self->sidemenu.shown) {
        QBContextMenuHide(self->sidemenu.ctx, false);
    }
}

void svNewTVGuideSetDefaultPosition(QBWindowContext epg_)
{
    EPGContext epg = (EPGContext) epg_;
    if(!epg->super_.window){
        SVTESTRELEASE(epg->activeEvent);
        epg->activeEvent = NULL;
        return;
    }
    EPGInfo self = epg->super_.window->prv;
    SvEPGGridSetActivePosition(self->grid, SvTimeNow(), NULL);
    SvTVChannel channel = svNewTVGuideGetFocus((QBWindowContext) epg);
    SvEPGGridSetActiveChannel(self->grid, SvDBObjectGetID((SvDBObject) channel));
}

SvLocal void EPGContextTimeUpdated(SvGenericObject self_, bool firstTime, QBTimeDateUpdateSource source)
{
    EPGContext self = (EPGContext) self_;
    // We need to react on every time update, as we need to handle e.g. TOT changes (with current API we are unable
    // to distinguish exact cause of time update)
    svNewTVGuideSetDefaultPosition((QBWindowContext) self);
}

void svNewTVGuideSetFocus(QBWindowContext epg_, SvEPGEvent event){
    EPGContext epg = (EPGContext) epg_;
    SVTESTRELEASE(epg->activeEvent);
    epg->activeEvent = SVTESTRETAIN(event);

    if(!epg->super_.window){
        return;
    }
    EPGInfo self = epg->super_.window->prv;
    time_t startTime = event?event->startTime:SvTimeNow();
    SvEPGGridSetActivePosition(self->grid, startTime, NULL);
    SvTVChannel ch = SvTVContextGetCurrentChannel(epg->appGlobals->newTV);
    SvEPGGridSetActiveChannel(self->grid,
            event ? event->channelID : SvDBObjectGetID((SvDBObject) ch));
    SvNewTVGuideChannelSelect(epg->super_.window, ch);
    QBTVChannelFrameSetChannel(self->channelFrame, ch);
    epg->activeChannel = ch;
}

SvTVChannel svNewTVGuideGetFocus(QBWindowContext epg_)
{
    EPGContext epg = (EPGContext) epg_;

    if(!epg->super_.window)
        return NULL;

    EPGInfo self = epg->super_.window->prv;
    return SvEPGGridGetActiveChannel(self->grid);
}

SvTVChannel svNewTVGuideGetChannel(QBWindowContext epg_)
{
    EPGContext epg = (EPGContext) epg_;

    if(!epg->super_.window)
        return NULL;

    EPGInfo self = epg->super_.window->prv;
    return self->ctx->activeChannel;
}

void svNewTVGuideSetChannel(QBWindowContext epg_, SvTVChannel channel)
{
    if(!channel)
        return;

    EPGContext epg = (EPGContext) epg_;

    if(!epg->super_.window)
        return;

    SvNewTVGuideChannelSelect(epg->super_.window, channel);
    EPGInfo info = epg->super_.window->prv;
    QBTVChannelFrameSetChannel(info->channelFrame, channel);
}

void svNewTVGuideGetActiveEvent(QBWindowContext epg_, SvEPGEvent *event)
{
    EPGContext epg = (EPGContext) epg_;

    if (event == NULL)
        return;

    if(!epg->super_.window)
        *event = NULL;
    else
        *event = epg->activeEvent;
}

void svNewTVGuideSetPlaylist(QBWindowContext epg_, SvGenericObject playlist)
{
    EPGContext epg = (EPGContext) epg_;

    if (playlist && !SvObjectEquals(epg->view, playlist)) {
        SVTESTRELEASE(epg->view);
        epg->view = SVRETAIN(playlist);
        SvEPGDataWindowSetChannelView(epg->epgWindow, (SvGenericObject) playlist, NULL );

        if (epg->super_.window) {
            EPGInfo self = epg->super_.window->prv;
            SvEPGGridReinitialize(self->grid);
        }
    }
}

SvGenericObject svNewTVGuideGetCurrentPlaylist(QBWindowContext epg_)
{
    EPGContext epg = (EPGContext) epg_;
    return epg->view;
}

QBContextMenu svNewTVGuideGetContextMenu(QBWindowContext epg_)
{
    EPGContext epg = (EPGContext) epg_;
    EPGInfo self = epg->super_.window->prv;
    return self->sidemenu.ctx;
}
