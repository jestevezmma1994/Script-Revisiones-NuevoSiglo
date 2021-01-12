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

#ifndef QB_NETWORK_INTERFACE_H_
#define QB_NETWORK_INTERFACE_H_

/**
 * @file QBNetworkInterface.h Network interfaces of different types declarations.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBNetworkMonitor/QBWiFiNetwork.h>
#include <QBNetworkManager/Utils/QBNetworkManagerTypes.h>
#include <QBDataModel3/QBSortedList.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <QBNetworkMonitor/QBNetworkInterfaceTypes.h>

/**
 * @defgroup QBNetworkInterface Network Interface
 * @ingroup QBNetworkMonitor
 * @{
 **/

/**
 * Get runtime type identification object representing type of QBNetworkInterface class.
 * @return QBNetworkInterface class type handle
 **/
SvType
QBNetworkInterface_getType(void);

/**
 * Get ID of an interface assigned by an operating system (e.g., eth0, ra0)
 *
 * @param[in] self  network interface handle
 * @return ID of an interface assigned by an operating system
 **/
SvString
QBNetworkInterfaceGetID(QBNetworkInterface self);

/**
 * Assign IP address to the interface
 *
 * @param[in] self  network interface handle
 * @param[in] ip    IP address to be assigned
 **/
void
QBNetworkInterfaceSetIP(QBNetworkInterface self, SvString ip);

/**
 * Get IP address assigned to the interface
 *
 * @param[in] self  network interface handle
 * @return IP address assigned to the interface
 **/
SvString
QBNetworkInterfaceGetIP(QBNetworkInterface self);

/**
 * Assign netmask address to the interface
 *
 * @param[in] self    network interface handle
 * @param[in] netmask netmask address to be assigned
 **/
void
QBNetworkInterfaceSetNetmask(QBNetworkInterface self, SvString netmask);

/**
 * Get netmask address assigned to the interface
 *
 * @param[in] self  network interface handle
 * @return netmask address assigned to the interface
 **/
SvString
QBNetworkInterfaceGetNetmask(QBNetworkInterface self);

/**
 * Assign default gateway address to the interface
 *
 * @param[in] self    network interface handle
 * @param[in] gateway default gateway address to be assigned
 **/
void
QBNetworkInterfaceSetDefaultGateway(QBNetworkInterface self, SvString gateway);

/**
 * Get default gateway address assigned to the interface
 *
 * @param[in] self  network interface handle
 * @return default gateway address assigned to the interface
 **/
SvString
QBNetworkInterfaceGetDefaultGateway(QBNetworkInterface self);

/**
 * Assign DNS address to the interface
 *
 * @param[in] self  network interface handle
 * @param[in] dns   DNS address to be assigned
 **/
void
QBNetworkInterfaceSetDNS1(QBNetworkInterface self, SvString dns);

/**
 * Get DNS address assigned to the interface
 *
 * @param[in] self  network interface handle
 * @return DNS address assigned to the interface
 **/
SvString
QBNetworkInterfaceGetDNS1(QBNetworkInterface self);

/**
 * Assign fallback DNS address to the interface
 *
 * @param[in] self  network interface handle
 * @param[in] dns   fallback DNS address to be assigned
 **/
void
QBNetworkInterfaceSetDNS2(QBNetworkInterface self, SvString dns);

/**
 * Get fallback DNS address assigned to the interface
 *
 * @param[in] self  network interface handle
 * @return fallback DNS address assigned to the interface
 **/
SvString
QBNetworkInterfaceGetDNS2(QBNetworkInterface self);

/**
 * Set gateway tenure paremeter to the interface
 *
 * @param[in] self  network interface handle
 * @param[in] gatewayTenure value to be assigned
 **/
void
QBNetworkInterfaceSetGatewayTenure(QBNetworkInterface self, const int gatewayTenure);

/**
 * Get gateway tenure paremeter assigned to the interface
 *
 * @param[in] self  network interface handle
 * @return gateway tenure assigned to the interface
 **/
int
QBNetworkInterfaceGetGatewayTenure(QBNetworkInterface self);

/**
 * Set availibility paremeter to the interface
 *
 * @param[in] self  network interface handle
 * @param[in] isAvailable value to be assigned
 **/
void
QBNetworkInterfaceSetIsAvailable(QBNetworkInterface self, const bool isAvailable);

/**
 * Get availibility parameter assigned to the interface
 *
 * @param[in] self  network interface handle
 * @return isAvailable flag assigned to the interface
 **/
bool
QBNetworkInterfaceGetIsAvailable(QBNetworkInterface self);

/**
 * Set status of a network interface
 *
 * @param[in] self  network interface handle
 * @param[in] status status to be set
 **/
void
QBNetworkInterfaceSetStatus(QBNetworkInterface self, QBNetworkManagerInterfaceStatus status);

/**
 * Get status of the interface
 *
 * @param[in] self  network interface handle
 * @return current status of the interface
 **/
QBNetworkManagerInterfaceStatus
QBNetworkInterfaceGetStatus(QBNetworkInterface self);

/**
 * Set address provider (DHCP, STATIC) to the interface
 *
 * @param[in] self          network interface handle
 * @param[in] addressProvider  address provider (DHCP, STATIC) to be set
 **/
void
QBNetworkInterfaceSetAddressProvider(QBNetworkInterface self, QBNetworkManagerInterfaceAddressProvider addressProvider);

/**
 * Get address provider (DHCP, STATIC) of the interface
 *
 * @param[in] self  network interface handle
 * @return address provider (DHCP, STATIC) of the interface
 **/
QBNetworkManagerInterfaceAddressProvider
QBNetworkInterfaceGetAddressProvider(QBNetworkInterface self);

