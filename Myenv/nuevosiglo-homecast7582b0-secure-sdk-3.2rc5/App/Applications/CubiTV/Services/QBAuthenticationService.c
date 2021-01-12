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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <main.h>

#include <SvJSON/SvJSONClassRegistry.h>
#include <SvJSON/SvJSONParse.h>
#include <Services/core/JSONserializer.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <safeFiles.h>

#include "QBAuthenticationService.h"

struct QBAuthenticationAccountDetails_t {
    struct SvObject_ super_;

    int      serviceId;
    SvString login;
    SvString password;
    bool     autoLogin;
};
typedef struct QBAuthenticationAccountDetails_t *QBAuthenticationAccountDetails;

struct QBAuthenticationUserDetails_t {
    struct SvObject_ super_;

    SvString name;
    SvString pin;
    SvArray  accounts;
};
typedef struct QBAuthenticationUserDetails_t *QBAuthenticationUserDetails;

struct QBAuthenticationService_t
{
    struct SvObject_ super_;

    SvString accountsFile;

    QBAuthenticationUserDetails currentUser;
    SvArray users;
    SvHashTable data;

    SvWeakList listeners;

    QBJSONSerializer JSONserializer;
};

// --------------- Account ---------------

SvLocal void
QBAuthenticationAccountDetails__dtor__(void *ptr)
{
    QBAuthenticationAccountDetails self = ptr;

    SVRELEASE(self->login);
    SVRELEASE(self->password);
}

SvLocal SvType
QBAuthenticationAccountDetails_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAuthenticationAccountDetails__dtor__
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBAuthenticationAccountDetails",
                            sizeof(struct QBAuthenticationAccountDetails_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal QBAuthenticationAccountDetails
QBAuthenticationAccountDetailsCreate(int serviceId, SvString login, SvString password, bool autoLogin)
{
    QBAuthenticationAccountDetails self = (QBAuthenticationAccountDetails) SvTypeAllocateInstance(QBAuthenticationAccountDetails_getType(), NULL);

    self->serviceId = serviceId;
    self->autoLogin = autoLogin;
    self->login     = SVRETAIN(login);
    self->password  = SVRETAIN(password);

    return self;
}

// --------------- User ----------------------

SvLocal void
QBAuthenticationUserDetails__dtor__(void *ptr)
{
    QBAuthenticationUserDetails self = ptr;

    SVRELEASE(self->name);
    SVRELEASE(self->pin);
    SVRELEASE(self->accounts);
}

SvLocal SvType
QBAuthenticationUserDetails_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAuthenticationUserDetails__dtor__
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBAuthenticationUserDetails",
                            sizeof(struct QBAuthenticationUserDetails_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal QBAuthenticationUserDetails
QBAuthenticationUserDetailsCreate(SvString name, SvString pin, SvArray accounts)
{
    QBAuthenticationUserDetails self = (QBAuthenticationUserDetails) SvTypeAllocateInstance(QBAuthenticationUserDetails_getType(), NULL);

    self->name     = SVRETAIN(name);
    self->pin      = SVRETAIN(pin);
    self->accounts = SVRETAIN(accounts);

    return self;
}

// ------------------- Service --------------------

SvLocal void
QBAuthenticationServiceNotifyListeners(QBAuthenticationService self)
{
    SvIterator it = SvWeakListIterator(self->listeners);
    SvGenericObject listener = NULL;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBAuthenticationServiceListener, listener, currentUserChanged, self->currentUser->name);
    }
}

SvLocal QBAuthenticationUserDetails
QBAuthenticationServiceFindUser(QBAuthenticationService self, SvString userName)
{
    SvIterator it = SvArrayIterator(self->users);
    QBAuthenticationUserDetails user = NULL;
    while ((user = (QBAuthenticationUserDetails) SvIteratorGetNext(&it))) {
        if (SvObjectEquals((SvGenericObject) user->name, (SvGenericObject) userName))
            return user;
    }
    return NULL;
}

