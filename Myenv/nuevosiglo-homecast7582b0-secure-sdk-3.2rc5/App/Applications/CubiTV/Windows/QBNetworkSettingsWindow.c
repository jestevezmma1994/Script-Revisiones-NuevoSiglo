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

#include "QBNetworkSettingsWindow.h"
#include <ContextMenus/QBRoutingSettingsListPane.h>
#include <QBSecureLogManager.h>
#include <Utils/QBNetworkSettingsUtils.h>
#include <ContextMenus/QBNetworkSettingsAdvancedPane.h>
#include <ContextMenus/wifi/QBWiFiManualSettingsPane.h>
#include <ContextMenus/wifi/QBWiFiNetworkListPane.h>
#include <Widgets/QBSimpleDialog.h>
#include <Widgets/customerLogo.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <QBNetworkMonitor/QBNetworkInterface.h>
#include <QBNetworkMonitor/QBNetworkMonitorListener.h>
#include <QBNetworkMonitor/QBIPSecVPN.h>
#include <SWL/label.h>
#include <SWL/button.h>
#include <SWL/events.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/widget.h>
#include <settings.h>
#include <QBInput/QBInputCodes.h>
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBActiveArray.h>
#include <libintl.h>
#include <main.h>
#include <QBJSONUtils.h>


typedef struct QBNetworkSettingsContext_ {
    struct QBWindowContext_t super_;

    SvApplication res;
    SvScheduler scheduler;
    QBInitLogic initLogic;
    QBNetworkMonitor networkMonitor;
    QBApplicationController controller;
    QBTextRenderer textRenderer;

    SvHashTable idToInterface;
    QBActiveArray availInterfacesArray;

    int settingsCtx;
    bool isStrict;

    struct QBNetworkSettingsSideMenu_ {
        QBContextMenu ctx;
    } sidemenu;

    SvWidget window, hintLabel;
    SvWidget okBtn, routingBtn;

    QBSortedList buttons;

    struct {
        SvWidget vpnStatus;
        SvWidget interfaceID;
        SvWidget localIP;
        SvWidget localPort;
        SvWidget vpnServerIP;
        SvWidget vpnServerPort;
        SvWidget vpnIP;
        SvWidget vpnNetmask;
        SvWidget vpnDNS;
    } vpnLabels;
} *QBNetworkSettingsContext;


typedef struct InterfaceButton_ {
    struct SvObject_ super_;
    SvWidget button;
    SvString ifId;
} *InterfaceButton;

SvLocal void
InterfaceButtonDestroy(void *self_)
{
    InterfaceButton self = (InterfaceButton) self_;
    svWidgetDestroy(self->button);
    SVRELEASE(self->ifId);
}

SvLocal SvType
InterfaceButton_getType(void)
{
    static const struct SvObjectVTable_ vtable = {
        .destroy = InterfaceButtonDestroy
    };

    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("InterfaceButton",
                            sizeof(struct InterfaceButton_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &vtable,
                            NULL);
    }

    return type;
}


SvLocal void
QBNetworkSettingsContextAddInterfaceButton(QBNetworkSettingsContext self, SvWidget button, SvString ifId)
{
    InterfaceButton btn = (InterfaceButton) SvTypeAllocateInstance(InterfaceButton_getType(), NULL);
    btn->button = button;
    btn->ifId = SVRETAIN(ifId);
    QBSortedListInsert(self->buttons, (SvObject) btn);
    SVRELEASE(btn);
}

SvLocal void
QBNetworkSettingsContextRemoveInterfaceButton(QBNetworkSettingsContext self, SvString ifId)
{
    InterfaceButton bwi;
    SvIterator it = QBSortedListIterator(self->buttons);
    while ((bwi = (InterfaceButton) SvIteratorGetNext(&it))) {
        if (SvObjectEquals((SvObject) bwi->ifId, (SvObject) ifId)) {
            QBSortedListRemove(self->buttons, (SvObject) bwi);
            break;
        }
    }
}

SvLocal SvString
QBNetworkSettingsContextGetIdOfFocusedInterface(QBNetworkSettingsContext self)
{
    InterfaceButton bwi;
    SvIterator it = QBSortedListIterator(self->buttons);
    while ((bwi = (InterfaceButton) SvIteratorGetNext(&it))) {
        if (svWidgetIsFocused(bwi->button))
            return bwi->ifId;
    }
    return NULL;
}

