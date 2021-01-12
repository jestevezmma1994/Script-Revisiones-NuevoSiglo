/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connecwion with a service bureau,
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

#include "HomecastFactory.h"
#include <libintl.h>
#include <settings.h>
#include <SWL/button.h>
#include <SWL/events.h>
#include <SWL/label.h>
#include <QBCAS.h>
#include <QBSmartcard2Interface.h>
#include <QBICSmartcardInfo.h>
#include <QBPlatformHAL/QBPlatformOTP.h>
#include <QBPlatformHAL/QBPlatformEvent.h>
#include <init.h>
#include <main.h>
#include <Windows/channelscanning.h>
#include <Services/core/QBChannelScanningConfManager.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <Logic/GUILogic.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <SvCore/SvLog.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/widget.h>
#include <SWL/events.h>
#include <libintl.h>

struct QBHomecastFactoryContext_ {
    struct QBWindowContext_t super_;
    AppGlobals appGlobals;
};
typedef struct QBHomecastFactoryContext_* QBHomecastFactoryContext;

struct QBHomecastFactory_ {
    struct SvObject_ super_;
    SvWidget resetButton;
    SvWidget scanButton;
    SvWidget conaxModuleVersionLabel;
    SvWidget middlewareStatusLabel;
    SvWidget chipidStatusLabel;
    SvWidget otpStatusLabel;
    QBCASCmd cmd;
    QBCASCmd otpCmd;;

    char passCode[4];

    QBHomecastFactoryContext ctx;
};
typedef struct QBHomecastFactory_* QBHomecastFactory;

SvLocal void QBHomecastFactoryUpdateMiddlewareStatus(QBHomecastFactory self)
{
    CubiwareMWCustomerInfoStatus status = CubiwareMWCustomerInfoGetStatus(self->ctx->appGlobals->customerInfoMonitor);

    SvString text = NULL;
    int errorCode = 0;
    //Copy from QBConfigurationMenu
    switch (status) {
        case CubiwareMWCustomerInfoStatus_ok:
            text = SvStringCreate(gettext("Ok"), NULL);
            break;
        case CubiwareMWCustomerInfoStatus_error:
            errorCode = CubiwareMWCustomerInfoGetErrorCode(self->ctx->appGlobals->customerInfoMonitor);
            text = SvStringCreateWithFormat("%s #%x", gettext("Error"), errorCode);
            break;
        case CubiwareMWCustomerInfoStatus_unknown:
        default:
            text = SvStringCreate(gettext("Unknown"), NULL);
            break;
    }

    svLabelSetText(self->middlewareStatusLabel, SvStringCString(text));

    SVTESTRELEASE(text);
}

SvLocal void QBHomecastFactoryCustomerInfoMonitorStatusChanged(SvObject self_)
{
    QBHomecastFactory self = (QBHomecastFactory) self_;
    QBHomecastFactoryUpdateMiddlewareStatus(self);
}

SvLocal void QBHomecastFactoryCustomerInfoMonitorInfoChanged(SvGenericObject self_, SvString customerId, SvHashTable customerInfo)
{
}

SvLocal SvType QBHomecastFactory_getType(void)
{
    static SvType type = NULL;
    static const struct CubiwareMWCustomerInfoListener_ customerInfoMethods = {
        .customerInfoChanged = QBHomecastFactoryCustomerInfoMonitorInfoChanged,
        .statusChanged       = QBHomecastFactoryCustomerInfoMonitorStatusChanged
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBHomecastFactory",
                            sizeof(struct QBHomecastFactory_),
                            SvObject_getType(), &type,
                            CubiwareMWCustomerInfoListener_getInterface(), &customerInfoMethods,
                            NULL);
    }

    return type;
}

SvLocal void factorySmartcardOTPCallback(void *self_, const char *msg)
{
    QBHomecastFactory self = self_;

    svLabelSetText(self->otpStatusLabel, msg);
}

