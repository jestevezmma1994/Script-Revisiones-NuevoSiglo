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

/* SMP/sdplib/sdp.h */
/**********************************************************************
 *** VIDEO ON DEMAND                                                ***
 *** -------------------------------------------------------------- ***
 *** Module: RTSP Client                                            ***
 *** File:   sdp.h                                                  ***
 *** Author: Maciej Grzonkowski                                     ***
 *** Date:   02.07.2003                                             ***
 *** Desc:                                                          ***
 **********************************************************************/
#ifndef _SDP_H
#define _SDP_H

#include <netaddr.h>
#include <dataformat/media_format.h> // needed for "stream_t"

#include <stdio.h>

/**
 * Use these defaults when max values are not specified in the sdp
 */
#define MAX_RTP_PACKET_SIZE  65536
#define MAX_MEDIA_FRAME_SIZE 65536
#define MAX_CODEC_NAME_SIZE 32

#define XCAST_UNKNOWN -1
#define XCAST_UNICAST   0
#define XCAST_MULTICAST 1

#define DEFAULT_AUDIO_CLOCK 90000

#define MAX_MPEGTS_PIDS 4

//TODO increase buffers if should be greater than 9 (mifcc=Ax)
#define MAX_AUDIO_STREAMS 9

typedef enum {
  SERVER_OTHER=0,
  SERVER_ISMA,
  SERVER_SV,
  SERVER_SV_SV,
  SERVER_SV_ISMA,
  SERVER_SEACHANGE,
  SERVER_TRUECAST,
  SERVER_KASENNA,
  SERVER_MMS,
  SERVER_4SIGHT,
  SERVER_TOSHIBA,
  SERVER_EVENTIS_VOD,
  SERVER_CUBIWARE_GENERIC,
  SERVER_ORBIT_2X,
  SERVER_HARMONIC,
  SERVER_BROADPEAK,
} server_type_t;


extern const char *servers_names[];
typedef struct mp4_object_header_struct mp4_object_header_t;

typedef enum {
  FEC_NONE = 0,
  FEC_BRIS,
} fec_t;

struct fec_params {
  fec_t type;
  int block;       // number of packets per block
  int data;        // number of data packets per block
  int parity;      // number of parity packets per block
  int packetsize; // packet size excluding top-level RTP header
};

struct sdp_stream_info {
  int payload;
  char * tag;
  int xcast; // UNICAST or MULTICAST
  netaddr group; // group address (for multicast only)
  int ttl; // time to live (for multicast only)
  int port; // specified in the SDP (multicast) or set by RTP client (host byte order)
  int server_port;
  int nports; // number of ports per stream
  int no_rtp;

  char* codecname;
  unsigned int clockfreq;
  char* control;
  double framerate;
  unsigned int length; // in media samples
  double startnpt;
  double endnpt;
  int prebuffer;
  int minbuffer;
  int maxbuffer;
  int avgbps;   // avarage bitrate for the whole content
  unsigned int maxsize; // may be NULL
  unsigned int packetsize; // may be NULL
  unsigned int esid;
  unsigned int efmt_length;
  unsigned char* efmt;
  unsigned int asf_length;
  unsigned char* asfhdr;
  unsigned int mp4type,mp4plid;
  struct fec_params fec;
};

struct sdp_video_info {
  struct sdp_stream_info st;
  unsigned int width;
  unsigned int height;
  unsigned int aspect_width;
  unsigned int aspect_height;
  unsigned int bitcount;
  unsigned int packetization_mode;
  unsigned int mp4len; // H.264 NAL unit length in mp4-based contents
  unsigned int avc_profile,avc_level,avc_compat,avc_len;
};

struct sdp_audio_info {
  struct sdp_stream_info st;
  unsigned int formattag;
  unsigned int channels;
  unsigned int samplespersec; // unencoded
  unsigned int avgbytespersec; // encoded
  unsigned int blockalign;
  unsigned int bitspersample;
  int lsb_first, is_signed; //< valid only for PCM
  // AAC and CELP payload specific
  char *mode;
  int constant_size;
  int size_length;
  int index_length;
  int index_delta_length;
  int profile;
};

struct sdp_content_metainfo {
  char *title;
  char *author;
  char *copyright;
  char *description;
  char *rating;
};

struct sdp_mpegts_info {
  int video_pid_cnt;               //video pid counter <= MAX_MPEGTS_PIDS
  int video_pids[MAX_MPEGTS_PIDS];
  char video_codec[MAX_MPEGTS_PIDS][MAX_CODEC_NAME_SIZE];
  int audio_pid_cnt;               //audio pid counter <= MAX_MPEGTS_PIDS
  int audio_pids[MAX_MPEGTS_PIDS];
  int audio_codec[MAX_MPEGTS_PIDS];
  int pcr_pid_cnt;                 //pcr pid counter <= MAX_MPEGTS_PIDS
  int pcr_pids[MAX_MPEGTS_PIDS];
  int pmt_pid_cnt;                 //pmt pid counter <= MAX_MPEGTS_PIDS
  int pmt_pids[MAX_MPEGTS_PIDS];
};

struct sdp_info {
  unsigned int id;    // negative value means unspecified
                      // warning: now 0 means unspecified (marekp, 20050914)
  char* cname;
  int live;  // is live streaming
  int xcast; // UNICAST or MULTICAST
  server_type_t server_type; // which server do we talk to
  //  int schg;  // is streamed from SeaChange server
  int mms;   // is streamed from Microsoft Media Server
  netaddr group; // group address (for multicast only)
  int ttl; // time to live (for multicast only)
  char* control;
  int version;
  char* asfbuf;    // mms only
  int asflen;      // mms only
  struct sdp_video_info vi;
  struct sdp_audio_info ai[MAX_AUDIO_STREAMS];
  int audio_cnt;   // number of audio streams
  // ISMA mandatory fields
  int isma_profile;
  float isma_lowest_version;
  float isma_authored_version;
  mp4_object_header_t *iod;
  struct sdp_mpegts_info mpegts_info;  //mpegts only
  struct sdp_content_metainfo metainfo;
};

/**
 * Parse provided SDP string and create sdp_info structure containing
 * all fetched information. The caller is responsible for freeing the
 * resulting structure. If the *asf_parser_out is not null use it as a source
 * of the information also. If the *asf_parser_out is null and pgmpu (MMS
 * only) header is present in the sdpbody then rebuild the asf header
 * and store it in the *asf_hdr pointer. */
struct sdp_info *sdp_parse_sdp(char * sdpbody, int server_type, FILE* fp);

/**
 * Destroy provided sdp structure */
void sdp_destroy(struct sdp_info* sdp);

/**
 * Print SDP information to the stderr */
void sdp_print_sdp(FILE* fp, struct sdp_info* sdp);

#endif
