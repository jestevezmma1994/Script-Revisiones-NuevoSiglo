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

#include "QBNetworkInformationMenuItemController.h"

#include <libintl.h>

#include <SvCore/SvCommonDefs.h>
#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <SWL/QBFrame.h>
#include <TranslationMerger.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBTypes.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBNATMonitor/QBNATMonitor.h>
#include <QBNetManager.h>
#include <main.h>
#include <Services/core/cableModemMonitor.h>
#include <Widgets/QBXMBItemConstructor.h>

struct QBNetworkInformationMenuItemController_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBActiveTree tree;

    SvBitmap icon;
    QBFrameConstructor *focus;
    QBFrameConstructor *inactiveFocus;
    QBXMBItemConstructor itemConstructor;
    unsigned int settingsCtx;

    SvRID iconRID, searchIconRID;

    SvHashTable ifaceToNodes;
    QBActiveTreeNode dns1, dns2, modemStatus, modemMac;
};

SvLocal void
QBNetworkInformationMenuItemControllerDestroy(void *self_)
{
    QBNetworkInformationMenuItemController self = self_;
    SVTESTRELEASE(self->icon);
    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);

    QBXMBItemConstructorDestroy(self->itemConstructor);

    SVTESTRELEASE(self->modemStatus);
    SVTESTRELEASE(self->modemMac);
    SVTESTRELEASE(self->dns1);
    SVTESTRELEASE(self->dns2);
    SVTESTRELEASE(self->ifaceToNodes);

    SVRELEASE(self->tree);
}

SvLocal SvWidget
QBNetworkInformationMenuItemControllerCreateItem(SvObject self_,
                                                 SvObject node_,
                                                 SvObject path,
                                                 SvApplication app,
                                                 XMBMenuState initialState)
{
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return NULL;

    QBNetworkInformationMenuItemController self = (QBNetworkInformationMenuItemController) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    QBXMBItem item = QBXMBItemCreate();

    item->loadingRID = self->iconRID;

    SvString iconPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("icon"));
    item->iconURI.URI = SVTESTRETAIN(iconPath);
    item->iconURI.isStatic = true;

    item->caption = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    const char *captionStr, *translatedCaptionStr;
    captionStr = SvStringCString(item->caption);
    translatedCaptionStr = gettext(captionStr);
    if (translatedCaptionStr != captionStr)
        item->caption = SvStringCreate(translatedCaptionStr, NULL);
    else
        SVRETAIN(item->caption);

    const char *subcaptionStr, *translatedSubcaptionStr;
    item->subcaption = (SvString) (QBActiveTreeNodeGetAttribute(node, SVSTRING("subcaption")));
    if (item->subcaption) {
        subcaptionStr = SvStringCString(item->subcaption);
        translatedSubcaptionStr = gettext(subcaptionStr);
        if (translatedSubcaptionStr != subcaptionStr)
            item->subcaption = SvStringCreate(translatedSubcaptionStr, NULL);
        else
            SVRETAIN(item->subcaption);
    }

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);
    return ret;
}

SvLocal void
QBNetworkInformationMenuItemControllerSetItemState(SvObject self_,
                                                   SvWidget item_,
                                                   XMBMenuState state,
                                                   bool isFocused)
{
    QBNetworkInformationMenuItemController self = (QBNetworkInformationMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}


SvLocal void
QBNetworkInformationMenuFillNetworkInterface(QBNetworkInformationMenuItemController self, QBActiveTreeNode mainNode, SvString iface);

SvLocal void
QBNetworkInformationMenuNodeUpdate(QBActiveTreeNode node, const char *, SvObject value);

typedef struct QBNetworkInformationMenuNetworkStatusNodes_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvString iface;
    QBActiveTreeNode ip, mask, gateway, mac, parent;
    QBActiveTree tree;
} *QBNetworkInformationMenuNetworkStatusNodes;

