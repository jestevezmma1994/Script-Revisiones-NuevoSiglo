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

#ifndef SV_INPUT_PLUGIN_H
#define SV_INPUT_PLUGIN_H

#include <SvCore/SvErrorInfo.h>
#include <dataformat/QBContentSource.h>
#include <SvPlayerKit/SvMemCounter.h>
#include <SvPlayerKit/SvContent.h>
#include <SvPlayerKit/SvPlayerConfig.h>
#include <SvPlayerKit/SvPlayerCommonTypes.h>
#include <SvPlayerControllers/SvPlayerControllers.h>
#include <fibers/c/fibers.h>
#include <QBCASSession.h>

#include <stdbool.h>

struct svdataformat;

/**
  * @addtogroup SvPlayerKit_InputPlugin
  * @{
  * @file sv_input_plugin.h Low level API between input plugins and player.
  *
  * Input plugin is part of software which fetches multimedia streams from
  * network, disk or other media and pushes it to player.
  */


struct sv_data_sink;

struct sv_input_plugin_f;
struct sv_input_plugin;

struct sv_input_plugin_caps {
    bool live;    /// true for live TV (with timeshift also), false for NPvr and VoD
    bool prefill; /// true when buffer shoud be preffiled before start playback
    bool broadcast;

    // for live-plugins:
    bool record;  /// record
    bool remote_timeshift; /// remote timeshift

    // for non-live plugins:
    bool pause;   /// pause playback
    bool autopause; /// will gracefully handle out-of-buffers situation by pausing the reading of data from source

    bool seek_zero; /// seek to the beginning
    bool seek_any; /// seek to any position

    bool speed;   /// change playback speed without changing content itself (e.g. slow playback)
    bool trick;   /// fast forward, fast backward

    bool reencryption; /// @c true if input could perform a reencryption
};

/** Callback to be used by input plugin to notify about state changes */
struct sv_input_plugin_callbacks {
    void (*error) (void *owner, struct sv_input_plugin *plugin, int error);

    /** Plugin has successfully detected data format that it will be spitting out.
     *  or:
     *  Plugin has detected a new, better data format (e.g. first it knew that it is TS, and the next time it knows about sid/pmt_pid).

     *  @param[in] owner callback self data
     *  @param[in] plugin input plugin which send callbackE
     *  @param[in] format detected format
     *  @param[in] source detected source
     *  @return @c 0 on success, value < 0 on failure
     */
    int (*ready) (void *owner,
                  struct sv_input_plugin *plugin,
                  struct svdataformat *format,
                  QBContentSource source);

    int (*event) (void *owner, struct sv_input_plugin *input, SvString name, void *arg);

    /** Plugin has finished processing all its commands.
        MAY NOT come during a call to play().
        The current playback state of the plugin is stable, and may be checked with TODO.
     */
    int (*cmd_handled) (void *owner, struct sv_input_plugin *plugin);

    /** Plugin has stopped handling commands due to a command handling failure.
        MAY NOT come during a call to play().
        The current playback state of the plugin is stable, and may be checked with TODO.
        // TODO: definitely use SvErrorInfo here. -def
     */
    int (*cmd_failed) (void *owner, struct sv_input_plugin *plugin, int error);

    /**
     *  Plugin has assigned new session id
     *
     *  @param[in] owner callback self data
     *  @param[in] plugin input plugin which send callback
     *  @param[in] sessionId assigned session id
     */
    void (*set_session_id) (void *owner, struct sv_input_plugin *plugin, int8_t sessionId);
};

/** Input plugin working mode */
typedef enum sv_input_plugin_input_mode {
    /** default working mode */
    sv_input_plugin_input_mode__default,
    /** Reencrypt content for PVR/TS purpose */
    sv_input_plugin_input_mode__reencryption,
} sv_input_plugin_input_mode;

/** Parameters passed to input plugin */
struct sv_input_plugin_params {
    SvScheduler scheduler;
    SvContent content;
    /// Specify input plugin working mode
    sv_input_plugin_input_mode input_mode;

    struct sv_telem *telem;  // owner will be attaching pipeline sink directly to this telem
    SvPlayerConfig playerConfig;
    SvPlayerUsage playerUsage;
    QBCASSession casSession;
};

/** Parameter passed to get_state() callback */
struct sv_input_plugin_state {
    double wanted_speed;
    double current_speed;
    double pos;
    double duration; /// @c 0.0 if unknown

    struct {
        bool enabled;
        bool active;
        double start;
        double end;
    } timeshift;

    /// average bitrate of all downloaded data: video, audio, subtitles, etc.
    /// it is measured only for streams which have more than one quality variant
    int64_t avarageBitrate;
    /// last selected bitrate
    int bitrate;
};

/** Functions which should be implemented by input plugin */
struct sv_input_plugin_f {
    void (*destroy)   (struct sv_input_plugin *plugin);

    /** Attach sink (being an entry point of an output pipeline).
     */
    void (*set_sink) (struct sv_input_plugin *plugin, struct sv_data_sink *sink);

    /** Inform the plugin that the sink (attached earlier) is ready for pushing data.
        "Ready" means that the pipeline is not expected to block the data flow by some
        long-running initialization.
        Plugin might use this information to apply some previously delayed play request.
        Plugin might use this information to enable clock recovery from its data source.
     */
    void (*output_connected) (struct sv_input_plugin *plugin);

    /** May be rejected immediately, returning negative error. // TODO: definitely use SvErrorInfo.
        May be handled immediately, returning 1.
        May be scheduled for asynchronous handling - if so, then:
          failure notifications come from cmd_failed() handler.
          success notifications come from cmd_handled() handler.
        Only for non-live plugins.
     */
    int (*play) (struct sv_input_plugin *plugin, double speed, double npt, bool timeshift);

    void (*get_state) (struct sv_input_plugin *plugin, struct sv_input_plugin_state *state);

    struct sv_input_plugin_caps (*get_caps) (const struct sv_input_plugin *plugin);

    /**
     * Create track controllers.
     *
     * @param[in] plugin input plugin instance
     * @param[in,out] controllers created track controllers
     */
    void (*get_controllers)(struct sv_input_plugin *plugin, SvPlayerControllers controllers);

    /** This function might be set to null.
     \param name specific to destination module
     \param p pointer to private data, interpreted by destination module
     \returns zero on success, INVALID when ignored, otherwise fatal error.
    */
    int (*set_opt) (struct sv_input_plugin *plugin, const char *name, void *p);
};

#define FILL_INPLUGIN_PTRS(plug, prefix) do { \
        (plug).destroy = &prefix ## _destroy; \
        (plug).set_sink = &prefix ## _set_sink; \
        (plug).output_connected = &prefix ## _output_connected; \
        (plug).play = &prefix ## _play; \
        (plug).get_state = &prefix ## _get_state; \
        (plug).get_caps = &prefix ## _get_caps; \
        (plug).get_controllers = &prefix ## _get_controllers; \
        (plug).set_opt = &prefix ## _set_opt; \
} while (0)

struct sv_input_plugin {
    struct sv_input_plugin_f functor;

    const struct sv_input_plugin_callbacks *callbacks;
    void *owner;
};

typedef int (sv_input_plugin_create_fun_t) (const struct sv_input_plugin_params *params,
                                            struct sv_input_plugin **out,
                                            SvErrorInfo *error_out);

/** @} */

#endif // #ifndef SV_INPUT_PLUGIN_H
