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

#include "mainmenu.h"

#include <QBInput/QBInputCodes.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/clock.h>
#include <SWL/icon.h>
#include <SWL/fade.h>
#include <QBConf.h>
#include <Menus/QBChannelMenu.h>
#include <Menus/menubar.h>
#include <Menus/QBChannelMenu.h>
#include <Menus/menuchoice.h>
#include <Menus/QBTVChannelMenu.h>
#include <Menus/QBTVCategoriesMenu.h>
#include <Menus/QBRadioChannelMenu.h>
#include <Menus/QBFavoritesMenu.h>
#include <Menus/QBPVRMenu.h>
#include <Menus/QBNPvrMenu.h>
#include <Menus/QBReminderMenu.h>
#include <Menus/QBParentalControlMenu.h>
#include <Menus/QBConfigurationMenu.h>
#include <Menus/QBNewConfigurationMenu.h>
#include <Menus/QBConaxMenuHandler.h>
#include <Menus/QBViewRightMenu.h>
#include <Menus/QBLatensTestMenu.h>
#include <Menus/QBLatensEngineeringMenu.h>
#include <Menus/QBViewRightIPTVMenu.h>
#include <Menus/QBCryptoguardMenu.h>
#include <Menus/QBConaxMenuItemController.h>
#include <Menus/QBEnvMenuHandler.h>
#include <Menus/QBBrowserMenu.h>
#include <Menus/QBGamesMenu.h>
#include <Menus/QBHotplugMenu.h>
#include <Menus/QBStaticUrlsMenu.h>
#include <Menus/QBFileBrowserMenu.h>
#include <Menus/QBWebshellMenu.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <Widgets/QBMenuCaption.h>
#include <Widgets/customerLogo.h>
#include <Hints/mainmenuHints.h>
#include <Logic/MainMenuLogic.h>
#include <QBContextSwitcher.h>
#include <Windows/newtv.h>
#include <Utils/appType.h>
#include <Utils/autoTest.h>
#include <Widgets/QBNetworkStatusIndicator.h>
#include <Widgets/QBNATStatusIndicator.h>
#include <Widgets/QBTraxisStateIndicator.h>
#include <Widgets/QBStatusIndicator.h>
#include <Widgets/QBIndicatorsPane.h>
#include <Windows/tv/QBTVChannelFrame.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <main.h>
#include <Logic/GUILogic.h>

#define MANUAL_SELECTION_HIDE_DELAY_MS 2000

static const char *menuTransparencyConfigKey = "MENU_TRANSPARENCY";

struct QBMainMenuContext_t {
    struct QBWindowContext_t super_;
    SvWeakList menuGlobalHandlers;
    QBTreePathMap skinPathMap;
    SvHashTable manualSelectionListeners;

    AppGlobals appGlobals;

    QBTreePathMap pathMap;

    SvWidget window;

    bool onScreen;

    bool flushScreenOnContextSwitch;              /**< If screen should be flushed during context switching */
};
typedef struct QBMainMenuContext_t* QBMainMenuContext;

struct QBMainMenu_t {
    SvWidget logo;
    SvWidget connectionStateIcons;
    SvWidget bg;
    SvWidget menuBar;
    SvWidget caption, clock;

    QBMainMenuContext ctx;

    QBMainMenuHints hints;

    SvWidget channelFrame;

    SvFiber manualSelectionFiber;
    SvFiberTimer manualSelectionTimer;
};
typedef struct QBMainMenu_t* QBMainMenuInternal;

SvLocal void QBMainMenuContextPropagateContextChosen(QBWindowContext ctx_);

SvLocal void QBMainMenuContextPropagateChosen(QBWindowContext ctx_, SvGenericObject path, int position);

SvInterface QBMainMenuChanges_getInterface(void)
{
    static SvInterface interface = NULL;
    if (!interface) {
        SvInterfaceCreateManaged("QBMainMenuChanges", sizeof(struct QBMainMenuChanges_t),
                                 NULL, &interface, NULL);
    }
    return interface;
}

SvLocal int QBMainMenuCalcAlpha(QBMainMenuContext ctx, const char* value)
{
    const uint8_t maxAlphaValue = ALPHA_SOLID;

    if (value == NULL) {
        return maxAlphaValue;
    }

    int transparency = atoi(value);
    return maxAlphaValue - (uint8_t)((double)maxAlphaValue*((double)transparency/100.0));
}

