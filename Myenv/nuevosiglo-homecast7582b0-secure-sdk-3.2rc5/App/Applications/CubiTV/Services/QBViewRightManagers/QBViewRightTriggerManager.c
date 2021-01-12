/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBViewRightTriggerManager.h"
#include <QBViewRightTriggers.h>
#include <Services/upgradeWatcher.h>
#include <QBViewRight.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <QBStringUtils.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/label.h>
#include <QBApplicationController.h>
#include <QBWidgets/QBDialog.h>
#include <QBShellCmds.h>
#include <QBTunerTypes.h>
#include <QBTunerInfo.h>
#include <Windows/channelscanning.h>
#include <SvJSON/SvJSONParse.h>
#include <SvJSON/SvJSONSerialize.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Services/core/QBChannelScanningConfManager.h>
#include <QBStaticStorage.h>
#include <QBConf.h>
#include <sv_tuner.h>
#include <tunerReserver.h>
#include <QBTuner.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <safeFiles.h>
#include <time.h>
#include <libintl.h>
#include <Logic/UpgradeLogic.h>
#include <QBDVBSatellitesDB.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <main.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBViewRightTriggerManager"
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 2, moduleName "LogLevel", "");

    #define log_fun(fmt, ...)   do { if (env_log_level() >= 6) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_debug(fmt, ...) do { if (env_log_level() >= 5) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_deep(fmt, ...)  do { if (env_log_level() >= 4) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_info(fmt, ...)  do { if (env_log_level() >= 3) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_state(fmt, ...) do { if (env_log_level() >= 2) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...) do { if (env_log_level() >= 1) SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
 #else
    #define log_fun(fmt, ...)
    #define log_debug(fmt, ...)
    #define log_deep(fmt, ...)
    #define log_info(fmt, ...)
    #define log_state(fmt, ...)
    #define log_error(fmt, ...)
#endif

#define QBVIEWRIGHTWATERMARK_FRAME_TIME_MS 40
#define QBVIEWRIGHTWATERMARK_BACKGROUND_COLOR_COUNT 16
#define QBVIEWRIGHTWATERMARK_FONT_COLOR_COUNT 16
#define QBVIEWRIGHTWATERMARK_DEFAULT_BACKGROUND_COLOR COLOR(0x00, 0x00, 0x00, ALPHA_TRANSPARENT)
#define QBVIEWRIGHTWATERMARK_DEFAULT_FONT_COLOR COLOR(0xFF, 0xFF, 0xFF, ALPHA_SOLID)

typedef struct QBViewRightWatermarkDialog_s *QBViewRightWatermarkDialog;

struct QBViewRightWatermarkDialog_s {
    QBGlobalWindow window;
    SvWidget label;
    SvWidget backgroundLabel;
    SvFiber fiber;
    SvFiberTimer showTimer;
    uint duration;
    bool active;

    bool blinkingMode;

    int xOffset;
    int yOffset;

    SvColor backgroundColorTable[QBVIEWRIGHTWATERMARK_BACKGROUND_COLOR_COUNT];
    SvColor fontColorTable[QBVIEWRIGHTWATERMARK_FONT_COLOR_COUNT];

    struct {
        bool visible;
        uint32_t hiddenTimeMs;
        uint32_t visibleTimeMs;
        SvFiber fiber;
        SvFiberTimer blinkTimer;
    } blinking;
};

#define QBVIEWRIGHT_TRIGGER_ID_HISTORY_LENGTH 16
#define QBVIEWRIGHT_TRIGGER_ID_HISTORY_FILE "/etc/vod/casData/viewright/triggerID"
#define QBVIEWRIGHT_TRIGGER_CUSTOMER_BINDING_FILE "/etc/vod/persistent/viewright/customer_binding"

typedef enum QBViewRightTriggerManagerDecodingState_e {
    QBViewRightTriggerManagerDecodingState_Unknown,
    QBViewRightTriggerManagerDecodingState_Working,
    QBViewRightTriggerManagerDecodingState_Stopped
} QBViewRightTriggerManagerDecodingState;

struct QBViewRightTriggerManager_s {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    struct QBViewRightWatermarkDialog_s watermark;
    QBViewRightWatermark viewRightWatermark;
    bool watermarkAttached;
    QBGlobalWindow globalWindow;
    bool visible;
    SvString boxType;

    QBTunerResv* resv;
    QBTunerReserver *tunerReserver;

    int tunerNum;
    SvWidget upgradeSearchDialog;
    SvWidget upgradeSearchDialogLabel;

    bool hasTerrestialTuner;
    bool hasCableTuner;
    bool hasSatelliteTuner;

    SvString customerID;
    SvHashTable triggerIDsHistory;
    bool customerIsValid;
    SvTime customerBindingDate;      // if customerBindingDate is not set we treat that feature as disabled
    QBViewRightTriggerManagerDecodingState decodingState;
    SvFiber customerBindingFiber;
    SvFiberTimer customerBindingFiberTimer;

    SvFiber upgradeFiber;
    SvFiberTimer upgradeFiberTimer;
    size_t upgradeLeftWaitingTime;

    QBJSONSerializer JSONserializer;
};

SvLocal void QBViewRightTriggerManagerWatermark(SvGenericObject self_, QBViewRightWatermark watermark);
SvLocal void QBViewRightTriggerManagerPinReset(SvGenericObject self_, QBViewRightPin pin);
SvLocal void QBViewRightTriggerManagerUpgrade(SvGenericObject self_, QBViewRightUpgrade upgrade);
SvLocal void QBViewRightTriggerManagerRescan(SvGenericObject self_, QBViewRightRescan rescan);
SvLocal void QBViewRightTriggerManagerCustomerBinding(SvGenericObject self_, QBViewRightCustomerBinding rescan);
SvLocal void QBViewRightTriggerManagerSwitchEnded(SvGenericObject self_, QBWindowContext from, QBWindowContext to);
SvLocal void QBViewRightTriggerManagerSwitchStarted(SvGenericObject self_, QBWindowContext from, QBWindowContext to);

SvLocal void QBViewRightWatermarkDialogCreate(QBViewRightTriggerManager self);
SvLocal void QBViewRightWatermarkDialogStep(void *self_);
SvLocal void QBViewRightWatermarkDialogStop(QBViewRightTriggerManager self);
SvLocal void QBViewRightWatermarkDialogDestroy(QBViewRightTriggerManager self);

SvLocal void QBViewRightTriggerManager__dtor__(void * self_)
{
    log_fun();
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) self_;
    QBViewRightWatermarkDialogDestroy(self);
    SVTESTRELEASE(self->boxType);
    SVTESTRELEASE(self->customerID);
    SVTESTRELEASE(self->triggerIDsHistory);
    SVTESTRELEASE(self->viewRightWatermark);
    SvFiberDestroy(self->customerBindingFiber);
    SvFiberDestroy(self->upgradeFiber);
    SVTESTRELEASE(self->resv);
    SVTESTRELEASE(self->tunerReserver);
    SVTESTRELEASE(self->JSONserializer);
}

