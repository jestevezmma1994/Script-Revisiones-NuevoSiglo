/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBCryptoguardForceTuneManager.h"
#include <QBCASCommonCryptoguardForceTuneListener.h>
#include <QBCASCommonCryptoguardForceTune.h>
#include <QBCAS.h>

#include <QBAppKit/QBGlobalStorage.h>

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvEnv.h>
#include <SvFoundation/SvType.h>

#include <mpeg_descriptors/dvb/terrestrial_delivery.h>
#include <mpeg_descriptors/dvb/cable_delivery.h>
#include <mpeg_descriptors/dvb/satellite_delivery.h>

#include <QBTuner.h>
#include <QBTunerTypes.h>
#include <QBDVBScannerTypes.h>
#include <QBDVBSatellitesDB.h>
#include <SvEPGDataLayer/Plugins/DVBEPGPlugin.h>
#include <sv_tuner.h>
#include <main.h>

#include <libintl.h>

#if SV_LOG_LEVEL > 0

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 2, "QBCryptoguardForceTuneManager", "");

#define log_fun(fmt, ...) do { if (env_log_level() >= 4) { SvLogNotice(COLBEG() "QBCryptoguardForceTuneManager :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)
#define log_debug(fmt, ...) do { if (env_log_level() >= 3) { SvLogNotice(COLBEG() "QBCryptoguardForceTuneManager :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)
#define log_state(fmt, ...) do { if (env_log_level() >= 2) { SvLogNotice(COLBEG() "QBCryptoguardForceTuneManager :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } } while (0)
#define log_error(fmt, ...) do { if (env_log_level() >= 1) { SvLogError(COLBEG() "QBCryptoguardForceTuneManager :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } } while (0)

#else

#define log_fun(fmt, ...)
#define log_debug(fmt, ...)
#define log_state(fmt, ...)
#define log_error(fmt, ...)

#endif

#define DVB_T_TAG 0x5A
#define DVB_C_TAG 0x44
#define DVB_S_TAG 0x43

struct QBCryptoguardForceTuneManager_s {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvFiber fiber;

    DVBEPGChannel channel;
    bool hasTerrestialTuner;
    bool hasCableTuner;
    bool hasSatelliteTuner;
};


SvLocal void QBCryptoguardForceTuneManagerDestroy(void *self_)
{
    log_fun();
    QBCryptoguardForceTuneManager self = self_;
    SVTESTRELEASE(self->channel);
    SvFiberDestroy(self->fiber);
}

SvLocal DVBEPGChannel QBCryptoguardForceTuneManagerCreateTerrestrialChannel(QBCryptoguardForceTuneManager self, mpeg_data *deliveryDescData, uint16_t sid)
{
    log_fun();

    if (!self->hasTerrestialTuner) {
        log_error("Cannot used terrestrial descriptor, there isn't any terrestrial tuner");
        return NULL;
    }

    mpeg_terrestrial_delivery_desc desc;
    if (mpeg_terrestrial_delivery_desc_init(&desc, deliveryDescData) < 0) {
        log_error("cannot parse terrestrial descriptor");
        return NULL;
    }

    mpeg_terrestrial_delivery_desc_report(&desc, "");

    DVBEPGChannel channel = DVBEPGChannelCreate(-1, sid, -1, QBDVBChannelType_TV);
    channel->params.mux_id.freq = desc.freq;
    channel->params.band = desc.bandwidth;

    channel->tunerType = QBTunerType_ter;

    mpeg_terrestrial_delivery_desc_destroy(&desc);

    return channel;
}

SvLocal DVBEPGChannel QBCryptoguardForceTuneManagerCreateCableChannel(QBCryptoguardForceTuneManager self, mpeg_data *deliveryDescData, uint16_t sid)
{
    log_fun();

    if (!self->hasCableTuner) {
        log_error("Cannot used cable descriptor, there isn't any cable tuner");
        return NULL;
    }

    mpeg_cable_delivery_desc desc;
    if (mpeg_cable_delivery_desc_init(&desc, deliveryDescData) < 0) {
        log_error("cannot parse cable descriptor");
        return NULL;
    }

    mpeg_cable_delivery_desc_report(&desc, "");

    DVBEPGChannel channel = DVBEPGChannelCreate(-1, sid, -1, QBDVBChannelType_TV);
    channel->params.mux_id.freq = desc.freq;
    channel->params.modulation = QBTunerModulationFromQAM(desc.modulation);
    channel->params.symbol_rate = desc.symbol_rate;

    channel->tunerType = QBTunerType_cab;

    mpeg_cable_delivery_desc_destroy(&desc);

    return channel;
}