/**
 * Set config (normal, fallback, panic) on the interface
 *
 * @param[in] self    network interface handle
 * @param[in] config  config (normal, fallback, panic) to be set
 **/
void
QBNetworkInterfaceSetConfig(QBNetworkInterface self, QBNetworkManagerInterfaceConfig config);

/**
 * Get config (normal, fallback, panic) currently set on the interface
 *
 * @param[in] self  network interface handle
 * @return config (normal, fallback, panic) currently set on the interface
 **/
QBNetworkManagerInterfaceConfig
QBNetworkInterfaceGetConfig(QBNetworkInterface self);

/**
 * Set routing type (LAN, WAN, LAN+WAN) to the interface
 *
 * @param[in] self        network interface handle
 * @param[in] routingType routing type (LAN, WAN, LAN+WAN) to be set
 **/
void
QBNetworkInterfaceSetRoutingType(QBNetworkInterface self, QBNetworkManagerInterfaceRouting routingType);

/**
 * Get routing type (LAN, WAN, LAN+WAN) of the interface
 *
 * @param[in] self  network interface handle
 * @return routing type (LAN, WAN, LAN+WAN) of the interface
 **/
QBNetworkManagerInterfaceRouting
QBNetworkInterfaceGetRoutingType(QBNetworkInterface self);

/**
 * Get runtime type identification object representing type of QBEthernetInterface class.
 * @return QBEthernetInterface class type handle
 **/
SvType
QBEthernetInterface_getType(void);

/**
 * Create a new instance of an object representing a ethernet interface
 *
 * @param[in] id interface ID
 * @return new instance of QBEthernetInterface
 **/
QBEthernetInterface
QBEthernetInterfaceCreate(SvString id);

/**
 * Get runtime type identification object representing type of QBWiFiInterface class.
 * @return QBWiFiInterface class type handle
 **/
SvType
QBWiFiInterface_getType(void);

/**
 * Create a new instance of an object representing a WiFi interface
 *
 * @param[in] id interface ID
 * @return new instance of QBWiFiInterface
 **/
QBWiFiInterface
QBWiFiInterfaceCreate(SvString id);

/**
 * Set chosen SSID on the WiFi interface
 *
 * @param[in] self WiFi network interface handle
 * @param[in] ssid SSID to be set
 **/
void
QBWiFiInterfaceSetSSID(QBWiFiInterface self, SvString ssid);

/**
 * Get current SSID that is set on the WiFi interface
 *
 * @param[in] self WiFi network interface handle
 * @return current SSID that is set on the WiFi interface
 **/
SvString
QBWiFiInterfaceGetSSID(QBWiFiInterface self);

/**
 * Add or update a WiFi network in interface's SSIDs list
 *
 * @param[in] self        WiFi network interface handle
 * @param[in] wifiNetwork WiFi network to be added/updated
 **/
void
QBWiFiInterfaceUpdateWiFiNetwork(QBWiFiInterface self, QBWiFiNetwork wifiNetwork);

/**
 * Remove a WiFi network in interface's SSIDs list
 *
 * @param[in] self  WiFi network interface handle
 * @param[in] ssid  SSID of a WiFi network to be removed
 **/
void
QBWiFiInterfaceRemoveWiFiNetwork(QBWiFiInterface self, SvString ssid);

/**
 * Get WiFi network with gived SSID
 *
 * @param[in] self  WiFi network interface handle
 * @param[in] ssid  SSID of a WiFi network to be returned
 * @return WiFi network with given SSID
 **/
QBWiFiNetwork
QBWiFiInterfaceGetWiFiNetwork(QBWiFiInterface self, SvString ssid);

/**
 * Get alphabetically sorted list of available WiFi networks
 *
 * @param[in] self  WiFi network interface handle
 * @return alphabetically sorted list of available WiFi networks
 **/
QBSortedList
QBWiFiInterfaceGetAvailableSSIDs(QBWiFiInterface self);

/**
 * Set connection error on the WiFi interface
 *
 * @param[in] self            WiFi network interface handle
 * @param[in] connectionError connection error to be set
 **/
void
QBWiFiInterfaceSetWiFiConnectionError(QBWiFiInterface self, QBNetworkManagerWiFiConnectionError connectionError);

/**
 * Get last connection error that was set on the WiFi interface
 *
 * @param[in] self WiFi network interface handle
 * @return last connection error that was set on the WiFi interface
 **/
QBNetworkManagerWiFiConnectionError
QBWiFiInterfaceGetWiFiConnectionError(QBWiFiInterface self);

/**
 * Get runtime type identification object representing type of QBMoCaInterface class.
 * @return QBMoCaInterface class type handle
 **/
SvType
QBMoCaInterface_getType(void);

/**
 * Create a new instance of an object representing a MoCa interface
 *
 * @param[in] id interface ID
 * @return new instance of QBMoCaInterface
 **/
QBMoCaInterface
QBMoCaInterfaceCreate(SvString id);

/**
 * Get runtime type identification object representing type of QBDocsisInterface class.
 * @return QBDocsisInterface class type handle
 **/
SvType
QBDocsisInterface_getType(void);

/**
 * Create a new instance of an object representing a cable modem interface
 *
 * @param[in] id interface ID
 * @return new instance of QBDocsisInterface
 **/
QBDocsisInterface
QBDocsisInterfaceCreate(SvString id);

/**
 * Get runtime type identification object representing type of QB3GInterface class.
 * @return QB3GInterface class type handle
 **/
SvType
QB3GInterface_getType(void);

/**
 * Create a new instance of an object representing a 3G modem interface
 *
 * @param[in] id interface ID
 * @return new instance of QB3GInterface
 **/
QB3GInterface
QB3GInterfaceCreate(SvString id);

/**
 * @}
**/

#endif //QB_NETWORK_INTERFACE_H_
