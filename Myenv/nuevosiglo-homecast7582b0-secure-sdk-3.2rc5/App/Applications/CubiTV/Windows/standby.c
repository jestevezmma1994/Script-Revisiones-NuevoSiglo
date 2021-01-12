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

#include "standby.h"
#include <libintl.h>
#include <fibers/c/fibers.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <QBInput/QBInputCodes.h>
#include <Logic/StandbyLogic.h>
#include <main.h>

// Interval between consecutive executions of standby services
#define STANDBY_SERVICE_EXECUTION_INTERVAL_SEC 1800
// Standby service startup retry interval
#define STANDBY_SERVICE_RETRY_INTERVAL_SEC 10

struct QBStandbyContext_t {
    struct QBWindowContext_t super_;

    AppGlobals appGlobals;

    SvFiber fiber;
    SvFiberTimer timer;

    SvGenericObject currentService;
    SvArray services;
    SvArray oneshotServices;
    SvHashTable finishedServices;

    int completedPhasesCnt;
};
typedef struct QBStandbyContext_t* QBStandbyContext;

SvLocal void QBStandbyContextStep(void *self_)
{
    QBStandbyContext self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    if (!self->currentService) {
        for (unsigned i = 0; i < SvArrayCount(self->oneshotServices); i++) {
            SvGenericObject service = SvArrayObjectAtIndex(self->oneshotServices, i);
            bool started = SvInvokeInterface(QBStandbyContextService, service, tryStart, (QBWindowContext) self);
            if (!started)
                continue;
            self->currentService = SVRETAIN(service);
            break;
        }
    }
    if (!self->currentService) {
        for (unsigned i = 0; i < SvArrayCount(self->services); i++) {
            SvGenericObject service = SvArrayObjectAtIndex(self->services, i);
            bool started = SvInvokeInterface(QBStandbyContextService, service, tryStart, (QBWindowContext) self);
            if (!started)
                continue;
            self->currentService = SVRETAIN(service);
            break;
        }
    }

    if (!self->currentService) {
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(STANDBY_SERVICE_RETRY_INTERVAL_SEC * 1000));
    }
}

SvLocal bool QBStandbyContextInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBStandbyContext self =  w->prv;
    bool res = QBStandbyLogicHandleInputEvent(self->appGlobals->standbyLogic, e);
    if (res)
        return res;

    // Pass some keys to global handler ...
    int code = e->ch;
    if (code == 0) {
        code = e->fullInfo.u.key.code;
    }
    if (code == QBKEY_ABORT || code == QBKEY_POWER)
        return false;
#if 0
    if (code == 'Q' || code == 'W')
        return false;
#endif

    // ... and consume the rest
    return true;
}

SvLocal void QBStandbyContextClean(SvApplication app, void *ptr)
{
}

SvLocal void QBStandbyContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBStandbyContext self = (QBStandbyContext) self_;

    svSettingsPushComponent("Standby.settings");
    SvWidget window = svSettingsWidgetCreate(app, "Standby");
    window->prv = self;
    window->clean = QBStandbyContextClean;
    svWidgetSetInputEventHandler(window, QBStandbyContextInputEventHandler);
    svSettingsPopComponent();
    self->super_.window = window;

    self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBStandby", QBStandbyContextStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    if (SvArrayCount(self->services) || SvArrayCount(self->oneshotServices))
        SvFiberActivate(self->fiber);
}

SvLocal void QBStandbyContextDestroyWindow(QBWindowContext self_)
{
    QBStandbyContext self = (QBStandbyContext) self_;
    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;

    SvFiberDestroy(self->fiber);

    if (self->currentService) {
        SvInvokeInterface(QBStandbyContextService, self->currentService, stop);
        SVRELEASE(self->currentService);
        self->currentService = NULL;
    }
}

SvLocal void QBStandbyContextDestroy(void *self_)
{
    QBStandbyContext self = self_;
    SVRELEASE(self->services);
    SVRELEASE(self->finishedServices);
    SVRELEASE(self->oneshotServices);
}