SvLocal SvWidget
QBNetworkSettingsContextGetButtonByInterfaceId(QBNetworkSettingsContext self, SvString ifId)
{
    InterfaceButton bwi;
    SvIterator it = QBSortedListIterator(self->buttons);
    while ((bwi = (InterfaceButton) SvIteratorGetNext(&it))) {
        if (SvObjectEquals((SvObject) bwi->ifId, (SvObject) ifId)) {
            return bwi->button;
        }
    }
    return NULL;
}

SvLocal void
QBNetworkSettingsContextButtonsRealign(QBNetworkSettingsContext self)
{
    svSettingsRestoreContext(self->settingsCtx);
    const int offYStart = svSettingsGetInteger("NetworkSettingsWindow.interface", "yOffset", -1);
    const int offYStep = svSettingsGetInteger("NetworkSettingsWindow.interface", "yOffsetStep", -1);
    svSettingsPopComponent();

    if (offYStart < 0 || offYStep < 0) {
        SvLogError("Network Settings window: Cannot Realign Buttons because settings yOffset or yOffsetStep is missing");
        return;
    }

    int offY = offYStart - offYStep;

    SvWidget focused = NULL;

    InterfaceButton bwi;
    SvIterator it = QBSortedListIterator(self->buttons);
    while ((bwi = (InterfaceButton) SvIteratorGetNext(&it))) {
        SvWidget button = bwi->button;
        button->off_y = offY += offYStep;
        svWidgetForceRender(button);
        if (!focused || svWidgetIsFocused(button))
            focused = button;
    }
    if (focused)
        svWidgetSetFocus(focused);
}

SvLocal void
QBNetworkSettingsContextHandleConnectionState(QBNetworkSettingsContext self)
{
    bool isConnected = QBNetworkMonitorIsAnyInterfaceConnected(self->networkMonitor);
    if (self->window) {
        if (self->isStrict) {
            svButtonSetDisabled(self->okBtn, !isConnected);
            svWidgetSetHidden(self->okBtn, !isConnected);
        }
        svWidgetSetHidden(self->hintLabel, isConnected);
    }
}

SvLocal SvWidget
QBNetworkSettingsContextCreateInterfaceButton(QBNetworkSettingsContext self, QBNetworkInterface interface)
{
    svSettingsRestoreContext(self->settingsCtx);
    SvString ifaceName = QBNetworkSettingsUtilsCreateInterfaceName(interface);

    SvWidgetId windowId = (self->window) ? svWidgetGetId(self->window) : 0;
    SvWidget button = svButtonNewFromSM(self->res, "NetworkSettingsWindow.interface", SvStringGetCString(ifaceName), 0, windowId);
    if (self->window)
        svSettingsWidgetAttach(self->window, button, "NetworkSettingsWindow.interface", 1);

    SVRELEASE(ifaceName);
    svSettingsPopComponent();

    SvString desc = QBNetworkSettingsUtilsCreateInterfaceConnectionDescription(interface);
    svButtonSetSubcaptionAndResize(button, SvStringGetCString(desc));
    SVRELEASE(desc);

    return button;
}

SvLocal void
QBNetworkSettingsContextDetachInterfaceButton(QBNetworkSettingsContext self, SvString interfaceId)
{
    SvWidget button = QBNetworkSettingsContextGetButtonByInterfaceId(self, interfaceId);
    if (svWidgetIsFocused(button)) // means that possibly the side menu of that button can be opened
        QBContextMenuHide(self->sidemenu.ctx, false);
    svWidgetDetach(button);
}

SvLocal void
QBNetworkSettingsContextInterfaceAdded(QBNetworkSettingsContext self,
                                       QBNetworkInterface interface)
{
    SvWidget button = QBNetworkSettingsContextCreateInterfaceButton(self, interface);
    QBNetworkSettingsContextAddInterfaceButton(self, button, QBNetworkInterfaceGetID(interface));
    QBNetworkSettingsContextButtonsRealign(self);
// for routing menu
    QBActiveTreeNode node = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("iface"), (SvObject) interface);
    QBActiveArrayAddObject(self->availInterfacesArray, (SvObject) node, NULL);
    SVRELEASE(node);
}

