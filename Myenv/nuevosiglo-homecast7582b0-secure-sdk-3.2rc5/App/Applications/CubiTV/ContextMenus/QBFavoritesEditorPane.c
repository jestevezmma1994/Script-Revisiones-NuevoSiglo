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

#include "QBFavoritesEditorPane.h"
#include "QBFavoritesEditorPaneInternal.h"

#include <libintl.h>
#include <settings.h>
#include <stdbool.h>
#include <SvEPGDataLayer/Views/SvEPGPersistentView.h>
#include <ContextMenus/QBListPane.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBVerticalMenu.h>
#include <SWL/QBFrame.h>
#include <SWL/label.h>
#include <QBOSK/QBOSKKey.h>
#include <QBOSK/QBOSK.h>
#include <ContextMenus/QBBasicPane.h>
#include <Menus/QBFavoritesMenu.h>
#include <Menus/QBChannelChoiceMenu.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Windows/mainmenu.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <main.h>

typedef enum {
    FavoritesEditorPaneOskValidatorStatus_OK,
    FavoritesEditorPaneOskValidatorStatus_EMPTY,
    FavoritesEditorPaneOskValidatorStatus_EXISTS
} FavoritesEditorPaneOskValidatorStatus;

struct QBFavoritesEditorPaneSortItemConstructor_t {
    struct SvObject_ super_;
    QBXMBItemConstructor itemConstructor;
    QBFrameConstructor* normalFocus;
    QBFrameConstructor* selectedFocus;
};
typedef struct QBFavoritesEditorPaneSortItemConstructor_t* QBFavoritesEditorPaneSortItemConstructor;

SvLocal SvWidget
QBFavoritesEditorPaneSortItemConstructorCreateItem(SvGenericObject self_,
                                        SvGenericObject node_,
                                        SvGenericObject path,
                                        SvApplication app,
                                        XMBMenuState initialState)
{
    QBFavoritesEditorPaneSortItemConstructor self = (QBFavoritesEditorPaneSortItemConstructor) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    SvTVChannel channel = (SvTVChannel) QBActiveTreeNodeGetAttribute(node, SVSTRING("channel"));
    QBXMBItem item = QBXMBItemCreate();
    item->caption = SVRETAIN(channel->name);
    item->focus = SVRETAIN(self->normalFocus);
    item->sortingFocus = SVRETAIN(self->selectedFocus);

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);
    return ret;
}

SvLocal void
QBFavoritesEditorPaneSortItemConstructorSetItemState(SvGenericObject self_,
                                          SvWidget item_,
                                          XMBMenuState state,
                                          bool isFocused)
{
    QBFavoritesEditorPaneSortItemConstructor self = (QBFavoritesEditorPaneSortItemConstructor) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBFavoritesEditorPaneSortItemConstructor__dtor__(void *self_)
{
    QBFavoritesEditorPaneSortItemConstructor self = self_;
    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVRELEASE(self->normalFocus);
    SVRELEASE(self->selectedFocus);
}

SvLocal SvType QBFavoritesEditorPaneSortItemConstructor_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBFavoritesEditorPaneSortItemConstructor__dtor__
    };
    static SvType type = NULL;
    static const struct XMBItemController_t methods = {
        .createItem = QBFavoritesEditorPaneSortItemConstructorCreateItem,
        .setItemState = QBFavoritesEditorPaneSortItemConstructorSetItemState
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBFavoritesEditorPaneSortItemConstructor",
                            sizeof(struct QBFavoritesEditorPaneSortItemConstructor_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &methods,
                            NULL);
    }
    return type;
}

SvLocal QBFavoritesEditorPaneSortItemConstructor QBFavoritesEditorPaneSortItemConstructorCreate(QBFavoritesEditorPane self)
{
    QBFavoritesEditorPaneSortItemConstructor constructor =(QBFavoritesEditorPaneSortItemConstructor)  SvTypeAllocateInstance(QBFavoritesEditorPaneSortItemConstructor_getType(), NULL);
    constructor->itemConstructor = QBXMBItemConstructorCreate("MenuItem", self->appGlobals->textRenderer);
    constructor->normalFocus = QBFrameConstructorFromSM("NormalFocus");
    constructor->selectedFocus = QBFrameConstructorFromSM("SelectedFocus");

    return constructor;
}

