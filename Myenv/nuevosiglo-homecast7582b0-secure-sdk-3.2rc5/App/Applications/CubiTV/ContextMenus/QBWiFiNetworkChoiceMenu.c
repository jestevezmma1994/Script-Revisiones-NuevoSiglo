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

#include "QBWiFiNetworkChoiceMenu.h"
#include <assert.h>
#include <settings.h>
#include <QBNetworkMonitor/QBWiFiNetwork.h>
#include <XMB2/XMBTypes.h>
#include <XMB2/XMBItemController.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/event.h>
#include <SWL/fade.h>
#include <SWL/QBFrame.h>

struct QBWiFiNetworkChoiceController_ {
    struct SvObject_ super_;

    SvFont font;
    SvFont subcaptionFont;
    SvColor textColor;
    int leftPadding;
    int subcaptionLeftPadding;
    int subcaptionYOffset;
    int spacing;
    int normalDim;
    int inactiveDim;
    int fontSize;
    int subcaptionFontSize;
    int mboxHeight;
    int width;
    int height;
    SvBitmap remembered;
    SvBitmap connected;
    SvBitmap connecting;
    SvBitmap unknown;
    SvBitmap secured;
    SvBitmap open;
    SvBitmap signalStrength[5];
    int focusXOffset;
    int focusYOffset;
    SvBitmap focusBitmap;
    int focusMinWidth;
    float focusFadeTime;

    QBTextRenderer renderer;

    void *callbackData;
    QBWiFiNetworkChoiceControllerCallbacks callbacks;
};

typedef struct QBWiFiNetworkChoiceItem_ {
    SvWidget title;
    SvWidget subtitle;
    SvWidget box;
    SvWidget stateIcon;
    SvWidget securityIcon;
    SvWidget signalIcon;
    SvWidget focus;

    int captionXPos;
    int mboxHeight;
    int padding;

    int maxWidth;

    SvEffectId fade;
    float focusFadeTime;
} *QBWiFiNetworkChoiceItem;

SvLocal void QBWiFiNetworkChoiceItemFillTitle(void *item_, QBTextRendererTask task)
{
    QBWiFiNetworkChoiceItem item = item_;
    SvBitmap title = task->surface;
    item->title->width = title->width;
    item->title->height = title->height;
    svWidgetSetBitmap(item->title, title);
    item->title->off_x = item->captionXPos + task->box.x;
    item->title->off_y = (item->box->height + item->mboxHeight) / 2 - 1.6 * task->box.y;
    int focusWidth = item->title->width + item->title->off_x + item->padding;
    if (focusWidth > item->maxWidth) {
        QBFrameSetWidth(item->focus, focusWidth);
        item->maxWidth = focusWidth;
    }

}

SvLocal void QBWiFiNetworkChoiceItemFillSubtitle(void *item_, QBTextRendererTask task)
{
    QBWiFiNetworkChoiceItem item = item_;
    SvBitmap subtitle = task->surface;
    item->subtitle->width = subtitle->width;
    item->subtitle->height = subtitle->height;
    svWidgetSetBitmap(item->subtitle, subtitle);
    item->subtitle->off_x = item->captionXPos + task->box.x;
    item->subtitle->off_y = (item->box->height + item->mboxHeight) / 2 + 0.1 * task->box.y;
    int focusWidth = item->subtitle->width + item->subtitle->off_x + item->padding;
    if (focusWidth > item->maxWidth) {
        QBFrameSetWidth(item->focus, focusWidth);
        item->maxWidth = focusWidth;
    }
}

SvLocal void QBWiFiNetworkChoiceItemShowFocus(QBWiFiNetworkChoiceItem item, bool immediate)
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

SvLocal void QBWiFiNetworkChoiceItemHideFocus(QBWiFiNetworkChoiceItem item, bool immediate)
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

SvLocal void QBWiFiNetworkChoiceItemEffectEventHandler(SvWidget item_, SvEffectEvent e)
{
    QBWiFiNetworkChoiceItem item = item_->prv;
    if(e->id == item->fade) {
        item->fade = 0;
        return;
    }
}

