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

#ifndef QB_SCHED_MANAGER_H
#define QB_SCHED_MANAGER_H

#include "QBSchedDesc.h"

#include <SvFoundation/SvString.h>
#include <SvPlayerKit/SvContent.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <fibers/c/fibers.h>
#include <QBTunerTypes.h>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum QBSchedManagerEventSource_ {
    QBSchedManagerEventSource_Error = -1,
    QBSchedManagerEventSource_PresentFollowing = 0,
    QBSchedManagerEventSource_EPGManager,
    QBSchedManagerEventSource_Count
} QBSchedManagerEventSource;

typedef struct QBSchedManagerAppCallbacks_s  QBSchedManagerAppCallbacks;
struct QBSchedManagerAppCallbacks_s
{
  /// extract info about event \a params->eventId from channel \a params->channelId
  /// \returns negative, if failed to get requested info, zero on success
  int  (*extractEventInfo) (void* target, const QBSchedParams* params, QBSchedParamsExtracted* extracted);

  /// get current event
  /// \returns negative, if failed to get requested event , zero on success
  int (*getCurrentEvent) (void* target, const SvString channelId, SvEPGEvent* out);

  /// instruct the app that it has to start watching given channel
  /// \returns negative on error, else zero
  int  (*startWatching)    (void* target, QBSchedDesc* desc);

  /// try to (un)reserve a tuner with given frequency \a muxid
  /// \returns 0 on success, negative on error
  int  (*reserveTuner)     (void* target, struct QBTunerParams* tunerParams, bool reserve);

  /// apply proper metadata+resources to the content that is to be recorded
  /// e.g. which tuner should be used
  /// \returns negative on error, else zero
  int  (*addResources)     (void* target, const QBSchedDesc* desc, SvContent content);

  int  (*getChannelCost)   (void* target, int costType);

  /// inform that someone want to delete given record
  /// \returns negative on error, else zero
  int (*stopUsing)         (void* target, const QBSchedDesc* desc);
};

void QBSchedManagerRegisterApp(void* target, const QBSchedManagerAppCallbacks* callbacks);

typedef struct QBSchedManagerInfoCallbacks_s  QBSchedManagerInfoCallbacks;
struct QBSchedManagerInfoCallbacks_s
{
  /**
   * Callback called when new schedule was added
   *
   * @param[in] target callback private data
   * @param[in] desc description of added schedule
   */
  void (*added) (void* target, QBSchedDesc* desc);

  /**
   * Callback called when schedule changed its state and/or range
   *
   * @param[in] target callback private data
   * @param[in] desc description of updated schedule
   * @param[in] oldState old scheduler state
   * @param[in] oldStartTime old statrt time
   * @param[in] oldStopTime old stop time state
   */
  void (*updated) (void* target, QBSchedDesc* desc, QBSchedState oldState, SvLocalTime oldStartTime, SvLocalTime oldStopTime);

  /**
   * Callback called when schedule was removed
   *
   * @param[in] target callback private data
   * @param[in] desc description of removed schedule
   */
  void (*removed) (void* target, QBSchedDesc* desc);

  /**
   * Callback called when recording cannot be started because of restrictions
   *
   * @param[in] target callback private data
   * @param[in] desc description of restricted recording
   */
  void (*restriction) (void* target, QBSchedDesc* desc);
};

void QBSchedManagerRegisterInfo(void* target, const QBSchedManagerInfoCallbacks* callbacks, SvString name);
void QBSchedManagerUnregisterInfo(void* target, const QBSchedManagerInfoCallbacks* callbacks);


void QBSchedManagerInit(SvScheduler scheduler);
void QBSchedManagerDeinit(void);
void QBSchedManagerStart(void);
void QBSchedManagerStop(void);

void QBSchedManagerSetStorage(QBRecordFSRoot* root);

void QBSchedManagerSetRecCostLimit(int totalCost);
void QBSchedManagerSetRecSessionsLimit(int cnt);
void QBSchedManagerSetTunerLimit(int cnt);
void QBSchedManagerSetEITMonitorStartMargin(int seconds);

unsigned int QBSchedManagerGetScheduledCount(void);

/** Notify that the time-date and time-zone info might have changed unecpectedly.
 *  The manager might want to update the schedules that are "duration-based"
 *  ("stop-time-based" do not need to be updated).
 */
void QBSchedManagerTimeDateRulesUpdated(void);

/** Notify that event \a ev has been parsed, or changed.
 *  If there are any event-based schedules for \a ev, update their start/stopTime.
 */
void QBSchedManagerEventUpdated(SvEPGEvent ev);


