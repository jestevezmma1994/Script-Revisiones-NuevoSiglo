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

#include <ctype.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvEPGDataLayer/Views/SvEPGNumericFilter.h>
#include <SvEPGDataLayer/Views/SvEPGSortingView.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Services/core/playlistManager.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBAppKit/QBObserver.h>
#include <QBAppKit/QBLocalConfig.h>
#include <Windows/newtv.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBFavoritesEditorPane.h>
#include <SvJSON/SvJSONParse.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include "QBFavoritesMenuController.h"
#include "menuchoice.h"
#include "QBFavoritesMenu.h"
#include <settings.h>
#include <Windows/newtvguide.h>
#include <Utils/appType.h>
#include <Logic/GUILogic.h>
#include <TranslationMerger.h>
#include <main.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>
#include <libintl.h>

#if SV_LOG_LEVEL > 0
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 2, "QBFavoritesMenuLogLevel", "")
    #define log_debug(fmt, ...)     do { if (env_log_level() >= 4) SvLogNotice(COLBEG() "%s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_info(fmt, ...)      do { if (env_log_level() >= 3) SvLogNotice(COLBEG() "%s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_warning(fmt, ...)   do { if (env_log_level() >= 2) SvLogWarning(COLBEG() "%s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...)     do { if (env_log_level() >= 1) SvLogError(COLBEG()  "%s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#else
    #define log_debug(fmt, ...)
    #define log_info(fmt, ...)
    #define log_warning(fmt, ...)
    #define log_error(fmt, ...)
#endif

struct QBFavoritesMenuHandler_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBActiveTree menuTree;
    struct QBFavoritesMenuHandlerSideMenu_t {
        QBContextMenu ctx;
    } sidemenu;
    QBPlaylistManager playlistManager;  /**< Handle of playlists manager */
    bool show_bouquets;                 /**< If @c true, bouquets are shown in Favorites menu as playlists */
    QBLocalConfig localConfig;          /**< Local config handler */
};
typedef struct QBFavoritesMenuHandler_t *QBFavoritesMenuHandler;

SvLocal void
QBFavoritesMenuHandlerDestroy(void *self_)
{
    QBFavoritesMenuHandler self = self_;
    SVRELEASE(self->menuTree);
    SVRELEASE(self->sidemenu.ctx);
    SVTESTRELEASE(self->localConfig);
    SVTESTRELEASE(self->playlistManager);
}

SvLocal void
QBFavoritesMenuChoosen(SvObject self_, SvObject node, SvObject nodePath, int position)
{
    QBFavoritesMenuHandler self = (QBFavoritesMenuHandler) self_;

    if (SvObjectIsInstanceOf(node, QBActiveTreeNode_getType())) {
        SvString ID = QBActiveTreeNodeGetID((QBActiveTreeNode) node);
        if (ID && !strncmp(SvStringCString(ID), "EPG-", 4)) {
            // switch to EPG for selected channels list
            SvString listID = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node, SVSTRING("listID"));
            QBServiceRegistry registry = QBServiceRegistryGetInstance();
            QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
            SvObject channelsList = QBPlaylistManagerGetById(playlists, listID);
            if (channelsList) {
                int count = SvInvokeInterface(SvEPGChannelView, channelsList, getCount);
                if (count) {
                    QBWindowContext ctx = QBGUILogicPrepareEPGContext(self->appGlobals->guiLogic, NULL, listID);
                    QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
                }
            }
        }
    } else if (SvObjectIsInstanceOf(node, SvTVChannel_getType())) {
        // find out which list have been choosen
        SvObject path = SvObjectCopy(nodePath, NULL);
        SvInvokeInterface(QBTreePath, path, truncate, 2);
        QBActiveTreeNode listMenuNode = QBActiveTreeFindNodeByPath(self->menuTree, path);
        if (listMenuNode) {
            SvString listID = QBActiveTreeNodeGetID(listMenuNode);
            SvTVChannel channel = (SvTVChannel) node;
            QBTVLogicPlayChannelFromPlaylist(self->appGlobals->tvLogic, listID, channel, SVSTRING("FavoritesMenu"));
            QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
        }
        SVRELEASE(path);
    }
}

