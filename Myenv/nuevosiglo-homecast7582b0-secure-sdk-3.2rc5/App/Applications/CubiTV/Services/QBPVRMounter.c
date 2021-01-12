/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBPVRMounter.h"

#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvWeakList.h>
#include <SvFoundation/SvHashTable.h>
#include <SvCore/SvCommonDefs.h>
#include <QBShellCmds.h>
#include <QBRecordFS/file.h>
#include <QBRecordFS/root.h>
#include <QBRecordFS/loader.h>
#include <QBRecordFS/RecMetaUtility.h>
#include <QBRecordFS/RecMetaKeys.h>
#include <Services/diskPVRProvider/QBDiskPVRProvider.h>
#include <Services/core/hotplugMounts.h>
#include <main.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <safeFiles.h>
#include <SvJSON/SvJSONParse.h>
#include <stdbool.h>
#include <QBCASKeyManagerMeta.h>
#include <QBCASAuthPluginsFactory.h>

#define QBPVRMOUNTER_FORMAT_CONF "/tmp/qb_format_pvr_configuration"
#define QBPVRMOUNTER_FORMAT_PARTITION_CONF "/tmp/qb_format_partition_conf"

typedef enum {
    QBPVRMounterTask_idle = 0,
    QBPVRMounterTask_format,
    QBPVRMounterTask_mount,
    QBPVRMounterTask_unmount,
    QBPVRMounterTask_remount
} QBPVRMounterTask;

struct QBPVRMounter_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvWeakList listeners;
    SvFiber fiber;
    SvFiberTimer timer;
    bool pvrMounted;
    bool isPVREnabled;
    bool isTSEnabled;
    QBPVRMounterTask task;
    QBRecordFSLoader filesystemLoader;
    SvHashTable disksHash;
    QBActiveTree mainTree;
    QBShellPipe pipe;
    QBDiskStatus mounted;
    QBDiskStatus current;
    SvArray possibleOtherPartitions;
    SvString possiblePartitionsPath;
    SvString metaRoot;
    SvString dataDev;
    bool mountingPVRFileSystemEnabled; // if false, QBRecordFS won't be mounted during attach of drive
};

SvLocal void
QBDiskStatusDestroy(void *self_)
{
    QBDiskStatus self = self_;
    SVRELEASE(self->disk);
    self->disk = NULL;
    SVTESTRELEASE(self->otherPartitions);
}

