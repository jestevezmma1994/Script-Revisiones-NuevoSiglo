/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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

#include <Menus/QBTVCategoriesMenu.h>

#include <Menus/QBFavoritesMenu.h>
#include <Menus/QBChannelMenu.h>
#include <Menus/menuchoice.h>
#include <Menus/QBChannelMenuInternal.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <XMB2/XMBMenuBar.h>
#include <Logic/GUILogic.h>
#include <Logic/InitLogic.h>
#include <Windows/mainmenu.h>
#include <SvEPGDataLayer/Views/SvEPGHashTableFilter.h>
#include <SvEPGDataLayer/Views/SvEPGSortingView.h>
#include <SvEPGDataLayer/SvEPGChannelListListener.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <ContextMenus/QBContextMenu.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBObserver.h>
#include <main.h>
#include <settings.h>
#include <libintl.h>


typedef struct QBTVCategoriesMenuItemListener_ {
    struct SvObject_ super_;
    QBActiveTree menuTree;
    QBActiveTreeNode node;
} *QBTVCategoriesMenuItemListener;

SvLocal void
QBTVCategoriesMenuItemListenerDestroy(void *self_)
{
    QBTVCategoriesMenuItemListener self = (QBTVCategoriesMenuItemListener) self_;
    SVRELEASE(self->node);
}

SvLocal void
QBTVCategoriesMenuItemListenerInit(QBTVCategoriesMenuItemListener self, SvObject list)
{
    SvInvokeInterface(SvEPGChannelView, list, addListener, (SvObject) self);
}

SvLocal void
QBTVCategoriesMenuItemListenerChannelUpdated(SvObject self_, SvTVChannel channel)
{
    QBTVCategoriesMenuItemListener self = (QBTVCategoriesMenuItemListener) self_;
    SvErrorInfo error = NULL;

    QBActiveTreePropagateNodeChange(self->menuTree, self->node, &error);
    if (error) {
        error = SvErrorInfoCreateWithExtendedInfo(error, "QBActiveTreePropagateSubTreeChange "
                                                          "failed for node with id %s",
                                                  SvStringGetCString(QBActiveTreeNodeGetID(self->node)));
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
}

SvLocal void
QBTVCategoriesMenuItemListenerChannelListCompleted(SvObject self_, int pluginID)
{
}

SvLocal SvType
QBTVCategoriesMenuItemListener_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTVCategoriesMenuItemListenerDestroy
    };

    static const struct SvEPGChannelListListener_ channelListListener = {
        .channelFound         = QBTVCategoriesMenuItemListenerChannelUpdated,
        .channelLost          = QBTVCategoriesMenuItemListenerChannelUpdated,
        .channelModified      = QBTVCategoriesMenuItemListenerChannelUpdated,
        .channelListCompleted = QBTVCategoriesMenuItemListenerChannelListCompleted,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTVCategoriesMenuItemListener",
                            sizeof(struct QBTVCategoriesMenuItemListener_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            SvEPGChannelListListener_getInterface(), &channelListListener,
                            NULL);
    }
    return type;
}

SvLocal QBTVCategoriesMenuItemListener
QBTVCategoriesMenuItemListenerCreate(QBActiveTree menuTree, QBActiveTreeNode node)
{
    QBTVCategoriesMenuItemListener itemListener =
        (QBTVCategoriesMenuItemListener) SvTypeAllocateInstance(QBTVCategoriesMenuItemListener_getType(), NULL);
    itemListener->menuTree = menuTree;
    itemListener->node = SVRETAIN(node);
    return itemListener;
}

typedef struct QBTVCategoriesMenuHandler_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvHashTable itemsListeners;
    QBPlaylistManager playlistsManager;
} *QBTVCategoriesMenuHandler;

SvLocal void
QBTVCategoriesMenuDestroy(void *self_)
{
    QBTVCategoriesMenuHandler self = (QBTVCategoriesMenuHandler) self_;
    SVRELEASE(self->itemsListeners);
}

SvLocal int
QBTVCategoriesMenuGetChannelsCountByCategoryNode(QBPlaylistManager playlistsManager, QBActiveTreeNode node)
{
    SvString bId = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("bouquetID"));
    if (!bId) {
        return -1;
    }
    SvString newID = SvStringCreateWithFormat("Bouquet_%s", SvStringGetCString(bId));
    SvObject list = QBPlaylistManagerGetById(playlistsManager, newID);
    SVRELEASE(newID);

    return (list) ? QBPlaylistManagerGetChannelsCount(playlistsManager, list) : -1;
}