SvLocal void
QBNetworkInformationMenuInterfaceChanged(SvObject self_, SvString iface, uint64_t mask)
{
    QBNetworkInformationMenuItemController self = (QBNetworkInformationMenuItemController) self_;
    AppGlobals appGlobals = self->appGlobals;

    if (mask & (1ll << QBNetAttr_dns1)) {
        SvObject dns1 = QBNetManagerGetAttribute(appGlobals->net_manager, iface, QBNetAttr_dns1, NULL);
        QBNetworkInformationMenuNodeUpdate(self->dns1, NULL, dns1);
    }
    if (mask & (1ll << QBNetAttr_dns2)) {
        SvObject dns2 = QBNetManagerGetAttribute(appGlobals->net_manager, iface, QBNetAttr_dns2, NULL);
        QBNetworkInformationMenuNodeUpdate(self->dns2, NULL, dns2);
    }
    if ((mask & (1ll << QBNetAttr_name))) {
        SvString ifaceID = SvStringCreateWithFormat("_iface:%s_", SvStringCString(iface));
        QBActiveTreeNode ifaceNode = QBActiveTreeFindNode(self->tree, ifaceID);
        SvObject path = ifaceNode ? QBActiveTreeCreateNodePath(self->tree, QBActiveTreeNodeGetID(ifaceNode)) : NULL;
        if (ifaceNode && path) {
            QBActiveTreeRemoveSubTree(self->tree, ifaceNode, NULL);
            QBNetworkInformationMenuNetworkStatusNodes nodes = (QBNetworkInformationMenuNetworkStatusNodes) SvHashTableFind(self->ifaceToNodes, (SvObject) iface);
            QBNetManagerRemoveListener(appGlobals->net_manager, (SvObject) nodes, iface, NULL);
            QBNetManagerRemoveListener(appGlobals->net_manager, (SvObject) self, iface, NULL);
            SvHashTableRemove(self->ifaceToNodes, (SvObject) iface);
        } else {
            QBActiveTreeNode mainNode = QBActiveTreeFindNode(self->tree, SVSTRING("NetworkStatus"));
            QBNetworkInformationMenuFillNetworkInterface(self, mainNode, iface);
            QBNetManagerAddListener(appGlobals->net_manager, (SvObject) self, iface, NULL);
        }
        SVTESTRELEASE(path);
        SVRELEASE(ifaceID);
    }
}

SvLocal void
QBNetworkInformationMenuCableModemChanged(SvObject self_, uint64_t mask)
{
    QBNetworkInformationMenuItemController self = (QBNetworkInformationMenuItemController) self_;

    if (mask & QBCableModemAttr_Status) {
        SvObject status = QBCableModemMonitorGetAttribute(self->appGlobals->cableModem, QBCableModemAttr_Status);
        QBNetworkInformationMenuNodeUpdate(self->modemStatus, NULL, status);

        QBActiveTreeNode node = QBActiveTreeFindNode(self->tree, SVSTRING("NetworkStatus"));
        SvObject path = QBActiveTreeCreateNodePath(self->tree, QBActiveTreeNodeGetID(node));
        size_t count = QBActiveTreeNodeGetChildNodesCount(node);
        QBActiveTreePropagateNodesChange(self->tree, path, 0, count, NULL);
        SVRELEASE(path);
    }
    if (mask & QBCableModemAttr_Mac) {
        SvObject status = QBCableModemMonitorGetAttribute(self->appGlobals->cableModem, QBCableModemAttr_Mac);
        QBNetworkInformationMenuNodeUpdate(self->modemMac, NULL, status);

        QBActiveTreeNode node = QBActiveTreeFindNode(self->tree, SVSTRING("NetworkStatus"));
        SvObject path = QBActiveTreeCreateNodePath(self->tree, QBActiveTreeNodeGetID(node));
        size_t count = QBActiveTreeNodeGetChildNodesCount(node);
        QBActiveTreePropagateNodesChange(self->tree, path, 0, count, NULL);
        SVRELEASE(path);
    }
}

SvLocal void
QBNetworkInformationMenuNetworkStatusNodesDestroy(void *self_)
{
    QBNetworkInformationMenuNetworkStatusNodes self = (QBNetworkInformationMenuNetworkStatusNodes) self_;
    SVRELEASE(self->ip);
    SVRELEASE(self->mask);
    SVRELEASE(self->gateway);
    SVRELEASE(self->mac);
    SVRELEASE(self->iface);
}

