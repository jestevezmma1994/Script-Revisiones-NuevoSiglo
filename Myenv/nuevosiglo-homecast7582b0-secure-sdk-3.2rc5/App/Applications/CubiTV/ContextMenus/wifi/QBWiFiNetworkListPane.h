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

#ifndef QB_WIFI_NETWORK_LIST_PANE_H_
#define QB_WIFI_NETWORK_LIST_PANE_H_

/**
 * @file QBWiFiNetworkListPane.h WiFi Network List Pane API for QBNetworking2
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <Services/core/QBTextRenderer.h>
#include <QBApplicationController.h>
#include <QBNetworkMonitor.h>
#include <QBNetworkMonitor/QBNetworkInterface.h>
#include <ContextMenus/QBContextMenu.h>
#include <CUIT/Core/types.h>
#include <fibers/c/fibers.h>

/**
 * @defgroup QBWiFiNetworkListPane QBWiFiNetworkListPane for QBNetworking2
 * @ingroup CubiTV
 * @{
 **/

typedef struct QBWiFiNetworkListPane_ *QBWiFiNetworkListPane;

/**
 * Creates WiFi network list pane
 *
 * @param[in] res               application handle
 * @param[in] scheduler         application scheduler handle
 * @param[in] textRenderer      text renderer handle
 * @param[in] controller        application controller handle
 * @param[in] networkMonitor    network monitor handle
 * @param[in] wifiInterface     WiFi interface for which the pane is created
 * @param[in] ctxMenu           context menu
 * @param[in] level             level on which the pane shall be shown
 * @return QBWiFiNetworkListPane new pane with a list of WiFi networks
  **/
QBWiFiNetworkListPane
QBWiFiNetworkListPaneCreate(SvApplication res,
                            SvScheduler scheduler,
                            QBTextRenderer textRenderer,
                            QBApplicationController controller,
                            QBNetworkMonitor networkMonitor,
                            QBWiFiInterface wifiInterface,
                            QBContextMenu ctxMenu,
                            int level);

/**
 * @}
**/

#endif /* QB_WIFI_NETWORK_LIST_PANE_H_ */