SvLocal void QBWiFiNetworkChoiceControllerApplyState(QBWiFiNetworkChoiceController self, QBWiFiNetworkChoiceItem item, XMBMenuState state, bool isFocused, bool immediate)
{
    if (isFocused) {
        QBWiFiNetworkChoiceItemShowFocus(item, immediate);
    } else {
        QBWiFiNetworkChoiceItemHideFocus(item, immediate);
    }
}

SvLocal SvWidget QBWiFiNetworkChoiceControllerCreateItem(SvObject self_, SvObject node, SvObject path, SvApplication app, XMBMenuState initialState)
{
    QBWiFiNetworkChoiceController self = (QBWiFiNetworkChoiceController) self_;

    SvString caption = NULL;
    SvString subcaption = NULL;

    if (self->callbacks->createNodeCaption) {
        caption = self->callbacks->createNodeCaption(self->callbackData, node);
    }

    if (SvObjectIsInstanceOf(node, QBWiFiNetwork_getType())) {
        if (self->callbacks->createNodeSubcaption) {
            subcaption = self->callbacks->createNodeSubcaption(self->callbackData, node);
        }
    }

    if (!caption)
        caption = SvStringCreate("???", NULL);

    if (!subcaption)
        subcaption = SvStringCreate("", NULL);

    SvWidget box = svWidgetCreateBitmap(app, self->width, self->height, NULL);

    SvWidget stateIcon = NULL;
    SvWidget signalIcon = NULL;
    SvWidget securityIcon = NULL;
    if (SvObjectIsInstanceOf(node, QBWiFiNetwork_getType())) {
        WiFiNetworkChoiceState state = self->callbacks->getState(self->callbackData, node);
        if (state == WiFiNetworkChoice_remembered) {
            stateIcon = svWidgetCreateBitmap(app, self->remembered->width, self->remembered->height, self->remembered);
        } else if (state == WiFiNetworkChoice_connecting) {
            stateIcon = svWidgetCreateBitmap(app, self->connecting->width, self->connecting->height, self->connecting);
        } else if (state == WiFiNetworkChoice_connected) {
            stateIcon = svWidgetCreateBitmap(app, self->connected->width, self->connected->height, self->connected);
        } else {
            stateIcon = svWidgetCreateBitmap(app, self->unknown->width, self->unknown->height, self->unknown);
        }

        QBWiFiNetworkSignalLevel signalLevel = self->callbacks->getSignalLevel(self->callbackData, node);
        if (signalLevel == QBWiFiNetworkSignalLevel_veryWeak) {
            signalIcon = svWidgetCreateBitmap(app, self->signalStrength[0]->width, self->signalStrength[0]->height, self->signalStrength[0]);
        } else if (signalLevel == QBWiFiNetworkSignalLevel_weak) {
            signalIcon = svWidgetCreateBitmap(app, self->signalStrength[1]->width, self->signalStrength[1]->height, self->signalStrength[1]);
        } else if (signalLevel == QBWiFiNetworkSignalLevel_moderate) {
            signalIcon = svWidgetCreateBitmap(app, self->signalStrength[2]->width, self->signalStrength[2]->height, self->signalStrength[2]);
        } else if (signalLevel == QBWiFiNetworkSignalLevel_good) {
            signalIcon = svWidgetCreateBitmap(app, self->signalStrength[3]->width, self->signalStrength[3]->height, self->signalStrength[3]);
        } else {
            signalIcon = svWidgetCreateBitmap(app, self->signalStrength[4]->width, self->signalStrength[4]->height, self->signalStrength[4]);
        }

        WiFiNetworkChoiceSecurity security = self->callbacks->getSecurity(self->callbackData, node);
        if (security == WiFiNetworkChoice_secure) {
            securityIcon = svWidgetCreateBitmap(app, self->secured->width, self->secured->height, self->secured);
        } else {
            securityIcon = svWidgetCreateBitmap(app, self->secured->width, self->secured->height, self->open);
        }
    }

    SvWidget title = svWidgetCreateBitmap(app, 0, 0, NULL);
    SvWidget subtitle = svWidgetCreateBitmap(app, 0, 0, NULL);

    QBFrameConstructData params = {
        .bitmap = self->focusBitmap,
        .width = self->focusBitmap->width,
        .height = self->focusBitmap->height,
        .minWidth = self->focusMinWidth,
    };
    SvWidget focus = QBFrameCreate(app, &params);

    title->alpha = self->inactiveDim;
    subtitle->alpha = self->inactiveDim;

    QBWiFiNetworkChoiceItem item = calloc(1, sizeof(struct QBWiFiNetworkChoiceItem_));
    item->title = title;
    item->subtitle = subtitle;
    item->box = box;
    if (SvObjectIsInstanceOf(node, QBWiFiNetwork_getType())) {
        item->stateIcon = stateIcon;
        item->signalIcon = signalIcon;
        item->securityIcon = securityIcon;
    }
    item->focus = focus;
    item->focusFadeTime = self->focusFadeTime;
    item->captionXPos = self->focusXOffset + self->leftPadding;
    item->mboxHeight = self->mboxHeight;
    item->padding = self->leftPadding;

    if (SvObjectIsInstanceOf(node, QBWiFiNetwork_getType())) {
        assert(signalIcon);
        svWidgetAttach(box, signalIcon, self->leftPadding, (box->height - signalIcon->height) / 2, 2);
        svWidgetAttach(box, securityIcon, self->leftPadding + signalIcon->width - securityIcon->width/2, box->height - signalIcon->height, 3);
        svWidgetAttach(box, stateIcon, self->leftPadding + signalIcon->width, (box->height - signalIcon->height) / 3, 2);
        svWidgetAttach(box, title, item->captionXPos, 0, 2);
    } else {
        svWidgetAttach(box, title, item->captionXPos, 0, 2);
    }
    svWidgetAttach(box, subtitle, item->captionXPos + self->subcaptionLeftPadding, self->subcaptionYOffset, 2);
    svWidgetAttach(box, focus, self->focusXOffset, self->focusYOffset, 1);
    focus->tree_alpha = ALPHA_TRANSPARENT;

    SvBitmap titleBitmap = QBTextRendererRender(self->renderer, caption, QBWiFiNetworkChoiceItemFillTitle, item, self->font, self->fontSize, self->textColor, self->width);
    SVRELEASE(caption);
    svWidgetSetBitmap(title, titleBitmap);

    SvBitmap subtitleBitmap = QBTextRendererRender(self->renderer, subcaption, QBWiFiNetworkChoiceItemFillSubtitle, item, self->subcaptionFont, self->subcaptionFontSize, self->textColor, self->width);
    SVRELEASE(subcaption);
    svWidgetSetBitmap(subtitle, subtitleBitmap);

    box->prv = item;
    svWidgetSetEffectEventHandler(box, QBWiFiNetworkChoiceItemEffectEventHandler);

    QBWiFiNetworkChoiceControllerApplyState(self, item, initialState, false, true);

    return box;
}

