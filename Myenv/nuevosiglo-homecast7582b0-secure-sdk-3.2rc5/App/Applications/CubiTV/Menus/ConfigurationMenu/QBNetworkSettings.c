/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBNetworkSettings.h"
#include <QBSecureLogManager.h>
#include <ContextMenus/QBNetworkSettingsAdvancedPane.h>
#include <ContextMenus/wifi/QBWiFiManualSettingsPane.h>
#include <ContextMenus/wifi/QBWiFiNetworkListPane.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <QBWidgets/QBDialog.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBSortedList.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <Menus/menuchoice.h>
#include <QBNetworkMonitor/QBNetworkMonitorListener.h>
#include <QBNetworkMonitor/QBWiFiNetwork.h>
#include <QBNetworkMonitor/QBNetworkInterface.h>
#include <Widgets/SideMenu.h>
#include <Widgets/QBSimpleDialog.h>
#include <Hints/hintsGetter.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <QBOSK/QBOSKKey.h>
#include <Utils/QBNetworkSettingsUtils.h>
#include <main.h>
#include <regex.h>
#include <libintl.h>
#include <settings.h>
#include <QBJSONUtils.h>


typedef struct QBNetworkSettingsMenuHandler_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBXMBItemConstructor itemConstructor;
    QBActiveTreeNode mainNode;
    QBFrameDescription* focus;
    QBFrameDescription* inactiveFocus;
    QBFrameDescription* disabledFocus;
    SvBitmap icon;

    struct QBNetworkSettingsSideMenu_ {
        QBContextMenu ctx;
    } sidemenu;

    QBSortedList availableInterfaces;
    SvHashTable idToInterface;

    QBNetworkInterface currentInterface;

    SvWidget dialog;

    int settingsCtx;

} *QBNetworkSettingsMenuHandler;


SvLocal SvObject QBNetworkSettingsMenuHandlerCreateMoCaSideMenuPane(QBNetworkSettingsMenuHandler self, int level)
{
    return (SvObject) QBNetworkSettingsAdvancedPaneCreate(self->appGlobals->res,
                                                          self->appGlobals->textRenderer,
                                                          self->appGlobals->networkMonitor,
                                                          self->currentInterface,
                                                          self->sidemenu.ctx,
                                                          level);
}

SvLocal void QBNetworkSettingsMenuHandlerMenuChosen(SvObject self_, SvObject node_, SvObject nodePath_, int position)
{
    QBNetworkSettingsMenuHandler self = (QBNetworkSettingsMenuHandler) self_;
    SvObject options = NULL;

    if (!SvObjectIsInstanceOf(node_, QBNetworkInterface_getType())) {
        return;
    }

    SVTESTRELEASE(self->currentInterface);
    self->currentInterface = SVRETAIN(node_);
    svSettingsRestoreContext(self->settingsCtx);

    if (SvObjectIsInstanceOf(node_, QBEthernetInterface_getType())) {
        if (QBNetworkInterfaceGetStatus((QBNetworkInterface) node_) < QBNetworkManagerInterfaceStatus_obtainingLink)
            return;
            options = (SvObject) QBNetworkSettingsAdvancedPaneCreate(self->appGlobals->res,
                                                                     self->appGlobals->textRenderer,
                                                                     self->appGlobals->networkMonitor,
                                                                     self->currentInterface,
                                                                     self->sidemenu.ctx,
                                                                     1);
    } else if (SvObjectIsInstanceOf(node_, QBWiFiInterface_getType())) {
        QBNetworkMonitorRequestWiFiScanning(self->appGlobals->networkMonitor, QBNetworkInterfaceGetID(self->currentInterface));
        options = (SvObject) QBWiFiNetworkListPaneCreate(self->appGlobals->res,
                                                         self->appGlobals->scheduler,
                                                         self->appGlobals->textRenderer,
                                                         self->appGlobals->controller,
                                                         self->appGlobals->networkMonitor,
                                                         (QBWiFiInterface) self->currentInterface,
                                                         self->sidemenu.ctx,
                                                         1);
    } else if (SvObjectIsInstanceOf(node_, QBMoCaInterface_getType())) {
        if (QBNetworkInterfaceGetStatus((QBNetworkInterface) node_) < QBNetworkManagerInterfaceStatus_obtainingLink)
            return;
            options = QBNetworkSettingsMenuHandlerCreateMoCaSideMenuPane(self, 1);
    } else if (SvObjectIsInstanceOf(node_, QBDocsisInterface_getType())) {
        // TODO implement me
    } else if (SvObjectIsInstanceOf(node_, QB3GInterface_getType())) {
        // TODO implement me
    } else {
        SvLogError("Unhandled interface type [%s]", SvTypeGetName(SvObjectGetType(node_)));
    }

    svSettingsPopComponent();

    if (options) {
        QBContextMenuShow(self->sidemenu.ctx);
        QBContextMenuPushPane(self->sidemenu.ctx, options);
        SVRELEASE(options);
    }
}

