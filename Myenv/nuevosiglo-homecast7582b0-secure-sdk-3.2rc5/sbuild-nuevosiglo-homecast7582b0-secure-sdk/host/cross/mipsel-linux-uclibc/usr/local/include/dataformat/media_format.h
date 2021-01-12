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

#ifndef media_format_h
#define media_format_h

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <SvPlayerKit/SvBuf.h>
#include <SvPlayerKit/SvChbuf.h>
#include <SvPlayerKit/SvMemCounter.h>
#include <intrusive_list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _stream_t {
  VIDEO_STREAM,
  AUDIO_STREAM,
  APPLICATION_STREAM,
  AGGREGATE_STREAM,
} stream_t;


typedef struct __attribute__ ((packed)) dmx_media_frame
{
  union { // note: this has to be the first member (otherwise change "dmx_frame_from_list" below)
    intrusive_list lst;
    struct {
      struct dmx_media_frame *next;
      struct dmx_media_frame *prev;
    } frm;
  };
  unsigned int frame_type;          //AUDIO /VIDEO or somthing else
  int stream_id;                    // n-th video or n-th audio. undefined when transport stream.

  float pts,npt;                    // frame PTS and stream NPT (start time)
  uint32_t real_pts, real_dts;
  int iskeyframe;                   // frame is a keyframe
  int lostframes;                   // lost frames before this frame
  int dropframe;                    // frame will be dropped
  int speed;                        // frame playback speed (for real pos.)
  int sid,seq;                      // session ID/total seqno
  unsigned int hseq;                // sequence nr of the packet this frame starts in
  unsigned int tseq;                // sequence nr of the packet this frame ends in
  int nonref;                       // non-reference frame (may be dropped)
  int64_t stream_offset;            // frame offset in stream

  struct SvChbuf_s chdata;
} dmx_frame;

inline static dmx_frame* dmx_frame_from_list(intrusive_list *lst) {
  return (dmx_frame*)lst;
};


static inline int free_frame(dmx_frame *frame)
{
  if (!frame) return 0;
  SvChbufClear(&frame->chdata);
  free(frame);
  return 1;
}

static inline dmx_frame* _new_frame(int size, const unsigned char *data, SvMemCounter mem_counter, const char *file, int line)
{
  dmx_frame *f = malloc(sizeof(dmx_frame));
  if (!f)
    return NULL;
  memset((char *)f, 0, sizeof(dmx_frame));

  SvChbufInit(&f->chdata);

  if (size) {
    // make it one big buffer.
    SvBuf sb = SvBufCreateInternal(size, mem_counter);
    if (!sb) {
      free_frame(f);
      return NULL;
    };
    if (data)
      memcpy(sb->data, data, size);
    SvChbufPushBack(&f->chdata, sb);
  };
  return f;
}
#define new_frame(size, data, mem_counter) _new_frame(size, data, mem_counter, __FILE__, __LINE__)

#if 0
static inline dmx_frame* empty_frame(unsigned int frame_type, int iskeyframe, float pts) {
  dmx_frame *frame = new_frame(0, NULL, 0);
  if (frame != NULL) {
    frame->frame_type = frame_type;
    frame->iskeyframe = iskeyframe;
    frame->pts = pts;
    frame->real_pts = frame->real_dts = (uint32_t)(uint64_t)((double)frame->pts * 90000.0);
    frame->iv_present = 0;
    frame->isencryptedframe = 0;
  }
  return frame;
}

#define dump_frame(f, frame, printContents) do {} while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif
