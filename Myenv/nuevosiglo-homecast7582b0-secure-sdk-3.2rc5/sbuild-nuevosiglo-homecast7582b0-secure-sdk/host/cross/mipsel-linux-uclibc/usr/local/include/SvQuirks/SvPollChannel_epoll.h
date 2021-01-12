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

#ifndef SV_POLL_CHANNEL__EPOLL_H_
#define SV_POLL_CHANNEL__EPOLL_H_

// this implementation of SvPollChannel can be used only on linux
#if defined __linux__

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <SvCore/SvTime.h>
#include "SvPollChannel_types.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Type of event on file descriptor to wait for using poll channel.
 **/
typedef enum {
    /// level-triggered read event
    SvPollCond_readLT = EPOLLIN,
    /// default read event (level-triggered)
    SvPollCond_read = SvPollCond_readLT,
    /// edge-triggered read event
    SvPollCond_readET = (EPOLLIN | EPOLLET),
    /// level-triggered write event
    SvPollCond_writeLT = EPOLLOUT,
    /// default write event (level-triggered)
    SvPollCond_write = SvPollCond_writeLT,
    /// edge-triggered write event
    SvPollCond_writeET = (EPOLLOUT | EPOLLET)
} SvPollCond;


// Poll channel class for epoll()-based implementation.
struct SvPollChannel_t {
    int size;
    int epollFd;
};


/**
 * Initialize poll channel.
 *
 * @param[in] channel   uninitialized poll channel
 * @param[in] size      number of descriptors in the channel
 * @return              @c -1 on error, any other value otherwise
 **/
static inline int SvPollChannelInit(SvPollChannel *channel, int size)
{
    channel->size = size;
    return (channel->epollFd = epoll_create(size));
}

static inline int SvPollChannelGetFD(const SvPollChannel *channel)
{
    return channel->epollFd;
}

/**
 * Close poll channel.
 *
 * @param[in] channel   initialized poll channel
 **/
static inline void SvPollChannelClose(SvPollChannel *channel)
{
    if (channel && channel->epollFd >= 0) {
        close(channel->epollFd);
        channel->epollFd = -1;
    }
}

static inline int SvPollChannelOp_(SvPollChannel *channel, int fd,
                                   int epollOp, unsigned epollEvents, void *ptr)
{
    SvPollEvent fixedEv;
    struct epoll_event *ev = (struct epoll_event *) ((void *) &fixedEv);

    if (!channel || channel->epollFd < 0 || (epollOp != EPOLL_CTL_DEL && epollEvents == 0)) {
        // invalid arguments
        errno = EINVAL;
        return -1;
    } else if (fd < 0) {
        // invalid descriptor
        errno = EBADF;
        return -1;
    }

    fixedEv.events = epollEvents;
    fixedEv.data.u64 = 0;
    fixedEv.data.ptr = ptr;
    return epoll_ctl(channel->epollFd, epollOp, fd, ev);
}

/**
 * Add new descriptor to wait for events on to the poll channel.
 *
 * @param[in] channel      poll channel
 * @param[in] fd           descriptor to wait for events on
 * @param[in] cond         binary mask of SvPollCond values
 * @param[in] ptr          opaque data pointer to be kept with descriptor
 * @return                 @c -1 on error, any other value otherwise
 **/
static inline int SvPollChannelAddCond(SvPollChannel *channel,
                                       int fd, unsigned cond, void *ptr)
{
    return SvPollChannelOp_(channel, fd, EPOLL_CTL_ADD, cond, ptr);
}

/**
 * Modify already added descriptor to wait for specific events on to the poll channel.
 *
 * @param[in] channel      poll channel
 * @param[in] fd           descriptor to wait for events on
 * @param[in] cond         binary mask of SvPollCond values
 * @param[in] ptr          opaque data pointer to be kept with descriptor
 * @return                 @c -1 on error, any other value otherwise
 **/
static inline int SvPollChannelModCond(SvPollChannel *channel,
                                       int fd, unsigned cond, void *ptr)
{
    return SvPollChannelOp_(channel, fd, EPOLL_CTL_MOD, cond, ptr);
}

/**
 * Remove descriptor from poll channel.
 *
 * @param[in] channel      poll channel
 * @param[in] fd           descriptor to be removed from channel
 * @return                 @c -1 on error, any other value otherwise
 **/
static inline int SvPollChannelDelCond(SvPollChannel *channel, int fd)
{
    return SvPollChannelOp_(channel, fd, EPOLL_CTL_DEL, 0, NULL);
}

/**
 * Wait for events on poll channel.
 *
 * This method waits for events on previously registered descriptors.
 *
 * @param[in] channel      poll channel
 * @param[out] events      array of event descriptors
 * @param[in] maxEvents    number of entries in @a events
 * @param[in] t            wait time; use zero time, returned by
 *                         SvTimeGetZero(), to return immediately
 * @return                 number of events returned in @a events,
 *                         @c -1 on error
 **/
static inline int SvPollChannelWait(SvPollChannel *channel,
                                    SvPollEvent *events, int maxEvents,
                                    SvTime t)
{
    if (!channel || channel->epollFd < 0 || !events || maxEvents < 1) {
        // invalid arguments
        errno = EINVAL;
        return -1;
    }

    int millis = SvTimeToMilliseconds(t);
    return epoll_wait(channel->epollFd, (struct epoll_event *) events, maxEvents, millis);
}


#ifdef __cplusplus
}
#endif

#endif // defined __linux__

#endif // SV_POLL_CHANNEL__EPOLL_H_
