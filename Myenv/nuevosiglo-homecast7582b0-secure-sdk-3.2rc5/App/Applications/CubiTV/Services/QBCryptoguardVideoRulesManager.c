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

#include "QBCryptoguardVideoRulesManager.h"

#include <QBCAS.h>

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvEnv.h>

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>

#include <QBCASCommonCryptoguardVideoRulesListener.h>

#include <QBPlatformHAL/QBPlatformOutput.h>

#include <main.h>

#include "OutputStandardService.h"

#if SV_LOG_LEVEL > 0

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 2, "QBCryptoguardVideoRulesManager", "")

#define log_fun(fmt, ...) do { if (env_log_level() >= 4) { SvLogNotice(COLBEG() "QBCryptoguardVideoRulesManager :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)
#define log_debug(fmt, ...) do { if (env_log_level() >= 3) { SvLogNotice(COLBEG() "QBCryptoguardVideoRulesManager :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)
#define log_state(fmt, ...) do { if (env_log_level() >= 2) { SvLogNotice(COLBEG() "QBCryptoguardVideoRulesManager :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } } while (0)
#define log_error(fmt, ...) do { if (env_log_level() >= 1) { SvLogError(COLBEG() "QBCryptoguardVideoRulesManager :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } } while (0)

#else

#define log_fun(fmt, ...)
#define log_debug(fmt, ...)
#define log_state(fmt, ...)
#define log_error(fmt, ...)

#endif

struct QBCryptoguardVideoRulesManager_s {
    struct SvObject_ super_;

    bool isServiceStarted;
    QBPlatformOutputPriority priority;
    AppGlobals appGlobals;
};


SvLocal bool QBCryptoguardVideoRulesManagerIsAnalogOutput(QBOutputType type)
{
    switch (type) {
        case QBOutputType_YPbPr:
        case QBOutputType_CVBS:
        case QBOutputType_SVideo:
        case QBOutputType_RGB:
        case QBOutputType_RF:
            return true;
        default:
            return false;
    }
}

SvLocal bool QBCryptoguardVideoRulesManagerIsDigitalOutput(QBOutputType type)
{
    switch (type) {
        case QBOutputType_HDMI:
        case QBOutputType_DVI:
            return true;
        default:
            return false;
    }
}

SvLocal
void QBCryptoguardVideoRulesManagerHandleCGMSA(QBCryptoguardVideoRulesManager self, QBCASCommonCryptoguardVideoRulesCGMSA videoRules)
{
    log_fun();

    // TODO ask Cryptoguard about this

    QBContentProtectionCGMSMode mode = QBContentProtectionCGMSMode_copyNever;
    QBContentProtection level = QBContentProtection_unknown;

    bool muteOnFail = false;

    switch (videoRules) {
        case QBCASCommonCryptoguardVideoRulesCGMSA_neutral:
            level = QBContentProtection_disable;
            muteOnFail = false;
            break;
        case QBCASCommonCryptoguardVideoRulesCGMSA_forcedDeactivation:
            level = QBContentProtection_disable;
            muteOnFail = false;
            break;
        case QBCASCommonCryptoguardVideoRulesCGMSA_supportedUnitsOn:
            level = QBContentProtection_required;
            muteOnFail = false;
            break;
        case QBCASCommonCryptoguardVideoRulesCGMSA_mandatoryOnAllUnits:
            level = QBContentProtection_required;
            muteOnFail = true;
            break;
    }

    int outputCnt = QBPlatformGetOutputsCount();

    QBVideoOutputConfig cfg;
    for (int outputID = 0; outputID < outputCnt; outputID++) {
        if (QBPlatformGetOutputConfig(outputID, &cfg) < 0) {
            log_error("Cannot get output cfg for output: %d", outputID);
            continue;
        }

        if (!QBCryptoguardVideoRulesManagerIsAnalogOutput(cfg.type))
            continue;

        if (!(cfg.capabilities & QBOutputCapability_CGMS) || QBPlatformSetCGMS(outputID, level, mode, self->priority) < 0) {
            log_error("QBPlatformSetCGMS() failed");
            QBPlatformOutputMute(outputID, muteOnFail, self->priority);
        } else {
            QBPlatformOutputMute(outputID, false, self->priority);
        }
    }
}

