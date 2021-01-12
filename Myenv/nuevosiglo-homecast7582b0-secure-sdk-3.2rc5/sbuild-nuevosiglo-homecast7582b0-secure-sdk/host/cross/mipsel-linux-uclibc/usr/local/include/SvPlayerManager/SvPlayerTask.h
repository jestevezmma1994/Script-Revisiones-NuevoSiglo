/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_PLAYER_TASK_H_
#define SV_PLAYER_TASK_H_

/**
 * @file SvPlayerTask.h Player task class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvType.h>

#include <SvPlayerControllers/SvPlayerTaskControllers.h>
#include <SvPlayerManager/SvPlayerTaskState.h>
#include <SvPlayerManager/SvPlayerTaskCapabilities.h>
#include <SvPlayerManager/SvPlayerTaskRestrictions.h>
#include <SvPlayerKit/SvContent.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvPlayerTask Player task class
 * @ingroup SvPlayerManagerLibrary
 * @{
 *
 * A class representing single media playback instance.
 **/

/**
 * Player task class.
 * @class SvPlayerTask SvPlayerTask.h <SvPlayerManager/SvPlayerTask.h>
 **/
typedef struct SvPlayerTask_ *SvPlayerTask;

/**
 * Get runtime type identification object representing player task class.
 *
 * @return SvPlayerTask type identification object
 **/
extern SvType
SvPlayerTask_getType(void);

/**
 * Start/pause playback.
 *
 * This method requests change in playback speed and/or position.
 * Media player supports only a limited set of speeds and may also
 * have some limitations on the capability to seek through the content.
 * Caller should always check what is the real speed and position
 * after change by listening on SvPlayerTaskListener::stateChanged()
 * notifications.
 *
 * @param[in] self      player task handle
 * @param[in] speed     desired playback speed: @c 1.0 for normal playback, @c 0.0 to pause,
 *                      less than @c 0.0 for reverse playback, more than @c 1.0 for fast forwarding
 * @param[in] position  desired playback position (use @c -1.0 for default position; current, if already playing)
 * @param[out] errorOut error info
 **/
extern void
SvPlayerTaskPlay(SvPlayerTask self,
                 double speed,
                 double position,
                 SvErrorInfo *errorOut);

/**
 * Enable or disable timeshift recording.
 *
 * @param[in] self      player task handle
 * @param[in] enable    @c true to enable timeshift recording,
 *                      @c false to disable
 * @return              @c 0 on success, any other value in case of error
 **/
extern int
SvPlayerTaskEnableTimeshiftRecording(SvPlayerTask self,
                                     bool enable);

/**
 * Start playing back from timeshift buffer, or go back to play real-time.
 *
 * @param[in] self      player task handle
 * @param[in] enable    @c true to start timeshift playback,
 *                      @c false to go back to real-time playback
 * @return              @c 0 on success, any other value in case of error
 **/
extern int
SvPlayerTaskEnableTimeshiftPlayback(SvPlayerTask self,
                                    bool enable);

/**
 * Release all resources and unlink from the underlying media player subsystem.
 * Multiple calls of this method on same task object are harmless.
 *
 * @note After calling this method, the only other valid methods that can be
 * called are SvPlayerTaskGetState() and SvPlayerTaskCapabilities().
 *
 * @param[in] self player task handle
 * @param[out] errorOut error info
 **/
extern void
SvPlayerTaskClose(SvPlayerTask self,
                  SvErrorInfo *errorOut);

/**
 * Get player task capabilities.
 *
 * @param[in] self player task handle
 * @return capabilities of the player task
 **/
extern SvPlayerTaskCapabilities
SvPlayerTaskGetCapabilities(SvPlayerTask self);

/**
 * Get current state of player task.
 *
 * @param[in] self player task handle
 * @return current state of player task
 **/
extern SvPlayerTaskState
SvPlayerTaskGetState(SvPlayerTask self);

/**
 * Pass parameter to/from the underlying media player subsystem.
 *
 * @param[in] self  player task handle
 * @param[in] name  name of the option
 * @param[in,out] p value of the option
 * @return          @c 0 on success, @c SV_ERR_INVALID when ignored,
 *                  any other value is a fatal error
 *
 * @internal
 **/
extern int
SvPlayerTaskSetOpt(SvPlayerTask self,
                   const char *name,
                   void *p);

/**
 * Set player restrictions.
 *
 * This method will set and apply restrictions for current playback.
 *
 * @param[in] self          player task handle
 * @param[in] restrictions  @see SvPlayerTaskRestrictions
 * @param[out] errorOut     error info
 **/
extern void
SvPlayerTaskSetRestrictions(SvPlayerTask self,
                            SvPlayerTaskRestrictions *restrictions,
                            SvErrorInfo *errorOut);

/**
 * Get content of player task.
 *
 * @param[in] self player task handle
 * @return content of player task
 **/
extern SvContent
SvPlayerTaskGetContent(SvPlayerTask self);

/**
 * Gets player task metadata
 *
 * @param[in] self player task
 * @return metadata @c NULL in case if there is no metadata for that session
 */
extern SvContentMetaData
SvPlayerTaskGetMetaData(SvPlayerTask self);

/**
 * Get player task controllers
 *
 * @param[in] self player task handle
 * @return player task controllers
 */
extern SvPlayerTaskControllers
SvPlayerTaskGetControllers(SvPlayerTask self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
