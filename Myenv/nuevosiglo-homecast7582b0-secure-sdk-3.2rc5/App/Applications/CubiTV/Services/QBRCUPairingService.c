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

#include "QBRCUPairingService.h"
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBAsyncService.h>
#include <QBAppKit/QBEventBus.h>
#include <ZRCProfileHAL.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvArray.h>
#include <stdbool.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBRCUPairingService"
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, moduleName "Debug", "");
    #define log_error(fmt, ...) do { if (env_log_level() > 0) { SvLogError(COLBEG() moduleName " ::" fmt COLEND_COL(red), ## __VA_ARGS__); } } while (0)
    #define log_warning(fmt, ...) do { if (env_log_level() > 0) { SvLogWarning(COLBEG() moduleName ":: " fmt COLEND_COL(yellow), ## __VA_ARGS__); } } while (0)
    #define log_state(fmt, ...) do { if (env_log_level() >= 1) { SvLogNotice(COLBEG() moduleName ":: " fmt COLEND_COL(blue), ## __VA_ARGS__); } } while (0)
    #define log_debug(fmt, ...) do { if (env_log_level() >= 2) { SvLogNotice(COLBEG() moduleName ":: " fmt COLEND_COL(green), ## __VA_ARGS__); } } while (0)
#else
    #define log_error(fmt, ...)
    #define log_warning(fmt, ...)
    #define log_state(fmt, ...)
    #define log_debug(fmt, ...)
#endif

struct QBRCUPairingService_ {
    struct SvObject_ super_;
    SvArray pairedDevicesList;
    int attemptsLeft;
    QBAsyncServiceState serviceState;
};

struct QBRCUPairingServiceEvent_ {
    struct QBPeerEvent_ super_;
    QBRCUPairingServiceEventType type;
};

SvLocal void
QBRCUPairingServiceDevicePairedCallback(void *callbackParameter, ZRCProfileHALDevice dev);

SvLocal SvString
QBRCUPairingServiceGetName(SvObject self_)
{
    return SVSTRING("QBRCUPairingService");
}

SvLocal SvImmutableArray
QBRCUPairingServiceGetDependencies(SvObject self_)
{
    return NULL;
}

SvLocal QBAsyncServiceState
QBRCUPairingServiceGetState(SvObject self_)
{
    QBRCUPairingService self = (QBRCUPairingService) self_;
    return self->serviceState;
}

SvLocal void
QBRCUPairingServiceStart(SvObject self_,
                         SvScheduler scheduler,
                         SvErrorInfo *errorOut)
{
    log_debug();

    QBRCUPairingService self = (QBRCUPairingService) self_;
    SvErrorInfo error = NULL;

    if (unlikely(self->serviceState == QBAsyncServiceState_running)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBRCUPairingService service already started.");
        goto out;
    }

    if (unlikely(ZRCProfileHALInit() < 0)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "RCU HAL Init failed");
        goto out;
    }

    log_state("Starting QBRCUPairingService!");
    self->serviceState = QBAsyncServiceState_running;

out:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBRCUPairingServiceStop(SvObject self_,
                        SvErrorInfo *errorOut)
{
    log_debug();

    QBRCUPairingService self = (QBRCUPairingService) self_;
    SvErrorInfo error = NULL;

    if (unlikely(self->serviceState != QBAsyncServiceState_running)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBRCUPairingService service is not running");
        goto out;
    }

    if (unlikely(ZRCProfileHALDeinit() < 0)) {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "RCU HAL Deinit failed");
        goto out;
    }

    log_state("Stopping QBRCUPairingService...");
    self->serviceState = QBAsyncServiceState_idle;
out:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBRCUPairingServiceDestroy(void *self_)
{
    QBRCUPairingService self = (QBRCUPairingService) self_;
    SVTESTRELEASE(self->pairedDevicesList);
}

SvType
QBRCUPairingService_getType(void)
{
    static const struct SvObjectVTable_ objectVT = {
        .destroy = QBRCUPairingServiceDestroy,
    };
    static struct QBAsyncService_ asyncServiceMethods = {
        .getName         = QBRCUPairingServiceGetName,
        .getDependencies = QBRCUPairingServiceGetDependencies,
        .getState        = QBRCUPairingServiceGetState,
        .start           = QBRCUPairingServiceStart,
        .stop            = QBRCUPairingServiceStop,
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRCUPairingService", sizeof(struct QBRCUPairingService_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVT,
                            QBAsyncService_getInterface(), &asyncServiceMethods,
                            NULL);
    }

    return type;
}

QBRCUPairingService
QBRCUPairingServiceCreate(void)
{
    QBRCUPairingService self = (QBRCUPairingService) SvTypeAllocateInstance(QBRCUPairingService_getType(), NULL);
    return self;
}

SvType
QBRCUPairingServiceEvent_getType(void)
{
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRCUPairingServiceEvent", sizeof(struct QBRCUPairingServiceEvent_),
                            QBPeerEvent_getType(), &type,
                            NULL);
    }

    return type;
}

SvLocal QBRCUPairingServiceEvent
QBRCUPairingServiceEventCreate(QBRCUPairingServiceEventType type)
{
    QBRCUPairingServiceEvent self = (QBRCUPairingServiceEvent) SvTypeAllocateInstance(QBRCUPairingServiceEvent_getType(), NULL);
    self->type = type;
    return self;
}