SvLocal void QBViewRightTriggerManagerSwitchStarted(SvGenericObject self_, QBWindowContext from, QBWindowContext to)
{
    log_fun();
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) self_;
    if (self->visible) {
        QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, self->globalWindow);
    }
    self->visible = false;
}

SvLocal void QBViewRightTriggerManagerSwitchEnded(SvGenericObject self_, QBWindowContext from, QBWindowContext to)
{
    log_fun();
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) self_;
    if (QBApplicationControllerCanAddGlobalWindowWithName(self->appGlobals->controller, SVSTRING("ViewRightTrigger"))) {
        self->visible = true;
        SvFiberActivate(self->watermark.fiber);
    }
}

SvLocal SvType QBViewRightTriggerManager_getType(void)
{
    log_fun();
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBViewRightTriggerManager__dtor__
    };
    static SvType type = NULL;

    static const struct QBViewRightTriggerListener_i methods = {
        .watermark       = QBViewRightTriggerManagerWatermark,
        .pinReset        = QBViewRightTriggerManagerPinReset,
        .upgrade         = QBViewRightTriggerManagerUpgrade,
        .rescan          = QBViewRightTriggerManagerRescan,
        .customerBinding = QBViewRightTriggerManagerCustomerBinding
    };

    static const struct QBContextSwitcherListener_t switcherMethods = {
        .started = QBViewRightTriggerManagerSwitchStarted,
        .ended   = QBViewRightTriggerManagerSwitchEnded
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBViewRightTriggerManager",
                            sizeof(struct QBViewRightTriggerManager_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBViewRightTriggerListener_getInterface(), &methods,
                            QBContextSwitcherListener_getInterface(), &switcherMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBViewRightTriggerManagerSaveHistory(QBViewRightTriggerManager self)
{
    log_fun();
    QBJSONSerializerAddJob(self->JSONserializer, (SvGenericObject) self->triggerIDsHistory, SVSTRING(QBVIEWRIGHT_TRIGGER_ID_HISTORY_FILE));
}

SvLocal bool QBViewRightTriggerManagerReadHistory(QBViewRightTriggerManager self)
{
    log_fun();
    char *data = NULL;
    QBFileToBuffer(QBVIEWRIGHT_TRIGGER_ID_HISTORY_FILE, &data);

    if (unlikely(!data)) {
        self->triggerIDsHistory = SvHashTableCreate(5, NULL);
        return false;
    }

    self->triggerIDsHistory = (SvHashTable) SvJSONParseString(data, false, NULL);
    free(data);
    if (unlikely(!self->triggerIDsHistory)) {
        log_error("SvJSONParseString return NULL");
        return false;
    } else if (unlikely(!SvObjectIsInstanceOf((SvObject) self->triggerIDsHistory, SvHashTable_getType()))) {
        log_error("SvJSONParseString self->triggerIDBacklog has inproper type");
        SVRELEASE(self->triggerIDsHistory);
        self->triggerIDsHistory = NULL;
        return false;
    }
    return true;
}

SvLocal void QBViewRightTriggerManagerLoadConfig(QBViewRightTriggerManager self, SvString configPath, SvErrorInfo *errorOut)
{
    log_fun();
    SvHashTable config = NULL;
    SvErrorInfo error = NULL;
    SvValue val = NULL;

    char* data = NULL;

    QBFileToBuffer(SvStringCString(configPath), &data);

    if (unlikely(!data)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Couldn't load config file");
        goto fini;
    }

    config = (SvHashTable) SvJSONParseString(data, false, NULL);
    free(data);

    if (!(config && SvObjectIsInstanceOf((SvObject) config, SvHashTable_getType()))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "ViewRight config file has got invalid format");
        goto fini;
    }

    val = (SvValue) SvHashTableFind(config, (SvObject) SVSTRING("boxType"));
    if (!val || !SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) || !SvValueIsString(val)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Couldn't read box type");
        goto fini;
    }

    self->boxType = SvValueGetString(val);
    SVRETAIN(self->boxType);

    val = (SvValue) SvHashTableFind(config, (SvObject) SVSTRING("customerID"));
    if (!val || !SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) || !SvValueIsString(val)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Couldn't read customerID");
        goto fini;
    }

    self->customerID = SvValueGetString(val);
    SVRETAIN(self->customerID);

fini:
    SVTESTRELEASE(config);
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void QBViewRightCustomerBindingStep(void *self_);
SvLocal void QBViewRightUpgradeStep(void *self_);

SvLocal void QBViewRightTriggerManagerTunerRevoked(void *self_, QBTunerReserver* reserver, QBTunerResv* resv);
SvLocal void QBViewRightTriggerManagerTunerObtained(void *self_, QBTunerReserver* reserver, QBTunerResv* resv, int tunerNum);

QBViewRightTriggerManager QBViewRightTriggerManagerCreate(AppGlobals appGlobals, SvString configFile)
{
    log_fun();
    log_deep("configFile=%s", SvStringCString(configFile));

    SvErrorInfo error = NULL;
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) SvTypeAllocateInstance(QBViewRightTriggerManager_getType(), &error);
    if (unlikely(error))
        goto fini;

    self->appGlobals = appGlobals;

    if (unlikely(!QBViewRightGetInstance())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "There is no ViewRight instance - can not create ViewRight Trigger manager");
        goto fini;
    }

    QBViewRightTriggerManagerLoadConfig(self, configFile, &error);
    if (unlikely(error))
        goto fini;

    struct stat sb;
    int status = stat("/etc/vod/persistent/", &sb);
    if (status == -1) {
        log_deep("Directory (%s) does not exist, will be created", "/etc/vod/persistent/");
        mkdir("/etc/vod/persistent/", 0777);
    }

    status = stat("/etc/vod/persistent/viewright/", &sb);
    if (status == -1) {
        log_deep("Directory (%s) does not exist, will be created", "/etc/vod/persistent/viewright/");
        mkdir("/etc/vod/persistent/viewright/", 0777);
    }

    status = stat("/etc/vod/casData/", &sb);
    if (status == -1) {
        log_deep("Directory (%s) does not exist, will be created", "/etc/vod/casData/");
        mkdir("/etc/vod/casData/", 0777);
    }

    status = stat("/etc/vod/casData/viewright/", &sb);
    if (status == -1) {
        log_deep("Directory (%s) does not exist, will be created", "/etc/vod/casData/viewright/");
        mkdir("/etc/vod/casData/viewright/", 0777);
    }

    QBViewRightWatermarkDialogCreate(self);

    if (!QBViewRightTriggerManagerReadHistory(self)) {
        log_info("Couldn't read history file");
    }

    char *customerBindingDateStr = NULL;
    uint retVal = QBFileToBuffer(QBVIEWRIGHT_TRIGGER_CUSTOMER_BINDING_FILE, &customerBindingDateStr);
    // if customerBindingDateStr does not exist we won't use this feature, e.g. customer binding file doesn't exist)
    if (retVal == 10) {
        self->customerIsValid = true;
        if (QBStringToDateTime(customerBindingDateStr, QBDateTimeFormat_YYYY_MM_DD, true, &self->customerBindingDate) < 0) {
            self->customerBindingDate = SvTimeGetZero();
            log_error("Invalid date format %s ", customerBindingDateStr);
            free(customerBindingDateStr);
            goto fini;
        }
    }
    free(customerBindingDateStr);

    QBApplicationControllerAddListener(appGlobals->controller, (SvGenericObject) self);
    QBViewRightTriggerListenerAdd(QBViewRightGetInstance(), (SvGenericObject) self);

    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    if (!tunerReserver) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Can not get tuner reserver from service registry");
        goto fini;
    }
    self->tunerReserver = SVRETAIN(tunerReserver);

    self->customerBindingFiber = SvFiberCreate(appGlobals->scheduler, NULL, "ViewRightCustomerBinding", QBViewRightCustomerBindingStep, self);
    self->customerBindingFiberTimer = SvFiberTimerCreate(self->customerBindingFiber);

    self->upgradeFiber = SvFiberCreate(appGlobals->scheduler, NULL, "ViewRightUpgrade", QBViewRightUpgradeStep, self);
    self->upgradeFiberTimer = SvFiberTimerCreate(self->upgradeFiber);

    static QBTunerResvCallbacks tunerCallbacks = {
        .tunerRevoked = QBViewRightTriggerManagerTunerRevoked,
        .tunerObtained = QBViewRightTriggerManagerTunerObtained,
    };
    self->resv = QBTunerResvCreate(SVSTRING("QBViewRightTriggerManager"), self, &tunerCallbacks);
    self->resv->params.priority = QBTUNERRESERVER_PRIORITY_FORCE_UPGRADE;

