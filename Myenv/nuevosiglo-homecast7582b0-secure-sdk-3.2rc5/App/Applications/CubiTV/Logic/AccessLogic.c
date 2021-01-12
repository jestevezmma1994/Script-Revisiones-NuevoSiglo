/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#include <libintl.h>
#include <SvFoundation/SvType.h>
#include <main.h>
#include <init.h>
#include <Utils/appType.h>
#include <Logic/AccessLogic.h>


struct QBAccessLogic_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
};

SvLocal SvType QBAccessLogic_getType(void)
{
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("QBAccessLogic", sizeof(struct QBAccessLogic_t),
                            SvObject_getType(), &type,
                            NULL);
    }
    return type;
}

QBAccessLogic QBAccessLogicCreate(AppGlobals appGlobals)
{
    QBAccessLogic self = (QBAccessLogic) SvTypeAllocateInstance(QBAccessLogic_getType(), NULL);

    self->appGlobals = appGlobals;

    return self;
}

void QBAccessLogicStart(QBAccessLogic self)
{
    // do nothing
}

void QBAccessLogicStop(QBAccessLogic self)
{
    // do nothing
}

bool QBAccessLogicPinAttemptsLimited(QBAccessLogic self)
{
    SvLogError("%s GENERIC LIMITED false", __func__);
    return false;
}

int QBAccessLogicGetPinAttemptsLeft(QBAccessLogic self)
{
    return INT_MAX;
}

int QBAccessLogicGetBlockPeriodLeft(QBAccessLogic self)
{
    return 0;
}
