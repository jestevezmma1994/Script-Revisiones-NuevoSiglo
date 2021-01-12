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

#ifndef QB_AUTHENTICATION_SERVICE_H_
#define QB_AUTHENTICATION_SERVICE_H_

#include <main_decl.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvInterface.h>
#include <stdbool.h>

typedef struct QBAuthenticationService_t *QBAuthenticationService;

QBAuthenticationService QBAuthenticationServiceCreate(void);
void QBAuthenticationServiceStart(QBAuthenticationService self);
void QBAuthenticationServiceStop(QBAuthenticationService self);

int QBAuthenticationServiceAddUser(QBAuthenticationService self, SvString userName, SvString pin);
int QBAuthenticationServiceSetCurrentUser(QBAuthenticationService self, SvString userName);

// Functions below operates on current user
void QBAuthenticationServiceSetupAccount(QBAuthenticationService self, int serviceId, SvString login, SvString password, bool autoLogin);
void QBAuthenticationServiceDeleteAccount(QBAuthenticationService self, int serviceId);
void QBAuthenticationServiceGetCredentials(QBAuthenticationService self, int serviceId, SvString *login, SvString *password, bool *autoLogin);

void QBAuthenticationServiceAddListener(QBAuthenticationService self, SvGenericObject listener);
void QBAuthenticationServiceRemoveListener(QBAuthenticationService self, SvGenericObject listener);

SvInterface QBAuthenticationServiceListener_getInterface(void);

typedef struct QBAuthenticationServiceListener_t {
    void (* currentUserChanged)(SvGenericObject self_, SvString userName);
} *QBAuthenticationServiceListener;

#endif // QB_AUTHENTICATION_SERVICE_H_