fini:
    if (unlikely(error)) {
        SVTESTRELEASE(self);
        self = NULL;
    }
    SvErrorInfoPropagate(error, NULL);
    return self;
}

SvLocal void QBViewRightTriggerManagerUnblockAllTuners(QBViewRightTriggerManager self)
{
    log_fun();

    self->hasTerrestialTuner = true;
    self->hasCableTuner = true;
    self->hasSatelliteTuner = true;
}

SvLocal int QBViewRightTriggerManagerSetAvailableTuners(QBViewRightTriggerManager self)
{
    log_fun();

    const struct QBTunerConfig* configs;
    const struct QBTunerConfigSet* sets;
    int config_cnt, set_cnt;
    int res = QBTunerGetConfig("native", &configs, &config_cnt, &sets, &set_cnt);
    if (res < 0) {
        log_error("No config found for [native]");
        return -1;
    }

    log_debug("Tuners found: %d", config_cnt);

    for (int id = 0; id < config_cnt; ++id) {
        switch (configs[id].props->type) {
            case QBTunerType_ter:
                self->hasTerrestialTuner = true;
                break;
            case QBTunerType_cab:
                self->hasCableTuner = true;
                break;
            case QBTunerType_sat:
                self->hasSatelliteTuner = true;
                break;
            case QBTunerType_unknown:
            default:
                log_error("Unknown Tuner type (%d)", configs[id].props->type);
                break;
        }
    }

    return 0;
}

SvLocal void QBViewRightTriggerManagerClearAvailableTuners(QBViewRightTriggerManager self)
{
    log_fun();

    self->hasTerrestialTuner = false;
    self->hasCableTuner = false;
    self->hasSatelliteTuner = false;
}

void QBViewRightTriggerManagerStart(QBViewRightTriggerManager self)
{
    log_fun();
    QBJSONSerializer JSONserializer = (QBJSONSerializer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBJSONSerializer"));
    if (!JSONserializer) {
        SvLogError("%s(): Can't get JSONSerializer",__func__);
        return;
    }
    self->JSONserializer = SVRETAIN(JSONserializer);
    SvFiberActivate(self->customerBindingFiber);

    if (QBViewRightTriggerManagerSetAvailableTuners(self) < 0) {
        QBViewRightTriggerManagerUnblockAllTuners(self);
    }
}

void QBViewRightTriggerManagerStop(QBViewRightTriggerManager self)
{
    log_fun();
    QBViewRightTriggerManagerClearAvailableTuners(self);
    QBViewRightWatermarkDialogStop(self);
    SvFiberDeactivate(self->customerBindingFiber);
    SvFiberEventDeactivate(self->customerBindingFiberTimer);
    SvFiberDeactivate(self->upgradeFiber);
    SvFiberEventDeactivate(self->upgradeFiberTimer);
}

/************************* triggers specific methods **************************/

//
// upgrade
//

SvLocal void QBViewRightTriggerManagerDestroyUpgradeSearchDialog(QBViewRightTriggerManager self)
{
    log_fun();

    if (self->upgradeSearchDialog) {
        QBDialogBreak(self->upgradeSearchDialog);
        self->upgradeSearchDialog = NULL;
    }
}

SvLocal void QBViewRightTriggerManagerShowUpgradeSearchDialog(QBViewRightTriggerManager self, SvString label)
{
    log_fun();
    svSettingsPushComponent("ViewRightTrigger.settings");
    QBDialogParameters dialogParams = {
        .app = self->appGlobals->res,
        .controller = self->appGlobals->controller,
        .ownerId = 0,
        .widgetName = "ViewrightTriggerUpgradeDialog",
    };
    self->upgradeSearchDialog = QBDialogGlobalNew(&dialogParams, SVSTRING("ViewrightTriggerUpgradeDialog"));

    QBDialogAddPanel(self->upgradeSearchDialog, SVSTRING("content"), NULL, 1);
    self->upgradeSearchDialogLabel = QBDialogAddLabel(self->upgradeSearchDialog, SVSTRING("content"), SvStringCString(label), SVSTRING("message"), 1);
    QBDialogSetTitle(self->upgradeSearchDialog, gettext("Upgrade triggered"));

    svSettingsPopComponent();
    QBDialogRun(self->upgradeSearchDialog, NULL, NULL);
}

SvLocal void QBViewRightTriggerManagerTunerRevoked(void *self_, QBTunerReserver* reserver, QBTunerResv* resv)
{
    log_fun();
    QBViewRightTriggerManager self = self_;
    self->tunerNum = -1;

    log_error("Triggered upgrade has been interrupted");
}

SvLocal void QBViewRightTriggerManagerTunerObtained(void *self_, QBTunerReserver* reserver, QBTunerResv* resv, int tunerNum)
{
    log_fun();
    QBViewRightTriggerManager self = self_;
    self->tunerNum = tunerNum;
}

SvLocal bool QBViewRightTriggerManagerSetCableTunerParams(QBViewRightTriggerManager self, QBViewRightUpgrade upgrade, struct QBTunerParams *tunerParams)
{
    log_fun();

    mpeg_cable_delivery_desc* CDS_descr = QBViewRightUpgradeGetCableDeliveryDescriptor(upgrade);
    if (!CDS_descr) {
        return false;
    }

    log_debug("trigger with cable descriptor");

    // trigger contains descriptor - using given frequency, modulation, symbolRate instead of default ones
    tunerParams->mux_id.freq = CDS_descr->freq;
    tunerParams->modulation = QBTunerModulationFromQAM(CDS_descr->modulation);
    tunerParams->symbol_rate = CDS_descr->symbol_rate;
    return true;
}

SvLocal bool QBViewRightTriggerManagerSetSatelliteTunerParams(QBViewRightTriggerManager self, QBViewRightUpgrade upgrade, struct QBTunerParams *tunerParams)
{
    log_fun();

    dvb_satellite_delivery_desc* desc = QBViewRightUpgradeGetSatelliteDeliveryDescriptor(upgrade);
    if (!desc) {
        return false;
    }

    log_debug("trigger with satellite descriptor");

    tunerParams->mux_id.freq = desc->freq;
    tunerParams->symbol_rate = desc->symbol_rate;

    int orbitalPosition = desc->orbital_position;
    if (desc->is_west) {
        orbitalPosition = 3600 - orbitalPosition;
    }

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
        QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), NULL);
    if (!satellitesDB) {
        log_error("QBGlobalStorageGetItem(QBDVBSatellitesDB_getType()) failed");
        return false;
    }
    SvArray satByPos = QBDVBSatellitesDBCreateListOfSatellitesByPosition(satellitesDB, orbitalPosition);
    if (!satByPos) {
        log_error("Cannot find any satellite for position (%d)", orbitalPosition);
        return false;
    }

    //get first found satellite
    QBDVBSatelliteDescriptor satDesc = (QBDVBSatelliteDescriptor) SvArrayObjectAtIndex(satByPos, 0);
    SVRELEASE(satByPos);

    if (!satDesc) {
        log_error("Found delivery descriptor pointing to satellite not present in satellite DB: orbital pos=%d", orbitalPosition);
        return false;
    }
    QBTunerLNBConfig lnbConfig = QBTunerGetLNBConfigByID(QBDVBSatelliteDescriptorGetID(satDesc));
    if (!lnbConfig) {
        log_error("Satellite not present in lnbConfig: %s", SvStringCString(QBDVBSatelliteDescriptorGetName(satDesc)));
        return false;
    }
    log_state("Found delivery descriptor pointing to other satellite: orbital pos=%d, satelliteID=%s", orbitalPosition, SvStringCString(lnbConfig->ID));
    strncpy(tunerParams->mux_id.satelliteID, SvStringCString(lnbConfig->ID), sizeof(tunerParams->mux_id.satelliteID) - 1);


    tunerParams->modulation = QBTunerModulationFromSatelliteModulationType(desc->modulation_type);
    if (tunerParams->modulation == QBTunerModulation_unknown) {
        log_state("modulation not defined, auto mode normally");
    }

    tunerParams->polarization = QBTunerPolarizationFromDescriptor(desc->polarization);
    if (tunerParams->polarization == QBTunerPolarization_unknown) {
        return false;
    }

    return true;
}

