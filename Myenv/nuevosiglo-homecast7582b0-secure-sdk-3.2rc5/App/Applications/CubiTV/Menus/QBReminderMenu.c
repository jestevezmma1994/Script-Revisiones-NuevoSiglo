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

#include "QBReminderMenu.h"

#include <libintl.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvArray.h>
#include <settings.h>
#include <QBDataModel3/QBTreeModel.h>
#include <ContextMenus/QBContextMenu.h>
#include <Menus/menuchoice.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <SWL/QBFrame.h>
#include <ContextMenus/QBBasicPane.h>
#include <Logic/timeFormat.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBConf.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <main.h>
#include <Hints/hintsGetter.h>

struct QBReminderMenu_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    struct QBPVRMenuHandlerSideMenu_t {
        QBContextMenu ctx;
    } sidemenu;
    QBReminderTask currentTask;
};

typedef struct QBReminderMenu_t *QBReminderMenu;

SvLocal void
QBReminderMenuDestroy(void *self_)
{
    QBReminderMenu self = (QBReminderMenu) self_;
    SVTESTRELEASE(self->sidemenu.ctx);
    SVTESTRELEASE(self->currentTask);
}

SvLocal void
QBReminderMenuContextMenuHide(QBReminderMenu self)
{
    QBContextMenuHide(self->sidemenu.ctx, false);
    SVTESTRELEASE(self->currentTask);
    self->currentTask = NULL;
}

SvLocal void
QBReminderMenuContextMenuDelete(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderMenu self = self_;
    if (SvStringEqualToCString(id, "OK")) {
        QBReminderRemoveByTask(self->appGlobals->reminder, self->currentTask);
    }
    QBReminderMenuContextMenuHide(self);
}

SvLocal void
QBReminderMenuContextMenuDeleteAll(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderMenu self = self_;
    if (SvStringEqualToCString(id, "OK")) {
        QBReminderRemoveAll(self->appGlobals->reminder);
    }
    QBReminderMenuContextMenuHide(self);
}

SvLocal void
QBReminderMenuChoosen(SvGenericObject self_, SvGenericObject node, SvGenericObject nodePath, int position)
{
    QBReminderMenu self = (QBReminderMenu) self_;
    QBReminderTask task = NULL;

    if (SvObjectIsInstanceOf(node, QBReminderTask_getType())) {
        task = (QBReminderTask) node;

        svSettingsPushComponent("BasicPane.settings");
        QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
        QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 1, SVSTRING("BasicPane"));

        SvArray confirmation = SvArrayCreate(NULL);
        SvArray ids = SvArrayCreate(NULL);
        SvString option = SvStringCreate(gettext("OK"), NULL);
        SvArrayAddObject(confirmation, (SvGenericObject) option);
        SvArrayAddObject(ids, (SvGenericObject) SVSTRING("OK"));
        SVRELEASE(option);

        option = SvStringCreate(gettext("Cancel"), NULL);
        SvArrayAddObject(confirmation, (SvGenericObject) option);
        SvArrayAddObject(ids, (SvGenericObject) SVSTRING("CANCEL"));
        SVRELEASE(option);

        option = SvStringCreate(gettext("Delete"), NULL);
        QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("delete"), option, SVSTRING("BasicPane"), confirmation, ids, QBReminderMenuContextMenuDelete, self);
        SVRELEASE(option);
        option = SvStringCreate(gettext("Delete all"), NULL);
        QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("delete all"), option, SVSTRING("BasicPane"), confirmation, ids, QBReminderMenuContextMenuDeleteAll, self);
        SVRELEASE(option);

        svSettingsPopComponent();
        SVTESTRELEASE(self->currentTask);
        self->currentTask = SVRETAIN(task);

        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
        QBContextMenuShow(self->sidemenu.ctx);

        SVRELEASE(options);
        SVRELEASE(ids);
        SVRELEASE(confirmation);
    }
}

SvLocal void
QBReminderMenuContextChoosen(SvGenericObject self_, SvGenericObject nodePath)
{
    QBReminderMenu self = (QBReminderMenu) self_;
    SvGenericObject node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath);
    QBReminderMenuChoosen(self_, node, nodePath, -1);
}

