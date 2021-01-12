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

#include "QBStandbyAgent.h"

#include <assert.h>
#include <main.h>
#include <init.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <safeFiles.h>
#include <Logic/StandbyLogic.h>
#include <Windows/standby.h>

#include <settings.h>
#include <Widgets/countdownDialog.h>
#include <QBWidgets/QBDialog.h>
#include <CUIT/Core/types.h>
#include <QBStringUtils.h>

#include <QBApplicationController.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <QBInput/QBInputService.h>
#include <QBConf.h>
#include <Services/standbyAgent/QBStandbyAgentService.h>
#include <Services/cecService.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <QBCAS.h>
#include <QBSecureLogManager.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_QBStandbyAgent, 0, "QBStandbyAgentLogLevel", "0:error, 1:state, 2:debug");

#define log_error(fmt, ...) do { if (env_QBStandbyAgent() >= 0) { SvLogError(  COLBEG() "QBStandbyAgent :: " fmt COLEND_COL(red),   ##__VA_ARGS__); } } while (0)
#define log_state(fmt, ...) do { if (env_QBStandbyAgent() >= 1) { SvLogNotice( COLBEG() "QBStandbyAgent :: " fmt COLEND_COL(blue),  ##__VA_ARGS__); } } while (0)
#define log_debug(fmt, ...) do { if (env_QBStandbyAgent() >= 2) { SvLogNotice( COLBEG() "QBStandbyAgent :: " fmt COLEND_COL(green), ##__VA_ARGS__); } } while (0)

/* according to COMMISSION REGULATION (EC) No 107/2009:
 * The SSTB shall be automatically switched from active mode into standby after less than three
 * hours in active mode following the last user interaction and/or a channel change with an alert
 * message two minutes before going into standby mode.
 */
// need to customize this parameter for other customers
static const unsigned int STANDBY_POPUP_TIMEOUT = 120;

struct QBStandbyAgent_s {
    struct SvObject_ super_;

    AppGlobals  appGlobals;

    SvFiber  fiber;
    SvFiberTimer  timer;

    QBStandbyStateData wantedState;
    QBStandbyStateData currentState;

    SvTime wantedStateLastChangeTime;

    QBWindowContext standbyContext;

    QBCountdownDialog standbyDialog;

    bool  shuttingDown;
    int  wakupDelay;

    SvWeakList services;

    bool deactivateStandby;
};

struct QBStandbyStateData_s {
    struct SvObject_ super_;
    SvWeakReference service;
    QBStandbyState state;
    bool popupWanted;
    SvString reason;
};

SvLocal void QBStandbyAgentSwitchNow(QBStandbyAgent agent, QBStandbyStateData stateToSwitch);

SvLocal void*
QBStandbyStateDataCopy(void *self_, SvErrorInfo *errorOut)
{
    QBStandbyStateData self = (QBStandbyStateData) self_;
    SvObject service = self->service ? SvWeakReferenceTakeReferredObject(self->service) : NULL;
    QBStandbyStateData copy = QBStandbyStateDataCreate(self->state, service, self->popupWanted, self->reason);
    SVTESTRELEASE(service);
    return copy;
}

SvLocal void
QBStandbyStateDataDestroy(void *self_)
{
    QBStandbyStateData self = self_;
    SVTESTRELEASE(self->service);
    SVRELEASE(self->reason);
}

