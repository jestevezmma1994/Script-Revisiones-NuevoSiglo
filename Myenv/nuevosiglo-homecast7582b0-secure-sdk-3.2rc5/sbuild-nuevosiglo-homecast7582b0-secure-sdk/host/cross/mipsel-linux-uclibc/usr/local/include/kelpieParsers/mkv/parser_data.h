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

#ifndef KPPARSERS_MKV_PARSER_DATA_H
#define KPPARSERS_MKV_PARSER_DATA_H

#include "../ebml/elem_desc.h"

#ifdef __cplusplus
extern "C" {
#endif


struct kpmkv_info
{
  uint32_t  pts2ms_nom;
  uint32_t  pts2ms_denom;

  double  duration; /// in pts2ms units

  char* title;
};

#define KPMVK_PTS2MS(pts, data) ((pts) * (data)->info.pts2ms_nom / (data)->info.pts2ms_denom)
#define KPMVK_MS2PTS(pts, data) ((pts) * (data)->info.pts2ms_denom / (data)->info.pts2ms_nom)

struct kpmkv_seek_pos
{
  kpebml_id_t  id;
  uint64_t  pos; // in the segment.
  bool  parsed; // HACK: since this is not parser data, but parser state.
};
struct kpmkv_cue_pos
{
  uint64_t  time;
  uint32_t  track;
  uint64_t  pos; // in the segment.
  uint32_t  block_num;
};

#define KPMKV_TRACK_TYPE__VIDEO     0x01
#define KPMKV_TRACK_TYPE__AUDIO     0x02
#define KPMKV_TRACK_TYPE__SUBTITLES 0x11

struct kpmkv_video
{
  uint32_t pixel_w;
  uint32_t pixel_h;
  uint32_t display_w;
  uint32_t display_h;
  bool interlaced;
};
struct kpmkv_audio
{
  uint32_t sample_freq;
  uint32_t channels;
};
struct kpmkv_track
{
  int num;
  uint32_t type;
  bool is_default;
  bool trick_track_flag;
  char* name;
  char* language;

  char* codec_id;
  char* codec_private;
  char* codec_name;
  int codec_private_len;
  uint32_t default_duration_ns; // of a single frame, in nanoseconds

  struct {
    bool is_active;
    uint8_t algo;   // as seen in ContentCompAlgo
    char* data;     // as seen in ContentCompSettings
    int data_len;
  } comp;

  union {
    struct kpmkv_video  video;
    struct kpmkv_audio  audio;
    // subtitles do not have any data, yet
  };
};

struct kpmkv_track* kpmkv_track_create(void);
void kpmkv_track_destroy(struct kpmkv_track* track);
int kpmkv_track_validate(const struct kpmkv_track* track);

/// Translate codec_id/codec_private from matroska format
/// into popular codec name and mp4-like codec_private.
extern
int kpmkv_track_translate(struct kpmkv_track* track);

struct kpmkv_parser_data
{
  // various file info.
  uint64_t segment_data_pos;  // in the file.
  uint64_t segment_data_size; // in the file.
  uint64_t cluster_start_pos; // in the file. 0 iff not yet known.
  uint64_t cluster_max_size;  // -1 iff not known.

  // Info
  struct kpmkv_info  info;

  // SeekHead - main
  struct {
    int cnt;
    int size;
    struct kpmkv_seek_pos* tab;
  } main_seek;

  // SeekHead - clusters
  struct {
    int cnt;
    int size;
    uint64_t* tab;
  } cluster_seek;

  // Cues
  struct {
    int cnt;
    int size;
    struct kpmkv_cue_pos* tab;
  } cues;

  // Track
  struct {
    int cnt;
    int size;
    struct kpmkv_track** tab;
  } track;
};


struct kpmkv_parser_data* kpmkv_parser_data_create(void);

SvExport
void kpmkv_parser_data_destroy(struct kpmkv_parser_data* parser_data);
SvExport
void kpmkv_parser_data_print(const struct kpmkv_parser_data* parser_data);
SvExport
struct kpmkv_track* kpmkv_parser_data_find_track(const struct kpmkv_parser_data* parser_data, int track_num);

SvExport
uint64_t kpmkv_parser_data_seek(const struct kpmkv_parser_data* parser_data, uint64_t* pts, bool* eos);
SvExport
uint64_t kpmkv_parser_data_seek_next(const struct kpmkv_parser_data* parser_data, uint64_t* pts, bool* eos);
SvExport
uint64_t kpmkv_parser_data_seek_prev(const struct kpmkv_parser_data* parser_data, uint64_t* pts, bool* eos);


#ifdef __cplusplus
}
#endif

#endif // #ifndef KPPARSERS_MKV_PARSER_DATA_H
