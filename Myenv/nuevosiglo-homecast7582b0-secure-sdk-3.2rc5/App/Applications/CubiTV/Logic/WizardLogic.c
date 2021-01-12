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

#include <Logic/WizardLogic.h>

#include <QBAppKit/QBServiceRegistry.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvEnv.h>
#include <QBPlatformHAL/QBPlatformOption.h>
#include <QBConf.h>
#include <sv_tuner.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <QBInput/QBInputCodes.h>
#include <TraxisWebClient/TraxisWebSessionManager.h>
#include <Utils/appType.h>
#include <Services/core/QBChannelScanningConfManager.h>
#include <Services/QBRCUPairingService.h>
#include <Windows/QBRCUPairingWindow.h>
#include <Windows/wizard.h>
#include <Windows/channelscanning.h>
#include <iso_639_table.h>
#include <init.h>
#include <main.h>

typedef enum {
    QBWizardModeDVB,
    QBWizardModeHybrid,
    QBWizardModeIP
} QBWizardMode;

struct QBWizardLogic_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvHashTable idToOption;
    SvHashTable idToSubOption;
    SvFiber fiber;
    SvFiberTimer timer;
    bool skipScanning;
    QBWizardMode mode;
};

SvLocal void QBWizardLogicSetupDVBScanning(QBWizardLogic self)
{
    SvLogNotice("Initializing channel scanning configuration");
    SvString channelScanningConfPath = QBInitLogicGetChannelScanningConfFileName(self->appGlobals->initLogic);
    QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
    if (channelScanningConfPath)
        QBChannelScanningConfManagerLoadConfiguration(channelScanningConf, channelScanningConfPath);
    SVRELEASE(channelScanningConfPath);

    SvString scanDefaultParamsFileName = QBInitLogicGetDefaultScanningParamsFileName(self->appGlobals->initLogic);
    SvString scanUserParamsFileName = QBInitLogicGetUserScanningParamsFileName(self->appGlobals->initLogic);
    QBChannelScanningConfManagerLoadUserParams(channelScanningConf, scanDefaultParamsFileName, scanUserParamsFileName);
    SVRELEASE(scanDefaultParamsFileName);
    SVRELEASE(scanUserParamsFileName);
}

SvLocal void QBWizardLogicStartChannelScanning(QBWizardLogic self)
{
    if (self->skipScanning) {
        QBInitLogicEnterApplication(self->appGlobals->initLogic);
        return;
    }

    QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
    SvString filename = QBInitLogicGetChannelScanningConfFileName(self->appGlobals->initLogic);
    QBChannelScanningConfManagerLoadConfiguration(channelScanningConf, filename);
    SVRELEASE(filename);

    SvString scanConf = SVTESTRETAIN(SvHashTableFind(self->idToOption, (SvObject) SVSTRING("scan")));
    if (!scanConf) {
        const char *scanConfStr = QBConfigGet("CHANNELSCANNINGCONF");
        scanConf = SvStringCreate(scanConfStr, NULL);
    }

    QBWizardLogicSetupDVBScanning(self);

    QBChannelScanningConf conf = QBChannelScanningConfManagerGetConf(channelScanningConf, scanConf);
    SVRELEASE(scanConf);

    QBWindowContext scan = QBChannelScanningContextCreate(self->appGlobals, SVSTRING("manual"));
    if (conf && conf->id && SvStringEqualToCString(conf->id, "automatic"))
        QBChannelScanningContextForceAutostart(scan, true);
    QBChannelScanningContextLoadConf(scan, conf);

    QBApplicationControllerCleanContextStack(self->appGlobals->controller);
    QBApplicationControllerPushContext(self->appGlobals->controller, scan);
    SVRELEASE(scan);

}

SvLocal void QBWizardLogicDestroy(void *self_)
{
    QBWizardLogic self = (QBWizardLogic) self_;

    SVTESTRELEASE(self->idToOption);
    SVTESTRELEASE(self->idToSubOption);
    if (self->fiber)
        SvFiberDestroy(self->fiber);
}

