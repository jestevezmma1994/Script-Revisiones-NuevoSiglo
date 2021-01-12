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

#include "QBStandByTimerTimeout.h"
#include <QBConf.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <fibers/c/fibers.h>

struct QBStandByTimerTimeout_t {
    struct SvObject_ super_;

    unsigned int timeout;

    SvFiber fiber;
    SvFiberTimer timer;

    QBStandbyAgent standbyAgent;
    SvScheduler scheduler;
};

SvLocal int
QBStandByTimerTimeoutGetTimeoutSeconds(const char *value)
{
    if (!value) {
        return 0;
    }

    int number = 0;
    char unit = 0;

    sscanf(value, "%d%c", &number, &unit);

    if (unit == 'H') {
        number *= 360;
    } else if (unit == 'M' || unit != 'S') {
        number *= 60;
    }

    return number;
}

SvLocal void
QBStandByTimerTimeoutConfigChanged(SvGenericObject self_, const char *key, const char *value)
{
    if (!value || !self_) {
        return;
    }
    QBStandByTimerTimeout self = (QBStandByTimerTimeout) self_;
    self->timeout = QBStandByTimerTimeoutGetTimeoutSeconds(value);
    if (self->fiber && self->timer) {
        SvFiberDeactivate(self->fiber);
        SvFiberEventDeactivate(self->timer);
        if (self->timeout > 0) {
            SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(self->timeout, 0));
        }
    }
}

SvLocal void
QBStandByTimerTimeoutDestroy(void *self_)
{
    QBStandByTimerTimeout self = self_;
    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->timer = NULL;
    }
}

SvLocal SvType
QBStandByTimerTimeout_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBStandByTimerTimeoutDestroy
    };
    static const struct QBConfigListener_t configMethods = {
        .changed = QBStandByTimerTimeoutConfigChanged
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBStandByTimerTimeout",
                            sizeof(struct QBStandByTimerTimeout_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBConfigListener_getInterface(), &configMethods,
                            NULL);
    }

    return type;
}

SvLocal void
QBStandByTimerTimeoutStep(void *self_)
{
    QBStandByTimerTimeout self = (QBStandByTimerTimeout) self_;

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    if (self->standbyAgent && self->timeout > 0) {
        const char *configValue = QBConfigGet("AUTO_POWER");
        QBStandbyStateData state = QBStandbyStateDataCreate(QBStandbyStateFromAutoPowerString(configValue), (SvObject) self, false, SVSTRING("standby from QBStandByTimer"));
        QBStandbyAgentSetWantedState(self->standbyAgent, state);
        SVRELEASE(state);
        QBConfigSet("STANDBYTIMERTIMEOUT", "0");
        QBConfigSave();
    }
}

QBStandByTimerTimeout
QBStandByTimerTimeoutCreate(SvScheduler scheduler, QBStandbyAgent standbyAgent)
{
    if (!scheduler || !standbyAgent) {
        SvLogError("[%s] Wrong parameters(scheduler:%p, standbyAgent:%p",
                   __func__, scheduler, standbyAgent);
        return NULL;
    }
    QBStandByTimerTimeout self = NULL;
    if ((self = (QBStandByTimerTimeout) SvTypeAllocateInstance(QBStandByTimerTimeout_getType(), NULL))) {
        self->standbyAgent = standbyAgent;
        self->scheduler = scheduler;
        self->fiber = SvFiberCreate(scheduler, NULL, "QBStandByTimerTimeoutFiber", QBStandByTimerTimeoutStep, self);
        self->timer = SvFiberTimerCreate(self->fiber);
        self->timeout = QBStandByTimerTimeoutGetTimeoutSeconds(QBConfigGet("STANDBYTIMERTIMEOUT"));
    }
    return self;
}

void
QBStandByTimerTimeoutStart(QBStandByTimerTimeout self)
{
    if (self->timeout > 0) {
        SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(self->timeout, 0));
    }
    QBConfigAddListener((SvGenericObject) self, "STANDBYTIMERTIMEOUT");
}

void
QBStandByTimerTimeoutStop(QBStandByTimerTimeout self)
{
    if (self->fiber && self->timer) {
        SvFiberDeactivate(self->fiber);
        SvFiberEventDeactivate(self->timer);
    }
    QBConfigRemoveListener((SvGenericObject) self, "STANDBYTIMERTIMEOUT");
}
