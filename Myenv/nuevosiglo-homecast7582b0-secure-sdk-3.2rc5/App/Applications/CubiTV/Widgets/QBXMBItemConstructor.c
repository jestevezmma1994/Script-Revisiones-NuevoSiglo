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

#include "QBXMBItemConstructor.h"
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/effect.h>
#include <CUIT/Core/event.h>
#include <settings.h>
#include <SWL/icon.h>
#include <SWL/fade.h>
#include <libintl.h>


SvLocal void QBXMBItem__dtor__(void *ptr)
{
    QBXMBItem self = ptr;
    SVTESTRELEASE(self->caption);
    SVTESTRELEASE(self->subcaption);
    SVTESTRELEASE(self->description);
    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVTESTRELEASE(self->sortingFocus);
    SVTESTRELEASE(self->disabledFocus);
    SVTESTRELEASE(self->icon);
    SVTESTRELEASE(self->iconRight);

    SVTESTRELEASE(self->iconURI.URI);
    SVTESTRELEASE(self->iconRightURI.URI);
}

SvType QBXMBItem_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBXMBItem__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBXMBItem",
                            sizeof(struct QBXMBItem_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBXMBItem QBXMBItemCreate(void)
{
    QBXMBItem item = (QBXMBItem) SvTypeAllocateInstance(QBXMBItem_getType(), NULL);
    item->iconRID = item->loadingRID = SV_RID_INVALID;

    return item;
}


SvType
QBXMBItemInfo_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBXMBItemInfo",
                            sizeof(struct QBXMBItemInfo_t),
                            SvObject_getType(),
                            &type,
                            NULL);
    }

    return type;
}


QBXMBItemInfo
QBXMBItemInfoCreate(void)
{
    QBXMBItemInfo self = (QBXMBItemInfo) SvTypeAllocateInstance(QBXMBItemInfo_getType(), NULL);

    return self;
}


