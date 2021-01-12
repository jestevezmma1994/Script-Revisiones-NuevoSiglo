/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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

#include <main.h>

#include "QBViewRightDVBPVRManager.h"

#include <Logic/PVRLogic.h>
#include <Services/QBAccessController/QBAccessManager.h>
#include <Services/QBAccessController/ViewRightAccessPlugin.h>
#include <Services/QBAccessController/ViewRightAccessPluginListener.h>
#include <settings.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Utils/authenticators.h>
#include <safeFiles.h>

#include <QBStaticStorage.h>
#include <SvJSON/SvJSONSerialize.h>
#include <SvJSON/SvJSONParse.h>

#include <Services/QBCASManager.h>
#include <QBViewport.h>
#include <Services/diskPVRProvider/QBDiskPVRProvider.h>
#include <Services/diskPVRProvider/QBDiskPVRRecording.h>
#include <QBViewRight.h>
#include <QBViewRightInfo.h>
#include <QBCAS.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvHashTable.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvQuirks/SvRuntimePrefix.h>

#include <libintl.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBViewRightDVBPVRManager"
    SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 3, moduleName "LogLevel", "0:error, 1:warning, 2:state, 3:info, 4:deep, 5:debug, 6:function");
    #define log_fun(fmt, ...)   do { if (env_log_level() >= 6) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_debug(fmt, ...) do { if (env_log_level() >= 5) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_deep(fmt, ...)  do { if (env_log_level() >= 4) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(cyan), __func__, ## __VA_ARGS__); } while (0)
    #define log_info(fmt, ...)  do { if (env_log_level() >= 3) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(cyan), __func__, ## __VA_ARGS__); } while (0)
    #define log_state(fmt, ...) do { if (env_log_level() >= 2) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } while (0)
    #define log_warn(fmt, ...)  do { if (env_log_level() >= 1) SvLogWarning(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...) do { if (env_log_level() >= 0) SvLogError(COLBEG()   moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#else
    #define log_fun(fmt, ...)
    #define log_debug(fmt, ...)
    #define log_deep(fmt, ...)
    #define log_info(fmt, ...)
    #define log_state(fmt, ...)
    #define log_warn(fmt, ...)
    #define log_error(fmt, ...)
#endif

#define QBVIEWRIGHTDVB_PVR_MANAGER_FILE "/etc/vod/app/viewRightCachedPin"
#define QBVIEWRIGHTDVB_IS_PARENTAL_CONTROLLED "vmx:is_parental_controlled"
#define QBVIEWRIGHTDVB_PVR_MANAGER_CONFIG_FILE "%s/etc/viewRightManager"

typedef enum QBViewRightDVBPVRManagerPinState_e {
    QBViewRightDVBPVRManagerPinState_notVerified,
    QBViewRightDVBPVRManagerPinState_verified,
    QBViewRightDVBPVRManagerPinState_notUsed
} QBViewRightDVBPVRManagerPinState;

struct QBViewRightDVBPVRManager_ {
    struct SvObject_ super_;
    QBCASSmartcardState cardState;
    QBAccessManager accessManger;
    SvWidget cardConfigurationDialog;
    bool blockedAV;

    AppGlobals appGlobals;

    bool isStarted;

    SvFiber fiber;
    bool checkingPin;

    QBCASCmd getInfoCmd;

    struct {
        SvString pin;
        QBViewRightDVBPVRManagerPinState state;
    } nonSmartcard;

    struct {
        SvString pin;
        QBViewRightDVBPVRManagerPinState state;
    } smartcard;

    bool forceCachedPinPopup;           // raise PIN popup for cached PIN on initialization if we haven't got cached PIN yet
};

SvLocal void QBViewRightDVBPVRManagerSaveCachedPin(QBViewRightDVBPVRManager self);

SvLocal void QBViewRightDVBPVRManagerStop(SvObject self_);

