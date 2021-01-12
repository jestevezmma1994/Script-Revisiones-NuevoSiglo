/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBRoutingSettings.h"
#include <ContextMenus/QBRoutingSettingsListPane.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBActiveArray.h>
#include <QBDataModel3/QBSortedList.h>
#include <Menus/menuchoice.h>
#include <QBNetworkMonitor/QBNetworkMonitorListener.h>
#include <QBNetworkMonitor/QBNetworkInterface.h>
#include <Widgets/SideMenu.h>
#include <QBMenu/QBMenu.h>
#include <libintl.h>
#include <settings.h>

typedef struct QBRoutingSettingsMenuHandler_ {
    struct SvObject_ super_;

    SvApplication res;
    QBTextRenderer textRenderer;
    QBNetworkMonitor networkMonitor;
    QBActiveTreeNode mainNode;
    QBActiveTree menuTree;
    QBApplicationController controller;

    struct QBRoutingSettingsSideMenu_ {
        QBContextMenu ctx;
    } sidemenu;

    QBSortedList availableInterfaces;
    SvHashTable idToInterface;
    QBActiveArray availInterfacesArray;

    int settingsCtx;

} *QBRoutingSettingsMenuHandler;

SvLocal void QBRoutingSettingsMenuHandlerDestroy(void *ptr)
{
    QBRoutingSettingsMenuHandler self = (QBRoutingSettingsMenuHandler) ptr;
    SVTESTRELEASE(self->sidemenu.ctx);
    SVRELEASE(self->availInterfacesArray);

    SVRELEASE(self->availableInterfaces);
    SVRELEASE(self->idToInterface);
}

SvLocal void QBRoutingSettingsMenuHandlerMenuChosen(SvObject self_, SvObject node_, SvObject nodePath_, int position)
{
    QBRoutingSettingsMenuHandler self = (QBRoutingSettingsMenuHandler) self_;
    SvObject options = NULL;

    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        options = (SvObject) QBRoutingSettingsListPaneCreateFromSettings("RoutingSettingsPane.settings",
                             self->res,
                             self->textRenderer,
                             self->networkMonitor,
                             self->availInterfacesArray,
                             self->sidemenu.ctx,
                             NULL);
    }

    if (options) {
        QBContextMenuShow(self->sidemenu.ctx);
        QBContextMenuPushPane(self->sidemenu.ctx, options);
        SVRELEASE(options);
    }
}

SvLocal void QBRoutingSettingsMenuHandlerInterfaceAdded(SvObject self_, QBNetworkInterface interface)
{
    QBRoutingSettingsMenuHandler self = (QBRoutingSettingsMenuHandler) self_;

    QBActiveTreeNode node = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("iface"), (SvObject) interface);
    QBActiveArrayAddObject(self->availInterfacesArray, (SvObject) node, NULL);
    SVRELEASE(node);
    QBSortedListInsert(self->availableInterfaces, (SvObject) interface);
    SvHashTableInsert(self->idToInterface, (SvObject) QBNetworkInterfaceGetID(interface), (SvObject) interface);

}

SvLocal void QBRoutingSettingsMenuHandlerInterfaceRemoved(SvObject self_, SvString interfaceID)
{
    QBRoutingSettingsMenuHandler self = (QBRoutingSettingsMenuHandler) self_;

    SvObject node;
    QBNetworkInterface iface;
    int length = QBActiveArrayCount(self->availInterfacesArray);
    for (int i = 0; i < length; i++) {
        node = QBActiveArrayObjectAtIndex(self->availInterfacesArray, i);
        iface = (QBNetworkInterface) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node, SVSTRING("iface"));
        if (SvObjectEquals((SvObject) interfaceID, ((SvObject) QBNetworkInterfaceGetID(iface)))) {
            QBActiveArrayRemoveObjectAtIndex(self->availInterfacesArray, i, NULL);
            break;
        }
    }
    SvObject interface = SvHashTableFind(self->idToInterface, (SvObject) interfaceID);
    if (interface) {
        QBSortedListRemove(self->availableInterfaces, interface);
        SvHashTableRemove(self->idToInterface, (SvObject) interfaceID);
    }
}

SvLocal void QBRoutingSettingsMenuHandlerInterfaceStateChanged(SvObject self_, SvString interfaceID)
{
    // not needed
}

