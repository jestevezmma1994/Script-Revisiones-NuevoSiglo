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

#include <settings.h>
#include <libintl.h>
#include <main.h>
#include <Widgets/countdownDialog.h>
#include "Widgets/ipTvInactivityDialog.h"


struct BoldIpTvInactivityDialog_ {
    QBCountdownDialog super_;
};
typedef struct BoldIpTvInactivityDialog_ *BoldIpTvInactivityDialog;

SvLocal SvString
BoldIpTvInactivityDialogGetTimeoutLabel(int timeout)
{
    const char *msg = ngettext("El canal se cambiará en %d segundo.",
                               "El canal se cambiará en %d segundos.",
                               timeout);

    return SvStringCreateWithFormat(msg, timeout, NULL);
}

SvWidget
BoldIpTvInactivityDialogCreate(AppGlobals appGlobals, unsigned int timeout)
{
    BoldIpTvInactivityDialog self = calloc(1, sizeof(struct BoldIpTvInactivityDialog_));

    svSettingsPushComponent("CountdownDialog.settings");
    self->super_ = QBCountdownDialogCreate(appGlobals->res, appGlobals->controller, gettext("Ok"), NULL, true, BoldIpTvInactivityDialogGetTimeoutLabel);
    
    QBCountdownDialogSetTitle(self->super_, gettext(" "));
    
    QBCountdownDialogSetDelay(self->super_, 0);
    
    QBCountdownDialogAddLabel(self->super_,
                     gettext("Si aún estás viendo este canal, por favor presioná OK"),
                     SVSTRING("hintA"), 1);
    

    QBCountdownDialogSetTimeout(self->super_, timeout);
    QBCountdownDialogAddTimerLabel(self->super_, SVSTRING("timeout-message"), 2);

    svSettingsPopComponent();
    return QBCountdownDialogGetDialog(self->super_);
}
