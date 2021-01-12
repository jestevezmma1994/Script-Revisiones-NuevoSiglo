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

#include "QBDiagnosticsWidget.h"

#include <libintl.h>

#include <QBCAS.h>
#include <QBSmartCardMonitor.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvTime.h>
#include <CUIT/Core/widget.h>
#include <dataformat/audio.h>
#include <dataformat/sv_data_format.h>
#include <QBDescrambler/basic_types.h>
#include <QBPlatformHAL/QBPlatformHDMIOutput.h>
#include <QBPlatformHAL/QBPlatformOutput.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <SvDataBucket2/SvDBObject.h>
#include <QBConfig.h>
#include <QBTunerTypes.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <QBAppKit/QBObserver.h>
#include <settings.h>
#include <SWL/gauge.h>
#include <SWL/viewport.h>
#include <sv_tuner.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <QBDiagnostics.h>
#include <QBNATMonitor/QBNATMonitor.h>
#include <QBNetManager.h>
#include <QBUpgrade/QBUpgradeService.h>
#include <SvEPGDataLayer/Plugins/DVBEPGChannel.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <Configurations/QBDiagnosticsWindowConfManager.h>
#include <Logic/DiagnosticsLogic.h>
#include <Logic/TVLogic.h>
#include <main.h>
#include <dvbTimeDate.h>
#include <Services/core/hotplugMounts.h>
#include <Services/core/QBTextRenderer.h>
#include <Utils/freqNames.h>
#include <Windows/newtv.h>
#include <Widgets/QBScrollBar.h>
#include <QBWidgets/QBGrid.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <TranslationMerger.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <QBAppKit/QBServiceRegistry.h>

struct QBDiagnosticsWidget_ {
    ///< super class
    struct SvObject_ super_;

    bool useRatioSettings;
    char *name;
    unsigned int settingsCtx;
    AppGlobals appGlobals;
    SvEPGManager epgManager;

    QBDiagnosticsWindowConf conf;
    QBDiagnostics diagnostics;

    SvWidget viewport;
    SvWidget content;

    SvWidget scrollBar;

    SvHashTable diagnosticsWidgets;

    QBCASCmd cmd;

    SvFiber fiberStbStatus;
    SvFiberTimer timerStbStatus;

    SvFiber fiberInterfaceStatus;
    SvFiberTimer timerInterfaceStatus;

    SvFiber fiberGuideData;
    SvFiberTimer timerGuideData;

    SvFiber fiberAudioVideoStatus;
    SvFiberTimer timerAudioVideoStatus;
};

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "DiagnosticsLogLevel", "");

#define log_error(fmt, ...) do { if (env_log_level() >= 0) SvLogError(  COLBEG() "[%s] " fmt COLEND_COL(red), __func__, ##__VA_ARGS__); } while (0)
#define log_warning(fmt, ...) do { if (env_log_level() >= 0) SvLogWarning( COLBEG() "[%s] " fmt COLEND_COL(yellow), __func__, ##__VA_ARGS__); } while (0)
#define log_state(fmt, ...) do { if (env_log_level() >= 1) SvLogNotice( COLBEG() "[%s] " fmt COLEND_COL(cyan), __func__, ##__VA_ARGS__); } while (0)
#define log_debug(fmt, ...) do { if (env_log_level() >= 2) SvLogNotice(          "[%s] " fmt, __func__, ##__VA_ARGS__); } while (0)


#define QB_DIAGNOSTICS_WIDGET_TIMER_INTERVAL 1000 // Ms
#define QB_DIAGNOSTICS_WIDGET_GUIDEDATA_INTERVAL 10000 // Ms
#define QB_DIAGNOSTICS_WIDGET_AUDIOVIDEOSTATUS_INTERVAL 10000 // Ms
#define QB_DIAGNOSTICS_WIDGET_SRM_PROJECT_FILE_PATH "/etc/SRM_PROJECT"

struct QBDiagnosticsWidgetElement_t {
    struct SvObject_ super_;
    SvWidget w; ///< contains QBDiagnosticsWidgetElement as prv
    SvWidget label, text;
};
typedef struct QBDiagnosticsWidgetElement_t* QBDiagnosticsWidgetElement;

typedef void(*DiagnosticsWindowSetter)(QBDiagnosticsWidget self);

SvLocal void QBDiagnosticsWidgetSmartcardState(void *self_, QBCASSmartcardState state);
SvLocal void QBDiagnosticsWidgetSmartcardEcmStatus(void* self_, uint8_t sessionId, const int16_t pid, QBCASEcmStatus status,
    bool is_extra_ecm, int no_access_code, const QBDescramblerCW* cw);
SvLocal QBDiagnosticsWidgetElement QBDiagnosticsWidgetCombineWindowElement(QBDiagnosticsWidget self, const char* labelStr,
    const char* textStr, const char* idStr);

static struct QBCASCallbacks_s smartCardCallbacks = {
    .smartcard_state = QBDiagnosticsWidgetSmartcardState,
    .ecm_status = QBDiagnosticsWidgetSmartcardEcmStatus
};


// Element
SvLocal void
QBDiagnosticsWidgetElementDestroy(void* self_)
{
    QBDiagnosticsWidgetElement self = self_;
    if (self->w) {
        svWidgetDestroy(self->w);
    }
    self->w = NULL;
}

SvLocal SvType
QBDiagnosticsWidgetElement_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDiagnosticsWidgetElementDestroy
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDiagnosticsWidgetElement",
                            sizeof(struct QBDiagnosticsWidgetElement_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal void
QBDiagnosticsWidgetElementSetLabel(QBDiagnosticsWidgetElement self, const char* label)
{
    if (self && self->label && label) {
        SvString val = SvStringCreate(gettext(label), NULL);
        QBAsyncLabelSetText(self->label, val);
        SVRELEASE(val);
    }
}

SvLocal void
QBDiagnosticsWidgetElementSetText(QBDiagnosticsWidgetElement self, const char* text)
{
    SvString val = NULL;
    if (self && self->text) {
        val = SvStringCreate((text ? gettext(text) : gettext("N/A")), NULL);
        QBAsyncLabelSetText(self->text, val);
        SVRELEASE(val);
    }
}

SvLocal void*
QBDiagnosticsWidgetElementFind(QBDiagnosticsWidget self, const char* keyStr)
{
    if (!self || !self->diagnosticsWidgets) {
        return NULL;
    }
    SvString key = SvStringCreate(keyStr, NULL);
    QBDiagnosticsWidgetElement element = (QBDiagnosticsWidgetElement) SvHashTableFind(self->diagnosticsWidgets, (SvObject) key);
    SVRELEASE(key);
    return element;
}

SvLocal void
QBDiagnosticsWidgetSetMacAddress(QBDiagnosticsWidget self)
{
    if (!self) {
        log_error("Wrong params(self:%p)", self);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "macAddress");
    if (!element) {
        log_debug("Elmenet not found");
        return;
    }

    const char* medium = QBConfigGet("MEDIUM");
    SvString activeIface = NULL;

    if (medium && !strcmp(medium, "WIFI")) {
        activeIface = SvStringCreate("ra0", 0);
    } else {
        activeIface = SvStringCreate("eth0", 0);
    }

    SvObject macAddress = NULL;
    if ((macAddress = QBNetManagerGetAttribute(self->appGlobals->net_manager, activeIface, QBNetAttr_hwAddr, NULL))) {
        if (SvObjectIsInstanceOf(macAddress, SvString_getType())) {
            QBDiagnosticsWidgetElementSetText(element, SvStringCString((SvString) macAddress));
        }
    }
    SVRELEASE(activeIface);
}

SvLocal void
QBDiagnosticsWidgetSetEthernetStatus(QBDiagnosticsWidget self, bool forceSet)
{
    if (!self) {
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "ethernetStatus");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    QBNATMonitorStatus status;
    if ((self->appGlobals->natMonitor && QBNATMonitorGetStatus(self->appGlobals->natMonitor, &status)
        && status.localIP ) || forceSet) {
        QBDiagnosticsWidgetElementSetText(element, gettext_noop("Enabled"));
    } else {
        QBDiagnosticsWidgetElementSetText(element, gettext_noop("Disabled"));
    }
}

SvLocal void
QBDiagnosticsWidgetSetIPAddress(QBDiagnosticsWidget self)
{
    if (!self) {
        log_error("Wrong params(self:%p)", self);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "ipAddress");
    if (!element) {
        log_debug("Element not found");
    }

    const char *val = QBConfigGet("NETWORK.TYPE");
    if (val && strcmp(val, "STATIC")) {
        const char* medium = QBConfigGet("MEDIUM");
        SvString activeIface = NULL;

        if (medium && !strcmp(medium, "WIFI")) {
            activeIface = SvStringCreate("ra0", 0);
        } else {
            activeIface = SvStringCreate("eth0", 0);
        }
        SvObject ipAddress = NULL;
        if ((ipAddress = QBNetManagerGetAttribute(self->appGlobals->net_manager, activeIface, QBNetAttr_IPv4_address, NULL))) {
            if (SvObjectIsInstanceOf(ipAddress, SvString_getType())) {
                if (element) {
                    QBDiagnosticsWidgetElementSetText(element, SvStringCString((SvString) ipAddress));
                }
                QBDiagnosticsWidgetSetEthernetStatus(self, true);
            }
        }
        SVRELEASE(activeIface);
    } else { //< false
        if (element) {
            QBDiagnosticsWidgetElementSetText(element, QBConfigGet("NETWORK.STATIC.IP"));
        }
    }
}

SvLocal void
QBDiagnosticsWidgetSetDeviceName(QBDiagnosticsWidget self)
{
    if (!self) {
        log_error("Wrong params(self:%p)", self);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "deviceName");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    const char *deviceName = QBConfigGet("DEVICE_NAME");
    if (!deviceName)
        deviceName = "CubiTV Device";
    QBDiagnosticsWidgetElementSetText(element, deviceName);
}

SvLocal void
QBDiagnosticsWidgetSetFirmwareVersion(QBDiagnosticsWidget self)
{
    if (!self) {
        log_error("Wrong params(self:%p)", self);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "swVersion");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    QBUpgradeService upgradeService = QBUpgradeServiceCreate(NULL);
    if (upgradeService) {
        SvString version = QBUpgradeServiceGetLocalInformativeVersion(upgradeService);
        if (version) {
            QBDiagnosticsWidgetElementSetText(element, SvStringCString(version));
        }
        SVRELEASE(upgradeService);
    }
}

SvLocal void
QBDiagnosticsWidgetSetTunedFrequency(QBDiagnosticsWidget self)
{
    if (!self || !self->appGlobals->newTV) {
        log_debug("Wrong params(self:%p, NewTV:%p)", self, (self ? self->appGlobals->newTV : NULL) );
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "tunedFrequency");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    SvTVChannel channel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
    if (!channel) {
        return;
    }
    int tunerNum = QBTVLogicGetTunerNum(self->appGlobals->tvLogic);
    if (tunerNum < 0) {
        return;
    }
    struct QBTunerStatus status = {
        .fast_lock = false,
    };
    sv_tuner_get_status(sv_tuner_get(tunerNum), &status);
    if (status.params.mux_id.freq > 0) {
        SvString freq = SvStringCreateWithFormat("%i MHz", status.params.mux_id.freq / 1000);
        QBDiagnosticsWidgetElementSetText(element, SvStringCString(freq));
        SVRELEASE(freq);
    }
}

SvLocal void
QBDiagnosticsWidgetSetMpegServiceID(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_debug("Wrong params(self:%p)", self);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "mpegService");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    int sid = QBDiagnosticsGetSID(self->diagnostics);
    if (sid < 0) {
        QBDiagnosticsWidgetElementSetText(element, NULL);
        return;
    }
    char* val = NULL;
    asprintf(&val, "%d", sid);
    if (val) {
        QBDiagnosticsWidgetElementSetText(element, val);
        free(val);
    }
}


