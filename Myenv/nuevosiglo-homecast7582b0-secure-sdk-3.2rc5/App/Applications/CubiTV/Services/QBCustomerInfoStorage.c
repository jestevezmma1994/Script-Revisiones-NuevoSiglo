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

#include "QBCustomerInfoStorage.h"
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <Services/core/QBMiddlewareManager.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvJSON/SvJSONParse.h>
#include <safeFiles.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <main_decl.h>
#include <main.h>

struct QBCustomerInfoStorage_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    SvString customerId;
    SvHashTable customerInfo;

    SvString filename;

    QBJSONSerializer JSONserializer;
};

SvLocal void
QBCustomerInfoStorageClean(QBCustomerInfoStorage self)
{
    if (!self->filename)
        return;

    QBJSONSerializerRemoveJob(self->JSONserializer, self->filename);
    remove(SvStringCString(self->filename));

    SVTESTRELEASE(self->customerId);
    self->customerId = NULL;

    SVTESTRELEASE(self->customerInfo);
    self->customerInfo = NULL;
}

SvLocal int
QBCustomerInfoStorageSave(QBCustomerInfoStorage self)
{
    if (!self->filename)
        return -1;

    if (!self->customerId || !self->customerInfo) {
        QBCustomerInfoStorageClean(self);
        return -1;
    }

    SvHashTable dict = SvHashTableCreate(11, NULL);
    SvHashTableInsert(dict, (SvGenericObject) SVSTRING("customerID"), (SvGenericObject) self->customerId);
    SvHashTableInsert(dict, (SvGenericObject) SVSTRING("customerInfo"), (SvGenericObject) self->customerInfo);

    QBJSONSerializerAddJob(self->JSONserializer, (SvGenericObject) dict, self->filename);

    SVRELEASE(dict);

    return 0;
}

SvLocal int
QBCustomerInfoStorageLoad(QBCustomerInfoStorage self)
{
    if (!self->filename) {
        SvLogWarning("%s: no filename set", __func__);
        return -1;
    }

    char *buf = NULL;
    QBFileToBuffer(SvStringCString(self->filename), &buf);
    if (!buf) {
        SvLogWarning("%s: failed to read customer info from file", __func__);
        return -1;
    }

    SvHashTable dict = (SvHashTable) SvJSONParseString(buf, false, NULL);
    if (!dict || !SvObjectIsInstanceOf((SvObject) dict, SvHashTable_getType()))
        goto fini;

    SvValue customerIdV = (SvValue) SvHashTableFind(dict, (SvGenericObject) SVSTRING("customerID"));
    if (!customerIdV || !SvObjectIsInstanceOf((SvObject) customerIdV, SvValue_getType()) || !SvValueIsString(customerIdV))
        goto fini;

    SvHashTable customerInfo = (SvHashTable) SvHashTableFind(dict, (SvGenericObject) SVSTRING("customerInfo"));
    if (!customerInfo || !SvObjectIsInstanceOf((SvObject) customerInfo, SvHashTable_getType()))
        goto fini;

    self->customerInfo = SVRETAIN(customerInfo);
    self->customerId = SVRETAIN(SvValueGetString(customerIdV));

    return 0;

fini:
    SVTESTRELEASE(dict);
    free(buf);

    return -1;
}

SvLocal void
QBCustomerInfoStorageMWDataChanged(SvGenericObject self_, QBMiddlewareManagerType middlewareType)
{
    QBCustomerInfoStorage self = (QBCustomerInfoStorage) self_;

    if (middlewareType != QBMiddlewareManagerType_Innov8on)
        return;

    SvString tmpCustomerId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    if (!SvObjectEquals((SvObject) self->customerId, (SvObject) tmpCustomerId)) {
        QBCustomerInfoStorageClean(self);
        if (self->appGlobals->customerInfoMonitor)
            CubiwareMWCustomerInfoSetCustomerId(self->appGlobals->customerInfoMonitor, tmpCustomerId);
    }
}

