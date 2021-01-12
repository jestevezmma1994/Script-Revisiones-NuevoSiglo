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

#include <Logic/MStoreLogic.h>

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvCoreErrorDomain.h>

struct QBMStoreLogic_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
};


SvLocal void
QBMStoreLogicDestroy(void *self_)
{
    //QBMStoreLogic self = self_;
}

SvLocal SvType
QBMStoreLogic_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMStoreLogicDestroy
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBMStoreLogic",
                            sizeof(struct QBMStoreLogic_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBMStoreLogic
QBMStoreLogicCreate(AppGlobals appGlobals, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBMStoreLogic self = (QBMStoreLogic)
        SvTypeAllocateInstance(QBMStoreLogic_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBMStoreLogic");
        goto out;
    }

    self->appGlobals = appGlobals;

out:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

void
QBMStoreLogicStart(QBMStoreLogic self)
{
}

void
QBMStoreLogicStop(QBMStoreLogic self)
{
}

bool
QBMStoreLogicHasSearch(QBMStoreLogic self)
{
    return true;
}

bool
QBMStoreLogicHasLatestArrivals(QBMStoreLogic self)
{
    return true;
}

bool
QBMStoreLogicHasOrderHistory(QBMStoreLogic self)
{
    return false;
}

bool
QBMStoreLogicHasMyRentals(QBMStoreLogic self)
{
    return false;
}

bool
QBMStoreLogicHasOrderBySMS(QBMStoreLogic self)
{
    return true;
}

void
QBMStoreLogicScheduleOrderHistory(QBMStoreLogic self,
                                  QBMStoreLogicOrderHistoryCallback callback,
                                  void *arg)
{
    callback(arg, NULL, NULL);
}

