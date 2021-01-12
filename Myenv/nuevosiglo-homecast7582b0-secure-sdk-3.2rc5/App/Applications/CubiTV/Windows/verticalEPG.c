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

#include "verticalEPG.h"

#include <libintl.h>
#include <ContextMenus/QBReminderEditorPane.h>
#include <SvCore/SvTime.h>
#include <Widgets/infoIcons.h>
#include <Widgets/QBTitle.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <SvEPG/SvEPGOverlay.h>
#include <QBOSK/QBOSKKey.h>
#include <QBSearch/QBSearchEngine.h>
#include <QBInput/QBInputCodes.h>
#include <settings.h>
#include <SWL/QBFrame.h>
#include <SWL/icon.h>
#include <SWL/clock.h>
#include <Widgets/QBEPGDisplayer.h>
#include <Widgets/tvEPGOverlay.h>
#include <Widgets/customerLogo.h>
#include <Widgets/QBTVPreview.h>
#include <Windows/searchResults.h>
#include <Windows/newtv.h>
#include <Windows/newtvguide.h>
#include <Windows/tv/QBTVChannelFrame.h>
#include <ContextMenus/QBExtendedInfoPane.h>
#include <Widgets/QBRecordingIcon.h>
#include <Logic/timeFormat.h>
#include <Logic/EventsLogic.h>
#include <Logic/EPGLogic.h>
#include <QBWindowContextInputEventHandler.h>
#include <CAGE/Core/Sv2DRect.h>
#include <Utils/QBEventUtils.h>
#include <Utils/recordingUtils.h>
#include <Services/core/appState.h>
#include <QBInput/QBInputEvent.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvEPGDataLayer/SvEPGEventsList.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvEPG/SvEPGDisplayer.h>
#include <Utils/recordingUtils.h>
#include <Services/core/appState.h>
#include <XMB2/XMBVerticalMenu.h>
#include <XMB2/XMBItemController.h>
#include <main.h>
#include <QBPCRatings/QBPCUtils.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBDSMCC/QBDSMCCAdsRotationService.h>
#include <Utils/dsmccUtils.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <QBConf.h>
#include <QBAppKit/QBServiceRegistry.h>

SvLocal bool VerticalEPGWindowInputEventHandler(SvWidget w, SvInputEvent ev);
SvLocal void VerticalEPGWindowClean(SvApplication app, void *ptr);
SvLocal SvWidget VerticalEPGArrowFromSM(SvWidget window, const char *name);
SvLocal SvType QBVerticalEPGMenuItemController_getType(void);

typedef struct VerticalEPGWindow_t *VerticalEPGWindow;
struct VerticalEPGWindow_t {
    struct QBWindowContext_t super_;

    unsigned int settingsCtx;

    AppGlobals appGlobals;
    SvEPGManager epgManager;
    SvEPGView view;
    SvEPGEventsList dataSource;
    SvTVChannel channel;

    SvGenericObject displayer;
    time_t activeTime;

    QBDSMCCAdsRotationService adsRotor;
};

typedef struct EPGVerticalInfo_t *EPGVerticalInfo;
struct EPGVerticalInfo_t {
    VerticalEPGWindow ctx;
    SvWidget logo;
    SvWidget title;
    SvWidget preview;
    SvWidget description;
    SvWidget infoIcons;
    SvWidget dateLabel;
    SvWidget arrowLeft, arrowRight, arrowUp, arrowDown;
    SvWidget adPreview;
    SvWidget adBottom;
    double arrowFadeTime;
    SvGenericObject itemController, itemControllerExt;

    SvFiber fiber;
    SvFiberTimer fiberTimer;
    SvScheduler scheduler;

    SvTimeRange currentTimeRange;
    SvTimeRange limitTimeRange;
    SvTimeRange endOfSearchRange;
    time_t autoSearchLimit;
    SvWidget channelFrame;
    SvWidget xmbMenu, xmbMenuExt;
    struct {
        QBContextMenu ctx;
        bool shown;
    } sidemenu;

    SvWidget focusWidget;        ///< additional widget than is placed between XMB menus when separateTextScrolling is enabled
    bool separateTextScrolling;  ///< @c true text and background images are located on separate XMB menus.
    bool showDetailed;           ///< show even description and event's end time
};

struct QBVerticalEPGMenuItemController_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    VerticalEPGWindow ctx;
    SvBitmap icon;
    SvBitmap inactiveBG;
    QBFrameConstructor *focus;
    QBXMBItemConstructor itemConstructor;
    bool useText;
    bool useBackground;
    bool showDetailed;
    int itemHeight;
    SvGenericObject overlayObj;
};
typedef struct QBVerticalEPGMenuItemController_t *QBVerticalEPGMenuItemController;

SvLocal void VerticalEPGWindowUpdateEvent(EPGVerticalInfo info);
SvLocal void
QBVerticalEPGWindowSetChannel(EPGVerticalInfo info, SvTVChannel channel);

#define MAX_DATE_LENGTH 128

SvLocal void QBVerticalEPGOnSideMenuClose(void *self_, QBContextMenu menu)
{
    VerticalEPGWindow epg = self_;
    EPGVerticalInfo self = epg->super_.window->prv;
    self->sidemenu.shown = false;
}

SvLocal void QBVerticalEPGWindowStep(void *self);
SvLocal void QBVerticalEPGWindowSetPosition(EPGVerticalInfo info, time_t time);
SvLocal void QBVerticalEPGWindowResizeTimeRange(EPGVerticalInfo self_, bool side);
SvLocal void VerticalEPGWindowUserMoving(EPGVerticalInfo self_);
SvLocal void VerticalEPGWindowManualChannelSelection(void *prv, SvTVChannel channel);
SvLocal bool VerticalEPGWindowInputEventHandlerImpl(EPGVerticalInfo info, const QBInputEvent *ev, bool direct);

SvLocal void
VerticalEPGUpdateFocusWidget(EPGVerticalInfo info)
{
    VerticalEPGWindow epg = info->ctx;
    bool visible = true;

    if (!info->focusWidget)
        return;

    if (!epg || !epg->dataSource) {
        visible = false;
    } else {
        size_t cnt = SvInvokeInterface(QBListModel, epg->dataSource, getLength);
        if (!cnt)
            visible = false;
    }
    svWidgetSetHidden(info->focusWidget, !visible);
}

SvLocal
SvTVChannel getChannelByOffset(SvEPGView view, SvTVChannel channel, int offset)
{
    if (!view)
        return NULL;
    int idx = 0;
    if (channel)
        idx = SvInvokeInterface(SvEPGChannelView, view, getChannelIndex, channel) + offset;
    int size = SvInvokeInterface(SvEPGChannelView, view, getCount);
    if (!size)
        return NULL;
    idx = (idx + size) % size;
    return SvInvokeInterface(SvEPGChannelView, view, getByIndex, idx);
}

SvLocal
void VerticalEPGWindowCheckChannel(VerticalEPGWindow self)
{
    if (!self->channel)
        return;

    if (self->view) {
        int idx = SvInvokeInterface(SvEPGChannelView, self->view, getChannelIndex, self->channel);
        if (idx < 0) {
            // channel not found on playlist - it might happens when we drop all channels
            SVRELEASE(self->channel);

            SVRELEASE(self->view);
            QBServiceRegistry registry = QBServiceRegistryGetInstance();
            QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
            self->view = (SvEPGView) QBPlaylistManagerGetCurrent(playlists);
            SVTESTRETAIN(self->view);

            if (!self->view) {
                self->view = (SvEPGView) QBPlaylistManagerGetById(playlists, SVSTRING("AllChannels"));
                SVRETAIN(self->view);
            }

            self->channel = SvInvokeInterface(SvEPGChannelView, self->view, getByIndex, 0);
            SVTESTRETAIN(self->channel);
        }
    }
}

