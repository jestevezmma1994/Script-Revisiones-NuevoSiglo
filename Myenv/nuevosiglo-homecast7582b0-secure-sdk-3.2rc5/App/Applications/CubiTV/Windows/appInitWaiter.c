/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#include "appInitWaiter.h"

#include <libintl.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/label.h>
#include <Widgets/customerLogo.h>
#include <QBWidgets/QBDialog.h>
#include <main.h>
#include <Logic/GUILogic.h>

struct QBAppInitWaiterContext_t {
    struct QBWindowContext_t super_;
    AppGlobals appGlobals;
};
typedef struct QBAppInitWaiterContext_t* QBAppInitWaiterContext;

struct QBAppInitWaiterInfo_t {
    SvWidget window;
    SvWidget logo;
    SvWidget dialog;
    QBAppInitWaiterContext ctx;
};
typedef struct QBAppInitWaiterInfo_t* QBAppInitWaiterInfo;

SvLocal void QBAppInitWaiterSetupDialog(QBAppInitWaiterInfo self)
{
    svSettingsPushComponent("AppInitWaiter.settings");
    AppGlobals appGlobals = self->ctx->appGlobals;
    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(self->window),
    };
    SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    QBDialogSetTitle(dialog, gettext(svSettingsGetString("Dialog", "title")));


    QBDialogSetTitle(dialog, gettext("Please wait, loading..."));

    self->dialog = dialog;

    svSettingsPopComponent();

    QBDialogRun(self->dialog, NULL, NULL);
}

SvLocal bool QBAppInitWaiterInputEventHandler(SvWidget w, SvInputEvent e)
{
    return true;
}

SvLocal void QBAppInitWaiterContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBAppInitWaiterContext self = (QBAppInitWaiterContext) self_;

    svSettingsPushComponent("AppInitWaiter.settings");

    SvWidget window = svSettingsWidgetCreate(app, "AppInitWaiterWindow");

    SvWidget windowTitle = svLabelNewFromSM(app, "AppInitWaiterWindow.Title");
    svSettingsWidgetAttach(window, windowTitle, "AppInitWaiterWindow.Title", 1);

    svSettingsPopComponent();

    QBAppInitWaiterInfo info = calloc(1, sizeof(struct QBAppInitWaiterInfo_t));
    info->ctx = self;
    info->window = window;

    window->prv = info;
    svWidgetSetInputEventHandler(window, QBAppInitWaiterInputEventHandler);

    self->super_.window = window;
    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(self->appGlobals->initLogic);
    if (logoWidgetName)
        info->logo = QBCustomerLogoAttach(self->super_.window, logoWidgetName, 1);

    QBAppInitWaiterSetupDialog(info);
}

SvLocal void QBAppInitWaiterContextDestroyWindow(QBWindowContext self_)
{
    QBAppInitWaiterContext self = (QBAppInitWaiterContext) self_;

    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal SvType QBAppInitWaiterContext_getType(void)
{
    static const struct QBWindowContextVTable_ contextVTable = {
        .createWindow  = QBAppInitWaiterContextCreateWindow,
        .destroyWindow = QBAppInitWaiterContextDestroyWindow
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBAppInitWaiterContext",
                            sizeof(struct QBAppInitWaiterContext_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            NULL);
    }

    return type;
}

QBWindowContext QBAppInitWaiterContextCreate(AppGlobals appGlobals)
{
    QBAppInitWaiterContext ctx = (QBAppInitWaiterContext) SvTypeAllocateInstance(QBAppInitWaiterContext_getType(), NULL);
    ctx->appGlobals = appGlobals;
    return (QBWindowContext) ctx;
}
