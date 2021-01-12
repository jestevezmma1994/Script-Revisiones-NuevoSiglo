/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_BUF_MAN_H_
#define SV_BUF_MAN_H_

#include "SvMemCounter.h"
#include <SvCore/SvTime.h>

#include <sys/uio.h> // needed for "struct iovec".

#ifdef __cplusplus
extern "C" {
#endif

struct SvBufMan_s;
typedef struct SvBufMan_s* SvBufMan;

struct SvBufManHandlers {
  void (*gotSpace) (void* owner, SvBufMan bufMan);
};

struct SvBufMan_s {
  int  (*getBuf)    (SvBufMan bufMan, unsigned int maxsize, struct iovec* iov);
  void (*commitBuf) (SvBufMan bufMan, const struct iovec* iov, int iov_cnt, SvTime time);

  const struct SvBufManHandlers* handlers;
  void* owner;

  unsigned int  max_mem_used;
  SvMemCounter  mem_counter;
};


static inline void SvBufManSetHandlers(SvBufMan bufMan, const struct SvBufManHandlers* handlers, void* owner)
{
  bufMan->handlers = handlers;
  bufMan->owner = owner;
};
static inline int SvBufManGetBuf(SvBufMan bufMan, unsigned int maxsize, struct iovec* iov)
{
  return bufMan->getBuf(bufMan, maxsize, iov);
};
static inline void SvBufManCommitBuf(SvBufMan bufMan, const struct iovec* iov, int iov_cnt, SvTime timePoint)
{
  bufMan->commitBuf(bufMan, iov, iov_cnt, timePoint);
};

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_BUF_MAN_H
