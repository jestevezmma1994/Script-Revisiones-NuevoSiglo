/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013 - 2016 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_VIEW_RIGHT_INFO_H
#define QB_VIEW_RIGHT_INFO_H

#include <QBCASInfo.h>
#include <stdint.h>
#include <SvFoundation/SvArray.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum QBViewRightMode_e {
    QBViewRightMode_error,
    QBViewRightMode_smartcard,
    QBViewRightMode_nonsmartcard,
    QBViewRightMode_notPresent
} QBViewRightMode;

typedef enum QBViewRightPairingState_e {
    QBViewRightPairingState_NoPairing,
    QBViewRightPairingState_PairingInProgress,
    QBViewRightPairingState_PairingOk,
    QBViewRightPairingState_PairingBad
} QBViewRightPairingState;

typedef enum QBViewRightPhysicalCardStatus_e {
    QBViewRightPhysicalCardStatus_unknown,
    QBViewRightPhysicalCardStatus_inserted,
    QBViewRightPhysicalCardStatus_removed,
} QBViewRightPhysicalCardStatus;

struct QBViewRightInfo_s {
    struct QBCASInfo_t base;
    SvString lib_version;
    SvString libDate;
    QBViewRightMode mode;
    SvArray wallets;
    SvArray walletNames;
    QBViewRightPairingState pairingState;
    QBViewRightPhysicalCardStatus physicalCardStatus;
    int maxRecordSessions;
};
typedef struct QBViewRightInfo_s* QBViewRightInfo;

QBViewRightInfo QBViewRightInfoCreate(SvErrorInfo *errorOut);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_INTEK_CONAX_SMARTCARD_INFO_H
