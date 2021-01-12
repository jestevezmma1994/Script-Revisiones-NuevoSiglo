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

#include <Logic/StandbyLogic.h>
#include <Logic/StandbyLogicPrivate.h>
#include <assert.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <Services/QBAccessController/QBAccessManager.h>
#include <QBPlatformHAL/QBPlatformOutput.h>
#include <QBPlatformHAL/QBPlatformFrontPanel.h>
#include <QBPlatformHAL/QBPlatformLED.h>
#include <QBPlatformHAL/QBPlatformOption.h>
#include <QBPlatformHAL/QBPlatformUtil.h>
#include <fibers/c/fibers.h>
#include <QBDVBScanner.h>
#include <QBConf.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Services/smartcard.h>
#include <Services/clocks.h>
#include <Windows/standby.h>
#include <Windows/standby/dvbscan.h>
#include <main.h>
#include <Services/core/QBChannelScanningConfManager.h>
#include <Services/upgradeWatcher.h>
#include <Services/QBReencryptionService.h>
#include <Services/core/QBStandardRastersManager.h>
#include <SvGfxHAL/SvGfxEngine.h>
#include <Logic/QBLogicFactory.h>
#include <QBDVBSatellitesDB.h>
#include <QBAppKit/QBGlobalStorage.h>

#define WAKEUP_REASON_PATH "/sys/info/wakeup"

typedef enum QBStandbyLogicWakeupReason_ {
    WakeupReason_notSet,           /**< Wake up reason not set */
    WakeupReason_unknown,          /**< Hard reset or in case of error */
    WakeupReason_ir,               /**< Leaving passive standby mode by pressing Power button on RCU */
    WakeupReason_frontPanelGPIO,   /**< Leaving passive standby mode by pressing Power button on front panel (GPIO wakeup reason) */
    WakeupReason_frontPanelKeypad, /**< Leaving passive standby mode by pressing Power button on front panel (Keypad wakeup reason*/
    WakeupReason_timer,            /**< Leaving passive standby mode on timer event */
    WakeupReason_cnt
} QBStandbyLogicWakeupReason;

SvLocal QBStandbyLogic QBStandbyLogicInit_(QBStandbyLogic self, AppGlobals appGlobals, SvHashTable pluginsWhiteList, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    self->appGlobals = appGlobals;
    self->appStartTime = SvTimeGet();
    self->delaySwitchToOff = true;

    SvErrorInfoPropagate(error, errorOut);
    return self;
}

SvLocal const char *
QBStandbyLogicWakeupReasonToString(QBStandbyLogicWakeupReason reason)
{
    static const char *wakeupReasonToString[] = {
        [WakeupReason_notSet]           = "NotSet",
        [WakeupReason_unknown]          = "Unknown",
        [WakeupReason_ir]               = "IR",
        [WakeupReason_frontPanelGPIO]   = "GPIO",
        [WakeupReason_frontPanelKeypad] = "KEYPAD",
        [WakeupReason_timer]            = "Timer"
    };
    assert(sizeof(wakeupReasonToString) / sizeof(wakeupReasonToString[0]) == WakeupReason_cnt);
    assert(reason < WakeupReason_cnt);

    return wakeupReasonToString[reason];
}

SvLocal QBStandbyLogicWakeupReason
QBStandbyLogicWakeupReasonFromString(const char *str, size_t length)
{
    assert(str);

    for (QBStandbyLogicWakeupReason reason = WakeupReason_notSet; reason < WakeupReason_cnt; reason++) {
        if (strncmp(str, QBStandbyLogicWakeupReasonToString(reason), length) == 0) {
            return reason;
        }
    }

    return WakeupReason_unknown;
}

