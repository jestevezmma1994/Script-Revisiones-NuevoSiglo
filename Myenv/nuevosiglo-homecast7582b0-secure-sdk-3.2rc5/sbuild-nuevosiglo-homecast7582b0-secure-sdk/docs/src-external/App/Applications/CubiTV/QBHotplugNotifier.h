/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBHOTPLUGNOTIFIER_H_
#define QBHOTPLUGNOTIFIER_H_

/**
 * @file QBHotplugNotifier.h Hotplug Notifier service API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>


/**
 * @defgroup QBHotplugNotifier Hotplug Notifier service
 * @ingroup CubiTV_services
 * @{
 **/

/**
 * It is a service that monitors hotplug events and shows a popup
 * notifying the user that a disk has been inserted.
 *
 * @class QBHotplugNotifier
 **/
typedef struct QBHotplugNotifier_t *QBHotplugNotifier;

/**
 * Creates an instance of Hotplug Notifier class.
 * @memberof QBHotplugNotifier
 * @param[in] appGlobals        CubiTV application
 * @return new Hotplug Notifier instance
 */
QBHotplugNotifier QBHotplugNotifierCreate(AppGlobals appGlobals);

/**
 * Starts listening to hotplug events. Now if a collection
 * of events will say that a disk has been added it will
 * show a popup.
 *
 * @memberof QBHotplugNotifier
 * @param[in] self      Hotplug Notifier instance
 */
void QBHotplugNotifierStart(QBHotplugNotifier self);

/**
 * Stops listening to hotplug events.
 *
 * @memberof QBHotplugNotifier
 * @param[in] self      Hotplug Notifier instance
 */
void QBHotplugNotifierStop(QBHotplugNotifier self);

/**
 * @}
 **/

#endif
