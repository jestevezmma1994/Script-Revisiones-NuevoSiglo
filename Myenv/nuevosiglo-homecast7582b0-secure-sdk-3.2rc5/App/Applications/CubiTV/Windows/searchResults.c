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

#include "searchResults.h"

#include <libintl.h>
#include <time.h>
#include <SvCore/SvTime.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvEPG/SvEPGDisplayer.h>
#include <SvEPG/SvEPGOverlay.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvArray.h>
#include <QBInput/QBInputCodes.h>
#include <QBSearch/QBSearchEngine.h>
#include <QBSearch/QBSearchProgressListener.h>
#include <SWL/button.h>
#include <SWL/clock.h>
#include <SWL/QBFrame.h>
#include <SWL/viewport.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBVerticalMenu.h>
#include <Widgets/QBEPGDisplayer.h>
#include <Menus/menuchoice.h>
#include <Windows/tv/QBTVChannelFrame.h>
#include <Widgets/QBXMBSearchConstructor.h>
#include <Widgets/QBScrollBar.h>
#include <Widgets/QBTitle.h>
#include <Widgets/QBRecordingIcon.h>
#include <Widgets/tvEPGOverlay.h>
#include <Widgets/customerLogo.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <QBWidgets/QBSearchProgressDialog.h>
#include <Logic/SearchResultsLogic.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBReminderEditorPane.h>
#include <ContextMenus/QBExtendedInfoPane.h>
#include <Utils/QBEventUtils.h>
#include <Logic/timeFormat.h>
#include <QBDataModel3/QBDataModel.h>
#include <main.h>
#include <Logic/GUILogic.h>

#define MAX_DATE_LENGTH 128

typedef struct SearchResultsContext_ *SearchResultsContext;
struct SearchResultsContext_ {
    struct QBWindowContext_t super_;
    AppGlobals appGlobals;
    SvObject dataSource;
    SvEPGView view;
    bool setDataSource; // information for reinitialize
    QBSearchProgressDialog searchProgressDialog; ///< dialog shown during search and in case of lack of results
    QBContextMenu menuToBringBack; ///< context menu hidden during search and brought back after cancelling the search
    SvArray contextStack; ///< pane stack of hidden menuToBringBack; hiding menu releases panes, but they are retained here
};

typedef struct searchResultsWindow_t *searchResultsWindow;
struct searchResultsWindow_t {
    SearchResultsContext ctx;

    SvWidget logo;
    SvWidget channelFrame;
    SvWidget description;
    SvWidget channelName;
    double arrowFadeTime;

    SvWidget xmbMenu, xmbScrollBar;

    SvWidget viewPort;
    SvWidget title;
    SvFont font;
    SvColor textColor;
    int fontSize, maxTextWidth;

    int resultCount;
    int resultCountStep;

    SvWidget viewportFocus;

    struct {
       QBContextMenu ctx;
       bool shown;
    } sidemenu;
};

SvLocal void QBSearchEngineOnSideMenuClose(void *self_, QBContextMenu menu)
{
    searchResultsWindow self = self_;
    self->sidemenu.shown = false;
    // We refresh cause user could set reminder and we want to show it on search results.
    size_t length = SvInvokeInterface(QBListModel, (self->ctx->dataSource), getLength);
    QBListDataSourceNotifyItemsChanged((QBListDataSource) self->ctx->dataSource, 0, length);
}

SvLocal void setupSearchProgressDialog(SearchResultsContext self)
{
    AppGlobals appGlobals = self->appGlobals;
    SvErrorInfo error = NULL;
    svSettingsPushComponent("searchResults.settings");
    self->searchProgressDialog = QBSearchProgressDialogCreate(appGlobals->res, appGlobals->controller, &error);
    svSettingsPopComponent();
    if (error) {
        goto err;
    }
    QBSearchProgressDialogSetListener(self->searchProgressDialog, (SvObject) self, &error);
    if (error) {
        goto err;
    }
    QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_progressState, &error);
    if (error) {
        goto err;
    }
    return;
err:
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
}