SvLocal void VerticalEPGWindowReinitializeWindow(QBWindowContext self_, SvArray itemList)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;
    if (!self->super_.window)
        return;

    EPGVerticalInfo info = (EPGVerticalInfo) self->super_.window->prv;
    if (!itemList || SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("CustomerLogo"))) {
        SvString logoWidgetName = QBInitLogicGetLogoWidgetName(self->appGlobals->initLogic);
        if (logoWidgetName)
            info->logo = QBCustomerLogoReplace(info->logo, self->super_.window, logoWidgetName, 1);
    }

    if (!itemList || SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("QBParentalControl"))) {
        VerticalEPGWindowUpdateEvent(info);
        SvEPGEventsListUpdateEPG(self->dataSource, &(info->currentTimeRange), self->channel);
        SvEPGManagerPropagateChannelChange(self->epgManager, self->channel, NULL);
    }
}

SvLocal void
VerticalEPGWindowOnChangePositionImpl(void *w_, const XMBMenuEvent data, bool handleEnterKey)
{
    EPGVerticalInfo info = ((SvWidget) w_)->prv;
    VerticalEPGWindow self = info->ctx;

    if (data->clicked && handleEnterKey) {
        QBInputEvent ev;
        ev.type = QBInputEventType_keyTyped;
        ev.u.key.code = QBKEY_ENTER;
        if (VerticalEPGWindowInputEventHandlerImpl(info, &ev, false)) {
            return;
        } else {
            QBVerticalEPGShowSideMenu(self->appGlobals->verticalEPG, SVSTRING(""), QBVerticalEPGSideMenuType_Full);
            return;
        }
    }
    SvEPGEvent event = self->dataSource ? (SvEPGEvent) SvInvokeInterface(QBListModel, (self->dataSource), getObject, data->position) : NULL;

    /**
     * Checking if position and prevPosition are valid (greater than 0) and are different from each other.
     * If it's true, this means user moved between events in verticalEPG. False means user still stands in the same position or
     * it's first time showing this channel on verticalEPG.
     * If user moved - activeTime is now equal focused event's startTime.
     * If not, activeTime is not changing its value.
     *
     * If position is valid and prevPosition is not valid, it means this is first time showing this channel on verticalEPG.
     * In this case, we are setting the position on current activeTime and not changing its value.
     */
    if ((int) data->position >= 0 && (int) data->prevPosition == -1) {
        QBVerticalEPGWindowSetPosition(info, self->activeTime);
    } else if ((int) data->prevPosition >= 0 && (int) data->position >= 0 && data->prevPosition != data->position) {
        if (event) {
            if (event->endTime <= info->limitTimeRange.startTime ||
                event->startTime >= info->limitTimeRange.endTime) {
                QBVerticalEPGWindowSetPosition(info, self->activeTime);
            } else {
                self->activeTime = event->startTime;
                VerticalEPGWindowUserMoving(info);
            }
        }
    }
    VerticalEPGWindowUpdateEvent(info);
}

SvLocal void
VerticalEPGWindowOnChangePosition(void *w_, const XMBMenuEvent data)
{
    VerticalEPGWindowOnChangePositionImpl(w_, data, true);
}

SvLocal void
VerticalEPGWindowOnChangePositionExt(void *w_, const XMBMenuEvent data)
{
    VerticalEPGWindowOnChangePositionImpl(w_, data, false);
}

SvLocal bool
VerticalEPGInputEventHandler(SvWidget widget, SvInputEvent event)
{
    EPGVerticalInfo prv = widget->prv;

    bool consumed = false;
    if (prv->xmbMenu)
        consumed = SvInvokeInterface(XMBMenu, prv->xmbMenu->prv, handleInputEvent, event);
    if (prv->xmbMenuExt)
        consumed |= SvInvokeInterface(XMBMenu, prv->xmbMenuExt->prv, handleInputEvent, event);

    return consumed;
}

SvLocal void VerticalEPGFakeClean(SvApplication app, void *ptr)
{
}