SvLocal void QBRoutingSettingsMenuHandlerVPNStateChanged(SvObject self_)
{
    // not needed
}
SvLocal int QBRoutingSettingsListPaneGatewayTenureCompareFn(void *prv, SvObject firstInterface_, SvObject secondInterface_)
{
    QBNetworkInterface firstInterface = (QBNetworkInterface) firstInterface_;
    QBNetworkInterface secondInterface = (QBNetworkInterface) secondInterface_;

    return QBNetworkInterfaceGetGatewayTenure(firstInterface) < QBNetworkInterfaceGetGatewayTenure(secondInterface);
}

SvLocal void QBRoutingSettingsMenuHandlerContextMenuChosen(SvObject self_, SvObject nodePath)
{
    QBRoutingSettingsMenuHandler self = (QBRoutingSettingsMenuHandler) self_;
    SvObject node_ = SvInvokeInterface(QBTreeModel, self->menuTree, getNode, nodePath);
    SvObject options = NULL;

    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        options = (SvObject) QBRoutingSettingsListPaneCreateFromSettings("FavoritesEditorPane.settings",
                             self->res,
                             self->textRenderer,
                             self->networkMonitor,
                             self->availInterfacesArray,
                             self->sidemenu.ctx,
                             NULL);
    }

    if (options) {
        QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) options);
        QBContextMenuShow(self->sidemenu.ctx);
        SVRELEASE(options);
    }
}

SvLocal SvType QBRoutingSettingsMenuHandler_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vTable = {
        .destroy = QBRoutingSettingsMenuHandlerDestroy
    };

    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBRoutingSettingsMenuHandlerMenuChosen
    };

    static const struct QBMenuContextChoice_t contextMethods = {
        .contextChoosen = QBRoutingSettingsMenuHandlerContextMenuChosen
    };

    static const struct QBNetworkMonitorListener_ networkMonitorMethods = {
        .interfaceAdded        = QBRoutingSettingsMenuHandlerInterfaceAdded,
        .interfaceRemoved      = QBRoutingSettingsMenuHandlerInterfaceRemoved,
        .interfaceStateChanged = QBRoutingSettingsMenuHandlerInterfaceStateChanged,
        .ipSecVPNStateChanged  = QBRoutingSettingsMenuHandlerVPNStateChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRoutingSettingsMenuHandler",
                            sizeof(struct QBRoutingSettingsMenuHandler_),
                            SvObject_getType(), &type,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBMenuContextChoice_getInterface(), &contextMethods,
                            QBNetworkMonitorListener_getInterface(), &networkMonitorMethods,
                            SvObject_getType(), &vTable,
                            NULL);
    }
    return type;
}

void QBRoutingSettingsMenuRegister(SvWidget menuBar, QBTreePathMap pathMap,
                                   SvApplication res, QBActiveTree menuTree,
                                   QBTextRenderer textRenderer,
                                   QBNetworkMonitor networkMonitor,
                                   QBApplicationController controller)
{
    QBActiveTreeNode mainNode = QBActiveTreeFindNode(menuTree, SVSTRING("RoutingSettings"));
    SvObject path = mainNode ? QBActiveTreeCreateNodePath(menuTree, QBActiveTreeNodeGetID(mainNode)) : NULL;
    if (!mainNode || !path)
        return;

    QBRoutingSettingsMenuHandler self = (QBRoutingSettingsMenuHandler)
            SvTypeAllocateInstance(QBRoutingSettingsMenuHandler_getType(), NULL);

    self->res = res;
    self->textRenderer = textRenderer;
    self->networkMonitor = networkMonitor;
    self->mainNode = mainNode;
    self->menuTree = menuTree;
    self->controller = controller;
    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", controller, res, SVSTRING("SideMenu"));
    QBTreePathMapInsert(pathMap, path, (SvObject) self, NULL);
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) self, NULL);

    self->availableInterfaces = QBSortedListCreateWithCompareFn(QBRoutingSettingsListPaneGatewayTenureCompareFn, self, NULL);
    self->idToInterface = SvHashTableCreate(3, NULL);
    self->availInterfacesArray = QBActiveArrayCreate(3, NULL);
    QBNetworkMonitorAddListener(networkMonitor, (SvObject) self);

    SVRELEASE(path);
    SVRELEASE(self);
}
