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

/**
  *  @addtogroup SvNetworking
  *  @{
  *  @file ionet.h Networking I/O abstraction layer interface file
  * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
  * @endxmlonly
  */


#ifndef _ionet_h
#define _ionet_h

#include <stdio.h>
#include "netaddr.h"
#include "proxysettings.h"

#include <SvCore/SvTime.h>

#include <sv-list.h>
#include <net/if.h>
#include <sys/uio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Endpoint types. */
enum epoint_type {
  EP_TCP = 0,                   /**< TCP endpoint */
  EP_UDP,                       /**< UDP endpoint */
  EP_SSL,                       /**< SSL encrypted endpoint */
  EP_FILE,                      /**< File input/output endpoint */
  EP_NETIO,            /**< UDP endpoint with accelerated packet IO */
  EP_VIRTUAL,                  /**< Callback-based virtual endpoint */
  EP_FILE_STORAGE,              /**< SvStorage endpoint */
};

/** Result codes returned by various functions. */
enum ep_result {
  EP_RES_SUCCESS = 0,           /**< Success */
  EP_RES_ERROR = -1,            /**< An error occured */
  EP_RES_TIMEOUT = -2,          /**< Timeout reached */
  EP_RES_INTR = -4,             /**< Interrupted system call */
  EP_RES_CLOSED = -8,           /**< connection closed */
};

/** Pointer to structure representing an endpoint. */
typedef struct epoint_t *epoint;

/** Endpoint connection state. */
enum epoint_connect_state {
  epoint_conn_disconnected = 0, /**< Endpoint is disconnected */
  epoint_conn_connected,        /**< Endpoint is connected */
  epoint_conn_pending,          /**< Data is pending (only with non-blocking sockets) */
};

/** Read callback type for EP_VIRTUAL */
typedef int (*epoint_virtual_receive_data)(void* private_data, unsigned char* data, int length, unsigned int timeout);

/* Endpoint structure (private). */
struct epoint_t {
  enum epoint_type type;        /* Type of endpoint */

  int fd;                          /* Socket file descriptor */
  int epollfd;                     /* Epoll descriptor */
  int netiofd;                     /* Netio descriptor */
  unsigned short localport;        /* Local port number, in network byte order */
  /* Current connection state.
      Meaningful for connection oriented endpoints only.  Connectionless
      endpoits are always ::epoint_conn_connected. */
  enum epoint_connect_state conn_state;
  netaddr peer;                    /* Peer ip address */
  netaddr local;                   /* Local ip address */
  int nonblock;
  int bound;

  int multicast;
  netaddr group;                   /* Multicast group address.  Meaningful for multicast endpoints only. */
  netaddr group_local;             /* Multicast local interface address */
  int if_index;                    /* Multicast local interface index */
  int ttl;                         /* Multicast TTL. Not meaningful for non-multicast endpoints. */
  int joined;
  int loopback;

  FILE *fp;                        /* File handle. */
  int loop_file;
  int eof_file;

  SvTime last_stats_print;

  void *prvdata;
  epoint_virtual_receive_data ep_virtual_receive_data;
};

/**
 * Endpoint address information.
 * Function epoint_get_local_addresses() stores result in this structure.
 * @warning Perhaps you need ::epoint_local_interface struct and epoint_get_local_interfaces()
 */
struct epoint_addresses_info {
  netaddr *addresses;           /**< Local addresses addresses */
  int size;                     /**< Number of addresses in \a addresses */
  unsigned short port;          /**< Port number */
};


/**
  * Structure describing local network interaces as returned by
  * epoint_get_local_interfaces().
  */
struct epoint_local_interface{
  int interface_index;              /**< Interface index */
  char interface_name[IFNAMSIZ];    /**< Interface name */
  netaddr addr;                     /**< Interface address */
};

/** Initialize ionet library
  */
void ionet_init(void);

/** Deinitialize ionet library
  */
void ionet_deinit(void);

/**
  *  @}
  *  @addtogroup SvNetworking_EPCreation
  *  @{
  */

/**
 * Create connection epoint of a specified type.
 * \param[in] type One of: ::EP_SSL, ::EP_TCP, ::EP_UDP, ::EP_NETIO, ::EP_FILE
 * \param[in] do_bind If true then bind to a specified port and an address, otherwise do not bind
 * \param[in] port Port number to bind to (in the network byte order) or 0 to find random port number
 * \param[in] local Local address to bind to
 * \param[in] reuse Reuse address when binding
 * \return New epoint structure or NULL in case of an error
 */
