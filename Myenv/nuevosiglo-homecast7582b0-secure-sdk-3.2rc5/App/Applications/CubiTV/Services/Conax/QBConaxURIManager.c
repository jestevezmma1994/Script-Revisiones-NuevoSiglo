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
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include "QBConaxURIManager.h"
#include "QBConaxPVRManager.h"
#include <SvCore/SvLog.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvWeakList.h>

#include <QBSmartcard2Interface.h>
#include <QBICSmartcardURIData.h>
#include <QBICSmartcardDecryptionState.h>
#include <QBICSmartcardOption.h>
#include <QBCAS.h>

#include <QBPlatformHAL/QBPlatformAudioOutput.h>
#include <QBPlatformHAL/QBPlatformOutput.h>
#include <QBPlatformHAL/QBPlatformViewport.h>
#include <QBPlatformHAL/QBPlatformImpl.h>

#include <dataformat/sv_data_format.h>
#include <SvPlayerManager/SvPlayerTask.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <QBDRMManager/QBDRMManager.h>

#include <stdbool.h>
#include <stddef.h>
#include <main.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBConaxURIManager"
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, moduleName "LogLevel", "");
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

struct QBConaxURIManager_s {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    QBICSmartcardURIData currentUriData;

    unsigned int analogueOutputs[QBVideoOutputs__max__];
    size_t analogueOutputsCnt;

    unsigned int digitalOutputs[QBVideoOutputs__max__];
    size_t digitalOutputsCnt;

    bool isFta;

    bool isServiceStarted;

    SvWeakList listeners;

    QBICSmartcardOptionConaxVersion conaxVersion;
};

SvLocal
void
QBConaxURIManagerDestroy(void *self_)
{
    log_fun();
    QBConaxURIManager self = (QBConaxURIManager) self_;
    if (self->isServiceStarted)
        QBConaxURIManagerStop(self);

    SVTESTRELEASE(self->currentUriData);
    SVTESTRELEASE(self->listeners);
}

// Analogue outputs handling

/**
 * This function should mute/unmute all analogue audio outputs
 * according to @a mute parameter.
 * @param[in] mute indicates if audio outputs should be muted (if @c true) or unmuted (if @c false)
 */
SvLocal
void
QBConaxURIManagerMuteAnalogueAudioOutputs(QBConaxURIManager self, bool mute)
{
    log_fun();
    int audioOutput = QBPlatformFindAudioOutput(QBAudioOutputType_analog, NULL);
    if (audioOutput >= 0) {
        QBPlatformMuteAudioOutput(audioOutput, mute, QBPlatformOutputPriority_content);
    }
}

/**
 * This function should enable @a analogueOutput according to @a enable
 * @param[in] analogueOutput analogue output ID
 * @param[in] enable indicates if output should be enabled (if @c true) or disabled (if @c false)
 */
SvLocal
void
QBConaxURIManagerEnableAnalogueOutput(QBConaxURIManager self, unsigned int analogueOutput, bool enable)
{
    log_fun();
    QBVideoOutputConfig cfg;
    QBPlatformGetOutputConfig(analogueOutput, &cfg);
    log_info("%s analogue output %s (%u)", (enable) ? "Enable" : "Disable", cfg.name, analogueOutput);

    if (cfg.enabled) {
        QBPlatformOutputMute(analogueOutput, !enable, QBPlatformOutputPriority_content);
    }
}

/**
 * This function should try to set analogue protection system (APS) on
 * @a analogueOutput according to @a apsType value.
 * @param[in] analogueOutput analogue output ID
 * @param[in] apsType indicates APS mode @ref QBICSmartcardURIDataAPSCopyControl
 * @return @c true if we successfully set APS according to @a apsType, @c false if we don't
 */
