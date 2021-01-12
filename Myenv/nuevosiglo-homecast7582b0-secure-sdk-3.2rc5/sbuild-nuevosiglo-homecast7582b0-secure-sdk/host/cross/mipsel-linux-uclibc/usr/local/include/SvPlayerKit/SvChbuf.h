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

#ifndef SV_CHBUF_H_
#define SV_CHBUF_H_

#include <stdint.h>
#include "SvBuf.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SvChbuf_s;
typedef struct SvChbuf_s* SvChbuf;


struct SvChbuf_s {
  unsigned int len;
  unsigned int buf_cnt;

  SvBuf first;
  SvBuf last;
};

#define SvChbufPrint(CHB, MESS) \
  for (;;) { \
    SvChbuf _chb = (CHB); \
    const char* _mess = (MESS); \
    SvLogNotice("SvChbufPrint(%p) : cnt=%u, len=%u  [%s]", _chb, _chb->buf_cnt, _chb->len, _mess); \
    SvBuf _curr = _chb->first; \
    for (; _curr; _curr = _curr->next) \
      SvLogNotice("  SvBuf=%p : data=%p, len=%u", _curr, _curr->data, _curr->len); \
    break; \
  };

void SvChbufInit(SvChbuf cb);
void SvChbufClear(SvChbuf cb);

SvBuf SvChbufFront(SvChbuf cb);
SvBuf SvChbufPopFront(SvChbuf cb);
SvBuf SvChbufPopBack(SvChbuf cb);
void SvChbufPushFront(SvChbuf cb, SvBuf sb);
void SvChbufPushBack(SvChbuf cb, SvBuf sb);
void SvChbufTransfer(SvChbuf target, SvChbuf source);

void SvChbufWrite(SvChbuf cb, unsigned int off, void* buf, unsigned int len);

void SvChbufSetLen_(SvChbuf cb, unsigned int len);
void SvChbufAppend_(SvChbuf cb, void* buf, unsigned int len);

/** Cuts "big_buf" into pieces of length not larger than "chunk_len" and pushes them into "cb".
 */
void SvChbufCut(SvChbuf cb, SvBuf big_buf, unsigned int chunk_len);

void SvChbufTruncateFront(SvChbuf cb, unsigned int to_skip);

#define svchbuf_wrap_decl(name, buf, len) \
  struct SvChbuf_s name; \
  struct SvBuf_s name##_svbuf_tmp; \
  do { \
    SvBufCreateStatic(&name##_svbuf_tmp, (buf), (len)); \
    SvChbufInit(&name); \
    SvChbufPushBack(&name, &name##_svbuf_tmp); \
  } while (0)


struct SvChbufCtx {
  const struct SvBuf_s* curr;
  unsigned int off;
  unsigned int left_len;
};

void SvChbufCtxInit(struct SvChbufCtx* ctx, const struct SvChbuf_s* cb);
void SvChbufCtxCopy(struct SvChbufCtx* ctx, void* dest_, unsigned int len);
void SvChbufCtxSkip(struct SvChbufCtx* ctx, unsigned int len);

#define SvChbufCopy(CHB, DEST, LEN) for (;;) { \
    struct SvChbufCtx _ctx_; \
    SvChbufCtxInit(&_ctx_, CHB); \
    SvChbufCtxCopy(&_ctx_, DEST, LEN); \
    break; \
}

#define SvChbufCopyWithOffset(CHB, DEST, LEN, OFFSET) for (;;) { \
    struct SvChbufCtx _ctx_; \
    SvChbufCtxInit(&_ctx_, CHB); \
    SvChbufCtxSkip(&_ctx_, OFFSET); \
    SvChbufCtxCopy(&_ctx_, DEST, LEN); \
    break; \
}

SvBuf SvBufCreateFromChbuf(const struct SvChbuf_s* cb, uint8_t purpose, uint32_t app_flags);
SvChbuf SvBufGetChbufData(SvBuf sb);
uint32_t* SvBufGetChbufFlags(SvBuf sb);


#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_CHBUF_H
