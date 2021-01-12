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

#include "QBTVChannelMenu.h"
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvArray.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <settings.h>
#include <QBOSK/QBOSKKey.h>
#include <Windows/newtv.h>
#include "QBChannelMenu.h"
#include "menuchoice.h"
#include <Windows/searchResults.h>
#include <Windows/mainmenu.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBOSKPane.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBSearchByCategoryPane.h>
#include <QBSearch/QBSearchAgent.h>
#include <QBSearch/QBSearchEngine.h>
#include <Services/TVChannelSearchAgent.h>
#include <Logic/GUILogic.h>
#include <Logic/InitLogic.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SearchAgents/EPGManSearchAgent.h>
#include <main.h>
#include <Windows/mainmenu.h>
#include <Utils/boldUtils.h>


struct QBTVChannelMenuHandler_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBTreeProxy proxy;
    QBContextMenu searchMenu;
    QBContextMenu searchByCategoryMenu;
    bool searchMenuShown;
    bool searchByCategoryMenuShown;
    QBOSKPane oskPane;
    QBBasicPane searchByCategoryPane;
    QBWindowContext searchResults; ///< context managing search results or lack thereof
};
typedef struct QBTVChannelMenuHandler_ *QBTVChannelMenuHandler;

SvLocal void QBTVChannelMenuHandlerDestroy(void *self_)
{
    QBTVChannelMenuHandler self = self_;
    SVTESTRELEASE(self->oskPane);
    SVTESTRELEASE(self->searchByCategoryPane);
    SVTESTRELEASE(self->searchMenu);
    SVTESTRELEASE(self->searchByCategoryMenu);
    SVTESTRELEASE(self->proxy);
    SVTESTRELEASE(self->searchResults);
}

SvLocal void QBTVChannelMenuHandlerOnSideMenuClose(void *ptr, QBContextMenu ctxMenu)
{
    QBTVChannelMenuHandler self = ptr;
    self->searchMenuShown = false;
    self->searchByCategoryMenuShown = false;
}

SvLocal void QBTVChannelMenuSearchKeyTyped(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBTVChannelMenuHandler self = ptr;
    if (key->type == QBOSKKeyType_enter && SvStringLength(input)) {
        SVTESTRELEASE(self->searchResults);
        self->searchResults = QBSearchResultsContextCreate(self->appGlobals);

        SvArray agents = SvArrayCreate(NULL);

        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        SvObject view = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
        SvObject agent = QBInitLogicCreateSearchAgent(self->appGlobals->initLogic,
                                                      EPGManSearchAgentMode_normal,
                                                      0, view);


        SvArrayAddObject(agents, agent);
        SVRELEASE(agent);

        SvGenericObject search = QBSearchEngineCreate(agents, input, NULL);
        SVRELEASE(agents);

        if (SvArrayCount(QBSearchEngineGetKeywords(search))) {
            QBSearchResultsSetDataSource(self->searchResults, search);
            QBSearchResultsContextExecute(self->searchResults, self->searchMenu);
        }

        SVRELEASE(search);
    }
}

SvLocal void QBTVChannelMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBTVChannelMenuHandler self = (QBTVChannelMenuHandler) self_;
    if (SvObjectIsInstanceOf(node_, SvTVChannel_getType())) {
        // BEGIN AMERELES #2206 Canal lineal que lleve a VOD (EPG a canal que lleva a VoD no funciona)
        QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
        QBTVLogicPlayChannelFromPlaylist(self->appGlobals->tvLogic, SVSTRING("TVChannels"), (SvTVChannel) node_, SVSTRING("ChannelMenu"));
        // END AMERELES #2206 Canal lineal que lleve a VOD (EPG a canal que lleva a VoD no funciona)
        //QBTVLogicPlayChannelFromPlaylist(self->appGlobals->tvLogic, SVSTRING("TVChannels"), (SvTVChannel) node_, SVSTRING("ChannelMenu"));
        //QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
    } else {
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        SvGenericObject playlist = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
        if (SvInvokeInterface(SvEPGChannelView, playlist, getCount) <= 0)
            return;

        SvString id = QBActiveTreeNodeGetID((QBActiveTreeNode) node_);
        if (SvStringEqualToCString(id, "TVG")) {
            QBTVLogicResumePlaying(self->appGlobals->tvLogic, SVSTRING("XMB"));
            QBWindowContext ctx = QBGUILogicPrepareEPGContext(self->appGlobals->guiLogic, NULL, SVSTRING("TVChannels"));
            QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
        } else if (SvStringEqualToCString(id, "TVSH") && !self->searchMenuShown) {
            
            // BEGIN AMERELES Buscador Global
            QBCarouselMenuItemService menu = (QBCarouselMenuItemService) 
                QBProvidersControllerServiceGetServiceByID(self->appGlobals->providersController, SVSTRING("VOD"), getBoldSearchServiceID());
            if (!menu || !QBCarouselMenuPushContext(menu, QBCarouselMenuGetOwnerTree(menu)))
            {
                SvInvokeInterface(QBMainMenu, self->appGlobals->main, switchToNode, SVSTRING("VOD"));
            }
            else
            {
                self->appGlobals->vodGridBrowserInitAsSearch = true;
            }
            return;
            // END AMERELES Buscador Global
            
            SVTESTRELEASE(self->oskPane);
            SVTESTRELEASE(self->searchMenu);
            self->searchMenu = QBContextMenuCreateFromSettings("SideMenu.settings", self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));
            svSettingsPushComponent("OSKPane.settings");
            self->oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
            SvErrorInfo error = NULL;
            QBOSKPaneInit(self->oskPane, self->appGlobals->scheduler, self->searchMenu, 1, SVSTRING("OSKPane"), QBTVChannelMenuSearchKeyTyped, self, &error);
            svSettingsPopComponent();
            if (!error) {
                QBContextMenuSetCallbacks(self->searchMenu, QBTVChannelMenuHandlerOnSideMenuClose, self);
                QBContextMenuPushPane(self->searchMenu, (SvGenericObject) self->oskPane);
                QBContextMenuShow(self->searchMenu);
                self->searchMenuShown = true;
            } else {
                SvErrorInfoWriteLogMessage(error);
                SvErrorInfoDestroy(error);
            }
        } else if (SvStringEqualToCString(id, "SearchByCategory") && !self->searchByCategoryMenuShown) {
            SVTESTRELEASE(self->searchByCategoryPane);
            SVTESTRELEASE(self->searchByCategoryMenu);

            const char *componentName = "SearchByCategoryPane.settings";
            svSettingsPushComponent(componentName);
            self->searchByCategoryMenu = QBContextMenuCreateFromSettings(componentName, self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));

            SvEPGView view = (SvEPGView) QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
            self->searchByCategoryPane = QBSearchByCategoryPaneCreateFromSettings(componentName, "SearchByCategoryPane.json", self->appGlobals, self->searchByCategoryMenu, SVSTRING("SearchByCategoryPane"), 1, view);
            svSettingsPopComponent();
            QBContextMenuSetCallbacks(self->searchByCategoryMenu, QBTVChannelMenuHandlerOnSideMenuClose, self);
            QBContextMenuShow(self->searchByCategoryMenu);
            QBContextMenuPushPane(self->searchByCategoryMenu, (SvGenericObject) self->searchByCategoryPane);
            self->searchByCategoryMenuShown = true;
        }
    }
}