/*
 * Typical stuff for pane management
 */

SvLocal void QBFavoritesEditorPane__dtor__(void *ptr)
{
    QBFavoritesEditorPane self = ptr;
    QBContextMenuSetInputEventHandler(self->contextMenu, NULL, NULL);
    SVRELEASE(self->options);
    SVTESTRELEASE(self->addRemoveListType);
    SVTESTRELEASE(self->channel);
    SVTESTRELEASE(self->tickedChannels);
    SVTESTRELEASE(self->channelSelectionPane);
    SVTESTRELEASE(self->listID);
    SVTESTRELEASE(self->listName);
    SVTESTRELEASE(self->sortDataSource);
    SVTESTRELEASE(self->sortingPane);
    SVTESTRELEASE(self->epgManager);
}

SvLocal void QBFavoritesEditorPaneShow(SvGenericObject self_)
{
    QBFavoritesEditorPane self = (QBFavoritesEditorPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->options, show);
}

SvLocal void QBFavoritesEditorPaneHide(SvGenericObject self_, bool immediately)
{
    QBFavoritesEditorPane self = (QBFavoritesEditorPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->options, hide, immediately);
}

SvLocal void QBFavoritesEditorPaneSetActive(SvGenericObject self_)
{
    QBFavoritesEditorPane self = (QBFavoritesEditorPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->options, setActive);
}

SvLocal bool QBFavoritesEditorPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvType QBFavoritesEditorPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBFavoritesEditorPane__dtor__
    };
    static SvType type = NULL;
    static const struct QBContextMenuPane_ methods = {
        .show             = QBFavoritesEditorPaneShow,
        .hide             = QBFavoritesEditorPaneHide,
        .setActive        = QBFavoritesEditorPaneSetActive,
        .handleInputEvent = QBFavoritesEditorPaneHandleInputEvent
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBFavoritesEditorPane",
                            sizeof(struct QBFavoritesEditorPane_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextMenuPane_getInterface(), &methods,
                            NULL);
    }
    return type;
}

/*
 * OSK
 */

SvLocal FavoritesEditorPaneOskValidatorStatus FavoritesEditorPaneValidateOskTextField(QBFavoritesEditorPane self, SvString newName)
{
    SvErrorInfo error = NULL;

    if (SvStringGetLength(newName) == 0) {
        return FavoritesEditorPaneOskValidatorStatus_EMPTY;
    }
    if (SvObjectEquals((SvObject) newName, (SvObject) self->listName)) {
        return FavoritesEditorPaneOskValidatorStatus_OK;
    }

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlistsManager = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));

    bool similarExists = QBPlaylistManagerSimilarPlaylistExists(playlistsManager, newName, self->listID, &error);
    if (error) {
        SvErrorInfoCreateWithExtendedInfo(error, "QBPlaylistManagerSimilarPlaylistExists failed");
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        return FavoritesEditorPaneOskValidatorStatus_EXISTS;
    }

    if (similarExists) {
        return FavoritesEditorPaneOskValidatorStatus_EXISTS;
    }
    return FavoritesEditorPaneOskValidatorStatus_OK;
}

SvLocal void QBFavoritesEditorPaneRenameList(QBFavoritesEditorPane self, SvString newName)
{
    QBActiveTreeNode node = QBActiveTreeFindNode(self->appGlobals->menuTree, self->listID);

    SvObject origPath = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, self->listID);
    SvObject path = SvObjectCopy(origPath, NULL);
    SvInvokeInterface(QBTreePath, path, truncate, -1);
    SVRELEASE(origPath);

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    QBPlaylistManagerRename(playlists, self->listID, newName);
    QBPlaylistManagerSave(playlists);

    QBActiveTreeNodeSetAttribute(node, SVSTRING("listName"), (SvGenericObject) newName);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvGenericObject) newName);
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, path, 0);
    size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
    QBActiveTreePropagateNodesChange(self->appGlobals->menuTree, path, 0, nodesCount, NULL);
    SVRELEASE(path);

    SVRELEASE(self->listName);
    self->listName = SVRETAIN(newName);
}

