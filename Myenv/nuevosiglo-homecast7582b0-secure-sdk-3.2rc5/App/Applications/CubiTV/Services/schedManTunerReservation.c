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

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvHashTable.h>
#include <SvPlayerKit/SvContentResources.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <tunerReserver.h>
#include <Services/diskPVRProvider/QBDiskPVRProvider.h>
#include <QBTuner.h>
#include <main.h>
#include "schedManTunerReservation.h"
#include <Logic/PVRLogic.h>

struct QBSchedManTunerReservation_s
{
    AppGlobals appGlobals;
    SvEPGManager epgManager;
    SvHashTable tunerParamsToResv;
    SvHashTable tunerParamsToTunerNum;
};

SvLocal int QBSchedManTunerReservation_extractEventInfo(void* self_, const QBSchedParams* params, QBSchedParamsExtracted* extracted);
SvLocal int QBSchedManTunerReservation_getCurrentEvent(void* self_, const SvString channelId, SvEPGEvent* out);
SvLocal int QBSchedManTunerReservation_startWatching(void* self_, QBSchedDesc* desc);
SvLocal int QBSchedManTunerReservation_obtainReleaseTuner(void* self_, struct QBTunerParams* tunerParams, bool reserve);
SvLocal int QBSchedManTunerReservation_addResources(void* self_, const QBSchedDesc* desc, SvContent content);
SvLocal int QBSchedManTunerReservation_getChannelCost(void* self_, int costType);
SvLocal int QBSchedManTunerReservation_stopUsing(void* self_, const QBSchedDesc* desc);

SvLocal void QBSchedManTunerReservation_tunerRevoked(void *self_, QBTunerReserver* reserver, QBTunerResv* resv);

static const QBDiskPVRProviderAppCallbacks QBSchedManTunerReservation_callbacks = {
    .extractEventInfo       = & QBSchedManTunerReservation_extractEventInfo,
    .getCurrentEvent        = & QBSchedManTunerReservation_getCurrentEvent,
    .startWatching          = & QBSchedManTunerReservation_startWatching,
    .reserveTuner           = & QBSchedManTunerReservation_obtainReleaseTuner,
    .addResources           = & QBSchedManTunerReservation_addResources,
    .getChannelCost         = & QBSchedManTunerReservation_getChannelCost,
    .stopUsing              = & QBSchedManTunerReservation_stopUsing,
};

static const QBTunerResvCallbacks QBTunerResv_callbacks = {
    .tunerRevoked = QBSchedManTunerReservation_tunerRevoked,
};

SvLocal int QBSchedManTunerReservation_extractEventInfo(void* self_, const QBSchedParams* params, QBSchedParamsExtracted* extracted)
{
    QBSchedManTunerReservation* self = self_;

    SvValue channelId = SvValueCreateWithString(params->channelId, NULL);
    SvTVChannel channel = SvEPGManagerFindTVChannel(self->epgManager, channelId, NULL);
    SVRELEASE(channelId);

    if (!channel)
        return -1;

    extracted->startTime = params->startTime;
    extracted->stopTime = params->stopTime;
    extracted->url = SVRETAIN(SvURLString(channel->sourceURL));
    extracted->channelName = SVRETAIN(channel->name);

    extracted->channelIsAdult = SVTESTRETAIN((SvValue) SvTVChannelGetAttribute(channel, SVSTRING("isAdult")));
    if (!extracted->channelIsAdult) {
        extracted->channelIsAdult = SvValueCreateWithBoolean(false, NULL);
    }
    return 0;
}

