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


#include <QBSecureLogManager.h>
#include <sv_tuner.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvGenericObject.h>
#include "tunerLogs.h"

#define SIGNAL_STATUS_RAPORT_FREQ (1 * 60)
#define LOST_SIGNAL_RAPORT_FREQ (10 * 60)

struct QBTunerLogs_t {
    struct SvObject_ super_;
    bool stopped;
    SvScheduler scheduler;
    SvHashTable tunerStatus;
};
typedef struct QBTunerLogs_t* QBTunerLogs;

struct QBTunerLogsTunerStatus_t {
    struct SvObject_ super_;
    int tunerNum;
    struct QBTunerStatus status;
    SvFiber fiber;
    SvFiberTimer timer;

    SvTime lastLostSignalPush;
    SvTime lastSignalStatusPush;
};
typedef struct QBTunerLogsTunerStatus_t* QBTunerLogsTunerStatus;

static QBTunerLogs tunerLogs = NULL;

SvLocal void QBTunerLogsTunerStatus__dtor__(void *self_)
{
    QBTunerLogsTunerStatus self = self_;
    SvFiberDestroy(self->fiber);
}

SvLocal SvType QBTunerLogsTunerStatus_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTunerLogsTunerStatus__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTunerLogsTunerStatus",
                            sizeof(struct QBTunerLogsTunerStatus_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void QBTunerLogsReportTunerStatus(void *self_)
{
    QBTunerLogsTunerStatus self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);    //active iff status was not changed at all

    struct QBTunerStatus status = self->status;

    const char *modulationStr;
    switch(status.params.modulation) {
    case QBTunerModulation_QAM16 :
        modulationStr = "unknown";
        break;
    case QBTunerModulation_QAM32 :
        modulationStr = "QAM32";
        break;
    case QBTunerModulation_QAM64 :
        modulationStr = "QAM64";
        break;
    case QBTunerModulation_QAM128 :
        modulationStr = "QAM128";
        break;
    case QBTunerModulation_QAM256 :
        modulationStr = "QAM256";
        break;
    case QBTunerModulation_QPSK :
        modulationStr = "QPSK";
        break;
    case QBTunerModulation_8PSK :
    default :
        modulationStr = "8PSK";
        break;
    }

    if(!status.full_lock && status.should_have_lock && SvTimeToMs(SvTimeSub(SvTimeGet(), self->lastLostSignalPush)) > LOST_SIGNAL_RAPORT_FREQ * 1000) {
        QBSecureLogEvent("Tuner", "Notice.Tuner.Status", "JSON:{\"tuner\":{\"id\":%i,\"frequency\":%i,\"modulation\":\"%s\",\"symbol_rate\":%i,\"signal\":\"lost\"}}",
                self->tunerNum, status.params.mux_id.freq, modulationStr, status.params.symbol_rate);
        self->lastLostSignalPush = SvTimeGet();
    }

    if(status.full_lock && SvTimeToMs(SvTimeSub(SvTimeGet(), self->lastSignalStatusPush)) > SIGNAL_STATUS_RAPORT_FREQ * 1000) {
        QBSecureLogEvent("Tuner", "Notice.Tuner.Status", "JSON:{\"tuner\":{\"id\":%i,\"frequency\":%i,\"modulation\":\"%s\",\"symbol_rate\":%i,\"signal\":\"ok\",\"signalStrength\":%i,\"signalQuality\":%i,\"snr\":\"%f dB\",\"preBER\":\"%f\",\"postBER\":\"%f\",\"signalLevel\":\"%f dBm\",\"avgShortPostBER\":\"%g\",\"avgLongPostBER\":\"%g\"}}",
                self->tunerNum, status.params.mux_id.freq, modulationStr, status.params.symbol_rate, status.signal_strength, status.signal_quality,
status.snr, status.preber, status.postber, status.signal_level, status.short_term_error, status.long_term_error);
        self->lastSignalStatusPush = SvTimeGet();
    }


    int raportTime = SIGNAL_STATUS_RAPORT_FREQ;
    if(SIGNAL_STATUS_RAPORT_FREQ > LOST_SIGNAL_RAPORT_FREQ)
        raportTime = LOST_SIGNAL_RAPORT_FREQ;

    SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(raportTime * 1000 + 1));
}

