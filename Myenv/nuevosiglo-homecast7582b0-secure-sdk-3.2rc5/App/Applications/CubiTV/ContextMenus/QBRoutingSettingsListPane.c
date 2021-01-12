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

#include "QBRoutingSettingsListPane.h"
#include <QBDataModel3/QBActiveTreeNode.h>
#include <ContextMenus/QBListPane.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBVerticalMenu.h>
#include <SWL/QBFrame.h>
#include <CUIT/Core/event.h>
#include <libintl.h>
#include <settings.h>

struct QBRoutingSettingsListPane_ {
    struct SvObject_ super_;

    SvApplication res;
    QBTextRenderer textRenderer;
    QBNetworkMonitor networkMonitor;

    int settingsCtx;
    QBContextMenu ctxMenu;
    QBActiveArray sortedDataSource;

    QBListPane listPane;
};

struct QBRoutingSortItemConstructor_ {
    struct SvObject_ super_;

    // applying standard XMB interface
    QBXMBItemConstructor itemConstructor;
    QBFrameConstructor* normalFocus;
    QBFrameConstructor* selectedFocus;
};

SvLocal SvWidget QBRoutingSortItemConstructorCreateItem(SvObject self_,
                                                        SvObject node_,
                                                        SvObject path,
                                                        SvApplication app,
                                                        XMBMenuState initialState)
{
    QBRoutingSortItemConstructor self = (QBRoutingSortItemConstructor) self_;

    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    QBNetworkInterface iface = (QBNetworkInterface) QBActiveTreeNodeGetAttribute(node, SVSTRING("iface"));
    QBXMBItem item = QBXMBItemCreate();
    item->caption = SvStringCreateWithFormat(gettext("Interface: %s"), SvStringGetCString(QBNetworkInterfaceGetID(iface)));
    item->focus = SVRETAIN(self->normalFocus);
    item->sortingFocus = SVRETAIN(self->selectedFocus);

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);

    SVRELEASE(item);
    return ret;
}

SvLocal void QBRoutingSortItemConstructorSetItemState(SvObject self_,
                                                      SvWidget item_,
                                                      XMBMenuState state,
                                                      bool isFocused)
{
    QBRoutingSortItemConstructor self = (QBRoutingSortItemConstructor) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBRoutingSortItemConstructorDestroy(void *self_)
{
    QBRoutingSortItemConstructor self = self_;

    SVRELEASE(self->normalFocus);
    SVRELEASE(self->selectedFocus);

    if (self->itemConstructor)
        QBXMBItemConstructorDestroy(self->itemConstructor);
}

SvLocal SvType QBRoutingSortItemConstructor_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRoutingSortItemConstructorDestroy
    };

    static SvType type = NULL;

    static const struct XMBItemController_t controller_methods = {
        .createItem   = QBRoutingSortItemConstructorCreateItem,
        .setItemState = QBRoutingSortItemConstructorSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRoutingSortItemConstructor",
                            sizeof(struct QBRoutingSortItemConstructor_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            NULL);
    }
    return type;
}

SvLocal QBRoutingSortItemConstructor QBRoutingSortItemConstructorCreate(QBRoutingSettingsListPane self)
{
    QBRoutingSortItemConstructor constructor = (QBRoutingSortItemConstructor) SvTypeAllocateInstance(QBRoutingSortItemConstructor_getType(), NULL);
    constructor->itemConstructor = QBXMBItemConstructorCreate("MenuItem", self->textRenderer);
    constructor->normalFocus = QBFrameConstructorFromSM("NormalFocus");
    constructor->selectedFocus = QBFrameConstructorFromSM("SelectedFocus");

    return constructor;
}

