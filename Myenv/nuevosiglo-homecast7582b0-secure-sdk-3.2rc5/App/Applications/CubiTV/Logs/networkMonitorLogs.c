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


#include "networkMonitorLogs.h"

#include <QBNetManager.h>
#include <QBSecureLogManager.h>
#include <inttypes.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvObject.h>
#include <QBNetworkMonitor/QBNetworkMonitorListener.h>
#include <QBNetworkMonitor/QBWiFiNetwork.h>
#include <QBNetworkMonitor/QBNetworkInterface.h>
#include <QBNetworkMonitor.h>

#define NETWORK_LOGS "Network"

struct QBNetworkMonitorLogs_t {
    struct SvObject_ super;
    QBNetworkMonitor networkMonitor;
};

static QBNetworkMonitorLogs networkMonitorLogs = NULL;

/**
 * Return SvString description of a given interface. Useful for logging.
 *
 * @param[in] interface that should be converted to string
 * @return SvString with description of the interface or NULL if error
 **/
SvLocal SvString QBNetworkMonitorLogsInterfaceToString(QBNetworkInterface interface)
{
    if (!interface)
        return NULL;

    const char *id;
    const char *ip;
    const char *netmask;
    const char *gateway;
    const char *dns1;
    const char *dns2;
    int gatewayTenure;

    if (QBNetworkInterfaceGetID(interface))
        id = SvStringCString(QBNetworkInterfaceGetID(interface));
    else
        id = "n/a";

    if (QBNetworkInterfaceGetIP(interface))
        ip = SvStringCString(QBNetworkInterfaceGetIP(interface));
    else
        ip = "n/a";

    if (QBNetworkInterfaceGetNetmask(interface))
        netmask = SvStringCString(QBNetworkInterfaceGetNetmask(interface));
    else
        netmask = "n/a";

    if (QBNetworkInterfaceGetDefaultGateway(interface))
        gateway = SvStringCString(QBNetworkInterfaceGetDefaultGateway(interface));
    else
        gateway = "n/a";

    if (QBNetworkInterfaceGetDNS1(interface))
        dns1 = SvStringCString(QBNetworkInterfaceGetDNS1(interface));
    else
        dns1 = "n/a";

    if (QBNetworkInterfaceGetDNS2(interface))
        dns2 = SvStringCString(QBNetworkInterfaceGetDNS2(interface));
    else
        dns2 = "n/a";

    gatewayTenure = QBNetworkInterfaceGetGatewayTenure(interface);

    return SvStringCreateWithFormat("\"interface\":{"
                                    "\"id\":\"%s\","
                                    "\"ip\":\"%s\","
                                    "\"netmask\":\"%s\","
                                    "\"gateway\":\"%s\","
                                    "\"dns1\":\"%s\","
                                    "\"dns2\":\"%s\","
                                    "\"gatewayTenure\":\"%d\"}",
                                    id,
                                    ip,
                                    netmask,
                                    gateway,
                                    dns1,
                                    dns2,
                                    gatewayTenure);
}

/**
 * Return SvString description of a given interface. Useful for logging.
 *
 * @param[in] interface that should be converted to string
 * @return SvString with description of the interface or NULL if error
 **/
SvLocal SvString QBNetworkMonitorLogsVPNToString(QBIPSecVPN vpn)
{
    if (vpn)
        return NULL;

    const char *interfaceID;
    const char *localIP;
    int localPort;
    const char *vpnServerIP;
    int vpnServerPort;
    const char *vpnIP;
    const char *vpnNetmask;
    const char *vpnDNS;

    SvString tmp = QBIPSecVPNGetInterfaceID(vpn);
    if (tmp)
        interfaceID = SvStringCString(tmp);
    else
        interfaceID = "n/a";

    tmp = QBIPSecVPNGetLocalIP(vpn);
    if (tmp)
        localIP = SvStringCString(tmp);
    else
        localIP = "n/a";

    localPort = QBIPSecVPNGetLocalPort(vpn);

    tmp = QBIPSecVPNGetVPNServerIP(vpn);
    if (tmp)
        vpnServerIP = SvStringCString(tmp);
    else
        vpnServerIP = "n/a";

    vpnServerPort = QBIPSecVPNGetVPNServerPort(vpn);

    tmp = QBIPSecVPNGetVPNIP(vpn);
    if (tmp)
        vpnIP = SvStringCString(tmp);
    else
        vpnIP = "n/a";

    tmp = QBIPSecVPNGetVPNNetmask(vpn);
    if (tmp)
        vpnNetmask = SvStringCString(tmp);
    else
        vpnNetmask = "n/a";

    tmp = QBIPSecVPNGetVPNDNS(vpn);
    if (tmp)
        vpnDNS = SvStringCString(tmp);
    else
        vpnDNS = "n/a";

    return SvStringCreateWithFormat("\"vpn\":{"
                                    "\"interfaceID\":\"%s\","
                                    "\"localIP\":\"%s\","
                                    "\"localPort\":\"%d\","
                                    "\"vpnServerIP\":\"%s\","
                                    "\"vpnServerPort\":\"%d\","
                                    "\"vpnIP\":\"%s\","
                                    "\"vpnNetmask\":\"%s\","
                                    "\"vpnDNS\":\"%s\"}",
                                    interfaceID,
                                    localIP,
                                    localPort,
                                    vpnServerIP,
                                    vpnServerPort,
                                    vpnIP,
                                    vpnNetmask,
                                    vpnDNS);
}

