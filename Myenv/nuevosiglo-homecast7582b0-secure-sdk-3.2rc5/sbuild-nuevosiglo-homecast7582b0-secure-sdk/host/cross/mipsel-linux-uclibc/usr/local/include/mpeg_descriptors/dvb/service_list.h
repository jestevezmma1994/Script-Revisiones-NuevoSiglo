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

#ifndef FILE_MPEG_SERVICE_LIST_DESC_H
#define FILE_MPEG_SERVICE_LIST_DESC_H

#include <mpeg_psi_parser.h>

// service_list_desc
typedef struct _mpeg_service_list_element mpeg_service_list_element;

struct _mpeg_service_list_element
{
    list_linkage_t linkage; // sv-list link

    int service_id;
    int service_type;
};

//
typedef struct _mpeg_service_list_desc mpeg_service_list_desc;

struct _mpeg_service_list_desc
{
    list_t main_loop_element_list;
    mpeg_data* data;
};

int mpeg_service_list_desc_init(mpeg_service_list_desc* desc, mpeg_data* data);
void mpeg_service_list_desc_destroy(mpeg_service_list_desc* desc);

// returns list of mpeg_service_list_element
list_t* mpeg_service_list_desc_get_list(mpeg_service_list_desc* desc);

void mpeg_service_list_desc_report(mpeg_service_list_desc* desc, const char* indent_string);

#endif
/* FILE_MPEG_SERVICE_LIST_DESC_H */