SvLocal void QBNetworkSettingsMenuHandlerContextMenuChosen(SvObject self_, SvObject nodePath)
{
    QBNetworkSettingsMenuHandler self = (QBNetworkSettingsMenuHandler) self_;
    SvObject node_ = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath);
    QBNetworkInterface interface = (QBNetworkInterface) node_;

    SvObject options = NULL;

    SVTESTRELEASE(self->currentInterface);
    self->currentInterface = SVRETAIN(node_);

    svSettingsRestoreContext(self->settingsCtx);

    if (SvObjectIsInstanceOf(node_, QBEthernetInterface_getType())) {
        if (QBNetworkInterfaceGetStatus(interface) >= QBNetworkManagerInterfaceStatus_obtainingLink)
            options = (SvObject) QBNetworkSettingsAdvancedPaneCreate(self->appGlobals->res,
                                                                     self->appGlobals->textRenderer,
                                                                     self->appGlobals->networkMonitor,
                                                                     self->currentInterface,
                                                                     self->sidemenu.ctx,
                                                                     1);
    } else if (SvObjectIsInstanceOf(node_, QBWiFiInterface_getType())) {
        options = (SvObject) QBWiFiManualSettingsPaneCreate(self->appGlobals->res,
                                                            self->appGlobals->scheduler,
                                                            self->appGlobals->textRenderer,
                                                            self->appGlobals->controller,
                                                            self->appGlobals->networkMonitor,
                                                            (QBWiFiInterface) self->currentInterface,
                                                            self->sidemenu.ctx,
                                                            1);
    } else if (SvObjectIsInstanceOf(node_, QBMoCaInterface_getType())) {
        if (QBNetworkInterfaceGetStatus((QBNetworkInterface) node_) >= QBNetworkManagerInterfaceStatus_obtainingLink) {
            options = QBNetworkSettingsMenuHandlerCreateMoCaSideMenuPane(self, 1);
        }
    }

    svSettingsPopComponent();

    if (options) {
        QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) options);
        QBContextMenuShow(self->sidemenu.ctx);
        SVRELEASE(options);
    }
}

SvLocal SvWidget QBNetworkSettingsMenuHandlerCreateItem(SvObject self_, SvObject node_, SvObject path, SvApplication app, XMBMenuState initialState)
{
    QBNetworkSettingsMenuHandler self = (QBNetworkSettingsMenuHandler) self_;
    QBXMBItem item = NULL;

    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        QBActiveTreeNode node = (QBActiveTreeNode) node_;
        item = QBXMBItemCreate();
        item->caption = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
        item->subcaption = SVTESTRETAIN(QBActiveTreeNodeGetAttribute(node, SVSTRING("subcaption")));
        item->focus = SVRETAIN(self->focus);
        item->disabledFocus = SVTESTRETAIN(self->disabledFocus);
        item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
        item->icon = SVRETAIN(self->icon);
    } else if (SvObjectIsInstanceOf(node_, QBNetworkInterface_getType())) {
        item = QBXMBItemCreate();
        QBNetworkInterface iface = (QBNetworkInterface) node_;
        item->caption = QBNetworkSettingsUtilsCreateInterfaceName(iface);
        item->focus = SVRETAIN(self->focus);
        item->disabledFocus = SVTESTRETAIN(self->disabledFocus);
        item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
        item->icon = SVRETAIN(self->icon);
        item->subcaption = QBNetworkSettingsUtilsCreateInterfaceConnectionDescription(iface);
    }

    if (!item)
        return NULL;

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);
    return ret;
}

