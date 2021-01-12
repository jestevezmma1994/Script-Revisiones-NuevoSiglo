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

#include "QBNoInputStandbyPlugin.h"
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <fibers/c/fibers.h>
#include <Logic/TVLogic.h>
#include <QBInput/QBInputService.h>
#include <Services/standbyAgent/QBStandbyAgentService.h>
#include <Services/QBStandbyAgent.h>
#include <QBConf.h>
#include <QBStringUtils.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvType.h>
#include <SvQuirks/SvQuirks.h>
#include <main.h>

ONLY_ON_NONSECURE(
SV_DECL_INT_ENV_FUN_DEFAULT(env_FastNoIputStandby, 0, "FastNoIputStandby", "");
)

struct QBNoInputStandbyPlugin_ {
    struct SvObject_ super_;

    AppGlobals  appGlobals;

    bool enabled;

    QBStandbyState currentState;
    QBStandbyState configState;

    SvTime wantedStandbyTimeout;

    SvTime startTime;
    SvFiber  fiber;
    SvFiberTimer  timer;
};


static const unsigned int REFRESH_TIME_MS = 1000;

SvLocal void
QBNoInputStandbyPluginCheckTimeoutChange(QBNoInputStandbyPlugin self);


//Fiber methods
SvLocal void
QBNoInputStandbyPluginStep(void* self_)
{
    QBNoInputStandbyPlugin self = (QBNoInputStandbyPlugin) self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(REFRESH_TIME_MS));

    //Check input state
    SvTime lastEventTime = QBInputServiceGetLastEventTime(NULL);

    if (SvTimeCmp(lastEventTime, self->startTime) < 0) {
        lastEventTime = self->startTime;
    }

    if (SvTimeToMs(lastEventTime) > 0) {
        if (SvTimeCmp(SvTimeSub(SvTimeGet(), self->wantedStandbyTimeout), lastEventTime) > 0) {
            // exceeded timeout - set standby state
            self->currentState =  self->configState;
            return;
        }
    }

    self->currentState = QBStandbyState_on;
}

SvLocal void
QBNoInputStandbyPluginStartFiber(QBNoInputStandbyPlugin self)
{
    if (SvTimeGetSeconds(self->wantedStandbyTimeout) <= 0) {
        return;
    }

    if (self->fiber) {
        return;
    }

    self->startTime = SvTimeGet();

    self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBNoInputStandbyPlugin", &QBNoInputStandbyPluginStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    SvFiberActivate(self->fiber);
}

SvLocal void
QBNoInputStandbyPluginStopFiber(QBNoInputStandbyPlugin self)
{
    if (!self->fiber) {
        return;
    }

    SvFiberDestroy(self->fiber);
    self->fiber = NULL;
    self->timer = NULL;
}


//standbyServiceMethods
SvLocal void
QBNoInputStandbyPluginStart(SvGenericObject self_)
{
    if (!self_) {
        return;
    }

    QBConfigAddListener(self_, "AUTO_POWER_TIMEOUT");
    QBConfigAddListener(self_, "AUTO_POWER");

    QBNoInputStandbyPlugin self = (QBNoInputStandbyPlugin) self_;

    self->enabled = true;
    self->currentState = QBStandbyState_unknown;

    const char *configValue = QBConfigGet("AUTO_POWER");
    self->configState = QBStandbyStateFromAutoPowerString(configValue);

    QBNoInputStandbyPluginStartFiber(self);

    QBNoInputStandbyPluginCheckTimeoutChange(self);
}

SvLocal void
QBNoInputStandbyPluginStop(SvGenericObject self_)
{
    if (!self_) {
        return;
    }

    QBConfigRemoveListener(self_, "AUTO_POWER_TIMEOUT");
    QBConfigRemoveListener(self_, "AUTO_POWER");

    QBNoInputStandbyPlugin self = (QBNoInputStandbyPlugin) self_;

    self->enabled = false;
    self->currentState = QBStandbyState_unknown;
    QBNoInputStandbyPluginStopFiber(self);
}

