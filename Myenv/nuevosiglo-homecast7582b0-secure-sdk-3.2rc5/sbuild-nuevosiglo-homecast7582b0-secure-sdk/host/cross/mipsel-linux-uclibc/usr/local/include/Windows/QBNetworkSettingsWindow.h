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

#ifndef QB_NETWORK_SETTINGS_WINDOW_H_
#define QB_NETWORK_SETTINGS_WINDOW_H_

/**
 * @file QBNetworkSettingsWindow.h Network settings Window for CubiTV
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <Logic/InitLogic.h>
#include <QBWindowContext.h>
#include <QBApplicationController.h>
#include <QBNetworkMonitor.h>
#include <Services/core/QBTextRenderer.h>
#include <fibers/c/fibers.h>

/**
 * @defgroup QBNetworkSettingsContext QBNetworkSettingsContext for CubiTV
 * @ingroup CubiTV
 * @{
 **/

/**
 * Get runtime type identification object representing network settings context class.
 * @return network settings context class
 **/
SvType
QBNetworkSettingsContext_getType(void);

/**
 * Create new Network settings context
 * @param[in] res                   application handle
 * @param[in] scheduler             application scheduler handle
 * @param[in] initLogic             QBInitLogic handle
 * @param[in] controller            application controller handle
 * @param[in] textRenderer          text renderer handle
 * @param[in] networkMonitor        network monitor handle
 * @param[in] isStrict              flag discriminating if it is not allowed to proceed without any added network interface in up state
 * @return                          new object or NULL in case of error
 */
QBWindowContext
QBNetworkSettingsContextCreate(SvApplication res,
                               SvScheduler scheduler,
                               QBInitLogic initLogic,
                               QBApplicationController controller,
                               QBTextRenderer textRenderer,
                               QBNetworkMonitor networkMonitor,
                               bool isStrict);

/**
 * @}
**/

#endif /* QB_NETWORK_SETTINGS_WINDOW_H_ */