SvLocal SvType
QBDiskStatus_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDiskStatusDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDiskStatus",
                            sizeof(struct QBDiskStatus_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvInterface QBPVRMounterListener_getInterface(void)
{
    static SvInterface interface = NULL;
    if (!interface) {
        SvInterfaceCreateManaged("QBPVRMounterListener", sizeof(struct QBPVRMounterListener_t), NULL, &interface, NULL);
    }

    return interface;
}

SvLocal void QBPVRMounterDestroy(void *self_)
{
    QBPVRMounter self = self_;
    SVRELEASE(self->listeners);
    SVRELEASE(self->disksHash);
    if (self->fiber)
        SvFiberDestroy(self->fiber);
    if (self->pipe)
        QBShellPipeClose(self->pipe);
    SVTESTRELEASE(self->mounted);
    SVTESTRELEASE(self->current);
    SVTESTRELEASE(self->possibleOtherPartitions);
    SVRELEASE(self->possiblePartitionsPath);
    SVTESTRELEASE(self->metaRoot);
    SVTESTRELEASE(self->dataDev);
}

QBDiskStatus QBPVRMounterGetDiskStatus(QBPVRMounter self, SvString id)
{
    QBDiskStatus diskStat = (QBDiskStatus) SvHashTableFind(self->disksHash, (SvObject) id);

    if (!diskStat) {
        SvLogError("Disk not found id=[%s]", SvStringCString(id));
        return NULL;
    }

    return diskStat;
}

SvLocal QBAppPVRDiskState QBPVRMounterGetDiskTypeSupport(QBPVRMounter self)
{
    if (self->isPVREnabled && self->isTSEnabled)    ///< default
        return QBPVRDiskState_pvr_ts_present;
    else if (self->isPVREnabled)                    ///< needed in some projects
        return QBPVRDiskState_pvr_present;
    else if (self->isTSEnabled)
        return QBPVRDiskState_ts_present;
    else
        return QBPVRDiskState_undefined;
}

void
QBPVRMounterAllowPVR(QBPVRMounter self, bool allowPVR)
{
    if (self->isPVREnabled == allowPVR)
        return;

    self->isPVREnabled = allowPVR;
}

void
QBPVRMounterAllowTS(QBPVRMounter self, bool allowTS)
{
    if (self->isTSEnabled == allowTS)
        return;

    self->isTSEnabled = allowTS;
}

void QBPVRMounterEnablePVRFileSystemMounting(QBPVRMounter self, bool enabled)
{
    self->mountingPVRFileSystemEnabled = enabled;
}

SvLocal void QBPVRMounterPropagateNodeChange(QBPVRMounter self, SvString id)
{
    QBActiveTreeNode node = QBActiveTreeFindNode(self->mainTree, id);
    if (!node)
        return;
    QBActiveTreePropagateNodeChange(self->mainTree, node, NULL);
}

bool QBPVRMounterIsFormatted(QBPVRMounter self, SvString id)
{
    QBDiskStatus diskStat = QBPVRMounterGetDiskStatus(self, id);
    return diskStat && (diskStat->pvr || diskStat->ts);
}

bool QBPVRMounterIsPVRUsed(QBPVRMounter self, SvString id)
{
    QBDiskStatus diskStat = QBPVRMounterGetDiskStatus(self, id);
    if (diskStat && diskStat->state == QBPVRMounterState_disk_mounted)
        return true;
    else
        return false;
}

SvLocal void QBPVRMounterGetOtherPartitions(QBPVRMounter self, QBDiskStatus diskStat, SvString rootNode)
{
    SVTESTRELEASE(diskStat->otherPartitions);
    diskStat->otherPartitions = NULL;
    char *filename;
    char *buf;
    asprintf(&filename, "%s/.qb_partition_config", SvStringCString(rootNode));
    QBFileToBuffer(filename, &buf);
    free(filename);
    if (!buf) {
        return;
    }

    SvIterator iter = SvArrayGetIterator(self->possibleOtherPartitions);
    SvValue possibleOtherPartition = NULL;
    char *paritionLabelString;

    while ((possibleOtherPartition = (SvValue) SvIteratorGetNext(&iter))) {
        asprintf(&paritionLabelString, "label=%s", SvStringCString(SvValueGetString(possibleOtherPartition)));
        if (strstr(buf, paritionLabelString)) {
            if (!diskStat->otherPartitions)
                diskStat->otherPartitions = SvArrayCreate(NULL);
            SvLogNotice("%s: adding partition %s", __func__, SvStringCString(SvValueGetString(possibleOtherPartition)));
            SvArrayAddObject(diskStat->otherPartitions, (SvObject) SvValueGetString(possibleOtherPartition));
        }
        free(paritionLabelString);
    }
    free(buf);
}

SvLocal void QBPVRMounterGetPartitions(QBPVRMounter self, QBDiskStatus diskStat)
{
    diskStat->pvr = false;
    diskStat->ts = false;
    diskStat->recLimit = -1;

    QBPartitionInfo metaPart = NULL;
    QBPartitionInfo curPart = NULL;
    SvIterator iter = SvArrayGetIterator(diskStat->disk->partitions);
    while ((curPart = (QBPartitionInfo) SvIteratorGetNext(&iter))) {
        if (curPart->type) {
            if (strcmp(SvStringCString(curPart->type), "pvrmeta") == 0) {
                metaPart = curPart;
                break;
            }
        }
    }

    if (!metaPart)
        return;

    char *filename;
    asprintf(&filename, "%s/.qb_pvr_partition_config", SvStringCString(metaPart->rootNode));
    int fd = open(filename, O_RDONLY);
    free(filename);
    if (fd < 0)
        return;

    char buf[1024];
    int ret = read(fd, buf, sizeof(buf) - 1);
    if (ret <= 0)
        goto fin;
    buf[ret] = 0;

    char *pvrStr = strstr(buf, "files=");
    char *tsStr = strstr(buf, "timeshift=");

    QBAppPVRDiskState diskSupport = QBPVRMounterGetDiskTypeSupport(self);
    if (pvrStr && (diskSupport == QBPVRDiskState_pvr_ts_present || diskSupport == QBPVRDiskState_pvr_present)) {
        diskStat->pvr = true;
    }
    if (tsStr && (diskSupport == QBPVRDiskState_pvr_ts_present || diskSupport == QBPVRDiskState_ts_present)) {
        diskStat->ts = true;
    }

    char *recLimitStr = strstr(buf, "reclimit=");
    if (recLimitStr) {
        diskStat->recLimit = strtoul(recLimitStr + 9, NULL, 0);
    }

    if (QBPVRLogicIsSpeedTestEnabled(self->appGlobals->pvrLogic)) {
        if (diskStat->recLimit >= 0 && diskStat->recLimit <= QBPVRLogicGetChannelCost(self->appGlobals->pvrLogic, QBDiskPVRProviderChannelCostClass_HDTV)) {
            diskStat->pvr = false;
            if (diskStat->recLimit < QBPVRLogicGetChannelCost(self->appGlobals->pvrLogic, QBDiskPVRProviderChannelCostClass_HDTV)) {
                diskStat->ts = false;
            }
        }
    }


    struct QBPVRLogicSupportedFeatures_ supportedFeatures;
    // Check supported features
    QBPVRLogicGetSupportedFeatures(self->appGlobals->pvrLogic, &supportedFeatures);
    diskStat->ts = diskStat->ts && supportedFeatures.timeshift;
    diskStat->pvr = diskStat->pvr && supportedFeatures.pvr;

    if (!self->possibleOtherPartitions) {
        SvLogNotice("%s: no possibleOtherPartitions", __func__);
        goto fin;
    }

    QBPVRMounterGetOtherPartitions(self, diskStat, metaPart->rootNode);

fin:
    close(fd);
}

SvLocal void QBPVRMounterGetPossibleOtherPartitions(QBPVRMounter self)
{
    const char *possiblePartitionsPath = SvStringCString(self->possiblePartitionsPath);
    char *buffer = NULL;
    QBFileToBuffer(possiblePartitionsPath, &buffer);

    SvArray possibleOtherPartitions = (SvArray) SvJSONParseString(buffer, false, NULL);
    free(buffer);
    if (possibleOtherPartitions && !SvObjectIsInstanceOf((SvObject) possibleOtherPartitions, SvArray_getType())) {
        SvLogError("%s: 'possibleOtherPartitions' is not an array", __func__);
        SVRELEASE(possibleOtherPartitions);
    } else if (possibleOtherPartitions) {
        SvIterator iter = SvArrayGetIterator(possibleOtherPartitions);
        SvValue possibleOtherPartition = NULL;
        while ((possibleOtherPartition = (SvValue) SvIteratorGetNext(&iter))) {
            if (!SvValueIsString(possibleOtherPartition)) {
                SvLogError("%s: 'possibleOtherPartitions' array contains at least one element which is not string", __func__);
                SVRELEASE(possibleOtherPartitions);
                return;
            }
        }
        SvLogNotice("Using possiblePartitions file [%s]", possiblePartitionsPath);
        self->possibleOtherPartitions = possibleOtherPartitions;
    }
}

SvLocal inline bool QBPVRMounterIsDiskComplete(QBPVRMounter self, QBDiskStatus diskStat)
{
    bool diskComplete = diskStat->disk->partCount != -1
                        && diskStat->disk->partCount <= (int) SvArrayGetCount(diskStat->disk->partitions);
    return diskComplete;
}

//Only to be called just after format when we have all partitions
SvLocal void QBPVRMounterUpdateDiskStat(QBPVRMounter self, QBDiskStatus diskStat)
{
    QBPVRMounterGetPartitions(self, diskStat);

    if ((diskStat->pvr || diskStat->ts) && QBPVRLogicIsDiskPVRCompatible(diskStat->disk)) {
        diskStat->state = QBPVRMounterState_disk_compatible;
    } else {
        diskStat->state = QBPVRMounterState_disk_incompatible;
    }
}

SvLocal void QBPVRMounterAutoMountPVRIfNeeded(QBPVRMounter self, QBDiskStatus diskStat)
{
    if (!self->pvrMounted && self->task == QBPVRMounterTask_idle && (diskStat->pvr || diskStat->ts)) {
        QBPVRMounterMount(self, diskStat->disk->devId);
    }
}

SvLocal void QBPVRMounterNotifyListeners(QBPVRMounter self, QBDiskStatus disk)
{
    SvIterator it = SvWeakListIterator(self->listeners);
    SvObject listener;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBPVRMounterListener, listener, mounterResultReceived, disk);
    }
}

