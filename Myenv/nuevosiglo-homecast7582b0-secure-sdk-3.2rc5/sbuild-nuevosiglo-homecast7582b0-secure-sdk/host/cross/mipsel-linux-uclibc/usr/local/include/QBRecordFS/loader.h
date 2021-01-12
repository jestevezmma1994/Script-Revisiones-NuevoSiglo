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

#ifndef QB_RECORD_FS_LOADER_H
#define QB_RECORD_FS_LOADER_H

/**
 * @file loader.h Record FS loader API
 **/
#include "types.h"

#include <fibers/c/fibers.h>

#include <stdbool.h>
#include <QBAppKit/QBObserver.h>
#include <SvFoundation/SvObject.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBRecordFSLoader_;
typedef struct QBRecordFSLoader_* QBRecordFSLoader;

/**
 * Creates loader object.
 * Blocks \a callbackSched from dead-end, until load/unload job is done.
 * When finished, notify observer.
 *
 * @param[in] callbackSched handle to a scheduler that will run callbacks and observable updates
 * @param[in] authPlgin     auth plugin handle, may be @c NULL
 * @param[in] observer      handle to receiver of notifications. Must implement @ref QBObserver
 * @return newly created instance of QBRecordFSLoader or @c NULL in case of error
 */
QBRecordFSLoader QBRecordFSLoaderCreate(SvScheduler callbackSched, SvObject authPlugin,
                                        SvObject observer);

/**
 * Stops loading, and/or starts unloading, then releases all resources, then notifies observer.
 *
 * @param[in] self          QBRecordFSLoader handle
 */
void QBRecordFSLoaderRequestAutodestruction(QBRecordFSLoader self);

/// Check current state of \a loader.
bool  QBRecordFSLoaderIsBusy(const QBRecordFSLoader self);


/// Starts loading QBRecordFSRoot with given parameters.
void QBRecordFSLoaderLoad(QBRecordFSLoader self,
                          const char *metaRoot,
                          const char *dataRoot);

/// Take ownershif of the QBRecordFSRoot that have been successfully loaded.
/// \returns loaded root, or null when failed
QBRecordFSRoot *QBRecordFSLoaderTakeRoot(QBRecordFSLoader self);

/// Pass ownership of \a root, that will be unloaded when Destroy() is called.
void QBRecordFSLoaderGiveRoot(QBRecordFSLoader self, QBRecordFSRoot *root);

SvType QBRecordFSLoader_getType(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_RECORD_FS_LOADER_H
