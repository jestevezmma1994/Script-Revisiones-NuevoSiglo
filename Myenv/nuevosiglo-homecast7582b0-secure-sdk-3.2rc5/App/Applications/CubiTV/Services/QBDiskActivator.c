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
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#include "QBDiskActivator.h"

#include <QBSchedManager.h>
#include <Services/standbyAgent/QBStandbyAgentService.h>
#include <Services/QBStandbyAgent.h>
#include <Services/core/hotplugMounts.h>

#include <fibers/c/fibers.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvArray.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <malloc.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBDiskActivator"

    SV_DECL_INT_ENV_FUN_DEFAULT( env_log_level, 2, moduleName "LogLevel", "" );

    #define log_debug(fmt, ...)     do { if(env_log_level() >= 5) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ##__VA_ARGS__); } while(0)
    #define log_warning(fmt, ...)   do { if(env_log_level() >= 1) SvLogWarning(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ##__VA_ARGS__); } while(0)
    #define log_error(fmt, ...)     do { if(env_log_level() >= 1) SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ##__VA_ARGS__); } while(0)
#else
    #define log_debug(fmt, ...)
    #define log_warning(fmt, ...)
    #define log_error(fmt, ...)
#endif /* SV_LOG_LEVEL */

// Default period between pinging disks
SvLocal const int DEFAULT_STEP_TIME_IN_SEC  = 20 * 60;    /* 20 min */
// Wake up time before scheduled record
SvLocal const int WAKE_UP_BEFORE_IN_SEC     =  2 * 60;    /*  2 min */
// Number of bytes we will read to wake up disk
SvLocal const int BYTES_TO_READ             = 32 * 1024;  /* 32 KB  */
// Number of device blocks to read at once
SvLocal const int BLOCKS_TO_READ_AT_ONCE    =  4;

struct QBDiskActivator_ {
    struct SvObject_ super_;
    QBStandbyAgent standbyAgent;
    QBHotplugMountAgent hotplugMountAgent;
    SvBackgroundScheduler scheduler;
    SvFiber diskFiber;
    SvFiberTimer diskTimer;
    SvArray diskPaths;
};

SvLocal void QBDiskActivatorAddDisk(QBDiskActivator self, QBDiskInfo disk)
{
    log_debug("Disk added (%s)", SvStringCString(disk->devId));
    SvArrayAddObject(self->diskPaths, (SvObject) disk);
}

SvLocal void QBDiskActivatorRemoveDisk(QBDiskActivator self, SvString path)
{
    QBDiskInfo disk;
    SvIterator it = SvArrayGetIterator(self->diskPaths);
    while ((disk = (QBDiskInfo) SvIteratorGetNext(&it))) {
        if (SvObjectEquals((SvObject) path, (SvObject) disk->devId)) {
            log_debug("Disk removed (%s)", SvStringCString(disk->devId));
            SvArrayRemoveObject(self->diskPaths, (SvObject) disk);
        }
    }
}

/*
 * standby service virtual methods
 */
SvLocal void QBDiskActivatorStandbyServiceStart(SvObject self_)
{
    QBDiskActivator self = (QBDiskActivator) self_;
    SvFiberActivate(self->diskFiber);
}

SvLocal void QBDiskActivatorStandbyServiceStop(SvObject self_)
{
}

SvLocal QBStandbyStateData QBDiskActivatorStandbyServiceCreateStateData(SvObject self_)
{
    return QBStandbyStateDataCreate(QBStandbyState_unknown, self_, false, SVSTRING("QBDiskActivatorStandbyService"));
}

SvLocal void QBDiskActivatorStandbyServiceResetState(SvObject self_, bool popupClose)
{
}

/*
 * hotplug agent virtual methods
 */
SvLocal void QBDiskActivatorPartitionAdded(SvObject self_, SvObject path, QBDiskInfo disk, QBPartitionInfo part)
{
}

SvLocal void QBDiskActivatorDiskAdded(SvObject self_, SvObject path, QBDiskInfo disk)
{
    if (disk && disk->devNode) {
        QBDiskActivatorAddDisk((QBDiskActivator) self_, disk);
    }
}

SvLocal void QBDiskActivatorPartitionRemoved(SvObject self_, SvObject path, SvString remId, SvString diskId)
{
    QBDiskActivator self = (QBDiskActivator) self_;

    if (remId && diskId && SvObjectEquals((SvObject) diskId, (SvObject) remId)) {
        QBDiskActivatorRemoveDisk(self, diskId);
    }
}

SvLocal void QBDiskActivatorDestroy(void *self_)
{
    QBDiskActivator self = (QBDiskActivator) self_;

    SvFiberDestroy(self->diskFiber);
    SvSchedulerBreakLoop(SvBackgroundSchedulerGetScheduler(self->scheduler));
    SvBackgroundSchedulerDestroy(self->scheduler);
    self->scheduler = NULL;
    SVRELEASE(self->hotplugMountAgent);
    SVRELEASE(self->standbyAgent);
    SVRELEASE(self->diskPaths);
}

