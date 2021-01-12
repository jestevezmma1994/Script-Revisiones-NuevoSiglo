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

#ifndef QB_NETWORK_MANAGER_TYPES_H_
#define QB_NETWORK_MANAGER_TYPES_H_

/**
 * @file QBNetworkManagerTypes.h Types used by QBNetworkManager and processes
 * that use the APIs of the QBNetworkManager controllers
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBNetworkManagerTypes Network Manager types
 * @ingroup QBNetworkManager
 * @{
 **/

/**
 * States of a network interface
 **/
typedef enum {
    QBNetworkManagerInterfaceStatus_notPresent,     /**< interface not available */
    QBNetworkManagerInterfaceStatus_disconnected,   /**< interface disconnected */
    QBNetworkManagerInterfaceStatus_obtainingLink,  /**< interface is connecting (means different things on a different types of interfaces) */
    QBNetworkManagerInterfaceStatus_linkObtained,   /**< interface has link obtained */
    QBNetworkManagerInterfaceStatus_obtainingIP,    /**< interface is obtaining IP parameters */
    QBNetworkManagerInterfaceStatus_connected       /**< interface is connected and has an IP parameters assigned */
} QBNetworkManagerInterfaceStatus;

/**
 * Mode of a network interface. Fallback mode is applied when failed
 * to set up the network interface with given parameters. Panic mode
 * is applied when cannot apply fallback configuration. IP settings of
 * the panic mode are hardcoded.
 **/
typedef enum {
    QBNetworkManagerInterfaceConfig_normal,     /**< normal mode - parameters are taken from QBNetworkManagerConf.json file */
    QBNetworkManagerInterfaceConfig_fallback,   /**< fallback mode - parameters are taken from QBNetworkManagerConf.json file (from fallback values) */
    QBNetworkManagerInterfaceConfig_panic,      /**< panic mode - parametrs are hardcoded in the source code and allow to ssh to the STB */
    QBNetworkManagerInterfaceConfig_cnt         /**< count of interface configurations*/
} QBNetworkManagerInterfaceConfig;

/**
 * Routing policy of a network interface
 **/
typedef enum {
    QBNetworkManagerInterfaceRouting_LAN,      /**< LAN only routing */
    QBNetworkManagerInterfaceRouting_WAN,      /**< WAN only routing */
    QBNetworkManagerInterfaceRouting_WAN_LAN,  /**< WAN+LAN routing */
    QBNetworkManagerInterfaceRouting_PPP,      /**< ppp routing (set gateway to 0.0.0.0) */
} QBNetworkManagerInterfaceRouting;

/**
 * Address provider to be used on a network interface
 **/
typedef enum {
    QBNetworkManagerInterfaceAddressProvider_dhcp,    /**< DHCP */
    QBNetworkManagerInterfaceAddressProvider_ppp,     /**< PPP */
    QBNetworkManagerInterfaceAddressProvider_static,  /**< static */
    QBNetworkManagerInterfaceAddressProvider_cnt      /**< count of types of address providers */
} QBNetworkManagerInterfaceAddressProvider;

/**
 * VPN type
 **/
typedef enum {
    QBNetworkManagerVPNType_none,       /**< VPN not enabled */
    QBNetworkManagerVPNType_racoon,     /**< racoon IPSec VPN */
    QBNetworkManagerVPNType_strongswan, /**< strongswan IPSec VPN */
} QBNetworkManagerVPNType;

/**
 * VPN start mode
 **/
typedef enum {
    QBNetworkManagerVPNStartMode_manual,       /**< VPN must be started manually */
    QBNetworkManagerVPNStartMode_automatic,    /**< VPN is started automatically by QBNetworkManager */
} QBNetworkManagerVPNStartMode;

/**
 * States of a IPSec VPN connection
 **/
typedef enum {
    QBNetworkManagerIPSecVPNStatus_disabled,     /**< IPSec VPN disabled */
    QBNetworkManagerIPSecVPNStatus_disconnected, /**< IPSec VPN disconnected */
    QBNetworkManagerIPSecVPNStatus_connecting,   /**< connecting to IPSec VPN in progress */
    QBNetworkManagerIPSecVPNStatus_connected     /**< IPSec VPN connected */
} QBNetworkManagerIPSecVPNStatus;

