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
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#ifndef QB_NETWORK_WATCHER_H_
#define QB_NETWORK_WATCHER_H_

/**
 * @file QBNetworkWatcher.h QBNetworkWatcher class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBNetworkMonitor.h>

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvErrorInfo.h>

#include <stdbool.h>

/**
* @defgroup QBNetworkWatcher Network State Watcher
* @ingroup CubiTV_services
* @{
*
**/

/**
* Diagnosis of interface's state.
**/
typedef enum {
    QBNetworkWatcherInterfaceDiagnosis_noDiagnosis = 0, /**< interface has not been diagnosed yet */
    QBNetworkWatcherInterfaceDiagnosis_disconnected,    /**< no carrier */
    QBNetworkWatcherInterfaceDiagnosis_noLAN,           /**< has carrier, but no IP and netmask */
    QBNetworkWatcherInterfaceDiagnosis_noWAN,           /**< has carrier, IP and netmask, but no gateway */
    QBNetworkWatcherInterfaceDiagnosis_noDNS,           /**< has carrier, IP, netmask and gateway, but no DNS */
    QBNetworkWatcherInterfaceDiagnosis_ok,              /**< has carrier, IP, netmask, gateway and DNS */
} QBNetworkWatcherInterfaceDiagnosis;

/**
 * Diagnosis of VPN state.
 **/
typedef enum {
    QBNetworkWatcherVPNDiagnosis_noDiagnosis = 0,   /**< VPN has not been diagnosed yet  */
    QBNetworkWatcherVPNDiagnosis_disconnected,      /**< not connected to VPN server */
    QBNetworkWatcherVPNDiagnosis_ok,                /**< connected to VPN server */
} QBNetworkWatcherVPNDiagnosis;

/**
 * Overall diagnosis of networking connectivity.
 **/
typedef enum {
    QBNetworkWatcherOverallDiagnosis_noDiagnosis = 0,   /**< connectivity was not diagnosed yet */
    QBNetworkWatcherOverallDiagnosis_disconnected,      /**< no interface has carrier */
    QBNetworkWatcherOverallDiagnosis_noLAN,             /**< LAN connection is not available */
    QBNetworkWatcherOverallDiagnosis_noWAN,             /**< WAN connection is not available */
    QBNetworkWatcherOverallDiagnosis_noDNS,             /**< DNS is not assigned */
    QBNetworkWatcherOverallDiagnosis_noVPN,             /**< VPN server is not connected */
    QBNetworkWatcherOverallDiagnosis_ok,                /**< everything is ok */
    QBNetworkWatcherOverallDiagnosis_noInterfaces       /**< there are no interfaces connected */
} QBNetworkWatcherOverallDiagnosis;

/**
* @class QBNetworkWatcherRequirements
*
* A set of network parameters that must be met
* to assume that the connectivity is alright.
**/
typedef struct QBNetworkWatcherRequirements_ {
    bool needsLink;                     /**< link on a network interface is required to meet the requirements */
    bool needsLAN;                      /**< LAN connection is required to meet the requirements */
    bool needsWAN;                      /**< WAN connection is required to meet the requirements */
    bool needsDNS;                      /**< DNS entry is required to meet the requirements */
    QBNetworkManagerVPNType vpnType;    /**< VPN type required to meet the requirements */
    int jitterToleranceMs;              /**< Tolerance for connection's state changes is required to meet the requirements */
} QBNetworkWatcherRequirements;