SvLocal QBStandbyLogicWakeupReason
QBStandbyLogicGetWakeupReason(void)
{
    static QBStandbyLogicWakeupReason wakeupReason = WakeupReason_notSet;
    if (wakeupReason != WakeupReason_notSet) {
        return wakeupReason;
    }
    wakeupReason = WakeupReason_unknown;

    SvData wakeupData = SvDataCreateFromFile(WAKEUP_REASON_PATH, -1, NULL);
    if (!wakeupData) {
        SvLogWarning("%s():%d Can't read wakeup reason from " WAKEUP_REASON_PATH, __func__, __LINE__);
        return wakeupReason;
    }

    char *wakeupStr = SvDataGetBytes(wakeupData);
    size_t wakeupStrLength = SvDataGetLength(wakeupData);
    if (wakeupStrLength == 0) {
        SVRELEASE(wakeupData);
        return wakeupReason;
    }
    if (wakeupStr[wakeupStrLength - 1] == '\n') {
        wakeupStrLength--;
    }

    wakeupReason = QBStandbyLogicWakeupReasonFromString(wakeupStr, wakeupStrLength);

    SVRELEASE(wakeupData);
    return wakeupReason;
}

SvLocal void
QBStandbyLogicWakeupBasedOnReason(QBStandbyLogic self)
{
    QBStandbyState InitialStandbyState = QBStandbyStateDataGetState(self->appGlobals->initialStandbyState);
    assert(InitialStandbyState != QBStandbyState_unknown);

    QBStandbyStateData wantedState = NULL;
    QBStandbyLogicWakeupReason wakeupReason = QBStandbyLogicGetWakeupReason();
    if (wakeupReason == WakeupReason_timer) {
        wantedState = QBStandbyStateDataCreate(QBStandbyState_off, (SvObject) self, false, SVSTRING("Timer power on"));
    } else if (wakeupReason == WakeupReason_unknown) {
        if (InitialStandbyState == QBStandbyState_off || InitialStandbyState == QBStandbyState_standby) {
            wantedState = QBStandbyStateDataCreate(QBStandbyState_standby, (SvObject) self, false,
                    SVSTRING("Unknown power on reason, previous Standby State: Passive or Active Standby"));
        } else {
            wantedState = QBStandbyStateDataCreate(QBStandbyState_on, (SvObject) self, false,
                    SVSTRING("Unknown power on reason, previous Standby State: No Stanby, GUI on"));
        }
    } else {
        wantedState = QBStandbyStateDataCreate(QBStandbyState_on, (SvObject) self, false, SVSTRING("User power on"));
    }
    assert(wantedState != NULL);

    QBStandbyAgentSetWantedState(self->appGlobals->standbyAgent, wantedState);
    SVRELEASE(wantedState);

    SvLogNotice("%s(): wakeupReason = %s", __func__, QBStandbyLogicWakeupReasonToString(wakeupReason));
}

bool QBStandbyLogicCanSwitch(QBStandbyLogic self, QBStandbyStateData currStateData, QBStandbyStateData wantedStateData)
{
    AppGlobals appGlobals = self->appGlobals;

    if (QBStandbyStateDataGetState(wantedStateData) != QBStandbyState_off)
        self->delaySwitchToOff = false;

    if (QBStandbyStateDataGetState(wantedStateData) > QBStandbyStateDataGetState(currStateData))
        return true;

    // check if remainders are active right now.
    int nearestWatch = QBSchedManagerGetNearest(QBSchedType_watch);
    if (nearestWatch >= 0 && nearestWatch < 5) {
        SvLogNotice("QBStandbyLogic : CanSwitch() -> %d sec to nearest watch", nearestWatch);
        return false;
    }

    if (QBStandbyStateDataGetState(wantedStateData) == QBStandbyState_standby)
        return true;

    assert(QBStandbyStateDataGetState(currStateData) == QBStandbyState_standby && QBStandbyStateDataGetState(wantedStateData) == QBStandbyState_off);

    // check if pvrs are active right now, or will be active in a short time.
    int nearestRecord = QBSchedManagerGetNearest(QBSchedType_record);
    if (nearestRecord >= 0 && nearestRecord < 3 * 60) {
        SvLogNotice("QBStandbyLogic : CanSwitch() -> %d sec to nearest record", nearestRecord);
        return false;
    }

    if (!self->delaySwitchToOff) {
        SvLogNotice("QBStandbyLogic : CanSwitch() -> switch to off is not delayed");
        return true;
    }

    // EPG monitor at least once scanned all frequencies
    if (appGlobals->epgMonitor && !QBEPGMonitorIsCompleted(appGlobals->epgMonitor)) {
        SvLogNotice("QBStandbyLogic : CanSwitch() -> epg monitor is still working");
        return false;
    }

    // standby agent has done each phase at least once
    if (!QBStandbyAgentIsCompleted(appGlobals->standbyAgent)) {
        SvLogNotice("QBStandbyLogic : CanSwitch() -> standby agent is still working");
        return false;
    }

    if (self->delaySwitchToOff) {
        SvTime now = SvTimeGet();
        int diffSec = SvTimeGetSeconds(SvTimeSub(now, self->appStartTime));
        const int minDelaySec = 120;
        SvLogNotice("QBStandbyLogic : CanSwitch() -> %d sec from startup (min=%d)", diffSec, minDelaySec);
        if (diffSec < minDelaySec)
            return false;
    }

    return true;
}