SvLocal void QBSmartcard2InterfaceOTPCallback(void *self_, const QBICSmartcardCmd *cmd, int status, SvObject info)
{
    QBHomecastFactory self = self_;
    self->otpCmd = NULL;

    char buf[30];
    snprintf(buf, sizeof(buf), "%i", status);
    factorySmartcardOTPCallback(self_, buf);
}

SvLocal const char *translateOTPstatus(QBPlatformEventOtpChangedStatus status)
{
    assert(status < QBPlatformEventOtpChangedStatus_cnt);

    const char *translateTable[] = {
        "locking OTP succeeded",        // QBPlatformEventOtpChangedStatus_success
        "locking OTP failed",           // QBPlatformEventOtpChangedStatus_failure
        "OTP is locked",                // QBPlatformEventOtpChangedStatus_locked
        "OTP is unlocked",              // QBPlatformEventOtpChangedStatus_unlocked
        "failed to read OTP status"     // QBPlatformEventOtpChangedStatus_error
    };

    return translateTable[status];
}

SvLocal void QBPlatformOTPCallback(void *prv, QBPlatformEvent event, long long int argA, SvObject argB)
{
    assert(event == QBPlatformEvent_otpChanged);

    factorySmartcardOTPCallback(prv, translateOTPstatus(argA));
}

SvLocal bool QBHomecastFactoryInputEventHandler(SvWidget window, SvInputEvent e)
{
    QBHomecastFactory self = window->prv;
    if (e->ch < '0' || e->ch > '9')
        goto fin;

    for (int i = 0; i < 3; i++)
        self->passCode[i] = self->passCode[i + 1];

    self->passCode[3] = e->ch;

    static const char passCode[] = { '1', '4', '3', '6' };

    if (!memcmp(self->passCode, passCode, sizeof(passCode))) {
        int ret;

        QBICSmartcardDriverVersion *ver = SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getDriverVersion);
        if (ver->currentConaxVersion == QBICSmartcardOption_conaxVersion__5_0_a) {
            // TODO: 'setOTP' should be moved to another interface (i.e. Homecast specific or QBCAS)
            ret = SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), options, QBSmartcard2InterfaceOTPCallback, self,
                                    &self->otpCmd, QBICSmartcardOption_setOTP);
        } else {
            if (QBPlatformOTPIsSupported()) {
                ret = QBPlatformOTPSetSecurityFuses(true);
            } else {
                SvLogError("Fusing OTP not supported on this platform");
                ret = -1;
            }
        }
        SVRELEASE(ver);

        if (ret) {
            char buf[64];
            snprintf(buf, sizeof(buf), "locking OTP failed: %i", ret);
            svLabelSetText(self->otpStatusLabel, buf);
        } else {
            svLabelSetText(self->otpStatusLabel, "In progress");
        }
    }

fin:
    return svWidgetInputEventHandler(window, e);
}

SvLocal void QBHomecastFactoryUserEventHandler(SvWidget window, SvWidgetId src, SvUserEvent e)
{
    if (e->code != SV_EVENT_BUTTON_PUSHED)
        return;

    QBHomecastFactory self = window->prv;

    if (svWidgetGetId(self->resetButton) == src) {
        FILE *f = fopen("/tmp/factoryReset", "w");
        if (f) {
            fclose(f);

            SvLogError("QBHomecastFactory: call QBInitStopApplication");
            QBInitStopApplication(self->ctx->appGlobals, true, "factory reset");
        }
        return;
    } else if (svWidgetGetId(self->scanButton) == src) {
        QBWindowContext ctx = QBChannelScanningContextCreate(self->ctx->appGlobals, SVSTRING("manual"));
        QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                      SVSTRING("QBChannelScanningConfManager"));
        QBChannelScanningConf conf = QBChannelScanningConfManagerGetConf(channelScanningConf, SVSTRING("factoryScan"));
        if (conf)
            QBChannelScanningContextLoadConf(ctx, conf);

        QBApplicationControllerPushContext(self->ctx->appGlobals->controller, ctx);
        SVRELEASE(ctx);
        return;
    }
}