SvLocal SvString
QBReminderGetHintsForPath(SvGenericObject self_, SvGenericObject path)
{
    QBReminderMenu self = (QBReminderMenu) self_;
    QBActiveTreeNode currentNode = (QBActiveTreeNode) SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, path);
    if (currentNode && SvObjectIsInstanceOf((SvObject) currentNode, QBActiveTreeNode_getType())) {
        return SVSTRING("tv_hint");
    } else if (currentNode) {
        return SVSTRING("reminders_hint");
    } else {
        return NULL;
    }
}

SvLocal SvType
QBReminderMenu_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBReminderMenuDestroy
    };
    static SvType type = NULL;
    static const struct QBMenuContextChoice_t menuContextChoiceMethods = {
        .contextChoosen = QBReminderMenuContextChoosen
    };
    static const struct QBMenuChoice_t menuChoiceMethods = {
        .choosen = QBReminderMenuChoosen
    };
    static const struct QBHintsGetter_t hintsMethods = {
        .getHintsForPath = QBReminderGetHintsForPath
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBReminderMenu",
                            sizeof(struct QBReminderMenu_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuContextChoice_getInterface(), &menuContextChoiceMethods,
                            QBMenuChoice_getInterface(), &menuChoiceMethods,
                            QBHintsGetter_getInterface(), &hintsMethods,
                            NULL);
    }

    return type;
}

SvLocal QBReminderMenu
QBReminderMenuCreate(AppGlobals appGlobals)
{
    QBReminderMenu self = (QBReminderMenu) SvTypeAllocateInstance(QBReminderMenu_getType(), NULL);

    self->appGlobals = appGlobals;
    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));

    return self;
}

struct QBReminderMenuItemController_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    QBXMBItemConstructor itemConstructor;
    SvBitmap icon;
};

typedef struct QBReminderMenuItemController_t *QBReminderMenuItemController;

SvLocal void
QBReminderMenuItemControllerDestroy(void *self_)
{
    QBReminderMenuItemController self = (QBReminderMenuItemController) self_;
    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVTESTRELEASE(self->icon);
    QBXMBItemConstructorDestroy(self->itemConstructor);
}

SvLocal SvWidget
QBReminderMenuItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBReminderMenuItemController self = (QBReminderMenuItemController) self_;
    SvString caption = SVSTRING("???");
    SvString subcaption = NULL;
    SvBitmap icon = NULL;

    if (SvObjectIsInstanceOf(node_, QBReminderTask_getType())) {
        QBReminderTask task = (QBReminderTask) node_;
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        SvGenericObject playlist = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
        SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, playlist, getByID, task->channelID);
        if (channel)
            caption = SVTESTRETAIN(channel->name);
#define MAX_DATE_LENGHT 128
        char buf[MAX_DATE_LENGHT];
        struct tm startTime;
        SvTimeBreakDown(SvTimeConstruct(task->startTime, 0), true, &startTime);
        strftime(buf, sizeof(buf), QBTimeFormatGetCurrent()->reminderTime, &startTime);
        subcaption = SvStringCreate(buf, NULL);
        icon = SVTESTRETAIN(self->icon);
    }

    QBXMBItem item = QBXMBItemCreate();
    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    item->caption = caption;
    item->subcaption = subcaption;
    item->icon = icon;

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);
    return ret;
}

SvLocal void
QBReminderMenuItemControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBReminderMenuItemController self = (QBReminderMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBReminderMenuItemControllerItemsChanged(SvGenericObject self_, size_t first, size_t count)
{
    QBReminderMenuItemController self = (QBReminderMenuItemController) self_;

    QBActiveTreeNode node = QBActiveTreeFindNode(self->appGlobals->menuTree, SVSTRING("reminders"));
    int remindersCount = SvInvokeInterface(QBListModel, QBReminderAgentGetTasks(self->appGlobals->reminderAgent), getLength);
    SvString subcaption = SvStringCreateWithFormat(ngettext("%i reminder", "%i reminders", remindersCount), remindersCount);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvGenericObject) subcaption);
    SVRELEASE(subcaption);

    QBActiveTreePropagateNodeChange(self->appGlobals->menuTree, node, NULL);
}