epoint epoint_create(enum epoint_type type, int do_bind, unsigned short port, netaddr *local, int reuse);

/**
 * Create connection epoint of a specified type and establish connection to given address.
 * \param[in] type One of: ::EP_SSL, ::EP_TCP, ::EP_UDP, ::EP_NETIO
 * \param[in] do_bind If true then bind to a specified port and an address, otherwise do not bind
 * \param[in] port Port number to bind to (in the network byte order) or 0 to find random port number
 * \param[in] local Local address to bind to
 * \param[in] remote Remote address to connect
 * \param[in] reuse Reuse address when binding
 * \return New epoint structure or NULL in case of an error
 */
epoint epoint_create_connected(enum epoint_type type, int do_bind, unsigned short port,
                               netaddr *remote, netaddr *local, int reuse);

/**
 * Create multicast epoint.
 * \param[in] type Type of socket to create
 * \param[in] do_bind If true then bind to a specified port and an address, otherwise do not bind
 * \param[in] port Port number to bind to (in the network byte order) or 0 to find random port number
 * \param[in] group IP address of multicast group to join
 * \param[in] ttl TTL value to set (use 0 for default).  Meaningful when sending packets to multicast group.
 * \param[in] join If true then join a multicast group.
 * \param[in] loop If true then loop multicast traffic back.  Helpful when runing server and client on the same host.
 * \param[in] local IP address of the interface to which multicast traffic should be sent.
 * Use INADDR_ANY (netaddr_get_any() return value)to go with default settings.
 * \param[in] reuse Reuse address when binding
 * \return New epoint structure or NULL in case of an error
 */
epoint epoint_create_multicast(enum epoint_type type, int do_bind, unsigned short port,
                               netaddr *group, int ttl, int join, int loop,
                               netaddr *local, int reuse);

/**
 * Create source group multicast epoint.
 * This will istruct gateway to enable routing multicast packets only
 * from a specific source address. Source multicast is supported in
 * IGMPv3 (IPv4) and MLDv2 (IPv6).
 * \param[in] type Type of socket to create
 * \param[in] do_bind If true then bind to a specified port and an address, otherwise do not bind
 * \param[in] port Port number to bind to (in the network byte order) or 0 to find random port number
 * \param[in] group IP address of multicast group to join
 * \param[in] source the address of the multicast group source
 * \param[in] ttl TTL value to set (use 0 for default).  Meaningful when sending packets to multicast group.
 * \param[in] join If true then join a multicast group.
 * \param[in] loop If true then loop multicast traffic back.  Helpful when runing server and client on the same host.
 * \param[in] local IP address of the interface to which multicast traffic should be sent.
 * Use INADDR_ANY (netaddr_get_any() return value)to go with default settings.
 * \param[in] reuse Reuse address when binding
 * \return New epoint structure or NULL in case of an error
 */
epoint epoint_create_source_multicast(enum epoint_type type, int do_bind, unsigned short port,
                                      netaddr *group, netaddr *source, int ttl, int join, int loop,
                                      netaddr *local, int reuse);

/**
 * Create file epoint.
 * \param[in] fn Name of file to open
 * \param[in] loop If true then read a file in an infinite loop
 * \return New epoint structure or NULL in case of an error
 */
epoint epoint_create_file(char *fn, int loop);


/**
 * Create virtual epoint
 * \param[in] prvdata Pointer that will be passed as \a private_data parameter of callback function.
 * \param[in] ep_virtual_receive_data Pointer to a read callback function.
 * \return New epoint structure or NULL in case of an error
 */
epoint epoint_create_virtual(void *prvdata, epoint_virtual_receive_data ep_virtual_receive_data);

/**
  *  @}
  *  @addtogroup SvNetworking_EPManagment
  *  @{
  */

/**
 * Set non-blocking mode
 * \param[in] ep A socket epoint
 * \return ::EP_RES_SUCCESS if success, error code otherwise
 */
enum ep_result epoint_set_nonblocking(epoint ep);

/**
 * Set listen queue length for connection oriented epoints
 * \param[in] ep A TCP server endpoint
 * \param[in] queue_length Requested listen queue length
 * \return ::EP_RES_SUCCESS if success, error code otherwise
 */
enum ep_result epoint_set_listen_queue(epoint ep, int queue_length);