int
QBAuthenticationServiceAddUser(QBAuthenticationService self, SvString userName, SvString pin)
{
    if (QBAuthenticationServiceFindUser(self, userName)) {
        SvLogError("%s : Can't create new user. User %s exists", __func__, SvStringCString(userName));
        return -1;
    }
    SvArray tmp = SvArrayCreateWithCapacity(5, NULL);
    QBAuthenticationUserDetails user = QBAuthenticationUserDetailsCreate(userName, pin, tmp);
    SvArrayAddObject(self->users, (SvGenericObject) user);
    SVRELEASE(tmp);
    SVRELEASE(user);
    QBJSONSerializerAddJob(self->JSONserializer, (SvGenericObject) self->data, self->accountsFile);
    return 0;
}

int
QBAuthenticationServiceSetCurrentUser(QBAuthenticationService self, SvString userName)
{
    QBAuthenticationUserDetails user = QBAuthenticationServiceFindUser(self, userName);
    if (!user) {
        SvLogError("%s : Can't set current user. User %s doesn't exist", __func__, SvStringCString(userName));
        return -1;
    }
    SVRELEASE(self->currentUser);
    self->currentUser = SVRETAIN(user);
    SvHashTableInsert(self->data, (SvGenericObject) SVSTRING("currentUser"), (SvGenericObject) self->currentUser->name);
    QBJSONSerializerAddJob(self->JSONserializer, (SvGenericObject) self->data, self->accountsFile);
    QBAuthenticationServiceNotifyListeners(self);
    return 0;
}

SvLocal QBAuthenticationAccountDetails
QBAuthenticationServiceFindAccount(QBAuthenticationService self, int serviceId)
{
    SvIterator it = SvArrayIterator(self->currentUser->accounts);
    QBAuthenticationAccountDetails account = NULL;
    while ((account = (QBAuthenticationAccountDetails) SvIteratorGetNext(&it))) {
        if (account->serviceId == serviceId)
            return account;
    }
    return NULL;
}

void
QBAuthenticationServiceSetupAccount(QBAuthenticationService self, int serviceId, SvString login, SvString password, bool autoLogin)
{
    QBAuthenticationAccountDetails account = QBAuthenticationServiceFindAccount(self, serviceId);
    if (account) {
        SVRELEASE(account->login);
        SVRELEASE(account->password);
        account->login = SVRETAIN(login);
        account->password = SVRETAIN(password);
        account->autoLogin = autoLogin;
    } else {
        account = QBAuthenticationAccountDetailsCreate(serviceId, login, password, autoLogin);
        SvArrayAddObject(self->currentUser->accounts, (SvGenericObject) account);
        SVRELEASE(account);
    }
    QBJSONSerializerAddJob(self->JSONserializer, (SvGenericObject) self->data, self->accountsFile);
    QBAuthenticationServiceNotifyListeners(self);
}

void QBAuthenticationServiceDeleteAccount(QBAuthenticationService self, int serviceId)
{
    SvGenericObject account = (SvGenericObject) QBAuthenticationServiceFindAccount(self, serviceId);
    SvArrayRemoveObject(self->currentUser->accounts, account);
    QBJSONSerializerAddJob(self->JSONserializer, (SvGenericObject) self->data, self->accountsFile);
    QBAuthenticationServiceNotifyListeners(self);
}

void
QBAuthenticationServiceGetCredentials(QBAuthenticationService self, int serviceId, SvString *login, SvString *password, bool *autoLogin)
{
    QBAuthenticationAccountDetails account = QBAuthenticationServiceFindAccount(self, serviceId);
    if (!account) {
        SvLogWarning("%s : User %s : no account for service %d", __func__, SvStringCString(self->currentUser->name), serviceId);
    }
    if (autoLogin)
        *autoLogin = account ? account->autoLogin : false;
    if (login)
        *login = account ? account->login : NULL;
    if (password)
        *password = account ? account->password : NULL;
}

SvLocal void
QBAuthenticationServiceWriteAccountDetails(SvGenericObject helper_, SvGenericObject userParams, SvHashTable desc, SvErrorInfo *errorOut)
{
    QBAuthenticationAccountDetails self = (QBAuthenticationAccountDetails) userParams;
    SvValue tmp = NULL;

    tmp = SvValueCreateWithInteger(self->serviceId, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("serviceId"), (SvGenericObject) tmp);
    SVRELEASE(tmp);

    tmp = SvValueCreateWithString(self->login, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("login"), (SvGenericObject) tmp);
    SVRELEASE(tmp);

    tmp = SvValueCreateWithString(self->password, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("password"), (SvGenericObject) tmp);
    SVRELEASE(tmp);

    tmp = SvValueCreateWithBoolean(self->autoLogin, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("autoLogin"), (SvGenericObject) tmp);
    SVRELEASE(tmp);
}