SvLocal void QBFavoritesEditorPaneOSKKeyPressed(void *self_, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBFavoritesEditorPane self = self_;
    if (key->type == QBOSKKeyType_enter) {
        if (FavoritesEditorPaneValidateOskTextField(self, input) != FavoritesEditorPaneOskValidatorStatus_OK)
            return;
        QBFavoritesEditorPaneRenameList(self, input);
        QBContextMenuHide(self->contextMenu, false);
    } else if (key->type == QBOSKKeyType_default || key->type == QBOSKKeyType_backspace) {
        SvWidget title = QBOSKPaneGetTitle(pane);
        switch(FavoritesEditorPaneValidateOskTextField(self, input)) {
            case FavoritesEditorPaneOskValidatorStatus_OK:
                svLabelSetText(title, "");
                break;
            case FavoritesEditorPaneOskValidatorStatus_EMPTY:
                svLabelSetText(title, gettext("List name cannot be empty"));
                break;
            case FavoritesEditorPaneOskValidatorStatus_EXISTS:
                svLabelSetText(title, gettext("List name already exists"));
                break;
        }
    }
}

/*
 * Add/Remove channels
 */

SvLocal ChannelChoiceTickState QBFavoritesEditorPaneAddRemoveChannelsIsTicked(void *self_, SvGenericObject channel)
{
    QBFavoritesEditorPane self = self_;
    if (SvHashTableFind(self->tickedChannels, channel)) {
        return ChannelChoiceTickState_On;
    }

    return ChannelChoiceTickState_Off;
}

SvLocal SvString QBFavoritesEditorPaneAddRemoveChannelsCreateCaption(void *self_, SvGenericObject channel_)
{
    QBFavoritesEditorPane self = self_;
    if (!SvObjectIsInstanceOf(channel_, SvTVChannel_getType()))
        return NULL;

    SvTVChannel channel = (SvTVChannel) channel_;

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject allChannels = QBPlaylistManagerGetById(playlists, self->addRemoveListType);
    SvString caption = SvStringCreateWithFormat("%03i %s", SvInvokeInterface(SvEPGChannelView, allChannels, getChannelNumber, channel), SvStringCString(channel->name));

    return caption;
}

SvLocal void QBFavoritesEditorPaneAddRemoveChannelsToggle(void *self_, QBListPane pane, SvGenericObject channel, int pos)
{
    QBFavoritesEditorPane self = self_;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvEPGPersistentView channelsList = (SvEPGPersistentView) QBPlaylistManagerGetById(playlists, self->listID);

    if(SvHashTableFind(self->tickedChannels, channel)) {
        SvHashTableRemove(self->tickedChannels, channel);
        SvEPGPersistentViewRemoveChannel(channelsList, (SvTVChannel) channel, NULL);
    } else {
        SvHashTableInsert(self->tickedChannels, channel, channel);
        SvEPGPersistentViewAddChannel(channelsList, (SvTVChannel) channel, NULL);
    }
    SvEPGManagerPropagateChannelChange(self->epgManager, (SvTVChannel) channel, NULL);

    if (SvInvokeInterface(SvEPGChannelView, channelsList, getCount) == 0) {
        SvString currentId = QBPlaylistManagerGetCurrentId(playlists);
        if (SvObjectEquals((SvObject) currentId, (SvObject) self->listID)) {
            QBPlaylistCursor cursor = QBPlaylistCursorsGet(self->appGlobals->playlistCursors, SVSTRING("TV"), self->addRemoveListType);
            QBTVLogicPlayChannelByCursor(self->appGlobals->tvLogic, cursor, SVSTRING("FavoritesEditorPane"));
        }
    }
}

/*
 * Sort channels
 */

