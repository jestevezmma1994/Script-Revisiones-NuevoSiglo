/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include <QBWindows/QBExternalAppLogic.h>
#include <QBWindows/QBExternalAppContext.h>
#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/event.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvLog.h>

struct QBExternalAppLogic_ {
    struct SvObject_ super_;
    QBWindowContext extAppCtx;
};

SvLocal void
QBExternalAppLogicDestroy(void *self_)
{
}

SvLocal SvType
QBExternalAppLogic_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBExternalAppLogicDestroy
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBExternalAppLogic",
                            sizeof(struct QBExternalAppLogic_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBExternalAppLogic
QBExternalAppLogicCreate(QBWindowContext ctx)
{
    if (!ctx) {
        SvLogError("External app window context is NULL!");
        return NULL;
    }
    QBExternalAppLogic self = (QBExternalAppLogic) SvTypeAllocateInstance(QBExternalAppLogic_getType(), NULL);
    self->extAppCtx = ctx;

    return self;
}

bool
QBExternalAppLogicInputEventHandler(QBExternalAppLogic self, QBInputApp app, SvInputEvent ev)
{
    if (ev->ch == QBKEY_MENU) {
        QBExternalAppContextClose(self->extAppCtx);
        return true;
    }
    if (!app) {
        return 0;
    }
    return QBInputAppHandleInputEvent(app, ev->fullInfo);
}
