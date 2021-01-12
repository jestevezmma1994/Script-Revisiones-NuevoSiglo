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

#include <Logic/GlobalInputLogic.h>
#include <Logic/GlobalInputLogicInternal.h>

#include <libintl.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvType.h>
#include <QBTimeLimit.h>
#include <settings.h>
#include <QBConf.h>
#include <QBInput/QBInputService.h>
#include <QBInput/QBInputDevice.h>
#include <QBInput/QBInputCodes.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <QBWidgets/QBDialog.h>
#include <Services/QBStandbyAgent.h>
#include <Services/core/playlistManager.h>
#include <Services/QBRCUPairingService.h>
#include <Windows/QBRCUPairingWindow.h>
#include <Logic/QBLogicFactory.h>
#include <Logic/GUILogic.h>
#include <main.h>

#include <QBShellCmds.h>

#if HomecastFactory == 1
#include <Windows/Factory/HomecastFactory.h>
#endif

// TODO: move it to constructor
static const unsigned int STANDBY_POPUP_TIMEOUT = 10;
static const unsigned int STANDBY_KEY_POWER_TIMEOUT = 2;

typedef enum {
    POWER_BUTTON_SOURCE_RC = 0,
    POWER_BUTTON_SOURCE_FP,
    POWER_BUTTON_SOURCE_UNKNOWN
} QBPowerButtonSource;

// Private API

SvLocal SvString
QBGlobalInputLogicStandbyUpdateTimeLabel(int timeout)
{
    const char *msg = ngettext("Active standby will start in %u second.",
                               "Active standby will start in %u seconds.",
                               timeout);

    return SvStringCreateWithFormat(msg, timeout);
}

SvLocal QBCountdownDialog
QBGlobalInputLogicStandbyDialogCreate(QBGlobalInputLogic self,
                                      AppGlobals appGlobals,
                                      unsigned int timeout)
{
    svSettingsPushComponent("StandbyDialog.settings");
    QBCountdownDialog dialog = QBCountdownDialogCreate(appGlobals->res,
                                                       appGlobals->controller,
                                                       gettext("Enter Active Standby"),
                                                       gettext("Cancel"), true,
                                                       QBGlobalInputLogicStandbyUpdateTimeLabel);

    QBCountdownDialogSetTitle(dialog, gettext("Enter Standby"));

    int position = 0;
    QBCountdownDialogAddLabel(dialog,
                              gettext("Press power button to cancel standby."),
                              SVSTRING("hintA"), position);

    position++;
    char *msg;
    asprintf(&msg,
             ngettext("Press and hold power button for %u second to enter passive standby.",
                      "Press and hold power button for %u seconds to enter passive standby.",
                      self->keyPowerTimeout),
             self->keyPowerTimeout);
    QBCountdownDialogAddLabel(dialog, msg,
                              SVSTRING("hintB"), position);
    free(msg);

    const char *standbyWarning = svSettingsGetString("StandbyDialog", "standbyWarning");
    if (standbyWarning) {
        position++;
        QBCountdownDialogAddLabel(dialog, gettext(standbyWarning), SVSTRING("hintC"), position);
    }

    position++;
    QBCountdownDialogSetTimeout(dialog, timeout);
    QBCountdownDialogAddTimerLabel(dialog, SVSTRING("timeout-message"), position);

    svSettingsPopComponent();
    return dialog;
}

SvLocal void
QBGlobalInputLogicStandbyPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBGlobalInputLogic self = self_;

    if ((buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) ||
        (!(buttonTag && SvStringEqualToCString(buttonTag, "cancel-button"))
         && QBCountdownDialogGetTimeout(self->standbyDialog) == 0)) {
        QBStandbyStateData state = QBStandbyStateDataCreate(QBStandbyState_standby,
                                                            (SvObject) self, false,
                                                            SVSTRING("power key"));
        QBStandbyAgentSetWantedState(self->appGlobals->standbyAgent, state);
        SVRELEASE(state);
    }

    self->standbyDialog = NULL;
}