SvLocal void
QBDiagnosticsWidgetSetMuxId(QBDiagnosticsWidget self) {

    if (!self || !self->diagnostics || !self->appGlobals->tvLogic) {
        log_debug("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "muxID");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    int tunerNum = QBTVLogicGetTunerNum(self->appGlobals->tvLogic);
    if (tunerNum < 0) {
        return;
    }
    struct QBTunerStatus status = {
        .fast_lock = false,
    };
    sv_tuner_get_status(sv_tuner_get(tunerNum), &status);

    if (status.params.mux_id.freq > 0) {
        SvString name = freqToName(status.params.mux_id.freq);
        if (name)
            QBDiagnosticsWidgetElementSetText(element, SvStringCString(name));
    }
}

SvLocal void
QBDiagnosticsWidgetSetNetworkName(QBDiagnosticsWidget self) {

    if (!self || !self->appGlobals->newTV) {
        log_debug("Wrong params(self:%p, newTV:%p)", self, self ? self->appGlobals->newTV : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "networkName");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    SvTVChannel curChannel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
    if (curChannel && SvObjectIsInstanceOf((SvObject) curChannel, DVBEPGChannel_getType())) {
        DVBEPGChannel channel = (DVBEPGChannel) curChannel;
        if (channel->network_name)
            QBDiagnosticsWidgetElementSetText(element, SvStringCString(channel->network_name));
    }
}

SvLocal void
QBDiagnosticsWidgetSetChannelName(QBDiagnosticsWidget self)
{
    if (!self || !self->appGlobals->newTV) {
        log_debug("Wrong params(self:%p, newTV:%p)", self, self ? self->appGlobals->newTV : NULL);
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "channelName");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    SvTVChannel channel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
    if (channel && channel->name) {
        QBDiagnosticsWidgetElementSetText(element, SvStringCString(channel->name));
    }
}

SvLocal void
QBDiagnosticsWidgetSetChannelNumber(QBDiagnosticsWidget self)
{
    if (!self || !self->appGlobals->newTV || !self->diagnostics) {
        log_debug("Wrong params(self:%p, newTV:%p)", self, self ? self->appGlobals->newTV : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "channelNumber");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    SvTVChannel channel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
    if (channel && channel->name) {
        char* number = NULL;
        if (asprintf(&number, "%d", channel->number)) {
            QBDiagnosticsWidgetElementSetText(element, number);
            free(number);
        }
    }
}

SvLocal void
QBDiagnosticsWidgetSetPLPID(QBDiagnosticsWidget self) {
    if (!self || !self->diagnostics) {
        log_debug("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "plpID");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    int tunerNum = QBTVLogicGetTunerNum(self->appGlobals->tvLogic);
    if (tunerNum < 0) {
        return;
    }
    struct QBTunerStatus status = {
        .fast_lock = false,
    };
    sv_tuner_get_status(sv_tuner_get(tunerNum), &status);

    if (status.params.mux_id.plp_id >= 0) {
        SvString plpStr = SvStringCreateWithFormat("%i", status.params.mux_id.plp_id);
        QBDiagnosticsWidgetElementSetText(element, SvStringCString(plpStr));
        SVRELEASE(plpStr);
    }
}

SvLocal void
QBDiagnosticsWidgetSetMwConnectivity(QBDiagnosticsWidget self)
{
    if (!self || !self->appGlobals->customerInfoMonitor) {
        log_debug("Wrong params(self:%p, interactiveServices:%p)", self, self ? self->appGlobals->customerInfoMonitor : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "mwConnectivity");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    CubiwareMWCustomerInfoStatus status = CubiwareMWCustomerInfoGetStatus(self->appGlobals->customerInfoMonitor);
    QBDiagnosticsWidgetElementSetText(element, status == CubiwareMWCustomerInfoStatus_ok ?
            gettext_noop("Available") : gettext_noop("Unavailable"));
}

SvLocal void
QBDiagnosticsWidgetSetCarrierLock(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_debug("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "carrierLock");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    int carrier_lock = QBDiagnosticsGetCarrierLock(self->diagnostics);
    if (carrier_lock > 0) {
        QBDiagnosticsWidgetElementSetText(element, gettext_noop("Yes"));
    } else if (carrier_lock == 0) {
        QBDiagnosticsWidgetElementSetText(element, gettext_noop("No"));
    } else {
        QBDiagnosticsWidgetElementSetText(element, NULL);
    }
}

SvLocal void
QBDiagnosticsWidgetSetModulation(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "modulation");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    QBTunerModulation modulation = QBDiagnosticsGetModulationMode(self->diagnostics);
    QBDiagnosticsWidgetElementSetText(element, QBTunerModulationToString(modulation));
}

SvLocal void
QBDiagnosticsWidgetSetPolarization(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "polarization");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    QBTunerPolarization polarization = QBDiagnosticsGetPolarizationMode(self->diagnostics);
    QBDiagnosticsWidgetElementSetText(element, QBTunerPolarizationToString(polarization));
}

SvLocal void
QBDiagnosticsWidgetSetSignalLevel(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "signalLevel");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    int carrier_lock = QBDiagnosticsGetCarrierLock(self->diagnostics);
    if (carrier_lock > 0) {
        char *signalLevel = NULL;
        double signalLevel_dBm = (double) QBDiagnosticsGetSignalLevel(self->diagnostics);
        double signalLevel_dBmV = signalLevel_dBm + 48.75; //75 Ohms
        asprintf(&signalLevel, "%fdBm (%fdBmV)", signalLevel_dBm, signalLevel_dBmV);
        if (signalLevel) {
            QBDiagnosticsWidgetElementSetText(element, signalLevel);
            free(signalLevel);
        }
    } else {
        QBDiagnosticsWidgetElementSetText(element, "---");
    }
}

SvLocal void
QBDiagnosticsWidgetSetSignalLevelBar(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "signalLevelBar");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    svGaugeSetValue(element->text, (double) QBDiagnosticsGetSignalStrength(self->diagnostics));
}

SvLocal void
QBDiagnosticsWidgetSetQualityLevelBar(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "qualityLevelBar");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    svGaugeSetValue(element->text, (double) QBDiagnosticsGetQualityLevel(self->diagnostics));
}

SvLocal void
QBDiagnosticsWidgetSetTSID(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "tsid");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    char* tsid = NULL;
    asprintf(&tsid, "%d", QBDiagnosticsGetTSID(self->diagnostics));
    if (tsid) {
        QBDiagnosticsWidgetElementSetText(element, tsid);
        free(tsid);
    }
}

SvLocal void
QBDiagnosticsWidgetSetSimCardStatus(QBDiagnosticsWidget self)
{
    if (!self) {
        log_error("Wrong params(self:%p)", self);
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "simCardStatus");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    QBCASSmartcardState cardStatus = QBCASSmartcardState_incorrect;
    if (self->appGlobals->smartCardMonitor) {
        cardStatus = QBSmartCardMonitorGetSmartCardStatus(self->appGlobals->smartCardMonitor);
    }
    char* status  = NULL;
    if (cardStatus == QBCASSmartcardState_removed) {
        asprintf(&status, gettext_noop("Removed"));
    } else if (cardStatus == QBCASSmartcardState_inserted) {
        asprintf(&status, gettext_noop("Inserted"));
    } else if (cardStatus == QBCASSmartcardState_correct) {
        asprintf(&status, gettext_noop("Correct"));
    } else if (cardStatus == QBCASSmartcardState_incorrect_ex || cardStatus == QBCASSmartcardState_incorrect
        || cardStatus == QBCASSmartcardState_problem) {
        asprintf(&status, gettext_noop("Problem"));
    }
    if (status) {
        QBDiagnosticsWidgetElementSetText(element, status);
        free(status);
    }
}

SvLocal void
QBDiagnosticsWidgetSetPreBER(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "preBER");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    char* preBER = NULL;
    asprintf(&preBER, "%e", (double)QBDiagnosticsGetPreBER(self->diagnostics));
    if (preBER) {
        QBDiagnosticsWidgetElementSetText(element, preBER);
        free(preBER);
    }
}

SvLocal void
QBDiagnosticsWidgetSetDVBTableStatus(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics || !self->appGlobals->newTV) {
        log_debug("Wrong params(self:%p, diagnostics:%p, newtv:%p)", self, self ? self->diagnostics : NULL, self ? self->appGlobals->newTV : NULL);
        return;
    }

    const QBDiagnosticDVBTableStatus *tableStatus = QBDiagnosticsGetDVBTableStatus(self->diagnostics);
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "patStatus");
    if (element) {
        QBDiagnosticsWidgetElementSetText(element, tableStatus->PAT ? gettext_noop("Available") : gettext_noop("Unavailable"));
    }
    element = QBDiagnosticsWidgetElementFind(self, "pmtStatus");
    if (element) {
        QBDiagnosticsWidgetElementSetText(element, tableStatus->PMT ? gettext_noop("Available") : gettext_noop("Unavailable"));
    }
    element = QBDiagnosticsWidgetElementFind(self, "catStatus");
    if (element) {
        QBDiagnosticsWidgetElementSetText(element, tableStatus->CAT ? gettext_noop("Available") : gettext_noop("Unavailable"));
    }
    element = QBDiagnosticsWidgetElementFind(self, "nitStatus");
    if (element) {
        QBDiagnosticsWidgetElementSetText(element, tableStatus->NIT ? gettext_noop("Available") : gettext_noop("Unavailable"));
    }
    element = QBDiagnosticsWidgetElementFind(self, "tdtStatus");
    if (element) {
        int tunerNum = QBTVLogicGetTunerNum(self->appGlobals->tvLogic);
        if (tunerNum >= 0 && SvTimeToMicroseconds(lstvDVBTimeDateGetLastUpdateTime(tunerNum))) {
            QBDiagnosticsWidgetElementSetText(element, gettext_noop("Available"));
        } else {
            QBDiagnosticsWidgetElementSetText(element, gettext_noop("Unavailable"));
        }
    }
    element = QBDiagnosticsWidgetElementFind(self, "sdtStatus");
    if (element) {
        QBDiagnosticsWidgetElementSetText(element, tableStatus->SDT ? gettext_noop("Available") : gettext_noop("Unavailable"));
    }
    element = QBDiagnosticsWidgetElementFind(self, "eitStatus");
    if (element) {
        SvEPGIterator eit = NULL;
        SvTVChannel channel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
        if (channel && SvObjectIsInstanceOf((SvObject) channel, DVBEPGChannel_getType())) {
            eit = SvEPGManagerCreateIterator(self->epgManager, SvDBObjectGetID((SvDBObject) channel), 0);
        }
        if (eit) {
            QBDiagnosticsWidgetElementSetText(element, gettext_noop("Available"));
        } else {
            QBDiagnosticsWidgetElementSetText(element, gettext_noop("Unavailable"));
        }
        SVTESTRELEASE(eit);
    }
}

