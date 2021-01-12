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

#include <main.h>

#include <Logic/NetworkLogic.h>
#include <Logic/NetworkLogicPrivate.h>
#include <Logic/QBLogicFactory.h>

#include <Windows/QBNetworkSettingsWindow.h>
#include <Widgets/networkProblemDialog.h>
#include <QBWidgets/QBDialog.h>

#include <Services/core/QBNetworkWatcher.h>
#include <QBNetworkMonitor.h>

#include <QBApplicationController.h>
#include <QBWindowContext.h>
#include <QBContextSwitcher.h>

#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBObserver.h>

#include <settings.h>

#include <SvFoundation/SvType.h>

#include <QBConfig.h>

#include <SvCore/SvLog.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>

#include <libintl.h>

#define QB_NETWORK_LOGIC_VPN_SETUP_TIMEOUT_SEC 5

SvLocal void
QBNetworkLogicNetworkWatcherPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBNetworkLogic self = (QBNetworkLogic) self_;

    QBWindowContext currentCtx = QBApplicationControllerGetCurrentContext(self->controller);
    bool isNetworkSettingsCtx = SvObjectIsInstanceOf((SvObject) currentCtx, QBNetworkSettingsContext_getType());
    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button") && !isNetworkSettingsCtx) {
        QBWindowContext ctx = QBNetworkSettingsContextCreate(self->res,
                                                             self->scheduler,
                                                             self->initLogic,
                                                             self->controller,
                                                             self->textRenderer,
                                                             self->networkMonitor, false);
        QBApplicationControllerPushContext(self->controller, ctx);
        SVRELEASE(ctx);
    }

    self->dialog = NULL;
}

SvLocal void
QBNetworkLogicShowDiagnosisPopup(QBNetworkLogic self, QBNetworkWatcherOverallDiagnosis diagnosis)
{
    QBWindowContext currentCtx = QBApplicationControllerGetCurrentContext(self->controller);
    bool isNetworkSettingsCtx = SvObjectIsInstanceOf((SvObject) currentCtx, QBNetworkSettingsContext_getType());
    if (isNetworkSettingsCtx)
        return;

    if (!self->dialog) {
        self->dialog = QBNetworkProblemDialogCreate(self->res, self->controller);
        SvWidget dialog = QBCountdownDialogGetDialog(self->dialog);
        QBDialogRun(dialog, self, QBNetworkLogicNetworkWatcherPopupCallback);
    }

    QBNetworkProblemDialogSetDiagnosis(self->dialog, diagnosis);
}

SvLocal void
QBNetworkLogicVPNIssueStep(void *self_)
{
    QBNetworkLogic self = self_;

    SvFiberDeactivate(self->vpnIssueFiber);
    SvFiberEventDeactivate(self->vpnIssueTimer);

    QBNetworkLogicShowDiagnosisPopup(self, QBNetworkWatcherOverallDiagnosis_noVPN);
}

SvLocal void
QBNetworkLogicNetworkWatcherStateChanged(SvObject self_, SvObject observedObject, SvObject arg)
{
    QBNetworkLogic self = (QBNetworkLogic) self_;

    QBNetworkWatcherOverallDiagnosis diagnosis = QBNetworkWatcherGetOverallDiagnosis(self->networkWatcher);

    if (diagnosis == QBNetworkWatcherOverallDiagnosis_ok) {
        if (self->vpnIssueTimer)
            SvFiberEventDeactivate(self->vpnIssueTimer);
        if (self->dialog) {
            SvWidget dialog = QBCountdownDialogGetDialog(self->dialog);
            QBDialogBreak(dialog);
        }
        return;
    }

    if (diagnosis == QBNetworkWatcherOverallDiagnosis_noVPN && self->diagnosis > diagnosis ) {
        SvFiberTimerActivateAfter(self->vpnIssueTimer, SvTimeFromMs(QB_NETWORK_LOGIC_VPN_SETUP_TIMEOUT_SEC * 1000));
    } else {
        QBNetworkLogicShowDiagnosisPopup(self, diagnosis);
    }

    self->diagnosis = diagnosis;
}

