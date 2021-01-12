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

#include "OutputStandardService.h"
#include <stdbool.h>
#include <SvCore/SvCommonDefs.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBPlatformHAL/QBPlatformOutput.h>
#include <SvGfxHAL/SvGfxEngine.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvString.h>
#include <QBViewport.h>
#include <Logic/VideoOutputLogic.h>

#define MAX_OUTPUTS 2

struct QBOutputStandardService_t {
    struct SvObject_ super_;
    SvFiber fiber;
    SvFiberTimer timer;
    bool active;
    QBViewport viewport;
    VideoOutputLogic videoOutputLogic;
    bool changing[MAX_OUTPUTS];
    QBOutputStandard currentMode[MAX_OUTPUTS];
};

SvLocal void QBOutputStandardService__dtor__(void *self_)
{
    QBOutputStandardService self = self_;
    SvFiberDestroy(self->fiber);
    SVRELEASE(self->videoOutputLogic);
}

SvLocal SvType QBOutputStandardService_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBOutputStandardService__dtor__
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBOutputStandardService",
                            sizeof(struct QBOutputStandardService_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal void QBOutputStandardServiceBlockAllOutputsDependencies(QBOutputStandardService self, bool block)
{
    QBViewportBlockVideo(self->viewport, SVSTRING("QBOutputStandardService"), QBViewportBlockVideoReason_unknown, block);
}

SvLocal void QBOutputStandardServiceBlockSingleOutputDependencies(QBOutputStandardService self, bool block, unsigned int outputID)
{
    SvGfxEngineEnableGfxOnOutput(outputID, !block, false);
}

SvLocal void QBOutputStandardServiceStep(void *self_)
{
    QBOutputStandardService self = self_;
    SvFiberEventDeactivate(self->timer);
    SvFiberDeactivate(self->fiber);

    if (QBViewportHasPendingCommands(self->viewport)) {
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(100));
        return;
    }

    unsigned int i;
    for (i = 0; i < MAX_OUTPUTS; i++) {
        if (self->changing[i]) {
            QBPlatformOutputSetStandard(i, self->currentMode[i], QBPlatformOutputPriority_application);

            QBVideoOutputConfig cfg;
            QBPlatformGetOutputConfig(i, &cfg);
            if ((cfg.capabilities & QBOutputCapability_SD) && cfg.masterID == i) {
                VideoOutputLogicSetupOverscanCompensation(self->videoOutputLogic);
            }
        }
    }

    QBOutputStandardServiceBlockAllOutputsDependencies(self, false);

    for (i = 0; i < MAX_OUTPUTS; i++) {
        if(self->changing[i]) {
            QBOutputStandardServiceBlockSingleOutputDependencies(self, false, i);
            self->changing[i] = false;
        }
    }

    self->active = false;
}

QBOutputStandardService QBOutputStandardServiceCreate(SvScheduler scheduler, QBViewport viewport, VideoOutputLogic videoOutputLogic)
{
    QBOutputStandardService self = (QBOutputStandardService) SvTypeAllocateInstance(QBOutputStandardService_getType(), NULL);
    self->fiber = SvFiberCreate(scheduler, NULL, "OutputStandardService", QBOutputStandardServiceStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    self->viewport = viewport;
    self->videoOutputLogic = SVRETAIN(videoOutputLogic);
    self->active = false;
    return self;
}

void QBOutputStandardServiceStart(QBOutputStandardService self)
{
}

void QBOutputStandardServiceStop(QBOutputStandardService self)
{
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
}

void QBOutputStandardServiceChangeMode(QBOutputStandardService self, QBOutputStandard newMode, unsigned int outputID)
{
    if (outputID >= MAX_OUTPUTS)
        return;

    if (self->currentMode[outputID] == newMode)
        return;

    self->currentMode[outputID] = newMode;
    self->changing[outputID] = true;

    if (!self->active) {
        self->active = true;
        SvFiberEventDeactivate(self->timer);
        SvFiberActivate(self->fiber);
    }

    QBOutputStandardServiceBlockAllOutputsDependencies(self, true);
    QBOutputStandardServiceBlockSingleOutputDependencies(self, true, outputID);
}
