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

#include "QBNetworkSettingsAdvancedWidget.h"

#include <main.h>
#include <QBWidgets/QBAsyncLabel.h>

#include <settings.h>
#include <QBInput/QBInputCodes.h>
#include <QBNetworkMonitor/QBNetworkInterface.h>
#include <QBNetworkMonitor/QBNetworkMonitorListener.h>

#include <QBWidgets/QBComboBox.h>
#include <SWL/input.h>
#include <SWL/button.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <SvCore/SvCoreErrorDomain.h>

#include <libintl.h>
#include <regex.h>


typedef struct QBNetworkSettingsAdvancedWidget_ {
    struct SvObject_ super_;

    SvApplication res;
    QBTextRenderer textRenderer;
    QBNetworkMonitor networkMonitor;
    int settingsCtx;
    char *widgetName;
    QBNetworkInterface networkInterface;

    void *callbackTarget;
    QBNetworkSettingsAdvancedWidgetOnFinishCallback callback;

    SvWidget networkAreaCombo;
    SvWidget addressProviderCombo;
    SvWidget ipInput;
    SvWidget netmaskInput;
    SvWidget gatewayInput, gatewayLabel;
    SvWidget dns1Input;
    SvWidget dns2Input;
    SvWidget saveButton;
    SvWidget cancelButton;
} *QBNetworkSettingsAdvancedWidget;

SvLocal int
QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(char *dst, const char *src, int n)
{
    #define NUM_MATCHES 16
    regex_t preg;
    regmatch_t pm[NUM_MATCHES];
    int err;
    const char *expr = "\\b(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b";
    assert(dst);
    assert(src);
    assert(expr);

    if (!(err = regcomp(&preg, expr, REG_EXTENDED))) {
        if (!(err = regexec(&preg, src, NUM_MATCHES, pm, 0)) && pm[n].rm_so > 0) {
            strncpy(dst, src + pm[n].rm_so, pm[n].rm_eo - pm[n].rm_so);
            dst[pm[n].rm_eo - pm[n].rm_so] = 0;
        }
        regfree(&preg);
        return err;
    }
    return err;
}

SvLocal void
QBNetworkSettingsAdvancedWidgetUserEventHandler(SvWidget w, SvWidgetId sender, SvUserEvent e)
{
    QBNetworkSettingsAdvancedWidget self = w->prv;

    if (svWidgetGetId(self->saveButton) == sender) {
        /* Change to DHCP/Static configuration */
        SvObject value = QBComboBoxGetValue(self->addressProviderCombo);
        if (SvObjectEquals(value, (SvObject) SVSTRING("DHCP"))) {
            QBNetworkInterfaceSetAddressProvider(self->networkInterface, QBNetworkManagerInterfaceAddressProvider_dhcp);
        } else if (SvObjectEquals(value, (SvObject) SVSTRING("static"))) {
            QBNetworkInterfaceSetAddressProvider(self->networkInterface, QBNetworkManagerInterfaceAddressProvider_static);
        } else {
            SvLogError("%s() : trying to save incorrect net configuration!", __func__);
            return;
        }

        value = QBComboBoxGetValue(self->networkAreaCombo);
        if (SvObjectEquals(value, (SvObject) SVSTRING("WAN"))) {
            QBNetworkInterfaceSetRoutingType(self->networkInterface, QBNetworkManagerInterfaceRouting_WAN_LAN);
        } else if (SvObjectEquals(value, (SvObject) SVSTRING("LAN"))) {
            QBNetworkInterfaceSetRoutingType(self->networkInterface, QBNetworkManagerInterfaceRouting_LAN);
        } else {
            SvLogError("%s() : trying to save incorrect area configuration!", __func__);
            return;
        }

        /* Set static network parameters */
        if (QBNetworkInterfaceGetAddressProvider(self->networkInterface) == QBNetworkManagerInterfaceAddressProvider_static) {
            char* ip_ = svInputGetText(self->ipInput);
            SvString ip = SvStringCreate(ip_, NULL);
            free(ip_);
            QBNetworkInterfaceSetIP(self->networkInterface, ip);
            SVRELEASE(ip);
            char* netmask_ = svInputGetText(self->netmaskInput);
            SvString netmask = SvStringCreate(netmask_, NULL);
            free(netmask_);
            QBNetworkInterfaceSetNetmask(self->networkInterface, netmask);
            SVRELEASE(netmask);
            if (QBNetworkInterfaceGetRoutingType(self->networkInterface) != QBNetworkManagerInterfaceRouting_LAN) {
                char* gateway_ = svInputGetText(self->gatewayInput);
                SvString gateway = SvStringCreate(gateway_, NULL);
                free(gateway_);
                QBNetworkInterfaceSetDefaultGateway(self->networkInterface, gateway);
                SVRELEASE(gateway);
            }
            char* dns1_ = svInputGetText(self->dns1Input);
            SvString dns1 = SvStringCreate(dns1_, NULL);
            free(dns1_);
            QBNetworkInterfaceSetDNS1(self->networkInterface, dns1);
            SVRELEASE(dns1);
            char* dns2_ = svInputGetText(self->dns2Input);
            SvString dns2 = SvStringCreate(dns2_, NULL);
            free(dns2_);
            QBNetworkInterfaceSetDNS2(self->networkInterface, dns2);
            SVRELEASE(dns2);
        }

        QBNetworkMonitorSaveNetworkParameters(self->networkMonitor, self->networkInterface);
    }

    if (self->callback) {
        self->callback(self->callbackTarget);
    }
}

