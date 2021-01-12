/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBNumericInput.h"

#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/effect.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/widget.h>
#include <QBInput/QBInputCodes.h>
#include <SWL/QBFrame.h>
#include <SWL/label.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvObject.h>
#include <assert.h>
#include <libintl.h>
#include <settings.h>
#include <stdio.h>
#include <stdlib.h>

struct QBNumericInput_ {
    // widgets
    SvWidget w;
    SvWidget label;
    SvWidget box;
    SvWidget boxFrame;

    // widget properties
    QBFrameParameters params;
    SvBitmap active;
    SvBitmap inactive;

    // input properties
    bool inputBoxIsActive;
    bool valueIsFinished;

    // value properties
    int minValue;
    int maxValue;
    int value;
    unsigned int decimalPartWidth;
};
typedef struct QBNumericInput_* QBNumericInput;

SvLocal void QBNumericInputShowValue(SvWidget w)
{
    QBNumericInput self = w->prv;
    char *buf;
    if (self->decimalPartWidth) {
        int divisor = 1;
        for (unsigned int i = 0; i < self->decimalPartWidth; i++)
            divisor *= 10;
        char *fbuf;
        asprintf(&fbuf, gettext("%s%%i.%%.%ui"), self->value < 0 ? "-" : "", self->decimalPartWidth);
        int value = self->value;
        value = value < 0 ? -value : value;
        asprintf(&buf, fbuf, value / divisor, value % divisor);
        free(fbuf);
    } else
        asprintf(&buf, "%i", self->value);
    svLabelSetText(self->label, buf);
    free(buf);
}

SvLocal bool QBNumericInputInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBNumericInput self = w->prv;
    if (e->ch >= '0' && e->ch <= '9') {
        int digit = e->ch - '0';
        if (self->value == 0 && digit == 0)
            return true;

        if (self->valueIsFinished)
            self->value = digit;
        else {
            if (self->value < 0) {
                self->value = self->value * 10 - digit;
                if (self->value < self->minValue)
                    self->value = -digit;
            } else {
                self->value = self->value * 10 + digit;
                if (self->value > self->maxValue)
                    self->value = digit;
            }
        }
        QBNumericInputShowValue(w);

        return true;
    }
    if (e->ch == QBKEY_BS) {
        self->value = self->value / 10;
        QBNumericInputShowValue(w);
        return true;
    }
    if (e->ch == '-' && self->value > 0) {
        if (self->value <= -self->minValue) {
            self->value = -self->value;
            QBNumericInputShowValue(w);
        }
        return true;
    }
    if (e->ch == '+' && self->value < 0) {
        if (self->value >= -self->maxValue) {
            self->value = -self->value;
            QBNumericInputShowValue(w);
        }
        return true;
    }

    return false;
}

SvLocal void QBNumericInputFocusEventHandler(SvWidget w, SvFocusEvent e)
{
    QBNumericInput self = w->prv;
    if (e->kind == SvFocusEventKind_GET && !self->inputBoxIsActive) {
        if (self->boxFrame) {
            self->params.bitmap = self->active;
            QBFrameSetBitmap(self->boxFrame, &self->params);
        }
        self->inputBoxIsActive = true;
    } else if (e->kind == SvFocusEventKind_LOST && self->inputBoxIsActive) {
        if (self->boxFrame) {
            self->params.bitmap = self->inactive;
            QBFrameSetBitmap(self->boxFrame, &self->params);
        }
        self->inputBoxIsActive = false;
    }
    svWidgetForceRender(self->box);

    if (e->kind != SvFocusEventKind_GET) {
        if (self->value < self->minValue) {
            self->value = self->minValue;
        } else if (self->value > self->maxValue) {
            self->value = self->maxValue;
        } else {
            return;
        }

        QBNumericInputShowValue(w);
    }

    self->valueIsFinished = e->kind == SvFocusEventKind_LOST;
}

