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

#ifndef FILE_MPEG_BAT_H
#define FILE_MPEG_BAT_H

#include "../mpeg_psi_parser.h"
#include <SvPlayerKit/SvChbuf.h>

typedef struct _mpeg_bat_element mpeg_bat_element;

struct _mpeg_bat_element
{
    list_linkage_t linkage; // sv-list link
    mpeg_data header;
    mpeg_descriptor_map* descriptor_map;
};

int mpeg_bat_element_get_tsid(mpeg_bat_element* es);
int mpeg_bat_element_get_onid(mpeg_bat_element* es);
mpeg_descriptor_map* mpeg_bat_element_get_descriptor_map(mpeg_bat_element* es);

// BAT parser
typedef struct _mpeg_bat_parser mpeg_bat_parser;

struct _mpeg_bat_parser
{
    mpeg_descriptor_map* top_level_descriptor_map; // network descriptors
    list_t main_loop_element_list; // list of mpeg_bat_element
    int bouquetid;
    int table_id;
    const struct SvChbuf_s* orig_chb;
};

void mpeg_bat_parser_init_ex(mpeg_bat_parser* parser, const struct SvChbuf_s* chb);
void mpeg_bat_parser_init(mpeg_bat_parser* parser, mpeg_table* nit);
void mpeg_bat_parser_destroy(mpeg_bat_parser* parser);

int mpeg_bat_parser_get_bouquetid(mpeg_bat_parser* parser);
int mpeg_bat_parser_get_table_id(mpeg_bat_parser* parser);
const struct SvChbuf_s* mpeg_bat_parser_get_orig_chb(mpeg_bat_parser* parser);

// returns list of mpeg_bat_element
list_t* mpeg_bat_parser_get_ts_list(mpeg_bat_parser* parser);

mpeg_bat_element* mpeg_bat_parser_get_ts(mpeg_bat_parser* parser, int tsid, int onid);

// get network level descriptors map
mpeg_descriptor_map* mpeg_bat_parser_get_network_info(mpeg_bat_parser* parser);

void mpeg_bat_parser_report(mpeg_bat_parser* parser);

#endif
/* FILE_MPEG_BAT_H */

