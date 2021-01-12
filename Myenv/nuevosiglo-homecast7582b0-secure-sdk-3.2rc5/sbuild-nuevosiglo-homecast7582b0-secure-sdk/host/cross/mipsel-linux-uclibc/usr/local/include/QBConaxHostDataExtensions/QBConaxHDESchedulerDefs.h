/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CONAX_HDE_SCHEDULER_DEFS_H
#define QB_CONAX_HDE_SCHEDULER_DEFS_H

/**
 * @file QBConaxHDESchedulerDefs.h Host Definitions for QBConaxHDEScheduler
 **/

/**
 * @ingroup QBConaxHDEScheduler
 * @{
 */

/**
 * Host Data Extensions scheduler class. It manages messages with sequence numbers.
 * Each message with a given sequence number is processed once.
 */
typedef struct QBConaxHDEScheduler_t* QBConaxHDEScheduler;

/**
 * Host Data Extensions message type. You register handlers for different values of this type
 * and later pass this type of messages to the scheduler for handling
 */
typedef long long int ConaxHDEMessage;

/**
 * Host Data Extensions message sequence number type. Value of this type is associated with
 * each message. If the value of sequence number for two equal messages is the same, the messages
 * are considered to be a single message.
 */
typedef int ConaxHDESequenceNumber;

/**
 * @}
 */

#endif //QB_CONAX_HDE_SCHEDULER_DEFS_H
