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

#include "QBXMBSearchConstructor.h"
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/effect.h>
#include <CUIT/Core/event.h>
#include <settings.h>
#include <SWL/fade.h>
#include <libintl.h>
#include <QBWidgets/QBAsyncLabel.h>


struct QBXMBSearchItemConstructor_t {
    SvFont font;
    SvColor textColor;
    int normalDim;
    int inactiveDim;
    int width;
    int height;

    int focusXOffset;
    int focusYOffset;
    int focusHeight;
    int focusWidth;
    float focusFadeTime;
    float showTime;

    QBAsyncLabelConstructor dateConstructor, timeConstructor, channelConstructor, titleConstructor;
    int dateXOff, dateYOff;
    int timeXOff, timeYOff;
    int channelXOff, channelYOff;
    int titleXOff, titleYOff;
};

struct QBXMBSearchItemInfo_t {
    SvWidget box;
    SvWidget icon;
    SvWidget focus, inactiveFocus;

    SvWidget date, time, channel, title;

    SvEffectId fade;
    float focusFadeTime;

    float showTime;
    SvEffectId show;
};
typedef struct QBXMBSearchItemInfo_t* QBXMBSearchItemInfo;

SvLocal void QBXMBSearchItem__dtor__(void *ptr)
{
    QBXMBSearchItem self = ptr;
    SVTESTRELEASE(self->date);
    SVTESTRELEASE(self->time);
    SVTESTRELEASE(self->channel);
    SVTESTRELEASE(self->title);
    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVTESTRELEASE(self->icon);
}