SvLocal void
QBGlobalInputLogicDisplayStandbyPopup(QBGlobalInputLogic self)
{
    self->standbyDialog = QBGlobalInputLogicStandbyDialogCreate(self, self->appGlobals, self->popupTimeout);
    SvWidget dlg = QBCountdownDialogGetDialog(self->standbyDialog);
    QBDialogRun(dlg, self, QBGlobalInputLogicStandbyPopupCallback);
}

SvLocal bool
QBGlobalInputLogicStartRCUPairing(QBGlobalInputLogic self)
{
    QBRCUPairingService rcuParingService = (QBRCUPairingService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                             SVSTRING("QBRCUPairingService"));
    if (!QBRCUPairingServiceIsRF4CEAvailable(rcuParingService)) {
        return false;
    }

    const int PAIRING_FROM_MENU_ATTEMPTS_COUNT = 2;
    QBRCUPairingServiceStartPairing(rcuParingService, PAIRING_FROM_MENU_ATTEMPTS_COUNT);

    SvObject currentContext = (SvObject) QBApplicationControllerGetCurrentContext(self->appGlobals->controller);
    if (SvObjectIsInstanceOf(currentContext, QBRCUPairingContext_getType())) {
        return true;
    }

    QBRCUPairingContext rcuPairingContext = QBRCUPairingContextCreate();
    QBApplicationControllerPushContext(self->appGlobals->controller, (QBWindowContext) rcuPairingContext);
    SVRELEASE(rcuPairingContext);
    return true;
}

SvLocal bool
QBGlobalInputLogicHandlePowerKey(QBGlobalInputLogic self,
                                 QBStandbyStateData stateData,
                                 QBPowerButtonSource powerSrc,
                                 uint32_t keyCode)
{
    // The QBKEY_STANDBY is generated as QBKEY_POWER pressed for a longer time.
    // If QBKEY_STANDBY is generated from front panel it means that we should
    // check if device supports RF4CE, and if so start pairing with a new
    // remote.
    if (powerSrc == POWER_BUTTON_SOURCE_FP && keyCode == QBKEY_STANDBY) {
        SvLogNotice("Detected long press power button from the front panel");
        if (QBGlobalInputLogicStartRCUPairing(self)) {
            return true;
        }
    }

    // If box is in any standby state, always wake him up
    if (QBStandbyStateDataGetState(stateData) != QBStandbyState_on) {
        QBStandbyStateDataSetState(stateData, QBStandbyState_on,
                                   QBStandbyStateDataGetPopupWanted(stateData),
                                   SVSTRING("global input logic: set on"));
        goto fini;
    }

    // Fix for generic: if we choose deep standby for hardware platform which doesn't support it
    // TODO: Check (API in PlatformHAL) if device supports deep standby
    QBStandbyState passiveStandby = QBStandbyState_off; // isDeepStandbySupported() ? QBStandbyState_off : QBStandbyState_standby;

    if (self->standbyDialog) {
        if (keyCode == QBKEY_POWER) { // Same as cancel, don't change application state
            QBDialogBreak(QBCountdownDialogGetDialog(self->standbyDialog));
            return true;
        } else { // QBKEY_STANDBY
            QBStandbyStateDataSetState(stateData, passiveStandby,
                                       QBStandbyStateDataGetPopupWanted(stateData),
                                       SVSTRING("global input logic dialog: set passive standby"));
            goto fini;
        }
    }

    const char *powerKeyFunction;
    if (powerSrc == POWER_BUTTON_SOURCE_FP)
        powerKeyFunction = QBConfigGet("FP_POWER");
    else
        powerKeyFunction = QBConfigGet("RC_POWER");

    if (!powerKeyFunction) {
        // default behavior: deep standby for power button on front panel, active for remote
        QBStandbyState standby = powerSrc == POWER_BUTTON_SOURCE_FP ? passiveStandby : QBStandbyState_standby;
        QBStandbyStateDataSetState(stateData, standby,
                                   QBStandbyStateDataGetPopupWanted(stateData),
                                   SVSTRING("global input logic power button: set passive standby"));
        goto fini;
    }

    if (strncmp(powerKeyFunction, "COMBINED", strlen("COMBINED")) == 0) {
        if (powerSrc == POWER_BUTTON_SOURCE_FP) // Impossible
            return true;

        if (keyCode == QBKEY_STANDBY) {
            // Long pressed power button, go to deep standby immediately
            QBStandbyStateDataSetState(stateData, passiveStandby,
                                       QBStandbyStateDataGetPopupWanted(stateData),
                                       SVSTRING("global input logic long pressed power button: set passive standby"));
        } else {
            QBGlobalInputLogicDisplayStandbyPopup(self);
        }
    } else if (strncmp(powerKeyFunction, "ACTIVE", strlen(powerKeyFunction)) == 0) {
        QBStandbyStateDataSetState(stateData, QBStandbyState_standby,
                                   QBStandbyStateDataGetPopupWanted(stateData),
                                   SVSTRING("global input logic set active standby"));
    } else {
        QBStandbyStateDataSetState(stateData, passiveStandby,
                                   QBStandbyStateDataGetPopupWanted(stateData),
                                   SVSTRING("global input logic set passive standby"));
    }

    /// TODO: disable current reminder, just like when switching channel from rc

fini:
    {
        QBStandbyStateData wantedStateData = (QBStandbyStateData) SvObjectCopy((SvObject) stateData, NULL);
        QBStandbyAgentSetWantedState(self->appGlobals->standbyAgent, wantedStateData);
        SVRELEASE(wantedStateData);
        return true;
    }
}

