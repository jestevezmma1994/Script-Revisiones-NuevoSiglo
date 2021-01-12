/*****************************************************************************
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
**
** This file is dual-licensed: Cubiware License 1.0 or GPLv2. Please read the
** respective license statements below. You are only allowed to "link" this header
** with Cubiware proprietary software when choosing Cubiware License 1.0.,
** otherwise you may use GPLv2 license.
**
** Cubiware Sp. z o.o. Software License Version 1.0
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
**
**  GPLv2 License:
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License along
**  with this program; if not, write to the Free Software Foundation, Inc.,
**  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**
*****************************************************************************/

#ifndef _SV_MOD_NETIO_H_
#define _SV_MOD_NETIO_H_

#ifndef MODULE
#include <sys/ioctl.h>
#include <sys/time.h> // needed for "struct timeval".
#include <sys/uio.h> // needed for "struct iovec".
#endif

#define NETIO_CHAR_MAJOR        161

typedef struct netio_s {
  int fd;                       // original _socket_ file descriptor
  unsigned flags;               // as per recvfrom
  int minpkt;                   // minimum number of packets to wait for
  int maxdelay;                 // maximum latency (used if <min,>0 packets available)
  int timeout;                  // timeout value (used if no packets available)
} netio_t;

#define IOCTL_NETIO_OPEN_SOCK    _IOW(NETIO_CHAR_MAJOR,1,struct netio_s)
//#define IOCTL_NETIO_CLOSE_SOCK   _IO (NETIO_CHAR_MAJOR,2)

typedef struct netio_sock_stats_s {
  int rbytes;      // bytes that the socket is using for storing incoming data
                   // (might be larger than the size of incoming data itself).
  int rbytes_max;
  int wbytes;
  int wbytes_max;
} netio_sock_stats_t;

#define IOCTL_NETIO_GET_SOCK_STATS  _IOW(NETIO_CHAR_MAJOR,2,struct netio_sock_stats_s)

typedef struct netio_readv_stats_s {
  struct iovec iov_stack[2];
  int iov_cnt;
  // output params.
  int bytes_filled;
  unsigned short first_seq; // if rtp.
  unsigned short last_seq;  // if rtp.
  struct timeval last_rcv_time;
} netio_readv_stats;

#define IOCTL_NETIO_READV  _IOW(NETIO_CHAR_MAJOR,3,struct netio_readv_stats_s)

typedef struct netio_peek_params_s {
  char* buf;
  // input/output params.
  int len;
  // output params.
  int full_len;
} netio_peek_params;

#define IOCTL_NETIO_PEEK  _IOW(NETIO_CHAR_MAJOR,4,struct netio_peek_params_s)

// drop maximum of <N> packets.
// overwrites param with amount of dropped packets.
#define IOCTL_NETIO_DROP  _IOW(NETIO_CHAR_MAJOR,5,int)


#define IOCTL_NETIO_ENABLE_META_HEADER  _IOW(NETIO_CHAR_MAJOR,10,int)
#define IOCTL_NETIO_ENABLE_RTP_SEQ      _IOW(NETIO_CHAR_MAJOR,11,int)

#endif
