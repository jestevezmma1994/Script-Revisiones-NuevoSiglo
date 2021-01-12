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

#ifndef QB_PVR_PROVIDER_H_
#define QB_PVR_PROVIDER_H_

/**
 * @file QBPVRProvider.h Virtual PVR provider class file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBPVRDirectory.h"
#include "QBPVRProviderRequest.h"
#include "QBPVRRecording.h"
#include "QBPVRTypes.h"

#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvPlayerKit/SvContent.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentTree.h>
#include <QBAppKit/QBAsyncService.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvTime.h>

/**
 * @defgroup QBPVRProvider QBPVRProvider: virtual PVR provider
 * @{
 **/

/**
 * My recordings category ID
 **/
#define MY_RECORDINGS_CATEGORY_ID "myRecordings"

/**
 * Schedule category ID
 **/
#define SCHEDULE_CATEGORY_ID "schedule"

/**
 * Schedule category ID
 **/
#define SCHEDULED_CATEGORY_ID "scheduled"

/**
 * Schedule category ID
 **/
#define ONGOING_CATEGORY_ID "ongoing"

/**
 * Schedule category ID
 **/
#define COMPLETED_CATEGORY_ID "completed"

/**
 * Schedule category ID
 **/
#define FAILED_CATEGORY_ID "failed"

/**
 * PVR provider layout.
 */
typedef enum {
    QBPVRProviderLayout_CubiTV,         ///< CubiTV PVR provider layout
    QBPVRProviderLayout_CubiConnect,    ///< Cubi Connect provider layout
} QBPVRProviderLayout;

/**
 * PVR provider parameters.
 */
typedef struct QBPVRProviderParams_ {
    QBPVRProviderLayout layout;         ///< provider layout
    bool addSearchToStaticCategories;   ///< should search be added to static categories
    bool addSearchToDirectories;        ///< should search be added to directories

    QBPVRSearchProviderTest searchProviderTest;   ///< search provider test associated params
} QBPVRProviderParams;

/**
 * PVR provider conflict reason.
 */
typedef enum {
    QBPVRProviderConflictReason_unknown,                        ///< unknown conflict reason
    QBPVRProviderConflictReason_driveTooSlow,                   ///< connected drive is too slow to perform added recording, even if you remove other
                                                                ///< scheduled recordings
    QBPVRProviderConflictReason_tooManyRecordings,              ///< too many simultaneous recordings
    QBPVRProviderConflictReason_tooManySimultaneousRecordings,  ///< too many simultaneous recordings
    QBPVRProviderConflictReason_lackOfSpace,                    ///< lack of disk space
    QBPVRProviderConflictReason_conflicts,                      ///< conflicts
} QBPVRProviderConflictReason;

/**
 * PVR provider capabilities.
 */
typedef struct QBPVRProviderCapabilities_ {
    QBPVRProviderType type;             ///< provider type

    bool ongoingRecs;                   ///< are ongoing recordings supported
    bool recordingsCustomizable;        ///< are recording customizable
    bool premiereFeature;               ///< is premiere feature enabled
    bool setMarginsFeature;             ///< is set margins feature enabled
    bool recoveryFeature;               ///< is recovery feature enabled
    bool canStopOngoing;                ///< is it possible to stop ongoing recordings
    bool manual;                        ///< are manual recordings supported
    bool event;                         ///< are event recordings supported
    bool series;                        ///< are series recordings supported
    bool keyword;                       ///< are keyword recordings supported
    bool repeated;                      ///< are repeated recordings supported
    bool directories;                   ///< are custom directories supported
    bool directoryQuota;                ///< is directory quota supported
    bool locks;                         ///< are locks supported
    bool viewed;                        ///< is viewed flag supported
    bool recoveryModeOfRecording;       ///< is recovery mode of recording supported

    QBPVRQuotaUnits quotaUnits;        ///< quota unit type
} QBPVRProviderCapabilities;

/**
 * PVR provider get recordings methods parameters.
 */
