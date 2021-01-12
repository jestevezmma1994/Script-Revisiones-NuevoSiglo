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

#include <Logic/VideoOutputLogic.h>
#include <Logic/QBLogicFactory.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBPlatformHAL/QBPlatformOutput.h>
#include <QBPlatformHAL/QBPlatformRFOutput.h>
#include <QBPlatformHAL/QBPlatformUtil.h>
#include <QBPlatformHAL/QBPlatformOption.h>
#include <Configurations/overscanMargins.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <SvGfxHAL/SvGfxEngine.h>
#include <QBViewport.h>
#include <QBAppKit/QBObserver.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <QBConf.h>
#include <main.h>
#include <Services/OutputStandardService.h>
#include <Services/HDMIService.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvEnv.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_force_separated, 0, "VideoOutputLogicForceOutputConfigurationSeparated", "");

#define NUM_RF_MODULATOR_CHANNELS 10

SvLocal inline bool VideoOutputLogicRFModulatorFindChannel(VideoOutputLogic self, unsigned int RFOutputID, unsigned int channel)
{
    return SvInvokeVirtual(VideoOutputLogic, self, RFModulatorFindChannel, RFOutputID, channel);
}

SvLocal inline void VideoOutputLogicSetAnalogVideoMode(VideoOutputLogic self, QBOutputStandard newMode)
{
    SvInvokeVirtual(VideoOutputLogic, self, setAnalogVideoMode, newMode);
}

SvLocal void VideoOutputLogicInit_(VideoOutputLogic self, AppGlobals appGlobals)
{
    self->appGlobals = appGlobals;

    self->tvSystem = QBPlatformGetTVSystemByName(QBConfigGet("TVSYSTEM"));
    self->serviceState = QBAsyncServiceState_idle;

    const char *v = QBConfigGet("AUTOMATICFRAMERATE");
    self->automaticFrameRateSelectionEnabled = (v && !strcmp(v, "enabled"));

    self->selectedAnalogMode = QBOutputStandard_none;

    self->videoInfoPresent = false;

    int i = QBPlatformFindOutput(QBOutputType_unknown, NULL, QBOutputCapability_SD, true, false);
    if (i >= 0) {
        self->analogOutputsPresent = true;
    }
    self->dependencies = SvImmutableArrayCreateWithTypedValues("s", NULL, "QBDualOutputHandler");
}

SvLocal void VideoOutputLogicSetAnalogVideoMode_(VideoOutputLogic self, QBOutputStandard newMode)
{
    QBVideoOutputConfig cfg;

    if (newMode == self->selectedAnalogMode && self->selectedAnalogMode != QBOutputStandard_none)
        return;

    self->selectedAnalogMode = newMode;

    unsigned int count = QBPlatformGetOutputsCount();
    for (unsigned int i = 0; i < count; i++) {
        if (QBPlatformGetOutputConfig(i, &cfg) == 0) {
            if (!cfg.enabled)
                continue;
            if ((cfg.capabilities & QBOutputCapability_SD) != 0 && cfg.masterID == i && cfg.mode != newMode) {
                QBOutputStandardServiceChangeMode(self->appGlobals->outputStandardService, newMode, i);
            }
        }
    }
}

SvLocal QBOutputStandard VideoOutputLogicSelectAnalogVideoMode(VideoOutputLogic self)
{
    QBOutputStandard newMode = QBOutputStandard_current;

    if (!self->automaticFrameRateSelectionEnabled || !self->videoInfoPresent) {
        newMode = (self->tvSystem == QBTVSystem_NTSC) ? QBOutputStandard_480i59 : QBOutputStandard_576i50;
        return newMode;
    }

    QBTVSystem tvSystem = QBPlatformGetTVSystemByVideoParams(self->videoInfo.height, self->videoInfo.frameRate);
    if (tvSystem != QBTVSystem_unknown) {
        newMode = (tvSystem == QBTVSystem_NTSC) ? QBOutputStandard_480i59 : QBOutputStandard_576i50;
    }

    if (newMode != QBOutputStandard_current) {
        SvLogNotice("VideoOutputLogic: using %s mode for analog outputs for %dx%d %d.%03d FPS content in automatic frame rate selection mode",
                    QBPlatformGetModeName(newMode),
                    self->videoInfo.width, self->videoInfo.height,
                    self->videoInfo.frameRate / 1000, self->videoInfo.frameRate % 1000);
    }

    return newMode;
}