QBXMBItemConstructor
QBXMBItemConstructorInit(QBXMBItemConstructor self, const char *widgetName, QBTextRenderer renderer)
{
    SvGlyphMetrics metricsStorage, *metrics;

    self->font = svSettingsCreateFont(widgetName, "font");
    self->textColor = svSettingsGetColor(widgetName, "textColor", 0);
    self->disabledTextColor = svSettingsGetColor(widgetName, "disabledTextColor", 0);
    self->textOffset = svSettingsGetInteger(widgetName, "textOffset", 0);
    self->spacing = svSettingsGetInteger(widgetName, "spacing", 0);
    self->leftPadding = svSettingsGetInteger(widgetName, "leftPadding", 0);
    self->rightPadding = svSettingsGetInteger(widgetName, "rightPadding", 0);
    self->normalDim = svSettingsGetInteger(widgetName, "normalDim", 0);
    self->inactiveDim = svSettingsGetInteger(widgetName, "inactiveDim", 0);
    self->fontSize = svSettingsGetInteger(widgetName, "fontSize", 0);
    self->subFontSize = svSettingsGetInteger(widgetName, "subFontSize", 0);
    self->subTextColor = svSettingsGetColor(widgetName, "subTextColor", self->textColor);
    self->descriptionFontSize = svSettingsGetInteger(widgetName, "descFontSize", 0);
    self->descriptionTextColor = svSettingsGetColor(widgetName, "descTextColor", self->textColor);
    self->disabledDescriptionTextColor = svSettingsGetColor(widgetName, "disabledDescTextColor", self->disabledTextColor);
    self->descriptionYOffset = svSettingsGetInteger(widgetName, "descYOffset", 0);
    self->disabledSubTextColor = svSettingsGetColor(widgetName, "disabledSubTextColor", self->disabledTextColor);
    self->subcaptionSpacing = svSettingsGetInteger(widgetName, "subcaptionSpacing", 0);
    self->width = svSettingsGetInteger(widgetName, "width", 0);
    self->height = svSettingsGetInteger(widgetName, "height", 0);
    self->focusXOffset = svSettingsGetInteger(widgetName, "focusXOffset", 0);
    self->focusYOffset = svSettingsGetInteger(widgetName, "focusYOffset", 0);
    self->focusWidth = svSettingsGetInteger(widgetName, "focusWidth", self->width);
    self->focusHeight = svSettingsGetInteger(widgetName, "focusHeight", self->height);
    self->focusFadeTime = svSettingsGetDouble(widgetName, "focusFadeTime", 0.0);
    self->showTime = svSettingsGetDouble(widgetName, "showTime", 0.0);
    self->minBottomPadding = svSettingsGetInteger(widgetName, "minBottomPadding", 0);
    self->hideInactiveText = svSettingsGetBoolean(widgetName, "hideInactiveText", false);
    self->showTextFocused = svSettingsGetBoolean(widgetName, "showTextFocused", false);
    self->hideInactiveFocus = svSettingsGetBoolean(widgetName, "hideInactiveFocus", false);
    self->titleMultiline = svSettingsGetBoolean(widgetName, "multilineTitle", false);
    self->subtitleMultiline = svSettingsGetBoolean(widgetName, "subtitleMultiline", false);
    self->isMarkup = svSettingsGetBoolean(widgetName, "isMarkup", false);
    self->iconHeight = svSettingsGetInteger(widgetName, "iconHeight", -1);
    self->iconWidth = svSettingsGetInteger(widgetName, "iconWidth", -1);
    self->activeRowHeightFactor = svSettingsGetInteger(widgetName, "activeRowHeightFactor", 1);
    self->maxDescLines = svSettingsGetInteger(widgetName, "maxDescLines", 1);
    const char* textEnd = svSettingsGetString(widgetName, "textEnd");

    if (textEnd)
        self->textEnd = SvStringCreate(textEnd, NULL);
    else if (self->titleMultiline || self->subtitleMultiline)
        self->textEnd = SVSTRING("…");

    self->renderer = renderer;

    // calculate mbox heights, will need them later
    // for correct vertical positioning of caption and subcaption
    SvFontSetSize(self->font, self->fontSize, NULL);
    metrics = SvFontGetGlyphMetrics(self->font, 'M', &metricsStorage, NULL);
    self->mboxHeight = (metrics->minY + 63) / 64;
    SvFontSetSize(self->font, self->subFontSize, NULL);
    metrics = SvFontGetGlyphMetrics(self->font, 'M', &metricsStorage, NULL);
    self->subMBoxHeight = (metrics->minY + 63) / 64;

    return self;
}


QBXMBItemConstructor QBXMBItemConstructorCreate(const char *widgetName, QBTextRenderer renderer)
{
    QBXMBItemConstructor self = (QBXMBItemConstructor) SvTypeAllocateInstance(QBXMBItemConstructor_getType(), NULL);

    return QBXMBItemConstructorInit(self, widgetName, renderer);
}

void QBXMBItemConstructorDestroy(QBXMBItemConstructor self)
{
    SVTESTRELEASE(self);
}

SvLocal void QBXMBItemInfoShow(QBXMBItemInfo item, int alpha, bool immediate)
{
    if (item->show) {
        svAppCancelEffect(item->box->app, item->show, SV_EFFECT_ABANDON);
        item->show = 0;
    }
    if (item->showSubcaption) {
        svAppCancelEffect(item->box->app, item->showSubcaption, SV_EFFECT_ABANDON);
        item->showSubcaption = 0;
    }

    if (item->showRightIcon) {
        svAppCancelEffect(item->box->app, item->showRightIcon, SV_EFFECT_ABANDON);
        item->showRightIcon = 0;
    }

    if (immediate) {
        item->title.w->tree_alpha = alpha;
        item->subcaption.w->tree_alpha = alpha;
        if (item->iconRight)
            item->iconRight->tree_alpha = alpha;
    } else {
        SvEffect effect = svEffectFadeNew(item->title.w, alpha, 1, item->showTime, SV_EFFECT_FADE_LOGARITHM);
        svEffectSetNotificationTarget(effect, item->box);
        item->show = svAppRegisterEffect(item->box->app, effect);
        effect = svEffectFadeNew(item->subcaption.w, alpha, 1, item->showTime, SV_EFFECT_FADE_LOGARITHM);
        item->showSubcaption = svAppRegisterEffect(item->box->app, effect);
        if (item->iconRight) {
            effect = svEffectFadeNew(item->iconRight, alpha, 1, item->showTime, SV_EFFECT_FADE_LOGARITHM);
            item->showRightIcon = svAppRegisterEffect(item->box->app, effect);
        }
    }
}

