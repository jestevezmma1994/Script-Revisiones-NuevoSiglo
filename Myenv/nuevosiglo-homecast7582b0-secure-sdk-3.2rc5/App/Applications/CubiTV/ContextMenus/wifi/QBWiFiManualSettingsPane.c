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

#include "QBWiFiManualSettingsPane.h"
#include "utils.h"
#include <ContextMenus/QBNetworkSettingsAdvancedPane.h>
#include <ContextMenus/QBBasicPane.h>
#include <QBNetworkMonitor/QBWiFiNetwork.h>
#include <QBNetworkMonitor.h>
#include <QBOSK/QBOSKKey.h>
#include <settings.h>
#include <libintl.h>

#define WEP_SUPPORTED 0

typedef struct QBWiFiManualPane_ {
    struct SvObject_ super_;

    SvApplication res;
    SvScheduler scheduler;
    QBTextRenderer textRenderer;
    QBApplicationController controller;
    QBNetworkMonitor networkMonitor;
    QBBasicPane pane;
    int settingsCtx;
    SvString ssid;
    QBBasicPaneItem ssidItem;
    QBBasicPaneItem securityItem;
    QBBasicPaneItem saveItem;
    QBContextMenu ctxMenu;
    QBNetworkManagerWiFiNetworkSecurity security;
    QBWiFiNetwork manualWiFiNetwork;
    QBWiFiInterface wifiInterface;
} *QBWiFiManualPane;


SvLocal void
QBWiFiManualPaneDisconnectFromWiFiNetwork(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBWiFiManualPane self = self_;
    QBNetworkMonitorDisconnectFromWiFiNetwork(self->networkMonitor, QBNetworkInterfaceGetID((QBNetworkInterface) self->wifiInterface));
    QBContextMenuHide(self->ctxMenu, false);
}

SvLocal void
QBWiFiManualPaneForgetWiFiNetwork(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBWiFiManualPane self = self_;

    if (SvStringEqualToCString(id, "OK")) {
        SvString ssid = QBWiFiInterfaceGetSSID(self->wifiInterface);
        QBNetworkMonitorForgetWiFiNetwork(self->networkMonitor,
                                          QBNetworkInterfaceGetID((QBNetworkInterface) self->wifiInterface),
                                          ssid);
    }

    QBContextMenuHide(self->ctxMenu, false);
}

SvLocal void
QBWiFiManualPaneSecurityChosen(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBWiFiManualPane self = (QBWiFiManualPane) self_;
    if (SvObjectEquals((SvObject) item->id, (SvObject) SVSTRING("Open"))) {
        self->security = QBNetworkManagerWiFiNetworkSecurity_none;
        SVTESTRELEASE(self->securityItem->subcaption);
        self->securityItem->subcaption = SvStringCreate(gettext("Open"), NULL);
    }
#if WEP_SUPPORTED
    else if (SvObjectEquals((SvObject) item->id, (SvObject) SVSTRING("WEP"))) {
        self->security = QBNetworkManagerWiFiNetworkSecurity_WEP;
        SVTESTRELEASE(self->securityItem->subcaption);
        self->securityItem->subcaption = SvStringCreate(gettext("WEP"), NULL);
    }
#endif
    else if (SvObjectEquals((SvObject) item->id, (SvObject) SVSTRING("WPA/WPA2"))) {
        self->security = QBNetworkManagerWiFiNetworkSecurity_WPA;
        SVTESTRELEASE(self->securityItem->subcaption);
        self->securityItem->subcaption = SvStringCreate(gettext("WPA/WPA2"), NULL);
    }

    QBBasicPaneOptionPropagateObjectChange(self->pane, self->securityItem);
    QBContextMenuPopPane(self->ctxMenu);
}

SvLocal void
QBWiFiManualPanePasswordTyped(void *self_, QBOSKPane pane, SvString password, unsigned int layout, QBOSKKey key)
{
    QBWiFiManualPane self = (QBWiFiManualPane) self_;
    if (key->type == QBOSKKeyType_enter) {
        if (QBWiFiPasswordTyped(self->res,
                                self->controller,
                                self->networkMonitor,
                                self->wifiInterface,
                                self->manualWiFiNetwork,
                                password,
                                true,
                                self->settingsCtx)) {
            QBContextMenuHide(self->ctxMenu, false);
        }
    }
}

