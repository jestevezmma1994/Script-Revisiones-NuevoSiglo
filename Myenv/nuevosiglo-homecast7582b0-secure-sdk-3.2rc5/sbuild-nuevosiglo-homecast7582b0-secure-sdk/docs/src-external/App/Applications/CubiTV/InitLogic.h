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

#ifndef QBINITLOGIC_H_
#define QBINITLOGIC_H_

/**
 * @file InitLogic.h
 * @brief Initial logic of CubiTV.
 **/

#include <main_decl.h>
#include <QBAppKit/QBPropertiesMap.h>
#include <stdbool.h>
#include <QBInput/Filters/QBLongKeyPressFilter.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SearchAgents/EPGManSearchAgent.h>

typedef struct QBInitLogic_t* QBInitLogic;

/**
 * Get runtime type identification object representing QBInitLogic type.
 *
 * @return QBInitLogic runtime type identification object
**/
SvType QBInitLogic_getType(void);

QBInitLogic QBInitLogicCreate(AppGlobals appGlobals) __attribute__((weak));

const char** QBInitLogicGetTextDomains(QBInitLogic self) __attribute__((weak));

/**
 * Create PVR provider.
 *
 * @param[in] self         QB Init Logic
 * @param[in] whitelist    whitelist of services that are allowed
 **/
void QBInitLogicCreatePVRProvider(QBInitLogic self, SvHashTable whitelist) __attribute__((weak));

void QBInitLogicPostInit(QBInitLogic self, SvHashTable whitelist) __attribute__((weak));

/**
 * Create minimal setup for tansformations.
 *
 * @param[in] self         QB Init Logic
 * @param[in] whitelist    whitelist table
 **/
void QBInitLogicMinimalPostInit(QBInitLogic self, SvHashTable whitelist) __attribute__((weak));

void QBInitLogicPostStart(QBInitLogic self) __attribute__((weak));

void QBInitLogicPreStop(QBInitLogic self) __attribute__((weak));

void QBInitLogicPreDestroy(QBInitLogic self, bool terminate) __attribute__((weak));

void QBInitLogicCreateAccessPlugin(QBInitLogic self) __attribute__((weak));

void QBInitLogicCreatePlaylists(QBInitLogic self) __attribute__((weak));

/**
 * Create filename of secure logger configuration file
 *
 * @param[in] self         QB Init Logic
 * @return created filename of secure logger configuration file
 */
SvString QBInitLogicCreateSecureLogConfigFileName(QBInitLogic self) __attribute__((weak));

/**
 * Retrive filename of player configuration file
 *
 * @param[in] self         QB Init Logic
 * @return filename of player configuration file
 */
SvString QBInitLogicGetPlayerConfigFileName(QBInitLogic self) __attribute__((weak));

void QBInitLogicCreateEPGManager(QBInitLogic self) __attribute__((weak));

bool QBInitLogicShouldEnterWizard(QBInitLogic self) __attribute__((weak));

void QBInitLogicEnterWizard(QBInitLogic self) __attribute__((weak));

void QBInitLogicLeaveWizard(QBInitLogic self) __attribute__((weak));

bool QBInitLogicIsWizardFinished(QBInitLogic self) __attribute__((weak));

void QBInitLogicEnterApplication(QBInitLogic self) __attribute__((weak));

SvString QBInitLogicGetChannelScanningConfFileName(QBInitLogic self) __attribute__((weak));

void QBInitLogicSetLocale(QBInitLogic self, SvString optVal) __attribute__((weak));

SvString QBInitLogicGetServiceListFileName(QBInitLogic self) __attribute__((weak));

SvString QBInitLogicGetLogoWidgetName(QBInitLogic self) __attribute__((weak));

SvString QBInitLogicGetClosedCaptioningOptionsFileName(QBInitLogic self) __attribute__((weak));

SvString QBInitLogicGetUserScanningParamsFileName(QBInitLogic self) __attribute__((weak));

SvString QBInitLogicGetDefaultScanningParamsFileName(QBInitLogic self) __attribute__((weak));

SvString QBInitLogicGetDiagnosticsOptionsFileName(QBInitLogic self) __attribute__((weak));

SvString QBInitLogicGetStandbyAgentPluginsFileName(QBInitLogic self) __attribute__((weak));

/**
 * Get map for long pressed keys.
 *
 * Map long (longer than 1 us) presses of EPG key to a single key-typed event
 *
 * @param[in] self    handle to QBInitLogic object
 * @return            array of long key press extended mapping
 **/
const QBLongKeyPressExtendedMapping *QBInitLogicGetKeyExtendedMap(QBInitLogic self) __attribute__((weak));

/**
 * Create MW Client Service and add plugins related to this service.
 *
 * @param[in] appGlobals    AppGlobals handle
 **/
void QBInitLogicCreateMWClientServices(AppGlobals appGlobals) __attribute__((weak));

/**
 * Create properties map for given service.
 *
 * @param[in] self          handle to QBInitLogic object
 * @param[in] serviceName   service name
 *
 * @return                  handle to created QBPropertiesMap object, @c NULL in case of error
 **/
QBPropertiesMap QBInitLogicCreateServiceProperties(QBInitLogic self, SvString serviceName) __attribute__((weak));

void QBInitLogicCreateBookmarkPlugins(QBInitLogic self) __attribute__((weak));

void QBInitLogicCreateContentManagersServices(QBInitLogic self) __attribute__((weak));

SvString QBInitLogicCreateFavoritesConfigurationFileName(QBInitLogic self) __attribute__((weak));

const char* QBInitLogicGetMWTimeoutText(QBInitLogic self) __attribute__((weak));

bool QBInitLogicGetAppStartupWaiterTimeout(QBInitLogic self, int* timeout) __attribute__((weak));

void QBInitLogicLoadHlsSSLAuthParams(QBInitLogic self) __attribute__((weak));

SvObject QBInitLogicGetChannelList(QBInitLogic self) __attribute__((weak));
/**
* Setup allowed muxes
*
* @param[in] self      QBInitLogic handle
**/
void QBInitLogicSetupMuxes(QBInitLogic self) __attribute__((weak));

/**
 * Initial logic for middleware service monitor. It creates service monitor
 * and sets adapter.
 *
 * @param[in] self      handle to QBInitLogic object
 **/
void QBInitLogicServiceMonitorCreate(QBInitLogic self) __attribute__((weak));

/**
 * Create search agent.
 *
 * @param[in] self     QBInitlogic handle
 * @param[in] mode     search mode
 * @param[in] category category id
 * @param[in] view     view
 * @return             search agent
 **/
SvObject
QBInitLogicCreateSearchAgent(QBInitLogic self,
                             EPGManSearchAgentMode mode,
                             int category,
                             SvObject view) __attribute__((weak));

/**
 * @}
 **/

#endif
