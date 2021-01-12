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

#ifndef QB_INTEK_CONAX_SMARTCARD_EVENT_H
#define QB_INTEK_CONAX_SMARTCARD_EVENT_H

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

enum QBICSmartcardEventAccessType_e {
    QBICSmartcardEventAccessType_minutes   = 0x0,
    QBICSmartcardEventAccessType_unlimited = 0x1,
    QBICSmartcardEventAccessType_credits   = 0x2,
};
typedef enum QBICSmartcardEventAccessType_e QBICSmartcardEventAccessType;

struct QBICSmartcardEvent_s {
    struct SvObject_ super_;

    int tag;
    SvString label;      // can be null
    SvString entitlement; // can be null

    /// should use time_t, if != 0, strings are here only for compatibility, will be removed in the future
    time_t auth_start;
    time_t auth_end;
    SvString auth_start_str;
    SvString auth_end_str;

    QBICSmartcardEventAccessType access_type;
    int time_left;
};
typedef struct QBICSmartcardEvent_s QBICSmartcardEvent;

extern SvType QBICSmartcardEvent_getType(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_INTEK_CONAX_SMARTCARD_EVENT_H