SvLocal bool dayEq(const struct tm *a, const struct tm *b)
{
    return (a->tm_mday == b->tm_mday) && (a->tm_mon == b->tm_mon) && (a->tm_year == b->tm_year);
}

SvLocal SvString smartDate(time_t timePoint)
{
    time_t now = SvTimeNow();
    time_t tomorrow = now + 24 * 3600;
    struct tm now_tm, time_tm, tomorrow_tm;

    SvTimeBreakDown(SvTimeConstruct(timePoint, 0), true, &time_tm);

    SvTimeBreakDown(SvTimeConstruct(now, 0), true, &now_tm);
    if (dayEq(&now_tm, &time_tm))
        return SvStringCreate(gettext("today"), NULL);

    SvTimeBreakDown(SvTimeConstruct(tomorrow, 0), true, &tomorrow_tm);
    if (dayEq(&tomorrow_tm, &time_tm))
        return SvStringCreate(gettext("tomorrow"), NULL);

    char buf[MAX_DATE_LENGTH];
    strftime(buf, MAX_DATE_LENGTH, QBTimeFormatGetCurrent()->searchResultsDate, &time_tm);
    return SvStringCreate(buf, NULL);
}

SvLocal bool QBSearchResultsInputEventHandler(SvWidget w, SvInputEvent event)
{
    searchResultsWindow self = w->prv;

    if (QBSearchResultsLogicHandleInputEvent(self->ctx->appGlobals->searchResultsLogic, (QBWindowContext) self->ctx, event))
        return true;

    switch (event->ch) {
        case QBKEY_DOWN:
        case QBKEY_UP:
            break;
        default:
            return false;
    }

    return true;
}

struct QBSearchResultsMenuItemController_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvBitmap icon;
    SvBitmap inactiveBG;
    QBFrameConstructor* focus;
    QBXMBSearchItemConstructor itemConstructor;
    SvEPGView view;

    SvGenericObject displayer;
    SvGenericObject  overlayObj;
};
typedef struct QBSearchResultsMenuItemController_t* QBSearchResultsMenuItemController;

SvLocal void QBSearchResultsMenuItemController__dtor__(void *ptr)
{
    QBSearchResultsMenuItemController self = ptr;
    SVRELEASE(self->icon);
    SVRELEASE(self->inactiveBG);
    SVRELEASE(self->focus);
    SVTESTRELEASE(self->view);
    SVTESTRELEASE(self->displayer);
    QBXMBSearchItemConstructorDestroy(self->itemConstructor);
    SVRELEASE(self->overlayObj);
}

SvLocal SvWidget QBSearchResultsMenuItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_,
                                                             SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBSearchResultsMenuItemController self = (QBSearchResultsMenuItemController) self_;
    QBXMBSearchItem item = (QBXMBSearchItem) SvTypeAllocateInstance(QBXMBSearchItem_getType(), NULL);
    SvEPGEvent event = NULL;
    QBFrameConstructData params = { .bitmap = NULL };

    if (SvObjectIsInstanceOf(node_, SvEPGEvent_getType())) {
        event = (SvEPGEvent) node_;
        char buf[MAX_DATE_LENGTH];
        struct tm startTime;
        SvTimeBreakDown(SvTimeConstruct(event->startTime, 0), true, &startTime);
        strftime(buf, MAX_DATE_LENGTH, QBTimeFormatGetCurrent()->searchResultsTime, &startTime);
        item->time = SvStringCreate(buf, NULL);
        item->date = smartDate(event->startTime);
        item->title = SVTESTRETAIN(QBEventUtilsGetTitleFromEvent(self->appGlobals->eventsLogic, self->appGlobals->langPreferences, event));
        SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, self->view, getByID, event->channelID);
        if (channel) {
            QBServiceRegistry registry = QBServiceRegistryGetInstance();
            QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
            item->channel = SvStringCreateWithFormat("%03d", QBPlaylistManagerGetGlobalNumberOfChannel(playlists, channel));
        }
    } else {
        item->date = SVSTRING("?");
        item->time = SVSTRING("?");
        item->channel = SVSTRING("?");
        item->title = SVSTRING("?");
    }

    SvWidget ret = NULL;

    item->focus = SVRETAIN(self->focus);
    item->icon = SVRETAIN(self->icon);
    ret = QBXMBSearchItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);
    SVRELEASE(self->inactiveBG);
    self->inactiveBG = SVRETAIN(SvInvokeInterface(SvEPGDisplayer, self->displayer, getBackground, event));
    params.bitmap = self->inactiveBG;
    params.height = self->inactiveBG->height;
    params.width = self->inactiveBG->width;
    params.minWidth = 0;

    SvWidget w = QBFrameCreate(app, &params);
    QBFrameSetWidth(w, ret->width);

    svWidgetAttach(ret, w, (ret->width - w->width) / 2, (ret->height - w->height) / 2, 0);

    if (SvObjectIsInstanceOf(node_, SvEPGEvent_getType())) {
        event = (SvEPGEvent) node_;
        SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, self->view, getByID, event->channelID);

        SvWidget over = SvInvokeInterface(SvEPGOverlay, self->overlayObj, setupEvent,
                                          ret, channel, event, 0, -1);
        if (over)
            svWidgetAttach(ret, over, 0, 0, 5);
    }

    return ret;
}

