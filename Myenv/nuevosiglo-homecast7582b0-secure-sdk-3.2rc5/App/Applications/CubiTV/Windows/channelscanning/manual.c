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

#include "manual.h"
#include <Widgets/QBChannelScanningWidget.h>
#include <Widgets/QBFrequencyInput.h>
#include <Widgets/QBNumericInput.h>
#include <Widgets/QBTitle.h>
#include <QBWidgets/QBComboBox.h>
#include <QBWidgets/QBDialog.h>
#include <Windows/SatelliteSetup.h>
#include <Windows/channelscanning.h>
#include <QBConf.h>
#include <QBDVBScanner.h>
#include <QBDVBScannerTypes.h>
#include <QBTunerTypes.h>
#include "defaultScanParams.h"
#include <sv_tuner.h>
#include <tunerReserver.h>
#include <CUIT/Core/widget.h>
#include <settings.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <fibers/c/fibers.h>
#include <libintl.h>
#include <main.h>
#include <main_decl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANUAL_HINT_DESCRIPTION "manual_hint"

struct QBChannelScanningManualContext_t {
    struct QBChannelScanningContext_t super_;
    AppGlobals appGlobals;
};
typedef struct QBChannelScanningManualContext_t* QBChannelScanningManualContext;

struct QBChannelScanningManual_t {
    QBChannelScanningManualContext ctx;

    SvWidget channelScanningWidget;
    SvWidget logo;
};
typedef struct QBChannelScanningManual_t* QBChannelScanningManual;

SvLocal void QBChannelScanningManualReinitializeWindow(QBWindowContext ctx_, SvArray itemList)
{
    QBChannelScanningManualContext ctx = (QBChannelScanningManualContext) ctx_;
    QBChannelScanningManual self = (QBChannelScanningManual) ctx->super_.super_.window->prv;
    if (!itemList || SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("CustomerLogo"))) {
        SvString logoWidgetName = QBInitLogicGetLogoWidgetName(ctx->appGlobals->initLogic);
        if (logoWidgetName)
            self->logo = QBCustomerLogoReplace(self->logo, ctx->super_.super_.window, logoWidgetName, 1);
    }
}

SvLocal void QBChannelScanningManualSetLogoAndTitle(QBChannelScanningManualContext ctx, SvApplication app, QBChannelScanningManual self)
{
    if (!ctx || !app || !self) {
        SvLogError("%s: NULL arguments passed!", __func__);
        return;
    }

    QBChannelScanningContext super = (QBChannelScanningContext) ctx;
    SvWidget channelScanningWidget = self->channelScanningWidget;

    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(ctx->appGlobals->initLogic);
    if (logoWidgetName)
        self->logo = QBCustomerLogoAttach(channelScanningWidget, logoWidgetName, 1);

    SvWidget title = QBTitleNew(app, "Title", ctx->appGlobals->textRenderer);
    svSettingsWidgetAttach(channelScanningWidget, title, "Title", 1);

    QBTunerType tunerType = QBTunerType_unknown;
    sv_tuner_get_type(0, &tunerType);

    SvString titleStr = SvStringCreate(gettext("Channel Scanning"), NULL);
    QBTitleSetText(title, titleStr, 0);
    SVRELEASE(titleStr);
    if (super->conf) {
        SvString nameStr = NULL;
        if (tunerType == QBTunerType_cab) {
            struct QBTunerProperties tunerProps = { .type = QBTunerType_unknown };
            sv_tuner_get_properties(0, &tunerProps);

            QBTunerStandard standard = QBTunerStandard_unknown;
            if (tunerProps.allowed.standard_cnt) {
                const char* standardString = QBConfigGet("SCANSTANDARD");
                if (standardString) {
                    standard = QBTunerStandardFromString(standardString);
                }
            }
            nameStr = SvStringCreateWithFormat("%s %s", QBTunerStandardToPrettyString(standard), gettext(SvStringCString(super->conf->name)));
        } else {
            nameStr = SvStringCreate(gettext(SvStringCString(super->conf->name)), NULL);
        }
        QBTitleSetText(title, nameStr, 1);
        SVRELEASE(nameStr);
    }
}

