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

#ifndef MSTORE_LOGIC_H_
#define MSTORE_LOGIC_H_

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <main_decl.h>

typedef struct QBMStoreLogic_ *QBMStoreLogic;

typedef void (*QBMStoreLogicOrderHistoryCallback)(void *self_, SvArray orders, SvString msg);


QBMStoreLogic QBMStoreLogicCreate(AppGlobals appGlobals, SvErrorInfo *errorOut) __attribute__((weak));

void QBMStoreLogicStart(QBMStoreLogic self) __attribute__((weak));
void QBMStoreLogicStop(QBMStoreLogic self) __attribute__((weak));

bool QBMStoreLogicHasSearch(QBMStoreLogic self) __attribute__((weak));
bool QBMStoreLogicHasLatestArrivals(QBMStoreLogic self) __attribute__((weak));
bool QBMStoreLogicHasOrderHistory(QBMStoreLogic self) __attribute__((weak));
bool QBMStoreLogicHasMyRentals(QBMStoreLogic self) __attribute__((weak));

/**
 * Decides if "Order by SMS" option should be visible
 *
 * @param[in] self      QBMStoreLogic handle
 * @return              @c true if menu should be visible and @c false otherwise
 **/
bool QBMStoreLogicHasOrderBySMS(QBMStoreLogic self) __attribute__((weak));

void QBMStoreLogicScheduleOrderHistory(QBMStoreLogic self, QBMStoreLogicOrderHistoryCallback callback, void *arg) __attribute__((weak));

#endif // MSTORE_LOGIC_H_
