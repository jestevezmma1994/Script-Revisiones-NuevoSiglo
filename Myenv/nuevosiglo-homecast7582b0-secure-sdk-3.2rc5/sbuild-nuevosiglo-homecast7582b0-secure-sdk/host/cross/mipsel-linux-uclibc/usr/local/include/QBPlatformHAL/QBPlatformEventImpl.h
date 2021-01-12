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

#ifndef QB_PLATFORM_EVENT_IMPL_H_
#define QB_PLATFORM_EVENT_IMPL_H_

/**
 * @file QBPlatformEventImpl.h Platform events delivery subsystem API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBPlatformHAL/QBPlatformEvent.h>
#include <SvFoundation/SvObject.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformEventDelivery Interface for delivering platform events
 * @ingroup QBPlatformHAL
 *
 * This interface is provided for platform HAL implementations to notify interested
 * parties about various events, either triggered by some other API (like changes
 * in audio/video outputs configuration) or asynchronous hardware events (like
 * HDMI hotplug or CEC message reception.)
 *
 * @{
 **/


// this file should only be used by platform-specific implementation of platform HAL!


/**
 * Initialize event delivery subsystem.
 *
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformInitEventDelivery(void);

/**
 * Close event delivery subsystem.
 **/
extern void
QBPlatformDeinitEventDelivery(void);

/**
 * Schedule delivery of an event to registered parties.
 *
 * This function adds new event to the queue. It can be safely called from any thread.
 * Event will be delivered in the context of a special fiber in the main thread.
 *
 * @param[in] event     event type
 * @param[in] argA      first optional argument, pass @c -1 if not applicable
 * @param[in] argB      second optional argument, pass @c NULL if not applicable
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformEnqueueEvent(QBPlatformEvent event,
                       long long int argA,
                       SvObject argB);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
