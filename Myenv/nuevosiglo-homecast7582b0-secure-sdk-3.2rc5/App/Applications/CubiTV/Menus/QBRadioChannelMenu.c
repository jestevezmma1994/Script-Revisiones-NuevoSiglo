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


#include "QBRadioChannelMenu.h"
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvArray.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <settings.h>
#include <QBOSK/QBOSKKey.h>
#include <QBOSK/QBOSK.h>
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
#include <Logic/GUILogic.h>
#include <Logic/InitLogic.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <SearchAgents/EPGManSearchAgent.h>

#include <main.h>

struct QBRadioChannelMenuHandler_ {
    struct SvObject_ super_;
    QBPlaylistManager playlists;
    AppGlobals appGlobals;
    QBContextMenu searchMenu;
    QBContextMenu searchByCategoryMenu;
    bool searchMenuShown;
    bool searchByCategoryMenuShown;
    QBOSKPane oskPane;
    QBBasicPane searchByCategoryPane;
    QBTreeProxy proxy;
    QBWindowContext searchResults; ///< context managing search results or lack thereof
};
typedef struct QBRadioChannelMenuHandler_ *QBRadioChannelMenuHandler;

SvLocal void QBRadioChannelMenuHandlerDestroy(void *self_)
{
    QBRadioChannelMenuHandler self = self_;
    SVTESTRELEASE(self->oskPane);
    SVTESTRELEASE(self->searchByCategoryPane);
    SVTESTRELEASE(self->searchMenu);
    SVTESTRELEASE(self->searchByCategoryMenu);
    SVRELEASE(self->proxy);
    SVTESTRELEASE(self->searchResults);
}

SvLocal void QBRadioChannelMenuHandlerOnSideMenuClose(void *ptr, QBContextMenu ctxMenu)
{
    QBRadioChannelMenuHandler self = ptr;
    self->searchMenuShown = false;
    self->searchByCategoryMenuShown = false;
}

SvLocal void QBRadioChannelMenuSearchKeyTyped(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBRadioChannelMenuHandler self = ptr;
    if (key->type == QBOSKKeyType_enter && SvStringLength(input)) {
        SVTESTRELEASE(self->searchResults);
        self->searchResults = QBSearchResultsContextCreate(self->appGlobals);

        SvArray agents = SvArrayCreate(NULL);

        SvObject view = QBPlaylistManagerGetById(self->playlists, SVSTRING("RadioChannels"));
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

SvLocal void QBRadioChannelMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBRadioChannelMenuHandler self = (QBRadioChannelMenuHandler) self_;
    if (SvObjectIsInstanceOf(node_, SvTVChannel_getType())) {
        QBTVLogicPlayChannelFromPlaylist(self->appGlobals->tvLogic, SVSTRING("RadioChannels"), (SvTVChannel) node_, SVSTRING("RadioChannelMenu"));
        QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
    } else {
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        SvGenericObject playlist = QBPlaylistManagerGetById(playlists, SVSTRING("RadioChannels"));
        if (SvInvokeInterface(SvEPGChannelView, playlist, getCount) <= 0)
            return;

        SvString id = QBActiveTreeNodeGetID((QBActiveTreeNode) node_);
        if (SvStringEqualToCString(id, "RDG")) {
            QBTVLogicResumePlaying(self->appGlobals->tvLogic, SVSTRING("XMB"));
            QBWindowContext ctx = QBGUILogicPrepareEPGContext(self->appGlobals->guiLogic, NULL, SVSTRING("RadioChannels"));
            QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
        } else if (SvStringEqualToCString(id, "RDSH") && !self->searchMenuShown) {
            if (!self->searchMenu) {
                SVTESTRELEASE(self->searchMenu);
                self->searchMenu = QBContextMenuCreateFromSettings("SideMenu.settings", self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));
                svSettingsPushComponent("OSKPane.settings");
                SVTESTRELEASE(self->oskPane);
                self->oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
                SvErrorInfo error = NULL;
                QBOSKPaneInit(self->oskPane, self->appGlobals->scheduler, self->searchMenu, 1, SVSTRING("OSKPane"), QBRadioChannelMenuSearchKeyTyped, self, &error);
                svSettingsPopComponent();
                if (!error) {
                    QBContextMenuSetCallbacks(self->searchMenu, QBRadioChannelMenuHandlerOnSideMenuClose, self);
                } else {
                    SvErrorInfoWriteLogMessage(error);
                    SvErrorInfoDestroy(error);
                    SVRELEASE(self->oskPane);
                    self->oskPane = NULL;
                    SVRELEASE(self->searchMenu);
                    self->searchMenu = NULL;
                }
            }
            if (self->searchMenu) {
                QBContextMenuPushPane(self->searchMenu, (SvGenericObject) self->oskPane);
                QBContextMenuShow(self->searchMenu);
                self->searchMenuShown = true;
            }
        } else if (SvStringEqualToCString(id, "SearchByCategory") && !self->searchByCategoryMenuShown) {
            const char* componentName = "SearchByCategoryPane.settings";
            svSettingsPushComponent(componentName);
            SVTESTRELEASE(self->searchByCategoryMenu);
            self->searchByCategoryMenu = QBContextMenuCreateFromSettings(componentName, self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));

            SVTESTRELEASE(self->searchByCategoryPane);

            SvEPGView view = (SvEPGView) QBPlaylistManagerGetById(self->playlists, SVSTRING("RadioChannels"));
            self->searchByCategoryPane = QBSearchByCategoryPaneCreateFromSettings("SearchByCategoryPane.settings", "SearchByCategoryRadioPane.json", self->appGlobals, self->searchByCategoryMenu, SVSTRING("SearchByCategoryPane"), 1, view);
            svSettingsPopComponent();
            QBContextMenuSetCallbacks(self->searchByCategoryMenu, QBRadioChannelMenuHandlerOnSideMenuClose, self);
            QBContextMenuShow(self->searchByCategoryMenu);
            QBContextMenuPushPane(self->searchByCategoryMenu, (SvGenericObject) self->searchByCategoryPane);
            self->searchByCategoryMenuShown = true;
        }
    }
}