SvLocal void QBHomecastFactoryClean(SvApplication app, void *self_)
{
    QBHomecastFactory self = self_;
    if (self->cmd)
        SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->cmd);
    if (self->otpCmd)
        SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->otpCmd);
    if (QBPlatformOTPIsSupported()) {
        QBPlatformRemoveEventCallback(QBPlatformOTPCallback, self);
    }
    SVRELEASE(self);
}

SvLocal void factorySmartcardCallback(void* self_, const QBCASCmd cmd, int status, QBCASInfo info)
{
    QBHomecastFactory self = self_;
    self->cmd = NULL;
    if (!info)
        return;
    svLabelSetText(self->conaxModuleVersionLabel, info->sw_ver ? SvStringCString(info->sw_ver) : "QB unknown");
    svLabelSetText(self->chipidStatusLabel, info->chip_id ? SvStringCString(info->chip_id) : "QB unknown");
}

SvLocal void QBHomecastFactoryContextReinitializeWindow(QBWindowContext self_, SvArray requests)
{
}

SvLocal void QBHomecastFactoryContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBHomecastFactoryContext self = (QBHomecastFactoryContext) self_;
    QBHomecastFactory factory = (QBHomecastFactory) SvTypeAllocateInstance(QBHomecastFactory_getType(), NULL);

    factory->ctx = self;

    //QBTITLE

    svSettingsPushComponent("HomecastFactory.settings");
    SvWidget window = QBGUILogicCreateBackgroundWidget(self->appGlobals->guiLogic, "Factory.bg", NULL);
    self->super_.window = window;

    SvWidget resetLabel = svLabelNewFromSM(app, "Factory.Label.Reset");
    svSettingsWidgetAttach(window, resetLabel, "Factory.Label.Reset", 0);
    SvWidget resetButton = svButtonNewFromSM(app, "Factory.Button.Reset", NULL, 1, svWidgetGetId(window));
    svSettingsWidgetAttach(window, resetButton, "Factory.Button.Reset", 0);

    SvWidget scanLabel = svLabelNewFromSM(app, "Factory.Label.Scan");
    svSettingsWidgetAttach(window, scanLabel, "Factory.Label.Scan", 0);
    SvWidget scanButton = svButtonNewFromSM(app, "Factory.Button.Scan", NULL, 2, svWidgetGetId(window));
    svSettingsWidgetAttach(window, scanButton, "Factory.Button.Scan", 0);

    SvWidget conaxModuleLabel = svLabelNewFromSM(app, "Factory.Label.ConaxModule");
    svSettingsWidgetAttach(window, conaxModuleLabel, "Factory.Label.ConaxModule", 0);
    SvWidget conaxModuleVersionLabel = svLabelNewFromSM(app, "Factory.Label.ConaxModuleVersion");
    svSettingsWidgetAttach(window, conaxModuleVersionLabel, "Factory.Label.ConaxModuleVersion", 0);

    SvWidget middlewareLabel = svLabelNewFromSM(app, "Factory.Label.Middleware");
    svSettingsWidgetAttach(window, middlewareLabel, "Factory.Label.Middleware", 0);
    SvWidget middlewareStatusLabel = svLabelNewFromSM(app, "Factory.Label.MiddlewareStatus");
    svSettingsWidgetAttach(window, middlewareStatusLabel, "Factory.Label.MiddlewareStatus", 0);

    SvWidget chipidLabel = svLabelNewFromSM(app, "Factory.Label.Chipid");
    svSettingsWidgetAttach(window, chipidLabel, "Factory.Label.Chipid", 0);
    SvWidget chipidStatusLabel = svLabelNewFromSM(app, "Factory.Label.ChipidStatus");
    svSettingsWidgetAttach(window, chipidStatusLabel, "Factory.Label.ChipidStatus", 0);

    SvWidget macLabel = svLabelNewFromSM(app, "Factory.Label.MAC");
    svSettingsWidgetAttach(window, macLabel, "Factory.Label.MAC", 0);
    SvWidget macStatusLabel = svLabelNewFromSM(app, "Factory.Label.MACStatus");
    svSettingsWidgetAttach(window, macStatusLabel, "Factory.Label.MACStatus", 0);

    SvWidget otpLabel = svLabelNewFromSM(app, "Factory.Label.OTP");
    svSettingsWidgetAttach(window, otpLabel, "Factory.Label.OTP", 0);
    SvWidget otpStatusLabel = svLabelNewFromSM(app, "Factory.Label.OTPStatus");
    svSettingsWidgetAttach(window, otpStatusLabel, "Factory.Label.OTPStatus", 0);

    svSettingsPopComponent();

    factory->resetButton = resetButton;
    factory->scanButton = scanButton;
    factory->conaxModuleVersionLabel = conaxModuleVersionLabel;
    factory->middlewareStatusLabel = middlewareStatusLabel;
    factory->chipidStatusLabel = chipidStatusLabel;
    factory->otpStatusLabel = otpStatusLabel;

    window->prv = factory;
    svWidgetSetUserEventHandler(window, QBHomecastFactoryUserEventHandler);
    svWidgetSetInputEventHandler(window, QBHomecastFactoryInputEventHandler);
    window->clean = QBHomecastFactoryClean;

    QBHomecastFactoryUpdateMiddlewareStatus(factory);
    SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, &factorySmartcardCallback, factory, &factory->cmd);
    CubiwareMWCustomerInfoAddListener(self->appGlobals->customerInfoMonitor, (SvGenericObject) factory);
    CubiwareMWCustomerInfoRefresh(self->appGlobals->customerInfoMonitor);

    char mac[20] = "unknown";
    QBNetManagerGetHWAddress(SVSTRING("eth0"), mac, sizeof(mac), NULL);
    svLabelSetText(macStatusLabel, mac);

    if (QBPlatformOTPIsSupported()) {
        QBPlatformAddEventCallback(QBPlatformOTPCallback, factory);
        QBPlatformOTPIsSecured(true);
    } else {
        QBICSmartcardDriverVersion *ver = SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getDriverVersion);
        if (ver->currentConaxVersion != QBICSmartcardOption_conaxVersion__5_0_a) {
            svLabelSetText(otpStatusLabel, "not supported");
        }
        SVRELEASE(ver);
    }

    QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
    if (channelScanningConf) {
        svWidgetSetFocus(factory->scanButton);
    } else {
        svButtonSetDisabled(factory->scanButton, true);
    }
}