SvLocal SvType
QBStandbyStateData_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBStandbyStateDataDestroy,
        .copy    = QBStandbyStateDataCopy
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBStandbyStateData",
                            sizeof(struct QBStandbyStateData_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

QBStandbyStateData
QBStandbyStateDataCreate(QBStandbyState state, SvObject service, bool popupWanted, SvString reason)
{
    if (!reason) {
        SvLogError("%s(): reason not set", __func__);
        return NULL;
    }

    QBStandbyStateData stateData = (QBStandbyStateData) SvTypeAllocateInstance(QBStandbyStateData_getType(), NULL);
    stateData->service = service ? SvWeakReferenceCreate(service, NULL) : NULL;
    stateData->state = state;
    stateData->popupWanted = popupWanted;
    stateData->reason = SVRETAIN(reason);
    return stateData;
}

void
QBStandbyStateDataSetState(QBStandbyStateData self, QBStandbyState state, bool popupWanted, SvString reason)
{
    if (!self || !reason) {
        SvLogError("%s(): invalid argument passed", __func__);
        return;
    }

    self->state = state;
    self->popupWanted = popupWanted;
    SVRELEASE(self->reason);
    self->reason = SVRETAIN(reason);
}

void QBStandbyStateDataSetFromStateData(QBStandbyStateData self, const QBStandbyStateData source)
{
    if (!self || !source) {
        SvLogError("%s(): invalid argument passed", __func__);
        return;
    }

    if (self != source) {
        QBStandbyStateDataSetState(self, source->state, source->popupWanted, source->reason);
    }
}

SvLocal void
QBStandbyAgentResetServices(QBStandbyAgent agent, bool popupClose)
{
    SvArray services = SvWeakListCreateElementsList(agent->services, NULL);
    SvIterator it = SvArrayGetIterator(services);
    SvObject service = NULL;
    while ((service = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBStandbyAgentService, service, resetState, popupClose);
    }
    SVRELEASE(services);
}

//Popup methods
SvLocal SvString
QBStandbyAgentUpdateTimeLabel(int timeout)
{
    const char *msg = ngettext("Automatic switch to standby in %u second.",
                               "Automatic switch to standby in %u seconds.",
                               timeout);

    return SvStringCreateWithFormat(msg, timeout);
}

SvLocal QBCountdownDialog
QBStandbyAgentDialogCreate(QBStandbyAgent self, AppGlobals appGlobals, unsigned int timeout)
{
    svSettingsPushComponent("StandbyDialog.settings");
    QBCountdownDialog dialog = QBCountdownDialogCreate(appGlobals->res,
                                                       appGlobals->controller,
                                                       gettext("Enter Standby"),
                                                       gettext("Cancel"),
                                                       true,
                                                       QBStandbyAgentUpdateTimeLabel);

    QBCountdownDialogSetTitle(dialog, gettext("Automatic Standby"));

    int position = 0;
    QBCountdownDialogSetTimeout(dialog, timeout);
    QBCountdownDialogAddTimerLabel(dialog, SVSTRING("timeout-message"), position);

    position++;
    QBCountdownDialogAddLabel(dialog,
                     gettext("Press 'Enter Standby' to immediately switch to standby."),
                     SVSTRING("hintA"), position);

    position++;
    QBCountdownDialogAddLabel(dialog,
                     gettext("Press 'Cancel' to abort automatic switch to standby."),
                     SVSTRING("hintB"), position);

    const char *standbyWarning = svSettingsGetString("StandbyDialog", "standbyWarning");
    if (standbyWarning) {
        position++;
        QBCountdownDialogAddLabel(dialog, gettext(standbyWarning), SVSTRING("hintC"), position);
    }

    svSettingsPopComponent();
    return dialog;
}

SvLocal void
QBStandbyAgentPopupCallback(void *agent_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBStandbyAgent agent = agent_;

    if ((buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) ||
        (!(buttonTag && SvStringEqualToCString(buttonTag, "cancel-button")) && QBCountdownDialogGetTimeout(agent->standbyDialog) == 0)) {
        QBStandbyStateData stateData = QBStandbyStateDataCreate(QBStandbyState_standby, (SvObject) agent, false, SVSTRING("popup callback"));
        QBStandbyAgentSwitchNow(agent, stateData);
        SVRELEASE(stateData);
    } else {
        QBStandbyAgentResetServices(agent, true);
    }

    agent->standbyDialog = NULL;
}

SvLocal void
QBStandbyAgentDisplayStandbyPopup(QBStandbyAgent self)
{
    self->standbyDialog = QBStandbyAgentDialogCreate(self, self->appGlobals, STANDBY_POPUP_TIMEOUT);
    SvWidget dlg = QBCountdownDialogGetDialog(self->standbyDialog);
    QBDialogRun(dlg, self, QBStandbyAgentPopupCallback);
}

SvLocal void
QBStandbyAgentCloseDialog(QBStandbyAgent agent)
{
    if (agent->standbyDialog) {
        SvWidget dlg = QBCountdownDialogGetDialog(agent->standbyDialog);
        QBDialogBreak(dlg);
        agent->standbyDialog = NULL;
    }
}

SvLocal const char* QBStandbyStateToString(QBStandbyState state)
{
    switch (state) {
        case QBStandbyState_off:      return "off";       // passive
        case QBStandbyState_standby:  return "standby";   // active
        case QBStandbyState_on:       return "on";        // on
        default: return "???";
    }
}

const char* QBStandbyStateDataToString(QBStandbyStateData state)
{
    return QBStandbyStateToString(state->state);
}

SvLocal QBStandbyState QBStandbyStateFromString(const char* str)
{
    if (!str) {
        return QBStandbyState_on;
    }

    for (QBStandbyState state = 0; state < QBStandbyState_cnt; ++state) {
        if (strcasecmp(QBStandbyStateToString(state), str) == 0) {
            return state;
        }
    }

    return QBStandbyState_on;
}

QBStandbyState QBStandbyStateDataGetState(const QBStandbyStateData state)
{
    return state->state;
}

bool QBStandbyStateDataGetPopupWanted(const QBStandbyStateData state)
{
    return state->popupWanted;
}

SvString QBStandbyStateDataGetReason(const QBStandbyStateData state)
{
    return state->reason;
}

SvLocal QBStandbyStateData QBStandbyStateDataCreateFromString(const char* str)
{
    QBStandbyState state = QBStandbyStateFromString(str);
    return QBStandbyStateDataCreate(state, NULL, false, SVSTRING("created from string"));
}

QBStandbyState QBStandbyStateFromAutoPowerString(const char *value)
{
    if (!value) {
        SvLogWarning("%s(): no value", __func__);
        return QBStandbyState_standby;
    }

    if (strcmp(value, "ACTIVE") == 0)
        return QBStandbyState_standby;
    else if (strcmp(value, "PASSIVE") == 0)
        return QBStandbyState_off;

    SvLogWarning("%s(): not known standby mode: '%s'", __func__, value);
    return QBStandbyState_standby;
}

SvLocal void QBStandbyAgent__dtor__(void *self_)
{
    QBStandbyAgent agent = self_;
    if (agent->fiber) {
        SvFiberDestroy(agent->fiber);
    }

    QBStandbyAgentCloseDialog(agent);

    SVRELEASE(agent->services);
    SVRELEASE(agent->wantedState);
    SVRELEASE(agent->currentState);
}

SvLocal void QBStandbyAgentSwitchStarted(SvObject self_, QBWindowContext from, QBWindowContext to)
{
    QBStandbyAgent agent = (QBStandbyAgent) self_;
    AppGlobals appGlobals = agent->appGlobals;
#if 0
    log_state("SwitchStarted : [%p][%s] -> [%p][%s] (ctx=%p)",
              from, from ? SvTypeGetName(SvObjectGetType((SvGenericObject)from)) : "",
              to,   to   ? SvTypeGetName(SvObjectGetType((SvGenericObject)to))   : "",
              agent->standbyContext);
#endif

    if (agent->standbyContext) {
        assert(to != agent->standbyContext);
        log_state("\"Standby\" WindowContext is being removed");
        SVRELEASE(agent->standbyContext);
        agent->standbyContext = NULL;

        log_state("Switching to state [on]");
        QBStandbyStateDataSetState(agent->currentState, QBStandbyState_on, agent->currentState->popupWanted, SVSTRING("agent switch started 'on'"));
        QBStandbyLogicLeaveStandby(appGlobals->standbyLogic);
        QBSecureLogEvent("QBStandbyAgent", "Notice.ActiveStandby.Leave", "JSON:{ }");
        if (appGlobals->cecService) {
            QBCecServiceOneTouchPlay(appGlobals->cecService);
        }

        return;
    }

    if (to && SvObjectIsInstanceOf((SvObject) to, QBStandbyContext_getType())) {
        log_state("\"Standby\" WindowContext is being added");
        agent->standbyContext = SVRETAIN(to);

        log_state("Switching to state [standby]");
        QBStandbyStateDataSetState(agent->currentState, QBStandbyState_standby, agent->currentState->popupWanted, SVSTRING("agent switch started 'standby'"));

        if (appGlobals->cecService) {
            QBCecServiceBroadcastStandby(appGlobals->cecService);
        }
        QBSecureLogEvent("QBStandbyAgent", "Notice.ActiveStandby.Enter", "JSON:{ }");
        QBStandbyLogicEnterStandby(appGlobals->standbyLogic);
        return;
    }
}

SvLocal void QBStandbyAgentSwitchEnded(SvObject self_, QBWindowContext from, QBWindowContext to)
{
}

SvType QBStandbyAgent_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBStandbyAgent__dtor__
    };
    static SvType type = NULL;
    static const struct QBContextSwitcherListener_t switcherMethods = {
        .started = QBStandbyAgentSwitchStarted,
        .ended = QBStandbyAgentSwitchEnded
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBStandbyAgent",
                            sizeof(struct QBStandbyAgent_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextSwitcherListener_getInterface(), &switcherMethods,
                            NULL);
    }
    return type;
}

