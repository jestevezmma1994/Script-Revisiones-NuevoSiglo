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

#include "QBPVODStorage.h"
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>

#include <SvQuirks/SvRuntimePrefix.h>

#include <SvJSON/SvJSONClassRegistry.h>
#include <SvJSON/SvJSONParse.h>

#include <QBShellCmds.h>

#include <main.h>
#include <Services/core/JSONserializer.h>
#include <Utils/value.h>
#include <safeFiles.h>

#include <QuadriCast/MStore-qb.h>


SV_DECL_STRING_ENV_FUN( env_fake_mstore_dir , "QBPVODFakeDir" , "Fake PushVoD directory path" );

#define log_error(fmt, ...)                                                 \
    do { if (1) { SvLogError("%s() :: " fmt, __func__, ##__VA_ARGS__); } } while (0)
#define log_debug(fmt, ...)                                                 \
    do { if (0) { SvLogNotice(COLBEG() "%s() :: " fmt COLEND_COL(blue), __func__, ##__VA_ARGS__); } } while (0)

struct QBPVODStorage_ {
    struct SvObject_ super_;

    SvString vendor;
    SvString model;
};
typedef struct QBPVODStorage_ *QBPVODStorage;

struct QBPVODStorageService_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvWeakList listeners;

    SvString file;
    SvArray allowedDisks;

    // MSTORE
    struct MStore {
        SvString     devId;
        SvString     path;
        SvString     metaPartPath;
        bool         mounted;

        SvFiber      fiber;
        SvFiberTimer timer;
        SvFiberTimer fakeTimer;
        QBShellPipe  pipe;
    } mstore;
};

SvLocal void
storageConnected(SvWeakList listeners, SvString rootNode, bool forced)
{
    SvIterator it = SvWeakListIterator(listeners);
    SvGenericObject listener;
    while ((listener = SvIteratorGetNext(&it)))
        SvInvokeInterface(QBPVODStorageListener, listener, storageConnected, rootNode, forced);
}

SvLocal void
storageDisconnected(SvWeakList listeners)
{
    SvIterator it = SvWeakListIterator(listeners);
    SvGenericObject listener;
    while ((listener = SvIteratorGetNext(&it)))
        SvInvokeInterface(QBPVODStorageListener, listener, storageDisconnected);
}

// QBPVODStorage

SvLocal void
QBPVODStorageDestroy(void *self_)
{
    QBPVODStorage self = self_;

    SVRELEASE(self->vendor);
    SVRELEASE(self->model);
}

SvLocal SvType
QBPVODStorage_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBPVODStorageDestroy
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBPVODStorage",
                            sizeof(struct QBPVODStorage_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal QBPVODStorage
QBPVODStorageCreate(SvString vendor, SvString model, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBPVODStorage self = NULL;

    if (!vendor || !model) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed :: vendor = %s, model = %s",
                                  vendor ? SvStringCString(vendor) : NULL,
                                  model ? SvStringCString(model) : NULL);
        goto out;
    }

    self = (QBPVODStorage) SvTypeAllocateInstance(QBPVODStorage_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBPVODStorage");
        goto out;
    }

    self->vendor = SVRETAIN(vendor);
    self->model  = SVRETAIN(model);

out:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

// QBPVODStorageService

SvLocal void
QBPVODStorageServiceMountStep(void *self_)
{
    QBPVODStorageService self = self_;

    SvFiberDeactivate(self->mstore.fiber);

    if (SvFiberEventIsActive(self->mstore.timer)) {
        SvFiberEventDeactivate(self->mstore.timer);

        if (!self->mstore.pipe)
            goto out;

        int ret = QBShellPipeHasAsyncResult(self->mstore.pipe);
        if (ret == 0) {
            SvFiberTimerActivateAfter(self->mstore.timer, SvTimeFromMs(250));
        } else if (ret == 1) {
            int result = QBShellPipeGetAsyncResult(self->mstore.pipe);
            QBShellPipeClose(self->mstore.pipe);
            self->mstore.pipe = NULL;
            if (result < 0)
                log_error("QBShellPipegetAsyncResult() call failed");
            else
                storageConnected(self->listeners, self->mstore.path, false);
        } else {
            log_error("QBShellPipeHasAsyncResult() call failed.");
        }
    }

out:
    if (self->mstore.fakeTimer && SvFiberEventIsActive(self->mstore.fakeTimer)) {
        SvFiberEventDeactivate(self->mstore.fakeTimer);

        const char *mstoreFakeDir = env_fake_mstore_dir();
        self->mstore.path = SvStringCreate(mstoreFakeDir, NULL);
        storageConnected(self->listeners, self->mstore.path, true);
    }
}

// JSON serializer methods

SvLocal SvGenericObject
QBPVODStorageServiceReadDetails(SvGenericObject helper_,
                                SvString className,
                                SvHashTable desc,
                                SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    SvGenericObject result = NULL;

    SvValue vendorV = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("vendor"));
    SvValue modelV  = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("model"));

    SvString vendor = SvValueTryGetString(vendorV);
    SvString model  = SvValueTryGetString(modelV);

    if (!vendor || !model) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "NULL attribute found :: vendor = %s, model = %s",
                                  vendor ? SvStringCString(vendor) : NULL,
                                  model ? SvStringCString(model) : NULL);
        goto out;
    }

    result = (SvGenericObject) QBPVODStorageCreate(vendor, model, &error);
    if (!result)
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "can't create QBPVODStorage object");

out:
    SvErrorInfoPropagate(error, errorOut);
    return result;
}

