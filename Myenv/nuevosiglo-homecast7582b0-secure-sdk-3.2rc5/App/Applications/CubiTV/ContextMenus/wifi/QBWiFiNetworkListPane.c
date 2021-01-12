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

#include "QBWiFiNetworkListPane.h"
#include "utils.h"
#include <ContextMenus/QBNetworkSettingsAdvancedPane.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBListPane.h>
#include <ContextMenus/QBWiFiNetworkChoiceMenu.h>
#include <QBNetworkMonitor/QBWiFiNetwork.h>
#include <QBNetworkMonitor.h>
#include <QBOSK/QBOSKKey.h>
#include <QBWidgets/QBDialog.h>
#include <settings.h>
#include <libintl.h>
#include <main.h>

struct QBWiFiNetworkListPane_ {
    struct SvObject_ super_;

    SvApplication res;
    SvScheduler scheduler;
    QBTextRenderer textRenderer;
    QBApplicationController controller;
    QBNetworkMonitor networkMonitor;
    QBListPane pane;
    int settingsCtx;
    QBContextMenu ctxMenu;
    QBWiFiNetwork editedWiFiNetwork;
    QBWiFiInterface wifiInterface;
};

SvLocal void
QBWiFiNetworkListPaneDisconnectFromWiFiNetwork(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBWiFiNetworkListPane self = self_;
    QBNetworkMonitorDisconnectFromWiFiNetwork(self->networkMonitor, QBNetworkInterfaceGetID((QBNetworkInterface) self->wifiInterface));
    QBContextMenuHide(self->ctxMenu, false);
}

SvLocal void
QBWiFiNetworkListPaneForgetWiFiNetwork(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBWiFiNetworkListPane self = self_;

    if (SvStringEqualToCString(id, "OK"))
        QBNetworkMonitorForgetWiFiNetwork(self->networkMonitor,
                                          QBNetworkInterfaceGetID((QBNetworkInterface) self->wifiInterface),
                                          QBWiFiNetworkGetSSID(self->editedWiFiNetwork));

    QBContextMenuHide(self->ctxMenu, false);
}

SvLocal void
QBWiFiNetworkListPaneShow(SvObject self_)
{
    QBWiFiNetworkListPane self = (QBWiFiNetworkListPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->pane, show);
}

SvLocal void
QBWiFiNetworkListPaneHide(SvObject self_, bool immediately)
{
    QBWiFiNetworkListPane self = (QBWiFiNetworkListPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->pane, hide, immediately);
}

SvLocal void
QBWiFiNetworkListPaneSetActive(SvObject self_)
{
    QBWiFiNetworkListPane self = (QBWiFiNetworkListPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->pane, setActive);
}

SvLocal bool
QBWiFiNetworkListPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvLocal void
QBWiFiNetworkListPaneDestroy(void *self_)
{
    QBWiFiNetworkListPane self = self_;
    SVRELEASE(self->textRenderer);
    SVRELEASE(self->controller);
    SVRELEASE(self->networkMonitor);
    SVRELEASE(self->pane);
    SVTESTRELEASE(self->editedWiFiNetwork);
    SVRELEASE(self->ctxMenu);
    SVRELEASE(self->wifiInterface);
}

SvLocal SvType
QBWiFiNetworkListPane_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBWiFiNetworkListPaneDestroy
    };

    static const struct QBContextMenuPane_ methods = {
        .show             = QBWiFiNetworkListPaneShow,
        .hide             = QBWiFiNetworkListPaneHide,
        .setActive        = QBWiFiNetworkListPaneSetActive,
        .handleInputEvent = QBWiFiNetworkListPaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBWiFiNetworkListPane",
                            sizeof(struct QBWiFiNetworkListPane_),
                            SvObject_getType(), &type,
                            QBContextMenuPane_getInterface(), &methods,
                            SvObject_getType(), &vtable,
                            NULL);
    }
    return type;
}

