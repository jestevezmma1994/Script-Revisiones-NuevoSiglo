/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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


#include <Services/QBDVBChannelLogos.h>
#include <QBDVBTableMonitor.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <QBDSMCC/QBDSMCCMonitor.h>
#include <QBDSMCCClient.h>
#include <QBConfig.h>
#include <SvJSON/SvJSONParse.h>
#include <QBDataModel3/QBListModel.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>
#include <fibers/c/fibers.h>
#include <arpa/inet.h>
#include <stdbool.h>


typedef enum {
    STATE_STOPPED,
    STATE_TABLE_MONITOR_RUNNING,
    STATE_DSMCC_RUNNING
} QBDVBChannelLogosServiceState;

struct QBDVBChannelLogosService_ {
    struct SvObject_ super_;

    QBDVBTableMonitor tableMonitor;
    QBTunerLogic tunerLogic;
    QBPlaylistManager playlists;

    QBDSMCCMonitor dsmccMonitor;

    SvFiber fiber;
    SvObject logosMap;
    int channelIndex;

    uint32_t carouselId;
    uint32_t carouselCacheLimit;
    uint16_t carouselPID;
    uint8_t wantedLinkageType;
    SvValue (*mapLogo)(QBDVBChannelLogosService, SvTVChannel);
    QBDVBChannelLogosServiceState state;
};

#if SV_LOG_LEVEL > 0
    #define moduleName "QBchannelLogosDSMCC"
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 2, moduleName "Debug", "");
    #define log_info(fmt, ...) do { if (env_log_level() >= 2) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } while (0)
    #define log_warning(fmt, ...) do { if (env_log_level() >= 1) SvLogWarning(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...) do { if (env_log_level() >= 1) SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#else
    #define log_info(fmt, ...)
    #define log_warning(fmt, ...)
    #define log_error(fmt, ...)
#endif

//forward declarations
SvLocal void QBDVBChannelLogosServiceLinkageDescriptorReceived(SvObject self_, QBDVBTableMonitorTableID tableID, uint8_t linkageType, SvData descriptorData);
SvLocal void QBDVBChannelLogosServiceDestroy(void *self_);
SvLocal void QBDVBChannelLogosServiceDSMCCDataReady(SvObject self_);
SvLocal void QBDVBChannelLogosServiceSetupDSMCC(QBDVBChannelLogosService self, SvData descriptorData);
SvLocal void QBDVBChannelLogosServiceFiberStep(void *self_);
SvLocal SvValue QBDVBChannelLogosServiceFindLogoByName(QBDVBChannelLogosService self, SvTVChannel channel);
SvLocal SvValue QBDVBChannelLogosServiceFindLogoByLCN(QBDVBChannelLogosService self, SvTVChannel channel);


SvLocal SvType QBDVBChannelLogosService_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDVBChannelLogosServiceDestroy
    };

    static const struct QBDVBLinkageDescriptorListener_ linkageListenerMethods = {
        .descriptorReceived = QBDVBChannelLogosServiceLinkageDescriptorReceived
    };

    static const struct QBDSMCCClientListener_ QBDSMCCClientListenerMethods = {
        .dataReady = QBDVBChannelLogosServiceDSMCCDataReady
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDVBChannelLogosService",
                            sizeof(struct QBDVBChannelLogosService_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBDVBLinkageDescriptorListener_getInterface(), &linkageListenerMethods,
                            QBDSMCCClientListener_getInterface(), &QBDSMCCClientListenerMethods,
                            NULL);
    }
    return type;
}