SvLocal void QBSearchResultsMenuItemControllerSetItemState(SvGenericObject self_, SvWidget item_,
                                                           XMBMenuState state, bool isFocused)
{
    QBSearchResultsMenuItemController self = (QBSearchResultsMenuItemController) self_;
    QBXMBSearchItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal SvType QBSearchResultsMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSearchResultsMenuItemController__dtor__
    };
    static SvType type = NULL;

    static const struct XMBItemController_t controllerMethods = {
        .createItem = QBSearchResultsMenuItemControllerCreateItem,
        .setItemState = QBSearchResultsMenuItemControllerSetItemState
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSearchResultsMenuItemController",
                            sizeof(struct QBSearchResultsMenuItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controllerMethods, NULL);
    }

    return type;
}

SvLocal void QBDescFill(void *prv_, QBTextRendererTask task)
{
    SvBitmap bitmap = task->surface;
    searchResultsWindow self = prv_;
    svWidgetSetBitmap(self->description, bitmap);
    self->description->width = bitmap->width;
    self->description->height = bitmap->height;

    //XXX hack - set position to 0
    svViewPortSetContents(self->viewPort, NULL);
    svViewPortSetContents(self->viewPort, self->description);

    svViewPortContentsSizeChanged(self->viewPort);
}

SvLocal void searchResultsRefresh(searchResultsWindow self)
{
    if (!self->ctx->setDataSource)
        return;
    int pos = XMBVerticalMenuGetPosition(self->xmbMenu);
    if(pos < 0) return;
    SvGenericObject item_ = SvInvokeInterface(QBListModel, (self->ctx->dataSource), getObject, pos);
    SvString text = NULL;
    if (SvObjectIsInstanceOf(item_, SvEPGEvent_getType())) {
        SvEPGEvent event = (SvEPGEvent) item_;
        AppGlobals appGlobals = self->ctx->appGlobals;
        SvEPGEventDesc desc = QBEventUtilsGetDescFromEvent(appGlobals->eventsLogic, appGlobals->langPreferences, event);
        text = (desc && desc->description) ? desc->description : SVSTRING("");
        // QBAsyncLabelSetText(self->description, event->description);
        SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, self->ctx->view, getByID, event->channelID);
        if (channel) {
            QBTVChannelFrameSetChannelName(self->channelFrame, channel->name);
            QBServiceRegistry registry = QBServiceRegistryGetInstance();
            QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
            QBTVChannelFrameSetChannelNumber(self->channelFrame, QBPlaylistManagerGetGlobalNumberOfChannel(playlists, channel));
        };
    }

    SvBitmap bitmap = QBTextRendererRender(self->ctx->appGlobals->textRenderer, text, QBDescFill, self, self->font,
            self->fontSize, self->textColor, self->maxTextWidth);
    svWidgetSetBitmap(self->description, bitmap);
    self->description->width = bitmap->width;
    self->description->height = bitmap->height;
}