QBStandbyAgent QBStandbyAgentCreate(AppGlobals appGlobals)
{
    QBStandbyAgent agent = (QBStandbyAgent) SvTypeAllocateInstance(QBStandbyAgent_getType(), NULL);
    agent->appGlobals = appGlobals;
    agent->currentState = QBStandbyStateDataCreate(QBStandbyState_unknown, (SvObject) agent, false, SVSTRING("init current not set yet"));
    agent->wantedState = QBStandbyStateDataCreate(QBStandbyState_unknown, (SvObject) agent, false, SVSTRING("init wanted not set yet"));

    QBStandbyAgentGetInitialState(agent->currentState, agent->wantedState);

    assert(agent->currentState->state != QBStandbyState_unknown);
    assert(agent->wantedState->state != QBStandbyState_unknown);

    log_state("Created in state : [%s] -> [%s], (reason=[%s])",
              QBStandbyStateDataToString(agent->currentState),
              QBStandbyStateDataToString(agent->wantedState),
              SvStringCString(QBStandbyStateDataGetReason(agent->wantedState)));

    agent->services = SvWeakListCreate(NULL);

    const char *deactivateStandby = QBConfigGet("DEACTIVATE_STANDBY");
    agent->deactivateStandby = deactivateStandby ? !strcmp("YES", deactivateStandby) : false;

    QBApplicationControllerAddListener(appGlobals->controller, (SvObject) agent);
    return agent;
}