SvLocal void
QBDiagnosticsWidgetSetSNR(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "snr");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    int carrier_lock = QBDiagnosticsGetCarrierLock(self->diagnostics);
    if (carrier_lock > 0) {
        char *snr = NULL;
        asprintf(&snr, "%idB", (int) QBDiagnosticsGetSNR(self->diagnostics));
        QBDiagnosticsWidgetElementSetText(element, snr);
        if (snr) {
            free(snr);
        }
    } else {
        QBDiagnosticsWidgetElementSetText(element, "---");
    }
}

SvLocal void
QBDiagnosticsWidgetSetChipID(QBDiagnosticsWidget self)
{
    if (!self) {
        log_error("Wrong params(self:%p)", self);
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "chipID");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    SvString chipID = NULL;
    if (self->appGlobals->smartCardMonitor) {
        chipID = QBSmartCardMonitorGetFullChipId(self->appGlobals->smartCardMonitor);
    }
    QBDiagnosticsWidgetElementSetText(element, chipID ? SvStringCString(chipID) : NULL);
}

SvLocal void
QBDiagnosticsWidgetSetShortTermError(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "shortTermError");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    double termError = QBDiagnosticsGetShortTermError(self->diagnostics);
    if (termError >= 0) {
        char* status = NULL;
        asprintf(&status, "%f", termError);
        if (status) {
            QBDiagnosticsWidgetElementSetText(element, status);
            free(status);
        }
    } else {
        QBDiagnosticsWidgetElementSetText(element, NULL);
    }
}

SvLocal void
QBDiagnosticsWidgetSetLongTermError(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ?  self->diagnostics : NULL);
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "longTermError");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    double termError = QBDiagnosticsGetLongTermError(self->diagnostics);
    if (termError >= 0) {
        char* status = NULL;
        asprintf(&status, "%f", termError);
        if (status) {
            QBDiagnosticsWidgetElementSetText(element, status);
            free(status);
        }
    } else {
        QBDiagnosticsWidgetElementSetText(element, NULL);
    }
}

SvLocal void
QBDiagnosticsWidgetSetBandwidth(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "bandwidth");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    SvString band = SvStringCreateWithFormat("%i", QBDiagnosticsGetBandwidth(self->diagnostics));
    QBDiagnosticsWidgetElementSetText(element, SvStringCString(band));
    SVRELEASE(band);
}

SvLocal void
QBDiagnosticsWidgetSetSymbolRate(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "symbolRate");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    int symbolRate = QBDiagnosticsGetSymbolRate(self->diagnostics);
    if (symbolRate >= 0) {
        char* symbol = NULL;
        asprintf(&symbol, "%d", symbolRate);
        if (symbol) {
            QBDiagnosticsWidgetElementSetText(element, symbol);
            free(symbol);
        }
    } else {
        QBDiagnosticsWidgetElementSetText(element, NULL);
    }
}

SvLocal void
QBDiagnosticsWidgetSetRFSignalStatus(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "RFSignalStatus");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    char* signalStatus = NULL;
    int snr = (int) QBDiagnosticsGetSNR(self->diagnostics);
    if (snr >= 37) {
        asprintf(&signalStatus, gettext_noop("Excellent"));
    } else if (snr >= 35 && snr < 37) {
        asprintf(&signalStatus, gettext_noop("Good"));
    } else if (snr >= 32 && snr < 35) {
        asprintf(&signalStatus, gettext_noop("Fair"));
    } else if (snr < 32) {
        asprintf(&signalStatus, gettext_noop("Poor"));
    } else {
        asprintf(&signalStatus, gettext_noop("No Signal Lock"));
    }
    if (signalStatus) {
        QBDiagnosticsWidgetElementSetText(element, signalStatus);
        free(signalStatus);
    }
}

SvLocal void
QBDiagnosticsWidgetSetSimID(QBDiagnosticsWidget self)
{
    if (!self || !self->appGlobals->smartCardMonitor) {
        log_error("Wrong params(self:%p, smart card monitor:%p)", self, self ? self->appGlobals->smartCardMonitor : NULL);
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "simID");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    SvString simID = QBSmartCardMonitorGetSmartCardIdRaw(self->appGlobals->smartCardMonitor);
    QBDiagnosticsWidgetElementSetText(element, simID ? SvStringCString(simID) : NULL);
}

SvLocal void
QBDiagnosticsWidgetGuideDataTimerStep(void *self_)
{
    QBDiagnosticsWidget self = self_;
    SvFiberDeactivate(self->fiberGuideData);
    SvFiberEventDeactivate(self->timerGuideData);
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "guideData");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    if (self->appGlobals->newTV) {
        SvEPGIterator eit = NULL;
        SvTVChannel channel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
        if (channel) {
            eit = SvEPGManagerCreateIterator(self->epgManager, SvDBObjectGetID((SvDBObject) channel), 0);
        }
        if (eit) {
            QBDiagnosticsWidgetElementSetText(element, gettext_noop("Available"));
        } else {
            QBDiagnosticsWidgetElementSetText(element, gettext_noop("Unavailable"));
        }
        SVTESTRELEASE(eit);
    }
    SvFiberTimerActivateAfter(self->timerGuideData, SvTimeFromMs(QB_DIAGNOSTICS_WIDGET_GUIDEDATA_INTERVAL));
}

SvLocal void
QBDiagnosticsWidgetSetChannelMap(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "channelMap");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    const QBDiagnosticDVBTableStatus *tableStatus = QBDiagnosticsGetDVBTableStatus(self->diagnostics);
    QBDiagnosticsWidgetElementSetText(element, tableStatus->PMT ? gettext_noop("Available") : gettext_noop("Unavailable"));
}


SvLocal void
QBDiagnosticsWidgetSetHDMIStatus(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        log_error("Wrong params(self:%p, diagnostics:%p)", self, self ? self->diagnostics : NULL);
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "hdmiStatus");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    bool isConnected = false;
    int outputs = QBPlatformFindOutput(QBOutputType_HDMI, NULL, 0, true, false);
    if (outputs >= 0 && QBPlatformHDMIOutputCheckConnection(outputs, &isConnected) == 0 && isConnected) {
        QBDiagnosticsWidgetElementSetText(element, gettext_noop("Enabled"));
    } else {
        QBDiagnosticsWidgetElementSetText(element, gettext_noop("Disabled"));
    }
}

SvLocal void
QBDiagnosticsWidgetSetUSBStatus(QBDiagnosticsWidget self)
{
    if (!self) {
        return;
    }
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "usbStatus");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    bool isWiFiPresent = false;
    bool isUSBInserted = false;
    if (self->appGlobals->net_manager) {
        SvArray interaceTab = QBNetManagerCreateInterfacesList(self->appGlobals->net_manager, NULL);
        if (interaceTab) {
            SvIterator it = SvArrayIterator(interaceTab);
            SvString interfaceName = NULL;
            while ((interfaceName = (SvString) SvIteratorGetNext(&it))) {
                if (SvStringEqualToCString(interfaceName, "ra0")) {
                    isWiFiPresent = true;
                    break;
                }
            }
            SVRELEASE(interaceTab);
        }
    }
    if (self->appGlobals->hotplugMountAgent) {
        SvArray disks = QBHotplugMountAgentGetAvailableDisks(self->appGlobals->hotplugMountAgent);
        if (disks && SvArrayCount(disks)) {
            isUSBInserted = true;
        }
    }
    if (isWiFiPresent || isUSBInserted) {
        QBDiagnosticsWidgetElementSetText(element, gettext_noop("Enabled"));
    } else {
        QBDiagnosticsWidgetElementSetText(element, gettext_noop("Disabled"));
    }
}

SvLocal void
QBDiagnosticsWidgetInterfaceStatusStep(void *self_)
{
    QBDiagnosticsWidget self = self_;
    // assert - help for clang false positive
    assert(self);

    QBDiagnosticsWidgetSetHDMIStatus(self);
    QBDiagnosticsWidgetSetIPAddress(self);
    QBDiagnosticsWidgetSetUSBStatus(self);

    SvFiberDeactivate(self->fiberInterfaceStatus);
    SvFiberEventDeactivate(self->timerInterfaceStatus);

    SvFiberTimerActivateAfter(self->timerInterfaceStatus, SvTimeFromMs(QB_DIAGNOSTICS_WIDGET_TIMER_INTERVAL));
}