SvLocal void
QBNetworkLogicContextSwitchStarted(SvObject self_, QBWindowContext from, QBWindowContext to)
{
}

SvLocal void
QBNetworkLogicContextSwitchEnded(SvObject self_, QBWindowContext from, QBWindowContext to)
{
    QBNetworkLogic self = (QBNetworkLogic) self_;

    if (SvObjectIsInstanceOf((SvObject) from, QBNetworkSettingsContext_getType())) {
        QBNetworkWatcherOverallDiagnosis diagnosis = QBNetworkWatcherGetOverallDiagnosis(self->networkWatcher);
        if (diagnosis != QBNetworkWatcherOverallDiagnosis_ok)
            QBNetworkLogicShowDiagnosisPopup(self, diagnosis);
    }
}

SvLocal QBNetworkLogic
QBNetworkLogicInit_(QBNetworkLogic self, QBNetManager *netManager, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    // we could register listener object, that will receive notifications when network
    // configuration changes, but we are refreshing the contents of SETTINGS menu every time
    // we enter that menu, so we don't really need this now
    if (netManager) {
        SvLogNotice("Creating net manager");
        *netManager = (QBNetManager) SvTypeAllocateInstance(QBNetManager_getType(), NULL);
        SvLogNotice("Initializing net manager");
        QBNetManagerInit(*netManager, self->networkMonitor, QBNetManagerInterfacesDisplayingPolicy_all, NULL);
        // don't want notifications about loopback
        QBNetManagerMaskInterface(*netManager, SVSTRING("lo"), true, NULL);
        QBNetManagerPollStep(*netManager, NULL);
        QBNetManagerSetGlobal(*netManager);
    }

    SvErrorInfoPropagate(error, errorOut);
    return self;
}

SvLocal void
QBNetworkLogicStart_(QBNetworkLogic self)
{
    if (self->started)
        return;

    QBNetworkManagerVPNType vpnType = QBNetworkMonitorGetVPNType(self->networkMonitor);

    SvLogNotice("Creating network watcher");
    QBNetworkWatcherRequirements params = SvInvokeVirtual(QBNetworkLogic, self, getRequirements, vpnType);

    if (vpnType != QBNetworkManagerVPNType_none) {
        self->vpnIssueFiber = SvFiberCreate(self->scheduler, NULL, "QBNetworkLogicVPNIssueStep", QBNetworkLogicVPNIssueStep, self);
        self->vpnIssueTimer = SvFiberTimerCreate(self->vpnIssueFiber);
    }

    self->networkWatcher = QBNetworkWatcherCreate(self->networkMonitor, &params, NULL);
    if (!self->networkWatcher) {
        SvLogError("can't create network watcher");
        return;
    }
    QBNetworkWatcherAddListener(self->networkWatcher, (SvObject) self, NULL);
    QBServiceRegistryRegisterService(QBServiceRegistryGetInstance(), (SvObject) self->networkWatcher, NULL);

    QBApplicationControllerAddListener(self->controller, (SvObject) self);

    self->diagnosis = QBNetworkWatcherOverallDiagnosis_ok;
    self->started = true;
}

SvLocal void
QBNetworkLogicStop_(QBNetworkLogic self)
{
    if (!self->started)
        return;

    QBApplicationControllerRemoveListener(self->controller, (SvObject) self);

    if (self->vpnIssueFiber) {
        SvFiberEventDeactivate(self->vpnIssueTimer);
        SvFiberDestroy(self->vpnIssueFiber);
        self->vpnIssueFiber = NULL;
    }

    SVTESTRELEASE(self->networkWatcher);
    self->networkWatcher = NULL;
    self->started = false;
}

