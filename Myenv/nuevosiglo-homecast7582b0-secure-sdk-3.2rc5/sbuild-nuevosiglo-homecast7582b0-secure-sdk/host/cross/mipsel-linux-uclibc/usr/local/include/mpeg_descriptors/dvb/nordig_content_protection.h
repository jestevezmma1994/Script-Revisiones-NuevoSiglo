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

#ifndef NORDIG_CONTENT_PROTECTION_H
#define NORDIG_CONTENT_PROTECTION_H

#include <stdint.h>
#include <mpeg_psi_parser.h>

typedef struct _nordig_cp_desc nordig_cp_desc;

typedef enum {
  nordig_cp_level_not_wanted = 0x00,
  nordig_cp_level_not_required = 0x01,
  nordig_cp_level_conditional = 0x02, // only for HD content
  nordig_cp_level_required = 0x03
} nordig_cp_level;

struct _nordig_cp_desc
{
  nordig_cp_level level;
};

int nordig_cp_desc_init(nordig_cp_desc* eac3, mpeg_data* data);
void nordig_cp_desc_destroy(nordig_cp_desc* eac3);

void nordig_cp_desc_report(const nordig_cp_desc* eac3, const char* indent_string);

#endif // NORDIG_CONTENT_PROTECTION_H
