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

#ifndef FILE_MPEG_EIT_H
#define FILE_MPEG_EIT_H

#include "../mpeg_psi_parser.h"
#include <SvPlayerKit/SvEPGEvent.h>

int mpeg_eit_section_get_segment_last_section_number( mpeg_section* eit );
int mpeg_eit_section_get_last_table_id( mpeg_section* eit );

int mpeg_eit_section_get_sid (const mpeg_section* section);
int mpeg_eit_section_get_tsid(const mpeg_section* section);
int mpeg_eit_section_get_onid(const mpeg_section* section);

//
typedef struct _mpeg_eit_element mpeg_eit_element;

struct _mpeg_eit_element
{
    list_linkage_t linkage; // sv-list link
    mpeg_data header;
    mpeg_descriptor_map* descriptor_map;
};

int mpeg_eit_element_get_eid(mpeg_eit_element* es);


int mpeg_eit_element_get_start_time_ex(mpeg_eit_element* es, time_t* utcdays, int* hour, int* min, int* sec);
int mpeg_eit_element_get_start_time(mpeg_eit_element* es, time_t* utc);
int mpeg_eit_element_get_duration(mpeg_eit_element* es, int* hour, int* min, int* sec);

int mpeg_eit_element_get_running_status(mpeg_eit_element* es);
int mpeg_eit_element_get_free_CA_mode(mpeg_eit_element* es);
mpeg_descriptor_map* mpeg_eit_element_get_descriptor_map(mpeg_eit_element* es);

// EIT parser
typedef struct _mpeg_eit_parser mpeg_eit_parser;

struct _mpeg_eit_parser
{
    list_t main_loop_element_list; // list of mpeg_eit_element

    int version_number;
    int sid;
    int tsid;
    int onid;
};

// section copy constr, usually preferred
void mpeg_eit_parser_init_section(mpeg_eit_parser* parser, mpeg_section* eit);

// table copy constr, may be useful for eit pf
void mpeg_eit_parser_init_table(mpeg_eit_parser* parser, mpeg_table* eit);

void mpeg_eit_parser_destroy(mpeg_eit_parser* parser);

int mpeg_eit_parser_get_version_number(mpeg_eit_parser* parser);
int mpeg_eit_parser_get_sid(mpeg_eit_parser* parser);
int mpeg_eit_parser_get_tsid(mpeg_eit_parser* parser);
int mpeg_eit_parser_get_onid(mpeg_eit_parser* parser);

// returns list of mpeg_eit_element
list_t* mpeg_eit_parser_get_event_list(mpeg_eit_parser* parser);
mpeg_eit_element* mpeg_eit_parser_get_event(mpeg_eit_parser* parser, int eid);

void mpeg_eit_parser_report(mpeg_eit_parser* parser);

SvEPGEvent
mpeg_eit_parser_parse_event(mpeg_eit_element* eit_element_iter, SvValue channelID);

#endif
/* FILE_MPEG_PMT_H */

