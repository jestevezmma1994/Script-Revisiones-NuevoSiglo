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

#ifndef NPVRLOGIC_H
#define NPVRLOGIC_H

/**
 * @file NPvrLogic.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief Logic for NPvr.
 **/

#include <NPvr/QBnPVRRecording.h>
#include <NPvr/QBnPVRQuota.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvFoundation/SvString.h>
#include <main_decl.h>
#include <stdbool.h>

/**
 * Check if user is able to create ongoing recording.
 *
 * @return      true if ongoing recordings are allowed, otherwise false
 **/
bool QBNPvrLogicAreOngoingRecsAllowed(void) __attribute__((weak));

/**
 * Check if user is able to set parameters when creating recoridngs.
 *
 * This method compares current window context with list of contexts defined in this method and returns true if user is able to:
 * - set channel, keyword and additional feature options when are enabled, while
 * creating keyword recording (e.g. in EPG view)
 * - set directory name and additional feature options when are enabled, while
 * creating series recording (e.g. in EPG view)
 *
 *
 * @param[in] appGlobals        AppGlobals handle
 * @return                      true if keyword and series recordings are custimozable, otherwise false
 **/
bool QBNPvrLogicAreRecordingsCustomizable(AppGlobals appGlobals) __attribute__((weak));

/**
 * Check weather premiere feature is enabled.
 *
 * Feature description:
 * Add new option (Premiere) to menus 'Record series' and 'Record keyword' in EPG.
 * Customer can choose between 'recording all' episodes and 'only new' ones.
 *
 * @return true if feature is enabled, otherwise false
 **/
bool QBNPvrLogicIsPremiereFeatureEnabled(void) __attribute__((weak));

/**
 * Check weather set margins feature is enabled.
 *
 * Feature description:
 * Add new option (Start/Stop Margins) to menus 'Record', 'Record series' and 'Record keyword' in EPG.
 *
 * @return true if feature is enabled, otherwise false
 **/
bool QBNPvrLogicIsSetMarginsFeatureEnabled(void) __attribute__((weak));

/**
 * Check weather recovery feature is enabled.
 *
 * Feature description:
 * Add new directory (Deleted) to NPvr menu, where will be moved removed recordings from directory 'Completed Recordings'.
 * User can choose option 'Space recovery' when schedules recording. He has two possible variants: 'Delete as needed' and
 * 'Delete manually'. Option 'Delete as needed' results, that this recording will be moved to Deleted directory,
 * when there will be not enough quota. 'Delete manually' results that recording can be removed only by user.
 *
 * Note: When feature is enabled, there should be also added node (id="npvr-deleted") to settings/CubiTV-ip.appmenu.
 *
 * @return true if feature is enabled, otherwise false
 **/
bool QBNPvrLogicIsRecoveryFeatureEnabled(void) __attribute__((weak));

/**
 * Check whether NPVR is chosen as preffered Start Over.
 *
 * Feature description:
 * Checks if when NPVR is available it will be used for Start Over
 *
 * @return true if NPVR is used for Start Over
 **/
bool QBNPvrLogicShouldNPVRBeUsedForStartOver(void) __attribute__((weak));

/**
 * Create custom format which will be used to display used and total quota in NPvr menu.
 *
 * @param[in] usedQuota     value of used quota
 * @param[in] totalQuota    value of total quota
 * @param[in] quotaUnits    quota unit (seconds or bytes)
 * @return                  string with custom quota format, otherwise NULL
 **/
SvString QBNPvrLogicCreateCustomQuotaFormat(uint64_t usedQuota, uint64_t totalQuota, QBnPVRQuotaUnits quotaUnits) __attribute__((weak));

/**
 * Get type of NPVR recording for given EPG event.
 *
 * @param[in] nPVRProvider  nPVRProvider handle
 * @param[in] event         EPG event
 * @return                  type of NPVR recording
 **/
QBnPVRRecordingType QBNPvrLogicGetRecordingTypeForEvent(SvObject nPVRProvider, SvEPGEvent event) __attribute__((weak));

/**
 * Try to schedule NPVR recording for given EPG event.
 *
 * @param[in] appGlobals        AppGlobals handle
 * @param[in] event             EPG event
 * @param[out] isScheduled      information about that if NPVR recording has been created
 * @return                      type of created NPVR recording
 **/
QBnPVRRecordingType QBNPvrLogicTryScheduleRecordingForEvent(AppGlobals appGlobals, SvEPGEvent event, bool *isScheduled) __attribute__((weak));

/**
 * @}
 **/

#endif // NPVRLOGIC_H