/**
 * Conflict reason for new schedule.
 **/
typedef enum {
  QBSchedManagerConflictReason_driveTooSlow,   ///< connected drive is too slow to perform added recording, even if you remove other
                                               ///< scheduled recordings
  QBSchedManagerConflictReason_tooManyRecordingSessions, ///< too many recording sessions
  QBSchedManagerConflictReason_other        ///< other reason
} QBSchedManagerConflictReason;

/** Test adding new schedule.
 * If not possible due to conflicts, returns reason in \a conflictReasonOut and zero or more QBSchedDesc* to \a conflictsOut.
 * Empty \a conflictsOut means adding new schedule won't succeed, but if there are elements, user can try to resolve conflict with picking one for removal.
 * If one or more schedule is chosen for removal, it should be added to \a treatAsRemoved, and then this function should be called again.
 * Repeat until no more conflicts are detected - then the app should remove all chosen recordings with \a QBSchedManagerDelete()
 * and use \a QBSchedManagerAdd().
 * \returns 0 when there are no more conflicts, negative if there are still conflicts
 */
int QBSchedManagerTestConflicts(const QBSchedParams* params,
                                SvArray treatAsRemoved,
                                SvArray* conflictsOut,
                                QBSchedManagerConflictReason* conflictReasonOut);

/** Test how long this schedule could last, causing no conflicts.
 * \returns -1 when no conflicts are possible
 * \returns duration in seconds, from now, when first conflict would happen
 */
int QBSchedManagerTestDuration(const QBSchedParams* params, SvArray treatAsRemoved);

/** Test how long is there for start of the next schedule.
 * \param type only schedules of this type, or \c -1 to check everything
 * \returns duration in seconds, from now, when first schedule will start
 * \returns 0, if a schedule is currently active
 * \returns -1, if nothing is schedule nor active
 */
int QBSchedManagerGetNearest(QBSchedType type);

/** Change scheduled start/stopTime of \a desc.
 */
void QBSchedManagerChange(QBSchedDesc* desc, SvLocalTime startTime, SvLocalTime stopTime);

/** Add new schedule. Conflicts will be ingored, and they will be automatically handled when the time comes.
 * \returns negative on error, zero on success
 */
int QBSchedManagerAdd(const QBSchedParams* params, QBSchedDesc** out);

/** Deletes a schedule (and all underlying files, if present).
 */
void QBSchedManagerDeleteRecording(QBSchedDesc* desc);

void QBSchedManagerDeleteAll(QBSchedType type, QBSchedState state);
void QBSchedManagerDeleteAllScheduled(QBSchedType type);
void QBSchedManagerDeleteAllActive(QBSchedType type);
void QBSchedManagerDeleteAllCompleted(void);

/** Stops an ongoing recording manually.
 *  Started recordings are being stopped, but recorded data is kept.
 *  Pure schedules are being removed entirelly.
 */
void QBSchedManagerStopRecording(QBSchedDesc* desc);

/** Stops ongoing "watching" schedule, if present.
 *  Should be used when app wants to change the channel manually.
 *  No-op when no "watch" is currently active.
 */
void QBSchedManagerStopWatching(void);

/** Stops all ongoing recordings manually.
 */
void QBSchedManagerStopAllRecordings(void);

int QBSchedManagerFind(SvString channelId, SvLocalTime start, SvLocalTime end, SvArray* results, bool failed);
int QBSchedManagerFindActive(SvString channelId, SvArray* results);


QBSchedDesc* QBSchedManagerFindRecording_(int meta_id);
SvArray QBSchedManagerGetAll(void);

void QBSchedManagerUnReserveTuner(struct QBTunerParams *tunerParams);

void QBSchedManagerPropagateChange(QBSchedDesc *desc);

/**
 * Get count of ongoing recordings.
 *
 * @return count of ongoing recordings, -1 on error
 */
int QBSchedManagerGetOngoingRecordingsCount(void);

/**
 * Set event source. This field is propagate to record sink by meta property.
 *
 * @param[in] eventSource event source enum
 */
void QBSchedManagerSetEventsSource(QBSchedManagerEventSource eventSource);

/**
 * Get event source.
 *
 * @return eventSource event source enum
 */
QBSchedManagerEventSource QBSchedManagerGetEventsSource(void);

/**
 * Load recording from disk
 *
 * @param[in] file handle to recording
 * @return @c 0 while success, @c -1 in case of error
 */
int QBSchedManagerLoadRecordingFromDisk(QBRecordFSFile* file);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_SCHED_MANAGER_H
