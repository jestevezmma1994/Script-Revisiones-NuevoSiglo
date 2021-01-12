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

#ifndef SV_AUTO_PLAYER_H
#define SV_AUTO_PLAYER_H

/**
 * @file sv_auto_player.h autoplayer library API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

/**
 * @defgroup autoplayer autoplayer: represents a single instance of media source
 * @ingroup MediaPlayer
 * @{
 *
 * Autoplayer object represents a single instance of media source,
 * which can be routed to output, recorded, timeshifted (or all of that at the same time).
 */

#include "sv_input_plugin.h" // needed for struct sv_input_plugin_caps
#include <SvPlayerKit/SvPlayerConfig.h>
#include <SvPlayerKit/SvDRMInfo.h>
#include <SvPlayerKit/SvPlayerCommonTypes.h>
#include <SvPlayerControllers/SvPlayerTaskControllers.h>
#include <SvFoundation/SvString.h>
#include <fibers/c/fibers.h>
#include <QBRecordFS/types.h>
#include <QBCAS.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// forward only
struct QBViewport_s;

/**
 * autoplayer class.
 * @class sv_auto_player
 */
struct sv_auto_player;

/**
 * autoplayer callbacks methods.
 * @class sv_auto_player_handlers
 *
 * Callbacks will always be called from some internal fiber, so never during any autoplayer API call.
 * All callbacks here are reentrant (during a callback, you can call any method on the player object, including destruction).
 */
struct sv_auto_player_handlers {
    /**
     * Input source error, the autoplayer must be destroyed.
     *
     * @param[in] error SV_ERR_*
     * @see sv_auto_player_destroy
     */
    void (*error) (void *owner,
                   struct sv_auto_player *player,
                   int error);

    /**
     * Opening phase is finished, all internal objects have been created.
     *
     * Once this is called (and during this call as well), you can safely call some additional methods, like getting stats, etc.
     *
     * @return 0 for success
     * @return SV_ERR_WAS_SHUTDOWN if the player object was destroyed during this callback
     * @return negative for other fatal error
     *
     * @see sv_auto_player_destroy
     */
    int (*open_done) (void *owner,
                      struct sv_auto_player *player);

    /**
     * While playing, reached End-of-stream for buffered data (there might still be content to play in decoder's internal buffers).
     *
     * @return 0 for success
     * @return SV_ERR_WAS_SHUTDOWN if the player object was destroyed during this callback
     * @return negative for other fatal error
     *
     * @see sv_auto_player_destroy
     */
    int (*buffer_eos) (void *owner,
                       struct sv_auto_player *player);

    /**
     * While playing, reached End-of-stream for entire data (including decoder internal buffers).
     *
     * @return 0 for success
     * @return SV_ERR_WAS_SHUTDOWN if the player object was destroyed during this callback
     * @return negative for other fatal error
     *
     * @see sv_auto_player_destroy
     */
    int (*output_eos) (void *owner,
                       struct sv_auto_player *player);

    /**
     * Custom event has happened.
     *
     * See player_events/ *.h for possibilities for @a name and @a arg
     *
     * @return 0 for success
     * @return SV_ERR_WAS_SHUTDOWN if the player object was destroyed during this callback
     * @return negative for other fatal error
     *
     * @see sv_auto_player_destroy
     */
    int (*event) (void *owner,
                  struct sv_auto_player *player,
                  SvString name,
                  void *arg);

    /**
     * Player has finished processing all its commands.
     *
     * The current playback state of the plugin is stable, and may be checked with status getters.
     *
     * @return 0 for success
     * @return SV_ERR_WAS_SHUTDOWN if the player object was destroyed during this callback
     * @return negative for other fatal error
     *
     * @see sv_auto_player_play
     * @see sv_auto_player_get_state
     * @see sv_auto_player_destroy
     */
    int (*cmd_handled) (void *owner,
                        struct sv_auto_player *player);

