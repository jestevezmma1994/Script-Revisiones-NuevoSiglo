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

#include "QBNetworkSettingsUtils.h"
#include <QBNetworkManager/Utils/QBNetworkManagerTypes.h>
#include <SvFoundation/SvString.h>
#include <libintl.h>

SvString QBNetworkSettingsUtilsCreateInterfaceName(QBNetworkInterface interface)
{
    if (SvObjectIsInstanceOf((SvObject) interface, QBEthernetInterface_getType()))
        return SvStringCreate(gettext("Wired (Ethernet)"), NULL);

    if (SvObjectIsInstanceOf((SvObject) interface, QBWiFiInterface_getType()))
        return SvStringCreate(gettext("Wireless (WiFi)"), NULL);

    if (SvObjectIsInstanceOf((SvObject) interface, QBMoCaInterface_getType()))
        return SvStringCreate(gettext("Wired (MoCa)"), NULL);

    if (SvObjectIsInstanceOf((SvObject) interface, QBDocsisInterface_getType()))
        return SvStringCreate(gettext("Wired (DOCSIS)"), NULL);

    if (SvObjectIsInstanceOf((SvObject) interface, QB3GInterface_getType()))
        return SvStringCreate(gettext("Wireless (3G)"), NULL);

    return (SvString) SvObjectCopy((SvObject) QBNetworkInterfaceGetID(interface), NULL);
}

SvLocal SvString QBWifiInterfaceConnectionErrorToString(QBWiFiInterface wifiInterface,
                                                        QBNetworkManagerWiFiConnectionError connectionError)
{
    SvString ssid = QBWiFiInterfaceGetSSID(wifiInterface);
    switch (connectionError) {
        case QBNetworkManagerWiFiConnectionError_wrongKey:
            return SvStringCreateWithFormat(gettext("Wrong key for %s"), SvStringCString(ssid));
        case QBNetworkManagerWiFiConnectionError_authFailed:
            return SvStringCreateWithFormat(gettext("Authentication with %s failed"), SvStringCString(ssid));
        case QBNetworkManagerWiFiConnectionError_connFailed:
            return SvStringCreateWithFormat(gettext("Connection with %s failed"), SvStringCString(ssid));
        case QBNetworkManagerWiFiConnectionError_noSSIDInfoFound:
            return SvStringCreateWithFormat(gettext("%s not found"), SvStringCString(ssid));
        default:
            return SvStringCreate(gettext("Disconnected"), NULL);
    }
}

SvString QBNetworkSettingsUtilsCreateInterfaceConnectionDescription(QBNetworkInterface interface)
{
    SvString subcaption;
    QBNetworkManagerInterfaceStatus status = QBNetworkInterfaceGetStatus(interface);
    switch (status) {
        case QBNetworkManagerInterfaceStatus_disconnected:
            if (SvObjectIsInstanceOf((SvObject) interface, QBWiFiInterface_getType())) {
                QBNetworkManagerWiFiConnectionError connectionError = QBWiFiInterfaceGetWiFiConnectionError((QBWiFiInterface) interface);
                subcaption = QBWifiInterfaceConnectionErrorToString((QBWiFiInterface) interface, connectionError);
            } else {
                subcaption = SvStringCreate(gettext("Disconnected"), NULL);
            }
            break;
        case QBNetworkManagerInterfaceStatus_obtainingLink:
            if (SvObjectIsInstanceOf((SvObject) interface, QBWiFiInterface_getType())) {
                SvString ssid = QBWiFiInterfaceGetSSID((QBWiFiInterface) interface);
                subcaption = SvStringCreateWithFormat("%s %s...", gettext("Connecting to"), ssid ? SvStringCString(ssid) : gettext("unknown"));
            } else {
                subcaption = SvStringCreate(gettext("Connecting..."), NULL);
            }
            break;
        case QBNetworkManagerInterfaceStatus_obtainingIP:
            subcaption = SvStringCreate(gettext("Obtaining IP address..."), NULL);
            break;
        case QBNetworkManagerInterfaceStatus_connected:
            if (SvObjectIsInstanceOf((SvObject) interface, QBWiFiInterface_getType())) {
                SvString ssid = QBWiFiInterfaceGetSSID((QBWiFiInterface) interface);
                subcaption = SvStringCreateWithFormat("%s %s: %s", gettext("Connected to"), SvStringCString(ssid), SvStringCString(QBNetworkInterfaceGetIP(interface)));
            } else {
                subcaption = SvStringCreateWithFormat("%s: %s", gettext("Connected"), SvStringCString(QBNetworkInterfaceGetIP(interface)));
            }
            break;
        default:
            subcaption = SvStringCreateWithFormat("%s %d", gettext("Unknown state"), status);
            break;
    }
    return subcaption;
}
