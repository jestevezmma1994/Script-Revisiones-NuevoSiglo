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

#ifndef QB_SCHED_DESC_H
#define QB_SCHED_DESC_H

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvPlayerKit/SvContentMetaData.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <QBRecordFS/types.h>
#include <QBTunerTypes.h>
#include <SvPlayerManager/SvPVRTask.h>
#include <SvCore/SvLocalTime.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum QBSchedType_e  QBSchedType;
enum QBSchedType_e
{
  QBSchedType_record,
  QBSchedType_watch,
};

typedef struct QBSchedParams_s  QBSchedParams;
struct QBSchedParams_s
{
  QBSchedType  type;

  SvString  channelId;
  // set if scheduled for certain event (probably from EPG grid), otherwise is NULL
  SvEPGEvent  event;

  /// iff invalid and eventId is valid, use event's start time (from extracted params)
  SvLocalTime  startTime;
  /// iff invalid and eventId is valid, use event's stop time (from extracted params)
  SvLocalTime  stopTime;
  /// iff >= 0, use real-time-clock to measure when to stop this recording, don't use "stopTime"
  int duration;

  int  startMargin;
  int  stopMargin;

  int  costType;
  int  priority;
  SvContentMetaData  meta;
};

void QBSchedParamsInit(QBSchedParams *params);
void QBSchedParamsCopy(QBSchedParams *dest, const QBSchedParams *src);
void QBSchedParamsDestroy(QBSchedParams *params);

typedef struct QBSchedParamsExtracted_s  QBSchedParamsExtracted;
struct QBSchedParamsExtracted_s
{
  SvLocalTime  startTime;
  SvLocalTime  stopTime;
  SvLocalTime  expirationTime;
  int cost; /// not to be serialized, is must be calculated from "params.costType" every time.

  SvString  url;
  SvString  channelName;
  SvValue   channelIsAdult; // SvValueType_boolean

  /// extracted from url above
  /// non-zero iff the channel needs to use tuner with this frequency
  struct QBTunerParams tunerParams;
};

typedef enum QBSchedState_e  QBSchedState;
enum QBSchedState_e
{
  QBSchedState_scheduled,
  QBSchedState_active,
  QBSchedState_completed,
  QBSchedState_missed,
  QBSchedState_failed,
  QBSchedState_removed,
};
const char* QBSchedStateToString(QBSchedState state);

typedef struct QBSchedDesc_s QBSchedDesc;
struct QBSchedDesc_s {
  struct SvObject_ super_;

  QBSchedParams  params;
  QBSchedParamsExtracted  extracted;

  QBSchedState  state;

  // Array of SvEPGEvents deserialized from meta or events from DVB (present, following).
  SvArray  epgEvents;

  /// only for "active" state, important for schedules that use "duration", not "stopTime"
  struct {
    SvTime  realStartTime;
    bool eventPassed; /// in p/f
  } active;

  /// only for type "record"
  struct {
    /// NULL iff not yet started, or recording failed to launch.
    SvPVRTask  pvrTask;
    QBRecordFSFile* file;
  } rec;
};

SvType QBSchedDesc_getType(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_SCHED_DESC_H