SvLocal bool QBViewRightTriggerManagerSetTerrestrialTunerParams(QBViewRightTriggerManager self, QBViewRightUpgrade upgrade, struct QBTunerParams *tunerParams)
{
    log_fun();
    // TODO: not implemented yet
    return false;
}

SvLocal void QBViewRightTriggerManagerUpgradePerform(QBViewRightTriggerManager self, QBViewRightUpgrade upgrade)
{
    log_fun();

    struct QBTunerParams tunerParams = QBTunerGetEmptyParams();

    bool tunerParamsSet = false;
    if (self->hasCableTuner) {
        tunerParamsSet = QBViewRightTriggerManagerSetCableTunerParams(self, upgrade, &tunerParams);
    }
    if (!tunerParamsSet && self->hasSatelliteTuner) {
        tunerParamsSet = QBViewRightTriggerManagerSetSatelliteTunerParams(self, upgrade, &tunerParams);
    }
    if (!tunerParamsSet && self->hasTerrestialTuner) {
        tunerParamsSet = QBViewRightTriggerManagerSetTerrestrialTunerParams(self, upgrade, &tunerParams);
    }

    if (tunerParamsSet) {
        char *tunerParamsString = QBTunerParamsToString(&tunerParams);
        log_debug("delivery descriptor received, tunerParams (%s)", tunerParamsString);
        free(tunerParamsString);

        // reserve tuner and tune
        QBTunerReserverStartTransaction(self->tunerReserver);
        if (tunerParams.mux_id.freq > 0) {
            self->resv->params.tunerParams = tunerParams;
            self->tunerNum = QBTunerReserverObtain(self->tunerReserver, self->resv);
        } else {
            self->tunerNum = -1;
        }
        QBTunerReserverEndTransaction(self->tunerReserver);

        if (self->tunerNum >= 0) {
            struct QBTunerMuxId currentMux = sv_tuner_get_curr_mux_id(self->tunerNum);
            if (!QBTunerMuxIdEqual(&currentMux, &(tunerParams.mux_id)))
                sv_tuner_tune(self->tunerNum, &tunerParams);
        }
    }

    UpgradeWatcherSetAllForced(self->appGlobals->upgradeWatcher, true);
    UpgradeWatcherIgnoreVersion(self->appGlobals->upgradeWatcher, QBViewRightUpgradeIsForced(upgrade));
    UpgradeWatcherUpgradeImmediately(self->appGlobals->upgradeWatcher, true);
    UpgradeWatcherRefreshNITUpgrade(self->appGlobals->upgradeWatcher);

    self->upgradeLeftWaitingTime = 120; // seconds

    SvString label = SvStringCreateWithFormat(gettext("Waiting for upgrade, (%zu) seconds left"), self->upgradeLeftWaitingTime);
    QBViewRightTriggerManagerShowUpgradeSearchDialog(self, label);
    SVRELEASE(label);

    SvFiberActivate(self->upgradeFiber);
}

SvLocal void QBViewRightTriggerManagerUpgradeNow(QBViewRightTriggerManager self, QBViewRightUpgrade upgrade)
{
    log_fun();
    QBViewRightTriggerManagerUpgradePerform(self, upgrade);
}

SvLocal void QBViewRightTriggerManagerUpgradeForced(QBViewRightTriggerManager self, QBViewRightUpgrade upgrade)
{
    log_fun();
    QBViewRightTriggerManagerUpgradePerform(self, upgrade);
}

/**
 * Updates trigger history with new trigger ID. If given trigger was already used, operation will fail.
 * @return if update operation succeeded
 */