SvLocal DVBEPGChannel QBCryptoguardForceTuneManagerCreateSatelliteChannel(QBCryptoguardForceTuneManager self, mpeg_data *deliveryDescData, uint16_t sid)
{
    log_fun();

    if (!self->hasSatelliteTuner) {
        log_error("Cannot used satellite descriptor, there isn't any satellite tuner");
        return NULL;
    }

    dvb_satellite_delivery_desc desc;
    if (dvb_satellite_delivery_desc_init(&desc, deliveryDescData) < 0) {
        log_error("cannot parse satellite descriptor");
        return NULL;
    }

    dvb_satellite_delivery_desc_report(&desc, "");

    DVBEPGChannel channel = DVBEPGChannelCreate(-1, sid, -1, QBDVBChannelType_TV);
    channel->params.mux_id.freq = desc.freq;
    channel->params.symbol_rate = desc.symbol_rate;

    int orbitalPosition = desc.orbital_position;
    if (desc.is_west) {
        orbitalPosition = 3600 - orbitalPosition;
    }

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
        QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), NULL);

    SvArray satByPos = satellitesDB ? QBDVBSatellitesDBCreateListOfSatellitesByPosition(satellitesDB, orbitalPosition) : NULL;
    if (satByPos) {
        //get first found satellite
        QBDVBSatelliteDescriptor satDesc = (QBDVBSatelliteDescriptor) SvArrayObjectAtIndex(satByPos, 0);
        SVRELEASE(satByPos);

        if (!satDesc) {
            log_error("Found delivery descriptor pointing to satellite not present in satellite DB: orbital pos=%d", orbitalPosition);
            goto failed;
        }
        QBTunerLNBConfig lnbConfig = QBTunerGetLNBConfigByID(QBDVBSatelliteDescriptorGetID(satDesc));
        if (!lnbConfig) {
            log_error("Satellite not present in lnbConfig: %s", SvStringCString(QBDVBSatelliteDescriptorGetName(satDesc)));
            goto failed;
        }
        log_state("Found delivery descriptor pointing to other satellite: orbital pos=%d, satelliteID=%s", orbitalPosition, SvStringCString(lnbConfig->ID));
        strncpy(channel->params.mux_id.satelliteID, SvStringCString(lnbConfig->ID), sizeof(channel->params.mux_id.satelliteID) - 1);
    }

    channel->params.modulation = QBTunerModulationFromSatelliteModulationType(desc.modulation_type);
    if (channel->params.modulation == QBTunerModulation_unknown) {
        log_state("modulation not defined, auto mode normally");
    }

    channel->params.polarization = QBTunerPolarizationFromDescriptor(desc.polarization);
    if (channel->params.polarization == QBTunerPolarization_unknown) {
        goto failed;
    }

    channel->tunerType = QBTunerType_sat;

    dvb_satellite_delivery_desc_destroy(&desc);
    return channel;

failed:
    dvb_satellite_delivery_desc_destroy(&desc);
    SVRELEASE(channel);
    return NULL;
}

SvLocal void QBCryptoguardForceTuneAddGlobalInfoToChannel(QBCryptoguardForceTuneManager self, DVBEPGChannel channel)
{
    log_fun();
    struct QBTunerMuxId *mux_id = &channel->params.mux_id;

    SvString id = SvStringCreateWithFormat("freq=%d:plp=%d:custom0=%u:sid=%d", mux_id->freq, mux_id->plp_id, mux_id->custom0, channel->sid);
    SvValue id_val = SvValueCreateWithString(id, NULL);
    SVRELEASE(id);

    SvDBObject obj = (SvDBObject) channel;
    SVRELEASE(obj->id);
    obj->id = id_val;

    SvTVChannel tvChannel = (SvTVChannel) channel;
    tvChannel->number = 0;
    SVRELEASE(tvChannel->name);
    tvChannel->name = SvStringCreate(gettext("CryptoGuard Force Tune"), NULL);

    char *tmpBuffer = sv_tuner_print_params(&channel->sid, &channel->tunerType, &channel->params);
    SvString url = SvStringCreateWithFormat("dvb://%s", tmpBuffer);
    free(tmpBuffer);
    log_state("channel source url (%s)", SvStringCString(url));

    SVTESTRELEASE(tvChannel->sourceURL);
    tvChannel->sourceURL = SvURLCreateWithString(url, NULL);
    SVRELEASE(url);
}