SvLocal void VerticalEPGWindowCreateWindow(QBWindowContext self_, SvApplication app)
{
    VerticalEPGWindow epg = (VerticalEPGWindow) self_;

    VerticalEPGWindowCheckChannel(epg);
    EPGVerticalInfo prv = (EPGVerticalInfo) calloc(1, sizeof(struct EPGVerticalInfo_t));
    prv->ctx = epg;

    SvTVChannel channel = SvTVContextGetCurrentChannel(epg->appGlobals->newTV);
    bool channelIsRadio = (channel && SvTVChannelGetAttribute(channel, SVSTRING("Radio"))) ? true : false;

    if (channelIsRadio) {
        svSettingsPushComponent("RadioVerticalEPG.settings");
    } else {
        svSettingsPushComponent("verticalEPG.settings");
    }
    epg->settingsCtx = svSettingsSaveContext();
    SvWindow window = svSettingsWidgetCreate(app, "verticalEPGWindow");
    if (!window) {
        SvLogError("no verticalEPG.settings");
        svSettingsPopComponent();
        free(prv);
        return;
    }

    if (svSettingsGetBoolean("verticalEPGWindow", "useTransparency", false)) {
        int transparency = 100;
        QBConfigGetInteger("MENU_TRANSPARENCY", &transparency);
        window->alpha = ALPHA_SOLID - (uint8_t) ((double) ALPHA_SOLID * ((double) transparency / 100.0));
        if (window->alpha != ALPHA_SOLID)
            SvTVContextSetOverlay(prv->ctx->appGlobals->newTV, NULL);
        else
            SvTVContextDisableOverlay(prv->ctx->appGlobals->newTV);
    }

    int backTimeLimit = svSettingsGetInteger("verticalEPGWindow", "backTimeLimit", 30);
    int forwardTimeLimit = svSettingsGetInteger("verticalEPGWindow", "forwardTimeLimit", 30);
    QBEPGLogicUpdateDefaultTimeLimitsForViews(epg->appGlobals->epgLogic, &backTimeLimit, &forwardTimeLimit);

    prv->autoSearchLimit = 24 * 3600;

    window->prv = prv;
    window->clean = VerticalEPGWindowClean;
    svWidgetSetInputEventHandler(window, VerticalEPGWindowInputEventHandler);

    prv->arrowFadeTime = svSettingsGetDouble("verticalEPGWindow", "arrowFadeTime", 0.0);

    if (svSettingsIsWidgetDefined("Title")) {
        prv->title = QBTitleNew(app, "Title", epg->appGlobals->textRenderer);
        svSettingsWidgetAttach(window, prv->title, "Title", 10);
    }

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    if (svSettingsIsWidgetDefined("TVPreview")) {
        SvString playlistId = QBPlaylistManagerGetCurrentId(playlists);
        QBPlaylistCursor cursor = QBPlaylistCursorsGet(epg->appGlobals->playlistCursors, SVSTRING("TV"), playlistId);
        if ((prv->preview = QBTVPreviewNew(epg->appGlobals, "TVPreview", cursor))) {
            svSettingsWidgetAttach(window, prv->preview, "TVPreview", 1);
        }
    }

    prv->description = QBAsyncLabelNew(app, "description", epg->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, prv->description, svWidgetGetName(prv->description), 10);

    SvWidget w = svSettingsWidgetCreate(app, "dateLabelBG");
    svSettingsWidgetAttach(window, w, svWidgetGetName(w), 10);
    prv->dateLabel = QBAsyncLabelNew(app, "dateLabel", epg->appGlobals->textRenderer);
    svSettingsWidgetAttach(w, prv->dateLabel, svWidgetGetName(prv->dateLabel), 10);

    prv->arrowDown = VerticalEPGArrowFromSM(window, "arrowDown");
    prv->arrowUp = VerticalEPGArrowFromSM(window, "arrowUp");
    prv->arrowLeft = VerticalEPGArrowFromSM(window, "arrowLeft");
    prv->arrowRight = VerticalEPGArrowFromSM(window, "arrowRight");

    prv->infoIcons = QBInfoIconsCreate(app, "infoIcons",
                                       epg->appGlobals->eventsLogic,
                                       epg->epgManager,
                                       epg->appGlobals->pc);
    svSettingsWidgetAttach(window, prv->infoIcons, "infoIcons", 10);

    SvScheduler scheduler = SvSchedulerGet();
    prv->scheduler = scheduler;
    prv->fiber = SvFiberCreate(scheduler, NULL, "EPGVerticalInfo", QBVerticalEPGWindowStep, prv);
    prv->fiberTimer = SvFiberTimerCreate(prv->fiber);
    SvFiberTimerActivateAfter(prv->fiberTimer, SvTimeFromMilliseconds(2000));

    prv->currentTimeRange = SvTimeRangeConstruct(epg->activeTime - prv->autoSearchLimit, epg->activeTime + prv->autoSearchLimit);
    prv->limitTimeRange = SvTimeRangeConstruct(SvTimeNow() - (backTimeLimit * 24 * 3600),
                                               SvTimeNow() + (forwardTimeLimit * 24 * 3600));
    if (svSettingsIsWidgetDefined("clock")) {
        SvWidget wClock = svClockNew(app, "clock");
        svSettingsWidgetAttach(window, wClock, svWidgetGetName(wClock), 1);
    }

    SvWidget infoLabel = QBAsyncLabelNew(app, "info", epg->appGlobals->textRenderer);
    if (infoLabel)
        svSettingsWidgetAttach(window, infoLabel, svWidgetGetName(infoLabel), 1);

    epg->super_.window = window;

    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(epg->appGlobals->initLogic);
    if (logoWidgetName && svSettingsIsWidgetDefined(SvStringGetCString(logoWidgetName)))
        prv->logo = QBCustomerLogoAttach(epg->super_.window, logoWidgetName, 1);

    SvWidget verticalBarWindow = svSettingsWidgetCreate(app, "verticalBarWindow");
    svSettingsWidgetAttach(window, verticalBarWindow, svWidgetGetName(verticalBarWindow), 0);

    SVTESTRELEASE(epg->dataSource);
    epg->dataSource = SvEPGManagerCreateEventsList(epg->epgManager, NULL);

    if (!epg->displayer) {
        epg->displayer = QBEPGDisplayerCreate(epg->appGlobals, "EPGWidget");
    }

    prv->separateTextScrolling = svSettingsGetBoolean("verticalEPGWindow", "separateTextScrolling", false);
    prv->showDetailed = svSettingsGetBoolean("verticalEPGWindow", "showDetailed", false);

    QBVerticalEPGMenuItemController itemController = (QBVerticalEPGMenuItemController)
                                                     SvTypeAllocateInstance(QBVerticalEPGMenuItemController_getType(), NULL);

    prv->itemController = (SvGenericObject) itemController;

    itemController->appGlobals = epg->appGlobals;
    itemController->icon = SVRETAIN(svSettingsGetBitmap("MenuItem", "icon"));
    itemController->inactiveBG = SVRETAIN(svSettingsGetBitmap("MenuItem", "bg"));
    itemController->focus = QBFrameConstructorFromSM("MenuItem.focus");
    itemController->itemConstructor = QBXMBItemConstructorCreate("MenuItem", epg->appGlobals->textRenderer);
    itemController->overlayObj = tvEPGOverlayCreate(epg->appGlobals, NULL, NULL, "MenuItem", NULL);
    itemController->ctx = epg;
    itemController->useText = true;
    itemController->useBackground = true;
    itemController->showDetailed = prv->showDetailed;
    itemController->itemHeight = svSettingsGetInteger("MenuItem", "height", -1);
    if (!prv->showDetailed) {
        //Setting descriptionYOffset disables modified (detailed) view in constructor.
        //This may be not obvious for person who modifies view and it can be forced here.
        itemController->itemConstructor->descriptionYOffset = 0;
    }

    if (prv->separateTextScrolling) {
        QBVerticalEPGMenuItemController itemController2 = (QBVerticalEPGMenuItemController)
                                                          SvTypeAllocateInstance(QBVerticalEPGMenuItemController_getType(), NULL);

        prv->itemControllerExt = (SvGenericObject) itemController2;

        itemController2->appGlobals = epg->appGlobals;
        itemController2->icon = itemController->icon;
        itemController2->inactiveBG = SVTESTRETAIN(itemController->inactiveBG);
        itemController2->focus = SVTESTRETAIN(itemController->focus);
        itemController2->itemConstructor = SVTESTRETAIN(itemController->itemConstructor);
        itemController2->overlayObj = itemController->overlayObj;
        itemController2->ctx = epg;
        itemController2->useText = true;
        itemController2->useBackground = false;
        itemController2->showDetailed = prv->showDetailed;
        itemController2->itemHeight = itemController->itemHeight;

        //adjust main item contriller
        itemController->useText = false;
        itemController->icon = NULL;
        itemController->overlayObj = NULL;
    }

    if (svSettingsIsWidgetDefined("verticalBar.focusMain")) {
        QBFrameParameters params = { .bitmap = NULL };
        int tmp = svSettingsGetInteger("verticalBar.focusMain", "width", -1);
        if (tmp <= 0)
            tmp = svSettingsGetInteger("MenuItem", "width", -1);
        params.width = params.minWidth = params.maxWidth = tmp;
        tmp = svSettingsGetInteger("verticalBar.focusMain", "height", -1);
        if (tmp <= 0)
            tmp = itemController->itemHeight * svSettingsGetInteger("verticalBar", "activeRowHeightFactor", 1);
        params.height = params.minHeight = params.maxHeight = tmp;
        params.bitmap = svSettingsGetBitmap("verticalBar.focusMain", "bg");
        prv->focusWidget = QBFrameCreate(app, &params);
        svSettingsWidgetAttach(verticalBarWindow, prv->focusWidget, "verticalBar.focusMain", 11);
        VerticalEPGUpdateFocusWidget(prv);
    }

    prv->xmbMenu = XMBVerticalMenuNew(app, "verticalBar", 0);
    svSettingsWidgetAttach(verticalBarWindow, prv->xmbMenu, svWidgetGetName(prv->xmbMenu), 10);
    XMBVerticalMenuSetNotificationTarget(prv->xmbMenu, window, VerticalEPGWindowOnChangePosition);
    XMBVerticalMenuConnectToDataSource(prv->xmbMenu, (SvGenericObject) epg->dataSource, (SvGenericObject) prv->itemController, NULL);

    if (prv->separateTextScrolling) {
        prv->xmbMenuExt = XMBVerticalMenuNew(app, "verticalBar", 0);
        svWidgetAttach(verticalBarWindow, prv->xmbMenuExt, prv->xmbMenu->off_x, prv->xmbMenu->off_y, 12);
        XMBVerticalMenuSetNotificationTarget(prv->xmbMenuExt, window, VerticalEPGWindowOnChangePositionExt);
        XMBVerticalMenuConnectToDataSource(prv->xmbMenuExt, (SvGenericObject) epg->dataSource, (SvGenericObject) prv->itemControllerExt, NULL);
    }

    verticalBarWindow->eventHandlers.inputEventHandler = VerticalEPGInputEventHandler;
    verticalBarWindow->prv = prv;
    verticalBarWindow->clean = VerticalEPGFakeClean;
    svWidgetSetFocusable(verticalBarWindow, true);
    svWidgetSetFocus(verticalBarWindow);

    prv->channelFrame = QBTVChannelFrameNew(app, "frame",
                                            epg->appGlobals->textRenderer,
                                            playlists,
                                            epg->appGlobals->tvLogic);
    QBTVChannelFrameSetCallback(prv->channelFrame, VerticalEPGWindowManualChannelSelection, prv);
    svSettingsWidgetAttach(window, prv->channelFrame, svWidgetGetName(prv->channelFrame), 10);

    svSettingsPopComponent();

    QBEPGLogicDefaultViewSet(epg->appGlobals, self_);

    SvGenericObject current = QBPlaylistManagerGetCurrent(playlists);
    if (current != (SvGenericObject) epg->view) {
        SVTESTRELEASE(epg->view);
        epg->view = SVTESTRETAIN(current);
    }

    QBVerticalEPGWindowSetChannel(prv, epg->channel);

    prv->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenuEPG.settings", epg->appGlobals->controller, app, SVSTRING("SideMenuEPG"));
    QBContextMenuSetCallbacks(prv->sidemenu.ctx, QBVerticalEPGOnSideMenuClose, epg);

    if (prv->preview) {
        SvWidget subs = svWidgetCreateBitmap(app, prv->preview->width, prv->preview->height, NULL);
        svWidgetAttach(prv->preview, subs, 0, 0, 10);
        SvTVContextAttachSubtitle(epg->appGlobals->newTV, subs);
    }

    SvString listName = QBPlaylistManagerGetCurrentName(playlists);
    if (!listName || SvObjectEquals((SvObject) QBPlaylistManagerGetCurrentType(playlists), (SvObject) SVSTRING("TV"))) {
        listName = SvStringCreate(gettext("EPG"), NULL);
    } else {
        SVRETAIN(listName);
    }

    QBTitleSetText(prv->title, listName, 0);
    SVTESTRELEASE(listName);

    QBVerticalEPGSetFocus((QBWindowContext) epg, epg->channel, epg->activeTime);

    if (epg->channel)
        SvEPGEventsListSetChannel(epg->dataSource, epg->channel, NULL);

    if (!(SvEPGManagerCheckIfRequestExists(epg->epgManager, NULL, epg->channel))) {
        SvEPGEventsListSetTimeRange(epg->dataSource, &(prv->currentTimeRange), NULL);
    }

    if (epg->appGlobals->dsmccMonitor) {
        QBContentProviderAddListener((QBContentProvider) epg->appGlobals->adsDSMCCDataProvider, (SvObject) epg);
        QBContentProviderStart((QBContentProvider) epg->appGlobals->adsDSMCCDataProvider, epg->appGlobals->scheduler);
        QBDSMCCAdsRotationServiceStart(epg->adsRotor, epg->appGlobals->scheduler);
    }

    QBVerticalEPGLogicPostWidgetsCreate(epg->appGlobals->verticalEPGLogic);

    QBReminderAddListener(epg->appGlobals->reminder, (SvObject) epg);
}