SvLocal void QBWiFiNetworkChoiceControllerSetItemState(SvObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBWiFiNetworkChoiceController self = (QBWiFiNetworkChoiceController) self_;
    QBWiFiNetworkChoiceItem item = item_->prv;

    QBWiFiNetworkChoiceControllerApplyState(self, item, state, isFocused, false);
}

SvLocal void QBWiFiNetworkChoiceControllerDestroy(void *ptr)
{
    QBWiFiNetworkChoiceController self = ptr;
    SVRELEASE(self->font);
    SVRELEASE(self->subcaptionFont);
    SVRELEASE(self->remembered);
    SVRELEASE(self->connected);
    SVRELEASE(self->connecting);
    SVRELEASE(self->unknown);
    SVRELEASE(self->secured);
    SVRELEASE(self->open);
    SVTESTRELEASE(self->focusBitmap);
    for (int i = 0; i < 5; i++)
        SVRELEASE(self->signalStrength[i]);
}

SvLocal SvType QBWiFiNetworkChoiceController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBWiFiNetworkChoiceControllerDestroy
    };
    static SvType type = NULL;
    static const struct XMBItemController_t controller_methods = {
        .createItem = QBWiFiNetworkChoiceControllerCreateItem,
        .setItemState = QBWiFiNetworkChoiceControllerSetItemState,
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBWiFiNetworkChoiceController",
                            sizeof(struct QBWiFiNetworkChoiceController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            NULL);
    }
    return type;
}