SvLocal
void QBCryptoguardVideoRulesManagerHandleHDCP(QBCryptoguardVideoRulesManager self, QBCASCommonCryptoguardVideoRulesHDCP videoRules)
{
    log_fun();

    // TODO ask Cryptoguard about this

    QBContentProtection level = QBContentProtection_unknown;

    bool muteOnFail = false;

    switch (videoRules) {
        case QBCASCommonCryptoguardVideoRulesHDCP_neutral:
            level = QBContentProtection_disable;
            muteOnFail = false;
            break;
        case QBCASCommonCryptoguardVideoRulesHDCP_forcedDeactivation:
            level = QBContentProtection_disable;
            muteOnFail = false;
            break;
        case QBCASCommonCryptoguardVideoRulesHDCP_supportedUnitsOn:
            level = QBContentProtection_required;
            muteOnFail = false;
            break;
        case QBCASCommonCryptoguardVideoRulesHDCP_mandatoryOnAllUnits:
            level = QBContentProtection_required;
            muteOnFail = true;
            break;
    }

    int outputCnt = QBPlatformGetOutputsCount();

    QBVideoOutputConfig cfg;
    for (int outputID = 0; outputID < outputCnt; outputID++) {
        if (QBPlatformGetOutputConfig(outputID, &cfg) < 0) {
            log_error("Cannot get output cfg for output: %d", outputID);
            continue;
        }

        if (!QBCryptoguardVideoRulesManagerIsDigitalOutput(cfg.type)) {
            continue;
        }

        if (!(cfg.capabilities & QBOutputCapability_HDCP) || QBPlatformSetHDCP(outputID, level, self->priority) < 0) {
            log_error("QBPlatformSetHDCP() failed");
            QBPlatformOutputMute(outputID, muteOnFail, self->priority);
        } else {
            QBPlatformOutputMute(outputID, false, self->priority);
        }
    }
}

SvLocal
void QBCryptoguardVideoRulesManagerHandleMacrovision(QBCryptoguardVideoRulesManager self, QBCASCommonCryptoguardVideoRulesMacrovision videoRules)
{
    log_fun();

    // TODO ask Cryptoguard about this

    QBContentProtectionMacrovisionSettings macrovisionSettings = {
        .mode = QBContentProtectionMacrovisionMode_agcOnly
    };
    bool muteOnFail = false;

    switch (videoRules) {
        case QBCASCommonCryptoguardVideoRulesMacrovision_neutral:
            macrovisionSettings.mode = QBContentProtectionMacrovisionMode_unknown;
            break;
        case QBCASCommonCryptoguardVideoRulesMacrovision_forcedDeactivation:
            macrovisionSettings.mode = QBContentProtectionMacrovisionMode_disabled;
            break;
        case QBCASCommonCryptoguardVideoRulesMacrovision_supportedUnitsOn:
            break;
        case QBCASCommonCryptoguardVideoRulesMacrovision_mandatoryOnAllUnits:
            muteOnFail = true;
            break;
    }

    int outputCnt = QBPlatformGetOutputsCount();

    QBVideoOutputConfig cfg;
    for (int outputID = 0; outputID < outputCnt; outputID++) {
        if (QBPlatformGetOutputConfig(outputID, &cfg) < 0) {
            log_error("Cannot get output cfg for output: %d", outputID);
            continue;
        }

        if (!QBCryptoguardVideoRulesManagerIsAnalogOutput(cfg.type)) {
            continue;
        }

        if (!(cfg.capabilities & QBOutputCapability_Macrovision) || QBPlatformSetMacrovision(outputID, &macrovisionSettings, self->priority) < 0) {
            log_error("Failed to set macrovision");
            QBPlatformOutputMute(outputID, muteOnFail, self->priority);
        } else {
            QBPlatformOutputMute(outputID, false, self->priority);
        }
    }
}

SvLocal
void QBCryptoguardVideoRulesManagerHandleAnalog(QBCryptoguardVideoRulesManager self, QBCASCommonCryptoguardVideoRulesAnalogHD videoRules)
{
    log_fun();

    // TODO ask Cryptoguard about this

    QBOutputStandard standard = QBOutputStandard_current;

    switch (videoRules) {
        case QBCASCommonCryptoguardVideoRulesAnalogHD_neutral:
            standard = QBOutputStandard_current;
            break;
        case QBCASCommonCryptoguardVideoRulesAnalogHD_maxResolution576i:
            standard = QBOutputStandard_576i50;
            break;
    }

    int outputCnt = QBPlatformGetOutputsCount();

    QBVideoOutputConfig cfg;
    for (int outputID = 0; outputID < outputCnt; outputID++) {
        if (QBPlatformGetOutputConfig(outputID, &cfg) < 0) {
            log_error("Cannot get output cfg for output: %d", outputID);
            continue;
        }

        if (QBCryptoguardVideoRulesManagerIsAnalogOutput(cfg.type)) {
            QBOutputStandardServiceChangeMode(self->appGlobals->outputStandardService, standard, outputID);
        }
    }
}

