/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef NPVR_TYPES_H
#define NPVR_TYPES_H

/**
 * @file QBnPVRTypes.h QBnPVRTypes set.
 **/

#include <NPvr/QBnPVRRecording.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvPlayerKit/SvEPGEvent.h>

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/**
 * @defgroup QBnPVRTypes The NPvr common types.
 * @ingroup NPvr
 * @{
 **/

/**
 * NPvr quota unit type.
 */
typedef enum NPvrQuotaUnitType_e  NPvrQuotaUnitType;
enum NPvrQuotaUnitType_e
{
    NPvrQuotaUnit_MB,
    NPvrQuotaUnit_GB,
    NPvrQuotaUnit_minute,
    NPvrQuotaUnit_hour,
    NPvrQuotaUnit_event,
};

/**
 * NPvr stats configuration type.
 */
typedef struct {
    struct {
        NPvrQuotaUnitType  unitType; /// how to interpret quota units received from the server
        uint32_t  scaleNom;    /// units received from the server need to be multiplied by this value
        uint32_t  scaleDenom;  /// units received from the server need to be divided by this value
    } quota;
} NPvrStatsConfig;

/**
 * NPvr provider capabilities.
 */
typedef struct {
    bool series;
    bool keyword;
    bool directories;
    bool directoryQuota;
    bool locks;

    QBnPVRQuotaUnits directoryQuotaUnits;
} QBnPVRProviderCapabilities;

/**
 * NPvr provider refresh times settings.
 */
typedef struct {
    time_t refreshRecordingsAfterMs;
    time_t refreshRecordingsAfterChannelsChangeMs;
    time_t firstRecordingRefreshDelayMs;
    unsigned int channelsPluginID;
} QBnPVRProviderInitParams;

/**
 * NPvr quota statistics.
 */
typedef struct {
    int64_t  quotaTotal;  /// -1 iff unknown
    int64_t  quotaUsed;   /// -1 iff unknown
} NPvrStats;

/** @} */

#endif // #ifndef NPVR_TYPES_H
