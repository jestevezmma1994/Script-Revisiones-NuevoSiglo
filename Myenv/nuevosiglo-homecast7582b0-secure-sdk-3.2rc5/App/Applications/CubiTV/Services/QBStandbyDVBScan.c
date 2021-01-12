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

#include "QBStandbyDVBScan.h"
#include <QBDVBSatellitesDB.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLog.h>
#include <tunerReserver.h>
#include <Logic/ChannelScanningLogic.h>
#include <main.h>

struct QBStandbyDVBScan_ {
    struct SvObject_ super_;
    int tunerNum;
    QBDVBScanner* scanner;
    int priority;

    AppGlobals appGlobals;

    void *callbackData;
    QBStandbyDVBScanCallbacks callbacks;
};

SvLocal void
QBStandbyDVBScanOnScanFinish(void *self_, QBDVBScanner* scanner)
{
    QBStandbyDVBScan self = self_;
    if (self->callbacks.onFinish)
        self->callbacks.onFinish(self->callbackData, self, self->scanner, false);
    QBStandbyDVBScanStop(self);
}

SvLocal void
QBStandbyDVBScanOnScanRevoke(void *self_, QBDVBScanner* scanner)
{
    QBStandbyDVBScan self = self_;

    if (self->scanner) {
        QBDVBScannerDestroy(self->scanner);
        self->scanner = NULL;
        if (self->callbacks.onFinish)
            self->callbacks.onFinish(self->callbackData, self, self->scanner, true);
    }
}

void
QBStandbyDVBScanStop(QBStandbyDVBScan self)
{
    if (self->scanner) {
        QBDVBScannerStop(self->scanner);
        QBDVBScannerDestroy(self->scanner);
        self->scanner = NULL;
    }
    self->tunerNum = -1;
}

int
QBStandbyDVBScanStart(QBStandbyDVBScan self, SvErrorInfo *errorOut)
{
    return QBStandbyDVBScanStartWithTunerResvParam(self, NULL, errorOut);
}

int
QBStandbyDVBScanStartWithTunerResvParam(QBStandbyDVBScan self, const QBTunerResvParams *resvParams, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    SvLogNotice("Start standby background scanning");
    if (!self->appGlobals->epgPlugin.DVB.channelList) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "DVBEPGPlugin not available");
        goto err;
    }

    QBDVBScannerParams params = QBDVBScannerParamsGetEmpty();
    if (!self->callbacks.prepareParams || !self->callbacks.prepareParams(self->callbackData, self, &params)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "can't prepare params");
        goto err;
    }

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
        QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), NULL);

    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));

    params.target = self;
    params.onFinish = QBStandbyDVBScanOnScanFinish;
    params.onRevoke = QBStandbyDVBScanOnScanRevoke;
    params.satellitesDB = satellitesDB;
    params.reserver = tunerReserver;
    params.priority = self->priority;
    params.tunerNum = -1;

    self->scanner = QBDVBScannerCreate(self->appGlobals->scheduler, NULL);
    if (self->callbacks.onStart) {
        self->callbacks.onStart(self->callbackData, self, self->scanner);
    }
    QBChannelScanningLogicSetDVBScannerPlugins(self->appGlobals->channelScanningLogic, self->scanner);
    QBDVBScannerStartWithTunerResvParam(self->scanner, &params, resvParams);

    return 0;

err:
    SvErrorInfoPropagate(error, errorOut);
    return -1;
}

SvLocal void QBStandbyDVBScan__dtor__(void *self_)
{
    QBStandbyDVBScan self = self_;
    QBStandbyDVBScanStop(self);
}

SvLocal SvType QBStandbyDVBScan_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBStandbyDVBScan__dtor__
    };
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("QBStandbyDVBScan",
                            sizeof(struct QBStandbyDVBScan_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBStandbyDVBScan
QBStandbyDVBScanCreate(AppGlobals appGlobals,
                          int priority,
                          SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBStandbyDVBScan self = (QBStandbyDVBScan)
        SvTypeAllocateInstance(QBStandbyDVBScan_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBStandbyDVBScan");
        goto fini;
    }

    self->appGlobals = appGlobals;
    self->priority = priority;

fini:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

void
QBStandbyDVBScanSetCallbacks(QBStandbyDVBScan self,
                                QBStandbyDVBScanCallbacks* callbacks,
                                void *callbackData)
{
    self->callbacks = *callbacks;
    self->callbackData = callbackData;
}

int
QBStandbyDVBScanGetTunerNum(QBStandbyDVBScan self)
{
    return self->tunerNum;
}