SvLocal void QBXMBItemInfoEffectEventHandler(SvWidget item_, SvEffectEvent e)
{
    QBXMBItemInfo item = item_->prv;
    if(e->id == item->focusFade) {
        item->focusFade = 0;
        return;
    } else if(e->id == item->inactiveFocusFade) {
        item->inactiveFocusFade = 0;
        return;
    } else if(e->id == item->sortingFocusFade) {
        item->sortingFocusFade = 0;
        return;
    } else if(e->id == item->disabledFocusFade) {
        item->disabledFocusFade = 0;
        return;
    } else if(e->id == item->show) {
        item->show = 0;
        return;
    }
}

void QBXMBItemInfoSetFocusWidth(QBXMBItemInfo item, int focusWidth)
{
    item->currentFocusWidth = focusWidth;
    if(item->focus)
        QBFrameSetWidth(item->focus, focusWidth);
    if(item->inactiveFocus)
        QBFrameSetWidth(item->inactiveFocus, focusWidth);
    if(item->sortingFocus)
        QBFrameSetWidth(item->sortingFocus, focusWidth);
    if(item->disabledFocus)
        QBFrameSetWidth(item->disabledFocus, focusWidth);
}

void QBXMBItemInfoPositionItems(QBXMBItemInfo item)
{
    if (!item->title.rendered)
        return;

    item->title.w->off_x = item->captionsXPos - item->title.baseLineX;

    int focusWidth = item->title.w->width + item->title.w->off_x + item->padding;

    if (item->descriptionYOffset) {
        item->title.w->off_y = item->constructor->spacing;
        if (item->subcaption.rendered) {
            item->subcaption.w->off_x = item->captionsXPos - item->title.baseLineX;
            item->subcaption.w->off_y = item->title.w->off_y + item->subcaptionSpacing + item->title.w->height;
        }
        if (item->description.rendered) {
            item->description.w->off_x = item->captionsXPos - item->description.baseLineX;
            item->description.w->off_y = item->descriptionYOffset;
        }
    } else {
        if (item->subcaption.rendered) {
            item->subcaption.w->off_x = item->captionsXPos - item->title.baseLineX;

            int tmp = item->subcaption.w->width + item->subcaption.w->off_x + item->padding;
            if (focusWidth < tmp)
                focusWidth = tmp;

            if (item->subcaption.multiLine) {
                // probably we can do it better
                int totalHeight = item->title.w->height + item->subcaptionSpacing + item->subcaption.w->height;
                item->title.w->off_y = (item->box->height - totalHeight) / 2;
                item->subcaption.w->off_y = item->title.w->off_y + item->subcaptionSpacing + item->title.w->height;
            } else {
                int halfHeight = item->box->height / 2;
                item->title.w->off_y = halfHeight - item->subcaptionSpacing / 2 - item->title.baseLineY;
                item->subcaption.w->off_y = halfHeight + item->subcaptionSpacing / 2 + item->subcaption.mboxHeight - item->subcaption.baseLineY;
            }
        } else {
            item->title.w->off_y = (item->box->height + item->title.mboxHeight) / 2 - item->title.baseLineY;
        }
    }
    QBXMBItemInfoSetFocusWidth(item, focusWidth);
}


static void QBXMBItemInfoSetFocusHeight(QBXMBItemInfo item, int newHeight)
{
    if (item->focus)
        QBFrameSetHeight(item->focus, newHeight);
    if (item->inactiveFocus)
        QBFrameSetHeight(item->inactiveFocus, newHeight);
    if (item->sortingFocus)
        QBFrameSetHeight(item->sortingFocus, newHeight);
    if (item->disabledFocus)
        QBFrameSetHeight(item->disabledFocus, newHeight);
    item->box->height = newHeight;
}