SvLocal SvString
QBNetworkSettingsAdvancedWidgetComboPrepare(void *self_, SvWidget combobox, SvObject value)
{
    QBNetworkSettingsAdvancedWidget self = (QBNetworkSettingsAdvancedWidget) self_;
    SvString ret = NULL;
    if (combobox == self->networkAreaCombo) {
        if (SvObjectEquals(value, (SvObject) SVSTRING("WAN")))
            ret = SvStringCreate(gettext("Internet"), NULL);
        else if (SvObjectEquals(value, (SvObject) SVSTRING("LAN")))
            ret = SvStringCreate(gettext("Local"), NULL);
    }

    if (combobox == self->addressProviderCombo) {
        if (SvObjectEquals(value, (SvObject) SVSTRING("DHCP")))
            ret = SvStringCreate(gettext("Auto(DHCP)"), NULL);
        else if (SvObjectEquals(value, (SvObject) SVSTRING("static")))
            ret = SvStringCreate(gettext("Manual(Static)"), NULL);
        else if (SvObjectEquals(value, (SvObject) SVSTRING("PPP"))){
            ret = SvStringCreate(gettext("unknown"), NULL);
        }
    }

    return ret;
}

SvLocal bool
QBNetworkSettingsAdvancedWidgetComboInput(void *self_, SvWidget combobox, SvInputEvent e)
{
    QBNetworkSettingsAdvancedWidget self = (QBNetworkSettingsAdvancedWidget) self_;
    char buf[15];
    SvString gatewayAddr = QBNetworkInterfaceGetDefaultGateway(self->networkInterface);

    SvObject value = QBComboBoxGetValue(combobox);
    if (combobox == self->networkAreaCombo) {
        if (SvObjectEquals(value, (SvObject) SVSTRING("LAN"))) {
            svWidgetSetHidden(self->gatewayInput, true);
            svWidgetSetHidden(self->gatewayLabel, true);
        } else if (SvObjectEquals((SvObject) value, (SvObject) SVSTRING("WAN"))) {
            svWidgetSetHidden(self->gatewayInput, false);
            svWidgetSetHidden(self->gatewayLabel, false);
            if (gatewayAddr && !QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(buf, SvStringCString(gatewayAddr), 0)) {
                svInputSetText(self->gatewayInput, SvStringCString(gatewayAddr));
            }
        } else {
            SvLogWarning("Incorrect network area has just been chosen!");
        }
    } else if (combobox == self->addressProviderCombo) {
        if (SvObjectEquals(value, (SvObject) SVSTRING("DHCP"))) {
            SvString ipAddr = QBNetworkInterfaceGetIP(self->networkInterface);
            SvString netmaskAddr = QBNetworkInterfaceGetNetmask(self->networkInterface);
            SvString dns1Addr = QBNetworkInterfaceGetDNS1(self->networkInterface);
            SvString dns2Addr = QBNetworkInterfaceGetDNS2(self->networkInterface);

            if (ipAddr && !QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(buf, SvStringCString(ipAddr), 0))
                svInputSetText(self->ipInput, SvStringCString(ipAddr));
            if (netmaskAddr && !QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(buf, SvStringCString(netmaskAddr), 0))
                svInputSetText(self->netmaskInput, SvStringCString(netmaskAddr));
            if (gatewayAddr && !QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(buf, SvStringCString(gatewayAddr), 0))
                svInputSetText(self->gatewayInput, SvStringCString(gatewayAddr));
            if (dns1Addr && !QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(buf, SvStringCString(dns1Addr), 0))
                svInputSetText(self->dns1Input, SvStringCString(dns1Addr));
            if (dns2Addr && !QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(buf, SvStringCString(dns2Addr), 0)) {
                svInputSetText(self->dns2Input, SvStringCString(dns2Addr));
            }

            svInputSetDisabled(self->ipInput, true);
            svInputSetDisabled(self->netmaskInput, true);
            svInputSetDisabled(self->gatewayInput, true);
            svInputSetDisabled(self->dns1Input, true);
            svInputSetDisabled(self->dns2Input, true);
        } else if (SvObjectEquals(QBComboBoxGetValue(self->addressProviderCombo), (SvObject) SVSTRING("static"))) {
            svInputSetDisabled(self->ipInput, false);
            svInputSetDisabled(self->netmaskInput, false);
            svInputSetDisabled(self->gatewayInput, false);
            svInputSetDisabled(self->dns1Input, false);
            svInputSetDisabled(self->dns2Input, false);
        } else {
            SvLogWarning("Incorrect network conf has just been chosen!");
        }
    }
    return false;
}

