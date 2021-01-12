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

#include "XMBCarousel.h"
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvInterface.h>
#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/effect.h>
#include <settings.h>
#include <SWL/fade.h>
#include <QBCarousel/QBVerticalCarousel.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBListProxy.h>
#include <QBDataModel3/QBTreeModel.h>
#include <XMB2/XMBVerticalMenu.h>

struct XMBCarousel_t {
    struct SvObject_ super_;
    SvWidget menu, carousel;
    SvWidget widget;
    void *notificationTarget;
    XMBMenuNotificationCallback notificationCallback;
    SvEffectId fade;
    double fadeDuration;
    int extraSpace;
};

typedef struct XMBCarousel_t *XMBCarousel;

SvLocal bool
XMBCarouselHandleInputEvent_(SvGenericObject self_,
                             SvInputEvent iev)
{
    XMBCarousel self = (XMBCarousel)self_;
    SvWidget carousel = self->carousel;

    if (iev->ch == QBKEY_ENTER || iev->ch == QBKEY_PGDN || iev->ch == QBKEY_PGUP) {
        return SvInvokeInterface(XMBMenu, self->menu->prv, handleInputEvent, iev);
    }
    return QBVerticalCarouselHandleInputEvent(carousel, iev);
}


SvLocal void
XMBCarouselSetNotificationTarget_(SvGenericObject self_,
                                  void *target,
                                  XMBMenuNotificationCallback callback)
{
    XMBCarousel self = (XMBCarousel) self_;
    self->notificationTarget = target;
    self->notificationCallback = callback;
}

SvLocal void
XMBCarouselConnectToDataSource_(SvGenericObject self_,
                                SvGenericObject dataSource,
                                SvGenericObject path,
                                SvGenericObject controller,
                                SvErrorInfo *errorOut) {
    XMBCarousel self = (XMBCarousel)self_;

    QBListProxy proxy = QBListProxyCreate(dataSource, path, NULL);

    XMBVerticalMenuConnectToDataSource(self->menu, (SvGenericObject) proxy, controller, errorOut);
    QBVerticalCarouselConnectToSource(self->carousel, (SvGenericObject) proxy, errorOut);

    SVRELEASE(proxy);
}

SvLocal int
XMBCarouselGetPosition_(SvGenericObject self_){
    XMBCarousel self = (XMBCarousel)self_;
    SvWidget menu = self->menu;
    int pos = XMBVerticalMenuGetPosition(menu);
    return pos;
}

SvLocal void
XMBCarouselSetPosition_(SvGenericObject self_,
                   unsigned int pos,
                   bool immediately,
                   SvErrorInfo *errorOut){
    XMBCarousel self = (XMBCarousel)self_;
    XMBVerticalMenuSetPosition(self->menu, pos, immediately, errorOut);
    QBVerticalCarouselSetActive(self->carousel, pos);
}

SvLocal void
XMBCarouselSetState_(SvGenericObject self_,
                XMBMenuState state){
    XMBCarousel self = (XMBCarousel)self_;
    SvWidget menu = self->menu;
    XMBVerticalMenuSetState(menu, state);
    SvEffect fade = NULL;
    if(state == XMBMenuState_inactive){
        fade = svEffectFadeNew(self->carousel, ALPHA_TRANSPARENT, true, self->fadeDuration, SV_EFFECT_FADE_LINEAR);
    }else if(state == XMBMenuState_normal){
        fade = svEffectFadeNew(self->carousel, ALPHA_SOLID, true, self->fadeDuration, SV_EFFECT_FADE_LINEAR);
    }
    QBVerticalCarouselSetState(self->carousel, (state == XMBMenuState_inactive ? QBVerticalCarouselState_inactive : QBVerticalCarouselState_active));
    if (fade) {
        svEffectSetNotificationTarget(fade, self->widget);
        if (self->fade)
            svAppCancelEffect(menu->app, self->fade, SV_EFFECT_ABANDON);
        self->fade = svAppRegisterEffect(menu->app, fade);
    }
}

SvLocal SvGenericObject
XMBCarouselGetItemController_(SvGenericObject self_){
    XMBCarousel self = (XMBCarousel)self_;
    SvWidget menu = self->menu;
    return XMBVerticalMenuGetItemController(menu);
}

SvLocal void
XMBCarouselSetItemController_(SvGenericObject self_,
                         SvGenericObject controller,
                         SvErrorInfo *errorOut){
    XMBCarousel self = (XMBCarousel)self_;
    SvWidget menu = self->menu;
    XMBVerticalMenuSetItemController(menu, controller, errorOut);
}

SvLocal SvWidget
XMBCarouselGetWidget_(SvGenericObject self_){
    XMBCarousel self = (XMBCarousel)self_;
    return self->widget;
}

