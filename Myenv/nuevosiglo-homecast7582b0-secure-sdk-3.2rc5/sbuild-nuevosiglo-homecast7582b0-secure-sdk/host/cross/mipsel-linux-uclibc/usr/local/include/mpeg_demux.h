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

#ifndef FILE_MPEG_DEMUX_H
#define FILE_MPEG_DEMUX_H

#include <sv-list.h>

#include "mpeg_common.h"
#include "mpeg_data.h"
#include "mpeg_ts_packet.h"

typedef struct _mpeg_pes_receiver mpeg_pes_receiver;
typedef struct _mpeg_section_receiver mpeg_section_receiver;

typedef struct _mpeg_ts_demux mpeg_ts_demux;

// references:
// ISO-13818-1 - 2.4.3 - p. 18/36 - transport packet
// ISO-13818-1 - 2.4.3.6 - p. 31/49 - PES packet
// ISO-13818-1 - 2.6 - p. 80/62 - descriptors
// ISO-13818-1 - 2.4.3 - p. 18/36, 2.4.4, 2.4.4.1, 2.4.4.2 - p. 59-61 / 41-43 - section demux

// continuity-counter handling
typedef enum
{
    MPEG_CC_CHECK,
    MPEG_CC_IGNORE_ON_PES_BOUNDARY,
    MPEG_CC_IGNORE_ALWAYS,
} mpeg_cc_handling;

// pes demux config
typedef struct _mpeg_pes_demux_config mpeg_pes_demux_config;

struct _mpeg_pes_demux_config
{
    mpeg_cc_handling cc_handling;
    bool enable_pes_ts_map;
};

void mpeg_pes_demux_config_init_default(mpeg_pes_demux_config* config);

// client interface
typedef void (*mpeg_pes_received_callback)( mpeg_pes_receiver* pes_receiver );

struct _mpeg_pes_receiver
{
    int pid;
    mpeg_pes_demux_config demux_config;

    //
    void* private_data; // user private data
    mpeg_pes_received_callback received;

    //
    list_t* notification_queue; // set by mpeg_pes_receiver_create
    mpeg_ts_demux* ts_demux;
    void* demux_private_data; // private data set by ts_demux
};

//
typedef enum mpeg_section_receive_policy_e mpeg_section_receive_policy;

enum mpeg_section_receive_policy_e
{
    MPEG_SECTION_RECEIVE_COLLECT_TABLE, // collect tables with different version_number
    MPEG_SECTION_RECEIVE_COLLECT_TABLE_EACH_INSTANCE, // collect tables, even with the same version_number
    MPEG_SECTION_RECEIVE_ROUTE_SECTIONS // collect all sections
};

// section demux config
typedef struct _mpeg_section_demux_config mpeg_section_demux_config;

struct _mpeg_section_demux_config
{
    mpeg_cc_handling cc_handling;
};

void mpeg_section_demux_config_init_default(mpeg_section_demux_config* config);

//
typedef void (*mpeg_psi_received_callback)( mpeg_section_receiver* section_receiver );

struct _mpeg_section_receiver
{
    int pid;
    mpeg_section_filter filter;
    mpeg_section_receive_policy receive_policy;

    mpeg_section_demux_config demux_config;

    //
    void* private_data; // user private data
    mpeg_psi_received_callback received;

    //
    list_t* notification_queue; // set by mpeg_section_receiver_create
    mpeg_ts_demux* ts_demux;
    void* demux_private_data; // private data set by ts_demux
};

// configs are copied, You don't have to store it
// use null ptrs to confs to use defaults
mpeg_ts_demux* mpeg_ts_demux_create( mpeg_ts_parser* ts_parser,
                                     mpeg_pes_demux_config* pes_demux_config,
                                     mpeg_section_demux_config* section_demux_config );

void mpeg_ts_demux_delete( mpeg_ts_demux* ts_demux );
int mpeg_ts_demux_read( mpeg_ts_demux* ts_demux );

//
mpeg_pes_demux_config* mpeg_ts_demux_get_pes_demux_config( mpeg_ts_demux* ts_demux );

mpeg_pes_receiver* mpeg_pes_receiver_create( mpeg_ts_demux* ts_demux );
void mpeg_pes_receiver_delete( mpeg_pes_receiver* pes_receiver );

void mpeg_pes_receiver_start( mpeg_pes_receiver* pes_receiver );

// it's ok to stop pes_receiver which wasn't started
void mpeg_pes_receiver_stop( mpeg_pes_receiver* pes_receiver );

typedef void ( * mpeg_pes_receiver_visit )( mpeg_pes_receiver* pes_receiver, void* private_data );

// remove and free \a pes_receiver,
// \pre custom delete is not necessary
void mpeg_pes_receiver_visit_delete( mpeg_pes_receiver* pes_receiver, void* private_data );

// reset \a pes_receiver,
// \pre custom reset s not necessary
void mpeg_pes_receiver_visit_reset( mpeg_pes_receiver* pes_receiver, void* private_data );

void mpeg_ts_demux_foreach_pes_receiver( mpeg_ts_demux* ts_demux,
                                         mpeg_pes_receiver_visit pes_receiver_visit,
                                         void* private_data
                                       );

//
mpeg_section_demux_config* mpeg_ts_demux_get_section_demux_config( mpeg_ts_demux* ts_demux );

mpeg_section_receiver* mpeg_section_receiver_create( mpeg_ts_demux* ts_demux );
void mpeg_section_receiver_delete( mpeg_section_receiver* section_receiver );

void mpeg_section_receiver_start( mpeg_section_receiver* section_receiver );

// it's ok to stop section_receiver which wasn't started
void mpeg_section_receiver_stop( mpeg_section_receiver* section_receiver );

// returns true whenever there's a table version - use mpeg_section_receiver_get_current_table to get it
bool mpeg_section_receiver_new_table( mpeg_section_receiver* section_receiver );

mpeg_table* mpeg_section_receiver_get_current_table( mpeg_section_receiver* section_receiver );

typedef void ( * mpeg_section_receiver_visit )( mpeg_section_receiver* section_receiver, void* private_data );

// remove and free \a section_receiver,
// \pre custom delete is not necessary
void mpeg_section_receiver_visit_delete( mpeg_section_receiver* section_receiver, void* private_data );

void mpeg_ts_demux_foreach_section_receiver( mpeg_ts_demux* ts_demux,
                                             mpeg_section_receiver_visit section_receiver_visit,
                                             void* private_data
                                       );

#endif

