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

#ifndef QB_NETLINK_DAEMON_UTILS_
#define QB_NETLINK_DAEMON_UTILS_

/**
 * @file QBNetlinkDaemonUtils.h QBNetlinkDaemon utils
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBNetlinkDaemonUtils/QBNetlinkDaemonTypes.h>
#include <stdbool.h>

#define DEVICE_UID          1002
#define DEVICE_GID          1002

#define APP_UID             1000
#define APP_GID             1000

#define NETWORK_UID         1001
#define NETWORK_GID         1001

/**
 * @defgroup QBNetlinkDaemonUtils QBNetlinkDaemonUtils
 * @ingroup QBNetlinkDaemon
 * @{
 **/

/**
 * Netlink message class that is created from a QBIPC message
 * received from QBNetlinkDaemon.
 *
 * @class QBNetlinkMessage
 **/
typedef struct QBNetlinkMessage_ {
    QBNetlinkDeviceClass deviceClass;   /**< class of a QBIPC notification from QBNetlinkDaemon */

    union {
        QBNetlinkNetClassNotificationFunctionName netFunctionName;  /**< net class function name */
    } functionName;     /**< function name depending on a notificationClass */

    char **functionParams;  /**< an array of parameters of a function */
    int functionParamsCnt;  /**< count of functionParams array */
} QBNetlinkMessage;

/**
 * Recall the QBNetlinkDaemon process to make it send all the
 * information about a specifed devices class via QBIPC.
 *
 * @param[in] devClass  a class of devices to be recalled by the QBNetlinkDaemon
 **/
void
QBNetlinkDaemonUtilsRecallDevices(QBNetlinkDeviceClass devClass);

/**
 * Check if QBNetlinkDaemon process is running.
 * The function checks if a pidfile of the process is available.
 *
 * @return  true if running, false otherwise
 **/
bool
QBNetlinkDaemonUtilsIsDaemonAlive(void);

/**
 * Deserialize a QBIPC message to a QBNetlinkMessage instance
 * @param[in] ipcMessage    an ipc message to be deserialized
 * @param[in] msgLen        size of the ipcMessage
 * @param[out] outMsg       a QBNetlinkMessage that mirrors deserialized ipcMessage
 * @return  zero on success, -1 otherwise
 **/
int
QBNetlinkDaemonUtilsDeserializeIPCMessage(const char *ipcMessage, const int msgLen, QBNetlinkMessage *outMsg);

/**
 * Destroy a netlink message.
 * The function must be called after QBNetlinkDaemonUtilsDeserializeIPCMessage,
 * nevertheless of a returned value.
 *
 * @param[in] msg   handle to a QBNetlinkMessage to be destroyed
 **/
void
QBNetlinkMessageDestroy(QBNetlinkMessage *msg);

/**
 * @}
**/

#endif /* QB_NETLINK_DAEMON_UTILS_ */
