/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DEMUX_H
#define QB_DEMUX_H

#include "QBDemuxTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void QBDemuxInit(void);

QBDemux* QBDemuxCreateDefault(const char* name);

void QBDemuxClose(QBDemux* demux);

int  QBDemuxHandlePCR(QBDemux* demux, int pcr_pid);
int  QBDemuxPushPackets(QBDemux* demux, const unsigned char* packets, unsigned int packetCnt);

bool QBDemuxHasHWMultiChannels(QBDemux* demux);

/** Create pid channel on demux.
 *
 *  @param[in] demux QB Demux handle
 *  @param[in] pid pid that is to be filtered by this channel
 *  @param[in] buffer_size size of internal buffer
 *  @param[in] use_proxy create demux for proxy purpose
 *  @param[in] name name of channel
 *  @param[out] channel_out created channel handle
 *  @return 0 on success, SV_ERR_* on error.
 */
int  QBDemuxChannelCreate(QBDemux* demux, int pid, int buffer_size, bool use_proxy, const char* name, QBDemuxChannel** channel_out);
void QBDemuxChannelDestroy(QBDemuxChannel* channel);
int  QBDemuxChannelRead(QBDemuxChannel* channel, unsigned char* buf, int max_len);
int  QBDemuxChannelFlush(QBDemuxChannel* channel);

void* QBDemuxChannelGetDriverPtr(const QBDemuxChannel* channel, int pid);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DEMUX_H