SvLocal bool QBViewRightTriggerManagerUpdateMsgID(QBViewRightTriggerManager self, QBViewRightGenericTrigger trigger)
{
    log_fun();
    uint32_t msgID = SvInvokeInterface(QBViewRightTrigger, trigger, getId);
    if (msgID == 0)
        // not storing trigger with ID = 0, as such trigger should be always executed
        return true;

    bool retVal = false;
    SvHashTableCount(self->triggerIDsHistory);
    SvString triggerType = SvStringCreateWithFormat("%s", SvObjectGetTypeName((SvGenericObject) trigger));
    SvArray idsArray = (SvArray) SvHashTableFind(self->triggerIDsHistory, (SvGenericObject) triggerType);

    if (idsArray == NULL) {
        idsArray = SvArrayCreate(NULL);
        SvHashTableInsert(self->triggerIDsHistory, (SvGenericObject) triggerType, (SvGenericObject) idsArray);
        SVRELEASE(idsArray);
    }

    SvValue triggerIDVal = SvValueCreateWithInteger(msgID, NULL);
    if (SvArrayIndexOfObject(idsArray, (SvGenericObject) triggerIDVal) >= 0) {
        log_deep("Trigger with id = %d already used", msgID);
        retVal = false;
    } else {
        //Add trigger id
        if (SvArrayCount(idsArray) >= QBVIEWRIGHT_TRIGGER_ID_HISTORY_LENGTH) {
            SvArrayRemoveObjectAtIndex(idsArray, 0); //Remove the oldest
        }
        SvArrayAddObject(idsArray, (SvGenericObject) triggerIDVal);
        retVal = true;
    }
    SVRELEASE(triggerIDVal);
    SVRELEASE(triggerType);
    QBViewRightTriggerManagerSaveHistory(self);

    return retVal;
}

SvLocal void QBViewRightTriggerManagerUpgrade(SvGenericObject self_, QBViewRightUpgrade upgrade)
{
    log_fun();
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) self_;

    log_state("Received upgrade trigger : ");
    log_state("    - triggerID = %u", SvInvokeInterface(QBViewRightTrigger, upgrade, getId));
    SvString boxType = QBViewRightUpgradeGetBoxType(upgrade);
    if (boxType) {
        log_state("    - boxType = %s", SvStringGetCString(QBViewRightUpgradeGetBoxType(upgrade)));
    }
    log_state("    - isFoced = %s", QBViewRightUpgradeIsForced(upgrade) ? "true" : "false");

    if (!self->appGlobals->upgradeWatcher)
        return;

    if (!QBViewRightTriggerManagerUpdateMsgID(self, (QBViewRightGenericTrigger) upgrade)) {
        return;
    }

    SvString manufacturerId = SvStringCreateSubString(boxType, 0, 3, NULL);
    SvString individualType = SvStringCreateSubString(boxType, 3, 8, NULL);

    SvString myManufacureId = SvStringCreateSubString(self->boxType, 0, 3, NULL);
    SvString myIndividualType = SvStringCreateSubString(self->boxType, 3, 8, NULL);
    //Check manufacture Id (3 bytes)
    if (!SvStringEqualToCString(manufacturerId, "000")) {
        if (!SvStringEqualToCString(manufacturerId, SvStringCString(myManufacureId))) {
            log_error("Manufacture id does not match got %s, my is %s", SvStringCString(manufacturerId), SvStringCString(myManufacureId));
            goto fini;
        }
        //Check individual type (5 bytes)
        if (!SvStringEqualToCString(individualType, "00000")
            && !SvStringEqualToCString(individualType, SvStringCString(myIndividualType))) {
            log_error("Wrong individual Type, myIndividualType=%s, individualType=%s",
                      SvStringCString(myIndividualType), SvStringCString(individualType));
            goto fini;
        }
    }

    QBViewRightUpgradeType type = QBViewRightUpgradeGetType(upgrade);

    switch (type) {
        case QBViewRightUpgradeType_Now:
            QBViewRightTriggerManagerUpgradeNow(self, upgrade);
            break;
        case QBViewRightUpgradeType_Forced:
            QBViewRightTriggerManagerUpgradeForced(self, upgrade);
            break;
        default:
            log_error("Invalid upgrade type = %d", type);
            break;
    }
fini:
    SVRELEASE(manufacturerId);
    SVRELEASE(myManufacureId);
    SVRELEASE(individualType);
    SVRELEASE(myIndividualType);
}

//
// customer binding
//
SvLocal void QBViewRightTriggerManagerStopDescrambling(QBViewRightTriggerManager self)
{
    log_fun();
    log_state("Stop TV playback");
    self->decodingState = QBViewRightTriggerManagerDecodingState_Stopped;
    QBTVLogicStopPlaying(self->appGlobals->tvLogic, SVSTRING("CAS"));
}

SvLocal void QBViewRightTriggerManagerResumeDescrambling(QBViewRightTriggerManager self)
{
    log_fun();
    log_state("Resume TV playback");
    self->decodingState = QBViewRightTriggerManagerDecodingState_Working;
    QBTVLogicResumePlaying(self->appGlobals->tvLogic, SVSTRING("CAS"));
}

SvLocal void QBViewRightCustomerBindingStep(void *self_)
{
    log_fun();
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) self_;

    SvFiberDeactivate(self->customerBindingFiber);
    SvFiberEventDeactivate(self->customerBindingFiberTimer);
    SvFiberTimerActivateAfter(self->customerBindingFiberTimer, SvTimeConstruct(1, 0));

    switch (self->decodingState) {
        case QBViewRightTriggerManagerDecodingState_Unknown:
            if (SvTimeCmp(self->customerBindingDate, SvTimeGetZero()) && (SvTimeCmp(self->customerBindingDate, SvTimeGetCurrentTime()) < 0)) {
                QBViewRightTriggerManagerStopDescrambling(self);
            } else {
                // content could be shown if customerBindingDate wasn't set or current date is lower or equal customerBindingDate
                self->decodingState = QBViewRightTriggerManagerDecodingState_Working;
            }
            break;
        case QBViewRightTriggerManagerDecodingState_Working:
            if (SvTimeCmp(self->customerBindingDate, SvTimeGetZero()) && (!self->customerIsValid || SvTimeCmp(self->customerBindingDate, SvTimeGetCurrentTime()) < 0)) {
                // if customerBindingDate is not set we treat that feature as disabled
                QBViewRightTriggerManagerStopDescrambling(self);
            }
            break;
        case QBViewRightTriggerManagerDecodingState_Stopped:
            if (SvTimeCmp(self->customerBindingDate, SvTimeGetZero()) && (self->customerIsValid && SvTimeCmp(self->customerBindingDate, SvTimeGetCurrentTime()) >= 0)) {
                QBViewRightTriggerManagerResumeDescrambling(self);
            }
            break;
    }
}

SvLocal void QBViewRightTriggerManagerCustomerBinding(SvGenericObject self_, QBViewRightCustomerBinding customerBinding)
{
    log_fun();
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) self_;
    SvTime customerDate = QBViewRightCustomerBindingGetDate(customerBinding);
    SvString customerID = QBViewRightCustomerBindingGetCustomerID(customerBinding);
    char customerDateStr[128] = {}; // current date format (i.e. SvDateFormat_YYYY_MM_DD) has 10 bytes

    QBStringFromDateTime(customerDateStr, customerDate, QBDateTimeFormat_YYYY_MM_DD, sizeof(customerDateStr), true);

    log_state("Received binding trigger :");
    if (customerID) {
        log_state("    - customerID = %s", SvStringGetCString(customerID));
    }
    log_state("    - customerDate = %s", customerDateStr);

    if (!SvStringEqualToCString(customerID, SvStringCString(self->customerID))) {
        log_error("Invalid customerID = %s, our one is %s stopping descrambling", SvStringCString(customerID), SvStringCString(self->customerID));
        QBViewRightTriggerManagerStopDescrambling(self);
        self->customerIsValid = false;
        QBBufferToFile(QBVIEWRIGHT_TRIGGER_CUSTOMER_BINDING_FILE, "");
        return;
    }
    self->customerIsValid = true;
    self->customerBindingDate = customerDate;

    if (QBBufferToFile(QBVIEWRIGHT_TRIGGER_CUSTOMER_BINDING_FILE, customerDateStr) <= 0) {
        log_error("Couldn't write date to file");
    }
}

