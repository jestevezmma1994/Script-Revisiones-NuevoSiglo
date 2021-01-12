/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_EVENT_BUS_H_
#define QB_EVENT_BUS_H_

/**
 * @file QBEventBus.h
 * @brief Event bus service class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <QBAppKit/QBAsyncService.h>
#include <QBAppKit/QBPeerEvent.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @page ReferenceManual
 *
 * @section EventBus Event bus service
 * Event bus service provides simple communication method between loosely
 * coupled senders and receivers. They don't really need to know about each
 * other (i.e. maintain references between each other), they just need to
 * agree on an event type.
 *
 * Although the public API allows creating multiple event bus instances,
 * most senders and receivers will use the default instance: you can access
 * it using QBServiceRegistryGetService() method with well-known name
 * "EventBus".
 *
 * @subsection SendingPeerEvents Sending events on the bus
 * Any object can send a peer event on the bus, it is a very simple operation:
@code
  #include <SvFoundation/SvObject.h>
  #include <QBAppKit/QBServiceRegistry.h>
  #include <QBAppKit/QBEventBus.h>

  QBButtonClickedEvent event = QBButtonClickedEventCreate(...);

  QBEventBus bus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
  QBEventBusPostEvent(bus, (QBPeerEvent) event, self, NULL);

  SVRELEASE(event);
@endcode
 *
 * @subsection ReceivingEvents Receiving events from the bus
 * To receive events class has to implement @ref QBPeerEventReceiver and
 * register itself in the bus asking for events of desired type:
@code
  #include <SvFoundation/SvType.h>
  #include <SvFoundation/SvObject.h>
  #include <QBAppKit/QBServiceRegistry.h>
  #include <QBAppKit/QBPeerEventReceiver.h>
  #include <QBAppKit/QBEventBus.h>

  SvLocal void
  HandlerHandleEvent(SvObject self, QBPeerEvent event, SvObject sender)
  {
      // event handling code
  }

  SvType
  Handler_getType(void)
  {
      static const struct QBPeerEventReceiver_ receiverMethods = {
          .handleEvent = HandlerHandleEvent
      };
      static SvType type = NULL;

      if (!type) {
          SvTypeCreateManaged("Handler",
                              sizeof(struct Handler_),
                              SvObject_getType(),
                              &type,
                              QBPeerEventReceiver_getInterface(), &receiverMethods,
                              NULL);
      }

      return type;
  }

  Handler
  HandlerCreate(void)
  {
      SvErrorInfo error = NULL;
      Handler self = SvTypeAllocateInstance(Handler_getType(), &error);

      // initialize self

      QBEventBus bus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
      QBEventBusRegisterReceiver(bus, self, QBButtonClickedEvent_getType(), &error);

      // further initialization, error handling

      return self;
  }
@endcode
**/


/**
 * @defgroup QBEventBus Event bus service
 * @ingroup QBAppKit
 * @{
 *
 * @link QBEventBus @endlink is a service that forwards peer events to interested
 * receivers. It provides simple communication method between loosely coupled
 * senders and receivers. Refer to @ref EventBus for more information.
 **/

/**
 * Event bus class.
 * @class QBEventBus QBEventBus.h <QBAppKit/QBEventBus.h>
 * @extends SvObject
 * @implements QBAsyncService
 * @implements QBInitializable
 **/
typedef struct QBEventBus_ *QBEventBus;


/**
 * Get runtime type identification object representing QBEventBus class.
 *
 * @relates QBEventBus

 * @return event bus class
 **/
extern SvType
QBEventBus_getType(void);

/**
 * Create an event bus service instance.
 *
 * @memberof QBEventBus
 *
 * @param[in] name       bus name, used for debugging
 * @param[out] errorOut  error info
 * @return               created event bus, @c NULL in case of error
 **/
extern QBEventBus
QBEventBusCreate(const char *name,
                 SvErrorInfo *errorOut);

/**
 * Check if event bus was started.
 *
 * @memberof QBEventBus
 *
 * @param[in] self      event bus handle
 * @return              @c true if event bus was started, @c false otherwise
 **/
extern bool
QBEventBusIsRunning(QBEventBus self);

