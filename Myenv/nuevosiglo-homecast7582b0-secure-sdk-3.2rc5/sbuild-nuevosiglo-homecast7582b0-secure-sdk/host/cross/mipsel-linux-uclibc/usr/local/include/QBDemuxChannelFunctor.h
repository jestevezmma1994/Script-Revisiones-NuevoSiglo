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

#ifndef QB_DEMUX_CHANNEL_FUNCTOR_H
#define QB_DEMUX_CHANNEL_FUNCTOR_H

#include "QBDemuxTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct QBDemuxChannelFunctor
{
  /** Destroy the receiver \a channel.
   *  All receivers must be destroyed before demux is unlinked or destroyed.
   */
  void (*destroy) (QBDemuxChannel* channel);

  /** Read at most \a max_len bytes from the receiver \a channel into \a buf.
   *  Always reads whole packets.
   *  \returns amount of bytes read, or zero when no full packets are available.
   *  \returns SV_ERR_* on error.
   */
  int (*read) (QBDemuxChannel* channel, unsigned char* buf, int max_len);

  /** Flush all buffered data.
   *  \returns amount of bytes flushed.
   *  \returns SV_ERR_* on error.
   */
  int (*flush) (QBDemuxChannel* channel);

  /** Return driver-specific object representing this pid-channel.
   *  \returns non-null on success, null on error.
   */
  void* (*get_driver_ptr) (const QBDemuxChannel* channel, int pid);
};

struct QBDemuxChannel_s
{
  const struct QBDemuxChannelFunctor* functor;

  QBDemux* demux;
};

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DEMUX_CHANNEL_FUNCTOR_H