    /**
     * Player has failed to handle given play command
     *
     * @param[in] error SV_ERR_*
     * @return 0 for success
     * @return SV_ERR_WAS_SHUTDOWN if the player object was destroyed during this callback
     * @return negative for other fatal error
     *
     * @see sv_auto_player_play
     * @see sv_auto_player_destroy
     *
     */
    int (*cmd_failed) (void *owner,
                       struct sv_auto_player *player,
                       int error);

    /**
     * Output module error, the autoplayer output capability has to be terminated
     *
     * @param[in] error SV_ERR_*
     * @return 0 for success
     * @return SV_ERR_WAS_SHUTDOWN if the player object was destroyed during this callback
     * @return negative for other fatal error
     *
     * @see sv_auto_player_enable_output
     * @see sv_auto_player_destroy
     */
    int (*output_error) (void *owner,
                         struct sv_auto_player *player,
                         int error);

    /**
     * Record module error, the autoplayer recording capability has to be terminated (either file-based, or timeshift-based)
     *
     * If @a file is given, then PVR recording has failed (then, recording to the file has to be stopped manually).
     * If @a file is @c NULL, then timeshift recording has failed, and player has already disabled timeshift.
     *
     * @param[in] file file that is failing to be recorded to, or @c NULL if thimeshift is failing
     * @param[in] error SV_ERR_*
     * @return 0 for success
     * @return SV_ERR_WAS_SHUTDOWN if the player object was destroyed during this callback
     * @return negative for other fatal error
     *
     * @see sv_auto_player_stop_recording
     * @see sv_auto_player_enable_timeshift_recording
     * @see sv_auto_player_destroy
     */
    int (*record_error) (void *owner,
                         struct sv_auto_player *player,
                         QBRecordFSFile *file,
                         int error);
    /*
     * Output module/input plugin provides information about playback/reencryption session
     * @param[in] sessionType type of session (reencryption/playback)
     * @param[in] sessionId session identificator
     */
    void (*set_session_id) (void *owner,
                            struct sv_auto_player *player,
                            QBCASSessionType sessionType,
                            int8_t sessionId);
};

/**
 * Autoplayer creation params.
 * @class sv_auto_player_params
 */
struct sv_auto_player_params {
    /// Scheduler to create all internal fibers with.
    SvScheduler scheduler;

    /// Input source description.
    SvContent content;

    /// Player configuration.
    SvPlayerConfig player_config;

    /// Instead of output pipeline, create reencryption-pipeline. Works only on PVR contents.
    bool reencrypt_mode;
    /// Specifies input plugin working mode
    sv_input_plugin_input_mode input_mode;

    /// Maximum memory (in bytes) to be used for buffering data, @c 0 for default value.
    /// After reaching this maximum, depending on input-source, new data will be dropped, or input will auto-pause reading data,
    /// or playback speed can increase (in server-side trick-modes).
    int max_memory_use;

    /// Callbacks.
    const struct sv_auto_player_handlers *handlers;
    /// Callbacks parameter.
    void *owner;

    /// indicates if player is used only for PVR, Playback or it can be shared
    SvPlayerUsage playerUsage;
};

/**
 * Restrictions for current playback
 */
struct sv_auto_player_restrictions {
    bool isFFSpeedLimited; /**< if @c true fast forward trick play speed limitation is enabled, @see maxSpeed */
    double maxFFSpeed; /**< maximum fast forward speed, used if @ref isFFSpeedLimited is set */
    bool jumpingDisallowed; /**< @c true if jumping is disallowed */
    bool pausingDisallowed; /**< @c true if content cannot be paused */
};

/**
 * Autoplayer status.
 * @class sv_auto_player_state
 */
struct sv_auto_player_state {
    /// Current playback speed on the output.
    double curr_scale;
    /// Requested playback speed, which needs to be processed before it can be applied.
    double wanted_scale;

    /// Current position on the output, in seconds.
    double curr_pos;
    /// Content duration in seconds (could change over time), @c 0.0 if unknown or in live playback
    double duration;

    /// Current System Time Clock on the output, measured in 90 kHz units. -1 if not known.
    /// Can be used to synchronize external media sources, e.g. subtitles.
    int64_t curr_stc90k;

