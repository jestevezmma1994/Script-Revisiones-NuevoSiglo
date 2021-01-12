/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
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

#ifndef SV_PLAYBACK_PARAMS_H_
#define SV_PLAYBACK_PARAMS_H_

struct SvPlaybackParams_s;
typedef struct SvPlaybackParams_s* SvPlaybackParams;

struct SvPlaybackParams_s
{
  // position in the stream goes N times faster than timestamps.
  double stream_scale;
  // add N seconds when calculating position in the stream.
  double stream_npt;

  // Stream might contain non-keyframes, but we are to ignore them.
  int only_keyframes;
  // We expect the stream to be jumping backwards during playback.
  // This is ok - we want the stream to be played in reversed direction.
  // Usefull in playing direct-ts in reversed-trick mode, by pushing large chunks
  // of data, and jumping backwards.
  int reversed;
  // needs prefilling, etc.
  int smooth_playback;
};

#if 0

/**
------------------------
scale = ( 0.0, 1.0 ]
(and beyond 1.0 if fast playback is to work similar to slow playback).
------------------------
*/

double scale = 0.8; // 1.0

// for: all
struct SvPlaybackParams_s  play_normal = {
  .stream_scale = 1.0,
  .npt = XXX,
  .only_keyframes = 0,
  .reversed = 0,
  .smooth_playback = 1,
};
display_scale = scale;

/**
------------------------
scale = (1.0, +inf)
(trick fwd, no fast playback).
------------------------
*/

double scale = 8.0;

// for: es
struct SvPlaybackParams_s  play_trickfwd_es = {
  .stream_scale = 1.0,
  .npt = XXX,
  .only_keyframes = 1,
  .reversed = 0,
  .smooth_playback = 0,
};
display_scale = scale;

// for: ts - svs, http, pvr_in, etc.
struct SvPlaybackParams_s  play_trickfwd_ts = {
  .stream_scale = 1.0,
  .npt = XXX,
  .only_keyframes = 1,
  .reversed = 0,
  .smooth_playback = 0,
};
display_scale = scale;

// for: ts - actvila, toshiba (scaled timestamps)
struct SvPlaybackParams_s  play_trickfwd_scaled = {
  .stream_scale = scale,
  .npt = XXX,
  .only_keyframes = 0,
  .reversed = 0,
  .smooth_playback = 0, // maybe 1
};
display_scale = 1.0;

/**
------------------------
scale = (-inf, 0.0)
------------------------
*/

double scale = -8.0;

// for: es // TODO
struct SvPlaybackParams_s  play_trickrew_es = {
  .stream_scale = scale,
  .npt = XXX,
  .only_keyframes = 0,
  .reversed = 0,
  .smooth_playback = 0,
};
display_scale = 1.0;

// for: ts - svs, http, pvr_in, etc.
struct SvPlaybackParams_s  play_trickrew_ts = {
  .stream_scale = 1.0,
  .npt = XXX,
  .only_keyframes = 1,
  .reversed = 1,
  .smooth_playback = 0,
};
display_scale = fabs(scale);

// for: ts - actvila, toshiba (scaled timestamps)
struct SvPlaybackParams_s  play_trickrew_ts_scaled = {
  .stream_scale = scale,
  .npt = XXX,
  .only_keyframes = 0,
  .reversed = 0,
  .smooth_playback = 0, // maybe 1
};
display_scale = 1.0;

#endif


#endif // #ifndef SV_PLAYBACK_PARAMS_H