SvLocal void QBTVChannelMenuHandlerSelectChannel(SvGenericObject self_, SvTVChannel channel)
{
    QBTVChannelMenuHandler self = (QBTVChannelMenuHandler) self_;

    SvObject nodePath_ = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, SVSTRING("TIVI"));
    if (!nodePath_)
        return;

    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->proxy, getIterator, NULL, 0);
    SvObject node = NULL;
    int position = 0;
    while ((node = QBTreeIteratorGetNextNode(&iter))) {
        if (SvObjectIsInstanceOf(node, SvTVChannel_getType()) && channel == (SvTVChannel) node)
            break;
        position++;
    }

    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvInvokeInterface(QBMenu, menuBar->prv, setSubmenuPosition, nodePath_, position, false);

    SVRELEASE(nodePath_);
}

SvLocal SvString QBTVChannelMenuHandlerGetPlaylistID(SvGenericObject self_)
{
    return SVSTRING("TVChannels");
}

SvLocal SvType QBTVChannelMenuHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTVChannelMenuHandlerDestroy
    };
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBTVChannelMenuChoosen
    };

    static const struct QBChannelMenu_ channelMenuMethods = {
        .selectChannel = QBTVChannelMenuHandlerSelectChannel,
        .getPlaylistID = QBTVChannelMenuHandlerGetPlaylistID
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTVChannelMenuHandler",
                            sizeof(struct QBTVChannelMenuHandler_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBChannelMenu_getInterface(), &channelMenuMethods,
                            NULL);
    }

    return type;
}

void QBTVChannelMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals, SvString rootID)
{
    SvObject path = NULL;
    QBActiveTreeNode node = QBActiveTreeFindNodeByID(appGlobals->menuTree, rootID, &path);
    if (!path)
        return;

    QBTVChannelMenuHandler handler = (QBTVChannelMenuHandler) SvTypeAllocateInstance(QBTVChannelMenuHandler_getType(), NULL);
    handler->appGlobals = appGlobals;

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject tvChannels = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
    handler->proxy = QBTreeProxyCreate((SvGenericObject) tvChannels, (SvGenericObject) node, NULL);
    QBActiveTreeMountSubTree(appGlobals->menuTree, (SvGenericObject) handler->proxy, path, NULL);

    QBTreePathMapInsert(pathMap, path, (SvGenericObject) handler, NULL);
    SVRELEASE(handler);

    svSettingsPushComponent("ChannelMenu.settings");
    QBChannelController channelController = QBChannelMenuControllerCreate(true, appGlobals, tvChannels);
    svSettingsPopComponent();
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) channelController, NULL);
    SVRELEASE(channelController);

    QBMainMenuAddManualChannelSelectionListener(appGlobals->main, (SvGenericObject) handler, (SvGenericObject) node);
}

void QBTVChannelMenuRegisterWithoutChannelList(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals, SvString rootID)
{
    SvObject path = QBActiveTreeCreateNodePath(appGlobals->menuTree, rootID);
    if (!path)
        return;

    QBTVChannelMenuHandler handler = (QBTVChannelMenuHandler) SvTypeAllocateInstance(QBTVChannelMenuHandler_getType(), NULL);
    handler->appGlobals = appGlobals;
    QBTreePathMapInsert(pathMap, path, (SvObject) handler, NULL);
    SVRELEASE(handler);

    SVRELEASE(path);
}