SvLocal void searchResultsSetAlpha(searchResultsWindow self, SvPixel alpha){
    self->xmbMenu->tree_alpha = alpha;
    self->description->tree_alpha = alpha;
    self->channelFrame->tree_alpha = alpha;
    self->xmbScrollBar->tree_alpha = alpha;
}

SvLocal void QBSearchResultsClean(SvApplication app, void *ptr)
{
    searchResultsWindow self = ptr;
    SVRELEASE(self->sidemenu.ctx);
    free(ptr);
}

SvLocal void SearchResultsContextReinitializeWindow(QBWindowContext ctx_, SvArray itemList)
{
    SearchResultsContext ctx = (SearchResultsContext) ctx_;
    searchResultsWindow self = (searchResultsWindow) ctx->super_.window->prv;
    if (!itemList || SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("CustomerLogo"))) {
        SvString logoWidgetName = QBInitLogicGetLogoWidgetName(ctx->appGlobals->initLogic);
        if (logoWidgetName)
            self->logo = QBCustomerLogoReplace(self->logo, ctx->super_.window, logoWidgetName, 1);
    }

    if (ctx->setDataSource && (!itemList || SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("QBParentalControl")))) {
        searchResultsRefresh(self);
        int cnt = SvInvokeInterface(QBListModel, ctx->dataSource, getLength);
        SvInvokeInterface(QBListModelListener, (SvGenericObject)self->xmbMenu->prv, itemsChanged, 0, cnt);
    }

}

SvLocal void
searchResultsOnChangePosition(void *w_, const XMBMenuEvent data)
{
    searchResultsWindow self = ((SvWidget ) w_)->prv;
    if(data->clicked) {
        QBSearchResultsContextShowSideMenu((QBWindowContext) self->ctx, QBSearchResultsLogicGetDefaultSideMenuItemId(self->ctx->appGlobals->searchResultsLogic));
    }

    searchResultsRefresh(self);
}

SvLocal void refreshResultCount(searchResultsWindow self, bool force)
{
    int len = SvInvokeInterface(QBListModel, self->ctx->dataSource, getLength);
    if (len - self->resultCount > self->resultCountStep || force) {
        SvString title = SvStringCreateWithFormat("%i %s", len, ngettext("result", "results", len));
        QBTitleSetText(self->title, title, 2);
        SVRELEASE(title);
        self->resultCount = len;
        if (len / self->resultCountStep > 3) {
            self->resultCountStep *= 2;
        }
    }
}

SvLocal void SearchResultsContextUpdateFill(SearchResultsContext ctx)
{
    searchResultsWindow self = (searchResultsWindow) ctx->super_.window->prv;
    refreshResultCount(self, true);
    searchResultsSetAlpha(self, ALPHA_SOLID);
    svSettingsPushComponent("searchResults.settings");
    AppGlobals appGlobals = ctx->appGlobals;
    QBSearchResultsMenuItemController itemController = (QBSearchResultsMenuItemController) SvTypeAllocateInstance(
        QBSearchResultsMenuItemController_getType(), NULL);

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    itemController->view = SVRETAIN(QBPlaylistManagerGetById(playlists, SVSTRING("AllChannels")));

    itemController->appGlobals = appGlobals;
    itemController->icon = SVRETAIN(svSettingsGetBitmap("MenuItem", "icon"));
    itemController->inactiveBG = SVRETAIN(svSettingsGetBitmap("MenuItem", "bg"));
    itemController->focus = QBFrameConstructorFromSM("MenuItem.focus");
    itemController->itemConstructor = QBXMBSearchItemConstructorCreate(appGlobals->res, "MenuItem", appGlobals->textRenderer);

    itemController->overlayObj = tvEPGOverlayCreate(appGlobals, NULL, NULL, "searchResultsWindow", NULL);
    itemController->displayer = QBEPGDisplayerCreate(appGlobals, "EPGWidget");

    XMBVerticalMenuConnectToDataSource(self->xmbMenu, ctx->dataSource, (SvObject) itemController, NULL);
    ctx->setDataSource = true;
    SVRELEASE(itemController);
    searchResultsRefresh(self);
    svSettingsPopComponent();
}