SvLocal
bool
QBConaxURIManagerTryToSetAnalogueProtection(QBConaxURIManager self, unsigned int analogueOutput, QBICSmartcardURIDataAPSCopyControl apsType)
{
    log_fun();
    QBVideoOutputConfig cfg;
    QBPlatformGetOutputConfig(analogueOutput, &cfg);
    log_info("Try to set analogue protection, type (%d), output %s (%u) ", apsType, cfg.name, analogueOutput);

    if ((apsType != QBICSmartcardURIDataAPSCopyControl_off) && !(cfg.capabilities & QBOutputCapability_Macrovision)) {
        log_info("Output (%s) (%d) does not have Macrovision capability", cfg.name, analogueOutput);
        return false;
    }

    switch (apsType) {
        case QBICSmartcardURIDataAPSCopyControl_off:
            cfg.contentProtection.protection.analog_video.macrovision_settings.mode = QBContentProtectionMacrovisionMode_disabled;
            QBPlatformUpdateContentProtection(analogueOutput, &cfg.contentProtection, QBPlatformOutputPriority_content);
            // Here we want to turn off APS protection, we don't care if it fails or not.
            return true;
        case QBICSmartcardURIDataAPSCopyControl_splitBurstOff:
            cfg.contentProtection.protection.analog_video.macrovision_settings.mode = QBContentProtectionMacrovisionMode_agcOnly;
            if (QBPlatformUpdateContentProtection(analogueOutput, &cfg.contentProtection, QBPlatformOutputPriority_content) == 0)
                return true;
            break;
        case QBICSmartcardURIDataAPSCopyControl_2LineSplitBurst:
            cfg.contentProtection.protection.analog_video.macrovision_settings.mode = QBContentProtectionMacrovisionMode_agc2Lines;
            if (QBPlatformUpdateContentProtection(analogueOutput, &cfg.contentProtection, QBPlatformOutputPriority_content) == 0)
                return true;
            break;
        case QBICSmartcardURIDataAPSCopyControl_4LineSplitBurst:
            cfg.contentProtection.protection.analog_video.macrovision_settings.mode = QBContentProtectionMacrovisionMode_agc4Lines;
            if (QBPlatformUpdateContentProtection(analogueOutput, &cfg.contentProtection, QBPlatformOutputPriority_content) == 0)
                return true;
            break;
    }

    return false;
}

/**
 * This function should try to downscale @a analogueOutput to < 520k pixels according to @a imageConstraint
 * @param[in] analogueOutput analogue output ID
 * @param[in] imageConstraint indicates if we should do downscaling @ref QBICSmartcardURIDataImageConstraint
 * @return @c true if we successfully downscale output or if output size is < 520k, @a false if we cannot downscale output
 */
SvLocal
bool
QBConaxURIManagerTryDownscaleAnalogueOutput(QBConaxURIManager self, unsigned int analogueOutput, QBICSmartcardURIDataImageConstraint imageConstraint)
{
    log_fun();
    // We don't support that now, so if we have HD analogue output, we return false to turn it off
    // TODO try to implement downscaling on HD analogue outputs

    QBVideoOutputConfig cfg;
    QBPlatformGetOutputConfig(analogueOutput, &cfg);
    log_info("Try downscale analogue output %s (%u), image constraint %d", cfg.name, analogueOutput, imageConstraint);

    switch (imageConstraint) {
        case QBICSmartcardURIDataImageConstraint_lessThen520kPixels:
            if (cfg.width * cfg.height < 520000)
                return true;
            break;
        case QBICSmartcardURIDataImageConstraint_off:
            return true;
    }

    return false;
}

