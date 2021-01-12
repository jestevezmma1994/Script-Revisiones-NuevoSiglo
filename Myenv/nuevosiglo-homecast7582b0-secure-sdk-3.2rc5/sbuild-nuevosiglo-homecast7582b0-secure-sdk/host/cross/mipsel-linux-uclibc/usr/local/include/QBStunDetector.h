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

#ifndef QB_STUN_MANAGER_H
#define QB_STUN_MANAGER_H

#include "QBStunSniffer.h"
#include <fibers/c/fibers.h>


struct QBStunDetector_s;
typedef struct QBStunDetector_s  QBStunDetector;


typedef struct QBStunDetectorCallbacks_s  QBStunDetectorCallbacks;
struct QBStunDetectorCallbacks_s
{
    void (*detection_done)  (void* target, QBStunDetector* man);
    void (*detection_error) (void* target, QBStunDetector* man, int svErr);
};

extern
QBStunDetector* QBStunDetectorCreate(const QBStunDetectorCallbacks* callbacks, void* target);
extern
void QBStunDetectorDestroy(QBStunDetector* stun);

extern
int QBStunDetectorStart(QBStunDetector* stun, SvScheduler sched);


typedef struct QBStunDetectorParams_s  QBStunDetectorParams;
struct QBStunDetectorParams_s
{
    const char* serverAddr; ///< stun server address
    int serverPort; ///< stun server port

    int retryCnt;  ///< how many packets to send before giving up
    int timeoutMs; ///< waiting for reply in each retry attempt (so total timeout is @a retryCnt * @a timeoutMs)
};

extern
void QBStunDetectorGetDefaultParams(QBStunDetectorParams* params);

extern
void QBStunDetectorSetParams(QBStunDetector* stun, const QBStunDetectorParams* params);

typedef struct QBStunDetectorResults_s  QBStunDetectorResults;
struct QBStunDetectorResults_s
{
    bool  udpBlocked;       /// no udp traffic got back, even in simplest cases
    bool  portPreserved;    /// mapped port == source port whenever possible
    bool  ipRestricted;     /// only packets from the same dest ip are let back through
    bool  portRestricted;   /// only packets from the same dest port are let back through

    bool  dependentMapping; /// different mapped port depending on destination ip/port

    const char* mappedIP; /// ip of our router
};

extern
bool QBStunDetectorIsDone(const QBStunDetector* detector);
extern
void QBStunDetectorGetResults(const QBStunDetector* detector, QBStunDetectorResults* results);

#endif // #ifndef QB_STUN_MANAGER_H
