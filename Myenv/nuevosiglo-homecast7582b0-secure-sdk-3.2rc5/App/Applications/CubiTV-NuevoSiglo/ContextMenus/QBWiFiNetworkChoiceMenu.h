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


#ifndef QBWIFICHOICEMENU_H
#define QBWIFICHOICEMENU_H

/**
 * @file QBWiFiNetworkChoiceMenu.h WiFi network choice context menu structures and API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBNetworkMonitor/QBWiFiNetwork.h>
#include <Services/core/QBTextRenderer.h>
#include <SvFoundation/SvObject.h>

/**
 * @defgroup QBNetworkMonitor Network Monitor
 * @ingroup CubiTV
 * @{
 **/

/** WiFi network state to be displayed on a WiFi widget */
typedef enum {
    WiFiNetworkChoice_disconnected, /**< disconnected and password not remembered */
    WiFiNetworkChoice_remembered, /**< disconnected and password remembered */
    WiFiNetworkChoice_connecting, /**< currently connecting */
    WiFiNetworkChoice_connected /**< connected (associated to WiFi network) */
} WiFiNetworkChoiceState;

/** WiFi network security type to be displayed on a WiFi widget */
typedef enum {
    WiFiNetworkChoice_open, /**< WiFi not secured */
    WiFiNetworkChoice_secure /**< WiFi secured with WEP/WPA/WPA2 */
} WiFiNetworkChoiceSecurity;

/**
 * WiFi network choice controller manages displaying of the
 * WiFi network choice context side menu
 *
 * @class QBWiFiNetworkChoiceController
 **/
typedef struct QBWiFiNetworkChoiceController_* QBWiFiNetworkChoiceController;

/**
 * Gets WiFi network state [unknown (disconnected and not remembered), remembered, connecting, connected]
 *
 * @param[in] self_ caller handle
 * @param[in] node_ context menu node handle
 * @return current WiFi network state
 */
typedef WiFiNetworkChoiceState (*QBWiFiNetworkChoiceControllerGetState)(void *self_, SvObject node_);

/**
 * Gets WiFi network security [open, secure]
 *
 * @param[in] self_ caller handle
 * @param[in] node_ context menu node handle
 * @return current WiFi network security
 */
typedef WiFiNetworkChoiceSecurity (*QBWiFiNetworkChoiceControllerGetSecurity)(void *self_, SvObject node_);

/**
 * Gets WiFi network signal level [very weak, weak, moderate, good, very good]
 *
 * @param[in] self_ caller handle
 * @param[in] node_ context menu node handle
 * @return current WiFi network signal level
 */
typedef QBWiFiNetworkSignalLevel (*QBWiFiNetworkChoiceControllerGetSignalLevel)(void *self_, SvObject node_);

/**
 * Creates WiFi network context menu node caption.
 *
 * @param[in] self_ caller handle
 * @param[in] node_ context menu node handle
 * @return WiFi network context menu node caption
 */
typedef SvString (*QBWiFiNetworkChoiceControllerCreateNodeCaption)(void *self_, SvObject node_);

/**
 * Creates WiFi network context menu node subcaption.
 *
 * @param[in] self_ caller handle
 * @param[in] node_ context menu node handle
 * @return WiFi network context menu node subcaption
 */
typedef SvString (*QBWiFiNetworkChoiceControllerCreateNodeSubcaption)(void *self_, SvObject node_);

/**
 * Set of WiFi network choice controller's callbacks
 *
 * @class QBWiFiNetworkChoiceControllerCallbacks
 **/
struct QBWiFiNetworkChoiceControllerCallbacks_ {
    QBWiFiNetworkChoiceControllerGetState getState; /**< pointer to a callback function that returns a WiFi network state */
    QBWiFiNetworkChoiceControllerGetSecurity getSecurity; /**< pointer to a callback function that returns a security type of a WiFi network */
    QBWiFiNetworkChoiceControllerGetSignalLevel getSignalLevel; /**< pointer to a callback function that returns a signal level of a WiFi network */
    QBWiFiNetworkChoiceControllerCreateNodeCaption createNodeCaption; /**< pointer to a callback function that creates a node caption for a WiFi network */
    QBWiFiNetworkChoiceControllerCreateNodeSubcaption createNodeSubcaption; /**< pointer to a callback function that creates a node subcaption for a WiFi network */
};
typedef struct QBWiFiNetworkChoiceControllerCallbacks_* QBWiFiNetworkChoiceControllerCallbacks;

/**
 * Creates WiFi network choice controller from given settings.
 *
 * @param[in] settings settings file path
 * @param[in] renderer handle to QBTextRenderer instance
 * @param[in] callbackData callback data to be used together with QBWiFiNetworkChoiceControllerCallbacks
 * @param[in] callbacks handle to QBWiFiNetworkChoiceControllerCallbacks instance
 * @return WiFi network context menu node subcaption
 */
QBWiFiNetworkChoiceController QBWiFiNetworkChoiceControllerCreateFromSettings(const char *settings, QBTextRenderer renderer, void *callbackData, QBWiFiNetworkChoiceControllerCallbacks callbacks);

/**
 * @}
**/

#endif //QBWIFICHOICEMENU_H