SvLocal
void
QBConaxURIManagerHandleAnalogueOutput(QBConaxURIManager self, unsigned int analogueOutput, const QBICSmartcardURIDataUsageRulesInformation* uri, bool* disableAudio)
{
    log_fun();
    bool enableOutput = true;

    if (uri->disableAnalogueOutputs == QBICSmartcardURIDataDisableAnalogueOutputs_on) {
        log_info("All analogue outputs must be disabled, Disabling output (%u)", analogueOutput);
        enableOutput = false;
    } else if (QBConaxURIManagerTryToSetAnalogueProtection(self, analogueOutput, uri->apsCopyControl) == false) {
        log_error("Cannot set Analogue Protection on output %d, APS mode: %d. Disabling this display...", analogueOutput, uri->apsCopyControl);
        enableOutput = false;
    } else if (QBConaxURIManagerTryDownscaleAnalogueOutput(self, analogueOutput, uri->imageConstraint) == false) {
        log_error("Cannot downscale HD content on output %d. Disabling this display...", analogueOutput);
        enableOutput = false;
    }

    QBConaxURIManagerEnableAnalogueOutput(self, analogueOutput, enableOutput);
    *disableAudio = !enableOutput;
}

SvLocal
void
QBConaxURIManagerHandleAnalogueOutputs(QBConaxURIManager self)
{
    log_fun();
    bool disableAllAnalogueOutputs = true;
    for (size_t i = 0; i < self->analogueOutputsCnt; ++i) {
        bool disableAudio = false;
        QBConaxURIManagerHandleAnalogueOutput(self, self->analogueOutputs[i], &self->currentUriData->usageRulesInformation, &disableAudio);
        disableAllAnalogueOutputs = disableAudio && disableAllAnalogueOutputs;
    }
    // analogue audio could be disabled only if all analogue outputs will be disabled
    QBConaxURIManagerMuteAnalogueAudioOutputs(self, disableAllAnalogueOutputs);
}


// Digital outputs handling

/**
 * This function should mute/unmute all digital audio outputs
 * according to @a mute parameter.
 * @param[in] mute indicates if audio outputs should be muted (if @c true) or unmuted (if @c false)
 */
SvLocal
void
QBConaxURIManagerMuteDigitalAudioOutputs(QBConaxURIManager self, bool mute)
{
    log_fun();
    int audioOutput = QBPlatformFindAudioOutput(QBAudioOutputType_HDMI, NULL);
    if (audioOutput >= 0)
        QBPlatformMuteAudioOutput(audioOutput, mute, QBPlatformOutputPriority_content);
}

/**
 * This function should enable @a digitalOutput according to @a enable
 * @param[in] digitalOutput analogue output ID
 * @param[in] enable indicates if output should be enabled (if @c true) or disabled (if @c false)
 */
SvLocal
void
QBConaxURIManagerEnableDigitalOutput(QBConaxURIManager self, unsigned int digitalOutput, bool enable)
{
    log_fun();
    QBVideoOutputConfig cfg;
    QBPlatformGetOutputConfig(digitalOutput, &cfg);
    log_info("%s digital output %s", (enable) ? "Enable" : "Disable", cfg.name);

    if (cfg.enabled) {
        QBPlatformOutputMute(digitalOutput, !enable, QBPlatformOutputPriority_content);
    }
}

/**
 * This function should try to set digital protection (HDCP) on
 * @a digitalOutput according to @a enable value.
 * @param[in] digitalOutput analogue output ID
 * @param[in] enable indicates if HDCP should be on (if @c true) or could be off (if @c false)
 * @return @c true if we successfully set HDCP according to @a enable, @c false if we don't
 */
SvLocal
bool
QBConaxURIManagerTryToEnableDigitalProtection(QBConaxURIManager self, unsigned int digitalOutput, bool enable)
{
    log_fun();
    log_info("Try to %s digital protection", (enable) ? "enable" : "disable");

    QBVideoOutputConfig cfg;
    QBPlatformGetOutputConfig(digitalOutput, &cfg);

    if ((cfg.capabilities & QBOutputCapability_HDCP) != QBOutputCapability_HDCP) {
        log_error("%s (%d) does not have HDCP capability", cfg.name, digitalOutput);
        return false;
    }

    cfg.contentProtection.protection.hdmi_video.hdcp_level = enable ? QBContentProtection_required : QBContentProtection_disable;
    cfg.contentProtection.priority = QBContentProtectionPriority_content;
    if (QBPlatformUpdateContentProtection(digitalOutput, &cfg.contentProtection, QBPlatformOutputPriority_content) < 0) {
        log_error("Cannot %s HDCP authentication on output (%d)", (enable) ? "enable" : "disable", digitalOutput);
        return false;
    }

    return true;
}

