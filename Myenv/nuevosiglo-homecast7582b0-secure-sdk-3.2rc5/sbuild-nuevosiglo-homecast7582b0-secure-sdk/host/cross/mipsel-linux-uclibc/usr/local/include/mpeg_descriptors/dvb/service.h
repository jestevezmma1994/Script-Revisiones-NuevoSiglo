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

#ifndef FILE_MPEG_SERVICE_DESC_H
#define FILE_MPEG_SERVICE_DESC_H

#include <mpeg_psi_parser.h>

// service_descriptor

typedef struct _mpeg_service_desc  mpeg_service_desc;

struct _mpeg_service_desc
{
  int type;
  unsigned char* service_name;
  unsigned char* provider_name;
};

int mpeg_service_desc_init(mpeg_service_desc* desc, mpeg_data* data);
void mpeg_service_desc_destroy(mpeg_service_desc* desc);

static inline int mpeg_service_desc_get_service_type(mpeg_service_desc* desc) {
  return desc->type;
}
static inline mpeg_data  mpeg_service_desc_get_provider_name(mpeg_service_desc* desc) {
  mpeg_data data = {
    .data = desc->provider_name,
    .length = strlen((char*)desc->provider_name),
  };
  return data;
}
static inline mpeg_data  mpeg_service_desc_get_service_name(mpeg_service_desc* desc) {
  mpeg_data data = {
    .data = desc->service_name,
    .length = strlen((char*)desc->service_name),
  };
  return data;
}

void mpeg_service_desc_report(mpeg_service_desc* desc, const char* indent_string);

#endif
/* FILE_MPEG_SERVICE_DESC_H */