SvLocal void
QBDiagnosticsWidgetSmartcardState(void *self_, QBCASSmartcardState state)
{
    QBDiagnosticsWidget self = (QBDiagnosticsWidget) self_;
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "simCardSubscriptionStatus");
    if (!element) {
        log_debug("Element not found");
        return;
    }
    char *caption = NULL;
    switch (state) {
    case QBCASSmartcardState_removed:
        caption = strdup(gettext_noop("No Card Inserted"));
        break;
    case QBCASSmartcardState_inserted:
        caption = strdup(gettext_noop("Inserted"));
        break;
    case QBCASSmartcardState_correct:
        caption = strdup(gettext_noop("Good"));
        break;
    case QBCASSmartcardState_incorrect_ex:
        caption = strdup(gettext_noop("Incorrect Conax Card"));
        break;
    case QBCASSmartcardState_incorrect:
        caption = strdup(gettext_noop("Incorrect Card"));
        break;
    case QBCASSmartcardState_problem:
        caption = strdup(gettext_noop("Card Problem - check card"));
        break;
    default:
        break;
    }
    if (caption) {
        QBDiagnosticsWidgetElementSetText(element, caption);
        free(caption);
    }
}

SvLocal char*
QBDiagnosticsWidgetSmartcardErrorCodeToString(int error_code)
{
    char *caption = NULL;
    if (error_code == 0) {
        caption = strdup(gettext_noop("Good"));
    } else {
        asprintf(&caption, "%i", error_code);
    }
    return caption;
}

SvLocal void
QBDiagnosticsWidgetSmartcardEcmStatus(void* self_, uint8_t sessionId, const int16_t pid, QBCASEcmStatus status,
                                      bool is_extra_ecm, int no_access_code, const QBDescramblerCW* cw)
{
    if (!SvPlayerManagerIsPlaybackSession(SvPlayerManagerGetInstance(), sessionId)) {
        return;
    }

    QBDiagnosticsWidget self = (QBDiagnosticsWidget) self_;
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "simCardSubscriptionStatus");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    char *caption = QBDiagnosticsWidgetSmartcardErrorCodeToString(no_access_code);
    if (caption) {
        QBDiagnosticsWidgetElementSetText(element, caption);
        free(caption);
    }
}

SvLocal void
QBDiagnosticsWidgetSetPATDetails(QBDiagnosticsWidget self)
{
    if (!self || !self->appGlobals->newTV || !self->diagnostics) {
        return;
    }
    SvArray pat_table = QBDiagnosticsGetPAT(self->diagnostics);
    if (!pat_table || !SvArrayCount(pat_table)) {
        log_debug("PAT Table is empty");
        return;
    }
    SvTVChannel channel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
    if (!channel || !channel->name) {
        log_debug("Channel(%p) is invalid", channel);
        return;
    }
    int sid = QBDiagnosticsGetSID(self->diagnostics);
    if (sid < 0) {
        return;
    }
    SvArray pmt_table = QBDiagnosticsGetPMT(self->diagnostics);
    if (!pmt_table || !SvArrayCount(pmt_table)) {
        log_debug("PMT Table is empty");
        return;
    }
    SvIterator it = SvArrayIterator(pmt_table);
    QBDiagnosticsPMTElement pmt_val = NULL;
    while ((pmt_val = (QBDiagnosticsPMTElement) SvIteratorGetNext(&it))) {
        if (pmt_val->sid == sid) {
            break;
        }
    }
    if (!pmt_val || pmt_val->sid != sid) {
        log_debug("PAT not found for sid=%d", sid);
        return;
    }
    it = SvArrayIterator(pat_table);
    QBDiagnosticsPATElement pat_val = NULL;
    while ((pat_val = (QBDiagnosticsPATElement) SvIteratorGetNext(&it))) {
        if (pat_val->program_number == pmt_val->program_number) {
            char* text = NULL;
            QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "pid");
            if (element) {
                if (asprintf(&text, "%d", pat_val->PID)) {
                    QBDiagnosticsWidgetElementSetText(element, text);
                    free(text);
                }
            }
            break;
        }
    }
}

SvLocal void
QBDiagnosticsWidgetSetPMTAudio(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        return;
    }
    int sid = QBDiagnosticsGetSID(self->diagnostics);
    if (sid < 0) {
        return;
    }
    SvArray pmt_table = QBDiagnosticsGetPMT(self->diagnostics);
    if (!pmt_table || !SvArrayCount(pmt_table)) {
        log_debug("PMT Table is empty");
        return;
    }
    SvIterator it = SvArrayIterator(pmt_table);
    QBDiagnosticsPMTElement pmt_val = NULL;
    while ((pmt_val = (QBDiagnosticsPMTElement) SvIteratorGetNext(&it))) {
        if (pmt_val->sid == sid) {
            break;
        }
    }
    if (!pmt_val || pmt_val->sid != sid) {
        return;
    }

    // Remove old ones
    SvArray array = (SvArray) SvHashTableFind(self->diagnosticsWidgets, (SvObject) SVSTRING("pmtAudio"));
    if (array) {
        QBDiagnosticsWidgetElement element = NULL;
        it = SvArrayIterator(array);
        while ((element = (QBDiagnosticsWidgetElement) SvIteratorGetNext(&it))) {
            QBGridRemoveWidget(self->content, element->w);
        }
        SvHashTableRemove(self->diagnosticsWidgets, (SvObject) SVSTRING("pmtAudio"));
    }
    if (!pmt_val->audio || !SvArrayCount(pmt_val->audio)) {
        return;
    }

    it = SvArrayIterator(pmt_val->audio);
    QBDiagnosticsPMTAudio audio_pmt = NULL;
    char *text = NULL;
    SvArray new_audio = SvArrayCreate(NULL);

    QBDiagnosticsWidgetElement element = NULL;
    while ((audio_pmt = (QBDiagnosticsPMTAudio) SvIteratorGetNext(&it))) {
        element = QBDiagnosticsWidgetCombineWindowElement(self, gettext_noop("Audio Codec"), QBAudioCodecToString(audio_pmt->codec), NULL);
        if (element) {
            SvArrayAddObject(new_audio, (SvObject) element);
            QBGridAddWidget(self->content, element->w);
            SVRELEASE(element);
        }
        element = QBDiagnosticsWidgetCombineWindowElement(self, gettext_noop("Audio Type"), QBAudioTypeToString(audio_pmt->type), NULL);
        if (element) {
            SvArrayAddObject(new_audio, (SvObject) element);
            QBGridAddWidget(self->content, element->w);
            SVRELEASE(element);
        }
        element = QBDiagnosticsWidgetCombineWindowElement(self, gettext_noop("Audio Lang"), audio_pmt->lang ? gettext(audio_pmt->lang) : gettext_noop("N/A"), NULL);
        if (element) {
            SvArrayAddObject(new_audio, (SvObject) element);
            QBGridAddWidget(self->content, element->w);
            SVRELEASE(element);
        }
        element = QBDiagnosticsWidgetCombineWindowElement(self, gettext_noop("Audio DMM"), QBAudioDualMonoModeToString(audio_pmt->dual_mono_mode), NULL);
        if (element) {
            SvArrayAddObject(new_audio, (SvObject) element);
            QBGridAddWidget(self->content, element->w);
            SVRELEASE(element);
        }
        asprintf(&text, "%d", audio_pmt->pid);
        element = QBDiagnosticsWidgetCombineWindowElement(self, gettext_noop("Audio PID"), text, NULL);
        if (element) {
            SvArrayAddObject(new_audio, (SvObject) element);
            QBGridAddWidget(self->content, element->w);
            SVRELEASE(element);
        }
        free(text);
    }
    SvHashTableInsert(self->diagnosticsWidgets, (SvObject) SVSTRING("pmtAudio"), (SvObject) new_audio);
    SVRELEASE(new_audio);

    svViewPortContentsSizeChanged(self->viewport);
}

SvLocal char*
format_type_to_string(enum svdataformattype type)
{
    char* video_type = NULL;
    switch (type) {
    case svdataformattype_ts:
        video_type = strdup("TS");
        break;
    case svdataformattype_asfpl:
        video_type = strdup("ASF");
        break;
    case svdataformattype_es:
        video_type = strdup("ES");
        break;
    case svdataformattype_direct_es:
        video_type = strdup("Direct ES");
        break;
    default:
        video_type = strdup("Uninitialized");
        break;
    }
    return video_type;
}

SvLocal void
QBDiagnosticsWidgetSetPMTVideo(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        return;
    }
    int sid = QBDiagnosticsGetSID(self->diagnostics);
    if (sid < 0) {
        return;
    }
    SvArray pmt_table = QBDiagnosticsGetPMT(self->diagnostics);
    if (!pmt_table || !SvArrayCount(pmt_table)) {
        log_debug("PMT Table is empty");
        return;
    }
    SvIterator it = SvArrayIterator(pmt_table);
    QBDiagnosticsPMTElement pmt_val = NULL;
    while ((pmt_val = (QBDiagnosticsPMTElement) SvIteratorGetNext(&it))) {
        if (pmt_val->sid == sid) {
            break;
        }
    }
    if (!pmt_val || pmt_val->sid != sid) {
        return;
    }

    SvArray array = (SvArray) SvHashTableFind(self->diagnosticsWidgets, (SvObject) SVSTRING("pmtVideo"));
    QBDiagnosticsWidgetElement element = NULL;
    if (array) { // Remove old ones
        it = SvArrayIterator(array);
        while ((element = (QBDiagnosticsWidgetElement) SvIteratorGetNext(&it))) {
            QBGridRemoveWidget(self->content, element->w); // detach
        }
        SvHashTableRemove(self->diagnosticsWidgets, (SvObject) SVSTRING("pmtVideo"));
    }
    if (!pmt_val->video || !SvArrayCount(pmt_val->video)) {
        return;
    }

    SvArray new_videos = SvArrayCreate(NULL);
    it = SvArrayIterator(pmt_val->video);
    QBDiagnosticsPMTVideo video_pmt = NULL;
    char* text = NULL;

    while ((video_pmt = (QBDiagnosticsPMTVideo) SvIteratorGetNext(&it))) {
        element = QBDiagnosticsWidgetCombineWindowElement(self, gettext_noop("Video Codec"), video_pmt->codec, NULL);
        if (element) {
            SvArrayAddObject(new_videos, (SvObject) element);
            QBGridAddWidget(self->content, element->w);
            SVRELEASE(element);
        }
        text = format_type_to_string(video_pmt->type);
        element = QBDiagnosticsWidgetCombineWindowElement(self, gettext_noop("Video Type"), text, NULL);
        if (element) {
            SvArrayAddObject(new_videos, (SvObject) element);
            QBGridAddWidget(self->content, element->w);
            SVRELEASE(element);
        }
        free(text);
        asprintf(&text, "%d", video_pmt->pid);
        element = QBDiagnosticsWidgetCombineWindowElement(self, gettext_noop("Video PID"), text, NULL);
        if (element) {
            SvArrayAddObject(new_videos, (SvObject) element);
            QBGridAddWidget(self->content, element->w);
            SVRELEASE(element);
        }
        free(text);
    }
    SvHashTableInsert(self->diagnosticsWidgets, (SvObject) SVSTRING("pmtVideo"), (SvObject) new_videos);
    SVRELEASE(new_videos);

    svViewPortContentsSizeChanged(self->viewport);
}