SvLocal void QBPVRMounterPartitionAdded(SvObject self_, SvObject path, QBDiskInfo disk, QBPartitionInfo part)
{
    QBPVRMounter self = (QBPVRMounter) self_;
    QBDiskStatus diskStat = (QBDiskStatus) SvHashTableFind(self->disksHash, (SvObject) disk->devId);
    if (!diskStat) {
        diskStat = (QBDiskStatus) SvTypeAllocateInstance(QBDiskStatus_getType(), NULL);
        diskStat->disk = SVRETAIN(disk);
        diskStat->state = QBPVRMounterState_disk_checking;
        SvHashTableInsert(self->disksHash, (SvObject) disk->devId, (SvObject) diskStat);
        SVRELEASE(diskStat);
    }
    if (diskStat->state != QBPVRMounterState_disk_formatting) {
        if (!QBPVRMounterIsDiskComplete(self, diskStat)) {
            diskStat->state = QBPVRMounterState_disk_checking;
        } else {
            //Now all partitions are detected after formatting, so disk can be mounted
            QBPVRMounterUpdateDiskStat(self, diskStat);
            QBPVRMounterNotifyListeners(self, diskStat);
            QBPVRMounterAutoMountPVRIfNeeded(self, diskStat);
        }
        QBPVRMounterPropagateNodeChange(self, diskStat->disk->devId);
    }
}

