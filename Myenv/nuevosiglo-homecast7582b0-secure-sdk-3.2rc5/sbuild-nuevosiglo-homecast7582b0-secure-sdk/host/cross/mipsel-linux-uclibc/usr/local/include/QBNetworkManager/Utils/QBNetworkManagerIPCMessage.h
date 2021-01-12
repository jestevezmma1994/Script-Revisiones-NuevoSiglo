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

#ifndef QB_NETWORK_MANAGER_IPC_MESSAGE_H_
#define QB_NETWORK_MANAGER_IPC_MESSAGE_H_

/**
 * @file QBNetworkManagerIPCMessage.h QBNetworkManager recognizable message API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBNetworkManager/Utils/QBNetworkManagerTypes.h>
#include <stdbool.h>

/**
 * @defgroup QBNetworkManagerIPCMessage QBNetworkManagerIPCMessage
 * @ingroup QBNetworkManager
 * @{
 **/

/**
 * IPC messages types
 **/
typedef enum {
    QBNetworkManagerIPCMessageType_outgoing,    /**< message sent from the QBNetworkManager process to another process */
    QBNetworkManagerIPCMessageType_incoming     /**< message sent from another process to the QBNetworkManager process */
} QBNetworkManagerIPCMessageType;

/**
 * Message data (message type + method + parameters) parsed out of an QBIPC message
 **/
typedef struct QBNetworkManagerIPCMessage_ QBNetworkManagerIPCMessage;

/**
 * Get type (incoming/outgoing) of an IPC message.
 *
 * @param[in] msg   the IPC message which type will be returned
 * @return          the type (incoming/outgoing) of the IPC message
 **/
QBNetworkManagerIPCMessageType
QBNetworkManagerIPCMessageGetMessageType(const QBNetworkManagerIPCMessage *msg);

/**
 * Get type of an outgoing IPC message.
 *
 * This method should be called only for those messages,
 * which type is QBNetworkManagerIPCMessageType_outgoing!
 *
 * @param[in] msg   the outgoing IPC message which type will be returned
 * @return          the type (method) of the IPC message
 **/
QBNetworkManagerOutgoingMessage
QBNetworkManagerIPCMessageGetOutgoingMessageType(const QBNetworkManagerIPCMessage *msg);

/**
 * Get interfaceID argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          interface ID argument from the IPC message, NULL if not available
 **/
const char *
QBNetworkManagerIPCMessageGetInterfaceID(const QBNetworkManagerIPCMessage *msg);

/**
 * Get interface status argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          interface status argument from the IPC message, NULL if not available
 **/
QBNetworkManagerInterfaceStatus
QBNetworkManagerIPCMessageGetStatus(const QBNetworkManagerIPCMessage *msg);

/**
 * Get interface medium argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          interface medium argument from the IPC message, NULL if not available
 **/
QBNetworkManagerInterfaceMedium
QBNetworkManagerIPCMessageGetMedium(const QBNetworkManagerIPCMessage *msg);

/**
 * Get interface config argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          interface config argument from the IPC message, NULL if not available
 **/
QBNetworkManagerInterfaceConfig
QBNetworkManagerIPCMessageGetConfig(const QBNetworkManagerIPCMessage *msg);

/**
 * Get address provider argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          address provider argument from the IPC message, NULL if not available
 **/
QBNetworkManagerInterfaceAddressProvider
QBNetworkManagerIPCMessageGetAddressProvider(const QBNetworkManagerIPCMessage *msg);

/**
 * Get routing policy argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          routing policy argument from the IPC message, NULL if not available
 **/
QBNetworkManagerInterfaceRouting
QBNetworkManagerIPCMessageGetRouting(const QBNetworkManagerIPCMessage *msg);

/**
 * Get IP address argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          IP address argument from the IPC message, NULL if not available
 **/
const char *
QBNetworkManagerIPCMessageGetIP(const QBNetworkManagerIPCMessage *msg);