SvLocal void QBXMBItemInfoFillDescription(void *item_, QBTextRendererTask task)
{
    SvBitmap description = task->surface;
    QBXMBItemInfo item = item_;
    svWidgetSetBitmap(item->description.w, description);
    item->description.rendered = true;
    item->description.multiLine = task->linesCount > 1;
    item->description.baseLineX = -task->box.x;
    item->description.baseLineY = task->box.y;
    item->description.w->width = item->description.w->bmp->width;
    item->description.w->height = item->description.w->bmp->height;
    if (svWidgetGetParent(item->description.w))
        item->positionCallback(item);
}

SvLocal void QBXMBItemCutExceededLinesFromDescription(void *item_, QBTextRendererTask task, SvTextLayout layout)
{
    QBXMBItemInfo item = item_;
    unsigned int maxDescriptionHeight = item->constructor->activeRowHeightFactor * item->constructor->height - item->constructor->descriptionYOffset;
    SvTextLayoutRemoveLinesExceedingMaxHeight(layout, maxDescriptionHeight, NULL);
}

SvLocal void QBXMBItemConstructorSetupFocusFade(SvWidget widget, SvWidget focus,
                                                float focusFadeTime, unsigned int alpha, SvEffectId *id,
                                                bool immediate)
{
    if(!focus)
        return;
    if(*id) {
        svAppCancelEffect(widget->app, *id, SV_EFFECT_ABANDON);
        *id = 0;
    }

    if (immediate) {
        focus->tree_alpha = alpha;
    } else {
        SvEffect effect = svEffectFadeNew(focus, alpha, 1, focusFadeTime, SV_EFFECT_FADE_LOGARITHM);
        svEffectSetNotificationTarget(effect, widget);
        *id = svAppRegisterEffect(widget->app, effect);
    }
}

SvLocal void QBXMBItemInfoShowFocus(QBXMBItemInfo item, unsigned int alpha1, unsigned int alpha2, unsigned int alpha3, bool immediate)
{
    if (!item->disabled || !item->disabledFocus)
        QBXMBItemConstructorSetupFocusFade(item->box, item->focus, item->focusFadeTime, alpha1, &item->focusFade, immediate);
    else
        QBXMBItemConstructorSetupFocusFade(item->box, item->disabledFocus, item->focusFadeTime, alpha1, &item->disabledFocusFade, immediate);
    QBXMBItemConstructorSetupFocusFade(item->box, item->inactiveFocus, item->focusFadeTime, alpha2, &item->inactiveFocusFade, immediate);
    QBXMBItemConstructorSetupFocusFade(item->box, item->sortingFocus, item->focusFadeTime, alpha3, &item->sortingFocusFade, immediate);
}

static void QBXMBItemConstructorApplyState(QBXMBItemConstructor self, QBXMBItemInfo item,
                                           XMBMenuState state, bool isFocused, bool immediate)
{
    unsigned int focusAlpha = 0;
    unsigned int inactiveFocusAlpha = 0;
    unsigned int sortingFocusAlpha = 0;

    if (state == XMBMenuState_normal || state == XMBMenuState_inactive) {
        sortingFocusAlpha = ALPHA_TRANSPARENT;
        if (isFocused && (state == XMBMenuState_normal || !self->hideInactiveFocus)) {
            focusAlpha = ALPHA_SOLID;
            inactiveFocusAlpha = ALPHA_TRANSPARENT;
        } else {
            focusAlpha = ALPHA_TRANSPARENT;
            if (self->hideInactiveText && state == XMBMenuState_inactive)
                inactiveFocusAlpha = ALPHA_TRANSPARENT;
            else
                inactiveFocusAlpha = ALPHA_SOLID;
        }
    } else if (state == XMBMenuState_sorting) {
        if (isFocused) {
            sortingFocusAlpha = ALPHA_SOLID;
            inactiveFocusAlpha = ALPHA_TRANSPARENT;
        } else {
            sortingFocusAlpha = ALPHA_TRANSPARENT;
            inactiveFocusAlpha = ALPHA_SOLID;
        }
        focusAlpha = ALPHA_TRANSPARENT;
    }

    QBXMBItemInfoShowFocus(item, focusAlpha, inactiveFocusAlpha, sortingFocusAlpha, immediate);
    QBXMBItemInfoSetFocusHeight(item, isFocused ? self->height * self->activeRowHeightFactor : self->height);

    if (isFocused) {
        if (item->icon)
            item->icon->tree_alpha = ALPHA_SOLID;
        unsigned int alpha = ALPHA_TRANSPARENT;
        if (self->showTextFocused || state != XMBMenuState_inactive || !self->hideInactiveText) {
            alpha = self->normalDim;
        }
        QBXMBItemInfoShow(item, alpha, immediate);
    } else {
        switch (state) {
            case XMBMenuState_normal:
            case XMBMenuState_sorting:
                if (item->icon)
                    item->icon->tree_alpha = ALPHA_SOLID;
                QBXMBItemInfoShow(item, self->normalDim, immediate);
                break;
            case XMBMenuState_inactive:
                if (item->icon)
                    item->icon->tree_alpha = self->inactiveDim;
                QBXMBItemInfoShow(item, self->hideInactiveText ? ALPHA_TRANSPARENT : self->normalDim, immediate);

                break;
            default:
                break;
        }
    }
}

