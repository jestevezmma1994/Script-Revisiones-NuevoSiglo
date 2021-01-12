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

#ifndef QB_DISK_PVR_DIRECTORY_H_
#define QB_DISK_PVR_DIRECTORY_H_

/**
 * @file QBDiskPVRDirectory.h QBDiskPVRDirectory.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBDiskPVRRecording.h"

#include <QBPVRDirectoryInternal.h>
#include <QBPVRDirectory.h>
#include <QBPVRTypes.h>
#include <QBContentManager/QBContentCategory.h>
#include <SvFoundation/SvHashTable.h>
#include <SvCore/SvErrorInfo.h>

#include <stdbool.h>

/**
 * @defgroup QBDiskPVRDirectory Disk pvr directory.
 * @ingroup QBDiskPVRProvider
 * @{
 **/

/**
 * QBDiskPVRDirectory init params.
 **/
typedef struct QBDiskPVRDirectoryInitParams_ {
    struct QBPVRDirectoryInitParams_ super_;       ///< super class
    int days;                                      ///< days bitmask
} *QBDiskPVRDirectoryInitParams;

/**
 * QBDiskPVR directory.
 * @class QBDiskPVRDirectory
 * @extends QBPVRDirectory
 **/
typedef struct QBDiskPVRDirectory_ *QBDiskPVRDirectory;

/**
 * Create disk PVR directory from description in hash table.
 *
 * @memberof QBDiskPVRDirectory
 *
 * @param[in] parent                    parent content category
 * @param[in] desc                      hash table with description
 * @param[in] addSearch                 should search node be added
 * @param[in] searchProviderTest        search provider test associated data
 * @param[out] errorOut                 error info
 * @return                              created directory, @c NULL in case of error
 **/
QBDiskPVRDirectory
QBDiskPVRDirectoryCreateWithDesc(QBContentCategory parent,
                                 SvHashTable desc,
                                 bool addSearch,
                                 QBPVRSearchProviderTest searchProviderTest,
                                 SvErrorInfo *errorOut);

/**
 * Serialize disk PVR directory.
 *
 * @memberof QBDiskPVRDirectory
 *
 * @param[in] self              disk PVR directory handle
 * @param[in] desc              hash table to which serialize
 * @param[out] errorOut         error info
 **/
void
QBDiskPVRDirectorySerialize(QBDiskPVRDirectory self, SvHashTable desc, SvErrorInfo *errorOut);

/**
 * Create disk PVR directory from params.
 *
 * @memberof QBDiskPVRDirectory
 *
 * @param[in] params            parameters with which it should be created
 * @param[out] errorOut         error info
 * @return                      created directory, @c NULL in case of error
 **/
QBDiskPVRDirectory
QBDiskPVRDirectoryCreate(QBDiskPVRDirectoryInitParams params,
                         SvErrorInfo *errorOut);

/**
 * Start disk PVR directory.
 *
 * @memberof QBDiskPVRDirectory
 *
 * @param[in] self              disk PVR directory handle
 * @param[out] errorOut         error info
 **/
void
QBDiskPVRDirectoryStart(QBDiskPVRDirectory self,
                        SvErrorInfo *errorOut);

/**
 * Stop disk PVR directory.
 *
 * @memberof QBDiskPVRDirectory
 *
 * @param[in] self              disk PVR directory handle
 * @param[out] errorOut         error info
 **/
void
QBDiskPVRDirectoryStop(QBDiskPVRDirectory self,
                       SvErrorInfo *errorOut);

/**
 * Stop recording disk PVR directory.
 *
 * @memberof QBDiskPVRDirectory
 *
 * This action can't be undone,
 * after that directory becomes normal directory.
 *
 * @param[in] self              disk PVR directory handle
 **/
void
QBDiskPVRDirectoryStopRecording(QBDiskPVRDirectory self);

/**
 * Update disk PVR directory.
 *
 * @memberof QBDiskPVRDirectory
 *
 * @param[in] self              disk PVR directory handle
 * @param[in] params            update parameters
 **/
void
QBDiskPVRDirectoryUpdate(QBDiskPVRDirectory self,
                         QBPVRDirectoryUpdateParams params);

/**
 * Add recording to disk PVR directory.
 *
 * @memberof QBDiskPVRDirectory
 *
 * @param[in] self              disk PVR directory handle
 * @param[in] rec               recording
 **/
void
QBDiskPVRDirectoryAddRecording(QBDiskPVRDirectory self,
                               QBDiskPVRRecording rec);

/**
 * Remove recording from disk PVR directory.
 *
 * @memberof QBDiskPVRDirectory
 *
 * @param[in] self              disk PVR directory handle
 * @param[in] rec               recording
 **/
void
QBDiskPVRDirectoryRemoveRecording(QBDiskPVRDirectory self,
                                  QBDiskPVRRecording rec);

/**
 * Blacklist recording in PVR directory.
 *
 * @memberof QBDiskPVRDirectory
 *
 * Recording blacklisting prevents recording being added again when we've
 * deleted scheduled recording
 *
 * @param[in] self              disk PVR directory handle
 * @param[in] rec               recording
 **/
void
QBDiskPVRKeywordDirectoryBlacklistRecording(QBDiskPVRDirectory self,
                                            QBDiskPVRRecording rec);

/**
 * Check if event is matching keyword recording.
 *
 * @param[in] self                  disk PVR directory handle
 * @param[in] event                 EPG event handle
 * @return                          @c true when event matches keyword recording, @c false otherwise
 **/
bool
QBDiskPVRDirectoryMatchKeyword(QBDiskPVRDirectory self,
                               SvEPGEvent event);

/**
 * Check if event is matching repeated recording.
 *
 * @param[in] self                  disk PVR directory handle
 * @param[in] event                 EPG event handle
 * @return                          @c true when event is repeated recording, @c false otherwise
 **/
bool
QBDiskPVRDirectoryMatchRepeated(QBDiskPVRDirectory self,
                                SvEPGEvent event);

/**
 * @}
 **/

#endif /* QB_DISK_PVR_DIRECTORY_H_ */
