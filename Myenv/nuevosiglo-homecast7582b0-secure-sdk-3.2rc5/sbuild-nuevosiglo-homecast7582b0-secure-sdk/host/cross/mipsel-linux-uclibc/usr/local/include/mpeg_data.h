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

#ifndef FILE_MPEG_DATA_H
#define FILE_MPEG_DATA_H

#include <sv-list.h>

#include "mpeg_common.h"

typedef struct _mpeg_pes_data mpeg_pes_data;
typedef struct _mpeg_section mpeg_section;
typedef struct _mpeg_section_filter mpeg_section_filter;
typedef struct _mpeg_table mpeg_table;

// pes callback data
struct _mpeg_pes_data
{
    // private sv-list link - used by notification queue
    list_linkage_t linkage;

    // stream offset - relative to reset position, ts packet aligned
    off_t stream_offset;

    // notification data is placed here for simplicity
    int discontinuity;

    // pes data
    int length;
    uint8_t* data;

    // pes_ts_map
    int* pes_ts_map;
    int pes_ts_map_n; // number of entries in pes_ts_map == number of packets for PES
};

void mpeg_pes_data_delete(mpeg_pes_data* pes_data);

//
struct _mpeg_section
{
    // private sv-list link - used by notification queue
    list_linkage_t linkage;

    // stream offset
    // @warning - relative to reset position
    // @warning - offset isn't packet aligned, it's an offset of the table_id field
    off_t stream_offset;

    // section data
    int length;

    uint8_t* data;
    uint8_t data_[0];
};

// create section and copy data
mpeg_section* mpeg_section_create( const uint8_t* data, int length, off_t stream_offset );

mpeg_section* mpeg_section_clone( const mpeg_section* section );
void mpeg_section_delete( mpeg_section* section );
void mpeg_section_clear( mpeg_section* section );

// basic section parsing - see ISO-13818-1, sec. 2.4.4.11
int mpeg_section_get_table_id( const mpeg_section* section );

int mpeg_section_get_syntax_indicator( const mpeg_section* section);

// length of the whole section, not the section_length field, btw. shall be equal to section->length
int mpeg_section_get_length( const mpeg_section* section );

// variant of mpeg_section_get_length for const uint8_t*
int mpeg_section_buffer_length( const uint8_t* buffer );

int mpeg_section_get_table_id_extension( const mpeg_section* section );
int mpeg_section_get_version_number( const mpeg_section* section );
int mpeg_section_get_section_number( const mpeg_section* section );
int mpeg_section_get_last_section_number( const mpeg_section* section );

// get ptr to section_private_data - the area after the last_section_number
// never null, but it may happen that mpeg_section_get_data_length <= 0
const uint8_t* mpeg_section_get_data( const mpeg_section* section );
uint8_t* mpeg_section_get_data_non_const( mpeg_section* section );

int mpeg_section_get_data_length( const mpeg_section* section );

// get the value of the CRC_32 field
uint32_t mpeg_section_get_CRC_32( const mpeg_section* section );
void mpeg_section_set_CRC_32_( mpeg_section* section, uint32_t crc );

// checks crc, returns 0 on success
uint32_t mpeg_section_crc32_check( const mpeg_section* section );
uint32_t mpeg_section_crc32_calc( const mpeg_section* section);
uint32_t mpeg_section_crc32_calc2(const uint8_t* data, int len);

// recalculate the CRC_32 field
void mpeg_section_crc32_compute( mpeg_section* section );
//
#define MPEG_MAX_SECTION_FILTER_LENGTH  12

struct _mpeg_section_filter
{
    uint8_t value[ MPEG_MAX_SECTION_FILTER_LENGTH ];
    uint8_t mask[ MPEG_MAX_SECTION_FILTER_LENGTH ];
    uint8_t neg_mask[ MPEG_MAX_SECTION_FILTER_LENGTH ];
};

// basic filter API
void mpeg_section_filter_init( mpeg_section_filter* filter );
void mpeg_section_filter_destroy( mpeg_section_filter* filter );
void mpeg_section_filter_clear( mpeg_section_filter* filter );
void mpeg_section_filter_set_mask( mpeg_section_filter* filter,
                                   const int bitOffset, const int nBits, uint32_t value );
void mpeg_section_filter_set_neg_mask( mpeg_section_filter* filter,
                                       const int bitOffset, const int nBits, uint32_t value );
bool mpeg_section_filter_match( const mpeg_section_filter* filter, const uint8_t * section, int section_length );

// helper functions for casual tasks -
// they are using the basic API
void mpeg_section_filter_set_table_id( mpeg_section_filter* filter, int table_id );

// set filter for section_syntax_indicator = 1, current_next_indicator = 1
void mpeg_section_filter_set_indicators( mpeg_section_filter* filter );
void mpeg_section_filter_set_table_id_extension( mpeg_section_filter* filter, int table_id_extension );
void mpeg_section_filter_set_neg_version_number( mpeg_section_filter* filter, int version_number );

void mpeg_section_filter_match_report( const mpeg_section_filter* filter,
                                       const uint8_t * section, int section_length );

//
struct _mpeg_table
{
    // private sv-list link - used by notification queue
    list_linkage_t linkage;

    mpeg_section* sections[ MPEG_SECTION_MAX_NUMBER_PER_TABLE ];
    int last_section_number;
    int collected_section_number;
    int version_number;

    // notification data is placed here for simplicity
    int error_code_placeholder;
};

mpeg_table* mpeg_table_create( void );
void mpeg_table_delete( mpeg_table* table );
void mpeg_table_clear( mpeg_table* table );
bool mpeg_table_append_section( mpeg_table* table, mpeg_section* section );
bool mpeg_table_is_complete( const mpeg_table* table );
off_t mpeg_table_get_stream_offset( const mpeg_table* table );
off_t mpeg_table_round_stream_offset( const mpeg_table* table );
int mpeg_table_get_version_number( const mpeg_table* table );

void mpeg_section_report( const mpeg_section* section );
void mpeg_table_report( const mpeg_table* table );

#endif