SvLocal void QBXMBItemInfoFillTitle(void *item_, QBTextRendererTask task)
{
    SvBitmap title = task->surface;
    QBXMBItemInfo item = item_;
    svWidgetSetBitmap(item->title.w, title);
    item->title.rendered = true;
    item->title.baseLineX = -task->box.x;
    item->title.baseLineY = task->box.y;
    item->title.w->width = item->title.w->bmp->width;
    item->title.w->height = item->title.w->bmp->height;
    if (svWidgetIsAttached(item->title.w))
        item->positionCallback(item);
}

SvLocal void QBXMBItemInfoFillSubcaption(void *item_, QBTextRendererTask task)
{
    SvBitmap subcaption = task->surface;
    QBXMBItemInfo item = item_;
    svWidgetSetBitmap(item->subcaption.w, subcaption);
    item->subcaption.rendered = true;
    item->subcaption.multiLine = task->linesCount > 1;
    item->subcaption.baseLineX = -task->box.x;
    item->subcaption.baseLineY = task->box.y;
    item->subcaption.w->width = item->subcaption.w->bmp->width;
    item->subcaption.w->height = item->subcaption.w->bmp->height;
    if (svWidgetIsAttached(item->subcaption.w))
        item->positionCallback(item);
}

SvLocal SvWidget
QBXMBItemConstructorCreateCaption(SvGenericObject self_, QBXMBItemInfo itemInfo, QBXMBItem itemData)
{
    QBXMBItemConstructor self = (QBXMBItemConstructor) self_;
    SvString caption = itemData->caption;
    SvWidget title = svWidgetCreateBitmap(itemInfo->box->app, 0, 0, NULL);

    itemInfo->title.rendered = false;
    itemInfo->title.mboxHeight = self->mboxHeight;
    itemInfo->title.w = title;

    if (caption && SvStringGetLength(caption)) {
        QBTextRendererTask task = QBTextRendererTaskNew(self->renderer, caption,
            QBXMBItemInfoFillTitle, itemInfo, self->font, self->fontSize,
            itemInfo->disabled ? self->disabledTextColor : self->textColor ,
            self->width - self->leftPadding - 2 * self->spacing
                - (itemInfo->icon ? itemInfo->icon->width : 0)
                - (itemInfo->iconRight ? itemInfo->iconRight->width : 0));

        task->isMarkup = self->isMarkup;
        task->maxLines = self->titleMultiline ? 0 : 1;
        task->textEnd = self->titleMultiline ? NULL : SVTESTRETAIN(self->textEnd);
        SvBitmap titleBitmap = QBTextRendererTaskStart(task);
        svWidgetSetBitmap(title, titleBitmap);
        SVRELEASE(task);
    }

    return title;
}

