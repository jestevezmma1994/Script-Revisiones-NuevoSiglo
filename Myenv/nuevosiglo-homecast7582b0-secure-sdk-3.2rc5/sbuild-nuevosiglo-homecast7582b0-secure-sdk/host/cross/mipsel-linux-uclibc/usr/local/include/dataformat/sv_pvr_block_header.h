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

#ifndef SV_PVR_BLOCK_HEADER_H
#define SV_PVR_BLOCK_HEADER_H

#include <SvCore/SvTime.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STORAGE_PLUGIN_BLOCK_SIZE   (248*1024)
#define STORAGE_PLUGIN_BLOCK_ALIGN    (4*1024)

// only relative values are meaningful.
struct sv_pvr_time
{
  unsigned int  sec;
  unsigned int  usec;
};

static inline struct sv_pvr_time  sv_time_to_pvr_time(SvTime t)
{
  struct sv_pvr_time res = {
    .sec  = SvTimeGetSeconds(t),
    .usec = SvTimeGetMicroseconds(t),
  };
  return res;
};
static inline SvTime pvr_time_to_sv_time(struct sv_pvr_time t)
{
  return SvTimeConstruct(t.sec, t.usec);
};

#define PRV_BLOCK_HEADER_MAGIC  (0xb10cf3ad)

#define SV_PVR_BLOCK_MAX_OFFSETS  8

struct sv_pvr_block_header
{
  unsigned int  magick;
  unsigned int  data_offset;
  unsigned int  data_len;
  struct sv_pvr_time  first_packet_time;
  struct sv_pvr_time  last_packet_time;
  unsigned int  offset_cnt;
  unsigned int  offset_tab[SV_PVR_BLOCK_MAX_OFFSETS];
};

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_PVR_BLOCK_HEADER_H
