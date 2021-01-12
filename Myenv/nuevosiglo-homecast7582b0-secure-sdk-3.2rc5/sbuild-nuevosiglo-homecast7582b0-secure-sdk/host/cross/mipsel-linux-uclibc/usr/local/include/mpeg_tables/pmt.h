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

#ifndef FILE_MPEG_PMT_H
#define FILE_MPEG_PMT_H

#include "../mpeg_psi_parser.h"

#include <dataformat/sv_data_format.h>
#include <SvPlayerKit/SvChbuf.h>
#include <SvPlayerKit/SvBuf.h>

// PMT
// references:
// ISO-13818-1 - 2.4.4.8 - Program Map Table

// PMT - ES
typedef struct _mpeg_pmt_element mpeg_pmt_element;

struct _mpeg_pmt_element
{
    list_linkage_t linkage; // sv-list link
    mpeg_data header;
    mpeg_descriptor_map* descriptor_map;
};

int mpeg_pmt_element_get_stream_type(mpeg_pmt_element* es);
int mpeg_pmt_element_get_PID(mpeg_pmt_element* es);
mpeg_descriptor_map* mpeg_pmt_element_get_descriptor_map(mpeg_pmt_element* es);

// PMT parser
typedef struct _mpeg_pmt_parser mpeg_pmt_parser;

struct _mpeg_pmt_parser
{
    SvBuf  orig_section;

    mpeg_descriptor_map* program_info_descriptor_map; // program-level descriptors
    list_t pmt_element_list; // list of mpeg_pmt_element

    int version_number;
    int program_number;
    int PCR_PID;
};

bool mpeg_pmt_parser_init(mpeg_pmt_parser *parser, mpeg_table *pmt);
bool mpeg_pmt_parser_init_from_section(mpeg_pmt_parser *parser, SvBuf sb);
bool mpeg_pmt_parser_init_from_table(mpeg_pmt_parser *parser, SvChbuf chb);
void mpeg_pmt_parser_destroy(mpeg_pmt_parser* parser);

int mpeg_pmt_parser_get_version_number(mpeg_pmt_parser* parser);
int mpeg_pmt_parser_get_program_number(mpeg_pmt_parser* parser);
int mpeg_pmt_parser_get_PCR_PID(mpeg_pmt_parser* parser);
mpeg_descriptor_map* mpeg_pmt_parser_get_program_info(mpeg_pmt_parser* parser);

// returns list of mpeg_pmt_element
list_t* mpeg_pmt_parser_get_es_list(mpeg_pmt_parser* parser);

mpeg_pmt_element* mpeg_pmt_parser_get_es(mpeg_pmt_parser* parser, int PID);
void mpeg_pmt_parser_get_pids_array(mpeg_pmt_parser* parser, int** o_pids, int* o_npids);

int mpeg_pmt_parser_get_program_info_length( mpeg_pmt_parser* parser );
int mpeg_pmt_parser_get_es_info_length( mpeg_pmt_parser* parser );

// PMT Generator
// mpeg_table* must be freed mpeg_table_delete
mpeg_table* mpeg_pmt_parser_generate(mpeg_pmt_parser* parser);

void mpeg_pmt_parser_report(mpeg_pmt_parser* parser);

struct svdataformat* mpeg_pmt_parser_create_simple_data_format(mpeg_pmt_parser* pmt_parser);

#endif
/* FILE_MPEG_PMT_H */