SvLocal SvType QBDiskActivator_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDiskActivatorDestroy
    };

    static const struct QBStandbyAgentService_ standbyServiceMethods = {
        .createStateData = QBDiskActivatorStandbyServiceCreateStateData,
        .resetState      = QBDiskActivatorStandbyServiceResetState,
        .start           = QBDiskActivatorStandbyServiceStart,
        .stop            = QBDiskActivatorStandbyServiceStop,
    };

    static const struct QBHotplugMountAgentListener_t hotplugMethods = {
        .partitionAdded   = QBDiskActivatorPartitionAdded,
        .partitionRemoved = QBDiskActivatorPartitionRemoved,
        .diskAdded        = QBDiskActivatorDiskAdded,
        .diskRemoved      = QBDiskActivatorPartitionRemoved
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDiskActivator",
                            sizeof(struct QBDiskActivator_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBStandbyAgentService_getInterface(), &standbyServiceMethods,
                            QBHotplugMountAgentListener_getInterface(), &hotplugMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBDiskActivatorReadFromDisk(SvString pathToRead)
{
    ssize_t ret;
    int bytesRead = 0;

    int fd = open(SvStringCString(pathToRead), O_RDONLY | O_DIRECT);
    if (fd == -1) {
        log_warning("Error during open(): %s", strerror(errno));
        return;
    }

    unsigned long numblocks = 0;
    if (ioctl(fd, BLKSSZGET, &numblocks) < 0) {
        log_warning("Error during ioctl(): %s", strerror(errno));
        goto error;
    }

    const unsigned long bufferSize = numblocks * BLOCKS_TO_READ_AT_ONCE;
    char *buff = memalign(numblocks, bufferSize);
    if (buff == NULL) {
        log_error("Out of memory - memalign() failed");
        goto error;
    }

    while (bytesRead < BYTES_TO_READ) {
        ret = read(fd, buff, bufferSize);
        if (ret > 0) {
            bytesRead += ret;
        } else {
            log_warning("read() operation failed");
            free(buff);
            goto error;
        }
    }
    free(buff);
error:
    log_debug("Bytes read from device: %d", bytesRead);
    close(fd);
}

SvLocal void QBDiskActivatorPingDisks(QBDiskActivator self)
{
    QBDiskInfo disk;
    SvIterator it = SvArrayGetIterator(self->diskPaths);
    while ((disk = (QBDiskInfo) SvIteratorGetNext(&it))) {
        log_debug("Pinging disk: %s", SvStringCString(disk->devNode));
        QBDiskActivatorReadFromDisk(disk->devNode);
    }
}

SvLocal void QBDiskActivatorStep(void *self_)
{
    log_debug("Doing disk activator step");
    QBDiskActivator self = (QBDiskActivator) self_;
    SvFiberDeactivate(self->diskFiber);
    SvFiberEventDeactivate(self->diskTimer);

    if (self->standbyAgent && QBStandbyAgentIsStandby(self->standbyAgent)) {
        int nearestSched = QBSchedManagerGetNearest(-1);
        if (nearestSched > WAKE_UP_BEFORE_IN_SEC) {
            nearestSched -= WAKE_UP_BEFORE_IN_SEC;
            log_debug("Next wake up after %d sec (%d sec before scheduled event)", nearestSched, WAKE_UP_BEFORE_IN_SEC);
            SvFiberTimerActivateAfter(self->diskTimer, SvTimeFromMs(1000 * nearestSched));
        } else if (nearestSched >= 0) {
            QBDiskActivatorPingDisks(self);
            log_debug("Next wake up after %d sec", DEFAULT_STEP_TIME_IN_SEC);
            SvFiberTimerActivateAfter(self->diskTimer, SvTimeFromMs(1000 * DEFAULT_STEP_TIME_IN_SEC));
        } else {
            log_debug("No need to schedule wake up nor keep disks alive"); /* do nothing */
        }
    } else {
        QBDiskActivatorPingDisks(self);
        log_debug("Next wake up after %d sec", DEFAULT_STEP_TIME_IN_SEC);
        SvFiberTimerActivateAfter(self->diskTimer, SvTimeFromMs(1000 * DEFAULT_STEP_TIME_IN_SEC));
    }
}

void QBDiskActivatorStart(QBDiskActivator self)
{
    if (self->hotplugMountAgent) {
        QBHotplugMountAgentAddListener(self->hotplugMountAgent, (SvObject) self);
    }

    SvFiberActivate(self->diskFiber);
}

void QBDiskActivatorStop(QBDiskActivator self)
{
    SvFiberEventDeactivate(self->diskTimer);
    SvFiberDeactivate(self->diskFiber);

    if (self->hotplugMountAgent) {
        QBHotplugMountAgentRemoveListener(self->hotplugMountAgent, (SvObject) self);
    }
}

QBDiskActivator QBDiskActivatorCreate(QBStandbyAgent standbyAgent, QBHotplugMountAgent hotplugMountAgent)
{
    if (!standbyAgent) {
        log_error("Creating disk activator failed: standbyAgent is not defined");
        return NULL;
    }

    if (!hotplugMountAgent) {
        log_error("Creating disk activator failed: hotplugMountAgent is not defined");
        return NULL;
    }

    QBDiskActivator self = (QBDiskActivator) SvTypeAllocateInstance(QBDiskActivator_getType(), NULL);

    self->scheduler = SvBackgroundSchedulerCreate("QBDiskActivator");
    self->diskPaths = SvArrayCreate(NULL);
    self->diskFiber = SvFiberCreate(SvBackgroundSchedulerGetScheduler(self->scheduler), NULL, "DiskActivator", QBDiskActivatorStep, self);
    self->diskTimer = SvFiberTimerCreate(self->diskFiber);
    self->standbyAgent = SVRETAIN(standbyAgent);
    self->hotplugMountAgent = SVRETAIN(hotplugMountAgent);

    return self;
}
