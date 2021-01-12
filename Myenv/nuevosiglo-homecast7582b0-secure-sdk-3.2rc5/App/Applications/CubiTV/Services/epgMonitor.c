/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#include "epgMonitor.h"
#include "../main.h"

#include <assert.h>
#include <SvCore/SvEnv.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvDeque.h>
#include <sv_tuner.h>
#include <QBTuner.h>
#include <QBTunerInfo.h>
#include <QBTunerTypes.h>
#include <SvEPGDataLayer/Plugins/DVBEPGChannel.h>
#include <SvEPGDataLayer/SvChannelMerger.h>
#include <SvEPGDataLayer/SvEPGChannelListListener.h>
#include <SvCore/SvCoreErrorDomain.h>

#define CHANNEL_ID(channel)  (SvStringCString(SvValueGetString(((SvDBObject)tvChannel)->id)))

#define STEP_ONE_TUNER_MAX_LOOPS 10

#define log_state(fmt, ...) do { if (0) SvLogNotice("epgMonitor : " fmt, ##__VA_ARGS__); } while (0)
#define log_info(fmt, ...)  do { if (0) SvLogNotice("epgMonitor : " fmt, ##__VA_ARGS__); } while (0)
SV_DECL_INT_ENV_FUN_DEFAULT(envQBEPGMonitorPrint, 0, "QBEPGMonitorPrint", "0: not debug logs, 1: print debug logs")

struct MuxState {
    struct SvObject_ super_;

    struct QBTunerMuxId muxid;

    /// try to tune according to each channel's tune params, until lock is achieved
    SvArray channels;
};

static void MuxStateDestroy(void* self_)
{
    struct MuxState* self = (struct MuxState*) self_;
    SVTESTRELEASE(self->channels);
}

static unsigned int MuxStateHash(void* self_)
{
    struct MuxState* self = (struct MuxState*) self_;
    return self->muxid.freq + self->muxid.plp_id;
}

static bool MuxStateEquals(void* a_, void* b_)
{
    struct MuxState* a = (struct MuxState*) a_;
    struct MuxState* b = (struct MuxState*) b_;
    return QBTunerMuxIdEqual(&a->muxid, &b->muxid);
}

static SvType MuxState_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = MuxStateDestroy,
        .equals  = MuxStateEquals,
        .hash    = MuxStateHash
    };
    static SvType myType = NULL;
    if (!myType) {
        SvTypeCreateManaged("MuxState",
                            sizeof(struct MuxState),
                            SvObject_getType(),
                            &myType,
                            SvObject_getType(), &objectVTable,
                            NULL);
    };
    return myType;
}

struct TunerState
{
    int workerId;
    int tunerId;
    QBTunerResv* resv;

    struct MuxState* mux;

    int nextChannelIdx;   /// to iterate through "mux->channels"
    bool tuned;           /// tuner was ordered to tune to desired frequency
    bool locked;
    SvTime lockStartTime;
};

struct QBEPGMonitor_s {
    struct SvObject_ super_;

    SvGenericObject channelMerger;
    QBTunerReserver* reserver;
    int readingDuration;        /// in seconds

    int freqHandledCnt;         /// how many frequencies were handled

    int tunerCnt;
    SvArray muxes;
    struct TunerState* tuners;

    SvHashTable channelRevMap;  /// map: DVBEPGChannel -> MuxState
    SvHashTable muxHT;          /// set: MuxState
    struct MuxState* muxHTKey;  /// for searching in "muxHT"

    SvDeque muxFifo;

    SvFiber fiber;
    SvFiberTimer timer;
};

SvLocal void QBEPGMonitor_print(const QBEPGMonitor* mon, const char* msg)
{
#if SV_LOG_LEVEL > 0
    if (envQBEPGMonitorPrint()) {
        SvLogNotice("QBEPGMonitor [%s]", msg);

        SvLogNotice("    tuners=%d, fifo=%d, ht=%d",
                mon->tunerCnt, (int)SvDequeCount(mon->muxFifo), (int)SvHashTableCount(mon->muxHT));

        size_t i;
        for (i = 0; i < SvDequeCount(mon->muxFifo); i++) {
            const struct MuxState* mux = (const struct MuxState*) SvDequeAt(mon->muxFifo, i);
            SvLogNotice("      -> freq=%d, plp_id=%d (%p)", mux->muxid.freq, mux->muxid.plp_id, mux);
        }
    }
#endif
}

