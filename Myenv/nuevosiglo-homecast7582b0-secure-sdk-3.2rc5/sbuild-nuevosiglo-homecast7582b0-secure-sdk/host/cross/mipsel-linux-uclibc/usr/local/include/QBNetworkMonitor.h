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

#ifndef QB_NETWORK_MONITOR_H_
#define QB_NETWORK_MONITOR_H_

/**
 * @file QBNetworkMonitor.h Netwok Monitor API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


#include <QBNetworkMonitor/QBWiFiNetwork.h>
#include <QBNetworkMonitor/QBIPSecVPN.h>
#include <QBNetworkMonitor/QBNetworkInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <fibers/c/fibers.h>
#include <stdbool.h>

/**
 * @defgroup QBNetworkMonitor Network Monitor
 * @ingroup CubiTV
 * @{
 **/

#define MIN_WPA_KEY_LENGTH 8
#define MAX_WPA_KEY_LENGTH 63

/** WiFi network connect result type */
typedef enum {
    QBWiFiNetworkParamsCheck_ok = 0,             /**< all ok */
    QBWiFiNetworkParamsCheck_emptyPassword,      /**< password is empty */
    QBWiFiNetworkParamsCheck_invalidWPAPassword, /**< invalid password to security standard */
    QBWiFiNetworkParamsCheck_badParams,          /**< some bad params given  */
} QBWiFiNetworkParamsCheck;

/**
 * Network Monitor manages networking configurations on a high level.
 * It passes network configuration to a network manager process,
 * receives current interfaces' states and provides it to its listeners.
 *
 * @class QBNetworkMonitor
 **/
typedef struct QBNetworkMonitor_ *QBNetworkMonitor;

/**
 * Creates a new instance of Network Monitor.
 *
 * @memberof QBNetworkMonitor
 * @param[in] scheduler SvScheduler handle
 * @return new instance of QBNetworkMonitor
 **/
QBNetworkMonitor
QBNetworkMonitorCreate(SvScheduler scheduler);

/**
 * Starts the instance of Network Monitor.
 *
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 **/

void
QBNetworkMonitorStart(QBNetworkMonitor self);

/**
 * Stops the instance of Network Monitor.
 *
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 **/
void
QBNetworkMonitorStop(QBNetworkMonitor self);

/**
 * Adds a listener to be notified via QBNetworkMonitorListener interface
 *
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @param[in] listener listener object handle that implements the QBNetworkMonitorListener interface
 **/

void
QBNetworkMonitorAddListener(QBNetworkMonitor self, SvObject listener);

/**
 * Removes a listener
 *
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @param[in] listener listener object handle that implements the QBNetworkMonitorListener interface
 **/
void
QBNetworkMonitorRemoveListener(QBNetworkMonitor self, SvObject listener);

/**
 * Saves and commits network parameters of an interface.
 *
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @param[in] interface handle that contains parameters that will be serialized and commited
 **/
void
QBNetworkMonitorSaveNetworkParameters(QBNetworkMonitor self, QBNetworkInterface interface);

/**
 * Return interface of the given ID.
 *
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @param[in] interfaceID id of interface that should be returned
 * @return handle to interface of a given id or null if such interface doesn't exist
 **/
QBNetworkInterface
QBNetworkMonitorGetInterface(QBNetworkMonitor self, SvString interfaceID);

/**
 * Get a handle to IPSec VPN class.
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @return a handle to IPSec VPN class, @c NULL if handle does not exist
 */
QBIPSecVPN
QBNetworkMonitorGetIPSecVPN(QBNetworkMonitor self);

/**
 * Get type of VPN from config.
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor
 * @return VPN type
 */
QBNetworkManagerVPNType
QBNetworkMonitorGetVPNType(QBNetworkMonitor self);

/** Connects to WiFi network on given interface with requested parameters.
 *
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @param[in] interfaceID id of interface that should be used to connect a WiFi network
 * @param[in] ssid SSID of the WiFi network
 * @param[in] passwd password of the WiFi Network, might be NULL if the password is not required
 * @param[in] securityMode type of wireless security that should be applying for connection
 * @param[in] manual true if the user typed the SSID manually
 * @return connect params correctness check result
 **/
QBWiFiNetworkParamsCheck
QBNetworkMonitorConnectWiFiNetwork(QBNetworkMonitor self, SvString interfaceID, SvString ssid, SvString passwd, QBNetworkManagerWiFiNetworkSecurity securityMode, bool manual);

/** Disconnets from WiFi network on given interface.
 *
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @param[in] interfaceID id of interface that should be disconnected from any WiFi network
 **/
void
QBNetworkMonitorDisconnectFromWiFiNetwork(QBNetworkMonitor self, SvString interfaceID);

/** Forgets a password for a given WiFi SSID and disconnects from the SSID on given interface (if connected)
 *
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @param[in] interfaceID id of interface that should be disconnected from any WiFi network
 * @param[in] ssid SSID of the WiFI network
 **/
void
QBNetworkMonitorForgetWiFiNetwork(QBNetworkMonitor self, SvString interfaceID, SvString ssid);

/** Gets last recently used manual WiFi network
 *
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @return last recently used manual WiFI network
 **/
QBWiFiNetwork
QBNetworkMonitorGetManualWiFiNetwork(QBNetworkMonitor self);

/** Request asynchronous WiFi SSIDs scanning on given interface
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @param[in] interfaceID id of interface on which WiFi SSIDs will be scanned
 **/
void
QBNetworkMonitorRequestWiFiScanning(QBNetworkMonitor self, SvString interfaceID);

/** Sets VPN type
 *
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @param[in] vpnType type of VPN to be used by QBNetworkManager
 */
void
QBNetworkMonitorSetVPNType(QBNetworkMonitor self, QBNetworkManagerVPNType vpnType);

/** Tells if there is any interface which is up.
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @return logic value if there is any interface which is up.
 **/
bool
QBNetworkMonitorIsAnyInterfaceConnected(QBNetworkMonitor self);

/** Recalls all interfaces, by calling interfaceAdded interface on the given listener.
 * Given listener must be valid (previously added) Network Monitor listener.
 * @memberof QBNetworkMonitor
 * @param[in] self QBNetworkMonitor instance handle
 * @param[in] listener valid listener of Network Monitor
 **/
void
QBNetworkMonitorRecallInterfaces(QBNetworkMonitor self, SvObject listener);

/**
 * @}
**/

#endif //QB_NETWORK_MONITOR_H_
