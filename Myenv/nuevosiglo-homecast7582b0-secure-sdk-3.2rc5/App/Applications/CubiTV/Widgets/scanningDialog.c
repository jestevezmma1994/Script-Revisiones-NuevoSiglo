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

#include "scanningDialog.h"

#include <libintl.h>
#include <settings.h>
#include <QBWidgets/QBDialog.h>
#include <Utils/appType.h>
#include <main.h>
#include "countdownDialog.h"

typedef struct QBScanningDialog_ *QBScanningDialog;

SvLocal SvString
QBScanningDialogGetTimeoutLabel(int timeout)
{
    const char *msg = ngettext("Channel scanning will start in %d second.",
                               "Channel scanning will start in %d seconds.",
                               timeout);

    return SvStringCreateWithFormat(msg, timeout, NULL);
}

SvWidget
QBScanningDialogCreate(AppGlobals appGlobals, unsigned int timeout)
{
    svSettingsPushComponent("CountdownDialog.settings");
    QBCountdownDialog self = QBCountdownDialogCreate(appGlobals->res,
                                                     appGlobals->controller,
                                                     gettext("Scan now"),
                                                     gettext("Ask again later"),
                                                     true,
                                                     QBScanningDialogGetTimeoutLabel);
    char *msg;

    QBCountdownDialogSetTitle(self, gettext("Channels update"));
    msg = gettext("DVB Channels parameters have changed.");
    QBCountdownDialogAddLabel(self, msg, SVSTRING("main-message"), 0);
    QBCountdownDialogAddLabel(self,
                     gettext("Press OK button to start channel scanning."),
                     SVSTRING("hintA"), 1);

    int position = 2;
    QBAppPVRDiskState diskState = QBAppStateServiceGetPVRDiskState(appGlobals->appState);
    if (diskState == QBPVRDiskState_pvr_present || diskState == QBPVRDiskState_pvr_ts_present) {
        QBCountdownDialogAddLabel(self,
                        gettext("Warning: All current recordings will be stopped!"),
                        SVSTRING("hintB"), position);
        position++;
    }
    if (diskState == QBPVRDiskState_ts_present || diskState == QBPVRDiskState_pvr_ts_present) {
        QBCountdownDialogAddLabel(self,
                        gettext("Warning: Timeshift will be forsaken"),
                        SVSTRING("hintC"), position);
        position++;
    }
    QBCountdownDialogAddLabel(self,
                     gettext("Do not turn off the STB during channel scanning."),
                     SVSTRING("hintD"), position);

    if (timeout > 0) {
        position++;
        QBCountdownDialogSetTimeout(self, timeout);
        QBCountdownDialogAddTimerLabel(self, SVSTRING("timeout-message"), position);
    }

    int delay = svSettingsGetInteger("CountdownDialog", "delay", 5);
    QBCountdownDialogSetDelay(self, delay);

    svSettingsPopComponent();

    SvWidget w = QBCountdownDialogGetDialog(self);
    return w;
}
