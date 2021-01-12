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

#include <Services/BackgroundScanner.h>

#include <stdbool.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <sv_tuner.h>
#include <QBTuner.h>
#include <QBDVBScanner.h>
#include <QBDVBMuxesMap.h>
#include <SvEPGDataLayer/Plugins/DVBEPGPlugin.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Services/upgradeWatcher.h>
#include <Services/core/QBStandardRastersManager.h>
#include <main.h>
#include <QBConfig.h>
#include <QBDVBSatellitesDB.h>
#include <QBAppKit/QBGlobalStorage.h>

#include <Logic/ChannelScanningLogic.h>
#include <DVBScannerPlugins/SIDAsChannelNumberPlugin.h>

/* < GRIAL: Migración del SDK 1*/
#include <SvCore/SvLog.h>
#include <Services/core/QBChannelScanningConfManager.h>
#include <QBDVBChannelDesc.h>
#include <SvFoundation/SvIterator.h>
/* > GRIAL: Migración del SDK 1*/

typedef enum {
    BackgroundScannerState_idle = 0,
    BackgroundScannerState_scanning,
    BackgroundScannerState_finished
} BackgroundScannerState;


struct BackgroundScanner_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBDVBSatellitesDB satellitesDB;

    SvFiber fiber;
    SvFiberTimer timer;

    BackgroundScannerState state;
    unsigned int tunerNum;
    struct QBTunerMuxId mux;
    QBDVBScanner *scanner;
    DVBEPGPluginUpdateParams *updateParams;

    BackgroundScannerFinishedCallback callback;
    void *callbackData;
    
    int homeFrequency;
};


SvLocal void BackgroundScannerSaveResults(BackgroundScanner self)
{
    SvArray channels = QBDVBScannerGetChannels(self->scanner, true);
    SvHashTable networks = QBDVBScannerGetNetworks(self->scanner);
    SvHashTable muxes = QBDVBScannerGetMuxes(self->scanner);
    /// < GRIAL END: Migración del SDK 1
    /**
     * Avoid calling QBChannelScanningConfManagerGetConf more than once
    */
    if (self->homeFrequency < 0)
    {
        QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
        QBChannelScanningConf conf = QBChannelScanningConfManagerGetConf(channelScanningConf, SVSTRING("automatic"));
        self->homeFrequency = conf->range->freqMin; /// < Set min frequency
    }

    bool rejectNitTable = true;
    SvIterator itChan = SvArrayIterator(channels);
    QBDVBChannelDesc channel = NULL;

    //SvLogNotice("\n\nGRIAL: %s():%i Se ejecuta bkgnd escaner\n\n", __func__,__LINE__);
    /* { NR: Patch applied to avoid updating channels in Nuevosiglos and Montecable Common Frequencies
     *If Home Frequency is not found NIT is rejected */
    while((channel = (QBDVBChannelDesc) SvIteratorGetNext(&itChan))) {
        
        /*SvLogNotice("\n\nGRIAL: %s():%i channel tsid=%i, onid=%i, lcn=%i, isVisible=%i\n\n", __func__,__LINE__,
                    channel->tsid,
                    channel->onid,
                    channel->lcn,
                    channel->visible);
        */
        
    	SvHashTable variantTable = channel->muxidToVariant;
        SvIterator itVariant;

    	if (variantTable) 
            itVariant = SvHashTableValuesIterator(variantTable);
        else
            continue;

    	QBDVBChannelVariant* variant = NULL;

    	while ((variant = (QBDVBChannelVariant*) SvIteratorGetNext(&itVariant))) {
            //SvLogNotice("\n\nGRIAL: %s():%i mux_freq=%i\n\n", __func__,__LINE__,variant->params.mux_id.freq);
    		if (variant->params.mux_id.freq == self->homeFrequency){
    		    rejectNitTable = false;
    		    break;
    		}
    	}
    
    }
    // } NR
    /// > GRIAL END: Migración del SDK 1
    if (channels && SvArrayCount(channels) && (rejectNitTable == false) ) { // > GRIAL: Se agrega condicion (rejectNitTable == false)
        SvLogNotice("%s(): scanning finished, found %zu channels", __func__, SvArrayCount(channels));
        DVBEPGPlugin epgPlugin = (DVBEPGPlugin) self->appGlobals->epgPlugin.DVB.channelList;
        DVBEPGPluginStartUpdate(epgPlugin);
        DVBEPGPluginUpdate(epgPlugin, channels, networks, muxes, self->updateParams);
        SVTESTRELEASE(self->updateParams);
        self->updateParams = NULL;

        DVBEPGPlugin dvbPlugin = (DVBEPGPlugin) self->appGlobals->epgPlugin.DVB.channelList;
        DVBEPGPluginMarkAllUnseenAsDeadParams* markParams = DVBEPGPluginCreateMarkAllUnseenAsDeadParams(dvbPlugin);
        const QBDVBScannerParams* scannerParams = QBDVBScannerGetParams(self->scanner);
        if (scannerParams->metadata) {
            SvIterator keyIt = SvHashTableKeysIterator(scannerParams->metadata);
            SvIterator valuesIt = SvHashTableValuesIterator(scannerParams->metadata);
            SvHashTableInsertObjects(markParams->metadata, &keyIt, &valuesIt);
        }
        DVBEPGPluginMarkUnseenAsDead(dvbPlugin, 9, NULL, markParams);
        SVRELEASE(markParams);

        if (self->appGlobals->dvbMuxesMap) {
            QBDVBMuxesMapUpdate(self->appGlobals->dvbMuxesMap, self->scanner, NULL);
            QBDVBMuxesMapSave(self->appGlobals->dvbMuxesMap, "/etc/vod/ota-muxes", NULL);
        }

    } else {
        SvLogNotice("%s(): scanning finished, nothing found", __func__);
    }

    SVTESTRELEASE(channels);

    if (self->callback) {
        self->callback(self->callbackData, true);
    }
}