SvLocal bool
isInputFromChassis(QBInputDeviceID id)
{
    SvObject device = QBInputServiceGetDeviceByID(id, NULL);
    if (!device)
        return false;
    SvString name = SvInvokeInterface(QBInputDevice, device, getName);
    if (!name)
        return false;
    //SvLogNotice("  QBGlobalInputLogic :: InputEventHandler :: device=[%s]", SvStringCString(name));
    if (SvStringLength(name) >= 3) {
        // get pointer to the beginning, add length and subtract 3 (length of "_fp")
        return strcmp(SvStringCString(name) + SvStringLength(name) - 3, "_fp") == 0;
    } else {
        return false;
    }
}

SvLocal QBPowerButtonSource
decodePowerKey(const QBInputEvent *e, uint32_t *keyCode)
{
    int code = e->u.key.code;

    if (code != QBKEY_POWER && code != QBKEY_STANDBY)
        return POWER_BUTTON_SOURCE_UNKNOWN;

    /// React only on "released", to avoid
    /// 1. problems with auto-repeat
    /// 1. races with handling of "typed" and last user input time (which would always be next "release")
    if (e->type != QBInputEventType_keyReleased)
        return POWER_BUTTON_SOURCE_UNKNOWN;

    if (keyCode)
        *keyCode = code;

    return isInputFromChassis(e->src) ? POWER_BUTTON_SOURCE_FP : POWER_BUTTON_SOURCE_RC;
}

bool
QBGlobalInputLogicHandlePowerButton(QBGlobalInputLogic self,
                                    const QBInputEvent *e)
{
    uint32_t keyCode = 0;
    QBPowerButtonSource powerType = decodePowerKey(e, &keyCode);
    if (powerType == POWER_BUTTON_SOURCE_UNKNOWN)
        return false;

    QBStandbyStateData currentState = QBStandbyAgentGetCurrentDataState(self->appGlobals->standbyAgent);
    QBStandbyStateData state = (QBStandbyStateData) SvObjectCopy((SvObject) currentState, NULL);
    bool retVal = QBGlobalInputLogicHandlePowerKey(self, state, powerType, keyCode);
    SVRELEASE(state);
    return retVal;
}

