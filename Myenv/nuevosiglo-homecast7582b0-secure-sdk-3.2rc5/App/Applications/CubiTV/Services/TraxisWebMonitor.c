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

#include "TraxisWebMonitor.h"
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvString.h>
#include <TraxisWebClient/TraxisWebSessionManager.h>

struct TraxisWebMonitor_ {
    struct SvObject_ super_;
    QBMiddlewareManager middlewareManager;
    TraxisWebSessionManager sessionManager;
    SvString CPEID;
};


SvLocal void
TraxisWebMonitorMiddlewareDataChanged(SvGenericObject self_,
                                      QBMiddlewareManagerType middlewareType)
{
    TraxisWebMonitor self = (TraxisWebMonitor) self_;

    if (middlewareType != QBMiddlewareManagerType_Traxis)
        return;

    SvString CPEID = QBMiddlewareManagerGetId(self->middlewareManager, QBMiddlewareManagerType_Traxis);
    if (SvObjectEquals((SvObject) CPEID, (SvObject) self->CPEID))
        return;

    SVTESTRETAIN(CPEID);
    SVTESTRELEASE(self->CPEID);
    self->CPEID = CPEID;

    TraxisWebSessionManagerSetIdentification(self->sessionManager, self->CPEID, NULL, NULL, NULL);
}

SvLocal void
TraxisWebMonitor__dtor__(void *self_)
{
    TraxisWebMonitor self = self_;

    SVTESTRELEASE(self->middlewareManager);
    SVTESTRELEASE(self->sessionManager);
    SVTESTRELEASE(self->CPEID);
}

SvLocal SvType
TraxisWebMonitor_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = TraxisWebMonitor__dtor__
    };
    static const struct QBMiddlewareManagerListener_t methods = {
        .middlewareDataChanged = TraxisWebMonitorMiddlewareDataChanged
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("TraxisWebMonitor",
                            sizeof(struct TraxisWebMonitor_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMiddlewareManagerListener_getInterface(), &methods,
                            NULL);
    }

    return type;
}

TraxisWebMonitor
TraxisWebMonitorCreate(QBMiddlewareManager middlewareManager,
                       TraxisWebSessionManager sessionManager,
                       SvErrorInfo *errorOut)
{
    TraxisWebMonitor self = (TraxisWebMonitor) SvTypeAllocateInstance(TraxisWebMonitor_getType(), NULL);
    self->middlewareManager = SVRETAIN(middlewareManager);
    self->sessionManager = SVRETAIN(sessionManager);

    QBMiddlewareManagerAddListener(middlewareManager, (SvGenericObject) self);

    TraxisWebMonitorMiddlewareDataChanged((SvGenericObject) self, QBMiddlewareManagerType_Traxis);

    return self;
}
