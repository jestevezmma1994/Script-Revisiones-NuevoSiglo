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

#include <QBWindowContext.h>
#include <Windows/standby.h>
#include <main.h>
#include "dvbscan.h"

struct QBStandbyServiceDVBScan_ {
    struct SvObject_ super_;

    QBStandbyDVBScan bgScanner;

    QBWindowContext standby;        //not retained

    void *callbackData;
    QBStandbyDVBScanCallbacks callbacks;
};

SvLocal bool
QBStandbyServiceDVBScanPrepareParams(void *self_,
                                     QBStandbyDVBScan scan,
                                     QBDVBScannerParams* params)
{
    QBStandbyServiceDVBScan* self = self_;
    if (self->callbacks.prepareParams)
        return self->callbacks.prepareParams(self->callbackData, scan, params);
    else
        return true;
}

SvLocal void
QBStandbyServiceDVBScanOnScanStart(void *self_,
                                   QBStandbyDVBScan scan,
                                   QBDVBScanner* scanner)
{
    QBStandbyServiceDVBScan* self = self_;

    if (self->callbacks.onStart)
        self->callbacks.onStart(self->callbackData, scan, scanner);
}

SvLocal void
QBStandbyServiceDVBScanOnScanFinish(void *self_,
                                    QBStandbyDVBScan scan,
                                    QBDVBScanner* scanner,
                                    bool forced)
{
    QBStandbyServiceDVBScan* self = self_;

    if (!forced) {
        if (self->callbacks.onFinish)
            self->callbacks.onFinish(self->callbackData, scan, scanner, forced);

        SvInvokeInterface(QBStandbyContextService, self, stop);

    }
    QBStandbyContextServiceFinished(self->standby, (SvGenericObject) self, forced);
}

SvLocal void QBStandbyServiceDVBScanStop(SvGenericObject self_)
{
    QBStandbyServiceDVBScan* self = (QBStandbyServiceDVBScan*) self_;
    QBStandbyDVBScanStop(self->bgScanner);
}

SvLocal bool QBStandbyServiceDVBScanTryStart(SvGenericObject self_, QBWindowContext standby)
{
    QBStandbyServiceDVBScan* self = (QBStandbyServiceDVBScan*) self_;

    self->standby = standby;

    SvErrorInfo error = NULL;
    QBStandbyDVBScanStart(self->bgScanner, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        return false;
    }
    return true;
}

SvLocal void QBStandbyServiceDVBScan__dtor__(void *self_)
{
    QBStandbyServiceDVBScan* self = self_;
    QBStandbyDVBScanStop(self->bgScanner);
    SVRELEASE(self->bgScanner);
}

SvLocal SvType QBStandbyServiceDVBScan_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBStandbyServiceDVBScan__dtor__
    };
    static SvType type = NULL;
    static const struct QBStandbyContextService_ standbyServiceMethods = {
        .tryStart = QBStandbyServiceDVBScanTryStart,
        .stop = QBStandbyServiceDVBScanStop,
    };
    if (!type) {
        SvTypeCreateManaged("QBStandbyServiceDVBScan",
                            sizeof(struct QBStandbyServiceDVBScan_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBStandbyContextService_getInterface(), &standbyServiceMethods,
                            NULL);
    }

    return type;
}

QBStandbyServiceDVBScan* QBStandbyServiceDVBScanCreate(AppGlobals appGlobals)
{
    QBStandbyServiceDVBScan* self = (QBStandbyServiceDVBScan*) SvTypeAllocateInstance(QBStandbyServiceDVBScan_getType(), NULL);
    self->bgScanner = QBStandbyDVBScanCreate(appGlobals, QBTUNERRESERVER_PRIORITY_STANDBY, NULL);
    return self;
}

void QBStandbyServiceDVBScanSetCallbacks(QBStandbyServiceDVBScan* self, QBStandbyDVBScanCallbacks* callbacks, void *callbackData)
{
    self->callbacks = *callbacks;
    self->callbackData = callbackData;
    static struct QBStandbyDVBScanCallbacks_ cb = {
        .prepareParams = QBStandbyServiceDVBScanPrepareParams,
        .onFinish      = QBStandbyServiceDVBScanOnScanFinish,
        .onStart       = QBStandbyServiceDVBScanOnScanStart,
    };
    QBStandbyDVBScanSetCallbacks(self->bgScanner, &cb, self);
}

int QBStandbyServiceDVBScanGetTunerNum(QBStandbyServiceDVBScan* self)
{
    return QBStandbyDVBScanGetTunerNum(self->bgScanner);
}
