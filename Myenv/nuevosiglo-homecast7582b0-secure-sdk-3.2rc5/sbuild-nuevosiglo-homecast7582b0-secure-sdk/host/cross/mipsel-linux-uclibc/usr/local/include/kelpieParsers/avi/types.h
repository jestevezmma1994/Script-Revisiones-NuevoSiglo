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

#ifndef KPPARSERS_AVI_PARSER_TYPES_H
#define KPPARSERS_AVI_PARSER_TYPES_H

#include "../external/common_types.h"

#ifdef __cplusplus
extern "C" {
#endif

enum kpavi_stream_type
{
  kpavi_stream_type__video,
  kpavi_stream_type__audio,
  kpavi_stream_type__subtitle,

  kpavi_stream_type__other = -1,
};

struct kpavi_stream_audio
{
  uint32_t  format;

  uint32_t channels;

  uint32_t rate;

  uint32_t avg_bps;
  uint16_t block_align;
  uint16_t bits_per_sample;
};

struct kpavi_stream_video
{
  uint32_t  fourcc;

  uint32_t  w;
  uint32_t  h;
  uint16_t  bits;
};

struct kpavi_index
{
  uint32_t  cnt; // should be equal to frame_cnt (but during parsing, it can be different)
  uint32_t* offset;
  uint32_t* size_accumulation; // sum of sizes of this and all previous frames from this stream
  uint32_t* keyframe_bits;
};

struct kpavi_stream
{
  enum kpavi_stream_type  type;

  uint32_t  frame_duration; // a.k.a. "scale", given in "rate" units
  uint32_t  rate;
  uint32_t  frame_cnt; // a.k.a. "len"

  bool  is_cbr; // must calculate frame position/duration based on data sizes
  uint32_t  total_size; // calculated from index

  uint8_t* private_data;
  uint32_t private_data_len;

  union {
    struct kpavi_stream_video  video;
    struct kpavi_stream_audio  audio;
  };

  struct kpavi_index  index;
};

struct kpavi_stream* kpavi_stream_create(void);
void kpavi_stream_destroy(struct kpavi_stream* stream);

void kpavi_stream_print(const struct kpavi_stream* stream);

struct kpavi_info
{
  bool has_index;
  bool must_use_index;

  uint32_t  movi_pos;
  uint32_t  movi_size;

  bool  index_offsets_relative_to_movi; // relative to 'movi' chunk, or absolute in the file
  int32_t  index_offsets_modifier; // index offsets do not point to start of a frame chunk, but are N bytes off

  bool  movi_with_embedded_list; // inside 'movi', instead of pure frames, there is LIST('rec ') chunk, and then, inside, there are the frames - this moves the frames start by 12 bytes

  uint32_t  stream_cnt;
  struct kpavi_stream** streams;
};

struct kpavi_info* kpavi_info_create(void);
void kpavi_info_destroy(struct kpavi_info* info);

/// \returns duration in milliseconds
uint64_t kpavi_info_get_duration(const struct kpavi_info* info);

void kpavi_info_print(const struct kpavi_info* info);

#ifdef __cplusplus
}
#endif

#endif // #ifndef KPPARSERS_AVI_PARSER_TYPES_H