SvLocal void QBViewRightDVBPVRManagerDestroy(void *self_)
{
    log_fun();

    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) self_;

    if (self->isStarted) {
        QBViewRightDVBPVRManagerStop((SvObject) self);
    }

    SVRELEASE(self->accessManger);
    SVTESTRELEASE(self->nonSmartcard.pin);
    SVTESTRELEASE(self->smartcard.pin);
    SvFiberDestroy(self->fiber);
}

SvLocal void QBViewRightDVBPVRManagerDialogCallback(void *ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    log_fun();

    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) ptr;

    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        SvFiberActivate(self->fiber);
    }

    self->checkingPin = false;
    self->cardConfigurationDialog = NULL;
}

SvLocal void QBViewRightDVBPVRManagerShowCardConfigurationDialog(QBViewRightDVBPVRManager self, QBViewRightPinSelect pinSelect)
{
    log_fun();

    SvString domain = NULL;
    char *message = NULL;
    switch (pinSelect) {
        case QBViewRightPinSelect_NonSmartcard:
            domain = SVSTRING("QBViewRightPinType_NonScInternalPin");
            message = gettext("Non smartcard configuration changed - please enter PIN");
            break;
        case QBViewRightPinSelect_Smartcard:
            domain = SVSTRING("QBViewRightPinType_ScInternalPin");
            message = gettext("Smartcard configuration changed - please enter PIN");
            break;
        default:
            log_error("Invalid pin select %d", pinSelect);
            return;
    }

    SvObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, domain);
    svSettingsPushComponent("ViewRightDVBPVRManager.settings");
    self->cardConfigurationDialog = QBAuthDialogCreate(self->appGlobals, authenticator, NULL, message, false, SVSTRING("ViewRightManagerPINPopup"), NULL);

    QBDialogRun(self->cardConfigurationDialog, self, QBViewRightDVBPVRManagerDialogCallback);
    svSettingsPopComponent();

    self->checkingPin = true;

    log_info("Shown: 'Card configuration' dialog.");
}

SvLocal void QBViewRightDVBPVRManagerRecordingAdded(SvObject self_, const char* caller)
{
    log_fun();

    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) self_;

    if (!self->isStarted) {
        log_error("Service is not started");
        return;
    }

    SvFiberActivate(self->fiber);
}

SvLocal void QBViewRightDVBPVRManagerPinStatus(void *self_, QBViewRightPinStatus pinStatus)
{
    log_fun();

    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) self_;

    if (self->nonSmartcard.pin && self->nonSmartcard.state == QBViewRightDVBPVRManagerPinState_notVerified) {
        if (pinStatus == QBViewRightPinStatus_OK) {
            log_info("Non smartcard pin has been verified. It can be used.");
            self->nonSmartcard.state = QBViewRightDVBPVRManagerPinState_verified;
        } else {
            log_info("Non smartcard pin hasn't been confirmed. It cannot not be used");
            SVRELEASE(self->nonSmartcard.pin);
            self->nonSmartcard.pin = NULL;
        }
        goto fini;
    }

    if (self->smartcard.pin && self->smartcard.state == QBViewRightDVBPVRManagerPinState_notVerified) {
        if (pinStatus == QBViewRightPinStatus_OK) {
            log_info("Smartcard pin has been verified. It can be used.");

            self->smartcard.state = QBViewRightDVBPVRManagerPinState_verified;
        } else {
            log_info("Smartcard pin hasn't been confirmed. It cannot be used");
            SVRELEASE(self->smartcard.pin);
            self->smartcard.pin = NULL;
        }
    }

fini:
    SvFiberActivate(self->fiber);
    self->checkingPin = false;
}

SvLocal void QBViewRightDVBPVRManagerVerifyPin(QBViewRightDVBPVRManager self, QBViewRightPinSelect pinSelect, SvString pin)
{
    log_fun();

    QBViewRight viewRight = (QBViewRight) QBCASGetInstance();

    QBViewRightCheckPin(viewRight, (SvObject) self, pinSelect, pin);
    self->checkingPin = true;
}