SvLocal void QBEPGMonitor_finish_mux(QBEPGMonitor* mon, struct TunerState* tuner, bool more, bool handled)
{
    if (more) {
        if (handled) {
            SvDequePushBack(mon->muxFifo, (SvGenericObject)tuner->mux, NULL);
            mon->freqHandledCnt += 1;
        } else {
            SvDequePushFront(mon->muxFifo, (SvGenericObject)tuner->mux, NULL);
        }
    }
    if (tuner->tunerId >= 0) {
        QBTunerReserverRelease(mon->reserver, tuner->resv);
    }
    tuner->tunerId = -1;

    SVRELEASE(tuner->mux);
    tuner->mux = NULL;

    tuner->tuned = false;
}

static struct TunerState* find_tuner_from_resv(const QBEPGMonitor* mon, QBTunerResv* resv)
{
    int i;
    for (i = 0; i < mon->tunerCnt; ++i) {
        if (mon->tuners[i].resv == resv)
            return &mon->tuners[i];
    }

    // should not happen
    SvLogError("find_tuner_from_resv() -> not found ERROR ERROR");
    return NULL;
}

SvLocal void QBEPGMonitor_tunerReserverRevoked(void* mon_, QBTunerReserver* reserver, QBTunerResv* resv)
{
    QBEPGMonitor* mon = (QBEPGMonitor*) mon_;
    struct TunerState* tuner = find_tuner_from_resv(mon, resv);

    log_info("Worker[%d] : Revoked tuner[%d]", tuner->workerId, tuner->tunerId);

    if (tuner->mux) {
        QBEPGMonitor_finish_mux(mon, tuner, true, false);
    }

    tuner->tunerId = -1;
}

static struct QBTunerResvCallbacks_s  g_tunerReserverCallbacks = {
    .tunerRevoked  = &QBEPGMonitor_tunerReserverRevoked,
    .tunerObtained = NULL,
};


static struct MuxState* find_mux_state_from_muxid(const QBEPGMonitor* mon, const struct QBTunerMuxId* muxid)
{
    mon->muxHTKey->muxid = *muxid;

    return (struct MuxState*) SvHashTableFind(mon->muxHT, (SvGenericObject)mon->muxHTKey);
}

static struct MuxState* alloc_new_mux_state(QBEPGMonitor* mon, const struct QBTunerMuxId* muxid)
{
    struct MuxState* state = (struct MuxState*) SvTypeAllocateInstance(MuxState_getType(), NULL);
    state->muxid = *muxid;
    state->channels = SvArrayCreateWithCapacity(10, NULL);

    SvHashTableInsert(mon->muxHT, (SvGenericObject)state, (SvGenericObject)state);
    QBEPGMonitor_print(mon, "push_back() ...");
    SvDequePushBack(mon->muxFifo, (SvGenericObject)state, NULL);
    QBEPGMonitor_print(mon, "push_back() ...");
    SVRELEASE(state);
    return state;
}

static struct MuxState* find_mux_state_from_channel(const QBEPGMonitor* mon, SvTVChannel tvChannel)
{
    return (struct MuxState*) SvHashTableFind(mon->channelRevMap, (SvGenericObject)tvChannel);
}

SvLocal const struct QBTunerParams* QBEPGMonitorGetTunerParams(SvTVChannel tvChannel)
{
    if (SvObjectIsInstanceOf((SvObject) tvChannel, DVBEPGChannel_getType())) {
        return &((DVBEPGChannel) tvChannel)->params;
    }

    SvString urlString = SvURLString(tvChannel->sourceURL);
    if (strncmp("dvb://", SvStringCString(urlString), 6) != 0)
        return NULL;

    static struct QBTunerParams tunerParams = {
        .mux_id = {
            .freq = 0,
        },
    };
    if (QBTunerParamsFromString(SvStringCString(urlString) + 6, &tunerParams) < 0)
        return NULL;

    return &tunerParams;
}

SvLocal bool QBEPGMonitorIsMuxAllowed(QBEPGMonitor* mon, const struct QBTunerMuxId* mux)
{
    bool isMuxAllowed = false;
    QBTunerMuxIdObj* muxObj;
    SvIterator it = SvArrayGetIterator(mon->muxes);
    while ((muxObj = (QBTunerMuxIdObj *) SvIteratorGetNext(&it))) {
        if (QBTunerMuxIdEqual(&muxObj->mux_id, mux)) {
            isMuxAllowed = true;
            break;
        }
    }
    return isMuxAllowed;
}

