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

#ifndef DRM_STREAM_INTERNAL_H
#define DRM_STREAM_INTERNAL_H

#include <SvDRM/SvDRMStream.h>
#include <SvDRM/SvDRMPlugin.h>

#include <intrusive_list.h>
#include <SvFoundation/SvArray.h>
#include <SvPlayerKit/SvDRMInfo.h>
#include <SvPlayerKit/SvContentConstraints.h>
#include <SvPlayerKit/SvMemCounter.h>
#include <SvPlayerKit/SvChbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SvDRMStream_s
{
  intrusive_list  lst; // node of: list of all streams in plugin.

  const struct drm_stream_handlers* handlers;
  void* owner;

  int my_id;
  SvDRMPlugin  plug;

  int alignment;
  SvDRMInfo  drm_info;
  SvContentConstraints  constraints;
  SvMemCounter  mem_counter;

  struct SvChbuf_s  enc_data;
  struct SvChbuf_s  dec_data; /// data already decrypted. it should be given back to "owner" asap.
  short eos;
  short setup_done;
  bool sw_enabled;

  SvArray  enc_ecm_tab;
  SvArray  dec_ecm_tab;

  void* plug_prv; // managed by specific SvDRMPlugin instance
};

// HIDDEN
int SvDRMStreamSetupDone(SvDRMStream stream);
// HIDDEN
void SvDRMStreamError(SvDRMStream stream, int err);

static inline int drm_stream_can_decrypt(SvDRMStream stream)
{
  SvDRMPlugin plug = stream->plug;
  int data_size = stream->enc_data.len;
  return data_size > 0 &&
        (stream->eos || data_size >= plug->decrypt_chunk_size);
};

#ifdef __cplusplus
}
#endif

#endif // #ifndef DRM_STREAM_INTERNAL_H
