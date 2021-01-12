/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_NET_MANAGER_H_
#define QB_NET_MANAGER_H_

/**
 * @file QBNetManager.h
 * @brief Network Manager library API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <fibers/c/fibers.h>
#include <QBNetManager/QBNetManagerConnection.h>
#include <QBNetworkMonitor.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup NetworkManager QBNetManager: Network Manager library
 * @ingroup Utils
 * @{
 *
 * Network Manager library provides a service for reading and monitoring
 * network configuration.
 **/


/**
 * @defgroup NetworkManagerErrorDomain Network Manager error domain
 * @{
 **/

/**
 * Network Manager error domain.
 **/
#define QBNetManagerErrorDomain "com.cubiware.QBNetManager"

/**
 * Network manager error codes.
 **/
typedef enum {
   /** reserved value, not an actual error code */
   QBNetManagerError_OK = 0,
   /** attribute unsupported on an interface */
   QBNetManagerError_badAttribute,
   /** invalid type of an attribute's value */
   QBNetManagerError_badValueType,
   /** invalid value for an attribute */
   QBNetManagerError_badValue,
   /** attribute can not be modified */
   QBNetManagerError_readOnly,
   /** out of memory for storing attributes */
   QBNetManagerError_noMemory,
   /** system error */
   QBNetManagerError_systemError,
   /** @cond */
   QBNetManagerError_last = QBNetManagerError_systemError
   /** @endcond */
} QBNetManagerError;

/**
 * @}
 **/

/**
 * Network manager displaying policy.
 **/
typedef enum {
   /** display all interfaces */
    QBNetManagerInterfacesDisplayingPolicy_all = 0,
   /** display used only interfaces */
    QBNetManagerInterfacesDisplayingPolicy_usedOnly,
} QBNetManagerDisplayPolicy;

/**
 * @}
 **/

/**
 * @defgroup NetworkManagerTypes Network Manager data types
 * @{
 **/

/**
 * Network interface types.
 **/
typedef enum {
   /** unknown interface type */
   QBNetIfaceType_unknown = 0,
   /** loopback interface */
   QBNetIfaceType_loopback = 1,
   /** IEEE 802.3 Ethernet */
   QBNetIfaceType_Ethernet = 2,
   /** IEEE 802.1q Ethernet VLAN (unused) */
   QBNetIfaceType_EtherVLAN = 3,
   /** IEEE 802.11 wireless Ethernet */
   QBNetIfaceType_WiFi = 4,
   /** SIT (IPv6 over IPv4) tunnel */
   QBNetIfaceType_SIT = 5,
   /** PPPoE (PPP over Ethernet), used in ADSL */
   QBNetIfaceType_PPPoE = 6,
   /** PPPoA (PPP over ATM), used in ADSL */
   QBNetIfaceType_PPPoA = 7,
   /** some other type */
   QBNetIfaceType_other = 15
} QBNetIfaceType;

/**
 * Medium types, mostly for Ethernet and WiFi.
 **/
typedef enum {
   /** unknown medium */
   QBNetIfaceMedium_unknown = 0,
   /** old 'thin' 10Base2 Ethernet */
   QBNetIfaceMedium_10Base2 = 1,
   /** 10Base-T Ethernet */
   QBNetIfaceMedium_10Base_T = 10,
   /** 100Base-TX Ethernet */
   QBNetIfaceMedium_100Base_TX = 100,
   /** 1000Base-T Ethernet */
   QBNetIfaceMedium_1000Base_T = 1000,
   /** 802.11b WiFi */
   QBNetIfaceMedium_802_11b = 22,
   /** 802.11g WiFi */
   QBNetIfaceMedium_802_11g = 54,
   /** 802.11n WiFi */
   QBNetIfaceMedium_802_11n = 250 // this value does not mean anything
} QBNetIfaceMedium;

/**
 * Attributes of network interfaces.
 **/