SvInterface QBStandbyContextService_getInterface(void)
{
    static SvInterface interface = NULL;
    if (!interface) {
        SvInterfaceCreateManaged("QBStandbyContextService", sizeof(struct QBStandbyContextService_), NULL, &interface, NULL);
    }

    return interface;
}

SvType QBStandbyContext_getType(void)
{
    static SvType type = NULL;

    static const struct QBWindowContextVTable_ contextVTable = {
        .super_        = {
            .destroy   = QBStandbyContextDestroy
        },
        .createWindow  = QBStandbyContextCreateWindow,
        .destroyWindow = QBStandbyContextDestroyWindow
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBStandbyContext",
                            sizeof(struct QBStandbyContext_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            NULL);
    }

    return type;
}

QBWindowContext QBStandbyContextCreate(AppGlobals appGlobals)
{
    QBStandbyContext self = (QBStandbyContext) SvTypeAllocateInstance(QBStandbyContext_getType(), NULL);
    self->appGlobals = appGlobals;
    self->services = SvArrayCreate(NULL);
    self->oneshotServices = SvArrayCreate(NULL);
    self->finishedServices = SvHashTableCreate(32, NULL);
    QBStandbyLogicPrepareServices(appGlobals->standbyLogic, (QBWindowContext) self);
    return (QBWindowContext) self;
}

bool QBStandbyContextIsCompleted(QBWindowContext self_)
{
    QBStandbyContext self = (QBStandbyContext)self_;
    return (self->completedPhasesCnt > 0 || !SvArrayCount(self->services)) && !SvArrayCount(self->oneshotServices);
}

void QBStandbyContextDisableAutoOn(QBWindowContext self_)
{
}

void QBStandbyContextServiceFinished(QBWindowContext self_, SvGenericObject service, bool forcedFinish)
{
    QBStandbyContext self = (QBStandbyContext) self_;

    if (!SvObjectIsImplementationOf(service, QBStandbyContextService_getInterface())) {
        SvLogWarning("%s(): service of type %s doesn't implement QBStandbyContextService interface", __func__, service ? SvObjectGetTypeName(service) : "--");
        return;
    }

    if (self->currentService != service) {
        SvLogError("%s: service finished but was not run", __func__);
    }
    self->currentService = NULL;

    bool oneshot = (SvArrayIndexOfObject(self->oneshotServices, service) >= 0);

    if (!forcedFinish) {
        if (oneshot) {
            SvArrayRemoveObject(self->oneshotServices, service);
        } else {
            SvHashTableInsert(self->finishedServices, service, service);
            if (SvHashTableCount(self->finishedServices) >= SvArrayCount(self->services)) {
                self->completedPhasesCnt++;
                SvHashTableRemoveAllObjects(self->finishedServices);
            }
        }
    }

    if (oneshot) {
        if (forcedFinish) {
            SvArrayRemoveObject(self->oneshotServices, service);
            SvArrayAddObject(self->oneshotServices, service);
        }
    } else {
        SvArrayRemoveObject(self->services, service);
        SvArrayAddObject(self->services, service);
    }
    SVRELEASE(service); //release because we set currentService to NULL

    SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(STANDBY_SERVICE_EXECUTION_INTERVAL_SEC * 1000));
}

void QBStandbyContextAddService(QBWindowContext self_, SvGenericObject service, bool oneshot)
{
    QBStandbyContext self = (QBStandbyContext) self_;

    if (!SvObjectIsImplementationOf(service, QBStandbyContextService_getInterface())) {
        SvLogWarning("%s(): service of type %s doesn't implement QBStandbyContextService interface", __func__, service ? SvObjectGetTypeName(service) : "--");
        return;
    }

    if (!self->currentService && self->fiber)
        SvFiberActivate(self->fiber);

    if (oneshot)
        SvArrayAddObject(self->oneshotServices, service);
    else
        SvArrayAddObject(self->services, service);
}
