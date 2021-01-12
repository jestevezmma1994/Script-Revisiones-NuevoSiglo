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

#include "miniFavoriteList.h"

#include <QBApplicationController.h>
#include <QBDataModel3/QBDataModel.h>
#include <main.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <Menus/QBChannelMenu.h>
#include <Windows/newtv.h>
#include <QBConf.h>
#include <QBInput/QBInputCodes.h>
#include <SWL/QBFrame.h>
#include <SWL/fade.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBVerticalMenu.h>
#include <XMB2/XMBItemController.h>
#include <Widgets/QBScrollBar.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <stddef.h>


struct QBMiniFavoriteItemController_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvBitmap icon;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    QBXMBItemConstructor itemConstructor, inactiveItemConstructor;
};
typedef struct QBMiniFavoriteItemController_t* QBMiniFavoriteItemController;

SvLocal void QBMiniFavoriteItemController__dtor__(void *ptr)
{
    QBMiniFavoriteItemController self = ptr;
    SVRELEASE(self->icon);
    SVRELEASE(self->focus);
    SVRELEASE(self->inactiveFocus);
    QBXMBItemConstructorDestroy(self->itemConstructor);
    QBXMBItemConstructorDestroy(self->inactiveItemConstructor);
}

SvLocal SvWidget QBMiniFavoriteItemControllerCreateItem(SvGenericObject self_, SvGenericObject list, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBMiniFavoriteItemController self = (QBMiniFavoriteItemController) self_;

    QBXMBItem item = QBXMBItemCreate();
    SvWidget ret = NULL;
    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVRETAIN(self->inactiveFocus);
    item->icon = SVRETAIN(self->icon);

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    item->caption = SVRETAIN(QBPlaylistManagerGetPlaylistName(playlists, list));
    if(SvInvokeInterface(QBListModel, list, getLength)>0) {
        ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    }else{
        ret = QBXMBItemConstructorCreateItem(self->inactiveItemConstructor, item, app, initialState);
    }
    SVRELEASE(item);
    return ret;
}

SvLocal void QBMiniFavoriteItemControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBMiniFavoriteItemController self = (QBMiniFavoriteItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}


SvLocal SvType QBMiniFavoriteItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMiniFavoriteItemController__dtor__
    };
    static SvType type = NULL;

    static const struct XMBItemController_t controllerMethods = {
        .createItem = QBMiniFavoriteItemControllerCreateItem,
        .setItemState = QBMiniFavoriteItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMiniFavoriteItemController",
                            sizeof(struct QBMiniFavoriteItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controllerMethods,
                            NULL);
    }

    return type;
}


struct QBMiniFavoriteList_t {
    struct QBLocalWindow_t super_;
    SvWidget xmbMenu;
    QBActiveArray lists;
    AppGlobals appGlobals;

    SvEffectId fadeId;
    double fadeTime;
    bool visible;
};

SvLocal void
QBMiniFavoriteList__dtor__(void *self_)
{
    QBMiniFavoriteList self = self_;
    svWidgetDestroy(self->super_.window);
    SVRELEASE(self->lists);
}

SvLocal void QBMiniFavoriteListPropagateChannelsAdd(SvGenericObject self_,
                                                    SvGenericObject dataSource,
                                                    size_t first, size_t count)
{
    QBMiniFavoriteList self = (QBMiniFavoriteList) self_;
    if ((first == 0) && (count == SvInvokeInterface(QBListModel, dataSource, getLength))) {
        QBActiveArrayPropagateObjectsChange(self->lists, QBActiveArrayIndexOfObject(self->lists, dataSource, NULL), 1, NULL);
    }
}

SvLocal void QBMiniFavoriteListPropagateChannelsRemove(SvGenericObject self_,
                                                       SvGenericObject dataSource,
                                                       size_t first, size_t count)
{
    QBMiniFavoriteList self = (QBMiniFavoriteList) self_;
    if (SvInvokeInterface(QBListModel, dataSource, getLength) == 0) {
        QBActiveArrayPropagateObjectsChange(self->lists, QBActiveArrayIndexOfObject(self->lists, dataSource, NULL), 1, NULL);
    }
}

SvLocal void QBMiniFavoriteListPropagateChannelsDoNothing(SvGenericObject self_,
                                                          SvGenericObject dataSource,
                                                          size_t first, size_t count)
{

}