SvLocal void
QBNetworkInformationMenuNodeUpdate(QBActiveTreeNode node, const char *captionStr, SvObject value)
{
    if (!node)
        return;
    SvString caption = captionStr ? SvStringCreate(captionStr, NULL) : NULL;
    SvString stringVal = NULL;

    if (value && SvObjectIsInstanceOf(value, SvString_getType()) && SvStringLength((SvString) value) > 0) {
        stringVal = (SvString) value;
        SVRETAIN(stringVal);
    } else {
        stringVal = SvStringCreate(gettext_noop("N/A"), NULL);
    }

    if (caption)
        QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvObject) caption);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvObject) stringVal);

    SVRELEASE(stringVal);
    SVTESTRELEASE(caption);
}

SvLocal void
QBNetworkInformationMenuNetworkStatusNodesFill(QBNetworkInformationMenuNetworkStatusNodes self, SvString iface, uint64_t update)
{
    AppGlobals appGlobals = self->appGlobals;

    if (update & (1ll << QBNetAttr_IPv4_address)) {
        SvObject ip = QBNetManagerGetAttribute(appGlobals->net_manager, iface, QBNetAttr_IPv4_address, NULL);
        QBNetworkInformationMenuNodeUpdate(self->ip, NULL, ip);
    }
    if (update & (1ll << QBNetAttr_IPv4_netMask)) {
        SvObject mask = QBNetManagerGetAttribute(appGlobals->net_manager, iface, QBNetAttr_IPv4_netMask, NULL);
        QBNetworkInformationMenuNodeUpdate(self->mask, NULL, mask);
    }

    if (update & (1ll << QBNetAttr_IPv4_gateway)) {
        SvObject gateway = QBNetManagerGetAttribute(appGlobals->net_manager, iface, QBNetAttr_IPv4_gateway, NULL);
        QBNetworkInformationMenuNodeUpdate(self->gateway, NULL, gateway);
    }

    if (update & (1ll << QBNetAttr_hwAddr)) {
        SvObject mac = QBNetManagerGetAttribute(appGlobals->net_manager, iface, QBNetAttr_hwAddr, NULL);
        QBNetworkInformationMenuNodeUpdate(self->mac, NULL, mac);
    }
}

SvLocal void
QBNetworkInformationMenuNetworkStatusNodesChanged(SvObject self_, SvString iface, uint64_t mask)
{
    QBNetworkInformationMenuNetworkStatusNodes self = (QBNetworkInformationMenuNetworkStatusNodes) self_;
    QBNetworkInformationMenuNetworkStatusNodesFill(self, self->iface, mask);

    SvObject path = QBActiveTreeCreateNodePath(self->tree, QBActiveTreeNodeGetID(self->parent));
    if (path) {
        size_t count = QBActiveTreeNodeGetChildNodesCount(self->parent);
        QBActiveTreePropagateNodesChange(self->tree, path, 0, count, NULL);
        SVRELEASE(path);
    }
}

