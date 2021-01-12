/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_ASYNC_RESOLVER_H
#define QB_ASYNC_RESOLVER_H

/**
 * @file QBAsyncResolver.h QBAsyncResolver library API
 **/

#include <stdbool.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBAsyncResolver QBAsyncResolver: asynchronous DNS name resolver
 * @ingroup Networking
 * @{
 *
 * QBAsyncResolverRequest object represents a single asynchronous query to DNS server.
 * Request is processed asynchronously using default instance of SvScheduler
 * (so that scheduler must be running for the engine to work.)
 * Results are returned using a callback provided at request's creation time.
 * Request can be dropped (cancelled) at any time.
 *
 * For usage example see @ref AsynchronousDNSNameResolver.
 **/

/**
 * @page ReferenceManual
 *
 * @section AsynchronousDNSNameResolver Using asynchronous DNS name resolver
 * Standard DNS name resolver provided by the C library (gethostbyname() and
 * getaddrinfo() functions) are blocking, so they are not suitable for use
 * in interactive GUI code. QBAsyncResolver library provides simple interface
 * for resolving DNS domain in asynchronous fashion.
 *
 * @code
 * #include <SvCore/SvLog.h>
 * #include <QBAsyncResolver.h>
 * #include <netinet/in.h>
 * #include <arpa/inet.h>
 *
 * static void dns_callback(void* prv,
 *                          QBAsyncResolverRequest* req,
 *                          int status,
 *                          int addrCnt,
 *                          struct sockaddr_storage* addrs)
 * {
 *     if (status == QB_ASYNC_RESOLVER_SUCCESS) {
 *         int i;
 *         for (i = 0; i < addrCnt; i++) {
 *             // always check address family!
 *             if (addrs[i].ss_family == AF_INET) {
 *                 const struct sockaddr_in *addr = (struct sockaddr_in *) &addrs[i];
 *                 SvLogNotice("%d: %s", i, inet_ntoa(addr->sin_addr));
 *             } else {
 *                 const struct sockaddr_in6 *addr = (struct sockaddr_in6 *) &addrs[i];
 *                 char str[INET6_ADDRSTRLEN];
 *                 inet_ntop(AF_INET6, &(addr->sin6_addr), str, sizeof(str));
 *                 SvLogNotice("%d: %s", i, str);
 *             }
 *         }
 *     } else if (status == QB_ASYNC_RESOLVER_TIMEOUT) {
 *         SvLogError("DNS request for domain name %s timed out", QBAsyncResolverRequestGetDomainName(req));
 *     } else if (status == QB_ASYNC_RESOLVER_NOT_FOUND) {
 *         SvLogWarning("can't resolve domain name %s", QBAsyncResolverRequestGetDomainName(req));
 *     } else {
 *         SvLogError("DNS request for domain name %s failed", QBAsyncResolverRequestGetDomainName(req));
 *     }
 * }
 *
 * QBAsyncResolverRequest* req;
 * req = QBAsyncResolverGetHostByName("www.cubiware.com", AF_INET, -1, dsn_callback, NULL);
 * @endcode
 **/

/// Resolving finished with some results.
#define QB_ASYNC_RESOLVER_SUCCESS 0
/// Resolving failed with an error.
#define QB_ASYNC_RESOLVER_FAIL 1
/// Resolving finished, but found no results.
#define QB_ASYNC_RESOLVER_NOT_FOUND 2
/// Resolving finished without receiving any answer.
#define QB_ASYNC_RESOLVER_TIMEOUT 3


/**
 * Asynchronous DNS request type.
 **/
typedef struct QBAsyncResolverRequest_s QBAsyncResolverRequest;


/**
 * Asynchronous DNS request callback function type.
 *
 * @note After this callback fires, the request will be destroyed automatically.
 * Do not destroy (cancel) the request in the callback nor after.
 * @param[in] prv     opaque data pointer bound to the callback
 * @param[in] req     finished DNS request
 * @param[in] status  status of the asynchronous DNS request
 * @param[in] addrCnt number of addresses in @a addrs array
 * @param[in] addrs   array of addresses (either IPv4 or IPv6)
 *                    associated with requested domain name
 **/

typedef void (QBAsyncResolverHostCallback)(void* prv,
                                           QBAsyncResolverRequest* req,
                                           int status,
                                           int addrCnt,
                                           struct sockaddr_storage* addrs);

/**
 * Create and start new asynchronous DNS request.
 *
 * @note In case of failure when @a family == @c AF_INET6 (no IPv6 address found), IPv4 resolution is
 * attempted and you might get IPv4 address in the callback. Make sure you check value of address family
 * in the callback!
 *
 * @param[in] domainName name of the host to resolve
 * @param[in] family   @c AF_INET, @c AF_INET6, @c -1 for default (@c AF_INET)
 * @param[in] timeout  time (in milliseconds) after which request will timeout (FIXME: what to pass here if I want to wait forever?)
 * @param[in] retryCnt how many times the request should be retried
 * @param[in] callback function that will be called when request is finished (either successfully, by timeout or error)
 * @param[in] target   opaque data pointer for @a callback function
 * @return             created asynchronous DNS request handle, @c NULL in case of internal error
 **/
extern QBAsyncResolverRequest*
QBAsyncResolverGetHostByName(const char* domainName,
                             int family,
                             int timeout,
                             int retryCnt,
                             QBAsyncResolverHostCallback* callback,
                             void* target);
/**
 * Get DNS domain name for which this DNS request was created.
 *
 * @param[in] req   asynchronous DNS request handle
 * @return          DNS domain name
 **/
extern const char*
QBAsyncResolverRequestGetDomainName(const QBAsyncResolverRequest* req);

/**
 * Cancel ongoing request.
 *
 * This function cancels ongoing DNS request.
 * Callback is not called for a cancelled request.
 *
 * @param[in] req   asynchronous DNS request handle
 **/
extern void
QBAsyncResolverCancelRequest(QBAsyncResolverRequest* req);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_ASYNC_RESOLVER_H