/**
 * Get port number of server endpoint.
 * \param[in] ep A TCP server endpoint
 * \return Port number that \a ep is bound to, in network byte order.
 */
unsigned short epoint_get_localport(epoint ep);

/**
 * Get connection state of an endpoint.
 * \param[in] ep Endpoint
 * \return Endpoint's connection state
 * \remark This function is meaningful for connection-oriented (TCP or
 * SSL) endpoints only.  Connectionless endpoints will always return
 * ::epoint_conn_connected.
 */
enum epoint_connect_state epoint_get_connect_state(epoint ep);

/**
 * Connect epoint to server.
 * Meaningful for TCP and SSL epoints only.  Connectionless sockets
 * will automatically get "connected" status. Check endpoint's
 * connection state with epoint_get_connect_state() to find out the
 * result of the connection.  Non-blocking sockets _can_ return with
 * ::epoint_conn_pending status (not implemented for SSL (yet)), use
 * epoint_connect_check() later to progress/finish connecting.
 * \param[in] ep A TCP or SSL epoint
 * \param[in] addr Server IP address
 * \return ::EP_RES_SUCCESS if success, error code otherwise
 */
enum ep_result epoint_connect(epoint ep, const netaddr* addr);

/**
 * Checks if non-blocking connect has completed.
 * Calling code has to check \a ep -> \a conn_state for the final result.
 * \return ::EP_RES_SUCCESS if success, error code otherwise
 */
enum ep_result epoint_connect_check(epoint ep);

/**
 * Accept connection from remote host.
 * Meaningful for TCP server epoints only.
 * \param[in] ep Server epoint
 * \param[out] addr Place for IP address of connected client
 * \param[out] addr_size Place for size of \a addr
 * \return Pointer to epoint handle with opened connection or NULL in case of an error
 */
epoint epoint_accept(epoint ep, netaddr* addr, socklen_t *addr_size);

/**
 * Disconnect endpoint.
 * Meaningful for connection-oriented endpoints only.
 * \return ::EP_RES_SUCCESS if success, error code otherwise
 */
enum ep_result epoint_disconnect(epoint ep);

/**
 * Destroy endpoint.
 * \param[in] ep Endpoint to destroy.
 * \return ::EP_RES_SUCCESS if success, error code otherwise
 */
enum ep_result epoint_destroy(epoint ep);

/**
  *  @}
  *  @addtogroup SvNetworking_EPDataIO
  *  @{
  */

/**
 * Send data to default peer.
 * Data can be sent over connected or connectionless endpoint only.
 * Connectionless endpoint can be used only if peer has been
 * specified with epoint_set_default_peer().
 * \param[in] ep Connected or connectionless endpoint
 * \param[in] data Data to transfer
 * \param[in] length Length of \a data, maximum number of bytes to send.  Should be greater than zero.
 * \return Number of sent bytes on success
 * \return Error code (negative value from ::ep_result) on error or timeout
 */
int epoint_send_data(epoint ep, unsigned char* data, int length);

int epoint_send_data_v(epoint ep, const struct iovec *iov, int iovcnt);

/**
 * Send data to default peer with timeout.
 * Data can be sent over connected or connectionless endpoint only.
 * Connectionless endpoint can be used only if peer has been
 * specified with epoint_set_default_peer().
 * \param[in] ep Connected or connectionsless endpoint
 * \param[in] data Data to transfer
 * \param[in] length Length of \a data buffer, maximum number of bytes to send.  Should be greater than zero.
 * \param[in] timeout Timeout in microseconds
 * \return Number of sent bytes on success.
 * \return Error code (negative value from ::ep_result) on error or timeout
 *
 */
int epoint_send_data_timeout(epoint ep, unsigned char* data, int length, unsigned int timeout);

/**
 * Send data to specified peer.
 * This function can be used only with connectionless endpoint.
 * \param[in] ep Connectionless endpoint
 * \param[in] data Data to transfer
 * \param[in] length Length of \a data, maximum number of bytes to send.  Should be greater than zero.
 * \param[in] peer Peer address to send data to
 * \return Number of sent bytes on success
 * \return Error code (negative value from ::ep_result) on error or timeout
 */
int epoint_send_data_to(epoint ep, unsigned char* data, int length, const netaddr* peer);

