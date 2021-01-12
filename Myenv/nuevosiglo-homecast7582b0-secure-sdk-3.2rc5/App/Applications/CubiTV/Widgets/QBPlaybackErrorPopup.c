/*****************************************************************************
** Cubiware K.K. Software License Version 1.1
**
** Copyright (C) 2015 Cubiware K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware K.K.
**
** Any User wishing to make use of this Software must contact Cubiware K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include "QBPlaybackErrorPopup.h"
#include <QBWidgets/QBDialog.h>
#include <settings.h>

SvWidget QBPlaybackErrorPopupCreate(QBApplicationController controller, QBWindowContext windowCtx, const char* title, const char* explanation)
{
    SvApplication app = windowCtx->window->app;
    svSettingsPushComponent("NoSignalDialog.settings");
    QBDialogParameters params = {
        .app        = app,
        .controller = controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(windowCtx->window)
    };
    SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeNonFocusable);
    dialog->off_x = (windowCtx->window->width - dialog->width) / 2;
    dialog->off_y = (windowCtx->window->height - dialog->height) / 2;
    svSettingsPopComponent();
    QBDialogSetTitle(dialog, title);
    if (explanation && *explanation) {
        QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
        QBDialogAddLabel(dialog, SVSTRING("content"), explanation, SVSTRING("text"), 1);
    }
    return dialog;
}