SvLocal bool QBChannelScanningManualInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBChannelScanningManual self = w->prv;
    if (QBChannelScanningWidgetHandleInputEvent(self->channelScanningWidget, e))
        return true;

    return svWidgetInputEventHandler(w, e);
}

SvLocal void QBChannelScanningManualCreateWindow(QBWindowContext ctx_, SvApplication app)
{
    QBChannelScanningManualContext ctx = (QBChannelScanningManualContext) ctx_;
    QBChannelScanningManual self = calloc(1, sizeof(struct QBChannelScanningManual_t));

    if (!self) {
        SvLogError("%s: Can not allocate instance of self", __func__);
        return;
    }

    svSettingsPushComponent("ChannelScanningManual.settings");

    QBChannelScanningParams params = NULL;
    QBChannelScanningContext super = (QBChannelScanningContext) ctx_;
    struct QBChannelScanningParamsCreationParams param = {
        .withFailureDialog = false,
        .forceAutostart    = super->forceAutostart,
        .autostart         = super->autostart,
        .forceAutosave     = false,
        .autosave          = false,
        .forcePriority     = false,
        .priority          = false,
        .forceAutomatic    = super->forceAutomatic,
        .automatic         = super->automatic,
        .appGlobals        = NULL
    };

    params = QBChannelScanningParamsCreate(ctx->super_.conf, param);
    SvWidget channelScanningWidget = QBChannelScanningWidgetCreate(ctx->appGlobals->res, SvSchedulerGet(),
                                                                   "ChannelScanning", params, NULL);
    SVRELEASE(params);

    if (!channelScanningWidget) {
        SvLogError("%s: Can not create channelScanningWidget", __func__);
        free(self);
        return;
    }

    if (ctx->appGlobals->hintManager && QBChannelScanningWidgetSideMenuEnabled(channelScanningWidget)) {
        QBHintManagerSetHintsFromString(ctx->appGlobals->hintManager, SVSTRING(MANUAL_HINT_DESCRIPTION));
    }

    self->channelScanningWidget = channelScanningWidget;
    QBChannelScanningManualSetLogoAndTitle(ctx, app, self);

    SvWidget window = svWidgetCreate(app, ctx->appGlobals->res->width, ctx->appGlobals->res->height);
    svWidgetSetFocusable(window, false);
    window->prv = self;
    svWidgetAttach(window, channelScanningWidget, 0, 0, 0);
    ctx_->window = window;
    svWidgetSetInputEventHandler(window, QBChannelScanningManualInputEventHandler);

    svSettingsPopComponent();
}

SvLocal void QBChannelScanningManualDestroyWindow(QBWindowContext ctx_)
{
    QBChannelScanningManualContext ctx = (QBChannelScanningManualContext) ctx_;
    QBWindowContext wctx = (QBWindowContext) ctx;
    SvWidget window = wctx->window;
    if (ctx->appGlobals->hintManager) {
        QBHintManagerSetHintsFromString(ctx->appGlobals->hintManager, NULL);
    }
    svWidgetDestroy(window);
    wctx->window = NULL;
}

SvLocal SvType QBChannelScanningManualContext_getType(void)
{
    static SvType type = NULL;
    static const struct QBWindowContextVTable_ contextVTable = {
        .reinitializeWindow = QBChannelScanningManualReinitializeWindow,
        .createWindow       = QBChannelScanningManualCreateWindow,
        .destroyWindow      = QBChannelScanningManualDestroyWindow
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBChannelScanningManualContext",
                            sizeof(struct QBChannelScanningManualContext_t),
                            QBChannelScanningContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            NULL);
    }

    return type;
}

QBWindowContext QBChannelScanningManualContextCreate(AppGlobals appGlobals)
{
    QBChannelScanningManualContext ctx = (QBChannelScanningManualContext) SvTypeAllocateInstance(QBChannelScanningManualContext_getType(), NULL);

    QBChannelScanningContextInit((QBChannelScanningContext) ctx, appGlobals);

    ctx->appGlobals = appGlobals;

    return (QBWindowContext) ctx;
}
