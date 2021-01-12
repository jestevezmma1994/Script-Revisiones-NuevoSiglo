/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_DEMUX_MULTI_CHANNEL_H
#define QB_DEMUX_MULTI_CHANNEL_H

#include "QBDemux.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int QBDemuxMultiChannelCreate(QBDemux* demux, int total_buffer_size, int max_pid_cnt, QBDemuxChannel** multi_channel_out);

int QBDemuxMultiChannelRemovePid(QBDemuxChannel* multi_channel, int pid);

// if 'exclusive' flags is set we always want to have a unique hw channel ('shared' flag is used only when 'exclusive' is 'false')
int QBDemuxMultiChannelAddPid(QBDemuxChannel* multi_channel,
                              int pid,
                              int buffer_size,
                              bool shared,
                              bool count_in_flush,
                              const char* name,
                              bool exclusive);

int QBDemuxMultiChannelAddCustom(QBDemuxChannel* multi_channel,
                                 QBDemuxChannel* channel,
                                 int pid,
                                 bool shared,
                                 bool count_in_flush,
                                 const char* name);

int QBDemuxMultiChannelGetPids(const QBDemuxChannel* multi_channel, int* pids_out, int max_pids);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DEMUX_MULTI_CHANNEL_H
