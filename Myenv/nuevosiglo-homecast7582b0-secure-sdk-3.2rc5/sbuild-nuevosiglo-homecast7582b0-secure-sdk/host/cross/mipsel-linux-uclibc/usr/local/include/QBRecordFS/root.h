/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
****************************************************************************/

#ifndef QB_RECORD_FS_ROOT_H
#define QB_RECORD_FS_ROOT_H

#include "types.h"

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvObject.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern SvType QBRecordFSRoot_getType(void);

int QBRecordFSUtilsIsDeviceInternal(const char* devPath);

/// Creates new Root object and loads all metadata from the underlying device.
QBRecordFSRoot* QBRecordFSRootOpen(const char* metaRoot, const char* dataRoot, SvObject authPlugin);

/// Free all in-memory structures and detach from the underlying device.
void QBRecordFSRootClose(const QBRecordFSRoot* root);

bool QBRecordFSRootHasFileStorage(const QBRecordFSRoot* root);
bool QBRecordFSRootHasTimeshiftStorage(const QBRecordFSRoot* root);
int QBRecordFSRootIsInternal(const QBRecordFSRoot* root);

void QBRecordFSRootSetGlobal(QBRecordFSRoot* root);
QBRecordFSRoot* QBRecordFSRootGetGlobal(void);

/** Detach all sessions from \a root.
 *  It could be done automatically in RootDestroy(), but it is usually done in another thread,
 *  so it's better to detach all sessions synchronously.
 */
void QBRecordFSRootDetachAllSessions(QBRecordFSRoot* root);
void QBRecordFSRootDetachAllFiles(QBRecordFSRoot* root);


struct QBRecordFSRootInfo_s
{
  int32_t fileCnt;

  int64_t bytesUsed;
  int64_t bytesMax;

  int64_t timeshiftBytesMax;
};
typedef struct QBRecordFSRootInfo_s  QBRecordFSRootInfo;

/** Get device statistics.
 *  @returns 0 on success, negative on error (invalid state)
 */
int QBRecordFSRootGetInfo(const QBRecordFSRoot* root, QBRecordFSRootInfo* info);


/** Get all files (active or not).
 *  @returns array of QBRecordFSFile objects, must be released
 */
SvArray QBRecordFSRootGetAllFiles(const QBRecordFSRoot* root);

struct QBRecordFSCallbacks_s
{
  void (*rootFSAdded) (void *callbackData);
  void (*rootFSRemoved) (void *callbackData);
  void (*rootFSDataModified) (void *callbackData);
};
typedef struct QBRecordFSCallbacks_s QBRecordFSCallbacks;
/** Set callbacks for root object.
 */
void QBRecordFSSetCallbacks(QBRecordFSCallbacks *callbacks, void *callbackData);
void QBRecordFSUnsetCallbacks(QBRecordFSCallbacks *callbacks);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_RECORD_FS_ROOT_H