SvLocal void
QBDiagnosticsWidgetSetPMTDetails(QBDiagnosticsWidget self)
{
    if (!self || !self->diagnostics) {
        return;
    }
    int sid = QBDiagnosticsGetSID(self->diagnostics);
    if (sid < 0) {
        return;
    }
    SvArray pmt_table = QBDiagnosticsGetPMT(self->diagnostics);
    if (!pmt_table || !SvArrayCount(pmt_table)) {
        log_debug("PMT Table is empty");
        return;
    }
    SvIterator it = SvArrayIterator(pmt_table);
    QBDiagnosticsPMTElement pmt_val = NULL;
    while ((pmt_val = (QBDiagnosticsPMTElement) SvIteratorGetNext(&it))) {
        if (pmt_val->sid == sid) {
            break;
        }
    }
    if (!pmt_val || pmt_val->sid != sid) {
        log_debug("pmt not found for sid=%d", sid);
        return;
    }
    char* text = NULL;
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "sid");
    if (element) {
        if (asprintf(&text, "%d", pmt_val->sid)) {
            QBDiagnosticsWidgetElementSetText(element, text);
            free(text);
        }
    }
    element = QBDiagnosticsWidgetElementFind(self, "pcr");
    if (element) {
        if (asprintf(&text, "%d", pmt_val->pcr_pid)) {
            QBDiagnosticsWidgetElementSetText(element, text);
            free(text);
        }
    }
    element = QBDiagnosticsWidgetElementFind(self, "programNumber");
    if (element) {
        if (asprintf(&text, "%d", pmt_val->program_number)) {
            QBDiagnosticsWidgetElementSetText(element, text);
            free(text);
        }
    }
    element = QBDiagnosticsWidgetElementFind(self, "pmtVersionNumber");
    if (element) {
        if (asprintf(&text, "%d", pmt_val->version_number)) {
            QBDiagnosticsWidgetElementSetText(element, text);
            free(text);
        }
    }
    svViewPortContentsSizeChanged(self->viewport);
}

SvLocal void
QBDiagnosticsWidgetStbStatusStep(void *self_)
{
    QBDiagnosticsWidget self = self_;

    SvFiberDeactivate(self->fiberStbStatus);
    SvFiberEventDeactivate(self->timerStbStatus);

    const QBDiagnosticProcStat *stat = QBDiagnosticsGetCpuMemoryStatus(self->diagnostics);
    if (!stat) {
        goto fini;
    }
    char val[20];
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "uptime");
    if (element && snprintf(val, sizeof(val), "%d sec", (int)stat->uptimeSec) > 0) {
        QBDiagnosticsWidgetElementSetText(element, val);
    }
    element = QBDiagnosticsWidgetElementFind(self, "idleTime");
    if (element && snprintf(val, sizeof(val), "%d sec", (int)stat->idleSec) > 0) {
        QBDiagnosticsWidgetElementSetText(element, val);
    }
    element = QBDiagnosticsWidgetElementFind(self, "userTime");
    if (element && snprintf(val, sizeof(val), "%3.2f sec", stat->userTimeSec) > 0) {
        QBDiagnosticsWidgetElementSetText(element, val);
    }
    element = QBDiagnosticsWidgetElementFind(self, "systemTime");
    if (element && snprintf(val, sizeof(val), "%3.2f sec", stat->kernelTimeSec) > 0) {
        QBDiagnosticsWidgetElementSetText(element, val);
    }
    element = QBDiagnosticsWidgetElementFind(self, "threadCount");
    if (element &&  snprintf(val, sizeof(val), "%lu", stat->num_threads) > 0) {
        QBDiagnosticsWidgetElementSetText(element, val);
    }
    element = QBDiagnosticsWidgetElementFind(self, "virtualMemory");
    if (element &&  snprintf(val, sizeof(val), "%lu kB", stat->vsize / 1024) > 0) {
        QBDiagnosticsWidgetElementSetText(element, val);
    }
    element = QBDiagnosticsWidgetElementFind(self, "vmRss");
    if (element &&  snprintf(val, sizeof(val), "%lu", stat->rss) > 0) {
        QBDiagnosticsWidgetElementSetText(element, val);
    }
    element = QBDiagnosticsWidgetElementFind(self, "totalMem");
    if (element &&  snprintf(val, sizeof(val), "%lu kB", stat->totalMem) > 0) {
        QBDiagnosticsWidgetElementSetText(element, val);
    }
    element = QBDiagnosticsWidgetElementFind(self, "usedMem");
    if (element &&  snprintf(val, sizeof(val), "%lu kB", stat->usedMem) > 0) {
        QBDiagnosticsWidgetElementSetText(element, val);
    }
    element = QBDiagnosticsWidgetElementFind(self, "freeMem");
    if (element &&  snprintf(val, sizeof(val), "%lu kB", stat->freeMem) > 0) {
        QBDiagnosticsWidgetElementSetText(element, val);
    }
    element = QBDiagnosticsWidgetElementFind(self, "buffMem");
    if (element &&  snprintf(val, sizeof(val), "%lu kB", stat->buffMem) > 0) {
        QBDiagnosticsWidgetElementSetText(element, val);
    }
fini:
    SvFiberTimerActivateAfter(self->timerStbStatus, SvTimeFromMs(QB_DIAGNOSTICS_WIDGET_TIMER_INTERVAL));
}

SvLocal void
QBDiagnosticsWidgetAudioVideoStatusTimerStep(void *self_)
{
    QBDiagnosticsWidget self = self_;
    SvFiberDeactivate(self->fiberAudioVideoStatus);
    SvFiberEventDeactivate(self->timerAudioVideoStatus);
    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "audioStatus");
    if (element) {
        if (QBDiagnosticsIsAudioAvailable(self->diagnostics)) {
            QBDiagnosticsWidgetElementSetText(element, gettext_noop("OK"));
        } else {
            QBDiagnosticsWidgetElementSetText(element, gettext_noop("Error"));
        }
    }
    element = QBDiagnosticsWidgetElementFind(self, "videoStatus");
    if (element) {
        if (QBDiagnosticsIsVideoAvailable(self->diagnostics)) {
            QBDiagnosticsWidgetElementSetText(element, gettext_noop("OK"));
        } else {
            QBDiagnosticsWidgetElementSetText(element, gettext_noop("Error"));
        }
    }
    SvFiberTimerActivateAfter(self->timerAudioVideoStatus, SvTimeFromMs(QB_DIAGNOSTICS_WIDGET_AUDIOVIDEOSTATUS_INTERVAL));
}

SvLocal void
QBDiagnosticsWidgetSetExtraSwVersion(QBDiagnosticsWidget self)
{
    if (!self) {
        log_error("Wrong params(self:%p)", self);
        return;
    }

    QBDiagnosticsWidgetElement element = QBDiagnosticsWidgetElementFind(self, "extraSwVersion");
    if (!element) {
        log_debug("Element not found");
        return;
    }

    SvData data = SvDataCreateFromFile(QB_DIAGNOSTICS_WIDGET_SRM_PROJECT_FILE_PATH, -1, NULL);
    if (!data) {
        QBDiagnosticsWidgetElementSetText(element, "Unknown");
        log_warning("Unable to open SRM_PROJECT file (%s)", QB_DIAGNOSTICS_WIDGET_SRM_PROJECT_FILE_PATH);
        return;
    }

    char *swVersion = SvDataGetBytes(data);
    swVersion[SvDataGetLength(data)] = 0;
    QBDiagnosticsWidgetElementSetText(element, swVersion);
    SVRELEASE(data);
}

SvLocal void
QBDiagnosticsWidgetElementCleanup(SvApplication app, void *self_)
{
    QBDiagnosticsWidgetElement self = self_;

    self->w = NULL;
}


SvLocal void
QBDiagnosticsWidgetElementInit(QBDiagnosticsWidget self,
                               QBDiagnosticsWidgetElement element,
                               SvApplication app,
                               const char* label,
                               const char* text,
                               const char* id)
{
    svSettingsRestoreContext(self->settingsCtx);

    char buf[256];
    snprintf(buf, 256, "%s.widget", self->name);

    if (self->useRatioSettings)
        element->w = svSettingsWidgetCreateWithRatio(app, buf, self->viewport->width, self->viewport->height);
    else
        element->w = svSettingsWidgetCreate(app, buf);

    element->w->prv = element;
    element->w->clean = QBDiagnosticsWidgetElementCleanup;

    QBTextRenderer renderer = self->appGlobals->textRenderer;

    if (id && (strcmp(id, "signalLevelBar") == 0 || strcmp(id, "qualityLevelBar") == 0)) {
        snprintf(buf, 256, "%s.widget.Gauge", self->name);
        if (self->useRatioSettings) {
            element->text = svGaugeNewFromRatio(app, element->w->width, element->w->height, buf, SV_GAUGE_HORIZONTAL, SV_GAUGE_STYLE_BASIC, 0, 100, 0);
            if (element->text) {
                svSettingsWidgetAttachWithRatio(element->w, element->text, buf, 1);
            }
        } else {
            element->text = svGaugeNewFromSM(app, buf, SV_GAUGE_HORIZONTAL, SV_GAUGE_STYLE_BASIC, 0, 100, 0);
            if (element->text) {
                svSettingsWidgetAttach(element->w, element->text, buf, 1);
            }
        }
    } else {
        snprintf(buf, 256, "%s.widget.text", self->name);
        if (self->useRatioSettings) {
            element->text = QBAsyncLabelNewFromRatioSettings(app, element->w->width, element->w->height, buf,
                                                             renderer, NULL);
            if (element->text) {
                QBDiagnosticsWidgetElementSetText(element, text);
                svSettingsWidgetAttachWithRatio(element->w, element->text, buf, 1);
            }
        } else {
            element->text = QBAsyncLabelNew(app, buf, renderer);
            if (element->text) {
                QBDiagnosticsWidgetElementSetText(element, text);
                svSettingsWidgetAttach(element->w, element->text, buf, 1);
            }
        }
    }

    snprintf(buf, 256, "%s.widget.label", self->name);
    if (self->useRatioSettings) {
        element->label = QBAsyncLabelNewFromRatioSettings(app, element->w->width, element->w->height, buf, renderer,
                                                          NULL);
        if (element->label) {
            if (label) {
                QBDiagnosticsWidgetElementSetLabel(element, label);
            }
            svSettingsWidgetAttachWithRatio(element->w, element->label, buf, 1);
        }
    } else {
        element->label = QBAsyncLabelNew(app, buf, renderer);
        if (element->label) {
            if (label) {
                QBDiagnosticsWidgetElementSetLabel(element, label);
            }
            svSettingsWidgetAttach(element->w, element->label, buf, 1);
        }
    }

    svSettingsPopComponent();
}

