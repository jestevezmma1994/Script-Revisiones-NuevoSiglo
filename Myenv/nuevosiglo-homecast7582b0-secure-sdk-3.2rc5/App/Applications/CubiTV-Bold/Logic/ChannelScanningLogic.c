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

#include <Logic/ChannelScanningLogic.h>
#include <Logic/ChannelScanningLogicInternal.h>

#include <Logic/QBLogicFactory.h>
#include <QBInput/QBInputCodes.h>
#include <QBConf.h>
#include <main.h>
#include <Services/core/QBChannelScanningConfManager.h>
#include <Services/upgradeWatcher.h>
#include <Windows/channelscanning.h>
#include <QBDVBMuxesMap.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>

struct QBChannelScanningLogic_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    bool forcingUpdate;
    bool firstBoot;
};

SvLocal QBWindowContext
QBChannelScanningLogicCreateForcedContext_(QBChannelScanningLogic self, QBChannelScanningConf conf)
{
    QBWindowContext ctx = QBChannelScanningContextCreate(self->appGlobals, SVSTRING("manual"));

    QBChannelScanningContextLoadConf(ctx, conf);
    QBChannelScanningContextForceAutomatic(ctx, true);
    QBChannelScanningContextForce(ctx, true);
    QBChannelScanningContextForcePriority(ctx, 200);
    return ctx;
}

void
QBChannelScanningLogicShowChannelScanning(QBChannelScanningLogic self)
{
    QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
    QBChannelScanningConf conf = QBChannelScanningConfManagerGetConf(channelScanningConf, SVSTRING("automatic"));
    QBChannelScanningLogicShowChannelScanningWithConf(self, conf);
}

SvLocal void
QBChannelScanningLogicShowChannelScanningWithConf_(QBChannelScanningLogic self, QBChannelScanningConf conf)
{
    if (self->forcingUpdate || self->firstBoot)
        return;

    self->forcingUpdate = true;
    QBWindowContext ctx = QBChannelScanningLogicCreateForcedContext(self, conf);
    QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
    QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
    SVRELEASE(ctx);
}

SvString
QBChannelScanningLogicGetDefaultScanningMethod(QBChannelScanningLogic self)
{
    return SVSTRING("manual");
}

SvLocal void
QBChannelScanningLogicScanningFinished_(QBChannelScanningLogic self, QBChannelScanningConf conf, bool successfulScan)
{
    if (conf) {
        QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                      SVSTRING("QBChannelScanningConfManager"));
        QBChannelScanningConfManagerSetDefaultConf(channelScanningConf, conf->id);
    }

    if (self->forcingUpdate) {
        QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
        self->forcingUpdate = false;
    }

    if (successfulScan) {
        if (!QBInitLogicIsWizardFinished(self->appGlobals->initLogic))
            QBInitLogicEnterApplication(self->appGlobals->initLogic);

        QBTVLogicPlayChannelByIndexFromPlaylist(self->appGlobals->tvLogic, SVSTRING("TVChannels"), 0, SVSTRING("channel scanning"));
        if (conf && SvStringEqualToCString(conf->id, "automatic")) {
            if (self->appGlobals->operateScanningManager)
                QBOperateScanningManagerChannelScanningFinished(self->appGlobals->operateScanningManager);
            QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
        }
    }
}

SvLocal bool
QBChannelScanningLogicInputEventHandler_(QBChannelScanningLogic self, const QBInputEvent *e)
{
    unsigned keyCode = e->u.key.code;
    if (keyCode == QBKEY_MENU && !QBInitLogicIsWizardFinished(self->appGlobals->initLogic)) {
        QBInitLogicEnterApplication(self->appGlobals->initLogic);
        return true;
    }
    return false;
}