SvLocal void QBMainMenuClean(SvApplication app, void *menu_)
{
    QBMainMenuInternal menu = menu_;
    SVTESTRELEASE(menu->hints);
    free(menu);
}

SvLocal SvGenericObject QBMainMenuGetCurrentMenuNode(QBMainMenuInternal menu)
{
    SvGenericObject path = SvInvokeInterface(QBMenu, menu->menuBar->prv, getPosition);
    SvGenericObject parentPath = NULL;
    if (SvInvokeInterface(QBTreePath, path, getLength) > 1) {
        parentPath = SvObjectCopy(path, NULL );
        SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
    } else {
        parentPath = SVRETAIN(path);
    }

    SvGenericObject node = SvInvokeInterface(QBTreeModel, menu->ctx->appGlobals->menuTree, getNode, parentPath);
    SVRELEASE(parentPath);
    return node;
}

SvLocal void QBMainMenuHideManualChannelSelectionStep(void *menu_)
{
    QBMainMenuInternal menu = (QBMainMenuInternal) menu_;
    SvFiberEventDeactivate(menu->manualSelectionTimer);
    SvFiberDeactivate(menu->manualSelectionFiber);

    QBMenuCaptionRemoveListener(menu->caption, (SvGenericObject) menu->ctx);

    if (menu->channelFrame) {
        menu->channelFrame->tree_alpha = ALPHA_SOLID;
        svWidgetDestroy(menu->channelFrame);
        menu->channelFrame = NULL;
    }
}

SvLocal void QBMainMenuHideManualChannelSelectionWithDelay(QBMainMenuInternal menu, int delayMs)
{
    if (delayMs == 0)
        QBMainMenuHideManualChannelSelectionStep(menu);
    else
        SvFiberTimerActivateAfter(menu->manualSelectionTimer, SvTimeFromMs(delayMs));
}

SvLocal void QBMainMenuManualChannelSelected(void *prv, SvTVChannel channel)
{
    QBMainMenuInternal menu = prv;
    if (!menu->channelFrame || !channel)
        return;

    SvGenericObject currentNode = QBMainMenuGetCurrentMenuNode(menu);
    SvGenericObject handler = SvHashTableFind(menu->ctx->manualSelectionListeners, currentNode);

    if (handler) {
        SvInvokeInterface(QBChannelMenu, handler, selectChannel, channel);
    }

    QBMainMenuHideManualChannelSelectionWithDelay(menu, MANUAL_SELECTION_HIDE_DELAY_MS);
}

SvLocal void QBMainMenuShowManualChannelSelection(QBMainMenuInternal menu)
{
    SvFiberEventDeactivate(menu->manualSelectionTimer);
    svSettingsPushComponent("MainWnd.settings");
    if (!svSettingsIsWidgetDefined("ChannelFrame"))
        goto fini;

    SvGenericObject currentNode = QBMainMenuGetCurrentMenuNode(menu);
    SvGenericObject handler = SvHashTableFind(menu->ctx->manualSelectionListeners, currentNode);
    if (handler) {
        AppGlobals appGlobals = menu->ctx->appGlobals;
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        menu->channelFrame = QBTVChannelFrameNew(appGlobals->res, "ChannelFrame", appGlobals->textRenderer, playlists, appGlobals->tvLogic);
        QBTVChannelFrameSetCallback(menu->channelFrame, QBMainMenuManualChannelSelected, menu);
        menu->channelFrame->tree_alpha = ALPHA_TRANSPARENT;

        QBMenuCaptionAddListener(menu->caption, (SvGenericObject) menu->ctx);

        int xOffset = QBMenuCaptionGetLabelWidth(menu->caption) +
                      svSettingsGetInteger("Caption", "xOffset", 0) +
                      svSettingsGetInteger("ChannelFrame", "xOffset", 0);

        int yOffset = svSettingsGetInteger("ChannelFrame", "yOffset", 0);

        svWidgetAttach(menu->ctx->super_.window, menu->channelFrame, xOffset, yOffset, 1);
        SvEffect effect = svEffectFadeNew(menu->channelFrame, ALPHA_SOLID, true, 0.55, SV_EFFECT_FADE_LOGARITHM);
        svAppRegisterEffect(menu->ctx->appGlobals->res, effect);
    }

fini:
    svSettingsPopComponent();
}