SvLocal void QBMiniFavoriteListPropagateChannelsReplace(SvGenericObject self_,
                                                        SvGenericObject dataSource,
                                                        size_t first, size_t count)
{
    QBMiniFavoriteList self = (QBMiniFavoriteList) self_;
    if ((first == 0) && (count == SvInvokeInterface(QBListModel, dataSource, getLength))) {
        QBActiveArrayPropagateObjectsChange(self->lists, QBActiveArrayIndexOfObject(self->lists, dataSource, NULL), 1, NULL);
    }
}

SvLocal SvType QBMiniFavoriteList_getType(void)
{
    static const struct QBWindowVTable_ vtable = {
        .super_      = {
            .destroy = QBMiniFavoriteList__dtor__
        }
    };
    static SvType type = NULL;


    static const struct QBListModelListener2_ listMethods = {
        .itemsAdded = QBMiniFavoriteListPropagateChannelsAdd,
        .itemsRemoved = QBMiniFavoriteListPropagateChannelsRemove,
        .itemsChanged = QBMiniFavoriteListPropagateChannelsDoNothing,
        .itemsReordered = QBMiniFavoriteListPropagateChannelsDoNothing,
        .itemsReplaced = QBMiniFavoriteListPropagateChannelsReplace
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMiniFavoriteList",
                            sizeof(struct QBMiniFavoriteList_t),
                            QBLocalWindow_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            QBListModelListener2_getInterface(), &listMethods,
                            NULL);
    }

    return type;
}

SvLocal bool QBMiniFavoriteListInputEventHandler(SvWidget w, SvInputEvent ie)
{
    switch (ie->ch) {
        case QBKEY_UP:
        case QBKEY_DOWN:
            break;
        default:
            return false;
    }
    return true;
}

SvLocal void
QBMiniFavoriteListClean(SvApplication app, void *prv)
{
}

SvLocal void
QBMiniFavoriteListOnChangePosition(void *w_, const XMBMenuEvent data)
{
    QBMiniFavoriteList self = ((SvWidget) w_)->prv;

    if (data->clicked){
        SvGenericObject list = (SvGenericObject) SvInvokeInterface(QBListModel, self->lists, getObject, data->position);
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        SvString listID = QBPlaylistManagerGetPlaylistId(playlists, list);

        if (SvInvokeInterface(QBListModel, list, getLength)>0) {
            QBPlaylistCursor cursor = QBPlaylistCursorsGet(self->appGlobals->playlistCursors, SVSTRING("TV"), listID);
            QBTVLogicPlayChannelByCursor(self->appGlobals->tvLogic, cursor, SVSTRING("MiniFavoriteList"));
        }
    }
}