SvLocal void QBNetworkSettingsAdvancedWidgetAddInputWidget(SvWidget widget,
                                                           bool useRatio,
                                                           SvWidget inputWidget,
                                                           SvWidget inputLabel,
                                                           SvString attrStr, bool show)
{
    char buf[15];
    memset(buf, 0, 15);

    if (attrStr && !QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(buf, SvStringCString(attrStr), 0))
        svInputSetText(inputWidget, SvStringCString(attrStr));

    if (useRatio) {
        svSettingsWidgetAttachWithRatio(widget, inputWidget, svWidgetGetName(inputWidget), 1);
        svSettingsWidgetAttachWithRatio(widget, inputLabel, svWidgetGetName(inputLabel), 1);
    } else {
        svSettingsWidgetAttach(widget, inputWidget, svWidgetGetName(inputWidget), 1);
        svSettingsWidgetAttach(widget, inputLabel, svWidgetGetName(inputLabel), 1);
    }

    if (!show) {
        svWidgetSetHidden(inputWidget, true);
        svWidgetSetHidden(inputLabel, true);
    }
}

SvLocal void
QBNetworkSettingsAdvancedWidgetDestroy(void *ptr)
{
    QBNetworkSettingsAdvancedWidget self = ptr;

    free(self->widgetName);
    SVRELEASE(self->textRenderer);
    SVRELEASE(self->networkMonitor);
    SVRELEASE(self->networkInterface);
}

SvLocal void
QBNetworkSettingsAdvancedWidgetSetRouting(QBNetworkSettingsAdvancedWidget self,
                                          QBNetworkManagerInterfaceRouting routing)
{
    switch (routing) {
        case QBNetworkManagerInterfaceRouting_WAN:
        case QBNetworkManagerInterfaceRouting_WAN_LAN:
            QBComboBoxSetValue(self->networkAreaCombo, (SvObject) SVSTRING("WAN"));
            break;
        default:
            QBComboBoxSetValue(self->networkAreaCombo, (SvObject) SVSTRING("LAN"));
            break;

    }
}

