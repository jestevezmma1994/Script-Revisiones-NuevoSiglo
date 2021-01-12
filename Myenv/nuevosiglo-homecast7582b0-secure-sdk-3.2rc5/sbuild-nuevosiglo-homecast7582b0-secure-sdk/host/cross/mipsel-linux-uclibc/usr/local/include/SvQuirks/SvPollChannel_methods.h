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

#ifndef SV_POLL_CHANNEL__METHODS_H_
#define SV_POLL_CHANNEL__METHODS_H_

#if defined(SV_POLL_CHANNEL_DEFINED) || defined(DOXYGEN)

/**
 * @file SvPollChannel_methods.h
 * @brief Poll channel methods definitions
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvTime.h>
#include "SvPollChannel_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup SvPollChannel
 * @{
 **/

/**
 * Initialize poll channel.
 *
 * @param[in] channel   uninitialized poll channel
 * @param[in] size      number of descriptors in the channel
 * @return              @c -1 on error, any other value otherwise
 **/
extern int SvPollChannelInit(SvPollChannel *channel, int size);

/**
 * Close poll channel.
 *
 * @param[in] channel   initialized poll channel
 **/
extern void SvPollChannelClose(SvPollChannel *channel);

/**
 * Add new descriptor to wait for events on to the poll channel.
 *
 * @param[in] channel      poll channel
 * @param[in] fd           descriptor to wait for events on
 * @param[in] cond         binary mask of SvPollCond values
 * @param[in] ptr          opaque data pointer to be kept with descriptor
 * @return                 @c -1 on error, any other value otherwise
 **/
extern int SvPollChannelAddCond(SvPollChannel *channel,
                                int fd, unsigned cond, void *ptr);

/**
 * Modify already added descriptor to wait for specific events on to the poll ch
annel.
 *
 * @param[in] channel      poll channel
 * @param[in] fd           descriptor to wait for events on
 * @param[in] cond         binary mask of SvPollCond values
 * @param[in] ptr          opaque data pointer to be kept with descriptor
 * @return                 @c -1 on error, any other value otherwise
 **/
extern int SvPollChannelModCond(SvPollChannel *channel,
                                int fd, unsigned cond, void *ptr);

/**
 * Remove descriptor from poll channel.
 *
 * @param[in] channel      poll channel
 * @param[in] fd           descriptor to be removed from channel
 * @return                 @c -1 on error, any other value otherwise
 **/
extern int SvPollChannelDelCond(SvPollChannel *channel, int fd);

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
extern int SvPollChannelWait(SvPollChannel *channel,
                             SvPollEvent *events, int maxEvents,
                             SvTime t);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif // defined(SV_POLL_CHANNEL_DEFINED) || defined(DOXYGEN)

#endif // SV_POLL_CHANNEL__METHODS_H_
