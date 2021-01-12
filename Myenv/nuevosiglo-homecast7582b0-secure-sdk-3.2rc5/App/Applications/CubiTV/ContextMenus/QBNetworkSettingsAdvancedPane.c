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

#include "QBNetworkSettingsAdvancedPane.h"
#include <Widgets/QBNetworkSettingsAdvancedWidget.h>
#include <QBWidgets/QBComboBox.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <ContextMenus/QBContainerPane.h>
#include <SWL/input.h>
#include <SWL/button.h>
#include <CUIT/Core/event.h>
#include <QBNetworkMonitor/QBNetworkInterface.h>
#include <libintl.h>
#include <main.h>
#include <regex.h>
#include <settings.h>


struct QBNetworkSettingsAdvancedPane_ {
    struct SvObject_ super_;

    QBContainerPane containerPane;
    int settingsCtx;

    QBContextMenu ctxMenu;
    SvWidget networkSettings;
};

SvLocal void
QBNetworkSettingsAdvancedPaneOnNetworkSettingsFinished(void *target)
{
    QBNetworkSettingsAdvancedPane self = target;
    QBContextMenuHide(self->ctxMenu, false);
}

SvLocal void
QBNetworkSettingsAdvancedPaneContainerOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBNetworkSettingsAdvancedPane self = self_;
    if (self->networkSettings) {
        svWidgetAttach(frame, self->networkSettings, self->networkSettings->off_x, self->networkSettings->off_y, 0);
    }
}

SvLocal void
QBNetworkSettingsAdvancedPaneContainerSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBNetworkSettingsAdvancedPane self = self_;
    if (self->networkSettings) {
        svWidgetSetFocus(self->networkSettings);
    }
}

SvLocal void
QBNetworkSettingsAdvancedPaneContainerOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBNetworkSettingsAdvancedPane self = self_;
    svWidgetDetach(self->networkSettings);
}

SvLocal void
QBNetworkSettingsAdvancedPaneInit(QBNetworkSettingsAdvancedPane self,
                                  SvApplication res,
                                  QBTextRenderer textRenderer,
                                  QBNetworkMonitor networkMonitor,
                                  QBNetworkInterface currentInterface,
                                  QBContextMenu ctxMenu,
                                  int level)
{
    self->ctxMenu = SVRETAIN(ctxMenu);

    self->containerPane = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    static struct QBContainerPaneCallbacks_t moreInfoCallbacks = {
        .onShow    = QBNetworkSettingsAdvancedPaneContainerOnShow,
        .onHide    = QBNetworkSettingsAdvancedPaneContainerOnHide,
        .setActive = QBNetworkSettingsAdvancedPaneContainerSetActive,
    };

    svSettingsPushComponent("NetworkSettingsAdvanced.settings");
    self->settingsCtx = svSettingsSaveContext();
    self->networkSettings = QBNetworkSettingsAdvancedWidgetCreate(res, textRenderer, networkMonitor, "Advanced", currentInterface, NULL);
    QBNetworkSettingsAdvancedWidgetSetOnFinishCallback(self->networkSettings, self, QBNetworkSettingsAdvancedPaneOnNetworkSettingsFinished);
    QBContainerPaneInit(self->containerPane, res, ctxMenu, level, SVSTRING("AdvancedExtendedInfo"), &moreInfoCallbacks, self);
    svSettingsPopComponent();
}

SvLocal void
QBNetworkSettingsAdvancedPaneShow(SvObject self_)
{
    QBNetworkSettingsAdvancedPane self = (QBNetworkSettingsAdvancedPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->containerPane, show);
}

SvLocal void
QBNetworkSettingsAdvancedPaneHide(SvObject self_, bool immediately)
{
    QBNetworkSettingsAdvancedPane self = (QBNetworkSettingsAdvancedPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->containerPane, hide, immediately);
}

SvLocal void
QBNetworkSettingsAdvancedPaneSetActive(SvObject self_)
{
    QBNetworkSettingsAdvancedPane self = (QBNetworkSettingsAdvancedPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->containerPane, setActive);
    QBNetworkSettingsAdvancedWidgetSetFocus(self->networkSettings);
}

SvLocal bool
QBNetworkSettingsAdvancedPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvLocal void
QBNetworkSettingsAdvancedPaneDestroy(void *ptr)
{
    QBNetworkSettingsAdvancedPane self = ptr;

    if (self->networkSettings)
        svWidgetDestroy(self->networkSettings);

    SVRELEASE(self->containerPane);
    SVRELEASE(self->ctxMenu);
}

SvLocal SvType
QBNetworkSettingsAdvancedPane_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBNetworkSettingsAdvancedPaneDestroy
    };

    static const struct QBContextMenuPane_ methods = {
        .show             = QBNetworkSettingsAdvancedPaneShow,
        .hide             = QBNetworkSettingsAdvancedPaneHide,
        .setActive        = QBNetworkSettingsAdvancedPaneSetActive,
        .handleInputEvent = QBNetworkSettingsAdvancedPaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNetworkSettingsAdvancedPane",
                            sizeof(struct QBNetworkSettingsAdvancedPane_),
                            SvObject_getType(), &type,
                            QBContextMenuPane_getInterface(), &methods,
                            SvObject_getType(), &vtable,
                            NULL);
    }
    return type;
}

QBNetworkSettingsAdvancedPane
QBNetworkSettingsAdvancedPaneCreate(SvApplication res,
                                    QBTextRenderer textRenderer,
                                    QBNetworkMonitor networkMonitor,
                                    QBNetworkInterface interface,
                                    QBContextMenu ctxMenu,
                                    int level)
{
    QBNetworkSettingsAdvancedPane self = (QBNetworkSettingsAdvancedPane)
                                         SvTypeAllocateInstance(QBNetworkSettingsAdvancedPane_getType(), NULL);
    QBNetworkSettingsAdvancedPaneInit(self, res, textRenderer, networkMonitor, interface, ctxMenu, level);
    return self;
}
