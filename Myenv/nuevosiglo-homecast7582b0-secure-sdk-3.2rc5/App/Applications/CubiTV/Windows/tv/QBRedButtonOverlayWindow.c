/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBRedButtonOverlayWindow.h"
#include <main.h>
#include <Widgets/QBRedButtonOverlay.h>
#include <QBLocalWindowManager.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SvFoundation/SvType.h>


struct QBRedButtonOverlayWindow_ {
    struct QBLocalWindow_t super_;
    AppGlobals appGlobals;

    SvWidget redButtonOverlay;
    bool visible;
};

SvLocal void QBRedButtonOverlayWindowDestroy(void *self_)
{
    QBRedButtonOverlayWindow self = self_;
    svWidgetDestroy(self->super_.window);
}

SvLocal SvType QBRedButtonOverlayWindow_getType(void)
{
    static const struct QBWindowVTable_ vtable = {
        .super_      = {
            .destroy = QBRedButtonOverlayWindowDestroy
        }
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRedButtonOverlayWindow",
                            sizeof(struct QBRedButtonOverlayWindow_),
                            QBLocalWindow_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            NULL);
    }

    return type;
}

QBRedButtonOverlayWindow QBRedButtonOverlayWindowNew(AppGlobals appGlobals, SvHashTable data)
{
    if (!appGlobals) {
        SvLogError("%s(): got NULL appGlobals", __func__);
        return NULL;
    }

    SvApplication app = appGlobals->res;
    QBRedButtonOverlayWindow self = (QBRedButtonOverlayWindow) SvTypeAllocateInstance(QBRedButtonOverlayWindow_getType(), NULL);

    self->appGlobals = appGlobals;

    svSettingsPushComponent("RedButtonOverlay.settings");
    self->redButtonOverlay = QBRedButtonOverlayNew(app, "RedButtonOverlay", data);
    svSettingsPopComponent();

    QBLocalWindowInit((QBLocalWindow) self, self->redButtonOverlay, QBLocalWindowTypeFocusable);

    return self;
}

void QBRedButtonOverlayWindowReplaceContent(QBRedButtonOverlayWindow self, SvHashTable data)
{
    if (!self) {
        SvLogError("%s(): got NULL self", __func__);
        return;
    }

    QBRedButtonOverlayReplaceContent(self->redButtonOverlay, data);
}


void QBRedButtonOverlayWindowShow(QBRedButtonOverlayWindow self)
{
    if (!self) {
        SvLogError("%s(): got NULL self", __func__);
        return;
    }

    QBRedButtonOverlayReset(self->redButtonOverlay);
    QBApplicationControllerAddLocalWindow(self->appGlobals->controller, (QBLocalWindow) self);
    self->visible = true;
}

void QBRedButtonOverlayWindowHide(QBRedButtonOverlayWindow self)
{
    if (!self) {
        SvLogError("%s(): got NULL self", __func__);
        return;
    }

    QBApplicationControllerRemoveLocalWindow(self->appGlobals->controller, (QBLocalWindow) self);
    self->visible = false;
}

bool QBRedButtonOverlayWindowIsVisible(QBRedButtonOverlayWindow self)
{
    if (!self) {
        SvLogError("%s(): got NULL self", __func__);
        return false;
    }

    return self->visible;
}