SvLocal void BackgroundScannerOnStep(void *self_, QBDVBScanner *scanner)
{
}

SvLocal void BackgroundScannerOnFinish(void *self_, QBDVBScanner *scanner)
{
    BackgroundScanner self = self_;

    if (self->state != BackgroundScannerState_scanning)
        return;

    self->state = BackgroundScannerState_finished;
    QBDVBScannerStop(self->scanner);
}

SvLocal void BackgroundScannerStep(void *self_)
{
    BackgroundScanner self = self_;

    if (self->state == BackgroundScannerState_finished) {
        BackgroundScannerSaveResults(self);
    }

    if (self->state == BackgroundScannerState_scanning) {
        // check if tuner can be still used
        struct sv_tuner_state *tuner = sv_tuner_get(self->tunerNum);
        struct QBTunerStatus status;
        sv_tuner_get_status(tuner, &status);
        if (!status.full_lock || !QBTunerMuxIdEqual(&status.params.mux_id, &self->mux)) {
            SvLogNotice("%s(): tuner lost, scanning cancelled", __func__);
            self->state = BackgroundScannerState_finished;
            QBDVBScannerStop(self->scanner);
            if (self->callback) {
                self->callback(self->callbackData, false);
            }
        }
    }

    if (self->state != BackgroundScannerState_scanning) {
        if (self->state == BackgroundScannerState_finished) {
            QBDVBScannerDestroy(self->scanner);
            self->scanner = NULL;
            self->state = BackgroundScannerState_idle;
        }
        if (self->fiber) {
            SvFiberDestroy(self->fiber);
            self->fiber = NULL;
        }
        return;
    }

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(0, 200 * 1000));
}

SvLocal void BackgroundScannerDestroy(void *self_)
{
    BackgroundScanner self = self_;
    BackgroundScannerStopScanning(self);
    SVTESTRELEASE(self->updateParams);
    SVTESTRELEASE(self->satellitesDB);
}

SvLocal SvType BackgroundScanner_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = BackgroundScannerDestroy
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("BackgroundScanner",
                            sizeof(struct BackgroundScanner_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

BackgroundScanner BackgroundScannerCreate(AppGlobals appGlobals)
{
    BackgroundScanner self = (BackgroundScanner) SvTypeAllocateInstance(BackgroundScanner_getType(), NULL);

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
        QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), NULL);

    self->satellitesDB = SVTESTRETAIN(satellitesDB);
    self->appGlobals = appGlobals;
    self->state = BackgroundScannerState_idle;
    self->homeFrequency = -1;

    return self;
}

void BackgroundScannerSetCallback(BackgroundScanner self, BackgroundScannerFinishedCallback callback, void *owner)
{
    self->callback = callback;
    self->callbackData = owner;
}

