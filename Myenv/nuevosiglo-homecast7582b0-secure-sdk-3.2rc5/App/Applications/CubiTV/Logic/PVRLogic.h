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

#ifndef PVR_LOGIC_H
#define PVR_LOGIC_H

/**
 * @file PVRLogic.h
 * @brief PVR logic API
 **/

#include <Services/diskPVRProvider/QBDiskPVRProvider.h>
#include <main_decl.h>
#include <SvFoundation/SvString.h>
#include <stdbool.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <QBUSBPerfTester.h>
#include <Services/core/hotplugMounts.h>

typedef struct QBPVRLogic_t *QBPVRLogic;

typedef enum PvrType_ {
    PvrType_PVR,
    PvrType_NPVR
} PvrType;

/**
 * @brief List of supported features
 */
struct QBPVRLogicSupportedFeatures_ {
    /**
     * @brief indicates if timeshift recording should be enabled
     */
    bool timeshift;
    /**
     * @brief indicates if pvr recording should be enabled
     */
    bool pvr;
};

QBPVRLogic QBPVRLogicNew(AppGlobals appGlobals) __attribute__((weak));
void QBPVRLogicStart(QBPVRLogic self) __attribute__((weak));
void QBPVRLogicStop(QBPVRLogic self) __attribute__((weak));

/**
 * Get operator specific limit (maximum allowed cost)
 *
 * @param[in] self QBPVRLogic instance
 * @return Record limit or @c -1 if not present
 */
int QBPVRGetOperatorRecordLimit(QBPVRLogic self) __attribute__((weak));

/**
* Set maximum allowed cost of all simultaneous recordings
*
* @param[in] self QBPVRLogic instance
* @param[in] reclimit new value of record limit
*/
void QBPVRLogicSetRecLimit(QBPVRLogic self, int reclimit) __attribute__((weak));

/**
 * Set maximum allowed recording sessions
 *
 * @param[in] self QBPVRLogic instance
 * @param[in] maxRecordingSessions
 */
void QBPVRLogicSetMaxRecordingSessionsLimit(QBPVRLogic self, int maxRecordingSessions) __attribute__((weak));

/**
* Checks if USB PVR speed test is enabled
*
* @param[in] self QBPVRLogic instance
* @return true if is enabled, false otherwise
*/
bool QBPVRLogicIsSpeedTestEnabled(QBPVRLogic self) __attribute__((weak));

/**
* Get channel recording cost.
*
* @param[in] self                       QBPVRLogic instance
* @param[in] channelPVRCostClass        class of a channel for PVR cost
* @return channel recording cost
*/
int QBPVRLogicGetChannelCost(QBPVRLogic self, QBDiskPVRProviderChannelCostClass channelPVRCostClass) __attribute__((weak));

/**
* Calculate cost from USB performance test
*
* @param[in] self QBPVRLogic instance
* @param[in] test USB performance test
* @return calculated cost
*/
int QBPVRLogicGetRecLimitFromUSBPerfTest(QBPVRLogic self, QBUSBPerfTest test);

int QBPVRLogicCreatePVRFormatConf(QBPVRLogic self, SvString confFileName, SvString diskID) __attribute__((weak));

bool QBPVRLogicHasInternalStorage(void) __attribute__((weak));

/**
 * Check whether disk size is enough for PVR
 *
 * @param[in] self       QBPVRLogic instance
 * @param[in] diskInfo   QBDiskInfo structure
 * @return true if disk can be used for PVR, otherwise false
 */
bool QBPVRLogicIsDiskSizeEnoughForPVR(QBPVRLogic self, QBDiskInfo diskInfo) __attribute__((weak));

/**
 * Returns average bitrate of SD video in bps
 *
 * @param[in] self QBPVRLogic instance
 * @return average bitrate value
 */
int QBPVRLogicGetSDVideoAverageBitrate(QBPVRLogic self) __attribute__((weak));

/**
 * Returns average bitrate of HD video in bps
 *
 * @param[in] self QBPVRLogic instance
 * @return average bitrate value
 */
int QBPVRLogicGetHDVideoAverageBitrate(QBPVRLogic self) __attribute__((weak));

/**
  * Function checks disk compatibility with PVR and timeshift
  *
  * @param[in] diskInfo     description of disk being checked
  * @return                 @c true if given disk is TS and PVR compatible, @c false otherwise
  **/
bool QBPVRLogicIsDiskPVRCompatible(QBDiskInfo diskInfo) __attribute__((weak));

/**
 * Get client specific supported features
 *
 * @param[in] self QBPVRLogic instance
 * @param[out] supportedFeatures supported features list
 */
void QBPVRLogicGetSupportedFeatures(QBPVRLogic self, struct QBPVRLogicSupportedFeatures_ *supportedFeatures) __attribute__((weak));

#endif
