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

#ifndef QB_NETLINK_DAEMON_TYPES_
#define QB_NETLINK_DAEMON_TYPES_

/**
 * @file QBNetlinkDaemonTypes.h QBNetlinkDaemonListenerTypes
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBNetlinkDaemonTypes QBNetlinkDaemonListener types
 * @ingroup QBNetlinkDaemonListener
 * @{
 **/

/// path where listeners IPC sockets are created
#define QB_NETLINK_DAEMON_LISTENER_PATH "/var/run/netlink/listeners/"

/*************************CONSTANTS COMMON ACROSS DEVICES*************************/

/**
 * Class of a QBIPC notification from QBNetlinkDaemon.
 */
typedef enum {
    QBNetlinkDeviceClass_net, /**< networking notification class */
    QBNetlinkDeviceClass_usb, /**< USB notification class */
    QBNetlinkDeviceClass_tty, /**< TTY notification class */
    QBNetlinkDeviceClass_cnt  /**< count of notification classes */
} QBNetlinkDeviceClass;

/**
 * Parameter type of an QBIPC message. There are only two first
 * parameters declared in the enum, because they are always common
 * for all types of messages.
 *
 * QBIPC messages are constructed in following template:
 * \#class\#functionName\#param1\#...\#paramN\#
 *
 * Example:
 * \#net#add\#eth0\#
 */
typedef enum {
    QBNetlinkNotificationParamType_class,           /**< device class parameter of the QBIPC message */
    QBNetlinkNotificationParamType_functionName,    /**< function name parameter of the QBIPC message */
    QBNetlinkNotificationParamType_cnt              /**< count of common QBIPC messages parameters types */
} QBNetlinkNotificationParamType;

/**
 * Parameters of "add" IPC function
 */
typedef enum {
    QBNetlinkAddDeviceFunctionArg_devID,            /**< device ID */
    QBNetlinkAddDeviceFunctionArg_cnt               /**< count of parameters */
} QBNetlinkAddDeviceFunctionArg;

/**
 * Parameters of "remove" IPC function
 */
typedef enum {
    QBNetlinkRemoveDeviceFunctionArg_devID,         /**< device ID */
    QBNetlinkRemoveDeviceFunctionArg_cnt            /**< count of parameters */
} QBNetlinkRemoveDeviceFunctionArg;

/*************************CONSTANTS FOR NETWORK DEVICES*************************/

/**
 * QBIPC function types for network devices.
 */
typedef enum {
    QBNetlinkNetClassNotificationFunctionName_add,      /**< add a network device function type */
    QBNetlinkNetClassNotificationFunctionName_remove,   /**< remove a network device function type */
    QBNetlinkNetClassNotificationFunctionName_link,     /**< link changed on a network device function type */
    QBNetlinkNetClassNotificationFunctionName_cnt       /**< count of function types for network devices */
} QBNetlinkNetClassNotificationFunctionName;

/**
 * Parameters of "link" IPC function
 */
typedef enum {
    QBNetlinkNetClassLinkFunctionArg_interfaceID,       /**< network interface ID */
    QBNetlinkNetClassLinkFunctionArg_isUp,              /**< is network interface up */
    QBNetlinkNetClassLinkFunctionArg_isRunning,         /**< is network interface running */
    QBNetlinkNetClassLinkFunctionArg_cnt                /**< count of parameters */
} QBNetlinkNetClassLinkFunctionArg;

/*************************CONSTANTS FOR CMDS TO NETLINK DAEMON*************************/

/**
 * Common arguments for commands to be sent to a QBNetlinkDaemon.
 **/
typedef enum {
    QBNetlinkDaemonCommandArg_cmdName = 0,  /**< command name */
    QBNetlinkDaemonCommandArg_cnt           /**< count of common arguments */
} QBNetlinkDaemonCommandArg;

/**
 * Commands that can be sent to a QBNetlinkDaemon
 **/
typedef enum {
    QBNetlinkDaemonCommand_recall = 0,  /**< recall command - triggers notification about class of devices */
    QBNetlinkDaemonCommand_cnt          /**< count of commands */
} QBNetlinkDaemonCommand;

/**
 * Recall command arguments
 **/
typedef enum {
    QBNetlinkDaemonCommandRecallArg_deviceClass = 0,    /**< device class to be recalled */
    QBNetlinkDaemonCommandRecallArg_cnt                 /**< count of recall command arguments */
} QBNetlinkDaemonCommandRecallArg;

/**
 * @}
**/

#endif /* QB_NETLINK_DAEMON_TYPES_ */
