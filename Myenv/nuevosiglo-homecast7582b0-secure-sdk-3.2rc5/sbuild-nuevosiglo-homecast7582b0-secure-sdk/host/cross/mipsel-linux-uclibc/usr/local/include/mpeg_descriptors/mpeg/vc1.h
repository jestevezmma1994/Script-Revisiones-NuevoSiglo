/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
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

#ifndef FILE_MPEG_VC1_DESC_H
#define FILE_MPEG_VC1_DESC_H

#include <mpeg_psi_parser.h>

// vc1 registration descriptor
enum mpeg_vc1_desc_type
{
    MPEG_ES_ET_NONE,
    MPEG_ES_ET_VC1
};

typedef struct _mpeg_vc1_desc mpeg_vc1_desc;

struct _mpeg_vc1_desc
{
    enum mpeg_vc1_desc_type type;
    int profile_level;
    int alignment_type;
    int bufsize_exp;
    int hrd_buffer;
};

int mpeg_vc1_desc_init(mpeg_vc1_desc* vc1, mpeg_data* data);
void mpeg_vc1_desc_destroy(mpeg_vc1_desc* vc1);

void mpeg_vc1_desc_report(mpeg_vc1_desc* vc1, const char* indent_string);

#endif
/* _MPEG_PARSER_H */

