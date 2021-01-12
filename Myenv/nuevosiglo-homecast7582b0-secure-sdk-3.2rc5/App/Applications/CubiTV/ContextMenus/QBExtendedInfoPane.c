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

#include "QBExtendedInfoPane.h"
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <QBReinitializable.h>
#include <Widgets/extendedInfo.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBContainerPane.h>
#include <main.h>

struct QBExtendedInfoPane_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBContainerPane container;
    int settingsCtx;
    SvWidget extendedInfo;
};

SvLocal void QBExtendedInfoPane__dtor__(void *ptr)
{
    QBExtendedInfoPane self = ptr;
    SVRELEASE(self->container);
    svWidgetDestroy(self->extendedInfo);
}

SvLocal void QBExtendedInfoPaneContainerOnShow(void *ptr, QBContainerPane pane, SvWidget frame){
    QBExtendedInfoPane self = ptr;
    svSettingsRestoreContext(self->settingsCtx);
    svSettingsWidgetAttach(frame, self->extendedInfo, "ExtendedInfo", 1);
    svSettingsPopComponent();
}

SvLocal void QBExtendedInfoPaneContainerSetActive(void *ptr, QBContainerPane pane, SvWidget frame){
    QBExtendedInfoPane self = ptr;
    QBExtendedInfoSetFocus(self->extendedInfo);
    QBExtendedInfoResetPosition(self->extendedInfo);
}

SvLocal void QBExtendedInfoPaneContainerOnHide(void *ptr, QBContainerPane pane, SvWidget frame){
    QBExtendedInfoPane self = ptr;
    svWidgetDetach(self->extendedInfo);
}

SvLocal void QBExtendedInfoPaneShow(SvGenericObject self_)
{
    QBExtendedInfoPane self = (QBExtendedInfoPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->container, show);
}

SvLocal void QBExtendedInfoPaneHide(SvGenericObject self_, bool immediately)
{
    QBExtendedInfoPane self = (QBExtendedInfoPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->container, hide, immediately);
}

SvLocal void QBExtendedInfoPaneSetActive(SvGenericObject self_)
{
    QBExtendedInfoPane self = (QBExtendedInfoPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->container, setActive);
}

SvLocal bool QBExtendedInfoPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvLocal void QBExtendedInfoPaneReinitialize(SvGenericObject self_, SvArray requestors)
{
    QBExtendedInfoPane self = (QBExtendedInfoPane) self_;

    if (!requestors || SvArrayContainsObject(requestors, (SvGenericObject) SVSTRING("QBParentalControl"))) {
        QBExtendedInfoReinitialize(self->extendedInfo);
    }
}

SvType QBExtendedInfoPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBExtendedInfoPane__dtor__
    };
    static SvType type = NULL;

    static const struct QBReinitializable_t reinitializableMethods = {
        .reinitialize = QBExtendedInfoPaneReinitialize
    };

    static const struct QBContextMenuPane_ methods = {
        .show             = QBExtendedInfoPaneShow,
        .hide             = QBExtendedInfoPaneHide,
        .setActive        = QBExtendedInfoPaneSetActive,
        .handleInputEvent = QBExtendedInfoPaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBExtendedInfoPane",
                            sizeof(struct QBExtendedInfoPane_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextMenuPane_getInterface(), &methods,
                            QBReinitializable_getInterface(), &reinitializableMethods,
                            NULL);
    }

    return type;
}

void QBExtendedInfoPaneInit(QBExtendedInfoPane self, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, int level, SvArray events)
{
    self->settingsCtx = svSettingsSaveContext();
    self->appGlobals = appGlobals;
    self->extendedInfo = QBExtendedInfoNew(appGlobals->res, "ExtendedInfo",  appGlobals);

    self->container = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    static struct QBContainerPaneCallbacks_t moreInfoCallbacks = {
            .onShow = QBExtendedInfoPaneContainerOnShow,
            .onHide = QBExtendedInfoPaneContainerOnHide,
            .setActive = QBExtendedInfoPaneContainerSetActive,
    };
    QBContainerPaneInit(self->container, appGlobals->res, ctxMenu, level, widgetName, &moreInfoCallbacks, self);
    QBExtendedInfoSetEvents(self->extendedInfo, events);
}

QBExtendedInfoPane QBExtendedInfoPaneCreateFromSettings(const char *settings, AppGlobals appGlobals,
        QBContextMenu ctxMenu, SvString widgetName, int level, SvArray events)
{
    svSettingsPushComponent(settings);
    QBExtendedInfoPane self = (QBExtendedInfoPane) SvTypeAllocateInstance(QBExtendedInfoPane_getType(), NULL);
    QBExtendedInfoPaneInit(self, appGlobals, ctxMenu, widgetName, level, events);
    svSettingsPopComponent();

    return self;
}