SvLocal void
QBNetworkSettingsAdvancedWidgetSetAddressProvider(QBNetworkSettingsAdvancedWidget self,
                                                  QBNetworkManagerInterfaceAddressProvider addressProvider)
{
    switch (addressProvider) {
        case QBNetworkManagerInterfaceAddressProvider_dhcp:
            QBComboBoxSetValue(self->addressProviderCombo, (SvObject) SVSTRING("DHCP"));
            break;
        case QBNetworkManagerInterfaceAddressProvider_ppp:
            QBComboBoxSetValue(self->addressProviderCombo, (SvObject) SVSTRING("PPP"));
            break;
        default:
            QBComboBoxSetValue(self->addressProviderCombo, (SvObject) SVSTRING("static"));
            break;
    }
}

SvLocal void
QBNetworkSettingsAdvancedWidgetClean(SvApplication app, void *prv)
{
    QBNetworkSettingsAdvancedWidget self = prv;
    SVRELEASE(self);
}

SvLocal void
QBNetworkSettingsAdvancedWidgetInterfaceAdded(SvObject self_, QBNetworkInterface interface)
{
    // stub implemenetation
}

SvLocal void
QBNetworkSettingsAdvancedWidgetInterfaceRemoved(SvObject self_, SvString interfaceID)
{
    // stub implemenentation
}

SvLocal void
QBNetworkSettingsAdvancedWidgetInterfaceStateChanged(SvObject self_, SvString interfaceID)
{
    QBNetworkSettingsAdvancedWidget self = (QBNetworkSettingsAdvancedWidget) self_;
    QBNetworkInterface interface = QBNetworkMonitorGetInterface(self->networkMonitor, interfaceID);

    QBNetworkManagerInterfaceStatus status = QBNetworkInterfaceGetStatus(interface);

    if (status < QBNetworkManagerInterfaceStatus_obtainingIP) {
        if (self->callback)
            self->callback(self->callbackTarget);
        return;
    }

    QBNetworkManagerInterfaceRouting routingType = QBNetworkInterfaceGetRoutingType(self->networkInterface);
    QBNetworkManagerInterfaceAddressProvider addressProvider = QBNetworkInterfaceGetAddressProvider(interface);
    SvString ip = QBNetworkInterfaceGetIP(interface);
    SvString netmask = QBNetworkInterfaceGetNetmask(interface);
    SvString gateway = QBNetworkInterfaceGetDefaultGateway(interface);
    SvString dns1 = QBNetworkInterfaceGetDNS1(interface);
    SvString dns2 = QBNetworkInterfaceGetDNS2(interface);

    QBNetworkSettingsAdvancedWidgetSetRouting(self, routingType);
    QBNetworkSettingsAdvancedWidgetSetAddressProvider(self, addressProvider);
    char buf[15];
    bool isProperAddr = (ip && QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(buf, SvStringCString(ip), 0) == 0);
    svInputSetText(self->ipInput, isProperAddr ? SvStringCString(ip) : NULL);
    isProperAddr = (netmask && QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(buf, SvStringCString(netmask), 0) == 0);
    svInputSetText(self->netmaskInput, isProperAddr ? SvStringCString(netmask) : NULL);
    isProperAddr = (gateway && QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(buf, SvStringCString(gateway), 0) == 0);
    svInputSetText(self->gatewayInput, isProperAddr ? SvStringCString(gateway) : NULL);
    isProperAddr = (dns1 && QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(buf, SvStringCString(dns1), 0) == 0);
    svInputSetText(self->dns1Input, isProperAddr ? SvStringCString(dns1) : NULL);
    isProperAddr = (dns2 && QBNetworkSettingsAdvancedWidgetValidateIPv4FamilyAddress(buf, SvStringCString(dns2), 0) == 0);
    svInputSetText(self->dns2Input, isProperAddr ? SvStringCString(dns2) : NULL);

    if (routingType == QBNetworkManagerInterfaceRouting_LAN) {
        svWidgetSetHidden(self->gatewayInput, true);
        svWidgetSetHidden(self->gatewayLabel, true);
    } else {
        svWidgetSetHidden(self->gatewayInput, false);
        svWidgetSetHidden(self->gatewayLabel, false);
    }
}

SvLocal void
QBNetworkSettingsAdvancedWidgetVPNStateChanged(SvObject self_)
{
    // stub implemenentation
}

