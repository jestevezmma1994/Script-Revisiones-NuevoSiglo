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

#ifndef QB_PVR_PROVIDER_VTABLE_H_
#define QB_PVR_PROVIDER_VTABLE_H_

/**
 * @file QBPVRProviderVTable.h Virtual PVR provider class VTable.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBPVRDirectory.h"
#include "QBPVRProvider.h"
#include "QBPVRProviderRequest.h"
#include "QBPVRRecording.h"
#include "QBPVRTypes.h"

#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvPlayerKit/SvContent.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentTree.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvTime.h>

/**
 * @defgroup QBPVRProviderVTable QBPVRProviderVTable: Virtual PVR provider class VTable
 * @ingroup QBPVRProvider
 * @{
 **/

/**
 * Virtual methods of the PVR provider class.
 **/
typedef const struct QBPVRProviderVTable_ {
    /// virtual methods of the base class
    struct QBContentProviderVTable_ super_;

    /**
     * Get content tree.
     *
     * @param[in] self          QBPVRProvider handle
     * @return                  content tree of PVR provider
     */
    QBContentTree (*getTree)(QBPVRProvider self);

    /**
     * Get recording by Id.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] recordingId   recording id
     * @return                  recording with given id, @c NULL if not found
     */
    QBPVRRecording (*getRecordingById)(QBPVRProvider self, SvString recordingId);

    /**
     * Get recording tuner number for an active recording.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] recording     recording handle
     * @return                  current recording duration
     */
    int (*getRecordingCurrentDuration)(QBPVRProvider self,
                                       QBPVRRecording recording);

    /**
     * Update recording.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] recording     recording to be updated
     * @param[in] params        recording update parameters
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*updateRecording)(QBPVRProvider self,
                                            QBPVRRecording recording,
                                            QBPVRRecordingUpdateParams params);

    /**
     * Move recording to directory.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] recording     recording to move
     * @param[in] directory     new directory
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*moveRecording)(QBPVRProvider self,
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
    int (*checkRecordingConflicts)(QBPVRProvider self,
                                   QBPVRRecordingSchedParams params,
                                   SvArray treatAsRemoved,
                                   SvArray *conflicts,
                                   int *timeWithoutConflicts,
                                   QBPVRProviderConflictReason *conflictReason);
    /**
     * Schedule new recording.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] params        recording schedule parameters
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*scheduleRecording)(QBPVRProvider self, QBPVRRecordingSchedParams params);

    /**
     * Stop given recording.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] recording     recording
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*stopRecording)(QBPVRProvider self, QBPVRRecording recording);

    /**
     * Delete given recording.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] recording     recording
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*deleteRecording)(QBPVRProvider self, QBPVRRecording recording);

    /**
     * Delete all recordings that are in the given state.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] params        state of recordings to be deleted
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*deleteAllInState)(QBPVRProvider self,
                                             QBPVRProviderDeleteAllInStateParams params);

    /**
     * Delete all recordings from the given category.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] category      category
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*deleteAllFromCategory)(QBPVRProvider self, QBContentCategory category);

    /**
     * Lock/unlock given recording.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] recording     recording to be locked/unlocked
     * @param[in] lock          requested state of the lock
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*lockRecording)(QBPVRProvider self, QBPVRRecording recording, bool lock);

    /**
     * Mark recording viewed.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] recording     recording to be marked viewed
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*markRecordingViewed)(QBPVRProvider self, QBPVRRecording recording);

    /**
     * Add PVR provider listener.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] listener      listener object
     */
    void (*addListener)(QBPVRProvider self, SvObject listener);

    /**
     * Remove PVR provider listener.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] listener      listener object
     */
    void (*removeListener)(QBPVRProvider self, SvObject listener);

    /**
     * Create PVR directory.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] params        new directory parameters
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*createDirectory)(QBPVRProvider self,
                                            QBPVRDirectoryCreateParams params);

    /**
     * Delete PVR directory.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] directory     directory
     * @param[in] params        directory delete parameters
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*deleteDirectory)(QBPVRProvider self,
                                            QBPVRDirectory directory,
                                            QBPVRDirectoryDeleteParams params);

    /**
     * Update PVR directory.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] directory     directory
     * @param[in] params        directory update parameters
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*updateDirectory)(QBPVRProvider self,
                                            QBPVRDirectory directory,
                                            QBPVRDirectoryUpdateParams params);

    /**
     * Return list of directories.
     *
     * @param[in] self          QBPVRProvider handle
     * @return array of directories
     */
    SvArray (*listDirectories)(QBPVRProvider self);

    /**
     * Check if directory limit is already reached.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] type          type of directory to be checked
     * @return                  is limit reached
     */
    bool (*checkDirectoryLimit)(QBPVRProvider self, QBPVRDirectoryType type);

    /**
     * Check if scheduled limit is already reached.
     *
     * @memberof QBPVRProvider
     *
     * @param[in] self          QBPVRProvider handle
     * @return                  is limit reached
     */
    bool (*checkScheduledLimit)(QBPVRProvider self);

    /**
     * Stop recordings from given directory if it's scheduled series, keyword or repeated recordings.
     * After that operation it will became normal directory.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] directory     directory
     * @return                  created QBPVRProviderRequest
     */
    QBPVRProviderRequest (*stopDirectory)(QBPVRProvider self, QBPVRDirectory directory);

    /**
     * Get capabilites of QBPVR provider implementation.
     *
     * @param[in] self          QBPVRProvider handle
     * @return                  provider's capabilities
     */
    const QBPVRProviderCapabilities *(*getCapabilities)(QBPVRProvider self);

    /**
     * Get current recording for channel
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] channel       channel handle
     * @return                  current recording handle, @c NULL if no recordings
     */
    QBPVRRecording (*getCurrentRecordingForChannel)(QBPVRProvider self, SvTVChannel channel);

    /**
     * Get all recordings that start before end time and end after start time
     * with given params
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] channelId     channel id
     * @param[in] start         search start point
     * @param[in] stop          search end point
     * @param[in] params        search parameters
     * @return                  array of recordings in time, it's retained,
     *                          so the returned array must be released afterwards,
     *                          if there are no recordings that fulfill given constraints
     *                          @c NULL is returned
     */
    SvArray (*findRecordingsInTimeRange)(QBPVRProvider self, SvString channelId,
                                         SvTime start, SvTime end,
                                         QBPVRProviderGetRecordingsParams params);

    /**
     * Get all recordings with given params
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] params        get recordings parameters
     * @return                  array of recordings, it's retained,
     *                          so the returned array must be released afterwards,
     *                          if there are no recordings that fulfill given constraints
     */
    SvArray (*getRecordings)(QBPVRProvider self,
                             QBPVRProviderGetRecordingsParams params);

    /**
     * Tell if PVR provider can record.
     *
     * @param[in] self          QBPVRProvider handle
     * @return                  @c true if provider can record
     */
    bool (*canRecord)(QBPVRProvider self);

    /**
     * Tell if given EPG event is recordable.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] event         SvEPGEvent handle
     * @return                  @c true if event is recordable
     */
    bool (*isEventRecordable)(QBPVRProvider self, SvEPGEvent event);

    /**
     * Tell if recording of the given event can be scheduled.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] event         SvEPGEvent handle
     * @return                  @c true if the recording of the event can be scheduled
     */
    bool (*canScheduleEventRecording)(QBPVRProvider self, SvEPGEvent event);

    /**
     * Create information string with recording failure reason.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] recording     failed QBPVRRecording
     * @return                  failure reason description as SvString
     */
    SvString (*createRecordingFailureReasonString)(QBPVRProvider self, QBPVRRecording recording);

    /**
     * Set recording property.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] recording     recording handle
     * @param[in] key           property key
     * @param[in] value         property value
     */
    void (*setRecordingProperty)(QBPVRProvider self,
                                 QBPVRRecording recording,
                                 SvString key,
                                 SvObject value);

    /**
     * Get recording property.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] recording     recording handle
     * @param[in] key           property key
     * @return                  property value
     */
    SvObject (*getRecordingProperty)(QBPVRProvider self,
                                     QBPVRRecording recording,
                                     SvString key);

    /**
     * Create content for given recording.
     *
     * @param[in] self          QBPVRProvider handle
     * @param[in] recording     recording handle
     * @return                  content for recording
     */
    SvContent (*createContentForRecording)(QBPVRProvider self,
                                           QBPVRRecording recording);

    /**
     * Get current quota;
     *
     * @param[in] self          QBPVRProvider handle
     * @return                  quota
     */
    QBPVRQuota (*getCurrentQuota)(QBPVRProvider self);

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
    SvArray (*listActiveDirectoriesForEvent)(QBPVRProvider self,
                                             SvEPGEvent event);
} *QBPVRProviderVTable;

/**
 * @}
 **/

#endif /* QB_PVR_PROVIDER_VTABLE_H_ */