SvLocal bool QBMainMenuHandleManualSelection(QBMainMenuInternal menu, SvInputEvent e)
{
    bool ret = false;

    if ((e->ch >= '0' && e->ch <= '9')) {
        if (!menu->channelFrame)
            QBMainMenuShowManualChannelSelection(menu);

        if (menu->channelFrame) {
            SvFiberEventDeactivate(menu->manualSelectionTimer);
            ret = QBTVChannelFrameInputEventHandler(menu->channelFrame, &e->fullInfo) ? true : false;
        }
    }

    return ret;
}

SvLocal bool QBMainMenuInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBMainMenuInternal menu = w->prv;

    bool ret = QBMainMenuLogicHandleInputEvent(menu->ctx->appGlobals->mainLogic, e);
    if (ret)
        return true;

    if (e->ch == QBKEY_MENU) {
        QBApplicationControllerSwitchToRoot(menu->ctx->appGlobals->controller);
        return true;
    }

    if (e->ch == QBKEY_FUNCTION) {
        QBMainMenuContextPropagateContextChosen((QBWindowContext)menu->ctx);
        return true;
    }

    return QBMainMenuHandleManualSelection(menu, e);
}

SvLocal void QBMainMenuUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e)
{
    if (e->code != SV_EVENT_QB_MENU_SELECTED)
        return;

    QBMainMenuInternal menu = w->prv;
    QBMenuEvent event = e->prv;

    SvGenericObject handler = NULL;

    SvIterator iter = SvWeakListIterator(menu->ctx->menuGlobalHandlers);
    while ((handler = (SvGenericObject) SvIteratorGetNext(&iter))) {
        if (SvObjectIsImplementationOf(handler, QBMenuChoice_getInterface())) {
            SvInvokeInterface(QBMenuChoice, handler, choosen, event->node, event->nodePath, event->position);
        }
    }

    if (menu->channelFrame) {
        QBInputEvent ev;
        ev.type = QBInputEventType_keyTyped;
        ev.u.key.code = QBKEY_ENTER;
        int ret = QBTVChannelFrameInputEventHandler(menu->channelFrame, &ev);
        QBMainMenuHideManualChannelSelectionWithDelay(menu, 0);
        if (ret)
            return;
    }

    if (event->clicked) {
        QBMainMenuContextPropagateChosen((QBWindowContext) menu->ctx, event->nodePath, event->position);
    }
}

SvLocal void QBMainMenuSetTransparency(SvGenericObject ctx_, const char *value)
{
    QBMainMenuContext ctx = (QBMainMenuContext)ctx_;
    QBMainMenuInternal menu = ctx->window->prv;
    menu->bg->tree_alpha = QBMainMenuCalcAlpha(ctx, value);
}

SvLocal void QBMainMenuConfigChanged(SvGenericObject ctx_, const char *key, const char *value)
{
    if (strncmp(key, menuTransparencyConfigKey, strlen(menuTransparencyConfigKey)) == 0) {
        QBMainMenuSetTransparency(ctx_, value);
    }
}

SvLocal void QBMainMenuCaptionChanged(SvGenericObject ctx_)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;
    QBMainMenuInternal menu = (QBMainMenuInternal) ctx->window->prv;

    svSettingsPushComponent("MainWnd.settings");

    menu->channelFrame->off_x = QBMenuCaptionGetWidth(menu->caption) +
                  svSettingsGetInteger("Caption", "xOffset", 0) +
                  svSettingsGetInteger("ChannelFrame", "xOffset", 0);

    svSettingsPopComponent();
}

SvLocal bool QBMainMenuAutoTestCheckEnter(void* target,
                                          SvObject dataSource,
                                          SvInputEvent event)
{
    if (QBIsAutoTestEvent(&event->fullInfo)) {
        SvGenericObject path = SvInvokeInterface(QBMenu, ((SvWidget)target)->prv, getPosition);
        SvGenericObject node = SvInvokeInterface(QBTreeModel, dataSource , getNode, path);
        if (SvObjectIsInstanceOf(node, QBActiveTreeNode_getType())) {
            SvValue vAttr = (SvValue) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node, SVSTRING("autoTestDisable"));
            if (vAttr && SvValueGetBoolean(vAttr)) {
                SvLogNotice("%s(): node not allowed for auto tests", __func__);
                return true;
            }
        }

    }
    return false;
}