/**
 * param[in] sideUp           TRUE = UP, FALSE = DOWN
 */
SvLocal void
QBVerticalEPGWindowResizeTimeRange(EPGVerticalInfo self_, bool sideUp)
{
    EPGVerticalInfo self = self_;

    if (self->endOfSearchRange.endTime - self->endOfSearchRange.startTime > 0) {
        if ((self->endOfSearchRange.startTime - self->currentTimeRange.startTime >= self->autoSearchLimit)
            || (self->currentTimeRange.endTime - self->endOfSearchRange.endTime >= self->autoSearchLimit)) {
            return;
        }
    }
    if (sideUp && (self->currentTimeRange.startTime >= self->limitTimeRange.startTime))
        self->currentTimeRange.startTime -= self->autoSearchLimit;
    if (!sideUp && (self->currentTimeRange.endTime <= self->limitTimeRange.endTime))
        self->currentTimeRange.endTime += self->autoSearchLimit;
}

SvLocal void
VerticalEPGWindowReminderUpdate(SvObject self_, QBReminderTask task)
{
    VerticalEPGWindow epg = (VerticalEPGWindow) self_;
    SvWindow window = epg->super_.window;
    EPGVerticalInfo self = window->prv;

    size_t count = SvInvokeInterface(QBListModel, epg->dataSource, getLength);
    if (count <= 0)
        return;

    SvEPGEvent event = (SvEPGEvent) SvInvokeInterface(QBListModel, epg->dataSource, getObject, 0);
    if (unlikely(!SvObjectIsInstanceOf((SvObject) event, SvEPGEvent_getType())))
        return;
    if (!SvObjectEquals((SvObject) event->channelID, (SvObject) task->channelID))
        return;

    size_t i = 0;
    do {
        if (task->startTime == event->startTime && task->endTime == event->endTime) {
            SvInvokeInterface(QBListModelListener, self->xmbMenu->prv, itemsChanged, i, 1);
            break;
        }
    } while ((++i < count) && (event = (SvEPGEvent) SvInvokeInterface(QBListModel, epg->dataSource, getObject, i)));
}

SvLocal void
VerticalEPGWindowReminderAdded(SvObject self_, QBReminderTask task)
{
    VerticalEPGWindowReminderUpdate(self_, task);
}

SvLocal void
VerticalEPGWindowReminderRemoved(SvObject self_, QBReminderTask task)
{
    VerticalEPGWindowReminderUpdate(self_, task);
}

SvLocal void
QBVerticalEPGWindowStep(void *self_)
{
    EPGVerticalInfo self = self_;
    VerticalEPGWindow epg = self->ctx;
    if (!(SvEPGManagerCheckIfRequestExists(epg->epgManager, NULL, epg->channel))) {
        int pos = XMBVerticalMenuGetPosition(self->xmbMenu);
        int count = SvInvokeInterface(QBListModel, epg->dataSource, getLength);
        if ((pos >= 0) && (pos <= 5)) {
            QBVerticalEPGWindowResizeTimeRange(self, true);
        }
        if ((pos >= (count - 10)) && (pos != -1)) {
            QBVerticalEPGWindowResizeTimeRange(self, false);
        }
        if (pos == -1) {
            self->currentTimeRange.startTime -= self->autoSearchLimit;
            self->currentTimeRange.endTime += self->autoSearchLimit;
        }

        SvEPGEventsListSetTimeRange(epg->dataSource, &(self->currentTimeRange), NULL);
        if (pos != -1) {
            if (self->endOfSearchRange.endTime - self->endOfSearchRange.startTime == 0)
                self->endOfSearchRange = self->currentTimeRange;
        }
    } else {
        if (self->endOfSearchRange.endTime - self->endOfSearchRange.startTime > 0)
            self->endOfSearchRange = SvTimeRangeConstruct(0, 0);
    }

    SvFiberDeactivate(self->fiber);
    SvFiberTimerActivateAfter(self->fiberTimer, SvTimeFromMs(500));
}

SvLocal void VerticalEPGWindowDestroyWindow(QBWindowContext self_)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;
    QBReminderRemoveListener(self->appGlobals->reminder, (SvObject) self);
    QBVerticalEPGLogicPreWidgetsDestroy(self->appGlobals->verticalEPGLogic);
    SvTVContextDisableOverlay(self->appGlobals->newTV);
    SvTVContextDetachSubtitle(self->appGlobals->newTV);
    if (self->appGlobals->dsmccMonitor) {
        QBDSMCCAdsRotationServiceStop(self->adsRotor);
        QBDSMCCAdsRotationServiceClearContent(self->adsRotor);
        QBContentProviderRemoveListener((QBContentProvider) self->appGlobals->adsDSMCCDataProvider, (SvObject) self);
        QBContentProviderStop((QBContentProvider) self->appGlobals->adsDSMCCDataProvider);
    }
    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal void
VerticalEPGWindowDestroy(void *self_)
{
    VerticalEPGWindow self = self_;

    SVTESTRELEASE(self->view);
    SVTESTRELEASE(self->dataSource);
    SVTESTRELEASE(self->channel);
    SVTESTRELEASE(self->displayer);
    SVTESTRELEASE(self->adsRotor);
    SVTESTRELEASE(self->epgManager);
}

SvLocal bool
VerticalEPGWindowWCTXInputEventHandler(SvGenericObject w, const QBInputEvent *e);