SvLocal void
QBNetworkSettingsContextInterfaceRemoved(QBNetworkSettingsContext self, SvString interfaceId)
{
    QBNetworkSettingsContextDetachInterfaceButton(self, interfaceId);
    QBNetworkSettingsContextRemoveInterfaceButton(self, interfaceId);
    QBNetworkSettingsContextButtonsRealign(self);
    QBNetworkSettingsContextHandleConnectionState(self);
// for routing menu
    SvObject node;
    QBNetworkInterface iface;
    int length = QBActiveArrayCount(self->availInterfacesArray);
    for (int i = 0; i < length; i++) {
        node = QBActiveArrayObjectAtIndex(self->availInterfacesArray, i);
        iface = (QBNetworkInterface) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node, SVSTRING("iface"));
        if (SvObjectEquals((SvObject) interfaceId, ((SvObject) QBNetworkInterfaceGetID(iface)))) {
            QBActiveArrayRemoveObjectAtIndex(self->availInterfacesArray, i, NULL);
            break;
        }
    }
}

SvLocal void
QBNetworkSettingsContextInterfaceChanged(QBNetworkSettingsContext self,
                                         QBNetworkInterface interface)
{
    SvWidget button = QBNetworkSettingsContextGetButtonByInterfaceId(self, QBNetworkInterfaceGetID(interface));

    SvString ifaceName = QBNetworkSettingsUtilsCreateInterfaceName(interface);
    SvString desc = QBNetworkSettingsUtilsCreateInterfaceConnectionDescription(interface);

    svButtonSetCaptionAndResize(button, SvStringGetCString(ifaceName));
    svButtonSetSubcaptionAndResize(button, SvStringGetCString(desc));

    SVRELEASE(ifaceName);
    SVRELEASE(desc);

    QBNetworkSettingsContextHandleConnectionState(self);
}

SvLocal void
QBNetworkSettingsContextErrorDialogShow(QBNetworkSettingsContext self, int settingsCtx, const char *title, const char *message)
{
    svSettingsRestoreContext(settingsCtx);
    QBDialogParameters params = {
        .app        = self->res,
        .controller = self->controller,
        .widgetName = "Dialog",
        .ownerId    = 0
    };
    SvWidget dialog = QBSimpleDialogCreate(&params, title, message);
    QBDialogRun(dialog, NULL, NULL);
    char *escapedTitle = QBStringCreateJSONEscapedString(title);
    char *escapedMsg = QBStringCreateJSONEscapedString(message);
    QBSecureLogEvent("NetworkSettings", "ErrorShown.Configuration.NetworkSettings", "JSON:{\"description\":\"%s: %s\"}", escapedTitle, escapedMsg);
    free(escapedTitle);
    free(escapedMsg);
    svSettingsPopComponent();
}

SvLocal void
QBNetworkSettingsContextInterfaceAdded_(SvObject self_, QBNetworkInterface interface)
{
    QBNetworkSettingsContext self = (QBNetworkSettingsContext) self_;

    const SvObject prevInterface = SvHashTableFind(self->idToInterface, (SvObject) QBNetworkInterfaceGetID(interface));
    SvHashTableInsert(self->idToInterface, (SvObject) QBNetworkInterfaceGetID(interface), (SvObject) interface);

    if (prevInterface)
        QBNetworkSettingsContextInterfaceChanged(self, interface);
    else
        QBNetworkSettingsContextInterfaceAdded(self, interface);
}

SvLocal void
QBNetworkSettingsContextInterfaceRemoved_(SvObject self_, SvString interfaceID)
{
    QBNetworkSettingsContext self = (QBNetworkSettingsContext) self_;

    QBNetworkInterface interface = (QBNetworkInterface) SvHashTableFind(self->idToInterface, (SvObject) interfaceID);
    if (interface) {
        QBNetworkSettingsContextInterfaceRemoved(self, QBNetworkInterfaceGetID(interface));
        SvHashTableRemove(self->idToInterface, (SvObject) interfaceID);
    }
}