#define STATE_FILE  "/etc/vod/standbyState"

void QBStandbyAgentGetInitialState(QBStandbyStateData initial, QBStandbyStateData wanted)
{
    char* wantedStr = NULL;
    QBFileToBuffer(STATE_FILE, &wantedStr);
    QBStandbyStateData wantedState = QBStandbyStateDataCreateFromString(wantedStr);
    free(wantedStr);

    QBStandbyLogicGetStandbyStatus(wantedState, initial, wanted);

    SVRELEASE(wantedState);
}

QBStandbyStateData QBStandbyAgentGetCurrentDataState(const QBStandbyAgent agent)
{
    return agent->currentState;
}

void QBStandbyAgentSetWantedState(QBStandbyAgent agent, QBStandbyStateData stateData)
{
    if (!stateData) {
        SvLogError("%s(): invalid argument passed", __func__);
        return;
    }

    log_state("SetWantedState : [%s] (reason=[%s])", QBStandbyStateDataToString(stateData), SvStringCString(stateData->reason));
    SVTESTRELEASE(agent->wantedState);
    agent->wantedState = SVRETAIN(stateData);
    agent->wantedStateLastChangeTime = SvTimeGet();

    QBBufferToFile(STATE_FILE, QBStandbyStateDataToString(stateData));

    if (agent->fiber) {
        SvFiberActivate(agent->fiber);
    }
}

