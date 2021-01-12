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

#ifndef FILE_MPEG_ES_DECODER_H
#define FILE_MPEG_ES_DECODER_H

#include <sv-list.h>
#include <dataformat/media_format.h>
#include <sdp.h>

#include "mpeg_common.h"
#include "mpeg_time.h"
#include "mpeg_demux.h"
#include "mpeg_psi_receiver.h"

//#include "mpeg_psi/mpeg_pat.h"
//#include "mpeg_psi/mpeg_pmt.h"

// @todo odcyklic zaleznosci ze specyficznymi dla typu es_parserami



// MPEG Packetized Elementary Stream (PES) packet
typedef struct _mpeg_pes_packet mpeg_pes_packet;

struct _mpeg_pes_packet
{
    int disc;   // packet lost before this PES
    int length; // length after the PES_packet_length field
    int offset; // payload offset

    int aligned; // starts with StartCode or SyncWord

    mpeg_time pts;
    mpeg_time dts;
    uint32_t rate; // in 50bps units

    mpeg_pes_data* pes_data;

    uint8_t* data;           // header (a part following the pes_length field), followed by the pes payload
};

int mpeg_parse_pes ( mpeg_pes_packet* packet, mpeg_pes_data* pes_data );


// MPEG PES frame
typedef struct _mpeg_frame mpeg_frame;

struct _mpeg_frame
{
    // private sv-list link - used by frame_queue in decoder
    list_linkage_t linkage;

    //
    stream_t type;

    int error; // error code placeholder

    dmx_frame* frame;
};

void mpeg_frame_delete( mpeg_frame* mf );
void mpeg_frame_report( mpeg_frame* mf );

//
typedef struct _mpeg_config_audio mpeg_config_audio;

struct _mpeg_config_audio
{
    struct sdp_audio_info* ai;
    int mpegts_info_index;
};

//
typedef struct _mpeg_config_video mpeg_config_video;

struct _mpeg_config_video
{
    struct sdp_video_info* vi;
    int mpegts_info_index;
};

//

//
typedef struct _mpeg_es_parser mpeg_es_parser;
typedef struct _mpeg_es_parser_visitor mpeg_es_parser_visitor;

typedef void (*mpeg_es_parser_accept)(mpeg_es_parser* pes, mpeg_es_parser_visitor* visitor);
typedef void (*mpeg_es_parser_visit_1)(mpeg_es_parser* pes, void* visitor_private_data);
typedef void (*mpeg_es_parser_visit_0)(mpeg_es_parser* pes);

struct _mpeg_es_parser_visitor
{
    mpeg_es_parser_visit_0 visit_audio_0;
    mpeg_es_parser_visit_1 visit_audio_1;
    mpeg_es_parser_visit_0 visit_video_0;
    mpeg_es_parser_visit_1 visit_video_1;

    mpeg_es_parser_visit_0 visit_autodetection_0;
    mpeg_es_parser_visit_1 visit_autodetection_1;

    void* private_data;
};

void mpeg_es_parser_visitor_clear(mpeg_es_parser_visitor* visitor);

//
typedef struct _mpeg_es_decoder mpeg_es_decoder;

//
struct _mpeg_es_parser
{
    // private sv-list link
    list_linkage_t linkage;

    //
    mpeg_es_decoder* dec;

    //
    mpeg_time cpts;
    mpeg_time cdts;
    int cpts_reset_flag;  // cpts and cdts is invalid (set by mpeg_reset during stream discontinuities )

    // ES frame parser state
    // @todo - probably should be moved out to type-specific parsers
    int maxsize;
    int length;
    uint8_t* buffer;

    //
    mpeg_pes_receiver* pes_receiver;

    // subclass static stuff
    stream_t type; // @deprecated, avoid using it, currently used only by es/pes_payload parser
    void* config_data; // mpeg_config_audio, mpeg_config_video, ...
    mpeg_es_parser_visit_0 config_data_destroy;
    mpeg_es_parser_accept stream_type_accept;

    // subclass factories for autodetection, streaming
    mpeg_es_parser_visit_0 autodetection_subclass_init;
    mpeg_es_parser_visit_0 streaming_subclass_init;
    mpeg_es_parser_visit_0 streaming_pes_payload_subclass_init;


    // subclass object stuff
    // set by subclass constructor
    void* subclass_private_data;
    mpeg_es_parser_visit_0 subclass_destroy;
    mpeg_es_parser_accept codec_type_accept;
};

mpeg_es_parser* mpeg_es_parser_create( mpeg_es_decoder* dec );
void mpeg_es_parser_delete( mpeg_es_parser* pes );

void mpeg_es_parser_autodetection_start( mpeg_es_parser* pes );
void mpeg_es_parser_streaming_start( mpeg_es_parser* pes );
void mpeg_es_parser_streaming_pes_payload_start( mpeg_es_parser* pes );

void mpeg_es_parser_stop( mpeg_es_parser* pes );

int add_byte ( mpeg_es_parser* pes, uint8_t b );
int add_buffer ( mpeg_es_parser* pes, uint8_t* source, size_t size );
int add_code_to_buffer ( mpeg_es_parser* pes, int code );

off_t mpeg_pes_parser_get_stream_offset( mpeg_es_parser* pes, mpeg_pes_data* pes_data, int offset );

