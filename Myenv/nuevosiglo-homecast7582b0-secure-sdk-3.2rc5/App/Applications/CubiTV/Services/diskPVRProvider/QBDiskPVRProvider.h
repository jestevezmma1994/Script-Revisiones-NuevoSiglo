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

#ifndef QB_DISK_PVR_PROVIDER_H_
#define QB_DISK_PVR_PROVIDER_H_

/**
 * @file QBDiskPVRProvider.h Disk PVR provider file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBPVRProvider.h>
#include <QBSchedManager.h>
#include <QBContentManager/QBContentTree.h>
#include <QBRecordFS/types.h>
#include <SvCore/SvErrorInfo.h>
#include <main_decl.h>

/**
 * @defgroup QBDiskPVRProvider QBDiskPVRProvider: Disk PVR provider
 * @{
 **/

/**
 * Disk PVR channel cost classes.
 */
typedef enum {
    QBDiskPVRProviderChannelCostClass_SDTV,         ///< standard definition TV channel cost class
    QBDiskPVRProviderChannelCostClass_HDTV,         ///< high definition TV channel cost class
    QBDiskPVRProviderChannelCostClass_radio,        ///< radio channel cost class
} QBDiskPVRProviderChannelCostClass;

/**
 * QBDiskPVR provider.
 *
 * QBDiskPVR provider is PVR provider that uses as a storage for recordings
 * storage drive connected to stb.
 *
 * @class QBDiskPVRProvider
 * @extends QBPVRProvider
 **/
typedef struct QBDiskPVRProvider_ *QBDiskPVRProvider;

/**
 * QBDiskPVRProvider application callbacks.
 **/
typedef QBSchedManagerAppCallbacks QBDiskPVRProviderAppCallbacks;

/**
 * PVR quantitative limits.
 */
typedef struct QBDiskPVRProviderQuantitativeLimits_ {
    int maxNormalDirsCnt;        ///< max. count of normal directories, set as -1 to use default
    int maxKeywordsDirsCnt;      ///< max. count of keyword recordings' directories, set as -1 to use default
    int maxDirsCnt;              ///< max. count of all directories, set as -1 to use default
    int maxRecordingsCnt;        ///< max. count of PVR recordings of all types, set as -1 to use default
} QBDiskPVRProviderQuantitativeLimits;

/**
 * QBDiskPVRProvider params.
 **/
typedef struct QBDiskPVRProviderParams_ {
    QBPVRProviderParams super_;         ///< PVR provider parameters;
    size_t tunerCnt;                    ///< tuner count
    QBDiskPVRProviderQuantitativeLimits limits; ///< limits of directories and recordings count
} QBDiskPVRProviderParams;

/**
 * Create disk PVR provider.
 *
 * @memberof QBDiskPVRProvider
 *
 * @param[in] appGlobals        application globals variables
 * @param[in] scheduler         scheduler
 * @param[in] params            parameters needed for disk PVR provider creation
 * @param[out] errorOut         error info
 * @return                      created provider, @c NULL in case of error
 **/
QBDiskPVRProvider
QBDiskPVRProviderCreate(AppGlobals appGlobals,
                        SvScheduler scheduler,
                        QBDiskPVRProviderParams params,
                        SvErrorInfo *errorOut);

/**
 * Set storage of disk PVR provider.
 *
 * @memberof QBDiskPVRProvider
 *
 * @param[in] self      disk PVR provider handle
 * @param[in] root      record storage root
 **/
void
QBDiskPVRProviderSetStorage(QBDiskPVRProvider self,
                            QBRecordFSRoot *root);

/**
 * Register app callbacks.
 *
 * @memberof QBDiskPVRProvider
 *
 * @param[in] self      disk PVR provider handle
 * @param[in] target    callbacks target
 * @param[in] callbacks callbacks
 **/
void
QBDiskPVRProviderRegisterAppCallbacks(QBDiskPVRProvider self,
                                      void *target,
                                      const QBDiskPVRProviderAppCallbacks* callbacks);

/**
 * Unreserve tuner.
 *
 * @memberof QBDiskPVRProvider
 *
 * @param[in] self              disk PVR provider handle
 * @param[in] tunerParams       tuner params
 **/
void
QBDiskPVRProviderUnreserveTuner(QBDiskPVRProvider self,
                                struct QBTunerParams *tunerParams);

/**
 * Set total cost of simultaneous recordings
 *
 * @memberof QBDiskPVRProvider
 *
 * @param[in] self              disk PVR provider handle
 * @param[in] totalCost         total cost of simultaneous recordings
 **/
void
QBDiskPVRProviderSetRecordingsLimit(QBDiskPVRProvider self, size_t totalCost);

/**
 * Set recording simultaneous session limit.
 *
 * @memberof QBDiskPVRProvider
 *
 * @param[in] self              disk PVR provider handle
 * @param[in] cnt               recordings count
 **/
void
QBDiskPVRProviderSetRecordingSessionsLimit(QBDiskPVRProvider self, size_t cnt);

/**
 * Get mux id of a recording.
 *
 * @memberof QBDiskPVRProvider
 *
 * @param[in] self              disk PVR provider handle
 * @param[in] rec               recording
 * @return                      mux id
 **/
const struct QBTunerMuxId *
QBDiskPVRProviderGetRecordingsMuxId(QBDiskPVRProvider self, QBPVRRecording rec);

/**
 * @}
 **/

#endif /* QB_DISK_PVR_PROVIDER_H_ */
