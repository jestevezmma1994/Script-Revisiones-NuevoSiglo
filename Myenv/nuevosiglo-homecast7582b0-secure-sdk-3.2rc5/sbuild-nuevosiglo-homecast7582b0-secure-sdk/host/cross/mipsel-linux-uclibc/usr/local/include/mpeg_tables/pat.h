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

#ifndef FILE_MPEG_PAT_H
#define FILE_MPEG_PAT_H

#include "../mpeg_psi_parser.h"
#include <SvPlayerKit/SvChbuf.h>
#include <SvPlayerKit/SvBuf.h>

// PAT
// references:
// ISO-13818-1 - 2.4.4.3 - Program Association Table

// represents pat element
typedef struct _mpeg_pat_element mpeg_pat_element;

struct _mpeg_pat_element
{
    list_linkage_t linkage; // sv-list link
    int program_number;
    int PID;
};

//
typedef struct _mpeg_pat_parser mpeg_pat_parser;

struct _mpeg_pat_parser
{
    list_t pat_element_list;
};

void mpeg_pat_parser_init(mpeg_pat_parser* parser, mpeg_table* pat);
void mpeg_pat_parser_init_from_section(mpeg_pat_parser* parser, SvBuf sb);
void mpeg_pat_parser_init_from_table(mpeg_pat_parser* parser, SvChbuf chb);
void mpeg_pat_parser_destroy(mpeg_pat_parser* parser);

// returns mpeg_pat_element* for a given program_number or 0 if not present
mpeg_pat_element* mpeg_pat_parser_get_program_number(mpeg_pat_parser* parser, int program_number);

// returns PID for any program_number != MPEG_PROGRAM_NUMBER_NIT
mpeg_pat_element* mpeg_pat_parser_get_first(mpeg_pat_parser* parser);

// return list of mpeg_pat_element
list_t* mpeg_pat_parser_get_pid_list(mpeg_pat_parser* parser);

void mpeg_pat_parser_report(mpeg_pat_parser* parser);

#endif
/* FILE_MPEG_PAT_H */

