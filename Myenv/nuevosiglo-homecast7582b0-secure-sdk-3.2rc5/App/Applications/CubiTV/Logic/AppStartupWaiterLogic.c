/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include <Logic/AppStartupWaiterLogic.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>

struct QBAppStartupWaiterLogic_ {
    struct SvObject_ super_;
};


int
QBAppStartupWaiterLogicInputHandler(QBAppStartupWaiterLogic self,
                                    SvInputEvent ev)
{
    return 0;
}

SvLocal void
QBAppStartupWaiterLogicDestroy(void *self_)
{
//    QBAppStartupWaiterLogic self = self_;
}

SvLocal SvType
QBAppStartupWaiterLogic_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAppStartupWaiterLogicDestroy
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBAppStartupWaiterLogic",
                            sizeof(struct QBAppStartupWaiterLogic_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBAppStartupWaiterLogic
QBAppStartupWaiterLogicCreate(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBAppStartupWaiterLogic self = (QBAppStartupWaiterLogic)
        SvTypeAllocateInstance(QBAppStartupWaiterLogic_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBAppStartupWaiterLogic");
        goto fini;
    }


fini:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