typedef struct QBPVRProviderGetRecordingsParams_ {
    bool includeRecordingsInDirectories;        ///< include recordings in directories in results

    /// array indexed by states for which recordings should be included in results
    bool inStates[QBPVRRecordingState_cnt];

    /// if set to true, all recordings that can be played will be returned
    bool playable;
} QBPVRProviderGetRecordingsParams;

/**
 * PVR provider delete all in state method parameters.
 */
typedef struct QBPVRProviderDeleteAllInStateParams_ {
    /// array indexed by states for which recordings should be deleted
    bool inStates[QBPVRRecordingState_cnt];
} QBPVRProviderDeleteAllInStateParams;

/**
 * QBPVR provider.
 * @class QBPVRProvider
 * @implements QBAsyncService
 * @extends QBContentProvider
 **/
typedef struct QBPVRProvider_ {
    struct QBContentProvider_ super_;    ///< super type

    QBAsyncServiceState state;           ///< current state
    SvScheduler scheduler;               ///< scheduler that shall be used for handling asynchronous tasks
} *QBPVRProvider;

/**
 * Get runtime type identification object representing QBPVRProvider class.
 *
 * @memberof QBPVRProvider
 *
 * @return QBPVRProvider runtime type identification object
 **/
extern SvType QBPVRProvider_getType(void);

/**
 * Get content tree.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @return                  content tree of PVR provider
 */
QBContentTree
QBPVRProviderGetTree(QBPVRProvider self);

/**
 * Get recording by Id.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] recordingId   recording id
 * @return                  recording with given id, @c NULL if not found
 */
QBPVRRecording
QBPVRProviderGetRecordingById(QBPVRProvider self, SvString recordingId);

/**
 * Get current duration of a recording.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] recording     recording handle
 * @return                  current recording duration
 */
int
QBPVRProviderGetRecordingCurrentDuration(QBPVRProvider self,
                                         QBPVRRecording recording);

/**
 * Update recording basing on the recording and directory IDs.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] recording     recording to be updated
 * @param[in] params        recording update parameters
 * @return                  created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderUpdateRecording(QBPVRProvider self,
                             QBPVRRecording recording,
                             QBPVRRecordingUpdateParams params);

/**
 * Move recording to directory.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] recording     recording to be updated
 * @param[in] directory     new directory, if @c NULL it will
 *                          be move to default directory
 * @return                  created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderMoveRecording(QBPVRProvider self,
                           QBPVRRecording recording,
                           QBPVRDirectory directory);

/**
 * Check if recording created with given schedule parameters would have conflicts.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self                      QBPVRProvider handle
 * @param[in] params                    recording schedule parameters
 * @param[in] treatAsRemoved            treat those recordings as if they were removed
 * @param[out] conflicts                conflicts array
 * @param[out] timeWithoutConflicts     time without conflicts
 * @param[out] conflictReason           conflict reason
 * @return                              @c 0 if no conflicts found, @c <0 otherwise
 */
int
QBPVRProviderCheckRecordingConflicts(QBPVRProvider self,
                                     QBPVRRecordingSchedParams params,
                                     SvArray treatAsRemoved,
                                     SvArray *conflicts,
                                     int *timeWithoutConflicts,
                                     QBPVRProviderConflictReason *conflictReason);

/**
 * Schedule new recording.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self         QBPVRProvider handle
 * @param[in] params       recording schedule parameters
 * @return                 created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderScheduleRecording(QBPVRProvider self, QBPVRRecordingSchedParams params);

/**
 * Stop given recording.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] recording     recording
 * @return                  created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderStopRecording(QBPVRProvider self, QBPVRRecording recording);

/**
 * Delete given recording.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] recording     recording
 * @return                  created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderDeleteRecording(QBPVRProvider self, QBPVRRecording recording);

/**
 * Delete all recordings that are in the given state.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] params        state of recordings to be deleted
 * @return                  created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderDeleteAllInState(QBPVRProvider self,
                              QBPVRProviderDeleteAllInStateParams params);

/**
 * Delete all recordings from the given category.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] category      category
 * @return                  created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderDeleteAllFromCategory(QBPVRProvider self, QBContentCategory category);

/**
 * Lock/unlock given recording.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] recording     recording to be locked/unlocked
 * @param[in] lock          requested state of the lock
 * @return                  created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderLockRecording(QBPVRProvider self, QBPVRRecording recording, bool lock);

/**
 * Mark recording viewed.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] recording     recording to be marked viewed
 * @return                  created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderMarkRecordingViewed(QBPVRProvider self, QBPVRRecording recording);

/**
 * Add PVR provider listener.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] listener      listener object
 */