SvLocal
bool
QBConaxURIManagerTryToEnableSPDIFProtection(QBConaxURIManager self, bool enable)
{
    log_fun();
    log_info("Try to %s protection on SPDIF output", (enable) ? "enable" : "disable");

    // TODO: Add support
    return false;
}

SvLocal
void
QBConaxURIManagerHandleDigitalOutput(QBConaxURIManager self, unsigned int digitalOutput, const QBICSmartcardURIDataUsageRulesInformation* uri, bool* disableAudio)
{
    log_fun();
    if ((uri->emiCopyControl == QBICSmartcardURIDataEMICopyControl_copyFreely) &&
        (uri->redistributionControl == QBICSmartcardURIDataRedistributionControl_off)) {
        QBConaxURIManagerTryToEnableDigitalProtection(self, digitalOutput, false);
    } else {
        if (QBConaxURIManagerTryToEnableDigitalProtection(self, digitalOutput, true) == false) {
            log_error("Cannot set HDCP on output %d. Disabling this display...", 0);
            QBConaxURIManagerEnableDigitalOutput(self, digitalOutput, false);
            *disableAudio = true;
            return;
        }
    }

    QBConaxURIManagerEnableDigitalOutput(self, digitalOutput, true);
    *disableAudio = false;
}

SvLocal
void
QBConaxURIManagerHandleSPDIFOutput(QBConaxURIManager self, const QBICSmartcardURIDataUsageRulesInformation* uri)
{
    log_fun();
    int audioOutput = QBPlatformFindAudioOutput(QBAudioOutputType_SPDIF, NULL);
    if (audioOutput < 0) {
        return;
    }

    if ((uri->emiCopyControl == QBICSmartcardURIDataEMICopyControl_copyFreely) &&
        (uri->redistributionControl == QBICSmartcardURIDataRedistributionControl_off)) {
        QBConaxURIManagerTryToEnableSPDIFProtection(self, false);
    } else {
        if (!QBConaxURIManagerTryToEnableSPDIFProtection(self, true)) {
            // FIXME: We should disable here SPDIF output, but since we don't have for now implemented SPDIF protection
            // and Conax is ok with unmuting it (CUB-3378) below lines are commented.
            // log_error("Cannot set protection on SPDIF output. Disabling this output...");
            // QBPlatformMuteAudioOutput(audioOutput, true, QBPlatformOutputPriority_content);
            // return;
        }
    }
    QBPlatformMuteAudioOutput(audioOutput, false, QBPlatformOutputPriority_content);
}

SvLocal
void
QBConaxURIManagerHandleDigitalOutputs(QBConaxURIManager self)
{
    log_fun();
    bool disableDigitalAudioOutputs = false;
    for (size_t i = 0; i < self->digitalOutputsCnt; i++) {
        bool tmp = false;
        QBConaxURIManagerHandleDigitalOutput(self, self->digitalOutputs[i], &self->currentUriData->usageRulesInformation, &tmp);
        disableDigitalAudioOutputs = tmp || disableDigitalAudioOutputs;
    }
    QBConaxURIManagerHandleSPDIFOutput(self, &self->currentUriData->usageRulesInformation);
    // If any handler disable audio output we should disable all digital outputs
    QBConaxURIManagerMuteDigitalAudioOutputs(self, disableDigitalAudioOutputs);
}

