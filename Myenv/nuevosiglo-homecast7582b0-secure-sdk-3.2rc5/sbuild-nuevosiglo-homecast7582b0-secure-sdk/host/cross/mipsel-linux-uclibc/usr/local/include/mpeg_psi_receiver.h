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

#ifndef FILE_MPEG_PSI_RECEIVER_H
#define FILE_MPEG_PSI_RECEIVER_H

#include "mpeg_demux.h"
#include "mpeg_psi_parser.h"

//
typedef struct _mpeg_psi_receiver mpeg_psi_receiver;

mpeg_psi_receiver* mpeg_psi_receiver_create(
        mpeg_ts_demux* ts_demux,
        mpeg_psi_received_callback pmt_received, // standard mpeg_section_receiver callback - \see mpeg_demux.h
        void* private_data, // this will be put to mpeg_section_receiver private_data in pmt_received
        int program_number,
        int pmt_pid // -1 iff unknown, PAT will be needed then
    );

void mpeg_psi_receiver_delete( mpeg_psi_receiver* psi_receiver );

#endif
/* FILE_PSI_RECEIVER_H */