SvLocal void QBFavoritesEditorPaneSortSave(QBFavoritesEditorPane self)
{
    SvEPGPersistentView channelsList;
    QBActiveTreeNode node;
    SvArray channels;
    SvTVChannel ch;
    size_t i, cnt;

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    channelsList = (SvEPGPersistentView) QBPlaylistManagerGetById(playlists, self->listID);
    if (!channelsList || !SvObjectIsInstanceOf((SvObject) channelsList, SvEPGPersistentView_getType()))
        return;

    cnt = SvInvokeInterface(QBListModel, self->sortDataSource, getLength);
    channels = SvArrayCreateWithCapacity(cnt, NULL);
    for (i = 0; i < cnt; i++) {
        node = (QBActiveTreeNode) SvInvokeInterface(QBListModel, self->sortDataSource, getObject, i);
        ch = (SvTVChannel) QBActiveTreeNodeGetAttribute(node, SVSTRING("channel"));
        SvArrayAddObject(channels, (SvGenericObject) ch);
    }

    SvEPGPersistentViewSetChannels(channelsList, channels, NULL);
    SVRELEASE(channels);

    QBPlaylistManagerSave(playlists);
}

SvLocal void QBFavoritesEditorPaneSortChannelsDrag(void *self_, QBListPane pane, SvGenericObject channel, int pos)
{
    QBFavoritesEditorPane self = self_;

    SvWidget menu = QBListPaneGetMenu(self->sortingPane);
    XMBVerticalMenuSetState(menu, XMBMenuState_sorting);
}

SvLocal void QBFavoritesEditorPaneSortEnded(void *self_, int prev, int next)
{
    QBFavoritesEditorPane self = self_;

    SvWidget menu = QBListPaneGetMenu(self->sortingPane);
    XMBVerticalMenuSetState(menu, XMBMenuState_normal);
    QBActiveArrayMoveObject(self->sortDataSource, prev, next, NULL);
    QBFavoritesEditorPaneSortSave(self);
}

SvLocal void QBFavoritesEditorPaneCheckLimitBack(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFavoritesEditorPane self = self_;
    QBContextMenuPopPane(self->contextMenu);
}

SvLocal void QBFavoritesEditorPaneShowLimitPane(QBFavoritesEditorPane self)
{
    const char *msg = gettext("You have too many favorites lists.");

    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->contextMenu, 2, SVSTRING("BasicPane"));
    SvString title = SvStringCreate(msg, NULL);
    QBBasicPaneSetTitle(options, title);
    SVRELEASE(title);
    SvString option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("cancel"), option, QBFavoritesEditorPaneCheckLimitBack, self);
    SVRELEASE(option);
    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) options);
    SVRELEASE(options);
    svSettingsPopComponent();
}

SvLocal bool QBFavoritesEditorPaneCheckLimit(QBFavoritesEditorPane self)
{
#define MAX_FAVORITES 20
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    return  QBPlaylistManagerGetListCountForType(playlists, SVSTRING("FAVORITES")) < MAX_FAVORITES;
#undef MAX_FAVORITES
}

/*
 * First level pane callbacks
 */

SvLocal void QBFavoritesEditorPaneAddNewList(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFavoritesEditorPane self = self_;

    SvGenericObject channelsList;
    SvWidget menuBar = NULL;
    unsigned int i = 0;
    SvString name;

    if (!QBFavoritesEditorPaneCheckLimit(self)) {
        QBFavoritesEditorPaneShowLimitPane(self);
        return;
    }

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    // find unused name
    do {
        name = SvStringCreateWithFormat(gettext("List no. %u"), ++i);
        if ((channelsList = QBPlaylistManagerGet(playlists, name)))
            SVRELEASE(name);
    } while (channelsList);
    channelsList = QBPlaylistManagerCreatePlaylist(playlists, name, SVSTRING("FAVORITES"));

    SvValue epgVal = SvValueCreateWithBoolean(true, NULL);
    QBPlaylistManagerSetPlaylistMetaData(playlists, channelsList, (SvGenericObject) SVSTRING("EPG"), (SvGenericObject) epgVal);
    SVRELEASE(epgVal);

    QBPlaylistManagerSave(playlists);
    SVRELEASE(name);

    if (self->appGlobals->main)
        menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    if (menuBar)
        QBFavoritesMenuAddChannelsList(menuBar, self->appGlobals, channelsList, SVSTRING("FAVS"), -1, true);

    QBContextMenuHide(self->contextMenu, false);

    SVRELEASE(channelsList);
}