SvLocal
void
QBConaxURIManagerHandleRetentionLimit(QBConaxURIManager self, QBICSmartcardURIData uriData)
{
    log_fun();
    QBConaxPVRManager pvrManager = (QBConaxPVRManager) self->appGlobals->casPVRManager;

    if (!pvrManager) {
        log_error("PVR manager is NULL.");
        return;
    }

    QBConaxPVRManagerSetURIData(pvrManager, uriData);
}

SvLocal
void
QBConaxURIManagerHandleTrickPlay(QBConaxURIManager self, const QBICSmartcardURIDataUsageRulesInformation* uri)
{
    log_fun();
    log_info("URI Trick Play = %d", (int) uri->trickPlayControl);

    SvPlayerManager man = SvPlayerManagerGetInstance();
    SvPlayerTask playerTask = SvPlayerManagerGetPlayerTask(man, -1);
    if (playerTask) {
        SvPlayerTaskState state = SvPlayerTaskGetState(playerTask);
        SvPlayerTaskRestrictions restrictions = state.restrictions;

        // Clear up restrictions controlled by Conax URI.
        restrictions.jumpingDisallowed = false;
        restrictions.isFFSpeedLimited = false;
        restrictions.maxFFSpeed = 0;
        restrictions.pausingDisallowed = false;

        switch (uri->trickPlayControl) {
            case QBICSmartcardURIDataTrickPlayControl_noRestriction:
                break;
            case QBICSmartcardURIDataTrickPlayControl_noJumpingMaxFF2X: /**< No skip/jump over frames, max fast forward 2X */
                restrictions.jumpingDisallowed = true;
                restrictions.isFFSpeedLimited = true;
                restrictions.maxFFSpeed = 2;
                break;
            case QBICSmartcardURIDataTrickPlayControl_noJumpingMaxFF4X: /**< No skip/jump over frames, max fast forward 4X */
                restrictions.jumpingDisallowed = true;
                restrictions.isFFSpeedLimited = true;
                restrictions.maxFFSpeed = 4;
                break;
            case QBICSmartcardURIDataTrickPlayControl_noJumpingMaxFF8X: /**< No skip/jump over frames, max fast forward 8X */
                restrictions.jumpingDisallowed = true;
                restrictions.isFFSpeedLimited = true;
                restrictions.maxFFSpeed = 8;
                break;
            case QBICSmartcardURIDataTrickPlayControl_noJumpingNoFF: /**< No skip/jump over frames or fast forward */
                restrictions.jumpingDisallowed = true;
                restrictions.isFFSpeedLimited = true;
                restrictions.maxFFSpeed = 1;
                break;
            case QBICSmartcardURIDataTrickPlayControl_noPauseNoJumpingNoFF: /**< No skip/jump over frames, fast forward and pause */
                restrictions.jumpingDisallowed = true;
                restrictions.pausingDisallowed = true;
                restrictions.isFFSpeedLimited = true;
                restrictions.maxFFSpeed = 1;
                break;
            default:
                log_info("No restrictions");
                break;
        }

        SvPlayerTaskSetRestrictions(playerTask, &restrictions, NULL);
    }
}

SvLocal
void
QBConaxURIManagerInterfaceNotify(QBConaxURIManager self, QBICSmartcardURIData uriData)
{
    log_fun();
    if (!self->appGlobals->DRMManager) {
        log_error("DRM Manager is NULL");
        return;
    }

    SvTime retentionTimeLimit = QBICSmartcardURIDataTimeshiftRetentionLimitToTime(uriData->usageRulesInformation.timeshiftRetentionLimit);
    bool isAvailable = uriData->usageRulesInformation.trickPlayControl != QBICSmartcardURIDataTrickPlayControl_noPauseNoJumpingNoFF;
    QBDRMManagerData data = {
        .availabilityTime = retentionTimeLimit,
        .isAvailable      = isAvailable,
    };
    QBDRMManagerInterfaceNotify(self->appGlobals->DRMManager, &data);
}

