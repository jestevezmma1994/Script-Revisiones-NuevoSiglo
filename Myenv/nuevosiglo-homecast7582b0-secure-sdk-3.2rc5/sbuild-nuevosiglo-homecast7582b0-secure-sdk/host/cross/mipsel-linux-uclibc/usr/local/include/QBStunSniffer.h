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

#ifndef QB_STUN_SNIFFER_H
#define QB_STUN_SNIFFER_H

#include <fibers/c/fibers.h>


struct QBStunSniffer_s;
typedef struct QBStunSniffer_s  QBStunSniffer;


typedef struct QBStunSnifferCallbacks_s  QBStunSnifferCallbacks;
struct QBStunSnifferCallbacks_s
{
    void (*finished) (void* target, QBStunSniffer* handler, const char* externalIp, int externalPort);
    void (*timeout)  (void* target, QBStunSniffer* handler);
    void (*error)    (void* target, QBStunSniffer* handler, int svErr);
};

extern
QBStunSniffer* QBStunSnifferCreate(int udpSocketFd, const QBStunSnifferCallbacks* callbacks, void* target);

extern
void QBStunSnifferDestroy(QBStunSniffer* stun);

extern
int QBStunSnifferStart(QBStunSniffer* stun, SvScheduler sched);


typedef struct QBStunSnifferParams_s  QBStunSnifferParams;
struct QBStunSnifferParams_s
{
    const char* serverAddr; ///< stun server address
    int serverPort; ///< stun server port

    bool changeResendIP;
    bool changeResendPort;

    int retryCnt;  ///< how many packets to send before giving up
    int timeoutMs; ///< waiting for reply in each retry attempt (so total timeout is @a retryCnt * @a timeoutMs)
};

extern
void QBStunSnifferGetDefaultParams(QBStunSnifferParams* params);

extern
void QBStunSnifferSetParams(QBStunSniffer* stun, const QBStunSnifferParams* params);

extern
const QBStunSnifferParams* QBStunSnifferGetParams(const QBStunSniffer* stun);

#endif // #ifndef QB_STUN_SNIFFER_H