// QBHotplugMountAgentListener virtual methods

SvLocal void
QBPVODStorageServicePartitionAdded(SvGenericObject self_,
                                   SvGenericObject path,
                                   QBDiskInfo disk,
                                   QBPartitionInfo part)
{
    QBPVODStorageService self = (QBPVODStorageService) self_;

    if (!QBPVODStorageServiceIsDiskAllowed(self, disk)) {
        log_debug("Partition from non pvod hdd");
        return;
    }

    log_debug("Partition from pvod hdd");

    log_debug("Added partition [%s : %s]", (part && part->partId) ? SvStringCString(part->partId) : NULL,
              (part && part->label) ? SvStringCString(part->label) : NULL);

    if (self->mstore.mounted)
        return;

    if (part && part->type && SvStringEqualToCString(part->type, "pvrmeta")) {
        SVTESTRETAIN(part->rootNode);
        SVTESTRELEASE(self->mstore.metaPartPath);
        self->mstore.metaPartPath = part->rootNode;

        log_debug("pvrmeta partition found");
    }

    if (part && part->label && SvStringEqualToCString(part->label, "MStoreVoD") &&
        QBPVODStorageServiceIsStorageSupported(self, SVSTRING("MStoreVoD"))) {

        SVTESTRETAIN(part->rootNode);
        SVTESTRELEASE(self->mstore.path);
        self->mstore.path = part->rootNode;

        SVTESTRETAIN(disk->devId);
        SVTESTRELEASE(self->mstore.devId);
        self->mstore.devId = disk->devId;

        log_debug("MStoreVoD partition found");
    }

    if (self->mstore.metaPartPath && self->mstore.path) {
        char* filename;
        char *buf;
        asprintf(&filename, "%s/.qb_partition_config", SvStringCString(self->mstore.metaPartPath));
        QBFileToBuffer(filename, &buf);

        if (!buf) {
            log_error("Can't read %s", filename);
            free(filename);
            return;
        }

        free(filename);
        log_debug("config :: %s", buf);

        if (strstr(buf, "label=MStoreVoD")) {
            log_debug("found MStoreVoD label");

            char cmd[32];
            snprintf(cmd, 32, "qb_mount_mstore %s", SvStringCString(disk->devId));
            if (self->mstore.pipe)
                QBShellPipeClose(self->mstore.pipe);
            self->mstore.pipe = QBShellPipeOpen(cmd, NULL);
            log_debug("Call `%s'", cmd);

            int sendStat = QBShellPipeRequestAsyncResult(self->mstore.pipe);
            if (sendStat != 0) {
                if (self->mstore.pipe)
                    QBShellPipeClose(self->mstore.pipe);
                log_error("QBShellPipeRequestAsyncResult() call failed");
            }

            if (self->mstore.timer)
                SvFiberTimerActivateAfter(self->mstore.timer, SvTimeFromMs(250));

            self->mstore.mounted = true;
        } else {
            log_error("Can't find MStoreVoD label in partition config file");
            return;
        }

        free(buf);
    }
}