SvLocal int QBSchedManTunerReservation_getCurrentEvent(void* self_, const SvString channelId, SvEPGEvent* out)
{
    QBSchedManTunerReservation* self = self_;

    SvEPGManager man = self->epgManager;
    if (!man) {
        return -1;
    }

    SvValue channelValueId = SvValueCreateWithString(channelId, NULL);
    SvTVChannel channel = SvEPGManagerFindTVChannel(man, channelValueId, NULL);
    SVRELEASE(channelValueId);

    if (!channel) {
        return -1;
    }
    time_t now = SvTimeNow();
    SvEPGIterator it = SvEPGManagerCreateIterator(man, SvDBObjectGetID((SvDBObject) channel), now);
    SvEPGEvent event = SvEPGIteratorGetNextEvent(it);
    SVTESTRELEASE(it);
    if (!event) {
        return -1;
    }
    SvTimeRange timeRange;
    SvTimeRangeInit(&timeRange, 0, 0);
    SvEPGEventGetTimeRange(event, &timeRange, NULL);
    if (SvTimeRangeContainsTimePoint(&timeRange, now)) {
        *out = event;
        return 0;
    }
    return -1;
}

SvLocal int QBSchedManTunerReservation_startWatching(void* self_, QBSchedDesc* desc)
{
    return 0;
}

SvLocal int QBSchedManTunerReservation_obtainTuner(QBSchedManTunerReservation* self, struct QBTunerParams* tunerParams)
{
    int tunerNum = -1;
    QBTunerParamsObj* tunerParamsVal = QBTunerParamsObjCreate(tunerParams);
    QBTunerResv* resv = (QBTunerResv*) SvHashTableFind(self->tunerParamsToResv, (SvGenericObject) tunerParamsVal);
    if (resv) {
        SvLogWarning("QBSchedMan requested frequency %i,%i which it already has", tunerParams->mux_id.freq, tunerParams->mux_id.plp_id);
        goto fini;
    }
    resv = QBTunerResvCreate(SVSTRING(__func__), self, &QBTunerResv_callbacks);
    resv->params.priority = QBTUNERRESERVER_PRIORITY_PVR;
    resv->params.tunerParams = *tunerParams;
    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    tunerNum = QBTunerReserverTryObtain(tunerReserver, resv);
    if (tunerNum >= 0) {
        SvValue tunerNumVal = SvValueCreateWithInteger(tunerNum, NULL);
        SvHashTableInsert(self->tunerParamsToResv, (SvGenericObject) tunerParamsVal, (SvGenericObject) resv);
        SvHashTableInsert(self->tunerParamsToTunerNum, (SvGenericObject) tunerParamsVal, (SvGenericObject) tunerNumVal);
        SVRELEASE(tunerNumVal);
    }
    SVRELEASE(resv);

fini:
    SVRELEASE(tunerParamsVal);

    return tunerNum >= 0 ? 0 : -1;
}

SvLocal int QBSchedManTunerReservation_releaseTuner(QBSchedManTunerReservation* self, struct QBTunerParams* tunerParams)
{
    QBTunerParamsObj* tunerParamsVal = QBTunerParamsObjCreate(tunerParams);
    QBTunerResv* resv = (QBTunerResv*) SvHashTableFind(self->tunerParamsToResv, (SvGenericObject) tunerParamsVal);
    if (!resv) {
        SvLogWarning("QBSchedMan wanted to release frequency=%i,%i which it did not request", tunerParams->mux_id.freq, tunerParams->mux_id.plp_id);
        SVRELEASE(tunerParamsVal);
        return -1;
    }
    SVRETAIN(resv);
    SvHashTableRemove(self->tunerParamsToTunerNum, (SvGenericObject) tunerParamsVal);
    SvHashTableRemove(self->tunerParamsToResv, (SvGenericObject) tunerParamsVal);

    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    QBTunerReserverRelease(tunerReserver, resv);
    SVRELEASE(resv);
    SVRELEASE(tunerParamsVal);

    return 0;
}

SvLocal int QBSchedManTunerReservation_obtainReleaseTuner(void* self_, struct QBTunerParams* tunerParams, bool reserve)
{
    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    if (!tunerReserver)
        return -1;

    QBSchedManTunerReservation* self = self_;

    //sheesh
    if (reserve) {
        return QBSchedManTunerReservation_obtainTuner(self, tunerParams);
    } else {
        return QBSchedManTunerReservation_releaseTuner(self, tunerParams);
    }
}

