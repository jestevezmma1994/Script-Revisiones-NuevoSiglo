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

#include "hddStandbyManager.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <QBShellCmds.h>

#include <QBPlatformHAL/QBPlatformFan.h>
#include <main.h>

struct QBHddStandbyManager_t {
    struct SvObject_ super_;
    SvScheduler scheduler;

    SvFiber fiber;
    SvFiberTimer pollTimer;
    QBShellPipe pollPipe;
    SvFiberTimer setStandbyTimer;
    QBShellPipe setStandbyPipe;

    bool diskInStandby;
    bool doStop;
    bool started;

    char *diskPath;
};

static const char *diskPath = "/tmp/qb_internal_storage";

SvLocal void QBHddStandbyManagerStep(void *self_)
{
    QBHddStandbyManager self = self_;
    SvFiberDeactivate(self->fiber);
    if (SvFiberEventIsActive(self->pollTimer)) {
        SvFiberEventDeactivate(self->pollTimer);
        if (self->pollPipe) {
            int ret = QBShellPipeHasAsyncResult(self->pollPipe);
            if (ret == 1) {
                int res = QBShellPipeGetAsyncResult(self->pollPipe);
                if (res) {
                    SvLogError("QBHddStandbyManager polling hdd status failed");
                    self->diskInStandby = false;
                } else {
                    char buf[128];
                    res = read(self->pollPipe->stdout_pipe, buf, sizeof(buf));
                    if (res > 0) {
                        buf[res] = 0;
                        if (strstr(buf, "standby")) {
                            self->diskInStandby = true;
                        } else {
                            self->diskInStandby = false;
                        }
                    } else {
                        self->diskInStandby = false;
                    }
                }
                QBShellPipeClose(self->pollPipe);
                self->pollPipe = NULL;
                SvFiberTimerActivateAfter(self->pollTimer, SvTimeFromMs(10 * 1000));
            } else if (ret == 0) {
                SvFiberTimerActivateAfter(self->pollTimer, SvTimeFromMs(250));
            } else {
                QBShellPipeClose(self->pollPipe);
                self->pollPipe = NULL;
                self->diskInStandby = false;
                SvFiberTimerActivateAfter(self->pollTimer, SvTimeFromMs(10 * 1000));
            }
        } else if (!self->doStop && self->started) {
            char* command;
            asprintf(&command, "hdparm -C %s", self->diskPath);
            self->pollPipe = QBShellPipeOpen(command, NULL);
            if (self->pollPipe) {
                int res = QBShellPipeRequestAsyncResult(self->pollPipe);
                if (res) {
                    QBShellPipeClose(self->pollPipe);
                    self->pollPipe = NULL;
                    self->diskInStandby = false;
                }
            } else {
                self->diskInStandby = false;
            }
            free(command);
            SvFiberTimerActivateAfter(self->pollTimer, SvTimeFromMs(250));
        }
    } else if (SvFiberEventIsActive(self->setStandbyTimer)) {
        SvFiberEventDeactivate(self->setStandbyTimer);
        int ret = QBShellPipeHasAsyncResult(self->setStandbyPipe);
        if (ret == 0)
            SvFiberTimerActivateAfter(self->setStandbyTimer, SvTimeFromMs(250));
        else if (ret == 1) {
                QBShellPipeGetAsyncResult(self->setStandbyPipe);
                QBShellPipeClose(self->setStandbyPipe);
                self->setStandbyPipe = NULL;
        } else if (ret == -1) {
            QBShellPipeClose(self->setStandbyPipe);
            self->setStandbyPipe = NULL;
            SvLogError("QBHddStandbyManager setting hdd standby failed");
        }
    }

    if (self->diskInStandby)
        QBPlatformSetFanSpeed(QBPlatformFanSpeed_none);
    else
        QBPlatformSetFanSpeed(QBPlatformFanSpeed_max);
}

SvLocal void setHddStandby(QBHddStandbyManager self, bool enable)
{
    if (self->setStandbyPipe) {
        QBShellPipeKill(self->setStandbyPipe);
        QBShellPipeClose(self->setStandbyPipe);
        self->setStandbyPipe = NULL;
    }

    char *command;
    asprintf(&command, "hdparm -S %i %s", enable ? 12 : 0, self->diskPath);

    self->setStandbyPipe = QBShellPipeOpen(command, NULL);
    if (self->setStandbyPipe) {
        int res = QBShellPipeRequestAsyncResult(self->setStandbyPipe);
        if (res) {
            QBShellPipeClose(self->setStandbyPipe);
            self->setStandbyPipe = NULL;
        }
        else {
            SvFiberTimerActivateAfter(self->setStandbyTimer, SvTimeFromMs(250));
        }
    }
    free(command);
}

SvLocal void QBHddStandbyManager__dtor__(void *self_)
{
    QBHddStandbyManager self = self_;
    if (self->fiber)
        SvFiberDestroy(self->fiber);

    if (self->diskPath)
        free(self->diskPath);
}

SvLocal SvType QBHddStandbyManager_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBHddStandbyManager__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBHddStandbyManager",
                            sizeof(struct QBHddStandbyManager_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBHddStandbyManager QBHddStandbyManagerCreate(AppGlobals appGlobals)
{
    QBHddStandbyManager self = (QBHddStandbyManager) SvTypeAllocateInstance(QBHddStandbyManager_getType(), NULL);
    self->scheduler = appGlobals->scheduler;

    self->fiber = SvFiberCreate(self->scheduler, NULL, "QBHddStandbyManager", QBHddStandbyManagerStep, self);
    self->pollTimer = SvFiberTimerCreate(self->fiber);
    self->setStandbyTimer = SvFiberTimerCreate(self->fiber);

    self->diskPath = NULL;

    long int path_max = pathconf(diskPath, _PC_PATH_MAX);
    if (path_max < 0) {
        SvLogEmergency("pathconf(%s) -> [%d, %s]", diskPath, errno, strerror(errno));
    } else {
        if (path_max == 0)
            path_max = PATH_MAX;

        char diskPathLink[path_max];
        int res = readlink(diskPath, diskPathLink, path_max);
        if (res < 0 || res >= path_max) {
            SvLogError("readlink(%s) -> [%d, %s]", diskPath, errno, strerror(errno));
        } else {
            diskPathLink[res] = '\0';
            self->diskPath = strdup(diskPathLink);
        }
    }

    //safe configuration for application start
    setHddStandby(self, false);
    QBPlatformSetFanSpeed(QBPlatformFanSpeed_max);


    return self;
}

void QBHddStandbyManagerStart(QBHddStandbyManager self)
{
    SvFiberTimerActivateAfter(self->pollTimer, SvTimeFromMs(250));
    self->diskInStandby = false;
    self->started = true;
    self->doStop = false;
    setHddStandby(self, true);
}

void QBHddStandbyManagerStop(QBHddStandbyManager self)
{
    QBPlatformSetFanSpeed(QBPlatformFanSpeed_max);
    setHddStandby(self, false);
    self->doStop = true;
    self->started = false;
    self->diskInStandby = false;
}

