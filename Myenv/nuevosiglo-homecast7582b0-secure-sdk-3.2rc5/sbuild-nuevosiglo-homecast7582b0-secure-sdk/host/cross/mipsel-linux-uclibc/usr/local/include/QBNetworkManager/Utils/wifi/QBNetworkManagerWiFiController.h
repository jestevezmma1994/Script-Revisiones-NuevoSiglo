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

#ifndef QB_NETWORK_MANAGER_WIFI_CONTROLLER_H
#define QB_NETWORK_MANAGER_WIFI_CONTROLLER_H

/**
 * @file QBNetworkManagerWiFiController.h WiFi controller API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBNetworkManager/Utils/QBNetworkManagerTypes.h>
#include <stdbool.h>

/**
 * @defgroup QBNetworkManagerWiFiController QBNetworkManagerWiFiController
 * @ingroup QBNetworkManager
 * @{
 **/

/**
 * Connect to a WiFi network (SSID) on a specified WiFi interface.
 * The QBNetworkManager will try to associate to the SSID and then will
 * apply DHCP or static configuration on the interface.
 * After calling this function, the SSID will be automatically reconnected
 * if the connection will be broken.
 * This is an asynchronous function.
 *
 * @param[in] interfaceID       id of a network interface on which the WiFi network should be connected
 * @param[in] ssid              SSID of a WiFi network to be connected
 * @param[in] passwd            a WEP or WPA/WPA2 key to be used to associate to the SSID
 * @param[in] security          a WEP or WPA/WPA2 security to be used while associating
 * @param[in] isManual          @c true if the WiFi network is not broadcasting its SSID
 **/
void
QBNetworkManagerConnectWiFiNetwork(const char *interfaceID,
                                   const char *ssid,
                                   const char *passwd,
                                   QBNetworkManagerWiFiNetworkSecurity security,
                                   bool isManual);

/**
 * Disconnect from a SSID (if connected) on a specified interface.
 * After calling this function, the QBNetworkManager will not try to
 * reconnect to the SSID.
 * This is an asynchronous function.
 *
 * @param[in] interfaceID       id of a network interface on which the WiFi network should be disconnected
 **/
void
QBNetworkManagerDisconnectWiFiNetwork(const char *interfaceID);

/**
 * Forget password of a WiFi network (SSID) if it was previously remembered.
 * If specified SSID is currently connected, then it will be disconnected too.
 * After calling this function, the QBNetworkManager will not try to
 * reconnect to the SSID.
 * This is an asynchronous function.
 *
 * @param[in] interfaceID       id of a network interface on which the WiFi network should be disconnected
 * @param[in] ssid              SSID of a WiFi network to be forgotten
 **/
void
QBNetworkManagerForgetWiFiNetwork(const char *interfaceID,
                                  const char *ssid);

/**
 * Request SSID scanning. Results will be delivered right after
 * the scanning is successful.
 * This is an asynchronous function.
 *
 * @param[in] interfaceID   id of a network interface on which a SSID scanning should be performed
 **/
void
QBNetworkManagerScanWiFi(const char *interfaceID);

/**
 * @}
**/

#endif /* QB_NETWORK_MANAGER_WIFI_CONTROLLER_H_ */
