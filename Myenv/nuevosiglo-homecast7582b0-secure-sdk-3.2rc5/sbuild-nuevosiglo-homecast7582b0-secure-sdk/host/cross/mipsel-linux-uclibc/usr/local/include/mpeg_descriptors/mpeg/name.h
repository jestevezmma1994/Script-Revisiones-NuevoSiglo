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

#ifndef FILE_MPEG_NAME_DESC_H
#define FILE_MPEG_NAME_DESC_H

#include <mpeg_psi_parser.h>

// broadcaster_name_desc
// network_name_desc

typedef struct _mpeg_name_desc mpeg_name_desc;

struct _mpeg_name_desc
{
  char* name;
};

int mpeg_name_desc_init(mpeg_name_desc* desc, mpeg_data* data);
void mpeg_name_desc_destroy(mpeg_name_desc* desc);

void mpeg_name_desc_report(const mpeg_name_desc* desc, const char* indent_string);

#endif
/* FILE_MPEG_NAME_DESC_H */

