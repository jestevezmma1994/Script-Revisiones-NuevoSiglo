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

#include "QBDiagnosticsWindow.h"

#include <libintl.h>
#include <SvCore/SvCommonDefs.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <QBInput/QBInputCodes.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <settings.h>
#include <QBApplicationController.h>
#include <QBWindowContext.h>
#include <Configurations/QBDiagnosticsWindowConfManager.h>
#include <Logic/InitLogic.h>
#include <main.h>
#include <main_decl.h>
#include <Widgets/customerLogo.h>
#include <Widgets/QBTitle.h>
#include <Widgets/QBDiagnosticsWidget.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Logic/GUILogic.h>

typedef struct QBDiagnosticsWindowContext_t {
    struct QBWindowContext_t super_;
    AppGlobals appGlobals;
    QBDiagnosticsWindowConf conf;

} *QBDiagnosticsWindowContext;

typedef struct QBDiagnosticsWindow_t {
    QBDiagnosticsWindowContext ctx;

    SvWidget logo;
} *QBDiagnosticsWindow;

SvLocal bool
QBDiagnosticsWindowInputEventHandler(SvWidget w, SvInputEvent e)
{
    return false;
}

SvLocal void
QBDiagnosticsWindowCreateWindow(QBWindowContext ctx_, SvApplication app)
{
    QBDiagnosticsWindowContext ctx = (QBDiagnosticsWindowContext) ctx_;

    svSettingsPushComponent("Diagnostics.settings");

    SvWidget window = QBGUILogicCreateBackgroundWidget(ctx->appGlobals->guiLogic, "Diagnostics", NULL);
    if (!window) {
        SvLogError("Window widget creation failed");
        return;
    }
    SvWidget title = QBTitleNew(app, "Title", ctx->appGlobals->textRenderer);

    svSettingsWidgetAttach(window, title, "Title", 0);

    SvWidget infoLabel = QBAsyncLabelNew(app, "Diagnostics.info", ctx->appGlobals->textRenderer);
    const char* infoName = svWidgetGetName(infoLabel);
    if (infoLabel && infoName)
        svSettingsWidgetAttach(window, infoLabel, infoName, 1);

    QBDiagnosticsWindow self = calloc(1, sizeof(struct QBDiagnosticsWindow_t));
    self->ctx = ctx;

    svWidgetSetInputEventHandler(window, QBDiagnosticsWindowInputEventHandler);
    window->prv = self;

    ctx->super_.window = window;

    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(ctx->appGlobals->initLogic);
    if ( logoWidgetName ) {
        self->logo = QBCustomerLogoAttach(ctx->super_.window, logoWidgetName, 1);
    }

    SvWidget diagnosticsWidget = QBDiagnosticsWidgetCreate(ctx->appGlobals, "Diagnostics.DiagnosticsWidget", ctx->conf, NULL);
    if (!diagnosticsWidget) {
        SvLogError("Diagnostics Widget creation failed");
        return;
    }

    svSettingsWidgetAttach(window, diagnosticsWidget, "Diagnostics.DiagnosticsWidget", 0);

    svSettingsPopComponent();

    SvString titleStr = SvStringCreateWithFormat("%s > %s", gettext("Diagnostics"), gettext(SvStringCString(QBDiagnosticsWindowConfGetCaption(ctx->conf))));
    QBTitleSetText(title, titleStr, 0);
    SVRELEASE(titleStr);
}

SvLocal void
QBDiagnosticsWindowDestroyWindow(QBWindowContext ctx_)
{
    QBDiagnosticsWindowContext ctx = (QBDiagnosticsWindowContext) ctx_;

    svWidgetDestroy(ctx->super_.window);
    ctx->super_.window = NULL;
}

SvLocal void
QBDiagnosticsWindowReinitializeWindow(QBWindowContext ctx_, SvArray itemList)
{
    QBDiagnosticsWindowContext ctx = (QBDiagnosticsWindowContext) ctx_;
    QBDiagnosticsWindow self = (QBDiagnosticsWindow) ctx->super_.window->prv;
    if ( !itemList || SvArrayContainsObject(itemList, (SvObject) SVSTRING("CustomerLogo")) ) {
        SvString logoWidgetName = QBInitLogicGetLogoWidgetName(ctx->appGlobals->initLogic);
        if (logoWidgetName) {
            self->logo = QBCustomerLogoReplace(self->logo, ctx->super_.window, logoWidgetName, 1);
        }
    }
}

SvLocal void QBDiagnosticsWindowContext__dtor__(void *ptr)
{
    QBDiagnosticsWindowContext ctx = (QBDiagnosticsWindowContext) ptr;

    SVTESTRELEASE(ctx->conf);
    ctx->conf = NULL;

    if (ctx->super_.window)
        svWidgetDestroy(ctx->super_.window);
}

SvType
QBDiagnosticsWindowContext_getType(void)
{
    static SvType type = NULL;
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_             = {
            .destroy        = QBDiagnosticsWindowContext__dtor__
        },
        .reinitializeWindow = QBDiagnosticsWindowReinitializeWindow,
        .createWindow       = QBDiagnosticsWindowCreateWindow,
        .destroyWindow      = QBDiagnosticsWindowDestroyWindow
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDiagnosticsWindowContext",
                            sizeof(struct QBDiagnosticsWindowContext_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            NULL);
    }
    return type;
}

QBWindowContext
QBDiagnosticsWindowContextCreate(AppGlobals appGlobals, SvString confID)
{
    QBDiagnosticsWindowContext self = (QBDiagnosticsWindowContext)
        SvTypeAllocateInstance(QBDiagnosticsWindowContext_getType(), NULL);

    if (!(appGlobals->diagnosticsLogic && appGlobals->diagnosticsManager)) {
        SvLogWarning("QBDiagnostics service is not enabled!");
    }

    self->appGlobals = appGlobals;
    self->conf = SVTESTRETAIN(QBDiagnosticsManagerGetConf(appGlobals->diagnosticsManager, confID));
    if (!self->conf) {
        SvLogError("%s(): QBDiagnosticsManagerGetConf failed", __func__);
        SVRELEASE(self);
        return NULL;
    }

    return (QBWindowContext) self;
}
