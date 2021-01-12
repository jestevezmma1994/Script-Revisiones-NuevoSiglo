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

#ifndef FILE_MPEG_NIT_H
#define FILE_MPEG_NIT_H

#include "../mpeg_psi_parser.h"
#include <SvPlayerKit/SvChbuf.h>

/**
 * @file nit.h
 **/

/**
 * @defgroup mpeg_nit Table NIT class
 * @ingroup mpeg
 * @{
 **/

typedef struct _mpeg_nit_element mpeg_nit_element;

struct _mpeg_nit_element
{
    list_linkage_t linkage; // sv-list link
    mpeg_data header;
    mpeg_descriptor_map* descriptor_map;
};

int mpeg_nit_element_get_tsid(mpeg_nit_element* es);
int mpeg_nit_element_get_onid(mpeg_nit_element* es);
mpeg_descriptor_map* mpeg_nit_element_get_descriptor_map(mpeg_nit_element* es);

// NIT parser
typedef struct _mpeg_nit_parser mpeg_nit_parser;

struct _mpeg_nit_parser
{
    mpeg_descriptor_map* top_level_descriptor_map; // network descriptors
    list_t main_loop_element_list; // list of mpeg_nit_element
    int nid;
    int table_id;
    const struct SvChbuf_s* orig_chb;
};

/**
 * Parse NIT table in table of buffor
 *
 * @param[out] parser   NIT parser object
 * @param[in] chb       Data to parse
 **/
void mpeg_nit_parser_init_ex(mpeg_nit_parser* parser, const struct SvChbuf_s* chb);

/**
 * Parse NIT table in buffor
 *
 * @param[out] parser   NIT parser object
 * @param[in] buf       Data to parse
 **/
void mpeg_nit_parser_init_from_buf(mpeg_nit_parser* parser, SvBuf buf);

/**
 * Parse NIT table in mpeg_table
 *
 * @param[out] parser   NIT parser object
 * @param[in] nit       Data to parse
 **/
void mpeg_nit_parser_init(mpeg_nit_parser* parser, mpeg_table* nit);

/**
 * Destroy NIT parser
 *
 * @param[out] parser   NIT parser object
 **/
void mpeg_nit_parser_destroy(mpeg_nit_parser* parser);

int mpeg_nit_parser_get_nid(mpeg_nit_parser* parser);
int mpeg_nit_parser_get_table_id(mpeg_nit_parser* parser);
const struct SvChbuf_s* mpeg_nit_parser_get_orig_chb(mpeg_nit_parser* parser);

// returns list of mpeg_nit_element
list_t* mpeg_nit_parser_get_ts_list(mpeg_nit_parser* parser);

mpeg_nit_element* mpeg_nit_parser_get_ts(mpeg_nit_parser* parser, int tsid, int onid);

// get network level descriptors map
mpeg_descriptor_map* mpeg_nit_parser_get_network_info(mpeg_nit_parser* parser);

char* mpeg_nit_get_network_name_strdup(mpeg_nit_parser* parser);

void mpeg_nit_parser_report(mpeg_nit_parser* parser);

/**
  * @}
  **/

#endif
/* FILE_MPEG_PMT_H */