SvLocal void VideoOutputLogicHandleDeinterlacerState(VideoOutputLogic self)
{
    const char* deinterlace = QBConfigGet("DEINTERLACE_MODE");
    if (!deinterlace || strncmp(deinterlace, "none", sizeof("none") - 1) == 0)
        return;

    bool newState = true;

    if (strncmp(deinterlace, "auto", sizeof("auto") - 1) == 0) {
        newState = self->videoInterlaced;
    } else if (strncmp(deinterlace, "forceDisable", sizeof("forceDisable") - 1) == 0) {
        newState = false;
    } else if (strncmp(deinterlace, "forceEnable", sizeof("forceEnable") - 1) == 0) {
        newState = true;
    } else {
        SvLogWarning("Unknown deinterlace mode: %s", deinterlace);
        return;
    }

    QBVideoOutputConfig cfg;
    for (unsigned int i = 0; QBPlatformGetOutputConfig(i, &cfg) >= 0; i++) {
        if (cfg.masterID != i)
            continue;

        if (cfg.active) {
            SVAUTOSTRING(outputNameStr, cfg.name);
            SVAUTOSTRINGVALUE(outputNameVal, outputNameStr);
            SVAUTOSTRING(newStateStr, newState ? "enable-deinterlace" : "disable-deinterlace");

            SvLogNotice("VideoOutputLogicHandleDeinterlacerState - %s deinterlace for output %s", newState ? "Enabling" : "Disabling", cfg.name);
            QBPlatformSetOption(newStateStr, outputNameVal);

            // We cann't enable deinterlace on more then one output
            if (newState == true)
                return;
        }
    }
}

SvLocal void VideoOutputLogicVideoFormatChanged(SvGenericObject self_,
                                                const QBViewportVideoInfo *videoInfo)
{
    VideoOutputLogic self = (VideoOutputLogic) self_;

    self->videoInfo = *videoInfo;
    self->videoInfoPresent = true;

    if (self->analogOutputsPresent && self->automaticFrameRateSelectionEnabled) {
        QBOutputStandard newMode = VideoOutputLogicSelectAnalogVideoMode(self);
        VideoOutputLogicSetAnalogVideoMode(self, newMode);
    }

    self->videoInterlaced = videoInfo->interlaced;
    VideoOutputLogicHandleDeinterlacerState(self);
}

SvLocal void VideoOutputLogicVideoFormatInfoUpdated(SvObject self_,
                                                    const QBViewportVideoInfo *videoInfo)
{
}

SvLocal SvString VideoOutputLogicGetName(SvObject self_)
{
    return SVSTRING("VideoOutputLogic");
}

SvLocal SvImmutableArray VideoOutputLogicGetDependencies(SvObject self_)
{
    VideoOutputLogic self = (VideoOutputLogic) self_;
    return self->dependencies;
}

SvLocal QBAsyncServiceState VideoOutputLogicGetState(SvObject self_)
{
    VideoOutputLogic self = (VideoOutputLogic) self_;
    return self->serviceState;
}

SvLocal void VideoOutputLogicOutputChanged(SvObject self_, SvObject observedObject, SvObject arg)
{
    QBDualOutputHandler dualOutputHandler = (QBDualOutputHandler) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBDualOutputHandler"));
    if (!dualOutputHandler ||
        dualOutputHandler != (QBDualOutputHandler) observedObject) {
        return;
    }
    VideoOutputLogic self = (VideoOutputLogic) self_;
    VideoOutputLogicHandleDeinterlacerState(self);
}