SvLocal void QBNetworkSettingsMenuHandlerSetItemState(SvObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBNetworkSettingsMenuHandler self = (QBNetworkSettingsMenuHandler) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBNetworkSettingsMenuHandlerInterfaceAdded(SvObject self_, QBNetworkInterface interface)
{
    QBNetworkSettingsMenuHandler self = (QBNetworkSettingsMenuHandler) self_;

    QBSortedListInsert(self->availableInterfaces, (SvObject) interface);
    SvHashTableInsert(self->idToInterface, (SvObject) QBNetworkInterfaceGetID(interface), (SvObject) interface);
}

SvLocal void QBNetworkSettingsMenuHandlerInterfaceRemoved(SvObject self_, SvString interfaceID)
{
    QBNetworkSettingsMenuHandler self = (QBNetworkSettingsMenuHandler) self_;

    SvObject interface = SvHashTableFind(self->idToInterface, (SvObject) interfaceID);
    if (interface) {
        if ((SvObject) self->currentInterface == interface)
            QBContextMenuHide(self->sidemenu.ctx, false);
        QBSortedListRemove(self->availableInterfaces, interface);
        SvHashTableRemove(self->idToInterface, (SvObject) interfaceID);
    }
}

SvLocal void
QBNetworkSettingsMenuHandlerErrorDialogShow(AppGlobals appGlobals, int settingsCtx, const char *title, const char *message)
{
    svSettingsRestoreContext(settingsCtx);
    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0,
    };
    SvWidget dialog = QBSimpleDialogCreate(&params, title, message);
    QBDialogRun(dialog, NULL, NULL);
    char *escapedTitle = QBStringCreateJSONEscapedString(title);
    char *escapedMsg = QBStringCreateJSONEscapedString(message);
    QBSecureLogEvent("NetworkSettings", "ErrorShown.ConfigurationMenu.NetworkSettings",
                     "JSON:{\"description\":\"%s: %s\"}", escapedTitle, escapedMsg);
    free(escapedTitle);
    free(escapedMsg);
    svSettingsPopComponent();
}

SvLocal void QBNetworkSettingsMenuHandlerInterfaceStateChanged(SvObject self_, SvString interfaceID)
{
    QBNetworkSettingsMenuHandler self = (QBNetworkSettingsMenuHandler) self_;
    SvObject interface_ = SvHashTableFind(self->idToInterface, (SvObject) interfaceID);

    SvIterator iter = QBSortedListIterator(self->availableInterfaces);
    SvObject object;
    int idx = 0;
    while ((object = SvIteratorGetNext(&iter))) {
        if (object == interface_) {
            QBNetworkInterface interface = (QBNetworkInterface) interface_;

            if (QBNetworkInterfaceGetStatus(interface) == QBNetworkManagerInterfaceStatus_connected &&
                QBNetworkInterfaceGetConfig(interface) != QBNetworkManagerInterfaceConfig_normal) {
                SvString title = SvStringCreateWithFormat(gettext("Incorrect network settings (%s)"), SvStringCString(interfaceID));
                char *message = gettext("Restored last working network configuration.");
                QBNetworkSettingsMenuHandlerErrorDialogShow(self->appGlobals, self->settingsCtx, SvStringCString(title), message);
                SVRELEASE(title);
            }
            QBListDataSourceNotifyItemsChanged((QBListDataSource) self->availableInterfaces, idx, 1);
            break;
        }
        idx++;
    }
}

SvLocal void QBNetworkMonitorMenuHandlerVPNStateChanged(SvObject self_)
{
    // not implemented yet
}

SvLocal SvString QBNetworkSettingsMenuHandlerGetHintsForPath(SvObject self_, SvObject path)
{
    return SVSTRING("networkInterfaceSetup_hint");
}

SvLocal void QBNetworkSettingsMenuHandlerDestroy(void *self_)
{
    QBNetworkSettingsMenuHandler self = (QBNetworkSettingsMenuHandler) self_;
    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVTESTRELEASE(self->sidemenu.ctx);
    SVRELEASE(self->availableInterfaces);
    SVRELEASE(self->idToInterface);
    SVTESTRELEASE(self->currentInterface);
    SVRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVTESTRELEASE(self->disabledFocus);
    SVRELEASE(self->icon);
}

