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

#ifndef QB_DESCRAMBLER_CHANNEL_H
#define QB_DESCRAMBLER_CHANNEL_H

/**
 * @file SMP/HAL/QBDescrambler/HALproto/channel.h
 * @brief Descrambler channel API
 *
 * IMPORTANT! - This API is also used by libraries delivered by external vendors (e.g. OpenTech)
 **/

#include <QBDescrambler/types.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBDescramblerChannel Descrambler channel
 * @ingroup QBDescrambler
 * @{
 **/

struct QBDescramblerChannel_s;
typedef struct QBDescramblerChannel_s QBDescramblerChannel;

QBDescramblerChannel* QBDescramblerChannelAlloc(QBDescramblerPidChannelStub* channelStub, uint16_t pid);
void QBDescramblerChannelFree(QBDescramblerChannel* descChannel);

int QBDescramblerChannelUpdateKeys(QBDescramblerChannel* descChannel,
                                   const QBDescramblerCW *cw,
                                   bool updateOdd, bool updateEven);

int QBDescramblerChannelClearKeys(QBDescramblerChannel* descChannel);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DESCRAMBLER_CHANNEL_H