SvLocal
void QBMainMenuContextInitIndicators(QBMainMenuInternal menu)
{
    AppGlobals appGlobals = menu->ctx->appGlobals;

    svSettingsPushComponent("ConnectionStateIcons.settings");

    if (!svSettingsIsWidgetDefined("ConnectionStateIcons"))
        goto out;

    char buf[128];

    //Indicators pane filling
    if (appGlobals->net_manager || appGlobals->natMonitor || appGlobals->traxisWebSessionManager) {
        menu->connectionStateIcons = QBIndicatorsPaneCreate(appGlobals, "ConnectionStateIcons");
        if (!menu->connectionStateIcons) {
            SvLogWarning("%s:%d : Can't create Connection state icons pane", __func__, __LINE__);
            goto out;
        }

        svSettingsWidgetAttach(menu->ctx->window, menu->connectionStateIcons, svWidgetGetName(menu->connectionStateIcons), 2);

        SvWidget indicator = NULL;
        if (appGlobals->net_manager) {
            snprintf(buf, sizeof(buf), "%s.Net.%s", svWidgetGetName(menu->connectionStateIcons), "Default");
            indicator = QBNetworkStatusIndicatorCreate(appGlobals->res, buf, appGlobals->net_manager, NULL);
            if (indicator)
                QBIndicatorsPaneAddIndicator(menu->connectionStateIcons, indicator);
        }

        if (appGlobals->natMonitor) {
            snprintf(buf, sizeof(buf), "%s.NAT.%s", svWidgetGetName(menu->connectionStateIcons), "IP");
            indicator = QBNATStatusIndicatorCreate(appGlobals->res, buf, appGlobals->natMonitor);
            if (indicator)
                QBIndicatorsPaneAddIndicator(menu->connectionStateIcons, indicator);

            snprintf(buf, sizeof(buf), "%s.NAT.%s", svWidgetGetName(menu->connectionStateIcons), "UDP");
            indicator = QBNATStatusIndicatorCreate(appGlobals->res, buf, appGlobals->natMonitor);
            if(indicator)
                QBIndicatorsPaneAddIndicator(menu->connectionStateIcons, indicator);
        }

        if (appGlobals->traxisWebSessionManager) {
            snprintf(buf, sizeof(buf), "%s.Traxis", svWidgetGetName(menu->connectionStateIcons));
            indicator = QBTraxisStateIndicatorCreate(appGlobals->res, buf, appGlobals->traxisWebSessionManager);
            if (indicator)
                QBIndicatorsPaneAddIndicator(menu->connectionStateIcons, indicator);
        }
    }

out:
    svSettingsPopComponent();
}

// QBMainMenu methods

/**
 * Used to register local handlers (see @ref MainMenuHandlers).
 * @param[in] ctx_              Main Menu Context
 * @return internal tree path map
 **/
SvLocal QBTreePathMap
QBMainMenuContextGetPathMap(SvObject ctx_)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;
    return ctx->pathMap;
}

SvLocal QBTreePathMap
QBMainMenuContextGetSkinPathMap(SvObject ctx_)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;
    return ctx->skinPathMap;
}

/**
 * Returns instance of XMBMenuBar widget. Main Menu always uses it
 * as main widget for menu.
 * @param[in] ctx               Main Menu Context
 * @return XMBMenuBar widget
 **/
SvLocal SvWidget
QBMainMenuContextGetMenuBar(SvObject ctx_)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;
    QBMainMenuInternal menu = ctx->window->prv;
    return menu->menuBar;
}

/**
 * Switch to specific node.
 *
 * Switches to existing node directly in AppGlobals::menuTree, not in mounted
 * subtrees. If actual context is Menu then just jump into new node, otherwise
 * switch context then jump into.
 * @param[in] ctx_ Main Menu Context
 * @param[in] node node id
 * @returns   @c true if @a node was found in the main menu tree, else no switch is done
 */
SvLocal bool
QBMainMenuSwitchToNode(SvObject ctx_, SvString nodeStr)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;

    SvObject path = QBActiveTreeCreateNodePath(ctx->appGlobals->menuTree, nodeStr);
    if (!path)
        return false;

    SvWidget menuBar = QBMainMenuContextGetMenuBar((SvObject) ctx->appGlobals->main);
    SvInvokeInterface(QBMenu, menuBar->prv, setPosition, path, NULL);
    SVRELEASE(path);

    if (!ctx->super_.window) {
        QBApplicationControllerPushContext(ctx->appGlobals->controller, ctx->appGlobals->main);
    } else {
        QBLocalWindowManagerRemoveAll(ctx->appGlobals->localManager, ctx->appGlobals->main);
    }
    return true;
}

