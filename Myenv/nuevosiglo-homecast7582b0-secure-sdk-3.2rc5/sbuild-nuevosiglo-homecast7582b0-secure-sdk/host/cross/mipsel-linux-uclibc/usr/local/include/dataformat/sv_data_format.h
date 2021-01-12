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

#ifndef SV_DATA_FORMAT_H
#define SV_DATA_FORMAT_H

#include "sv_data_format_type.h"
#include "subtitle.h"
#include "audio.h"
#include "video.h"
#include "content_protection.h"

#include <SvPlayerKit/SvBuf.h>
#include <SvPlayerKit/SvDrmSystemInfo.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum svdataformattype {
  svdataformattype_uninitialized = 0,
  svdataformattype_ts,
  svdataformattype_asfpl,
  svdataformattype_es,
  svdataformattype_direct_es,
  svdataformattype_count,
};

enum svrtpformattype {
  svrtpformattype_none = 0,
  svrtpformattype_unknown, // needs autodetection
  svrtpformattype_rfc,
  svrtpformattype_ms,
  // TODO: fec?
  svrtpformattype_count,
};

enum svtsformattype {
  svtsformattype_unknown = 0, // needs autodetection
  svtsformattype_ts,
  svtsformattype_tts,
  svtsformattype_count,
};

// supported subtitles standards
enum svsubtitlestype {
  svsubtitlestype_unknown,
  svsubtitlestype_dvb,        // DVB subtitles, as described by sv_subtitle_service structure
  svsubtitlestype_ttx,        // teletext subtitles, as described by sv_teletext_service structure
  svsubtitlestype_scte27,     // SCT-27 subtitles, as described by sv_scte27_stream structure
  svsubtitlestype_cc,         // Closed Captions, as described by sv_closed_caption_subs_service structure
  svsubtitlestype_es,         // subtitles carried in ES, as described by sv_txt_subs_stream structure
  svsubtitlestype_count
};

struct sv_simple_video_format
{
  short is_active; // iff 0, no data for this channel.
  char* codec; // malloc'ed.
  char* desc; // e.g. view angle. // malloc'ed. may be null.

  int drm_session_id; // DRM Manager session id

  struct {
    bool dts_as_pts; // "pts" field of es frames really contain dts, so in some cases those need to be fixed (e.g. B-frames)
  } quirks;

  struct {
    bool valid;
    bool constraints[3];
    bool still_present;
    bool picture_24h;

    QBVideoAVCProfile profile;
    uint8_t level;
    uint8_t comp_flags;
  } avc;
};

struct sv_simple_audio_format
{
  short is_active; // iff 0, no data for this channel.
  QBAudioCodec codec;
  QBAudioDualMonoMode dual_mono_mode;
  short is_multichannel;
  QBAudioType type;   // e.g. "for visually impaired"
  char* lang; // audio language. // malloc'ed. may be null.
  char* desc; // malloc'ed. may be null.

  int drm_session_id; // DRM Manager session id

  struct {
    bool frames_not_aligned; // audio frames contain chunks of audio stream, but not properly cut
  } quirks;
};

struct sv_extended_video_format
{
  int width;
  int height;
  int aspect_width;  // zero if unspecified.
  int aspect_height; // zero if unspecified.

  int fps_nom; // zero if unspecified.
  int fps_denom;

  int efmtlen;
  unsigned char *efmt; // malloc'ed, may be null.
};

struct sv_extended_audio_format
{
  int freq, chan;
  int bits, align; // valid depending on audio type
  int average_bps;
  int samples_in_frame;

  // PCM only
  struct {
    int lsb_first;
    int is_signed;
  } pcm;

  // mpeg audio only
  struct {
    int version;
    int layer;
  } mpeg;

  // aac only
  struct {
    bool adts;
  } aac;

  // ac3 only
  struct {
    int bitstream_id;
  } ac3;

  int efmtlen;
  unsigned char *efmt; // malloc'ed, may be null.
};

struct sv_txt_subs_stream
{
  short is_active; // iff 0, no data for this channel.

  char* lang; // malloc'ed, may be null.
  char* desc; // malloc'ed, may be null.

  int drm_session_id; // DRM Manager session id

  int fmt_len;
  unsigned char* fmt; // malloc'ed, may be null.
};

#define MAX_CLOSED_CAPTION_SUBS_IN_STREAM 4

struct sv_closed_caption_subs_service
{
  char lang[4];
  QBSubtitleType  subtitling_type;
  union {
     uint16_t field;            // when subtitling_type == QBSubtitleType_cc_cea608
     uint16_t service_number;   // ... QBSubtitleType_cc_cea708
  };
  bool easy_reader;
  bool wide_aspect_ratio;
};

#define MAX_SUBTITLES_IN_STREAM  10

struct sv_subtitle_service
{
  char lang[4];
  QBSubtitleType  subtitling_type;
  uint16_t  composition_page;
  uint16_t  ancillary_page;
};

struct sv_subtitle_stream
{
  int pid;
  int cnt;
  struct sv_subtitle_service  tab[MAX_SUBTITLES_IN_STREAM];
};


#define MAX_TELETEXT_IN_STREAM  10

struct sv_scte27_stream
{
    int pid;
    char lang[4];
};

