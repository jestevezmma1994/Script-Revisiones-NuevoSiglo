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

#include "QBRCUPairingWindow.h"
#include <QBWindowContext.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <QBAppKit/QBEventBus.h>
#include <Services/QBRCUPairingService.h>
#include <QBApplicationController.h>
#include <SWL/label.h>
#include <SWL/button.h>
#include <SWL/events.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/widget.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvEnv.h>
#include <libintl.h>
#include <settings.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBRCUPairingWindow"
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 2, moduleName "LogLevel", "");
    #define log_debug(fmt, ...)     do { if (env_log_level() >= 5) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_warning(fmt, ...)   do { if (env_log_level() >= 1) SvLogWarning(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...)     do { if (env_log_level() >= 1) SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#else
    #define log_debug(fmt, ...)
    #define log_warning(fmt, ...)
    #define log_error(fmt, ...)
#endif /* SV_LOG_LEVEL */

struct QBRCUPairingContext_ {
    struct QBWindowContext_t super_;
    int settingsCtx;
    SvWidget windowCaption;
    SvWidget okButton;
};

SvLocal void
QBRCUPairingContextDestroy(void *self_)
{
}

SvLocal void
QBRCUPairingContextUserEventHandler(SvWidget w, SvWidgetId sender, SvUserEvent e)
{
    QBRCUPairingContext self = (QBRCUPairingContext) w->prv;

    if (e->code == SV_EVENT_BUTTON_PUSHED && sender == svWidgetGetId(self->okButton)) {
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBApplicationController controller = (QBApplicationController) QBServiceRegistryGetService(registry, SVSTRING("QBApplicationController"));
        QBApplicationControllerPopContext(controller);
    }
}

SvLocal void
QBRCUPairingContextClean(SvApplication app, void *self_)
{
}

SvLocal void
QBRCUPairingContextShowOKButton(QBRCUPairingContext self)
{
    if (self->okButton) {
        svButtonSetDisabled(self->okButton, false);
        svWidgetSetHidden(self->okButton, false);
        svWidgetSetFocus(self->okButton);
    }
}

SvLocal void
QBRCUPairingContextHideOKButton(QBRCUPairingContext self)
{
    if (self->okButton) {
        svButtonSetDisabled(self->okButton, true);
        svWidgetSetHidden(self->okButton, true);
    }
}
SvLocal void
QBRCUPairingContextSetSuccess(QBRCUPairingContext self)
{
    assert(self);

    svLabelSetText(self->windowCaption, gettext("Successfully paired device with the remote controller."));
    QBRCUPairingContextShowOKButton(self);
}

SvLocal void
QBRCUPairingContextSetFail(QBRCUPairingContext self)
{
    assert(self);

    svLabelSetText(self->windowCaption, gettext("Unable to pair device with a new remote controller."));
    QBRCUPairingContextShowOKButton(self);
}

SvLocal void
QBRCUPairingContextSetRestart(QBRCUPairingContext self)
{
    assert(self);
    svLabelSetText(self->windowCaption, gettext("Press and hold \"OK\" and \"MENU\" buttons until diode on remote blinks."));
    QBRCUPairingContextHideOKButton(self);
}

SvLocal void
QBRCUPairingContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBRCUPairingContext self = (QBRCUPairingContext) self_;

    svSettingsRestoreContext(self->settingsCtx);
    SvWidget window = svSettingsWidgetCreate(app, "QBRCUPairingWindow");
    window->prv = self;
    self->super_.window = window;
    window->clean = QBRCUPairingContextClean;

    SvWidget windowTitle = svLabelNewFromSM(app, "QBRCUPairingWindow.Title");
    svSettingsWidgetAttach(window, windowTitle, "QBRCUPairingWindow.Title", 1);

    self->windowCaption = svLabelNewFromSM(app, "QBRCUPairingWindow.Caption");
    svSettingsWidgetAttach(window, self->windowCaption, "QBRCUPairingWindow.Caption", 1);

    self->okButton = svButtonNew(app, "QBRCUPairingWindow.OKButton");
    svButtonSetCaption(self->okButton, "OK");
    QBRCUPairingContextHideOKButton(self);
    svSettingsWidgetAttach(window, self->okButton, "QBRCUPairingWindow.OKButton", 1);
    svButtonSetNotificationTarget(self->okButton, svWidgetGetId(window));

    svWidgetSetUserEventHandler(window, QBRCUPairingContextUserEventHandler);

    svSettingsPopComponent();

    QBEventBus bus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
    QBEventBusRegisterReceiver(bus, (SvObject) self, QBRCUPairingServiceEvent_getType(), NULL);
}

SvLocal void
QBRCUPairingContextDestroyWindow(QBWindowContext self_)
{
    QBRCUPairingContext self = (QBRCUPairingContext) self_;

    QBEventBus bus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
    QBEventBusUnregisterReceiver(bus, (SvObject) self, QBRCUPairingServiceEvent_getType(), NULL);

    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal void
QBRCUPairingContextHandleEvent(SvObject self_, QBPeerEvent event_, SvObject sender)
{
    QBRCUPairingContext self = (QBRCUPairingContext) self_;
    QBRCUPairingServiceEvent event = (QBRCUPairingServiceEvent) event_;
    QBRCUPairingServiceEventType eventType = QBRCUPairingServiceEventGetType(event);
    switch (eventType) {
        case QBRCUPairingServiceEventType_paired:
            QBRCUPairingContextSetSuccess(self);
            break;
        case QBRCUPairingServiceEventType_pairingFailed:
            QBRCUPairingContextSetFail(self);
            break;
        case QBRCUPairingServiceEventType_pairingStarted:
            QBRCUPairingContextSetRestart(self);
            break;
        default:
            log_error("Unexpected eventType: %d", eventType);
            break;
    }
}

SvType
QBRCUPairingContext_getType(void)
{
    static const struct QBWindowContextVTable_ windowContextVT = {
        .super_        = {
            .destroy   = QBRCUPairingContextDestroy,
        },
        .createWindow  = QBRCUPairingContextCreateWindow,
        .destroyWindow = QBRCUPairingContextDestroyWindow,
    };

    static const struct QBPeerEventReceiver_ receiverVT = {
        .handleEvent = QBRCUPairingContextHandleEvent,
    };

    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBRCUPairingContext", sizeof (struct QBRCUPairingContext_),
                            QBWindowContext_getType(), &type,
                            QBWindowContext_getType(), &windowContextVT,
                            QBPeerEventReceiver_getInterface(), &receiverVT,
                            NULL);
    }

    return type;
}

QBRCUPairingContext
QBRCUPairingContextCreate(void)
{
    QBRCUPairingContext self = (QBRCUPairingContext) SvTypeAllocateInstance(QBRCUPairingContext_getType(), NULL);

    svSettingsPushComponent("QBRCUPairingWindow.settings");
    self->settingsCtx = svSettingsSaveContext();
    svSettingsPopComponent();

    return self;
}
