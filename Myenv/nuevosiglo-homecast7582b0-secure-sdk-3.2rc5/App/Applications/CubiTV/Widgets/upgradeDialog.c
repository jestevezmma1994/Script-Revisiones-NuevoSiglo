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

#include "upgradeDialog.h"
#include <libintl.h>
#include <settings.h>
#include <QBWidgets/QBDialog.h>
#include <main.h>
#include "countdownDialog.h"


SvLocal SvString
QBUpgradeDialogGetTimeoutLabel(int timeout)
{
    const char *msg = ngettext("Automatic update will start in %d second.",
                               "Automatic update will start in %d seconds.",
                               timeout);

    return SvStringCreateWithFormat(msg, timeout, NULL);
}

SvWidget
QBUpgradeDialogCreate(AppGlobals appGlobals,
                      bool forcedUpgrade,
                      bool autoInstall,
                      SvString version,
                      SvString networkName,
                      unsigned int timeout)
{
    bool okFocused = true;
    if (!forcedUpgrade && !autoInstall)
        okFocused = false;

    svSettingsPushComponent("CountdownDialog.settings");
    QBCountdownDialog self = QBCountdownDialogCreate(appGlobals->res,
                                                     appGlobals->controller,
                                                     gettext("Update now"),
                                                     forcedUpgrade ? NULL : gettext("Ask again later"),
                                                     okFocused,
                                                     QBUpgradeDialogGetTimeoutLabel);
    const char *msg;

    QBCountdownDialogSetTitle(self, gettext("Firmware update"));
    char *tmp;
    if (networkName) {
        msg = gettext("New firmware version %s from network %s is available.");
        asprintf(&tmp, msg, SvStringCString(version), SvStringCString(networkName));
    } else {
        msg = gettext("New firmware version %s is available.");
        asprintf(&tmp, msg, SvStringCString(version));
    }
    QBCountdownDialogAddLabel(self, tmp, SVSTRING("main-message"), 0);
    free(tmp);

    QBCountdownDialogAddLabel(self,
                     gettext("Firmware update is required to make the STB work correctly."),
                     SVSTRING("hintA"), 1);

    QBCountdownDialogAddLabel(self,
                     gettext("Update will take about 10 minutes. During this time screen may blink and you won't be able to use your STB."),
                     SVSTRING("hintB"), 2);

    QBCountdownDialogAddLabel(self,
                     gettext("Do not turn off the STB during firmware update."),
                     SVSTRING("hintC"), 3);

    int delay = svSettingsGetInteger("CountdownDialog", "delay", 5);
    QBCountdownDialogSetDelay(self, delay);

    if (forcedUpgrade || autoInstall) {
        QBCountdownDialogSetTimeout(self, timeout);
        QBCountdownDialogAddTimerLabel(self, SVSTRING("timeout-message"), 4);
    }

    svSettingsPopComponent();
    SvWidget w = QBCountdownDialogGetDialog(self);
    return w;
}
