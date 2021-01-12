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

#ifndef MPEG_EAC3_DESC_H
#define MPEG_EAC3_DESC_H

#include <stdint.h>
#include <mpeg_psi_parser.h>

typedef struct _mpeg_eac3_desc  mpeg_eac3_desc;

typedef enum {
  eac3_service_unknown = 0,
  eac3_service_complete = 8,
  eac3_service_music_and_effects = 1,
  eac3_service_visually_impaired = 2,
  eac3_service_hearing_impaired = 3,
  eac3_service_dialogue = 4,
  eac3_service_commentary = 5,
  eac3_service_emergency = 14,
  eac3_service_voiceover = 7,
  eac3_service_karaoke = 15
} eac3_service_type;

typedef enum {
  eac3_mono = 0,
  eac3_dual_mono = 1,
  eac3_stereo = 2,
  eac3_dolby_stereo = 3, // Dolby Surround encoded in stereo
  eac3_multichannel = 4, // more than 2 channels
  eac3_multichannel_ex = 5, // more than 5.1 channels, only in E-AC3
  eac3_multiple_substreams = 6, // only in E-AC3
} eac3_channels_cfg;

struct _mpeg_eac3_desc
{
  bool component_type_valid;
  bool is_eac3;
  bool is_full_service;
  eac3_service_type service_type;
  eac3_channels_cfg channels;

  bool bsid_valid;
  short bsid;

  bool mainid_valid;
  short mainid;

  bool asvc_valid;
  short asvc;
};

int mpeg_eac3_desc_init(mpeg_eac3_desc* eac3, mpeg_data* data);
void mpeg_eac3_desc_destroy(mpeg_eac3_desc* eac3);

void mpeg_eac3_desc_report(const mpeg_eac3_desc* eac3, const char* indent_string);

#endif // MPEG_EAC3_DESC_H
