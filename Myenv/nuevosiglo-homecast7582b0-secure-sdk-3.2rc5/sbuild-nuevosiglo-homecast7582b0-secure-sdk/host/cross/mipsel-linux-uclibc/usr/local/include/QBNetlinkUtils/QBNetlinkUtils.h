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

#ifndef QB_NETLINK_UTILS_
#define QB_NETLINK_UTILS_

/**
 * @file QBNetlinkUtils.h QBNetlinkUtils API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


#include <stdbool.h>
#include <stdint.h>
#include <net/if.h>
#include <linux/rtnetlink.h>

/**
 * @defgroup QBNetlinkUtils Utility netlink functions
 * @{
 **/

#undef RTA_NEXT
/** Redefinition of RTA_NEXT netlink related macro. It is not properly defined in rtnetlink.h
 *  and generates alignment related compilation warning on many architectures when "-Wcast-align"
 *  flag is enabled.
 **/
#define RTA_NEXT(rta, attrlen)  ((attrlen) -= RTA_ALIGN((rta)->rta_len), \
                                 (struct rtattr *) (void *) (((char *) (rta)) + RTA_ALIGN((rta)->rta_len)))

#undef RTM_RTA
/** Redefinition of RTM_RTA netlink related macro. It is not properly defined in rtnetlink
 *  and generates alignment related compilation warning on many architectures when "-Wcast-align"
 *  flag is enabled.
 **/
#define RTM_RTA(r)      ((struct rtattr *) (void *) (((char *) (r)) + NLMSG_ALIGN(sizeof(struct rtmsg))))

#undef IFLA_RTA
/** Redefinition of IFLA_RTA netlink related macro. It is not properly defined in if_link.h
 *  and generates alignment related compilation warning on many architectures when "-Wcast-align"
 *  flag is enabled.
 **/
#define IFLA_RTA(r)     ((struct rtattr *) (void *) (((char *) (r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))

#undef NLMSG_NEXT
/** Redefinition of NLMSG_NEXT netlink related macro. It is not properly defined in netlink.h
 *  and generates alignment related compilation warning on many architectures when "-Wcast-align"
 *  flag is enabled.
 **/
#define NLMSG_NEXT(nlh, len)    ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
                                 (struct nlmsghdr *) (void *) (((char *) (nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))

/**
 * Send a netlink message o a socket.
 *
 * @param[in] netlinkMsgType    netlink message type (e.g., RTM_GETLINK, RTM_GETROUTE)
 * @param[in] socket            AF_NETLINK socket on which the message will be sent
 * @return                      0 on success, errno otherwise
 */
int
QBNetlinkSendNetlinkMessage(__u16 netlinkMsgType, int socket);

/**
 * Read incoming netlink event.
 *
 * @param[in] sockfd            AF_NETLINK socket on which the message is expected to be received
 * @param[out] buf              buf where the content of the event will be stored
 * @param[in] bufSize           size of the buf
 * return   the length of the message in bytes in case of success.
 *          If no messages are available to be received and the peer has performed an orderly shutdown, zero shall be returned.
 *          Otherwise, -1 shall be returned and errno set to indicate the error.
 */
int
QBNetlinkReceiveEvent(int sockfd, char *buf, size_t bufSize);

/**
 * @}
**/

#endif /* QB_NETLINK_UTILS_ */