SvLocal void QBTunerLogsTunerStateChanged(SvGenericObject self_, int tunerNum)
{
    QBTunerLogs self = (QBTunerLogs) self_;

    if(self->stopped)
        return;

    struct sv_tuner_state* tuner = sv_tuner_get(tunerNum);
    struct QBTunerStatus status;
    if(sv_tuner_get_status(tuner, &status))
        return;

    SVAUTOINTVALUE(tunerNumVal, tunerNum);

    QBTunerLogsTunerStatus tunerStatus = (QBTunerLogsTunerStatus) SvHashTableFind(self->tunerStatus, (SvGenericObject) tunerNumVal);

    if(!tunerStatus) {
        tunerStatus = (QBTunerLogsTunerStatus) SvTypeAllocateInstance(QBTunerLogsTunerStatus_getType(), NULL);
        SvValue vTunerNum = SvValueCreateWithInteger(tunerNum, NULL);
        SvHashTableInsert(self->tunerStatus, (SvObject) vTunerNum, (SvObject) tunerStatus);
        SVRELEASE(vTunerNum);
        SVRELEASE(tunerStatus);

        char *buf;
        asprintf(&buf, "tuner_logs=%i", tunerNum);
        tunerStatus->fiber = SvFiberCreate(self->scheduler, NULL, buf, QBTunerLogsReportTunerStatus, tunerStatus);
        tunerStatus->timer = SvFiberTimerCreate(tunerStatus->fiber);
        tunerStatus->tunerNum = tunerNum;
        free(buf);
    }

    if(abs(tunerStatus->status.signal_strength - status.signal_strength) > 5 ||
       abs(tunerStatus->status.signal_quality - status.signal_quality) > 5 ||
       tunerStatus->status.full_lock != status.full_lock ||
       tunerStatus->status.should_have_lock != status.should_have_lock) {

        tunerStatus->status = status;
        SvFiberActivate(tunerStatus->fiber);
    }
}

SvLocal void QBTunerLogs__dtor__(void *self_)
{
    QBTunerLogs self = self_;
    SVRELEASE(self->tunerStatus);
}

SvLocal SvType QBTunerLogs_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTunerLogs__dtor__
    };
    static SvType type = NULL;
    static const struct QBTunerMonitorListener_t methods = {
        .tunerStateChanged = QBTunerLogsTunerStateChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTunerLogs",
                            sizeof(struct QBTunerLogs_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBTunerMonitorListener_getInterface(), &methods,
                            NULL);
    }

    return type;
}

void QBTunerLogsCreate(SvScheduler scheduler, QBTunerMonitor tunerMonitor)
{
    if (!tunerMonitor)
        return;

    tunerLogs = (QBTunerLogs) SvTypeAllocateInstance(QBTunerLogs_getType(), NULL);
    tunerLogs->stopped = true;
    tunerLogs->tunerStatus = SvHashTableCreate(10, NULL);
    tunerLogs->scheduler = scheduler;
    QBTunerMonitorAddListener(tunerMonitor, (SvGenericObject) tunerLogs);
}

void QBTunerLogsStart(void)
{
    if (!tunerLogs)
        return;
    tunerLogs->stopped = false;
}

void QBTunerLogsStop(void)
{
    if (!tunerLogs)
        return;

    tunerLogs->stopped = true;
    SvHashTableRemoveAllObjects(tunerLogs->tunerStatus);
}

void QBTunerLogsDestroy(void)
{
    SVTESTRELEASE(tunerLogs);
    tunerLogs = NULL;
}
