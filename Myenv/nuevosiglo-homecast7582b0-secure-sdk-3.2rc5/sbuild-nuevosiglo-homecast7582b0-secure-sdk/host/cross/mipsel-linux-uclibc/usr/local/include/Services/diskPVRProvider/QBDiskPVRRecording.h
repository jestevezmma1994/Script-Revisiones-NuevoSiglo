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

#ifndef QB_DISK_PVR_RECORDING_H_
#define QB_DISK_PVR_RECORDING_H_

/**
 * @file QBDiskPVRRecording.h Disk pvr recording file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBSchedDesc.h>
#include <SvPlayerKit/SvContent.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>

#include <stdbool.h>

/**
 * @defgroup QBDiskPVRRecording Disk pvr recording.
 * @ingroup QBDiskPVRProvider
 * @{
 **/

/**
 * Recording meta key of group id
 **/
#define REC_META_KEY__GROUP_ID  "group:id" // string

/**
 * QBDiskPVR recording.
 *
 * Disk PVR recording is a representation of recording that uses
 * storage drive connected to a STB.
 *
 * @class QBDiskPVRRecording
 * @extends QBPVRRecording
 **/
typedef struct QBDiskPVRRecording_ *QBDiskPVRRecording;

/**
 * Create disk PVR recording from sched manager recording description.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] desc              sched manager recording description
 * @param[out] errorOut         error info
 * @return                      created directory, @c NULL in case of error
 **/
QBDiskPVRRecording
QBDiskPVRRecordingCreate(QBSchedDesc *desc, SvErrorInfo *errorOut);

/**
 * Update disk PVR recording from its sched manager recording description.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 **/
void
QBDiskPVRRecordingUpdate(QBDiskPVRRecording self);

/**
 * Get sched manager recording description from recording.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 * @return                      sched manager recording description associated with recording
 **/
const QBSchedDesc *
QBDiskPVRRecordingGetSchedDesc(QBDiskPVRRecording self);

/**
 * Get recording group id.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 * @return                      group id
 **/
SvString
QBDiskPVRRecordingGetGroupId(QBDiskPVRRecording self);

/**
 * Does disk PVR recording have recording.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 * @return                      @c true if recording has file associated with it,
 *                              @c false otherwise
 **/
bool
QBDiskPVRRecordingHasFile(QBDiskPVRRecording self);

/**
 * Set lock on a recording.
 *
 * @memberof QBDiskPVRRecording
 *
 * Lock prevents recording from being removed.
 *
 * @param[in] self              disk PVR recording handle
 * @param[in] lock              lock value
 **/
void
QBDiskPVRRecordingSetLock(QBDiskPVRRecording self, bool lock);

/**
 * Set as already viewed.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 **/
void
QBDiskPVRRecordingSetAsAlreadyViewed(QBDiskPVRRecording self);

/**
 * Set group Id.
 *
 * @memberof QBDiskPVRRecording
 *
 * By group ID recordings are marked as part of PVR directory.
 *
 * @param[in] self              disk PVR recording handle
 * @param[in] groupId           group id
 **/
void
QBDiskPVRRecordingSetGroupId(QBDiskPVRRecording self, SvString groupId);

/**
 * Set recording property.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 * @param[in] key               property key
 * @param[in] value             property value
 **/
void
QBDiskPVRRecordingSetRecordingProperty(QBDiskPVRRecording self,
                                       SvString key,
                                       SvObject value);

/**
 * Get recording property.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 * @param[in] key               property key
 * @return                      property value, @c NULL if not found
 **/
SvObject
QBDiskPVRRecordingGetRecordingProperty(QBDiskPVRRecording self,
                                       SvString key);

/**
 * Change recording start and end time.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 * @param[in] startTime         new start time
 * @param[in] endTime           new end time
 **/
void
QBDiskPVRRecordingChangeStartEndTime(QBDiskPVRRecording self,
                                     SvTime startTime,
                                     SvTime endTime);

/**
 * Stop recording.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 **/
void
QBDiskPVRRecordingStop(QBDiskPVRRecording self);

/**
 * Delete recording.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 **/
void
QBDiskPVRRecordingDelete(QBDiskPVRRecording self);

/**
 * Get file size.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 * @return                      file size, @c -1 if no file present
 **/
int64_t
QBDiskPVRRecordingGetFileSize(QBDiskPVRRecording self);

/**
 * Get recording duration.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 * @return                      recording duration in seconds
 **/
int
QBDiskPVRRecordingGetDuration(QBDiskPVRRecording self);

/**
 * Get recording mux id.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 * @return                      recording mux id handle
 **/
const struct QBTunerMuxId *
QBDiskPVRRecordingGetMuxId(QBDiskPVRRecording self);

/**
 * Create content for recording.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 * @return                      content, @c NULL if content can't be created;
 **/
SvContent
QBDiskPVRRecordingCreateContent(QBDiskPVRRecording self);

/**
 * Check if recording will change position,
 * either because it's state changed, or it's added/removed to a directory.
 * WARNING this function needs to be in sync with QBDiskPVRProviderLayout.
 *
 * @memberof QBDiskPVRRecording
 *
 * @param[in] self              disk PVR recording handle
 * @return                      @c true if recording will change position, @c false otherwise
 **/
bool
QBDiskPVRRecordingWillRecordingChangePosition(QBDiskPVRRecording self);

/**
 * @}
 **/

#endif /* QB_DISK_PVR_RECORDING_H_ */