SvLocal void QBNetworkMonitorLogsInterfaceAdded(SvObject self_, QBNetworkInterface interface)
{
    SvString interfaceDesc = QBNetworkMonitorLogsInterfaceToString(interface);

    if (likely(interfaceDesc)) {
        QBSecureLogEvent(NETWORK_LOGS, "Notice.Network.InterfaceAdd", "JSON:{\"action\":\"add\",%s}", SvStringCString(interfaceDesc));
        SVRELEASE(interfaceDesc);
    } else {
        SvLogError("%s: Can't create network description for secure event logging.", __func__);
    }
}

SvLocal void QBNetworkMonitorLogsInterfaceRemoved(SvObject self_, SvString interfaceID)
{
    if (likely(interfaceID)) {
        QBSecureLogEvent(NETWORK_LOGS, "Notice.Network.InterfaceRemove", "JSON:{\"action\":\"remove\",\"id\":\"%s\"}", SvStringCString(interfaceID));
    }
}

SvLocal void QBNetworkMonitorLogsInterfaceStateChanged(SvObject self_, SvString interfaceID)
{
    QBNetworkMonitorLogs self = (QBNetworkMonitorLogs) self_;
    if (interfaceID) {
        QBNetworkInterface interface = QBNetworkMonitorGetInterface(self->networkMonitor, interfaceID);

        if (!interface) {
            SvLogWarning("%s: Can't find interface: %s", __func__, SvStringCString(interfaceID));
            return;
        }

        SvString interfaceDesc = QBNetworkMonitorLogsInterfaceToString(interface);
        if (!interfaceDesc) {
            SvLogWarning("%s: Can't create description of interface: %s", __func__, SvStringCString(interfaceID));
            return;
        }
        QBSecureLogEvent(NETWORK_LOGS, "Notice.Network.InterfaceChange", "JSON:{\"action\":\"modify\",%s}", SvStringCString(interfaceDesc));
        SVRELEASE(interfaceDesc);
    }
}

SvLocal void QBNetworkMonitorLogsIPSecVPNStateChanged(SvObject self_)
{
    QBNetworkMonitorLogs self = (QBNetworkMonitorLogs) self_;
    QBIPSecVPN vpn = QBNetworkMonitorGetIPSecVPN(self->networkMonitor);

    if (!vpn) {
        SvLogWarning("%s: Can't find VPN", __func__);
        return;
    }

    SvString vpnDesc = QBNetworkMonitorLogsVPNToString(vpn);
    if (!vpnDesc) {
        SvLogWarning("%s: Can't create description of VPN", __func__);
        return;
    }
    QBSecureLogEvent(NETWORK_LOGS, "Notice.Network.VPNChange", "JSON:{\"action\":\"modify\",%s}", SvStringCString(vpnDesc));
    SVRELEASE(vpnDesc);
}

SvLocal SvType QBNetworkMonitorLogs_getType(void)
{
    static SvType type = NULL;

    static const struct QBNetworkMonitorListener_ networkMonitorMethods = {
        .interfaceAdded         = QBNetworkMonitorLogsInterfaceAdded,
        .interfaceRemoved       = QBNetworkMonitorLogsInterfaceRemoved,
        .interfaceStateChanged  = QBNetworkMonitorLogsInterfaceStateChanged,
        .ipSecVPNStateChanged   = QBNetworkMonitorLogsIPSecVPNStateChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNetworkMonitorLogs",
                            sizeof(struct QBNetworkMonitorLogs_t),
                            SvObject_getType(),
                            &type,
                            QBNetworkMonitorListener_getInterface(), &networkMonitorMethods,
                            NULL);
    }
    return type;
}

void QBNetworkMonitorLogCreate(QBNetworkMonitor networkMonitor_)
{
    networkMonitorLogs = (QBNetworkMonitorLogs) SvTypeAllocateInstance(QBNetworkMonitorLogs_getType(), NULL);
    if (unlikely(!networkMonitorLogs)) {
        SvLogError("%s: Can't create instance of QBNetworkMonitorLogs.", __func__);
        return;
    }

    networkMonitorLogs->networkMonitor = networkMonitor_;
}
void QBNetworkMonitorLogStart(void)
{
    if (!networkMonitorLogs || !networkMonitorLogs->networkMonitor) {
        SvLogError("%s: Can't start logger for network monitor.", __func__);
        return;
    }
    QBNetworkMonitorAddListener(networkMonitorLogs->networkMonitor, (SvObject) networkMonitorLogs);
}

void QBNetworkMonitorLogStop(void)
{
    if (!networkMonitorLogs || !networkMonitorLogs->networkMonitor) {
        SvLogError("%s: Can't stop logger for network monitor.", __func__);
        return;
    }

    QBNetworkMonitorRemoveListener(networkMonitorLogs->networkMonitor, (SvObject) networkMonitorLogs);
}
void QBNetworkMonitorLogDestroy(void)
{
    SVTESTRELEASE(networkMonitorLogs);
    networkMonitorLogs = NULL;
}
