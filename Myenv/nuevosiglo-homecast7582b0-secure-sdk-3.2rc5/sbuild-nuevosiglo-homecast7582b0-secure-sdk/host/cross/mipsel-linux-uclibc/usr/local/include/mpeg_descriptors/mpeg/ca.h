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

#ifndef FILE_MPEG_CA_DESC_H
#define FILE_MPEG_CA_DESC_H

#include <mpeg_psi_parser.h>

// ca_descriptor
typedef struct _mpeg_ca_desc mpeg_ca_desc;

struct _mpeg_ca_desc
{
    mpeg_data* data;
};

int mpeg_ca_desc_init(mpeg_ca_desc* desc, mpeg_data* data);
void mpeg_ca_desc_destroy(mpeg_ca_desc* desc);
int mpeg_ca_desc_get_CA_system_ID(mpeg_ca_desc* desc);
int mpeg_ca_desc_get_CA_PID(mpeg_ca_desc* desc);
const uint8_t* mpeg_ca_desc_get_private_data(mpeg_ca_desc* desc);
int mpeg_ca_desc_get_private_data_length(mpeg_ca_desc* desc);

void mpeg_ca_desc_create( mpeg_data* ca_desc,
                        int CA_system_ID,
                        int CA_PID,
                        uint8_t* private_data,
                        int private_data_length );

void mpeg_ca_desc_report( mpeg_ca_desc* ca_desc, const char* indent_string );


#endif
/* FILE_MPEG_CA_DESC_H */

