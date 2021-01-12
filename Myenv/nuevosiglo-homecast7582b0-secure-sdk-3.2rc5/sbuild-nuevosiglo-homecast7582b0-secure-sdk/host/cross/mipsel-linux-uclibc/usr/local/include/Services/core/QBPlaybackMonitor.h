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
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#ifndef QB_PLAYBACK_MONITOR_H_
#define QB_PLAYBACK_MONITOR_H_

/**
 * @file QBPlaybackMonitor.h Playback Monitor Service class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvPlayerManager/SvPlayerTask.h>
#include <Services/core/volume.h>

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvData.h>
#include <SvFoundation/SvString.h>
#include <fibers/c/fibers.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>

#include <stdbool.h>

/**
* @defgroup PlaybackMonitor - Service for monitoring playback events
* @ingroup CubiTV_services
* @{
*
* This service is responsible for monitoring playback events.
*
**/

/**
* This service is responsible for  monitoring playback events.
*
* @class QBPlaybackMonitor
*
**/
typedef struct QBPlaybackMonitor_ *QBPlaybackMonitor;

/**
 * Create playback monitor service.
 *
 * @param[in] volumeService             handle to QBVolume object
 * @param[out] errorOut                 error information
 *
 * @return                              returns created QBPlaybackMonitor, @c NULL in case of error
 **/
QBPlaybackMonitor QBPlaybackMonitorCreate(QBVolume volumeService, SvErrorInfo *errorOut);

/**
 * Playback monitoring is working.
 *
 * @param[in] self                      handle to QBPlaybackMonitor object
 *
 * @return                              true if it is working, false otherwise
 **/
bool QBPlaybackMonitorIsWorking(QBPlaybackMonitor self);

/**
 * Start monitoring playback task.
 *
 * @param[in] self                      handle to QBPlaybackMonitor object
 * @param[in] playerTask                handle to SvPlayerTask object
 * @param[in] eventModuleName           module name of logged event
 * @param[in] eventSource               source of log event
 * @param[in] eventData                 playback data to send in long
 * @param[out] errorOut                 error info
 *
 * @return                              true on success, false otherwise
 **/
bool QBPlaybackMonitorPlaybackStarted(QBPlaybackMonitor self, SvPlayerTask playerTask, SvString eventModuleName,
                                      SvString eventSource, SvString eventData, SvErrorInfo *errorOut);

/**
 * Stop monitoring playback task.
 *
 * @param[in] self                       handle to QBPlaybackMonitor object
 * @param[out] duration                  duration of playback (in secods)
 * @param[out] errorOut                  error info
 *
 * @return                               true on success, false otherwise
 **/
bool QBPlaybackMonitorPlaybackStoped(QBPlaybackMonitor self, int64_t *duration, SvErrorInfo *errorOut);

/**
 * Change mute status information.
 *
 * @param[in] self                       handle to QBPlaybackMonitor object
 * @param[in] changedToMute              tells whether sound will be muted or not
 *
 **/
void QBPlaybackMonitorVolumeUpdate(QBPlaybackMonitor self, bool changedToMute);

/**
 * Handler for events from player.
 *
 * @param[in] self                       handle to QBPlaybackMonitor object
 * @param[in] name                       name of the event
 * @param[in] arg                        pointer to event data
 * @param[out] errorOut                  error info
 *
 **/
void QBPlaybackMonitorPlayerEventHandler(QBPlaybackMonitor self, SvString name, void *arg, SvErrorInfo *errorOut);

/**
* @}
**/

#endif /* QB_PLAYBACK_MONITOR_H_ */