//
// rescan
//
SvLocal void QBViewRightTriggerManagerRescanAddModulation(QBChannelScanningConf self, QBTunerModulation modulation)
{
    log_fun();
    for (int i = 0; i < self->modulationCnt; i++) {
        if (self->modulation[i] == modulation)
            return;
    }
    self->modulationCnt++;
    self->modulation = realloc(self->modulation, sizeof(QBTunerModulation) * self->modulationCnt);
    memmove(self->modulation + 1, self->modulation, sizeof(QBTunerModulation) * (self->modulationCnt - 1));
    self->modulation[0] = modulation;
}

SvLocal void QBViewRightTriggerManagerRescanAddSymbolRate(QBChannelScanningConf self, int symbolRate)
{
    log_fun();
    for (int i = 0; i < self->symbolRateCnt; i++) {
        if (self->symbolRate[i] == symbolRate)
            return;
    }
    self->symbolRateCnt++;
    self->symbolRate = realloc(self->symbolRate, sizeof(int) * self->symbolRateCnt);
    memmove(self->symbolRate + 1, self->symbolRate, sizeof(QBTunerModulation) * (self->symbolRateCnt - 1));
    self->symbolRate[0] = symbolRate;
}

SvLocal void QBViewRightTriggerManagerRescan(SvGenericObject self_, QBViewRightRescan rescan)
{
    log_fun();
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) self_;

    log_state("Received rescan trigger : ");
    log_state("    - triggerID = %u", SvInvokeInterface(QBViewRightTrigger, rescan, getId));

    if (!QBViewRightTriggerManagerUpdateMsgID(self, (QBViewRightGenericTrigger) rescan)) {
        return;
    }

    mpeg_cable_delivery_desc * desc = QBViewRightRescanGetCableDeliveryDescriptor(rescan);
    QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
    QBChannelScanningConf conf = QBChannelScanningConfManagerGetConf(channelScanningConf, SVSTRING("automatic"));
    if (conf && desc) {
        QBChannelScanningConf conf2 = (QBChannelScanningConf) SvObjectCopy((SvGenericObject) conf, NULL);
        free(conf2->range);

        uint descCnt = mpeg_cable_delivery_desc_cnt(desc);
        conf2->rangeCnt = conf->rangeCnt + descCnt;
        conf2->range = malloc(conf2->rangeCnt * sizeof(struct QBChannelScanningConfRange_t));
        for (uint i = 0; i < descCnt; i++) {
            mpeg_cable_delivery_desc currentDesc;
            mpeg_cable_delivery_desc_get(desc, i, &currentDesc);
            log_state("Adding descriptor with parameters: freq = %d, modulation = %d, symbolRate = %d",
                      currentDesc.freq, currentDesc.modulation, currentDesc.symbol_rate);
            conf2->range[i].freqMin = conf2->range[i].freqMax = currentDesc.freq;
            conf2->range[i].freqJump = conf->range[0].freqJump;
            QBViewRightTriggerManagerRescanAddModulation(conf2, QBTunerModulationFromQAM(currentDesc.modulation));
            QBViewRightTriggerManagerRescanAddSymbolRate(conf2, currentDesc.symbol_rate);
        }

        int i;
        for (i = 0; i < conf->rangeCnt; i++) {
            conf2->range[i + descCnt].freqMin = conf->range[i].freqMin;
            conf2->range[i + descCnt].freqMax = conf->range[i].freqMax;
            conf2->range[i + descCnt].freqJump = conf->range[i].freqJump;
        }
        SVAUTORELEASE(conf2);
        conf = conf2;
    }

    QBChannelScanningLogicShowChannelScanningWithConf(self->appGlobals->channelScanningLogic, conf);
}

//
// watermark
//

SvLocal void QBViewRightTriggerManagerWatermarkSetPosition(QBViewRightTriggerManager self, QBViewRightWatermark viewRightWatermark)
{
    log_fun();

    QBViewRightWatermarkDialog wt = &self->watermark;

    uint app_w = self->appGlobals->res->width;
    uint app_h = self->appGlobals->res->height;

    uint wDrawingArea = app_w - svLabelGetWidth(wt->label);
    uint hDrawingArea = app_h - svLabelGetHeight(wt->label);

    uint xOff = 0;
    uint yOff = 0;

    QBViewRightWatermarkType type = QBViewRightWatermarkGetType(self->viewRightWatermark);
    switch (type) {
        case QBViewRightWatermarkType_solid:
        case QBViewRightWatermarkType_blinking:
            xOff = rand() % (wDrawingArea + 1);
            yOff = rand() % (hDrawingArea + 1);
            break;
        case QBViewRightWatermarkType_solidWithPositionAndColor:
        {
            uint16_t watermarkXPosition = QBViewRightWatermarkGetXPosition(viewRightWatermark);
            if (watermarkXPosition > QBVIEW_RIGHT_WATERMARK_MAX_X_POSITION) {
                log_error("watermark x position (0x%X) out of range (0x%X)", watermarkXPosition, QBVIEW_RIGHT_WATERMARK_MAX_X_POSITION);
                break;
            }

            uint16_t watermarkYPosition = QBViewRightWatermarkGetYPosition(viewRightWatermark);
            if (watermarkYPosition > QBVIEW_RIGHT_WATERMARK_MAX_Y_POSITION) {
                log_error("watermark y position (0x%X) out of range (0x%X)", watermarkYPosition, QBVIEW_RIGHT_WATERMARK_MAX_Y_POSITION);
                break;
            }

            xOff = watermarkXPosition * wDrawingArea / QBVIEW_RIGHT_WATERMARK_MAX_X_POSITION;
            yOff = watermarkYPosition * hDrawingArea / QBVIEW_RIGHT_WATERMARK_MAX_Y_POSITION;
            break;
        }
        default:
            log_error("Unknown watermark type (%d), position set to (0,0)", type);
            break;
    }

    QBViewportMode mode;
    Sv2DRect rect;
    QBViewportGetVideoWindow(QBViewportGet(), &mode, &rect);

    if (mode == QBViewportMode_windowed) {
        xOff = (xOff * (rect.width - svLabelGetWidth(wt->label)) / app_w) + rect.x;
        yOff = (yOff * (rect.height - wt->label->height) / app_h) + rect.y;
    }

    wt->xOffset = xOff;
    wt->yOffset = yOff;
    log_debug("watermark position (%u, %u)", wt->xOffset, wt->yOffset);
}

