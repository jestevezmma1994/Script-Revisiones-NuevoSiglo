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

#ifndef FILE_MPEG_SDT_H
#define FILE_MPEG_SDT_H

#include "../mpeg_psi_parser.h"
#include <SvPlayerKit/SvChbuf.h>

typedef struct _mpeg_sdt_element mpeg_sdt_element;

struct _mpeg_sdt_element
{
    list_linkage_t linkage; // sv-list link
    mpeg_data header;
    mpeg_descriptor_map* descriptor_map;
};

int mpeg_sdt_element_get_sid(mpeg_sdt_element* es);
bool mpeg_sdt_element_get_EIT_schedule_flag(mpeg_sdt_element* es);
bool mpeg_sdt_element_get_EIT_pf_flag(mpeg_sdt_element* es);
int mpeg_sdt_element_get_running_status(mpeg_sdt_element* es);
bool mpeg_sdt_element_get_free_CA_mode(mpeg_sdt_element* es);
mpeg_descriptor_map* mpeg_sdt_element_get_descriptor_map(mpeg_sdt_element* es);

char* mpeg_sdt_element_get_service_name_strdup(mpeg_sdt_element* es);
char* mpeg_sdt_element_get_provider_name_strdup(mpeg_sdt_element* es);

// SDT parser
typedef struct _mpeg_sdt_parser mpeg_sdt_parser;

struct _mpeg_sdt_parser
{
    list_t main_loop_element_list; // list of mpeg_sdt_element

    int version_number;
    int tsid;
    int onid;
    int table_id;

    const struct SvChbuf_s* orig_chb;
};

void mpeg_sdt_parser_init_ex(mpeg_sdt_parser* parser,
                             const struct SvChbuf_s* chb);

void mpeg_sdt_parser_init(mpeg_sdt_parser* parser, mpeg_table* sdt);
void mpeg_sdt_parser_destroy(mpeg_sdt_parser* parser);

int mpeg_sdt_parser_get_version_number(mpeg_sdt_parser* parser);
int mpeg_sdt_parser_get_tsid(mpeg_sdt_parser* parser);
int mpeg_sdt_parser_get_onid(mpeg_sdt_parser* parser);
int mpeg_sdt_parser_get_table_id(mpeg_sdt_parser* parser);
const struct SvChbuf_s* mpeg_sdt_parser_get_orig_chb(mpeg_sdt_parser* parser);

// returns list of mpeg_sdt_element
list_t* mpeg_sdt_parser_get_service_list(mpeg_sdt_parser* parser);
mpeg_sdt_element* mpeg_sdt_parser_get_service(mpeg_sdt_parser* parser, int sid);

void mpeg_sdt_parser_report(mpeg_sdt_parser* parser);

#endif
/* FILE_MPEG_PMT_H */

