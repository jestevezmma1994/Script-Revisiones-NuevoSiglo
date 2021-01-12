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

#ifndef MPEG_DTS_DESC_H
#define MPEG_DTS_DESC_H

#include <stdint.h>
#include <mpeg_psi_parser.h>

typedef struct _mpeg_dts_desc  mpeg_dts_desc;

typedef enum {
  extended_surround_none = 0,
  extended_surround_matrixed = 1,
  extended_surround_discrete = 2
} extended_surround;

struct _mpeg_dts_desc
{
  int sample_rate;

  int bitrate;
  bool bitrate_is_open;
  bool bitrate_is_variable;
  bool bitrate_is_lossless;

  int surround_mode;
  extended_surround extended_surround_mode;

  struct {
    uint8_t normal; // 1 to 3
    uint8_t surround; // 0 to 2
    uint8_t lfe; // 0 or 1
  } channels;
};

int mpeg_dts_desc_init(mpeg_dts_desc* dts, mpeg_data* data);
void mpeg_dts_desc_destroy(mpeg_dts_desc* dts);

void mpeg_dts_desc_report(const mpeg_dts_desc* dts, const char* indent_string);

#endif // MPEG_DTS_DESC_H