SvLocal void QBViewRightTriggerManagerWatermarkSetColor(QBViewRightTriggerManager self, QBViewRightWatermark viewRightWatermark)
{
    log_fun();
    QBViewRightWatermarkDialog wt = &self->watermark;

    QBViewRightWatermarkType type = QBViewRightWatermarkGetType(self->viewRightWatermark);
    switch (type) {
        case QBViewRightWatermarkType_solid:
        case QBViewRightWatermarkType_blinking:
        {
            ///random color
            wt->backgroundLabel->fill_color = COLOR(0x00, 0x00, 0x00, ALPHA_TRANSPARENT);
            svWidgetForceRender(wt->backgroundLabel);

            svLabelSetColor(wt->label, COLOR(rand() % 0x100, rand() % 0x100, rand() % 0x100, 0xFF));
            break;
        }
        case QBViewRightWatermarkType_solidWithPositionAndColor:
        {
            //get the color from the color array
            uint16_t fontColorIndex = QBViewRightWatermarkGetFontColorIndex(viewRightWatermark);
            uint16_t bkgColorIndex = QBViewRightWatermarkGetBackgroundColorIndex(viewRightWatermark);

            SvColor fontColor = wt->fontColorTable[fontColorIndex];
            SvColor bkgColor = wt->backgroundColorTable[bkgColorIndex];

            svLabelSetColor(wt->label, fontColor);

            ///the background size must be the size of text
            wt->backgroundLabel->height = svLabelGetHeight(wt->label) + 10;
            wt->backgroundLabel->width = svLabelGetWidth(wt->label) + 10;

            wt->backgroundLabel->fill_color = bkgColor;
            svWidgetForceRender(wt->backgroundLabel);
            break;
        }
        default:
            log_error("Unknown watermark type (%d)", type);
            break;
    }
}

SvLocal void QBViewRightTriggerManagerWatermarkAttach(QBViewRightTriggerManager self)
{
    log_fun();
    QBViewRightWatermarkDialog wt = &self->watermark;

    if (self->watermarkAttached) {
        return;
    }

    svWidgetDetach(wt->label);
    svWidgetDetach(wt->backgroundLabel);

    QBViewRightTriggerManagerWatermarkSetPosition(self, self->viewRightWatermark);
    QBViewRightTriggerManagerWatermarkSetColor(self, self->viewRightWatermark);

    svWidgetAttach(wt->window->window, wt->label, wt->xOffset, wt->yOffset, 2);
    svWidgetAttach(wt->window->window, wt->backgroundLabel, wt->xOffset - 5, wt->yOffset, 1);

    self->watermarkAttached = true;
}

SvLocal void QBViewRightTriggerManagerFillWatermark(QBViewRightTriggerManager self)
{
    log_fun();
    QBViewRightWatermarkDialog wt = &self->watermark;
    SvString text = QBViewRightWaremarkGetText(self->viewRightWatermark);

    svLabelSetText(wt->label, SvStringCString(text));
    // Make sure that label is not hidden.
    svWidgetSetHidden(wt->label, false);
    svWidgetForceRender(wt->label);

    wt->duration = QBViewRightWatermarkGetDuration(self->viewRightWatermark);

    QBViewRightWatermarkType type = QBViewRightWatermarkGetType(self->viewRightWatermark);
    switch (type) {
        case QBViewRightWatermarkType_solidWithPositionAndColor:
        case QBViewRightWatermarkType_solid:
            wt->blinkingMode = false;
            break;
        case QBViewRightWatermarkType_blinking:
            wt->blinkingMode = true;
            wt->blinking.visible = true;
            wt->blinking.hiddenTimeMs = QBViewRightWatermarkGetHiddenFramesCount(self->viewRightWatermark) * QBVIEWRIGHTWATERMARK_FRAME_TIME_MS;
            wt->blinking.visibleTimeMs = QBViewRightWatermarkGetVisibleFramesCount(self->viewRightWatermark) * QBVIEWRIGHTWATERMARK_FRAME_TIME_MS;
            break;
        default:
            log_error("Unknown watermark type (%d). Show it anyway as 'solid' one.", type);
            break;
    }

    QBViewRightTriggerManagerWatermarkAttach(self);

    wt->active = true;
    self->globalWindow = wt->window;
}

SvLocal void QBViewRightTriggerManagerWatermark(SvGenericObject self_, QBViewRightWatermark watermark)
{
    log_fun();
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) self_;
    QBViewRightWatermarkDialog wt = &self->watermark;

    log_state("Received watermark : ");
    log_state("    - duration : %u", QBViewRightWatermarkGetDuration(watermark));
    SvString text = QBViewRightWaremarkGetText(watermark);
    if (text) {
        log_state("    - text : %s", SvStringGetCString(text));
    }

    QBViewRightWatermarkType type = QBViewRightWatermarkGetType(watermark);
    switch (type) {
        case QBViewRightWatermarkType_solid:
            log_state("    - type : solid");
            break;
        case QBViewRightWatermarkType_blinking:
            log_state("    - type : blinking");
            log_state("    - visible frames count : %u", QBViewRightWatermarkGetVisibleFramesCount(watermark));
            log_state("    - hidden frames count : %u", QBViewRightWatermarkGetHiddenFramesCount(watermark));
            break;
        case QBViewRightWatermarkType_solidWithPositionAndColor:
            log_state("    - type : solidWithPositionAndColor");
            log_state("    - x position : 0x%X/0x%X", QBViewRightWatermarkGetXPosition(watermark), QBVIEW_RIGHT_WATERMARK_MAX_X_POSITION);
            log_state("    - y position : 0x%X/0x%X", QBViewRightWatermarkGetYPosition(watermark), QBVIEW_RIGHT_WATERMARK_MAX_Y_POSITION);
            log_state("    - font color index : 0x%X", QBViewRightWatermarkGetFontColorIndex(watermark));
            log_state("    - background color index : 0x%X", QBViewRightWatermarkGetBackgroundColorIndex(watermark));
            break;
        default:
            log_error("    - type : unknown (%d)", type);
            break;
    }

    // "deactivate" everything due to watermark state change
    if (wt->blinkingMode) {
        SvFiberDeactivate(wt->blinking.fiber);
        SvFiberEventDeactivate(wt->blinking.blinkTimer);
    }
    SvFiberEventDeactivate(wt->showTimer);
    SvFiberDeactivate(wt->fiber);

    SVTESTRELEASE(self->viewRightWatermark);
    self->viewRightWatermark = SVRETAIN(watermark);
    self->watermarkAttached = false;
    wt->active = true;
    SvFiberActivate(wt->fiber);
}

SvLocal void QBViewRightWatermarkDialogBlinkingStep(void *self_)
{
    log_fun();
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) self_;
    QBViewRightWatermarkDialog wt = &self->watermark;

    SvFiberDeactivate(wt->blinking.fiber);
    SvFiberEventDeactivate(wt->blinking.blinkTimer);
    if (wt->blinking.visible) {
        log_debug("Hide fingerprint");
        svWidgetSetHidden(wt->label, true);
        wt->blinking.visible = false;
        SvFiberTimerActivateAfter(wt->blinking.blinkTimer, SvTimeConstruct(0, wt->blinking.hiddenTimeMs * 1000));
    } else {
        log_debug("Show fingerprint");
        svWidgetSetHidden(wt->label, false);
        svWidgetForceRender(wt->label);
        wt->blinking.visible = true;
        SvFiberTimerActivateAfter(wt->blinking.blinkTimer, SvTimeConstruct(0, wt->blinking.visibleTimeMs * 1000));
    }

    svWidgetForceRender(wt->label);
}