SvLocal void QBNumericInputClean(SvApplication app, void *self_)
{
    QBNumericInput self = self_;
    SVTESTRELEASE(self->active);
    SVTESTRELEASE(self->inactive);
    free(self);
}

SvLocal SvWidget QBNumericInputNewFromSettingsType(SvApplication app, const char *widgetName, bool useRatio, unsigned int width, unsigned int height)
{
    QBNumericInput self = calloc(1, sizeof(struct QBNumericInput_));
    SvWidget w = useRatio ? svSettingsWidgetCreateWithRatio(app, widgetName, width, height) : svSettingsWidgetCreate(app, widgetName);
    if (!w) {
        SvLogError("%s() %d", __func__, __LINE__);
        free(self);
        return NULL;
    }
    svWidgetSetFocusable(w, true);
    w->prv = self;
    w->clean = QBNumericInputClean;
    svWidgetSetInputEventHandler(w, QBNumericInputInputEventHandler);
    svWidgetSetFocusEventHandler(w, QBNumericInputFocusEventHandler);

    self->w = w;
    char *buf;

    asprintf(&buf, "%s.Box", widgetName);
    self->active = SVTESTRETAIN(svSettingsGetBitmap(buf, "active"));
    self->inactive = SVTESTRETAIN(svSettingsGetBitmap(buf, "inactive"));

    SvWidget box = NULL;
    box = useRatio ? svSettingsWidgetCreateWithRatio(app, buf, width, height) : svSettingsWidgetCreate(app, buf);
    if (!box) {
        free(buf);
        svWidgetDestroy(w);
        SvLogError("%s() %d", __func__, __LINE__);
        return NULL;
    }
    self->box = box;

    svWidgetAttach(w, box, 0, 0, 1);

    QBFrameParameters params = {
        .width = box->width,
        .height = box->height,
        .bitmap = self->inactive
    };
    free(buf);

    if (params.width <= 0 || params.height <= 0) {
        svWidgetDestroy(w);
        return NULL;
    }
    self->params = params;
    self->boxFrame = QBFrameCreate(app, &self->params);
    svWidgetAttach(self->box, self->boxFrame, 0, 0, 1);

    SvWidget label = NULL;
    asprintf(&buf, "%s.Choice", widgetName);
    label = useRatio ? svLabelNewFromRatio(app, buf, width, height) : svLabelNewFromSM(app, buf);
    if (!label) {
        free(buf);
        SvLogError("%s() %d", __func__, __LINE__);
        svWidgetDestroy(w);
        return NULL;
    }
    self->label = label;
    useRatio ? svSettingsWidgetAttachWithRatio(box, label, buf, 1) : svSettingsWidgetAttach(self->box, label, buf, 1);
    free(buf);

    self->valueIsFinished = true;

    QBNumericInputShowValue(w);
    return w;
}

SvWidget QBNumericInputNewFromSM(SvApplication app, const char *widgetName)
{
    return QBNumericInputNewFromSettingsType(app, widgetName, false, 0, 0);
}

SvWidget QBNumericInputNewFromRatio(SvApplication app, const char *widgetName, unsigned int parentWidth, unsigned int parentHeight)
{
    return QBNumericInputNewFromSettingsType(app, widgetName, true, parentWidth, parentHeight);
}

void QBNumericInputSetValueAndRange(SvWidget w, int initialValue, int minValue, int maxValue)
{
    QBNumericInput self = (QBNumericInput) w->prv;
    self->value = initialValue;
    assert(minValue <= maxValue);
    self->minValue = minValue;
    self->maxValue = maxValue;
    QBNumericInputShowValue(w);
}

void QBNumericInputSetDecimalPartWidth(SvWidget w, unsigned int width)
{
    ((QBNumericInput) w->prv)->decimalPartWidth = width;
    QBNumericInputShowValue(w);
}

int QBNumericInputGetValue(SvWidget w)
{
    return w ? ((QBNumericInput) w->prv)->value : -1;
}