SvLocal void QBStandbyAgentSwitchNow(QBStandbyAgent agent, QBStandbyStateData stateToSwitch)
{
    log_state("SwitchNow : [%s] -> [%s] (reason=[%s])",
              QBStandbyStateDataToString(agent->currentState),
              QBStandbyStateDataToString(stateToSwitch),
              SvStringCString(QBStandbyStateDataGetReason(stateToSwitch)));

    if (stateToSwitch->state == agent->currentState->state) {
        SvLogError("%s(): alrady in state [%s]", __func__, QBStandbyStateDataToString(agent->currentState));
        return;
    }

    assert(stateToSwitch->state != QBStandbyState_unknown && stateToSwitch->state != QBStandbyState_cnt);
    QBStandbyStateDataSetFromStateData(agent->currentState, stateToSwitch);

    AppGlobals appGlobals = agent->appGlobals;

    /// Do the switch.
    if (stateToSwitch->state == QBStandbyState_on) {
        assert(agent->standbyContext);
        QBApplicationControllerPopContext(appGlobals->controller);
    } else if (stateToSwitch->state == QBStandbyState_standby) {
        QBWindowContext ctx = QBStandbyContextCreate(appGlobals);
        QBApplicationControllerPushContext(appGlobals->controller, ctx);
        SVRELEASE(ctx);
    } else {
        agent->shuttingDown = true;
        agent->wakupDelay = QBStandbyLogicGetWakeupDelay(appGlobals->standbyLogic);

        QBStandbyLogicEnterPassiveStandby(appGlobals->standbyLogic);

        SvLogNotice("QBStandbyAgent: call QBInitStopApplication (stateToSwitch %s)", QBStandbyStateDataToString(stateToSwitch));
        QBInitStopApplication(appGlobals, true, "passive standby");
    }
}

SvLocal QBStandbyStateData QBStandbyAgentCreateCalcStateToSwitch(QBStandbyAgent agent)
{
    QBStandbyStateData wantedState = (QBStandbyStateData) SvObjectCopy((SvObject) agent->wantedState, NULL);

    if (agent->currentState->state == wantedState->state) {
        SVRELEASE(wantedState);
        return (QBStandbyStateData) SvObjectCopy((SvObject) agent->currentState, NULL);
    }

    /// Always go to "off" through "standby".
    /// This is to allow some extra services to start", then we have to wait for those services
    /// to finish before going to "off".
    if (agent->currentState->state == QBStandbyState_on && wantedState->state == QBStandbyState_off) {
        QBStandbyStateDataSetState(wantedState, QBStandbyState_standby, wantedState->popupWanted, SVSTRING("switch to 'off' pass by 'standby'"));
    }

    if (QBStandbyLogicCanSwitch(agent->appGlobals->standbyLogic, agent->currentState, wantedState)) {
        return wantedState;
    }

    /// do not switch
    SVRELEASE(wantedState);
    return (QBStandbyStateData) SvObjectCopy((SvObject) agent->currentState, NULL);
}