SvLocal void
QBWiFiNetworkListPaneConnectToWiFiNetwork(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBWiFiNetworkListPane self = (QBWiFiNetworkListPane) self_;
    QBNetworkMonitorConnectWiFiNetwork(self->networkMonitor,
                                       QBNetworkInterfaceGetID((QBNetworkInterface) self->wifiInterface),
                                       QBWiFiNetworkGetSSID(self->editedWiFiNetwork),
                                       QBWiFiNetworkGetPassword(self->editedWiFiNetwork),
                                       QBWiFiNetworkGetSecurityMode(self->editedWiFiNetwork),
                                       false);

    QBContextMenuHide(self->ctxMenu, false);
}

SvLocal QBBasicPane
QBWiFiNetworkListPaneCreateWiFiNetworkSettings(QBWiFiNetworkListPane self, QBWiFiNetwork network)
{
    QBBasicPane wifiSettingsPane = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    svSettingsRestoreContext(self->settingsCtx);
    QBBasicPaneInit(wifiSettingsPane, self->res, self->scheduler, self->textRenderer, self->ctxMenu, 2, SVSTRING("BasicPane"));
    SvString optionStr;
    assert(SvObjectIsInstanceOf((SvObject) self->wifiInterface, QBWiFiInterface_getType()));
    QBNetworkManagerInterfaceStatus status = QBNetworkInterfaceGetStatus((QBNetworkInterface) self->wifiInterface);
    SvString ssid = QBWiFiInterfaceGetSSID(self->wifiInterface);
    if (status >= QBNetworkManagerInterfaceStatus_obtainingLink && SvObjectEquals((SvObject) ssid, (SvObject) QBWiFiNetworkGetSSID(network))) {
        optionStr = SvStringCreate(gettext("Disconnect"), NULL);
        QBBasicPaneAddOption(wifiSettingsPane, SVSTRING("Disconnect"), optionStr, QBWiFiNetworkListPaneDisconnectFromWiFiNetwork, self);
        SVRELEASE(optionStr);
    } else {
        optionStr = SvStringCreate(gettext("Connect"), NULL);
        QBBasicPaneAddOption(wifiSettingsPane, SVSTRING("Connect"), optionStr, QBWiFiNetworkListPaneConnectToWiFiNetwork, self);
        SVRELEASE(optionStr);
    }
    SvArray options = SvArrayCreate(NULL);
    SvArray ids = SvArrayCreate(NULL);
    optionStr = SvStringCreate(gettext("Yes"), NULL);
    SvArrayAddObject(options, (SvObject) optionStr);
    SVRELEASE(optionStr);
    SvArrayAddObject(ids, (SvObject) SVSTRING("OK"));
    optionStr = SvStringCreate(gettext("No"), NULL);
    SvArrayAddObject(options, (SvObject) optionStr);
    SVRELEASE(optionStr);
    SvArrayAddObject(ids, (SvObject) SVSTRING("CANCEL"));
    if (QBWiFiNetworkGetSecurityMode(network) != QBNetworkManagerWiFiNetworkSecurity_none && QBWiFiNetworkGetPassword(network)) {
        optionStr = SvStringCreate(gettext("Forget the network"), NULL);
        QBBasicPaneAddOptionWithConfirmation(wifiSettingsPane, SVSTRING("FORGET"), optionStr, SVSTRING("BasicPane"), options, ids, QBWiFiNetworkListPaneForgetWiFiNetwork, self);
        SVRELEASE(optionStr);
    }

    QBNetworkSettingsAdvancedPane advancedPane = QBNetworkSettingsAdvancedPaneCreate(self->res,
                                                                                     self->textRenderer,
                                                                                     self->networkMonitor,
                                                                                     (QBNetworkInterface) self->wifiInterface,
                                                                                     self->ctxMenu,
                                                                                     3);

    if (status >= QBNetworkManagerInterfaceStatus_obtainingLink && SvObjectEquals((SvObject) ssid, (SvObject) QBWiFiNetworkGetSSID(network))) {
        optionStr = SvStringCreate(gettext("Advanced..."), NULL);
        QBBasicPaneAddOptionWithSubpane(wifiSettingsPane, SVSTRING("Advanced..."), optionStr, (SvObject) advancedPane);
        SVRELEASE(optionStr);
    }
    SVRELEASE(advancedPane);
    SVRELEASE(options);
    SVRELEASE(ids);
    svSettingsPopComponent();

    return wifiSettingsPane;
}