SvLocal void QBPVRMounterDiskAdded(SvObject self_, SvObject path, QBDiskInfo disk)
{
    QBPVRMounterPartitionAdded(self_, path, disk, NULL);
}

SvLocal void QBPVRMounterLoaderUpdated(SvObject self_, SvObject loader_, SvObject arg)
{
    QBPVRMounter self = (QBPVRMounter) self_;

    if(loader_ && !SvObjectIsInstanceOf(loader_, QBRecordFSLoader_getType())) {
        return;  //in case of listening to another objects
    }
    QBRecordFSLoader loader = (QBRecordFSLoader) loader_;

    assert(loader == self->filesystemLoader);
    self->filesystemLoader = NULL;

    QBRecordFSRoot *recordFsRoot = NULL;
    if (self->mountingPVRFileSystemEnabled) {
        recordFsRoot = QBRecordFSLoaderTakeRoot(loader);
        QBRecordFSLoaderRequestAutodestruction(loader);
    }

    self->task = QBPVRMounterTask_idle;
    QBDiskStatus diskStat = QBPVRMounterGetDiskStatus(self, self->current->disk->devId);
    if (recordFsRoot || !self->mountingPVRFileSystemEnabled) {
        if (self->mountingPVRFileSystemEnabled) {
            SvLogNotice("QBRecordFSLoader -> finished");
        }
        SVTESTRELEASE(self->mounted);
        self->mounted = SVRETAIN(self->current);
        diskStat->state = QBPVRMounterState_disk_mounted;
        self->pvrMounted = true;
        QBPVRLogicSetRecLimit(self->appGlobals->pvrLogic, diskStat->recLimit);
        if (self->mountingPVRFileSystemEnabled) {
            QBDiskPVRProviderSetStorage((QBDiskPVRProvider) self->appGlobals->pvrProvider, recordFsRoot);
        }
        QBPVRMounterPropagateNodeChange(self, diskStat->disk->devId);
        QBPVRMounterNotifyListeners(self, diskStat);
        if (self->mountingPVRFileSystemEnabled) {
            if (diskStat->pvr && diskStat->ts)
                QBAppStateServiceSetPVRDiskState(self->appGlobals->appState, QBPVRDiskState_pvr_ts_present);
            else if (diskStat->pvr)
                QBAppStateServiceSetPVRDiskState(self->appGlobals->appState, QBPVRDiskState_pvr_present);
            else if (diskStat->ts)
                QBAppStateServiceSetPVRDiskState(self->appGlobals->appState, QBPVRDiskState_ts_present);
        } else {
            QBAppStateServiceSetPVRDiskState(self->appGlobals->appState, QBPVRDiskState_undefined);
        }
        SVRELEASE(self->current);
        self->current = NULL;
    } else {
        SvLogError("QBRecordFSLoader -> failed!");

        diskStat->state = QBPVRMounterState_error_mounting;

        QBPVRMounterPropagateNodeChange(self, diskStat->disk->devId);
        QBPVRMounterNotifyListeners(self, self->current);
        SVRELEASE(self->current);
        self->current = NULL;
    }
}

SvLocal SvObject QBPVRMounterMetaFileAuthPluginCreate(QBPVRMounter self)
{
    SvObject cas = QBCASGetInstance();
    SvObject plugin = NULL;
    if (SvObjectIsImplementationOf(cas, QBCASAuthPluginsFactory_getInterface())) {
        plugin = SvInvokeInterface(QBCASAuthPluginsFactory, cas, createMetaFileAuthPlugin);
    }

    return plugin;
}

