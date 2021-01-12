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

#include "firstboot.h"

#include <libintl.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/label.h>
#include <SWL/button.h>
#include <SWL/events.h>
#include <main.h>
#include <Windows/channelscanning.h>
#include <Logic/GUILogic.h>

struct QBFirstBootContext_t {
    struct QBWindowContext_t super_;
    AppGlobals appGlobals;
};
typedef struct QBFirstBootContext_t* QBFirstBootContext;

struct QBFirstBootInfo_t {
    SvWidget proceed;
    QBFirstBootContext ctx;
};
typedef struct QBFirstBootInfo_t* QBFirstBootInfo;

SvLocal void QBFirstBootUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e)
{
    QBFirstBootInfo self = w->prv;
    AppGlobals appGlobals = self->ctx->appGlobals;
    if(e->code == SV_EVENT_BUTTON_PUSHED && src == svWidgetGetId(self->proceed)) {
        //Take first boot, put tv, put channel scanning
        QBApplicationControllerTakeContext(appGlobals->controller);
        QBApplicationControllerPutContext(appGlobals->controller, appGlobals->newTV);
        QBWindowContext channelscanning = QBChannelScanningContextCreate(appGlobals, NULL);
        QBChannelScanningContextForce(channelscanning, true);
        QBApplicationControllerPushContext(appGlobals->controller, channelscanning);
        SVRELEASE(channelscanning);
        return;
    }
}

SvLocal void QBFirstBootCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBFirstBootContext self = (QBFirstBootContext) self_;

    svSettingsPushComponent("FirstBoot.settings");

    SvWidget window = QBGUILogicCreateBackgroundWidget(self->appGlobals->guiLogic, "FirstBootWindow", NULL);
    if(svSettingsIsWidgetDefined("CustomerLogo")) {
        SvWidget logo = svSettingsWidgetCreate(app, "CustomerLogo");
        if(logo)
            svSettingsWidgetAttach(window, logo, "CustomerLogo", 1);
    }

    SvWidget label = svLabelNewFromSM(app, "Greeting");
    SvWidget proceed = svButtonNewFromSM(app, "Proceed", gettext("Proceed"), 0, svWidgetGetId(window));

    svSettingsWidgetAttach(window, label, "Greeting", 1);
    svSettingsWidgetAttach(window, proceed, "Proceed", 1);

    svSettingsPopComponent();

    svWidgetSetFocus(proceed);

    QBFirstBootInfo info = calloc(1, sizeof(struct QBFirstBootInfo_t));
    info->proceed = proceed;
    info->ctx = self;

    window->prv = info;
    svWidgetSetUserEventHandler(window, QBFirstBootUserEventHandler);

    self->super_.window = window;
}

SvLocal void QBFirstBootDestroyWindow(QBWindowContext self_)
{
    QBFirstBootContext self = (QBFirstBootContext) self_;
    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal SvType QBFirstBootContext_getType(void)
{
    static SvType type = NULL;
    static const struct QBWindowContextVTable_ contextVTable = {
        .createWindow  = QBFirstBootCreateWindow,
        .destroyWindow = QBFirstBootDestroyWindow
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBFirstBootContext",
                            sizeof(struct QBFirstBootContext_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            NULL);
    }

    return type;
}

QBWindowContext QBFirstBootContextCreate(AppGlobals appGlobals)
{
    QBFirstBootContext ctx = (QBFirstBootContext) SvTypeAllocateInstance(QBFirstBootContext_getType(), NULL);

    ctx->appGlobals = appGlobals;

    return (QBWindowContext) ctx;
}