SvLocal void
QBNetworkSettingsContextInterfaceStateChanged_(SvObject self_, SvString interfaceID)
{
    QBNetworkSettingsContext self = (QBNetworkSettingsContext) self_;
    QBNetworkInterface interface = (QBNetworkInterface) SvHashTableFind(self->idToInterface, (SvObject) interfaceID);

    if (interface) {
        if (QBNetworkInterfaceGetStatus(interface) == QBNetworkManagerInterfaceStatus_connected &&
            QBNetworkInterfaceGetConfig(interface) != QBNetworkManagerInterfaceConfig_normal) {
            SvString title = SvStringCreateWithFormat(gettext("Incorrect network settings (%s)"), SvStringCString(interfaceID));
            char *message = gettext("Restored last working network configuration.");
            QBNetworkSettingsContextErrorDialogShow(self, self->settingsCtx, SvStringCString(title), message);
            SVRELEASE(title);
        }

        QBNetworkSettingsContextInterfaceChanged(self, interface);
    }
}

SvLocal void
QBNetworkSettingsClearVPNParams(QBNetworkSettingsContext self)
{
    svLabelSetText(self->vpnLabels.vpnStatus, gettext("disabled"));
    svLabelSetText(self->vpnLabels.interfaceID, gettext("N/A"));
    svLabelSetText(self->vpnLabels.localIP, gettext("N/A"));
    svLabelSetText(self->vpnLabels.localPort, gettext("N/A"));
    svLabelSetText(self->vpnLabels.vpnServerIP, gettext("N/A"));
    svLabelSetText(self->vpnLabels.vpnServerPort, gettext("N/A"));
    svLabelSetText(self->vpnLabels.vpnIP, gettext("N/A"));
    svLabelSetText(self->vpnLabels.vpnNetmask, gettext("N/A"));
    svLabelSetText(self->vpnLabels.vpnDNS, gettext("N/A"));
}

SvLocal void
QBNetworkSettingsFillLabelWithInteger(SvWidget label, int value)
{
    SvString valueStr = SvStringCreateWithFormat("%d", value);
    svLabelSetText(label, valueStr ? SvStringGetCString(valueStr) : gettext("N/A"));
    SVRELEASE(valueStr);
}

SvLocal void
QBNetworkSettingsFillVPNStatus(QBNetworkSettingsContext self, QBIPSecVPN secVPN)
{
    if (self->vpnLabels.vpnStatus) {
        char *label = NULL;
        QBNetworkManagerIPSecVPNStatus status = QBIPSecVPNGetStatus(secVPN);
        switch (status) {
            case QBNetworkManagerIPSecVPNStatus_disabled:
                svLabelSetText(self->vpnLabels.vpnStatus, gettext("disabled"));
                break;
            case QBNetworkManagerIPSecVPNStatus_disconnected:
                svLabelSetText(self->vpnLabels.vpnStatus, gettext("disconnected"));
                break;
            case QBNetworkManagerIPSecVPNStatus_connecting:
                asprintf(&label, gettext("connecting (%d)"), QBIPSecVPNGetConnectionCounter(secVPN));
                svLabelSetText(self->vpnLabels.vpnStatus, label);
                free(label);
                break;
            case QBNetworkManagerIPSecVPNStatus_connected:
                svLabelSetText(self->vpnLabels.vpnStatus, "connected");
                break;
        }
    }
}