SvLocal
void
QBConaxURIManagerReactOnURIData(QBConaxURIManager self)
{
    log_fun();
    QBConaxURIManagerHandleAnalogueOutputs(self);
    QBConaxURIManagerHandleDigitalOutputs(self);
    QBConaxURIManagerHandleRetentionLimit(self, self->currentUriData);

    if (self->conaxVersion == QBICSmartcardOption_conaxVersion__6_0_enhanced_pvr) {
        QBConaxURIManagerHandleTrickPlay(self, &self->currentUriData->usageRulesInformation);
    }

    QBConaxURIManagerInterfaceNotify(self, self->currentUriData);
}

SvLocal
void
QBConaxURIManagerSmartcardDecryptionStateCallback(void * target, const QBICSmartcardSessionDescription *sessionDesc, SvGenericObject obj)
{
    log_fun();

    QBICSmartcardDecryptionState decryptionState = (QBICSmartcardDecryptionState) obj;
    QBConaxURIManager self = (QBConaxURIManager) target;
    log_deep("decryption (%s), sessionId(%d), sessionType(%d)",
            (decryptionState->start) ? "started" : "stopped", sessionDesc->sessionId, sessionDesc->sessionType);

    // only 'sessionId' is valid here in session description, we cannot use session description to check if we are playback
    if (sessionDesc->sessionType != QBCASSessionType_playback) {
        log_deep("decryption state callback received not from playback session, ignore it, sessionId(%d), sessionType(%d)",
                 sessionDesc->sessionId, sessionDesc->sessionType);
        return;
    }

    if (decryptionState->start) {
        log_info("decryption started");
        SVTESTRELEASE(self->currentUriData);
        // New session for new service has started.
        // Set up default URI.
        self->currentUriData = QBICSmartcardURIDataCreateDefault();
        self->isFta = decryptionState->isFta;
        if (self->isFta) {
            log_info("Channel changed to FTA, set up no restrictions.");
            // If channel is FTA, no restriction should be applied.
            QBICSmartcardURIDataSetNoRestriction(self->currentUriData);
        } else {
            log_info("Channel changed to SCRAMBLED, set up default URI.");
        }
        QBConaxURIManagerReactOnURIData(self);
    }
}

SvLocal
void
QBConaxURIManagerSmartcardURIDataCallback(void * target, const QBICSmartcardSessionDescription *sessionDesc, SvGenericObject obj)
{
    log_fun();

    log_deep("Received URI for sessionId(%d), sessionType(%d)", sessionDesc->sessionId, sessionDesc->sessionType);

    if (sessionDesc->sessionType != QBCASSessionType_playback) {
        log_error("Received URI callback not from playback session, ignore it, sessionId(%d), sessionType(%d)",
                 sessionDesc->sessionId, sessionDesc->sessionType);
        return;
    }

    QBICSmartcardURIData uriData = (QBICSmartcardURIData) obj;
    QBConaxURIManager self = (QBConaxURIManager) target;

    if (self->isFta) {
        log_error("URI data on FTA channel! Shouldn't be there.");
        return;
    }

    if (!SvObjectEquals((SvObject) self->currentUriData, (SvObject) uriData)) {
        log_info("updating URI data");
        SVTESTRELEASE(self->currentUriData);
        self->currentUriData = SVRETAIN(uriData);
        QBConaxURIManagerReactOnURIData(self);
    } else {
        log_info("URI data same as previous");
    }
}