SvLocal void
QBCustomerInfoStorageCustomerInfoChanged(SvGenericObject self_, SvString customerId, SvHashTable customerInfo)
{
    QBCustomerInfoStorage self = (QBCustomerInfoStorage) self_;

    self->customerId = SVTESTRETAIN(customerId);
    self->customerInfo = SVTESTRETAIN(customerInfo);

    QBCustomerInfoStorageSave(self);
}

SvLocal void
QBCustomerInfoStorageStatusChanged(SvGenericObject self_)
{
}

void
QBCustomerInfoStorageStart(QBCustomerInfoStorage self)
{
    if (!self) {
        SvLogError("%s: NULL self passed!", __func__);
        return;
    }

    QBJSONSerializer JSONserializer = (QBJSONSerializer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBJSONSerializer"));
    if (!JSONserializer) {
        SvLogError("%s: NULL JSONserializer!", __func__);
        return;
    }
    self->JSONserializer = SVRETAIN(JSONserializer);

    int result = QBCustomerInfoStorageLoad(self);
    if (!self->appGlobals->customerInfoMonitor)
        return;

    if (result == 0) {
        CubiwareMWCustomerInfoSetCustomerId(self->appGlobals->customerInfoMonitor, self->customerId);
        CubiwareMWCustomerInfoSetCustomerInfo(self->appGlobals->customerInfoMonitor, self->customerInfo);
    } else {
        SvString tmpCustomerId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        CubiwareMWCustomerInfoSetCustomerId(self->appGlobals->customerInfoMonitor, tmpCustomerId);
    }
}

void
QBCustomerInfoStorageStop(QBCustomerInfoStorage self)
{
    if (!self) {
        SvLogError("%s: NULL self passed!", __func__);
        return;
    }

    QBCustomerInfoStorageSave(self);
}

SvLocal void
QBCustomerInfoStorageDestroy(void *self_)
{
    QBCustomerInfoStorage self = (QBCustomerInfoStorage) self_;

    SVTESTRELEASE(self->customerId);
    SVTESTRELEASE(self->customerInfo);
    SVTESTRELEASE(self->filename);
    SVTESTRELEASE(self->JSONserializer);
}

SvLocal SvType
QBCustomerInfoStorage_getType(void)
{
   static const struct SvObjectVTable_ objectVTable = {
      .destroy = QBCustomerInfoStorageDestroy
   };
   static SvType type = NULL;

   static const struct QBMiddlewareManagerListener_t mwManagerMethods = {
       .middlewareDataChanged = QBCustomerInfoStorageMWDataChanged
   };

   static const struct CubiwareMWCustomerInfoListener_ customerInfoMethods = {
       .customerInfoChanged = QBCustomerInfoStorageCustomerInfoChanged,
       .statusChanged = QBCustomerInfoStorageStatusChanged
   };

   if (unlikely(!type)) {
      SvTypeCreateManaged("QBCustomerInfoStorage",
                          sizeof(struct QBCustomerInfoStorage_t),
                          SvObject_getType(),
                          &type,
                          SvObject_getType(), &objectVTable,
                          QBMiddlewareManagerListener_getInterface(), &mwManagerMethods,
                          CubiwareMWCustomerInfoListener_getInterface(), &customerInfoMethods,
                          NULL);
   }

   return type;
}

QBCustomerInfoStorage
QBCustomerInfoStorageCreate(AppGlobals appGlobals, SvString filename)
{
    if (!appGlobals) {
        SvLogError("%s: NULL appGlobals passed!", __func__);
        return NULL;
    }

    QBCustomerInfoStorage self = (QBCustomerInfoStorage) SvTypeAllocateInstance(QBCustomerInfoStorage_getType(), NULL);

    self->appGlobals = appGlobals;
    self->filename = SVTESTRETAIN(filename);

    return self;
}