SvLocal void
QBNetworkSettingsFillVPNParams(QBNetworkSettingsContext self)
{
    QBIPSecVPN secVPN = NULL;
    secVPN = QBNetworkMonitorGetIPSecVPN(self->networkMonitor);
    if (!secVPN) {
        QBNetworkSettingsClearVPNParams(self);
        return;
    }

    QBNetworkSettingsFillVPNStatus(self, secVPN);

    if (self->vpnLabels.interfaceID) {
        SvString interfaceID = QBIPSecVPNGetInterfaceID(secVPN);
        svLabelSetText(self->vpnLabels.interfaceID, interfaceID ? SvStringGetCString(interfaceID) : gettext("N/A"));
    }

    if (self->vpnLabels.localIP) {
        SvString localIP = QBIPSecVPNGetLocalIP(secVPN);
        svLabelSetText(self->vpnLabels.localIP, localIP ? SvStringGetCString(localIP) : gettext("N/A"));
    }

    if (self->vpnLabels.localPort) {
        int localPort = QBIPSecVPNGetLocalPort(secVPN);
        QBNetworkSettingsFillLabelWithInteger(self->vpnLabels.localPort, localPort);
    }

    if (self->vpnLabels.vpnServerIP) {
        SvString vpnServerIP = QBIPSecVPNGetVPNServerIP(secVPN);
        svLabelSetText(self->vpnLabels.vpnServerIP, vpnServerIP ? SvStringGetCString(vpnServerIP) : gettext("N/A"));
    }

    if (self->vpnLabels.vpnServerPort) {
        int vpnServerPort = QBIPSecVPNGetVPNServerPort(secVPN);
        QBNetworkSettingsFillLabelWithInteger(self->vpnLabels.vpnServerPort, vpnServerPort);
    }

    if (self->vpnLabels.vpnIP) {
        SvString vpnIP = QBIPSecVPNGetVPNIP(secVPN);
        svLabelSetText(self->vpnLabels.vpnIP, vpnIP ? SvStringGetCString(vpnIP) : gettext("N/A"));
    }

    if (self->vpnLabels.vpnNetmask) {
        SvString vpnNetmask = QBIPSecVPNGetVPNNetmask(secVPN);
        svLabelSetText(self->vpnLabels.vpnNetmask, vpnNetmask ? SvStringGetCString(vpnNetmask) : gettext("N/A"));
    }

    if (self->vpnLabels.vpnDNS) {
        SvString vpnDNS = QBIPSecVPNGetVPNDNS(secVPN);
        svLabelSetText(self->vpnLabels.vpnDNS, vpnDNS ? SvStringGetCString(vpnDNS) : gettext("N/A"));
    }
}

SvLocal SvWidget
QBNetworkSettingsCreateVPNRow(QBNetworkSettingsContext self, const char *keyLabelName, const char *valueLabelName)
{
    SvWidget keylabel = svLabelNewFromSM(self->res, keyLabelName);
    svSettingsWidgetAttach(self->super_.window, keylabel, keyLabelName, 1);
    SvWidget valueLabel = svLabelNewFromSM(self->res, valueLabelName);
    svSettingsWidgetAttach(self->super_.window, valueLabel, valueLabelName, 1);
    return valueLabel;
}

SvLocal void
QBNetworkSettingsAddVPNParamIfDefined(QBNetworkSettingsContext self, SvWidget *dest,
                                      const char *keyLabelName, const char *valueLabelName)
{
    if (svSettingsIsWidgetDefined(keyLabelName) && svSettingsIsWidgetDefined(valueLabelName)) {
        *dest = QBNetworkSettingsCreateVPNRow(self, keyLabelName, valueLabelName);
    }
}

SvLocal void
QBNetworkSettingsAddVPNParams(QBNetworkSettingsContext self)
{
    QBNetworkSettingsAddVPNParamIfDefined(self, &self->vpnLabels.vpnStatus,
                                          "NetworkSettingsWindow.VPNStatus",
                                          "NetworkSettingsWindow.VPNStatusValue");
    QBNetworkSettingsAddVPNParamIfDefined(self, &self->vpnLabels.interfaceID,
                                          "NetworkSettingsWindow.InterfaceID",
                                          "NetworkSettingsWindow.InterfaceIDValue");
    QBNetworkSettingsAddVPNParamIfDefined(self, &self->vpnLabels.localIP,
                                          "NetworkSettingsWindow.LocalIP",
                                          "NetworkSettingsWindow.LocalIPValue");
    QBNetworkSettingsAddVPNParamIfDefined(self, &self->vpnLabels.localPort,
                                          "NetworkSettingsWindow.LocalPort",
                                          "NetworkSettingsWindow.LocalPortValue");
    QBNetworkSettingsAddVPNParamIfDefined(self, &self->vpnLabels.vpnServerIP,
                                          "NetworkSettingsWindow.VPNServerIP",
                                          "NetworkSettingsWindow.VPNServerIPValue");
    QBNetworkSettingsAddVPNParamIfDefined(self, &self->vpnLabels.vpnServerPort,
                                          "NetworkSettingsWindow.VPNServerPort",
                                          "NetworkSettingsWindow.VPNServerPortValue");
    QBNetworkSettingsAddVPNParamIfDefined(self, &self->vpnLabels.vpnIP,
                                          "NetworkSettingsWindow.VPNIP",
                                          "NetworkSettingsWindow.VPNIPValue");
    QBNetworkSettingsAddVPNParamIfDefined(self, &self->vpnLabels.vpnNetmask,
                                          "NetworkSettingsWindow.VPNNetmask",
                                          "NetworkSettingsWindow.VPNNetmaskValue");
    QBNetworkSettingsAddVPNParamIfDefined(self, &self->vpnLabels.vpnDNS,
                                          "NetworkSettingsWindow.VPNDNS",
                                          "NetworkSettingsWindow.VPNDNSValue");

    QBNetworkSettingsFillVPNParams(self);
}