SvLocal SvWidget
QBXMBItemConstructorCreateSubcaption(SvGenericObject self_, QBXMBItemInfo itemInfo, QBXMBItem itemData)
{
    QBXMBItemConstructor self = (QBXMBItemConstructor) self_;
    SvString caption = itemData->subcaption;

    SvWidget subcaption = svWidgetCreateBitmap(itemInfo->box->app, 0, 0, NULL);

    itemInfo->subcaption.w = subcaption;
    itemInfo->subcaption.rendered = false;
    itemInfo->subcaption.mboxHeight = self->subMBoxHeight;

    if (caption && SvStringLength(caption) != 0) {
        QBTextRendererTask task = QBTextRendererTaskNew(self->renderer, caption,
            QBXMBItemInfoFillSubcaption, itemInfo, self->font, self->subFontSize,
            itemInfo->disabled ? self->disabledSubTextColor : self->subTextColor,
            self->width - self->leftPadding - 2 * self->spacing
                - (itemInfo->icon ? itemInfo->icon->width : 0));

        task->maxLines = self->subtitleMultiline ? 0 : 1;
        task->textEnd = self->subtitleMultiline ? NULL : SVTESTRETAIN(self->textEnd);
        SvBitmap subcaptionBitmap = QBTextRendererTaskStart(task);
        svWidgetSetBitmap(subcaption, subcaptionBitmap);
        SVRELEASE(task);
    }

    return subcaption;
}

SvLocal SvWidget
QBXMBItemConstructorCreateDescription(SvGenericObject self_, QBXMBItemInfo itemInfo, QBXMBItem itemData)
{
    QBXMBItemConstructor self = (QBXMBItemConstructor) self_;
    SvString caption = itemData->description;

    SvWidget description = svWidgetCreateBitmap(itemInfo->box->app, 0, 0, NULL);

    itemInfo->description.w = description;
    itemInfo->description.rendered = false;
    itemInfo->description.mboxHeight = self->subMBoxHeight;

    if (caption && SvStringLength(caption) != 0) {
        QBTextRendererRenderParams params = {
            .text               = caption,
            .callback           = QBXMBItemInfoFillDescription,
            .callbackData       = itemInfo,
            .layoutCallback     = QBXMBItemCutExceededLinesFromDescription,
            .layoutCallbackData = itemInfo,
            .font               = self->font,
            .fontSize           = self->descriptionFontSize,
            .textColor          = itemInfo->disabled ? self->disabledSubTextColor : self->subTextColor,
            .maxWidth           = self->width - self->leftPadding - 2 * self->spacing
                                  - (itemInfo->icon ? itemInfo->icon->width : 0),
            .mode               = QBTextRendererMode_renderAll,
        };
        QBTextRendererTask task = QBTextRendererTaskNewFromParams(self->renderer, &params);
        task->maxLines = self->maxDescLines;
        task->textEnd = self->subtitleMultiline ? NULL : SVTESTRETAIN(self->textEnd);
        SvBitmap descriptionBitmap = QBTextRendererTaskStart(task);
        svWidgetSetBitmap(description, descriptionBitmap);
        SVRELEASE(task);
    }

    return description;
}