SvLocal void SearchResultsContextCreateWindow(QBWindowContext ctx_, SvApplication app)
{
    SearchResultsContext ctx = (SearchResultsContext) ctx_;
    searchResultsWindow self = (searchResultsWindow) calloc(sizeof(struct searchResultsWindow_t), 1);
    AppGlobals appGlobals = ctx->appGlobals;

    svSettingsPushComponent("searchResults.settings");

    SvWindow window = NULL;

    window = QBGUILogicCreateBackgroundWidget(ctx->appGlobals->guiLogic, "searchResultsWindow", NULL);

    self->arrowFadeTime = svSettingsGetDouble("searchResultsWindow", "arrowFadeTime", 0.0);
    window->prv = self;
    svWidgetSetInputEventHandler(window, QBSearchResultsInputEventHandler);
    window->clean = QBSearchResultsClean;
    self->ctx = ctx;

    self->resultCountStep = 1;

    self->channelName = QBAsyncLabelNew(app, "channelName", appGlobals->textRenderer);
    svSettingsWidgetAttach(window, self->channelName, svWidgetGetName(self->channelName), 10);

    self->viewportFocus = svSettingsWidgetCreate(app, "viewport.focus");
    svSettingsWidgetAttach(window, self->viewportFocus, svWidgetGetName(self->viewportFocus), 0);
    SvWidget w;
    w = svSettingsWidgetCreate(app, "viewport.focus.left");
    svSettingsWidgetAttach(self->viewportFocus, w, svWidgetGetName(w), 0);
    w = svSettingsWidgetCreate(app, "viewport.focus.right");
    svSettingsWidgetAttach(self->viewportFocus, w, svWidgetGetName(w), 0);
    self->viewportFocus->tree_alpha = ALPHA_TRANSPARENT;

    SvWidget wClock = svClockNew(app, "clock");
    svSettingsWidgetAttach(window, wClock, svWidgetGetName(wClock), 1);

    SvWidget verticalBarWindow = svSettingsWidgetCreate(app, "verticalBarWindow");
    svSettingsWidgetAttach(window, verticalBarWindow, svWidgetGetName(verticalBarWindow), 0);

    self->xmbMenu = XMBVerticalMenuNew(app, "verticalBar", 0);
    svSettingsWidgetAttach(verticalBarWindow, self->xmbMenu, svWidgetGetName(self->xmbMenu), 10);
    XMBVerticalMenuSetNotificationTarget(self->xmbMenu, window, searchResultsOnChangePosition);

    SvWidget scrollBar = QBScrollBarNew(app, "XMBScrollBar");
    svSettingsWidgetAttach(window, scrollBar, svWidgetGetName(scrollBar), 2);
    XMBVerticalMenuAddRangeListener(self->xmbMenu, QBScrollBarGetRangeListener(scrollBar), NULL);
    self->xmbScrollBar = scrollBar;

    svWidgetSetFocusable(self->xmbMenu, true);
    svWidgetSetFocus(self->xmbMenu);

    const char *wName = "viewport";
    self->viewPort = svViewPortNew(app, wName);
    svSettingsWidgetAttach(window, self->viewPort, wName, 2);

    wName = "description";
    self->description = svSettingsWidgetCreate(app, wName);
    svViewPortSetContents(self->viewPort, self->description);
    self->fontSize = svSettingsGetInteger(wName, "fontSize", 0);
    self->textColor = svSettingsGetColor(wName, "textColor", 0);
    self->font = svSettingsCreateFont(wName, "font");
    self->maxTextWidth = self->description->width;

    svWidgetSetFocusable(self->viewPort, true);

    self->title = QBTitleNew(app, "Title", appGlobals->textRenderer);
    svSettingsWidgetAttach(window, self->title, "Title", 1);
    SvString title = SvStringCreate(gettext("Search"), NULL);
    QBTitleSetText(self->title, title, 0);
    if(ctx->dataSource){
        SvString keywords = QBSearchEngineGetKeywordsString(ctx->dataSource);
        QBTitleSetText(self->title, keywords, 1);
    }
    QBTitleSetText(self->title, title, 0);
    SVRELEASE(title);

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    self->channelFrame = QBTVChannelFrameNew(app, "frame", appGlobals->textRenderer, playlists, appGlobals->tvLogic);
    svSettingsWidgetAttach(window, self->channelFrame, svWidgetGetName(self->channelFrame), 10);
    searchResultsRefresh(self);

    svSettingsPopComponent();
    searchResultsSetAlpha(self, ALPHA_TRANSPARENT);

    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, app, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBSearchEngineOnSideMenuClose, self);

    ctx->super_.window = window;
    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(ctx->appGlobals->initLogic);
    if (logoWidgetName)
        self->logo = QBCustomerLogoAttach(ctx->super_.window, logoWidgetName, 1);

    SearchResultsContextUpdateFill(ctx);
}