struct sv_teletext_service
{
  char lang[4];
  uint8_t  type;
  uint8_t  magazine;
  uint8_t  page;
};

struct sv_teletext_stream
{
  int pid;
  char* lang; // text language. // malloc'ed. may be null.
  int cnt;
  struct sv_teletext_service  tab[MAX_TELETEXT_IN_STREAM];
};


struct ca_format
{
  int pid;
  int system_id;
  int es_pid; // -1 iff for all elementary streams
};


struct sv_data_format_ts
{
  enum svtsformattype type;

  int sid;     // <  0 if unknown
  int pmt_pid; // <= 0 if unknown
  int pcr_pid; // <= 0 if unknown

  // if non-zero, basic format detection was performed
  // and all (video_*, audio_*, ca*) fields are valid.
  short format_detected;

  SvBuf  pmt_section;

  int* video_pid;
  int* audio_pid;

  uint8_t scrambling_mode; // as seen in 0x65 descriptor in pmt

  short ca_cnt;
  short ca_cnt_max;
  struct ca_format* ca;
  // unique pids - extracted from ca array above
  short ca_pid_cnt;
  short ca_pid_cnt_max;
  int* ca_pids;

  short subtitle_cnt;
  short subtitle_cnt_max;
  struct sv_subtitle_stream** subtitle;

  short teletext_cnt;
  short teletext_cnt_max;
  struct sv_teletext_stream** teletext;

  short scte27_cnt;
  short scte27_cnt_max;
  struct sv_scte27_stream **scte27;

  short closed_caption_subs_cnt;
  struct sv_closed_caption_subs_service closed_caption_subs[4];
};

struct sv_data_format_es
{
  struct sv_extended_video_format* video;
  struct sv_extended_audio_format* audio;

  int txt_subs_cnt;
  int txt_subs_cnt_max;
  struct sv_txt_subs_stream* txt_subs;
};

struct sv_data_format_asfpl
{
  /// asf header from describe command, or starting chunk of an asf file.
  SvBuf  header;
  /// if non-zero, basic format detection was performed
  /// and all (video_*, audio_*) fields are valid.
  bool  format_detected;

  int packet_len;
  int video_id;
  int audio_id;
};

/**
 * Buffering properties.
 **/
struct sv_data_format_buffer_props
{
    bool syncAVFrames;          /// should sync AV frames
    int prefill_size;           /// number of bytes to be buffered before playback starts
    int32_t  prefill_duration;  /// duration in 90kHz units to be buffered before playback starts
};

struct svdataformat
{
  long int ref_cnt;
  struct svdataformat* source_format;
  enum svdataformattype type;

  // (t)ts may be wrapped in rfc-rtp
  // asfpl may be wrapped in ms-rtp
  // es    may be wrapped in sv-rtp
  enum svrtpformattype  rtp_format;

  struct sv_content_protection  content_protection;

  SvDrmSystemInfo drm_system_info;

  short video_cnt;
  short video_cnt_max;
  short audio_cnt;
  short audio_cnt_max;

  struct sv_simple_video_format* video;
  struct sv_simple_audio_format* audio;

  union {
    struct sv_data_format_ts  ts;
    struct sv_data_format_es  es;
    struct sv_data_format_asfpl  asfpl;
  };

  struct sv_data_format_buffer_props buffer_props;  /// buffering properties
};


#define svdataformat_has_video(_f_) ( (_f_)->video_cnt > 0 )
#define svdataformat_has_audio(_f_) ( (_f_)->audio_cnt > 0 )
#define svdataformat_video_codec(_f_, _n_) ( (_f_)->video[(_n_)].codec )
#define svdataformat_audio_fmt(_f_, _n_)   ( (_f_)->audio[(_n_)].fmt )

// Construction

struct svdataformat* svdataformat_alloc_(const char* file, const char* function, int line);
#if SV_LOG_LEVEL > 0
#define svdataformat_alloc()  svdataformat_alloc_( __FILE__, __FUNCTION__, __LINE__)
#else
#define svdataformat_alloc()  svdataformat_alloc_( "", "", 0 )
#endif

void svdataformat_add_video(struct svdataformat* format);
void svdataformat_add_audio(struct svdataformat* format);
void svdataformat_add_txt_subs(struct svdataformat* format);
void svdataformat_add_teletext(struct svdataformat* format, struct sv_teletext_stream* stream);
void svdataformat_add_sublitle(struct svdataformat* format, struct sv_subtitle_stream* stream);
void svdataformat_add_scte27(struct svdataformat* format, struct sv_scte27_stream* stream);
int svdataformat_add_closed_caption_subs(struct svdataformat* format, struct sv_closed_caption_subs_service* stream);

void svdataformat_add_ca(struct svdataformat* format, int pid, int system_id, int es_pid);

bool svdataformat_ts_has_es_pid(const struct svdataformat* format, int pid);

/** Set the type and perform basic initialization of corresponding fields.
 */
void svdataformat_set_type(struct svdataformat* format, enum svdataformattype type);

/// Calculates canonical form of video framerate (nom/denom) given approximate framerate.
/// @param fps1000  framerate multiplied by 1000
void svdataformat_set_video_fps(struct sv_extended_video_format* video_ex, int fps1000);


#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_DATA_FORMAT_H
