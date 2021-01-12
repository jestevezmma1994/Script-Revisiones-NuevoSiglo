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

#ifndef QB_WIFI_NETWORK_H_
#define QB_WIFI_NETWORK_H_

/**
 * @file QBWiFiNetwork.h WiFi network API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBNetworkMonitor/QBNetworkInterfaceTypes.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvWeakReference.h>
#include <QBNetworkManager/Utils/QBNetworkManagerTypes.h>

/**
 * @defgroup QBWiFiNetwork WiFi network
 * @ingroup QBNetworkMonitor
 * @{
 **/

/** WiFi network association state */
typedef enum {
    QBWiFiNetwork_disconnected = 0, /**< disconnected */
    QBWiFiNetwork_connecting, /**< connecting */
    QBWiFiNetwork_connected, /**< connected */
} QBWiFiNetworkConnectionState;


/** WiFi network signal level type (level in ascending order).
 *
 *  Wireless signal strength is traditionally measured in either percentile or dBm
 *  (the power ratio in decibels of the measured power referenced to one milliwatt.)
 *  The level of 100% is equivalent to the signal level of -35 dBm and higher,
 *  e.g. both -25 dBm and -15 dBm will be shown as 100%, because this level of signal is very high.
 *  The level of 1% is equivalent to the signal level of -95 dBm. Between -95 dBm and -35 dBm,
 *  the percentage scale is linear, i.e. 50% is equivalent to -65 dBm. */
typedef enum {
    QBWiFiNetworkSignalLevel_veryWeak = -80, /**< below -80dBm */
    QBWiFiNetworkSignalLevel_weak = -72, /**< above or equal -72dBm */
    QBWiFiNetworkSignalLevel_moderate = -64, /**< above or equal -64dBm */
    QBWiFiNetworkSignalLevel_good = -57, /**< above or equal -57dBm */
    QBWiFiNetworkSignalLevel_veryGood = -50, /**< above or equal -50dBm */
} QBWiFiNetworkSignalLevel;

/**
 * WiFi network is a class that represents scanned WiFi SSID and
 *
 * @class QBWiFiNetwork
 **/
typedef struct QBWiFiNetwork_ *QBWiFiNetwork;

/**
 * Get runtime type identification object representing type of QBWiFiNetwork class.
 * @return QBWiFiNetwork class type handle
 **/
SvType
QBWiFiNetwork_getType(void);

/**
 * Create a new instance of an object representing a WiFi network
 *
 * @param[in] interface       origin interface for created network
 * @param[in] ssid            WiFi network SSID
 * @param[in] passwd          WiFi network authentication key
 * @param[in] flags           security flags, should be provided in a format provided by wpa_supplicant
 * @param[in] level           signal level in dB
 * @param[in] connectionState current connection state of the WiFi network
 * @param[in] securityMode    security mode of the WiFi network
 * @return new instance of QBWiFiNetwork
 **/
QBWiFiNetwork
QBWiFiNetworkCreate(QBWiFiInterface interface,
                    SvString ssid,
                    SvString passwd,
                    SvString flags,
                    int level,
                    QBWiFiNetworkConnectionState connectionState,
                    QBNetworkManagerWiFiNetworkSecurity securityMode);

/**
 * Assign SSID to a WiFi network
 *
 * @param[in] self WiFi network handle
 * @param[in] ssid SSID to be set
 **/
void
QBWiFiNetworkSetSSID(QBWiFiNetwork self, SvString ssid);

/**
 * Get SSID of a WiFi network
 *
 * @param[in] self  WiFi network handle
 * @return SSID of a WiFi network
 **/
SvString
QBWiFiNetworkGetSSID(QBWiFiNetwork self);

/**
 * Assign authentication password to a WiFi network
 *
 * @param[in] self     WiFi network handle
 * @param[in] password authentication password to be set
 **/
void
QBWiFiNetworkSetPassword(QBWiFiNetwork self, SvString password);

/**
 * Get authentication password of a WiFi network
 *
 * @param[in] self  WiFi network handle
 * @return authentication password of a WiFi network
 **/
SvString
QBWiFiNetworkGetPassword(QBWiFiNetwork self);

/**
 * Set security flags to a WiFi network
 *
 * @param[in] self  WiFi network handle
 * @param[in] flags security flags (should be passed in '[]' brackets, e.g., [WPA-PSK-TKIP+CCMP][WPA2-PSK-TKIP+CCMP][ESS])
 **/
void
QBWiFiNetworkSetFlags(QBWiFiNetwork self, SvString flags);

/**
 * Get security flags of a WiFi network
 *
 * @param[in] self  WiFi network handle
 * @return security flags of a WiFi network
 **/
SvString
QBWiFiNetworkGetFlags(QBWiFiNetwork self);

/**
 * Set signal level in dB to a WiFi network
 *
 * @param[in] self  WiFi network handle
 * @param[in] level signal level in dB
 **/
void
QBWiFiNetworkSetSignalLevel(QBWiFiNetwork self, int level);

/**
 * Get signal level in dB of a WiFi network
 *
 * @param[in] self  WiFi network handle
 * @return signal level in dB
 **/
int
QBWiFiNetworkGetSignalLevel(QBWiFiNetwork self);

/**
 * Get signal level in human readable scale.
 *
 * @param[in] self  WiFi network handle
 * @return          signal level
 **/
QBWiFiNetworkSignalLevel
QBWiFiNetworkGetSignalLevelInHumanScale(QBWiFiNetwork self);

/**
 * Set security mode (none, WEP, WPA/WPA2) to a WiFi network
 *
 * @param[in] self         WiFi network handle
 * @param[in] securityMode security mode (none, WEP, WPA/WPA2)
 **/
void
QBWiFiNetworkSetSecurityMode(QBWiFiNetwork self, QBNetworkManagerWiFiNetworkSecurity securityMode);

/**
 * Get security mode of a WiFi network
 *
 * @param[in] self  WiFi network handle
 * @return security mode (none, WEP, WPA/WPA2) of a WiFi network
 **/
QBNetworkManagerWiFiNetworkSecurity
QBWiFiNetworkGetSecurityMode(QBWiFiNetwork self);

/**
 * Get network origin interface.
 *
 * @param[in] self  WiFi network handle
 * @return          weak reference to QBWiFiInterface
 **/
SvWeakReference
QBWiFiNetworkGetOriginInterface(QBWiFiNetwork self);

/**
 * @}
**/

#endif //QB_WIFI_NETWORK_H_
