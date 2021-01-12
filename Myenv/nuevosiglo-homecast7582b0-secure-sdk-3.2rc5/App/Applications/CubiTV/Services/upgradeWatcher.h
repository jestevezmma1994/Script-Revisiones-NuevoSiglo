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

#ifndef UPGRADE_WATCHER_H_
#define UPGRADE_WATCHER_H_

/**
 * @file upgradeWatcher.h Upgrade Watcher service API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBDVBScanner.h>
#include <main_decl.h>
#include <QBUpgrade/QBDVBUpgradeSelector.h>

/**
 * @defgroup QBUpgradeWatcher Upgrade Watcher service
 * @ingroup CubiTV_services
 * @{
 *
 * Service that listens to upgrade notifications, filters them and passes
 * the information to Upgrade Logic, which in turn may request starting of upgrade.
 **/

/**
 * Creates a new Upgrade Watcher service instance. It will create upgrade
 * monitors, listen to them and manage their notifications. If they report
 * a too old version, it will be ignored, if the report a newer version
 * then Upgrade Logic will notified, unless it is already handled.
 *
 * Upon request is responsible for downloading upgrades and notifying
 * about its progress.
 *
 * When Upgrade Logic decides to use an upgrade, then Upgrade Watcher will
 * notify when the upgrade is available and can be started.
 *
 * @param[in] appGlobals        CubiTV application
 * @return                      new instance of QBUpgradeWatcher service
 **/
extern SvGenericObject UpgradeWatcherCreate(AppGlobals appGlobals);

/**
 * Allows to obtain DVB selector for this platform and software.
 *
 * @param[in] self              Upgrade Watcher service instance
 * @return                      DVB Upgrade selector
 **/
extern QBDVBUpgradeSelector* UpgradeWatcherGetSelector(SvGenericObject self);

/**
 * Requests start of upgrade download.
 *
 * @param[in] self              Upgrade Watcher service instance
 * @param[in] version           unused
 **/
extern void UpgradeWatcherStartDownload(SvGenericObject self, SvString version);

/**
 * Starts the Upgrade Watcher service, it will now monitor for upgrades.
 *
 * @param[in] self              Upgrade Watcher service instance
 **/
extern void UpgradeWatcherStart(SvGenericObject self);

/**
 * Stops the Upgrade Watcher service, it will no longer monitor for upgrades.
 *
 * @param[in] self              Upgrade Watcher service instance
 **/
extern void UpgradeWatcherStop(SvGenericObject self);

/**
 * Requests upgrade check via HTTP.
 *
 * @param[in] self_              Upgrade Watcher service instance
 **/
extern void UpgradeWatcherForceHTTPUpgradeCheck(SvObject self_);

/**
 * Drop old upgrade URI and wait for a new one
 *
 * @param[in] self_              Upgrade Watcher service instance
 **/
extern void UpgradeWatcherRefreshNITUpgrade(SvGenericObject self_);

/**
 * Don't check version number, any new version will upgrade current one
 *
 * @param[in] self_              Upgrade Watcher service instance
 * @param[in] enabled            @c true if version should be ignored, @c false only higher version are supported
 **/
extern void UpgradeWatcherIgnoreVersion(SvGenericObject self_, bool enabled);

/**
 * Treat all received upgraded as forced (i.e. user doesn't have ability to postpone it)
 *
 * @param[in] self_              Upgrade Watcher service instance
 * @param[in] forced             @c true if all upgrades will be tread as 'forced', @c false use information from upgrade stream
 **/
extern void UpgradeWatcherSetAllForced(SvGenericObject self_, bool forced);

/**
 * Perform an upgrade just after received upgrade information from NIT
 *
 * @param[in] self_              Upgrade Watcher service instance
 * @param[in] immediately        @c true if upgrade should be done immediately, @c false if random time from 1 to 15 minutes should be used
 **/
extern void UpgradeWatcherUpgradeImmediately(SvGenericObject self_, bool immediately);


/**
 * @}
 **/
#endif