SvLocal void VerticalEPGWindowDSMCCObjectReceived(SvObject self_, SvString serviceID)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;
    if (SvStringEqualToCString(serviceID, "adverts")) {
        SvHashTable data = QBDSMCCDataProviderGetObject(self->appGlobals->adsDSMCCDataProvider);
        QBDSMCCAdsRotationServiceSetContent(self->adsRotor, data);
    }
}

SvLocal void VerticalEPGWindowSetAds(SvObject epg_, SvHashTable images)
{
    VerticalEPGWindow epg = (VerticalEPGWindow) epg_;
    SvWindow window = epg->super_.window;
    EPGVerticalInfo self = window->prv;

    DSMCCUtilsEPGSetAds(epg->appGlobals->res, window, epg->settingsCtx, &self->adPreview, &self->adBottom, images);
}

SvType VerticalEPGWindow_getType(void)
{
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_             = {
            .destroy        = VerticalEPGWindowDestroy
        },
        .reinitializeWindow = VerticalEPGWindowReinitializeWindow,
        .createWindow       = VerticalEPGWindowCreateWindow,
        .destroyWindow      = VerticalEPGWindowDestroyWindow
    };
    static const struct QBWindowContextInputEventHandler_t inputEventHandler = {
        .handleInputEvent = VerticalEPGWindowWCTXInputEventHandler
    };
    static const struct QBDSMCCDataProviderListener_ dsmccDataProviderListenerMethods = {
        .objectReceived = VerticalEPGWindowDSMCCObjectReceived
    };
    static const struct QBDSMCCAdsRotationServiceListener_ dsmccAdsRotationServiceListenerMethods = {
        .setAds = VerticalEPGWindowSetAds
    };

    static const struct QBReminderListener_t reminderListenerMethods = {
        .reminderAdded   = VerticalEPGWindowReminderAdded,
        .reminderRemoved = VerticalEPGWindowReminderRemoved,
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("VerticalEPGWindow",
                            sizeof(struct VerticalEPGWindow_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            QBWindowContextInputEventHandler_getInterface(), &inputEventHandler,
                            QBDSMCCDataProviderListener_getInterface(), &dsmccDataProviderListenerMethods,
                            QBDSMCCAdsRotationServiceListener_getInterface(), &dsmccAdsRotationServiceListenerMethods,
                            QBReminderListener_getInterface(), &reminderListenerMethods,
                            NULL);
    }
    return type;
}


SvLocal void
VerticalEPGWindowArrowsSetActive(EPGVerticalInfo self, int pos)
{
    VerticalEPGWindow epg = self->ctx;

    if (!epg->dataSource)
        return;

    size_t cnt = SvInvokeInterface(QBListModel, epg->dataSource, getLength);
    if (self->arrowUp)
        svIconSwitch(self->arrowUp, pos > 0 ? 1 : 0, 0, self->arrowFadeTime);
    if (self->arrowDown)
        svIconSwitch(self->arrowDown, ((ssize_t) pos + 1 < (ssize_t) cnt) ? 1 : 0, 0, self->arrowFadeTime);
}


SvLocal void
QBVerticalEPGWindowSetPosition(EPGVerticalInfo info, time_t timePoint)
{
    VerticalEPGWindow self = info->ctx;
    int n = SvInvokeInterface(QBListModel, self->dataSource, getLength);
    if (!n)
        return;
    int idx = 0;
    for (idx = 0; idx < n; ++idx) {
        SvEPGEvent event = (SvEPGEvent) SvInvokeInterface(QBListModel, self->dataSource, getObject, idx);
        if (event->endTime > timePoint)
            break;
    }
    if (idx == n)
        idx = n - 1;
    XMBVerticalMenuSetPosition(info->xmbMenu, idx, 1, NULL);
    if (info->xmbMenuExt)
        XMBVerticalMenuSetPosition(info->xmbMenuExt, idx, 1, NULL);
}

SvLocal void
VerticalEPGWindowUserMoving(EPGVerticalInfo self_)
{
    EPGVerticalInfo self = self_;
    VerticalEPGWindow epg = self->ctx;

    size_t pos = (size_t) XMBVerticalMenuGetPosition(self->xmbMenu);
    size_t last = (size_t) SvInvokeInterface(QBListModel, (epg->dataSource), getLength) - 1;
    SvEPGEvent nextEvent = NULL, prevEvent = NULL;

    size_t middleVisibleEventPosition = XMBVerticalMenuGetSlotsCount(self->xmbMenu) / 2;

    if ((pos >= middleVisibleEventPosition) && (pos < last - middleVisibleEventPosition)) {
        nextEvent = epg->dataSource ? (SvEPGEvent) SvInvokeInterface(QBListModel, (epg->dataSource), getObject, pos + 7) : NULL;
        prevEvent = epg->dataSource ? (SvEPGEvent) SvInvokeInterface(QBListModel, (epg->dataSource), getObject, pos - 3) : NULL;
    } else if (pos >= middleVisibleEventPosition && pos >= last - middleVisibleEventPosition) {
        nextEvent = epg->dataSource ? (SvEPGEvent) SvInvokeInterface(QBListModel, (epg->dataSource), getObject, last) : NULL;
        prevEvent = epg->dataSource ? (SvEPGEvent) SvInvokeInterface(QBListModel, (epg->dataSource), getObject, pos - 3) : NULL;
    } else if (pos < middleVisibleEventPosition && pos < last - middleVisibleEventPosition) {
        nextEvent = epg->dataSource ? (SvEPGEvent) SvInvokeInterface(QBListModel, (epg->dataSource), getObject, pos + 7) : NULL;
        prevEvent = epg->dataSource ? (SvEPGEvent) SvInvokeInterface(QBListModel, (epg->dataSource), getObject, 0) : NULL;
    } else if (pos < middleVisibleEventPosition && pos >= last - middleVisibleEventPosition) {
        nextEvent = epg->dataSource ? (SvEPGEvent) SvInvokeInterface(QBListModel, (epg->dataSource), getObject, last) : NULL;
        prevEvent = epg->dataSource ? (SvEPGEvent) SvInvokeInterface(QBListModel, (epg->dataSource), getObject, 0) : NULL;
    }

    if (!nextEvent || !prevEvent)
        return;

    if (self->currentTimeRange.startTime <= prevEvent->startTime - 12 * 3600) {
        if (self->currentTimeRange.endTime >= nextEvent->endTime + 12 * 3600) {
            return;
        } else {
            self->currentTimeRange.endTime = nextEvent->endTime + (12 * 3600);
        }
    } else if (self->currentTimeRange.endTime >= nextEvent->endTime + 12 * 3600) {
        if (self->currentTimeRange.startTime <= prevEvent->startTime - 12 * 3600) {
            return;
        } else {
            self->currentTimeRange.startTime = prevEvent->startTime - (12 * 3600);
        }
    } else {
        self->currentTimeRange.startTime = prevEvent->startTime - (12 * 3600);
        self->currentTimeRange.endTime = nextEvent->endTime + (12 * 3600);
    }

    if (!(SvEPGManagerCheckIfRequestExists(epg->epgManager, NULL, epg->channel))) {
        SvEPGEventsListSetTimeRange(epg->dataSource, &(self->currentTimeRange), NULL);
    }
}

