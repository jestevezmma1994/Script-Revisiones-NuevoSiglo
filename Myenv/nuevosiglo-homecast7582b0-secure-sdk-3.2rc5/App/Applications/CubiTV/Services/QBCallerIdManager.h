/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCALLERIDMANAGER_H
#define QBCALLERIDMANAGER_H

#include <main_decl.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <Services/core/JSONserializer.h>

typedef struct QBCallerIdManager_t* QBCallerIdManager;

QBCallerIdManager QBCallerIdManagerCreate(AppGlobals appGlobals, SvString historyFilename, QBJSONSerializer JSONserializer, int historySize);
void QBCallerIdManagerStart(QBCallerIdManager self, SvErrorInfo *errorOut);
void QBCallerIdManagerStop(QBCallerIdManager self, SvErrorInfo *errorOut);
void QBCallerIdManagerAddListener(QBCallerIdManager self, SvGenericObject listener);
void QBCallerIdManagerRemoveListener(QBCallerIdManager self, SvGenericObject listener);

SvArray QBCallerIdManagerGetCallsHistory(QBCallerIdManager self);
int QBCallerIdManagerGetHistorySize(QBCallerIdManager self);

SvInterface QBCallerIdManagerListener_getInterface(void);

typedef struct QBCallerIdManagerListener_t {
    void (*callsDataChanged)(SvGenericObject self_);
} *QBCallerIdManagerListener;

#endif // QBCALLERIDMANAGER_H