/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBSMARTCARDMONITOR_H_
#define QBSMARTCARDMONITOR_H_

#include <QBCAS.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvCoreTypes.h>

SvInterface
QBSmartCardMonitorListener_getInterface(void);

typedef struct QBSmartCardMonitorListener_t {
    void (*statusChanged)(SvObject self_);
} *QBSmartCardMonitorListener;

typedef struct QBSmartCardMonitor_t* QBSmartCardMonitor;

QBSmartCardMonitor
QBSmartCardMonitorCreate(void);

/**
 * Start smart card monitor
 *
 * @param[in] self          smartcard monitor handle
 */
void
QBSmartCardMonitorStart(QBSmartCardMonitor self);

/**
 * Stop smart card monitor
 *
 * @param[in] self          smartcard monitor handle
 */
void
QBSmartCardMonitorStop(QBSmartCardMonitor self);

void
QBSmartCardMonitorAddListener(QBSmartCardMonitor self, SvObject listener);
void
QBSmartCardMonitorRemoveListener(QBSmartCardMonitor self, SvObject listener);

/**
 * Return SmartCard ID number in compacted format, e.g. "12345678901"
 *
 * @param[in] self          smartcard monitor handle
 * @return                  smartcard ID
 **/
SvString
QBSmartCardMonitorGetSmartCardId(QBSmartCardMonitor self);

/**
 * Return SmartCard ID number in raw format, e.g. "123 4567 8901 - 2"
 *
 * @param[in] self          smartcard monitor handle
 * @return                  smartcard ID
 **/
SvString
QBSmartCardMonitorGetSmartCardIdRaw(QBSmartCardMonitor self);

SvString
QBSmartCardMonitorGetSmartcardIdWithCheckSum(QBSmartCardMonitor self);
SvString
QBSmartCardMonitorGetChipId(QBSmartCardMonitor self);

/**
 * Return full SmartCard chip ID number.
 *
 * @param[in] self     smartcard monitor handle
 * @return             smartcard chip full id
 **/
SvString
QBSmartCardMonitorGetFullChipId(QBSmartCardMonitor self);

QBCASSmartcardState
QBSmartCardMonitorGetSmartCardStatus(QBSmartCardMonitor self);

/**
 * Get system id
 *
 * @param[in] self          smartcard monitor handle
 * @return                  system id, -1 when system id is unknown
 */
int
QBSmartCardMonitorGetCaSystemId(QBSmartCardMonitor self);

#endif /* QBSMARTCARDMONITOR_H_ */