SvLocal void QBCryptoguardForceTuneManagerDoForceTune(SvObject self_, int8_t sessionId, QBCASCommonCryptoguardForceTune forceTune)
{
    log_fun();
    QBCryptoguardForceTuneManager self = (QBCryptoguardForceTuneManager) self_;

    log_state("DoForceTune Received, sessionId (%d), destinationSID (0x%X), tag (0x%X)", sessionId, forceTune->destinationSID, forceTune->tag);

    uint8_t delivery_desc[2 + DELIVERY_SYSTEM_DESCRIPTOR_LENGTH] = {}; // header (2B) + data (11B)
    delivery_desc[0] = forceTune->tag;
    delivery_desc[1] = DELIVERY_SYSTEM_DESCRIPTOR_LENGTH;
    memcpy(delivery_desc + 2, forceTune->descriptorData, DELIVERY_SYSTEM_DESCRIPTOR_LENGTH);

    mpeg_data mpeg_delivery_data;
    mpeg_delivery_data.length = sizeof(delivery_desc);
    mpeg_delivery_data.data = delivery_desc;

    DVBEPGChannel channel = NULL;

    switch (forceTune->tag) {
        case DVB_T_TAG:
            channel = QBCryptoguardForceTuneManagerCreateTerrestrialChannel(self, &mpeg_delivery_data, forceTune->destinationSID);
            break;
        case DVB_C_TAG:
            channel = QBCryptoguardForceTuneManagerCreateCableChannel(self, &mpeg_delivery_data, forceTune->destinationSID);
            break;
        case DVB_S_TAG:
            channel = QBCryptoguardForceTuneManagerCreateSatelliteChannel(self, &mpeg_delivery_data, forceTune->destinationSID);
            break;
        default:
            log_error("Unsupported descriptor tag (0x%02X)", forceTune->tag);
    }

    if (channel) {
        QBCryptoguardForceTuneAddGlobalInfoToChannel(self, channel);

        SVTESTRELEASE(self->channel);
        self->channel = channel;

        // we cannot set channel in this callback because it will call QBCASCommonFlush/StartDescrambling and mutex lock happens
        // all callbacks from QBCASCommon are called under mutex
        SvFiberActivate(self->fiber);
    }
}

SvLocal SvType QBCryptoguardForceTuneManager_getType(void)
{
    log_fun();
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBCryptoguardForceTuneManagerDestroy,
    };

    struct QBCASCommonCryptoguardForceTuneListener_s cryptoguardForceTuneMethods = {
        .doForceTune = QBCryptoguardForceTuneManagerDoForceTune,
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBCryptoguardForceTuneManager",
                            sizeof(struct QBCryptoguardForceTuneManager_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBCASCommonCryptoguardForceTuneListener_getInterface(), &cryptoguardForceTuneMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBCryptoguardForceTuneManagerStep(void *self_)
{
    log_fun();
    QBCryptoguardForceTuneManager self = (QBCryptoguardForceTuneManager) self_;
    SvFiberDeactivate(self->fiber);

    if (self->channel) {
        QBTVLogicPlayChannel(self->appGlobals->tvLogic, (SvTVChannel) self->channel, NULL);
    }
}

QBCryptoguardForceTuneManager QBCryptoguardForceTuneManagerCreate(AppGlobals appGlobals)
{
    log_fun();
    QBCryptoguardForceTuneManager self = (QBCryptoguardForceTuneManager) SvTypeAllocateInstance(QBCryptoguardForceTuneManager_getType(), NULL);

    self->appGlobals = appGlobals;
    self->fiber = SvFiberCreate(appGlobals->scheduler, NULL, "QBCryptoguardForceTuneManagerStep", QBCryptoguardForceTuneManagerStep, self);

    return self;
}

SvLocal void QBCryptoguardForceTuneManagerUnblockAllTuners(QBCryptoguardForceTuneManager self)
{
    log_fun();

    self->hasTerrestialTuner = true;
    self->hasCableTuner = true;
    self->hasSatelliteTuner = true;
}

SvLocal int QBCryptoguardForceTuneManagerSetAvailableTuners(QBCryptoguardForceTuneManager self)
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
    int id;
    for (id = 0; id < config_cnt; ++id) {
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

SvLocal void QBCryptoguardForceTuneManagerClearAvailableTuners(QBCryptoguardForceTuneManager self)
{
    log_fun();

    self->hasTerrestialTuner = false;
    self->hasCableTuner = false;
    self->hasSatelliteTuner = false;
}

void QBCryptoguardForceTuneManagerStart(QBCryptoguardForceTuneManager self)
{
    log_fun();
    if (QBCryptoguardForceTuneManagerSetAvailableTuners(self) < 0) {
        QBCryptoguardForceTuneManagerUnblockAllTuners(self);
    }
    QBCASCommonCryptoguardForceTuneListenerAdd((QBCASCommonCryptoguard) QBCASGetInstance(), (SvObject) self);
}

void QBCryptoguardForceTuneManagerStop(QBCryptoguardForceTuneManager self)
{
    log_fun();
    SvFiberDeactivate(self->fiber);
    QBCryptoguardForceTuneManagerClearAvailableTuners(self);
    QBCASCommonCryptoguardForceTuneListenerRemove((QBCASCommonCryptoguard) QBCASGetInstance(), (SvObject) self);
}
