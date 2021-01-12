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

#include "QBRatingWidget.h"
#include <SvFoundation/SvObject.h>
#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/widget.h>
#include <settings.h>

struct QBRatingWidget_ {
    SvBitmap on;
    SvBitmap off;

    unsigned int itemsCount;
    SvWidget *items;
};
typedef struct QBRatingWidget_* QBRatingWidget;

SvLocal void QBRatingWidgetClean(SvApplication app, void *self_)
{
    QBRatingWidget self = self_;
    SVTESTRELEASE(self->on);
    SVTESTRELEASE(self->off);
    free(self->items);
    free(self);
}

SvWidget QBRatingWidgetCreateWithRatio(SvApplication app, const char* widgetName,
                                       unsigned int width, unsigned int height, bool focused)
{
    QBRatingWidget self = calloc(1, sizeof(*self));
    SvWidget w = svSettingsWidgetCreateWithRatio(app, widgetName, width, height);
    w->prv = self;
    w->clean = QBRatingWidgetClean;

    if (focused) {
        self->on = SVTESTRETAIN(svSettingsGetBitmap(widgetName, "focusedOn"));
        self->off = SVTESTRETAIN(svSettingsGetBitmap(widgetName, "focusedOff"));
    } else {
        self->on = SVTESTRETAIN(svSettingsGetBitmap(widgetName, "nonfocusedOn"));
        self->off = SVTESTRETAIN(svSettingsGetBitmap(widgetName, "nonfocusedOff"));
    }
    self->itemsCount = svSettingsGetInteger(widgetName, "itemsCount", 1);
    self->items = calloc(self->itemsCount, sizeof(SvWidget));
    int spacing = (int) ((double) width * svSettingsGetDouble(widgetName, "spacingRatio", 0));
    int leftPadding = (int) ((double) width * svSettingsGetDouble(widgetName, "leftPaddingRatio", 0));
    int topPadding = (int) ((double) height * svSettingsGetDouble(widgetName, "topPaddingRatio", 0));
    char *itemWidgetName;
    asprintf(&itemWidgetName, "%s.Item", widgetName);
    int xOffset = leftPadding;
    for (size_t i = 0; i < self->itemsCount; i++) {
        self->items[i] = svSettingsWidgetCreateWithRatio(app, itemWidgetName, width, height);
        svWidgetSetBitmap(self->items[i], self->off);
        svWidgetAttach(w, self->items[i], xOffset, topPadding, 1);
        xOffset += spacing + self->items[i]->width;
    }
    free(itemWidgetName);

    return w;
}

SvWidget QBRatingWidgetCreate(SvApplication app, const char* widgetName)
{
    // Validate rating widget resolution
    int itemsCount = svSettingsGetInteger(widgetName, "itemsCount", -1);
    if (itemsCount <= 0)
        return NULL;

    QBRatingWidget self = calloc(1, sizeof(*self));
    SvWidget w = svSettingsWidgetCreate(app, widgetName);
    w->prv = self;
    w->clean = QBRatingWidgetClean;

    self->on = SVTESTRETAIN(svSettingsGetBitmap(widgetName, "on"));
    self->off = SVTESTRETAIN(svSettingsGetBitmap(widgetName, "off"));
    self->itemsCount = itemsCount;
    self->items = calloc(self->itemsCount, sizeof(SvWidget));
    int spacing = svSettingsGetInteger(widgetName, "spacing", 0);
    int leftPadding = svSettingsGetInteger(widgetName, "leftPadding", 0);
    int topPadding = svSettingsGetInteger(widgetName, "topPadding", 0);
    char *itemWidgetName;
    asprintf(&itemWidgetName, "%s.Item", widgetName);
    int xOffset = leftPadding;
    for (unsigned int i = 0; i < self->itemsCount; i++) {
        self->items[i] = svSettingsWidgetCreate(app, itemWidgetName);
        svWidgetSetBitmap(self->items[i], self->off);
        svWidgetAttach(w, self->items[i], xOffset, topPadding, 1);
        xOffset += spacing + self->items[i]->width;
    }
    free(itemWidgetName);

    return w;
}

unsigned int QBRatingWidetGetResolution(SvWidget w)
{
    QBRatingWidget self = w->prv;
    return self->itemsCount;
}

void QBRatingWidetSetRating(SvWidget w, unsigned int rating)
{
    QBRatingWidget self = w->prv;
    for (unsigned int i = 0; i < rating && i < self->itemsCount; i++) {
        svWidgetSetBitmap(self->items[i], self->on);
    }
    for (unsigned int i = rating; i < self->itemsCount; i++) {
        svWidgetSetBitmap(self->items[i], self->off);
    }
}