void
QBPVRProviderAddListener(QBPVRProvider self, SvObject listener);

/**
 * Remove PVR provider listener.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] listener      listener object
 */
void
QBPVRProviderRemoveListener(QBPVRProvider self, SvObject listener);

/**
 * Create PVR directory.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] params        new directory parameters
 * @return                  created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderCreateDirectory(QBPVRProvider self, QBPVRDirectoryCreateParams params);

/**
 * Delete PVR directory.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] directory     directory
 * @param[in] params        directory update parameters
 * @return                  created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderDeleteDirectory(QBPVRProvider self,
                             QBPVRDirectory directory,
                             QBPVRDirectoryDeleteParams params);

/**
 * Update PVR directory.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] directory     directory
 * @param[in] params        data identifying recording
 * @return                  created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderUpdateDirectory(QBPVRProvider self,
                             QBPVRDirectory directory,
                             QBPVRDirectoryUpdateParams params);

/**
 * Return list of the PVR directories.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @return array of directories
 */
SvArray
QBPVRProviderCreateDirectoriesList(QBPVRProvider self);

/**
 * Check if directory limit is already reached.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] type          type of directory to be checked
 * @return                  is limit reached
 */
bool
QBPVRProviderCheckDirectoryLimit(QBPVRProvider self, QBPVRDirectoryType type);

/**
 * Check if scheduled limit is already reached.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self         QBPVRProvider handle
 * @return                 is limit reached
 */
bool
QBPVRProviderCheckScheduledLimit(QBPVRProvider self);

/**
 * Stop recordings from given directory if it's scheduled series,
 * keyword or repeated recordings.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] directory     directory
 * @return                  created QBPVRProviderRequest
 */
QBPVRProviderRequest
QBPVRProviderStopDirectory(QBPVRProvider self, QBPVRDirectory directory);

/**
 * Get capabilities of PVR provider implementation.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @return                  provider's capabilities
 */
const QBPVRProviderCapabilities *
QBPVRProviderGetCapabilities(QBPVRProvider self);

/**
 * Get current recording for channel
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] channel       channel handle
 * @return                  current recording handle, @c NULL if no recordings
 */
QBPVRRecording
QBPVRProviderGetCurrentRecordingForChannel(QBPVRProvider self, SvTVChannel channel);

/**
 * Get all recordings that start before end time and end after start time
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] channelId     channel id
 * @param[in] start         search start point
 * @param[in] end           search end point
 * @param[in] params        search parameters
 * @return                  array of recordings in time, it's retained,
 *                          so the returned array must be released afterwards,
 *                          if there are no recordings that fulfill given constraints,
 *                          @c NULL is returned
 */
SvArray
QBPVRProviderFindRecordingsInTimeRange(QBPVRProvider self, SvString channelId,
                                       SvTime start, SvTime end,
                                       QBPVRProviderGetRecordingsParams params);

/**
 * Get all recordings with given params
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self         QBPVRProvider handle
 * @param[in] params       get parameters
 * @return                 array of recordings, it's retained,
 *                         so the returned array must be released afterwards,
 *                         if there are no recordings that fulfill given constraints
 */
SvArray
QBPVRProviderGetRecordings(QBPVRProvider self,
                           QBPVRProviderGetRecordingsParams params);

/**
 * Tell if PVR provider can record.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @return                  @c true if provider can record
 */
bool
QBPVRProviderCanRecord(QBPVRProvider self);

/**
 * Tell if given EPG event is recordable.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] event         SvEPGEvent handle
 * @return                  @c true if event is recordable
 */
bool
QBPVRProviderIsEventRecordable(QBPVRProvider self, SvEPGEvent event);