SvLocal void
QBNetworkSettingsContextVPNStateChanged_(SvObject self_)
{
    QBNetworkSettingsFillVPNParams((QBNetworkSettingsContext) self_);
}

SvLocal void
QBNetworkSettingsContextUserEventHandler(SvWidget w, SvWidgetId sender, SvUserEvent e)
{
    QBNetworkSettingsContext self = w->prv;

    if (e->code == SV_EVENT_BUTTON_PUSHED) {
        SvObject options = NULL;

        SvString interfaceId = QBNetworkSettingsContextGetIdOfFocusedInterface(self);
        if (interfaceId) {
            QBNetworkInterface interface = (QBNetworkInterface) SvHashTableFind(self->idToInterface, (SvObject) interfaceId);

            svSettingsRestoreContext(self->settingsCtx);

            if (SvObjectIsInstanceOf((SvObject) interface, QBEthernetInterface_getType())) {
                if (QBNetworkInterfaceGetStatus(interface) >= QBNetworkManagerInterfaceStatus_obtainingIP)
                    options = (SvObject) QBNetworkSettingsAdvancedPaneCreate(self->res,
                                                                             self->textRenderer,
                                                                             self->networkMonitor,
                                                                             interface,
                                                                             self->sidemenu.ctx,
                                                                             1);
            } else if (SvObjectIsInstanceOf((SvObject) interface, QBWiFiInterface_getType())) {
                QBNetworkMonitorRequestWiFiScanning(self->networkMonitor, QBNetworkInterfaceGetID(interface));
                options = (SvObject) QBWiFiNetworkListPaneCreate(self->res,
                                                                 self->scheduler,
                                                                 self->textRenderer,
                                                                 self->controller,
                                                                 self->networkMonitor,
                                                                 (QBWiFiInterface) interface,
                                                                 self->sidemenu.ctx,
                                                                 1);
            } else if (SvObjectIsInstanceOf((SvObject) interface, QBMoCaInterface_getType())) {
            } else if (SvObjectIsInstanceOf((SvObject) interface, QBDocsisInterface_getType())) {
            }

            svSettingsPopComponent();

            if (options) {
                QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) options);
                QBContextMenuShow(self->sidemenu.ctx);
                SVRELEASE(options);
            }

            return;
        }
        if (sender == svWidgetGetId(self->okBtn)) {
            QBApplicationControllerPopContext(self->controller);
            return;
        }
        if (sender == svWidgetGetId(self->routingBtn)) {
            options = (SvObject) QBRoutingSettingsListPaneCreateFromSettings("RoutingSettingsPane.settings",
                                                                             self->res,
                                                                             self->textRenderer,
                                                                             self->networkMonitor,
                                                                             self->availInterfacesArray,
                                                                             self->sidemenu.ctx,
                                                                             NULL);
            if (options) {
                QBContextMenuShow(self->sidemenu.ctx);
                QBContextMenuPushPane(self->sidemenu.ctx, options);
                SVRELEASE(options);
            }
            return;
        }
    }
}