SvLocal int
XMBCarouselGetExtraSpace_(SvGenericObject self_){
    XMBCarousel self = (XMBCarousel)self_;
    return self->extraSpace;
}

SvLocal void
XMBCarousel__dtor__(void *self_)
{
//    XMBCarousel self = (XMBCarousel)self_;

}

SvLocal void XMBCarouselSetFocus_(SvGenericObject self_, bool focus) {
    XMBCarousel self = (XMBCarousel)self_;
    SvInvokeInterface(XMBMenu, self->menu->prv, setFocus, focus);
}

SvLocal SvType
XMBCarousel_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = XMBCarousel__dtor__
    };
    static const struct XMBMenu_ menuInterface = {
        .connectToDataSource = XMBCarouselConnectToDataSource_,
        .getPosition = XMBCarouselGetPosition_,
        .setPosition = XMBCarouselSetPosition_,
        .handleInputEvent = XMBCarouselHandleInputEvent_,
        .setNotificationTarget = XMBCarouselSetNotificationTarget_,
        .setState = XMBCarouselSetState_,
        .setItemController = XMBCarouselSetItemController_,
        .getItemController = XMBCarouselGetItemController_,
        .getWidget = XMBCarouselGetWidget_,
        .getExtraSpace = XMBCarouselGetExtraSpace_,
        .setFocus = XMBCarouselSetFocus_

    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("XMBCarousel",
                            sizeof(struct XMBCarousel_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBMenu_getInterface(), &menuInterface,
                            NULL);
    }

    return type;
}


SvLocal void
XMBCarouselUserEventHandler(SvWidget w,
                            SvWidgetId sender,
                            SvUserEvent event)
{
    XMBCarousel self = (XMBCarousel)w->prv;

    if (event->code == SV_EVENT_QB_CAROUSEL_NOTIFICATION && sender == svWidgetGetId(self->carousel)) {
        QBVerticalCarouselNotification cnot = event->prv;
        if (cnot->type == QB_CAROUSEL_NOTIFICATION_ACTIVE_ELEMENT) {
            XMBVerticalMenuSetPosition(self->menu, cnot->dataIndex, cnot->immediately, NULL);
        }
        return;
    }
}

SvLocal void
XMBCarouselEffectEventHandler(SvWidget carousel,
                          SvEffectEvent event)
{
    XMBCarousel self = (XMBCarousel)carousel->prv;

    if (event->id == self->fade)
        self->fade = 0;
}

SvLocal void
XMBCarouselClean(SvApplication app, void *prv)
{
    XMBCarousel self = (XMBCarousel)prv;
    SVRELEASE(self);
}

SvWidget XMBCarouselGetCarouselWidget(SvWidget carousel)
{
    XMBCarousel self = (XMBCarousel)carousel->prv;
    return self->carousel;
}

SvLocal void
XMBCarouselOnChangePosition(void *w_, const XMBMenuEvent oldData)
{
    XMBCarousel self = ((SvWidget) w_)->prv;

    if (self->notificationTarget && self->notificationCallback) {
        struct XMBMenuEvent_ data = {
            .prevPosition = oldData->prevPosition,
            .position = oldData->position,
            .clicked = oldData->clicked
        };
        self->notificationCallback(self->notificationTarget, &data);
    }
}

SvWidget XMBCarouselNew(SvApplication app, const char *name){
    XMBCarousel self = (XMBCarousel)SvTypeAllocateInstance(XMBCarousel_getType(), NULL);
    self->widget = svWidgetCreateBitmap(app, app->width, app->height, NULL);

    svWidgetSetUserEventHandler(self->widget, XMBCarouselUserEventHandler);
    svWidgetSetEffectEventHandler(self->widget, XMBCarouselEffectEventHandler);
    self->widget->prv = self;
    self->widget->clean = XMBCarouselClean;
    svWidgetSetName(self->widget, "XMBCarousel");

    char *widgetName = NULL;
    asprintf(&widgetName, "%s.menu", name);
    self->menu = XMBVerticalMenuNew(app, widgetName, NULL);
    XMBVerticalMenuSetNotificationTarget(self->menu, self->widget, XMBCarouselOnChangePosition);
    svSettingsWidgetAttach(self->widget, self->menu, widgetName, 0);
    free(widgetName);
    asprintf(&widgetName, "%s.carousel", name);
    self->carousel = QBVerticalCarouselNewFromSM(app, widgetName);
    QBVerticalCarouselSetNotificationTarget(self->carousel, svWidgetGetId(self->widget));
    svSettingsWidgetAttach(self->widget, self->carousel, widgetName, 0);
    free(widgetName);

    self->fadeDuration = svSettingsGetDouble(name, "fadeDuration", 0.2);
    self->extraSpace = svSettingsGetInteger(name, "extraSpace", 0);

    return self->widget;
}