/**
 * Whenever user selects an item in menu handlers registered using this
 * method will be notified of it. The handlers must implement QBMenuChoice
 * interface.
 * @param[in] ctx_              Main Menu Context
 * @param[in] handler           object that will be notified
 **/
SvLocal void
QBMainMenuContextAddGlobalHandler(SvObject ctx_, SvObject handler)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;
    SvWeakListPushFront(ctx->menuGlobalHandlers, handler, NULL);
}

/**
 * The Main Menu caches all its widgets even if they are not shown
 * on screen. In order to initialize these widgets and all other
 * internal state this method must be called before doing any actions
 * with Main Menu Context.
 * @param[in] ctx_              Main Menu Context
 **/
SvLocal void
QBMainMenuContextInit(SvObject ctx_)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;
    AppGlobals appGlobals = ctx->appGlobals;

    svSettingsPushComponent("MainWnd.settings");

    SvApplication app = appGlobals->res;
    SvWidget window = QBGUILogicCreateBackgroundWidget(appGlobals->guiLogic, "MainWindow", NULL);
    ctx->window = window;
    svWidgetSetBitmap(window, NULL);
    SvWidget bg = NULL;

    svWidgetSetName(window, "QBMainMenuContext");

    bool useQBFrame = svSettingsGetBoolean("MainWindow", "frame", false);
    if (useQBFrame) {
        bg = QBFrameCreateFromSM(app, "MainWindow.Bg");
    } else {
        bg = svIconNew(app, "MainWindow.Bg");
    }

    svWidgetAttach(window, bg, 0, 0, 0);

    QBMainMenuInternal menu = calloc(1, sizeof(struct QBMainMenu_t));

    svSettingsPushComponent("MenuBar.settings");
    menu->menuBar = QBMenuBarCreate(app, ctx->appGlobals);
    svSettingsWidgetAttach(window, menu->menuBar, svWidgetGetName(menu->menuBar), 2);
    SvInvokeInterface(QBMenu, menu->menuBar->prv, setNotificationTarget, svWidgetGetId(window));
    SvInvokeInterface(QBMenu, menu->menuBar->prv, setOnEnterCallback, menu->menuBar, &QBMainMenuAutoTestCheckEnter);
    svSettingsPopComponent();
    menu->ctx = ctx;

    ctx->flushScreenOnContextSwitch = svSettingsGetBoolean("ContextSwitcher", "flushScreen", true);

    menu->bg = bg;

    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(ctx->appGlobals->initLogic);
    if (logoWidgetName)
        menu->logo = QBCustomerLogoAttach(window, logoWidgetName, 2);

    window->prv = menu;
    window->clean = QBMainMenuClean;
    svWidgetSetInputEventHandler(window, QBMainMenuInputEventHandler);
    svWidgetSetUserEventHandler(window, QBMainMenuUserEventHandler);

    svWidgetSetFocus(menu->menuBar);

    QBParentalControlMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBTVChannelMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals, SVSTRING("TIVI"));
    QBRadioChannelMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBHotplugMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBConfigurationMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBNewConfigurationMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBConaxMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBViewRightIPTVMenuRegister(ctx->pathMap, ctx->appGlobals);
    QBViewRightMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBLatensTestMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBLatensEngineeringMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBCryptoguardMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBEnvMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBConaxMenuItemControllerRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBPVRMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBNPvrMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBReminderMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals, SVSTRING("TIVI"));
    QBBrowserMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBGamesMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBStaticUrlsMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBWebshellMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);

    QBMainMenuLogicRegisterMenus(ctx->appGlobals->mainLogic, menu->menuBar, ctx->pathMap);
    QBFavoritesMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals);
    QBTVCategoriesMenuRegister(menu->menuBar, ctx->pathMap, ctx->appGlobals, SVSTRING("TIVICategories"));

    menu->caption = QBMenuCaptionNew(app, "Caption", ctx->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, menu->caption, svWidgetGetName(menu->caption), 0);
    QBMenuCaptionRegister(menu->caption, ctx->appGlobals);

    menu->clock = svClockNew(app, "clock");
    svSettingsWidgetAttach(window, menu->clock, svWidgetGetName(menu->clock), 1);

    svSettingsPopComponent();


    QBMainMenuContextInitIndicators(menu);

    if (ctx->appGlobals->hintManager) {
        menu->hints = QBMainMenuHintsCreate(ctx->appGlobals->hintManager, ctx->pathMap, (SvGenericObject) ctx->appGlobals->menuTree);
        QBMainMenuContextAddGlobalHandler(ctx_, (SvGenericObject) menu->hints);
    }

    QBConfigAddListener((SvGenericObject) ctx, menuTransparencyConfigKey);

    QBMainMenuSetTransparency((SvGenericObject) ctx, QBConfigGet(menuTransparencyConfigKey));
}