/**
 * Get netmask address argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          netmask address argument from the IPC message, NULL if not available
 **/
const char *
QBNetworkManagerIPCMessageGetNetmask(const QBNetworkManagerIPCMessage *msg);

/**
 * Get gateway address argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg               a handle to an IPC message
 * @param[in] addressProvider   addressProvider for which gateway will be returned
 * @return          gateway address argument from the IPC message, NULL if not available
 **/
const char *
QBNetworkManagerIPCMessageGetGateway(const QBNetworkManagerIPCMessage *msg,
                                     QBNetworkManagerInterfaceAddressProvider addressProvider);

/**
 * Get dns1 address argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          dns1 address argument from the IPC message, NULL if not available
 **/
const char *
QBNetworkManagerIPCMessageGetDNS1(const QBNetworkManagerIPCMessage *msg);

/**
 * Get dns2 address argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          dns2 address argument from the IPC message, NULL if not available
 **/
const char *
QBNetworkManagerIPCMessageGetDNS2(const QBNetworkManagerIPCMessage *msg);

/**
 * Get domain address argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          domain address argument from the IPC message, NULL if not available
 **/
const char *
QBNetworkManagerIPCMessageGetDomain(const QBNetworkManagerIPCMessage *msg);

/**
 * Get interface gateway tenure argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          interface gateway tenure argument from the IPC message, @c -1 if not available
 **/
int
QBNetworkManagerIPCMessageGetGatewayTenure(const QBNetworkManagerIPCMessage *msg);

/**
 * Get interface isAvailable flag (if present) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          isAvailable flag from the IPC message, @c -1 if not available
 **/
int
QBNetworkManagerIPCMessageGetIsAvailable(const QBNetworkManagerIPCMessage *msg);

/**
 * Get status of IPSec VPN.
 * If requested argument is not available in the message then
 * a suitable log will be printed and QBNetworkManagerVPNStatus_disabled value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return IPSec VPN status argument from the IPC message, QBNetworkManagerIPSecVPNStatus_disabled if not available
 */
QBNetworkManagerIPSecVPNStatus
QBNetworkManagerIPCMessageGetIPSecVPNStatus(const QBNetworkManagerIPCMessage *msg);

/**
 * Get local IP assigned by IPSec VPN.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return local IP assigned by IPSec VPN, NULL if not available
 */
const char *
QBNetworkManagerIPCMessageGetIPSecVPNLocalIP(const QBNetworkManagerIPCMessage *msg);

/**
 * Get local port assigned by IPSec VPN.
 * If requested argument is not available in the message then
 * a suitable log will be printed and -1 value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return local port assigned by IPSec VPN, -1 if not available
 */
int
QBNetworkManagerIPCMessageGetIPSecVPNLocalPort(const QBNetworkManagerIPCMessage *msg);

/**
 * Get VPN server IP assigned by IPSec VPN.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return VPN server IP assigned by IPSec VPN, NULL if not available
 */
const char *
QBNetworkManagerIPCMessageGetIPSecVPNServerIP(const QBNetworkManagerIPCMessage *msg);

/**
 * Get VPN server port assigned by IPSec VPN.
 * If requested argument is not available in the message then
 * a suitable log will be printed and -1 value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return VPN server IP assigned by IPSec VPN, -1 if not available
 */
int
QBNetworkManagerIPCMessageGetIPSecVPNServerPort(const QBNetworkManagerIPCMessage *msg);

/**
 * Get VPN IP assigned for IPSec VPN.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return VPN IP assigned for IPSec VPN, -1 if not available
 */
const char *
QBNetworkManagerIPCMessageGetIPSecVPNVPNIP(const QBNetworkManagerIPCMessage *msg);

/**
 * Get VPN netmask assigned for IPSec VPN.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return VPN netmask assigned for IPSec VPN, -1 if not available
 */