SvLocal void SearchResultsContextDestroyWindow(QBWindowContext ctx_)
{
    SearchResultsContext ctx = (SearchResultsContext) ctx_;
    searchResultsWindow self = ctx->super_.window->prv;
    SVTESTRELEASE(self->font);
    svWidgetDestroy(ctx->super_.window);
    ctx->super_.window = NULL;
}

SvLocal void SearchResultsContextDestroy(void *self_)
{
    SearchResultsContext self = self_;
    SVTESTRELEASE(self->dataSource);
    SVTESTRELEASE(self->view);
    SVTESTRELEASE(self->searchProgressDialog);
    SVTESTRELEASE(self->contextStack);
}

SvLocal void SearchResultsContextItemsAdded(SvObject self_,
                                            size_t idx, size_t count)
{
    SearchResultsContext ctx = (SearchResultsContext) self_;
    if (ctx->super_.window) {
        searchResultsWindow self = (searchResultsWindow) ctx->super_.window->prv;
        refreshResultCount(self, false);
    }
}

SvLocal void SearchResultsContextItemsRemoved(SvObject self_,
                                              size_t idx, size_t count)
{
    SearchResultsContext ctx = (SearchResultsContext) self_;
    if (ctx->super_.window) {
        searchResultsWindow self = (searchResultsWindow) ctx->super_.window->prv;
        refreshResultCount(self, false);
    }
}

SvLocal void SearchResultsContextItemsChanged(SvObject self_,
                                              size_t idx, size_t count)
{
}

SvLocal void SearchResultsContextItemsReordered(SvObject self_,
                                                size_t idx, size_t count)
{
}

SvLocal void QBSearchResultsContextDropMenuToBringBack(SearchResultsContext self)
{
    self->menuToBringBack = NULL;
    SVTESTRELEASE(self->contextStack);
    self->contextStack = NULL;
}

SvLocal void QBSearchProgressUpdate(SvObject self_, int progress)
{
    SearchResultsContext ctx = (SearchResultsContext) self_;
    if (progress < 100) {
        return;
    }

    SvErrorInfo error = NULL;
    if (SvInvokeInterface(QBListModel, ctx->dataSource, getLength) > 0) {
        //don't bring side menu back after successful search
        QBSearchResultsContextDropMenuToBringBack(ctx);
        QBSearchProgressDialogSetState(ctx->searchProgressDialog, QBSearchProgressDialog_finishedState, &error);
        if (error) {
            goto err;
        }
        QBSearchProgressDialogHide(ctx->searchProgressDialog, &error);
        if (error) {
            goto err;
        }
        QBApplicationControllerPushContext(ctx->appGlobals->controller, (QBWindowContext) ctx);
    } else {
        if (ctx->searchProgressDialog) {
            QBSearchProgressDialogSetState(ctx->searchProgressDialog, QBSearchProgressDialog_noResultState, &error);
            if (error) {
                goto err;
            }
        }
    }
    return;
err:
    SVTESTRELEASE(ctx->searchProgressDialog);
    ctx->searchProgressDialog = NULL;
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
}