SvLocal int QBStandbyLogicGetWakeupDelay_(QBStandbyLogic self)
{
    int minSec = 8 * 60 * 60;

    // check nearest PVRs/remainders
    int nearestSched = QBSchedManagerGetNearest(-1);
    if (nearestSched >= 0 && minSec > nearestSched)
        minSec = nearestSched;

    // reminders from CubiTV are ignored in "off" mode

    // calculate result
    minSec -= 1 * 60;
    if (minSec < 0)
        minSec = 0;

    return minSec;
}

SvLocal void QBStandbyLogicEnterStandby_(QBStandbyLogic self)
{
    SvLogNotice("%s()", __func__);

    if (self->isStopped) {
        SvLogNotice("%s(): already stopped", __func__);
        return;
    }
    if (self->inStandby) {
        SvLogError("%s(): already in standby", __func__);
        return;
    }

    self->inStandby = true;

    QBStandbyAgentStopServices(self->appGlobals->standbyAgent);

    QBGlobalPlayerControllerStopCurrentController(self->appGlobals->playerController);

    QBSmartcardServiceEnterActiveStandby();

    if (self->appGlobals->reencryptionService) {
        QBReencryptionServiceEnterStandby(self->appGlobals->reencryptionService);
    }

    // power-off all video outputs (will switch SCART to loop mode)
    int output;
    for (output = QBPlatformGetOutputsCount() - 1; output >= 0; --output) {
        QBVideoOutputConfig cfg;
        if (QBPlatformGetOutputConfig(output, &cfg) >= 0 && cfg.enabled && !cfg.powerOff) {
            SvGfxEngineEnableGfxOnOutput(output, false, false);
            QBPlatformOutputSetPowerOff(output, true, QBPlatformOutputPriority_application);
        }
    }

    const char *fp = QBConfigGet("STANDBYFRONTPANELCLOCK");
    if (!fp || !strcmp(fp, "ON")) {
        self->frontPanelClock = QBFrontPanelClockCreate(SvSchedulerGet());
        QBFrontPanelClockStart(self->frontPanelClock);
    } else {
        QBPlatformShowOnFrontPanel("    ");
    }

    if (self->appGlobals->hddStandbyManager) {
        QBHddStandbyManagerStart(self->appGlobals->hddStandbyManager);
    }

    if (QBPlatformGetLEDsCount() > 0)
        QBPlatformSetLEDBrightness(0, QBPlatformLEDBrightness_max);
    if (QBPlatformGetLEDsCount() > 1)
        QBPlatformSetLEDBrightness(1, QBPlatformLEDBrightness_off);

    QBAccessManagerInvalidateAllAuthentications(self->appGlobals->accessMgr, NULL);
}

SvLocal bool QBStandbyLogicHandleInputEvent_(QBStandbyLogic self, SvInputEvent ev)
{
    return false;
}

SvLocal void QBStandbyLogicEnterPassiveStandby_(QBStandbyLogic self)
{
    SvLogNotice("%s()", __func__);

    if (!self->inStandby) {
        SvLogError("%s(): must be called from active standby", __func__);
        return;
    }

    // disable all LEDs to preserve power
    int count = QBPlatformGetLEDsCount();
    for (int led = 0; led < count; led++) {
        QBPlatformSetLEDBrightness(led, QBPlatformLEDBrightness_off);
    }
}