SvLocal void
QBChannelScanningLogicProcessFoundChannels_(QBChannelScanningLogic self, QBChannelScanningConf conf, QBDVBScanner *scanner)
{
    SvArray channels = QBDVBScannerGetChannels(scanner, true);
    SvHashTable networks = QBDVBScannerGetNetworks(scanner);
    SvHashTable muxes = QBDVBScannerGetMuxes(scanner);
    if (channels && SvArrayCount(channels)) {
        DVBEPGPlugin dvbPlugin = (DVBEPGPlugin) self->appGlobals->epgPlugin.DVB.channelList;
        DVBEPGPluginStartUpdate(dvbPlugin);

        DVBEPGPluginUpdateParams *updateParams = DVBEPGPluginCreateUpdateParams(dvbPlugin);
        DVBEPGPluginUpdateParamsFillFromScanningConf(updateParams, conf);
        DVBEPGPluginUpdate(dvbPlugin, channels, networks, muxes, updateParams);
        SVRELEASE(updateParams);

        int *checkedFrequencies = NULL;
        int checkedFrequenciesCnt = 0;

        const QBDVBScannerParams *params = QBDVBScannerGetParams(scanner);

        if (!conf || !conf->dropAll) {
            int size = sizeof(int) * params->freqCnt;
            if (size) {
                checkedFrequencies = malloc(size);
                memcpy(checkedFrequencies, params->freq, size);
                checkedFrequenciesCnt = params->freqCnt;
            }
        }
        DVBEPGPluginMarkAllUnseenAsDeadParams *markParams = DVBEPGPluginCreateMarkAllUnseenAsDeadParams(dvbPlugin);
        const QBDVBScannerParams *scannerParams = QBDVBScannerGetParams(scanner);
        if (scannerParams->metadata) {
            SvIterator keyIt = SvHashTableKeysIterator(scannerParams->metadata);
            SvIterator valuesIt = SvHashTableValuesIterator(scannerParams->metadata);
            SvHashTableInsertObjects(markParams->metadata, &keyIt, &valuesIt);
        }
        DVBEPGPluginMarkUnseenAsDead(dvbPlugin, checkedFrequenciesCnt, checkedFrequencies, markParams);
        SVRELEASE(markParams);
        if (self->appGlobals->dvbMuxesMap) {
            QBDVBMuxesMapUpdate(self->appGlobals->dvbMuxesMap, scanner, NULL);
            QBDVBMuxesMapSave(self->appGlobals->dvbMuxesMap, "/etc/vod/ota-muxes", NULL);
        }
    }

    SVTESTRELEASE(channels);
}

bool
QBChannelScanningLogicIsScanSuccessful(QBChannelScanningLogic self, QBChannelScanningConf conf, QBDVBScanner *scanner)
{
    bool ret = true;
    SvArray channels = QBDVBScannerGetChannels(scanner, true);
    if (!channels || !SvArrayCount(channels)) {
        ret = false;
        goto fini;
    }
    //TODO
fini:
    SVTESTRELEASE(channels);
    return ret;
}

SvHashTable
QBChannelScanningLogicGetResults(QBChannelScanningLogic self)
{
    return NULL;
}

SvString
QBChannelScanningLogicGetMWTimeoutMessageTag(QBChannelScanningLogic self)
{
    return NULL;
}

SvLocal bool
QBChannelScanningLogicIsRadioEnabled_(QBChannelScanningLogic self)
{
    return true;
}

SvLocal void
QBChannelScanningLogicSetDVBScannerPlugins_(QBChannelScanningLogic self, QBDVBScanner *scanner)
{
    // XXX: This shows how to add SIDAsChannelNumberPlugin
    /*
    SIDAsChannelNumberPlugin plugin = SIDAsChannelNumberPluginCreate();
    QBDVBScannerAddPlugin(scanner, (SvGenericObject) plugin);
    SVRELEASE(plugin);
    */
}

SvLocal QBChannelScanningLogic
QBChannelScanningLogicInit_(QBChannelScanningLogic self, AppGlobals appGlobals, SvErrorInfo *errorOut)
{
    self->appGlobals = appGlobals;
    return self;
}

SvType
QBChannelScanningLogic_getType(void)
{
    static SvType type = NULL;

    static const struct QBChannelScanningLogicVTable_ logicVTable = {
        .init                        = QBChannelScanningLogicInit_,
        .createForcedContext         = QBChannelScanningLogicCreateForcedContext_,
        .processFoundChannels        = QBChannelScanningLogicProcessFoundChannels_,
        .scanningFinished            = QBChannelScanningLogicScanningFinished_,
        .inputEventHandler           = QBChannelScanningLogicInputEventHandler_,
        .showChannelScanningWithConf = QBChannelScanningLogicShowChannelScanningWithConf_,
        .isRadioEnabled              = QBChannelScanningLogicIsRadioEnabled_,
        .setDVBScannerPlugins        = QBChannelScanningLogicSetDVBScannerPlugins_,
    };

    if (!type) {
        SvTypeCreateVirtual("QBChannelScanningLogic",
                            sizeof(struct QBChannelScanningLogic_),
                            SvObject_getType(),
                            sizeof(logicVTable),
                            &logicVTable,
                            &type,
                            NULL);
    }
    return type;
}

QBChannelScanningLogic
QBChannelScanningLogicCreate(AppGlobals appGlobals, SvErrorInfo *errorOut)
{
    QBChannelScanningLogic self = NULL;
    SvErrorInfo error = NULL;

    SvType logicClass = QBLogicFactoryFindImplementationOf(QBLogicFactoryGetInstance(),
                                                           QBChannelScanningLogic_getType(), &error);
    if (!logicClass) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error, "can't find channel scanning logic implementation");
        goto fini;
    }

    self = (QBChannelScanningLogic) SvTypeAllocateInstance(logicClass, &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "can't allocate %s", SvTypeGetName(logicClass));
        goto fini;
    }

    if (!SvInvokeVirtual(QBChannelScanningLogic, self, init, appGlobals, &error)) {
        SVRELEASE(self);
        self = NULL;
        goto fini;
    }
    SvLogNotice("%s(): created instance of %s", __func__, SvTypeGetName(logicClass));

fini:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}
