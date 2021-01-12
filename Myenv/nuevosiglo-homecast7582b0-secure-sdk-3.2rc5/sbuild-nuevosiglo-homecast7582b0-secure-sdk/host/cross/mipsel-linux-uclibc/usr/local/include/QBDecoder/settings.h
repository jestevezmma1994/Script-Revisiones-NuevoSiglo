/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_DECODER_SETTINGS_H
#define QB_DECODER_SETTINGS_H

#include <QBViewport.h>
#include <CAGE/Core/Sv2DRect.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <dataformat/sv_data_format_type.h>
#include <dataformat/audio.h>
#include <dataformat/QBContentSource.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBDecoderAudioOutputSettings
{
  QBAudioCodec  codec;
  bool  multichannel;

  bool  force_mute;

  uint32_t  latency; /// in ms

  // TODO: sync delay, ...
};

struct QBDecoderVideoOutputSettings
{
  QBContentDisplayMode  contentMode;
  QBViewportMode  viewportMode;
  Sv2DRect  rect;
};

#define QBDecoderMaxAudioOutputs  4
#define QBDecoderMaxVideoOutputs  4

struct QBDecoderOutputSettings
{
  struct svdataformat* format;
  /** source of content */
  QBContentSource source;

  struct {
    int  track;

    int volume;
    bool muted;

    struct QBDecoderAudioOutputSettings  outputs[QBDecoderMaxAudioOutputs];
  } audio;

  struct {
    int  track;
    bool blocked;

    struct QBDecoderVideoOutputSettings  outputs[QBDecoderMaxVideoOutputs];
  } video;
};

struct QBDecoderWantedSettings
{
  struct svdataformat* format;
  /** source of content */
  QBContentSource source;

  struct {
    int  track;

    int volume;
  } audio;

  struct {
    int  track;
    bool blocked;

    QBContentDisplayMode  contentModes[QBDecoderMaxVideoOutputs];//TODO: move to platform hal
  } video;
};


void QBDecoderExpandWantedSettings(int debug_id,
                                   const struct QBDecoderWantedSettings* wanted,
                                   struct QBDecoderOutputSettings* curr,
                                   int videoOutputCnt, int audioOutputCnt,
                                   bool mutedVideo, bool mutedAudio);

bool QBDecoderAudioOutputSettingsIsChanged(int debug_id,
                                           const struct QBDecoderAudioOutputSettings* oldTab,
                                           const struct QBDecoderAudioOutputSettings* newTab, int cnt);

#ifdef __cplusplus
}
#endif


#endif // #ifndef QB_DECODER_SETTINGS_H
