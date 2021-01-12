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

#ifndef SV_TS_STREAM_TYPE_H
#define SV_TS_STREAM_TYPE_H

#include "sv_data_format.h"

#ifdef __cplusplus
extern "C" {
#endif

enum sv_ts_stream_type
{
  sv_ts_stream_type__pure_ts  = 0x1,
  sv_ts_stream_type__pure_tts = 0x2,
  sv_ts_stream_type__rtp_ts   = 0x4,
  sv_ts_stream_type__rtp_tts  = 0x8,
};
#define sv_ts_stream_type_all_ts  (sv_ts_stream_type__pure_ts | sv_ts_stream_type__pure_tts)
#define sv_ts_stream_type_all_rtp (sv_ts_stream_type__rtp_ts | sv_ts_stream_type__rtp_tts)

#define sv_ts_stream_type_all (sv_ts_stream_type_all_ts | sv_ts_stream_type_all_rtp)

int sv_detect_ts_stream_type(const unsigned char* buf, unsigned int len, int packet_len, int possible_formats, unsigned int* offset_out);

#define sv_ts_stream_type_is_valid(flags) ({ \
    int bit_cnt = 0; \
    if ((flags) & sv_ts_stream_type__pure_ts)  bit_cnt++; \
    if ((flags) & sv_ts_stream_type__pure_tts) bit_cnt++; \
    if ((flags) & sv_ts_stream_type__rtp_ts)   bit_cnt++; \
    if ((flags) & sv_ts_stream_type__rtp_tts)  bit_cnt++; \
    (bit_cnt == 1); \
  })

#define sv_ts_stream_type_is_rtp_known(flags) ({ \
    int bit_cnt = 0; \
    if ((flags) & sv_ts_stream_type__pure_ts || \
        (flags) & sv_ts_stream_type__pure_tts) bit_cnt++; \
    if ((flags) & sv_ts_stream_type__rtp_ts  || \
        (flags) & sv_ts_stream_type__rtp_tts)  bit_cnt++; \
    (bit_cnt == 1); \
  })

#define sv_ts_stream_type_is_rtp(flags) ({ \
    int bit_cnt = 0; \
    if ((flags) & sv_ts_stream_type__rtp_ts  || \
        (flags) & sv_ts_stream_type__rtp_tts)  bit_cnt++; \
    (bit_cnt == 1); \
  })

#define sv_ts_stream_type_is_ts(flags) ({ \
    int bit_cnt = 0; \
    if ((flags) & sv_ts_stream_type__pure_ts  || \
        (flags) & sv_ts_stream_type__rtp_ts)  bit_cnt++; \
    (bit_cnt == 1); \
  })

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_TS_STREAM_TYPE_H