SvLocal void QBHomecastFactoryContextDestroyWindow(QBWindowContext self_)
{
    QBHomecastFactoryContext self = (QBHomecastFactoryContext) self_;
    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal SvType QBHomecastFactoryContext_getType(void)
{
    static const struct QBWindowContextVTable_ contextVTable = {
        .reinitializeWindow = QBHomecastFactoryContextReinitializeWindow,
        .createWindow       = QBHomecastFactoryContextCreateWindow,
        .destroyWindow      = QBHomecastFactoryContextDestroyWindow
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBHomecastFactoryContext",
                            sizeof(struct QBHomecastFactoryContext_),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            NULL);
    }

    return type;
}

QBWindowContext QBHomecastFactoryContextCreate(AppGlobals appGlobals)
{
    // TODO: Not HomecastFactoryContext is using QBICSmartcard interface, but it looks like it should not be connected to any specific CAS
    if (!SvObjectIsImplementationOf(QBCASGetInstance(), QBSmartcard2Interface_getInterface())) {
        SvLogError("%s :: QBHomecastFactoryContextCreate could be only created for Conax CAS type", __func__);
        return NULL;
    }

    QBHomecastFactoryContext self = (QBHomecastFactoryContext) SvTypeAllocateInstance(QBHomecastFactoryContext_getType(), NULL);
    self->appGlobals = appGlobals;

    return (QBWindowContext) self;
}