SvLocal void QBEPGMonitor_channel_found(SvGenericObject self, SvTVChannel tvChannel)
{
    QBEPGMonitor* mon = (QBEPGMonitor*)self;

    const struct QBTunerParams* params = QBEPGMonitorGetTunerParams(tvChannel);
    if (!params)
        return;

    if (tvChannel->isDead) {
        log_state("Channel found [%s] - dead!", CHANNEL_ID(tvChannel));
        return;
    }

    if (SvObjectIsInstanceOf((SvObject) tvChannel, DVBEPGChannel_getType())) {
        DVBEPGChannel channel = (DVBEPGChannel) tvChannel;
        if (channel->type == QBDVBChannelType_unknown || channel->type == QBDVBChannelType_Data) {
            log_state("Channel found [%s] - type=%d, ignored", CHANNEL_ID(channel), channel->type);
            return;
        }
    }

    struct MuxState* mux = find_mux_state_from_muxid(mon, &params->mux_id);
    log_state("Channel found [%s] : muxid=[%d,%d] - %s",
              CHANNEL_ID(tvChannel),
              params->mux_id.freq, params->mux_id.plp_id,
              mux ? "old" : "new");

    if (mon->muxes) {
        if (!QBEPGMonitorIsMuxAllowed(mon, &params->mux_id)) {
            log_state("Omitting channel [%s] - mux not allowed", CHANNEL_ID(channel));
            return;
        }
    }
    if (!mux) {
        mux = alloc_new_mux_state(mon, &params->mux_id);
    }
    SvArrayAddObject(mux->channels, (SvGenericObject) tvChannel);
    SvHashTableInsert(mon->channelRevMap, (SvGenericObject) tvChannel, (SvGenericObject) mux);
}

SvLocal void QBEPGMonitor_channel_lost(SvGenericObject self, SvTVChannel tvChannel)
{
    QBEPGMonitor* mon = (QBEPGMonitor*)self;

    const struct QBTunerParams* params = QBEPGMonitorGetTunerParams(tvChannel);

    int freq = -1;
    int plp_id = -1;

    if (params) {
        freq = params->mux_id.freq;
        plp_id = params->mux_id.plp_id;
    }

    struct MuxState* mux= find_mux_state_from_channel(mon, tvChannel);
    if (!mux) {
        log_state("Channel lost [%s] : freq=%d, plp_id=%d - not found", CHANNEL_ID(tvChannel), freq, plp_id);
        return;
    }

    log_state("Channel lost [%s] : freq=%d,plp_id=%d (old=%d,%d)", CHANNEL_ID(tvChannel), freq, plp_id, mux->muxid.freq, mux->muxid.plp_id);
    QBEPGMonitor_print(mon, "");

    int idx = SvArrayIndexOfObjectIdenticalTo(mux->channels, (SvGenericObject)tvChannel);

    SvArrayRemoveObjectAtIndex(mux->channels, idx);
    SvHashTableRemove(mon->channelRevMap, (SvGenericObject)tvChannel);

    struct TunerState* tuner = NULL;
    int i;
    for (i = 0; i < mon->tunerCnt; ++i) {
        tuner = &mon->tuners[i];
        if (tuner->mux == mux) {
            if (idx < tuner->nextChannelIdx) {
                tuner->nextChannelIdx -= 1;
            }
            break;
        };
        tuner = NULL;
    }

    if (SvArrayCount(mux->channels) > 0)
        return;

    if (tuner) {
        QBEPGMonitor_finish_mux(mon, tuner, false, false);
    } else {
        SvDequeRemove(mon->muxFifo, (SvGenericObject)mux);
    }

    log_info("             [%s] : freq=%d, plp_id=%d - removing freq=%d, plp_id=%d (%p)", CHANNEL_ID(tvChannel), freq, plp_id, mux->muxid.freq, mux->muxid.plp_id, mux);
    SvHashTableRemove(mon->muxHT, (SvGenericObject)mux);
    QBEPGMonitor_print(mon, "removed");
}

SvLocal void QBEPGMonitor_channelList_completed(SvGenericObject self, int pluginID)
{
}