SvLocal
void QBCryptoguardVideoRulesManagerHandleDigital(QBCryptoguardVideoRulesManager self, QBCASCommonCryptoguardVideoRulesDigitalHD videoRules)
{
    log_fun();

    // TODO ask Cryptoguard about this

    QBOutputStandard standard = QBOutputStandard_current;

    switch (videoRules) {
        case QBCASCommonCryptoguardVideoRulesDigitalHD_neutral:
            standard = QBOutputStandard_current;
            break;
        case QBCASCommonCryptoguardVideoRulesDigitalHD_maxResolution1080p:
            standard = QBOutputStandard_1080i50;
            break;
        case QBCASCommonCryptoguardVideoRulesDigitalHD_maxResolution1080i:
            standard = QBOutputStandard_1080i50;
            break;
        case QBCASCommonCryptoguardVideoRulesDigitalHD_maxResolution720p:
            standard = QBOutputStandard_720p50;
            break;
        case QBCASCommonCryptoguardVideoRulesDigitalHD_maxResolution720i:
            standard = QBOutputStandard_720p50;
            break;
        case QBCASCommonCryptoguardVideoRulesDigitalHD_maxResolution576i:
            standard = QBOutputStandard_576i50;
            break;
    }

    int outputCnt = QBPlatformGetOutputsCount();

    QBVideoOutputConfig cfg;
    for (int outputID = 0; outputID < outputCnt; outputID++) {
        if (QBPlatformGetOutputConfig(outputID, &cfg) < 0) {
            log_error("Cannot get output cfg for output: %d", outputID);
            continue;
        }

        if (QBCryptoguardVideoRulesManagerIsDigitalOutput(cfg.type)) {
            QBOutputStandardServiceChangeMode(self->appGlobals->outputStandardService, standard, outputID);
        }
    }
}

SvLocal
void QBCryptoguardVideoRulesManageVideoRulesChanged(SvGenericObject self_, int8_t sessionId, QBCASCommonCryptoguardVideoRules videoRules)
{
    log_fun();

    QBCryptoguardVideoRulesManager self = (QBCryptoguardVideoRulesManager) self_;

    QBCryptoguardVideoRulesManagerHandleCGMSA(self, videoRules->cgmsa);
    QBCryptoguardVideoRulesManagerHandleHDCP(self, videoRules->hdcp);
    QBCryptoguardVideoRulesManagerHandleMacrovision(self, videoRules->macrovision);
    QBCryptoguardVideoRulesManagerHandleAnalog(self, videoRules->analogHD);
    QBCryptoguardVideoRulesManagerHandleDigital(self, videoRules->digitalHD);
}

SvLocal
void QBCryptoguardVideoRulesManagerDestroy(void* self_)
{
    log_fun();

    QBCryptoguardVideoRulesManager self = (QBCryptoguardVideoRulesManager) self_;
    QBCryptoguardVideoRulesManagerStop(self);
}

SvExport
void QBCryptoguardVideoRulesManagerStart(QBCryptoguardVideoRulesManager self)
{
    log_fun();

    QBCASCommonCryptoguardVideoRulesListenerAdd((QBCASCommonCryptoguard) QBCASGetInstance(), (SvObject) self);
}

SvExport
void QBCryptoguardVideoRulesManagerStop(QBCryptoguardVideoRulesManager self)
{
    log_fun();

    QBCASCommonCryptoguardVideoRulesListenerRemove((QBCASCommonCryptoguard) QBCASGetInstance(), (SvObject) self);
}

SvExport
QBCryptoguardVideoRulesManager QBCryptoguardVideoRulesManagerCreate(AppGlobals appGlobals)
{
    log_fun();

    QBCryptoguardVideoRulesManager self = (QBCryptoguardVideoRulesManager) SvTypeAllocateInstance(QBCryptoguardVideoRulesManager_getType(), NULL);
    self->isServiceStarted = false;
    self->priority = QBPlatformOutputPriority_content;
    self->appGlobals = appGlobals;

    return self;
}

SvExport
SvType QBCryptoguardVideoRulesManager_getType(void)
{
    log_fun();

    static const struct QBCASCommonCryptoguardVideoRulesListener_s cryptoguardVideoRulesMethods = {
        .videoRulesChanged = QBCryptoguardVideoRulesManageVideoRulesChanged
    };

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBCryptoguardVideoRulesManagerDestroy
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBCryptoguardVideoRulesManager",
                            sizeof(struct QBCryptoguardVideoRulesManager_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBCASCommonCryptoguardVideoRulesListener_getInterface(), &cryptoguardVideoRulesMethods,
                            NULL);
    }

    return type;
}