    /// playback discontinuity since start of playback
    int64_t playbackDiscontinuityInMs;
    /// discontinuity counter
    int64_t playbackDiscontinuityCounter;

    /// EOS on the output.
    bool eos;

    /// Buffered data ready to be played, in seconds.
    double buffered_sec;

    /// Memory used to store buffered data, in bytes.
    int32_t used_bytes;

    /// Properties related to timeshift.
    struct {
        /// Timeshift recording has been enabled.
        bool recording;
        /// Timeshift playback has been requested.
        bool wanted;
        /// Timeshift playback has been requested and applied.
        bool enabled;
        /// Timeshift recording buffer - earliest position that is still accessible to play, in seconds.
        double range_start;
        /// Timeshift recording buffer - latest position that is accessible to play, in seconds.
        double range_end;
    } timeshift;

    /// current restrictions
    struct sv_auto_player_restrictions restrictions;

    /// average bitrate of all downloaded data: video, audio, subtitles, etc.
    /// it is measured only for streams which have more than one quality variant
    int64_t avarageBitrate;
    /// last selected bitrate
    int bitrate;
};

/**
 * Create autoplayer instance and start opening input source from given url.
 *
 * You can freely call autoplayer functions on this instance right away, but depending on the input source,
 * some calls will be deferred to after the opening phase is done, and then the validity of those calls will be verified,
 * and some might fail at that time (with proper notification).
 *
 * @param[in] params  creation parameters
 * @param[out] out created autoplayer instance
 * @param[out] error_out unused
 * @returns 0 on success, nagative on error
 */
int sv_auto_player_create(const struct sv_auto_player_params *params,
                          struct sv_auto_player **out,
                          SvErrorInfo *error_out);

/**
 * Destroy given autoplayer instance.
 *
 * Internal objects might take some additional time to be fully destroyed (in other fibers/threads).
 * No callbacks will be issued ever again from this autoplayer instance.
 *
 * @param[in] player  autoplayer instance to be destroyed
 */
void sv_auto_player_destroy(struct sv_auto_player *player);

/**
 * Check if \a content is compatible with this player instance (would use the same input source).
 *
 * This is to check if additional playback/recording could be attached to this instance.
 *
 * @param[in] player  autoplayer instance that is already running
 * @param[in] content input source description that is to be compared with @a player input source
 * @returns @c true if @a content describes input source that @a player is already using
 * @see sv_auto_player_enable_output
 * @see sv_auto_player_start_recording
 */
bool sv_auto_player_is_compatible(const struct sv_auto_player *player,
                                  SvContent content);

/**
 * Enable playback of input source to the a/v output.
 *
 * @param[in] player autoplayer instance
 * @param[in] enable @c true if output is to be enabled, @c false if disabled
 * @param[in] drm_info decryption parameter
 * @param[in] viewport output description object
 * @returns 0 on success, nagative on error
 */
int sv_auto_player_enable_output(struct sv_auto_player *player,
                                 bool enable,
                                 SvDRMInfo drm_info,
                                 struct QBViewport_s *viewport);

/**
 * Enable recording of input source to a timeshift buffer. This does NOT mean that timeshift playback it to be used.
 *
 * For playback from timeshift buffer, please see @e sv_auto_player_enable_timeshift_playback.
 * If @a enable is @c false, drops entire buffer that was being collected.
 *
 * @param[in] player autoplayer instance
 * @param[in] enable @c true if timeshift recording is to be enabled, @c false if disabled
 * @returns 0 on success, nagative on error
 *
 * @see sv_auto_player_enable_timeshift_playback
 * @see sv_auto_player_enable_output
 */
int sv_auto_player_enable_timeshift_recording(struct sv_auto_player *player,
                                              bool enable);

/**
 * Start recording input source to a file (PVR scenario).
 *
 * Only one recording can be started from a single autoplayer instance at the same time.
 *
 * @param[in] player autoplayer instance
 * @param[in] meta  metadata to be stored in the recorded file for later access (has no impact on the actual recording process)
 * @param[out] file_out newly created pvr file
 * @returns 0 on success, nagative on error
 *
 * @see sv_auto_player_stop_recording
 */
