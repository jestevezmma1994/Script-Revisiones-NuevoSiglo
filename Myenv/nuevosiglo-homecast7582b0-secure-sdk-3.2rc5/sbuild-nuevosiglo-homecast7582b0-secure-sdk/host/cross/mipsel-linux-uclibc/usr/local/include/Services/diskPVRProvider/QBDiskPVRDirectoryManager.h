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

#ifndef QB_DISK_PVR_DIRECTORY_MANAGER_H_
#define QB_DISK_PVR_DIRECTORY_MANAGER_H_

/**
 * @file QBDiskPVRDirectoryManager.h Disk pvr directory manager file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBDiskPVRDirectory.h"
#include "QBDiskPVRProvider.h"
#include <QBContentManager/QBContentCategory.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <main_decl.h>

#include <stdbool.h>

/**
 * @defgroup QBDiskPVRDirectoryManager Disk pvr directory manager.
 * @ingroup QBDiskPVRProvider
 * @{
 **/

/**
 * QBDiskPVR directory manager.
 *
 * @brief Disk PVR directory manager is responsible for handling directories,
 * this includes creating and removing of them, serializing and deserializing from
 * file, adding and removing recording recordings from them.
 *
 * @class QBDiskPVRDirectoryManager
 * @extends SvObject
 **/
typedef struct QBDiskPVRDirectoryManager_ *QBDiskPVRDirectoryManager;

/**
 * Create disk PVR directory manager.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] provider                  disk pvr provider handle
 * @param[in] parent                    parent category for directories
 * @param[in] addSearchToDirectories    should search node be added to directories
 * @param[in] searchProviderTest        search provider test associated data
 * @param[in] limits                    quantitative limits for directories and recordings count
 * @param[out] errorOut                 error info
 * @return                              created directory manager, @c NULL in case of error
 **/
QBDiskPVRDirectoryManager
QBDiskPVRDirectoryManagerCreate(QBDiskPVRProvider provider,
                                QBContentCategory parent,
                                bool addSearchToDirectories,
                                QBPVRSearchProviderTest searchProviderTest,
                                QBDiskPVRProviderQuantitativeLimits limits,
                                SvErrorInfo *errorOut);

/**
 * Start PVR directory manager.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] self              directory manager handle
 * @param[out] errorOut         error info
 **/
void
QBDiskPVRDirectoryManagerStart(QBDiskPVRDirectoryManager self,
                               SvErrorInfo *errorOut);

/**
 * Stop PVR directory manager.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] self              directory manager handle
* @param[out] errorOut          error info
 **/
void
QBDiskPVRDirectoryManagerStop(QBDiskPVRDirectoryManager self,
                              SvErrorInfo *errorOut);

/**
 * Get directories list.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] self              directory manager handle
 * @return                      directories list
 **/
SvArray
QBDiskPVRDirectoryManagerGetDirectoriesList(QBDiskPVRDirectoryManager self);

/**
 * Create new directory from recording schedule params.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] self              directory manager handle
 * @param[in] params            recording schedule params
 * @param[out] errorOut         error info
 * @return                      created directory, @c NULL in case of error
 **/
QBDiskPVRDirectory
QBDiskPVRDirectoryManagerCreateDirectoryFromRecordingSchedParams(QBDiskPVRDirectoryManager self,
                                                                 QBPVRRecordingSchedParams params,
                                                                 SvErrorInfo *errorOut);

/**
 * Create new directory from directory create params.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] self              directory manager handle
 * @param[in] params            directory create params
 * @param[out] errorOut         error info
 * @return                      created directoryr, @c NULL in case of error
 **/
QBDiskPVRDirectory
QBDiskPVRDirectoryManagerCreateDirectoryFromParams(QBDiskPVRDirectoryManager self,
                                                   QBPVRDirectoryCreateParams params,
                                                   SvErrorInfo *errorOut);

/**
 * Update directory.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] self              directory manager handle
 * @param[in] dir               directory handle
 * @param[in] params            directory update params
 **/
void
QBDiskPVRDirectoryManagerUpdateDirectory(QBDiskPVRDirectoryManager self,
                                         QBDiskPVRDirectory dir,
                                         QBPVRDirectoryUpdateParams params);

/**
 * Stop directory recording.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] self              directory manager handle
 * @param[in] dir               directory handle
 **/
void
QBDiskPVRDirectoryManagerStopDirectory(QBDiskPVRDirectoryManager self,
                                       QBDiskPVRDirectory dir);

/**
 * Delete all from directory.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] self              directory manager handle
 * @param[in] dir               directory handle
 * @param[in] params            directory delete params
 **/
void
QBDiskPVRDirectoryManagerDeleteAllFromDirectory(QBDiskPVRDirectoryManager self,
                                                QBDiskPVRDirectory dir,
                                                QBPVRDirectoryDeleteParams params);

/**
 * Delete directory.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] self              directory manager handle
 * @param[in] dir               directory handle
 * @param[in] params            directory delete params
 **/
void
QBDiskPVRDirectoryManagerDeleteDirectory(QBDiskPVRDirectoryManager self,
                                         QBDiskPVRDirectory dir,
                                         QBPVRDirectoryDeleteParams params);

/**
 * Check directories limit.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] self              directory manager handle
 * @param[in] type              directory type
 * @return                      @c true if limit is reached, @c false otherwise
 **/
bool
QBDiskPVRDirectoryManagerIsDirectoryLimitExceeded(QBDiskPVRDirectoryManager self,
                                                  QBPVRDirectoryType type);

/**
 * Add recording to directory.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] self              directory manager handle
 * @param[in] rec               recording
 * @return                      @c 0 if recording was added to some directory,
 *                              @c <0 otherwise
 **/
int
QBDiskPVRDirectoryManagerAddRecording(QBDiskPVRDirectoryManager self, QBDiskPVRRecording rec);

/**
 * Remove recording from directory.
 *
 * @memberof QBDiskPVRDirectoryManager
 *
 * @param[in] self              directory manager handle
 * @param[in] rec               recording
 * @return                      @c 0 if recording was removed from some directory,
 *                              @c <0 otherwise
 **/
int
QBDiskPVRDirectoryManagerRemoveRecording(QBDiskPVRDirectoryManager self, QBDiskPVRRecording rec);

/**
 * @}
 **/

#endif /* QB_DISK_PVR_DIRECTORY_MANAGER_H_ */
