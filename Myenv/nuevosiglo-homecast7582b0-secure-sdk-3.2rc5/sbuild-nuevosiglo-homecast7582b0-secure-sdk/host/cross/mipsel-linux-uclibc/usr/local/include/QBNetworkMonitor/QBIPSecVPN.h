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

#ifndef QB_IPSEC_VPN_H_
#define QB_IPSEC_VPN_H_

/**
 * @file QBIPSecVPN.h IPSec VPN API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBNetworkMonitor/QBNetworkInterfaceTypes.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvWeakReference.h>
#include <QBNetworkManager/Utils/QBNetworkManagerTypes.h>

/**
 * @defgroup QBIPSecVPN IPSec VPN
 * @ingroup QBNetworkMonitor
 * @{
 **/

/**
 * IPSec VPN representation class.
 * It keeps all IPSec VPN data that might be presented in GUI
 *
 * @class QBIPSecVPN
 **/
typedef struct QBIPSecVPN_ *QBIPSecVPN;

/**
 * Get runtime type identification object representing type of QBIPSecVPN class.
 * @return QBIPSecVPN class type handle
 **/
SvType
QBIPSecVPN_getType(void);

/**
 * Create a new instance of a QBIPSecVPN class
 *
 * @param[in] status        VPN connection status
 * @param[in] interfaceID   VPN interface ID
 * @param[in] localIP       local IP address
 * @param[in] localPort     local port
 * @param[in] vpnServerIP   VPN server IP address
 * @param[in] vpnServerPort VPN server port
 * @param[in] vpnIP         VPN IP address
 * @param[in] vpnNetmask    VPN netmask
 * @param[in] vpnDNS        VPN DNS address
 * @param[in] connectionCounter counter of connection attemps,
                                it is reset when connection succeeds
 * @param[out] errorOut       error info
 * @return new instance of QBWiFiNetwork
 **/
QBIPSecVPN
QBIPSecVPNCreate(QBNetworkManagerIPSecVPNStatus status,
                 SvString interfaceID,
                 SvString localIP,
                 int localPort,
                 SvString vpnServerIP,
                 int vpnServerPort,
                 SvString vpnIP,
                 SvString vpnNetmask,
                 SvString vpnDNS,
                 unsigned int connectionCounter,
                 SvErrorInfo *errorOut);

/**
 * Get VPN connection status
 *
 * @param[in] self  QBIPSecVPN handle
 * @return VPN connection status
 **/
QBNetworkManagerIPSecVPNStatus
QBIPSecVPNGetStatus(QBIPSecVPN self);

/**
 * Get interface ID used for VPN connection
 *
 * @param[in] self  QBIPSecVPN handle
 * @return VPN interface ID
 **/
SvString
QBIPSecVPNGetInterfaceID(QBIPSecVPN self);

/**
 * Get local IP
 *
 * @param[in] self  QBIPSecVPN handle
 * @return VPN local IP
 **/
SvString
QBIPSecVPNGetLocalIP(QBIPSecVPN self);

/**
 * Get local port
 *
 * @param[in] self  QBIPSecVPN handle
 * @return
 **/
int
QBIPSecVPNGetLocalPort(QBIPSecVPN self);

/**
 * Get VPN server IP
 *
 * @param[in] self  QBIPSecVPN handle
 * @return VPN server IP
 **/
SvString
QBIPSecVPNGetVPNServerIP(QBIPSecVPN self);

/**
 * Get VPN server port
 *
 * @param[in] self  QBIPSecVPN handle
 * @return VPN server port
 **/
int
QBIPSecVPNGetVPNServerPort(QBIPSecVPN self);

/**
 * Get VPN IP
 *
 * @param[in] self  QBIPSecVPN handle
 * @return VPN IP
 **/
SvString
QBIPSecVPNGetVPNIP(QBIPSecVPN self);

/**
 * Get VPN netmask
 *
 * @param[in] self  QBIPSecVPN handle
 * @return VPN netmask
 **/
SvString
QBIPSecVPNGetVPNNetmask(QBIPSecVPN self);

/**
 * Get VPN DNS
 *
 * @param[in] self  QBIPSecVPN handle
 * @return VPN DNS
 **/
SvString
QBIPSecVPNGetVPNDNS(QBIPSecVPN self);

/**
 * Get VPN connection counter. The counter is incremented every time
 * a connection attempt fails. The counter is reset when the connection
 * attempt succeeds or VPN is disabled due to lack of WAN connectivity.
 *
 * @param[in] self  QBIPSecVPN handle
 * @return VPN connection counter
 **/
unsigned int
QBIPSecVPNGetConnectionCounter(QBIPSecVPN self);


/**
 * @}
**/

#endif //QB_WIFI_NETWORK_H_