typedef enum {
   // basic attributes, mostly available on all interfaces

   /** name of the interface, assigned by operating system */
   QBNetAttr_name = 0,
   /** interface type (as integer from QBNetIfaceType range) */
   QBNetAttr_type = 1,
   /** boolean flag marking that an interface is enabled */
   QBNetAttr_enabled = 2,
   /** hardware address (as string), for example: Ethernet MAC */
   QBNetAttr_hwAddr = 3,
   /** boolean flag describing link status */
   QBNetAttr_linkStatus = 4,
   /** Maximum Transfer Unit as integer */
   QBNetAttr_MTU = 5,
   /** medium type as QBNetIfaceMedium integer */
   QBNetAttr_medium = 6,
   /** OS-specific interface index as integer */
   QBNetAttr_index = 7,

   /** list of DNS search domains (whitespace-separated string) */
   QBNetAttr_dnsSearchList = 8,
   /** address of the first DNS server (IPv4 or IPv6 address as string) */
   QBNetAttr_dns1 = 9,
   /** address of the second DNS server (IPv4 or IPv6 address as string) */
   QBNetAttr_dns2 = 10,

   /** IPv4 address in dotted-decimal notation (as string) */
   QBNetAttr_IPv4_address = 13,
   /** IPv4 network mask in dotted-decimal notation (as string) */
   QBNetAttr_IPv4_netMask = 14,
   /** IPv4 gateway address in dotted-decimal notation (as string) */
   QBNetAttr_IPv4_gateway = 15,
   /** IPv4 broadcast address in dotted-decimal notation (as string) */
   QBNetAttr_IPv4_broadcast = 16,

   /** comma-separated list of IPv6 addresses in colon-separated notation,
    *  ordered by scope value (as string) */
   QBNetAttr_IPv6_addresses = 20,
   /** default IPv6 gateway address (as string) */
   QBNetAttr_IPv6_gateway = 21,

   // attributes specific for particular interface types

   /** VLAN-specific: base Ethernet interface name (as string) */
   QBNetAttr_VLAN_baseIface = 32,
   /** VLAN-specific: 802.1q VLAN ID (as integer) */
   QBNetAttr_VLAN_ID = 33,

   /** WiFi-specific: current network ESSID */
   QBNetAttr_WiFi_network = 33,
   /** WiFi-specific: MAC address of the access point */
   QBNetAttr_WiFi_accessPoint = 34,
   /** WiFi-specific: radio channel number */
   QBNetAttr_WiFi_channel = 35,
   /** WiFi-specific: signal quality as float in 0.0 - 1.0 range */
   QBNetAttr_WiFi_linkQuality = 36,
   /** WiFi-specific: encryption used as QBNetIfaceEncryption integer */
   QBNetAttr_WiFi_encryption = 37,
   /** WiFi-specific: passphrase for selected ESSID */
   QBNetAttr_WiFi_password = 38,
   /** WiFi-specific: number of times networks were scanned */
   QBNetAttr_WiFi_scanCount = 39,
   /** WiFi-specific: list of available networks, as an array of objects
    *  with attributes from "20" to "24" */
   QBNetAttr_WiFi_networks = 40,

   /** PPPoE-specific: base Ethernet interface name (as string) */
   QBNetAddr_PPPoE_baseIface = 32,
   /** PPPoE-specific: service name (as string) */
   QBNetAddr_PPPoE_service = 33,
   /** PPPoE-specific: MAC address of the access concentrator */
   QBNetAttr_PPPoE_concentrator = 34,
   /** PPPoE-specific: session ID */
   QBNetAttr_PPPoE_sessionID = 35,
   /** PPPoE-specific: user name for authorization (as string) */
   QBNetAddr_PPPoE_userName = 37,
   /** PPPoE-specific: password for authorization (as string) */
   QBNetAddr_PPPoE_password = 38,
   /** PPPoE-specific: list of available services, as an array of objects
    *  with attributes "32", "33" and "34" */
   QBNetAddr_PPPoE_services = 40
} QBNetAttribute;

/**
 * @}
 **/


/**
 * @defgroup QBNetListener Network Manager listener interface
 * @{
 **/

/**
 * Network listener interface.
 **/
typedef const struct QBNetListener_t {
   /**
    * Notify that some attributes of a network interface
    * have changed.
    *
    * @param[in] self_     listener handle
    * @param[in] iface     name of the interface
    * @param[in] mask      bitmask marking changed attributes
    **/
   void (*netAttributeChanged)(SvObject self_,
                               SvString iface,
                               uint64_t mask);
} *QBNetListener;


/**
 * Get runtime type identification object
 * representing Network Listener interface.
 *
 * @return QBNetListener interface object
 **/
extern SvInterface
QBNetListener_getInterface(void);

/**
 * @}
 **/


/**
 * @defgroup QBNetWiFiNetworkListener WiFi network listener interface
 * @{
 **/

/**
 * WiFi network listener interface.
 **/