SvLocal int QBSchedManTunerReservation_addResources(void* self_, const QBSchedDesc* desc, SvContent content)
{
    QBSchedManTunerReservation* self = self_;
    if (desc->extracted.tunerParams.mux_id.freq > 0)
    {
        QBTunerParamsObj* tunerParamsVal = QBTunerParamsObjCreate(&desc->extracted.tunerParams);
        SvValue tunerNumVal = (SvValue) SvHashTableFind(self->tunerParamsToTunerNum, (SvGenericObject) tunerParamsVal);
        SVRELEASE(tunerParamsVal);
        if (!tunerNumVal) {
            SvLogWarning("QBSchedMan seems to assume that it has access to tuner, when it has not");
            return -1;
        }
        SvContentMetaDataSetIntegerProperty(SvContentGetResources(content), SVSTRING(SV_CONTENT_RESOURCE__TUNER), SvValueGetInteger(tunerNumVal));
    }
    return 0;
}

SvLocal int QBSchedManTunerReservation_getChannelCost(void* self_, int channelPVRCostClass)
{
  QBSchedManTunerReservation* self = self_;
  return QBPVRLogicGetChannelCost(self->appGlobals->pvrLogic,
                                  (QBDiskPVRProviderChannelCostClass) channelPVRCostClass);
}

SvLocal int QBSchedManTunerReservation_stopUsing(void* self_, const QBSchedDesc* desc)
{
    QBSchedManTunerReservation* self = self_;
    if (!self->appGlobals->reencryptionService)
        return 0;

    return QBReencryptionServiceStopUsingRecord(self->appGlobals->reencryptionService, desc);
}

SvLocal void QBSchedManTunerReservation_tunerRevoked(void *self_, QBTunerReserver* reserver, QBTunerResv* resv)
{
    QBSchedManTunerReservation* self = self_;
    QBDiskPVRProviderUnreserveTuner((QBDiskPVRProvider) self->appGlobals->pvrProvider,
                                    &resv->params.tunerParams);
}

QBSchedManTunerReservation* QBSchedManTunerReservationCreate(AppGlobals appGlobals, SvEPGManager epgManager)
{
    QBSchedManTunerReservation* self = calloc(1, sizeof(struct QBSchedManTunerReservation_s));
    self->appGlobals = appGlobals;
    self->epgManager = SVRETAIN(epgManager);
    self->tunerParamsToResv = SvHashTableCreate(11, NULL);
    self->tunerParamsToTunerNum = SvHashTableCreate(11, NULL);
    QBDiskPVRProviderRegisterAppCallbacks((QBDiskPVRProvider) appGlobals->pvrProvider, self, &QBSchedManTunerReservation_callbacks);

    return self;
}

void QBSchedManTunerReservationDestroy(QBSchedManTunerReservation* self)
{
    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    if(!tunerReserver)
        goto fini;

    QBTunerReserverStartTransaction(tunerReserver);
    SvIterator it = SvHashTableKeysIterator(self->tunerParamsToResv);
    QBTunerParamsObj* tunerParamsVal;
    while ((tunerParamsVal = (QBTunerParamsObj*) SvIteratorGetNext(&it))) {
        QBTunerResv* resv = (QBTunerResv*) SvHashTableFind(self->tunerParamsToResv, (SvGenericObject) tunerParamsVal);
        QBDiskPVRProviderUnreserveTuner((QBDiskPVRProvider) self->appGlobals->pvrProvider,
                                        &tunerParamsVal->params);
        QBTunerReserverRelease(tunerReserver, resv);
    }
    QBTunerReserverEndTransaction(tunerReserver);
fini:
    SVRELEASE(self->tunerParamsToResv);
    SVRELEASE(self->tunerParamsToTunerNum);
    SVRELEASE(self->epgManager);
    free(self);
}
