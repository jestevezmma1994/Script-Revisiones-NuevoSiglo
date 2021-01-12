/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBConaxTimeshiftMonitor.h"
#include <SvFoundation/SvObject.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvEnv.h>
#include <QBICSmartcardPVRMetadata.h>
#include <fibers/c/fibers.h>
#include <QBCAS.h>

#include <SvPlayerManager/SvPlayerTask.h>
#include <SvPlayerManager/SvPlayerManager.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, "QBConaxTimeshiftMonitorLogLevel", "");

#define log_error(fmt, ...) do { if (env_log_level() >= 1) { SvLogError(COLBEG() "%s : " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } } while (0)
#define log_state(fmt, ...) do { if (env_log_level() >= 2) { SvLogNotice(COLBEG() "%s : " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } } while (0)
#define log_info(fmt, ...) do { if (env_log_level() >= 3) { SvLogNotice(COLBEG() "%s : " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)
#define log_fun(fmt, ...) do { if (env_log_level() >= 4) { SvLogNotice(COLBEG() "%s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)

struct QBConaxTimeshiftMonitor_s {
    struct SvObject_ super_;

    bool isServiceStarted;
    SvFiber fiber;
    SvFiberTimer timer;
};

SvLocal
void QBConaxTimeshiftMonitorStep(void *self_)
{
    log_fun();
    QBConaxTimeshiftMonitor self = (QBConaxTimeshiftMonitor) self_;

    SvFiberDeactivate(self->fiber);

    SvPlayerManager man = SvPlayerManagerGetInstance();
    SvPlayerTask playerTask = SvPlayerManagerGetPlayerTask(man, -1);

    if (!playerTask) {
        goto fini;
    }

    SvContentMetaData meta = SvPlayerTaskGetMetaData(playerTask);
    if (!meta) {
        log_error("Can't get content metadata.");
        goto fini;
    }

    SvObject obj = NULL;
    SvContentMetaDataGetObjectProperty(meta, SVSTRING(REC_META_KEY__CONAX_METADATA), &obj);

    if (!obj) {
        goto fini;
    }

    SvPlayerTaskState state = SvPlayerTaskGetState(playerTask);
    QBICSmartcardPVRMetadataSetTimeshiftDuration((QBICSmartcardPVRMetadata) obj, SvTimeConstruct(state.timeshift.range_end - state.timeshift.range_start, 0));

fini:
    SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(1, 0));
}

SvLocal void QBConaxTimeshiftMonitorDestroy(void *self_)
{
    log_fun();
    QBConaxTimeshiftMonitor self = (QBConaxTimeshiftMonitor) self_;

    if (self->isServiceStarted) {
        QBConaxTimeshiftMonitorStop(self);
    }

    SvFiberEventDestroy(self->timer);
    SvFiberDestroy(self->fiber);
}

SvLocal SvType QBConaxTimeshiftMonitor_getType(void)
{
    log_fun();
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBConaxTimeshiftMonitorDestroy
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBConaxTimeshiftMonitor",
                            sizeof(struct QBConaxTimeshiftMonitor_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

void QBConaxTimeshiftMonitorStart(QBConaxTimeshiftMonitor self)
{
    log_fun();
    if (self->isServiceStarted) {
        log_error("Service has been already started.");
        return;
    }

    self->isServiceStarted = true;
    SvFiberActivate(self->fiber);
}

void QBConaxTimeshiftMonitorStop(QBConaxTimeshiftMonitor self)
{
    log_fun();
    if (!self->isServiceStarted) {
        log_error("Service hasn't been started.");
        return;
    }

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    self->isServiceStarted = false;
}

QBConaxTimeshiftMonitor QBConaxTimeshiftMonitorCreate(SvScheduler scheduler)
{
    log_fun();
    QBConaxTimeshiftMonitor self = (QBConaxTimeshiftMonitor) SvTypeAllocateInstance(QBConaxTimeshiftMonitor_getType(), NULL);

    self->isServiceStarted = false;
    self->fiber = SvFiberCreate(scheduler, NULL, "QBConaxTimeshiftMonitor", QBConaxTimeshiftMonitorStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);

    return self;
}