SvLocal void
QBFavoritesMenuContextChoosen(SvObject self_, SvObject nodePath)
{
    QBFavoritesMenuHandler self = (QBFavoritesMenuHandler) self_;
    QBFavoritesEditorPane pane = NULL;
    SvObject node_;
    SvString ID = NULL;
    node_ = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath);
    if (!node_)
        return;

    bool takeParent = false;
    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        QBActiveTreeNode node = (QBActiveTreeNode) node_;
        ID = QBActiveTreeNodeGetID(node);
        if (!strncmp(SvStringCString(ID), "EPG-", 4)) {
            takeParent = true;
        }
    } else if (SvObjectIsInstanceOf(node_, SvTVChannel_getType())) {
        takeParent = true;
    }

    if (takeParent) {
        SvObject parentPath = SvObjectCopy(nodePath, NULL);
        SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
        node_ = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, parentPath);
        SVRELEASE(parentPath);
        if (!node_)
            return;
    }

    ID = QBActiveTreeNodeGetID((QBActiveTreeNode) node_);

    pane = QBFavoritesEditorPaneCreateFromSettings("FavoritesEditorPane.settings", "QBFavoritesEditorPane.json", self->appGlobals, self->sidemenu.ctx, SVSTRING("FavoritesEditorPane"), ID, NULL);

    if (pane) {
        QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) pane);
        QBContextMenuShow(self->sidemenu.ctx);
        SVRELEASE(pane);
    }
}

SvLocal size_t
QBFavoritesMenuCountStaticCategories(QBPlaylistManager playlistsManager)
{
    size_t staticCategories = 0;
    SvObject playlist;
    SvArray playlists = QBPlaylistManagerGetPlaylists(playlistsManager, NULL);
    SvIterator iter = SvArrayIterator(playlists);
    while ((playlist = SvIteratorGetNext(&iter))) {
        SvValue rootNodeId = (SvValue) QBPlaylistManagerGetPlaylistMetaData(playlistsManager, playlist, (SvObject) SVSTRING("root_node_id"));
        SvValue isInternal = (SvValue) QBPlaylistManagerGetPlaylistMetaData(playlistsManager, playlist, (SvObject) SVSTRING("isInternal"));
        if (!rootNodeId || !isInternal || !SvValueGetBoolean(isInternal)) {
            SvString type = QBPlaylistManagerGetPlaylistType(playlistsManager, playlist);
            if (!SvStringEqualToCString(type, "CATEGORY") && !SvStringEqualToCString(type, "FAVORITES"))
                staticCategories++;
        }
    }
    return staticCategories;
}

void
QBFavoritesMenuRenameChannelsList(QBActiveTree menuTree, SvString channelListID, SvString channelListName)
{
    log_debug("Rename channel list %s. New name: %s", SvStringCString(channelListID), SvStringCString(channelListName));
    QBActiveTreeNode menuNode = QBActiveTreeFindNode(menuTree, SVSTRING("FAVS"));
    QBActiveTreeNode node = QBActiveTreeNodeFindChildByID(menuNode, channelListID);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvObject) channelListName);
    QBActiveTreePropagateNodeChange(menuTree, node, NULL);
}

SvLocal void
QBFavoritesMenuUpdated(SvObject self_, SvObject observedObject, SvObject arg_)
{
    QBFavoritesMenuHandler self = (QBFavoritesMenuHandler) self_;
    QBPlaylistChangeParams arg = (QBPlaylistChangeParams) arg_;
    SvWidget menuBar = NULL;
    if (!self->show_bouquets) {
        SvString type = QBPlaylistManagerGetPlaylistType(self->playlistManager,
                                                         arg->channelList);
        if (SvStringEqualToCString(type, "CATEGORY")) {
            return;
        }
    }
    if (self->appGlobals->main)
        menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);

    switch (arg->type) {
        case QBPlaylist_CategoryAdded:
            if (menuBar) {
                QBFavoritesMenuAddChannelsList((SvWidget) menuBar, self->appGlobals, arg->channelList, SVSTRING("FAVS"), arg->position, true);
            }
            break;
        case QBPlaylist_CategoryRemoved:
            if (menuBar) {
                QBFavoritesMenuRemoveChannelsList(self->appGlobals->menuTree, menuBar, arg->channelListID);
            }
            break;
        case QBPlaylist_CategoryRenamed:
            QBFavoritesMenuRenameChannelsList(self->appGlobals->menuTree, arg->channelListID, arg->channelListName);
            break;
        default:
            log_error("incorrect argument type: %d", arg->type);
            break;
    }
}

