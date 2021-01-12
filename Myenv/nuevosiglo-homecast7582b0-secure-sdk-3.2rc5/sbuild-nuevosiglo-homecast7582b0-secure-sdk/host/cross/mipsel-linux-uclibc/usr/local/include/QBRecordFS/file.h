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

#ifndef QB_RECORD_FS_FILE_H
#define QB_RECORD_FS_FILE_H

#include "types.h"

#include <QBRecordFS/QBRecordFSDataEncryptionPlugin.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvArray.h>
#include <SvPlayerKit/SvContentMetaData.h>
#include <SvPlayerKit/SvContent.h>
#include <SvPlayerKit/SvBuf.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern SvType QBRecordFSFile_getType(void);


QBRecordFSFile* QBRecordFSFileCreate(QBRecordFSRoot* root, SvContentMetaData meta);

typedef struct {
  void (*read_finished)(void *callbackData, SvBuf sb, int svError);
} QBRecordFSFileCallbacks;

void QBRecordFSFileSetCallbacks(QBRecordFSFile* file, const QBRecordFSFileCallbacks* callbacks, void *callbackData);

typedef void (*QBRecordFSFileEncryptionErrorCallback)(void *callbackData, int svError);

/**
 * Set decryption plugin.
 *
 * @param[in] file record fs file
 * @param[in] plugin decryption plugin
 * @param[in] callback plugin error handler callback
 * @param[in] callbackData first parameter passed to callback
 */
void QBRecordFSFileSetDecryptionPlugin(QBRecordFSFile* file, SvObject plugin, QBRecordFSFileEncryptionErrorCallback callback, void *callbackData);

/**
 * Set encryption plugin.
 *
 * @param[in] file record fs file
 * @param[in] plugin encryption plugin
 * @param[in] callback plugin error handler callback
 * @param[in] callbackData first parameter passed to callback
 */
void QBRecordFSFileSetEncryptionPlugin(QBRecordFSFile* file, SvObject plugin, QBRecordFSFileEncryptionErrorCallback callback, void *callbackData);

void QBRecordFSFileCancelRead(QBRecordFSFile* file);

/** Closes the file for writing.
 *  The file is no longer being recorded to, even if the underlying recording session is still active.
 */
void QBRecordFSFileDeactivate(QBRecordFSFile* file);

/** Removes the file.
 *  Removes all blocks/events that were used by this file.
 */
void QBRecordFSFileRemove(QBRecordFSFile* file);


QBRecordFSFile* QBRecordFSRootFindFile(const QBRecordFSRoot* root, int id);
QBRecordFSFile* QBRecordFSRootFindFileByUrl(const QBRecordFSRoot* root, SvString url);

typedef struct QBRecordFSFileEventInfo_s  QBRecordFSFileEventInfo;
struct QBRecordFSFileEventInfo_s
{
  int wantedId;     /// -1 when not used
  bool dropDataUntilSeen;

  int presentId;    /// -1 when unknown
  int followingId;  /// -1 when unknown

  bool wantedSeenAsPresent;
  bool wantedSeenAsFollowing;

  SvTime lastChangeTime;
  SvTime lastUpdateTime; /// event info was received, even if it's no new information
  int updateValidTime;
};

struct QBRecordFSFileInfo_s
{
  int  id;
  bool isActive;
  bool isLocked; /// cannot be deleted atm (e.g. it is being read)

  SvContentMetaData  meta;
  SvObject  format;

  int64_t  size;
  double  duration;

  /// Only valid if "isActive" == true
  QBRecordFSFileEventInfo  eventInfo;
};
typedef struct QBRecordFSFileInfo_s  QBRecordFSFileInfo;

void QBRecordFSFileGetInfo(const QBRecordFSFile* file, QBRecordFSFileInfo* info);

SvContentMetaData QBRecordFSFileGetMeta(const QBRecordFSFile* file);
int QBRecordFSFileStoreMeta(const QBRecordFSFile* file);

void QBRecordFSFileForceMeta_(QBRecordFSFile* file, SvContentMetaData meta);

/**
 * Setter for meta field.
 *
 * Uses string cache internally.
 *
 * @param[in] file file handle
 * @param[in] meta metadata to be set
 * @param[in] authenticated @c true if metadata has been authenticated, @false if metadata authentication was not performed
 */
void QBRecordFSFileSetMeta(QBRecordFSFile* file, SvContentMetaData meta, const bool authenticated);

SvContent QBRecordFSCreateContent(const QBRecordFSFile* file);
SvArray QBRecordFSFileGetEvents(const QBRecordFSFile* file);

#if 0
SvArray QBRecordFSFileGetBookmarks(const QBRecordFSFile* file);

int QBRecordFSFileAddBookmark(QBRecordFSFile* file, const char* name, SvObject obj);
int QBRecordFSFileRemoveBookmark(QBRecordFSFile* file, SvObject obj);
SvObject QBRecordFSFileFindBookmark(const QBRecordFSFile* file, const char* name);
#endif

/** Monitor changes in current event id. If \a dropDatauntilSeen is true, then record only when it is equal to given \a eventId.
  */
void QBRecordFSFileSetEventId(QBRecordFSFile* file, int eventId, bool dropDatauntilSeen);

/** Store event by serializing given @a obj, and monitor accurate recording if needed.
 *  The \a obj event will be placed in the stream after last-written block.
 *  @param obj  object to be serialized
 *  @param eventId  for monitoring accurate recording of an event
 *  @param followingId  for monitoring accurate recording of an event
 */
int QBRecordFSFileEventsChanged(QBRecordFSFile* file,
                                SvObject obj,
                                int presentId, int followingId);

int QBRecordFSFileEventsUpdated(QBRecordFSFile* file, int validSeconds);

/** Store format of data that will be written to this session.
 *  @param obj  object to be serialized
 */
int QBRecordFSFileSetFormat(QBRecordFSFile* file, SvObject obj);
SvObject QBRecordFSFileGetFormat(const QBRecordFSFile* file);

/** Schedule next block to be written.
 *  @param sb  this buffer is treated as read-only, will be released after it's written to the disk
 *  @returns  TODO
 */
int QBRecordFSFileWriteBlock(QBRecordFSFile* file, SvBuf sb);

/**
 * @brief Read a block of data from the file, from given pvr block.
 * Always check for valid read range before calling this function.
 * @param[in] file QBRecordFSFile file
 * @param[in] blockNumber block number
 * @param[in,out] sb buffer for data, its size cannot be bigger than pvr block size
 * @returns 1 on success, 0 when the read would block, negative on error (I/O error, or when reading from outside of range).
 */
int QBRecordFSFileReadFromBlock(QBRecordFSFile* file, uint32_t blockNumber, SvBuf sb);

int QBRecordFSFileRewriteChunk(QBRecordFSFile* file, SvBuf sb, uint64_t offset);

int QBRecordFSFileLock_(QBRecordFSFile* file, const char* reason);
void QBRecordFSFileUnlock_(QBRecordFSFile* file, const char* reason);

#if SV_LOG_LEVEL > 0
#  define QBRecordFSFileLock(file, reason) QBRecordFSFileLock_(file, reason)
#  define QBRecordFSFileUnlock(file, reason) QBRecordFSFileUnlock_(file, reason)
#else
#  define QBRecordFSFileLock(file, reason) QBRecordFSFileLock_(file, "")
#  define QBRecordFSFileUnlock(file, reason) QBRecordFSFileUnlock_(file, "")
#endif


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_RECORD_FS_FILE_H
