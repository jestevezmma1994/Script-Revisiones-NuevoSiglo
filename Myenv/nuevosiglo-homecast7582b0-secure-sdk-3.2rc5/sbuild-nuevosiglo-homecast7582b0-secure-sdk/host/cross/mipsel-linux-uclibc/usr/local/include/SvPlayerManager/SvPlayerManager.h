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

#ifndef SV_PLAYER_MANAGER_H_
#define SV_PLAYER_MANAGER_H_

/**
 * @file SvPlayerManager.h Player manager library API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvPlayerKit/SvContent.h>
#include <SvPlayerKit/SvContentMetaData.h>
#include <SvPlayerKit/SvPlayerConfig.h>

#include <SvPlayerManager/SvPVRTask.h>
#include <SvPlayerManager/SvPlayerTask.h>

#include <QBCAS.h>

struct QBViewport_s;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvPlayerManagerLibrary SvPlayerManager: player manager library
 * @ingroup MediaPlayer
 *
 * SvPlayerManager library provides an interface for managing player/PVR tasks.
 *
 * SvPlayerManager is a singleton class that keeps track of all player/PVR tasks.
 * A task object represents a single running, controllable playback/PVR instance,
 * implemented by the lower level media player subsystem.
 *
 * Separate tasks can share resources, if possible (most notably, input data sources).
 * The manager's sole purpose is to connect tasks that can share resources. Every time a new task
 * is being created, it can use a brand new resource, or share a resource with some existing task(s).
 *
 * Tasks are SvObjects, so they should be SVRETAIN()'ed and SVRELEASE()'d appriopiately.
 * Additionally, each task has its own close() method, which should be called to make the task release its
 * resources immediately. This will be done automatically in case of autodestruction (after enough
 * SVRELEASE()s, when the reference counter of the task drops to @c 0),
 * but if the application layer knows that the task should be closed at a certain time, it should do so explicitly.
 * The application layer should avoid relying on automatic destruction to perform resource cleanup. This is because
 * the automatic destruction might be launched at any time in the future, not immediately, thus disallowing
 * resource reusage by the application, immediately following SVRELEASE() of the task.
 **/


/**
 * @defgroup SvPlayerManager Player manager class
 * @ingroup SvPlayerManagerLibrary
 * @{
 *
 * A singleton class that manages all player and PVR tasks.
 **/

/**
 * Player manager class.
 *
 * @note This is not an SvObject!
 **/
typedef struct SvPlayerManager_ *SvPlayerManager;


/**
 * Get a handle to the player manager singleton.
 *
 * @return SvPlayerManager handle
 **/
extern SvPlayerManager
SvPlayerManagerGetInstance(void);

/**
 * Set input mode to reencryption if enabled or to default otherwise
 *
 * @param[in] self     player manager handle
 * @param[in] enable   true if reencryption should be enabled
 *
 **/
extern void
SvPlayerManagerSetReencryption(SvPlayerManager self,
                               bool enable);

/**
 * Set up player configuration. If it is not set the default configuration is used.
 * @param[in] self          player manager handle
 * @param[in] playerConfig  player configuration handle
 */
extern void
SvPlayerManagerSetPlayerConfig(SvPlayerManager self,
                               SvPlayerConfig playerConfig);

/**
 * Destroy player manager singleton.
 **/
extern void
SvPlayerManagerCleanup(void);

/**
 * Get player task associated with requested session identifier.
 *
 * @param[in] self      player manager handle
 * @param[in] sessionId requested session identifier, pass @c -1 for any
 * @return player task or @c NULL in case if there is no player task associated with requested session
 */
extern SvPlayerTask
SvPlayerManagerGetPlayerTask(SvPlayerManager self,
                             const int8_t sessionId);

/**
 * Get PVR task associated with requested session identifier.
 *
 * @param[in] self      player manager handle
 * @param[in] sessionId requested session identifier
 * @return PVR task or @c NULL in case if there is no PVR task associated with requested session
 */