SvLocal SvType
QBFavoritesMenuHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBFavoritesMenuHandlerDestroy
    };
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBFavoritesMenuChoosen
    };
    static const struct QBMenuContextChoice_t contextMethods = {
        .contextChoosen = QBFavoritesMenuContextChoosen
    };
    static const struct QBObserver_ observerMethods = {
        .observedObjectUpdated = QBFavoritesMenuUpdated
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBFavoritesMenuHandler",
                            sizeof(struct QBFavoritesMenuHandler_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBObserver_getInterface(), &observerMethods,
                            QBMenuContextChoice_getInterface(), &contextMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBFavoritesMenuHandlerOnSideMenuClose(void *self_, QBContextMenu ctx)
{
    QBContextMenuHide(ctx, false);
}


void
QBFavoritesMenuRemoveChannelsList(QBActiveTree menuTree, SvWidget menuBar,
                                  SvString listId)
{
    log_debug("Remove channel list %s.", SvStringCString(listId));
    SvObject path = NULL;
    QBActiveTreeNode listMenuNode = QBActiveTreeFindNodeByID(menuTree, listId, &path);
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, NULL, NULL);
    QBActiveTreeRemoveSubTree(menuTree, listMenuNode, NULL);
}

void
QBFavoritesMenuAddChannelsList(SvWidget menuBar,
                               AppGlobals appGlobals,
                               SvObject channelsList,
                               SvString nodeID,
                               int position,
                               bool extended)
{
    QBActiveTreeNode listMenuNode, menuNode, node;
    QBFavoritesController favoritesController;

    if (!nodeID) {
        nodeID = SVSTRING("FAVS");
    }
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvString listName = QBPlaylistManagerGetPlaylistName(playlists, channelsList);
    SvString listId = QBPlaylistManagerGetPlaylistId(playlists, channelsList);
    if (!listId) {
        log_debug("listId is NULL");
        return;
    }
    log_debug("Add new channel list. listName:%s, listID:%s position:%d", SvStringCString(listName), SvStringCString(listId), position);

    menuNode = QBActiveTreeFindNode(appGlobals->menuTree, nodeID);
    listMenuNode = QBActiveTreeNodeCreate(listId, NULL, NULL);
    QBActiveTreeNodeSetAttribute(listMenuNode, SVSTRING("caption"), (SvObject) listName);
    QBActiveTreeNodeSetAttribute(listMenuNode, SVSTRING("listID"), (SvObject) listId);

    if (extended) {
        SvString ID = SvStringCreateWithFormat("EPG-%s", SvStringCString(listId));
        node = QBActiveTreeNodeCreate(ID, NULL, NULL);
        SVRELEASE(ID);

        SvString guide = SvStringCreate(gettext_noop("Guide"), NULL);
        QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvObject) guide);
        SVRELEASE(guide);

        QBActiveTreeNodeSetAttribute(node, SVSTRING("listID"), (SvObject) listId);
        QBActiveTreeNodeAddChildNode(listMenuNode, node, NULL);
        SVRELEASE(node);
    }

    if (position >= 0) {
        size_t staticCategories = QBFavoritesMenuCountStaticCategories(playlists);
        size_t childsCount = QBActiveTreeNodeGetChildNodesCount(menuNode);

        if (childsCount < position + staticCategories) {
            QBActiveTreeAddSubTree(appGlobals->menuTree, menuNode, listMenuNode, NULL);
        } else {
            QBActiveTreeInsertSubTree(appGlobals->menuTree, menuNode, listMenuNode, position + staticCategories - 1, NULL);
        }
    } else {
        QBActiveTreeAddSubTree(appGlobals->menuTree, menuNode, listMenuNode, NULL);
    }

    SvObject path = QBActiveTreeCreateNodePath(appGlobals->menuTree, listId);
    if (!path) {
        log_error("can't create node path for %s", SvStringCString(listId));
        return;
    }
    QBTreeProxy proxy = QBTreeProxyCreate(channelsList, (SvObject) listMenuNode, NULL);
    if (proxy) {
        QBActiveTreeMountSubTree(appGlobals->menuTree, (SvObject) proxy, path, NULL);
        SVRELEASE(proxy);
    }

    SVRELEASE(listMenuNode);

    svSettingsPushComponent("ChannelMenu.settings");
    favoritesController = QBFavoritesMenuControllerCreate(appGlobals, channelsList);
    svSettingsPopComponent();
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) favoritesController, NULL);
    SVRELEASE(favoritesController);
    SVRELEASE(path);
}

static bool isSvValueAndIsValueKind(SvObject obj, SvValueType type)
{
    return (SvObjectIsInstanceOf(obj, SvValue_getType()) &&
            (SvValueGetType((SvValue) obj) == type));
}

