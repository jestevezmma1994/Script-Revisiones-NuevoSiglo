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

#ifndef FILE_MPEG_TS_PACKET_H
#define FILE_MPEG_TS_PACKET_H

#include "mpeg_common.h"
#include "mpeg_pcr.h"

#include <SvPlayerKit/SvChbuf.h>
#include <sv-list.h>

//
typedef struct _mpeg_ts_parser mpeg_ts_parser;

struct _mpeg_ts_parser
{
    struct SvChbuf_s chdata;
    unsigned int first_buf_offset;

    int transport_packet_size;
    int transport_packet_begin_stuffing;
    off_t current_stream_offset;

    int buffer_length; // sum buffer_list length

    unsigned char* wrapped_packet_buff;
    const unsigned char* put_back_packet;
    const unsigned char* last_packet;
};

//
void mpeg_ts_parser_init ( mpeg_ts_parser* parser,
                          int transport_packet_size,
                          int transport_packet_begin_stuffing );

void mpeg_ts_parser_destroy ( mpeg_ts_parser* parser );
void mpeg_ts_parser_reset ( mpeg_ts_parser* parser );

// gains ownership of "sb".
void mpeg_ts_parser_push ( mpeg_ts_parser* parser, SvBuf sb );


off_t mpeg_ts_parser_get_current_packet_offset ( mpeg_ts_parser* parser );
const unsigned char* mpeg_ts_parser_get_packet ( mpeg_ts_parser* parser );
off_t mpeg_ts_parser_round_stream_offset( mpeg_ts_parser* parser, off_t stream_offset );

unsigned char* mpeg_ts_parser_put_back_packet( mpeg_ts_parser* parser );
int mpeg_ts_parser_get_offset_in_packet( const mpeg_ts_parser* parser, const unsigned char* ptr );

int mpeg_ts_parser_get_transport_packet_size( mpeg_ts_parser* parser );
int mpeg_ts_parser_get_transport_packet_begin_stuffing( mpeg_ts_parser* parser );

int mpeg_ts_parser_get_packet_number( mpeg_ts_parser* parser );

//
typedef struct _mpeg_packet mpeg_packet;

/**
 * ISO-13818-1, p. 18/36
 */
struct _mpeg_packet
{
    int unit_start;     // payload_unit_start_indicator
    int pid;
    int cnt;            // continuity_counter
    int priority;       // transport priority
    int adapt;          // is adaptation data present
    int discontinuity;  // --//--

#ifdef USE_MPEG_CR
    mpeg_pcr pcr;       // --//--
    mpeg_pcr opcr;      // --//--
#endif

    int length;         // length of the payload

    const uint8_t* data;
    const uint8_t* packet; // original packet data
};



#endif
