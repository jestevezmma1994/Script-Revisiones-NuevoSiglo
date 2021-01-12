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

#ifndef QB_CONAX_HDE_SCHEDUKER_H
#define QB_CONAX_HDE_SCHEDUKER_H

/**
 * @file QBConaxHDEScheduler.h Host Data Extensions scheduler definition file
 **/

/**
 * @defgroup QBConaxHDEScheduler Host Data Extensions scheduler
 * @ingroup CubiTV_services
 * @{
 */

#include <QBConaxHostDataExtensions/QBConaxHDESchedulerDefs.h>
#include <QBConaxHostDataExtensions/QBConaxHDEHandler.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>

/**
 * Function for creating a new instance of Host Data Extensions scheduler.
 * The scheduler is designed as a filter for messages with sequence numbers.
 * It makes sure that a message with a given sequence number is served exactly once.
 * The general principle is that you create a handler implementing QBConaxHDEHandler
 * and then register it in the scheduler to serve a particular type of messages.
 * Later when you receive a message with it's sequence number you pass it to the scheduler.
 * If the sequence number for this message is different than the last served
 * sequence number for this message then the handler will be called, otherwise nothing happens.
 *
 * Furthermore the scheduler is designed to enable repeating of messages if handler do not report completion
 * within a given timeout. Default setting for the scheduler is not to repeat any messages.
 * To change the default settings use QBConaxHDESchedulerSetDefaultRepeatTime. To change the setting
 * for a particular handler use QBConaxHDESchedulhandlertenerRepeatTime.
 *
 * There is no need to unregister handlers prior to destroying QBConaxHDEScheduler.
 * However if the handler is to be destroyed and it is still possible that a message for
 * this handler will be received, then the handler should remove itself from handlers registered in
 * scheduler either by calling QBConaxHDESchedulerRemoveHandlerByMessage or QBConaxHDESchedulerRemoveHandler.
 *
 * The normal execution should be as follows:
 * 1. Create QBConaxHDEScheduler
 * 2. [optional] set default repeat time for handlers using QBConaxHDESchedulerSetDefaultRepeatTime
 * 3. register one or more handlers with QBConaxHDESchedulerSetHandler
 * 4. [optional] use QBConaxHDESchedulerSetHandlerRepeatTime to change the repeat time for particular handlers
 * 5. start processing messages using QBConaxHDESchedulerProcessMessage
 *
 * @param[in] serializationFilePath path to file that will be used as a storage for sequence numbers of
 *                                  processed messages (program should be grandted read write access
 *                                  to this file)
 * @param[in] scheduler scheduler (usually global scheduler)
 * @param[out] errorOut output for errors that may occur during object creation
 *
 */
QBConaxHDEScheduler QBConaxHDESchedulerCreate(SvString serializationFilePath,
                                              SvScheduler scheduler,
                                              SvErrorInfo *errorOut);

/**
 * Sets the repeat timeout for handlers registered after the call to this function.
 *
 * @param[in] self this object
 * @param[in] milliseconds timeout value in milliseconds. If negative the timeout will be disabled (default)
 */
void QBConaxHDESchedulerSetDefaultRepeatTime(QBConaxHDEScheduler self,
                                             int milliseconds);

/**
 * set new handler in the scheduler for a given message. The handler must implement QBConaxHDEHandler
 *
 * @param[in] self this object
 * @param[in] handler the handler object implementing QBConaxHDEHandler
 * @param[in] message message which will be handled by a handler
 * @param[out] errorOut error output
 */
void QBConaxHDESchedulerSetHandler(QBConaxHDEScheduler self,
                                   SvObject handler,
                                   ConaxHDEMessage message,
                                   SvErrorInfo *errorOut);

/**
 * unregister handler for a given message
 *
 * @param[in] self this object
 * @param[in] message message for which handler should be unregistered
 */
void QBConaxHDESchedulerRemoveHandlerByMessage(QBConaxHDEScheduler self,
                                               ConaxHDEMessage message);

/**
 * unregister handler from scheduler
 *
 * @param[in] self this object
 * @param[in] handler the handler to be removed from the list of handlers
 */
void QBConaxHDESchedulerRemoveHandler(QBConaxHDEScheduler self,
                                      SvObject handler);

/**
 * set the timeout for a given handler. If a message processing will not be reported as
 * completed by the handler within this timeout the handler will be called again.
 *
 * @param[in] self this object
 * @param[in] handler the handler for which the timeout is to be set
 * @param[in] milliseconds timeout value. (if negative the timeout will be disabled).
 */
void QBConaxHDESchedulerSetHandlerRepeatTime(QBConaxHDEScheduler self,
                                             SvObject handler,
                                             int milliseconds);

/**
 * pass a message to be processed by the scheduler. If a handler was registered for this message
 * and the sequence number is different than last sequence number passed to the scheduler
 * for this message then the message and sequence number pair is remembered as "in progress"
 * and the handler is called. If the same message with the same sequence number
 * was processed last time or is being processed at the moment ("in progress") then nothing happens.
 *
 * @param[in] self this object
 * @param[in] message message to be processed
 * @param[in] sequenceNumber sequence number for the message
 * @param[out] errorOut error output. Set if processing a message fails (for example handler signals failure)
 */
void QBConaxHDESchedulerProcessMessage(QBConaxHDEScheduler self,
                                       ConaxHDEMessage message,
                                       ConaxHDESequenceNumber sequenceNumber,
                                       SvErrorInfo *errorOut);

/**
 * method called by the handler to indicate that message processing is finished.
 * This method changes the state of message processing from "in progress" to "completed"
 * and cancels any timeout watchers waiting to repeat a message to the handler
 *
 * @param[in] self this object
 * @param[in] handler handler object reporting completion
 * @param[in] sequenceNumber sequence number of a message that the handler is reporting completion of
 * @param[out] errorOut error output
 */
void QBConaxHDESchedulerTransactionFinished(QBConaxHDEScheduler self,
                                            SvObject handler,
                                            ConaxHDESequenceNumber sequenceNumber,
                                            SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif //QB_CONAX_HDE_SCHEDUKER_H
