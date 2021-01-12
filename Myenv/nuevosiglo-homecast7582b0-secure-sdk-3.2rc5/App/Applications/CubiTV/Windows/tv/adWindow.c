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

#include <QBApplicationController.h>
#include <SWL/fade.h>
#include <main.h>
#include <settings.h>
#include "adWindow.h"

struct QBAdWindow_t {
    struct QBLocalWindow_t super_;

    AppGlobals appGlobals;

    SvWidget   ad;
    SvEffectId fadeId;
    double     fadeTime;
    bool       visible;
};

SvLocal void
QBAdWindowDestroy(void *self_)
{
    QBAdWindow self = self_;
    svWidgetDestroy(self->super_.window);
}

SvLocal SvType QBAdWindow_getType(void)
{
    static const struct QBWindowVTable_ vtable = {
        .super_      = {
            .destroy = QBAdWindowDestroy
        }
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBAdWindow",
                            sizeof(struct QBAdWindow_t),
                            QBLocalWindow_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            NULL);
    }

    return type;
}

SvLocal void
QBAdWindowClean(SvApplication app, void *prv)
{
}

void
QBAdWindowSetAd(QBAdWindow self, SvWidget ad)
{
    if (self->ad)
        svWidgetDestroy(self->ad);
    self->ad = ad;
    if (ad)
        svWidgetAttach(self->super_.window, ad, ad->off_x, ad->off_y, 1);
}

QBAdWindow
QBAdWindowNew(AppGlobals appGlobals)
{
    SvApplication app = appGlobals->res;
    QBAdWindow self = (QBAdWindow) SvTypeAllocateInstance(QBAdWindow_getType(), NULL);
    SvWidget window = svWidgetCreateBitmap(app, app->width, app->height, NULL);
    QBLocalWindowInit((QBLocalWindow) self, window, QBLocalWindowTypeNonFocusable);
    window->prv = self;
    window->clean = QBAdWindowClean;

    self->appGlobals = appGlobals;

    self->fadeTime = 0.3; //svSettingsGetDouble("localWindow", "fadeTime", 0.3);
    self->ad = NULL;

    return self;
}

void
QBAdWindowShow(QBAdWindow self)
{
    QBApplicationControllerAddLocalWindow(self->appGlobals->controller,
                                          (QBLocalWindow) self);

    self->visible = true;
    if (self->fadeId)
        svAppCancelEffect(self->appGlobals->res, self->fadeId, SV_EFFECT_ABANDON);
    self->super_.window->tree_alpha = ALPHA_TRANSPARENT;
    SvEffect effect = svEffectFadeNew(self->super_.window, ALPHA_SOLID, true,
                                      self->fadeTime, SV_EFFECT_FADE_LOGARITHM);
    self->fadeId = svAppRegisterEffect(self->appGlobals->res, effect);
}

void
QBAdWindowHide(QBAdWindow self)
{

    QBApplicationControllerRemoveLocalWindow(self->appGlobals->controller,
                                             (QBLocalWindow) self);
    self->visible = false;
}

bool
QBAdWindowIsVisible(QBAdWindow self)
{
    return self->visible;
}