SvLocal void QBSearchResultsContextBringBackContextMenu(SearchResultsContext self)
{
    if (self->menuToBringBack) {
        for (SvIterator iterator = SvArrayGetIterator(self->contextStack); SvIteratorHasNext(&iterator);) {
            SvObject pane = (SvObject) SvIteratorGetNext(&iterator);
            QBContextMenuPushPane(self->menuToBringBack, pane);
        }
        QBContextMenuShow(self->menuToBringBack);
        self->menuToBringBack = NULL;
    }
    QBSearchResultsContextDropMenuToBringBack(self);
}

SvLocal void QBSearchResultsContextOkCallback(SvObject self_, QBSearchProgressDialog dialog)
{
    SvErrorInfo error = NULL;
    SearchResultsContext self = (SearchResultsContext) self_;
    QBSearchProgressDialogHide(dialog, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;

    QBSearchResultsContextBringBackContextMenu(self);
}

SvLocal void QBSearchResultsContextCancelCallback(SvObject self_, QBSearchProgressDialog dialog)
{
    SvErrorInfo error = NULL;
    SearchResultsContext self = (SearchResultsContext) self_;
    QBSearchProgressDialogHide(dialog, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
    //stop search
    QBSearchEngineSearchStop(self->dataSource);

    QBSearchResultsContextBringBackContextMenu(self);
}

SvLocal SvType SearchResultsContext_getType(void)
{
    static SvType type = NULL;
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_             = {
            .destroy        = SearchResultsContextDestroy
        },
        .reinitializeWindow = SearchResultsContextReinitializeWindow,
        .createWindow       = SearchResultsContextCreateWindow,
        .destroyWindow      = SearchResultsContextDestroyWindow
    };
    static const struct QBListModelListener_t methods = {
        .itemsAdded     = SearchResultsContextItemsAdded,
        .itemsRemoved   = SearchResultsContextItemsRemoved,
        .itemsChanged   = SearchResultsContextItemsChanged,
        .itemsReordered = SearchResultsContextItemsReordered
    };
    static const struct QBSearchProgressListener_ progressMethods = {
        .updated = QBSearchProgressUpdate
    };
    static const struct QBSearchProgressDialogListener_ searchProgressDialogListener = {
        .onOkPressed     = QBSearchResultsContextOkCallback,
        .onCancelPressed = QBSearchResultsContextCancelCallback,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("SearchResultsContext",
                            sizeof(struct SearchResultsContext_),
                            QBWindowContext_getType(), &type,
                            QBWindowContext_getType(), &contextVTable,
                            QBListModelListener_getInterface(), &methods,
                            QBSearchProgressListener_getInterface(), &progressMethods,
                            QBSearchProgressDialogListener_getInterface(), &searchProgressDialogListener,
                            NULL);
    }

    return type;
}

void QBSearchResultsContextExecute(QBWindowContext ctx_, QBContextMenu menuToBringBack)
{
    SearchResultsContext ctx = (SearchResultsContext) ctx_;
    SvErrorInfo error = NULL;
    if (menuToBringBack) {
        SvIterator iterator = SvArrayGetIterator(menuToBringBack->stack);
        ctx->contextStack = SvArrayCreateWithValues(&iterator, &error);
        if (error) {
            //don't hide the side menu
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
        } else {
            ctx->menuToBringBack = menuToBringBack;
            QBContextMenuHide(menuToBringBack, false);
        }
    }
    QBSearchProgressDialogShow(ctx->searchProgressDialog, &error);
    if (error) {
        goto err;
    }
    QBSearchEngineSearchStart(ctx->dataSource);
    return;
err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
}

void QBSearchResultsSetDataSource(QBWindowContext ctx_, SvObject dataSource)
{
    SearchResultsContext ctx = (SearchResultsContext) ctx_;
    if (ctx->dataSource) {
        SvInvokeInterface(QBDataModel, ctx->dataSource, removeListener, (SvObject) ctx, NULL);
    }
    SVTESTRELEASE(ctx->dataSource);
    ctx->dataSource = SVTESTRETAIN(dataSource);
    SvInvokeInterface(QBDataModel, ctx->dataSource, addListener, (SvObject) ctx, NULL);
    QBSearchEngineSetProgressListener(ctx->dataSource, (SvObject) ctx_);
}

QBWindowContext QBSearchResultsContextCreate(AppGlobals appGlobals)
{
    SearchResultsContext self;
    self = (SearchResultsContext) SvTypeAllocateInstance(SearchResultsContext_getType(), NULL);

    if(!self) return NULL;

    self->appGlobals = appGlobals;
    self->setDataSource = false;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    self->view = SVRETAIN(QBPlaylistManagerGetById(playlists, SVSTRING("AllChannels")));

    setupSearchProgressDialog(self);

    return (QBWindowContext) self;
}

void QBSearchResultsContextShowExtendedInfo(QBWindowContext ctx_)
{
    SearchResultsContext ctx = (SearchResultsContext) ctx_;
    if(!ctx_->window)
        return;

    searchResultsWindow self = ctx_->window->prv;

    int pos = XMBVerticalMenuGetPosition(self->xmbMenu);
    SvEPGEvent event = (SvEPGEvent) SvInvokeInterface(QBListModel, ctx->dataSource, getObject, pos);
    if (!SvObjectIsInstanceOf((SvObject) event, SvEPGEvent_getType()))
        return;

    if(!self->sidemenu.shown && event) {
        self->sidemenu.shown = true;
        SvArray events = SvArrayCreate(NULL);
        SvArrayAddObject(events, (SvGenericObject)event);
        QBExtendedInfoPane pane = QBExtendedInfoPaneCreateFromSettings("ReminderEditorPane.settings", ctx->appGlobals,
                self->sidemenu.ctx, SVSTRING("ExtendedInfo"), 1, events);
        SVRELEASE(events);
        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) pane);
        QBContextMenuShow(self->sidemenu.ctx);
        SVRELEASE(pane);
    }
}