SvLocal void QBEPGMonitor_channel_modified(SvGenericObject self, SvTVChannel tvChannel)
{
    QBEPGMonitor* mon = (QBEPGMonitor*)self;

    const struct QBTunerParams* params = QBEPGMonitorGetTunerParams(tvChannel);

    int freq = -1;
    int plp_id = -1;

    if (params) {
        freq = params->mux_id.freq;
        plp_id = params->mux_id.plp_id;
    }

    struct MuxState* mux = find_mux_state_from_channel(mon, tvChannel);
    log_state("Channel modified [%s] : muxid=[%d,%d]",
              CHANNEL_ID(tvChannel),
              freq, plp_id);

    log_info("Channel modified [%s] : freq=%d, plp_id=%d (old=%d,%d)", CHANNEL_ID(tvChannel), freq, plp_id, mux ? mux->muxid.freq : -1, mux ? mux->muxid.plp_id : -1);

    bool wasPresent = (mux != NULL);
    bool shouldBePresent = params && !tvChannel->isDead;
    if (wasPresent != shouldBePresent) {
        log_state("                 [%s] : freq=%d, plp_id=%d - was=%d, shouldBe=%d", CHANNEL_ID(tvChannel), freq, plp_id, wasPresent, shouldBePresent);
        if (wasPresent) {
            QBEPGMonitor_channel_lost(self, tvChannel);
        } else {
            QBEPGMonitor_channel_found(self, tvChannel);
        }
        return;
    }

    if ( (!mux) ||
         (params && QBTunerMuxIdEqual(&mux->muxid, &params->mux_id)) ) {
        log_state("                 [%s] : freq=%d, plp_id=%d - no change", CHANNEL_ID(tvChannel), freq, plp_id);
        return;
    }

    QBEPGMonitor_channel_lost(self, tvChannel);
    QBEPGMonitor_channel_found(self, tvChannel);
}

static void QBEPGMonitor__dtor__(void* mon_)
{
    QBEPGMonitor* mon = (QBEPGMonitor*) mon_;
    int i;
    for (i = 0; i < mon->tunerCnt; ++i) {
        struct TunerState* tuner = &mon->tuners[i];
        SVRELEASE(tuner->resv);
    }
    free(mon->tuners);
    SVTESTRELEASE(mon->muxes);
    SVRELEASE(mon->channelRevMap);
    SVRELEASE(mon->muxFifo);
    SVRELEASE(mon->muxHT);
    SVRELEASE(mon->muxHTKey);
    SVRELEASE(mon->reserver);
    SVRELEASE(mon->channelMerger);
}

static SvType QBEPGMonitor_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBEPGMonitor__dtor__
    };
    static SvType myType = NULL;

    static const struct SvEPGChannelListListener_ listMethods = {
        .channelFound         = &QBEPGMonitor_channel_found,
        .channelModified      = &QBEPGMonitor_channel_modified,
        .channelLost          = &QBEPGMonitor_channel_lost,
        .channelListCompleted = &QBEPGMonitor_channelList_completed
    };

    if (!myType) {
        SvTypeCreateManaged("QBEPGMonitor",
                            sizeof(struct QBEPGMonitor_s),
                            SvObject_getType(),
                            &myType,
                            SvObject_getType(), &objectVTable,
                            SvEPGChannelListListener_getInterface(), &listMethods,
                            NULL);
    }
    return myType;
}

QBEPGMonitor* QBEPGMonitorCreate(SvGenericObject merger, QBTunerReserver* reserver, int tunerCnt)
{
    QBEPGMonitor* mon = (QBEPGMonitor*) SvTypeAllocateInstance(QBEPGMonitor_getType(), NULL);

    mon->channelMerger = SVRETAIN(merger);
    mon->reserver = SVRETAIN(reserver);

    mon->tunerCnt = tunerCnt;
    mon->tuners = (struct TunerState*) calloc(sizeof(struct TunerState), tunerCnt);
    int i;
    for (i = 0; i < tunerCnt; ++i) {
        struct TunerState* tuner = &mon->tuners[i];
        tuner->tunerId = -1;
        tuner->workerId = i;

        QBTunerResv* resv = QBTunerResvCreate(SVSTRING("QBEPGMonitor"), mon, &g_tunerReserverCallbacks);
        tuner->resv = resv;

        int prio = QBTUNERRESERVER_PRIORITY_EPG_MONITOR_BASE;
        prio += i * QBTUNERRESERVER_PRIORITY_EPG_MONITOR_JUMP;

        resv->params.priority = prio;
        resv->params.tunerParams = QBTunerGetEmptyParams();
    }
    mon->muxes = NULL;
    mon->muxFifo = SvDequeCreate(NULL);
    mon->channelRevMap = SvHashTableCreate(97, NULL);
    mon->muxHT = SvHashTableCreate(29, NULL);
    mon->muxHTKey = (struct MuxState*) SvTypeAllocateInstance(MuxState_getType(), NULL);

    return mon;
}

