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

#ifndef QB_NPVR_PROVIDER_H
#define QB_NPVR_PROVIDER_H

/**
 * @file QBnPVRProvider.h QBnPVRProvider interface file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <NPvr/QBnPVRTypes.h>
#include <NPvr/QBnPVRProviderRequest.h>
#include <NPvr/QBnPVRQuota.h>
#include <NPvr/QBnPVRRecording.h>
#include <NPvr/QBnPVRDirectory.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvPlayerKit/SvContent.h>
#include <SvEPGDataLayer/SvEPGManager.h>

/**
 * @defgroup QBnPVRProvider The QBnPVRProvider interface.
 * @ingroup NPvr
 * @{
 **/

/**
 * QBnPVRProvider interface
 **/
typedef const struct QBnPVRProvider_s
{
    /**
     * Initialize QBNPvr provider.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] params        QBNPvrProvider refreshing params
     * @param[in] epgManager    SvEPGManager handle
     */
    void (*init)(SvObject self_, QBnPVRProviderInitParams params, SvEPGManager epgManager);

    /**
     * Start QBNPvr provider.
     *
     * @param[in] self_         QBNPvrProvider handle
     */
    void (*start)(SvObject self_);

    /**
     * Stop QBNPvr provider.
     *
     * @param[in] self_         QBNPvrProvider handle
     */
    void (*stop)(SvObject self_);

    // recs

    /**
     * List recordings basing on the given event.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] event         SvEPGEvent object to be found in the provider's recordings
     * @param[out] recordings   array with NPvr recordings on a given event
     */
    void (*listRecordingsByEvent)(SvObject self_, SvEPGEvent event, SvArray recordings);

    /**
     * Get QBNPvr recording by Id.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] directoryId   recording Id
     * @return                  QBnPVRRecording instance
     */
    QBnPVRRecording (*getRecordingById)(SvObject self_, SvString recordingId);

    /**
     * List recordings basing on the given directory Id.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] directoryId   directory Id as string
     * @param[out] recordings   array with NPvr recordings on a given event
     */
    void (*listRecordingsByDirectory)(SvObject self_, SvString directoryId, SvArray recordings);

    /**
     * Refresh data of the recordings of the given event.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] event         SvEPGEvent object to be found in the provider's recordings
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*refreshRecordingsByEvent)(SvObject self_, SvEPGEvent event);

    /**
     * Refresh data of the given recording.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] recording     QBnPVRRecording to be refreshed
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*refreshRecording)(SvObject self_, QBnPVRRecording recording);

    /**
     * Refresh data of the all provider's recordings.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*refreshAllRecordings)(SvObject self_);

    /**
     * Update recording basing on the recording and directory IDs.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] params        data identifying recording
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*updateRecording)(SvObject self_, QBnPVRRecordingUpdateParams params);

    /**
     * Schedule new recording.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] params        data needed to schedule new recording
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*scheduleRecording)(SvObject self_, QBnPVRRecordingSchedParams params);

    /**
     * Delete given recordings.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] recordings    array of recordings to delete
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*deleteRecordings)(SvObject self_, SvArray recordings);

    /**
     * Delete all recordings from the given directory that are in one of the given states.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] directoryId   directory Id
     * @param[in] states        states of the recordings to be deleted
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*deleteAllInStatesFromDirectory)(SvObject self_, SvString directoryId, SvArray states);

    /**
     * Lock/unlock given recording.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] recording     recording to be locked/unlocked
     * @param[in] lock          requested state of the lock
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*lockRecording)(SvObject self_, QBnPVRRecording recording, bool lock);

    /**
     * Add QBNPvr provider recordings listener.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] listener      listener object
     */
    void (*addRecordingListener)(SvObject self_, SvObject listener);

    /**
     * Remove QBNPvr provider recordings listener.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] listener      listener object
     */
    void (*removeRecordingListener)(SvObject self_, SvObject listener);

    // dirs

    /**
     * Create QBNPvr directory.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] params        data needed to create new directory
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*createDirectory)(SvObject self_, QBnPVRDirectoryCreateParams params);

    /**
     * Delete QBNPvr directory.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] params        data needed to delete directory
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*deleteDirectory)(SvObject self_, QBnPVRDirectoryDeleteParams params);

    /**
     * Refresh QBNPvr directory.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] directoryId   directory Id
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*refreshDirectory)(SvObject self_, SvString directoryId);

    /**
     * Update QBNPvr directory.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] params        data identifying recording
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*updateDirectory)(SvObject self_, QBnPVRDirectoryUpdateParams params);

    /**
     * Return list of the QBNPvr directories.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[out] directories  array of the directories
     */
    void (*listDirectories)(SvObject self_, SvArray directories);

    /**
     * Update provider's list of directories.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*getDirectories)(SvObject self_);

    /**
     * Get QBNPvr directory by Id.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] directoryId   directory Id
     * @return                  QBnPVRDirectory instance
     */
    QBnPVRDirectory (*getDirectoryById)(SvObject self_, SvString directoryId);

    /**
     * Get count of the QBNPvr directories.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @return                  directories count
     */
    int (*getDirectoriesCount)(SvObject self_);

    /**
     * Add QBNPvr directory listener.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] listener      listener object
     * @param[in] directoryId   Id of the directory
     */
    void (*addDirectoryInfoListener)(SvObject self_, SvObject listener, SvString directoryId);

    /**
     * Remove QBNPvr directory listener.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] listener      listener object
     * @param[in] directoryId   Id of the directory
     */
    void (*removeDirectoryInfoListener)(SvObject self_, SvObject listener, SvString directoryId);

    // series recs

    /**
     * Get QBNpvr directory by series Id.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] seriesId      series Id
     * @return                  QBnPVRDirectory instance
     */
    QBnPVRDirectory (*getSeriesById)(SvObject self_, SvString seriesId);

    /**
     * Schedule QBNPvr series recordings.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] params        schedule parameters
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*scheduleSeries)(SvObject self_, QBnPVRRecordingSchedParams params);

    /**
     * Delete all recordings from the given series.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] seriesId      series Id
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*deleteSeries)(SvObject self_, SvString seriesId);

    // keyword recs

    /**
     * Get QBNpvr directory by keyword Id.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] keywordId     keyword Id
     * @return                  QBnPVRDirectory instance
     */
    QBnPVRDirectory (*getKeywordById)(SvObject self_, SvString keywordId);

    /**
     * Schedule QBNPvr keyword recordings.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] params        schedule parameters
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*scheduleKeyword)(SvObject self_, QBnPVRRecordingSchedParams params);

    /**
     * Delete all recordings for the given keyword.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] keywordId     keyword Id
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*deleteKeyword)(SvObject self_, SvString keywordId);

    /**
     * Set quota limit for the given QBNPvr recording.
     *
     * @param[in] self_       QBNPvrProvider handle
     * @param[in] scheduleId  schedule identifier
     * @param[in] params      schedule update parameters
     * @return                created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*updateSchedule)(SvObject self_, SvString scheduleId, QBnPVRRecordingScheduleUpdateParams params);

    /**
     * Set quota limit for the given QBNPvr directory.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] directoryId   directory Id
     * @param[in] quota         max quota data
     * @param[in] removePolicy  recordings remove policy
     * @return                  created QBnPVRProviderRequest
     */
    QBnPVRProviderRequest (*setQuota)(SvObject self_, SvString directoryId, QBnPVRProviderQuotaUpdate quota, QBnPVRQuotaPolicy removePolicy);

    /**
     * Get capabilites of QBNPvr provider implementation.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @return                  QBnPVRProviderCapabilities - provider's capabilities
     */
    QBnPVRProviderCapabilities (*getCapabilities)(SvObject self_);

    /**
     * Tell if the given EPG event is recordable.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] event         SvEPGEvent handle
     * @return                  TRUE if the event is recordable
     */
    bool (*isEventRecordable)(SvObject self, SvEPGEvent event);

    /**
     * Tell if recording of the given event can be scheduled.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] event         SvEPGEvent handle
     * @return                  TRUE if the recording of the event can be scheduled
     */
    bool (*canScheduleEventRecording)(SvObject self, SvEPGEvent event);

    // utils

    /**
     * Get info string of the QBNPvr recording failure.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] recording     failed QBnPVRRecording
     * @return                  failure reason description as SvString
     */
    SvString (*createRecordingFailureReasonString)(SvObject self_, QBnPVRRecording recording);

    /**
     * Create SvContent using given QBNPvr recording.
     *
     * @param[in] self_         QBNPvrProvider handle
     * @param[in] recording     QBnPVRRecording than will be used to create SvContent
     * @return                  created SvContent
     */
    SvContent (*createContent)(SvObject self, QBnPVRRecording recording);

} *QBnPVRProvider;

/**
 * Get runtime type identification object representing
 * QBnPVRProvider interface.
 **/
extern SvInterface
QBnPVRProvider_getInterface(void);

/**
 * Tell if the recording belongs to the global NPvr directory.
 * @param[in] rec   recording to be checked
 * @return          true if the recording belongs to the global directory
 */
extern bool
QBnPVRProviderRecordingDirIsGlobal(QBnPVRRecording rec);

/**
 * Tell if the NPvr directory is global(main) directory.
 * @param[in] dir   directory to be checked
 * @return          true if the directory is global
 */
extern bool
QBnPVRProviderDirectoryIsGlobal(QBnPVRDirectory dir);

/**
 * Returns string representing global(main) NPvr directory
 * @return          global directory name as SvString
 */
extern SvString
QBnPVRProviderGetGlobalDirectoryId(void);

/**
 * Translate NPvr recording state to string.
 * @param state     recording state
 * @return          state as SvString
 */
extern SvString
QBnPVRProviderTranslateRecordingStateToString(QBnPVRRecordingState state);

/** @} */

#endif // #ifndef QB_NPVR_PROVIDER_H