SvLocal bool
QBTVCategoriesMenuNodeSelected(SvObject self_, SvObject node_, SvObject nodePath)
{
    QBTVCategoriesMenuHandler self = (QBTVCategoriesMenuHandler) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    if (SvObjectIsInstanceOf(node_, SvTVChannel_getType())) { // Channel - menu leaf
        QBTVLogicPlayChannel(self->appGlobals->tvLogic, (SvTVChannel) node_, SVSTRING("ChannelMenu"));
        QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
        return true;
    }
    SvString nodeID = QBActiveTreeNodeGetID(node);
    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
    SvString parentID = QBActiveTreeNodeGetID(parent);
    SvObject parentList = QBPlaylistManagerGetById(self->playlistsManager, parentID);

    if (nodeID && !strncmp(SvStringCString(nodeID), "EPG-", 4)) { // TV channels guide
        if (SvInvokeInterface(SvEPGChannelView, parentList, getCount) > 0) {
            QBWindowContext ctx = QBGUILogicPrepareEPGContext(self->appGlobals->guiLogic, NULL, parentID);
            QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
        }
        return false;
    }

    SvObject list = QBPlaylistManagerGetById(self->playlistsManager, nodeID);
    if (!SvStringEqualToCString(parentID, "TIVICategories") && list) { // Playlist menu level
        if (QBPlaylistManagerGetChannelsCount(self->playlistsManager, list) <= 0) {
            return true;
        }
    }

    if (SvStringEqualToCString(parentID, "TIVICategories")) { // Category menu level
        if (QBTVCategoriesMenuGetChannelsCountByCategoryNode(self->playlistsManager, node) <= 0) {
            return true;
        }
    }

    return false;
}
SvLocal void
QBTVCategoriesMenuCategoryAdded(QBTVCategoriesMenuHandler self, QBPlaylistChangeParams arg, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    SvString nodeID = NULL;
    QBTVCategoriesMenuItemListener itemListener = NULL;

    nodeID = SvStringCreateWithFormat("TV-%s", SvStringGetCString(arg->channelListName));
    QBActiveTreeNode node = QBActiveTreeFindNode(self->appGlobals->menuTree, nodeID);
    if (!node) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState, "Failed to get tree node!");
        goto fini;
    }
    itemListener = QBTVCategoriesMenuItemListenerCreate(self->appGlobals->menuTree, node);
    QBTVCategoriesMenuItemListenerInit(itemListener, arg->channelList);
    SvHashTableInsert(self->itemsListeners, (SvObject) nodeID, (SvObject) itemListener);

    QBActiveTreePropagateNodeChange(self->appGlobals->menuTree, node, &error);
    if (error) {
        error = SvErrorInfoCreateWithExtendedInfo(error, "QBActiveTreePropagateSubTreeChange "
                                                          "failed for node with id %s", SvStringGetCString(nodeID));
        goto fini;
    }
fini:
    SVTESTRELEASE(itemListener);
    SVTESTRELEASE(nodeID);
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBTVCategoriesMenuUpdated(SvObject self_, SvObject observedObject, SvObject arg_)
{
    QBTVCategoriesMenuHandler self = (QBTVCategoriesMenuHandler) self_;
    QBPlaylistChangeParams arg = (QBPlaylistChangeParams) arg_;
    SvErrorInfo error = NULL;

    switch (arg->type) {
        case QBPlaylist_CategoryAdded:
            QBTVCategoriesMenuCategoryAdded(self, arg, &error);
            if (error) {
                error = SvErrorInfoCreateWithExtendedInfo(error, "QBTVCategoriesMenuCategoryAdded failed");
                SvErrorInfoWriteLogMessage(error);
                SvErrorInfoDestroy(error);
            }
            break;
        case QBPlaylist_CategoryRemoved:
            break;
        case QBPlaylist_CategoryRenamed:
            break;
    }
}

SvLocal SvType QBTVCategoriesMenuHandler_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTVCategoriesMenuDestroy
    };

    static const struct QBMenuEventHandler_ selectMethods = {
        .nodeSelected = QBTVCategoriesMenuNodeSelected
    };

    static const struct QBObserver_ observerMethods = {
        .observedObjectUpdated = QBTVCategoriesMenuUpdated
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTVCategoriesMenuHandler",
                            sizeof(struct QBTVCategoriesMenuHandler_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &selectMethods,
                            QBObserver_getInterface(), &observerMethods,
                            NULL);
    }

    return type;
}