SvLocal void QBStandbyLogicStart_(QBStandbyLogic self)
{
    SvLogNotice("%s()", __func__);

    QBStandbyAgentStartServices(self->appGlobals->standbyAgent);

    QBStandbyLogicWakeupBasedOnReason(self);

    self->noActiveStandbyHappened = true;
}

SvLocal void QBStandbyLogicStopInternal(QBStandbyLogic self)
{
    if (self->appGlobals->reencryptionService) {
        QBReencryptionServiceLeaveStandby(self->appGlobals->reencryptionService);
    }

    if (self->appGlobals->upgradeLogic) {
        QBUpgradeLogicStandbyLeft(self->appGlobals->upgradeLogic);
    }
    SVTESTRELEASE(self->frontPanelClock);
    self->frontPanelClock = NULL;

    if (self->appGlobals->hddStandbyManager) {
        QBHddStandbyManagerStop(self->appGlobals->hddStandbyManager);
    }

    QBStandbyAgentStopServices(self->appGlobals->standbyAgent);

    if (QBPlatformGetFrontPanelFlags() & QBFrontPanelCapability_clock)
        QBPlatformClockOnFrontPanel(0, 0);
}

SvLocal void QBStandbyLogicStop_(QBStandbyLogic self)
{
    SvLogNotice("%s()", __func__);

    self->isStopped = true;
    QBStandbyLogicStopInternal(self);
}

SvLocal void QBStandbyLogicLeaveStandby_(QBStandbyLogic self)
{
    SvLogNotice("%s()", __func__);

    if (self->isStopped) {
        SvLogNotice("%s(): already stopped", __func__);
        return;
    }
    if (!self->inStandby) {
        SvLogError("%s(): not in standby state", __func__);
        return;
    }

    self->inStandby = false;
    QBStandbyLogicStopInternal(self);

    // power-on all outputs
    int output;
    for (output = QBPlatformGetOutputsCount() - 1; output >= 0; --output) {
        QBVideoOutputConfig cfg;
        if (QBPlatformGetOutputConfig(output, &cfg) >= 0 && cfg.enabled && cfg.powerOff) {
            SvGfxEngineEnableGfxOnOutput(output, false, false);
            QBPlatformOutputSetPowerOff(output, false, QBPlatformOutputPriority_application);
            SvGfxEngineEnableGfxOnOutput(output, true, false);
        }
    }

    QBSmartcardServiceLeaveActiveStandby();

    if (!self->noActiveStandbyHappened || QBStandbyState_off != QBStandbyStateDataGetState(self->appGlobals->initialStandbyState)) {
        QBGlobalPlayerControllerResumeCurrentController(self->appGlobals->playerController);
    }
    self->noActiveStandbyHappened = false;

    if (QBPlatformGetLEDsCount() > 0)
        QBPlatformSetLEDBrightness(0, QBPlatformLEDBrightness_off);
    if (QBPlatformGetLEDsCount() > 1)
        QBPlatformSetLEDBrightness(1, QBPlatformLEDBrightness_max);

    QBStandbyAgentStartServices(self->appGlobals->standbyAgent);

    if (QBPlatformGetLEDsCount() > 1)
        QBPlatformSetLEDBrightness(1, QBPlatformLEDBrightness_max);
}

