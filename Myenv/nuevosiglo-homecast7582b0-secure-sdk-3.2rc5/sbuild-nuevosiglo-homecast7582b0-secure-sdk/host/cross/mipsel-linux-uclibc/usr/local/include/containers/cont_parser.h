/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CONTAINER_PARSER_H
#define CONTAINER_PARSER_H

#include <SvPlayerControllers/SvPlayerControllers.h>
#include <SvFoundation/SvHashTable.h>
#include <SvPlayerKit/SvChbuf.h>
#include <dataformat/QBContentSource.h>

#include <stdbool.h>
#include <stdint.h>

struct svdataformat;

struct cont_parser;

#define CONT_PARSER_STREAM_TYPE_VIDEO  0
#define CONT_PARSER_STREAM_TYPE_AUDIO  1
#define CONT_PARSER_STREAM_TYPE_SUBS   2

struct cont_parser_play_stats
{
  double  speed;
  double  scale;
  double  start_pos; /// in seconds. -1.0 iff default
};

struct cont_parser_frame_stats
{
  short stream_type;    /// CONT_PARSER_STREAM_TYPE_*
  short stream_num;     /// audio/video track, as specified in svdataformat.
  short is_keyframe;
  long long int pts90k; /// -1 iff unknown
  long long int duration90k; /// -1 iff unknown
};

struct cont_parser_params_
{
    bool timeshift; /// true if timeshift is enabled
};

struct cont_parser_caps
{
  bool remote_timeshift; /// remote timeshift

  bool pause;     /// pause playback

  bool seek_zero; /// seek to the beginning
  bool seek_any;  /// seek to any position

  bool speed;     /// change playback speed without changing content itself (e.g. slow playback)
  bool trick;     /// fast forward, fast backward
  bool live;      /// true for live TV, false for PvR, NPvR and VoD
  bool prefill;   /// prefill buffer before start playback
};

struct cont_parser_state_
{
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

struct cont_parser_output_positions
{
    double position;            /// playback position in seconds
    uint64_t played_pts;        /// playback position in 90k
    uint64_t buffered_pts;      /// buffered time in 90k
};

struct cont_parser_callbacks
{
  /// Data format has been successfully detected.
  /// \a format has already been retained.
  /// \a sourceProto information about detected protocol
  /// \a meta any meta-information that was detected during opening, can be null
  /// \return 0 or fatal error SV_ERR_*
  int (*format) (void* owner, struct cont_parser* parser, struct svdataformat* format, QBContentSourceProtocolType sourceProto, SvHashTable meta);

  /// Play request has been handled.
  /// \a stats results - might be different from what was passed to \a play() request.
  /// \return 0 or fatal error SV_ERR_*
  int (*playing) (void* owner, struct cont_parser* parser, const struct cont_parser_play_stats* stats);

  /// New chunk of data has arrived.
  /// \return 0 or fatal error SV_ERR_*
  int (*frame) (void* owner, struct cont_parser* parser, SvChbuf chb, const struct cont_parser_frame_stats* stats);

  /// New meta data has arrived.
  /// \return 0 or fatal error SV_ERR_*
  int (*meta_frame) (void* owner, struct cont_parser* parser, SvObject key_);

  /// No more frames will come.
  /// A \a seek() operation resets eos state.
  void (*eos) (void* owner, struct cont_parser* parser);

  /// A discontinuity has occurred.
  /// During this callback, the owner MUST either destroy the parser, or seek it to another position to recover.
  /// \a acceptable zero iff pipeline should be flushed. nonzero iff jump could be ignored.
  /// \return 0 or fatal error SV_ERR_*
  int (*discontinuity) (void* owner, struct cont_parser* parser, long long int pts90k, int acceptable);

  /// Fatal error has occurred.
  /// After receiving this callback, only \a close() and \a get_byte_range() functions can be safely called.
  void (*error) (void* owner, struct cont_parser* parser, int err);

  /// Event occurred.
  /// Can be used for example to inform about additional content data (variants).
  int  (*event) (void* owner, struct cont_parser* parser, SvString name, void* arg);

  /// Get the position and the buffered time.
  void (*getPosition)(void* owner, struct cont_parser* parser, struct cont_parser_output_positions* output_positions);

  // TODO: no_space() ?
};


struct cont_parser
{
  void (*destroy) (struct cont_parser* parser);

  /// Start/pause playback.
  /// Must not be called before receiving \a format() callback.
  /// \returns 1 iff handled immediately, with same parameters.
  /// \returns 0 iff scheduled. \a playing() callback will come later (but not when still inside \a play() function).
  /// \returns fatal errors from SV_ERR_*.
  int (*play) (struct cont_parser* parser, struct cont_parser_play_stats* stats, struct cont_parser_params_* params);

  void (*set_callbacks) (struct cont_parser* parser, const struct cont_parser_callbacks* callbacks, void* owner);

  /// Get content length, in bytes and/or seconds.
  void (*get_size) (const struct cont_parser* parser, int64_t* bytes, int64_t* duration90k);

  /// Get track controllers
  void (*get_controllers) (const struct cont_parser* parser, SvPlayerControllers controllers);

  /** This function might be set to null.
   \param name specific to destination module
   \param p pointer to private data, interpreted by destination module
   \returns >= 0 on success, INVALID when ignored, otherwise fatal error.
  */
  int (*set_opt) (struct cont_parser* parser, const char* name, void* p);

  /// Valid after format() callback.
  struct cont_parser_caps  (*get_caps) (const struct cont_parser* parser);

  /// Get timeshift status
  void (*get_state) (const struct cont_parser* parser, struct cont_parser_state_* state);
};


#endif // #ifndef CONTAINER_PARSER_H
