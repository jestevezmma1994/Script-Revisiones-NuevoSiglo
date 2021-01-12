/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_POLL_CHANNEL__DEVPOLL_H_
#define SV_POLL_CHANNEL__DEVPOLL_H_

// this implementation of SvPollChannel can be used only on Solaris
#if defined __sun__

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/poll.h>
#include "SvPollChannel_types.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    SvPollCond_readLT = 1,
    SvPollCond_read = SvPollCond_readLT,
    SvPollCond_readET = 2,
    SvPollCond_writeLT = 3,
    SvPollCond_write = SvPollCond_writeLT,
    SvPollCond_writeET = 4
} SvPollCond;

// Poll channel class for /dev/poll based implementation.
struct SvPollChannel_t {
    int size;
    int pollFd;
    int fdCount;
    unsigned char *fdMasks;
    void **prvPtrs;
    struct pollfd *fds;
};


#define SV_POLL_CHANNEL_DEFINED 1
#include <SvQuirks/SvPollChannel_methods.h>

static inline int SvPollChannelGetFD(const SvPollChannel *channel)
{
    return channel->pollFd;
}


#ifdef __cplusplus
}
#endif

#endif // defined __sun__

#endif // SV_POLL_CHANNEL__DEVPOLL_H_