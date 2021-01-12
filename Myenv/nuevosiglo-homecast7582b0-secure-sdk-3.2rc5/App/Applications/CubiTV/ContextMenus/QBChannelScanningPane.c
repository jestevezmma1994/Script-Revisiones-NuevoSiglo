/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBChannelScanningPane.h"
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBContainerPane.h>
#include <ContextMenus/QBBasicPane.h>
#include <QBReinitializable.h>
#include <Widgets/extendedInfo.h>
#include <settings.h>
#include <QBTunerSettings.h>
#include <QBConfig.h>
#include <libintl.h>
#include <main.h>

#define SPECTRAL_INVERSION_CONFIG_ID "TUNER.SPECTRAL_INVERSION_MODE"

struct QBChannelScanningPane_t {
    struct SvObject_ super_;
    QBContextMenu contextMenu;
    QBBasicPane advancedOptionsPane;                 //first level pane
    QBBasicPane spectralInversionOptionsPane;        //second level pane
    int settingsCtx;
};

SvLocal void QBChannelScanningPane__dtor__(void *ptr)
{
    QBChannelScanningPane self = ptr;
    SVRELEASE(self->advancedOptionsPane);
    SVTESTRELEASE(self->spectralInversionOptionsPane);
}

SvLocal void QBChannelScanningPaneShow(SvObject self_)
{
    QBChannelScanningPane self = (QBChannelScanningPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->advancedOptionsPane, show);
}

SvLocal void QBChannelScanningPaneHide(SvObject self_, bool immediately)
{
    QBChannelScanningPane self = (QBChannelScanningPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->advancedOptionsPane, hide, immediately);
}

SvLocal void QBChannelScanningPaneSetActive(SvObject self_)
{
    QBChannelScanningPane self = (QBChannelScanningPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->advancedOptionsPane, setActive);
}

SvLocal bool QBChannelScanningPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvType QBChannelScanningPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBChannelScanningPane__dtor__
    };
    static SvType type = NULL;
    static const struct QBContextMenuPane_ methods = {
        .show = QBChannelScanningPaneShow,
        .hide = QBChannelScanningPaneHide,
        .setActive = QBChannelScanningPaneSetActive,
        .handleInputEvent = QBChannelScanningPaneHandleInputEvent
    } ;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBChannelScanningPane",
                            sizeof(struct QBChannelScanningPane_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextMenuPane_getInterface(), &methods,
                            NULL);
    }
    return type;
}

SvLocal void QBChannelScanningPaneSpecInvModeChosen(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBConfigSet(SPECTRAL_INVERSION_CONFIG_ID,SvStringCString(id));
    QBConfigSave();
    QBChannelScanningPane self = self_;
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal SvString QBChannelScanningPaneCreateSubcaption(void)
{
    QBTunerSettingsSpectralInversionMode mode = QBTunerSettingsGetSpectralInversionMode();
    if (mode == QBTunerSettingsSpectralInversionMode_normal) {
        return SvStringCreate(gettext("Normal"), NULL);
    } else if (mode == QBTunerSettingsSpectralInversionMode_inverted) {
        return SvStringCreate(gettext("Inverted"), NULL);
    } else if (mode == QBTunerSettingsSpectralInversionMode_auto) {
        return SvStringCreate(gettext("Auto"), NULL);
    }
    SvLogError("%s: unknown spectral inversion mode", __func__);
    return SvStringCreate("Unknown", NULL);
}

void QBChannelScanningPaneInit(QBChannelScanningPane self, struct QBChannelScanningPaneParams* params, QBContextMenu ctxMenu, SvString itemNamesFilename)
{
    self->contextMenu = ctxMenu;
    self->settingsCtx = svSettingsSaveContext();
    self->advancedOptionsPane = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(self->advancedOptionsPane, params->app, params->scheduler, params->textRenderer, ctxMenu, 1, SVSTRING("BasicPane"));
    QBBasicPaneLoadOptionsFromFile(self->advancedOptionsPane, itemNamesFilename);
    SvString title = SvStringCreate(gettext("Advanced settings"), NULL);
    QBBasicPaneSetTitle(self->advancedOptionsPane, title);
    SVRELEASE(title);

    self->spectralInversionOptionsPane = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(self->spectralInversionOptionsPane, params->app, params->scheduler, params->textRenderer, self->contextMenu, 2, SVSTRING("BasicPane"));
    QBBasicPaneLoadOptionsFromFile(self->spectralInversionOptionsPane, itemNamesFilename);

    QBBasicPaneAddOption(self->spectralInversionOptionsPane, SVSTRING("normal"), NULL, QBChannelScanningPaneSpecInvModeChosen, self);
    QBBasicPaneAddOption(self->spectralInversionOptionsPane, SVSTRING("inverted"), NULL, QBChannelScanningPaneSpecInvModeChosen, self);
    QBBasicPaneAddOption(self->spectralInversionOptionsPane, SVSTRING("auto"), NULL, QBChannelScanningPaneSpecInvModeChosen, self);

    SvString subcaption = QBChannelScanningPaneCreateSubcaption();
    QBBasicPaneAddOptionWithSubpaneAndSubcaption(self->advancedOptionsPane, SVSTRING("SpectralInversionMode"), NULL, subcaption, (SvObject) self->spectralInversionOptionsPane);
    SVRELEASE(subcaption);
}

QBChannelScanningPane QBChannelScanningPaneCreateFromSettings(const char *settings, const char *itemNamesFilename, struct QBChannelScanningPaneParams* params, QBContextMenu ctxMenu)
{
    svSettingsPushComponent(settings);
    QBChannelScanningPane self = (QBChannelScanningPane) SvTypeAllocateInstance(QBChannelScanningPane_getType(), NULL);
    SvString filename = SvStringCreate(itemNamesFilename, NULL);
    QBChannelScanningPaneInit(self, params, ctxMenu, filename);
    SVRELEASE(filename);
    svSettingsPopComponent();
    return self;
}