/**
* @class QBNetworkWatcher
*
* The service checks if network connectivity is broken and reports
* possible reasons to its listeners. The service listens to
* QBNetworkMonitor and if any networking parameters changes
* it starts a procedure to check if all services are available.
* After every check it notifies its listeners about current
* networking status (if the status has changed).
*
* The service assumes that the connection is broken if all
* interfaces are not compliant with requirements or if VPN
* is required and it is not setup. If at least one interface
* is compliant with requirements, the overall diagnosis is
* assumed as okay.
*
* The service has configurable duration of jitter filter
* (Set via QBNetworkWatcherRequirements.jitterToleranceMs:
* non-positive values are replaced with default 1 sec)
* (in case of very short network state changes)
* not to notify its listners if there is no need to this.
*
* The VPN connection problem is discovered 20 seconds after all other
* requirements seems to be fine for the VPN to be setup.
* VPN disconnection is discovered immediately (with the
* set tolerance  of jitter filter), when VPN state changes and the
* service gets direct notification from QBNetworkMonitor.
*
* There are two types of issues that might occur:
* - interface setup problem
* - vpn setup problem
*
* These states are merged into overall diagnosis.
* The listener can retrieve each diagnosis with
* appropriate methods.
*
**/
typedef struct QBNetworkWatcher_ *QBNetworkWatcher;

/**
 * Create an instance of QBNetworkWatcher class.
 *
 * @param[in] networkMonitor    QBNetworkMonitor handle
 * @param[in] requirements      connection requirements
 * @param[out] errorOut         error information
 * @return  network watcher instance handle
 **/
QBNetworkWatcher
QBNetworkWatcherCreate(QBNetworkMonitor networkMonitor,
                       QBNetworkWatcherRequirements *requirements,
                       SvErrorInfo *errorOut);

/**
 * Add a listener of the QBNetworkWatcher service. The listener needs to implement
 * QBObserver interface.
 *
 * @param[in] self      QBNetworkWatcher handle
 * @param[in] listener  listener object to be added
 * @param[out] errorOut error info
 */
void
QBNetworkWatcherAddListener(QBNetworkWatcher self, SvObject listener, SvErrorInfo *errorOut);

/**
 * Remove a listener of QBNetworkWatcher service.
 *
 * @param[in] self      QBNetworkWatcher handle
 * @param[in] listener  listener object to be added
 * @param[out] errorOut error info
 */
void
QBNetworkWatcherRemoveListener(QBNetworkWatcher self, SvObject listener, SvErrorInfo *errorOut);

/**
 * Get network interface which is owner of default gateway.
 *
 * @param[in] self      QBNetworkWatcher handle
 * @param[out] errorOut error info
 *
 * @return owner of default gateway, @c NULL in case of error or lack of working gateway
 */
QBNetworkInterface
QBNetworkWatcherGetGatewayOwner(QBNetworkWatcher self, SvErrorInfo *errorOut);

/**
 * Get current overall diagnosis of networking status.
 *
 * @param[in] self  QBNetworkWatcher handle
 * @return current overall diagnosis of networking status
 **/
QBNetworkWatcherOverallDiagnosis
QBNetworkWatcherGetOverallDiagnosis(QBNetworkWatcher self);

/**
 * Get ID of a network interface for which the overall diagnosis has been made.
 * If there is more than one interface, then the overall diagnosis is prepared
 * for the interface that has the nearest compliance with requirements.
 *
 * @param[in] self  QBNetworkWatcher handle
 * @return  ID of a network interface for which the overall diagnosis has been made
 **/
SvString
QBNetworkWatcherGetDiagnosedInterfaceID(QBNetworkWatcher self);

/**
 * Get diagnosis of a network interface. The diagnosis is about
 * the interface's state only.
 *
 * @param[in] self  QBNetworkWatcher handle
 * @param[in] interfaceID   ID of a network interface for which the diagnose should be returned
 * @param[out] errorOut error info
 * @return  diagnosis of a network interface
 **/
QBNetworkWatcherInterfaceDiagnosis
QBNetworkWatcherGetDiagnosisForInterface(QBNetworkWatcher self,
                                         SvString interfaceID,
                                         SvErrorInfo *errorOut);

/**
 * Get diagnosis of a VPN. The diagnosis is about the VPN's state only.
 *
 * @param[in] self  QBNetworkWatcher handle
 * @return  diagnosis of a VPN
 **/
QBNetworkWatcherVPNDiagnosis
QBNetworkWatcherGetVPNDiagnosis(QBNetworkWatcher self);

/**
* @}
**/

#endif /* QB_NETWORK_WATCHER_H_ */