#if HomecastFactory == 1
// AMERELES #1853 Desactivación a demanda y por STB de la protección HDCP
SvLocal
void QBGlobalInputLogicHDCPDisableCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBShellExec("reboot");
}
#endif

bool
QBGlobalInputLogicHandleHomecastFactory(QBGlobalInputLogic self,
                                        const QBInputEvent *e)
{
#if HomecastFactory == 1
    if (e->type == QBInputEventType_keyTyped) {
        QBMagicCodeAdd(self->factoryMagicCode, e->u.key.code);
        if (QBMagicCodeMatches(self->factoryMagicCode)) {
            QBWindowContext ctx = QBHomecastFactoryContextCreate(self->appGlobals);
            QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
            SVRELEASE(ctx);
            return true;
        }
        // AMERELES BEGIN #1853 Desactivación a demanda y por STB de la protección HDCP
        QBMagicCodeAdd(self->hdcp_disabling_code, e->u.key.code);
        if (QBMagicCodeMatches(self->hdcp_disabling_code))
        {
            svSettingsPushComponent("pvrPlayer.settings");
            
            QBDialogParameters params = {
              .app        = self->appGlobals->res,
              .controller = self->appGlobals->controller,
              .widgetName = "Dialog",
              .ownerId    = 0
            };
            SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
            
            QBDialogAddButton(dialog, SVSTRING("button"), gettext("OK"), 1);
            
            const char *HDCPENABLE = QBConfigGet("HDCPENABLE");
            if (HDCPENABLE && strcmp(HDCPENABLE, "disabled") == 0)
            {
                QBDialogSetTitle(dialog, "FUNCIÓN H - ON");
                
                QBConfigSet("HDCPENABLE", "enabled");
                QBConfigSave();
            }
            else
            {
                QBDialogSetTitle(dialog, "FUNCIÓN H - OFF");
                
                QBConfigSet("HDCPENABLE", "disabled");
                QBConfigSave();
            }
            
            QBDialogRun(dialog, self, QBGlobalInputLogicHDCPDisableCallback);
            svSettingsPopComponent();
            return true;
        }
        // AMERELES END #1853 Desactivación a demanda y por STB de la protección HDCP
    }
#endif
    return false;
}

bool
QBGlobalInputLogicHandleEPGKey(QBGlobalInputLogic self,
                               const QBInputEvent *e)
{
    QBTimeLimitBegin(time_event_handler, QBTimeDefaultTimeThreshold);
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    if (playlists && self->appGlobals->guiLogic) {
        SvObject playlist = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
        if (SvInvokeInterface(SvEPGChannelView, playlist, getCount) <= 0)
            return false;
        QBWindowContext ctx = QBGUILogicPrepareEPGContext(self->appGlobals->guiLogic, NULL, NULL);
        QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
    }
    QBTimeLimitEnd_(time_event_handler, "QBGlobalInputLogicInputEventHandler.EPG");
    return false;
}

// QBGlobalInputLogic virtual methods

SvLocal QBGlobalInputLogic
QBGlobalInputLogicInit_(QBGlobalInputLogic self,
                        AppGlobals appGlobals,
                        SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    self->appGlobals = appGlobals;

#if HomecastFactory == 1
    uint16_t factoryMagicCode[] = { QBKEY_RED, QBKEY_GREEN, QBKEY_GREEN, QBKEY_YELLOW, QBKEY_RED };
    self->factoryMagicCode = QBMagicCodeCreate(factoryMagicCode, sizeof(factoryMagicCode) / sizeof(*factoryMagicCode));

    // AMERELES #1853 Desactivación a demanda y por STB de la protección HDCP
    uint16_t hdcp_disabling_code[] = {QBKEY_RED, QBKEY_RED, QBKEY_GREEN, QBKEY_GREEN, QBKEY_RED};
    self->hdcp_disabling_code = QBMagicCodeCreate(hdcp_disabling_code, sizeof(hdcp_disabling_code) / sizeof(*hdcp_disabling_code));
#endif

    self->popupTimeout = STANDBY_POPUP_TIMEOUT;
    self->keyPowerTimeout = STANDBY_KEY_POWER_TIMEOUT;

    SvErrorInfoPropagate(error, errorOut);
    return self;
}