SvLocal SvWidget QBTVCategoriesMenuCreateItemCallback(SvObject self_, SvObject node_,
                                                      SvObject path, SvApplication app,
                                                      XMBMenuState initialState)
{
    if (!self_ || !node_ || !app) {
        SvLogError("Invalid parameter: self = %p; node_ = %p; app = %p", self_, node_, app);
        return NULL;
    }

    if (SvObjectIsInstanceOf(node_, DVBEPGChannel_getType())) {
        return NULL;
    }

    QBChannelController self = (QBChannelController) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    int channelsCount = -1;

    SvString nodeID = QBActiveTreeNodeGetID(node);
    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
    SvString parentID = QBActiveTreeNodeGetID(parent);

    QBXMBItem itemData = QBChannelControllerCreateItemData(self, node_, app);
    QBXMBItemInfo item = (QBXMBItemInfo) QBChannelControllerCreateItemInfo(self, node_);

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvObject list = QBPlaylistManagerGetById(playlists, nodeID);

    if (!SvStringEqualToCString(parentID, "TIVICategories") && list) { // Playlist menu level
        channelsCount = QBPlaylistManagerGetChannelsCount(playlists, list);
    } else if (SvStringEqualToCString(parentID, "TIVICategories")) { // Category menu level
        channelsCount = QBTVCategoriesMenuGetChannelsCountByCategoryNode(playlists, node);
    }
    if (channelsCount <= 0) {
        item->disabled = true;
        itemData->disabled = true;
    } else if (channelsCount > 0) {
        itemData->subcaption = SvStringCreateWithFormat(ngettext("%d channel", "%d channels",
                                                                 channelsCount),
                                                        channelsCount);
    }
    SvWidget ret = QBXMBItemConstructorInitItem(self->itemConstructor, itemData, app, initialState, item);

    SVRELEASE(itemData);
    return ret;
}

SvLocal void QBTVCategoriesMenuRegisterCallbacks(QBChannelController self)
{
    struct QBChannelControllerCallbacks_s callbacks = {
        .createItem = QBTVCategoriesMenuCreateItemCallback,
    };
    QBChannelMenuControllerSetCallbacks((SvObject) self, &callbacks);
}

SvLocal void
QBTVCategoriesMenuAddListenerToNode(QBTVCategoriesMenuHandler self,
                                    QBActiveTreeNode node,
                                    SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    SvString id = QBActiveTreeNodeGetID(node);
    QBTVCategoriesMenuItemListener itemListener =
        QBTVCategoriesMenuItemListenerCreate(self->appGlobals->menuTree, node);

    SvObject list = QBPlaylistManagerGetById(self->playlistsManager, id);
    if (!list) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBPlaylistManagerGetById failed for id %s",
                                  SvStringGetCString(id));
        SvErrorInfoPropagate(error, errorOut);
    }
    QBTVCategoriesMenuItemListenerInit(itemListener, list);
    SvHashTableInsert(self->itemsListeners, (SvObject) id, (SvObject) itemListener);
    SVRELEASE(itemListener);
}

SvLocal void
QBTVCategoriesMenuAddListenersToLanguageNodes(QBTVCategoriesMenuHandler self,
                                              QBActiveTreeNode rootNode)
{
    self->itemsListeners = SvHashTableCreate(19, NULL);
    SvIterator iterator = QBActiveTreeNodeChildNodesIterator(rootNode);
    SvObject obj = NULL;
    SvErrorInfo error = NULL;
    while ((obj = SvIteratorGetNext(&iterator))) {
        SvIterator childIterator = QBActiveTreeNodeChildNodesIterator((QBActiveTreeNode) obj);
        while ((obj = SvIteratorGetNext(&childIterator))) {
            QBTVCategoriesMenuAddListenerToNode(self, (QBActiveTreeNode) obj, &error);
            if (error) {
                error = SvErrorInfoCreateWithExtendedInfo(error, "QBTVCategoriesMenuAddListenerToNode failed");
                SvErrorInfoWriteLogMessage(error);
                SvErrorInfoDestroy(error);
                error = NULL;
                continue;
            }
        }
    }
}

void QBTVCategoriesMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals, SvString rootID)
{
    if (!menuBar || !pathMap || !appGlobals || !rootID) {
        SvErrorInfo error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                              "menuBar = %p, pathMap = %p, appGlobals = %p, rootID = %p",
                                              menuBar, pathMap, appGlobals, rootID);
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        return;
    }
    SvObject path = NULL;
    QBActiveTreeNode rootNode = QBActiveTreeFindNodeByID(appGlobals->menuTree, rootID, &path);
    if (!path) {
        SvLogWarning("path is NULL. Submenu will not be created for rootID %s.", SvStringGetCString(rootID));
        return;
    }
    QBTVCategoriesMenuHandler self =
        (QBTVCategoriesMenuHandler) SvTypeAllocateInstance(QBTVCategoriesMenuHandler_getType(), NULL);

    self->appGlobals = appGlobals;

    QBTreePathMapInsert(pathMap, path, (SvObject) self, NULL);
    SVRELEASE(self);

    svSettingsPushComponent("ChannelMenu.settings");
    QBChannelController channelController = QBChannelMenuControllerCreate(true, appGlobals, NULL);
    QBTVCategoriesMenuRegisterCallbacks(channelController);
    svSettingsPopComponent();

    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) channelController, NULL);
    SVRELEASE(channelController);
    SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, path, (SvObject) self, NULL);

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    self->playlistsManager = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));

    QBTVCategoriesMenuAddListenersToLanguageNodes(self, rootNode);

    QBObservable bouquetsService = (QBObservable) QBServiceRegistryGetService(registry, SVSTRING("QBDVBBouquets"));
    if (bouquetsService) {
        QBObservableAddObserver(bouquetsService, (SvObject) self, NULL);
    }
    SVRELEASE(path);
}
