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

#ifndef STORAGE_INPUT_RA_H
#define STORAGE_INPUT_RA_H

#include "storage_input.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SvChbuf_s;

struct storage_input_ra;

struct storage_input_ra_callbacks
{
  void (*error)     (void* owner, struct storage_input_ra* input, int err);
  int  (*more_data) (void* owner, struct storage_input_ra* input);
  void (*paused)    (void* owner, struct storage_input_ra* input, bool pause);
};


struct storage_input_ra* storage_input_ra_create(struct storage_input* sub_input,
                                                 const struct storage_input_ra_callbacks* callbacks,
                                                 void* target);


void storage_input_ra_destroy(struct storage_input_ra* input);

void storage_input_ra_return_data(struct storage_input_ra* input, struct SvChbuf_s* chdata);

void storage_input_ra_set_limit(struct storage_input_ra* input, int64_t byte_limit, int ops_limit);
bool storage_input_ra_limit_reached(const struct storage_input_ra* input);

void storage_input_ra_print(const struct storage_input_ra* input);


/// Re-establishes a new range for reading.
/// If the new range is not valid, \a input will switch to eos state.
void storage_input_ra_seek(struct storage_input_ra* input, uint64_t pos, uint64_t max_size);

/// Moves forward, within the current range.
void storage_input_ra_skip(struct storage_input_ra* input, uint64_t size);

/// Reads \a size bytes starting from current position. Does not forward the position.
/// Returns amount of bytes read, which might be less than requested (even 0).
int storage_input_ra_peek(const struct storage_input_ra* input, void* dest, uint32_t size);

/// Reads \a size bytes starting from current position.
/// Does not read anything unless has at least \a size bytes - until then 0 is returned.
int storage_input_ra_try_read(struct storage_input_ra* input, void* dest, uint32_t size);

// Same as try_read(), but data is put into \a cb_fill
//int storage_input_ra_try_read_cb(struct storage_input_ra* input, uint32_t size, struct SvChbuf_s* cb_fill);

int64_t storage_input_ra_get_position(const struct storage_input_ra* input);
int64_t storage_input_ra_get_size(const struct storage_input_ra* input);

/// Returns non-zero iff no more data is comming.
/// (Note that some data might still be available for reading.)
/// This function is to be used after receiving 0 from peek/try_read* function calls.
bool storage_input_ra_is_eos(const struct storage_input_ra* input);


#ifdef __cplusplus
}
#endif

#endif // #ifndef STORAGE_INPUT_RA_H
