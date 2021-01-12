/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBUPGRADELOGIC_H_
#define QBUPGRADELOGIC_H_

/**
 * @file UpgradeLogic.h Upgrade logic API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <QBUpgrade/QBDVBUpgradeSelector.h>
#include <QBUpgrade/QBUpgradeService.h>
#include <main_decl.h>


/**
 * @defgroup UpgradeLogic Upgrade logic
 * @ingroup CubiTV_logic
 * @{
 **/

/**
 * Upgrade Logic manages upgrades, it decides which, when and how upgrades
 * should be done. It is notified about new upgrades or upgrade download
 * progress by CubiTV application.
 *
 * Usually it tries to upgrade boxes in such way that not all of them will
 * upgrade at the same time. It is done to reduce peaks on servers with which
 * CubiTV connects. The simplest way of achieving that is by processing the upgrade
 * a random amount of time after detecting it.
 *
 * @class QBUpgradeLogic
 **/
typedef struct QBUpgradeLogic_t* QBUpgradeLogic;

/**
 * Creates a new instance of Upgrade Logic.
 *
 * @memberof QBUpgradeLogic
 * @param[in] appGlobals    CubiTV application
 * @return new instance of Upgrade Logic
 **/
extern QBUpgradeLogic
QBUpgradeLogicCreate(AppGlobals appGlobals) __attribute__((weak));

/**
 * Starts the instance, it will now do its job.
 *
 * @memberof QBUpgradeLogic
 * @param[in] self          upgrade logic handle
 **/
extern void
QBUpgradeLogicStart(QBUpgradeLogic self) __attribute__((weak));

/**
 * Stops the instance, it will now stop doing its job as soon as possible
 *
 * @memberof QBUpgradeLogic
 * @param[in] self          upgrade logic handle
 **/
extern void
QBUpgradeLogicStop(QBUpgradeLogic self) __attribute__((weak));

/**
 * Requests that Upgrade Logic will obtain all configuration variables again
 * and reconfigure itself based on them. Called when the application changes
 * the configuration variables in order to apply changes.
 *
 * @memberof QBUpgradeLogic
 * @param[in] self          upgrade logic handle
 **/
extern void
QBUpgradeLogicReconfigure(QBUpgradeLogic self) __attribute__((weak));

/**
 * Notifies that standby has been left. If an upgrade is available
 * and user confirmation is needed, then it can't be given during standby.
 * This method will be called to notify that standby has been left.
 *
 * @memberof QBUpgradeLogic
 * @param[in] self      upgrade logic handle
 **/
extern void
QBUpgradeLogicStandbyLeft(QBUpgradeLogic self) __attribute__((weak));

/**
 * Called when user wants to immediately upgrade if the upgrade is available.
 *
 * @memberof QBUpgradeLogic
 * @param[in] self      upgrade logic handle
 **/
extern void
QBUpgradeLogicCheck(QBUpgradeLogic self) __attribute__((weak));

/**
 * Notify that firmware image is available for download.
 *
 * @memberof QBUpgradeLogic
 *
 * @param[in] self      upgrade logic handle
 * @param[in] monitor   upgrade monitor handle, can be @c NULL
 * @param[in] version   new firmware version
 * @param[in] networkName name of the DVB network this firmware comes from
 * @param[in] upgradeURI where to get this firmware from
 * @param[in] forced    @c true if upgrade is marked as forced
 * @param[in] upgradeImmediately perform upgrade immediately
 **/
extern void
QBUpgradeLogicDownloadAvailable(QBUpgradeLogic self,
                                SvObject monitor,
                                SvString version,
                                SvString networkName,
                                SvURI upgradeURI,
                                bool forced,
                                bool upgradeImmediately) __attribute__((weak));

/**
 * Notify that firmware download progress changed.
 *
 * @memberof QBUpgradeLogic
 *
 * @param[in] self      upgrade logic handle
 * @param[in] progress  download progress from @c 0 to @c 100
 **/
extern void
QBUpgradeLogicDownloadProgressChanged(QBUpgradeLogic self,
                                      unsigned int progress) __attribute__((weak));

/**
 * Notify that firmware image is available for install (it means is already downloaded).
 *
 * @memberof QBUpgradeLogic
 *
 * @param[in] self      upgrade logic handle
 * @param[in] monitor   upgrade monitor handle, can be @c NULL
 * @param[in] version   new firmware version
 * @param[in] networkName name of the DVB network this firmware comes from
 * @param[in] upgradeURI URI to firmware image
 * @param[in] forced    @c true if upgrade is marked as forced
 * @param[in] upgradeImmediately perform upgrade immediately
 **/
extern void
QBUpgradeLogicUpgradeAvailable(QBUpgradeLogic self,
                               SvObject monitor,
                               SvString version,
                               SvString networkName,
                               SvURI upgradeURI,
                               bool forced,
                               bool upgradeImmediately) __attribute__((weak));

/**
 * Notifies user about upgrade (shows install popup dialog)
 *
 * @memberof QBUpgradeLogic
 *
 * @param[in] self       upgrade logic handle
 */
void QBUpgradeLogicUpgradeNotify(QBUpgradeLogic self);

/**
 * Create ssl parameters for upgrade server.
 *
 * @param[in]  self       upgrade logic handle
 * @return                ssl parameters for upgrade server
 **/
extern SvSSLParams
QBUpgradeLogicCreateSSLParams(QBUpgradeLogic self) __attribute__((weak));

/**
 * @}
 **/


#endif