SvLocal void QBFavoritesEditorPaneListByIndexSelected(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFavoritesEditorPane self = self_;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    QBPlaylistManagerSetListNumberingById(playlists, self->listID, SvEPGViewChannelNumbering_byIndex);
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void QBFavoritesEditorPaneListByNumberSelected(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFavoritesEditorPane self = self_;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    QBPlaylistManagerSetListNumberingById(playlists, self->listID, SvEPGViewChannelNumbering_byNumber);
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void QBFavoritesEditorPaneChangeListNumbering(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFavoritesEditorPane self = self_;
    svSettingsRestoreContext(self->settingsCtx);
    QBBasicPane numbering = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(numbering, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->contextMenu, 2, SVSTRING("BasicPane"));
    SvString option = SvStringCreate(gettext("By Index"), NULL);
    QBBasicPaneAddOption(numbering, SVSTRING("by index"), option, QBFavoritesEditorPaneListByIndexSelected, self);
    SVRELEASE(option);
    option = SvStringCreate(gettext("By Channel Number"), NULL);
    QBBasicPaneAddOption(numbering, SVSTRING("by channel number"), option, QBFavoritesEditorPaneListByNumberSelected, self);
    SVRELEASE(option);
    svSettingsPopComponent();
    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) numbering);
    SVRELEASE(numbering);
}

SvLocal void QBFavoritesEditorPaneRemoveList(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFavoritesEditorPane self = self_;

    if(!SvStringEqualToCString(id, "CANCEL")) {
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        SvString currentId = QBPlaylistManagerGetCurrentId(playlists);
        if (SvObjectEquals((SvObject) currentId, (SvObject) self->listID)) {
            QBPlaylistCursor cursor = QBPlaylistCursorsGet(self->appGlobals->playlistCursors, SVSTRING("TV"), SVSTRING("TVChannels"));
            QBTVLogicPlayChannelByCursor(self->appGlobals->tvLogic, cursor, SVSTRING("FavoritesEditorPane"));
        }
        SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
        QBFavoritesMenuRemoveChannelsList(self->appGlobals->menuTree, menuBar, self->listID);
        QBPlaylistManagerRemove(playlists, self->listID);
        QBPlaylistManagerSave(playlists);
    }
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void QBFavoritesEditorPaneRemoveChannel(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFavoritesEditorPane self = self_;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    if (!SvStringEqualToCString(id, "CANCEL")) {
        SvGenericObject view = QBPlaylistManagerGetById(playlists, self->listID);
        if (view && SvObjectIsInstanceOf(view, SvEPGPersistentView_getType()))
            SvEPGPersistentViewRemoveChannel((SvEPGPersistentView) view, self->channel, NULL);
    }
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void QBFavoritesEditorPaneSortChannels(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFavoritesEditorPane self = self_;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject playlist = QBPlaylistManagerGetById(playlists, self->listID);
    int cnt = SvInvokeInterface(QBListModel, playlist, getLength);
    self->sortDataSource = QBActiveArrayCreate(cnt, NULL);
    int i;
    for(i = 0; i < cnt; i++) {
        SvGenericObject channel = SvInvokeInterface(QBListModel, playlist, getObject, i);
        QBActiveTreeNode node = QBActiveTreeNodeCreate(NULL, NULL, NULL);
        QBActiveTreeNodeSetAttribute(node, SVSTRING("channel"), channel);
        QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvGenericObject) ((SvTVChannel)channel)->name);
        QBActiveArrayAddObject(self->sortDataSource, (SvGenericObject) node, NULL);
        SVRELEASE(node);
    }


    svSettingsRestoreContext(self->settingsCtx);

    QBFavoritesEditorPaneSortItemConstructor constructor = QBFavoritesEditorPaneSortItemConstructorCreate(self);

    QBListPane channels = (QBListPane) SvTypeAllocateInstance(QBListPane_getType(), NULL);
    static struct QBListPaneCallbacks_t callbacks = {
        .selected = QBFavoritesEditorPaneSortChannelsDrag,
        .reordered = QBFavoritesEditorPaneSortEnded
    };
    QBListPaneInit(channels, self->appGlobals->res, (SvGenericObject) self->sortDataSource, (SvGenericObject) constructor, &callbacks, self, self->contextMenu, 2, SVSTRING("ListPane"));

    SVRELEASE(constructor);

    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) channels);
    self->sortingPane = channels;
}

