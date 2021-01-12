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

#ifndef QB_CONAX_HDE_HANDLER_H
#define QB_CONAX_HDE_HANDLER_H

/**
 * @file QBConaxHDEHandler.h Host Data Extensions handler interface definition file
 **/

#include <QBConaxHostDataExtensions/QBConaxHDESchedulerDefs.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>

/**
 * @defgroup QBConaxHDEHandler Host Data Extensions handler interface
 * @ingroup CubiTV_services
 * @{
 */

/**
 * method called when handler is registered in QBConaxHDEScheduler
 *
 * @param[in] self_ this object
 * @param[in] hdeScheduler scheduler to which this object was added as a handler
 */
typedef void (*QBConaxHDESchedulerRegistrationNotification)(SvObject self_, QBConaxHDEScheduler hdeScheduler);

/**
 * method called when a handler is supposed to perform its action.
 *
 * It is possible to construct handlers that handle messages within the call to this method,
 * as well as handlers that can take some time to finish message handling.
 * If a handler manages to handle message within the call to this function it should set the output
 * parameter finished to true. In this case the handler can use only this callback and not care of
 * QBConaxHDESchedulerRegistrationNotification.
 *
 * If handling of message takes more time than this method execution,
 * the handler must leave the finished parameter as is
 * and signal the message processing completion by the call to QBConaxHDESchedulerTransactionFinished.
 * In this case the handler is obligated to store the pointer passed to it via
 * QBConaxHDESchedulerRegistrationNotification in order to be able to communicate back.
 *
 * The handler is not allowed to use both methods of signaling handling completion
 * at the same time (for example set the finished to true, and call
 * QBConaxHDESchedulerTransactionFinished within this callback).
 * It is safe to call QBConaxHDESchedulerTransactionFinished within this function as long
 * as this function does not set the finished parameter to true and as long as
 * this method does not passes more messages to QBConaxHDEScheduler, however if this handler
 * can handle message within the call to this function it is recommended to use finished
 * parameter to signal completion.
 *
 * If this method signals any errors via errorOut (regardless of finished parameter value)
 * the message will not be marked as handled, and thus may be repeated if another message
 * with the same sequence number comes or if the scheduler is configured to repeat messages
 * not handled within timeout.
 *
 * @param[in] self_ this object
 * @param[in] sequenceNumber number of the request for this particular call of QBConaxHDECallback.
 *                       This number is to be passed back to QBConaxHDEScheduler,
 *                       by the call to QBConaxHDESchedulerTransactionFinished,
 *                       when the request has been handled.
 * @param[out] finished pointer to a bool that should be set if the message was handled within the call to this function
 * @param[out] errorOut output for any errors that may occur during message handling
 */
typedef void (*QBConaxHDECallback)(SvObject self_, ConaxHDESequenceNumber sequenceNumber, bool *finished, SvErrorInfo *errorOut);

/**
 * handler interface for Host Data Extensions. Instances of objects implementing this interface
 * should be added to QBConaxHDEScheduler via a call to QBConaxHDESchedulerSetHandler
 */
struct QBConaxHDEHandler_ {
    QBConaxHDESchedulerRegistrationNotification registerNotification;
    QBConaxHDECallback callback;
};
typedef struct QBConaxHDEHandler_ *QBConaxHDEHandler;
SvExport SvInterface QBConaxHDEHandler_getInterface(void);

/**
 * @}
 **/

#endif //QB_CONAX_HDE_HANDLER_H
