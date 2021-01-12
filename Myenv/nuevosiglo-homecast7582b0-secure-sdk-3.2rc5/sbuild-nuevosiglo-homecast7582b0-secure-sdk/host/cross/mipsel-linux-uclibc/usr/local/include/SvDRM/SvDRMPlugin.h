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

#ifndef DRM_PLUGIN_H
#define DRM_PLUGIN_H

#include "types_fwd.h"

#include <fibers/c/fibers.h>
#include <intrusive_list.h>

#ifdef __cplusplus
extern "C" {
#endif

struct drm_plugin_params;

struct drm_plugin_functor
{
  /// check if internal initialization has finished.
  /// \returns WOULDBLOCK, FINALIZE or error.
  int  (*get_init_status) (SvDRMPlugin pl);
  /// must call SvDRMPluginDeinit() inside.
  void (*destroy) (SvDRMPlugin pl);

  int  (*set_opt) (SvDRMPlugin pl, int option, void* value);

  /// starts setup process of  \a stream, and allocates stream's internal structures.
  /// also, check status of the setup process, if it is already in progress.
  /// \returns WOULDBLOCK, TIMEBARRIER, FINALIZE or error.
  /// When finished (or failed), plugin internals (subclass) will wake up our fiber,
  /// so we can check on the progress of the setup operation.
  int  (*setup_stream)   (SvDRMPlugin pl, SvDRMStream stream);

  /// take ecms from \a stream->enc_ecm_tab, increasing \a stream->pushed_ecms_cnt accordingly.
  /// also, check status of previously pushed ECMs, and return decrypted ecms in \a stream->dec_ecm_tab.
  /// \returns WOULDBLOCK, TIMEBARRIER
  /// \returns INCOMPATIBLE  when software decryption should be used, use decrypt_data() from now on.
  /// \returns error.
  /// When finished (or failed), plugin internals (subclass) will wake up our fiber,
  /// so we can check on the progress of the decryption process.
  int  (*decrypt_ecms)   (SvDRMPlugin pl, SvDRMStream stream);

  /// processes data from \a stream->enc_data into \a stream->dec_data.
  /// this function MUST be set to null iff software decryption is not supported
  /// \returns WOULDBLOCK, TIMEBARRIER or an error code.
  int  (*decrypt_data) (SvDRMPlugin pl, SvDRMStream stream);

  /// always succeeds.
  void (*reset_decryption) (SvDRMPlugin pl, SvDRMStream stream);
  /// always succeeds.
  /// \returns amount of bytes flushed or an error code.
  int  (*flush_data) (SvDRMPlugin pl, SvDRMStream stream);

  /// deallocates stream's internal structures.
  void (*close_stream)   (SvDRMPlugin pl, SvDRMStream stream);
};


enum drm_plug_op_type
{
  drm_plug_op_type__setup,
  //drm_plug_op_type__decrypt,
  //drm_plug_op_type__close,
};

struct drm_plug_op
{
  intrusive_list  lst; // in drm plugin.

  SvDRMStream  stream;
  enum drm_plug_op_type  op_type;
};


struct SvDRMPlugin_s
{
  intrusive_list  lst; // in drm manager.
  SvDRMManager  manager;

  const char* name;
  int drm_id;
  const struct drm_plugin_functor* functor;
  int decrypt_chunk_size;

  short is_ready; // 0 until get_init_status() returns FINALIZE.
  short is_broken; // 1 iff init failed.
  int stream_cnt;
  intrusive_list  stream_lst; // SvDRMStream
  SvDRMStream  active_stream;

  // scheduled operations.
  intrusive_list  op_lst; // struct drm_plug_op*

  SvFiber  fiber;
};


struct drm_plugin_params
{
  const char* name;
  int  drm_id;
  int  decrypt_chunk_size;
  SvDRMManager  manager; /// can be null - then, a global manager will be used.

  SvFiberGroup  fiber_group; /// can be null - then, a new, independent group will be created.
};

void SvDRMPluginDestroy(SvDRMPlugin pl);
void SvDRMPluginSetOpt(SvDRMPlugin pl, int option, void* value);

/// internal - only for plugin writers
void SvDRMPluginInit(SvDRMPlugin pl, const struct drm_plugin_params* params, const struct drm_plugin_functor* functor);
/// internal - only for plugin writers
void SvDRMPluginDeinit(SvDRMPlugin pl);


/// HIDDEN
void SvDRMPluginSetupStream(SvDRMPlugin pl, SvDRMStream stream);
/// HIDDEN
void SvDRMPluginRemoveStream(SvDRMPlugin pl, SvDRMStream stream);
/// HIDDEN
void _SvDRMPluginActivate(SvDRMPlugin pl, const char* reason);

#ifdef __cplusplus
}
#endif

#endif // #ifndef DRM_PLUGIN_H