SvLocal SvType QBWizardLogic_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBWizardLogicDestroy
    };
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("QBWizardLogic",
                            sizeof(struct QBWizardLogic_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal void QBWizardLogicRefreshMode(QBWizardLogic self)
{
    if (QBAppTypeIsDVB())
        self->mode = QBWizardModeDVB;
    else if (QBAppTypeIsHybrid())
        self->mode = QBWizardModeHybrid;
    else
        self->mode = QBWizardModeIP;
}

QBWizardLogic QBWizardLogicCreate(AppGlobals appGlobals)
{
    QBWizardLogic self = (QBWizardLogic) SvTypeAllocateInstance(QBWizardLogic_getType(), NULL);

    self->appGlobals = appGlobals;
    self->skipScanning = false;

    QBWizardLogicRefreshMode(self);

    return self;
}

void QBWizardLogicLoadingAllServicesFinished(QBWizardLogic self)
{
    const char* mode = QBConfigGet("HYBRIDMODE");

    if (SvStringEqualToCString(SVSTRING("IP"), mode)) {
        QBInitLogicEnterApplication(self->appGlobals->initLogic);
    } else if (SvStringEqualToCString(SVSTRING("DVB"), mode) || SvStringEqualToCString(SVSTRING("HYBRID"), mode)) {
        const char* tunerType = QBConfigGet("DEFAULTTUNERTYPE");
        if (strcmp(tunerType, "unusedOption") != 0)
            QBWizardLogicStartChannelScanning(self);
        else
            QBInitLogicEnterApplication(self->appGlobals->initLogic);
    }
}

QBWizardOption QBWizardLogicCreateWizardOption(QBWizardLogic self, QBWizardOption option)
{
    return SVRETAIN(option);
}

SvString QBWizardLogicGetOptionFileName(QBWizardLogic self)
{
    if (self->mode == QBWizardModeDVB)
        return SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/wizard-%s", SvGetRuntimePrefix(), "dvb");

    if (self->mode == QBWizardModeHybrid)
        return SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/wizard-%s", SvGetRuntimePrefix(), "hybrid");

    return SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/wizard-%s", SvGetRuntimePrefix(), "ip");
}

void QBWizardLogicModeChanged(QBWizardLogic self, SvHashTable idToOption, SvHashTable idToSubOption)
{
    SVTESTRELEASE(self->idToOption);
    self->idToOption = SVRETAIN(idToOption);
    SVTESTRELEASE(self->idToSubOption);
    self->idToSubOption = SVRETAIN(idToSubOption);
    SvString currentMode = SVTESTRETAIN(SvHashTableFind(self->idToOption, (SvObject) SVSTRING("mode")));

    if (currentMode) {
        QBConfigSet("HYBRIDMODE", SvStringCString(currentMode));
        QBConfigSave();

        QBWizardLogicRefreshMode(self);
    }

    SVTESTRELEASE(currentMode);
}

SvLocal void QBWizardLogicSetOptionOrDefault(QBWizardLogic self, SvString optionId, const char *defaultValue)
{
    assert(self); assert(optionId);
    SvString value = (SvString) SvHashTableFind(self->idToOption, (SvObject) optionId);
    const char *cValue;
    if (value)
        cValue = SvStringCString(value);
    else
        cValue = defaultValue;
    QBConfigSet(SvStringCString(optionId), cValue);
}

SvLocal void QBWizardLogicSetBinaryOptionWithNumericSubOption(QBWizardLogic self, SvString nameInOption, const char *nameInConfig)
{
    SvString enabledString = (SvString) SvHashTableFind(self->idToOption, (SvObject) nameInOption);
    if (!enabledString || SvStringEqualToCString(enabledString, "disabled")) {
        // FIXME: when user chooses 'disabled', he means it. Using QBConfigSet(..., NULL) means 'pick some defaults for me' and that migth not be the same.
        QBConfigSet(nameInConfig, NULL);
    } else {
        SvValue value = (SvValue) SvHashTableFind(self->idToSubOption, (SvObject) nameInOption);
        if (value) {
            SvString valueStr = SvStringCreateWithFormat("%d", SvValueGetInteger(value));
            QBConfigSet(nameInConfig, SvStringCString(valueStr));
            SVRELEASE(valueStr);
        }
    }
}

void QBWizardLogicSetOption(QBWizardLogic self, SvHashTable idToOption, SvHashTable idToSubOption)
{
    SVTESTRELEASE(self->idToOption);
    self->idToOption = SVRETAIN(idToOption);
    SVTESTRELEASE(self->idToSubOption);
    self->idToSubOption = SVRETAIN(idToSubOption);

    {
        SvString langStr = (SvString) SvHashTableFind(self->idToOption, (SvObject) SVSTRING("lang"));
        if (!langStr)
            langStr = SVSTRING("en_US");

        QBInitLogicSetLocale(self->appGlobals->initLogic, langStr);

        const char* locale = SvStringCString(langStr);
        QBConfigSet("LANG", locale);

        SvString langCode = SvStringCreateWithCStringAndLength(locale, 2, NULL);
        if (self->appGlobals->traxisWebSessionManager)
            TraxisWebSessionManagerSetLanguage(self->appGlobals->traxisWebSessionManager, langCode, NULL);

        SVRELEASE(langCode);
        const char* lang;
        {
            char tmp[3] = {0};
            strncpy(tmp, locale, sizeof(tmp)-1);
            lang = iso639_1to3(tmp);
        }
        const char* defaultLang = "eng";

        const char *primaryAudioLang = QBConfigGet("AUDIOLANG");
        const char *secondaryAudioLang = QBConfigGet("AUDIOLANGSECONDARY");
        if (primaryAudioLang && strcmp(primaryAudioLang, lang) == 0) {
            // current primary audio language selected,
            // keep secondary language if already set
            if (!secondaryAudioLang)
                QBConfigSet("AUDIOLANGSECONDARY", defaultLang);
        } else {
            // other primary audio language selected,
            // use previous primary as new secondary
            QBConfigSet("AUDIOLANGSECONDARY", primaryAudioLang ?: defaultLang);
            QBConfigSet("AUDIOLANG", lang);
        }

        const char *primarySubtitlesLang = QBConfigGet("SUBTITLESLANG");
        const char *secondarySubtitlesLang = QBConfigGet("SUBTITLESLANGSECONDARY");
        if (primarySubtitlesLang && strcmp(primarySubtitlesLang, lang) == 0) {
            // current primary subtitles language selected,
            // keep secondary language if already set
            if (!secondarySubtitlesLang)
                QBConfigSet("SUBTITLESLANGSECONDARY", defaultLang);
        } else {
            // other primary subtitles language selected,
            // use previous primary as new secondary
            QBConfigSet("SUBTITLESLANGSECONDARY", primarySubtitlesLang ?: defaultLang);
            QBConfigSet("SUBTITLESLANG", lang);
        }
    }

    {
        SvString tunerTypeStr = (SvString) SvHashTableFind(self->idToOption, (SvObject) SVSTRING("tuner"));
        if (tunerTypeStr) {
            SvString standard = (SvString) SvHashTableFind(self->idToOption, (SvObject) SVSTRING("standard"));
            if (standard)
                QBConfigSet("SCANSTANDARD", SvStringCString(standard));

            const char* tunerType = SvStringCString(tunerTypeStr);

            QBConfigSet("DEFAULTTUNERTYPE", tunerType);
            if (self->appGlobals->watchdog) {
                QBWatchdogContinue(self->appGlobals->watchdog, NULL);
            }
            QBTunerLogicSwitchAllTuners(self->appGlobals->tunerLogic, tunerType);
        }
    }

    QBWizardLogicSetBinaryOptionWithNumericSubOption(self, SVSTRING("nidSelection"), "NID");
    QBWizardLogicSetBinaryOptionWithNumericSubOption(self, SVSTRING("onidSelection"), "ONID");

    QBWizardLogicSetOptionOrDefault(self, SVSTRING("PROVIDERS.CHANNELS.DVB"), "enabled");
    QBWizardLogicSetOptionOrDefault(self, SVSTRING("PROVIDERS.EPG.DVB"), "eit");
    QBWizardLogicSetOptionOrDefault(self, SVSTRING("PROVIDERS.CHANNELS.IP"), "unusedOption");
    QBWizardLogicSetOptionOrDefault(self, SVSTRING("PROVIDERS.EPG.IP"), "unusedOption");
    QBWizardLogicSetOptionOrDefault(self, SVSTRING("PROVIDERS.NPVR"), "unusedOption");
    QBWizardLogicSetOptionOrDefault(self, SVSTRING("PROVIDERS.CUTV"), "unusedOption");
    QBWizardLogicSetOptionOrDefault(self, SVSTRING("PROVIDERS.VOD"), "cubimw");
    QBWizardLogicSetOptionOrDefault(self, SVSTRING("PROVIDERS.WEBSERVICES"), "cubimw");

    QBWizardLogicSetOptionOrDefault(self, SVSTRING("TUNER.SPECTRAL_INVERSION_MODE"), "normal");


    QBConfigSet("FIRSTBOOT", "NO");
    QBConfigSave();

    self->skipScanning = false;
    QBInitLoadAllServices(self->appGlobals->initializer);
}

void QBWizardLogicEnterRCUPairing(QBWizardLogic self)
{
    QBRCUPairingService rcuPairingService = (QBRCUPairingService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                              SVSTRING("QBRCUPairingService"));
    if (!QBRCUPairingServiceIsPairingNeeded(rcuPairingService, NULL))
        return;

    QBRCUPairingContext rcuPairingContext = QBRCUPairingContextCreate();
    QBApplicationController controller = (QBApplicationController) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                               SVSTRING("QBApplicationController"));
    QBApplicationControllerPushContext(controller, (QBWindowContext) rcuPairingContext);
    QBRCUPairingServiceStartPairing(rcuPairingService, -1);
    SVRELEASE(rcuPairingContext);
}

bool QBWizardLogicHandleInput(QBWizardLogic self, const QBInputEvent* e)
{
    if (self->appGlobals->volumeWidget && QBVolumeWidgetInputEventHandler(self->appGlobals->volumeWidget, e)) {
        return true;
    }

    QBDualOutputHandler dualOutputHandler =
        (QBDualOutputHandler) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                          SVSTRING("QBDualOutputHandler"));
    if (dualOutputHandler && QBDualOutputHanderInputEventHandler(dualOutputHandler, e)) {
        return true;
    }

    switch (e->u.key.code) {
        case QBKEY_ABORT:
            // allow to quit application in wizard screen
            return false;
        default:
            break;
    }
    return true;
}

void QBWizardLogicAddListener(QBWizardLogic self, SvObject listener)
{

}

void QBWizardLogicRemoveListener(QBWizardLogic self, SvObject listener)
{

}