SvLocal void
QBWiFiManualPaneSSIDTyped(void *self_, QBOSKPane pane, SvString ssid, unsigned int layout, QBOSKKey key)
{
    QBWiFiManualPane self = (QBWiFiManualPane) self_;

    bool isSaveDisabled = self->saveItem->itemDisabled;
    if (key->type == QBOSKKeyType_enter && SvStringGetLength(ssid) > 0) {
        QBWiFiNetworkSetSSID(self->manualWiFiNetwork, ssid);
        SVTESTRELEASE(self->ssidItem->subcaption);
        self->ssidItem->subcaption = SVRETAIN(ssid);

        SVTESTRELEASE(self->ssid);
        self->ssid = SVRETAIN(ssid);

        QBBasicPaneOptionPropagateObjectChange(self->pane, self->ssidItem);
        QBContextMenuPopPane(self->ctxMenu);

        self->saveItem->itemDisabled = false;
        SVTESTRELEASE(self->saveItem->subcaption);
        self->saveItem->subcaption = NULL;
    } else {
        self->saveItem->itemDisabled = true;
        SVTESTRELEASE(self->saveItem->subcaption);
        self->saveItem->subcaption = SvStringCreate(gettext("SSID is unknown"), NULL);
    }

    if (isSaveDisabled != self->saveItem->itemDisabled)
        QBBasicPaneOptionPropagateObjectChange(self->pane, self->saveItem);
}

SvLocal void
QBWiFiManualPaneConnectNetwork(void *self_, SvString id_, QBBasicPane pane, QBBasicPaneItem item)
{
    QBWiFiManualPane self = (QBWiFiManualPane) self_;
    SvString id = QBNetworkInterfaceGetID((QBNetworkInterface) self->wifiInterface);
    QBNetworkMonitorConnectWiFiNetwork(self->networkMonitor,
                                       id,
                                       QBWiFiNetworkGetSSID(self->manualWiFiNetwork),
                                       QBWiFiNetworkGetPassword(self->manualWiFiNetwork),
                                       QBWiFiNetworkGetSecurityMode(self->manualWiFiNetwork),
                                       true);

    QBContextMenuHide(self->ctxMenu, false);
}

SvLocal void
QBWiFiManualPaneAddAndConnectNetwork(void *self_, SvString id_, QBBasicPane pane, QBBasicPaneItem item)
{
    if (item->itemDisabled)
        return;

    QBWiFiManualPane self = (QBWiFiManualPane) self_;
    QBWiFiNetworkSetSSID(self->manualWiFiNetwork, self->ssid);
    QBWiFiNetworkSetSecurityMode(self->manualWiFiNetwork, self->security);

    if (QBWiFiNetworkGetSecurityMode(self->manualWiFiNetwork) != QBNetworkManagerWiFiNetworkSecurity_none) {
        SvObject passwdInput = QBWiFiInputSideMenuCreate(self->scheduler, self->ctxMenu, SVSTRING("WiFiPasswordOSKPane"), "WIFIOSKPane.oskmap", 3, QBWiFiManualPanePasswordTyped, self);
        QBContextMenuPushPane(self->ctxMenu, passwdInput);
        SVRELEASE(passwdInput);
    } else {
        SvString id = QBNetworkInterfaceGetID((QBNetworkInterface) self->wifiInterface);
        QBNetworkMonitorConnectWiFiNetwork(self->networkMonitor,
                                           id,
                                           QBWiFiNetworkGetSSID(self->manualWiFiNetwork),
                                           QBWiFiNetworkGetPassword(self->manualWiFiNetwork),
                                           QBWiFiNetworkGetSecurityMode(self->manualWiFiNetwork),
                                           true);

        QBContextMenuHide(self->ctxMenu, false);
    }
}


SvLocal QBBasicPane
QBWiFiSettingsPaneCreate(QBWiFiManualPane manualPane, int level)
{
    QBBasicPane wifiSettingsPane = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(wifiSettingsPane, manualPane->res, manualPane->scheduler, manualPane->textRenderer, manualPane->ctxMenu, level, SVSTRING("BasicPane"));
    SvString optionStr;

    QBNetworkManagerInterfaceStatus status = QBNetworkInterfaceGetStatus((QBNetworkInterface) manualPane->wifiInterface);
    SvString ssid = QBWiFiInterfaceGetSSID(manualPane->wifiInterface);
    if (status >= QBNetworkManagerInterfaceStatus_obtainingLink) {
        optionStr = SvStringCreateWithFormat(gettext("Disconnect %s"), ssid ? SvStringCString(ssid) : gettext("unknown"));
        QBBasicPaneAddOption(wifiSettingsPane, SVSTRING("Disconnect"), optionStr, QBWiFiManualPaneDisconnectFromWiFiNetwork, manualPane);
        SVRELEASE(optionStr);
    }

    QBWiFiNetwork network = QBWiFiInterfaceGetWiFiNetwork(manualPane->wifiInterface, ssid);
    if (network && QBWiFiNetworkGetPassword(network)) {
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

        optionStr = SvStringCreateWithFormat(gettext("Forget %s"), SvStringCString(ssid));
        QBBasicPaneAddOptionWithConfirmation(wifiSettingsPane, SVSTRING("FORGET"), optionStr, SVSTRING("BasicPane"), options, ids, QBWiFiManualPaneForgetWiFiNetwork, manualPane);
        SVRELEASE(optionStr);
        SVRELEASE(options);
        SVRELEASE(ids);
    }

    return wifiSettingsPane;
}