SvLocal void QBRadioChannelMenuHandlerSelectChannel(SvGenericObject self_, SvTVChannel channel)
{
    QBRadioChannelMenuHandler self = (QBRadioChannelMenuHandler) self_;
    SvGenericObject nodePath_ = NULL;
    QBActiveTreeFindNodeByID(self->appGlobals->menuTree, SVSTRING("RDIO"), &nodePath_);
    if (!nodePath_)
        return;

    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->proxy, getIterator, NULL, 0);
    SvGenericObject node = NULL;
    int position = 0;
    while((node = QBTreeIteratorGetNextNode(&iter))) {
        if (SvObjectIsInstanceOf(node, SvTVChannel_getType()) && channel == (SvTVChannel) node)
            break;

        position++;
    }

    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvInvokeInterface(QBMenu, menuBar->prv, setSubmenuPosition, nodePath_, position, false);
}

SvLocal SvString QBRadioChannelMenuHandlerGetPlaylistID(SvGenericObject self_)
{
    return SVSTRING("RadioChannels");
}

SvLocal SvType QBRadioChannelMenuHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRadioChannelMenuHandlerDestroy
    };
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBRadioChannelMenuChoosen
    };

    static const struct QBChannelMenu_ channelMenuMethods = {
        .selectChannel = QBRadioChannelMenuHandlerSelectChannel,
        .getPlaylistID = QBRadioChannelMenuHandlerGetPlaylistID
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRadioChannelMenuHandler",
                            sizeof(struct QBRadioChannelMenuHandler_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBChannelMenu_getInterface(), &channelMenuMethods,
                            NULL);
    }

    return type;
}

void QBRadioChannelMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvGenericObject path = NULL;
    QBActiveTreeNode node = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("RDIO"), &path);
    if(!path)
        return;

    QBRadioChannelMenuHandler handler = (QBRadioChannelMenuHandler) SvTypeAllocateInstance(QBRadioChannelMenuHandler_getType(), NULL);
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlistsManager = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    handler->playlists = playlistsManager;
    handler->appGlobals = appGlobals;

    SvGenericObject radioChannels = QBPlaylistManagerGetById(playlistsManager, SVSTRING("RadioChannels"));
    handler->proxy = QBTreeProxyCreate((SvGenericObject) radioChannels, (SvGenericObject) node, NULL);
    QBActiveTreeMountSubTree(appGlobals->menuTree, (SvGenericObject) handler->proxy, path, NULL);


    QBTreePathMapInsert(pathMap, path, (SvGenericObject) handler, NULL);
    SVRELEASE(handler);

    svSettingsPushComponent("ChannelMenu.settings");
    QBChannelController channelController = QBChannelMenuControllerCreate(true, appGlobals, radioChannels);
    svSettingsPopComponent();
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) channelController, NULL);
    SVRELEASE(channelController);

    QBMainMenuAddManualChannelSelectionListener(appGlobals->main, (SvGenericObject) handler, (SvGenericObject) node);
}
