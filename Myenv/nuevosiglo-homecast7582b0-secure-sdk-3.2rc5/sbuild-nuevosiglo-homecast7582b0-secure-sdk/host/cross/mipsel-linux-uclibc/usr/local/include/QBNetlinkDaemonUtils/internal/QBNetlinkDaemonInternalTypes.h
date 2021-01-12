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

#include <QBNetlinkDaemonUtils/QBNetlinkDaemonTypes.h>

#ifndef QB_NETLINK_DAEMON_INTERNAL_TYPES_
#define QB_NETLINK_DAEMON_INTERNAL_TYPES_

/**
 * @file QBNetlinkDaemonInternalTypes.h QBNetlinkDaemonListener types
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBNetlinkDaemonInternalTypes QBNetlinkDaemonListenerInternalTypes
 * @ingroup QBNetlinkDaemonListenerTypes
 * @{
 **/

/// path where QBNetlinkDaemon stores its pidfile
#define QB_NETLINK_DAEMON_PIDFILE "/var/run/netlink/QBNetlinkDaemon.pidfile"

/// path to recall socket of the QBNetlinkDaemon
#define QB_NETLINK_LISTENER_COMMAND_PATH "/var/run/netlink/cmds"

/// A keyword for network class events
#define QB_NETLINK_DEVICE_CLASS_NET "net"

/// Function name that is included in a QBIPC message when a network device has just been added
#define QB_NETLINK_FUNCTION_ADD_NET_DEVICE "add"
/// Function name that is included in a QBIPC message when a network device has just been removed
#define QB_NETLINK_FUNCTION_REMOVE_NET_DEVICE "remove"
/// Function name that is included in a QBIPC message when a network device has just changed its link status
#define QB_NETLINK_FUNCTION_NET_DEVICE_LINK_CHANGED "link"

/**
 * Converts device class enum to string
 *
 * @param[in] deviceClass enum to be converted to string
 * @return string representation of the deviceClass
 **/
const char *
deviceClassToString(QBNetlinkDeviceClass deviceClass);

/**
 * Converts string to deviceClass enum
 *
 * @param[in] deviceClassStr string to be converted to the deviceClass enum
 * @return string representation of the deviceClass
 **/
QBNetlinkDeviceClass
stringToDeviceClass(const char *deviceClassStr);

// -------------------------Set of commands ---------------------------
/// Recall command name
#define QB_NETLINK_DAEMON_CMD_RECALL "recall"

/**
 * @}
**/

#endif /* QB_NETLINK_DAEMON_INTERNAL_TYPES_ */
