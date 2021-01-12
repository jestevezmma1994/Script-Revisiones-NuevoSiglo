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

#include "QBIPSecVPNStatusMenu.h"

#include <main.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBTypes.h>
#include <QBMenu/QBMenu.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBNetworkMonitor.h>
#include <QBNetworkMonitor/QBNetworkMonitorListener.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <SWL/QBFrame.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <TranslationMerger.h>
#include <libintl.h>

typedef struct QBIPSecVPNStatusMenuItemController_ {
    struct SvObject_ super_;

    QBActiveTree tree;

    QBNetworkMonitor networkMonitor;

    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    QBXMBItemConstructor itemConstructor;

    SvRID iconRID;

    QBActiveTreeNode statusNode;
    QBActiveTreeNode interfaceNode;
    QBActiveTreeNode localIPNode;
    QBActiveTreeNode localPortNode;
    QBActiveTreeNode vpnServerIPNode;
    QBActiveTreeNode vpnServerPortNode;
    QBActiveTreeNode vpnIPNode;
    QBActiveTreeNode vpnNetmaskNode;
    QBActiveTreeNode vpnDNSNode;
} *QBIPSecVPNStatusMenuItemController;

SvLocal const char *
ipSecVPNStatusToString(QBNetworkManagerIPSecVPNStatus ipSecVPNStatus)
{
    switch (ipSecVPNStatus) {
        case QBNetworkManagerIPSecVPNStatus_disabled:
            return gettext_noop("Disabled");
        case QBNetworkManagerIPSecVPNStatus_disconnected:
            return gettext_noop("Disconnected");
        case QBNetworkManagerIPSecVPNStatus_connecting:
            return gettext_noop("Connecting");
        case QBNetworkManagerIPSecVPNStatus_connected:
            return gettext_noop("Connected");
    }

    return gettext_noop("Unknown");
}

SvLocal void
QBIPSecVPNStatusMenuItemControllerDestroy(void *self_)
{
    QBIPSecVPNStatusMenuItemController self = self_;
    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);

    QBXMBItemConstructorDestroy(self->itemConstructor);

    SVRELEASE(self->tree);
    SVRELEASE(self->networkMonitor);
}

