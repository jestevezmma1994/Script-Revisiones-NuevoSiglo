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

#include "QBListPane.h"
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <QBDataModel3/QBListModel.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/label.h>
#include <Widgets/SideMenu.h>
#include <XMB2/XMBVerticalMenu.h>

struct QBListPane_t {
    struct SvObject_ super_;
    SvWidget sideMenu;
    int level;

    SvGenericObject source;
    SvGenericObject userItemConstructor;


    SvWidget window;
    SvWidget menu;
    SvWidget title;

    SvString widgetName;
    SvString titleName;

    QBContextMenu contextMenu;

    bool shown;

    int wantedWidth;
    int savedWidth;

    QBListPaneCallbacks callbacks;
    void *callbackData;
};

SvLocal void QBListPaneShow(SvGenericObject self_)
{
    QBListPane self = (QBListPane) self_;
    if(self->shown)
        return;
    self->shown = true;
    SvWidget frame = SideMenuGetFrame(self->sideMenu, self->level);

    self->savedWidth = SideMenuGetFrameWidth(self->sideMenu, self->level);
    SideMenuSetFrameWidth(self->sideMenu, self->level, self->wantedWidth, false);
    QBContextMenuSetDepth(self->contextMenu, self->level, false);

    self->window->width = frame->width;
    self->window->height = frame->height;
    svWidgetAttach(frame, self->window, 0, 0, 1);
}

SvLocal void QBListPaneHide(SvGenericObject self_, bool immediately)
{
    QBListPane self = (QBListPane) self_;
    if(!self->shown)
        return;
    self->shown = false;
    svWidgetDetach(self->window);
    SideMenuSetFrameWidth(self->sideMenu, self->level, self->savedWidth, immediately);
    QBContextMenuSetDepth(self->contextMenu, self->level - 1, immediately);
}

SvLocal void QBListPaneSetActive(SvGenericObject self_)
{
    QBListPane self = (QBListPane) self_;
    svWidgetSetFocus(self->menu);
}

SvLocal bool QBListPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvLocal void QBListPane__dtor__(void *ptr)
{
    QBListPane self = ptr;
    svWidgetDetach(self->menu);

    if(self->title) {
        svWidgetDetach(self->title);
        svWidgetDestroy(self->title);
    }
    svWidgetDestroy(self->window);
    svWidgetDestroy(self->menu);
    SVTESTRELEASE(self->source);
    SVRELEASE(self->widgetName);
    SVRELEASE(self->userItemConstructor);
    SVRELEASE(self->titleName);
}


SvType QBListPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBListPane__dtor__
    };
    static SvType type = NULL;
    static const struct QBContextMenuPane_ methods = {
        .show             = QBListPaneShow,
        .hide             = QBListPaneHide,
        .setActive        = QBListPaneSetActive,
        .handleInputEvent = QBListPaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBListPane",
                            sizeof(struct QBListPane_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextMenuPane_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvLocal void QBListPaneWindowClean(SvApplication app, void *ptr)
{

}

SvLocal bool QBListPaneWindowInputEventHandler(SvWidget window, SvInputEvent e)
{
    QBListPane self = window->prv;
    return QBContextMenuHandleInput(self->contextMenu, (SvObject) self, e);
}

QBListPane QBListPaneCreateFromSettings(const char *settings, SvApplication app, SvGenericObject source, SvGenericObject constructor, QBListPaneCallbacks callback, void *callbackData, QBContextMenu ctxMenu, unsigned level, SvString widgetName)
{
    svSettingsPushComponent(settings);
    QBListPane self = (QBListPane) SvTypeAllocateInstance(QBListPane_getType(), NULL);
    QBListPaneInit(self, app, source, constructor, callback, callbackData, ctxMenu, level, widgetName);
    svSettingsPopComponent();

    return self;
}

void QBListPaneSetSource(QBListPane self, SvGenericObject source)
{
    SVTESTRELEASE(self->source);
    self->source = SVTESTRETAIN(source);
    if(self->source) {
        XMBVerticalMenuConnectToDataSource(self->menu, (SvGenericObject) self->source, (SvGenericObject) self->userItemConstructor, NULL);
        QBListPaneSetActive((SvGenericObject)self);
    }
}

SvLocal void
QBListPaneOnChangePosition(void *w_, const XMBMenuEvent data)
{
    QBListPane self = ((SvWidget) w_)->prv;

    if(!data->clicked)
        return;

    if(data->type == XMBMenuEventType_reordered) {
        if(self->callbacks->reordered)
            self->callbacks->reordered(self->callbackData, data->prevPosition, data->position);
        return;
    }
    SvGenericObject lastSelectedObj = SvInvokeInterface(QBListModel, self->source, getObject, data->position);
    if(data->type == XMBMenuEventType_selected)
        if(self->callbacks->selected)
            self->callbacks->selected(self->callbackData, self, lastSelectedObj, data->position);
}

void QBListPaneInit(QBListPane self, SvApplication app, SvGenericObject source, SvGenericObject constructor, QBListPaneCallbacks callbacks, void *callbackData, QBContextMenu ctxMenu, unsigned level, SvString widgetName)
{
    self->titleName = SvStringCreateWithFormat("%s.Title", SvStringCString(widgetName));
    self->widgetName = SVRETAIN(widgetName);
    self->sideMenu = ctxMenu->sideMenu;
    self->source = SVTESTRETAIN(source);
    self->window = svWidgetCreateBitmap(app, 0, 0, NULL);
    self->menu = XMBVerticalMenuNew(app, SvStringCString(widgetName), NULL);
    self->level = level;
    self->contextMenu = ctxMenu;
    self->callbacks = callbacks;
    self->callbackData = callbackData;
    int sideMenuWidth = SideMenuGetFrameWidth(self->sideMenu, self->level);
    self->wantedWidth = svSettingsGetInteger(SvStringCString(widgetName), "width", sideMenuWidth);
    self->userItemConstructor = SVRETAIN(constructor);
    if(svSettingsIsWidgetDefined(SvStringCString(self->titleName))) {
        self->title = svLabelNewFromSM(app, SvStringCString(self->titleName));
    }


    self->window->prv = self;
    self->window->clean = QBListPaneWindowClean;
    svWidgetSetInputEventHandler(self->window, QBListPaneWindowInputEventHandler);

    svWidgetSetFocusable(self->menu, true);

    XMBVerticalMenuSetNotificationTarget(self->menu, self->window, QBListPaneOnChangePosition);

    if(self->source)
        XMBVerticalMenuConnectToDataSource(self->menu, (SvGenericObject) self->source, (SvGenericObject) self->userItemConstructor, NULL);

    svSettingsWidgetAttach(self->window, self->menu, SvStringCString(self->widgetName), 0);
    if(self->title)
        svSettingsWidgetAttach(self->window, self->title, SvStringCString(self->titleName), 0);
}

SvWidget QBListPaneGetMenu(QBListPane self)
{
    return self->menu;
}

int QBListPaneGetLevel(QBListPane self)
{
    return self->level;
}