SvLocal SvGenericObject
QBAuthenticationServiceReadAccountDetails(SvGenericObject helper_, SvString className, SvHashTable desc, SvErrorInfo *errorOut)
{
    SvValue serviceId = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("serviceId"));
    SvValue login     = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("login"));
    SvValue password  = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("password"));
    SvValue autoLogin = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("autoLogin"));

    if (!serviceId || !login || !password || !autoLogin)
        return NULL;

    if (!SvObjectIsInstanceOf((SvObject) serviceId, SvValue_getType()) || !SvObjectIsInstanceOf((SvObject) login, SvValue_getType()) || !SvObjectIsInstanceOf((SvObject) password, SvValue_getType()) || !SvObjectIsInstanceOf((SvObject) autoLogin, SvValue_getType())) {
        return NULL;
    }

    if (!SvValueIsInteger(serviceId) || !SvValueIsString(login) ||
        !SvValueIsString(password) || !SvValueIsBoolean(autoLogin)) {
        return NULL;
    }

    return (SvGenericObject) QBAuthenticationAccountDetailsCreate(SvValueGetInteger(serviceId), SvValueGetString(login),
                                                                  SvValueGetString(password), SvValueGetBoolean(autoLogin));
}

SvLocal void
QBAuthenticationServiceWriteUserDetails(SvGenericObject helper_, SvGenericObject userParams, SvHashTable desc, SvErrorInfo *errorOut)
{
    QBAuthenticationUserDetails self = (QBAuthenticationUserDetails) userParams;
    SvValue tmp = NULL;

    tmp = SvValueCreateWithString(self->name, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("name"), (SvGenericObject) tmp);
    SVRELEASE(tmp);

    tmp = SvValueCreateWithString(self->pin, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("pin"), (SvGenericObject) tmp);
    SVRELEASE(tmp);

    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("accounts"), (SvGenericObject) self->accounts);
}

SvLocal SvGenericObject
QBAuthenticationServiceReadUserDetails(SvGenericObject helper_, SvString className, SvHashTable desc, SvErrorInfo *errorOut)
{
    SvValue name     = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("name"));
    SvValue pin      = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("pin"));
    SvArray accounts = (SvArray) SvHashTableFind(desc, (SvGenericObject) SVSTRING("accounts"));

    if (!name || !pin || !accounts)
        return NULL;

    if (!SvObjectIsInstanceOf((SvObject) name, SvValue_getType()) || !SvObjectIsInstanceOf((SvObject) pin, SvValue_getType()) || !SvObjectIsInstanceOf((SvObject) accounts, SvArray_getType())) {
        return NULL;
    }

    if (!SvValueIsString(name) || !SvValueIsString(pin)) {
        return NULL;
    }

    return (SvGenericObject) QBAuthenticationUserDetailsCreate(SvValueGetString(name), SvValueGetString(pin), accounts);
}

SvLocal void
QBAuthenticationServiceCreateDefaults(QBAuthenticationService self)
{
    SvArray userAccounts = SvArrayCreateWithCapacity(5, NULL);
    self->currentUser = QBAuthenticationUserDetailsCreate(SVSTRING("default"), SVSTRING("0000"), userAccounts);
    self->users = SvArrayCreateWithCapacity(5, NULL);
    SvArrayInsertObjectAtIndex(self->users, (SvGenericObject) self->currentUser, 0);
    self->data = SvHashTableCreate(2, NULL);
    SvHashTableInsert(self->data, (SvGenericObject) SVSTRING("users"), (SvGenericObject) self->users);
    SvHashTableInsert(self->data, (SvGenericObject) SVSTRING("currentUser"), (SvGenericObject) self->currentUser->name);
    SVRELEASE(userAccounts);
    SVRELEASE(self->currentUser);
    SVRELEASE(self->users);
}

SvLocal void QBAuthenticationService__dtor__(void *self_)
{
    QBAuthenticationService self = self_;

    SVRELEASE(self->listeners);
    SVRELEASE(self->data);
    SVTESTRELEASE(self->JSONserializer);
}

