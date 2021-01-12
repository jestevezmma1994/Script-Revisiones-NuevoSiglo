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

#ifndef SV_BUF_H_
#define SV_BUF_H_

#include "SvMemCounter.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SvBuf_s;
typedef struct SvBuf_s* SvBuf;

typedef void (svbuf_destroy_fun) (struct SvBuf_s*);

/// SvBuf can freely modify data/len variables (those are not used in destructor).
#define SVBUF_FLAG__PTR_MOVABLE  (0x1)
/// SvBuf position is wrapping around, using only 33 bits (used only for timestamps carried in pos
#define SVBUF_FLAG__POSITION_WRAPPED_33BITS  (0x2)
/// SvBuf can't be used in PVR
#define SVBUF_FLAG__NOT_RECORDABLE (0x4)
/// SvBuf can't be used in timieshift
#define SVBUF_FLAG__NOT_TIMESHIFTABLE (0x8)

/// This SvBuf contains pure data
#define SVBUF_PURPOSE__PURE_DATA  0x00

/// This empty SvBuf is a marker of discontinuity in the stream.
#define SVBUF_PURPOSE__DISC       0x01
/// This empty SvBuf is an ECM bearer.
#define SVBUF_PURPOSE__ECM        0x02
/// This empty SvBuf is a PMT bearer (in the form of svdataformat).
#define SVBUF_PURPOSE__PMT        0x03
/// This empty SvBuf is an epg event bearer (in the form of SvEPGEvent).
#define SVBUF_PURPOSE__EVENT      0x04
/// This empty SvBuf is an PCR bearer (in the "pos" field).
#define SVBUF_PURPOSE__PCR        0x05
/// This empty SvBuf contains SvSampleEncryptionInfo object
#define SVBUF_PURPOSE__KEY        0x06

/// This empty SvBuf has SvChbuf stored inside
#define SVBUF_PURPOSE__STORE      0x10

/// Same as STORE, but it is marked to contain FRAME_PROPETIES as first SvBuf
#define SVBUF_PURPOSE__FRAME              0x20
/// This empty SvBuf keeps "frame_properties" structure.
#define SVBUF_PURPOSE__FRAME_PROPERTIES   0x21

#define SVBUF_PURPOSE__DRM_SAMPLE_INFO   0x41

struct SvBuf_s {
  // or any other intrusive list.
  struct SvBuf_s* next;

  unsigned char* data;
  unsigned int len;

  int16_t ref_cnt;
  uint8_t flags;
  uint8_t purpose;

  /// files position (in bytes), or content position (in 90kHz units)
  /// -1 iff unknown
  uint64_t  pos;

  svbuf_destroy_fun* destroy_fun;
  void* destroy_arg;
  SvMemCounter mem_counter;
};

void SvBufReportRelease(SvBuf sb);

__attribute__((unused))
static inline void SvBufRetain(SvBuf sb)
{
  sb->ref_cnt++;
};
__attribute__((unused))
static inline void SvBufRelease(SvBuf sb)
{
  sb->ref_cnt--;
  //SvBufReportRelease(sb);
  if (sb->ref_cnt == 0) {
    sb->destroy_fun(sb);
  };
};

SvBuf SvBufCreate(unsigned char* buf, unsigned int len,
                  svbuf_destroy_fun* destroy_fun, void* destroy_arg,
                  SvMemCounter mem_counter);
// will be released with free().
SvBuf SvBufCreateExternal(unsigned char* buf, unsigned int len, SvMemCounter mem_counter);
SvBuf SvBufCreateInternal(unsigned int len, SvMemCounter mem_counter);
SvBuf SvBufCreateAligned(unsigned int len, unsigned int align, SvMemCounter mem_counter);

SvBuf SvBufCreateStatic(SvBuf sb, unsigned char* buf, unsigned int len);

SvBuf SvBufCreateSub(SvBuf sb, unsigned int off, unsigned int len);
// copies data using CreateInternal.
SvBuf SvBufDeepCopy(SvBuf sb, SvMemCounter mem_counter);

SvBuf SvBufShallowCopy(SvBuf sb);

__attribute__((unused))
static inline SvBuf SvBufChangeToSub(SvBuf sb, unsigned int off, unsigned int len)
{
  if (!(sb->flags & SVBUF_FLAG__PTR_MOVABLE) || sb->ref_cnt > 1) {
    SvBuf sb2 = SvBufCreateSub(sb, off, len);
    SvBufRelease(sb);
    return sb2;
  } else {
    sb->data += off;
    sb->len = len;
    return sb;
  };
};

#if 0
void SvBufFree(SvBuf sb);
#else
static inline void SvBufFree(SvBuf sb)
{
  free(sb);
};
#endif

#ifdef __cplusplus
}
#endif

#endif // #ifndef SVBUF_H
