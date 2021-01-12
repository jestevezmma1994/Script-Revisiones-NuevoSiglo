/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_INTEK_CONAX_SMARTCARD_PURSE_H
#define QB_INTEK_CONAX_SMARTCARD_PURSE_H

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBICSmartcardPurseDebit_s {
    struct SvObject_ super_;

    int tag;
    SvString label; // may be null
    int consumed_tokens;

    /// should use time_t, if != 0, strings are here only for compatibility, will be removed in the future
    time_t last_access_time;
    SvString last_access_time_str;
};
typedef struct QBICSmartcardPurseDebit_s QBICSmartcardPurseDebit;


struct QBICSmartcardPurseCredit_s {
    struct SvObject_ super_;

    SvString label; // may be null
    int tokens;
};
typedef struct QBICSmartcardPurseCredit_s QBICSmartcardPurseCredit;


struct QBICSmartcardPurse_s {
    struct SvObject_ super_;

    uint16_t purse_ref;
    SvString label;

    int balance;
};
typedef struct QBICSmartcardPurse_s QBICSmartcardPurse;


struct QBICSmartcardPurseStatus_s {
    struct SvObject_ super_;

    /// optional
    SvArray debits;
    SvArray credits;
};
typedef struct QBICSmartcardPurseStatus_s QBICSmartcardPurseStatus;


extern SvType  QBICSmartcardPurse_getType(void);
extern SvType  QBICSmartcardPurseDebit_getType(void);
extern SvType  QBICSmartcardPurseCredit_getType(void);
extern SvType  QBICSmartcardPurseStatus_getType(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_INTEK_CONAX_SMARTCARD_PURSE_H
