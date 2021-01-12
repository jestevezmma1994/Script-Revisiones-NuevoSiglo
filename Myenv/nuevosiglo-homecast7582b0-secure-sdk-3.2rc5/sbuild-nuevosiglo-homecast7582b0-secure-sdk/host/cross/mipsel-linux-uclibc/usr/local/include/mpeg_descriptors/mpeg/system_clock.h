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

#ifndef MPEG_SYSTEM_CLOCK_DESC_H
#define MPEG_SYSTEM_CLOCK_DESC_H

#include <mpeg_psi_parser.h>

typedef struct _mpeg_system_clock_desc  mpeg_system_clock_desc;

struct _mpeg_system_clock_desc
{
  bool is_external;
  int  clock_accuracy_integer;
  int  clock_accuracy_exponent;
};

int mpeg_system_clock_desc_init(mpeg_system_clock_desc* desc, mpeg_data* data);
void mpeg_system_clock_desc_destroy(mpeg_system_clock_desc* desc);

void mpeg_system_clock_desc_report(const mpeg_system_clock_desc* desc, const char* indent_string);

#endif // MPEG_SYSTEM_CLOCK_DESC_H