SvLocal void
QBWiFiManualPaneShow(SvObject self_)
{
    QBWiFiManualPane self = (QBWiFiManualPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->pane, show);
}

SvLocal void
QBWiFiManualPaneHide(SvObject self_, bool immediately)
{
    QBWiFiManualPane self = (QBWiFiManualPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->pane, hide, immediately);
}

SvLocal void
QBWiFiManualPaneSetActive(SvObject self_)
{
    QBWiFiManualPane self = (QBWiFiManualPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->pane, setActive);
}

SvLocal bool
QBWiFiManualPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvLocal void
QBWiFiManualPaneDestroy(void *self_)
{
    QBWiFiManualPane self = self_;
    SVRELEASE(self->pane);
    SVTESTRELEASE(self->ssid);
    SVTESTRELEASE(self->manualWiFiNetwork);
    SVRELEASE(self->ctxMenu);
    SVRELEASE(self->wifiInterface);
    SVRELEASE(self->networkMonitor);
    SVRELEASE(self->controller);
    SVRELEASE(self->textRenderer);
}

SvLocal SvType
QBWiFiManualPane_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBWiFiManualPaneDestroy
    };

    static const struct QBContextMenuPane_ methods = {
        .show             = QBWiFiManualPaneShow,
        .hide             = QBWiFiManualPaneHide,
        .setActive        = QBWiFiManualPaneSetActive,
        .handleInputEvent = QBWiFiManualPaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBWiFiManualPane",
                            sizeof(struct QBWiFiManualPane_),
                            SvObject_getType(), &type,
                            QBContextMenuPane_getInterface(), &methods,
                            SvObject_getType(), &vtable,
                            NULL);
    }
    return type;
}


SvLocal void
QBWiFiManualPaneInit(QBWiFiManualPane self, int level)
{
    self->pane = QBBasicPaneCreateFromSettings("BasicPane.settings", self->res, self->scheduler, self->textRenderer, self->ctxMenu, level, SVSTRING("BasicPane"));

    SvString translated;

    SVTESTRELEASE(self->manualWiFiNetwork);
    self->manualWiFiNetwork = SVTESTRETAIN(QBNetworkMonitorGetManualWiFiNetwork(self->networkMonitor));
    SvString ssid = self->manualWiFiNetwork ? QBWiFiNetworkGetSSID(self->manualWiFiNetwork) : NULL;

    if (ssid) {
        QBNetworkManagerInterfaceStatus status = QBNetworkInterfaceGetStatus((QBNetworkInterface) self->wifiInterface);
        SvString currentSSID = QBWiFiInterfaceGetSSID(self->wifiInterface);
        if (status == QBNetworkManagerInterfaceStatus_disconnected || !SvObjectEquals((SvObject) ssid, (SvObject) currentSSID)) {
            translated = SvStringCreateWithFormat("%s %s", gettext("Connect"), SvStringCString(ssid));
            QBBasicPaneAddOption(self->pane, SVSTRING("Manual save"), translated, QBWiFiManualPaneConnectNetwork, self);
            SVRELEASE(translated);
        } else {
            translated = SvStringCreateWithFormat("%s %s", gettext("Disconnect"), SvStringCString(ssid));
            QBBasicPaneAddOption(self->pane, SVSTRING("Manual save"), translated, QBWiFiManualPaneDisconnectFromWiFiNetwork, self);
            SVRELEASE(translated);
        }
    } else {
        self->manualWiFiNetwork = QBWiFiNetworkCreate(self->wifiInterface, NULL, NULL, NULL, 0, QBWiFiNetwork_disconnected, QBNetworkManagerWiFiNetworkSecurity_none);
    }

    SvObject ssidInput = QBWiFiInputSideMenuCreate(self->scheduler, self->ctxMenu, SVSTRING("WiFiSSIDOSKPane"), "WIFIOSKPane.oskmap", level + 1, QBWiFiManualPaneSSIDTyped, self);
    SvObject security = SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit((QBBasicPane) security, self->res, self->scheduler, self->textRenderer, self->ctxMenu, level + 1, SVSTRING("BasicPane"));

    translated = SvStringCreate(gettext("Open"), NULL);
    QBBasicPaneAddOption((QBBasicPane) security, SVSTRING("Open"), translated, QBWiFiManualPaneSecurityChosen, self);
    SVRELEASE(translated);
#if WEP_SUPPORTED
    translated = SvStringCreate(gettext("WEP"), NULL);
    QBBasicPaneAddOption((QBBasicPane) security, SVSTRING("WEP"), translated, QBWiFiManualPaneSecurityChosen, self);
    SVRELEASE(translated);
#endif
    translated = SvStringCreate(gettext("WPA/WPA2"), NULL);
    QBBasicPaneAddOption((QBBasicPane) security, SVSTRING("WPA/WPA2"), translated, QBWiFiManualPaneSecurityChosen, self);
    SVRELEASE(translated);
    translated = SvStringCreate(gettext("WiFi Session ID"), NULL);
    self->ssidItem = QBBasicPaneAddOptionWithSubpane(self->pane, SVSTRING("Manual ssid"), translated, ssidInput);
    SVRELEASE(translated);
    translated = SvStringCreate(gettext("WiFi Security"), NULL);
    self->securityItem = QBBasicPaneAddOptionWithSubpane(self->pane, SVSTRING("Manual security"), translated, security);
    self->securityItem->subcaption = SvStringCreate(gettext("Open"), NULL);
    QBBasicPaneOptionPropagateObjectChange(self->pane, self->securityItem);
    SVRELEASE(translated);
    translated = SvStringCreate(gettext("Add & Connect"), NULL);
    SvString subcaption = SvStringCreate(gettext("SSID is unknown!"), NULL);
    self->saveItem = QBBasicPaneAddOptionWithSubcaption(self->pane, SVSTRING("Manual save"), translated, subcaption, QBWiFiManualPaneAddAndConnectNetwork, self);
    SVRELEASE(subcaption);
    self->saveItem->itemDisabled = true;
    QBBasicPaneOptionPropagateObjectChange(self->pane, self->saveItem);

    SVRELEASE(translated);
    SVRELEASE(ssidInput);
    SVRELEASE(security);
}