/**
 * Set default peer address.
 * It will be used in epoint_send_data() and epoint_send_data() for
 * connectionless epoints.
 * \param[in] ep Connectionless endpoint
 * \param[in] peer Peer address to set
 */
void epoint_set_default_peer(epoint ep, netaddr* peer);

/**
 * Receive data on endpoint.
 * \param[in] ep Connected or connectionsless endpoint
 * \param[in] data Pointer to a buffer for received data
 * \param[in] length Length of \a data buffer, maximum number of bytes to receive.  Should be greater than zero.
 * \param[in] timeout Timeout in microseconds
 * \param[in] from Peer address to use for connectionless endpoints
 * \param[in] from_size Size of \a from.
 * \return Number of read bytes on success
 * \return 0 on eof
 * \return Error code (negative value from ::ep_result) on error or timeout
 */
int epoint_receive_data(epoint ep, unsigned char* data, int length, unsigned int timeout,
                        netaddr* from, socklen_t *from_size);

int epoint_receive_data_v(epoint ep, const struct iovec *iov, int iovcnt);

/**
 * Receive fixed amount of data.  This function reads
 * exactly \a length bytes.
 * \param[in] ep Connected or connectionsless endpoint
 * \param[in] data Pointer to a buffer for received data
 * \param[in] length Length of \a data buffer, maximum number of bytes to receive.  Should be greater than zero.
 * \param[in] timeout Timeout in microseconds
 * \param[in] from Peer address to use for connectionless endpoints
 * \param[in] from_size Size of \a from.
 * \return ::EP_RES_TIMEOUT on timeout
 * \return ::EP_RES_ERROR on error
 * \return Number of read bytes (\a length) on success
 * \return 0 on eof
 * \return Error code (negative value from ::ep_result) on error or timeout
 */
int epoint_receive_data_exact(epoint ep, unsigned char* data, int length, unsigned int timeout,
                              netaddr* from, socklen_t *from_size);


/**
 * Wait until data is available on the epoint or sync fd. TCP epoint only.
 * \param[in] ep TCP epoint
 * \param[in] syncfd Open file on socket descriptor
 * \return 0 if there is data on syncfd
 * \return 1 if there is data on epoint
 */
int epoint_wait_for_data(epoint ep, int syncfd);

/**
  *  @}
  *  @addtogroup SvNetworking_EPManagment
  *  @{
  */
/**
 * Grow socket send or receive buffer size.
 * \param[in] ep Endpoint
 * \param[in] dir 1 for send buffers, 0 for recv buffers
 * \param[in] size Required buffer size
 * \return New buffer size.  It may be lower than requested because of
 * max buffer size set in linux kernel. This may be tuned via /proc.
 */
int epoint_inc_buffer(epoint ep, int dir, int size);

/**
 * Get local IP address of endpoint
 * \param[in] ep Socket endpoint
 * \param[out] addr_size Size of \a dst
 * \param[out] dst Network address variable to fill with local address info, or \a NULL to allocate new ::netaddr structure
 * \return First found, non-loopback IP address
 * \return \a ANY if no address is found
 */
netaddr *epoint_get_local_address(epoint ep, socklen_t *addr_size, netaddr *dst);

netaddr *epoint_get_interface_address(epoint ep, socklen_t *addr_size, netaddr *dst, const char *interface_name);
/**
 * Get all non-loopback IP addresses of endpoint
 * \param[in] ep Socket endpoint
 * \param[out] info Pointer to a structure containing the addresses
 * \return ::EP_RES_SUCCESS on success
 * \return ::EP_RES_ERROR on error
 */
int epoint_get_local_addresses(epoint ep, struct epoint_addresses_info *info);

/**
 * Free local addresses structure returned by epoint_get_local_addresses().
 */
void epoint_free_local_addresses(struct epoint_addresses_info *info);

/**
 * Check whether provided ip address is one of local addreses.  This
 * function does not require synchronization - it is read only.
 * \param[in] ep Socket endpoint
 * \param[in] addr Address to check
 * \return Non-zero if \a addr is \a ep's local address, zero otherwise
 */
int epoint_is_local_address(epoint ep, netaddr *addr);

/**
 * Same as epoint_get_local_addresses(), but provides interface name and interface index
 * \param[in] ep Socket endpoint
 * \return List of ::epoint_local_interface structures
 */
list_t* epoint_get_local_interfaces(epoint ep);

void check_socket_stats(epoint ep);

/**
  *  @}
  */

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @}
 */

#endif
