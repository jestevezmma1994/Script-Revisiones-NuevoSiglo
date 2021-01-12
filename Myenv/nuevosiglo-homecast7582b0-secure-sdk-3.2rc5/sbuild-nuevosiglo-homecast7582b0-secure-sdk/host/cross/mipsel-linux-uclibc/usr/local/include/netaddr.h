/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2005 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

/* SMP/ionetlib/ionet.h */

#ifndef _ionet_netaddr_h
#define _ionet_netaddr_h

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @addtogroup SvNetworking_Netaddr
  * @{
  * @file netaddr.h Network address manipulation interface
  * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
  * @endxmlonly
  */

#define psockaddr(x) ((struct sockaddr *)x)

/** Network address type. */
typedef struct sockaddr_storage netaddr;

/** Get size of ::netaddr pointer contents.
    \param[in] addr Network address
    \return Size of memory allocated for pointer, in bytes
 */
int netaddr_get_size(const netaddr *addr);

/** Get port of network address.
    \param[in] addr Network address
    \return Port of network address
 */
unsigned short netaddr_get_port(const netaddr *addr);

/** Set port of network address.
    \param[in] addr Network address to change
    \param[in] port Port to set
  */
void netaddr_set_port(netaddr *addr, unsigned short port);

/** Compare network addresses.
    \param[in] a First address
    \param[in] b Second address
    \return <0 if \a a is numerically less than \a b, or addresses are
    of different families (AF_INET and AF_INET6)
    \return 0 if \a a is numerically equal to \a b
    \return >0 if \a a is numerically greater than \a b
    \remark This function is useful as compare parameter for functions
    like stdlib's qsort().  To simply compare if two addresses are the
    same, you may rather want to use netaddr_is_equal().
 */
int netaddr_compare(const netaddr *a, const netaddr *b);

/** Check if two network addresses are equal.
    \param[in] a First address
    \param[in] b Second address
    \return Non-zero if addresses are equal
    \return 0 if addresses are different
 */
int netaddr_is_equal(const netaddr *a, const netaddr *b);

/** Check if network address is loopback.
    \param[in] a Address to check
    \return Non-zero if address is a loopback address
    \return 0 if address is not a loopback address
 */
int netaddr_is_loopback(const netaddr *a);

/** Check if network address is unspecified.
    \param[in] a Address to check
    \return Non-zero if address is unspecified
    \return 0 if address is specified
    \remark Unspecified address is netaddr equivalent of IPv4 INADDR_ANY.
 */
int netaddr_is_any(const netaddr *a);

/** Check if network address is multicast.
    \param[in] a Address to check
    \return Non-zero if address is multicast
    \return 0 if address is not multicast
 */
int netaddr_is_multicast(const netaddr *a);

/** Create ::netaddr pointer for unspecified address.
    \param[out] size Pointer content size, in bytes.
    \param[in] dst ::netaddr pointer to fill, or \a NULL to allocate new pointer
    \return \a dst or newly allocated ::netaddr pointer on success
    \return NULL on failure
    \remark This is netaddr equivalent of IPv4 INADDR_ANY. Such
    address for opening a listening socket that should listen on all
    available interfaces.
  */
netaddr *netaddr_get_any(socklen_t *size, netaddr *dst);

/** Create netaddr pointer for specified IP number string.
    \param[in] taddr IP number as string, either in dotted-quad
    "aaa.bbb.ccc.ddd" IPv4 format, or in any allowed IPv6 address
    format
    \param[out] size Pointer content size, in bytes.
    \param[in] dst ::netaddr pointer to fill, or \a NULL to allocate new pointer
    \return \a dst or newly allocated ::netaddr pointer on success
    \return NULL on failure
  */
netaddr *netaddr_get_address(const char* taddr, socklen_t *size, netaddr *dst);

/** Create a copy of network address.
    \param[in] src Source ::netaddr pointer to copy
    \param[out] size Pointer content size, in bytes.
    \param[in] dst ::netaddr pointer to fill, or \a NULL to allocate new pointer
    \return \a dst or newly allocated ::netaddr pointer on success
    \return NULL on failure
  */
netaddr *netaddr_copy(const netaddr *src, socklen_t *size, netaddr *dst);

/** Convert AF_INET (IPv4) ::netaddr pointer to an AF_INET6 (IPv6) one.
    \param[in] src AF_INET ::netaddr pointer to convert
    \param[out] size Pointer content size, in bytes.
    \param[in] dst ::netaddr pointer to fill, or \a NULL to allocate new pointer
    \return \a dst or newly allocated ::netaddr pointer on success
    \return NULL on failure
  */
netaddr *netaddr_convert(const netaddr *src, socklen_t *size, netaddr *dst);

/** Get string representation of ::netaddr.
    \param[in] addr Network address
    \return String representation of \a addr
  */
const char *netaddr_as_string(const netaddr *addr);

/** Get string representation of ::netaddr.
    \param[in] addr Network address
    \param[in] buf Return buffer
    \param[in] maxlen Length of the return buffer
    \return String representation of \a addr
  */
const char* netaddr_as_string2(const netaddr *addr, char* buf, int maxlen);

/** Free memory allocated for a ::netaddr structure.
    \param[in] addr Network address.
  */
void netaddr_free(netaddr *addr);

/** Check if given ip address is from "private" address range.
    \param[in] addr Network address
    \return non-zero iff given address is from "private" range
  */
int netaddr_is_from_private_network(const netaddr *addr);

int netaddr_is_from_private_network_str(const char* ip);

/**
  * @}
  */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