SvLocal void VideoOutputLogicStart(SvObject self_, SvScheduler scheduler, SvErrorInfo *errorOut)
{
    VideoOutputLogic self = (VideoOutputLogic) self_;
    self->serviceState = QBAsyncServiceState_starting;
    if (self->appGlobals->videoFormatMonitor) {
        QBVideoFormatMonitorAddListener(self->appGlobals->videoFormatMonitor, (SvObject) self);
    }
    QBDualOutputHandler dualOutputHandler = (QBDualOutputHandler) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBDualOutputHandler"));
    if (dualOutputHandler) {
        QBObservableAddObserver((QBObservable) dualOutputHandler, (SvObject) self, NULL);
        VideoOutputLogicHandleDeinterlacerState(self);
    }
    self->serviceState = QBAsyncServiceState_running;
}

SvLocal void VideoOutputLogicStop(SvObject self_, SvErrorInfo *errorOut)
{
    VideoOutputLogic self = (VideoOutputLogic) self_;
    self->serviceState = QBAsyncServiceState_stopping;
    if (self->appGlobals->videoFormatMonitor) {
        QBVideoFormatMonitorRemoveListener(self->appGlobals->videoFormatMonitor, (SvObject) self);
    }
    QBDualOutputHandler dualOutputHandler = (QBDualOutputHandler) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBDualOutputHandler"));
    if (dualOutputHandler) {
        QBObservableRemoveObserver((QBObservable) dualOutputHandler, (SvObject) self, NULL);
    }
    self->serviceState = QBAsyncServiceState_idle;
}

SvLocal void VideoOutputLogicVerifyConfig_(VideoOutputLogic self, QBVideoOutputConfig *outputs, unsigned int outputsCnt, int *RFChannel)
{
    int YPbPrOutputID = QBPlatformFindOutput(QBOutputType_YPbPr, NULL, 0, false, false);
    int RGBOutputID = QBPlatformFindOutput(QBOutputType_RGB, NULL, 0, false, false);
    if (YPbPrOutputID >= 0 && RGBOutputID >= 0) {
        // disable either RGB or YPbPr output according to SCART signal type
        const char *SCART = QBConfigGet("VIDEOOUTEURO");
        if (SCART && !strcmp(SCART, "RGB")) {
            outputs[YPbPrOutputID].enabled = false;
            outputs[RGBOutputID].enabled = true;
        } else {
            outputs[YPbPrOutputID].enabled = true;
            outputs[RGBOutputID].enabled = false;
        }
    }

    int RFOutputID = QBPlatformFindOutput(QBOutputType_RF, NULL, 0, false, false);
    if (RFOutputID >= 0) {
        const char *RF = QBConfigGet("RFMODULATORCHANNEL");
        if ((RF == NULL) || (strcmp(RF, "disabled") == 0))
            outputs[RFOutputID].enabled = false;
        else {
            errno = 0;
            char *endPtr = NULL;
            unsigned int channel = strtoul(RF, &endPtr, 0);
            if ((errno != 0) || (*endPtr != '\0')) {
                SvLogError("%s: invalid value of RFMODULATORCHANNEL variable: %s ", __func__, RF);
                outputs[RFOutputID].enabled = false;
            } else {
                bool found = VideoOutputLogicRFModulatorFindChannel(self, RFOutputID, channel);

                if (found) {
                    outputs[RFOutputID].enabled = true;
                    *RFChannel = channel;
                } else {
                    SvLogWarning("%s: RFMODULATORCHANNEL is %u, but this RF channel is not supported -> disabling RF output", __func__, channel);
                    outputs[RFOutputID].enabled = false;
                }
            }
        }
    }
}

SvLocal void VideoOutputLogicSetupOverscanCompensation_(VideoOutputLogic self)
{
    QBVideoOutputConfig cfg = {
        .name = NULL,
        .enabled = false,
    };

    if (!self->analogOutputsPresent)
        return;

    // we assume there is only one master analog video output
    unsigned int analogMasterOutputID = QBPlatformFindOutput(QBOutputType_unknown, NULL, QBOutputCapability_SD, true, false);
    if (QBPlatformGetOutputConfig(analogMasterOutputID, &cfg) >= 0 && cfg.enabled && analogMasterOutputID == cfg.masterGfxID) {
        SvLogNotice("CubiTV: activating overscan compensation on SD outputs");
        Sv2DRect outputRect = {
            .width = 0,
            .height = 0,
        };
        SvString outputMarginsFileName = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/overscan-margins", SvGetRuntimePrefix());
        QBOverscanMarginsGetRect(&outputRect, cfg, outputMarginsFileName);
        SVRELEASE(outputMarginsFileName);
        SvGfxEngineSetupViewport(analogMasterOutputID, QBViewportMode_windowed, &outputRect, NULL);
    }
}