SvLocal SvType
QBNetworkInformationMenuNetworkStatusNodes_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNetworkInformationMenuNetworkStatusNodesDestroy
    };
    static SvType type = NULL;
    static const struct QBNetListener_t methods = {
        .netAttributeChanged = QBNetworkInformationMenuNetworkStatusNodesChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNetworkInformationMenuNetworkStatusNodes",
                            sizeof(struct QBNetworkInformationMenuNetworkStatusNodes_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBNetListener_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvLocal void
QBNetworkInformationMenuFillNetworkInterface(QBNetworkInformationMenuItemController self, QBActiveTreeNode mainNode, SvString iface)
{
    AppGlobals appGlobals = self->appGlobals;

    //Find proper index for that interface
    SvIterator it = SvHashTableKeysIterator(self->ifaceToNodes);
    int idx = 0;
    SvString itiface;
    while ((itiface = (SvString) SvIteratorGetNext(&it))) {
        if (strcmp(SvStringCString(itiface), SvStringCString(iface)) > 0)
            idx++;
    }

    SvString id = SvStringCreateWithFormat("_iface:%s_", SvStringCString(iface));
    QBActiveTreeNode parent_node = QBActiveTreeNodeCreate(id, NULL, NULL);
    QBActiveTreeNodeSetAttribute(parent_node, SVSTRING("caption"), (SvObject) iface);
    QBActiveTreeInsertSubTree(self->tree, mainNode, parent_node, idx, NULL);
    SVRELEASE(id);

    QBNetworkInformationMenuNetworkStatusNodes nodes = (QBNetworkInformationMenuNetworkStatusNodes) SvTypeAllocateInstance(
        QBNetworkInformationMenuNetworkStatusNodes_getType(), NULL);

    nodes->appGlobals = appGlobals;
    nodes->ip = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBNetworkInformationMenuNodeUpdate(nodes->ip, gettext_noop("IP"), NULL);
    QBActiveTreeAddSubTree(self->tree, parent_node, nodes->ip, NULL);

    nodes->mask = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBNetworkInformationMenuNodeUpdate(nodes->mask, gettext_noop("Network mask"), NULL);
    QBActiveTreeAddSubTree(self->tree, parent_node, nodes->mask, NULL);

    nodes->gateway = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBNetworkInformationMenuNodeUpdate(nodes->gateway, gettext_noop("Gateway"), NULL);
    QBActiveTreeAddSubTree(self->tree, parent_node, nodes->gateway, NULL);

    nodes->mac = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBNetworkInformationMenuNodeUpdate(nodes->mac, gettext_noop("MAC address"), NULL);
    QBActiveTreeAddSubTree(self->tree, parent_node, nodes->mac, NULL);

    nodes->parent = mainNode;
    nodes->iface = SVRETAIN(iface);
    nodes->tree = self->tree;
    QBNetworkInformationMenuNetworkStatusNodesChanged((SvObject) nodes, iface, -1);
    QBNetManagerAddListener(appGlobals->net_manager, (SvObject) nodes, iface, NULL);

    SvHashTableInsert(self->ifaceToNodes, (SvObject) iface, (SvObject) nodes);

    SVRELEASE(nodes);
    SVRELEASE(parent_node);
}

SvLocal void
QBNetworkInformationMenuFillNetworkSettings(QBNetworkInformationMenuItemController self, AppGlobals appGlobals, QBActiveTreeNode mainNode)
{
    self->ifaceToNodes = SvHashTableCreate(20, NULL);

    SvArray iface_tab = QBNetManagerCreateInterfacesList(appGlobals->net_manager, NULL);

    QBActiveTreeNode dns1 = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBActiveTreeNode dns2 = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBActiveTreeNode modemStatus = NULL;
    QBActiveTreeNode modemMac = NULL;

    if (iface_tab) {
        SvIterator it = SvArrayIterator(iface_tab);
        SvString iface;
        while ((iface = (SvString) SvIteratorGetNext(&it))) {
            QBNetworkInformationMenuFillNetworkInterface(self, mainNode, iface);
        }
    }

    QBActiveTreeNode node = QBActiveTreeFindNode(self->tree, SVSTRING("CableModem"));
    SvObject path = node ? QBActiveTreeCreateNodePath(self->tree, QBActiveTreeNodeGetID(node)) : NULL;
    if (path && self->appGlobals->cableModem) {
        modemStatus = QBActiveTreeNodeCreate(NULL, NULL, NULL);
        modemMac = QBActiveTreeNodeCreate(NULL, NULL, NULL);

        SvObject cableModemStatus = QBCableModemMonitorGetAttribute(self->appGlobals->cableModem, QBCableModemAttr_Status);
        QBNetworkInformationMenuNodeUpdate(modemStatus, gettext_noop("Cable modem status"), cableModemStatus);
        SvObject cableModemMac = QBCableModemMonitorGetAttribute(self->appGlobals->cableModem, QBCableModemAttr_Mac);
        QBNetworkInformationMenuNodeUpdate(modemMac, gettext_noop("HFC MAC ID"), cableModemMac);

        QBActiveTreeAddSubTree(self->tree, node, modemStatus, NULL);
        QBActiveTreeAddSubTree(self->tree, node, modemMac, NULL);
    }
    SVTESTRELEASE(path);

    QBNetworkInformationMenuNodeUpdate(dns1, gettext_noop("DNS1"), NULL);
    QBNetworkInformationMenuNodeUpdate(dns2, gettext_noop("DNS2"), NULL);
    QBActiveTreeAddSubTree(self->tree, mainNode, dns1, NULL);
    QBActiveTreeAddSubTree(self->tree, mainNode, dns2, NULL);
    self->dns1 = dns1;
    self->dns2 = dns2;
    self->modemStatus = modemStatus;
    self->modemMac = modemMac;
    SVTESTRELEASE(iface_tab);

    QBNetworkInformationMenuInterfaceChanged((SvObject) self, NULL, (1ll << QBNetAttr_dns1) | (1ll << QBNetAttr_dns2));
}

SvLocal QBActiveTreeNode
QBNetworkInformationMenuAddSimpleNode(QBActiveTreeNode parentNode,
                                      SvString caption, SvString subcaption)
{
    QBActiveTreeNode node = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    if (caption)
        QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvObject) caption);
    if (subcaption)
        QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvObject) subcaption);

    QBActiveTreeNodeAddSubTree(parentNode, node, NULL);
    SVRELEASE(node);
    return node;
}