SvLocal void QBViewRightDVBPVRManagerOnEnteredPinStatus(SvObject self_, SvString domain, SvString pin, QBAuthStatus status)
{
    log_fun();
    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) self_;

    if (SvStringEqualToCString(domain, "QBViewRightPinType_NonScInternalPin") || SvStringEqualToCString(domain, "QBViewRightPinType_NonScPin")) {
        switch (status) {
            case QBAuthStatus_OK:
                log_info("Got correct Non smartcard PIN = %s. Saving it.", SvStringGetCString(pin));
                SVTESTRELEASE(self->nonSmartcard.pin);
                self->nonSmartcard.pin = SVRETAIN(pin);
                self->nonSmartcard.state = QBViewRightDVBPVRManagerPinState_verified;
                break;
            case QBAuthStatus_invalid:
            case QBAuthStatus_rejected:
                if (self->nonSmartcard.pin && SvObjectEquals((SvObject) self->nonSmartcard.pin, (SvObject) pin)) {
                    log_info("Cached non smartcard PIN is no longer valid");
                    SVTESTRELEASE(self->nonSmartcard.pin);
                    self->nonSmartcard.pin = NULL;
                    self->nonSmartcard.state = QBViewRightDVBPVRManagerPinState_notVerified;
                }
                break;
            default:
                break;
        }
    }

/*
    If cached pin is enterted on smartcard it unblocks all sessions. It seems to be VMX bug.
    TODO: Check this with VMX and fix.
*/
#if 0
    if (SvStringEqualToCString(domain, "QBViewRightPinType_ScInternalPin") || SvStringEqualToCString(domain, "QBViewRightPinType_IPPV_PC_1")) {
        switch (status) {
            case QBAuthStatus_OK:
                log_info("Got correct Smartcard PIN = %s. Saving it.", SvStringGetCString(pin));
                SVTESTRELEASE(self->smartcard.pin);
                self->smartcard.pin = SVRETAIN(pin);
                self->smartcard.state = QBViewRightDVBPVRManagerPinState_verified;
                break;
            case QBAuthStatus_invalid:
            case QBAuthStatus_rejected:
                if (self->smartcard.pin && SvObjectEquals((SvObject) self->smartcard.pin, (SvObject) pin)) {
                    log_info("Cached Smartcard PIN is no longer valid");
                    SVTESTRELEASE(self->smartcard.pin);
                    self->smartcard.pin = NULL;
                    self->smartcard.state = QBViewRightDVBPVRManagerPinState_notVerified;
                }
                break;
            default:
                break;
        }
    }
#endif

    QBViewRightDVBPVRManagerSaveCachedPin(self);
}

SvLocal void QBViewRightDVBPVRManagerStep(void *self_)
{
    log_fun();
    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) self_;

    SvFiberDeactivate(self->fiber);

    if (self->checkingPin) {
        return;
    }

    if (self->nonSmartcard.state == QBViewRightDVBPVRManagerPinState_notVerified) {
        if (self->nonSmartcard.pin) {
            log_info("Non smartcard pin is present but not verified. Verify it.");
            QBViewRightDVBPVRManagerVerifyPin(self, QBViewRightPinSelect_NonSmartcard, self->nonSmartcard.pin);
        } else {
            if (self->forceCachedPinPopup) {
                log_info("Non smartcard pin is not present. Ask user for it");
                QBViewRightDVBPVRManagerShowCardConfigurationDialog(self, QBViewRightPinSelect_NonSmartcard);
            } else {
                log_info("Non smartcard pin is not present yet. Not going to ask user for it though");
            }
        }
        return;
    }

    if (self->smartcard.state == QBViewRightDVBPVRManagerPinState_notVerified) {
        if (self->smartcard.pin) {
            log_info("Smartcard pin is present but not verified. Verify it.");
            QBViewRightDVBPVRManagerVerifyPin(self, QBViewRightPinSelect_Smartcard, self->smartcard.pin);
        } else {
            if (self->forceCachedPinPopup) {
                log_info("Smartcard pin is not present. Ask user for it");
                QBViewRightDVBPVRManagerShowCardConfigurationDialog(self, QBViewRightPinSelect_Smartcard);
            } else {
                log_info("Smartcard pin is not present yet. Not going to ask user for it though");
            }
        }
    }
}