SvLocal void VideoOutputLogicSetSCARTSignalType_(VideoOutputLogic self, const char *type)
{
    QBVideoOutputConfig cfg, masterCfg;

    if (!self->analogOutputsPresent)
        return;

    // we assume there is only one master analog video output
    unsigned int analogMasterOutputID = QBPlatformFindOutput(QBOutputType_unknown, NULL, QBOutputCapability_SD, true, false);
    if (QBPlatformGetOutputConfig(analogMasterOutputID, &masterCfg) < 0 || !masterCfg.enabled)
        return;

    int CVBSOutputID = QBPlatformFindOutput(QBOutputType_CVBS, NULL, 0, false, false);
    int RGBOutputID = QBPlatformFindOutput(QBOutputType_RGB, NULL, 0, false, false);
    int YPbPrOutputID = QBPlatformFindOutput(QBOutputType_YPbPr, NULL, 0, false, false);

    if (!strcmp(type, "RGB")) {
        // output RGB signal on SCART
        if (YPbPrOutputID >= 0) {
            // disable YPbPr output
            QBPlatformGetOutputConfig(YPbPrOutputID, &cfg);
            if (cfg.enabled) {
                QBPlatformOutputSetPowerOff(YPbPrOutputID, true, QBPlatformOutputPriority_application);
                QBPlatformOutputDisable(YPbPrOutputID);
            }
        }
        if (RGBOutputID >= 0) {
            // enable RGB output
            QBPlatformGetOutputConfig(RGBOutputID, &cfg);
            if ((int) cfg.masterID != RGBOutputID) {
                QBPlatformGetOutputConfig(cfg.masterID, &masterCfg);
                cfg.mode = masterCfg.mode;
                cfg.aspectRatio = masterCfg.aspectRatio;
            }
            QBPlatformOutputEnable(RGBOutputID, cfg.mode, cfg.aspectRatio);
            QBPlatformOutputSetPowerOff(RGBOutputID, false, QBPlatformOutputPriority_application);
        }
    } else {
        // output CVBS signal on SCART
        if (RGBOutputID >= 0) {
            // disable RGB output
            QBPlatformGetOutputConfig(RGBOutputID, &cfg);
            if (cfg.enabled) {
                QBPlatformOutputSetPowerOff(RGBOutputID, true, QBPlatformOutputPriority_application);
                QBPlatformOutputDisable(RGBOutputID);
            }
        }
        if (CVBSOutputID >= 0) {
            // enable CVBS output
            QBPlatformGetOutputConfig(CVBSOutputID, &cfg);
            if (!cfg.enabled || cfg.powerOff) {
                if ((int) cfg.masterID != CVBSOutputID) {
                    QBPlatformGetOutputConfig(cfg.masterID, &masterCfg);
                    cfg.mode = masterCfg.mode;
                    cfg.aspectRatio = masterCfg.aspectRatio;
                }
                QBPlatformOutputEnable(CVBSOutputID, cfg.mode, cfg.aspectRatio);
                QBPlatformOutputSetPowerOff(CVBSOutputID, false, QBPlatformOutputPriority_application);
            }
        }
        if (YPbPrOutputID >= 0) {
            // enable YPbPr output
            QBPlatformGetOutputConfig(YPbPrOutputID, &cfg);
            if ((int) cfg.masterID != YPbPrOutputID) {
                QBPlatformGetOutputConfig(cfg.masterID, &masterCfg);
                cfg.mode = masterCfg.mode;
                cfg.aspectRatio = masterCfg.aspectRatio;
            }
            QBPlatformOutputEnable(YPbPrOutputID, cfg.mode, cfg.aspectRatio);
            QBPlatformOutputSetPowerOff(YPbPrOutputID, false, QBPlatformOutputPriority_application);
        }
    }
}

