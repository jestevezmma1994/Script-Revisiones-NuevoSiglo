/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SENTIVISION_ASF_TYPES
#define SENTIVISION_ASF_TYPES

/**
 * @file asftypes.h Data types used by ASF library
 **/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup asflib
 * @{
 **/

#undef u64
typedef uint32_t  u64[2];
typedef const uint32_t  u64c[2];

typedef struct __attribute__((__packed__)) guid_ {
  uint32_t g1;
  uint16_t g2[2];
  uint8_t g3[8];
} guid;
typedef guid SvGuid;

typedef uint8_t uuid[16];
typedef uint8_t ftime[8]; // nanosecs ticks from Jan 1 1601
// typedef uint8_t stime[8]; // miliseconds

typedef struct SvAsfRect_ {
  uint32_t left;
  uint32_t top;
  uint32_t right;
  uint32_t bottom;
} SvAsfRect;

// WAVEFORMATEX
typedef struct __attribute__((__packed__)) {
  uint16_t format; // 0x85 mp3, 0x161 - wma, 0x162,0x163 - wma9
  uint16_t channels;
  uint32_t samples_per_second;
  uint32_t avg_bytes_per_second;
  uint16_t block_aligment;
  uint16_t bits_per_sample;
  uint16_t codec_spec_data_length;
  uint8_t  efmt[0];
} waveformatex;

// BITMAPINFOHEADER
typedef struct __attribute__((__packed__)) {
  uint32_t format_data_size;
  uint32_t width;
  uint32_t height;
  uint16_t reserved;
  uint16_t bits_per_pixel;
  uint8_t fourcc[4];
  uint32_t image_size;
  uint32_t horizontal_pixels_per_meter;
  uint32_t vertical_pixels_per_meter;
  uint32_t colors_used;
  uint32_t imp_colors_used;
} bitmapinfoheader;

// VIDEOINFOHEADER
typedef struct __attribute__((__packed__)) {
  SvAsfRect src;
  SvAsfRect dst;
  uint32_t    bitrate;
  uint32_t    biterror;
  uint32_t    ftime_high;
  uint32_t    ftime_low;
  bitmapinfoheader bmh;
  uint8_t     efmt[0];
} videoinfoheader;

// ASFMEDIATYPE
typedef struct asfmediatype_ {
  SvGuid majortype;
  SvGuid subtype;
  uint32_t    bFixedSizeSamples;
  uint32_t    bTemporalCompression;
  uint32_t    lSampleSize;
  SvGuid formattype;
  uint32_t    pUnk;
  uint32_t    cbFormat;
  uint8_t     pbFormat[0];
} asfmediatype;

struct asf_parser_;

typedef enum {
  STREAM_UNKNOWN,
  STREAM_AUDIO,
  STREAM_VIDEO,
} stream_type_t;

typedef enum asf_code_ {
  ASF_ERROR,
  ASF_NO_MORE_ITEMS,
} asf_code;

/****************************************************
 *** HEADER OBJECTS *********************************
 ****************************************************/

typedef struct __attribute__((__packed__)) {
  guid object_id;
  u64 length;
  guid file_id;
  u64 file_size;
  u64 creation_date;
  u64 total_data_units; // number of data units in the data Object
  u64 play_duration;  // Duration of the Content [100ns]
  u64 send_duration;  // Duration of the streaming [100ns]
  u64 preroll;        // buffering time [ms]
  uint32_t flags;          // 0x1 - live, 0x2 - seekable
  uint32_t min_data_unit_size;
  uint32_t max_data_unit_size;
  uint32_t max_bitrate;    // bits per second
} file_prop_object;

typedef struct __attribute__((__packed__)) {
  uint32_t encoded_width;
  uint32_t encoded_height;
  uint8_t reserved;
  uint16_t format_data_length;
  bitmapinfoheader format;
} video_format;

typedef struct __attribute__((__packed__)) {
  guid object_id;
  u64 length;
  guid stream_type; // audio/video
  guid error_correction_type;
  u64 time_offset;  // beggining of the timeline (reference PTS), equal in all streams
  uint32_t type_spec_length;
  uint32_t error_corr_data_length;
  uint16_t flags; // bits 0-6 stream number (1-127), 0x8000 - encrypted
  uint32_t reserved; // always 0
  union {
    waveformatex audio;
    video_format video;
  } data;
  uint8_t* codec_spec_data;
  uint8_t* error_corr_data;
  stream_type_t type;
} stream_prop_object;

struct header_object_s
{
  guid object_id;
  u64 length;
  uint32_t header_count;
  uint8_t reserved[2]; // [2] has to be 0x02
  file_prop_object* file_prop;
  stream_prop_object** stream_prop;
  uint32_t stream_max;
  uint32_t stream_count;
  uint32_t time_offset;  // equal in all streams
} __attribute__((__packed__));
typedef struct header_object_s  header_object;

/****************************************************
 *** DATA OBJECTS ***********************************
 ****************************************************/

typedef struct sub_payload_
{
  uint8_t length;
  uint8_t* data;
} sub_payload;

typedef struct payload_
{
  uint8_t compressed;
  uint32_t data_length;   // 8,16,32 from payload header (grouping), or from DataUnit header
  uint8_t stream_number;
  uint8_t key_frame;      // whether payload belongs to the media object which is a keyframe
  uint32_t media_number;  // 0,8,16,32 (media object the payload belongs to (wrapped???))
  uint32_t rep_len;       // 0,8,16,32 (1-compressed, otherwise 0 or >=8)
  // uncompressed:
  uint32_t offset;        // 0,8,16,32 (offset in the media object)
  uint32_t sample_size;   // replicated data
  uint32_t sample_pts;    // replicated data
  uint8_t* ext_rep;       // over 8 bytes, specified in Extended Stream Properties Object
  uint8_t* data;          // payload data
  struct payload_* next;
  // compressed:
  uint32_t pts;           // 0,8,16,32 (based on offset len)
  uint8_t pts_delta;
  struct asf_parser_ *asf;
  uint32_t processed;     // bytes processed in this compressed payload
} payload;

typedef struct data_unit_
{
  // Error correction info
  uint8_t err_corr_present;
  uint8_t err_corr_length;
  uint8_t* err_corr;   // ErrorCorrection Object in the Header

  // Payload parsing info
  // length flags
  uint8_t grouping;
  uint8_t seq_len;
  uint8_t pad_len;
  uint8_t size_len; // grouping -> from payload flags
  // prop flags
  uint8_t rep_len;
  uint8_t offset_len;
  uint8_t media_len;
  uint8_t stream_len;

  uint32_t length;     // 0,8,16,32 (0 only for grouping)
  uint32_t seq;        // 0,8,16,32
  uint32_t pad;        // 0,8,16,32
  uint32_t send_time;  // [ms]
  uint16_t duration;   // [ms]

  struct asf_parser_* asf;
  payload* comp_pl; // compressed payload
  uint32_t processed;    // bytes processed in this data unit
  uint8_t payload_count;
} data_unit;

typedef struct __attribute__((__packed__)) {
  guid object_id;
  u64 length;
  guid file_id;
  u64 total_data_units;
  uint16_t reserved;
  // all data units are sorted by send_time
  struct asf_parser_* asf;
  data_unit* unit;
  uint32_t processed;  // bytes processed in this data object
  uint32_t unit_size;  // if fixed
  uint32_t unit_count;
  uint32_t unit_total;
} data_object;


extern void asf_print_header(const header_object* hdr);
extern void asf_print_data(const data_object* data);
extern void asf_print_data_unit(const data_unit* unit);
extern void asf_print_payload(const payload* pl);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