SvLocal WiFiNetworkChoiceState
QBWiFiNetworkListPaneGetNetworkState(void *self_, SvObject network_)
{
    QBWiFiNetworkListPane self = (QBWiFiNetworkListPane) self_;
    QBWiFiNetwork network = (QBWiFiNetwork) network_;
    assert(SvObjectIsInstanceOf((SvObject) self->wifiInterface, QBWiFiInterface_getType()));
    QBNetworkManagerInterfaceStatus status = QBNetworkInterfaceGetStatus((QBNetworkInterface) self->wifiInterface);
    SvString currentSSID = QBWiFiInterfaceGetSSID(self->wifiInterface);
    SvString ssid = QBWiFiNetworkGetSSID(network);

    if (!SvObjectEquals((SvObject) currentSSID, (SvObject) ssid)) {
        if (QBWiFiNetworkGetPassword(network))
            return WiFiNetworkChoice_remembered;

        return WiFiNetworkChoice_disconnected;
    }

    if (status == QBNetworkManagerInterfaceStatus_disconnected) {
        if (QBWiFiNetworkGetPassword(network))
            return WiFiNetworkChoice_remembered;

        return WiFiNetworkChoice_disconnected;
    }

    if (status == QBNetworkManagerInterfaceStatus_obtainingLink) {
        return WiFiNetworkChoice_connecting;
    }


    return WiFiNetworkChoice_connected;
}

SvLocal WiFiNetworkChoiceSecurity
QBWiFiNetworkListPaneGetNetworkSecurity(void *self_, SvObject network_)
{
    QBWiFiNetwork network = (QBWiFiNetwork) network_;
    if (QBWiFiNetworkGetSecurityMode(network) != QBNetworkManagerWiFiNetworkSecurity_none)
        return WiFiNetworkChoice_secure;

    return WiFiNetworkChoice_open;

}

SvLocal QBWiFiNetworkSignalLevel
QBWiFiNetworkListPaneGetNetworkSignalLevel(void *self_, SvObject network_)
{
    if (!SvObjectIsInstanceOf(network_, QBWiFiNetwork_getType()))
        return 0;

    QBWiFiNetwork network = (QBWiFiNetwork) network_;
    return QBWiFiNetworkGetSignalLevelInHumanScale(network);
}

SvLocal SvString
QBWiFiNetworkListPaneCreateNetworkCaption(void *self_, SvObject network_)
{
    if (SvObjectIsInstanceOf((SvObject) network_, SvString_getType()))
        return SVRETAIN((SvObject) network_);
    QBWiFiNetwork network = (QBWiFiNetwork) network_;
    return SVRETAIN(QBWiFiNetworkGetSSID(network));
}

SvLocal SvString
QBWiFiNetworkListPaneCreateNetworkSubcaption(void *self_, SvObject network_)
{
    if (SvObjectIsInstanceOf((SvObject) network_, SvString_getType()))
        return NULL;
    QBWiFiNetwork network = (QBWiFiNetwork) network_;
    return SVTESTRETAIN(QBWiFiNetworkGetFlags(network));
}

SvLocal void
QBWiFiNetworkListPaneWiFiPasswordTyped(void *self_, QBOSKPane pane, SvString password, unsigned int layout, QBOSKKey key)
{
    QBWiFiNetworkListPane self = (QBWiFiNetworkListPane) self_;
    if (key->type == QBOSKKeyType_enter) {
        if (QBWiFiPasswordTyped(self->res,
                                self->controller,
                                self->networkMonitor,
                                self->wifiInterface,
                                self->editedWiFiNetwork,
                                password,
                                false,
                                self->settingsCtx)) {
            QBContextMenuHide(self->ctxMenu, false);
        }
    }
}