SvLocal void QBPVRMounterPVRDetach(QBPVRMounter self)
{
    if (self->filesystemLoader) {
        SvLogNotice("Cancelling fs loader in progress");
        QBRecordFSLoaderRequestAutodestruction(self->filesystemLoader);
        self->filesystemLoader = NULL;
        return;
    }

    self->pvrMounted = false;

    QBRecordFSRoot *recordFsRoot = QBRecordFSRootGetGlobal();
    if (!recordFsRoot) {
        SvLogNotice("record_root_detach() : no root attached now");
        return;
    }

    QBRecordFSRootDetachAllSessions(recordFsRoot);
    QBRecordFSRootDetachAllFiles(recordFsRoot);

    QBDiskPVRProviderSetStorage((QBDiskPVRProvider) self->appGlobals->pvrProvider, NULL);

    SvObject metaFileAuthPlugin = QBPVRMounterMetaFileAuthPluginCreate(self);
    QBRecordFSLoader loader = QBRecordFSLoaderCreate(SvSchedulerGet(), metaFileAuthPlugin, (SvObject)self);
    SVTESTRELEASE(metaFileAuthPlugin);

    QBRecordFSLoaderGiveRoot(loader, recordFsRoot);
    QBRecordFSLoaderRequestAutodestruction(loader);
    QBAppStateServiceSetPVRDiskState(self->appGlobals->appState, QBPVRDiskState_undefined);
}

SvLocal void QBPVRMounterPVRAttach(QBPVRMounter self)
{
    if (self->filesystemLoader) {
        SvLogError("record_root_attach() : a loader is already in progress");
        return;
    }
    QBRecordFSRoot *recordFsRoot = QBRecordFSRootGetGlobal();
    if (recordFsRoot) {
        SvLogError("record_root_attach() : a root is already attached");
        return;
    }
    if (self->appGlobals->metaKeyManager) {
        QBCASKeyManagerMetaSetMetaRoot(self->appGlobals->metaKeyManager, SvStringCString(self->metaRoot));
        QBCASKeyManagerMetaUpdatePVRKeys(self->appGlobals->metaKeyManager);
    }

    if (self->mountingPVRFileSystemEnabled) {
        SvObject metaFileAuthPlugin = QBPVRMounterMetaFileAuthPluginCreate(self);
        QBRecordFSLoader loader = QBRecordFSLoaderCreate(SvSchedulerGet(), metaFileAuthPlugin, (SvObject)self);
        SVTESTRELEASE(metaFileAuthPlugin);
        QBRecordFSLoaderLoad(loader, SvStringCString(self->metaRoot), SvStringCString(self->dataDev));
        self->filesystemLoader = loader;
        SvLogNotice("Loading fs root in progress");
    } else {
        QBPVRMounterLoaderUpdated((SvObject) self, NULL, NULL);
        SvLogNotice("Mounted without PVR file system (QBRecordFS)");
    }
}

SvLocal void QBPVRMounterPartitionRemoved(SvObject self_, SvObject path, SvString remId, SvString diskId)
{
    QBPVRMounter self = (QBPVRMounter) self_;
    QBDiskStatus diskStat = QBPVRMounterGetDiskStatus(self, diskId);

    if (diskStat) {
        if (!strncmp(SvStringCString(diskStat->disk->devId), SvStringCString(remId), 3)) {
            if (diskStat->state == QBPVRMounterState_disk_mounted) {
                QBPVRMounterPVRDetach(self);
            }
        }
        if (SvObjectEquals((SvObject) remId, (SvObject) diskId))
            SvHashTableRemove(self->disksHash, (SvObject) diskId);
    }
}

SvType QBPVRMounter_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBPVRMounterDestroy
    };
    static SvType type = NULL;

    static const struct QBHotplugMountAgentListener_t hotplugMethods = {
        .partitionAdded   = QBPVRMounterPartitionAdded,
        .partitionRemoved = QBPVRMounterPartitionRemoved,
        .diskAdded        = QBPVRMounterDiskAdded,
        .diskRemoved      = QBPVRMounterPartitionRemoved
    };

    static const struct QBObserver_ observerMethods = {
        .observedObjectUpdated = QBPVRMounterLoaderUpdated
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBPVRMounter",
                            sizeof(struct QBPVRMounter_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBHotplugMountAgentListener_getInterface(), &hotplugMethods,
                            QBObserver_getInterface(), &observerMethods,
                            NULL);
    }
    return type;
}