typedef const struct QBNetWiFiNetworkListener_t {
   /**
    * Notify that the list of WiFi networks have changed.
    *
    * @param[in] self_    listener handle
    * @param[in] iface    name of the interface
    * @param[in] networks list of QBNetWiFiNetwork instances that are visible through @a iface
    **/
   void (*networkListChanged)(SvObject self_,
                              SvString iface,
                              SvArray networks);
} *QBNetWiFiNetworkListener;

/**
 * Get runtime type identification object
 * representing WiFi network listener interface.
 *
 * @return QBNetWiFiNetworkListener interface object
 **/
extern SvInterface
QBNetWiFiNetworkListener_getInterface(void);

/**
 * @}
 **/


/**
 * @defgroup QBNetWiFiNetwork WiFi network class
 * @{
 **/

/**
 * WiFi network class.
 * @class QBNetWiFiNetwork
 * @extends SvObject
 *
 * An instance of this class represents single WiFi network.
 */
typedef struct QBNetWiFiNetwork_t {
    /// base class
    struct SvObject_ super_;

    SvString ssid;
    SvString bssid;
    unsigned bitrate_bps;   /**< highest available bitrate [b/s], for example: 54000000 for 54Mbps */
    unsigned channel;       /**< radio channel in use */
    unsigned rssi;          /**< received signal strength indication */
    unsigned quality;       /**< signal quality (in %) */
    int level;              /**< signal strength level [dBm] (1dBm equals power of 1mW) */
    int noise;              /**< background noise strength [dBm] */
    /// encryption type
    enum {
        WIFI_ENC_UNKNOWN=0,
        WIFI_ENC_NONE=1,
        WIFI_ENC_OPEN=1,
        WIFI_ENC_WEP=0x2,
        WIFI_ENC_TKIP=0x4,
        WIFI_ENC_AES=0x8,
    } encryption;
    enum {
        WIFI_AUTH_UKNOWN=0,
        WIFI_AUTH_OPEN=0x1,
        WIFI_AUTH_WEP=0x2,
        WIFI_AUTH_WPA=0x4,
        WIFI_AUTH_WPA_PSK=0x8,
        WIFI_AUTH_WPA2=0x10,
        WIFI_AUTH_WPA2_PSK=0x20,
    } authorization;
    enum {
        WIFI_MODE_UNKNOWN=0,
        WIFI_MODE_B=0x1,
        WIFI_MODE_G=0x2,
        WIFI_MODE_BG=0x3,
        WIFI_MODE_N=0x4,
        WIFI_MODE_BGN=0x7,
    } mode;
} *QBNetWiFiNetwork;

/**
 * Get runtime type identification object
 * representing WiFi network class.
 *
 * @return QBNetWiFiNetwork type identification object
 **/
extern SvType
QBNetWiFiNetwork_getType(void);

/**
 * @}
 **/


/**
 * @defgroup QBNetManager Network Manager class
 * @{
 **/

/**
 * Network Manager class.
 * @class QBNetManager
 * @extends SvObject
 **/
typedef struct QBNetManager_ *QBNetManager;


/**
 * Get runtime type identification object
 * representing Network Manager class.
 *
 * @return QBNetManager type identification object
 **/
extern SvType
QBNetManager_getType(void);


/**
 * Initialize network manager instance.
 *
 * @param[in] self              network manager handle
 * @param[in] networkMonitor    network monitor handle
 * @param[in] displayPolicy     display policy
 * @param[out] errorOut         error info
 * @return                      @a self or @c NULL in case of error
 **/
extern QBNetManager
QBNetManagerInit(QBNetManager self,
                 QBNetworkMonitor networkMonitor,
                 QBNetManagerDisplayPolicy displayPolicy,
                 SvErrorInfo *errorOut);

/**
 * Poll for changes on network interfaces for one single time.
 *
 * @param[in] self              network manager handle
 * @param[out] errorOut         error info
 **/
extern void
QBNetManagerPollStep(QBNetManager self,
                     SvErrorInfo *errorOut);

/**
 * Start polling for changes on network interfaces.
 *
 * @param[in] self              network manager handle
 * @param[in] scheduler         handle to a scheduler to be used for polling
 *                              (@c NULL to use default one)
 * @param[in] period            poll period in milliseconds
 * @param[out] errorOut         error info
 **/
extern void
QBNetManagerStart(QBNetManager self,
                  SvScheduler scheduler,
                  unsigned int period,
                  SvErrorInfo *errorOut);

/**
 * Stop polling for changes on network interfaces.
 *
 * @param[in] self              network manager handle
 * @param[out] errorOut         error info
 **/
extern void
QBNetManagerStop(QBNetManager self,
                 SvErrorInfo *errorOut);

