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

#include "SvEPGChannelListWaiter.h"

#include <libintl.h>
#include <stdbool.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGChannelListListener.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <Windows/appStartupWaiter.h>
#include <Utils/appType.h>

/**
 * The plugin waits for SvEPGManager which restores channels from flash.
 * The plugin registers on notification about new channels from SvEPGManager. If the channelsTimeoutMs time passed
 * and there weren't any channels then the plugin reports that it has finished its job.
 * But if the plugin receives any channels it checks the result of SvEPGManagerIsPropagatingChannels.
 * It waits for propagation's end and then reports that the application can continue.
 **/

struct SvEPGChannelListWaiter_ {
    struct SvObject_ super_;

    SvScheduler scheduler;
    SvEPGManager epgManager;

    bool hasChannels;
    bool hasFinished;

    SvTime startTime;
    int channelsTimeoutMs;

    SvFiber fiber;
    SvFiberTimer timer;
};

SvLocal void
SvEPGChannelListWaiterStep(void* self_)
{
    SvEPGChannelListWaiter self = (SvEPGChannelListWaiter) self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    if (self->epgManager && SvEPGManagerIsPropagatingChannels(self->epgManager)) {
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(250));
        return;
    }
    self->hasFinished = true;
}

// ----------------- SvEPGChannelListListener methods -----------------------

SvLocal void
SvEPGChannelListWaiterChannelAdded(SvObject self_, SvTVChannel channel)
{
    SvEPGChannelListWaiter self = (SvEPGChannelListWaiter) self_;

    self->hasChannels = true;
}

SvLocal void
SvEPGChannelListWaiterChannelRemoved(SvObject self_, SvTVChannel channel)
{
}

SvLocal void
SvEPGChannelListWaiterChannelModified(SvObject self_, SvTVChannel channel)
{
}

SvLocal void
SvEPGChannelListWaiterChannelListCompleted(SvObject self_, int pluginID)
{
}
// ------------------- QBAppStartupWaiterPlugin methods ---------------------

SvLocal bool
SvEPGChannelListWaiterTimeout(SvEPGChannelListWaiter self)
{
    if (SvTimeToMilliseconds(SvTimeSub(SvTimeGet(), self->startTime)) > self->channelsTimeoutMs) {
        return true;
    }

    return false;
}

SvLocal bool
SvEPGChannelListWaiterHasFinished(SvObject self_)
{
    SvEPGChannelListWaiter self = (SvEPGChannelListWaiter) self_;

    if (self->hasFinished)
        return true;

    if (SvEPGChannelListWaiterTimeout(self)) {
        return true;
    }

    if (self->hasChannels && !SvFiberEventIsArmed(self->timer)) {
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(2000));
    }

    return false;
}

SvLocal void
SvEPGChannelListWaiterStart(SvObject self_)
{
    SvEPGChannelListWaiter self = (SvEPGChannelListWaiter) self_;
    self->hasFinished = false;
    self->startTime = SvTimeGet();
}

SvLocal void
SvEPGChannelListWaiterStop(SvObject self_)
{
    SvEPGChannelListWaiter self = (SvEPGChannelListWaiter) self_;
    if (self->fiber) {
        SvFiberDeactivate(self->fiber);
        SvFiberEventDeactivate(self->timer);
    }
}

SvLocal void
SvEPGChannelListWaiterDestroy(void *self_)
{
    SvEPGChannelListWaiter self = self_;

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->timer = NULL;
    }

    SVRELEASE(self->epgManager);
}

SvLocal SvType
SvEPGChannelListWaiter_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = SvEPGChannelListWaiterDestroy
    };

    static const struct SvEPGChannelListListener_ methods = {
        .channelFound         = SvEPGChannelListWaiterChannelAdded,
        .channelLost          = SvEPGChannelListWaiterChannelRemoved,
        .channelModified      = SvEPGChannelListWaiterChannelModified,
        .channelListCompleted = SvEPGChannelListWaiterChannelListCompleted,
    };

    static const struct QBAppStartupWaiterPlugin_ appStartupMethods = {
        .start           = SvEPGChannelListWaiterStart,
        .stop            = SvEPGChannelListWaiterStop,
        .hasFinished     = SvEPGChannelListWaiterHasFinished,
    };

    if (!type) {
        SvTypeCreateManaged("SvEPGChannelListWaiter",
                            sizeof(struct SvEPGChannelListWaiter_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            SvEPGChannelListListener_getInterface(), &methods,
                            QBAppStartupWaiterPlugin_getInterface(), &appStartupMethods,
                            NULL);
    }

    return type;
}

SvEPGChannelListWaiter
SvEPGChannelListWaiterCreate(SvScheduler scheduler,
                             const int channelsTimeoutMs,
                             SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    SvEPGChannelListWaiter self = NULL;

    SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                         SVSTRING("SvEPGManager"));
    if (!epgManager) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBServiceRegistryGetService(\"SvEPGManager\") failed");
        goto fini;
    }
    if (!scheduler) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidArgument, error,
                                           "NULL scheduler argument passed");
        goto fini;
    }

    if (channelsTimeoutMs < 0) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidArgument, error,
                                           "invalid timeout value passed: channelsTimeoutMs = %d", channelsTimeoutMs);
        goto fini;
    }

    self = (SvEPGChannelListWaiter) SvTypeAllocateInstance(SvEPGChannelListWaiter_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate SvEPGChannelListWaiter");
        goto fini;
    }

    self->scheduler = scheduler;
    self->epgManager = SVRETAIN(epgManager);
    self->channelsTimeoutMs = channelsTimeoutMs;

    self->hasChannels = false;
    self->hasFinished = false;

    self->fiber = SvFiberCreate(self->scheduler, NULL, "SvEPGChannelListWaiter", &SvEPGChannelListWaiterStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);

    SvEPGManagerAddChannelListListener(self->epgManager, (SvObject) self, NULL);

fini:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

