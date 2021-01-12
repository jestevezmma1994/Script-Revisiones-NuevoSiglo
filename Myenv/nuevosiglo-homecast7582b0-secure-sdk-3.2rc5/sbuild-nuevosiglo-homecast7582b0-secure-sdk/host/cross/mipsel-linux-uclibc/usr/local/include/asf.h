/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SENTIVISION_ASF
#define SENTIVISION_ASF

/**
 * @file asf.h ASF library API
 **/

#include "asftypes.h"
#include <dataformat/sv_data_format.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup asflib asflib: a library for parsing ASF files
 * @ingroup MediaPlayer
 * @{
 *
 * asflib is an implementation of a parser for ASF (Advanced Systems Format) container files.
 **/

typedef int asf_read_fn(void*, void*, int);

typedef struct asf_parser_ {
  asf_read_fn* read_cb;
  void* read_cb_arg;

  header_object* header;
  data_object* data;

  int reset;
  int direction; // >0 forward, <0 rewind
  // Packet queue
  payload *first, *last;
  // the following fields are valid after stream parsing
  float pts, min_fps, max_fps, avg_fps, fps;
  uint32_t fcnt, clock;
  uint32_t rate;
  float frametime;
  uint32_t afcnt, vfcnt;
  uint32_t amaxframe, vmaxframe;
  int encrypted_content;
} asf_parser;

/**
 * Create parser that will consume data provided in memory.
 *
 * @param[in] buf       data to be parsed
 * @param[in] len       length of the data
 *
 * @return new instance of asf_parser
 **/
extern asf_parser* asf_open_from_memory(const char* buf, int len);

/**
 * Parse information about audio/video streams in asf based on
 * data provided in memory.
 * @param[in] buf       data to be parsed
 * @param[in] len       length of the data
 * @return new sv data format describing audio/video streams
 **/
extern struct svdataformat* asf_parse_from_memory(const char* buf, int len);

/**
 * Create parser for the specified ASF read callback.
 * @param[in] asf_cb    callback called to read
 * @param[in] arg       callback private data
 * @param[in] cont      FIXME
 * @return new instance of asf_parser
 **/
extern asf_parser* asf_open_cb(asf_read_fn* asf_cb, void* arg, int cont);

/**
 * Sets read callback for the parser. It will be called to obtain data.
 * \note see asf_open_cb()
 *
 * @param[in] parser    parser handle
 * @param[in] asf_cb    callback called to read
 * @param[in] arg       callback private data
 **/
extern void asf_set_cb(asf_parser* parser, asf_read_fn* asf_cb, void* arg);

/**
 * Reset ASF parser.
 * @param[in] parser        asf_parser instance
 * @param[in] direction     FIXME
 **/
extern void asf_reset(asf_parser* parser, int direction);

/**
 * Print current parser status
 *
 * @param[in] parser        parser handle
 **/
extern void asf_print(const asf_parser* parser);

/**
 * Get audio and video stream id if present.
 *
 * @param[in] asf           parser handle
 * @param[out] video        returns found video stream id
 * @param[out] audio        returns found audio stream id
 **/
extern void asf_get_stream_ids(const asf_parser* asf, short* video, short* audio);

#if 0
/**
 * Return next media frame from the file or NULL when:
 * 1) end of file is reached (code==ASF_NO_MORE_ITEMS) or
 * 2) error occured
 **/
extern struct dmx_media_frame* asf_get_frame(asf_parser* parser, u32* type, asf_code* code);
#endif

/**
 * Close ASF parser.
 * @param[in] parser    parser instance
 **/
extern void asf_close(asf_parser* parser);

/**
 * Parse media type from byte stream.
 * @param[in] str       string to parse
 * @param[in] len       length of str
 * @return Asf media type instance
 **/
extern asfmediatype* asf_get_media_type(unsigned char* str, int len);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