SvLocal void
QBMainMenuContextDeinit(SvObject ctx_)
{

}

// QBContext switch listener methods

SvLocal void
QBMainMenuContextSwitchStarted(SvGenericObject ctx_, QBWindowContext from, QBWindowContext to)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;
    ctx->onScreen = false;

    if (ctx->flushScreenOnContextSwitch)
        SvTVContextDisableOverlay(ctx->appGlobals->newTV);

    SvWidget menuBar = QBMainMenuContextGetMenuBar((SvObject) ctx->appGlobals->main);
    SvInvokeInterface(QBMenu, menuBar->prv, setCurrentMenuActiveState, false);
}

SvLocal void
QBMainMenuContextSwitchEnded(SvGenericObject ctx_, QBWindowContext from, QBWindowContext to)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;

    SvWidget menuBar = QBMainMenuContextGetMenuBar((SvObject) ctx->appGlobals->main);
    SvInvokeInterface(QBMenu, menuBar->prv, setCurrentMenuActiveState, true);

    ctx->onScreen = true;
    SvTVContextSetOverlay(ctx->appGlobals->newTV, NULL);
}

// QBWindowsContextInterface methods

SvLocal void
QBMainMenuContextReinitializeWindow(QBWindowContext ctx_, SvArray itemList)
{
   QBMainMenuContext ctx = (QBMainMenuContext) ctx_;

   //inform xmb that nodes need to be updated
   if (!itemList || SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("QBParentalControl"))) {
       QBActiveTree menuTree = ctx->appGlobals->menuTree;

       // get root nodes count
       QBTreeIterator treeIter = SvInvokeInterface(QBTreeModel, menuTree, getIterator, NULL, 0);
       size_t nodesCount = QBTreeIteratorGetNodesCount(&treeIter);

       // get root node path
       QBActiveTreeNode rootNode = QBActiveTreeGetRootNode(menuTree);
       SvString rootNodeID = QBActiveTreeNodeGetID(rootNode);

       SvObject rootPath = QBActiveTreeCreateNodePath(menuTree, rootNodeID);

       // propagate root nodes change
       QBActiveTreePropagateNodesChange(menuTree, rootPath, 0, nodesCount, NULL);

       SVRELEASE(rootPath);

       // get subtrees nodes and propagate nodes change
       SvImmutableArray array = QBActiveTreeGetMountedSubTrees(menuTree);
       SvIterator iter = SvImmutableArrayIterator(array);
       SvGenericObject node;
       while ((node = SvIteratorGetNext(&iter)) != NULL) {
           if (SvObjectIsInstanceOf(node, QBTreeProxy_getType())) {
               QBTreeProxyPropagateStaticItemsChange((QBTreeProxy) node, NULL);
           } else if (SvObjectIsInstanceOf(node, QBContentTree_getType())) {
               QBContentTreePropagateStaticItemsChange((QBContentTree) node, NULL);
           }
       }
   }
   if (!itemList || SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("CustomerLogo"))) {
       QBMainMenuInternal menu = (QBMainMenuInternal) ctx->window->prv;
       SvString logoWidgetName = QBInitLogicGetLogoWidgetName(ctx->appGlobals->initLogic);
       if (logoWidgetName) {
           menu->logo = QBCustomerLogoReplace(menu->logo, ctx->window, logoWidgetName, 2);
       }
   }

   if (ctx->appGlobals->providersController)
       QBProvidersControllerServiceReinitialize(ctx->appGlobals->providersController, itemList);

   //caption and clock are adjusted automatically to the new locale
}