SvLocal void QBFavoritesEditorPaneShowAddRemoveChannels(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFavoritesEditorPane self = self_;

    SVTESTRELEASE(self->addRemoveListType);
    if (id && SvStringEqualToCString(id, "add/remove tv channels")) {
        self->addRemoveListType = SvStringCreate("TVChannels", NULL);
    } else if (id && SvStringEqualToCString(id, "add/remove radio channels")) {
        self->addRemoveListType = SvStringCreate("RadioChannels", NULL);
    } else { // shouldn't happen
        self->addRemoveListType = NULL;
    }

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject playlist = QBPlaylistManagerGetById(playlists, self->listID);
    SvGenericObject allChannels = QBPlaylistManagerGetById(playlists, self->addRemoveListType);
    SVTESTRELEASE(self->tickedChannels);
    self->tickedChannels = SvHashTableCreate(SvInvokeInterface(SvEPGChannelView, playlist, getCount) + 1, NULL);
    int cnt = SvInvokeInterface(QBListModel, playlist, getLength);
    int i;
    for(i = 0; i < cnt; i++) {
        SvGenericObject channel = SvInvokeInterface(QBListModel, playlist, getObject, i);
        SvHashTableInsert(self->tickedChannels, channel, channel);
    }

    QBListPane channels = (QBListPane) SvTypeAllocateInstance(QBListPane_getType(), NULL);
    static struct QBChannelChoiceControllerCallbacks_t callbacks = {
        .isTicked = QBFavoritesEditorPaneAddRemoveChannelsIsTicked,
        .createNodeCaption = QBFavoritesEditorPaneAddRemoveChannelsCreateCaption
    };
    QBChannelChoiceController constructor = QBChannelChoiceControllerCreateFromSettings("ChannelChoiceMenu.settings", self->appGlobals->textRenderer, allChannels, self, &callbacks);

    svSettingsRestoreContext(self->settingsCtx);
    static struct QBListPaneCallbacks_t listCallbacks = {
        .selected = QBFavoritesEditorPaneAddRemoveChannelsToggle
    };
    QBListPaneInit(channels, self->appGlobals->res, allChannels, (SvGenericObject) constructor, &listCallbacks, self, self->contextMenu, 2, SVSTRING("ListPane"));
    svSettingsPopComponent();
    SVRELEASE(constructor);

    SVTESTRELEASE(self->channelSelectionPane);
    self->channelSelectionPane = channels;

    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) channels);
}

/*
 * Initialization
 */