SvLocal void QBStandbyAgent_step(void* agent_)
{
    QBStandbyAgent agent = (QBStandbyAgent) agent_;
    SvFiberDeactivate(agent->fiber);
    SvFiberTimerActivateAfter(agent->timer, SvTimeFromMs(1000));

    log_debug("shown standby dialog : %s ", agent->standbyDialog ? "YES" : "NO");

    if (agent->standbyDialog) {
        return;     // wait on user action
    }

    SvArray services = SvWeakListCreateElementsList(agent->services, NULL);
    SvArray servicesToStates = SvArrayCreate(NULL);
    SvIterator it = SvArrayGetIterator(services);
    SvObject item = NULL;
    while ((item = SvIteratorGetNext(&it))) {
        QBStandbyStateData stateData = SvInvokeInterface(QBStandbyAgentService, item, createStateData);
        log_debug("add state data = %s", QBStandbyStateDataToString(stateData));
        SvArrayAddObject(servicesToStates, (SvObject) stateData);
        SVRELEASE(stateData);
    }

    QBStandbyStateData wantedStateData = QBStandbyStateDataCreate(agent->wantedState->state, (SvObject) agent, agent->wantedState->popupWanted, SVSTRING("agent step"));
    bool changed = QBStandbyLogicMergeWantedStates(agent->appGlobals->standbyLogic, servicesToStates, wantedStateData);
    SVRELEASE(services);
    SVRELEASE(servicesToStates);

    if (agent->deactivateStandby) {
        QBStandbyStateDataSetState(wantedStateData, QBStandbyState_on, false, SVSTRING("forced by DEACTIVATE_STANDBY"));
    }

    log_debug("changed=[%s], current wanted state: [%s], new wanted state: [%s], old reason: [%s], new reason: [%s]",
              changed ? "YES" : "NO",
              QBStandbyStateDataToString(agent->wantedState),
              QBStandbyStateDataToString(wantedStateData),
              SvStringCString(agent->wantedState->reason),
              SvStringCString(wantedStateData->reason));

    if (agent->wantedState->state != wantedStateData->state) {
        log_state("SetWantedState : [%s] (reason=[%s])", QBStandbyStateDataToString(wantedStateData), SvStringCString(wantedStateData->reason));
        QBStandbyStateDataSetFromStateData(agent->wantedState, wantedStateData);
        agent->wantedStateLastChangeTime = SvTimeGet();

        QBBufferToFile(STATE_FILE, QBStandbyStateDataToString(wantedStateData));
    }

    /// Check if auto-switch from "on" is still needed.
    if ((agent->currentState->state == QBStandbyState_on) &&
        (agent->wantedState->state != QBStandbyState_on)) {
        // this value equals SvTimeGetZero() when no input was ever seen
        SvTime lastEventTime = QBInputServiceGetLastEventTime(NULL);
        if (SvTimeCmp(agent->wantedStateLastChangeTime, lastEventTime) < 0) {
            QBStandbyStateData state = QBStandbyStateDataCreate(QBStandbyState_on, (SvObject) agent, false, SVSTRING("user input detected"));
            QBStandbyAgentSetWantedState(agent, state);
            SVRELEASE(state);
        }
    }

    /// Check if the switch is possible.
    QBStandbyStateData stateToSwitch = QBStandbyAgentCreateCalcStateToSwitch(agent);
    log_debug("current state: [%s], state to switch: [%s], old current reason: [%s], state to switch reason: [%s]",
              QBStandbyStateDataToString(agent->currentState),
              QBStandbyStateDataToString(stateToSwitch),
              SvStringCString(agent->currentState->reason),
              SvStringCString(stateToSwitch->reason));

    if (stateToSwitch->state == agent->currentState->state) {
        log_debug("do not perform switch");
        goto fini;
    }

    if (changed &&
        wantedStateData->popupWanted &&                                                                           // if the pupup is wanted by any plugin
        ((stateToSwitch->state == QBStandbyState_standby) || (stateToSwitch->state == QBStandbyState_off))) {     // and STB is going into standby mode
        log_state("show standby popup");
        QBStandbyAgentDisplayStandbyPopup(agent);                                                                 // show popup
        goto fini;
    }

    /// Do the switch.
    QBStandbyAgentSwitchNow(agent, stateToSwitch);

fini:
    SVRELEASE(stateToSwitch);
    SVRELEASE(wantedStateData);
}

bool QBStandbyAgentIsStandby(const QBStandbyAgent self)
{
    if (!self || !self->fiber) {
        log_error("%s(): agent not exits or not started yet", __func__);
        return false;
    }

    const QBStandbyStateData current = QBStandbyAgentGetCurrentDataState(self);
    return current->state != QBStandbyState_on;


}