extern SvPVRTask
SvPlayerManagerGetPVRTask(SvPlayerManager self,
                          int8_t sessionId);

/**
 * Check if session is connected with task and provide linked session id
 * If sessionId is playback session then linked session is reencryption session from the same task (if reencryption is active).
 * If sessionId is reencryption session then linked session is playback session from the same task (if playback is active on this task)
 *
 * @param[in] task should be SvPlayerTask or SvPVRTask
 * @param[in] sessionId provided by startDecryption QBCAS method
 * @param[out] linked_session_id second session used by the same task or -1 if not available
 *
 * @return QBCASSessionType_none if sessionId is not connected with task
 * @return QBCASSessionType_playback/QBCASSessionType_reencryption type of sessionId if sessionId is connected with task
 */
extern QBCASSessionType
SvPlayerManagerTaskGetSessionInfo(SvObject task,
                                  int8_t sessionId,
                                  int8_t *linked_session_id);

/**
 * Check if session is a playback one
 *
 * @param[in] self      player manager handle
 * @param[in] sessionId provided by startDecryption QBCAS method
 *
 * @return @c true if session is a playback session else @c false
 */
extern bool
SvPlayerManagerIsPlaybackSession(SvPlayerManager self,
                                 uint8_t sessionId);

/**
 * Parameters for the player task.
 **/
typedef struct SvPlayerTaskParams_s {
    /// content to be played
    SvContent content;

    /**
     * A flag notifying that the task should not be linked to an existing
     * player/PVR task nor any other future player/PVR tasks should be linked
     * to this task; if set, manager will always create a new, separate task
     * (by default, tasks are connected together if the URI of the content
     * is the same).
     **/
    bool standalone;

    /**
     * Max allowed memory for use by created player object.
     *
     * This value will be shared by all linked player/PVR tasks.
     * If one value is used at creation time, and then another value when
     * linking a new task, the new value will be ignored.
     * Use @c 0 for default.
     **/
    int max_memory_use;

    /// A handle to the object implementing the @ref SvPlayerTaskListener.
    SvObject delegate;

    /// viewport on which content should be displayed
    struct QBViewport_s *viewport;

    /// Requested initial playback speed (@c 1.0 means normal playback).
    double speed;

    /// Requested initial position, usually @c 0.0 to start from the beginning.
    double position;

    /// A flag notifying that the task's purpose is to reencrypt content
    bool reencrypt;
} SvPlayerTaskParams;

/**
 * Create new player task and make it play with given parameters.
 *
 * This method creates new player task.
 * The task can change the requested speed and position as needed.
 * Parameters actually used by the task can be queried using
 * SvPlayerTaskGetState(), immediately after this method returns
 * or after SvPlayerTaskListener::stateChanged() notification.
 *
 * @param[in] self      player manager handle
 * @param[in] params    desired player task parameters
 * @param[out] errorOut error info
 * @return              created player task or @c NULL in case of error
 **/
extern SvPlayerTask
SvPlayerManagerCreatePlayerTask(SvPlayerManager self,
                                const SvPlayerTaskParams *params,
                                SvErrorInfo *errorOut);

/**
 * Parameters for the PVR task.
 **/
typedef struct {
    /// Content to be recorded.
    SvContent content;

    /// Metadata for the PVR task, used by the caller
    SvContentMetaData meta;

    /// @see SvPlayerTaskParams::standalone
    bool standalone;

    /// @see SvPlayerTaskParams::max_memory_use
    int max_memory_use;

    /// A handle to the object implementing the @ref SvPVRTaskListener.
    SvObject delegate;
} SvPVRTaskParams;

/**
 * Create new PVR task and make it record with given parameters.
 *
 * @param[in] self      player manager handle
 * @param[in] params    desired PVR task parameters
 * @param[out] errorOut error info
 * @return              created PVR task or @c NULL in case of error
 **/
extern SvPVRTask
SvPlayerManagerCreatePVRTask(SvPlayerManager self,
                             const SvPVRTaskParams *params,
                             SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