bool BackgroundScannerIsScanning(BackgroundScanner self)
{
    return self->state == BackgroundScannerState_scanning;
}

void BackgroundScannerStartScanning(BackgroundScanner self, unsigned int tunerNum, QBChannelScanningConf conf)
{
    if (self->state != BackgroundScannerState_idle) {
        SvLogWarning("%s(): already started!", __func__);
        return;
    }

    AppGlobals appGlobals = self->appGlobals;
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBStandardRastersManager standardRastersManager = (QBStandardRastersManager)
        QBGlobalStorageGetItem(globalStorage, QBStandardRastersManager_getType(), NULL);
    QBStandardRastersConfig rastersConfig = QBStandardRastersManagerGetConfig(standardRastersManager);

    struct sv_tuner_state *tuner = sv_tuner_get(tunerNum);
    struct QBTunerStatus status;
    sv_tuner_get_status(tuner, &status);

    QBDVBScannerParams params = {
        .forced = {
            .band = NULL,
        },
    };

    params.freq = malloc(sizeof(int));
    params.freq[0] = status.params.mux_id.freq;
    params.freqCnt = 1;

    params.band = status.params.band;
    params.forced.band = NULL;

    params.modulations = malloc(sizeof(QBTunerModulation));
    params.modulations[0] = status.params.modulation;
    params.modulationCnt = 1;

    params.symbolRates = malloc(sizeof(int));
    params.symbolRates[0] = status.params.symbol_rate;
    params.symbolRateCnt = 1;

    int NID, ONID, BOUQUETID, confTmp = 0, ans = 0;
    ans = QBConfigGetInteger("NID", &confTmp);
    NID = ans ? -1 : confTmp;
    ans = QBConfigGetInteger("ONID", &confTmp);
    ONID = ans ? -1 : confTmp;
    ans = QBConfigGetInteger("BOUQUET_ID", &confTmp);
    BOUQUETID = ans ? -1 : confTmp;

    const char* scanStandard = QBConfigGet("SCANSTANDARD");
    const char* defaultSatelliteID = QBConfigGet("DEFAULTSATELLITE");

    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    QBChannelScanningConfFillParams(conf, &params, rastersConfig, tunerReserver, NID, ONID, BOUQUETID, scanStandard, self->satellitesDB, defaultSatelliteID);

    params.target = self;
    params.onFinish = BackgroundScannerOnFinish;
    params.onStep = BackgroundScannerOnStep;

    self->tunerNum = tunerNum;
    params.tunerNum = tunerNum;

    self->mux = status.params.mux_id;
    self->scanner = QBDVBScannerCreate(appGlobals->scheduler, conf->DVBScannerPlugins);
    QBDVBScannerSetPassiveMode(self->scanner, true, status.params.mux_id);
    self->state = BackgroundScannerState_scanning;

    QBChannelScanningLogicSetDVBScannerPlugins(appGlobals->channelScanningLogic, self->scanner);
    QBDVBScannerStart(self->scanner, &params);

    // XXX: This shows how to add SIDAsChannelNumberPlugin
    /**/
    SIDAsChannelNumberPlugin plugin = SIDAsChannelNumberPluginCreate();
    QBDVBScannerAddPlugin(self->scanner, (SvGenericObject) plugin);
    SVRELEASE(plugin);
    /**/

    SVTESTRELEASE(self->updateParams);
    self->updateParams = DVBEPGPluginCreateUpdateParams((DVBEPGPlugin) appGlobals->epgPlugin.DVB.channelList);
    DVBEPGPluginUpdateParamsFillFromScanningConf(self->updateParams, conf);

    self->fiber = SvFiberCreate(appGlobals->scheduler, NULL, "BackgroundScanner", BackgroundScannerStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(0, 200 * 1000));

    SvLogNotice("%s(): started", __func__);
}

void BackgroundScannerStopScanning(BackgroundScanner self)
{
    if (self->state == BackgroundScannerState_idle)
        return;

    if (self->scanner) {
        if (self->state == BackgroundScannerState_scanning)
            QBDVBScannerStop(self->scanner);
        QBDVBScannerDestroy(self->scanner);
        self->scanner = NULL;
    }

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
    }

    self->state = BackgroundScannerState_idle;
}