SvLocal void
QBNetworkInformationMenuNATMonitorChanged(void *self_, QBNATMonitor monitor)
{
    QBNetworkInformationMenuItemController self = self_;

    QBActiveTreeNode mainNode = QBActiveTreeFindNode(self->tree, SVSTRING(gettext_noop("NATStatus")));
    SvObject mainPath = mainNode ? QBActiveTreeCreateNodePath(self->tree, QBActiveTreeNodeGetID(mainNode)) : NULL;
    if (!mainPath)
        return;

    /// remove all nodes except the first (iface)
    size_t cnt = QBActiveTreeNodeGetChildNodesCount(mainNode);
    for (; cnt > 1; cnt--) {
        QBActiveTreeNode node = QBActiveTreeNodeGetChildByIndex(mainNode, 1);
        QBActiveTreeNodeRemoveChildNode(mainNode, node);
    };

    /// get current status
    QBNATMonitorStatus status;
    QBNATMonitorGetStatus(self->appGlobals->natMonitor, &status);

    /// fill-out the nodes:

    /// iface : this node should be preserved at all times
    SvString iface = status.localIface;
    if (!iface)
        iface = SVSTRING("---");
    QBActiveTreeNode ifaceNode = NULL;
    if (cnt > 0) {
        ifaceNode = QBActiveTreeNodeGetChildByIndex(mainNode, 0);
    } else {
        ifaceNode = QBActiveTreeNodeCreate(NULL, NULL, NULL);
        QBActiveTreeNodeSetAttribute(ifaceNode, SVSTRING("caption"), (SvObject) SVSTRING(gettext_noop("Default interface")));
        QBActiveTreeAddSubTree(self->tree, mainNode, ifaceNode, NULL);
        SVRELEASE(ifaceNode);
    };
    QBActiveTreeNodeSetAttribute(ifaceNode, SVSTRING("subcaption"), (SvObject) iface);

    /// ip
    SvString ip = status.localIP;
    if (!ip)
        ip = SVSTRING("---");
    QBNetworkInformationMenuAddSimpleNode(mainNode, SVSTRING(gettext_noop("IP")), ip);

    /// udp status (node)
    QBActiveTreeNode udpNode = QBNetworkInformationMenuAddSimpleNode(mainNode, SVSTRING(gettext_noop("UDP receiving")), NULL);

    bool udpReceivingPossible = false;

    /// stun stuff
    if (status.ipIsPublic) {
        udpReceivingPossible = true;
    } else if (status.mappedIP) {
        QBNetworkInformationMenuAddSimpleNode(mainNode, SVSTRING(gettext_noop("Public IP")), status.mappedIP);
        QBNetworkInformationMenuAddSimpleNode(mainNode, SVSTRING(gettext_noop("UDP port behind NAT")),
                                              status.portPreserved ? SVSTRING(gettext_noop("Preserved")) : SVSTRING(gettext_noop("Randomized")));
        if (status.ipRestricted)
            QBNetworkInformationMenuAddSimpleNode(mainNode, SVSTRING(gettext_noop("STUN IP restricted")), NULL);
        if (status.portRestricted)
            QBNetworkInformationMenuAddSimpleNode(mainNode, SVSTRING(gettext_noop("STUN Port Restricted")), NULL);

        udpReceivingPossible = status.portPreserved || (!status.portRestricted && !status.ipRestricted);
    } else {
        QBNetworkInformationMenuAddSimpleNode(mainNode, SVSTRING(gettext_noop("No STUN Server Connection")), NULL);
    };

    /// udp status (subcaption)
    QBActiveTreeNodeSetAttribute(udpNode, SVSTRING("subcaption"),
                                 (SvObject) (udpReceivingPossible ? SVSTRING(gettext_noop("OK")) : SVSTRING(gettext_noop("Not Possible"))));

    /// propagate changes in gui
    cnt = QBActiveTreeNodeGetChildNodesCount(mainNode);
    QBActiveTreePropagateNodesChange(self->tree, mainPath, 0, cnt, NULL);

    SVRELEASE(mainPath);
}

