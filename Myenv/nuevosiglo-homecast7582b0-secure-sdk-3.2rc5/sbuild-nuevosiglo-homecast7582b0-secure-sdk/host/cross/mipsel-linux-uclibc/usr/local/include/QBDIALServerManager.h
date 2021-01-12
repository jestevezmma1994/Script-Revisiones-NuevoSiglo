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
#ifndef QB_DIAL_SERVER_MANAGER_H_
#define QB_DIAL_SERVER_MANAGER_H_

/**
 * @file QBDIALServerManager.h
 * @brief QBDIALServerManager class enables launching and managing DIAL server applications.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBDIALServerManager class.
 * @{
 **/

#include <QBAppsManager.h>
#include <QBNetworkMonitor.h>
#include <Services/core/GlobalPlayer.h>
#include <QBViewport.h>
#include <QBApplicationControllerTypes.h>
#include <CUIT/Core/types.h>
#include <SvCore/SvErrorInfo.h>

typedef struct QBDIALServerManager_ *QBDIALServerManager;

/**
 * QBDIALServerManager initialization parameters
 */
struct QBDIALServerManagerParams_ {
    QBAppsManager appsManager; ///< rpc applications manager handle
    QBNetworkMonitor networkMonitor; ///< network monitor handle
    SvApplication app; ///< application handle
    QBApplicationController controller; ///< application controller handle
    QBViewport viewport; ///< viewport handle
    QBGlobalPlayerController playerController; ///< player controller handle
    SvScheduler scheduler; ///< scheduler handle
};
typedef struct QBDIALServerManagerParams_* QBDIALServerManagerParams;

/**
 * Create new DIAL server manager instance.
 * DIAL server manager service allows launching and managing DIAL server applications.
 * DIAL stands for Discovery And Launch - a protocol that second-screen devices (eg. mobile)
 * can use to discover and launch apps on first-screen devices (eg. stb, tv).
 *
 * @param[in] params            dial server parameters
 * @param[out] errorOut         error info
 * @return                      @a DIAL server manager handle or @c NULL in case of error
 **/
QBDIALServerManager
QBDIALServerManagerCreate(QBDIALServerManagerParams params, SvErrorInfo *errorOut);

/**
 * Start dial server manager service.
 *
 * @param[in] self          dial server manager handle
 * @param[out] errorOut     error info
 **/
void
QBDIALServerManagerStart(QBDIALServerManager self, SvErrorInfo *errorOut);

/**
 * Stop dial server manager service.
 *
 * @param[in] self          dial server manager handle
 * @param[out] errorOut     error info
 **/
void
QBDIALServerManagerStop(QBDIALServerManager self, SvErrorInfo *errorOut);

/**
 * @}
 **/
#endif /* QB_DIAL_SERVER_MANAGER_H_ */
