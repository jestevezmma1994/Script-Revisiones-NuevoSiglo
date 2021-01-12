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

#ifndef UTILS_H_
#define UTILS_H_

#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBOSKPane.h>
#include <QBApplicationController.h>
#include <QBNetworkMonitor.h>
#include <QBNetworkMonitor/QBWiFiNetwork.h>
#include <QBNetworkMonitor/QBNetworkInterface.h>
#include <SvFoundation/SvString.h>
#include <CUIT/Core/types.h>
#include <fibers/c/fibers.h>

/**
 * Show dialog with message about invalid length of WiFi password.
 *
 * @param[in] app           application handle
 * @param[in] controller    application controller handle
 * @param[in] settingsCtx   settings context
 **/
void
QBWifiPasswordTypedErrorDialogShow(SvApplication app, QBApplicationController controller, int settingsCtx);

/**
 * Utility function that connects to a WiFi network or shows a dialog when it is not possible to
 * associate with a SSID with given key.
 *
 * @param[in] res               application handle
 * @param[in] controller        application controller handle
 * @param[in] networkMonitor    network monitor handle
 * @param[in] wifiInterface     an interface on which the SSID should be connected
 * @param[in] wifiNetwork       a WiFi network to be associated
 * @param[in] password          a key to be used to associate to the SSID
 * @param[in] manual            @c true if the SSID is hidden (not broadcasted), @c false otherwise
 * @param[in] settingsCtx       settings context id for the popup with an error
 **/
bool QBWiFiPasswordTyped(SvApplication res,
                         QBApplicationController controller,
                         QBNetworkMonitor networkMonitor,
                         QBWiFiInterface wifiInterface,
                         QBWiFiNetwork wifiNetwork,
                         SvString password,
                         bool manual,
                         int settingsCtx);

/**
 * Utility function that creates a side menu with an OSK that enables
 * typying WiFi SSID or key.
 *
 * @param[in] scheduler     application scheduler handle
 * @param[in] ctxMenu       context menu handle
 * @param[in] widgetName    widget name in settings
 * @param[in] map           oskmap filepath
 * @param[in] level         pane level
 * @param[in] callback      callback function for the OSK pane
 * @param[in] callbackData  callback data for the OSK pane
 **/
SvObject
QBWiFiInputSideMenuCreate(SvScheduler scheduler,
                          QBContextMenu ctxMenu,
                          SvString widgetName,
                          const char *map,
                          int level,
                          QBOSKPaneKeyTyped callback,
                          void *callbackData);

#endif /* UTILS_H_ */