QBMiniFavoriteList QBMiniFavoriteListNew(AppGlobals appGlobals)
{
    SvApplication app = appGlobals->res;
    QBMiniFavoriteList self = (QBMiniFavoriteList) SvTypeAllocateInstance(QBMiniFavoriteList_getType(), NULL);
    SvWidget window = svWidgetCreateBitmap(app, app->width, app->height, NULL);
    QBLocalWindowInit((QBLocalWindow) self, window, QBLocalWindowTypeFocusable);
    window->prv = self;
    svWidgetSetInputEventHandler(window, QBMiniFavoriteListInputEventHandler);
    window->clean = QBMiniFavoriteListClean;

    self->appGlobals = appGlobals;
    svSettingsPushComponent("tvMiniFavoriteList.settings");

    self->fadeTime = svSettingsGetDouble("localWindow", "fadeTime", 0.3);
    SvWidget bg = QBFrameCreateFromSM(app, "verticalBar.bg");
    SvWidget scrollBar = QBScrollBarNew(app, "ScrollBar");
    svSettingsWidgetAttach(bg, scrollBar, svWidgetGetName(scrollBar), 2);

    self->xmbMenu = XMBVerticalMenuNew(app, "verticalBar", 0);
    svSettingsWidgetAttach(window, self->xmbMenu, svWidgetGetName(self->xmbMenu), 10);

    XMBVerticalMenuAddRangeListener(self->xmbMenu, QBScrollBarGetRangeListener(scrollBar), NULL);
    XMBVerticalMenuSetBG(self->xmbMenu, bg);

    QBMiniFavoriteItemController itemController = (QBMiniFavoriteItemController)
            SvTypeAllocateInstance(QBMiniFavoriteItemController_getType(), NULL);
    itemController->appGlobals = appGlobals;
    itemController->icon = SVRETAIN(svSettingsGetBitmap("MenuItem", "icon"));
    itemController->focus = QBFrameConstructorFromSM("MenuItem.focus");
    itemController->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    itemController->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);
    itemController->inactiveItemConstructor = QBXMBItemConstructorCreate("InactiveMenuItem", appGlobals->textRenderer);

    SvIterator iter;
    SvArray playlists;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlistsManager = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    playlists = QBPlaylistManagerGetPlaylists(playlistsManager, NULL);

    SvGenericObject playlist;
    size_t cnt = SvArrayCount(playlists);
    SvValue isPlaylistInternal = NULL;
    size_t idx = cnt;
    while (idx-- > 0) {
        playlist = SvArrayAt(playlists, idx);
        isPlaylistInternal = (SvValue) QBPlaylistManagerGetPlaylistMetaData(playlistsManager, playlist, (SvObject) SVSTRING("isInternal"));
        SvValue rootNodeId = (SvValue) QBPlaylistManagerGetPlaylistMetaData(playlistsManager, playlist, (SvObject) SVSTRING("root_node_id"));
        if ((!rootNodeId &&
             isPlaylistInternal && SvObjectIsInstanceOf((SvObject) isPlaylistInternal, SvValue_getType()) &&
             SvValueIsBoolean(isPlaylistInternal) && SvValueGetBoolean(isPlaylistInternal))
            || SvInvokeInterface(QBListModel, playlist, getLength) == 0) {
            SvArrayRemoveObjectAtIndex(playlists, idx);
        }
    }

    QBPlaylistManagerSortPlaylists(playlistsManager, playlists);
    iter = SvArrayIterator(playlists);
    self->lists = QBActiveArrayCreate(cnt, NULL);
    while ((playlist = SvIteratorGetNext(&iter))) {
        QBActiveArrayAddObject(self->lists, (SvGenericObject) playlist, NULL);
        SvInvokeInterface(QBDataModel, playlist, addListener, (SvGenericObject) self, NULL);
    }

    XMBVerticalMenuConnectToDataSource(self->xmbMenu, (SvGenericObject) self->lists, (SvGenericObject)itemController, NULL);
    SVRELEASE(itemController);
    XMBVerticalMenuSetNotificationTarget(self->xmbMenu, window, QBMiniFavoriteListOnChangePosition);

    svWidgetSetFocusable(self->xmbMenu, true);
    svWidgetSetFocus(self->xmbMenu);

    svSettingsPopComponent();
    self->visible = false;

    return self;
}

SvLocal void
QBMiniFavoriteListSelectCurrentList(QBMiniFavoriteList self)
{
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvString currentId = QBPlaylistManagerGetCurrentId(playlists);
    size_t idx = 0;
    size_t len = SvInvokeInterface(QBListModel, self->lists, getLength);
    while (idx < len) {
        SvObject list = (SvObject) SvInvokeInterface(QBListModel, self->lists, getObject, idx);
        SvObject id = (SvObject) QBPlaylistManagerGetPlaylistId(playlists, list);
        if (SvObjectEquals((SvObject) currentId, id))
            break;
        idx++;
    }
    XMBVerticalMenuSetPosition(self->xmbMenu, idx, 1, NULL);
}

void QBMiniFavoriteListShow(QBMiniFavoriteList self)
{
    QBMiniFavoriteListSelectCurrentList(self);
    QBApplicationControllerAddLocalWindow(self->appGlobals->controller, (QBLocalWindow) self);
    if(self->fadeId)
        svAppCancelEffect(self->appGlobals->res, self->fadeId, SV_EFFECT_ABANDON);
    self->super_.window->tree_alpha = ALPHA_TRANSPARENT;
    SvEffect effect = svEffectFadeNew(self->super_.window, ALPHA_SOLID, 1, self->fadeTime, SV_EFFECT_FADE_LOGARITHM);
    self->fadeId = svAppRegisterEffect(self->appGlobals->res, effect);
    self->visible = true;
}

void QBMiniFavoriteListHide(QBMiniFavoriteList self)
{
    QBApplicationControllerRemoveLocalWindow(self->appGlobals->controller, (QBLocalWindow)self);
    self->visible = false;
}

bool QBMiniFavoriteListIsVisible(QBMiniFavoriteList self)
{
    if(!self)
        return false;
    return self->visible;
}

