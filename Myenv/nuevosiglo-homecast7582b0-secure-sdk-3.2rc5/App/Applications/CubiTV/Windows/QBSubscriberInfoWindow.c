/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBSubscriberInfoWindow.h"

#include <Widgets/customerLogo.h>
#include <QBSmartCardMonitor.h>
#include <QBConfig.h>
#include <QBNetManager.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <main.h>
#include <SWL/label.h>
#include <SWL/icon.h>
#include <settings.h>
#include <SvFoundation/SvType.h>
#include <libintl.h>

typedef struct QBSubscriberInfoContext_ {
    struct QBWindowContext_t super_;
    AppGlobals appGlobals;
    int settingsCtx;
} *QBSubscriberInfoContext;

SvLocal void
QBSuscriberInfoSetBoxID(QBSubscriberInfoContext self, SvApplication app, SvWidget windowItem)
{
    SvString chipID = NULL;
    if (self->appGlobals->smartCardMonitor) {
        chipID = QBSmartCardMonitorGetFullChipId(self->appGlobals->smartCardMonitor);
    }

    if (chipID) {
        QBAsyncLabelSetCText(windowItem, SvStringCString(chipID));
    } else {
        QBAsyncLabelSetCText(windowItem, gettext("N/A"));
    }
}

SvLocal void
QBSubscriberInfoSetSerialNumber(QBSubscriberInfoContext self, SvApplication app, SvWidget windowItem)
{
    SvString simID = NULL;
    if (self->appGlobals->smartCardMonitor) {
        simID = QBSmartCardMonitorGetSmartCardIdRaw(self->appGlobals->smartCardMonitor);
    }

    if (simID) {
        QBAsyncLabelSetCText(windowItem, SvStringCString(simID));
    } else {
        QBAsyncLabelSetCText(windowItem, gettext("N/A"));
    }
}

SvLocal void
QBSubscriberInfoSetMacAddress(QBSubscriberInfoContext self, SvApplication app, SvWidget windowItem)
{
    const char *medium = QBConfigGet("MEDIUM");
    SvString activeIface = NULL;

    if (medium && !strcmp(medium, "WIFI")) {
        activeIface = SvStringCreate("ra0", 0);
    } else {
        activeIface = SvStringCreate("eth0", 0);
    }

    SvObject macAddress = NULL;
    if ((macAddress = QBNetManagerGetAttribute(self->appGlobals->net_manager, activeIface, QBNetAttr_hwAddr, NULL))) {
        if (SvObjectIsInstanceOf(macAddress, SvString_getType())) {
            QBAsyncLabelSetCText(windowItem, SvStringCString((SvString) macAddress));
            SVRELEASE(activeIface);
            return;
        }
    }
    QBAsyncLabelSetCText(windowItem, gettext("N/A"));
    SVRELEASE(activeIface);
}

SvLocal void
QBSubscriberInfoContextClean(SvApplication app, void *self_)
{
}

SvLocal void
QBSubscriberInfoContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBSubscriberInfoContext self = (QBSubscriberInfoContext) self_;

    svSettingsRestoreContext(self->settingsCtx);

    SvWidget window = svSettingsWidgetCreate(app, "QBSubscriberInfoWindow");
    window->prv = self;
    self->super_.window = window;
    window->clean = QBSubscriberInfoContextClean;

    SvWidget windowCaptionBig = svLabelNewFromSM(app, "QBSubscriberInfoWindow.CaptionBig");
    svSettingsWidgetAttach(window, windowCaptionBig, "QBSubscriberInfoWindow.CaptionBig", 1);

    SvWidget windowGoBackBar = QBAsyncLabelNew(app, "QBSubscriberInfoWindow.windowInfo", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, windowGoBackBar, "QBSubscriberInfoWindow.windowInfo", 1);

    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(self->appGlobals->initLogic);
    if (logoWidgetName) {
        QBCustomerLogoAttach(self->super_.window, logoWidgetName, 1);
    }

    SvWidget windowHelpLogo = svIconNew(app, "QBSubscriberInfoWindow.logoHelp");
    if (windowHelpLogo) {
        svSettingsWidgetAttach(window, windowHelpLogo, "QBSubscriberInfoWindow.logoHelp", 1);
    }

    SvWidget windowsBoxID = QBAsyncLabelNew(app, "QBSubscriberInfoWindow.BoxID", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, windowsBoxID, "QBSubscriberInfoWindow.BoxID", 1);
    SvWidget windowsBoxIDVal = QBAsyncLabelNew(app, "QBSubscriberInfoWindow.BoxIDVal", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, windowsBoxIDVal, "QBSubscriberInfoWindow.BoxIDVal", 1);
    QBSuscriberInfoSetBoxID(self, app, windowsBoxIDVal);

    SvWidget windowsSerialNumber = QBAsyncLabelNew(app, "QBSubscriberInfoWindow.SerialNumber", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, windowsSerialNumber, "QBSubscriberInfoWindow.SerialNumber", 1);
    SvWidget windowsSerialNumberVal = QBAsyncLabelNew(app, "QBSubscriberInfoWindow.SerialNumberVal", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, windowsSerialNumberVal, "QBSubscriberInfoWindow.SerialNumberVal", 1);
    QBSubscriberInfoSetSerialNumber(self, app, windowsSerialNumberVal);

    SvWidget windowsMACAddress = QBAsyncLabelNew(app, "QBSubscriberInfoWindow.MACAddress", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, windowsMACAddress, "QBSubscriberInfoWindow.MACAddress", 1);
    SvWidget windowsMACAddressVal = QBAsyncLabelNew(app, "QBSubscriberInfoWindow.MACAddressVal", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, windowsMACAddressVal, "QBSubscriberInfoWindow.MACAddressVal", 1);
    QBSubscriberInfoSetMacAddress(self, app, windowsMACAddressVal);

    svSettingsPopComponent();
}

SvLocal void
QBSubscriberInfoContextDestroyWindow(QBWindowContext self_)
{
    QBSubscriberInfoContext self = (QBSubscriberInfoContext) self_;

    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal SvType
QBSubscriberInfoContext_getType(void)
{
    static const struct QBWindowContextVTable_ windowContextVT = {
        .createWindow  = QBSubscriberInfoContextCreateWindow,
        .destroyWindow = QBSubscriberInfoContextDestroyWindow,
    };

    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBSubscriberInfoContext", sizeof (struct QBSubscriberInfoContext_),
                            QBWindowContext_getType(), &type,
                            QBWindowContext_getType(), &windowContextVT,
                            NULL);
    }

    return type;
}

QBWindowContext
QBSubscriberInfoContextCreate(AppGlobals appGlobals)
{
    QBSubscriberInfoContext self = (QBSubscriberInfoContext) SvTypeAllocateInstance(QBSubscriberInfoContext_getType(), NULL);

    self->appGlobals = appGlobals;

    svSettingsPushComponent("SubscriberInfo.settings");
    self->settingsCtx = svSettingsSaveContext();
    svSettingsPopComponent();

    return (QBWindowContext) self;
}
