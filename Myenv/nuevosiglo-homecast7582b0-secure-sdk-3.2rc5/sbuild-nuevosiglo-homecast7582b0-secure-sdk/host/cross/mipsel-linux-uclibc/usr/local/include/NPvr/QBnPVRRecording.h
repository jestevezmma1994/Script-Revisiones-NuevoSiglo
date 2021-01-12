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

#ifndef NPVR_RECORDING_H
#define NPVR_RECORDING_H

/**
 * @file QBnPVRRecording.h QBnPVRRecording class.
 * @brief Stores detailed info about recording.
 **/

#include <SvPlayerKit/SvEPGEvent.h>
#include <SvPlayerKit/SvContent.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvCoreTypes.h>

#include <time.h>
#include <stdbool.h>

#include <NPvr/QBnPVRQuota.h>

/**
 * @defgroup QBnPVRRecording The NPvr recording class.
 * @ingroup NPvr
 * @{
 **/

/**
 * QBnPVRRecording type.
 */
struct QBnPVRRecording_;
typedef struct QBnPVRRecording_  *QBnPVRRecording;

/**
 * State of the NPvr recording.
 */
typedef enum {
    QBnPVRRecordingState_scheduled,   /// server accepted to schedule the recording
    QBnPVRRecordingState_active,      /// recording physically exists, and so is accessible for playback (at least some part of it)
    QBnPVRRecordingState_completed,   /// recording physically exists, and so is accessible for playback (completely)
    QBnPVRRecordingState_failed,      ///
    QBnPVRRecordingState_missed,      ///
    QBnPVRRecordingState_removed      /// recording was removed from the server
} QBnPVRRecordingState;

/**
 * Type of the NPvr recording.
 */
typedef enum {
    QBnPVRRecordingType_none,
    QBnPVRRecordingType_keyword,
    QBnPVRRecordingType_series,
    QBnPVRRecordingType_event,
    QBnPVRRecordingType_OTR,
    QBnPVRRecordingType_manual
} QBnPVRRecordingType;

/**
 * Type of the NPvr recording change.
 */
typedef enum {
    QBnPVRRecordingListenerState_Added,
    QBnPVRRecordingListenerState_Changed,
    QBnPVRRecordingListenerState_Removed
} QBnPVRRecordingListenerState;

typedef enum {
    QBnPVRRecordingSpaceRecovery_DeleteOldest,
    QBnPVRRecordingSpaceRecovery_Manual
} QBnPVRRecordingSpaceRecovery;

/*! Stores info about recording deletion reason. */
typedef enum {
    QBnPVRRecordingDeleteReason_DeleteOldest = 10,  /*!< There was to small amount of quota. */
    QBnPVRRecordingDeleteReason_Manual              /*!< Deleted by user. */
} QBnPVRRecordingDeleteReason;

/**
 * QBnPVRRecording series ID type.
 */
typedef SvString QBnPVRRecordingSeries;

/**
 * QBnPVRRecording keyword ID type.
 */
typedef SvString QBnPVRRecordingKeyword;

/**
 * QBnPVRRecording failure reason type.
 */
typedef int QBnPVRRecordingFailureReason;

/**
 * QBnPVRRecording schedule command parameters.
 **/
typedef struct QBnPVRRecordingSchedParams_ {
    SvString channelId;
    QBnPVRRecordingType type;
    SvEPGEvent event;
    int startMargin;
    int endMargin;
    SvString keyword;
    SvValue recoveryOption;
    SvString seriesID;
    SvValue recordPremieresOnly;
} *QBnPVRRecordingSchedParams;

/**
 * QBnPVRRecording update command parameters.
 **/
typedef struct {
    SvString recordingId;
    SvString directoryId;
    SvValue recoveryOption;
} QBnPVRRecordingUpdateParams;

/**
 * QBnPVRRecording type.
 */
struct QBnPVRRecording_ {
  struct SvObject_ super_;

  SvString  id;
  SvString  channelId;

  SvString  channelName;

  SvEPGEvent event;

  SvTime    startTime;
  SvTime    endTime;

  QBnPVRRecordingType type;
  QBnPVRRecordingState  state;

  QBnPVRRecordingFailureReason failureReason;
  QBnPVRRecordingDeleteReason deleteReason;

  bool  playable; /// some servers do allow playback while still recording, some only when completed
  bool isAdult;

  SvString sourceUrl;

  SvTime startMargin;
  SvTime endMargin;
  SvString directoryId;
  QBnPVRQuota quota;

  QBnPVRRecordingSpaceRecovery recoveryOption;
};

/**
 * Get runtime type identification object representing QBnPVRRecording class.
 **/
extern SvType
QBnPVRRecording_getType(void);

typedef const struct QBnPVRProviderRecordingListener_ {
  void (*added)(SvObject self, QBnPVRRecording recording);
  void (*changed)(SvObject self, QBnPVRRecording recording);
  void (*removed)(SvObject self, QBnPVRRecording recording);
} *QBnPVRProviderRecordingListener;

/**
 * Get runtime type identification object representing
 * QBnPVRProviderRecordingListener interface.
 **/
extern SvInterface
QBnPVRProviderRecordingListener_getInterface(void);

/**
 * @}
 **/

#endif // #ifndef NPVR_RECORDING_H