SvLocal void check_disk_status(QBPVRMounter self)
{
    if (!self->pipe)
        return;

    int ret = QBShellPipeHasAsyncResult(self->pipe);
    if (ret == 1) {
        int result = QBShellPipeGetAsyncResult(self->pipe);

        QBShellPipeClose(self->pipe);
        self->pipe = NULL;
        if (result != 0) {
            if (self->task == QBPVRMounterTask_format) {
                self->current->state = QBPVRMounterState_error_formatting;
                self->task = QBPVRMounterTask_idle;
                QBPVRMounterPropagateNodeChange(self, self->current->disk->devId);
                QBPVRMounterNotifyListeners(self, self->current);
            } else if (self->task == QBPVRMounterTask_mount) {
                self->current->state = QBPVRMounterState_error_mounting;
                self->task = QBPVRMounterTask_idle;
                QBPVRMounterPropagateNodeChange(self, self->current->disk->devId);
                QBPVRMounterNotifyListeners(self, self->current);
            } else if (self->task == QBPVRMounterTask_unmount || self->task == QBPVRMounterTask_remount) {
                self->mounted->state = QBPVRMounterState_error_unmounting;
                self->task = QBPVRMounterTask_idle;
                QBPVRMounterPropagateNodeChange(self, self->mounted->disk->devId);
                QBPVRMounterNotifyListeners(self, self->mounted);
                SVRELEASE(self->mounted);
                self->mounted = NULL;
            }
            SVTESTRELEASE(self->current);
            self->current = NULL;
        } else {
            if (self->task == QBPVRMounterTask_format) {
                QBPVRMounterUpdateDiskStat(self, self->current);
                self->task = QBPVRMounterTask_idle;
                //If we received all callbacks about new partitions then we can use the disk
                //Otherwise we have to wait for AddPartition callbacks and there call UpdateDiskState
                if (!QBPVRMounterIsDiskComplete(self, self->current)) {
                    self->current->state = QBPVRMounterState_disk_checking;
                } else {
                    QBPVRMounterUpdateDiskStat(self, self->current);
                }
                QBPVRMounterPropagateNodeChange(self, self->current->disk->devId);
                QBPVRMounterNotifyListeners(self, self->current);

                QBPVRMounterAutoMountPVRIfNeeded(self, self->current);
            } else if (self->task == QBPVRMounterTask_unmount || self->task == QBPVRMounterTask_remount) {
                SVTESTRELEASE(self->metaRoot);
                self->metaRoot = NULL;
                self->mounted->state = QBPVRMounterState_disk_compatible;
                QBPVRMounterPropagateNodeChange(self, self->mounted->disk->devId);
                QBPVRMounterNotifyListeners(self, self->mounted);
                SVRELEASE(self->mounted);
                self->mounted = NULL;

                if (self->task == QBPVRMounterTask_unmount) {
                    self->task = QBPVRMounterTask_idle;
                } else if (self->task == QBPVRMounterTask_remount) {
                    self->task = QBPVRMounterTask_mount;
                    QBPVRMounterMount(self, self->current->disk->devId);
                }
            } else if (self->task == QBPVRMounterTask_mount) {
                SVTESTRELEASE(self->metaRoot);
                self->metaRoot = NULL;
                SVTESTRELEASE(self->dataDev);
                self->dataDev = NULL;
                QBPartitionInfo part;
                SvIterator it = SvArrayGetIterator(self->current->disk->partitions);
                while ((part = (QBPartitionInfo) SvIteratorGetNext(&it))) {
                    if (part->type) {
                        if (strcmp(SvStringCString(part->type), "pvrmeta") == 0) {
                            self->metaRoot = SVRETAIN(part->rootNode);
                        } else if (strcmp(SvStringCString(part->type), "qb_xtream") == 0) {
                            self->dataDev = SVRETAIN(part->devNode);
                        }
                    }
                }
                if (!self->metaRoot || !self->dataDev) {
                    self->current->state = QBPVRMounterState_error_mounting;
                    self->task = QBPVRMounterTask_idle;
                    QBPVRMounterPropagateNodeChange(self, self->current->disk->devId);
                    QBPVRMounterNotifyListeners(self, self->current);
                    return;
                }
                QBPVRMounterPVRAttach(self);
            }
        }
    } else if (ret == 0) {
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(250));
    } else {
        QBShellPipeClose(self->pipe);
        self->pipe = NULL;
        if (self->task == QBPVRMounterTask_format) {
            self->current->state = QBPVRMounterState_error_formatting;
            self->task = QBPVRMounterTask_idle;
            QBPVRMounterPropagateNodeChange(self, self->current->disk->devId);
            QBPVRMounterNotifyListeners(self, self->current);
        } else if (self->task == QBPVRMounterTask_mount) {
            self->current->state = QBPVRMounterState_error_mounting;
            self->task = QBPVRMounterTask_idle;
            QBPVRMounterPropagateNodeChange(self, self->current->disk->devId);
            QBPVRMounterNotifyListeners(self, self->current);
        } else if (self->task == QBPVRMounterTask_unmount || self->task == QBPVRMounterTask_remount) {
            self->mounted->state = QBPVRMounterState_error_unmounting;
            self->task = QBPVRMounterTask_idle;
            QBPVRMounterPropagateNodeChange(self, self->mounted->disk->devId);
            QBPVRMounterNotifyListeners(self, self->mounted);
            SVRELEASE(self->mounted);
            self->mounted = NULL;
        }

        SVTESTRELEASE(self->current);
        self->current = NULL;
    }
}