SvLocal SvType
QBNetworkSettingsAdvancedWidget_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBNetworkSettingsAdvancedWidgetDestroy
    };

    static const struct QBNetworkMonitorListener_ monitorListenerMethods = {
        .interfaceAdded        = QBNetworkSettingsAdvancedWidgetInterfaceAdded,
        .interfaceRemoved      = QBNetworkSettingsAdvancedWidgetInterfaceRemoved,
        .interfaceStateChanged = QBNetworkSettingsAdvancedWidgetInterfaceStateChanged,
        .ipSecVPNStateChanged  = QBNetworkSettingsAdvancedWidgetVPNStateChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNetworkSettingsAdvancedWidget",
                            sizeof(struct QBNetworkSettingsAdvancedWidget_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &vtable,
                            QBNetworkMonitorListener_getInterface(), &monitorListenerMethods,
                            NULL);
    }
    return type;
}

SvLocal void
QBNetworkSettingsAdvancedWidgetCreateSubWidgets(SvWidget widget, const char *widgetName, bool useRatio,
                                                unsigned int parentWidth, unsigned int parentHeight, QBNetworkInterface interface)
{
    QBNetworkSettingsAdvancedWidget self = widget->prv;

    char *subWidgetName = NULL;
    SvWidget w = NULL;
    asprintf(&subWidgetName, "%s.networkArea.desc", widgetName);
    if (useRatio) {
        w = QBAsyncLabelNewFromRatioSettings(self->res, parentWidth, parentHeight, subWidgetName, self->textRenderer, NULL);
        svSettingsWidgetAttachWithRatio(widget, w, subWidgetName, 1);
    } else {
        w = QBAsyncLabelNew(self->res, subWidgetName, self->textRenderer);
        svSettingsWidgetAttach(widget, w, subWidgetName, 1);
    }
    free(subWidgetName);

    asprintf(&subWidgetName, "%s.networkArea.comboBox", widgetName);
    if (useRatio) {
        self->networkAreaCombo = QBComboBoxNewFromRatio(self->res, subWidgetName, parentWidth, parentHeight);
    } else {
        self->networkAreaCombo = QBComboBoxNewFromSM(self->res, subWidgetName);
    }
    free(subWidgetName);

    {
        QBComboBoxCallbacks cb = { QBNetworkSettingsAdvancedWidgetComboPrepare, QBNetworkSettingsAdvancedWidgetComboInput, NULL, NULL, NULL, NULL };
        QBComboBoxSetCallbacks(self->networkAreaCombo, self, cb);
    }

    if (useRatio) {
        svSettingsWidgetAttachWithRatio(widget, self->networkAreaCombo, svWidgetGetName(self->networkAreaCombo), 1);
    } else {
        svSettingsWidgetAttach(widget, self->networkAreaCombo, svWidgetGetName(self->networkAreaCombo), 1);
    }

    QBNetworkManagerInterfaceAddressProvider addressProvider = QBNetworkInterfaceGetAddressProvider(interface);

    SvArray newtorkAreaValues = SvArrayCreate(NULL);
    SvArrayAddObject(newtorkAreaValues, (SvObject) SVSTRING("WAN"));
    if (addressProvider != QBNetworkManagerInterfaceAddressProvider_ppp)
        SvArrayAddObject(newtorkAreaValues, (SvObject) SVSTRING("LAN"));
    QBComboBoxSetContent(self->networkAreaCombo, newtorkAreaValues);
    SVTESTRELEASE(newtorkAreaValues);

    QBNetworkManagerInterfaceRouting routingType = QBNetworkInterfaceGetRoutingType(self->networkInterface);
    QBNetworkSettingsAdvancedWidgetSetRouting(self, routingType);

    asprintf(&subWidgetName, "%s.networkConfiguration.desc", widgetName);
    if (useRatio) {
        w = QBAsyncLabelNewFromRatioSettings(self->res, parentWidth, parentHeight, subWidgetName, self->textRenderer, NULL);
        svSettingsWidgetAttachWithRatio(widget, w, subWidgetName, 1);
    } else {
        w = QBAsyncLabelNew(self->res, subWidgetName, self->textRenderer);
        svSettingsWidgetAttach(widget, w, subWidgetName, 1);
    }
    free(subWidgetName);

    asprintf(&subWidgetName, "%s.networkConfiguration.comboBox", widgetName);
    if (useRatio) {
        self->addressProviderCombo = QBComboBoxNewFromRatio(self->res, subWidgetName, parentWidth, parentHeight);
    } else {
        self->addressProviderCombo = QBComboBoxNewFromSM(self->res, subWidgetName);
    }
    free(subWidgetName);

    {
        QBComboBoxCallbacks cb = { QBNetworkSettingsAdvancedWidgetComboPrepare, QBNetworkSettingsAdvancedWidgetComboInput, NULL, NULL, NULL, NULL };
        QBComboBoxSetCallbacks(self->addressProviderCombo, self, cb);
    }

    if (useRatio) {
        svSettingsWidgetAttachWithRatio(widget, self->addressProviderCombo, svWidgetGetName(self->addressProviderCombo), 1);
    } else {
        svSettingsWidgetAttach(widget, self->addressProviderCombo, svWidgetGetName(self->addressProviderCombo), 1);
    }

    SvArray newtorkConfigurationContent = SvArrayCreate(NULL);
    if (addressProvider != QBNetworkManagerInterfaceAddressProvider_ppp) {
        SvArrayAddObject(newtorkConfigurationContent, (SvObject) SVSTRING("DHCP"));
        SvArrayAddObject(newtorkConfigurationContent, (SvObject) SVSTRING("static"));
    } else {
        SvArrayAddObject(newtorkConfigurationContent, (SvObject) SVSTRING("PPP"));
    }

    QBComboBoxSetContent(self->addressProviderCombo, newtorkConfigurationContent);
    SVTESTRELEASE(newtorkConfigurationContent);

    QBNetworkSettingsAdvancedWidgetSetAddressProvider(self, addressProvider);

    asprintf(&subWidgetName, "%s.IP.input", widgetName);
    if (useRatio) {
        self->ipInput = svInputNewFromRatio(self->res, subWidgetName, parentWidth, parentHeight, SvInputMode_IPv4, 12);
    } else {
        self->ipInput = svInputNewFromSM(self->res, subWidgetName, SvInputMode_IPv4, 12);
    }
    free(subWidgetName);
    asprintf(&subWidgetName, "%s.netmask.input", widgetName);
    if (useRatio) {
        self->netmaskInput = svInputNewFromRatio(self->res, subWidgetName, parentWidth, parentHeight, SvInputMode_IPv4, 12);
    } else {
        self->netmaskInput = svInputNewFromSM(self->res, subWidgetName, SvInputMode_IPv4, 12);
    }
    free(subWidgetName);
    asprintf(&subWidgetName, "%s.gateway.input", widgetName);
    if (useRatio) {
        self->gatewayInput = svInputNewFromRatio(self->res, subWidgetName, parentWidth, parentHeight, SvInputMode_IPv4, 12);
    } else {
        self->gatewayInput = svInputNewFromSM(self->res, subWidgetName, SvInputMode_IPv4, 12);
    }
    free(subWidgetName);
    asprintf(&subWidgetName, "%s.DNS1.input", widgetName);
    if (useRatio) {
        self->dns1Input = svInputNewFromRatio(self->res, subWidgetName, parentWidth, parentHeight, SvInputMode_IPv4, 12);
    } else {
        self->dns1Input = svInputNewFromSM(self->res, subWidgetName, SvInputMode_IPv4, 12);
    }
    free(subWidgetName);
    asprintf(&subWidgetName, "%s.DNS2.input", widgetName);
    if (useRatio) {
        self->dns2Input = svInputNewFromRatio(self->res, subWidgetName, parentWidth, parentHeight, SvInputMode_IPv4, 12);
    } else {
        self->dns2Input = svInputNewFromSM(self->res, subWidgetName, SvInputMode_IPv4, 12);
    }
    free(subWidgetName);

    asprintf(&subWidgetName, "%s.IP.desc", widgetName);
    SvWidget ipLabel = NULL;
    if (useRatio) {
        ipLabel = QBAsyncLabelNewFromRatioSettings(self->res, parentWidth, parentHeight, subWidgetName, self->textRenderer, NULL);
    } else {
        ipLabel = QBAsyncLabelNew(self->res, subWidgetName, self->textRenderer);
    }
    free(subWidgetName);
    asprintf(&subWidgetName, "%s.netmask.desc", widgetName);
    SvWidget netmaskLabel = NULL;
    if (useRatio) {
        netmaskLabel = QBAsyncLabelNewFromRatioSettings(self->res, parentWidth, parentHeight, subWidgetName, self->textRenderer, NULL);
    } else {
        netmaskLabel = QBAsyncLabelNew(self->res, subWidgetName, self->textRenderer);
    }
    free(subWidgetName);
    asprintf(&subWidgetName, "%s.gateway.desc", widgetName);
    SvWidget gatewayLabel = NULL;
    if (useRatio) {
        gatewayLabel = QBAsyncLabelNewFromRatioSettings(self->res, parentWidth, parentHeight, subWidgetName, self->textRenderer, NULL);
    } else {
        gatewayLabel = QBAsyncLabelNew(self->res, subWidgetName, self->textRenderer);
    }
    free(subWidgetName);
    asprintf(&subWidgetName, "%s.DNS1.desc", widgetName);
    SvWidget dns1Label = NULL;
    if (useRatio) {
        dns1Label = QBAsyncLabelNewFromRatioSettings(self->res, parentWidth, parentHeight, subWidgetName, self->textRenderer, NULL);
    } else {
        dns1Label = QBAsyncLabelNew(self->res, subWidgetName, self->textRenderer);
    }
    free(subWidgetName);
    asprintf(&subWidgetName, "%s.DNS2.desc", widgetName);
    SvWidget dns2Label = NULL;
    if (useRatio) {
        dns2Label = QBAsyncLabelNewFromRatioSettings(self->res, parentWidth, parentHeight, subWidgetName, self->textRenderer, NULL);
    } else {
        dns2Label = QBAsyncLabelNew(self->res, subWidgetName, self->textRenderer);
    }
    free(subWidgetName);

    QBNetworkSettingsAdvancedWidgetAddInputWidget(widget, useRatio, self->ipInput, ipLabel, QBNetworkInterfaceGetIP(self->networkInterface), true);
    QBNetworkSettingsAdvancedWidgetAddInputWidget(widget, useRatio, self->netmaskInput, netmaskLabel, QBNetworkInterfaceGetNetmask(self->networkInterface), true);
    bool showGateway = (routingType == QBNetworkManagerInterfaceRouting_WAN || routingType == QBNetworkManagerInterfaceRouting_WAN_LAN);
    QBNetworkSettingsAdvancedWidgetAddInputWidget(widget, useRatio, self->gatewayInput, gatewayLabel, QBNetworkInterfaceGetDefaultGateway(self->networkInterface), showGateway);
    QBNetworkSettingsAdvancedWidgetAddInputWidget(widget, useRatio, self->dns1Input, dns1Label, QBNetworkInterfaceGetDNS1(self->networkInterface), true);
    QBNetworkSettingsAdvancedWidgetAddInputWidget(widget, useRatio, self->dns2Input, dns2Label, QBNetworkInterfaceGetDNS2(self->networkInterface), true);

    self->gatewayLabel = gatewayLabel;

    asprintf(&subWidgetName, "%s.saveButton", widgetName);
    if (useRatio) {
        self->saveButton = svButtonNewFromRatio(self->res, subWidgetName, gettext("Save"), 0, svWidgetGetId(widget), parentWidth, parentHeight);
        svSettingsWidgetAttachWithRatio(widget, self->saveButton, subWidgetName, 1);
    } else {
        self->saveButton = svButtonNewFromSM(self->res, subWidgetName, gettext("Save"), 0, svWidgetGetId(widget));
        svSettingsWidgetAttach(widget, self->saveButton, subWidgetName, 1);
    }
    free(subWidgetName);

    asprintf(&subWidgetName, "%s.cancelButton", widgetName);
    if (useRatio) {
        self->cancelButton = svButtonNewFromRatio(self->res, subWidgetName, gettext("Cancel"), 0, svWidgetGetId(widget), parentWidth, parentHeight);
        svSettingsWidgetAttachWithRatio(widget, self->cancelButton, subWidgetName, 1);
    } else {
        self->cancelButton = svButtonNewFromSM(self->res, subWidgetName, gettext("Cancel"), 0, svWidgetGetId(widget));
        svSettingsWidgetAttach(widget, self->cancelButton, subWidgetName, 1);
    }
    free(subWidgetName);
}