SvLocal void QBViewRightDVBPVRManageCallbackInfo(void* self_, const QBCASCmd cmd, int status, QBCASInfo info)
{
    log_fun();

    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) self_;
    self->getInfoCmd = NULL;

    QBViewRightInfo vmxInfo = (QBViewRightInfo) info;

    if (QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->type == QBPVRProviderType_disk) {
        QBPVRLogicSetMaxRecordingSessionsLimit(self->appGlobals->pvrLogic, vmxInfo->maxRecordSessions);
    }
}

SvLocal void QBViewRightDVBPVRManagerSmartcardState(void* target, QBCASSmartcardState state)
{
    log_fun();

    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) target;

    self->cardState = state;

    if (self->cardState == QBCASSmartcardState_correct) {
        if (self->getInfoCmd) {
            SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->getInfoCmd);
            self->getInfoCmd = NULL;
        }
        SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, &QBViewRightDVBPVRManageCallbackInfo, self, &self->getInfoCmd);
    }

    QBViewRight viewRight = (QBViewRight) QBCASGetInstance();

    if (QBViewRightIsNonSmartcardEnabled(viewRight)) {
        self->nonSmartcard.state = QBViewRightDVBPVRManagerPinState_notVerified;
    } else {
        self->nonSmartcard.state = QBViewRightDVBPVRManagerPinState_notUsed;
    }

/*
    If cached pin is enterted on smartcard it unblocks all sessions. It seems to be VMX bug.
    TODO: Check this with VMX and fix.
*/
#if 0
    if (QBViewRightIsSmartcardEnabled(viewRight)) {
        self->smartcard.state = QBViewRightDVBPVRManagerPinState_notVerified;
    } else {
        self->smartcard.state = QBViewRightDVBPVRManagerPinState_notUsed;
    }
#else
    self->smartcard.state = QBViewRightDVBPVRManagerPinState_notUsed;
#endif

    SvFiberActivate(self->fiber);
}

SvLocal void QBViewRightDVBPVRManagerLoadCachedPin(QBViewRightDVBPVRManager self)
{
    log_fun();

    char *buffer;
    QBFileToBuffer(QBVIEWRIGHTDVB_PVR_MANAGER_FILE, &buffer);
    if (!buffer) {
        return;
    }
    SvHashTable cachedPins = (SvHashTable) SvJSONParseString(buffer, true, NULL);
    free(buffer);
    if (!cachedPins) {
        log_error("cannot parse %s, JSON is corrupted", QBVIEWRIGHTDVB_PVR_MANAGER_FILE);
        return;
    }

    if (!SvObjectIsInstanceOf((SvObject) cachedPins, SvHashTable_getType())) {
        log_error("cannot parse %s, JSON does not contain a hash table", QBVIEWRIGHTDVB_PVR_MANAGER_FILE);
        SVRELEASE(cachedPins);
        return;
    }

    SvObject nscPin = SvHashTableFind(cachedPins, (SvObject) SVSTRING("nonSmartcardPin"));

    if (nscPin && SvObjectIsInstanceOf(nscPin, SvValue_getType()) && SvValueIsString((SvValue) nscPin)) {
        self->nonSmartcard.pin = SVRETAIN(SvValueGetString((SvValue) nscPin));
        log_info("Loaded non smartcard pin = %s", SvStringGetCString(self->nonSmartcard.pin));
    }

    SvObject scPin = SvHashTableFind(cachedPins, (SvObject) SVSTRING("smartcardPin"));

    if (scPin && SvObjectIsInstanceOf(scPin, SvValue_getType()) && SvValueIsString((SvValue) scPin)) {
        self->smartcard.pin = SVRETAIN(SvValueGetString((SvValue) scPin));
        log_info("Loaded smartcard pin = %s", SvStringGetCString(self->smartcard.pin));
    }

    SVRELEASE(cachedPins);
}