SvLocal void QBPVRMounterStep(void *self_)
{
    QBPVRMounter self = (QBPVRMounter) self_;

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    check_disk_status(self);
}

void QBPVRMounterStart(QBPVRMounter self, AppGlobals appGlobals)
{
    self->mainTree = appGlobals->menuTree;
    self->appGlobals = appGlobals;
    QBHotplugMountAgentAddListener(appGlobals->hotplugMountAgent, (SvGenericObject) self);
    QBRecordFSRoot* recordFsRoot = QBRecordFSRootGetGlobal();
    if (recordFsRoot) {
        self->pvrMounted = true;
    }
}

QBPVRMounter QBPVRMounterCreate(SvScheduler scheduler, SvString possiblePartitionsPath)
{
    QBPVRMounter self = (QBPVRMounter) SvTypeAllocateInstance(QBPVRMounter_getType(), NULL);
    self->listeners = SvWeakListCreate(NULL);
    self->fiber = SvFiberCreate(scheduler, NULL, "CommandFiber", QBPVRMounterStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    self->disksHash = SvHashTableCreate(10, NULL);
    self->isPVREnabled = true;
    self->isTSEnabled = true;
    self->mountingPVRFileSystemEnabled = true;
    self->possiblePartitionsPath = SVRETAIN(possiblePartitionsPath);
    QBPVRMounterGetPossibleOtherPartitions(self);

    return self;
}

void QBPVRMounterAddListener(QBPVRMounter self, SvObject listener)
{
    SvWeakListPushFront(self->listeners, listener, NULL);
}

void QBPVRMounterRemoveListener(QBPVRMounter self, SvObject listener)
{
    SvWeakListRemoveObject(self->listeners, listener);
}

void QBPVRMounterUnmount(QBPVRMounter self, SvString id)
{
    self->task = QBPVRMounterTask_unmount;
    QBPVRMounterPVRDetach(self);

    self->mounted->state = QBPVRMounterState_disk_unmounting;
    QBPVRMounterPropagateNodeChange(self, id);

    SvString shortDev = self->mounted->disk->devId;
    if (!shortDev)
        goto error;
    char *command;
    asprintf(&command, "qb_pvr_mount_usb_rw %s no", SvStringCString(shortDev));

    SvLogNotice("command [%s]", command);
    self->pipe = QBShellPipeOpen(command, NULL);
    free(command);
    if (!self->pipe)
        goto error;
    QBShellPipeRequestAsyncResult(self->pipe);

    if (self->fiber)
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(250));
    return;
error:
    self->mounted->state = QBPVRMounterState_error_unmounting;
    self->task = QBPVRMounterTask_idle;
    QBPVRMounterPropagateNodeChange(self, self->mounted->disk->devId);
    QBPVRMounterNotifyListeners(self, self->mounted);
    SVRELEASE(self->mounted);
    self->mounted = NULL;
}

void QBPVRMounterMount(QBPVRMounter self, SvString id)
{
    self->task = QBPVRMounterTask_mount;

    SVTESTRELEASE(self->current);
    self->current = SVRETAIN(QBPVRMounterGetDiskStatus(self, id));

    QBRecordFSRoot *recordFsRoot = QBRecordFSRootGetGlobal();
    if (recordFsRoot) {
        QBPVRMounterUnmount(self, self->mounted->disk->devId);
        if (self->task != QBPVRMounterTask_idle)
            self->task = QBPVRMounterTask_remount;
        return;
    }

    char *command = NULL;

    SvString shortDev = self->current->disk->devId;
    if (!shortDev)
        goto error;
    asprintf(&command, "qb_pvr_mount_usb_rw %s yes", SvStringCString(shortDev));

    SvLogNotice("command [%s]", command);

    self->pipe = QBShellPipeOpen(command, NULL);
    if (!self->pipe)
        goto error;

    int sendStat = QBShellPipeRequestAsyncResult(self->pipe);
    if (sendStat != 0) {
        if (self->pipe)
            QBShellPipeClose(self->pipe);
        self->pipe = NULL;
        goto error;
    }

    free(command);
    command = NULL;

    if (self->fiber)
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(250));

    self->current->state = QBPVRMounterState_disk_mounting;
    QBPVRMounterPropagateNodeChange(self, id);

    return;