void VideoOutputLogicSetRFModulator(VideoOutputLogic self, const char *type)
{
    QBVideoOutputConfig cfg, masterCfg;

    if (!self->analogOutputsPresent)
        return;

    // we assume there is only one master analog video output
    unsigned int analogMasterOutputID = QBPlatformFindOutput(QBOutputType_unknown, NULL, QBOutputCapability_SD, true, false);
    if (QBPlatformGetOutputConfig(analogMasterOutputID, &masterCfg) < 0 || !masterCfg.enabled)
        return;

    int RFOutputID = QBPlatformFindOutput(QBOutputType_RF, NULL, 0, false, false);
    if (RFOutputID >= 0) {
        if (strcmp(type, "disabled") != 0) {
            errno = 0;
            char *endPtr = NULL;
            unsigned int channel = strtoul(type, &endPtr, 0);
            if ((errno != 0) || (*endPtr != '\0')) {
                SvLogError("%s: invalid value of RF Channel argument: %s", __func__, type);
                QBPlatformGetOutputConfig(RFOutputID, &cfg);
                if ((int) cfg.masterID != RFOutputID) {
                    QBPlatformGetOutputConfig(cfg.masterID, &masterCfg);
                    cfg.mode = masterCfg.mode;
                    cfg.aspectRatio = masterCfg.aspectRatio;
                }
                QBPlatformOutputSetPowerOff(RFOutputID, true, QBPlatformOutputPriority_application);
                QBPlatformOutputDisable(RFOutputID);
            } else {
                bool found = VideoOutputLogicRFModulatorFindChannel(self, RFOutputID, channel);

                if (!found) {
                    SvLogWarning("%s: RF Channel argument is %s, but this RF channel is not supported -> doing nothing", __func__, type);
                    return;
                }

                QBPlatformRFOutputSetChannelNumber(RFOutputID, channel);

                QBPlatformGetOutputConfig(RFOutputID, &cfg);
                if ((int) cfg.masterID != RFOutputID) {
                    QBPlatformGetOutputConfig(cfg.masterID, &masterCfg);
                    cfg.mode = masterCfg.mode;
                    cfg.aspectRatio = masterCfg.aspectRatio;
                }
                QBPlatformOutputEnable(RFOutputID, cfg.mode, cfg.aspectRatio);
                QBPlatformOutputSetPowerOff(RFOutputID, false, QBPlatformOutputPriority_application);
            }
        } else {
            QBPlatformGetOutputConfig(RFOutputID, &cfg);
            if ((int) cfg.masterID != RFOutputID) {
                QBPlatformGetOutputConfig(cfg.masterID, &masterCfg);
                cfg.mode = masterCfg.mode;
                cfg.aspectRatio = masterCfg.aspectRatio;
            }
            QBPlatformOutputSetPowerOff(RFOutputID, true, QBPlatformOutputPriority_application);
            QBPlatformOutputDisable(RFOutputID);
        }
    }
}

SvLocal void VideoOutputLogicTVSystemChanged_(VideoOutputLogic self, QBTVSystem tvSystem)
{
    if (!self) {
        SvLogError("%s(): NULL self passed", __func__);
        return;
    } else if (tvSystem != QBTVSystem_PAL && tvSystem != QBTVSystem_NTSC) {
        SvLogError("%s(): invalid TV system %d passed", __func__, (int) tvSystem);
        return;
    }

    self->tvSystem = tvSystem;

    const char *sdConfig = QBConfigGet("VIDEOOUTSD");
    if (sdConfig && strcasecmp(sdConfig, "NONE")) {
        QBConfigSet("VIDEOOUTSD", QBPlatformGetTVSystemName(tvSystem));
    }

    if (self->analogOutputsPresent) {
        if (!self->automaticFrameRateSelectionEnabled) {
            QBOutputStandard newMode = (tvSystem == QBTVSystem_NTSC) ? QBOutputStandard_480i59 : QBOutputStandard_576i50;
            VideoOutputLogicSetAnalogVideoMode(self, newMode);
        }
    }

    QBConfigSet("TVSYSTEM", QBPlatformGetTVSystemName(tvSystem));
}