void QBEPGMonitorSetReadingDuration(QBEPGMonitor* mon, int seconds)
{
    if (!mon)
        return;

    log_state("SetReadingDuration(%d)", seconds);
    mon->readingDuration = seconds;
}


SvLocal void QBEPGMonitor_step_one_tuner(QBEPGMonitor* mon, struct TunerState* tuner, SvTime now)
{
    int maxLoops = STEP_ONE_TUNER_MAX_LOOPS;
    for (;; maxLoops--) {
        if (maxLoops <= 0) {
            return;
        }

        /// get a frequency to work on
        if (!tuner->mux) {
            QBEPGMonitor_print(mon, "pop_front() ...");
            tuner->mux = (struct MuxState *) SvDequeTakeFront(mon->muxFifo);
            if (!tuner->mux) {
                log_state("Worker[%d] : no frequency to scan", tuner->workerId);
                return; // nothing to do for now.
            }
            QBEPGMonitor_print(mon, "pop_front() - done");

            log_state("Worker[%d] : new frequency %d,%d (%p) (refcnt=%zd)",
                      tuner->workerId, tuner->mux->muxid.freq, tuner->mux->muxid.plp_id,
                      tuner->mux,
                      SvObjectGetRetainCount((SvGenericObject)tuner->mux));

            tuner->tuned = false;
            tuner->locked = false;
            tuner->nextChannelIdx = 0;
        }

        int channelCnt = (int)SvArrayCount(tuner->mux->channels);
        assert(channelCnt > 0);

        /// check reading timeout
        if (tuner->locked) {
            int sec = SvTimeGetSeconds(SvTimeSub(now, tuner->lockStartTime));
            if (sec < mon->readingDuration)
                return;
            log_state("Worker[%d] : finished reading freq %d, plp_id %d, tuner[%d]", tuner->workerId, tuner->mux->muxid.freq, tuner->mux->muxid.plp_id, tuner->tunerId);
            QBEPGMonitor_finish_mux(mon, tuner, true, true);
            continue;
        }

        /// try to get a lock for parameters from one channel

        /// check tuning status
        if (tuner->tuned) {
            struct QBTunerMuxId muxid = sv_tuner_get_curr_mux_id(tuner->tunerId);
            struct QBTunerStatus status;
            sv_tuner_get_status(sv_tuner_get(tuner->tunerId), &status);

            int freq = tuner->mux->muxid.freq;
            int plp_id = tuner->mux->muxid.plp_id;
            if (status.full_lock) {
                log_info("Worker[%d] : got lock on tuner[%d] on mux=[%d,%d]", tuner->workerId, tuner->tunerId, muxid.freq, muxid.plp_id);
                tuner->locked = true;
                tuner->lockStartTime = now;
                return; /// wait for reading timeout
            }

            if (!status.should_have_lock)
                return; /// wait a little longer for the lock

            log_info("Worker[%d] : got lock on tuner[%d] on mux=[%d,%d]", tuner->workerId, tuner->tunerId, muxid.freq, muxid.plp_id);
            /// no lock for those parameters ...
            tuner->tuned = false;

            /// ... try with params from next channel
            tuner->nextChannelIdx += 1;
            log_state("Worker[%d] :  next channel tuner[%d] for freq %d plp_id %d (%p) (idx=%d, cnt=%d)",
                      tuner->workerId, tuner->tunerId, freq, plp_id, tuner->mux,
                      tuner->nextChannelIdx, channelCnt);
        }

        /// no more params?
        if (tuner->nextChannelIdx >= channelCnt) {
            log_state("Worker[%d] : no more tuning params tuner[%d] for freq %d, plp_id %d (idx=%d, cnt=%d) - finishing",
                      tuner->workerId, tuner->tunerId, tuner->mux->muxid.freq, tuner->mux->muxid.plp_id,
                      tuner->nextChannelIdx, channelCnt);
            QBEPGMonitor_finish_mux(mon, tuner, true, true);
            continue;
        }

        /// start tuning with new params
        SvTVChannel tvChannel = (SvTVChannel) SvArrayAt(tuner->mux->channels, tuner->nextChannelIdx);

        const struct QBTunerParams* params = QBEPGMonitorGetTunerParams(tvChannel);
        if (!params)
            return;

        if ((tuner->tunerId >= 0) && !QBTunerParamsAreEqual(params, &tuner->resv->params.tunerParams)) {
            log_info("tuning parameters are going to change...");
            QBEPGMonitor_finish_mux(mon, tuner, true, false);
            continue;
        }

        tuner->resv->params.tunerParams = *params;
        tuner->tunerId = QBTunerReserverTryObtain(mon->reserver, tuner->resv);
        if (tuner->tunerId >= 0) {
            int res = sv_tuner_tune(tuner->tunerId, params);
            if (res == 0) {
                tuner->tuned = true;
                return; /// wait for lock
            }
        }

        if (tuner->tunerId >= 0) {
            log_state("Worker[%d] : tuning to freq %d, plp_id %d, tuner[%d] failed - skipping", tuner->workerId, tuner->mux->muxid.freq, tuner->mux->muxid.plp_id, tuner->tunerId);
        } else {
            log_state("Worker[%d] : did not get tuner for freq %d, plp_id %d - skipping", tuner->workerId, tuner->mux->muxid.freq, tuner->mux->muxid.plp_id);
        }

        // if we were unable to tune to freq because of reservation or invalid params check next frequency in next step
        QBEPGMonitor_finish_mux(mon, tuner, true, true);
    }
}