SvLocal void QBStandbyAgent_want_standby_state(void *agent_)
{
    QBStandbyAgent agent = (QBStandbyAgent) agent_;
    QBStandbyState currState = QBStandbyStateDataGetState(agent->currentState);
    if (currState == QBStandbyState_on)
        SvInvokeInterface(QBCAS, QBCASGetInstance(), activeStandby, false);
    else if (currState == QBStandbyState_standby)
        SvInvokeInterface(QBCAS, QBCASGetInstance(), activeStandby, true);
}

static const struct QBCASCallbacks_s s_cas_callbacks = {
    .want_standby_state = &QBStandbyAgent_want_standby_state,
};

void QBStandbyAgentStart(QBStandbyAgent agent, SvScheduler scheduler)
{
    if (agent->fiber) {
        log_error("Start() : already started!");
        return;
    }

    agent->fiber = SvFiberCreate(scheduler, NULL, "QBStandbyAgent", &QBStandbyAgent_step, agent);
    agent->timer = SvFiberTimerCreate(agent->fiber);
    SvFiberActivate(agent->fiber);

    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks)&s_cas_callbacks, agent, "StandbyAgent");
    log_state("QBStandbyAgent started");
}

void QBStandbyAgentStop(QBStandbyAgent agent)
{
    if (!agent->fiber) {
        return;
    }

    SvFiberDestroy(agent->fiber);
    agent->fiber = NULL;
    agent->timer = NULL;

    if (agent->standbyContext) {
        QBStandbyContextDisableAutoOn(agent->standbyContext);
        SVRELEASE(agent->standbyContext);
        agent->standbyContext = NULL;
    }

    log_state("QBStandbyAgent stoped");
}

bool QBStandbyAgentIsCompleted(const QBStandbyAgent agent)
{
    if (!agent->standbyContext) {
        return true;
    }

    return QBStandbyContextIsCompleted(agent->standbyContext);
}

int QBStandbyAgentGetWakeupDelay(const QBStandbyAgent agent)
{
    if (!agent->shuttingDown) {
        return 0;
    }

    return agent->wakupDelay;
}

void QBStandbyAgentAddService(const QBStandbyAgent agent, SvObject service)
{
    if (!agent || !service)
        return;

    if (!SvObjectIsImplementationOf(service, QBStandbyAgentService_getInterface())) {
        log_error("Interface is not implemented");
        return;
    }

    SvArray services = SvWeakListCreateElementsList(agent->services, NULL);
    SvIterator it = SvArrayGetIterator(services);
    SvObject agentService = NULL;
    while ((agentService = SvIteratorGetNext(&it))) {
        if (SvObjectGetType(service) == SvObjectGetType(agentService)) {
            SvLogWarning("QBStandbyAgentAddService: plugin of that type is already enabled: %s", SvObjectGetTypeName(service));
        }
    }
    SVRELEASE(services);
    SvWeakListPushBack(agent->services, service, NULL);
}

void QBStandbyAgentStartServices(const QBStandbyAgent agent)
{
    if (!agent) {
        return;
    }

    SvArray services = SvWeakListCreateElementsList(agent->services, NULL);
    SvIterator it = SvArrayGetIterator(services);
    SvObject item = NULL;
    while ((item = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBStandbyAgentService, item, resetState, false);
        SvInvokeInterface(QBStandbyAgentService, item, start);
    }
    SVRELEASE(services);
}

void QBStandbyAgentStopServices(const QBStandbyAgent agent)
{
    if (!agent) {
        return;
    }

    SvArray services = SvWeakListCreateElementsList(agent->services, NULL);
    SvIterator it = SvArrayGetIterator(services);
    SvObject item = NULL;
    while ((item = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBStandbyAgentService, item, resetState, false);
        SvInvokeInterface(QBStandbyAgentService, item, stop);
    }
    SVRELEASE(services);
}