SvLocal QBWiFiManualPane
QBWiFiManualPaneCreate(SvApplication res,
                       SvScheduler scheduler,
                       QBTextRenderer textRenderer,
                       QBApplicationController controller,
                       QBNetworkMonitor networkMonitor,
                       QBWiFiInterface wifiInterface,
                       QBContextMenu ctxMenu,
                       int level)
{
    QBWiFiManualPane self = (QBWiFiManualPane)
                            SvTypeAllocateInstance(QBWiFiManualPane_getType(), NULL);

    self->settingsCtx = svSettingsSaveContext();
    self->res = res;
    self->scheduler = scheduler;
    self->textRenderer = SVRETAIN(textRenderer);
    self->controller = SVRETAIN(controller);
    self->networkMonitor = SVRETAIN(networkMonitor);
    self->ctxMenu = SVRETAIN(ctxMenu);
    self->wifiInterface = SVRETAIN(wifiInterface);

    QBWiFiManualPaneInit(self, level);
    return self;
}

QBBasicPane
QBWiFiManualSettingsPaneCreate(SvApplication res,
                               SvScheduler scheduler,
                               QBTextRenderer textRenderer,
                               QBApplicationController controller,
                               QBNetworkMonitor networkMonitor,
                               QBWiFiInterface wifiInterface,
                               QBContextMenu ctxMenu,
                               int level)
{
    QBWiFiManualPane wifiManualPane = QBWiFiManualPaneCreate(res,
                                                             scheduler,
                                                             textRenderer,
                                                             controller,
                                                             networkMonitor,
                                                             wifiInterface,
                                                             ctxMenu,
                                                             level + 1);

    QBBasicPane wifiSettingsPane = QBWiFiSettingsPaneCreate(wifiManualPane, level);

    QBNetworkManagerInterfaceStatus status = QBNetworkInterfaceGetStatus((QBNetworkInterface) wifiInterface);
    if (status >= QBNetworkManagerInterfaceStatus_obtainingIP) {
        QBNetworkSettingsAdvancedPane advancedPane = QBNetworkSettingsAdvancedPaneCreate(res,
                                                                                         textRenderer,
                                                                                         networkMonitor,
                                                                                         (QBNetworkInterface) wifiInterface,
                                                                                         ctxMenu,
                                                                                         level + 1);

        SvString optionStr = SvStringCreate(gettext("Advanced..."), NULL);
        QBBasicPaneAddOptionWithSubpane(wifiSettingsPane, SVSTRING("advanced"), optionStr, (SvObject) advancedPane);
        SVRELEASE(optionStr);

        SVRELEASE(advancedPane);
    }

    SvString optionStr = SvStringCreate(gettext("Connect to hidden Wi-Fi..."), NULL);

    QBBasicPaneAddOptionWithSubpane(wifiSettingsPane, SVSTRING("hidden"), optionStr, (SvObject) wifiManualPane);
    SVRELEASE(optionStr);

    SVRELEASE(wifiManualPane);

    return wifiSettingsPane;
}