SvLocal bool VideoOutputLogicRFModulatorFindChannel_(VideoOutputLogic self, unsigned int RFOutputID, unsigned int channel)
{
    unsigned int channels[NUM_RF_MODULATOR_CHANNELS];
    ssize_t RFMNumOfChannels = 0;
    size_t i = 0;

    RFMNumOfChannels = QBPlatformRFOutputGetSupportedChannels(RFOutputID, channels, NUM_RF_MODULATOR_CHANNELS);
    if (RFMNumOfChannels < 0)
        return false;
    for (i = 0; i < NUM_RF_MODULATOR_CHANNELS && i < (size_t) RFMNumOfChannels; i++) {
        if (channels[i] == channel) {
            return true;
        }
    }
    return false;
}

SvLocal void VideoOutputLogicDestructor(void* self_)
{
    VideoOutputLogic self = (VideoOutputLogic) self_;
    SVTESTRELEASE(self->dependencies);
}

SvType VideoOutputLogic_getType(void)
{
    static SvType type = NULL;

    static const struct VideoOutputLogicVTable_ logicVTable = {
        .super_                    = {
            .destroy               = VideoOutputLogicDestructor
        },
        .init                      = VideoOutputLogicInit_,
        .setupOverscanCompensation = VideoOutputLogicSetupOverscanCompensation_,
        .setSCARTSignalType        = VideoOutputLogicSetSCARTSignalType_,
        .TVSystemChanged           = VideoOutputLogicTVSystemChanged_,
        .verifyConfig              = VideoOutputLogicVerifyConfig_,
        .RFModulatorFindChannel    = VideoOutputLogicRFModulatorFindChannel_,
        .setAnalogVideoMode        = VideoOutputLogicSetAnalogVideoMode_
    };

    static struct QBVideoFormatListener_ methods = {
        .formatChanged = VideoOutputLogicVideoFormatChanged,
        .infoUpdated   = VideoOutputLogicVideoFormatInfoUpdated
    };

    static struct QBObserver_ observerMethods = {
        .observedObjectUpdated = VideoOutputLogicOutputChanged
    };

    static struct QBAsyncService_ asyncServiceMethods = {
        .getName         = VideoOutputLogicGetName,
        .getDependencies = VideoOutputLogicGetDependencies,
        .getState        = VideoOutputLogicGetState,
        .start           = VideoOutputLogicStart,
        .stop            = VideoOutputLogicStop
    };

    if (unlikely(!type)) {
        SvTypeCreateVirtual("VideoOutputLogic",
                            sizeof(struct VideoOutputLogic_),
                            SvObject_getType(),
                            sizeof(logicVTable), &logicVTable,
                            &type,
                            QBVideoFormatListener_getInterface(), &methods,
                            QBAsyncService_getInterface(), &asyncServiceMethods,
                            QBObserver_getInterface(), &observerMethods,
                            NULL);
    }

    return type;
}

VideoOutputLogic VideoOutputLogicCreate(AppGlobals appGlobals, SvErrorInfo *errorOut)
{
    VideoOutputLogic self = NULL;
    SvErrorInfo error = NULL;

    SvType logicClass = QBLogicFactoryFindImplementationOf(QBLogicFactoryGetInstance(), VideoOutputLogic_getType(), &error);
    if (!logicClass)
        goto fini;

    self = (VideoOutputLogic) SvTypeAllocateInstance(logicClass, &error);

    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "can't allocate %s", SvTypeGetName(logicClass));
        goto fini;
    }

    VideoOutputLogicInit(self, appGlobals);

fini:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}


void VideoOutputLogicEnableAutomaticFrameRateSelection(VideoOutputLogic self)
{
    if (!self) {
        SvLogError("%s(): NULL self passed", __func__);
        return;
    }

    self->automaticFrameRateSelectionEnabled = true;

    if (self->analogOutputsPresent) {
        QBOutputStandard newMode = VideoOutputLogicSelectAnalogVideoMode(self);
        VideoOutputLogicSetAnalogVideoMode(self, newMode);
    }
}

