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

#ifndef QB_INTEK_CONAX_SMARTCARD_USER_TEXT_H
#define QB_INTEK_CONAX_SMARTCARD_USER_TEXT_H

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum QBICSmartcardMarkUserTextType_e {
    QBICSmartcardMarkUserTextType_shown,
    QBICSmartcardMarkUserTextType_dropped,
} QBICSmartcardMarkUserTextType;

struct QBICSmartcardUserText_s {
    struct SvObject_ super_;

    int slot;

    time_t start_time;
    int duration; /// seconds
    int interval; /// seconds
    bool user_ack_allowed; /// user can dismiss this message by e.g. pressing "ok" button - this hides this message before "duration" passes

    int repetition;     /// how many times this message is to be displayed
    int display_counter; /// how many times this message was already displayed

    SvString text;
};
typedef struct QBICSmartcardUserText_s QBICSmartcardUserText;

extern SvType  QBICSmartcardUserText_getType(void);


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_INTEK_CONAX_SMARTCARD_USER_TEXT_H
