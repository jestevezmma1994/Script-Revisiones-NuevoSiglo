/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#include <Menus/QBFavoritesMenuController.h>

#include <libintl.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Menus/QBChannelMenu.h>
#include <Menus/QBChannelMenuInternal.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <XMB2/XMBMenuBar.h>
#include <XMB2/XMBItemController.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Windows/mainmenu.h>
#include <Widgets/QBMenuCaptionGetter.h>
#include <Translations/QBTranslator.h>
#include <main.h>

struct QBFavoritesController_t {
    struct SvObject_ super_;

    QBChannelController channelController;
    QBTreeProxy proxy;
    SvString channelsListID;
    AppGlobals appGlobals;
};

SvLocal QBXMBItem QBFavoritesControllerCreateGenericItemData(QBFavoritesController self, QBActiveTreeNode node, SvApplication app)
{
    QBXMBItem itemData = QBXMBItemCreate();
    SvString listID = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("listID"));
    SvString nodeID = QBActiveTreeNodeGetID(node);

    if (nodeID && !strncmp(SvStringCString(nodeID), "EPG-", 4)) {
        SvString caption = (SvString)QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
        if (caption) {
            const char *translatedCaptionStr = gettext(SvStringCString(caption));
            itemData->caption = SvStringCreate(translatedCaptionStr, NULL);
        }
    } else {
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        itemData->caption = SvStringCreate(QBTranslatorGetPlaylistName(playlists, listID), NULL);
    }
    itemData->subcaption = SVTESTRETAIN(QBActiveTreeNodeGetAttribute(node, SVSTRING("subcaption")));

    itemData->loadingRID = self->channelController->iconRID;

    SvString iconPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("icon"));
    itemData->iconURI.URI = SVTESTRETAIN(iconPath);
    itemData->iconURI.isStatic = true;
    return itemData;
}

SvLocal QBXMBItem QBFavoritesControllerCreateItemData(QBFavoritesController self, SvObject node_, SvApplication app)
{
    QBXMBItem item = NULL;
    if (SvObjectIsInstanceOf(node_, SvTVChannel_getType())) {
        item = QBChannelControllerCreateChannelItemData(self->channelController, (SvTVChannel) node_, app);
    } else {
        item = QBFavoritesControllerCreateGenericItemData(self, (QBActiveTreeNode) node_, app);
    }

    item->focus = SVRETAIN(self->channelController->focus);
    item->inactiveFocus = SVTESTRETAIN(self->channelController->inactiveFocus);
    item->disabledFocus = SVTESTRETAIN(self->channelController->disabledFocus);

    return item;
}

SvLocal QBXMBItemInfo QBFavoritesControllerCreateItemInfo(QBFavoritesController self, SvGenericObject node_)
{
    QBXMBItemInfo itemInfo;
    if (SvObjectIsInstanceOf(node_, SvTVChannel_getType())) {
        itemInfo = (QBXMBItemInfo) QBChannelControllerCreateItemInfo(self->channelController, node_);
    } else {
        itemInfo = QBXMBItemInfoCreate();
    }

    return itemInfo;
}


SvLocal SvWidget QBFavoritesControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBFavoritesController self = (QBFavoritesController) self_;
    QBXMBItem itemData = QBFavoritesControllerCreateItemData(self, node_, app);
    QBXMBItemInfo itemInfo = QBFavoritesControllerCreateItemInfo(self, node_);
    SvWidget ret = QBXMBItemConstructorInitItem(self->channelController->itemConstructor, itemData, app, initialState, itemInfo);
    SVRELEASE(itemData);
    return ret;
}

SvLocal void QBFavoritesControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBFavoritesController self = (QBFavoritesController) self_;
    QBXMBItemConstructorSetItemState(self->channelController->itemConstructor, item_, state, isFocused);
}

SvLocal SvString QBFavoritesControllerGetCaption(SvGenericObject self_, SvGenericObject path, SvGenericObject node_)
{
    if (SvInvokeInterface(QBTreePath, path, getLength) <= 1)
        return NULL;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvString caption = SvStringCreate(QBTranslatorGetPlaylistName(playlists, (SvString)QBActiveTreeNodeGetAttribute(node, SVSTRING("listID"))), NULL);
    SVAUTORELEASE(caption);
    return caption;
}

SvLocal void QBFavoritesControllerSelectChannel(SvGenericObject self_, SvTVChannel channel)
{
    QBFavoritesController self = (QBFavoritesController) self_;

    SvObject nodePath_ = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, self->channelsListID);
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

    SvWidget menuBar = SvInvokeInterface(QBMainMenu, (SvObject) self->appGlobals->main, getMenu);
    SvInvokeInterface(QBMenu, menuBar->prv, setSubmenuPosition, nodePath_, position, false);

    SVRELEASE(nodePath_);
}

SvLocal SvString QBFavoritesControllerGetPlaylistID(SvGenericObject self_)
{
    QBFavoritesController self = (QBFavoritesController) self_;
    return self->channelsListID;
}

SvLocal void QBFavoritesControllerDestroy(void *self_)
{
    QBFavoritesController self = self_;
    SVRELEASE(self->channelController);
    SVTESTRELEASE(self->proxy);
    SVTESTRELEASE(self->channelsListID);
}

SvLocal SvType QBFavoritesController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBFavoritesControllerDestroy
    };
    static SvType type = NULL;
    static const struct XMBItemController_t controller_methods = {
        .createItem = QBFavoritesControllerCreateItem,
        .setItemState = QBFavoritesControllerSetItemState,
    };

    static const struct QBMenuCaptionGetter_t caption_methods = {
        .getMenuCaptionForPath = QBFavoritesControllerGetCaption
    };

    static const struct QBChannelMenu_ channelMenuMethods = {
        .selectChannel = QBFavoritesControllerSelectChannel,
        .getPlaylistID = QBFavoritesControllerGetPlaylistID
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBFavoritesController",
                            sizeof(struct QBFavoritesController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            QBMenuCaptionGetter_getInterface(), &caption_methods,
                            QBChannelMenu_getInterface(), &channelMenuMethods,
                            NULL);
    }

    return type;
}


QBFavoritesController QBFavoritesMenuControllerCreate(AppGlobals appGlobals, SvGenericObject channelsList)
{
    QBFavoritesController self = (QBFavoritesController) SvTypeAllocateInstance(QBFavoritesController_getType(), NULL);
    self->appGlobals = appGlobals;
    self->channelController = QBChannelMenuControllerCreate(false, appGlobals, channelsList);

    if (!channelsList)
        goto fini;

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    self->channelsListID = QBPlaylistManagerGetPlaylistId(playlists, channelsList);
    if (!self->channelsListID) {
        goto fini;
    }

    SVRETAIN(self->channelsListID);

    QBActiveTreeNode node = QBActiveTreeFindNode(appGlobals->menuTree, self->channelsListID);
    if (!node)
        goto fini;

    self->proxy = QBTreeProxyCreate((SvGenericObject) channelsList, (SvGenericObject) node, NULL);
    QBMainMenuAddManualChannelSelectionListener(appGlobals->main, (SvGenericObject) self, (SvGenericObject) node);

fini:
    return self;
}
