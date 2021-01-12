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

#ifndef STORAGE_INPUT_H
#define STORAGE_INPUT_H

#include <fibers/c/fibers.h>
#include <SvCore/SvTime.h>
#include <SvPlayerKit/SvBufMan.h>
#include <SvPlayerKit/SvBuf.h>

#include <stdint.h>

#define STORAGE_INPUT_DEFAULT_BLOCK_SIZE (127 * 1024)

struct storage_input;

typedef enum {
    storage_input_type__unknown,
    storage_input_type__pvr,
    storage_input_type__timeshift,
    storage_input_type__http,
    storage_input_type__file,
} storage_input_type;

struct storage_input_callbacks
{
  /// New chunk of data has arrived.
  /// \return 0 or fatal error SV_ERR_*
  int (*data) (void* owner, struct storage_input* input, SvBuf sb, int64_t pos);

  /// No more data will come.
  /// A \a seek() operation resets eos state.
  void (*eos) (void* owner, struct storage_input* input);

  // Data transfer was paused (because of full buffer), or unpaused.
  void (*paused) (void* owner, struct storage_input* input, bool pause);

  /// Position has changed due to some internal reasons.
  /// This callback won't fire when position is changed explicitly (e.g with seek).
  /// You can safely call \a seek() from inside this callback, but note that you can get this callback immediately again
  /// if requested position is before valid range, which can be acquired with \a get_byte_range().
  /// \return 0 or fatal error SV_ERR_*
  int (*position_lost) (void* owner, struct storage_input* input);

  /// Fatal error has occured.
  /// After receiving this callback, only \a destroy() and \a get_byte_range() functions can be safely called.
  void (*error) (void* owner, struct storage_input* input, int err);

  // TODO: no_space() ?
};


struct storage_input
{
  SvScheduler  scheduler;
  const char* url;
  storage_input_type type;

  SvBufMan  buf_man;

  /// Always succeeds.
  void (*destroy) (struct storage_input* input);

  /// Has to be used before first call to \a unpause().
  void (*set_callbacks) (struct storage_input* input, const struct storage_input_callbacks* callbacks, void* owner);

  /// Always succeeds.
  /// FIXME: this is not true for http, which does not know the exact file size.
  void (*get_byte_range) (struct storage_input* input, int64_t* curr, int64_t* start, int64_t* end);

  /// Pause/unpause reading process.
  /// User can assume that \a data() or \a eos() callback will not be comming when paused.
  void (*unpause) (struct storage_input* input);
  void (*pause) (struct storage_input* input);
  /// \param duration will unpause after this time. can be used for low-grained control of bitrate.
  void (*pause_timed) (struct storage_input* input, SvTime duration);

  /// Seeks to the exact byte position \a pos and start reading data up to position \a pos + \a len, when \a eos will be reported.
  /// Use \a get_byte_range() to check range of available positions.
  /// Always succeeds. Successive read() will validate the position.
  void (*seek) (struct storage_input* input, int64_t pos, int64_t len);

  /// Hint that given range will be \a seek()'ed to in the near future.
  /// Storage instance might preload this data and use it after proper \a seek() request.
  /// When such data is being dropped is an implementation-detail.
  void (*prebuffer) (struct storage_input* input, int64_t pos, int64_t len);

  /// Enable/disable live mode.
  /// In live mode, plugin does not report eos, even if no data is available anymore.
  /// Instead, plugin makes checks (from time to time) if more input might be available
  /// (e.g. when someone appended more data to a file).
  /// \returns 0 or INVALID when live mode is not supported.
  int (*live_mode) (struct storage_input* input, int enable);
};


#endif // #ifndef STORAGE_INPUT_H
