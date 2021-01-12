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

#include "networkProblemDialog.h"

#include <Utils/appType.h>
#include <QBWidgets/QBDialog.h>
#include <settings.h>
#include <main.h>
#include <TranslationMerger.h>
#include <libintl.h>

SvLocal const char *
diagnosisToString(QBNetworkWatcherOverallDiagnosis diagnosis)
{
    switch (diagnosis) {
        case QBNetworkWatcherOverallDiagnosis_disconnected:
            return gettext_noop("no interface connected");
        case QBNetworkWatcherOverallDiagnosis_noLAN:
            return gettext_noop("cannot connect to network");
        case QBNetworkWatcherOverallDiagnosis_noWAN:
            return gettext_noop("network connected, but no Internet access");
        case QBNetworkWatcherOverallDiagnosis_noDNS:
            return gettext_noop("DNS is not setup");
        case QBNetworkWatcherOverallDiagnosis_noVPN:
            return gettext_noop("VPN is not setup");
        case QBNetworkWatcherOverallDiagnosis_ok:
            return gettext_noop("connected");
        default:
            return gettext_noop("unknown connection error");
    }
}

SvLocal SvString
QBNetworkProblemDialogGetTimeoutLabel(int timeout)
{
    const char *msg = ngettext("Channel scanning will start in %d second.",
                               "Channel scanning will start in %d seconds.",
                               timeout);

    return SvStringCreateWithFormat(msg, timeout, NULL);
}

QBCountdownDialog
QBNetworkProblemDialogCreate(SvApplication res, QBApplicationController controller)
{
    svSettingsPushComponent("NetworkProblemDialog.settings");
    QBCountdownDialog self = QBCountdownDialogCreate(res, controller, gettext("Configure Network"), gettext("Ignore"), true, QBNetworkProblemDialogGetTimeoutLabel);
    char *msg;

    QBCountdownDialogSetTitle(self, gettext("Network Problem Detected"));
    msg = gettext("Some services are unavailable due to lack of connectivity. Please check your network setup.");
    QBCountdownDialogAddLabel(self, msg, SVSTRING("main-message"), 0);
    QBCountdownDialogAddLabel(self,
                     gettext("Press 'Configure Network' to see your network settings."),
                     SVSTRING("hintA"), 1);

    int delay = svSettingsGetInteger("CountdownDialog", "delay", 5);
    QBCountdownDialogSetDelay(self, delay);

    svSettingsPopComponent();

    return self;
}

void
QBNetworkProblemDialogSetDiagnosis(QBCountdownDialog self, QBNetworkWatcherOverallDiagnosis diagnosis)
{
    char *desc = NULL;
    asprintf(&desc, "%s: %s", gettext("Error description"), gettext(diagnosisToString(diagnosis)));
    QBCountdownDialogAddLabel(self, desc, SVSTRING("hintA"), 2);
    free(desc);
}