SvType QBXMBSearchItem_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBXMBSearchItem__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBXMBSearchItem",
                            sizeof(struct QBXMBSearchItem_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBXMBSearchItemConstructor QBXMBSearchItemConstructorCreate(SvApplication app, const char *widgetName, QBTextRenderer renderer)
{
    QBXMBSearchItemConstructor self = calloc(1, sizeof(struct QBXMBSearchItemConstructor_t));
    self->font = svSettingsCreateFont(widgetName, "font");
    self->textColor = svSettingsGetColor(widgetName, "textColor", 0);
    self->normalDim = svSettingsGetInteger(widgetName, "normalDim", 0);
    self->inactiveDim = svSettingsGetInteger(widgetName, "inactiveDim", 0);
    self->width = svSettingsGetInteger(widgetName, "width", 0);
    self->height = svSettingsGetInteger(widgetName, "height", 0);
    self->focusXOffset = svSettingsGetInteger(widgetName, "focusXOffset", 0);
    self->focusYOffset = svSettingsGetInteger(widgetName, "focusYOffset", 0);
    self->focusWidth = svSettingsGetInteger(widgetName, "focusWidth", self->width);
    self->focusHeight = svSettingsGetInteger(widgetName, "focusHeight", self->height);
    self->focusFadeTime = svSettingsGetDouble(widgetName, "focusFadeTime", 0.0);
    self->showTime = svSettingsGetDouble(widgetName, "showTime", 0.0);
    char *name = NULL;
    asprintf(&name, "%s.dateLabel", widgetName);
    self->dateConstructor = QBAsyncLabelConstructorCreate(app, name, renderer, NULL);
    self->dateXOff = svSettingsGetInteger(name, "xOffset", 0);
    self->dateYOff = svSettingsGetInteger(name, "yOffset", 0);
    free(name);
    asprintf(&name, "%s.timeLabel", widgetName);
    self->timeConstructor = QBAsyncLabelConstructorCreate(app, name, renderer, NULL);
    self->timeXOff = svSettingsGetInteger(name, "xOffset", 0);
    self->timeYOff = svSettingsGetInteger(name, "yOffset", 0);
    free(name);
    asprintf(&name, "%s.channelLabel", widgetName);
    self->channelConstructor = QBAsyncLabelConstructorCreate(app, name, renderer, NULL);
    self->channelXOff = svSettingsGetInteger(name, "xOffset", 0);
    self->channelYOff = svSettingsGetInteger(name, "yOffset", 0);
    free(name);
    asprintf(&name, "%s.titleLabel", widgetName);
    self->titleConstructor = QBAsyncLabelConstructorCreate(app, name, renderer, NULL);
    self->titleXOff = svSettingsGetInteger(name, "xOffset", 0);
    self->titleYOff = svSettingsGetInteger(name, "yOffset", 0);
    free(name);

    return self;
}


void QBXMBSearchItemConstructorDestroy(QBXMBSearchItemConstructor self)
{
    SVTESTRELEASE(self->dateConstructor);
    SVTESTRELEASE(self->timeConstructor);
    SVTESTRELEASE(self->channelConstructor);
    SVTESTRELEASE(self->titleConstructor);
    SVRELEASE(self->font);
    free(self);
}

SvLocal void QBXMBSearchItemInfoShowFocus(QBXMBSearchItemInfo item, bool immediate)
{
    if (item->fade) {
        svAppCancelEffect(item->focus->app, item->fade, SV_EFFECT_ABANDON);
        item->fade = 0;
    }

    if (immediate) {
        item->focus->alpha = ALPHA_SOLID;
    } else {
        SvEffect effect = svEffectFadeNew(item->focus, ALPHA_SOLID, 1, item->focusFadeTime, SV_EFFECT_FADE_LOGARITHM);
        svEffectSetNotificationTarget(effect, item->box);
        item->fade = svAppRegisterEffect(item->focus->app, effect);
    }
}

SvLocal void QBXMBSearchItemInfoHideFocus(QBXMBSearchItemInfo item, bool immediate)
{
    if (item->fade) {
        svAppCancelEffect(item->focus->app, item->fade, SV_EFFECT_ABANDON);
        item->fade = 0;
    }

    if (immediate) {
        item->focus->alpha = ALPHA_TRANSPARENT;
    } else {
        SvEffect effect = svEffectFadeNew(item->focus, ALPHA_TRANSPARENT, 1, item->focusFadeTime, SV_EFFECT_FADE_LOGARITHM);
        svEffectSetNotificationTarget(effect, item->box);
        item->fade = svAppRegisterEffect(item->focus->app, effect);
    }
}

SvLocal void QBXMBSearchItemInfoShow(QBXMBSearchItemInfo item, int alpha, bool immediate)
{
    if (item->show) {
        svAppCancelEffect(item->box->app, item->show, SV_EFFECT_ABANDON);
        item->show = 0;
    }

    if (immediate) {
        item->title->alpha = alpha;
    } else {
        SvEffect effect = svEffectFadeNew(item->title, alpha, 0, item->showTime, SV_EFFECT_FADE_LOGARITHM);
        svEffectSetNotificationTarget(effect, item->box);
        item->show = svAppRegisterEffect(item->box->app, effect);
    }
}

SvLocal void QBXMBSearchItemInfoEffectEventHandler(SvWidget item_, SvEffectEvent e)
{
    QBXMBSearchItemInfo item = item_->prv;
    if(e->id == item->fade) {
        item->fade = 0;
        return;
    } else if(e->id == item->show) {
        item->show = 0;
        return;
    }
}

SvLocal void QBXMBSearchItemConstructorApplyState(QBXMBSearchItemConstructor self, QBXMBSearchItemInfo item,
                                                  XMBMenuState state, bool isFocused, bool immediate)
{
    if (isFocused) {
        if (item->icon)
            item->icon->alpha = ALPHA_SOLID;
        QBXMBSearchItemInfoShowFocus(item, immediate);
        QBXMBSearchItemInfoShow(item, ALPHA_SOLID, immediate);
    } else {
        switch (state) {
            case XMBMenuState_normal:
                if (item->icon)
                    item->icon->alpha = ALPHA_SOLID;
                QBXMBSearchItemInfoHideFocus(item, immediate);
                QBXMBSearchItemInfoShow(item, self->normalDim, immediate);
                break;
            case XMBMenuState_inactive:
                if (item->icon)
                    item->icon->alpha = self->inactiveDim;
                QBXMBSearchItemInfoHideFocus(item, immediate);
                QBXMBSearchItemInfoShow(item, 0, immediate);
                break;
            default:
                break;
        }
    }
}

SvWidget QBXMBSearchItemConstructorCreateItem(QBXMBSearchItemConstructor self, QBXMBSearchItem itemData, SvApplication app,
                                              XMBMenuState initialState)
{
    SvWidget box = svWidgetCreateBitmap(app, self->width, self->height, NULL);
    SvWidget icon = NULL;

    itemData->focus->data.minHeight = itemData->focus->data.maxHeight = self->focusHeight;

    SvWidget focus = QBFrameCreateFromConstructor(app, itemData->focus);
    focus->tree_alpha = ALPHA_TRANSPARENT;

    QBXMBSearchItemInfo item = calloc(1, sizeof(struct QBXMBSearchItemInfo_t));

    if(!itemData->date) itemData->date = SVSTRING("");
    if(!itemData->time) itemData->time = SVSTRING("");
    if(!itemData->channel) itemData->channel = SVSTRING("");
    if(!itemData->title) itemData->title = SVSTRING("");

    item->date = QBAsyncLabelNewFromConstructor(app, self->dateConstructor);
    item->time = QBAsyncLabelNewFromConstructor(app, self->timeConstructor);
    item->channel = QBAsyncLabelNewFromConstructor(app, self->channelConstructor);
    item->title = QBAsyncLabelNewFromConstructor(app, self->titleConstructor);
    svWidgetAttach(box, item->date, self->dateXOff, self->dateYOff, 5);
    svWidgetAttach(box, item->time, self->timeXOff, self->timeYOff, 5);
    svWidgetAttach(box, item->channel, self->channelXOff, self->channelYOff, 5);
    svWidgetAttach(box, item->title, self->titleXOff, self->titleYOff, 5);
    QBAsyncLabelSetText(item->date, itemData->date);
    QBAsyncLabelSetText(item->time, itemData->time);
    QBAsyncLabelSetText(item->channel, itemData->channel);
    QBAsyncLabelSetText(item->title, itemData->title);

    svWidgetAttach(box, focus, self->focusXOffset, self->focusYOffset, 1);

//    title->alpha = self->inactiveDim;
//    subcaption->alpha = self->inactiveDim;

    SvWidget inactiveFocus = NULL;
    if(itemData->inactiveFocus){
        itemData->inactiveFocus->data.minHeight = itemData->inactiveFocus->data.maxHeight = self->focusHeight;
        inactiveFocus = QBFrameCreateFromConstructor(app, itemData->inactiveFocus);
        svWidgetAttach(box, inactiveFocus, self->focusXOffset, self->focusYOffset, 0);
    }

    item->box = box;
    item->icon = icon;
    item->focus = focus;
    item->inactiveFocus = inactiveFocus;
    item->focusFadeTime = self->focusFadeTime;
    item->showTime = self->showTime;

    box->prv = item;
    svWidgetSetEffectEventHandler(box, QBXMBSearchItemInfoEffectEventHandler);

    QBXMBSearchItemConstructorApplyState(self, item, initialState, false, true);

    return box;
}


void QBXMBSearchItemConstructorSetItemState(QBXMBSearchItemConstructor self, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBXMBSearchItemInfo item = item_->prv;
    QBXMBSearchItemConstructorApplyState(self, item, state, isFocused, false);
}