SvLocal bool
QBNetworkSettingsContextInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBNetworkSettingsContext self = w->prv;

    if (e->ch == QBKEY_FUNCTION) {
        SvObject options = NULL;

        SvString interfaceId = QBNetworkSettingsContextGetIdOfFocusedInterface(self);
        if (interfaceId) {
            QBNetworkInterface interface = (QBNetworkInterface) SvHashTableFind(self->idToInterface, (SvObject) interfaceId);

            svSettingsRestoreContext(self->settingsCtx);

            if (SvObjectIsInstanceOf((SvObject) interface, QBEthernetInterface_getType())) {
                if (QBNetworkInterfaceGetStatus(interface) >= QBNetworkManagerInterfaceStatus_obtainingIP)
                    options = (SvObject) QBNetworkSettingsAdvancedPaneCreate(self->res,
                                                                             self->textRenderer,
                                                                             self->networkMonitor,
                                                                             interface,
                                                                             self->sidemenu.ctx,
                                                                             1);
            } else if (SvObjectIsInstanceOf((SvObject) interface, QBWiFiInterface_getType())) {
                options = (SvObject) QBWiFiManualSettingsPaneCreate(self->res,
                                                                    self->scheduler,
                                                                    self->textRenderer,
                                                                    self->controller,
                                                                    self->networkMonitor,
                                                                    (QBWiFiInterface) interface,
                                                                    self->sidemenu.ctx,
                                                                    1);
            } else if (SvObjectIsInstanceOf((SvObject) interface, QBMoCaInterface_getType())) {
            } else if (SvObjectIsInstanceOf((SvObject) interface, QBDocsisInterface_getType())) {
            }

            svSettingsPopComponent();

            if (options) {
                QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) options);
                QBContextMenuShow(self->sidemenu.ctx);
                SVRELEASE(options);
            }
            return true;
        }
    } else
        return svWidgetInputEventHandler(w, e);

    return false;
}

SvLocal void
QBNetworkSettingsContextAttachButtons(QBNetworkSettingsContext self)
{
    if (!self->window)
        return;

    svSettingsRestoreContext(self->settingsCtx);
    InterfaceButton bwi;
    SvIterator it = QBSortedListIterator(self->buttons);
    while ((bwi = (InterfaceButton) SvIteratorGetNext(&it))) {
        SvWidget button = bwi->button;
        if (svWidgetIsWindow(button)) {
            svButtonSetNotificationTarget(button, svWidgetGetId(self->window));
            svSettingsWidgetAttach(self->window, button, "NetworkSettingsWindow.interface", 1);
        }
    }
    svSettingsPopComponent();
    QBNetworkSettingsContextButtonsRealign(self);
}

SvLocal void
QBNetworkSettingsContextDetachButtons(QBNetworkSettingsContext self)
{
    InterfaceButton bwi;
    SvIterator it = QBSortedListIterator(self->buttons);
    while ((bwi = (InterfaceButton) SvIteratorGetNext(&it))) {
        svWidgetDetach(bwi->button);
    }
}

SvLocal void
QBNetworkSettingsContextFakeClean(SvApplication app, void *prv)
{
}

SvLocal void
QBNetworkSettingsContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBNetworkSettingsContext self = (QBNetworkSettingsContext) self_;

    svSettingsRestoreContext(self->settingsCtx);

    SvWidget window = svSettingsWidgetCreate(app, "NetworkSettingsWindow");

    SvWidget windowTitle = svLabelNewFromSM(app, "NetworkSettingsWindow.Title");
    svSettingsWidgetAttach(window, windowTitle, "NetworkSettingsWindow.Title", 1);

    self->window = window;

    window->prv = self;
    svWidgetSetInputEventHandler(window, QBNetworkSettingsContextInputEventHandler);
    svWidgetSetUserEventHandler(window, QBNetworkSettingsContextUserEventHandler);
    window->clean = QBNetworkSettingsContextFakeClean;

    self->super_.window = window;
    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(self->initLogic);
    if (logoWidgetName)
        QBCustomerLogoAttach(self->super_.window, logoWidgetName, 1);

    self->hintLabel = QBAsyncLabelNew(self->res, "NetworkSettingsWindow.desc", self->textRenderer);
    svSettingsWidgetAttach(self->window, self->hintLabel, "NetworkSettingsWindow.desc", 1);

    self->okBtn = svButtonNewFromSM(self->res, "NetworkSettingsWindow.done", NULL, 0, svWidgetGetId(self->window));
    svSettingsWidgetAttach(self->window, self->okBtn, "NetworkSettingsWindow.done", 1);

    self->routingBtn = svButtonNewFromSM(self->res, "NetworkSettingsWindow.routing", NULL, 0, svWidgetGetId(self->window));
    svSettingsWidgetAttach(self->window, self->routingBtn, "NetworkSettingsWindow.routing", 1);

    QBNetworkManagerVPNType vpnType = QBNetworkMonitorGetVPNType(self->networkMonitor);

    if (vpnType != QBNetworkManagerVPNType_none)
        QBNetworkSettingsAddVPNParams(self);

    svSettingsPopComponent();

    QBNetworkSettingsContextAttachButtons(self);
    QBNetworkSettingsContextHandleConnectionState(self);
}

