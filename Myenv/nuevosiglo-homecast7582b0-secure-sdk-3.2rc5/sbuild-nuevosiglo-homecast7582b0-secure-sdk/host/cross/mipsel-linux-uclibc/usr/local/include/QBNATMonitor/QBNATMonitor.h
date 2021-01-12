/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_NAT_MONITOR_H
#define QB_NAT_MONITOR_H

#include <QBNetManager.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvString.h>
#include <QBAppKit/QBObservable.h>

#include <stdbool.h>

typedef struct QBNATMonitor_ *QBNATMonitor;

typedef struct {
    SvString localIface;
    SvString localIP;
    SvString mappedIP;

    bool ipIsPublic;
    bool udpBlocked;
    bool portPreserved;
    bool portRestricted;
    bool ipRestricted;
} QBNATMonitorStatus;

QBNATMonitor QBNATMonitorCreate(QBNetManager netManager);

void QBNATMonitorStart(QBNATMonitor self, SvScheduler scheduler);
void QBNATMonitorStop(QBNATMonitor self);

bool QBNATMonitorGetStatus(QBNATMonitor self, QBNATMonitorStatus *status);


typedef void (QBNATMonitorCallbackFn)(void *target, QBNATMonitor monitor);

void QBNATMonitorRegisterCallback(QBNATMonitor self, QBNATMonitorCallbackFn *fn, void *target);
void QBNATMonitorUnregisterCallback(QBNATMonitor self, QBNATMonitorCallbackFn *fn, void *target);

/**
 * Add NAT monitor observer.
 *
 * @param[in] self      NAT monitor handle
 * @param[in] observer  handle to an object implementing @ref QBObserver
 * @param[out] errorOut error info
 **/
static inline void
QBNATMonitorAddObserver(QBNATMonitor self,
                        SvObject observer,
                        SvErrorInfo *errorOut)
{
    QBObservableAddObserver((QBObservable) self, observer, errorOut);
}

/**
 * Remove NAT monitor observer.
 *
 * @param[in] self      NAT monitor handle
 * @param[in] observer  handle to a previously registered observer
 * @param[out] errorOut error info
 **/
static inline void
QBNATMonitorRemoveObserver(QBNATMonitor self,
                           SvObject observer,
                           SvErrorInfo *errorOut)
{
    QBObservableRemoveObserver((QBObservable) self, observer, errorOut);
}

#endif // #ifndef QB_NAT_MONITOR_H