SvLocal SvType
QBNetworkInformationMenuItemController_getType(void)
{
    static SvType type = NULL;
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNetworkInformationMenuItemControllerDestroy
    };

    static const struct XMBItemController_t controller_methods = {
        .createItem   = QBNetworkInformationMenuItemControllerCreateItem,
        .setItemState = QBNetworkInformationMenuItemControllerSetItemState,
    };

    static const struct QBNetListener_t netMethods = {
        .netAttributeChanged = QBNetworkInformationMenuInterfaceChanged,
    };

    static const struct QBCableModemListener_t cableModemMethods = {
        .cableModemAttributeChanged = QBNetworkInformationMenuCableModemChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNetworkInformationMenuItemController",
                            sizeof(struct QBNetworkInformationMenuItemController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            QBNetListener_getInterface(), &netMethods,
                            QBCableModemListener_getInterface(), &cableModemMethods,
                            NULL);
    }

    return type;
}

QBNetworkInformationMenuItemController
QBNetworkInformationMenuItemControllerCreate(AppGlobals appGlobals, QBActiveTree tree, SvErrorInfo *errorOut)
{
    QBNetworkInformationMenuItemController self = (QBNetworkInformationMenuItemController)
                                                  SvTypeAllocateInstance(QBNetworkInformationMenuItemController_getType(), NULL);

    self->appGlobals = appGlobals;
    self->tree = SVRETAIN(tree);
    self->settingsCtx = svSettingsSaveContext();

    svSettingsPushComponent("Configuration.settings");

    self->searchIconRID = svSettingsGetResourceID("MenuItem", "searchIcon");
    self->iconRID = svSettingsGetResourceID("MenuItem", "icon");
    self->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        self->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    self->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);

    QBActiveTreeNode node = QBActiveTreeFindNode(tree, SVSTRING("NetworkStatus"));
    if (node) {
        QBNetworkInformationMenuFillNetworkSettings(self, appGlobals, node);

        if (appGlobals->net_manager)
            QBNetManagerAddListener(appGlobals->net_manager, (SvObject) self, NULL, NULL);
        if (appGlobals->cableModem)
            QBCableModemMonitorAddListener(appGlobals->cableModem, (SvObject) self);
    }

    node = QBActiveTreeFindNode(tree, SVSTRING("NATStatus"));
    if (node && appGlobals->natMonitor) {
        QBNetworkInformationMenuNATMonitorChanged(self, appGlobals->natMonitor);
        QBNATMonitorRegisterCallback(appGlobals->natMonitor, QBNetworkInformationMenuNATMonitorChanged, self);
    }

    svSettingsPopComponent();

    return self;
}