SvLocal void QBViewRightDVBPVRManagerSaveCachedPin(QBViewRightDVBPVRManager self)
{
    log_fun();

    SvHashTable cachedPins = SvHashTableCreate(2, NULL);

    if (self->nonSmartcard.pin) {
        SvHashTableInsert(cachedPins, (SvObject) SVSTRING("nonSmartcardPin"), (SvObject) self->nonSmartcard.pin);
    }

    if (self->smartcard.pin) {
        SvHashTableInsert(cachedPins, (SvObject) SVSTRING("smartcardPin"), (SvObject) self->smartcard.pin);
    }

    SvString cachedPinsJson = SvJSONDataToString((SvObject) cachedPins, true, NULL);

    if (!cachedPinsJson) {
        log_error("Couldn't serialize cachedPins");
        goto fini;
    }

    if (!QBStaticStorageWriteFile(SVSTRING(QBVIEWRIGHTDVB_PVR_MANAGER_FILE), cachedPinsJson)) {
        log_error("Couldn't couldn't save %s file", QBVIEWRIGHTDVB_PVR_MANAGER_FILE);
        goto fini;
    }

fini:
    SVTESTRELEASE(cachedPinsJson);
    SVRELEASE(cachedPins);
}

static const struct QBCASCallbacks_s callbacks = {
    .smartcard_state = QBViewRightDVBPVRManagerSmartcardState,
};

SvLocal void QBViewRightDVBPVRManagerStart(SvObject self_)
{
    log_fun();

    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) self_;

    if (self->isStarted) {
        return;
    }

    // The same plugin is used for smartcard and nonsmartcard.
    ViewRightAccessPlugin plugin = (ViewRightAccessPlugin) QBAccessManagerGetDomainPlugin(self->accessManger, SVSTRING("QBViewRightPinType_ScInternalPin"));

    if (!plugin) {
        log_error("Couldn't get ViewRightAccessPlugin");
        return;
    }

    self->isStarted = true;

    QBViewRightDVBPVRManagerSmartcardState((void *) self, SvInvokeInterface(QBCAS, QBCASGetInstance(), getState));
    QBViewRightDVBPVRManagerLoadCachedPin(self);
    ViewRightAccessPluginAddListener(plugin, (SvObject) self);

    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, &callbacks, self, "QBViewRightDVBPVRManager");
}

SvLocal void QBViewRightDVBPVRManagerStop(SvObject self_)
{
    log_fun();

    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) self_;

    if (!self->isStarted) {
        log_error("Service is not started");
        return;
    }

    self->isStarted = false;

    ViewRightAccessPlugin plugin = (ViewRightAccessPlugin) QBAccessManagerGetDomainPlugin(self->accessManger, SVSTRING("QBViewRightPinType_ScInternalPin"));

    if (plugin) {
        ViewRightAccessPluginRemoveListener(plugin, (SvObject) self);
    }

    if (self->cardConfigurationDialog) {
        QBDialogBreak(self->cardConfigurationDialog);
    }

    SvInvokeInterface(QBCAS, QBCASGetInstance(), removeCallbacks, &callbacks, self);
    if (self->getInfoCmd) {
        SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->getInfoCmd);
        self->getInfoCmd = NULL;
    }
    SvFiberDeactivate(self->fiber);
}