SvLocal void
QBReminderMenuItemControllerLangChanged(SvGenericObject self_, const char *key, const char *value)
{
    QBReminderMenuItemControllerItemsChanged(self_, 0, 0);
}

SvLocal SvType
QBReminderMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBReminderMenuItemControllerDestroy
    };
    static SvType type = NULL;
    static const struct XMBItemController_t XMBItemControllerMethods = {
        .createItem   = QBReminderMenuItemControllerCreateItem,
        .setItemState = QBReminderMenuItemControllerSetItemState
    };
    static const struct QBListModelListener_t listListenerMethods = {
        .itemsAdded     = QBReminderMenuItemControllerItemsChanged,
        .itemsRemoved   = QBReminderMenuItemControllerItemsChanged,
        .itemsChanged   = QBReminderMenuItemControllerItemsChanged,
        .itemsReordered = QBReminderMenuItemControllerItemsChanged
    };
    static const struct QBConfigListener_t config_methods = {
        .changed = QBReminderMenuItemControllerLangChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBReminderMenuItemController",
                            sizeof(struct QBReminderMenuItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &XMBItemControllerMethods,
                            QBListModelListener_getInterface(), &listListenerMethods,
                            QBConfigListener_getInterface(), &config_methods,
                            NULL);
    }

    return type;
}

SvLocal QBReminderMenuItemController
QBReminderMenuItemControllerCreate(AppGlobals appGlobals)
{
    QBReminderMenuItemController self = (QBReminderMenuItemController) SvTypeAllocateInstance(QBReminderMenuItemController_getType(), NULL);

    self->appGlobals = appGlobals;

    self->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);
    self->icon = SVTESTRETAIN(svSettingsGetBitmap("MenuItem", "icon"));
    self->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        self->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    return self;
}

void
QBReminderMenuRegister(SvWidget menuBar,
                       QBTreePathMap pathMap,
                       AppGlobals appGlobals,
                       SvString rootID)
{
    QBActiveTreeNode node;

    if (!appGlobals->reminder) {
        return;
    }

    // find path where TV sub-tree is mounted
    SvObject origPath = QBActiveTreeCreateNodePath(appGlobals->menuTree, rootID);
    if (!origPath)
        return;

    // find path of "remainders" node in TV menu
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, appGlobals->menuTree, getIterator, origPath, 0);
    SvObject path = NULL;
    do {
        path = QBTreeIteratorGetCurrentNodePath(&iter);
        node = (QBActiveTreeNode) QBTreeIteratorGetNextNode(&iter);
        if (!node || !SvObjectIsInstanceOf((SvObject) node, QBActiveTreeNode_getType()))
            break;
        SvString nodeID = QBActiveTreeNodeGetID((QBActiveTreeNode) node);
        if (SvStringEqualToCString(nodeID, "reminders"))
            break;
    } while (node);

    if (node && path) {
        SvGenericObject tasks = QBReminderAgentGetTasks(appGlobals->reminderAgent);
        QBTreeProxy proxy = QBTreeProxyCreate((SvGenericObject) tasks, NULL, NULL);
        QBActiveTreeMountSubTree(appGlobals->menuTree, (SvGenericObject) proxy, path, NULL);
        SVRELEASE(proxy);
    }

    if (node && path) {
        svSettingsPushComponent("Pvr.settings");
        QBReminderMenu menuHandler = QBReminderMenuCreate(appGlobals);
        QBTreePathMapInsert(pathMap, path, (SvGenericObject) menuHandler, NULL);
        SVRELEASE(menuHandler);
        QBReminderMenuItemController itemController = QBReminderMenuItemControllerCreate(appGlobals);
        QBConfigAddListener((SvGenericObject) itemController, "LANG");
        SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) itemController, NULL);
        SvInvokeInterface(QBDataModel, QBReminderAgentGetTasks(appGlobals->reminderAgent),
                          addListener, (SvGenericObject) itemController, NULL);
        SVRELEASE(itemController);
        svSettingsPopComponent();
        QBReminderMenuItemControllerItemsChanged((SvGenericObject) itemController, 0, 0);
    }

    SVRELEASE(origPath);
}