QBDVBChannelLogosService QBDVBChannelLogosServiceCreate(SvString confPath, QBTunerLogic tunerLogic, QBPlaylistManager playlists)
{
    if (!tunerLogic || !playlists) {
        log_error("%s: NULL pointer passed: tunerLogic=%p, playlists=%p", __func__, tunerLogic, playlists);
        return NULL;
    }

    QBDVBChannelLogosService self = NULL;
    SvObject confFileContent = NULL;

    confFileContent = SvJSONParseFile(SvStringGetCString(confPath), false, NULL);
    if (!confFileContent) {
        log_error("failed to read or parse file %s", SvStringGetCString(confPath));
        goto fini;
    }

    if (!SvObjectIsInstanceOf(confFileContent, SvHashTable_getType())) {
        log_error("config file content is not a hash table");
        goto fini;
    }

    SvObject serviceIdType = SvHashTableFind((SvHashTable) confFileContent, (SvObject) SVSTRING("serviceIdType"));
    if (!SvObjectIsInstanceOf(serviceIdType, SvValue_getType()) || !SvValueIsString((SvValue) serviceIdType)) {
        log_error("serviceIdType is not a string");
        goto fini;
    }
    SvString serviceIdTypeStr = SvValueGetString((SvValue) serviceIdType);
    SvValue (*mapLogo)(QBDVBChannelLogosService, SvTVChannel) = NULL;
    if (SvStringEqualToCString(serviceIdTypeStr, "LCN")) {
        mapLogo = QBDVBChannelLogosServiceFindLogoByLCN;
    }
    if (SvStringEqualToCString(serviceIdTypeStr, "name")) {
        mapLogo = QBDVBChannelLogosServiceFindLogoByName;
    }
    if (!mapLogo) {
        log_error("serviceIdTypeStr should be \"LCN\" or \"name\", and is \"%s\"", SvStringGetCString(serviceIdTypeStr));
        goto fini;
    }

    SvObject linkageType = SvHashTableFind((SvHashTable) confFileContent, (SvObject) SVSTRING("linkageType"));
    if (!SvObjectIsInstanceOf((SvObject) linkageType, SvValue_getType()) || !SvValueIsInteger((SvValue) linkageType)) {
        log_error("linkageType is not an integer");
        goto fini;
    }

    SvObject cacheQuotaKB = SvHashTableFind((SvHashTable) confFileContent, (SvObject) SVSTRING("cacheQuotaKB"));
    if (!SvObjectIsInstanceOf((SvObject) cacheQuotaKB, SvValue_getType()) || !SvValueIsInteger((SvValue) cacheQuotaKB)) {
        log_error("cacheQuotaKB is not an integer");
        goto fini;
    }

    self = (QBDVBChannelLogosService) SvTypeAllocateInstance(QBDVBChannelLogosService_getType(), NULL);
    self->mapLogo = mapLogo;
    self->wantedLinkageType = SvValueGetInteger((SvValue) linkageType);
    self->carouselCacheLimit = 1024 * SvValueGetInteger((SvValue) cacheQuotaKB);

    self->tunerLogic = SVRETAIN(tunerLogic);
    self->playlists = SVRETAIN(playlists);
    self->state = STATE_STOPPED;

fini:
    SVTESTRELEASE(confFileContent);
    return self;
}

SvLocal void QBDVBChannelLogosServiceDestroy(void *self_)
{
    QBDVBChannelLogosService self = (QBDVBChannelLogosService) self_;
    if (self->state != STATE_STOPPED)
        QBDVBChannelLogosServiceStop(self);

    SVRELEASE(self->tunerLogic);
    SVRELEASE(self->playlists);
}

void QBDVBChannelLogosServiceStart(QBDVBChannelLogosService self)
{
    if (!self) {
        log_error("%s: NULL pointer passed", __func__);
        return;
    }

    if (self->state == STATE_STOPPED) {
        self->tableMonitor =
            SVTESTRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBDVBTableMonitor")));
        if (!self->tableMonitor) {
            log_error("DVB table monitor is NULL");
            return;
        }
        QBDVBTableMonitorAddListener(self->tableMonitor, (SvObject) self);
        self->fiber = SvFiberCreate(SvSchedulerGet(), NULL, "DVBChannelLogosService", QBDVBChannelLogosServiceFiberStep, self);
        self->state = STATE_TABLE_MONITOR_RUNNING;
    }
}

void QBDVBChannelLogosServiceStop(QBDVBChannelLogosService self)
{
    if (!self) {
        log_error("%s: NULL pointer passed", __func__);
        return;
    }

    if (self->state != STATE_STOPPED) {
        SvFiberDestroy(self->fiber);
        SVTESTRELEASE(self->logosMap);
        self->logosMap = NULL;
        if (self->tableMonitor) {
            QBDVBTableMonitorRemoveListener(self->tableMonitor, (SvObject) self);
            SVRELEASE(self->tableMonitor);
            self->tableMonitor = NULL;
        } else {
            log_error("DVB table monitor is NULL");
        }

        if (self->state == STATE_DSMCC_RUNNING) {
            QBDSMCCMonitorStop(self->dsmccMonitor);
            SVRELEASE(self->dsmccMonitor);
            self->dsmccMonitor = NULL;
        }

        self->state = STATE_STOPPED;
    }
}

SvLocal void QBDVBChannelLogosServiceSetDSMCCParams(QBDVBChannelLogosService self, uint16_t carouselPID, uint32_t carouselId)
{
    log_info("Setting new DSMCC carousel ID = %x for channelLogos", carouselId);
    self->carouselId = carouselId;
    self->carouselPID = carouselPID;
    QBDSMCCClient dsmccClient = QBDSMCCMonitorGetClient(self->dsmccMonitor);
    QBDSMCCClientRemoveListener(dsmccClient, (SvObject) self);

    QBDSMCCMonitorSetCarouselId(self->dsmccMonitor, carouselId);
    QBDSMCCMonitorSetPID(self->dsmccMonitor, carouselPID);

    dsmccClient = QBDSMCCMonitorGetClient(self->dsmccMonitor);
    QBDSMCCClientSetCacheSize(dsmccClient, self->carouselCacheLimit);
    QBDSMCCClientAddListener(dsmccClient, (SvObject) self);

    self->state = STATE_DSMCC_RUNNING;
}