SvLocal void
QBWiFiNetworkListPaneNetworkSelected(void *self_, QBListPane pane, SvObject network_, int pos)
{
    QBWiFiNetworkListPane self = self_;
    SvObject options = NULL;

    if (SvObjectIsInstanceOf(network_, QBWiFiNetwork_getType())) {
        QBWiFiNetwork network = (QBWiFiNetwork) network_;

        SVTESTRELEASE(self->editedWiFiNetwork);
        self->editedWiFiNetwork = SVRETAIN(network);

        assert(SvObjectIsInstanceOf((SvObject) self->wifiInterface, QBWiFiInterface_getType()));
        QBNetworkManagerInterfaceStatus status = QBNetworkInterfaceGetStatus((QBNetworkInterface) self->wifiInterface);
        SvString ssid = QBWiFiNetworkGetSSID(self->editedWiFiNetwork);
        SvString currentSSID = QBWiFiInterfaceGetSSID(self->wifiInterface);

        if (!QBWiFiNetworkGetPassword(network) &&
            (status == QBNetworkManagerInterfaceStatus_disconnected || !SvObjectEquals((SvObject) ssid, (SvObject) currentSSID)) &&
            QBWiFiNetworkGetSecurityMode(network) != QBNetworkManagerWiFiNetworkSecurity_none) {
            options = QBWiFiInputSideMenuCreate(self->scheduler, self->ctxMenu, SVSTRING("WiFiPasswordOSKPane"), "WIFIOSKPane.oskmap", 2, QBWiFiNetworkListPaneWiFiPasswordTyped, self);
        } else {
            options = (SvObject) QBWiFiNetworkListPaneCreateWiFiNetworkSettings(self, network);
        }
    }

    if (options) {
        QBContextMenuShow(self->ctxMenu);
        QBContextMenuPushPane(self->ctxMenu, options);
        SVRELEASE(options);
    }
}


SvLocal void
QBWiFiNetworkListPaneNetworksReordered(void *self_, int prev, int next)
{
}

SvLocal SvObject
QBWiFiNetworkListPaneInit(QBWiFiNetworkListPane self, QBWiFiInterface wifiInterface, int level)
{
    static struct QBWiFiNetworkChoiceControllerCallbacks_ controllerCallbacks = {
        .getState             = QBWiFiNetworkListPaneGetNetworkState,
        .getSecurity          = QBWiFiNetworkListPaneGetNetworkSecurity,
        .getSignalLevel       = QBWiFiNetworkListPaneGetNetworkSignalLevel,
        .createNodeCaption    = QBWiFiNetworkListPaneCreateNetworkCaption,
        .createNodeSubcaption = QBWiFiNetworkListPaneCreateNetworkSubcaption,
    };
    QBWiFiNetworkChoiceController constructor = QBWiFiNetworkChoiceControllerCreateFromSettings("WiFiNetworkChoiceMenu.settings", self->textRenderer, self, &controllerCallbacks);

    static struct QBListPaneCallbacks_t listPaneCallbacks = {
        .selected  = QBWiFiNetworkListPaneNetworkSelected,
        .reordered = QBWiFiNetworkListPaneNetworksReordered,
    };
    QBSortedList availableSSIDs = QBWiFiInterfaceGetAvailableSSIDs(wifiInterface);

    self->pane = QBListPaneCreateFromSettings("WiFiNetworkChoiceMenu.settings", self->res, (SvObject) availableSSIDs, (SvObject) constructor, &listPaneCallbacks, self, self->ctxMenu, level, SVSTRING("ListPane"));

    SVRELEASE(constructor);

    return (SvObject) self->pane;
}


QBWiFiNetworkListPane
QBWiFiNetworkListPaneCreate(SvApplication res,
                            SvScheduler scheduler,
                            QBTextRenderer textRenderer,
                            QBApplicationController controller,
                            QBNetworkMonitor networkMonitor,
                            QBWiFiInterface wifiInterface,
                            QBContextMenu ctxMenu,
                            int level)
{
    QBWiFiNetworkListPane self = (QBWiFiNetworkListPane)
                                 SvTypeAllocateInstance(QBWiFiNetworkListPane_getType(), NULL);

    self->res = res;
    self->scheduler = scheduler;
    self->textRenderer = SVRETAIN(textRenderer);
    self->controller = SVRETAIN(controller);
    self->networkMonitor = SVRETAIN(networkMonitor);
    self->settingsCtx = svSettingsSaveContext();
    self->ctxMenu = SVRETAIN(ctxMenu);
    self->wifiInterface = SVRETAIN(wifiInterface);

    QBWiFiNetworkListPaneInit(self, wifiInterface, level);
    return self;
}
