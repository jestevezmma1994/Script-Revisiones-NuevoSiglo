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

#include "QBContainerPane.h"
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <settings.h>
#include <Widgets/SideMenu.h>


struct QBContainerPane_t {
    struct SvObject_ super_;
    QBContextMenu contextMenu;
    SvWidget sideMenu;

    bool shown;

    int level;

    QBContainerPaneCallbacks callbacks;
    void *callbackData;

    SvWidget window;
    int frameWidth;
    unsigned int settingsContext;
    SvString widgetName;
};


int
QBContainerPaneGetLevel(QBContainerPane self)
{
    return self->level;
}

SvLocal void QBContainerPaneWindowFakeClean(SvApplication app, void *ptr)
{
}

SvLocal void QBContainerPane__dtor__(void *ptr)
{
    QBContainerPane self = ptr;
    if(self->shown) {
        if(self->callbacks && self->callbacks->onHide)
            self->callbacks->onHide(self->callbackData, self, self->window);
    }

    if(self->callbacks && self->callbacks->onDestroy)
        self->callbacks->onDestroy(self->callbackData, self);

    SVTESTRELEASE(self->widgetName);

    svWidgetDestroy(self->window);
}

SvLocal bool QBContainerPaneInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBContainerPane self = w->prv;
    return QBContextMenuHandleInput(self->contextMenu, (SvGenericObject) self, e);
}

SvLocal void QBContainerPaneShow(SvGenericObject self_)
{
    QBContainerPane self = (QBContainerPane) self_;
    if(self->shown)
        return;
    self->shown = true;

    SvWidget frame = SideMenuGetFrame(self->sideMenu, self->level);

    svSettingsRestoreContext(self->settingsContext);
    int newWidth = -1;
    if(svSettingsIsWidgetDefined(SvStringCString(self->widgetName)))
        newWidth = svSettingsGetInteger(SvStringCString(self->widgetName), "boxWidth", -1);
    self->frameWidth = SideMenuGetFrameWidth(self->sideMenu, self->level);
    if(newWidth > 0)
        SideMenuSetFrameWidth(self->sideMenu, self->level, newWidth, false);
    svSettingsPopComponent();

    svWidgetAttach(frame, self->window, 0, 0, 0);
    self->window->height = frame->height;
    self->window->width = frame->width;

    if(self->callbacks && self->callbacks->onShow)
        self->callbacks->onShow(self->callbackData, self, self->window);

    QBContextMenuSetDepth(self->contextMenu, self->level, false);
}

SvLocal void QBContainerPaneHide(SvGenericObject self_, bool immediately)
{
    QBContainerPane self = (QBContainerPane) self_;
    if(!self->shown)
        return;
    self->shown = false;

    SideMenuSetFrameWidth(self->sideMenu, self->level, self->frameWidth, immediately);

    if(self->callbacks && self->callbacks->onHide)
        self->callbacks->onHide(self->callbackData, self, self->window);

    svWidgetDetach(self->window);

    QBContextMenuSetDepth(self->contextMenu, self->level - 1, immediately);
}

SvLocal void QBContainerPaneSetActive(SvGenericObject self_)
{
    QBContainerPane self = (QBContainerPane) self_;

    if(self->callbacks && self->callbacks->setActive)
        self->callbacks->setActive(self->callbackData, self, self->window);
}

SvLocal bool QBContainerPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvType QBContainerPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBContainerPane__dtor__
    };
    static SvType type = NULL;
    static const struct QBContextMenuPane_ methods = {
        .show             = QBContainerPaneShow,
        .hide             = QBContainerPaneHide,
        .setActive        = QBContainerPaneSetActive,
        .handleInputEvent = QBContainerPaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBContainerPane",
                            sizeof(struct QBContainerPane_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextMenuPane_getInterface(), &methods,
                            NULL);
    }

    return type;
}

QBContainerPane QBContainerPaneCreateFromSettings(const char *settings, SvApplication app, QBContextMenu ctxMenu, int level,  SvString widgetName, QBContainerPaneCallbacks callbacks, void *callbackData)
{
    svSettingsPushComponent(settings);
    QBContainerPane self = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    QBContainerPaneInit(self, app, ctxMenu, level, widgetName, callbacks, callbackData);
    svSettingsPopComponent();

    return self;
}

void QBContainerPaneInit(QBContainerPane self, SvApplication app, QBContextMenu ctxMenu, int level, SvString widgetName, QBContainerPaneCallbacks callbacks, void *callbackData)
{
    self->contextMenu = ctxMenu;
    self->level = level;
    self->callbacks = callbacks;
    self->callbackData = callbackData;
    self->sideMenu = ctxMenu->sideMenu;
    self->settingsContext = svSettingsSaveContext();
    self->widgetName = SVRETAIN(widgetName);
    self->window = svWidgetCreateBitmap(app, 0, 0, NULL);
    self->window->clean = QBContainerPaneWindowFakeClean;
    self->window->prv = self;
    svWidgetSetInputEventHandler(self->window, QBContainerPaneInputEventHandler);
}