SvLocal void
VerticalEPGWindowUpdateEvent(EPGVerticalInfo info)
{
    VerticalEPGWindow self = info->ctx;

    int pos = XMBVerticalMenuGetPosition(info->xmbMenu);

    VerticalEPGUpdateFocusWidget(info);
    VerticalEPGWindowArrowsSetActive(info, pos);
    SvEPGEvent event = self->dataSource ? (SvEPGEvent) SvInvokeInterface(QBListModel, (self->dataSource), getObject, pos) : NULL;
    SvEPGEventDesc desc = NULL;
    if (event)
        desc = QBEventUtilsGetDescFromEvent(self->appGlobals->eventsLogic, self->appGlobals->langPreferences, event);
    if (!desc) {
        QBAsyncLabelSetText(info->description, SVSTRING(""));
        QBTitleSetText(info->title, SVSTRING(""), 1);
        return;
    }
    SvStringBuffer buf = SvStringBufferCreate(NULL);
    if (event->PC) {
        SvString rating = EventsLogicPCRatingToString(self->appGlobals->eventsLogic, event->PC);
        if (rating) {
            SvStringBufferAppendFormatted(buf, NULL, "%s", SvStringCString(rating));
            SVRELEASE(rating);
        }
    }
    if (SvStringBufferLength(buf))
        SvStringBufferAppendFormatted(buf, NULL, "\n");

    if (desc->shortDescription) {
        SvStringBufferAppendFormatted(buf, NULL, "%s", SvStringCString(desc->shortDescription));
    } else if (desc->description) {
        SvStringBufferAppendFormatted(buf, NULL, "%s", SvStringCString(desc->description));
    }
    SvString str = SvStringBufferCreateContentsString(buf, NULL);
    QBAsyncLabelSetText(info->description, str);
    SVRELEASE(str);
    SVRELEASE(buf);

    char buf1[MAX_DATE_LENGTH], buf2[MAX_DATE_LENGTH];
    struct tm startTime, endTime;
    SvTimeBreakDown(SvTimeConstruct(event->startTime, 0), true, &startTime);
    strftime(buf1, sizeof(buf1), QBTimeFormatGetCurrent()->verticalEPGTime, &startTime);
    SvTimeBreakDown(SvTimeConstruct(event->endTime, 0), true, &endTime);
    strftime(buf2, sizeof(buf2), QBTimeFormatGetCurrent()->verticalEPGTime, &endTime);

    SvString s = desc->title ? SVRETAIN(desc->title) : SVSTRING("");
    SvString title = SvStringCreateWithFormat("%s %s-%s", SvStringCString(s), buf1, buf2);
    QBTitleSetText(info->title, title, 1);
    SVRELEASE(title);
    SVRELEASE(s);

    strftime(buf1, sizeof(buf1), QBTimeFormatGetCurrent()->verticalEPGDate, &startTime);
    QBAsyncLabelSetText(info->dateLabel, SVAUTORELEASE(SvStringCreate(buf1, NULL)));
}

SvLocal void
QBVerticalEPGWindowSetChannel(EPGVerticalInfo info, SvTVChannel channel)
{
    VerticalEPGWindow self = info->ctx;
    if (!self)
        return;

    if (!self->view) {
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        self->view = (SvEPGView) QBPlaylistManagerGetCurrent(playlists);
        SVTESTRETAIN(self->view);
    }
    if (!self->view)
        return;

    info->currentTimeRange = SvTimeRangeConstruct(self->activeTime - info->autoSearchLimit, self->activeTime + info->autoSearchLimit);
    if (!channel) {
        XMBVerticalMenuConnectToDataSource(info->xmbMenu, NULL, info->itemController, NULL);
        if (info->xmbMenuExt)
            XMBVerticalMenuConnectToDataSource(info->xmbMenuExt, NULL, info->itemControllerExt, NULL);
        QBAsyncLabelSetText(info->description, SVSTRING(""));
        QBTitleSetText(info->title, SVSTRING(""), 1);
        /// FIXME: set self->channel to NULL ??
        return;
    }

    SVTESTRELEASE(self->channel);
    self->channel = SVTESTRETAIN(channel);

    XMBVerticalMenuConnectToDataSource(info->xmbMenu, (SvGenericObject) self->dataSource, info->itemController, NULL);
    if (info->xmbMenuExt)
        XMBVerticalMenuConnectToDataSource(info->xmbMenuExt, (SvGenericObject) self->dataSource, info->itemControllerExt, NULL);
    SvEPGEventsListSetChannel(self->dataSource, self->channel, NULL);
    if (!(SvEPGManagerCheckIfRequestExists(self->epgManager, NULL, self->channel))) {
        SvEPGEventsListSetTimeRange(self->dataSource, &(info->currentTimeRange), NULL);
    }

    int idx = SvInvokeInterface(SvEPGChannelView, self->view, getChannelIndex, channel);
    if (info->arrowLeft)
        svIconSwitch(info->arrowLeft, idx > 0 ? 1 : 0, 0, info->arrowFadeTime);
    if (info->arrowRight) {
        size_t cnt = SvInvokeInterface(SvEPGChannelView, self->view, getCount);
        svIconSwitch(info->arrowRight, ((ssize_t) idx + 1 < (ssize_t) cnt) ? 1 : 0, 0, info->arrowFadeTime);
    }

    QBVerticalEPGWindowSetPosition(info, self->activeTime);

    QBInfoIconsSetByTVChannel(info->infoIcons, channel);
    QBTVChannelFrameSetChannel(info->channelFrame, channel);
    VerticalEPGWindowUpdateEvent(info);
}

SvLocal
void VerticalEPGWindowManualChannelSelection(void *prv, SvTVChannel channel)
{
    EPGVerticalInfo self = prv;
    QBVerticalEPGWindowSetChannel(self, channel);
}

SvLocal bool
VerticalEPGWindowInputEventHandlerImpl(EPGVerticalInfo info, const QBInputEvent *ev, bool direct)
{
    VerticalEPGWindow epg = info->ctx;

    if (ev->type != QBInputEventType_keyTyped) {
        return false;
    }

    if (QBTVChannelFrameInputEventHandler(info->channelFrame, ev)) {
        return true;
    }

    if (QBVerticalEPGLogicInputHandler(epg->appGlobals->verticalEPGLogic, ev)) {
        return true;
    }

    unsigned keyCode = ev->u.key.code;

    if (direct) {
        SvTVChannel ch = NULL;
        switch (keyCode) {
            case QBKEY_LEFT:
            case QBKEY_CHDN:
                ch = getChannelByOffset(epg->view, epg->channel, -1);
                if (ch)
                    QBVerticalEPGWindowSetChannel(info, ch);
                return true;
            case QBKEY_RIGHT:
            case QBKEY_CHUP:
                ch = getChannelByOffset(epg->view, epg->channel, 1);
                if (ch)
                    QBVerticalEPGWindowSetChannel(info, ch);
                return true;
            default:
                break;
        }
    }

    QBAppPVRDiskState diskState = QBAppStateServiceGetPVRDiskState(epg->appGlobals->appState);

    switch (keyCode) {
        case QBKEY_STOP:
            if (diskState == QBPVRDiskState_pvr_present || diskState == QBPVRDiskState_pvr_ts_present)
                QBVerticalEPGShowSideMenu((QBWindowContext) epg, SVSTRING(""), QBVerticalEPGSideMenuType_Stop);  //Show appropriate side menu
            return true;
        case QBKEY_EPG:
        {
            int pos = XMBVerticalMenuGetPosition(info->xmbMenu);
            VerticalEPGWindowArrowsSetActive(info, pos);
            SvEPGEvent event = epg->dataSource ? (SvEPGEvent) SvInvokeInterface(QBListModel, (epg->dataSource), getObject, pos) : NULL;
            if (event) {
                svNewTVGuideSetFocus(epg->appGlobals->newTVGuide, event);
            } else {
                //svNewTVGuideSetFocusEx(epg->appGlobals->newTVGuide, epg->channel, epg->activeTime);
            }
            QBApplicationControllerSwitchTop(epg->appGlobals->controller, epg->appGlobals->newTVGuide);
            return true;
        }
    }

    return false;
}

SvLocal bool
VerticalEPGWindowInputEventHandler(SvWidget w, SvInputEvent ev)
{
    EPGVerticalInfo info = w->prv;
    return VerticalEPGWindowInputEventHandlerImpl(info, &ev->fullInfo, true);
}