SvLocal SvType QBNetworkSettingsMenuHandler_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBNetworkSettingsMenuHandlerDestroy
    };

    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBNetworkSettingsMenuHandlerMenuChosen
    };

    static const struct QBMenuContextChoice_t contextMethods = {
        .contextChoosen = QBNetworkSettingsMenuHandlerContextMenuChosen
    };

    static const struct XMBItemController_t methods = {
        .createItem = QBNetworkSettingsMenuHandlerCreateItem,
        .setItemState = QBNetworkSettingsMenuHandlerSetItemState
    };

    static const struct QBNetworkMonitorListener_ networkMonitorMethods = {
        .interfaceAdded = QBNetworkSettingsMenuHandlerInterfaceAdded,
        .interfaceRemoved = QBNetworkSettingsMenuHandlerInterfaceRemoved,
        .interfaceStateChanged = QBNetworkSettingsMenuHandlerInterfaceStateChanged,
        .ipSecVPNStateChanged = QBNetworkMonitorMenuHandlerVPNStateChanged,
    };

    static const struct QBHintsGetter_t hintsMethods = {
        .getHintsForPath = QBNetworkSettingsMenuHandlerGetHintsForPath
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNetworkSettingsMenuHandler",
                            sizeof(struct QBNetworkSettingsMenuHandler_),
                            SvObject_getType(), &type,
                            XMBItemController_getInterface(), &methods,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBMenuContextChoice_getInterface(), &contextMethods,
                            QBNetworkMonitorListener_getInterface(), &networkMonitorMethods,
                            QBHintsGetter_getInterface(), &hintsMethods,
                            SvObject_getType(), &vtable,
                            NULL);
    }

    return type;
}

SvLocal int QBNetworkSettingsMenuHandlerNetworkNameCompareFn(void *prv, SvObject firstInterface_, SvObject secondInterface_)
{
    QBNetworkInterface firstInterface = (QBNetworkInterface) firstInterface_;
    QBNetworkInterface secondInterface = (QBNetworkInterface) secondInterface_;

    return strcmp(SvStringCString(QBNetworkInterfaceGetID(firstInterface)), SvStringCString(QBNetworkInterfaceGetID(secondInterface)));
}

void QBNewNetworkSettingsMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    QBActiveTreeNode mainNode = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("NewNetworkSettings"));
    SvObject path = mainNode ? QBActiveTreeCreateNodePath(appGlobals->menuTree, QBActiveTreeNodeGetID(mainNode)) : NULL;
    if (!mainNode || !path)
        return;

    QBNetworkSettingsMenuHandler self = (QBNetworkSettingsMenuHandler)
            SvTypeAllocateInstance(QBNetworkSettingsMenuHandler_getType(), NULL);

    self->appGlobals = appGlobals;
    self->mainNode = mainNode;
    svSettingsPushComponent("NetworkSettings.settings");
    self->settingsCtx = svSettingsSaveContext();
    self->icon = SVRETAIN(svSettingsGetBitmap("MenuItem", "icon"));
    self->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);
    self->focus = QBFrameConstructorFromSM("MenuItem.focus");

    const char *inactiveFocusWidgetName = "MenuItem.inactiveFocus";
    if (svSettingsIsWidgetDefined(inactiveFocusWidgetName)) {
        self->inactiveFocus = QBFrameConstructorFromSM(inactiveFocusWidgetName);
    }

    const char *disabledFocusWidgetName = "MenuItem.disabledFocus";
    if (svSettingsIsWidgetDefined(disabledFocusWidgetName)) {
        self->disabledFocus = QBFrameConstructorFromSM(disabledFocusWidgetName);
    }
    svSettingsPopComponent();

    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));

    QBTreePathMapInsert(pathMap, path, (SvObject) self, NULL);

    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) self, NULL);

    self->availableInterfaces = QBSortedListCreateWithCompareFn(QBNetworkSettingsMenuHandlerNetworkNameCompareFn, self, NULL);
    self->idToInterface = SvHashTableCreate(3, NULL);
    QBTreeProxy availableInterfacesProxy = QBTreeProxyCreate((SvObject) self->availableInterfaces, (SvObject) self->mainNode, NULL);
    QBActiveTreeMountSubTree(self->appGlobals->menuTree, (SvObject) availableInterfacesProxy, path, NULL);
    SVRELEASE(availableInterfacesProxy);

    QBNetworkMonitorAddListener(appGlobals->networkMonitor, (SvObject) self);
    QBNetworkMonitorRecallInterfaces(appGlobals->networkMonitor, (SvObject) self);

    SVRELEASE(path);
    SVRELEASE(self);
}