SvLocal bool
QBNetworkSettingsAdvancedWidgetInputEventHandler(SvWidget widget, SvInputEvent event)
{
    QBNetworkSettingsAdvancedWidget self = (QBNetworkSettingsAdvancedWidget) widget->prv;
    if ((svWidgetIsFocused(self->cancelButton) && event->ch == QBKEY_DOWN) ||
        (svWidgetIsFocused(self->networkAreaCombo) && event->ch == QBKEY_UP)) {
        return true;
    }
    return false;
}

SvLocal SvWidget
QBNetworkSettingsAdvancedWidgetCreateFromParams(SvApplication res, QBTextRenderer textRenderer,
                                                QBNetworkMonitor networkMonitor, const char *widgetName, bool useRatio,
                                                unsigned int parentWidth, unsigned int parentHeight,
                                                QBNetworkInterface interface, SvErrorInfo *errorOut)
{
    SvWidget widget = NULL;
    SvErrorInfo error = NULL;

    if (!svSettingsIsWidgetDefined(widgetName)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "widget [%s] not found", widgetName);
        goto err;
    }

    if (useRatio)
        widget = svSettingsWidgetCreateWithRatio(res, widgetName, parentWidth, parentHeight);
    else
        widget = svSettingsWidgetCreate(res, widgetName);

    if (!widget) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "can't create widget [%s]", widgetName);
        goto err;
    }

    QBNetworkSettingsAdvancedWidget self =
        (QBNetworkSettingsAdvancedWidget) SvTypeAllocateInstance(QBNetworkSettingsAdvancedWidget_getType(), &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBNetworkSettingsAdvancedWidget");
        goto err;
    }
    self->widgetName = strdup(widgetName);
    self->res = res;
    self->textRenderer = SVRETAIN(textRenderer);
    self->networkMonitor = SVRETAIN(networkMonitor);
    self->networkInterface = SVRETAIN(interface);
    QBNetworkMonitorAddListener(networkMonitor, (SvObject) self);
    svWidgetSetUserEventHandler(widget, QBNetworkSettingsAdvancedWidgetUserEventHandler);
    svWidgetSetInputEventHandler(widget, QBNetworkSettingsAdvancedWidgetInputEventHandler);
    widget->clean = QBNetworkSettingsAdvancedWidgetClean;
    widget->prv = self;

    QBNetworkSettingsAdvancedWidgetCreateSubWidgets(widget, widgetName, useRatio, parentWidth, parentHeight, interface);
    return widget;