SvLocal void QBMainMenuChangesNotify(QBWindowContext ctx, QBMainMenuState state)
{
    SvGenericObject handler = NULL;
    SvGenericObject path = NULL;
    SvWidget menuBar = QBMainMenuContextGetMenuBar((SvObject) ctx);
    if ((path = SvInvokeInterface(QBMenu, menuBar->prv, getPosition))) {
        handler = QBTreePathMapFind(QBMainMenuContextGetPathMap((SvObject) ctx), path);
        if (handler && SvObjectIsImplementationOf(handler, QBMainMenuChanges_getInterface())) {
            SvInvokeInterface(QBMainMenuChanges, handler, changed, state, path);
        }
    }
}

SvLocal void QBMainMenuContextCreateWindow(QBWindowContext ctx_, SvApplication app)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;
    ctx->super_.window = ctx->window;
    QBMainMenuInternal menu = ctx->window->prv;
    if (menu->hints)
        QBMainMenuHintsStart(menu->hints);
    QBApplicationControllerAddListener(ctx->appGlobals->controller, (SvGenericObject) ctx);
    QBMainMenuLogicPostWidgetsCreate(ctx->appGlobals->mainLogic);
    QBMainMenuChangesNotify((QBWindowContext)ctx, QBMainMenuState_Created);

    menu->manualSelectionFiber = SvFiberCreate(ctx->appGlobals->scheduler, NULL, "QBMainMenuHideManualChannelSelection", &QBMainMenuHideManualChannelSelectionStep, menu);
    menu->manualSelectionTimer = SvFiberTimerCreate(menu->manualSelectionFiber);
}

SvLocal void QBMainMenuContextDestroyWindow(QBWindowContext ctx_)
{
    QBMainMenuContext ctx = (QBMainMenuContext)ctx_;
    QBMainMenuChangesNotify((QBWindowContext)ctx, QBMainMenuState_Destroyed);

    QBConaxMenuOnSwitch(ctx->appGlobals->conaxMenuHandler, false);
    ctx->super_.window = NULL;
    QBMainMenuInternal menu = ctx->window->prv;
    if (menu->hints)
        QBMainMenuHintsStop(menu->hints);
    QBApplicationControllerRemoveListener(ctx->appGlobals->controller, (SvGenericObject) ctx);
    QBMainMenuLogicPreWidgetsDestroy(ctx->appGlobals->mainLogic);

    if (menu->manualSelectionFiber) {
        if (menu->channelFrame)
            QBMainMenuHideManualChannelSelectionWithDelay(menu, 0);
        SvFiberEventDeactivate(menu->manualSelectionTimer);
        SvFiberDestroy(menu->manualSelectionFiber);
        menu->manualSelectionFiber = NULL;
    }

}

SvLocal void QBMainMenuContextDestroy(void *ptr)
{
    QBMainMenuContext ctx = (QBMainMenuContext)ptr;
    SVRELEASE(ctx->menuGlobalHandlers);
    SVRELEASE(ctx->manualSelectionListeners);
    SVRELEASE(ctx->pathMap);
    SVRELEASE(ctx->skinPathMap);
    svWidgetDestroy(ctx->window);
}

SvType QBMainMenuContext_getType(void)
{
    static SvType type = NULL;

    static const struct QBWindowContextVTable_ contextVTable = {
        .super_             = {
            .destroy        = QBMainMenuContextDestroy
        },
        .reinitializeWindow = QBMainMenuContextReinitializeWindow,
        .createWindow       = QBMainMenuContextCreateWindow,
        .destroyWindow      = QBMainMenuContextDestroyWindow
    };

    static const struct QBContextSwitcherListener_t switchMethods = {
        .started = QBMainMenuContextSwitchStarted,
        .ended   = QBMainMenuContextSwitchEnded
    };

    static const struct QBConfigListener_t configMethods = {
        .changed = QBMainMenuConfigChanged
    };

    static const struct QBMenuCaptionListener_ menuCaptionListenerMethods = {
        .changed = QBMainMenuCaptionChanged
    };

    static const struct QBMainMenu_ mainMenu = {
        .init              = QBMainMenuContextInit,
        .deinit            = QBMainMenuContextDeinit,
        .addGlobalHandler  = QBMainMenuContextAddGlobalHandler,
        .getMenu           = QBMainMenuContextGetMenuBar,
        .switchToNode      = QBMainMenuSwitchToNode,
        .getPathMap        = QBMainMenuContextGetPathMap,
        .getSkinPathMap    = QBMainMenuContextGetSkinPathMap
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMainMenuContext",
                            sizeof(struct QBMainMenuContext_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            QBContextSwitcherListener_getInterface(), &switchMethods,
                            QBConfigListener_getInterface(), &configMethods,
                            QBMenuCaptionListener_getInterface(), &menuCaptionListenerMethods,
                            QBMainMenu_getInterface(), &mainMenu,
                            NULL);
    }

    return type;
}