/**
 * Start event delivery on the bus.
 *
 * @memberof QBEventBus
 * @hideinitializer
 *
 * @param[in] self      event bus handle
 * @param[in] scheduler scheduler to be used for delivering events
 * @param[out] errorOut error info
 **/
static inline void
QBEventBusStart(QBEventBus self,
                SvScheduler scheduler,
                SvErrorInfo *errorOut)
{
    SvInvokeInterface(QBAsyncService, self, start, scheduler, errorOut);
}

/**
 * Stop event delivery on the bus.
 *
 * @memberof QBEventBus
 * @hideinitializer
 *
 * @param[in] self      event bus handle
 * @param[out] errorOut error info
 **/
static inline void
QBEventBusStop(QBEventBus self,
               SvErrorInfo *errorOut)
{
    SvInvokeInterface(QBAsyncService, self, stop, errorOut);
}

/**
 * Register receiver interested in peer events of given type.
 *
 * This method establishes a relation between a peer event type and an object
 * that wants to receive such events. Receiver is remembered via a weak
 * reference, so it will be automatically forgotten when it's destroyed.

 * Registering for the same type of event more than once is silently ignored.
 *
 * Receiver can register for multiple related event types at once by specifying
 * a superclass of those types. Event bus guarantees that every posted event
 * is delivered to every interested receiver only once, no matter how many
 * registrations (for this event type and its super types) receiver has.
 *
 * @memberof QBEventBus
 *
 * @param[in] self      event bus handle
 * @param[in] receiver  object implementing @ref QBPeerEventReceiver
 * @param[in] eventType class of events @a receiver wants to receive
 * @param[out] errorOut error info
 **/
extern void
QBEventBusRegisterReceiver(QBEventBus self,
                           SvObject receiver,
                           SvType eventType,
                           SvErrorInfo *errorOut);

/**
 * Register receiver interested in peer events of given type
 * sent by one particular sender.
 *
 * This method is similar to QBEventBusRegisterReceiver(), but selects only
 * a subset of all events of given type to be received: ones sent by specific
 * sender.
 *
 * If the same receiver is registered for the same type of event, but without
 * specifying the sender (i.e. using QBEventBusRegisterReceiver()), calling
 * this method doesn't have any effect.
 *
 * Sender is remembered via a weak reference, so the registration will be
 * automatically forgotten when the sender is destroyed.
 *
 * @memberof QBEventBus
 *
 * @param[in] self      event bus handle
 * @param[in] receiver  object implementing @ref QBPeerEventReceiver
 * @param[in] eventType class of events @a receiver wants to receive
 * @param[in] sender    sender whose events @a receiver wants to receive
 * @param[out] errorOut error info
 **/
extern void
QBEventBusRegisterReceiverForSender(QBEventBus self,
                                    SvObject receiver,
                                    SvType eventType,
                                    SvObject sender,
                                    SvErrorInfo *errorOut);

/**
 * Unregister previously registered receiver.
 *
 * This method reverts effects of one or more previous calls
 * to QBEventBusRegisterReceiver(). By specifying @a eventType that is
 * a super type of multiple event types @a receiver has registered for,
 * all those registrations can be removed in a single call.
 *
 * @memberof QBEventBus
 *
 * @param[in] self      event bus handle
 * @param[in] receiver  object implementing @ref QBPeerEventReceiver
 * @param[in] eventType class of events @a receiver no longer wants to receive
 * @param[out] errorOut error info
 **/
extern void
QBEventBusUnregisterReceiver(QBEventBus self,
                             SvObject receiver,
                             SvType eventType,
                             SvErrorInfo *errorOut);

/**
 * Enqueue event to be delivered to registered receivers.
 *
 * @memberof QBEventBus
 *
 * @param[in] self      event bus handle
 * @param[in] event     event to be enqueued
 * @param[in] sender    object that sends this @a event
 * @param[out] errorOut error info
 **/
extern void
QBEventBusPostEvent(QBEventBus self,
                    QBPeerEvent event,
                    SvObject sender,
                    SvErrorInfo *errorOut);

/**
 * Forget all enqueued events.
 *
 * @memberof QBEventBus
 *
 * @param[in] self      event bus handle
 * @param[out] errorOut error info
 **/
extern void
QBEventBusFlushEvents(QBEventBus self,
                      SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