SvLocal SvWidget
QBXMBItemConstructorCreateIcon(SvGenericObject self_, QBXMBItemInfo itemInfo, QBXMBItem itemData, QBXMBItemAlignment alignment)
{
    bool right = (alignment == QBXMBItemAlignment_Right);
    QBXMBItemConstructor self = (QBXMBItemConstructor) self_;
    SvWidget icon = NULL;
    SvBitmap iconBitmap = right ? itemData->iconRight : itemData->icon;
    SvString iconURL = right ? itemData->iconRightURI.URI : itemData->iconURI.URI;
    SvRBPolicy policy = SvRBPolicy_auto;
    if (right) {
        policy = itemData->iconRightURI.isStatic ? SvRBPolicy_static : SvRBPolicy_auto;
    } else {
        policy = itemData->iconURI.isStatic ? SvRBPolicy_static : SvRBPolicy_auto;
    }

    int iconHeight = 0, iconWidth = 0;
    if (iconBitmap) {
        iconHeight = self->iconHeight > 0 ? self->iconHeight : (int) iconBitmap->height;
        iconWidth = self->iconWidth > 0 ? self->iconWidth : (int) iconBitmap->width;
        if (self->iconHeight > 0 && self->iconWidth > 0) {
            int d = self->iconHeight * iconBitmap->width - self->iconWidth * iconBitmap->height;
            if (d>0) {
                iconHeight = self->iconHeight * iconBitmap->width / self->iconWidth;
            } else {
                iconWidth = self->iconWidth * iconBitmap->height / self->iconHeight;
            }
        }
        icon = svWidgetCreateBitmap(itemInfo->box->app, iconWidth, iconHeight, iconBitmap);
    } else if (itemData->loadingRID != SV_RID_INVALID ||
               itemData->iconRID != SV_RID_INVALID ||
               iconURL) {
        icon = svIconNewWithSize(itemInfo->box->app, self->iconWidth, self->iconHeight, false);
        unsigned int idx = 0;
        if (itemData->loadingRID != SV_RID_INVALID) {
            svIconSetBitmapFromRID(icon, 0, itemData->loadingRID);
            ++idx;
        }
        if (iconURL) {
            svIconSetBitmapFromURIWithRBPolicy(icon, idx, SvStringCString(iconURL), policy);
        } else if (itemData->iconRID != SV_RID_INVALID) {
            svIconSetBitmapFromRID(icon, idx, itemData->iconRID);
        }

        svIconSwitch(icon, idx, 0, 0.0);
    }

    if (right)
        itemInfo->iconRight = icon;
    else
        itemInfo->icon = icon;

    return icon;
}

SvLocal void QBXMBItemConstructorCleanup(SvApplication app, void *prv)
{
    QBXMBItemInfo prv_item = (QBXMBItemInfo) prv;
    SVTESTRELEASE(prv_item);
}

SvWidget QBXMBItemConstructorCreateItem(QBXMBItemConstructor self, QBXMBItem itemData,
                                        SvApplication app, XMBMenuState initialState)
{
    QBXMBItemInfo item = QBXMBItemInfoCreate();
    return QBXMBItemConstructorInitItem(self, itemData, app, initialState, item);
}