QBWindowContext QBMainMenuContextCreate(AppGlobals appGlobals)
{
    QBMainMenuContext ctx = (QBMainMenuContext) SvTypeAllocateInstance(QBMainMenuContext_getType(), NULL);
    ctx->appGlobals = appGlobals;
    ctx->menuGlobalHandlers = SvWeakListCreate(NULL);
    ctx->manualSelectionListeners = SvHashTableCreate(11, NULL);
    ctx->pathMap = QBTreePathMapCreate(NULL);
    ctx->skinPathMap = QBTreePathMapCreate(NULL);

    return (QBWindowContext) ctx;
}

/**
 * Reguests an artificial sidemenu request on current menu element event
 * to be propagated.
 * @param[in] ctx_              Main Menu Context
 **/
SvLocal void
QBMainMenuContextPropagateContextChosen(QBWindowContext ctx_)
{
    SvGenericObject handler = NULL;
    SvGenericObject path = NULL;

    SvWidget menuBar = QBMainMenuContextGetMenuBar((SvObject) ctx_);
    if ((path = SvInvokeInterface(QBMenu, menuBar->prv, getPosition))) {
        handler = QBTreePathMapFind(QBMainMenuContextGetPathMap((SvObject) ctx_), path);
        if (handler && SvObjectIsImplementationOf(handler, QBMenuContextChoice_getInterface())) {
            SvInvokeInterface(QBMenuContextChoice, handler, contextChoosen, path);
        }
    }
}

/**
 * Requests an artificial element selection event to be propagated.
 * @param[in] ctx_              Main Menu Context
 * @param[in] path              path of selected node
 * @param[in] position          position of selected node in its level
 **/
SvLocal void
QBMainMenuContextPropagateChosen(QBWindowContext ctx_, SvGenericObject path, int position)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;
    SvGenericObject node = SvInvokeInterface(QBTreeModel, ctx->appGlobals->menuTree, getNode, path);
    if (!path || !node)
        return;

    if (!QBMainMenuLogicChosen(ctx->appGlobals->mainLogic, node, path)) {
        SvGenericObject handler = QBTreePathMapFind(QBMainMenuContextGetPathMap((SvObject) ctx_), path);
        if (handler && SvObjectIsImplementationOf(handler, QBMenuChoice_getInterface())) {
            SvInvokeInterface(QBMenuChoice, handler, choosen, node, path, position);
        }
    }
}

void QBMainMenuAddManualChannelSelectionListener(QBWindowContext ctx_, SvGenericObject listener, SvGenericObject node)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;
    SvHashTableInsert(ctx->manualSelectionListeners, node, listener);
}

SvWidget
QBMainMenuContextInitAdditionalIndicator(SvObject ctx_, const char *widgetName, const char *indicatorName)
{
    QBMainMenuContext ctx = (QBMainMenuContext) ctx_;
    QBMainMenuInternal menu = ctx->window->prv;
    AppGlobals appGlobals = menu->ctx->appGlobals;

    SvWidget indicator = NULL;
    if (!svSettingsIsWidgetDefined(widgetName))
        goto out;

    if (!menu->connectionStateIcons) {
        menu->connectionStateIcons = QBIndicatorsPaneCreate(appGlobals, widgetName);
        if (!menu->connectionStateIcons) {
            SvLogWarning("%s:%d : Can't create %s icon pane", __func__, __LINE__, widgetName);
            goto out;
        }

        svSettingsWidgetAttach(menu->ctx->window, menu->connectionStateIcons, svWidgetGetName(menu->connectionStateIcons), 1);
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "%s.%s", svWidgetGetName(menu->connectionStateIcons), indicatorName);
    indicator = QBStatusIndicatorCreate(appGlobals->res, buf);
    if (indicator) {
        QBIndicatorsPaneAddIndicator(menu->connectionStateIcons, indicator);
    }

out:
    return indicator;
}

