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

#include "QBHelpInfoWindow.h"
#include <Widgets/customerLogo.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <SWL/label.h>
#include <SWL/icon.h>
#include <settings.h>
#include <SvFoundation/SvType.h>
#include <libintl.h>

typedef struct QBHelpInfoContext_ {
    struct QBWindowContext_t super_;
    QBInitLogic initLogic;
    QBTextRenderer textRenderer;
    int settingsCtx;
} *QBHelpInfoContext;

SvLocal void
QBHelpInfoContextClean(SvApplication app, void *self_)
{
}

SvLocal void
QBHelpInfoContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBHelpInfoContext self = (QBHelpInfoContext) self_;

    svSettingsRestoreContext(self->settingsCtx);

    SvWidget window = svSettingsWidgetCreate(app, "QBHelpInfoWindow");
    window->prv = self;
    self->super_.window = window;
    window->clean = QBHelpInfoContextClean;

    SvWidget windowCaptionBig = svLabelNewFromSM(app, "QBHelpInfoWindow.CaptionBig");
    svSettingsWidgetAttach(window, windowCaptionBig, "QBHelpInfoWindow.CaptionBig", 1);

    SvWidget windowCaptionSmall = QBAsyncLabelNew(app, "QBHelpInfoWindow.CaptionSmall", self->textRenderer);
    svSettingsWidgetAttach(window, windowCaptionSmall, "QBHelpInfoWindow.CaptionSmall", 1);

    SvWidget windowGoBackBar = QBAsyncLabelNew(app, "QBHelpInfoWindow.windowInfo", self->textRenderer);
    svSettingsWidgetAttach(window, windowGoBackBar, "QBHelpInfoWindow.windowInfo", 1);

    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(self->initLogic);
    if (logoWidgetName) {
        QBCustomerLogoAttach(self->super_.window, logoWidgetName, 1);
    }

    SvWidget windowHelpLogo = svIconNew(app, "QBHelpInfoWindow.logoHelp");
    if (windowHelpLogo) {
        svSettingsWidgetAttach(window, windowHelpLogo, "QBHelpInfoWindow.logoHelp", 1);
    }

    svSettingsPopComponent();
}

SvLocal void
QBHelpInfoContextDestroyWindow(QBWindowContext self_)
{
    QBHelpInfoContext self = (QBHelpInfoContext) self_;

    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal void
QBHelpInfoContextDestroy(void *self_)
{
    QBHelpInfoContext self = (QBHelpInfoContext) self_;

    SVRELEASE(self->initLogic);
    SVRELEASE(self->textRenderer);
}

SvLocal SvType
QBHelpInfoContext_getType(void)
{
    static const struct QBWindowContextVTable_ windowContextVT = {
        .super_        = {
            .destroy   = QBHelpInfoContextDestroy,
        },
        .createWindow  = QBHelpInfoContextCreateWindow,
        .destroyWindow = QBHelpInfoContextDestroyWindow,
    };

    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBHelpInfoContext", sizeof (struct QBHelpInfoContext_),
                            QBWindowContext_getType(), &type,
                            QBWindowContext_getType(), &windowContextVT,
                            NULL);
    }

    return type;
}

QBWindowContext
QBHelpInfoContextCreate(QBInitLogic initLogic,
                        QBTextRenderer textRenderer)
{
    QBHelpInfoContext self = (QBHelpInfoContext) SvTypeAllocateInstance(QBHelpInfoContext_getType(), NULL);

    self->initLogic = SVRETAIN(initLogic);
    self->textRenderer = SVRETAIN(textRenderer);

    svSettingsPushComponent("HelpInfo.settings");
    self->settingsCtx = svSettingsSaveContext();
    svSettingsPopComponent();

    return (QBWindowContext) self;
}