SvLocal void QBSearchResultsContextShowSideMenu_(QBWindowContext ctx_, SvString id, size_t idx, bool useID)
{
    SearchResultsContext ctx = (SearchResultsContext) ctx_;
    if(!ctx_->window)
        return;

    searchResultsWindow self = ctx_->window->prv;

    int pos = XMBVerticalMenuGetPosition(self->xmbMenu);
    SvEPGEvent event = (SvEPGEvent) SvInvokeInterface(QBListModel, ctx->dataSource, getObject, pos);
    if (!SvObjectIsInstanceOf((SvObject) event, SvEPGEvent_getType()))
        return;

    SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, self->ctx->view, getByID, event->channelID);

    if(!self->sidemenu.shown && channel) {
        QBReminderEditorPane pane = QBReminderEditorPaneCreateFromSettings("ReminderEditorPane.settings", "EPGReminderEditorPane.json", self->ctx->appGlobals, self->sidemenu.ctx, SVSTRING("ReminderEditor"), channel, event);
        if(!pane)
            return;
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
}

void QBSearchResultsContextShowSideMenu(QBWindowContext ctx_, SvString id)
{
    QBSearchResultsContextShowSideMenu_(ctx_, id, 0, true);

}

void QBSearchResultsContextShowSideMenuOnIndex(QBWindowContext ctx_, size_t idx)
{
    QBSearchResultsContextShowSideMenu_(ctx_, NULL, idx, false);
}

bool QBSearchResultsContextIsSideMenuShown(QBWindowContext ctx_)
{
    if(!ctx_->window)
        return false;

    searchResultsWindow self = ctx_->window->prv;

    return self->sidemenu.shown;
}

void QBSearchResultsContextHideSideMenu(QBWindowContext ctx_)
{
    if(!ctx_->window)
        return;

    searchResultsWindow self = ctx_->window->prv;
    if(self->sidemenu.shown) {
        QBContextMenuHide(self->sidemenu.ctx, false);
    }
}

void QBSearchResultsGetActiveEvent(QBWindowContext ctx_, SvEPGEvent *event)
{
    *event = NULL;
    if(!ctx_->window)
        return;

    searchResultsWindow self = ctx_->window->prv;
    int pos = XMBVerticalMenuGetPosition(self->xmbMenu);
    if (pos < 0) return;
    *event = (SvEPGEvent) SvInvokeInterface(QBListModel, self->ctx->dataSource, getObject, pos);
}
