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

#ifndef QB_RECORD_FS_SESSION_H
#define QB_RECORD_FS_SESSION_H

#include "types.h"

#include <QBRecordFS/QBRecordFSDataEncryptionPlugin.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvArray.h>
#include <SvPlayerKit/SvBuf.h>
#include <dataformat/sv_pvr_block_header.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern SvType QBRecordFSSession_getType(void);

typedef struct {
    void (*read_finished)(void *callbackData, SvBuf sb, int svError);
} QBRecordFSSessionCallbacks;

QBRecordFSSession* QBRecordFSSessionCreate(QBRecordFSRoot* root);
void QBRecordFSSessionSetCallbacks(QBRecordFSSession* session, const QBRecordFSSessionCallbacks* callbacks, void *callbackData);
void QBRecordFSSessionDestroy(QBRecordFSSession* session);

typedef void (*QBRecordFSSessionEncryptionErrorCallback)(void *callbackData, int svError);
/**
 * Set encryption plugin.
 * This method should be called only once.
 *
 * @param[in] session record fs session
 * @param[in] plugin encryption plugin
 * @param[in] callback plugin error handler callback
 * @param[in] callbackData first parameter passed to callback
 */
void QBRecordFSSessionSetEncryptionPlugin(QBRecordFSSession *session, SvObject plugin, QBRecordFSSessionEncryptionErrorCallback callback, void *callbackData);

struct QBRecordFSSessionInfo_s
{
  int64_t  minOffset;
  int64_t  maxOffset;
};
typedef struct QBRecordFSSessionInfo_s  QBRecordFSSessionInfo;

void QBRecordFSSessionGetInfo(const QBRecordFSSession* session, QBRecordFSSessionInfo* info);

/** Schedule next block to be written.
 *  @param sb  this buffer is treated as read-only, will be released after it's written to the disk
 *  @returns  TODO
 */
int QBRecordFSSessionWriteBlock(QBRecordFSSession* session, SvBuf sb);

/**
 * @brief Read a block of data from the session, from given pvr block.
 * Always check for valid read range before calling this function.
 * @param[in] session QBRecordFSSession session
 * @param[in] blockNumber block number
 * @param[in,out] sb buffer for data, its size cannot be bigger than pvr block size
 * @returns 1 on success, 0 when the read would block, negative on error (I/O error, or when reading from outside of range).
 */
int QBRecordFSSessionReadFromBlock(QBRecordFSSession* session, uint32_t blockNumber, SvBuf sb);

void QBRecordFSSessionCancelRead(QBRecordFSSession* session);

int QBRecordFSSessionReadBorderPacketTime(QBRecordFSSession* session,
                                          struct sv_pvr_time* first_packet_time,
                                          struct sv_pvr_time* start_packet_time,
                                          struct sv_pvr_time* end_packet_time);

int QBRecordFSSessionGetEraseBlockSize(QBRecordFSSession* session);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_RECORD_FS_SESSION_H