/**
 * Add or remove an interface to/from the list
 * of ignored interfaces.
 *
 * @param[in] self              network manager handle
 * @param[in] iface             name of the interface
 * @param[in] ignore            @c true to ignore this interface
 * @param[out] errorOut         error info
 **/
extern void
QBNetManagerMaskInterface(QBNetManager self,
                          SvString iface,
                          bool ignore,
                          SvErrorInfo *errorOut);

/**
 * Register network listener for notifications about
 * changes on some network interface.
 *
 * @param[in] self              network manager handle
 * @param[in] listener          network listener handle (an object implementing
 *                              @ref QBNetListener)
 * @param[in] iface             name of the interface this listener
 *                              is interested in (@c NULL for all interfaces)
 * @param[out] errorOut         error info
 **/
extern void
QBNetManagerAddListener(QBNetManager self,
                        SvObject listener,
                        SvString iface,
                        SvErrorInfo *errorOut);

/**
 * Unregister network listener from notifications
 * about changes on some network interface.
 *
 * @param[in] self              network manager handle
 * @param[in] listener          network listener handle, previously registered
 *                              using QBNetManagerAddListener()
 * @param[in] iface             name of the interface this listener
 *                              is no longer interested in
 * @param[out] errorOut         error info
 **/
extern void
QBNetManagerRemoveListener(QBNetManager self,
                           SvObject listener,
                           SvString iface,
                           SvErrorInfo *errorOut);

/**
 * Create list of available interface names.
 *
 * @param[in] self              network manager handle
 * @param[out] errorOut         error info
 * @return                      array of interface names as SvString
 *                              (released by the caller), @c NULL on error
 **/
extern SvArray
QBNetManagerCreateInterfacesList(QBNetManager self,
                                 SvErrorInfo *errorOut);

/**
 * Get value of a single attribute for some network interface.
 *
 * This method returns a value of a network interface attribute.
 * Caller is responsible for releasing returned value.
 *
 * @param[in] self      network manager handle
 * @param[in] iface     name of the interface
 * @param[in] attr      attribute to get
 * @param[out] errorOut error info
 * @return              value of the attribute, @c NULL on error
 **/
extern SvObject
QBNetManagerGetAttribute(QBNetManager self,
                         SvString iface,
                         QBNetAttribute attr,
                         SvErrorInfo *errorOut);

/**
 * Get HW address of the network interface.
 *
 * @param[in] iface     name of the interface
 * @param[out] address  hardware address of the interface
 * @param[in] maxLength number of bytes that can be written to @a address
 * @param[out] errorOut error info
 * @return              number of bytes of the HW address (can be greater than
 *                      @a maxLength), @c -1 in case of error
 **/
extern size_t
QBNetManagerGetHWAddress(SvString iface,
                         char *address,
                         size_t maxLength,
                         SvErrorInfo *errorOut);

/**
 * Get name of the default IPv4 interface.
 *
 * This method returns the name of the interface that is used
 * for communication with the default gateway.
 *
 * @param[in] self      network manager handle
 * @param[out] errorOut error info
 * @return              name of the default interface,
 *                      @c NULL if unavailable
 **/
extern SvString
QBNetManagerGetDefaultIPv4Interface(QBNetManager self,
                                    SvErrorInfo *errorOut);

/**
 * Teardown all connections registered by QBNetManagerAddConnection.
 *
 * @param[in] self      network manager handle
 **/
void
QBNetManagerTeardownConnections(QBNetManager self);

/**
 * Start tracking a single connection.
 *
 * When the network status changes 'a lot', then some connections need
 * to be restarted.
 *
 * Network manager keeps track of registered connections using weak references.
 *
 * @param[in] self      network manager handle
 * @param[in] connection connection to track
 **/
extern void
QBNetManagerAddConnection(QBNetManager self,
                          QBNetManagerConnection connection);

/**
 * Stop tracking a connection.
 *
 * @param[in] self      network manager handle
 * @param[in] connection connection registered previously by QBNetManagerAddConnection()
 **/
extern void
QBNetManagerRemoveConnection(QBNetManager self,
                             QBNetManagerConnection connection);

/**
 * Register the global network manager instance.
 *
 * @param [in] self    network manager handle
 **/
extern void
QBNetManagerSetGlobal(QBNetManager self);

/**
 * Get the global network manager instance.
 *
 * @return              network manager handle registered using QBNetManagerSetGlobal()
 **/
extern QBNetManager
QBNetManagerGetGlobal(void);

/**
 * @}
 **/

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
