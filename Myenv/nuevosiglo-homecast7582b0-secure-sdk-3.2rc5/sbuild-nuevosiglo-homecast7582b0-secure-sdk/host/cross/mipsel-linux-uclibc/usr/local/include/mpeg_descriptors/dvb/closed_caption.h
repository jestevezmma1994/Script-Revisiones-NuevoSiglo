/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef FILE_MPEG_CLOSED_CAPTION_DESC_H
#define FILE_MPEG_CLOSED_CAPTION_DESC_H

#include <mpeg_psi_parser.h>
#include <dataformat/subtitle.h>

typedef struct _mpeg_closed_caption_desc  mpeg_closed_caption_desc;

struct _mpeg_closed_caption_desc
{
  mpeg_data* data;
};

struct mpeg_closed_caption_desc_service
{
  char lang[4];
  QBSubtitleType  subtitling_type;
  union {
     uint16_t field;
     uint16_t service_number;
  };
  bool easy_reader;
  bool wide_aspect_ratio;
};

int mpeg_closed_caption_desc_init(mpeg_closed_caption_desc* desc, mpeg_data* data);
void mpeg_closed_caption_desc_destroy(mpeg_closed_caption_desc* desc);
int mpeg_closed_caption_desc_cnt(const mpeg_closed_caption_desc* desc);
void mpeg_closed_caption_desc_get_service(const mpeg_closed_caption_desc* desc, int i, struct mpeg_closed_caption_desc_service* out);
void mpeg_closed_caption_desc_report(const mpeg_closed_caption_desc* desc, const char* indent_string);


#endif
/* FILE_MPEG_CLOSED_CAPTION_DESC_H */