/**
 * Tell if recording of the given event can be scheduled.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] event         SvEPGEvent handle
 * @return                  @c true if the recording of the event can be scheduled
 */
bool
QBPVRProviderCanScheduleEventRecording(QBPVRProvider self, SvEPGEvent event);

/**
 * Create information string with recording failure reason.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self         QBPVRProvider handle
 * @param[in] recording    failed QBPVRRecording
 * @return                 failure reason description as SvString
 */
SvString
QBPVRProviderCreateRecordingFailureReasonString(QBPVRProvider self, QBPVRRecording recording);

/**
 * Set recording property.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] recording     recording handle
 * @param[in] key           property key
 * @param[in] value         property value, if @c NULL passed delete property
 */
void
QBPVRProviderSetRecordingProperty(QBPVRProvider self,
                                  QBPVRRecording recording,
                                  SvString key,
                                  SvObject value);

/**
 * Get recording property.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] recording     recording handle
 * @param[in] key           property key
 * @return                  property value, @c NULL if property not found
 */
SvObject
QBPVRProviderGetRecordingProperty(QBPVRProvider self,
                                  QBPVRRecording recording,
                                  SvString key);

/**
 * Create content for given recording.
 *
 * @memberof QBPVRProvider
 *
 * @param[in] self          QBPVRProvider handle
 * @param[in] recording     recording handle
 * @return                  content for recording
 */
SvContent
QBPVRProviderCreateContentForRecording(QBPVRProvider self,
                                       QBPVRRecording recording);

/**
 * Get current quota;
 *
 * @param[in] self          QBPVRProvider handle
 * @return                  quota
 */
QBPVRQuota
QBPVRProviderGetCurrentQuota(QBPVRProvider self);

/**
 * Get active directories list for given EPG event.
 *
 * Active directories are directories related to non-stopped
 * keyword, repeated and series recordings.
 *
 * @param self              QBPVRProvider handle
 * @param event             EPG event handle
 * @return                  active directories list
 */
SvArray
QBPVRProviderCreateActiveDirectoriesListForEvent(QBPVRProvider self, SvEPGEvent event);

/**
 * QBPVRProvider listener.
 **/
typedef const struct QBPVRProviderListener_ {
    /**
     * New recording added.
     *
     * @param[in] self          listener handle
     * @param[in] recording     recording handle
     **/
    void (*recordingAdded)(SvObject self, QBPVRRecording recording);

    /**
     * Recording changed.
     *
     * @param[in] self          listener handle
     * @param[in] recording     recording handle
     * @param[in] oldRecording  old recording copy handle
     **/
    void (*recordingChanged)(SvObject self, QBPVRRecording recording, QBPVRRecording oldRecording);

    /**
     * Recording removed.
     *
     * @param[in] self          listener handle
     * @param[in] recording     recording handle
     **/
    void (*recordingRemoved)(SvObject self, QBPVRRecording recording);

    /**
     * Recording couldn't be started because of some restrictions (for example broadcaster limitation).
     *
     * @param[in] self          listener handle
     * @param[in] recording     recording handle
     **/
    void (*recordingRestricted)(SvObject self, QBPVRRecording recording);

    /**
     * PVR provider quota changed.
     *
     * This method is called when quota changes
     * (this includes situations in which provider state changes to
     * state in which recording is possible or vice versa).
     *
     * @param[in] self          listener handle
     * @param[in] recording     recording handle
     **/
    void (*quotaChanged)(SvObject self, QBPVRQuota quota);

    /**
     * Directory added.
     *
     * This method is called when new directory is added.
     *
     * @param[in] self          listener handle
     * @param[in] dir           added directory
     **/
    void (*directoryAdded)(SvObject self, QBPVRDirectory dir);
} *QBPVRProviderListener;

/**
 * Get runtime type identification object representing
 * QBPVRProviderListener interface.
 *
 * @return QBPVRProviderListener interface object
**/
extern SvInterface
QBPVRProviderListener_getInterface(void);

/**
 * @}
 **/

#endif /* QB_PVR_PROVIDER_H_ */
