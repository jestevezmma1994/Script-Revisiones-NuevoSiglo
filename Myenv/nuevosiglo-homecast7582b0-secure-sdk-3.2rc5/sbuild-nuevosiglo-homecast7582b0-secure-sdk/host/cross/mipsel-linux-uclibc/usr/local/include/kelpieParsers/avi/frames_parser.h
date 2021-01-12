/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef KPPARSERS_AVI_FRAMES_PARSER_H
#define KPPARSERS_AVI_FRAMES_PARSER_H

#include "types.h"
#include "../external/ra_input.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kpavi_frames_parser;

SvExport
struct kpavi_frames_parser* kpavi_frames_parser_create(const struct kpavi_info* info, kp_ra_input* input);
SvExport
void kpavi_frames_parser_destroy(struct kpavi_frames_parser* parser);

/// @returns  true iff eos is reached immediately
SvExport
bool kpavi_frames_parser_seek(struct kpavi_frames_parser* parser, uint64_t* pos90k);

SvExport
void kpavi_frames_parser_set_trick_mode(struct kpavi_frames_parser* parser, bool active, bool forward, double speed);


struct kpavi_frames_parser_frame_desc
{
  uint8_t stream_id;
  uint32_t size;

  uint64_t pts90k;
};

/// Call when input is set to proper position (start of metadata of next frame).
/// @returns KPAVI_RES__*
SvExport
int kpavi_frames_parser_get_next(struct kpavi_frames_parser* parser, struct kpavi_frames_parser_frame_desc* desc);

#ifdef __cplusplus
}
#endif

#endif // #ifndef KPPARSERS_AVI_FRAMES_PARSER_H