void VideoOutputLogicDisableAutomaticFrameRateSelection(VideoOutputLogic self)
{
    if (!self) {
        SvLogError("%s(): NULL self passed", __func__);
        return;
    }

    self->automaticFrameRateSelectionEnabled = false;

    if (self->analogOutputsPresent) {
        QBOutputStandard newMode = (self->tvSystem == QBTVSystem_NTSC) ? QBOutputStandard_480i59 : QBOutputStandard_576i50;
        VideoOutputLogicSetAnalogVideoMode(self, newMode);
    }
}

int VideoOutputLogicSetContentProtection(VideoOutputLogic self, const struct sv_content_protection *contentProtectionInfo)
{
    int ret = 0;
    unsigned int i = 0;
    unsigned int count = QBPlatformGetOutputsCount();
    QBVideoOutputConfig cfg;
    for (i = 0; i < count; i++) {
        if (QBPlatformGetOutputConfig(i, &cfg) == 0) {
            if (i != cfg.masterID)
                continue;
            if (QBPlatformUpdateContentProtection(i, contentProtectionInfo, QBPlatformOutputPriority_application) < 0)
                ret = -1;
        }
    }

    return ret;
}

void VideoOutputLogicAfterOutputsSetup(VideoOutputLogic self)
{
}

void VideoOutputLogicSetVideoContentMode(VideoOutputLogic self, const char* outputType, const char* modeName)
{
    if (VideoOutputLogicIsOutputConfigurationSeparated(self)) {
        lstvSetVideoContentMode(QBViewportGet(), outputType, modeName);
    } else {
        lstvSetVideoContentMode(QBViewportGet(), "SD", modeName);
        lstvSetVideoContentMode(QBViewportGet(), "HD", modeName);
    }
}

SvLocal QBContentDisplayMode VideoOutputLogicGetNextDisplayMode(const QBContentDisplayMode mode)
{
    // loop: stretched, letterBox, panScan
    switch (mode) {
        case QBContentDisplayMode_stretched:
            return QBContentDisplayMode_letterBox;
        case QBContentDisplayMode_letterBox:
            return QBContentDisplayMode_panScan;
        case QBContentDisplayMode_panScan:
            return QBContentDisplayMode_stretched;
        default:
            return mode;
    }
}

extern void VideoOutputLogicToggleVideoContentMode(VideoOutputLogic self)
{
    static const struct {
        QBOutputCapability capability;
        const char *confKey;
        const char *outputType;
    } outputs[] = {
        { QBOutputCapability_SD, "VIDEO_CONTENT_MODE_SD", "SD" },
        { QBOutputCapability_HD, "VIDEO_CONTENT_MODE_HD", "HD" },
    };
    for (size_t i = 0; i < sizeof(outputs) / sizeof(outputs[0]); ++i) {
        const int outputID = QBPlatformFindOutput(QBOutputType_unknown, NULL, outputs[i].capability, true, false);
        if (outputID < 0)
            continue;
        QBContentDisplayMode mode = QBPlatformFindContentDisplayModeByName(QBConfigGet(outputs[i].confKey));
        if (mode == QBContentDisplayMode_unknown)
            continue;
        QBContentDisplayMode nextMode = VideoOutputLogicGetNextDisplayMode(mode);
        VideoOutputLogicSetVideoContentMode(self, outputs[i].outputType, QBPlatformGetContentDisplayModeName(nextMode));
        QBConfigSet(outputs[i].confKey, QBPlatformGetContentDisplayModeName(nextMode));
    }
}

bool VideoOutputLogicIsOutputConfigurationSeparated(VideoOutputLogic self)
{
    if (env_force_separated()) {
        return true;
    }

    int SDOutputID = QBPlatformFindOutput(QBOutputType_unknown, NULL, QBOutputCapability_SD, true, false);
    QBVideoOutputConfig cfg = {
        .name = NULL,
        .enabled = false,
    };
    bool SDHasOwnGfx = (SDOutputID >= 0) && (QBPlatformGetOutputConfig(SDOutputID, &cfg) == 0) && (cfg.masterGfxID == cfg.masterID);
    return SDHasOwnGfx || SvInvokeInterface(QBCAS, QBCASGetInstance(), getCASType) != QBCASType_conax;
}