error:
    free(command);
    self->current->state = QBPVRMounterState_error_mounting;
    self->task = QBPVRMounterTask_idle;
    QBPVRMounterPropagateNodeChange(self, self->current->disk->devId);
    QBPVRMounterNotifyListeners(self, self->current);
    SVRELEASE(self->current);

    self->current = NULL;
}

SvLocal int QBPVRMounterCreateFormatPartitionConf(QBPVRLogic self, SvString filename, SvString diskID, SvString formatPartition)
{
    QBBufferToFile(SvStringCString(filename), SvStringCString(formatPartition));
    return 0;
}

SvLocal int QBPVRMounterCopyPVRFormatConf(QBPVRMounter self)
{
    QBPartitionInfo metaPart = NULL;
    QBPartitionInfo curPart = NULL;
    SvIterator iter = SvArrayGetIterator(self->current->disk->partitions);
    while ((curPart = (QBPartitionInfo) SvIteratorGetNext(&iter))) {
        if (curPart->type) {
            if (strcmp(SvStringCString(curPart->type), "pvrmeta") == 0) {
                metaPart = curPart;
                break;
            }
        }
    }

    if (!metaPart)
        return -1;

    char *filename = NULL;
    char *buf = NULL;
    asprintf(&filename, "%s/.qb_partition_config", SvStringCString(metaPart->rootNode));
    QBFileToBuffer(filename, &buf);
    free(filename);

    if (!buf) {
        return -1;
    }
    QBBufferToFile(QBPVRMOUNTER_FORMAT_CONF, buf);

    free(buf);
    return 0;
}

void QBPVRMounterFormat(QBPVRMounter self, SvString id, SvString formatPartition)
{
    char *command = NULL;
    self->task = QBPVRMounterTask_format;
    QBActiveTreeNode node = QBActiveTreeFindNode(self->mainTree, id);
    if (!node)
        goto error;

    QBPVRMounterPVRDetach(self);

    SVTESTRELEASE(self->current);
    self->current = SVRETAIN(QBPVRMounterGetDiskStatus(self, id));

    self->current->state = QBPVRMounterState_disk_formatting;
    QBActiveTreePropagateNodeChange(self->mainTree, node, NULL);
    QBPVRMounterNotifyListeners(self, self->current);

    remove(QBPVRMOUNTER_FORMAT_PARTITION_CONF);
    if (formatPartition) {
        if (QBPVRMounterCreateFormatPartitionConf(self->appGlobals->pvrLogic, SVSTRING(QBPVRMOUNTER_FORMAT_PARTITION_CONF), id, formatPartition)) {
            goto error;
        }
    }

    remove(QBPVRMOUNTER_FORMAT_CONF);
    if (formatPartition) {
        if (QBPVRMounterCopyPVRFormatConf(self)) {
            goto error;
        }
    } else {
        if (QBPVRLogicCreatePVRFormatConf(self->appGlobals->pvrLogic, SVSTRING(QBPVRMOUNTER_FORMAT_CONF), id)) {
            goto error;
        }
    }
    asprintf(&command, "qb_unmount_and_format %s", SvStringCString(self->current->disk->devId));
    SvLogNotice("format: %s", command);
    self->pipe = QBShellPipeOpen(command, NULL);
    int sendStat = QBShellPipeRequestAsyncResult(self->pipe);

    if (sendStat != 0) {
        self->current->state = QBPVRMounterState_error_formatting;
        QBActiveTreePropagateNodeChange(self->mainTree, node, NULL);

        goto error;
    }
    if (self->fiber)
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(250));

    free(command);
    return;
error:
    free(command);
    self->task = QBPVRMounterTask_idle;
    if (self->current) {
        self->current->state = QBPVRMounterState_error_formatting;
        QBPVRMounterPropagateNodeChange(self, self->current->disk->devId);
        QBPVRMounterNotifyListeners(self, self->current);
    }
    if (self->pipe) {
        QBShellPipeClose(self->pipe);
        self->pipe = NULL;
    }
    SVTESTRELEASE(self->current);
    self->current = NULL;
    return;
}

QBPVRMounterState QBPVRMounterGetState(QBPVRMounter self, SvString id)
{
    QBDiskStatus diskStat = QBPVRMounterGetDiskStatus(self, id);
    if (!diskStat)
        return QBPVRMounterState_no_state;

    return diskStat->state;
}

bool QBPVRMounterIsBusy(QBPVRMounter self)
{
    if (self->task == QBPVRMounterTask_idle)
        return false;
    else
        return true;
}

SvString QBPVRMounterGetMetaRoot(QBPVRMounter self)
{
    return self->metaRoot;
}

SvString QBPVRMounterGetDataDev(QBPVRMounter self)
{
    return self->dataDev;
}
