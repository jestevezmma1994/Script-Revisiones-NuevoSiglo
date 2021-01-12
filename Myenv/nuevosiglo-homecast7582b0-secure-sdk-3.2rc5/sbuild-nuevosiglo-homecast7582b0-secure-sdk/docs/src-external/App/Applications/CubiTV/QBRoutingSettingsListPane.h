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

#ifndef QBROUTINGSETTINGSLISTPANE_H
#define QBROUTINGSETTINGSLISTPANE_H

/**
 * @file  QBRoutingSettingsListPane.h
 * @brief Routing options select pane API
 **/

#include <Services/core/QBTextRenderer.h>
#include <QBNetworkMonitor.h>
#include <QBDataModel3/QBActiveArray.h>
#include <ContextMenus/QBContextMenu.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup QBRoutingSettingsListPane Routing options select pane class
 * @ingroup  CubiTV
 * @{
 **/

/**
 * Routing sort item constructor is specialized class for constructing
 * list pane items. It provides XMB interface for creating elements.
 *
 * @class QBRoutingSortItemConstructor
 **/
typedef struct QBRoutingSortItemConstructor_ *QBRoutingSortItemConstructor;

/**
 * Routing Settings List Pane manages setting the preffered gateway
 * by user.
 *
 * @class QBRoutingSettingsListPane
 **/
typedef struct QBRoutingSettingsListPane_ *QBRoutingSettingsListPane;

/**
 * Creates routing settings editable list pane. @a dataSource will be
 * sorted according to QBNetworkInterfaceGetGatewayTenure before display.
 *
 * @param[in] settings          File containing graphical widget data
 *                              to be displayed in this menu
 * @param[in] res               CUIT application handle
 * @param[in] textRenderer      text renderer handle
 * @param[in] networkMonitor    network monitor handle
 * @param[in] dataSource        list of available interfaces
 * @param[in] ctxMenu           context menu
 * @param[out] errorOut         handler to service error level higher
 * @return created routing settings pane
  **/
QBRoutingSettingsListPane
QBRoutingSettingsListPaneCreateFromSettings(const char *settings,
                                            SvApplication res,
                                            QBTextRenderer textRenderer,
                                            QBNetworkMonitor networkMonitor,
                                            QBActiveArray dataSource,
                                            QBContextMenu ctxMenu,
                                            SvErrorInfo *errorOut);
/**
 * @}
**/

#endif //QBROUTINGSETTINGSLISTPANE_H