SvLocal bool
VerticalEPGWindowWCTXInputEventHandler(SvGenericObject w, const QBInputEvent *ev)
{
    VerticalEPGWindow self = (VerticalEPGWindow) w;
    EPGVerticalInfo info = self->super_.window->prv;

    return VerticalEPGWindowInputEventHandlerImpl(info, ev, false);
}

SvLocal void QBVerticalEPGMenuItemController__dtor__(void *ptr)
{
    QBVerticalEPGMenuItemController self = ptr;
    SVTESTRELEASE(self->icon);
    SVTESTRELEASE(self->inactiveBG);
    SVTESTRELEASE(self->focus);
    QBXMBItemConstructorDestroy(self->itemConstructor);

    SVTESTRELEASE(self->overlayObj);
}

SvLocal SvWidget QBVerticalEPGMenuItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBVerticalEPGMenuItemController self = (QBVerticalEPGMenuItemController) self_;

    QBXMBItem item = QBXMBItemCreate();
    QBFrameParameters params = { .bitmap = NULL };

    SvEPGEvent event = NULL;

    if (SvObjectIsInstanceOf(node_, SvEPGEvent_getType())) {
        event = (SvEPGEvent) node_;
    }

    if (self->useText && event) {
        char bufstart[MAX_DATE_LENGTH], bufend[MAX_DATE_LENGTH];
        struct tm timeTmp;
        SvTimeBreakDown(SvTimeConstruct(event->startTime, 0), true, &timeTmp);
        strftime(bufstart, MAX_DATE_LENGTH, QBTimeFormatGetCurrent()->verticalEPGTime, &timeTmp);
        SvTimeBreakDown(SvTimeConstruct(event->endTime, 0), true, &timeTmp);
        strftime(bufend, MAX_DATE_LENGTH, QBTimeFormatGetCurrent()->verticalEPGTime, &timeTmp);
        SvString title = QBEventUtilsGetTitleFromEvent(self->appGlobals->eventsLogic, self->appGlobals->langPreferences, event);
        if (self->showDetailed) {
            item->caption = SVRETAIN(title);
            item->subcaption = SvStringCreateWithFormat("%s - %s", bufstart, bufend);
            SvEPGEventDesc desc = QBEventUtilsGetDescFromEvent(self->appGlobals->eventsLogic, self->appGlobals->langPreferences, event);
            SvString text = NULL;
            if (desc) {
                if (desc->description)
                    text = desc->description;
                else if (desc->shortDescription)
                    text = desc->shortDescription;
            }
            item->description = SVTESTRETAIN(text);
        } else {
            item->caption = SvStringCreateWithFormat("%s %s", bufstart, SvStringCString(title));
        }
    }

    if (self->useBackground) {
        if (event)
            params.bitmap = SvInvokeInterface(SvEPGDisplayer, self->ctx->displayer, getBackground, event);
        else
            params.bitmap = self->inactiveBG;
        params.height = self->itemHeight;
        params.minWidth = params.maxWidth = params.width = self->focus->data.width;
        item->focus = SVRETAIN(self->focus);
        item->icon = SVTESTRETAIN(self->icon);
        item->inactiveFocus = QBFrameDescriptionCreateWithParameters("", &params);
    }

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);

    if (self->overlayObj && event) {
        SvTVChannel channel = self->ctx->channel;

        SvWidget over = SvInvokeInterface(SvEPGOverlay, self->overlayObj, setupEvent,
                                          ret, channel, event, 0, -1);
        if (over)
            svWidgetAttach(ret, over, 0, 0, 1);
    }

    return ret;
}

SvLocal void QBVerticalEPGMenuItemControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBVerticalEPGMenuItemController self = (QBVerticalEPGMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal SvType QBVerticalEPGMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVerticalEPGMenuItemController__dtor__
    };
    static SvType type = NULL;
    static const struct XMBItemController_t controllerMethods = {
        .createItem   = QBVerticalEPGMenuItemControllerCreateItem,
        .setItemState = QBVerticalEPGMenuItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVerticalEPGMenuItemController",
                            sizeof(struct QBVerticalEPGMenuItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controllerMethods,
                            NULL);
    }

    return type;
}

SvLocal void VerticalEPGWindowClean(SvApplication app, void *ptr)
{
    EPGVerticalInfo info = (EPGVerticalInfo) ptr;

    SvFiberDestroy(info->fiber);
    info->scheduler = NULL;
    info->fiber = NULL;
    SVRELEASE(info->sidemenu.ctx);
    info->sidemenu.ctx = NULL;
    SVRELEASE(info->itemController);
    SVTESTRELEASE(info->itemControllerExt);
    free(info);
}


SvLocal SvWidget
VerticalEPGArrowFromSM(SvWidget window, const char *name)
{
    if (!svSettingsIsWidgetDefined(name))
        return NULL;

    SvBitmap inactive = svSettingsGetBitmap(name, "inactive");
    unsigned int width = inactive->width, height = inactive->height;

    SvBitmap active = svSettingsGetBitmap(name, "active");
    if (active->width)
        width = active->width;
    if (active->height)
        height = active->height;

    SvWidget w = svIconNewWithSize(window->app, width, height, false);
    svIconSetBitmap(w, 0, inactive);
    svIconSetBitmap(w, 1, active);
    svSettingsWidgetAttach(window, w, name, 10);

    return w;
}

QBWindowContext QBVerticalEPGContextCreate(AppGlobals appGlobals)
{
    VerticalEPGWindow self;

    self = (VerticalEPGWindow) SvTypeAllocateInstance(VerticalEPGWindow_getType(), NULL);

    if (!self) return NULL;

    self->appGlobals = appGlobals;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    self->view = (SvEPGView) QBPlaylistManagerGetCurrent(playlists);
    self->epgManager = SVRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));
    SVTESTRETAIN(self->view);

    if (appGlobals->dsmccMonitor) {
        self->adsRotor = QBDSMCCAdsRotationServiceCreate();
        QBDSMCCAdsRotationServiceAddListener(self->adsRotor, (SvObject) self);
    }

    return (QBWindowContext) self;
}

void QBVerticalEPGShowExtendedInfo(QBWindowContext self_)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;
    if (!self->super_.window)
        return;
    EPGVerticalInfo info = self->super_.window->prv;
    if (!info->sidemenu.ctx || !self->dataSource)
        return;

    int pos = XMBVerticalMenuGetPosition(info->xmbMenu);
    SvEPGEvent event = (SvEPGEvent) SvInvokeInterface(QBListModel, self->dataSource, getObject, pos);

    if (!info->sidemenu.shown && event) {
        info->sidemenu.shown = true;
        SvArray events = SvArrayCreate(NULL);
        SvArrayAddObject(events, (SvGenericObject) event);
        QBExtendedInfoPane pane = QBExtendedInfoPaneCreateFromSettings("ReminderEditorPane.settings", self->appGlobals,
                                                                       info->sidemenu.ctx, SVSTRING("ExtendedInfo"), 1, events);
        SVRELEASE(events);
        QBContextMenuPushPane(info->sidemenu.ctx, (SvGenericObject) pane);
        QBContextMenuShow(info->sidemenu.ctx);
        SVRELEASE(pane);
    }
}