void QBFavoritesEditorPaneInit(QBFavoritesEditorPane self, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, SvString listID, SvTVChannel channel, SvString itemNamesFilename)
{
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvEPGPersistentView channelsList = (SvEPGPersistentView) QBPlaylistManagerGetById(playlists, listID);

    self->appGlobals = appGlobals;
    self->epgManager = SVRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));
    self->contextMenu = ctxMenu;
    self->settingsCtx = svSettingsSaveContext();
    self->listName = SVRETAIN(QBPlaylistManagerGetPlaylistName(playlists, (SvGenericObject) channelsList));
    self->listID = SVRETAIN(listID);
    self->options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(self->options, appGlobals->res, appGlobals->scheduler, appGlobals->textRenderer, ctxMenu, 1, SVSTRING("BasicPane"));
    QBBasicPaneLoadOptionsFromFile(self->options, itemNamesFilename);
    SVTESTRELEASE(itemNamesFilename);

    QBBasicPaneAddOption(self->options, SVSTRING("add new list"), NULL, QBFavoritesEditorPaneAddNewList, self);

    bool isStatic = !SvStringEqualToCString(QBPlaylistManagerGetPlaylistType(playlists, (SvGenericObject) channelsList), "FAVORITES");

    if (!isStatic) {
        SvArray options = SvArrayCreate(NULL);
        SvArray ids = SvArrayCreate(NULL);
        SvString option = SvStringCreate(gettext("Remove"), NULL);
        SvArrayAddObject(options, (SvGenericObject) option);
        SvArrayAddObject(ids, (SvGenericObject) SVSTRING("OK"));
        SVRELEASE(option);
        option = SvStringCreate(gettext("Cancel"), NULL);
        SvArrayAddObject(options,  (SvGenericObject) option);
        SvArrayAddObject(ids, (SvGenericObject) SVSTRING("CANCEL"));
        SVRELEASE(option);
        QBBasicPaneAddOptionWithConfirmation(self->options, SVSTRING("remove list"), NULL, SVSTRING("BasicPane"), options, ids, QBFavoritesEditorPaneRemoveList, self);
        QBBasicPaneItem item = QBBasicPaneAddOptionWithOSK(self->options, SVSTRING("rename list"), NULL, SVSTRING("OSKPane"), QBFavoritesEditorPaneOSKKeyPressed, self);
        if (item) {
            QBOSKPaneSetInput((QBOSKPane) item->subpane, self->listName);
        }
        if (SvInvokeInterface(SvEPGChannelView, channelsList, getCount)) {
            QBBasicPaneAddOption(self->options, SVSTRING("rearrange list"), NULL, QBFavoritesEditorPaneSortChannels, self);
        }

        SvEPGPersistentView tvChannels = (SvEPGPersistentView) QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
        size_t tvChannelsCount = tvChannels ? SvInvokeInterface(SvEPGChannelView, tvChannels, getCount) : 0;

        SvEPGPersistentView radioChannels = (SvEPGPersistentView) QBPlaylistManagerGetById(playlists, SVSTRING("RadioChannels"));
        size_t radioChannelsCount = radioChannels ? SvInvokeInterface(SvEPGChannelView, radioChannels, getCount) : 0;

        if (tvChannelsCount || radioChannelsCount) {
            QBBasicPaneAddOption(self->options, SVSTRING("change list numbering"), NULL, QBFavoritesEditorPaneChangeListNumbering, self);
        }

        if (tvChannelsCount) {
            QBBasicPaneAddOption(self->options, SVSTRING("add/remove tv channels"), NULL, QBFavoritesEditorPaneShowAddRemoveChannels, self);
        }

        if (radioChannelsCount) {
            QBBasicPaneAddOption(self->options, SVSTRING("add/remove radio channels"), NULL, QBFavoritesEditorPaneShowAddRemoveChannels, self);
        }

        if(channel) {
            self->channel = SVRETAIN(channel);
            QBBasicPaneAddOptionWithConfirmation(self->options, SVSTRING("remove channel"), NULL, SVSTRING("BasicPane"), options, ids, QBFavoritesEditorPaneRemoveChannel, self);
        }
        SVRELEASE(options);
        SVRELEASE(ids);
    }
}

QBFavoritesEditorPane QBFavoritesEditorPaneCreateFromSettings(const char *settings, const char *itemNamesFilename, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, SvString listID, SvTVChannel channel)
{
    svSettingsPushComponent(settings);
    QBFavoritesEditorPane self = (QBFavoritesEditorPane) SvTypeAllocateInstance(QBFavoritesEditorPane_getType(), NULL);
    QBFavoritesEditorPaneInit(self, appGlobals, ctxMenu, widgetName, listID, channel, SvStringCreate(itemNamesFilename, NULL));
    svSettingsPopComponent();

    return self;
}
