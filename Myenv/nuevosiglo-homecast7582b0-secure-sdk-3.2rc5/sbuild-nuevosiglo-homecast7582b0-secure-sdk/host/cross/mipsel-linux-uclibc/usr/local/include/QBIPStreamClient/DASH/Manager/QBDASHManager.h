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

#ifndef QBIPSTREAMCLIENT_QBDASHMANAGER_H_
#define QBIPSTREAMCLIENT_QBDASHMANAGER_H_

/**
 * @file QBDASHManager.h MPEG-DASH Manager
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvPlayerControllers/SvPlayerTrackController.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvPlayerKit/SvBufMan.h>
#include <SvPlayerKit/SvPlayerConfig.h>

#include <SvFoundation/SvURL.h>

#include <SvCore/SvTime.h>
#include <SvCore/SvErrorInfo.h>

#include <fibers/c/fibers.h>

#include <stdbool.h>

/**
 * @defgroup QBDASHManager MPEG-DASH Manager
 * @ingroup QBIPStreamDASHSupport
 * @{
 * High-level representation of MPEG-DASH Manager
 *
 **/

/**
 * QBDASHManager constructor parameters
 * @struct QBDASHManagerParams
 */
typedef struct QBDASHManagerParams {
    SvScheduler scheduler;               //!< application scheduler handle
    SvBufMan bufMan;                     //!< buffer manager handle
    SvHTTPClientEngine httpClientEngine; //!< HTTP client engine handle
    SvPlayerConfig playerConfig;         //!< player config provider handle
    SvObject playbackInfoProvider;       //!< playback info provider handle
    SvURL manifestURL;                   //!< main DASH manifest URL
} QBDASHManagerParams;

/**
 * QBDASHManager.
 * @class QBDASHManager
 * @extends SvObject
 */
typedef struct QBDASHManager_ *QBDASHManager;


/**
 * Create an instance of QBDASHManager class.
 *
 * @param[in]  params            constructor parameters
 * @param[out] errorOut          errorInfo
 * @returns     QBDASHManager class instance handle
 **/
QBDASHManager
QBDASHManagerCreate(QBDASHManagerParams* params,
                    SvErrorInfo *errorOut);

/**
 * Start QBDASHManager.
 * The function must be called before calling QBDASHManagerPlay.
 *
 * @param[in] self                  QBDASHManager class instance handle
 * @param[out] errorOut             errorInfo
 **/
void
QBDASHManagerStart(QBDASHManager self, SvErrorInfo *errorOut);

/**
 * Stops QBDASHManager.
 *
 * @param[in] self                  QBDASHManager class instance handle
 * @param[out] errorOut             errorInfo
 **/
void
QBDASHManagerStop(QBDASHManager self, SvErrorInfo *errorOut);

/**
 * Start playback on QBDASHManager.
 *
 * @param[in] self                  QBDASHManager class instance handle
 * @param[in] speed                 desired playback speed
 * @param[in] position              initial playback position
 * @param[in] timeshift             indicate if timeshift has been enabled
 * @param[out] errorOut             errorInfo
 **/
void
QBDASHManagerPlay(QBDASHManager self,
                  double speed,
                  double position,
                  bool timeshift,
                  SvErrorInfo *errorOut);

/**
 * Get current playback position.
 *
 * @param[in] self  QBDASHManager class instance handle
 * @return          playback position in seconds
 **/
double
QBDASHManagerGetPlaybackPosition(QBDASHManager self);

/**
 * Check if timeshift is enabled.
 *
 * @param[in] self QBDASHManager class instance handle
 * @return         @c true if timeshift is enabled, otherwise @c false
 **/
bool
QBDASHManagerHasTimeshiftEnabled(QBDASHManager self);

/**
 * Get timeshift range.
 *
 * Timeshift range has following values:
 * 1. Start postion is timestamp from first available segment.
 * 2. End position is timestamp from last available segment minus value of
 * minBufferTime attribute.
 *
 * @param[in] self            QBDASHManager class instance handle
 * @param[out] timeshiftStart timeshift start time
 * @param[out] timeshiftEnd   timeshift end time
 **/
void
QBDASHManagerGetTimeshiftRange(QBDASHManager self,
                               double *timeshiftStart,
                               double *timeshiftEnd);

/**
 * Add listener of QBDASHManager.
 * Listener object must implement QBIPStreamManagerListener interface.
 *
 * @param[in] self                  QBDASHManager class instance handle
 * @param[in] listener              listener object handle
 * @param[out] errorOut             errorInfo
 **/
void
QBDASHManagerAddListener(QBDASHManager self,
                         SvObject listener,
                         SvErrorInfo *errorOut);

/**
 * Remove listener of QBDASHManager.
 *
 * @param[in] self                  QBDASHManager class instance handle
 * @param[in] listener              listener object handle
 * @param[out] errorOut             errorInfo
 **/
void
QBDASHManagerRemoveListener(QBDASHManager self,
                            SvObject listener,
                            SvErrorInfo *errorOut);

/**
 * Add frames listener of QBDASHManager.
 * Listener object must implement QBIPStreamFramesListener interface.
 *
 * @param[in] self                  QBDASHManager class instance handle
 * @param[in] listener              listener object handle
 * @param[out] errorOut             errorInfo
 **/
void
QBDASHManagerAddFramesListener(QBDASHManager self,
                               SvObject listener,
                               SvErrorInfo *errorOut);

/**
 * Remove frames listener of QBDASHManager.
 *
 * @param[in] self                  QBDASHManager class instance handle
 * @param[in] listener              listener object handle
 * @param[out] errorOut             errorInfo
 **/
void
QBDASHManagerRemoveFramesListener(QBDASHManager self,
                                  SvObject listener,
                                  SvErrorInfo *errorOut);

/**
 * Set audio and video controllers.
 *
 * DASH Manager uses controllers to listen on user track changes
 *
 * @param[in] self                  QBDASHManager class instance handle
 * @param[in] videoController       video tracks controller
 * @param[in] audioController       audio tracks controller
 */
void
QBDASHManagerSetTrackControllers(QBDASHManager self, SvPlayerTrackController videoController, SvPlayerTrackController audioController);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_QBDASHMANAGER_H_ */