const char *
QBNetworkManagerIPCMessageGetIPSecVPNNetmask(const QBNetworkManagerIPCMessage *msg);

/**
 * Get VPN DNS assigned for IPSec VPN.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return VPN DNS assigned for IPSec VPN, -1 if not available
 */
const char *
QBNetworkManagerIPCMessageGetIPSecVPNDNS(const QBNetworkManagerIPCMessage *msg);

/**
 * Get VPN connection counter. The counter is incremented every time
 * a connection attempt fails. The counter is reset when the connection
 * attempt succeeds or VPN is disabled due to lack of WAN connectivity.
 *
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return VPN connection counter
 */
unsigned int
QBNetworkManagerIPCMessageGetIPSecConnectionCounter(const QBNetworkManagerIPCMessage *msg);

/**
 * Get WiFi SSID argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          WiFi SSID address argument from the IPC message, NULL if not available
 **/
const char *
QBNetworkManagerIPCMessageGetWiFiSSID(const QBNetworkManagerIPCMessage *msg);

/**
 * Get WiFi connection error argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          WiFi connection error argument from the IPC message, NULL if not available
 **/
QBNetworkManagerWiFiConnectionError
QBNetworkManagerIPCMessageGetWiFiConnectionError(const QBNetworkManagerIPCMessage *msg);

/**
 * Get WiFi security flags address argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          WiFi security flags address argument from the IPC message, NULL if not available
 **/
const char *
QBNetworkManagerIPCMessageGetWiFiFlags(const QBNetworkManagerIPCMessage *msg);

/**
 * Get WiFi signal level argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          WiFi signal level argument from the IPC message, NULL if not available
 **/
int
QBNetworkManagerIPCMessageGetWiFiSignalLevel(const QBNetworkManagerIPCMessage *msg);

/**
 * Get WiFi password argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          WiFi password argument from the IPC message, NULL if not available
 **/
const char *
QBNetworkManagerIPCMessageGetWiFiPasswd(const QBNetworkManagerIPCMessage *msg);

/**
 * Get WiFi security policy argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          WiFi security policy argument from the IPC message, NULL if not available
 **/
QBNetworkManagerWiFiNetworkSecurity
QBNetworkManagerIPCMessageGetWiFiSecurity(const QBNetworkManagerIPCMessage *msg);

/**
 * Get WiFi manual network flag argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          WiFi manual network flag address argument from the IPC message, NULL if not available
 **/
bool
QBNetworkManagerIPCMessageGetWiFiManualFlag(const QBNetworkManagerIPCMessage *msg);

/**
 * Get WiFi current marker argument (if available) from an IPC message.
 * If requested argument is not available in the message then
 * a suitable log will be printed and NULL value returned.
 *
 * @param[in] msg   a handle to an IPC message
 * @return          WiFi current marker argument from the IPC message, NULL if not available
 **/
bool
QBNetworkManagerIPCMessageGetWiFiCurrentFlag(const QBNetworkManagerIPCMessage *msg);


/**
 * Parse a QBIPC message of a specified type
 *
 * @param[in] ipcMessage    a QBIPC formatted string to be parsed
 * @param[in] len           len of the ipcMessage
 * @param[in] msgType       message type (outgoing, incoming)
 * @return                  a handle to created QBNetworkManagerIPCMessage, NULL in case of an error
 **/
QBNetworkManagerIPCMessage *
QBNetworkManagerIPCMessageCreateFromString(const char *ipcMessage,
                                           int len,
                                           QBNetworkManagerIPCMessageType msgType);

/**
 * Destroy message data initialized with the QBNetworkManagerUtilsParseIPCMessage function.
 *
 * @param[in] message  IPC message to be destroyed
 **/
void
QBNetworkManagerIPCMessageDestroy(QBNetworkManagerIPCMessage *message);

/**
 * @}
**/

#endif /* QB_NETWORK_MANAGER_IPC_MESSAGE_H_ */