//
typedef enum
{
    STATE_WAIT_FOR_SEQ = 1,
    STATE_WAIT_FOR_GOP,
    STATE_WAIT_FOR_PICTURE,
    STATE_READ_PICTURE,
} pes_state;

//
typedef enum
{
    NOT_DETECTED = 0,
    DETECTED_OK = 2,
    GAVE_UP = -1
} detection_result;

//
typedef struct _mpeg_es_parser_autodetection mpeg_es_parser_autodetection;

struct _mpeg_es_parser_autodetection
{
    detection_result result;
};

//
typedef struct _mpeg_es_decoder_config mpeg_es_decoder_config;

struct _mpeg_es_decoder_config
{
    int transport_packet_size;
    int transport_packet_begin_stuffing;
    int program_number;
    int pmt_pid;
    bool disable_pts_wrapping;
};

void mpeg_es_decoder_config_init_default(mpeg_es_decoder_config* config);

//
struct _mpeg_es_decoder
{
    mpeg_es_decoder_config config;

    mpeg_ts_parser* ts_parser;
    mpeg_ts_demux* ts_demux;

    SvMemCounter mem_counter;
    struct sdp_info* sdp;
    struct svdataformat* input_format;
    mpeg_psi_receiver* psi_receiver;
    bool setup_pmt_flag;

    list_t es_parser_list;
    mpeg_es_parser_visit_0 es_parser_start;
    int audio_track_index;
    int video_track_index;

    //
    mpeg_time sts;
    double wraptime;
    list_t frame_queue;
};

// config is copied, You don't have to store it
// use null ptrs to confs to use defaults
mpeg_es_decoder* mpeg_es_decoder_create( mpeg_es_decoder_config* config,
                                         mpeg_pes_demux_config* pes_demux_config,
                                         mpeg_section_demux_config* section_demux_config );

void mpeg_es_decoder_delete( mpeg_es_decoder* dec );

void mpeg_es_decoder_apply_codec_type_visitor( mpeg_es_decoder* dec, mpeg_es_parser_visitor* visitor );
void mpeg_es_decoder_apply_stream_type_visitor( mpeg_es_decoder* dec, mpeg_es_parser_visitor* visitor );

void mpeg_es_decoder_start_autodetection( mpeg_es_decoder* dec );
bool mpeg_es_decoder_check_autodetection( mpeg_es_decoder* dec );

void mpeg_es_decoder_start_audio_track( mpeg_es_decoder* dec, int audio_track_index );
void mpeg_es_decoder_stop_audio_track( mpeg_es_decoder* dec );
void mpeg_es_decoder_switch_audio_track( mpeg_es_decoder* dec, int audio_track_index );

void mpeg_es_decoder_start_video_track( mpeg_es_decoder* dec, int video_track_index );
void mpeg_es_decoder_stop_video_track( mpeg_es_decoder* dec );
void mpeg_es_decoder_switch_video_track( mpeg_es_decoder* dec, int video_track_index );

void mpeg_es_decoder_start_streaming( mpeg_es_decoder* dec );
void mpeg_es_decoder_start_streaming_pes_payload( mpeg_es_decoder* dec );
void mpeg_es_decoder_stop( mpeg_es_decoder* dec );

struct sdp_info* mpeg_es_decoder_get_sdp( mpeg_es_decoder* dec );
struct svdataformat* mpeg_es_decoder_get_dataformat( mpeg_es_decoder* dec );
mpeg_ts_parser* mpeg_es_decoder_get_ts_parser( mpeg_es_decoder* dec );
mpeg_ts_demux* mpeg_es_decoder_get_ts_demux( mpeg_es_decoder* dec );

mpeg_es_decoder_config* mpeg_es_decoder_get_config( mpeg_es_decoder* dec );
mpeg_pes_demux_config* mpeg_es_decoder_get_pes_demux_config( mpeg_es_decoder* dec );
mpeg_section_demux_config* mpeg_es_decoder_get_section_demux_config( mpeg_es_decoder* dec );

double mpeg_get_pts ( mpeg_es_decoder* dec, mpeg_time* pts, mpeg_time* dts );
double mpeg_get_current_pts ( mpeg_es_decoder* dec, mpeg_es_parser* pes );

void mpeg_save_pts_dts ( mpeg_es_parser* pes, mpeg_pes_packet* packet );
void mpeg_save_sts ( mpeg_es_decoder* dec, mpeg_pes_packet* packet );

void mpeg_fill_real_pts_dts ( mpeg_es_parser* pes, dmx_frame* frame );

mpeg_frame* mpeg_es_decoder_get_frame( mpeg_es_decoder* dec );
void mpeg_es_decoder_reset_frame_queue( mpeg_es_decoder* dec );

/// \return - 0 - mpeg_pes_packet is invalid, es_parser should be reset
/// \return - otherwise - mpeg_pes_packet is valid, mpeg_pes_data* must be mpeg_pes_data_delete-ed
mpeg_pes_data* mpeg_es_parser_pes_received( mpeg_es_parser* pes, mpeg_pes_packet* packet );

void mpeg_enqueue_frame( mpeg_es_decoder* dec,
                         stream_t type,
                         int error,
                         dmx_frame* frame );

#endif