SvLocal void QBRoutingSettingsSwitchGatewayTenure(QBRoutingSettingsListPane self, int prev, int next)
{
    QBActiveTreeNode nodeNew = (QBActiveTreeNode) QBActiveArrayObjectAtIndex(self->sortedDataSource, next);
    QBActiveTreeNode nodeOld = (QBActiveTreeNode) QBActiveArrayObjectAtIndex(self->sortedDataSource, prev);
    QBNetworkInterface ifaceNew = (QBNetworkInterface) QBActiveTreeNodeGetAttribute(nodeNew, SVSTRING("iface"));
    QBNetworkInterface ifaceOld = (QBNetworkInterface) QBActiveTreeNodeGetAttribute(nodeOld, SVSTRING("iface"));

    int newGatewayTenure = QBNetworkInterfaceGetGatewayTenure(ifaceNew);
    int oldGatewayTenure = QBNetworkInterfaceGetGatewayTenure(ifaceOld);

    QBNetworkInterfaceSetGatewayTenure(ifaceNew, oldGatewayTenure);
    QBNetworkInterfaceSetGatewayTenure(ifaceOld, newGatewayTenure);

    QBNetworkMonitorSaveNetworkParameters(self->networkMonitor, ifaceNew);
    QBNetworkMonitorSaveNetworkParameters(self->networkMonitor, ifaceOld);
}

SvLocal void QBRoutingSettingsInterfaceDragged(void *self_, QBListPane pane, SvObject interface, int pos)
{
    QBRoutingSettingsListPane self = self_;

    SvWidget menu = QBListPaneGetMenu(self->listPane);
    XMBVerticalMenuSetState(menu, XMBMenuState_sorting);
}

SvLocal void QBRoutingSettingsChangesEnded(void *self_, int prev, int next)
{
    QBRoutingSettingsListPane self = self_;

    QBRoutingSettingsSwitchGatewayTenure(self, prev, next);

    SvWidget menu = QBListPaneGetMenu(self->listPane);
    XMBVerticalMenuSetState(menu, XMBMenuState_normal);
    QBActiveArrayMoveObject(self->sortedDataSource, prev, next, NULL);
}


SvLocal void
QBRoutingSettingsListPaneShow(SvObject self_)
{
    QBRoutingSettingsListPane self = (QBRoutingSettingsListPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->listPane, show);
}

SvLocal void
QBRoutingSettingsListPaneHide(SvObject self_, bool immediately)
{
    QBRoutingSettingsListPane self = (QBRoutingSettingsListPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->listPane, hide, immediately);
}

SvLocal void
QBRoutingSettingsListPaneSetActive(SvObject self_)
{
    QBRoutingSettingsListPane self = (QBRoutingSettingsListPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->listPane, setActive);
}

SvLocal bool
QBRoutingSettingsListPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvLocal void
QBRoutingSettingsListPaneDestroy(void *ptr)
{
    QBRoutingSettingsListPane self = ptr;

    SVRELEASE(self->sortedDataSource);
    SVRELEASE(self->listPane);
    SVRELEASE(self->textRenderer);
    SVRELEASE(self->networkMonitor);
    SVRELEASE(self->ctxMenu);
}

SvLocal SvType
QBRoutingSettingsListPane_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBRoutingSettingsListPaneDestroy
    };

    static const struct QBContextMenuPane_ methods = {
        .show             = QBRoutingSettingsListPaneShow,
        .hide             = QBRoutingSettingsListPaneHide,
        .setActive        = QBRoutingSettingsListPaneSetActive,
        .handleInputEvent = QBRoutingSettingsListPaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRoutingSettingsListPane",
                            sizeof(struct QBRoutingSettingsListPane_),
                            SvObject_getType(), &type,
                            QBContextMenuPane_getInterface(), &methods,
                            SvObject_getType(), &vtable,
                            NULL);
    }
    return type;
}