SvLocal QBStandbyStateData
QBNoInputStandbyPluginCreateStateData(SvGenericObject self_)
{
    QBNoInputStandbyPlugin self = (QBNoInputStandbyPlugin) self_;
    QBStandbyStateData stateData = QBStandbyStateDataCreate(self->currentState, (SvObject) self, true, SVSTRING("NoInput"));
    return stateData;
}

SvLocal void
QBNoInputStandbyPluginResetState(SvGenericObject self_, bool popupClose)
{
    QBNoInputStandbyPlugin self = (QBNoInputStandbyPlugin) self_;
    self->currentState = QBStandbyState_unknown;
    self->startTime = SvTimeGet();
}

SvLocal void
QBNoInputStandbyPluginCheckTimeoutChange(QBNoInputStandbyPlugin self)
{
    const char *value = QBConfigGet("AUTO_POWER_TIMEOUT");

    if ((!value) || (strcmp(value, "DISABLED") == 0)) {
        self->wantedStandbyTimeout = SvTimeFromMilliseconds(0);
    } else {
        SvTime duration;
        if (QBStringToTimeDuration(value, QBTimeDurationFormat_ISO8601_TIME, &duration) > 0) {
            self->wantedStandbyTimeout = duration;
ONLY_ON_NONSECURE(
            if (env_FastNoIputStandby()) {
                self->wantedStandbyTimeout.us /= 60;
            }
)
        } else {
            self->wantedStandbyTimeout = SvTimeFromMilliseconds(0);
        }
    }

    if (SvTimeGetSeconds(self->wantedStandbyTimeout) <= 0) {
        QBNoInputStandbyPluginStopFiber(self);
    } else if (self->enabled) {
        QBNoInputStandbyPluginStartFiber(self);
    }
}

SvLocal void
QBNoInputStandbyPluginConfigChanged(SvGenericObject self_, const char *key, const char *value)
{
    QBNoInputStandbyPlugin self = (QBNoInputStandbyPlugin) self_;

    if (strcmp(key, "AUTO_POWER") == 0) {
        self->configState = QBStandbyStateFromAutoPowerString(value);
    }

    if (strcmp(key, "AUTO_POWER_TIMEOUT") == 0) {
        QBNoInputStandbyPluginCheckTimeoutChange(self);
    }
}

SvLocal void
QBNoInputStandbyPluginDestroy(void *self_)
{
    QBNoInputStandbyPlugin self = self_;

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
    }
}

SvLocal SvType
QBNoInputStandbyPlugin_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNoInputStandbyPluginDestroy
    };
    static SvType type = NULL;

    static const struct QBStandbyAgentService_ standbyServiceMethods = {
        .createStateData = QBNoInputStandbyPluginCreateStateData,
        .resetState = QBNoInputStandbyPluginResetState,
        .start = QBNoInputStandbyPluginStart,
        .stop = QBNoInputStandbyPluginStop,
    };

    static const struct QBConfigListener_t configMethods = {
        .changed = QBNoInputStandbyPluginConfigChanged
    };

    if (!type) {
        SvTypeCreateManaged("QBNoInputStandbyPlugin",
                            sizeof(struct QBNoInputStandbyPlugin_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBStandbyAgentService_getInterface(), &standbyServiceMethods,
                            QBConfigListener_getInterface(), &configMethods,
                            NULL);
    }

    return type;
}

QBNoInputStandbyPlugin
QBNoInputStandbyPluginCreate(AppGlobals appGlobals, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBNoInputStandbyPlugin self = (QBNoInputStandbyPlugin)SvTypeAllocateInstance(QBNoInputStandbyPlugin_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBNoInputStandbyPlugin");
        goto fini;
    }

    self->appGlobals = appGlobals;
    self->enabled = false;
    self->currentState = QBStandbyState_unknown;

fini:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}
