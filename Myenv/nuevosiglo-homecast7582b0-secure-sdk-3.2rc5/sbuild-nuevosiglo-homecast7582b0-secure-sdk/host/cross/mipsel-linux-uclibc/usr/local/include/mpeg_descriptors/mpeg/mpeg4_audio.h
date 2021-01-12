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
****************************************************************************/

#ifndef MPEG_MPEG4_AUDIO_DESC_H
#define MPEG_MPEG4_AUDIO_DESC_H

#include <mpeg_psi_parser.h>
#include <dataformat/audio.h>

typedef struct _mpeg_mpeg4_audio_desc  mpeg_mpeg4_audio_desc;

enum AAC_component_type
{
  AAC_HE_mono = 0x01,
  AAC_HE_stereo = 0x03,
  AAC_HE_surround = 0x05,
  AAC_HE_visually_impaired = 0x40,
  AAC_HE_hard_of_hearing = 0x41,
  AAC_HE_supplementary_receiver_mix = 0x42,
  AAC_HE_v2_stereo = 0x43,
  AAC_HE_v2_visually_impaired = 0x44,
  AAC_HE_v2_hard_of_hearing = 0x45,
  AAC_HE_v2_supplementary_receiver_mix = 0x46,
  AAC_HE_visually_impaired_receiver_mix = 0x47,
  AAC_HE_visually_impaired_broadcaster_mix = 0x48,
  AAC_HE_v2_visually_impaired_receiver_mix = 0x49,
  AAC_HE_v2_visually_impaired_broadcaster_mix = 0x4a,
};

struct _mpeg_mpeg4_audio_desc
{
  int mode_;

  int profile;
  int level;

  bool AAC_type_present;
  int AAC_type;
};

int mpeg_mpeg4_audio_desc_init(mpeg_mpeg4_audio_desc* desc, mpeg_data* data);
void mpeg_mpeg4_audio_desc_destroy(mpeg_mpeg4_audio_desc* desc);

QBAudioCodec mpeg_mpeg4_audio_desc_get_codec(const mpeg_mpeg4_audio_desc* desc);

void mpeg_mpeg4_audio_desc_report(const mpeg_mpeg4_audio_desc* desc, const char* indent_string);

#endif // MPEG_MPEG4_AUDIO_DESC_H