SvLocal void
QBNetworkSettingsContextDestroyWindow(QBWindowContext self_)
{
    QBNetworkSettingsContext self = (QBNetworkSettingsContext) self_;

    QBNetworkSettingsContextDetachButtons(self);

    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
    self->window = NULL;
}

SvLocal void
QBNetworkSettingsContextDestroy(void* self_)
{
    QBNetworkSettingsContext self = (QBNetworkSettingsContext) self_;

    QBNetworkMonitorRemoveListener(self->networkMonitor, (SvObject) self);

    SVRELEASE(self->initLogic);
    SVRELEASE(self->controller);
    SVRELEASE(self->textRenderer);
    SVRELEASE(self->networkMonitor);
    SVRELEASE(self->availInterfacesArray);

    SVTESTRELEASE(self->sidemenu.ctx);
    SVRELEASE(self->buttons);
    SVRELEASE(self->idToInterface);
}

SvType
QBNetworkSettingsContext_getType(void)
{
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_        = {
            .destroy   = QBNetworkSettingsContextDestroy
        },
        .createWindow  = QBNetworkSettingsContextCreateWindow,
        .destroyWindow = QBNetworkSettingsContextDestroyWindow
    };

    static const struct QBNetworkMonitorListener_ networkMonitorMethods = {
        .interfaceAdded        = QBNetworkSettingsContextInterfaceAdded_,
        .interfaceRemoved      = QBNetworkSettingsContextInterfaceRemoved_,
        .interfaceStateChanged = QBNetworkSettingsContextInterfaceStateChanged_,
        .ipSecVPNStateChanged  = QBNetworkSettingsContextVPNStateChanged_,
    };

    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBNetworkSettingsContext",
                            sizeof(struct QBNetworkSettingsContext_),
                            QBWindowContext_getType(), &type,
                            QBNetworkMonitorListener_getInterface(), &networkMonitorMethods,
                            QBWindowContext_getType(), &contextVTable,
                            NULL);
    }

    return type;
}

SvLocal int
QBNetworkSettingsContextNetworkNameCompareFn(void *prv, SvObject firstInterfaceButton_, SvObject secondInterfaceButton_)
{
    InterfaceButton bwiFirst = (InterfaceButton) firstInterfaceButton_;
    InterfaceButton bwiSecond = (InterfaceButton) secondInterfaceButton_;
    return strcasecmp(SvStringCString(bwiFirst->ifId), SvStringCString(bwiSecond->ifId));
}


QBWindowContext
QBNetworkSettingsContextCreate(SvApplication res,
                               SvScheduler scheduler,
                               QBInitLogic initLogic,
                               QBApplicationController controller,
                               QBTextRenderer textRenderer,
                               QBNetworkMonitor networkMonitor,
                               bool isStrict)
{
    QBNetworkSettingsContext self = (QBNetworkSettingsContext) SvTypeAllocateInstance(QBNetworkSettingsContext_getType(), NULL);
    svSettingsPushComponent("NetworkSettingsWindow.settings");
    self->settingsCtx = svSettingsSaveContext();
    svSettingsPopComponent();
    self->res = res;
    self->scheduler = scheduler;
    self->initLogic = SVRETAIN(initLogic);
    self->controller = SVRETAIN(controller);
    self->textRenderer = SVRETAIN(textRenderer);
    self->networkMonitor = SVRETAIN(networkMonitor);
    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", self->controller, self->res, SVSTRING("SideMenu"));
    self->buttons = QBSortedListCreateWithCompareFn(QBNetworkSettingsContextNetworkNameCompareFn, self, NULL);
    self->idToInterface = SvHashTableCreate(7, NULL);
    self->availInterfacesArray = QBActiveArrayCreate(7, NULL);
    self->isStrict = isStrict;

    QBNetworkMonitorAddListener(self->networkMonitor, (SvObject) self);
    QBNetworkMonitorRecallInterfaces(self->networkMonitor, (SvObject) self);

    return (QBWindowContext) self;
}