SvLocal SvWidget
QBIPSecVPNStatusMenuItemControllerCreateItem(SvObject self_,
                                             SvObject node_,
                                             SvObject path,
                                             SvApplication app,
                                             XMBMenuState initialState)
{
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return NULL;

    QBIPSecVPNStatusMenuItemController self = (QBIPSecVPNStatusMenuItemController) self_;
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
QBIPSecVPNStatusMenuItemControllerSetItemState(SvObject self_,
                                                   SvWidget item_,
                                                   XMBMenuState state,
                                                   bool isFocused)
{
    QBIPSecVPNStatusMenuItemController self = (QBIPSecVPNStatusMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void
QBIPSecVPNStatusMenuItemControllerInterfaceAdded(SvObject self_, QBNetworkInterface interface)
{
    // not needed
}

SvLocal void
QBIPSecVPNStatusMenuItemControllerInterfaceRemoved(SvObject self_, SvString interfaceID)
{
    // not needed
}

SvLocal void
QBIPSecVPNStatusMenuItemControllerInterfaceStateChanged(SvObject self_, SvString interfaceID)
{
    // not needed
}

SvLocal void
QBIPSecVPNStatusMenuItemControllerIPSecVPNStateChanged(SvObject self_)
{
    QBIPSecVPNStatusMenuItemController self = (QBIPSecVPNStatusMenuItemController) self_;
    assert(self->statusNode);
    QBIPSecVPN vpn = QBNetworkMonitorGetIPSecVPN(self->networkMonitor);
    QBNetworkManagerIPSecVPNStatus status = vpn ? QBIPSecVPNGetStatus(vpn) : QBNetworkManagerIPSecVPNStatus_disabled;

    SvString statusStr = SvStringCreate(ipSecVPNStatusToString(status), NULL);
    QBActiveTreeNodeSetAttribute(self->statusNode, SVSTRING("subcaption"), (SvObject) statusStr);
    SVRELEASE(statusStr);

    SvString interfaceID = NULL;
    SvString localIP = NULL;
    SvString localPort = NULL;
    SvString vpnServerIP = NULL;
    SvString vpnServerPort = NULL;
    SvString vpnIP = NULL;
    SvString vpnNetmask = NULL;
    SvString vpnDNS = NULL;

    if (vpn) {
        interfaceID = SVTESTRETAIN(QBIPSecVPNGetInterfaceID(vpn));
        localIP = SVTESTRETAIN(QBIPSecVPNGetLocalIP(vpn));
        vpnServerIP = SVTESTRETAIN(QBIPSecVPNGetVPNServerIP(vpn));
        vpnIP = SVTESTRETAIN(QBIPSecVPNGetVPNIP(vpn));
        vpnNetmask = SVTESTRETAIN(QBIPSecVPNGetVPNNetmask(vpn));
        vpnDNS = SVTESTRETAIN(QBIPSecVPNGetVPNDNS(vpn));

        char *localPortStr = NULL;
        asprintf(&localPortStr, "%d", QBIPSecVPNGetLocalPort(vpn));
        localPort = SvStringCreate(localPortStr, NULL);
        free(localPortStr);

        char *vpnPortStr = NULL;
        asprintf(&vpnPortStr, "%d", QBIPSecVPNGetVPNServerPort(vpn));
        vpnServerPort = SvStringCreate(vpnPortStr, NULL);
        free(vpnPortStr);
    }

    SvString notAccesibleAddr = SvStringCreate(gettext("N/A"), NULL);
    interfaceID = interfaceID ? interfaceID : SVRETAIN(notAccesibleAddr);
    localIP = localIP ? localIP : SVRETAIN(notAccesibleAddr);
    localPort = localPort ? localPort : SVRETAIN(notAccesibleAddr);
    vpnServerIP = vpnServerIP ? vpnServerIP : SVRETAIN(notAccesibleAddr);
    vpnServerPort = vpnServerPort ? vpnServerPort : SVRETAIN(notAccesibleAddr);
    vpnIP = vpnIP ? vpnIP : SVRETAIN(notAccesibleAddr);
    vpnNetmask = vpnNetmask ? vpnNetmask : SVRETAIN(notAccesibleAddr);
    vpnDNS = vpnDNS > 0  ? vpnDNS : SVRETAIN(notAccesibleAddr);
    SVRELEASE(notAccesibleAddr);


    QBActiveTreeNodeSetAttribute(self->interfaceNode, SVSTRING("subcaption"), (SvObject) interfaceID);
    QBActiveTreeNodeSetAttribute(self->localIPNode, SVSTRING("subcaption"), (SvObject) localIP);
    QBActiveTreeNodeSetAttribute(self->localPortNode, SVSTRING("subcaption"), (SvObject) localPort);
    QBActiveTreeNodeSetAttribute(self->vpnServerIPNode, SVSTRING("subcaption"), (SvObject) vpnServerIP);
    QBActiveTreeNodeSetAttribute(self->vpnServerPortNode, SVSTRING("subcaption"), (SvObject) vpnServerPort);
    QBActiveTreeNodeSetAttribute(self->vpnIPNode, SVSTRING("subcaption"), (SvObject) vpnIP);
    QBActiveTreeNodeSetAttribute(self->vpnNetmaskNode, SVSTRING("subcaption"), (SvObject) vpnNetmask);
    QBActiveTreeNodeSetAttribute(self->vpnDNSNode, SVSTRING("subcaption"), (SvObject) vpnDNS);

    SVRELEASE(interfaceID);
    SVRELEASE(localIP);
    SVRELEASE(localPort);
    SVRELEASE(vpnServerIP);
    SVRELEASE(vpnServerPort);
    SVRELEASE(vpnIP);
    SVRELEASE(vpnNetmask);
    SVRELEASE(vpnDNS);

    QBActiveTreePropagateNodeChange(self->tree, self->interfaceNode, NULL);
    QBActiveTreePropagateNodeChange(self->tree, self->statusNode, NULL);
    QBActiveTreePropagateNodeChange(self->tree, self->localIPNode, NULL);
    QBActiveTreePropagateNodeChange(self->tree, self->localPortNode, NULL);
    QBActiveTreePropagateNodeChange(self->tree, self->vpnServerIPNode, NULL);
    QBActiveTreePropagateNodeChange(self->tree, self->vpnServerPortNode, NULL);
    QBActiveTreePropagateNodeChange(self->tree, self->vpnIPNode, NULL);
    QBActiveTreePropagateNodeChange(self->tree, self->vpnNetmaskNode, NULL);
    QBActiveTreePropagateNodeChange(self->tree, self->vpnDNSNode, NULL);
}

SvLocal SvType
QBIPSecVPNStatusMenuItemController_getType(void)
{
    static SvType type = NULL;
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBIPSecVPNStatusMenuItemControllerDestroy
    };

    static const struct XMBItemController_t controller_methods = {
        .createItem   = QBIPSecVPNStatusMenuItemControllerCreateItem,
        .setItemState = QBIPSecVPNStatusMenuItemControllerSetItemState,
    };

    static const struct QBNetworkMonitorListener_ networkMonitorListenerMethods = {
        .interfaceAdded = QBIPSecVPNStatusMenuItemControllerInterfaceAdded,
        .interfaceRemoved = QBIPSecVPNStatusMenuItemControllerInterfaceRemoved,
        .interfaceStateChanged = QBIPSecVPNStatusMenuItemControllerInterfaceStateChanged,
        .ipSecVPNStateChanged = QBIPSecVPNStatusMenuItemControllerIPSecVPNStateChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBIPSecVPNStatusMenuItemController",
                            sizeof(struct QBIPSecVPNStatusMenuItemController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            QBNetworkMonitorListener_getInterface(), &networkMonitorListenerMethods,
                            NULL);
    }

    return type;
}

void
QBIPSecVPNStatusMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    QBActiveTreeNode mainNode = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("IPSecVPNStatus"));
    SvObject path = mainNode ? QBActiveTreeCreateNodePath(appGlobals->menuTree, QBActiveTreeNodeGetID(mainNode)) : NULL;
    if (!mainNode || !path)
        return;

    QBIPSecVPNStatusMenuItemController self = (QBIPSecVPNStatusMenuItemController)
        SvTypeAllocateInstance(QBIPSecVPNStatusMenuItemController_getType(), NULL);

    self->tree = SVRETAIN(appGlobals->menuTree);
    self->networkMonitor = SVRETAIN(appGlobals->networkMonitor);

    svSettingsPushComponent("Configuration.settings");
    self->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);
    self->focus = QBFrameConstructorFromSM("MenuItem.focus");

    self->statusNode        = QBActiveTreeFindNode(self->tree, SVSTRING("IPSecVPNConnectionStatus"));
    self->interfaceNode     = QBActiveTreeFindNode(self->tree, SVSTRING("IPSecVPNInterface"));
    self->localIPNode       = QBActiveTreeFindNode(self->tree, SVSTRING("IPSecVPNLocalIP"));
    self->localPortNode     = QBActiveTreeFindNode(self->tree, SVSTRING("IPSecVPNLocalPort"));
    self->vpnServerIPNode   = QBActiveTreeFindNode(self->tree, SVSTRING("IPSecVPNServerIP"));
    self->vpnServerPortNode = QBActiveTreeFindNode(self->tree, SVSTRING("IPSecVPNServerPort"));
    self->vpnIPNode         = QBActiveTreeFindNode(self->tree, SVSTRING("IPSecVPNIP"));
    self->vpnNetmaskNode    = QBActiveTreeFindNode(self->tree, SVSTRING("IPSecVPNNetmask"));
    self->vpnDNSNode        = QBActiveTreeFindNode(self->tree, SVSTRING("IPSecVPNDNS"));

    const char *inactiveFocusWidgetName = "MenuItem.inactiveFocus";
    if (svSettingsIsWidgetDefined(inactiveFocusWidgetName)) {
        self->inactiveFocus = QBFrameConstructorFromSM(inactiveFocusWidgetName);
    }

    svSettingsPopComponent();

    QBTreePathMapInsert(pathMap, path, (SvObject) self, NULL);

    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) self, NULL);

    QBNetworkMonitorAddListener(appGlobals->networkMonitor, (SvObject) self);

    SVRELEASE(path);
    SVRELEASE(self);

    QBIPSecVPNStatusMenuItemControllerIPSecVPNStateChanged((SvObject) self);
}