SvLocal SvType
QBAuthenticationService_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAuthenticationService__dtor__
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBAuthenticationService",
                            sizeof(struct QBAuthenticationService_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBAuthenticationService
QBAuthenticationServiceCreate(void)
{
    QBAuthenticationService self = (QBAuthenticationService) SvTypeAllocateInstance(QBAuthenticationService_getType(), NULL);
    self->listeners = SvWeakListCreate(NULL);
    self->accountsFile = SVSTRING("/etc/vod/accounts");

    SvJSONClass helper = NULL;

    helper = SvJSONClassCreate(QBAuthenticationServiceWriteAccountDetails, QBAuthenticationServiceReadAccountDetails, NULL);
    SvJSONRegisterClassHelper(SVSTRING("QBAuthenticationAccountDetails"), (SvGenericObject) helper, NULL);
    SVRELEASE(helper);

    helper = SvJSONClassCreate(QBAuthenticationServiceWriteUserDetails, QBAuthenticationServiceReadUserDetails, NULL);
    SvJSONRegisterClassHelper(SVSTRING("QBAuthenticationUserDetails"), (SvGenericObject) helper, NULL);
    SVRELEASE(helper);

    return self;
}

void
QBAuthenticationServiceStart(QBAuthenticationService self)
{
    char *buffer = NULL;

    QBFileToBuffer(SvStringCString(self->accountsFile), &buffer);

    if (!buffer) {
        SvLogError("%s(): buffer is NULL", __func__);
        goto err;
    }

    self->data = (SvHashTable) SvJSONParseString(buffer, true, NULL);
    free(buffer);
    if (!self->data || !SvObjectIsInstanceOf((SvGenericObject) self->data, SvHashTable_getType())) {
        SvLogError("%s(): 'data' is invalid", __func__);
        goto err;
    }

    self->users = (SvArray) SvHashTableFind(self->data, (SvGenericObject) SVSTRING("users"));
    if (!self->users || !SvObjectIsInstanceOf((SvGenericObject) self->users, SvArray_getType())) {
        SvLogError("%s(): 'users' is invalid", __func__);
        goto err;
    }

    SvValue currentUser = (SvValue) SvHashTableFind(self->data, (SvGenericObject) SVSTRING("currentUser"));
    if (!currentUser || !SvObjectIsInstanceOf((SvGenericObject) currentUser, SvValue_getType())) {
        SvLogError("%s(): 'currentUser' is invalid", __func__);
        goto err;
    }

    self->currentUser = QBAuthenticationServiceFindUser(self, SvValueGetString(currentUser));
    if (!self->currentUser) {
        SvLogError("%s(): 'currentUser' not found", __func__);
        goto err;
    }
    QBJSONSerializer JSONserializer = (QBJSONSerializer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBJSONSerializer"));
    if (!JSONserializer) {
        SvLogError("%s(): 'JSONserializer' not found", __func__);
        goto err;
    }
    self->JSONserializer = SVRETAIN(JSONserializer);

    return;

err:
    SVTESTRELEASE(self->data);
    self->data = NULL;
    self->users = NULL;
    self->currentUser = NULL;
    QBAuthenticationServiceCreateDefaults(self);
}

void
QBAuthenticationServiceStop(QBAuthenticationService self)
{
    QBJSONSerializerAddJob(self->JSONserializer, (SvGenericObject) self->data, self->accountsFile);
}

void
QBAuthenticationServiceAddListener(QBAuthenticationService self, SvGenericObject listener)
{
    if (SvObjectIsImplementationOf(listener, QBAuthenticationServiceListener_getInterface())) {
        SvWeakListPushBack(self->listeners, listener, NULL);
    } else {
        SvLogWarning("%s(): listener %p of type %s doesn't implement QBAuthenticationServiceListener interface.",
                     __func__, listener, listener ? SvObjectGetTypeName(listener) : "--");
    }
}

void
QBAuthenticationServiceRemoveListener(QBAuthenticationService self, SvGenericObject listener)
{
    SvWeakListRemoveObject(self->listeners, listener);
}

SvInterface
QBAuthenticationServiceListener_getInterface(void)
{
    static SvInterface interface = NULL;
    if(!interface) {
        SvInterfaceCreateManaged("QBAuthenticationServiceListener",
                                 sizeof(struct QBAuthenticationServiceListener_t),
                                 NULL, &interface, NULL);
    }
    return interface;
}

