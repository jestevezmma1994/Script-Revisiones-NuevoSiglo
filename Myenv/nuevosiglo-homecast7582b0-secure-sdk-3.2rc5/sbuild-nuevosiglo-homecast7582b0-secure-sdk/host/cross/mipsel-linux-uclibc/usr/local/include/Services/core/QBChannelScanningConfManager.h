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


#ifndef QBCHANNELSCANNINGCONFMANAGER_H_
#define QBCHANNELSCANNINGCONFMANAGER_H_

/**
 * @file QBChannelScanningConfManager.h <Services/core/QBChannelScanningConfManager.h>
 *
 * Channel scanning configuration manager
 **/

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBDVBScanner.h>
#include <SvEPGDataLayer/Plugins/DVBEPGPlugin.h>
#include <Services/core/QBStandardRastersManager.h>
#include <Services/core/JSONserializer.h>
#include <tunerReserver.h>

/**
 * @defgroup QBChannelScanningConfigurationManager    Channel Scanning Configuration Manager
 * @ingroup CubiTV_configurations
 *
 * Loads channel scanning configuration (algorithms) and stores last user preferences
 * per scanning configuration. The configuration is used to fill QBDVBScannerParams.
 * Only parameters uninitialized in QBDVBScannerParams and provided in channel scanning
 * conf will be used.
 *
 * The configuration also allows to specify which QBDVBScannerPlugin are to be used.
 *
 * The configuration also allows to specify how to tread already found channels, when the
 * newly found will be saved.
 *
 * The configuration also allows to specify which options should be presented to the user
 * during configuring channel scanning.
 */

/**
 * Class for QBChannelScanningConfManager service.
 *
 * @class QBChannelScanningConfManager QBChannelScanningConfManager.h <Configurations/QBChannelScanningConfManager.h>
 * @extends SvObject
 * @implements QBAsyncService
 * @implements QBInitializable
 *
 * @note This class is intended to be used in cooperation with QBServiceRegistry,
 *       use "QBChannelScanningConfManager" key to get it from service registry.
 *
 * @see QBServiceRegistryGetService
 **/
typedef struct QBChannelScanningConfManager_t *QBChannelScanningConfManager;

SvType QBChannelScanningConfManager_getType(void);
void QBChannelScanningConfManagerLoadUserParams(QBChannelScanningConfManager self, SvString defaultParamsFilename, SvString userParamsFilename);
void QBChannelScanningConfManagerLoadConfiguration(QBChannelScanningConfManager self, SvString confFilename);

/**
 * Save configuration to JSON file.
 *
 * @param[in]   self            channel scanning configuration manager
 * @param[in]   confFilename    file name
 **/
void QBChannelScanningConfManagerSaveConfiguration(QBChannelScanningConfManager self, SvString confFilename);

QBChannelScanningConf QBChannelScanningConfManagerGetConf(QBChannelScanningConfManager self, SvString id);

/**
 * Update (or add if it's new) configuration in channel scanning configuration manager.
 *
 * @param[in]   self            channel scanning configuration manager
 * @param[in]   conf            channel scanning configuration to be updated or added
 **/
void QBChannelScanningConfManagerUpdateConf(QBChannelScanningConfManager self, QBChannelScanningConf conf);

void QBChannelScanningConfManagerSetDefaultConf(QBChannelScanningConfManager self, SvString id);
SvString QBChannelScanningConfManagerGetDefaultConf(QBChannelScanningConfManager self);
void QBChannelScanningConfFillUpdateParams(QBChannelScanningConf self, DVBEPGPluginUpdateParams* params);
void QBChannelScanningConfManagerSetUserParams(QBChannelScanningConfManager self, QBChannelScanningConf conf, QBChannelScanningConfUserParams params);
QBChannelScanningConfUserParams QBChannelScanningConfManagerGetUserParams(QBChannelScanningConfManager self, QBChannelScanningConf conf);

/**
 * @}
 **/

#endif // QBCHANNELSCANNINGCONFMANAGER_H_