SvLocal void
QBRoutingSettingsListPaneInit(QBRoutingSettingsListPane self)
{
    self->settingsCtx = svSettingsSaveContext();

    QBRoutingSortItemConstructor constructor = QBRoutingSortItemConstructorCreate(self);

    static struct QBListPaneCallbacks_t listCallbacks = {
        .selected  = QBRoutingSettingsInterfaceDragged,
        .reordered = QBRoutingSettingsChangesEnded
    };

    self->listPane = (QBListPane) SvTypeAllocateInstance(QBListPane_getType(), NULL);
    QBListPaneInit(self->listPane, self->res, (SvObject) self->sortedDataSource, (SvObject) constructor, &listCallbacks, self, self->ctxMenu, 1, SVSTRING("ListPane"));
    SVRELEASE(constructor);
}

SvLocal int QBRoutingSettingsListPaneGatewayTenureCompareFn(void *prv, SvObject firstNode_, SvObject secondNode_)
{
    QBActiveTreeNode firstNode = (QBActiveTreeNode) firstNode_;
    QBActiveTreeNode secondNode = (QBActiveTreeNode) secondNode_;

    QBNetworkInterface firstInterface = (QBNetworkInterface) QBActiveTreeNodeGetAttribute(firstNode, SVSTRING("iface"));
    QBNetworkInterface secondInterface = (QBNetworkInterface) QBActiveTreeNodeGetAttribute(secondNode, SVSTRING("iface"));
    if(!firstInterface) {
        return 1;
    }
    if(!secondInterface) {
        return -1;
    }

    return QBNetworkInterfaceGetGatewayTenure(firstInterface) < QBNetworkInterfaceGetGatewayTenure(secondInterface) ? 1 : -1;
}

SvLocal void QBRoutingSettingsListPaneSortDataSource(QBRoutingSettingsListPane self, QBActiveArray dataSource)
{
    int count = QBActiveArrayCount(dataSource);
    for (int idx = 0; idx < count; idx++) {
        if (idx == 0) {
            QBActiveArrayAddObject(self->sortedDataSource, QBActiveArrayObjectAtIndex(dataSource, idx), NULL);
            continue;
        }
        size_t x = QBActiveArrayLowerBound(self->sortedDataSource, 0, idx, QBActiveArrayObjectAtIndex(dataSource, idx),
                                           QBRoutingSettingsListPaneGatewayTenureCompareFn, NULL);
        QBActiveArrayInsertObjectAtIndex(self->sortedDataSource, x, QBActiveArrayObjectAtIndex(dataSource, idx), NULL);
    }
}

QBRoutingSettingsListPane
QBRoutingSettingsListPaneCreateFromSettings(const char *settings,
                                            SvApplication res,
                                            QBTextRenderer textRenderer,
                                            QBNetworkMonitor networkMonitor,
                                            QBActiveArray dataSource,
                                            QBContextMenu ctxMenu,
                                            SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    if (!settings || !res || !textRenderer || !networkMonitor || !dataSource || !ctxMenu) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidArgument, error,
                                           "NULL parametrer passed settings=%p, app=%p,\
                                           textRenderer=%p, networkMonitor=%p,\
                                           sortedDataSource=%p, ctxMenu=%p", settings,
                                           res, textRenderer, networkMonitor,
                                           dataSource, ctxMenu);
        goto out;
    }

    QBRoutingSettingsListPane self = (QBRoutingSettingsListPane)
             SvTypeAllocateInstance(QBRoutingSettingsListPane_getType(), NULL);

    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBRoutingSettingsListPane");
        goto out;
    }

    self->sortedDataSource = QBActiveArrayCreate(QBActiveArrayCount(dataSource), &error);
    if (!self->sortedDataSource) {
        SVRELEASE(self);
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBActiveArray sortedDataSource");
        goto out;
    }

    self->res = res;
    self->textRenderer = SVRETAIN(textRenderer);
    self->networkMonitor = SVRETAIN(networkMonitor);
    self->ctxMenu = SVRETAIN(ctxMenu);

    QBRoutingSettingsListPaneSortDataSource(self, dataSource);

    svSettingsPushComponent(settings);
    QBRoutingSettingsListPaneInit(self);
    svSettingsPopComponent();
    return self;
out:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}
