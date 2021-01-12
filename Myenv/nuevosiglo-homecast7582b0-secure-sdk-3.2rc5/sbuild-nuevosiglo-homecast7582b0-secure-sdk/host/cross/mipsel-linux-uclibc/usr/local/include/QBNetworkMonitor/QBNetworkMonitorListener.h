/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_NETWORK_MONITOR_LISTENER_H_
#define QB_NETWORK_MONITOR_LISTENER_H_

/**
 * @file QBNetworkMonitor.h Netwok Monitor API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <QBNetworkMonitor/QBNetworkInterface.h>
#include <QBNetworkMonitor/QBWiFiNetwork.h>
#include <stdbool.h>

/**
 * @defgroup QBNetworkMonitorListener Network Monitor Listener
 * @ingroup QBNetworkMonitor
 * @{
 **/

/**
 * Network Montior Listener is an interface that should be implemented by an object
 * that is going to be notified about networking changes. The API is used by
 * @ref QBNetworkMonitor.
 *
 * @class QBNetworkMonitorListener
 **/
typedef const struct QBNetworkMonitorListener_ {
    /**
     * Called when new network interface has just been added
     *
     * @param[in] self_ listener handle
     * @param[in] interface interface state data
    **/
    void (*interfaceAdded)(SvObject self_, QBNetworkInterface interface);
    /**
     * Called when a network interface has just been removed
     *
     * @param[in] self_ listener handle
     * @param[in] interfaceID id of the removed interface
    **/
    void (*interfaceRemoved)(SvObject self_, SvString interfaceID);
    /**
     * Called when state of a network interface has just been changed
     *
     * @param[in] self_ listener handle
     * @param[in] interfaceID id of the changed interface
    **/
    void (*interfaceStateChanged)(SvObject self_, SvString interfaceID);
    /**
     * Called when state of IPSec VPN has just been changed
     * @param[in] self_ listener handle
    **/
    void (*ipSecVPNStateChanged)(SvObject self_);

} *QBNetworkMonitorListener;

/* Get runtime type indentification object representing QBNetworkListener interface
 *
 * @return QBNetworkListener interface identification object
*/
SvInterface
QBNetworkMonitorListener_getInterface(void);

/**
 * @}
**/

#endif //QB_NETWORK_MONITOR_LISTENER_H_
