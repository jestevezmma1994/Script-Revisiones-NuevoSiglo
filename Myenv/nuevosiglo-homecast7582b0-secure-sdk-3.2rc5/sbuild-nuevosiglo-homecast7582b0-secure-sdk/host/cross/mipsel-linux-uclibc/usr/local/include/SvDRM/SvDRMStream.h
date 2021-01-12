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

#ifndef DRM_STREAM_H
#define DRM_STREAM_H

#include "types_fwd.h"

#include <fibers/c/fibers.h>
#include <SvFoundation/SvArray.h>
#include <SvPlayerKit/SvChbuf.h>
#include <SvPlayerKit/SvDRMInfo.h>
#include <SvPlayerKit/SvECM.h>

#ifdef __cplusplus
extern "C" {
#endif

struct drm_stream_handlers
{
  void (*error)                (void* owner, SvDRMStream stream, int error);
  /// aka "finished setup"
  int  (*ready)                (void* owner, SvDRMStream stream);
  /// \a data  is not to be freed, but contained buffers should be transfered from it.
  void (*decrypted_data_ready) (void* owner, SvDRMStream stream, SvChbuf data);
  void (*eos) (void* owner, SvDRMStream stream);
  /// \a ecm_tab  elements:SvECMControlWord. is not to be released, but contained elements should be transfered from it.
  void (*decrypted_ecms_ready) (void* owner, SvDRMStream stream, SvArray ecm_tab);
  /// software decryption should be used with this stream.
  int (*sw_enabled) (void* owner, SvDRMStream stream);
};

struct drm_stream_params
{
  const struct drm_stream_handlers* handlers;
  void* owner;

  SvFiberGroup    fiber_group; /// can be null
  SvMemCounter    mem_counter;

  SvDRMManager    manager; /// can be null, then, a global manager will be used.
  SvDRMInfo       drm_info;
};


int SvDRMStreamCreate(const struct drm_stream_params *params, SvDRMStream* stream_out);
void SvDRMStreamDestroy(SvDRMStream stream);

/** Enable software decryption mode.
 *  To be used at initalization time, when there are no ECMs in the stream.
 */
void SvDRMStreamEnableSWMode(SvDRMStream stream);

bool SvDRMStreamHasSWDecryption(SvDRMStream stream);
bool SvDRMStreamHasECMDecryption(SvDRMStream stream);

/// Stores the data internally and starts the decryption procedure.
/// Buffers contained in \a data will be transfered, but bytes themselves
/// will still be treated as "const".
/// Later, a decrypted_data_ready() callback will be called.
void SvDRMStreamPushData(SvDRMStream stream, SvChbuf data);

/// Informs the stream, that all pending data should be decrypted as soon
/// as possible - since no more data is comming, any data-gathering (into
/// larger chunks) will not work anymore.
/// If at any later time a caller would want to push data anyway, he can do so
/// freely - then, the eos state is revoked.
void SvDRMStreamEos(SvDRMStream stream);

/// Stops ongoing decryption (if any) and releases all gathered buffers.
void SvDRMStreamDropData(SvDRMStream stream);

/// Informs the stream, that it has to reduce its memory consumption by
/// dropping some data.
/// \returns amount of bytes released.
int SvDRMStreamFlushData(SvDRMStream stream);


/// Adds an ECM for decryption (if needed).
/// Decrypted ECM will be returned in decrypted_ecms_ready() callback.
/// \param ecmsec  not retained yet, read-only
void SvDRMStreamPushECM(SvDRMStream stream, SvECMSection ecmsec);


int SvDRMStreamGetId(const struct SvDRMStream_s* stream);

#ifdef __cplusplus
}
#endif

#endif // #ifndef DRM_STREAM_H