SvLocal bool QBStandbyLogicScanPrepareParams(void *self_, QBStandbyDVBScan scan, QBDVBScannerParams* params)
{
    QBStandbyLogic self = self_;
    AppGlobals appGlobals = self->appGlobals;
    QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
    QBChannelScanningConf conf = QBChannelScanningConfManagerGetConf(channelScanningConf, SVSTRING("automatic"));
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBStandardRastersManager standardRastersManager = (QBStandardRastersManager)
        QBGlobalStorageGetItem(globalStorage, QBStandardRastersManager_getType(), NULL);
    QBStandardRastersConfig rastersConfig = QBStandardRastersManagerGetConfig(standardRastersManager);

    int NID, ONID, BOUQUETID, confTmp = 0, status = 0;

    status = QBConfigGetInteger("NID", &confTmp);
    NID = status ? -1 : confTmp;

    status = QBConfigGetInteger("ONID", &confTmp);
    ONID = status ? -1 : confTmp;

    status = QBConfigGetInteger("BOUQUET_ID", &confTmp);
    BOUQUETID = status ? -1 : confTmp;

    const char* scanStandard = QBConfigGet("SCANSTANDARD");
    const char* defaultSatelliteID = QBConfigGet("DEFAULTSATELLITE");

    QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
        QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), NULL);

    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    QBChannelScanningConfFillParams(conf, params, rastersConfig, tunerReserver, NID, ONID, BOUQUETID, scanStandard, satellitesDB, defaultSatelliteID);
    SVTESTRELEASE(self->updateParams);
    self->updateParams = DVBEPGPluginCreateUpdateParams((DVBEPGPlugin) appGlobals->epgPlugin.DVB.channelList);
    DVBEPGPluginUpdateParamsFillFromScanningConf(self->updateParams, conf);

    SVTESTRELEASE(self->markParams);
    self->markParams = DVBEPGPluginCreateMarkAllUnseenAsDeadParams((DVBEPGPlugin) appGlobals->epgPlugin.DVB.channelList);
    if (params->metadata) {
        SvIterator keyIt = SvHashTableKeysIterator(params->metadata);
        SvIterator valuesIt = SvHashTableValuesIterator(params->metadata);
        SvHashTableInsertObjects(self->markParams->metadata, &keyIt, &valuesIt);
    }

    return true;
}

SvLocal void QBStandbyLogicScanOnFinish(void *self_, QBStandbyDVBScan scan, QBDVBScanner* scanner, bool forced)
{
    QBStandbyLogic self = self_;
    SvArray channels = QBDVBScannerGetChannels(scanner, true);
    SvHashTable networks = QBDVBScannerGetNetworks(scanner);
    SvHashTable muxes = QBDVBScannerGetMuxes(scanner);
    if (channels && SvArrayCount(channels)) {
        DVBEPGPlugin epgPlugin = (DVBEPGPlugin) self->appGlobals->epgPlugin.DVB.channelList;
        DVBEPGPluginStartUpdate(epgPlugin);
        DVBEPGPluginUpdate(epgPlugin, channels, networks, muxes, self->updateParams);
        SVTESTRELEASE(self->updateParams);
        self->updateParams = NULL;

        DVBEPGPluginMarkUnseenAsDead(epgPlugin, 0, NULL, self->markParams);
        SVTESTRELEASE(self->markParams);
        self->markParams = NULL;

        if (self->appGlobals->dvbMuxesMap) {
            QBDVBMuxesMapUpdate(self->appGlobals->dvbMuxesMap, scanner, NULL);
            QBDVBMuxesMapSave(self->appGlobals->dvbMuxesMap, "/etc/vod/ota-muxes", NULL);
        }
    }

    SVTESTRELEASE(channels);
}

SvLocal void QBStandbyLogicScanOnStart(void *self_, QBStandbyDVBScan scan, QBDVBScanner* scanner)
{
    QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
    QBChannelScanningConf conf = QBChannelScanningConfManagerGetConf(channelScanningConf, SVSTRING("automatic"));
    if (conf) {
        SvArray pluginsList = conf->DVBScannerPlugins;
        if (pluginsList) {
            SvIterator it = SvArrayIterator(pluginsList);
            SvObject plugin;
            while ((plugin = SvIteratorGetNext(&it))) {
                QBDVBScannerAddPlugin(scanner, plugin);
            }
        }
    }
}

SvLocal void QBStandbyLogicPrepareServices_(QBStandbyLogic self, QBWindowContext standby)
{
    if (!self->appGlobals->epgPlugin.DVB.channelList)
        return;

    QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
    QBChannelScanningConf conf = QBChannelScanningConfManagerGetConf(channelScanningConf, SVSTRING("automatic"));

    if (!conf)
        return;

    QBStandbyServiceDVBScan* scan = QBStandbyServiceDVBScanCreate(self->appGlobals);
    static struct QBStandbyDVBScanCallbacks_ callbacks = {
        .prepareParams = QBStandbyLogicScanPrepareParams,
        .onFinish      = QBStandbyLogicScanOnFinish,
        .onStart       = QBStandbyLogicScanOnStart
    };
    QBStandbyServiceDVBScanSetCallbacks(scan, &callbacks, self);
    QBStandbyContextAddService(standby, (SvObject) scan, false);
    SVRELEASE(scan);
}