SvLocal QBDiagnosticsWidgetElement
QBDiagnosticsWidgetCombineWindowElement(QBDiagnosticsWidget self, const char* labelStr, const char* textStr, const char* idStr)
{
    if (!self || !labelStr) {
        return NULL;
    }

    QBDiagnosticsWidgetElement element = NULL;
    if (!(element = (QBDiagnosticsWidgetElement) SvTypeAllocateInstance(QBDiagnosticsWidgetElement_getType(), NULL))) {
        return NULL;
    }
    QBDiagnosticsWidgetElementInit(self, element, self->appGlobals->res, labelStr, textStr, idStr);

    return element;
}


SvLocal DiagnosticsWindowSetter
get_setter(SvString id)
{
    if (SvStringEqualToCString(id, "macAddress")) {
        return QBDiagnosticsWidgetSetMacAddress;
    } else if (SvStringEqualToCString(id, "ipAddress")) {
        return QBDiagnosticsWidgetSetIPAddress;
    } else if (SvStringEqualToCString(id, "deviceName")) {
        return QBDiagnosticsWidgetSetDeviceName;
    } else if (SvStringEqualToCString(id, "swVersion")) {
        return QBDiagnosticsWidgetSetFirmwareVersion;
    } else if (SvStringEqualToCString(id, "tunedFrequency")) {
        return QBDiagnosticsWidgetSetTunedFrequency;
    } else if (SvStringEqualToCString(id, "mpegService")) {
        return QBDiagnosticsWidgetSetMpegServiceID;
    } else if (SvStringEqualToCString(id, "muxID")) {
        return QBDiagnosticsWidgetSetMuxId;
    } else if (SvStringEqualToCString(id, "networkName")) {
        return QBDiagnosticsWidgetSetNetworkName;
    } else if (SvStringEqualToCString(id, "channelName")) {
        return QBDiagnosticsWidgetSetChannelName;
    } else if (SvStringEqualToCString(id, "channelNumber")) {
        return QBDiagnosticsWidgetSetChannelNumber;
    } else if (SvStringEqualToCString(id, "plpID")) {
        return QBDiagnosticsWidgetSetPLPID;
    } else if (SvStringEqualToCString(id, "carrierLock")) {
        return QBDiagnosticsWidgetSetCarrierLock;
    } else if (SvStringEqualToCString(id, "signalLevel")) {
        return QBDiagnosticsWidgetSetSignalLevel;
    } else if (SvStringEqualToCString(id, "signalLevelBar")) {
        return QBDiagnosticsWidgetSetSignalLevelBar;
    } else if (SvStringEqualToCString(id, "qualityLevelBar")) {
        return QBDiagnosticsWidgetSetQualityLevelBar;
    } else if (SvStringEqualToCString(id, "tsid")) {
        return QBDiagnosticsWidgetSetTSID;
    } else if (SvStringEqualToCString(id, "simCardStatus")) {
        return QBDiagnosticsWidgetSetSimCardStatus;
    } else if (SvStringEqualToCString(id, "modulation")) {
        return QBDiagnosticsWidgetSetModulation;
    } else if (SvStringEqualToCString(id, "polarization")) {
        return QBDiagnosticsWidgetSetPolarization;
    } else if (SvStringEqualToCString(id, "preBER")) {
        return QBDiagnosticsWidgetSetPreBER;
    } else if (SvStringEqualToCString(id, "snr")) {
        return QBDiagnosticsWidgetSetSNR;
    } else if (SvStringEqualToCString(id, "chipID")) {
        return QBDiagnosticsWidgetSetChipID;
    } else if (SvStringEqualToCString(id, "shortTermError")) {
        return QBDiagnosticsWidgetSetShortTermError;
    } else if (SvStringEqualToCString(id, "longTermError")) {
        return QBDiagnosticsWidgetSetLongTermError;
    } else if (SvStringEqualToCString(id, "bandwidth")) {
        return QBDiagnosticsWidgetSetBandwidth;
    } else if (SvStringEqualToCString(id, "symbolRate")) {
        return QBDiagnosticsWidgetSetSymbolRate;
    } else if (SvStringEqualToCString(id, "RFSignalStatus")) {
        return QBDiagnosticsWidgetSetRFSignalStatus;
    } else if (SvStringEqualToCString(id, "simID")) {
        return QBDiagnosticsWidgetSetSimID;
    } else if (SvStringEqualToCString(id, "channelMap")) {
        return QBDiagnosticsWidgetSetChannelMap;
    } else if (SvStringEqualToCString(id, "hdmiStatus")) {
        return QBDiagnosticsWidgetSetHDMIStatus;
    } else if (SvStringEqualToCString(id, "ethernetStatus")) {
        return QBDiagnosticsWidgetSetIPAddress;
    } else if (SvStringEqualToCString(id, "usbStatus")) {
        return QBDiagnosticsWidgetSetUSBStatus;
    } else if (SvStringEqualToCString(id, "mwConnectivity")) {
        return QBDiagnosticsWidgetSetMwConnectivity;
    } else if (SvStringEqualToCString(id, "pid")) {
        return QBDiagnosticsWidgetSetPATDetails;
    } else if (SvStringEqualToCString(id, "sid")
            || SvStringEqualToCString(id, "pcr")
            || SvStringEqualToCString(id, "programNumber")
            || SvStringEqualToCString(id, "pmtVersionNumber"))
    {
        return QBDiagnosticsWidgetSetPMTDetails;
    } else if (SvStringEqualToCString(id, "pmtAudio")) {
        return QBDiagnosticsWidgetSetPMTAudio;
    } else if (SvStringEqualToCString(id, "pmtVideo")) {
        return QBDiagnosticsWidgetSetPMTVideo;
    } else if (SvStringEqualToCString(id, "patStatus")
            || SvStringEqualToCString(id, "pmtStatus")
            || SvStringEqualToCString(id, "catStatus")
            || SvStringEqualToCString(id, "nitStatus")
            || SvStringEqualToCString(id, "tdtStatus")
            || SvStringEqualToCString(id, "sdtStatus")
            || SvStringEqualToCString(id, "eitStatus"))
    {
        return QBDiagnosticsWidgetSetDVBTableStatus;
    } else if (SvStringEqualToCString(id, "extraSwVersion")) {
        return QBDiagnosticsWidgetSetExtraSwVersion;
    }
    return NULL;
}

SvLocal void
QBDiagnosticsWidgetHandleDiagnostic(QBDiagnosticsWidget self, SvString id, SvString label)
{
    svSettingsRestoreContext(self->settingsCtx);
    DiagnosticsWindowSetter setter = NULL;

    QBDiagnosticsWidgetElement element = NULL;
    if (label) {
        element = QBDiagnosticsWidgetCombineWindowElement(self, SvStringCString(label), NULL, SvStringCString(id));
        if (element) {
            SvHashTableInsert(self->diagnosticsWidgets, (SvObject)id, (SvObject) element);
            QBGridAddWidget(self->content, element->w);
        }
    } else if (SvStringEqualToCString(id, "spacer")) {
        // add spacer
        svSettingsRestoreContext(self->settingsCtx);

        char buf[256];
        snprintf(buf, 256, "%s.widget", self->name);

        SvWidget widget = NULL;
        if (self->useRatioSettings)
            widget = svSettingsWidgetCreateWithRatio(self->appGlobals->res, buf, self->viewport->width, self->viewport->height);
        else
            widget = svSettingsWidgetCreate(self->appGlobals->res, buf);

        QBGridAddWidget(self->content, widget);

        svSettingsPopComponent();
        goto fini;
    }

    // let customer logic handle diagnost
    if (element && QBDiagnosticsLogicHandleDiagnostic(self->appGlobals->diagnosticsLogic, id, element->text)) {
        // logic handles this diagnostic
        goto fini;
    }

    setter = get_setter(id);
    if (setter) {
        setter(self);
    }

    // handle special cases
    if (SvStringEqualToCString(id, "mwConnectivity")) {
        CubiwareMWCustomerInfoAddListener(self->appGlobals->customerInfoMonitor, (SvObject) self);
        CubiwareMWCustomerInfoRefresh(self->appGlobals->customerInfoMonitor);
    } else if (SvStringEqualToCString(id, "audioStatus") || SvStringEqualToCString(id, "videoStatus")) {
        if (!self->fiberAudioVideoStatus) {
            self->fiberAudioVideoStatus = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBDiagnosticsWidgetAvStatus", QBDiagnosticsWidgetAudioVideoStatusTimerStep, self);
            self->timerAudioVideoStatus = SvFiberTimerCreate(self->fiberAudioVideoStatus);
        }
        SvFiberActivate(self->fiberAudioVideoStatus);
    } else if (SvStringEqualToCString(id, "guideData")) {
        if (!self->fiberGuideData) {
            self->fiberGuideData = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBDiagnosticsWidgetGuideDateStep", QBDiagnosticsWidgetGuideDataTimerStep, self);
            self->timerGuideData = SvFiberTimerCreate(self->fiberGuideData);
        }
        SvFiberActivate(self->fiberGuideData);

    } else if (SvStringEqualToCString(id, "simCardSubscriptionStatus")) {
        QBCASSmartcardState state = SvInvokeInterface(QBCAS, QBCASGetInstance(), getState);
        QBDiagnosticsWidgetSmartcardState(self, state);
    } else if (SvStringEqualToCString(id, "uptime")
            || SvStringEqualToCString(id, "idleTime")
            || SvStringEqualToCString(id, "userTime")
            || SvStringEqualToCString(id, "systemTime")
            || SvStringEqualToCString(id, "threadCount")
            || SvStringEqualToCString(id, "virtualMemory")
            || SvStringEqualToCString(id, "vmRss")
            || SvStringEqualToCString(id, "totalMem")
            || SvStringEqualToCString(id, "usedMem")
            || SvStringEqualToCString(id, "freeMem")
            || SvStringEqualToCString(id, "buffMem"))
    {
        if (!self->fiberStbStatus) {
            self->fiberStbStatus = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBDiagnosticsWidgetStbStatus", QBDiagnosticsWidgetStbStatusStep, self);
            self->timerStbStatus = SvFiberTimerCreate(self->fiberStbStatus);
        }
        SvFiberActivate(self->fiberStbStatus);
    } else if (SvStringEqualToCString(id, "hdmiStatus")
            || SvStringEqualToCString(id, "ethernetStatus")
            || SvStringEqualToCString(id, "usbStatus"))
    {
        if (!self->fiberInterfaceStatus) {
            self->fiberInterfaceStatus = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBDiagnosticsWidgetInterfaceStatus", QBDiagnosticsWidgetInterfaceStatusStep, self);
            self->timerInterfaceStatus = SvFiberTimerCreate(self->fiberInterfaceStatus);
        }
        SvFiberActivate(self->fiberInterfaceStatus);
    }

fini:
    SVTESTRELEASE(element);

    svSettingsPopComponent();
}