/**
 * IPSec VPN authentication method
 **/
typedef enum {
    QBNetworkManagerIPSecVPNAuthenticationMethod_none,      /**< no authentication */
    QBNetworkManagerIPSecVPNAuthenticationMethod_XAuthPSK,  /**< xAuth-PSK */
} QBNetworkManagerIPSecVPNAuthenticationMethod;

/**
 * Medium of a network interface
 **/
typedef enum {
    QBNetworkManagerInterfaceMedium_unknown,    /**< unknown */
    QBNetworkManagerInterfaceMedium_loopback,   /**< loopback */
    QBNetworkManagerInterfaceMedium_ethernet,   /**< ethernet */
    QBNetworkManagerInterfaceMedium_WiFi,       /**< Wi-Fi */
    QBNetworkManagerInterfaceMedium_MoCa,       /**< MoCa */
    QBNetworkManagerInterfaceMedium_docsis,     /**< DOCSIS */
    QBNetworkManagerInterfaceMedium_3G          /**< 3G */
} QBNetworkManagerInterfaceMedium;

/**
 * WiFi network security type
 **/
typedef enum {
    QBNetworkManagerWiFiNetworkSecurity_none,   /**< no WiFi security */
    QBNetworkManagerWiFiNetworkSecurity_WEP,    /**< WEP security */
    QBNetworkManagerWiFiNetworkSecurity_WPA     /**< WPA/WPA2 security */
} QBNetworkManagerWiFiNetworkSecurity;

/**
 * IPC notifications (sent by the QBNetworkManager)
 **/
typedef enum {
    QBNetworkManagerOutgoingMessage_unknown,                /**< unknown type of message */
    QBNetworkManagerOutgoingMessage_interfaceChanged,       /**< a new network interface has been added, setup or removed */
    QBNetworkManagerOutgoingMessage_ipSecVPNStatusChanged,  /**< VPN status has changed */
    QBNetworkManagerOutgoingMessage_wifiNetworkScanned,     /**< a WiFi network has been scanned */
    QBNetworkManagerOutgoingMessage_wifiNetworkLost,        /**< a WiFi network is no longer available */
    QBNetworkManagerOutgoingMessage_wifiConfStored,         /**< a WiFi network configuration has just been remembered */
    QBNetworkManagerOutgoingMessage_wifiConfDeleted,        /**< a WiFi network configuration has just been deleted */
} QBNetworkManagerOutgoingMessage;

/**
 * WiFi disconnection reasons
 **/
typedef enum {
    QBNetworkManagerWiFiConnectionError_noError,            /**< no error occured - disconnection was made by user or an AP */
    QBNetworkManagerWiFiConnectionError_wrongKey,           /**< PSK or WEP key does not match */
    QBNetworkManagerWiFiConnectionError_connFailed,         /**< connection failed due to unknown reason */
    QBNetworkManagerWiFiConnectionError_authFailed,         /**< authentication to an AP failed */
    QBNetworkManagerWiFiConnectionError_timeout,            /**< connection failed due to a timeout was reached */
    QBNetworkManagerWiFiConnectionError_noSSIDInfoFound     /**< connection failed, because specified SSID could not be found */
} QBNetworkManagerWiFiConnectionError;

/**
 * Docsis events (link obtaining is performed by an external process)
 **/
typedef enum {
    QBNetworkManagerInterfaceDocsisEvent_initial,           /**< no docsis event has been received yet */
    QBNetworkManagerInterfaceDocsisEvent_linkFailed,        /**< cannot obtain link on a docsis interface */
    QBNetworkManagerInterfaceDocsisEvent_obtainingLink,     /**< trying to obtain the link */
    QBNetworkManagerInterfaceDocsisEvent_linkObtained       /**< link obtaining completed */
} QBNetworkManagerInterfaceDocsisEvent;

/**
 * @}
**/

#endif /* QB_NETWORK_MANAGER_TYPES_H_ */