SvWidget QBXMBItemConstructorInitItem(QBXMBItemConstructor self, QBXMBItem itemData,
                                      SvApplication app, XMBMenuState initialState,
                                      QBXMBItemInfo item)
{
    item->constructor = self;

    SvWidget box = svWidgetCreateBitmap(app, self->width, self->height, NULL);
    item->box = box;

    SvWidget iconRight = NULL;
    SvWidget icon = SvInvokeInterface(QBXMBItemRenderer, self, createIcon, item, itemData, QBXMBItemAlignment_Left);
    if (itemData->usesRightIcon)
        iconRight = SvInvokeInterface(QBXMBItemRenderer, self, createIcon, item, itemData, QBXMBItemAlignment_Right);
    SvWidget title = SvInvokeInterface(QBXMBItemRenderer, self, createCaption, item, itemData);
    SvWidget subcaption = SvInvokeInterface(QBXMBItemRenderer, self, createSubcaption, item, itemData);
    SvWidget description = SvInvokeInterface(QBXMBItemRenderer, self, createDescription, item, itemData);

    title->tree_alpha = self->inactiveDim;
    subcaption->tree_alpha = self->inactiveDim;
    if (iconRight)
        iconRight->tree_alpha = self->inactiveDim;

    SvWidget focus = NULL;
    if (itemData->focus) {
        focus = QBFrameCreateFromConstructor(app, itemData->focus);
    } else {
        //create transparent dummy frame
        QBFrameParameters params = {
            .width = self->width,
            .height = 1,
        };
        focus = QBFrameCreate(app, &params);
    }

    SvWidget inactiveFocus = NULL;
    if(itemData->inactiveFocus) {
        inactiveFocus = QBFrameCreateFromConstructor(app, itemData->inactiveFocus);
        svWidgetAttach(box, inactiveFocus, self->focusXOffset, self->focusYOffset, 0);
    }

    SvWidget sortingFocus = NULL;
    if(itemData->sortingFocus) {
        sortingFocus = QBFrameCreateFromConstructor(app, itemData->sortingFocus);
        svWidgetAttach(box, sortingFocus, self->focusXOffset, self->focusYOffset, 0);
    }

    SvWidget disabledFocus = NULL;
    if(itemData->disabledFocus) {
        disabledFocus = QBFrameCreateFromConstructor(app, itemData->disabledFocus);
        svWidgetAttach(box, disabledFocus, self->focusXOffset, self->focusYOffset, 0);
    }


    int offset = self->leftPadding;
    if(icon) {
        svWidgetAttach(box, icon, offset, (box->height - icon->height) / 2, 2);
        if (self->textOffset > 0)
            offset += self->spacing;
        else
            offset += icon->width + self->spacing;
    }

    if(iconRight)
        svWidgetAttach(box, iconRight, box->width - iconRight->width - self->rightPadding, (box->height - iconRight->height) / 2, 2);

    svWidgetAttach(box, title, offset, 0, 2);
    svWidgetAttach(box, subcaption, offset, 0, 2);
    svWidgetAttach(box, description, offset, 0, 2);
    svWidgetAttach(box, focus, self->focusXOffset, self->focusYOffset, 1);

    item->padding = self->leftPadding;      // right padding = left padding
    item->captionsXPos = offset + self->textOffset;
    item->icon = icon;
    item->iconRight = iconRight;
    item->focus = focus;
    item->inactiveFocus = inactiveFocus;
    item->sortingFocus = sortingFocus;
    item->disabledFocus = disabledFocus;
    item->focusFadeTime = self->focusFadeTime;
    item->showTime = self->showTime;
    item->subcaptionSpacing = self->subcaptionSpacing;
    item->descriptionYOffset = self->descriptionYOffset;
    item->minBottomPadding = self->minBottomPadding;
    item->positionCallback = QBXMBItemInfoPositionItems;
    item->disabled = itemData->disabled;

    box->clean = QBXMBItemConstructorCleanup;
    box->prv = item;
    svWidgetSetEffectEventHandler(box, QBXMBItemInfoEffectEventHandler);

    if (item->icon)
        item->icon->tree_alpha = self->inactiveDim;
    item->title.w->tree_alpha = ALPHA_SOLID;

    focus->tree_alpha = ALPHA_TRANSPARENT;
    if (sortingFocus)
        sortingFocus->tree_alpha = ALPHA_TRANSPARENT;
    if (inactiveFocus)
        inactiveFocus->tree_alpha = ALPHA_SOLID;
    if(disabledFocus)
        disabledFocus->tree_alpha = ALPHA_TRANSPARENT;

    item->subcaption.w->tree_alpha = ALPHA_SOLID;

    if (item->title.rendered || item->subcaption.rendered || item->description.rendered)
        item->positionCallback(item);

    QBXMBItemConstructorApplyState(self, item, initialState, false, true);

    return box;
}

void QBXMBItemConstructorSetItemState(QBXMBItemConstructor self, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBXMBItemInfo item = item_->prv;

    QBXMBItemConstructorApplyState(self, item, state, isFocused, false);
}

SvInterface
QBXMBItemRenderer_getInterface(void)
{
    static SvInterface interface = NULL;
    SvErrorInfo error = NULL;

    if (!interface) {
        SvInterfaceCreateManaged("QBXMBItemRenderer",
                                 sizeof(struct QBXMBItemRenderer_t),
                                 NULL, &interface, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            abort();
        }
    }

    return interface;
}


SvLocal void
QBXMBItemConstructor__dtor__(void *self_)
{
    QBXMBItemConstructor self = self_;

    SVRELEASE(self->font);
    SVTESTRELEASE(self->textEnd);
}


SvType
QBXMBItemConstructor_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBXMBItemConstructor__dtor__
    };
    static SvType type = NULL;
    static const struct QBXMBItemRenderer_t rendererMethods = {
        .createCaption = QBXMBItemConstructorCreateCaption,
        .createSubcaption = QBXMBItemConstructorCreateSubcaption,
        .createIcon = QBXMBItemConstructorCreateIcon,
        .createDescription = QBXMBItemConstructorCreateDescription,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBXMBItemConstructor",
                            sizeof(struct QBXMBItemConstructor_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBXMBItemRenderer_getInterface(), &rendererMethods,
                            NULL);
    }

    return type;
}
