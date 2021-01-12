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

#ifndef FILE_MPEG_MUX_H
#define FILE_MPEG_MUX_H

#include <sv-list.h>

#include "mpeg_common.h"
#include "mpeg_psi_parser.h"
#include "mpeg_ts_packet.h"

// segmentation of PSI tables into mpeg ts packets
typedef struct _mpeg_psi_mux mpeg_psi_mux;

struct _mpeg_psi_mux
{
    int PID;
    int continuity_counter;
    mpeg_ts_parser* parser;
};

void mpeg_psi_mux_init( mpeg_psi_mux* mux,
                        int PID,
                        mpeg_ts_parser* parser);

void mpeg_psi_mux_destroy(mpeg_psi_mux* mux);

int mpeg_psi_mux_get_PID(mpeg_psi_mux* mux);

int mpeg_psi_mux_put(mpeg_psi_mux* mux, mpeg_table* table, uint8_t* output_buffer, int max_packets);

/**
 * Segmentize data from buffer.
 *
 * @param[in] mux            segmentizer handler
 * @param[in] input_buffer   input buffer
 * @param[in] length         length of the buffer
 * @param[out] output_buffer destination buffer, may be NULL in order to get required output buffer size
 * @return                   length of required output buffer
 */
int mpeg_psi_mux_put_data(mpeg_psi_mux* mux, uint8_t* input_buffer, size_t length, uint8_t* output_buffer);

#endif
/* _MPEG_MUX_H */