SvLocal void QBViewRightWatermarkDialogStep(void *self_)
{
    log_fun();
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) self_;
    QBViewRightWatermarkDialog wt = &self->watermark;
    SvFiberDeactivate(wt->fiber);

    if (SvFiberEventIsActive(wt->showTimer)) {
        SvFiberEventDeactivate(wt->showTimer);
        QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, wt->window);
        // Deactivate blinking if appropriate
        if (wt->blinkingMode) {
            SvFiberDeactivate(wt->blinking.fiber);
            SvFiberEventDeactivate(wt->blinking.blinkTimer);
        }
        wt->active = false;
        // we don't need this anymore
        SVRELEASE(self->viewRightWatermark);
        self->viewRightWatermark = NULL;
    } else if (self->visible && wt->active) {
        // Deactivate blinking if appropriate
        if (wt->blinkingMode) {
            SvFiberDeactivate(wt->blinking.fiber);
            SvFiberEventDeactivate(wt->blinking.blinkTimer);
        }

        QBViewRightTriggerManagerFillWatermark(self);
        QBApplicationControllerAddGlobalWindow(self->appGlobals->controller, self->globalWindow);
        if (!SvFiberEventIsArmed(wt->showTimer)) {
            SvFiberTimerActivateAfter(wt->showTimer, SvTimeConstruct(wt->duration, 0));
        }

        if (wt->blinkingMode) {
            SvFiberActivate(wt->blinking.fiber);
        }
    }
}

SvLocal void QBViewRightWatermarkDialogCreate(QBViewRightTriggerManager self)
{
    log_fun();
    QBViewRightWatermarkDialog wt = &self->watermark;
    AppGlobals appGlobals = self->appGlobals;

    wt->fiber = SvFiberCreate(appGlobals->scheduler, NULL, "ViewRightWatermark", QBViewRightWatermarkDialogStep, self);
    wt->showTimer = SvFiberTimerCreate(wt->fiber);

    wt->blinking.fiber = SvFiberCreate(appGlobals->scheduler, NULL, "QBViewRightWatermarkDialogBlinking", QBViewRightWatermarkDialogBlinkingStep, self);
    wt->blinking.blinkTimer = SvFiberTimerCreate(wt->blinking.fiber);

    svSettingsPushComponent("ViewRightTrigger.settings");
    SvWidget window = svSettingsWidgetCreate(appGlobals->res, "Watermark");
    SvWidget label = svLabelNewFromSM(appGlobals->res, "Watermark.Label");
    SvWidget backgroundLabel = svSettingsWidgetCreate(appGlobals->res, "Watermark.Background");

    ///check the available background colors
    for (int i = 0; i < QBVIEWRIGHTWATERMARK_BACKGROUND_COLOR_COUNT; ++i) {
        SvString string = SvStringCreateWithFormat("backgroundColor%d", i);
        const char *tag = SvStringCString(string);
        wt->backgroundColorTable[i] = svSettingsGetColor("Watermark", tag, QBVIEWRIGHTWATERMARK_DEFAULT_BACKGROUND_COLOR);
        SVRELEASE(string);
    }

    ///check the available font colors
    for (int i = 0; i < QBVIEWRIGHTWATERMARK_FONT_COLOR_COUNT; ++i) {
        SvString string = SvStringCreateWithFormat("fontColor%d", i);
        const char *tag = SvStringCString(string);
        wt->fontColorTable[i] = svSettingsGetColor("Watermark", tag, QBVIEWRIGHTWATERMARK_DEFAULT_FONT_COLOR);
        SVRELEASE(string);
    }

    svSettingsPopComponent();

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, window, SVSTRING("ViewRightWatermark"));
    svWidgetAttach(window, label, 0, 0, 2);
    svWidgetAttach(window, backgroundLabel, 0, 0, 1);
    wt->window = globalWindow;
    wt->label = label;
    wt->backgroundLabel = backgroundLabel;
}

SvLocal void QBViewRightWatermarkDialogStop(QBViewRightTriggerManager self)
{
    log_fun();
    QBViewRightWatermarkDialog wt = &self->watermark;
    SvFiberDeactivate(wt->fiber);
    SvFiberEventDeactivate(wt->showTimer);
    SvFiberDeactivate(wt->blinking.fiber);
    SvFiberEventDeactivate(wt->blinking.blinkTimer);
}

SvLocal void QBViewRightWatermarkDialogDestroy(QBViewRightTriggerManager self)
{
    log_fun();
    QBViewRightWatermarkDialog wt = &self->watermark;
    SvFiberDestroy(wt->fiber);
    SvFiberDestroy(wt->blinking.fiber);
    SVTESTRELEASE(wt->window);
}

//
// pin reset
//
SvLocal void QBViewRightTriggerManagerPinReset(SvGenericObject self_, QBViewRightPin pin)
{
    log_fun();
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) self_;
    log_state("Received pin reset trigger.");
    SvString pinStr = QBViewRightPinGetString(pin);
    if (pinStr) {
        log_state("    - newPin = %s", SvStringGetCString(pinStr));
    }

    QBPasswordStatus status = QBAccessManagerResetPassword(self->appGlobals->accessMgr, SVSTRING("PC"), pinStr, NULL);
    if (status != QBPasswordStatus_OK) {
        log_error("Cannot reset PIN");
    }
}

SvLocal void QBViewRightUpgradeStep(void *self_)
{
    log_fun();
    QBViewRightTriggerManager self = (QBViewRightTriggerManager) self_;
    SvFiberDeactivate(self->upgradeFiber);
    SvFiberEventDeactivate(self->upgradeFiberTimer);

    if (self->upgradeLeftWaitingTime == 0) {
        log_state("Upgrade not found - aborting upgrade");

        QBTunerReserverStartTransaction(self->tunerReserver);
        QBTunerReserverRelease(self->tunerReserver, self->resv);
        self->tunerNum = -1;
        QBTunerReserverEndTransaction(self->tunerReserver);

        UpgradeWatcherSetAllForced(self->appGlobals->upgradeWatcher, false);
        UpgradeWatcherIgnoreVersion(self->appGlobals->upgradeWatcher, false);
        UpgradeWatcherUpgradeImmediately(self->appGlobals->upgradeWatcher, false);

        QBViewRightTriggerManagerDestroyUpgradeSearchDialog(self);
        return;
    }

    SvString label = SvStringCreateWithFormat(gettext("Waiting for upgrade, (%zu) seconds left"), self->upgradeLeftWaitingTime);
    svLabelSetText(self->upgradeSearchDialogLabel, SvStringCString(label));
    SVRELEASE(label);

    self->upgradeLeftWaitingTime--;
    SvFiberTimerActivateAfter(self->upgradeFiberTimer, SvTimeConstruct(1, 0));
}
