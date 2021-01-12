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

#ifndef QB_NETWORK_MANAGER_CONTROLLER_H
#define QB_NETWORK_MANAGER_CONTROLLER_H

/**
 * @file QBNetworkManagerController.h QBNetworkManager controller API (an external API
 * to be used by other processes to set state of the QBNetworkManager daemon process).
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBNetworkManager/Utils/QBNetworkManagerTypes.h>

/**
 * @defgroup QBNetworkManagerController QBNetworkManagerController
 * @ingroup QBNetworkManager
 * @{
 **/

/// Path for IPC listeners of the network manager
#define QB_NETWORK_MANAGER_LISTENERS_IPC_PATH "/var/run/networking/listeners/"

/**
 * Set a static IP address on a network interface. The IP address will be
 * physically set on the interface if an address provider is set to
 * QBNetworkManagerInterfaceAddressProvider_static (or if the address provider
 * will be set to this value after setting the IP).
 *
 * Remember to call QBNetworkManagerApplyConf() function to apply the setting!
 * This is an asynchronous function.
 *
 * @param[in] interfaceID   id of a network interface on which the IP address should be set
 * @param[in] ipAddress     a static IP address to be set on the interface
 **/
void
QBNetworkManagerSetStaticIPForInterface(const char *interfaceID,
                                        const char *ipAddress);

/**
 * Set a static netmask address on a network interface. The netmask address will be
 * physically set on the interface if an address provider is set to
 * QBNetworkManagerInterfaceAddressProvider_static (or if the address provider
 * will be set to this value after setting the netmask).
 *
 * Remember to call QBNetworkManagerApplyConf function to apply the setting!
 * This is an asynchronous function.
 *
 * @param[in] interfaceID   id of a network interface on which the netmask address should be set
 * @param[in] netmask       a static netmask address to be set on the interface
 **/
void
QBNetworkManagerSetStaticNetmaskForInterface(const char *interfaceID,
                                             const char *netmask);

/**
 * Set a static gateway address on a network interface. The gateway address will be
 * physically set on the interface if an address provider is set to
 * QBNetworkManagerInterfaceAddressProvider_static (or if the address provider
 * will be set to this value after setting the gateway).
 *
 * Remember to call QBNetworkManagerApplyConf function to apply the setting!
 * This is an asynchronous function.
 *
 * @param[in] interfaceID   id of a network interface on which the gateway address should be set
 * @param[in] gateway       a static gateway address to be set on the interface
 **/
void
QBNetworkManagerSetStaticGatewayForInterface(const char *interfaceID,
                                             const char *gateway);

/**
 * Set a static dns1 address on a network interface. The dns1 address will be
 * physically set on the interface if an address provider is set to
 * QBNetworkManagerInterfaceAddressProvider_static (or if the address provider
 * will be set to this value after setting the dns1).
 *
 * Remember to call QBNetworkManagerApplyConf function to apply the setting!
 * This is an asynchronous function.
 *
 * @param[in] interfaceID   id of a network interface on which the gateway address should be set
 * @param[in] dns1          a static dns1 address to be set on the interface
 **/
void
QBNetworkManagerSetStaticDNS1ForInterface(const char *interfaceID,
                                          const char *dns1);

/**
 * Set a static dns2 address on a network interface. The dns2 address will be
 * physically set on the interface if an address provider is set to
 * QBNetworkManagerInterfaceAddressProvider_static (or if the address provider
 * will be set to this value after setting the dns2).
 *
 * Remember to call QBNetworkManagerApplyConf function to apply the setting!
 * This is an asynchronous function.
 *
 * @param[in] interfaceID   id of a network interface on which the dns1 address should be set
 * @param[in] dns2          a static dns2 address to be set on the interface
 **/
void
QBNetworkManagerSetStaticDNS2ForInterface(const char *interfaceID,
                                          const char *dns2);

/**
 * Set a static domain address on a network interface. The domain address will be
 * physically set on the interface if an address provider is set to
 * QBNetworkManagerInterfaceAddressProvider_static (or if the address provider
 * will be set to this value after setting the domain).
 *
 * Remember to call QBNetworkManagerApplyConf function to apply the setting!
 * This is an asynchronous function.
 *
 * @param[in] interfaceID   id of a network interface on which the dns2 address should be set
 * @param[in] domain        a static domain address to be set on the interface
 **/
void
QBNetworkManagerSetStaticDomainForInterface(const char *interfaceID,
                                            const char *domain);

/**
 * Set an address provider (DHCP, static) on a network interface.
 *
 * Remember to call QBNetworkManagerApplyConf function to apply the setting!
 * This is an asynchronous function.
 *
 * @param[in] interfaceID       id of a network interface on which the address provider should be applied
 * @param[in] addressProvider   address provider to be set on the interface
 **/
void
QBNetworkManagerSetAddressProviderForInterface(const char *interfaceID,
                                               QBNetworkManagerInterfaceAddressProvider addressProvider);

/**
 * Set a routing policy (wan, wan+lan, lan) on a network interface.
 *
 * Remember to call QBNetworkManagerApplyConf function to apply the setting!
 * This is an asynchronous function.
 *
 * @param[in] interfaceID       id of a network interface on which the routing policy should be set
 * @param[in] routing           routing policy to be set on the interface
 **/
void
QBNetworkManagerSetRoutingForInterface(const char *interfaceID,
                                       QBNetworkManagerInterfaceRouting routing);

/**
 * Set a gateway tenure (a priority to be a gateway owner) on a network interface.
 *
 * Remember to call QBNetworkManagerApplyConf function to apply the setting!
 * This is an asynchronous function.
 *
 * @param[in] interfaceID       id of a network interface on which the gateway tenure should be set
 * @param[in] gatewayTenure     gateway tenure to be set on the interface
 **/
void
QBNetworkManagerSetGatewayTenureForInterface(const char *interfaceID,
                                             int gatewayTenure);

/**
 * Request the QBNetworkManager to store and apply settings
 * that were introduced by calling following functions:
 * - QBNetworkManagerSetStaticIPForInterface()
 * - QBNetworkManagerSetStaticNetmaskForInterface()
 * - QBNetworkManagerSetStaticGatewayForInterface()
 * - QBNetworkManagerSetStaticDNS1ForInterface()
 * - QBNetworkManagerSetStaticDNS2ForInterface()
 * - QBNetworkManagerSetStaticDomainForInterface()
 * - QBNetworkManagerSetAddressProviderForInterface()
 * - QBNetworkManagerSetRoutingForInterface()
 * - QBNetworkManagerSetGatewayTenureForInterface()
 *
 * The function is not called by above functions by default
 * to make it easy to configure all desired properties of a
 * network interface and apply them at once.
 * This is an asynchronous function.
 **/
void
QBNetworkManagerApplyConf(void);

/**
 * Request the QBNetworkManager to start VPN (via IPC).
 **/
void
QBNetworkManagerStartVPN(void);

/**
 * Request the QBNetworkManager to deliver all data
 * describing current state of networking interfaces (via IPC)
 * This is an asynchronous function.
 **/
void
QBNetworkManagerRecall(void);

/**
 * @}
**/

#endif /* QB_NETWORK_MANAGER_CONTROLLER_H_ */