SvLocal void QBDVBChannelLogosServiceSetupDSMCC(QBDVBChannelLogosService self, SvData descriptorData)
{
    uint16_t carouselPID = get32((uint8_t *) SvDataGetBytes(descriptorData), 16, 16);
    uint32_t carouselId = get32((uint8_t *) SvDataGetBytes(descriptorData), 32, 32);

    //setup DSMCC to receive data, and in both cases enter DSMCC_RUNNING state
    if (self->state == STATE_TABLE_MONITOR_RUNNING) {
        int tunerCnt = QBTunerLogicGetTunerCount(self->tunerLogic);
        self->dsmccMonitor = QBDSMCCMonitorCreate(tunerCnt);
        QBDSMCCMonitorStart(self->dsmccMonitor);
        QBDVBChannelLogosServiceSetDSMCCParams(self, carouselPID, carouselId);
    } else if (self->state == STATE_DSMCC_RUNNING && (carouselId != self->carouselId || self->carouselPID != carouselPID)) {
        QBDVBChannelLogosServiceSetDSMCCParams(self, carouselPID, carouselId);
    }
}

SvLocal void QBDVBChannelLogosServiceLinkageDescriptorReceived(SvObject self_, QBDVBTableMonitorTableID tableID, uint8_t linkageType, SvData descriptorData)
{
    if (!self_ || !descriptorData) {
        log_error("%s: NULL pointer passed: self_=%p, descriptorData=%p", __func__, self_, descriptorData);
        return;
    }

    QBDVBChannelLogosService self = (QBDVBChannelLogosService) self_;

    if (self->wantedLinkageType == linkageType) {
        QBDVBChannelLogosServiceSetupDSMCC(self, descriptorData);
    } else {
        log_info("%s: Useless linkage (unknown linkageType or NULL)", __func__);
    }
}

SvLocal SvValue QBDVBChannelLogosServiceFindLogoByLCN(QBDVBChannelLogosService self, SvTVChannel channel)
{
    SvString num_str = SvStringCreateWithFormat("%d", channel->number);
    assert(num_str);
    SvValue bitmap_val = (SvValue) SvHashTableFind((SvHashTable) self->logosMap, (SvObject) num_str);

    if (!bitmap_val || !SvValueIsString(bitmap_val)) {
        log_info("Missing logo image for %s", SvStringGetCString(num_str));
        bitmap_val = NULL;
    }

    SVRELEASE(num_str);
    return bitmap_val;
}

SvLocal SvValue QBDVBChannelLogosServiceFindLogoByName(QBDVBChannelLogosService self, SvTVChannel channel)
{
    SvValue bitmap_val = (SvValue) SvHashTableFind((SvHashTable) self->logosMap, (SvObject) channel->name);

    if (!bitmap_val || !SvValueIsString(bitmap_val)) {
        log_info("Missing logo image for %s", SvStringGetCString(channel->name));
        bitmap_val = NULL;
    }

    return bitmap_val;
}

SvLocal void QBDVBChannelLogosServiceFiberStep(void *self_)
{
    QBDVBChannelLogosService self = (QBDVBChannelLogosService) self_;
    SvObject allChannels = QBPlaylistManagerGetById(self->playlists, SVSTRING("AllChannels"));

    int n = SvInvokeInterface(QBListModel, allChannels, getLength);
    while (self->channelIndex < n) {
        SvTVChannel channel = (SvTVChannel) SvInvokeInterface(QBListModel, allChannels, getObject, self->channelIndex);

        SvValue bitmap_val = self->mapLogo(self, channel);
        if (bitmap_val) {
            SvString url_str = SvStringCreateWithFormat("file://%s/%s", DSMCC_CACHE_DIR, SvStringGetCString(SvValueGetString(bitmap_val)));
            SvURL url = SvURLCreateWithString(url_str, NULL);
            SvTVChannelSetLogoURL(channel, url);
            SVRELEASE(url);
            SVRELEASE(url_str);
        }
        ++(self->channelIndex);

        if (SvFiberTimePassed()) {
            return;
        }
    }

    //loop finished:
    SvFiberDeactivate(self->fiber);
    SVTESTRELEASE(self->logosMap);
    self->logosMap = NULL;
}

SvLocal void QBDVBChannelLogosServiceDSMCCDataReady(SvObject self_)
{
    QBDVBChannelLogosService self = (QBDVBChannelLogosService) self_;

    SVTESTRELEASE(self->logosMap);     //in case of overlap
    self->logosMap = SvJSONParseFile(DSMCC_CACHE_DIR "/manifest.json", false, NULL);

    if (!SvObjectIsInstanceOf((SvObject) self->logosMap, SvHashTable_getType())) {
        log_warning("%s: manifest.json doesn't exist or contains trashes", __func__);
        SVTESTRELEASE(self->logosMap);
        self->logosMap = NULL;
        return;
    }

    self->channelIndex = 0;
    SvFiberActivate(self->fiber);
}