SvLocal void QBViewRightDVBPVRManagerRecordingBlockedPopup(QBViewRightDVBPVRManager self)
{
    log_fun();

    svSettingsPushComponent("Dialog.settings");
    QBDialogParameters params = {
        .app        = self->appGlobals->res,
        .controller = self->appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0,
    };

    SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    QBDialogAddButton(dialog, SVSTRING("OK-button"), "OK", 1);
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    QBDialogAddLabel(dialog, SVSTRING("content"), gettext("Ongoing recording is unavilable due to parental control restrictions"), SVSTRING("text"), 1);
    svSettingsPopComponent();
    QBDialogRun(dialog, NULL, NULL);
}

SvLocal void QBViewRightDVBPVRManagerBlockAV(QBViewRightDVBPVRManager self, bool blockAV)
{
    log_fun();

    if (self->blockedAV == blockAV) {
        return;
    }

    self->blockedAV = blockAV;
    QBViewportBlockVideo(QBViewportGet(), SVSTRING("QBViewRightDVBPVRManager"), QBViewportBlockVideoReason_maturityRating, blockAV);
    QBViewportBlockAudio(QBViewportGet(), blockAV);
}

SvLocal void QBViewRightDVBPVRManagerPlaybackStarted(SvObject self_, QBPVRRecording recording)
{
    log_fun();

    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) self_;

    // VMX can not handle PC pin for ongoing recordings. User can watch recording after it's finished.
    if (recording->state != QBPVRRecordingState_active) {
        return;
    }

    SvValue isParentalControlledV = (SvValue) QBDiskPVRRecordingGetRecordingProperty((QBDiskPVRRecording) recording, SVSTRING(QBVIEWRIGHTDVB_IS_PARENTAL_CONTROLLED));

    if (isParentalControlledV && SvObjectIsInstanceOf((SvObject) isParentalControlledV, SvValue_getType()) && SvValueIsBoolean(isParentalControlledV)) {
        bool isParentalControlled = SvValueGetBoolean(isParentalControlledV);
        if (isParentalControlled) {
            QBViewRightDVBPVRManagerBlockAV(self, true);
            QBViewRightDVBPVRManagerRecordingBlockedPopup(self);
            return;
        }
    }
}

SvLocal void QBViewRightDVBPVRManagerPlaybackStopped(SvObject self_, QBPVRRecording recording)
{
    log_fun();

    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) self_;
    QBViewRightDVBPVRManagerBlockAV(self, false);
}

SvLocal void QBViewRightDVBPVRManagerLoadConfig(QBViewRightDVBPVRManager self, SvString configPath, SvErrorInfo *errorOut)
{
    SvHashTable config = NULL;
    SvErrorInfo error = NULL;
    SvValue val = NULL;
    char *data = NULL;

    log_fun();

    // this field is true by default
    self->forceCachedPinPopup = true;

    QBFileToBuffer(SvStringCString(configPath), &data);
    if (unlikely(!data)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "Couldn't load config file: %s",
                                  SvStringCString(configPath));
        goto fini;
    }

    config = (SvHashTable) SvJSONParseString(data, false, NULL);
    free(data);

    if (!(config && SvObjectIsInstanceOf((SvObject) config, SvHashTable_getType()))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "ViewRight config file has got invalid format");
        goto fini;
    }

    val = (SvValue) SvHashTableFind(config, (SvObject) SVSTRING("forceCachedPINpopup"));
    if (!val || !SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) || !SvValueIsBoolean(val)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Couldn't read forceCachedPINpopup parameter");
        goto fini;
    }
    self->forceCachedPinPopup = SvValueGetBoolean(val);