QBWiFiNetworkChoiceController QBWiFiNetworkChoiceControllerCreateFromSettings(const char *settings, QBTextRenderer renderer, void* callbackData, QBWiFiNetworkChoiceControllerCallbacks callbacks)
{
    QBWiFiNetworkChoiceController self = (QBWiFiNetworkChoiceController) SvTypeAllocateInstance(QBWiFiNetworkChoiceController_getType(), NULL);

    svSettingsPushComponent(settings);
    self->font = svSettingsCreateFont("WiFiNetworkChoiceName", "font");
    self->subcaptionFont = svSettingsCreateFont("WiFiNetworkChoiceName", "font");
    self->textColor = svSettingsGetColor("WiFiNetworkChoiceName", "textColor", 0);
    self->spacing = svSettingsGetInteger("WiFiNetworkChoiceName", "spacing", 0);
    self->leftPadding = svSettingsGetInteger("WiFiNetworkChoiceName", "leftPadding", 0);
    self->subcaptionLeftPadding = svSettingsGetInteger("WiFiNetworkChoiceName", "subcaptionLeftPadding", 0);
    self->subcaptionYOffset = svSettingsGetInteger("WiFiNetworkChoiceName", "subcaptionYOffset", 0);
    self->normalDim = svSettingsGetInteger("WiFiNetworkChoiceName", "normalDim", 0);
    self->inactiveDim = svSettingsGetInteger("WiFiNetworkChoiceName", "inactiveDim", 0);
    self->fontSize = svSettingsGetInteger("WiFiNetworkChoiceName", "fontSize", 0);
    self->subcaptionFontSize = svSettingsGetInteger("WiFiNetworkChoiceName", "subcaptionFontSize", 0);

    self->signalStrength[0] = SVRETAIN(svSettingsGetBitmap("WiFiNetworkStatus", "signalStrength0"));
    self->signalStrength[1] = SVRETAIN(svSettingsGetBitmap("WiFiNetworkStatus", "signalStrength1"));
    self->signalStrength[2] = SVRETAIN(svSettingsGetBitmap("WiFiNetworkStatus", "signalStrength2"));
    self->signalStrength[3] = SVRETAIN(svSettingsGetBitmap("WiFiNetworkStatus", "signalStrength3"));
    self->signalStrength[4] = SVRETAIN(svSettingsGetBitmap("WiFiNetworkStatus", "signalStrength4"));

    self->remembered = SVRETAIN(svSettingsGetBitmap("WiFiNetworkStatus", "rememberedIcon"));
    self->secured = SVRETAIN(svSettingsGetBitmap("WiFiNetworkStatus", "securedIcon"));
    self->open = SVRETAIN(svSettingsGetBitmap("WiFiNetworkStatus", "openIcon"));
    self->connecting = SVRETAIN(svSettingsGetBitmap("WiFiNetworkStatus", "connectingIcon"));
    self->connected = SVRETAIN(svSettingsGetBitmap("WiFiNetworkStatus", "connectedIcon"));
    self->unknown = SVRETAIN(svSettingsGetBitmap("WiFiNetworkStatus", "unknownIcon"));

    self->width = svSettingsGetInteger("WiFiNetworkChoiceName", "width", 0);
    self->height = svSettingsGetInteger("WiFiNetworkChoiceName", "height", 0);
    self->focusXOffset = svSettingsGetInteger("WiFiNetworkChoiceName", "focusXOffset", 0);
    self->focusYOffset = svSettingsGetInteger("WiFiNetworkChoiceName", "focusYOffset", 0);
    self->focusBitmap = SVRETAIN(svSettingsGetBitmap("WiFiNetworkChoiceName", "bg"));
    self->focusMinWidth = svSettingsGetInteger("WiFiNetworkChoiceName", "focusMinWidth", 0);
    self->focusFadeTime = svSettingsGetDouble("WiFiNetworkChoiceName", "focusFadeTime", 0.0);

    self->renderer = renderer;
    self->callbackData = callbackData;
    self->callbacks = callbacks;

    SvGlyphMetrics metricsStorage, *metrics;
    SvFontSetSize(self->font, self->fontSize, NULL);
    SvFontSetSize(self->subcaptionFont, self->subcaptionFontSize, NULL);
    metrics = SvFontGetGlyphMetrics(self->font, 'M', &metricsStorage, NULL);
    self->mboxHeight = (metrics->minY + 63) / 64;

    svSettingsPopComponent();

    return self;
}