bool QBStandbyLogicMergeWantedStates(QBStandbyLogic self, SvArray pluginsToStates, QBStandbyStateData finalStateData)
{
    bool changed = false;
    SvIterator it = SvArrayGetIterator(pluginsToStates);
    QBStandbyStateData stateData = NULL;
    while ((stateData = (QBStandbyStateData) SvIteratorGetNext(&it))) {
        if (QBStandbyStateDataGetState(stateData) < QBStandbyStateDataGetState(finalStateData)) {
            QBStandbyStateDataSetFromStateData(finalStateData, stateData);
            changed = true;
        }
    }

    return changed;
}

void QBStandbyLogicGetStandbyStatus(QBStandbyStateData previousWanted,
                                    QBStandbyStateData initial,
                                    QBStandbyStateData wanted)
{
    const char *firstBootStr = QBConfigGet("FIRSTBOOT");
    bool firstBoot = firstBootStr ? !strcmp(firstBootStr, "YES") : true;
    QBStandbyState initialState = firstBoot ? QBStandbyState_on : QBStandbyStateDataGetState(previousWanted);

    if (initial) {
        QBStandbyStateDataSetState(initial, initialState, false, SVSTRING("init state"));
    }

    if (wanted) {
        QBStandbyStateDataSetState(wanted, initialState, false, SVSTRING("init state"));
    }
}

SvLocal void QBStandbyLogicDestroy(void *self_)
{
    QBStandbyLogic self = self_;
    SVTESTRELEASE(self->frontPanelClock);
    SVTESTRELEASE(self->updateParams);
    SVTESTRELEASE(self->markParams);
}

SvType QBStandbyLogic_getType(void)
{
    static const struct QBStandbyLogicVTable_ logicVTable = {
        .super_              = {
            .destroy         = QBStandbyLogicDestroy
        },
        .init                = QBStandbyLogicInit_,
        .start               = QBStandbyLogicStart_,
        .stop                = QBStandbyLogicStop_,
        .handleInputEvent    = QBStandbyLogicHandleInputEvent_,
        .enterPassiveStandby = QBStandbyLogicEnterPassiveStandby_,
        .enterStandby        = QBStandbyLogicEnterStandby_,
        .leaveStandby        = QBStandbyLogicLeaveStandby_,
        .prepareServices     = QBStandbyLogicPrepareServices_,
        .getWakeupDelay      = QBStandbyLogicGetWakeupDelay_
    };

    static SvType type = NULL;

    if (!type) {
        SvTypeCreateVirtual("QBStandbyLogic",
                            sizeof(struct QBStandbyLogic_),
                            SvObject_getType(),
                            sizeof(logicVTable), &logicVTable,
                            &type,
                            NULL);
    }

    return type;
}

QBStandbyLogic QBStandbyLogicCreate(AppGlobals appGlobals, SvHashTable pluginsWhiteList, SvErrorInfo *errorOut)
{
    QBStandbyLogic self = NULL;
    SvErrorInfo error = NULL;

    SvType logicClass = QBLogicFactoryFindImplementationOf(QBLogicFactoryGetInstance(), QBStandbyLogic_getType(), &error);
    if (!logicClass)
        goto fini;

    self = (QBStandbyLogic) SvTypeAllocateInstance(logicClass, &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "can't allocate %s", SvTypeGetName(logicClass));
        goto fini;
    }

    if (!SvInvokeVirtual(QBStandbyLogic, self, init, appGlobals, pluginsWhiteList, &error)) {
        SVRELEASE(self);
        self = NULL;
        goto fini;
    }

    SvLogNotice("%s(): created instance of %s", __func__, SvTypeGetName(logicClass));

fini:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}
