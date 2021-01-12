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

#include "clocks.h"
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvType.h>
#include <QBPlatformHAL/QBPlatformFrontPanel.h>
#include <Logic/timeFormat.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBTimeDateMonitor.h>
#include <fibers/c/fibers.h>

struct QBFrontPanelClock_t {
    struct SvObject_ super_;
    SvFiber fiber;
    SvFiberTimer timer;
};
typedef struct QBFrontPanelClock_t *QBFrontPanelClock;

void QBFrontPanelUpdateClock_(SvGenericObject self_, bool firstTime, QBTimeDateUpdateSource source);
void QBFrontPanelUpdateClock(SvGenericObject self_);

SvLocal void QBFrontPanelClock__dtor__(void *self_)
{
    QBFrontPanelClock self = self_;
    SvFiberDestroy(self->fiber);
}

SvLocal SvType QBFrontPanelClock_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBFrontPanelClock__dtor__
    };
    static const struct QBTimeDateMonitorListener_ timeDateListenerMethods = {
        .systemTimeSet = QBFrontPanelUpdateClock_,
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBFrontPanelClock",
                            sizeof(struct QBFrontPanelClock_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBTimeDateMonitorListener_getInterface(), &timeDateListenerMethods, NULL);
    }
    return type;
}

SvLocal void QBFrontPanelClockStep(void *self_)
{
    QBFrontPanelClock self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    QBFrontPanelUpdateClock((SvGenericObject) self);
}

void
QBFrontPanelUpdateClock_(SvGenericObject self_, bool firstTime, QBTimeDateUpdateSource source)
{
    QBFrontPanelUpdateClock(self_);
}

void
QBFrontPanelUpdateClock(SvGenericObject self_)
{
    QBFrontPanelClock self = (QBFrontPanelClock) self_;
    if (!self)
        return;

    time_t now = SvTimeNow();
    if ( QBPlatformGetFrontPanelFlags() & QBFrontPanelCapability_clock) {
         QBPlatformUpdateClockOnFrontPanel(now);
    } else {
        char buf[32];
        struct tm tm;
        SvTimeBreakDown(SvTimeConstruct(now, 0), true, &tm);
        strftime(buf, sizeof(buf), QBTimeFormatGetCurrent()->frontPanelClockTime, &tm);
        QBPlatformShowOnFrontPanel(buf);
    }
    SvFiberTimerActivateAfter(self->timer, SvTimeFromMs((60 - (now % 60)) * 1000));
}

SvObject QBFrontPanelClockCreate(SvScheduler scheduler)
{
    QBFrontPanelClock self = (QBFrontPanelClock) SvTypeAllocateInstance(QBFrontPanelClock_getType(), NULL);
    self->fiber = SvFiberCreate(scheduler, NULL, "frontPanelClock", QBFrontPanelClockStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorAddListener(timeDateMonitor, (SvObject) self, NULL);

    return (SvGenericObject) self;
}

void QBFrontPanelClockStart(SvGenericObject self_)
{
    QBFrontPanelClock self = (QBFrontPanelClock) self_;
    SvFiberActivate(self->fiber);

    QBPlatformClockOnFrontPanelSetFormat(QBTimeFormatGetCurrent()->frontPanelClockTime);
}

void QBFrontPanelClockStop(SvGenericObject self_)
{
    QBFrontPanelClock self = (QBFrontPanelClock) self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
}