SvLocal void
QBDiagnosticsWidgetDestroy(void *self_)
{
    QBDiagnosticsWidget self = self_;

    if (self->appGlobals->diagnosticsLogic) {
        QBDiagnosticsLogicWindowDestroyed(self->appGlobals->diagnosticsLogic);
    }

    if (self->fiberStbStatus) {
        SvFiberDestroy(self->fiberStbStatus);
        self->fiberStbStatus = NULL;
    }

    if (self->fiberInterfaceStatus) {
        SvFiberDestroy(self->fiberInterfaceStatus);
        self->fiberInterfaceStatus = NULL;
    }

    if (self->fiberGuideData) {
        SvFiberDestroy(self->fiberGuideData);
        self->fiberGuideData = NULL;
    }

    if (self->fiberAudioVideoStatus) {
        SvFiberDestroy(self->fiberAudioVideoStatus);
        self->fiberAudioVideoStatus = NULL;
    }

    if (self->diagnostics) {
        QBDiagnosticsRemoveListener(self->diagnostics, (SvObject) self);
    }

    if (self->appGlobals->smartCardMonitor) {
        QBSmartCardMonitorRemoveListener(self->appGlobals->smartCardMonitor, (SvObject) self);
    }

    if (self->appGlobals->hotplugMountAgent) {
        QBHotplugMountAgentRemoveListener(self->appGlobals->hotplugMountAgent, (SvObject) self);
    }

    if (self->appGlobals->natMonitor) {
        QBNATMonitorRemoveObserver(self->appGlobals->natMonitor, (SvObject) self, NULL);
    }

    SVRELEASE(self->conf);
    SVRELEASE(self->diagnosticsWidgets);

    SvInvokeInterface(QBCAS, QBCASGetInstance(), removeCallbacks, (QBCASCallbacks)&smartCardCallbacks, (SvObject) self);

    if(self->diagnostics)
        QBDiagnosticsStop(self->diagnostics);
    SVTESTRELEASE(self->diagnostics);
    self->diagnostics = NULL;

    free(self->name);
    SVTESTRELEASE(self->epgManager);
}

SvLocal void
QBDiagnosticsWidgetNetworkSettingsChanged(SvObject self_, SvString iface, uint64_t mask)
{
    QBDiagnosticsWidget self = (QBDiagnosticsWidget) self_;
    if (mask & (1ll << QBNetAttr_IPv4_address)) {
        QBDiagnosticsWidgetSetIPAddress(self);
    }
    if (mask & (1ll << QBNetAttr_hwAddr)) {
        QBDiagnosticsWidgetSetMacAddress(self);
    }
    if (mask & (1ll << QBNetAttr_name) && !strcmp(SvStringCString(iface), "ra0")) {
        QBDiagnosticsWidgetSetUSBStatus(self);
    }
}

SvLocal void
QBDiagnosticsWidgetCustomerInfoMonitorInfoChanged(SvObject self_, SvString customerId, SvHashTable customerInfo)
{
}

SvLocal void
QBDiagnosticsWidgetCustomerInfoMonitorStatusChanged(SvObject self_)
{
    QBDiagnosticsWidget self = (QBDiagnosticsWidget) self_;
    QBDiagnosticsWidgetSetMwConnectivity(self);
}

SvLocal void
QBDiagnosticsWidgetDiagnosticsStateChanged(SvObject self_, QBDiagnosticsChangeType type)
{
    if (!self_) {
        return;
    }
    QBDiagnosticsWidget self = (QBDiagnosticsWidget) self_;
    switch (type) {
    case QBDiagnosticsChangeType_CarrierLock:
        QBDiagnosticsWidgetSetCarrierLock(self);
        break;
    case QBDiagnosticsChangeType_Modulation:
        QBDiagnosticsWidgetSetModulation(self);
        break;
    case QBDiagnosticsChangeType_Polarization:
        QBDiagnosticsWidgetSetPolarization(self);
        break;
    case QBDiagnosticsChangeType_SignalLevel:
        QBDiagnosticsWidgetSetSignalLevel(self);
        break;
    case QBDiagnosticsChangeType_SignalStrength:
        QBDiagnosticsWidgetSetSignalLevelBar(self);
        break;
    case QBDiagnosticsChangeType_QualityLevel:
        QBDiagnosticsWidgetSetQualityLevelBar(self);
        break;
    case QBDiagnosticsChangeType_PreBER:
        QBDiagnosticsWidgetSetPreBER(self);
        break;
    case QBDiagnosticsChangeType_SNR:
        QBDiagnosticsWidgetSetSNR(self);
        QBDiagnosticsWidgetSetRFSignalStatus(self);
        break;
    case QBDiagnosticsChangeType_DVBTableRepeated:
        // todo Reduce calls in this section
        QBDiagnosticsWidgetSetDVBTableStatus(self);
        if (self->fiberGuideData) {
            SvFiberActivate(self->fiberGuideData);
        }
        break;
    case QBDiagnosticsChangeType_PATDVBTableChanged:
        QBDiagnosticsWidgetSetChannelMap(self);
        QBDiagnosticsWidgetSetPATDetails(self);
        QBDiagnosticsWidgetSetDVBTableStatus(self);
        break;
    case QBDiagnosticsChangeType_PMTDVBTableChanged:
        QBDiagnosticsWidgetSetChannelMap(self);
        QBDiagnosticsWidgetSetDVBTableStatus(self);
        QBDiagnosticsWidgetSetPMTDetails(self);

        if (QBDiagnosticsWindowConfIsEnabled(self->conf, SVSTRING("pmtAudio"))) {
            QBDiagnosticsWidgetSetPMTAudio(self);
        }
        if (QBDiagnosticsWindowConfIsEnabled(self->conf, SVSTRING("pmtVideo"))) {
            QBDiagnosticsWidgetSetPMTVideo(self);
        }
        break;
    case QBDiagnosticsChangeType_ShortTermError:
        QBDiagnosticsWidgetSetShortTermError(self);
        break;
    case QBDiagnosticsChangeType_LongTermError:
        QBDiagnosticsWidgetSetLongTermError(self);
        break;
    case QBDiagnosticsChangeType_Bandwidth:
        QBDiagnosticsWidgetSetBandwidth(self);
        break;
    case QBDiagnosticsChangeType_SymbolRate:
        QBDiagnosticsWidgetSetSymbolRate(self);
        break;
    default:
        log_warning("QBDiagnosticsListener notification ignored (type:%d)", (int) type);
        break;
    }
}

SvLocal void
QBDiagnosticsWidgetSmartcardStatusChanged(SvObject self_)
{
    QBDiagnosticsWidget self = (QBDiagnosticsWidget) self_;
    if (!self) {
        return;
    }
    QBDiagnosticsWidgetSetSimCardStatus(self);
    QBDiagnosticsWidgetSetChipID(self);
    QBDiagnosticsWidgetSetSimID(self);
}

SvLocal void
QBDiagnosticsWidgetNATStatusChanged(SvObject self_,
                                    SvObject observable,
                                    SvObject arg)
{
    QBDiagnosticsWidget self = (QBDiagnosticsWidget) self_;
    if (!self) {
        return;
    }
    QBDiagnosticsWidgetSetEthernetStatus(self, false);
}

SvLocal void
QBDiagnosticsWidgetPartitionAdded(SvObject self_, SvObject path,
    QBDiskInfo disk, QBPartitionInfo part)
{
    QBDiagnosticsWidget self = (QBDiagnosticsWidget) self_;
    if (disk && !part && disk->devId) {
        QBDiagnosticsWidgetSetUSBStatus(self);
    }
}

SvLocal void
QBDiagnosticsWidgetPartitionRemoved(SvObject self_, SvObject path, SvString remId, SvString diskId)
{
    QBDiagnosticsWidget self = (QBDiagnosticsWidget) self_;
    if (diskId && remId && SvObjectEquals((SvObject) remId, (SvObject) diskId)) {
        QBDiagnosticsWidgetSetUSBStatus(self);
    }
}

SvLocal void
QBDiagnosticsWidgetDiskAdded(SvObject self_, SvObject path, QBDiskInfo disk)
{
    QBDiagnosticsWidget self = (QBDiagnosticsWidget) self_;
    if (disk && disk->devId) {
        QBDiagnosticsWidgetSetUSBStatus(self);
    }
}

