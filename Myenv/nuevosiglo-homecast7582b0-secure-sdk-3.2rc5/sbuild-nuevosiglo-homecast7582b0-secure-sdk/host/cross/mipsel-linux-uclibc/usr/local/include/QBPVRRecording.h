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

#ifndef QB_PVR_RECORDING_H_
#define QB_PVR_RECORDING_H_

/**
 * @file QBPVRRecording.h QBPVRRecording class.
 * @brief Stores detailed info about recording.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBPVRTypes.h"

#include <SvPlayerKit/SvEPGEvent.h>
#include <QBAppKit/QBObservable.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvWeakList.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvCore/SvTime.h>
#include <stdbool.h>
#include <time.h>

/**
 * @defgroup QBPVRRecording The PVR recording class.
 * @ingroup QBPVRProvider
 * @{
 **/

/**
 * State of the PVR recording.
 */
typedef enum {
    QBPVRRecordingState_unknown,                ///< unknown
    QBPVRRecordingState_scheduled,              ///< scheduled
    QBPVRRecordingState_active,                 ///< currently recording
    QBPVRRecordingState_completed,              ///< done
    QBPVRRecordingState_interrupted,            ///< recording interrupted
    QBPVRRecordingState_stoppedManually,        ///< stopped manually
    QBPVRRecordingState_stoppedNoSpace,         ///< stopped because of no free space
    QBPVRRecordingState_stoppedNoSource,        ///< stopped because of lack of source
    QBPVRRecordingState_failed,                 ///< recording failed
    QBPVRRecordingState_missed,                 ///< recording missed
    QBPVRRecordingState_removed,                ///< recording was removed
    QBPVRRecordingState_cnt                     ///< count of recording states
} QBPVRRecordingState;

/**
 * Recording delete reason.
 */
typedef enum {
    QBPVRRecordingDeleteReason_DeleteOldest,       ///< deleted due to auto recovery
    QBPVRRecordingDeleteReason_Manual              ///< manual, deleted by user
} QBPVRRecordingDeleteReason;

/**
 * QBPVRRecording failure reason type.
 */
typedef int QBPVRRecordingFailureReason;

/**
 * QBPVR recording.
 * @class QBPVRRecording
 * @extends QBObservable
 **/
typedef struct QBPVRRecording_ {
    struct QBObservable_ super_;        ///< super type

    SvWeakReference directory;          ///< directory, @c NULL if recording isn't in any directory

    bool lock;                          ///< recording locked
    bool viewed;                        ///< recording already viewed

    SvString  id;                       ///< recording id

    SvString channelId;                 ///< channel id
    SvString channelName;               ///< channel name

    SvEPGEvent event;                   ///< event handle to event for which recording was created

    SvArray epgEvents;                  ///< collected EPG events

    SvTime startTime;                   ///< recording start time
    SvTime endTime;                     ///< recording end time
    SvTime expirationTime;              ///< recording expiration time, @c > 0 if valid, otherwise recording will never expire

    QBPVRRecordingType type;            ///< recording type
    QBPVRRecordingState state;          ///< recording state

    QBPVRRecordingFailureReason failureReason;    ///< recording failure reason
    QBPVRRecordingDeleteReason deleteReason;      ///< recording delete reason

    bool playable;           ///< is recording playable
    bool isAdult;            ///< is recording adult

    QBPVRRecordingSpaceRecovery recoveryOption;    ///< recording space recovery option

    SvWeakList listeners;                          ///< listeners
} *QBPVRRecording;

/**
 * Get runtime type identification object representing QBPVRRecording class.
 *
 * @memberof QBPVRRecording
 *
 * @return QBPVRRecording runtime type identification object
 **/
extern SvType
QBPVRRecording_getType(void);

/**
 * @}
 **/

#endif /* QB_PVR_RECORDING_H_ */
