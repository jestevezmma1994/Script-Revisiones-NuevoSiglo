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

#ifndef SV_DATA_FORMAT_DETECT_H
#define SV_DATA_FORMAT_DETECT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct svdataformat;
struct SvChbuf_s;

int mpeg_audio_header_size(void);
unsigned char mpeg_audio_sync_byte(void);
int mpeg_audio_test_frame_header(const unsigned char* buf,
                                 int* version_out, int* layer_out,
                                 int* bitrate_out, int* sample_rate_out, int* channels_out,
                                 int* frame_len_out);

int ac3_header_size(void);
unsigned char ac3_sync_byte(void);
int ac3_audio_test_frame_header(const unsigned char* buf,
                                int* version_out,
                                int* bitrate_out, int* sample_rate_out, int* channels_out,
                                int* frame_len_out);

struct svdataformat* sv_detect_ts_format(const struct SvChbuf_s* chb, unsigned int* header_size_out);
struct svdataformat* sv_detect_wav_format(const struct SvChbuf_s* chb, unsigned int* header_size_out);
struct svdataformat* sv_detect_mpeg_audio_format(const struct SvChbuf_s* chb, unsigned int* header_size_out);
struct svdataformat* sv_detect_ac3_audio_format(const struct SvChbuf_s* chb, unsigned int* header_size_out);
struct svdataformat* sv_detect_aac_format(const struct SvChbuf_s* chb, unsigned int* header_size_out);
bool sv_detect_mkv_format(const struct SvChbuf_s* chb);
bool sv_detect_avi_format(const struct SvChbuf_s* chb);
bool sv_detect_flv_format(const struct SvChbuf_s* chb);
bool sv_detect_id3_format(const struct SvChbuf_s* chb, unsigned int* header_size_out);
bool sv_detect_hls_format(const struct SvChbuf_s* chb);

bool sv_detect_mp4_format(const struct SvChbuf_s* chb);
bool sv_detect_mp4_fragment_format(const struct SvChbuf_s* chb);


#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_DATA_FORMAT_DETECT_H