int sv_auto_player_start_recording(struct sv_auto_player *player,
                                   SvContentMetaData meta,
                                   QBRecordFSFile **file_out);

/**
 * Stop recording to given file. The file will NOT be deleted by this call.
 *
 * @param[in] player autoplayer instance
 * @param[in] file file that @a player currently records to, that was returned with @e sv_auto_player_start_recording
 * @returns 0 on success, nagative on error
 *
 * @see sv_auto_player_stop_recording
 */
int sv_auto_player_stop_recording(struct sv_auto_player *player,
                                  QBRecordFSFile *file);

/**
 * Enable/disable playback from timeshift buffer. This pauses playback when entering timeshift mode.
 *
 * A switch is made from/to live playback.
 * Timeshift recording must be enabled first.
 *
 * @param[in] player autoplayer instance
 * @param[in] enable @c true to switch to playback from timeshift buffer (seamless), @c false to switch to live playback
 *
 * @returns 0 on success, nagative on error
 *
 * @see sv_auto_player_enable_timeshift_recording
 * @see sv_auto_player_enable_output
 */
int sv_auto_player_enable_timeshift_playback(struct sv_auto_player *player,
                                             bool enable);

/**
 * Request playback with given @a scale, from given position @a position.
 * Works only for non-live plugins.
 *
 * May be rejected immediately, returning negative error.
 * May be handled immediately, returning @c 1.
 * May be scheduled for asynchronous handling, returning @c 0 - if so, then:
 *    failure notifications come from cmd_failed() handler.
 *    success notifications come from cmd_handled() handler.
 *
 * @param[in] player autoplayer instance
 * @param[in] scale playback speed, @c 1.0 means normal play, @c 0.0 means pause, > @c 1.0 or < @c -1.0 means trick-mode
 * @param[in] position start position, in seconds, or @c -1.0 for keeping current position that's presented on the output
 * @returns @c 1 on immediate success, negative on error, @c 0 when the request was scheduled only
 *
 * @see sv_auto_player_get_state
 */
int sv_auto_player_play(struct sv_auto_player *player,
                        double scale,
                        double position);

/**
 * Get player capabilities that are determined after input source has finished opening phase.
 *
 * @param[in] player autoplayer instance
 * @param[out] caps capabilites to be filled out
 * @returns @c 0 on success, negative on error (when caps are unknown yet)
 */
int sv_auto_player_get_caps(const struct sv_auto_player *player,
                            struct sv_input_plugin_caps *caps);

/**
 * Get player status.
 *
 * @param[in] player autoplayer instance
 * @param[out] state state struct to be filled out
 */
void sv_auto_player_get_state(const struct sv_auto_player *player,
                              struct sv_auto_player_state *state);

/**
 * Pass a custom parameter down to autoplayer's components.
 *
 * See player_opts/ *.h for possibilities for @a name and @a arg
 *
 * @param[in] player autoplayer instance
 * @param[in] name opt name
 * @param arg private data pointer, different meaning depending on @a name
 * @returns @c 0 on success, @c SV_ERR_INVALID when ignored, otherwise fatal error
 */
int sv_auto_player_set_opt(struct sv_auto_player *player,
                           const char *name,
                           void *arg);

/**
 * Set Restrictions
 *
 * This method will set and apply restrictions for current playback
 *
 * @param[in] player autoplayer instance
 * @param[in] restrictions @see sv_auto_player_restrictions
 * @returns @c 0 on success, negative on error
 **/
int sv_auto_player_set_restrictions(struct sv_auto_player *player, struct sv_auto_player_restrictions *restrictions);

/**
 * Get track controllers.
 *
 * @param[in] player autoplayer instance
 * @return Player Task Controllers
 */
SvPlayerTaskControllers sv_auto_player_get_controllers(struct sv_auto_player *player);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_AUTO_PLAYER_H