SvLocal void
QBVerticalEPGSearchKeyTyped(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    EPGVerticalInfo self = ptr;

    if (key->type == QBOSKKeyType_enter && SvStringLength(input)) {
        QBWindowContext searchResults = QBSearchResultsContextCreate(self->ctx->appGlobals);

        SvArray agents = SvArrayCreate(NULL);
        SvString playlistId = SvTVChannelGetAttribute(self->ctx->channel, SVSTRING("TV")) ? SVSTRING("TVChannels") : SVSTRING("RadioChannels");
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
QBVerticalEPGCreateAndPushSearchPane(VerticalEPGWindow self)
{
    EPGVerticalInfo info = self->super_.window->prv;
    //Prepare OSK
    svSettingsPushComponent("OSKPane.settings");
    QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
    SvErrorInfo error = NULL;
    QBOSKPaneInit(oskPane, self->appGlobals->scheduler, info->sidemenu.ctx, 1, SVSTRING("OSKPane"),
                  QBVerticalEPGSearchKeyTyped, info, &error);
    svSettingsPopComponent();
    if (!error) {
        QBContextMenuPushPane(info->sidemenu.ctx, (SvGenericObject) oskPane);
        QBContextMenuShow(info->sidemenu.ctx);
    } else {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVRELEASE(oskPane);
}

SvLocal void QBVerticalEPGShowSideMenu_(VerticalEPGWindow self, SvString id, size_t idx, QBVerticalEPGSideMenuType type, bool useID)
{
    if (!self->super_.window) {
        return;
    }

    EPGVerticalInfo info = self->super_.window->prv;
    if (!info->sidemenu.ctx || !self->dataSource) {
        return;
    }

    int pos = XMBVerticalMenuGetPosition(info->xmbMenu);
    SvEPGEvent event = (SvEPGEvent) SvInvokeInterface(QBListModel, self->dataSource, getObject, pos);

    if (!self->channel) {
        return;
    }

    if (info->sidemenu.shown) {
        return;
    }

    QBReminderEditorPane pane = NULL;
    const char *allowedOptions = NULL;

    switch (type) {
        case QBVerticalEPGSideMenuType_NPvrSeries:
            pane = QBReminderEditorPaneCreateAndPushWithNPvrFromSettings("ReminderEditorPane.settings", self->appGlobals, info->sidemenu.ctx, SVSTRING("ReminderEditorWithNPvr"), self->channel, event, QBReminderEditorPaneType_Series);
            break;
        case QBVerticalEPGSideMenuType_NPvrKeyword:
            pane = QBReminderEditorPaneCreateAndPushWithNPvrFromSettings("ReminderEditorPane.settings", self->appGlobals, info->sidemenu.ctx, SVSTRING("ReminderEditorWithNPvr"), self->channel, event, QBReminderEditorPaneType_Keyword);
            break;
        case QBVerticalEPGSideMenuType_Stop:
            allowedOptions = "EPGReminderEditorStopPane.json";
            break;
        case QBVerticalEPGSideMenuType_SearchOnly:
            QBVerticalEPGCreateAndPushSearchPane(self);
            break;
        case QBVerticalEPGSideMenuType_Full:
        default:
            allowedOptions = "EPGReminderEditorPane.json";
            break;
    }

    if (pane || !allowedOptions) {
        info->sidemenu.shown = true;
        SVTESTRELEASE(pane);
        return;
    }

    pane = QBReminderEditorPaneCreateFromSettings("ReminderEditorPane.settings", allowedOptions, self->appGlobals, info->sidemenu.ctx, SVSTRING("ReminderEditor"), self->channel, event);

    if (!pane) {
        return;
    }

    info->sidemenu.shown = true;

    QBContextMenuPushPane(info->sidemenu.ctx, (SvGenericObject) pane);
    QBContextMenuShow(info->sidemenu.ctx);

    if (useID) {
        QBReminderEditorPaneSetPosition(pane, id, true);
    } else {
        QBReminderEditorPaneSetPositionByIndex(pane, idx, true);
    }

    SVRELEASE(pane);
}

void QBVerticalEPGShowSideMenu(QBWindowContext self_, SvString id, QBVerticalEPGSideMenuType type)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;
    QBVerticalEPGShowSideMenu_(self, id, 0, type, true);
}

void QBVerticalEPGShowSideMenuOnIndex(QBWindowContext self_, size_t idx, QBVerticalEPGSideMenuType type)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;
    QBVerticalEPGShowSideMenu_(self, NULL, idx, type, false);
}

void QBVerticalEPGShowRecord(QBWindowContext self_)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;

    if (!self || !self->super_.window || !self->dataSource)
        return;

    EPGVerticalInfo info = self->super_.window->prv;

    int pos = XMBVerticalMenuGetPosition(info->xmbMenu);
    SvEPGEvent activeEvent = (SvEPGEvent) SvInvokeInterface(QBListModel, self->dataSource, getObject, pos);

    SvTVChannel channel = self->channel;
    if (!info->sidemenu.shown && channel && activeEvent) {
        QBReminderEditorPane pane = QBReminderEditorPaneCreateFromSettings("ReminderEditorPane.settings", "EPGReminderEditorPane.json", self->appGlobals, info->sidemenu.ctx, SVSTRING("ReminderEditor"), channel, activeEvent);

        if (!pane)
            return;
        info->sidemenu.shown = true;
        QBContextMenuPushPane(info->sidemenu.ctx, (SvObject) pane);
        QBContextMenuShow(info->sidemenu.ctx);
        QBReminderEditorPaneSetPosition(pane, SVSTRING("record"), true);
        QBReminderEditorPaneRecord(pane);
        SVRELEASE(pane);
    }
}

bool QBVerticalEPGStopRecording(QBWindowContext self_)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;
    SvEPGEvent event = NULL;
    QBVerticalEPGGetActiveEvent(self_, &event);
    if (!event)
        return false;

    return (QBRecordingUtilsStopRecordingForEvent(self->appGlobals->pvrProvider, event));
}

bool QBVerticalEPGIsSideMenuShown(QBWindowContext self_)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;
    if (!self->super_.window)
        return false;
    EPGVerticalInfo info = self->super_.window->prv;
    if (!info->sidemenu.ctx)
        return false;

    return info->sidemenu.shown;
}

void QBVerticalEPGHideSideMenu(QBWindowContext self_)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;
    if (!self->super_.window)
        return;
    EPGVerticalInfo info = self->super_.window->prv;
    if (!info->sidemenu.ctx)
        return;
    if (info->sidemenu.shown) {
        info->sidemenu.shown = false;
        QBContextMenuHide(info->sidemenu.ctx, false);
    }
}

void QBVerticalEPGSetFocus(QBWindowContext self_, SvTVChannel ch, time_t activeTime)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject current = QBPlaylistManagerGetCurrent(playlists);
    if (current != (SvGenericObject) self->view) {
        SVTESTRELEASE(self->view);
        self->view = SVTESTRETAIN(current);
    }
    if (activeTime) {
        self->activeTime = activeTime;
    } else {
        self->activeTime = SvTimeNow();
    }
    if (!ch)
        ch = getChannelByOffset(self->view, SvTVContextGetCurrentChannel(self->appGlobals->newTV), 0);

    if (!self->super_.window) {
        if (ch) {
            SVTESTRELEASE(self->channel);
            self->channel = SVRETAIN(ch);
        }
        return;
    }
    QBVerticalEPGWindowSetChannel(self->super_.window->prv, ch);
}

void QBVerticalEPGGetActiveEvent(QBWindowContext self_, SvEPGEvent *event)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;
    if (!self->super_.window)
        return;
    EPGVerticalInfo info = self->super_.window->prv;

    if (event == NULL) {
        SvLogError("NULL pointer");
        return;
    }

    if (!self->dataSource) {
        *event = NULL;
    } else {
        int pos = XMBVerticalMenuGetPosition(info->xmbMenu);
        *event = (SvEPGEvent) SvInvokeInterface(QBListModel, self->dataSource, getObject, pos);
    }
}

SvTVChannel QBVerticalEPGGetChannel(QBWindowContext self_)
{
    VerticalEPGWindow self = (VerticalEPGWindow) self_;
    return self->channel;
}


QBContextMenu QBVerticalEPGGetContextMenu(QBWindowContext epg_)
{
    VerticalEPGWindow self = (VerticalEPGWindow) epg_;
    EPGVerticalInfo info = self->super_.window->prv;
    return info->sidemenu.ctx;
}