SvLocal
void
QBConaxURIManagerCollectOutputs(QBConaxURIManager self)
{
    log_fun();
    QBVideoOutputConfig cfg;
    unsigned int outputsCnt = QBPlatformGetOutputsCount();
    self->digitalOutputsCnt = 0;
    self->analogueOutputsCnt = 0;

    for (unsigned int i = 0; i < outputsCnt; i++) {
        if (QBPlatformGetOutputConfig(i, &cfg) == 0) {
            if (cfg.type == QBOutputType_HDMI || cfg.type == QBOutputType_DVI) {
                self->digitalOutputs[self->digitalOutputsCnt++] = i;
                log_info("Digital output found: %s", cfg.name);
            }
            if (cfg.type == QBOutputType_YPbPr || cfg.type == QBOutputType_CVBS ||
                cfg.type == QBOutputType_SVideo || cfg.type == QBOutputType_RGB ||
                cfg.type == QBOutputType_RF) {
                self->analogueOutputs[self->analogueOutputsCnt++] = i;
                log_info("Analogue output found: %s", cfg.name);
            }
        }
    }
}

SvLocal
void
QBConaxURIManagerDriverVersionCallback(void *self_, SvGenericObject driverVersion)
{
    log_fun();
    QBConaxURIManager self = (QBConaxURIManager) self_;
    QBICSmartcardDriverVersion *smartcardDriverVersion = (QBICSmartcardDriverVersion *) driverVersion;

    self->conaxVersion = smartcardDriverVersion->currentConaxVersion;
    log_info("conax version(%s)", QBICSmartcardConaxVersionToString(self->conaxVersion));
}

SvLocal
void
QBConaxURIManagerSmartcardStateCallback(void* target, QBCASSmartcardState state)
{
    log_fun();
    QBConaxURIManager self = (QBConaxURIManager) target;
    if (self->currentUriData) {
        QBConaxURIManagerReactOnURIData(self);
    }
}

static struct QBICSmartcardCallbacks_s conaxCallbacks = {
    .uri_data         = QBConaxURIManagerSmartcardURIDataCallback,
    .decryption_state = QBConaxURIManagerSmartcardDecryptionStateCallback,
    .driver_version   = QBConaxURIManagerDriverVersionCallback,
};

static const struct QBCASCallbacks_s casCallbacks = {
    .smartcard_state = &QBConaxURIManagerSmartcardStateCallback,
};

void
QBConaxURIManagerStart(QBConaxURIManager self)
{
    log_fun();
    if (self->isServiceStarted == true)
        return;

    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks) & casCallbacks, self, "QBConaxURIManager");
    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), addConaxCallbacks, &conaxCallbacks, (void *) self, "ConaxURIManager");

    self->isServiceStarted = true;

    // get current conax version, could return NULL if QBICSmartcard is uninitialized, in that case 'driver_version' callback will be called
    QBICSmartcardDriverVersion* driverVersion = SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getDriverVersion);
    if (driverVersion) {
        self->conaxVersion = driverVersion->currentConaxVersion;
        SVRELEASE(driverVersion);
    }
}

void
QBConaxURIManagerStop(QBConaxURIManager self)
{
    log_fun();
    if (self->isServiceStarted == false)
        return;

    SvInvokeInterface(QBCAS, QBCASGetInstance(), removeCallbacks, (QBCASCallbacks) & casCallbacks, self);
    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), removeConaxCallbacks, &conaxCallbacks, (void *) self);

    self->isServiceStarted = false;
}

QBConaxURIManager
QBConaxURIManagerCreate(AppGlobals appGlobals)
{
    log_fun();
    if (!SvObjectIsImplementationOf(QBCASGetInstance(), QBSmartcard2Interface_getInterface())) {
        log_error("Conax URI Manager should be only created for Conax CAS type");
        return NULL;
    }

    QBConaxURIManager self = (QBConaxURIManager) SvTypeAllocateInstance(QBConaxURIManager_getType(), NULL);
    QBConaxURIManagerCollectOutputs(self);
    self->isServiceStarted = false;
    self->appGlobals = appGlobals;
    self->listeners = SvWeakListCreate(NULL);

    return self;
}

SvHidden
SvType
QBConaxURIManager_getType(void)
{
    log_fun();
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBConaxURIManagerDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBConaxURIManager",
                            sizeof(struct QBConaxURIManager_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}