QBRCUPairingServiceEventType
QBRCUPairingServiceEventGetType(QBRCUPairingServiceEvent self)
{
    return self->type;
}

SvLocal void
QBRCUPairingServiceLogDeviceInfo(ZRCProfileHALDevice dev)
{
    log_debug("dev = %p", dev);
    if (!dev) {
        return;
    }

    char physicalDeviceAddress[24];
    char *p = physicalDeviceAddress;
    for (unsigned i = 0; i < 8; ++i) {
        p += snprintf(p, 4, i < 7 ? "%02x:" : "%02x",
                      dev->address[i]);
    }
    log_debug("dev->address = %s", physicalDeviceAddress);
    log_debug("dev->devId = %d", dev->devId);
    log_debug("dev->vendorId = %d", dev->vendorId);
}

SvLocal void
QBRCUPairingServiceSendNotification(QBRCUPairingService self, QBRCUPairingServiceEventType type)
{
    log_state("Sending notification:");
    QBServiceRegistry sr = QBServiceRegistryGetInstance();
    QBEventBus eb = (QBEventBus) QBServiceRegistryGetService(sr, SVSTRING("EventBus"));
    QBRCUPairingServiceEvent event = QBRCUPairingServiceEventCreate(type);
    QBEventBusPostEvent(eb, (QBPeerEvent) event, (SvObject) self, NULL);
    SVRELEASE(event);
}

SvLocal int
QBRCUPairingServiceRestartPairing(QBRCUPairingService self)
{
    if (0 == self->attemptsLeft) {
        QBRCUPairingServiceSendNotification(self, QBRCUPairingServiceEventType_pairingFailed);
        return 0;
    }

    if (self->attemptsLeft > 0) {
        --self->attemptsLeft;
    }

    log_state("Restarting pairing.");

    int result = ZRCProfileHALStartPairing(QBRCUPairingServiceDevicePairedCallback, (void *) self);
    if (0 != result) {
        log_error("Unable to start pairing - result = [%d].", result);
        return -1;
    }

    QBRCUPairingServiceSendNotification(self, QBRCUPairingServiceEventType_pairingStarted);

    return 0;
}

SvLocal void
QBRCUPairingServiceDevicePairedCallback(void *callbackParameter, ZRCProfileHALDevice dev)
{
    log_debug();
    assert(callbackParameter);
    QBRCUPairingService self = (QBRCUPairingService) callbackParameter;

    if (NULL == dev) {
        log_error("dev is NULL.");
        QBRCUPairingServiceRestartPairing(self);
        return;
    }

    QBRCUPairingServiceLogDeviceInfo(dev);

    if (-1 == dev->devId) {
        log_state("no device paired");
        QBRCUPairingServiceRestartPairing(self);
        return;
    }

    QBRCUPairingServiceSendNotification(self, QBRCUPairingServiceEventType_paired);
}

int
QBRCUPairingServiceStartPairing(QBRCUPairingService self, const int maxAttempts)
{
    log_state("Starting pairing.");
    self->attemptsLeft = maxAttempts;
    return QBRCUPairingServiceRestartPairing(self);
}

int
QBRCUPairingServiceStopPairing(QBRCUPairingService self)
{
    log_state("Stopping pairng.");
    return 0;
}


bool
QBRCUPairingServiceIsRF4CEAvailable(QBRCUPairingService self)
{
    bool isAvailable = (0 == ZRCProfileHALProbe());
    log_state("RF4CE available: %s", isAvailable ? "yes" : "no");
    return isAvailable;
}

SvLocal bool
QBRCUPairingServiceIsAnyDevicePaired(QBRCUPairingService self, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!QBRCUPairingServiceIsRF4CEAvailable(self)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "RF4CE is unavailable");
        goto fini;
    }

    int result = ZRCProfileHALGetPairedDevices(&self->pairedDevicesList);
    if (0 != result) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Unable to get paired devices list");
        goto fini;
    }

    if (SvArrayGetCount(self->pairedDevicesList) > 0) {
        log_debug("Returning true as some devices already paired");
        return true;
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
    log_debug("Returning false as no devices found");
    return false;
}

bool
QBRCUPairingServiceIsPairingNeeded(QBRCUPairingService self, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    bool devicePaired = QBRCUPairingServiceIsAnyDevicePaired(self, &error);
    if (error) {
        goto fini;
    }

    return !devicePaired;

fini:
    SvErrorInfoPropagate(error, errorOut);
    return false;
}

void
QBRCUPairingServiceDropAllDevices(QBRCUPairingService self, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!QBRCUPairingServiceIsRF4CEAvailable(self)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "RF4CE is unavailable");
        goto fini;
    }

    int result = ZRCProfileHALGetPairedDevices(&self->pairedDevicesList);
    if (0 != result) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Unable to get paired devices list");
        goto fini;
    }

    SvIterator i = SvArrayGetIterator(self->pairedDevicesList);
    ZRCProfileHALDevice dev;
    while ((dev = (ZRCProfileHALDevice) SvIteratorGetNext(&i))) {
        result = ZRCProfileHALUnpairDevice(dev);
        if (0 != result) {
            log_error("Unable to unpair device %p.", dev);
        }
    }

    log_debug("Droped all RCU Devices.");
fini:
    SvErrorInfoPropagate(error, errorOut);
}