err:
    if (widget)
        svWidgetDestroy(widget);
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

SvWidget
QBNetworkSettingsAdvancedWidgetCreate(SvApplication res, QBTextRenderer textRenderer, QBNetworkMonitor networkMonitor, const char *widgetName,
                                      QBNetworkInterface interface, SvErrorInfo *errorOut)
{
    return QBNetworkSettingsAdvancedWidgetCreateFromParams(res, textRenderer, networkMonitor, widgetName, false, 0, 0, interface, errorOut);
}

SvWidget
QBNetworkSettingsAdvancedWidgetCreateFromRatio(SvApplication res, QBTextRenderer textRenderer, QBNetworkMonitor networkMonitor, const char *widgetName,
                                               unsigned int parentWidth, unsigned int parentHeight,
                                               QBNetworkInterface interface, SvErrorInfo *errorOut)
{
    return QBNetworkSettingsAdvancedWidgetCreateFromParams(res, textRenderer, networkMonitor, widgetName, true, parentWidth, parentHeight, interface, errorOut);
}

void
QBNetworkSettingsAdvancedWidgetSetOnFinishCallback(SvWidget widget, void *target, QBNetworkSettingsAdvancedWidgetOnFinishCallback callback)
{
    QBNetworkSettingsAdvancedWidget self = widget->prv;
    self->callback = callback;
    self->callbackTarget = target;
}

void
QBNetworkSettingsAdvancedWidgetSetFocus(SvWidget widget)
{
    QBNetworkSettingsAdvancedWidget self = widget->prv;
    svWidgetSetFocus(self->addressProviderCombo);
}