SvLocal void
QBPVODStorageServicePartitionRemoved(SvGenericObject self_,
                                     SvGenericObject path,
                                     SvString remId,
                                     SvString diskId)
{
    QBPVODStorageService self = (QBPVODStorageService) self_;

    if (SvObjectEquals((SvObject) diskId, (SvObject) self->mstore.devId)) {
        log_debug("MStoreVoD partition removed");
        storageDisconnected(self->listeners);
        self->mstore.mounted = false;
        SVTESTRELEASE(self->mstore.metaPartPath);
        self->mstore.metaPartPath = NULL;
        SVTESTRELEASE(self->mstore.path);
        self->mstore.path = NULL;
        storageDisconnected(self->listeners);
    }
}

SvLocal void QBPVODStorageServiceDiskAdded(SvGenericObject self_, SvGenericObject path, QBDiskInfo disk)
{
}

SvLocal void QBPVODStorageServiceDiskRemoved(SvGenericObject self_, SvGenericObject path, SvString remId, SvString diskId)
{
}

// QBPVRMounterListener virtual methods

SvLocal void
QBPVODMounterResultReceived(SvGenericObject self_, QBDiskStatus disk)
{
    QBPVODStorageService self = (QBPVODStorageService) self_;
    if (disk->state == QBPVRMounterState_disk_formatting) {
        log_debug("MStoreVoD partition formatting");
        storageDisconnected(self->listeners);
        self->mstore.mounted = false;
        SVTESTRELEASE(self->mstore.metaPartPath);
        self->mstore.metaPartPath = NULL;
        SVTESTRELEASE(self->mstore.path);
        self->mstore.path = NULL;
        storageDisconnected(self->listeners);
    }
}

// ----------------------------------------------------------------------------

SvLocal void
QBPVODStorageServiceDestroy(void *self_)
{
    QBPVODStorageService self = self_;

    if (self->mstore.fiber)
        SvFiberDestroy(self->mstore.fiber);

    if (self->mstore.pipe)
        QBShellPipeClose(self->mstore.pipe);

    SVRELEASE(self->file);
    SVTESTRELEASE(self->allowedDisks);
    SVRELEASE(self->listeners);

    SVTESTRELEASE(self->mstore.devId);
    SVTESTRELEASE(self->mstore.path);
    SVTESTRELEASE(self->mstore.metaPartPath);
}

SvLocal SvType
QBPVODStorageService_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBPVODStorageServiceDestroy
    };
    static SvType type = NULL;

    if (!type) {
        static struct QBHotplugMountAgentListener_t hotplugMethods = {
            .partitionAdded   = QBPVODStorageServicePartitionAdded,
            .partitionRemoved = QBPVODStorageServicePartitionRemoved,
            .diskAdded        = QBPVODStorageServiceDiskAdded,
            .diskRemoved      = QBPVODStorageServiceDiskRemoved,
        };

        static struct QBPVRMounterListener_t mounterMethods = {
            .mounterResultReceived = QBPVODMounterResultReceived,
        };

        SvTypeCreateManaged("QBPVODStorageService",
                            sizeof(struct QBPVODStorageService_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBHotplugMountAgentListener_getInterface(), &hotplugMethods,
                            QBPVRMounterListener_getInterface(), &mounterMethods,
                            NULL);
    }

    return type;
}

QBPVODStorageService
QBPVODStorageServiceCreate(AppGlobals appGlobals, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBPVODStorageService self = NULL;

    if (!appGlobals) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed :: appGlobals = NULL");
        goto out;
    }

    self = (QBPVODStorageService) SvTypeAllocateInstance(QBPVODStorageService_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBPVODStorageService");
        goto out;
    }

    self->appGlobals = appGlobals;

    self->listeners = SvWeakListCreate(&error);
    if (!self->listeners) {
        SVRELEASE(self);
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error, "can't create SvWeakList");
        goto out;
    }

    self->file = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/pvodStorage", SvGetRuntimePrefix());

    SvJSONClass helper = NULL;
    helper = SvJSONClassCreate(NULL, QBPVODStorageServiceReadDetails, &error);
    if (!helper) {
        SVRELEASE(self);
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error, "can't create SvJSONClass");
        goto out;
    }

    SvJSONRegisterClassHelper(SVSTRING("QBPVODStorage"), (SvGenericObject) helper, &error);
    if (error) {
        SVRELEASE(self);
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error, "can't register JSON class helper");
    }
    SVRELEASE(helper);

    self->mstore.fiber = SvFiberCreate(appGlobals->scheduler, NULL, "MStore serivce mounter",
                                       QBPVODStorageServiceMountStep, self);
    self->mstore.timer = SvFiberTimerCreate(self->mstore.fiber);
    self->mstore.devId = NULL;
    self->mstore.mounted = false;


out:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

void
QBPVODStorageServiceStart(QBPVODStorageService self)
{
    char *buffer = NULL;
    QBFileToBuffer(SvStringCString(self->file), &buffer);

    if (buffer) {
        SvErrorInfo error = NULL;
        SvArray allowedDisks = (SvArray) SvJSONParseString(buffer, true, &error);
        free(buffer);
        if (error) {
            SvLogError("%s() : can't parse json:", __func__);
            SvErrorInfoWriteLogMessage(error);
        }
        if (allowedDisks && SvObjectIsInstanceOf((SvObject) allowedDisks, SvArray_getType())) {
            bool correct = true;
            SvIterator it = SvArrayIterator(allowedDisks);
            QBPVODStorage storage;
            while ((storage = (QBPVODStorage) SvIteratorGetNext(&it))) {
                if (!SvObjectIsInstanceOf((SvObject) storage, QBPVODStorage_getType())) {
                    correct = false;
                    break;
                }
            }
            if (correct) {
                self->allowedDisks = allowedDisks;
            } else {
                SVRELEASE(allowedDisks);
            }
        } else {
            SVTESTRELEASE(allowedDisks);
        }
    }

    QBHotplugMountAgentAddListener(self->appGlobals->hotplugMountAgent,
                                   (SvGenericObject) self);

    QBPVRMounterAddListener(self->appGlobals->PVRMounter,
                            (SvGenericObject) self);

    const char *mstoreFakeDir = env_fake_mstore_dir();
    if (mstoreFakeDir) {
        log_debug("MStore fake dir set to ``%s''", mstoreFakeDir);
        self->mstore.fakeTimer = SvFiberTimerCreate(self->mstore.fiber);
        SvFiberTimerActivateAfter(self->mstore.fakeTimer, SvTimeFromMs(5 * 1000));
    }
}

void
QBPVODStorageServiceStop(QBPVODStorageService self)
{
    QBPVRMounterRemoveListener(self->appGlobals->PVRMounter,
                            (SvGenericObject) self);
    QBHotplugMountAgentRemoveListener(self->appGlobals->hotplugMountAgent,
                                      (SvGenericObject) self);
}

void
QBPVODStorageDisconnect(QBPVODStorageService self)
{
    storageDisconnected(self->listeners);
}

void
QBPVODStorageReconnect(QBPVODStorageService self)
{
    storageConnected(self->listeners, self->mstore.path, false);
}

bool
QBPVODStorageServiceIsDiskAllowed(QBPVODStorageService self,
                                  QBDiskInfo diskInfo)
{
    if (!self->allowedDisks)
        return true;

    SvIterator it = SvArrayIterator(self->allowedDisks);
    QBPVODStorage storage = NULL;
    while ((storage = (QBPVODStorage) SvIteratorGetNext(&it))) {
        if (SvObjectEquals((SvObject) storage->vendor, (SvObject) diskInfo->vendor) && SvObjectEquals((SvObject) storage->model, (SvObject) diskInfo->model))
            return true;
    }
    return false;
}

SvString
QBPVODStorageServiceGetStoragePath(QBPVODStorageService self, SvString storage)
{
    if (SvStringEqualToCString(storage, "MStoreVoD"))
        return self->mstore.path;
    else
        return NULL;
}

void
QBPVODStorageServiceAddListener(QBPVODStorageService self,
                                SvGenericObject listener)
{
    SvWeakListPushFront(self->listeners, listener, NULL);
}

void
QBPVODStorageServiceRemoveListener(QBPVODStorageService self,
                                   SvGenericObject listener)
{
    SvWeakListRemoveObject(self->listeners, listener);
}

SvInterface
QBPVODStorageListener_getInterface(void)
{
    static SvInterface interface = NULL;
    if (!interface) {
        SvInterfaceCreateManaged("QBPVODStorageListener",
                                 sizeof(struct QBPVODStorageListener_),
                                 NULL, &interface, NULL);
    }

    return interface;
}

bool
QBPVODStorageServiceIsStorageSupported(QBPVODStorageService self,
                                       SvString storage)
{
    if (SvStringEqualToCString(storage, "MStoreVoD")) {
        return !PMIsNull || !PMIsNull();
    }

    return false;
}