SvLocal QBNetworkWatcherRequirements
QBNetworkLogicGetRequirements_(QBNetworkLogic self, QBNetworkManagerVPNType vpnType)
{
    bool needsLAN = true;
    const char *needsLANConfig = QBConfigGet("NETWORK.REQUIREMENTS.LAN");
    if (needsLANConfig) {
        needsLAN = (strcmp(needsLANConfig, "enabled") == 0);
    }

    bool needsWAN = true;
    const char *needsWANConfig = QBConfigGet("NETWORK.REQUIREMENTS.WAN");
    if (needsWANConfig) {
        needsWAN = (strcmp(needsWANConfig, "enabled") == 0);
    }

    bool needsDNS = true;
    const char *needsDNSConfig = QBConfigGet("NETWORK.REQUIREMENTS.DNS");
    if (needsDNSConfig) {
        needsDNS = (strcmp(needsDNSConfig, "enabled") == 0);
    }

    QBNetworkWatcherRequirements params = {
        .needsLAN = needsLAN,
        .needsWAN = needsWAN,
        .needsDNS = needsDNS,
        .vpnType  = vpnType,
        .jitterToleranceMs = -1,
    };

    return params;
}

SvLocal void
QBNetworkLogicDestroy(void *self_)
{
    QBNetworkLogic self = self_;
    QBNetworkLogicStop_(self_);
    SVRELEASE(self->initLogic);
    SVRELEASE(self->controller);
    SVRELEASE(self->textRenderer);
    SVRELEASE(self->networkMonitor);
}

SvType
QBNetworkLogic_getType(void)
{
    static const struct QBNetworkLogicVTable_ logicVTable = {
        .super_             = {
            .destroy        = QBNetworkLogicDestroy
        },
        .init               = QBNetworkLogicInit_,
        .start              = QBNetworkLogicStart_,
        .stop               = QBNetworkLogicStop_,
        .getRequirements    = QBNetworkLogicGetRequirements_,
    };

    static struct QBObserver_ observable = {
        .observedObjectUpdated = QBNetworkLogicNetworkWatcherStateChanged,
    };

    static const struct QBContextSwitcherListener_t switchMethods = {
        .started = QBNetworkLogicContextSwitchStarted,
        .ended   = QBNetworkLogicContextSwitchEnded
    };

    static SvType type = NULL;

    if (!type) {
        SvTypeCreateVirtual("QBNetworkLogic",
                            sizeof(struct QBNetworkLogic_),
                            SvObject_getType(),
                            sizeof(logicVTable), &logicVTable,
                            &type,
                            QBObserver_getInterface(), &observable,
                            QBContextSwitcherListener_getInterface(), &switchMethods,
                            NULL);
    }

    return type;
}

QBNetworkLogic
QBNetworkLogicCreate(SvApplication res,
                     SvScheduler scheduler,
                     QBInitLogic initLogic,
                     QBApplicationController controller,
                     QBTextRenderer textRenderer,
                     QBNetworkMonitor networkMonitor,
                     QBNetManager *netManager,
                     SvErrorInfo *errorOut)
{
    QBNetworkLogic self = NULL;
    SvErrorInfo error = NULL;

    if (!res || !initLogic || !controller || !textRenderer || !networkMonitor) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                           error, "NULL argument passed [res = %p][initLogic = %p]"
                                           "[controller = %p][textRenderer = %p][networkMonitor = %p]",
                                           res, initLogic, controller, textRenderer, networkMonitor);
        goto fini;
    }

    SvType logicClass = QBLogicFactoryFindImplementationOf(QBLogicFactoryGetInstance(), QBNetworkLogic_getType(), &error);
    if (!logicClass)
        goto fini;

    self = (QBNetworkLogic) SvTypeAllocateInstance(logicClass, &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "can't allocate %s", SvTypeGetName(logicClass));
        goto fini;
    }

    self->res = res;
    self->scheduler = scheduler;
    self->initLogic = SVRETAIN(initLogic);
    self->controller = SVRETAIN(controller);
    self->textRenderer = SVRETAIN(textRenderer);
    self->networkMonitor = SVRETAIN(networkMonitor);

    if (!SvInvokeVirtual(QBNetworkLogic, self, init, netManager, &error)) {
        SVRELEASE(self);
        self = NULL;
        goto fini;
    }

    SvLogNotice("%s(): created instance of %s", __func__, SvTypeGetName(logicClass));

fini:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}