SvLocal void QBEPGMonitor_step(void* mon_)
{
    log_info("step() ...");
    QBEPGMonitor* mon = (QBEPGMonitor*)mon_;

    SvFiberDeactivate(mon->fiber);
    SvFiberTimerActivateAfter(mon->timer, SvTimeFromMs(1000));

    SvTime now = SvTimeGet();

    QBTunerReserverStartTransaction(mon->reserver);
    int i;
    for (i = 0; i < mon->tunerCnt; ++i) {
        struct TunerState* tuner = &mon->tuners[i];
        log_state("step() : worker[%d]", tuner->workerId);

        QBEPGMonitor_step_one_tuner(mon, tuner, now);
    }
    QBTunerReserverEndTransaction(mon->reserver);
}

void QBEPGMonitorStart(QBEPGMonitor* mon, SvScheduler scheduler)
{
    if (!mon)
        return;

    log_state("Start() ...");
    mon->fiber = SvFiberCreate(scheduler, NULL, "QBEPGMonitor", &QBEPGMonitor_step, mon);
    mon->timer = SvFiberTimerCreate(mon->fiber);

    SvFiberActivate(mon->fiber);

    SvInvokeInterface(SvChannelMerger, mon->channelMerger, addListener, (SvGenericObject) mon, NULL);
}

void QBEPGMonitorStop(QBEPGMonitor* mon)
{
    if (!mon)
        return;

    log_state("Stop() ...");
    if (!mon->fiber)
        return;
    SvFiberDestroy(mon->fiber);
    mon->fiber = NULL;
    mon->timer = NULL;

    int i;
    for (i = 0; i < mon->tunerCnt; ++i) {
        struct TunerState* tuner = &mon->tuners[i];
        if (tuner->mux)
            QBEPGMonitor_finish_mux(mon, tuner, true, true);
        tuner->tunerId = -1;
    }

    SvInvokeInterface(SvChannelMerger, mon->channelMerger, removeListener, (SvGenericObject) mon, NULL);
}

bool QBEPGMonitorIsCompleted(QBEPGMonitor* mon)
{
    if (!mon)
        return true;

    int freqCnt = SvHashTableCount(mon->muxHT);
    return mon->freqHandledCnt >= freqCnt;
}

void
QBEPGMonitorSetAllowedMuxes(QBEPGMonitor* self, SvArray muxes, SvErrorInfo* errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "QBEPGMonitor self is NULL");
        goto err;
    }
    if (!muxes) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "SvArray muxes is NULL");
        goto err;
    }
    if (SvArrayGetCount(muxes) == 0) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "muxes array is empty");
        goto err;
    }

    QBTunerMuxIdObj* muxObj;
    SvIterator it = SvArrayGetIterator(muxes);
    while ((muxObj = (QBTunerMuxIdObj *) SvIteratorGetNext(&it))) {
        if (!SvObjectIsInstanceOf((SvObject) muxObj, QBTunerMuxIdObj_getType())) {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "muxes array contains objects of wrong type");
            goto err;
        }
    }
    SVRETAIN(muxes);
    SVTESTRELEASE(self->muxes);
    self->muxes = muxes;
    return;
err:
    SvErrorInfoPropagate(error, errorOut);
    return;
}