void QBFavoritesMenuRegister(SvWidget menuBar,
                             QBTreePathMap pathMap,
                             AppGlobals appGlobals)
{
    QBFavoritesMenuHandler handler;
    QBFavoritesController favoritesController;

    SvObject path = QBActiveTreeCreateNodePath(appGlobals->menuTree, SVSTRING("FAVS"));
    if (!path)
        return;

    handler = (QBFavoritesMenuHandler) SvTypeAllocateInstance(QBFavoritesMenuHandler_getType(), NULL);
    handler->show_bouquets = false;

    handler->localConfig =
        QBLocalConfigCreate("QBFavoritesMenuConf.json", QBLocalConfigMode_readOnly, NULL);
    if (!handler->localConfig) {
        log_error("QBLocalConfigCreate failed");
    } else {
        QBLocalConfigResult retVal =
            QBLocalConfigGetBool(handler->localConfig, "SHOW_BOUQUETS", &handler->show_bouquets, NULL);
        if (retVal == QBLocalConfigResult_error) {
            log_error("QBLocalConfigGetBool failed");
        }
    }

    handler->appGlobals = appGlobals;
    handler->menuTree = SVRETAIN(appGlobals->menuTree);
    handler->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(handler->sidemenu.ctx, QBFavoritesMenuHandlerOnSideMenuClose, handler);

    QBTreePathMapInsert(pathMap, path, (SvObject) handler, NULL);

    svSettingsPushComponent("ChannelMenu.settings");
    favoritesController = QBFavoritesMenuControllerCreate(appGlobals, NULL);
    svSettingsPopComponent();
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) favoritesController, NULL);
    SVRELEASE(favoritesController);

    SvIterator iter;
    SvArray playlists;
    SvObject playlist;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    handler->playlistManager =
        SVTESTRETAIN((QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager")));

    // Use NULL as a type to get Favorites playlists and "CATEGORY" for playlists made from bouquets
    playlists = QBPlaylistManagerGetPlaylists(handler->playlistManager, NULL);

    if (handler->show_bouquets) {
        SvArray categoriesPlaylists = QBPlaylistManagerGetPlaylists(handler->playlistManager, SVSTRING("CATEGORY"));
        SvIterator catIter = SvArrayGetIterator(categoriesPlaylists);
        SvArrayAddObjects(playlists, &catIter);
    }

    QBPlaylistManagerSortPlaylists(handler->playlistManager, playlists);
    iter = SvArrayIterator(playlists);
    while ((playlist = SvIteratorGetNext(&iter))) {
        SvValue isPlaylistInternal = (SvValue) QBPlaylistManagerGetPlaylistMetaData(handler->playlistManager, playlist, (SvObject) SVSTRING("isInternal"));
        if (isPlaylistInternal && isSvValueAndIsValueKind((SvObject) isPlaylistInternal, SvValueType_boolean) && SvValueGetBoolean(isPlaylistInternal)) {
            continue;
        }

        SvValue epgVal = (SvValue) QBPlaylistManagerGetPlaylistMetaData(handler->playlistManager, playlist, (SvObject) SVSTRING("EPG"));
        bool epg_enabled = true;
        if (epgVal && isSvValueAndIsValueKind((SvObject) epgVal, SvValueType_boolean))
            epg_enabled = SvValueGetBoolean(epgVal);
        SvString nodeRootId = NULL;
        SvValue nodeRootIdVal = (SvValue) QBPlaylistManagerGetPlaylistMetaData(handler->playlistManager, playlist, (SvObject) SVSTRING("root_node_id"));
        if (isSvValueAndIsValueKind((SvObject) nodeRootIdVal, SvValueType_string)) {
            nodeRootId = SvValueGetString(nodeRootIdVal);
        }
        QBFavoritesMenuAddChannelsList(menuBar, appGlobals, playlist, nodeRootId, -1, epg_enabled);
    }

    QBObservable promotedCategories = (QBObservable) QBServiceRegistryGetService(registry, SVSTRING("QBPromotedCategoriesService"));
    if (promotedCategories) {
        QBObservableAddObserver(promotedCategories, (SvObject) handler, NULL);
    }

    QBObservable bouquetMonitor = (QBObservable) QBServiceRegistryGetService(registry, SVSTRING("QBDVBBouquets"));
    if (bouquetMonitor) {
        QBObservableAddObserver(bouquetMonitor, (SvObject) handler, NULL);
    }

    SVRELEASE(handler);
    SVRELEASE(path);
}
