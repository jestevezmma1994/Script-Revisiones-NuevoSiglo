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

#ifndef QBIPSTREAMCLIENT_QBHLSMANAGER_H_
#define QBIPSTREAMCLIENT_QBHLSMANAGER_H_

/**
 * @file QBHLSManager.h HLS manager
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <fibers/c/fibers.h>
#include <SvFoundation/SvInterface.h>
#include <SvPlayerKit/SvPlayerConfig.h>
#include <QBIPStreamClient/HLS/M3U8/QBM3U8MediaSegment.h>
#include <QBIPStreamClient/Common/QBIPStreamDownloaderListener.h>

/**
 * @defgroup QBHLSManager HLS manager
 * @{
 * High-level representation of HLS manager
 **/

/**
 * QBHLSManagerStreamType.
 * Specifies track of a HLS stream.
 */
typedef enum {
    QBHLSManagerStreamType_audio = 0,      //!< buffer manager audio stream type
    QBHLSManagerStreamType_video,          //!< video stream type
    QBHLSManagerStreamType_cnt             //!< count of stream types
} QBHLSManagerStreamType;

/**
 * QBHLSManagerErrorType
 * Specifies types of QBHLSManager's errors
 */
typedef enum {
    QBHLSManagerErrorType_nextChunkDownload,           //!< error on downloading next chunk
    QBHLSManagerErrorType_nextChunkChoosing,           //!< error on choosing next chunk to download
    QBHLSManagerErrorType_manifestManager,             //!< manifest manager error
    QBHLSManagerErrorType_newManifestHandling,         //!< error on handling new manifest
} QBHLSManagerErrorType;

/**
 * QBHLSManagerPlaybackState.
 * Specifies current state of HLS stream playback.
 */
typedef enum {
    QBHLSManagerPlaybackState_unknown = 0,     //!< unknown playback state
    QBHLSManagerPlaybackState_paused,          //!< paused
    QBHLSManagerPlaybackState_playing,         //!< playing
    QBHLSManagerPlaybackState_discontinuity,   //!< discontinuity detected
    QBHLSManagerPlaybackState_EOS,             //!< end of stream detected
} QBHLSManagerPlaybackState;

/**
 * QBHLSManager.
 * @class QBHLSManager
 * @extends SvObject
 */
typedef struct QBHLSManager_ *QBHLSManager;

/**
 * Create an instance of QBHLSManager class.
 *
 * @param[in] scheduler             application scheduler handle
 * @param[in] bufMan                buffer manager handle
 * @param[in] httpClientEngine      http client engine handle
 * @param[in] playerConfig          player config handle
 * @param[in] manifestURL           URL of HLS manifest
 * @param[out] errorOut             errorInfo
 * @returns QBHLSManager class instance handle
 **/
QBHLSManager
QBHLSManagerCreate(SvScheduler scheduler,
                   SvBufMan bufMan,
                   SvHTTPClientEngine httpClientEngine,
                   SvPlayerConfig playerConfig,
                   SvURL manifestURL,
                   SvErrorInfo *errorOut);

/**
 * Start QBHLSManager.
 *
 * The function must be called before calling QBHLSManagerPlay.
 *
 * @param[in] self                  QBHLSManager class instance handle
 * @param[out] errorOut             errorInfo
 **/
void
QBHLSManagerStart(QBHLSManager self,
                  SvErrorInfo *errorOut);

/**
 * Stop QBHLSManager.
 *
 * @param[in] self                  QBHLSManager class instance handle
 * @param[out] errorOut             errorInfo
 **/
void
QBHLSManagerStop(QBHLSManager self, SvErrorInfo *errorOut);


/**
 * Start playback on QBHLSManager.
 *
 * @param[in] self                  QBHLSManager class instance handle
 * @param[in] speed                 desired playback speed
 * @param[in] position              initial playback position
 * @param[out] errorOut             errorInfo
 **/
void
QBHLSManagerPlay(QBHLSManager self,
                 double speed,
                 double position,
                 SvErrorInfo *errorOut);

/**
 * Add listener of QBHLSManager.
 *
 * Listener object must implement QBHLSManagerListener interface.
 *
 * @param[in] self                  QBHLSManager class instance handle
 * @param[in] listener              listener object handle
 * @param[out] errorOut             errorInfo
 **/
void
QBHLSManagerAddListener(QBHLSManager self,
                        SvObject listener,
                        SvErrorInfo *errorOut);

/**
 * Remove listener of QBHLSManager.
 *
 * @param[in] self                  QBHLSManager class instance handle
 * @param[in] listener              listener object handle
 * @param[out] errorOut             errorInfo
 **/
void
QBHLSManagerRemoveListener(QBHLSManager self,
                           SvObject listener,
                           SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_QBHLSMANAGER_H_ */
