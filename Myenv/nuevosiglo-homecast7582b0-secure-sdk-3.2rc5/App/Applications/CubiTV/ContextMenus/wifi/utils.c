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

#include "utils.h"
#include <Widgets/QBSimpleDialog.h>
#include <QBWidgets/QBDialog.h>
#include <settings.h>
#include <libintl.h>
#include <main.h>

void
QBWifiPasswordTypedErrorDialogShow(SvApplication app, QBApplicationController controller, int settingsCtx)
{
    char *title = gettext("Invalid password");
    SvString message = SvStringCreateWithFormat(gettext("Password must contain between %d and %d characters"), MIN_WPA_KEY_LENGTH, MAX_WPA_KEY_LENGTH);

    svSettingsRestoreContext(settingsCtx);
    QBDialogParameters params = {
        .app        = app,
        .controller = controller,
        .widgetName = "Dialog",
        .ownerId    = 0,
    };
    SvWidget dialog = QBSimpleDialogCreate(&params, title, SvStringCString(message));
    QBDialogRun(dialog, NULL, NULL);
    svSettingsPopComponent();
    SVRELEASE(message);
}

bool QBWiFiPasswordTyped(SvApplication res,
                         QBApplicationController controller,
                         QBNetworkMonitor networkMonitor,
                         QBWiFiInterface wifiInterface,
                         QBWiFiNetwork wifiNetwork,
                         SvString password,
                         bool manual,
                         int settingsCtx)
{
    SvString id = QBNetworkInterfaceGetID((QBNetworkInterface) wifiInterface);
    QBWiFiNetworkParamsCheck res_ = QBNetworkMonitorConnectWiFiNetwork(networkMonitor,
                                                                      id,
                                                                      QBWiFiNetworkGetSSID(wifiNetwork),
                                                                      password,
                                                                      QBWiFiNetworkGetSecurityMode(wifiNetwork),
                                                                      manual);

    if (res_ == QBWiFiNetworkParamsCheck_invalidWPAPassword) {
        QBWifiPasswordTypedErrorDialogShow(res, controller, settingsCtx);
        return false;
    } else if (res_ == QBWiFiNetworkParamsCheck_emptyPassword)
        return false;

    return true;
}

SvObject
QBWiFiInputSideMenuCreate(SvScheduler scheduler,
                          QBContextMenu ctxMenu,
                          SvString widgetName,
                          const char *map,
                          int level,
                          QBOSKPaneKeyTyped callback,
                          void *callbackData)
{
    QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
    svSettingsPushComponent("WiFiOSK.settings");
    SvErrorInfo error = NULL;
    QBOSKPaneInitLayout(oskPane, scheduler, ctxMenu, map, level, widgetName, callback, callbackData, &error);
    if (!error) {
        svSettingsPopComponent();
    } else {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }

    return (SvObject) oskPane;
}