fini:
    SVTESTRELEASE(config);
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal SvType QBViewRightDVBPVRManager_getType(void)
{
    log_fun();

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBViewRightDVBPVRManagerDestroy,
    };

    static SvType type = NULL;

    static const struct QBCASPVRManager_ pvrMethods = {
        .recordingAdded = &QBViewRightDVBPVRManagerRecordingAdded,
        .start          = &QBViewRightDVBPVRManagerStart,
        .stop           = &QBViewRightDVBPVRManagerStop,
    };

    static const struct ViewRightAccessPluginListener_ accessPluginMethods = {
        .onEnteredPinStatus = &QBViewRightDVBPVRManagerOnEnteredPinStatus,
    };

    static const struct QBCASPVRPlaybackMonitor_ playbackMonitor = {
        .playbackStarted = QBViewRightDVBPVRManagerPlaybackStarted,
        .playbackStopped = QBViewRightDVBPVRManagerPlaybackStopped,
    };

    static const struct QBViewRightPinStatusListener_i pinStatusMethods = {
        .pinStatus = &QBViewRightDVBPVRManagerPinStatus,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBViewRightDVBPVRManager",
                            sizeof(struct QBViewRightDVBPVRManager_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBCASPVRManager_getInterface(), &pvrMethods,
                            QBViewRightPinStatusListener_getInterface(), &pinStatusMethods,
                            QBCASPVRPlaybackMonitor_getInterface(), &playbackMonitor,
                            ViewRightAccessPluginListener_getInterface(), &accessPluginMethods,
                            NULL);
    }

    return type;
}

QBViewRightDVBPVRManager QBViewRightDVBPVRManagerCreate(AppGlobals appGlobals)
{
    log_fun();

    QBViewRightDVBPVRManager self = (QBViewRightDVBPVRManager) SvTypeAllocateInstance(QBViewRightDVBPVRManager_getType(), NULL);

    SvString configFile = SvStringCreateWithFormat(QBVIEWRIGHTDVB_PVR_MANAGER_CONFIG_FILE, SvGetRuntimePrefix());
    QBViewRightDVBPVRManagerLoadConfig(self, configFile, NULL);
    SVRELEASE(configFile);

    self->accessManger = SVRETAIN(appGlobals->accessMgr);
    self->appGlobals = appGlobals;
    self->fiber = SvFiberCreate(appGlobals->scheduler, NULL, "QBViewRightDVBPVRManager", QBViewRightDVBPVRManagerStep, self);

    return self;
}

void QBViewRightDVBPVRManagerMarkParentalControlledEvent(QBViewRightDVBPVRManager self, int sessionId)
{
    log_fun();

    SvPVRTask pvrTask = SvPlayerManagerGetPVRTask(SvPlayerManagerGetInstance(), sessionId);
    if (!pvrTask) {
        return;
    }

    SvContentMetaData metaData = SvPVRTaskGetMetaData(pvrTask);
    if (!metaData) {
        return;
    }

    SvContentMetaDataSetBooleanProperty(metaData, SVSTRING(QBVIEWRIGHTDVB_IS_PARENTAL_CONTROLLED), true);
}

void QBViewRightDVBPVRManagerEnterCachedPin(QBViewRightDVBPVRManager self, QBViewRightPinType pinType, QBViewRightPinTextSelector textSelector)
{
    log_fun();

    if (!self->isStarted) {
        log_error("Service is not started");
        return;
    }

    QBViewRight viewRight = (QBViewRight) QBCASGetInstance();

    SvString cachedPin = NULL;
    switch (pinType) {
        case QBViewRightPinType_IPPV_PC_1:
            if (QBViewRightPinTextSelector_IPPV != textSelector) {
                cachedPin = self->smartcard.pin;
            }
            break;
        case QBViewRightPinType_NonScPin:
            cachedPin = self->nonSmartcard.pin;
            break;
        default:
            log_error("Not supported cached PIN type = %d", pinType);
            return;
    }

    if (cachedPin) {
        log_state("Using cached PIN = %s for type = %d", SvStringGetCString(cachedPin), pinType);
        QBViewRightEnterPin(viewRight, (SvObject) self, pinType, cachedPin);
    } else {
        log_warn("Cached PIN not present");
        QBViewRightEnterPin(viewRight, NULL, pinType, NULL);
    }
}