SvLocal SvType
QBDiagnosticsWidget_getType(void)
{
    static const struct SvObjectVTable_ vtable = {
        .destroy = QBDiagnosticsWidgetDestroy
    };

    static const struct QBNetListener_t netMethods = {
        .netAttributeChanged = QBDiagnosticsWidgetNetworkSettingsChanged,
    };
    static const struct CubiwareMWCustomerInfoListener_ customerInfoMethods = {
        .customerInfoChanged = QBDiagnosticsWidgetCustomerInfoMonitorInfoChanged,
        .statusChanged = QBDiagnosticsWidgetCustomerInfoMonitorStatusChanged,
    };
    static const struct QBDiagnosticsListener_t diagnosticsChange = {
        .stateChanged = QBDiagnosticsWidgetDiagnosticsStateChanged
    };
    static const struct QBSmartCardMonitorListener_t smartCardMethods = {
        .statusChanged = QBDiagnosticsWidgetSmartcardStatusChanged
    };
    static const struct QBObserver_ observerMethods = {
        .observedObjectUpdated = QBDiagnosticsWidgetNATStatusChanged
    };
    static const struct QBHotplugMountAgentListener_t hotplugMethods = {
        .partitionAdded = QBDiagnosticsWidgetPartitionAdded,
        .partitionRemoved = QBDiagnosticsWidgetPartitionRemoved,
        .diskAdded = QBDiagnosticsWidgetDiskAdded,
        .diskRemoved = QBDiagnosticsWidgetPartitionRemoved,
    };

    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("QBDiagnosticsWidget",
                            sizeof(struct QBDiagnosticsWidget_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &vtable,
                            QBNetListener_getInterface(), &netMethods,
                            CubiwareMWCustomerInfoListener_getInterface(), &customerInfoMethods,
                            QBDiagnosticsListener_getInterface(), &diagnosticsChange,
                            QBSmartCardMonitorListener_getInterface(), &smartCardMethods,
                            QBObserver_getInterface(), &observerMethods,
                            QBHotplugMountAgentListener_getInterface(), &hotplugMethods,
                            NULL);
    }

    return type;
}

SvLocal void
QBDiagnosticsWidgetCleanup(SvApplication app, void *self_)
{
    QBDiagnosticsWidget self = self_;
    SVRELEASE(self);
}

SvLocal bool
QBDiagnosticsWidgetInputEventHandler(SvWidget widget, SvInputEvent e)
{
    QBDiagnosticsWidget self = widget->prv;
    return svViewPortHandleInputEvent(self->viewport, e);
}

SvWidget
QBDiagnosticsWidgetCreate(AppGlobals appGlobals, const char *name, QBDiagnosticsWindowConf conf, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!appGlobals) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL appGlobals passed");
        goto err1;
    }
    if (!name) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL name passed");
        goto err1;
    }
    if (!conf) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL conf passed");
        goto err1;
    }

    SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                         SVSTRING("SvEPGManager"));
    if (!epgManager) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBServiceRegistryGetService(\"SvEPGManager\") failed");
        goto err1;
    }

    QBDiagnosticsWidget self = (QBDiagnosticsWidget) SvTypeAllocateInstance(QBDiagnosticsWidget_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBDiagnosticsWidget");
        goto err1;
    }

    self->name = strdup(name);
    self->settingsCtx = svSettingsSaveContext();
    self->appGlobals = appGlobals;
    self->epgManager = SVRETAIN(epgManager);
    self->conf = SVRETAIN(conf);

    SvWidget widget = svSettingsWidgetCreate(appGlobals->res, name);
    if (!widget) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate widget");
        goto err2;
    }

    widget->prv = self;
    svWidgetSetInputEventHandler(widget, QBDiagnosticsWidgetInputEventHandler);
    svWidgetSetFocusable(widget, true);
    widget->clean = QBDiagnosticsWidgetCleanup;

    char buf[256];

    snprintf(buf, 256, "%s.viewport", self->name);
    self->viewport = svViewPortNew(appGlobals->res, buf);
    svSettingsWidgetAttach(widget, self->viewport, buf, 1);

    self->diagnosticsWidgets = SvHashTableCreate(30, NULL);

    snprintf(buf, 256, "%s.content", self->name);
    self->content = QBGridCreate(appGlobals->res, buf);
    svViewPortSetContents(self->viewport, self->content);

    snprintf(buf, 256, "%s.ScrollBar", self->name);
    self->scrollBar = QBScrollBarNew(appGlobals->res, buf);
    if (self->scrollBar) {
        svSettingsWidgetAttach(widget, self->scrollBar, buf, 1);
        svViewPortAddRangeListener(self->viewport, QBScrollBarGetRangeListener(self->scrollBar), SvViewPortRangeOrientation_VERTICAL);
    }

    self->diagnostics = QBDiagnosticsCreate(QBTVLogicGetChannel(self->appGlobals->tvLogic),
                                            QBTVLogicGetTunerNum(self->appGlobals->tvLogic));
    QBDiagnosticsStart(self->diagnostics);

    SvArray enabledDiagnostics = QBDiagnosticsWindowConfGetDiagnostics(self->conf);
    SvIterator it = SvArrayIterator(enabledDiagnostics);
    QBDiagnosticInfo diagnosticInfo = NULL;
    while ((diagnosticInfo = (QBDiagnosticInfo) SvIteratorGetNext(&it))) {
        QBDiagnosticsWidgetHandleDiagnostic(self, QBDiagnosticInfoGetId(diagnosticInfo), QBDiagnosticInfoGetLabel(diagnosticInfo));
    }

    svViewPortContentsSizeChanged(self->viewport);
    svViewPortSetContentsPosition(self->viewport, 0, 0, true);

    if (self->diagnostics) {
        QBDiagnosticsAddListener(self->diagnostics, (SvObject) self);
    }

    if (self->appGlobals->smartCardMonitor) {
        QBSmartCardMonitorAddListener(self->appGlobals->smartCardMonitor, (SvObject) self);
    }

    if (self->appGlobals->hotplugMountAgent) {
        QBHotplugMountAgentAddListener(self->appGlobals->hotplugMountAgent, (SvObject) self);
    }

    if (self->appGlobals->natMonitor) {
        QBNATMonitorAddObserver(self->appGlobals->natMonitor, (SvObject) self, NULL);
    }

    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks) &smartCardCallbacks, (SvObject) self, "QBDiagnosticsWidget");

    return widget;
err2:
    SVRELEASE(self);
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

SvWidget
QBDiagnosticsWidgetCreateWithRatio(AppGlobals appGlobals,
                                   unsigned int parentWidth,
                                   unsigned int parentHeight,
                                   const char *name,
                                   QBDiagnosticsWindowConf conf,
                                   SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!appGlobals) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL appGlobals passed");
        goto err1;
    }
    if (!name) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL name passed");
        goto err1;
    }
    if (!conf) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL conf passed");
        goto err1;
    }

    QBDiagnosticsWidget self = (QBDiagnosticsWidget) SvTypeAllocateInstance(QBDiagnosticsWidget_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBDiagnosticsWidget");
        goto err1;
    }

    self->useRatioSettings = true;

    self->name = strdup(name);
    self->settingsCtx = svSettingsSaveContext();
    self->appGlobals = appGlobals;
    self->conf = SVRETAIN(conf);

    SvWidget widget = svSettingsWidgetCreateWithRatio(appGlobals->res, name, parentWidth, parentHeight);
    if (!widget) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate widget");
        goto err2;
    }

    widget->prv = self;
    svWidgetSetInputEventHandler(widget, QBDiagnosticsWidgetInputEventHandler);
    svWidgetSetFocusable(widget, true);
    widget->clean = QBDiagnosticsWidgetCleanup;

    char buf[256];

    snprintf(buf, 256, "%s.viewport", self->name);
    self->viewport = svViewPortNewWithRatio(appGlobals->res, widget->width, widget->height, buf);
    svSettingsWidgetAttachWithRatio(widget, self->viewport, buf, 1);

    self->diagnosticsWidgets = SvHashTableCreate(30, NULL);

    snprintf(buf, 256, "%s.content", self->name);
    self->content = QBGridCreateWithRatio(appGlobals->res, widget->width, widget->height, buf);
    svViewPortSetContents(self->viewport, self->content);

    snprintf(buf, 256, "%s.ScrollBar", self->name);
    self->scrollBar = QBScrollBarNewWithRatio(appGlobals->res, widget->width, widget->height, buf);
    if (self->scrollBar) {
        svSettingsWidgetAttachWithRatio(widget, self->scrollBar, buf, 1);
        svViewPortAddRangeListener(self->viewport, QBScrollBarGetRangeListener(self->scrollBar), SvViewPortRangeOrientation_VERTICAL);
    }

    self->diagnostics = QBDiagnosticsCreate(QBTVLogicGetChannel(self->appGlobals->tvLogic),
                                            QBTVLogicGetTunerNum(self->appGlobals->tvLogic));
    QBDiagnosticsStart(self->diagnostics);

    SvArray enabledDiagnostics = QBDiagnosticsWindowConfGetDiagnostics(self->conf);
    SvIterator it = SvArrayIterator(enabledDiagnostics);
    QBDiagnosticInfo diagnosticInfo = NULL;
    while ((diagnosticInfo = (QBDiagnosticInfo) SvIteratorGetNext(&it))) {
        QBDiagnosticsWidgetHandleDiagnostic(self, QBDiagnosticInfoGetId(diagnosticInfo), QBDiagnosticInfoGetLabel(diagnosticInfo));
    }

    svViewPortContentsSizeChanged(self->viewport);
    svViewPortSetContentsPosition(self->viewport, 0, 0, true);

    if (self->diagnostics) {
        QBDiagnosticsAddListener(self->diagnostics, (SvObject) self);
    }

    if (self->appGlobals->smartCardMonitor) {
        QBSmartCardMonitorAddListener(self->appGlobals->smartCardMonitor, (SvObject) self);
    }

    if (self->appGlobals->hotplugMountAgent) {
        QBHotplugMountAgentAddListener(self->appGlobals->hotplugMountAgent, (SvObject) self);
    }

    if (self->appGlobals->natMonitor) {
        QBNATMonitorAddObserver(self->appGlobals->natMonitor, (SvObject) self, NULL);
    }

    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks) &smartCardCallbacks, (SvObject) self, "QBDiagnosticsWidget");

    return widget;
err2:
    SVRELEASE(self);
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

bool
QBDiagnosticsWidgetHandleInputEvent(SvWidget w, SvInputEvent e)
{
    assert(w);
    return QBDiagnosticsWidgetInputEventHandler(w, e);
}