SvLocal bool
QBGlobalInputLogicHandleInputEvent_(QBGlobalInputLogic self,
                                    const QBInputEvent *e)
{
    switch (e->u.key.code) {
        case QBKEY_EPG:
            QBGlobalInputLogicHandleEPGKey(self, e);
            break;
        default:
            break;
    }

    if (QBGlobalInputLogicHandleHomecastFactory(self, e))
        return true;

    return QBGlobalInputLogicHandlePowerButton(self, e);
}

SvLocal void
QBGlobalInputLogicUpdateMainMenuPosition_(QBGlobalInputLogic self)
{
}

// SvObject virtual methods

SvLocal void
QBGlobalInputLogicDestroy(void *self_)
{
    QBGlobalInputLogic self = self_;
    if (self->standbyDialog) {
        SvWidget dlg = QBCountdownDialogGetDialog(self->standbyDialog);
        QBDialogBreak(dlg);
        self->standbyDialog = NULL;
    }
#if HomecastFactory == 1
    if (self->factoryMagicCode)
        QBMagicCodeDestroy(self->factoryMagicCode);
    
    // AMERELES BEGIN #1853 Desactivación a demanda y por STB de la protección HDCP
    if (self->hdcp_disabling_code)
        QBMagicCodeDestroy(self->hdcp_disabling_code);
    // AMERELES　END  #1853 Desactivación a demanda y por STB de la protección HDCP
#endif
    SVRELEASE(self->applicationController);
}

// Public API

SvType
QBGlobalInputLogic_getType(void)
{
    static SvType type = NULL;

    static const struct QBGlobalInputLogicVTable_ logicVTable = {
        .super_                 = {
            .destroy            = QBGlobalInputLogicDestroy,
        },
        .init                   = QBGlobalInputLogicInit_,
        .handleInputEvent       = QBGlobalInputLogicHandleInputEvent_,
        .updateMainMenuPosition = QBGlobalInputLogicUpdateMainMenuPosition_
    };

    if (!type) {
        SvTypeCreateVirtual("QBGlobalInputLogic",
                            sizeof(struct QBGlobalInputLogic_),
                            SvObject_getType(),
                            sizeof(logicVTable), &logicVTable,
                            &type,
                            NULL);
    }

    return type;
}

QBGlobalInputLogic
QBGlobalInputLogicCreate(AppGlobals appGlobals, SvErrorInfo *errorOut)
{
    QBGlobalInputLogic self = NULL;
    SvErrorInfo error = NULL;

    SvType logicClass = QBLogicFactoryFindImplementationOf(QBLogicFactoryGetInstance(),
                                                           QBGlobalInputLogic_getType(), &error);
    if (!logicClass)
        goto fini;

    self = (QBGlobalInputLogic) SvTypeAllocateInstance(logicClass, &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "can't allocate %s", SvTypeGetName(logicClass));
        goto fini;
    }

    if (!SvInvokeVirtual(QBGlobalInputLogic, self, init, appGlobals, &error)) {
        SVRELEASE(self);
        self = NULL;
        goto fini;
    }

    SvLogNotice("%s(): created instance of %s", __func__,
                SvTypeGetName(logicClass));

    QBServiceRegistry service = QBServiceRegistryGetInstance();
    self->applicationController = SVRETAIN(QBServiceRegistryGetService(service, SVSTRING("QBApplicationController")));

fini:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}
