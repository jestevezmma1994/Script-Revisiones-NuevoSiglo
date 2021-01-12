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

#include "QBTimeEditBox.h"

#include <SvCore/SvTime.h>
#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <settings.h>
#include <main.h>
#include <QBWidgets/QBComboBox.h>
#include <Logic/timeFormat.h>

struct QBTimeEditBox_t {
    SvWidget w;
    SvWidget comboBox;
    int timeStep;
    int inputPos;
    struct {
        void* target;
        QBTimeEditBoxInputCallback input;
        QBTimeEditBoxPrepareDataCallback prepare;
        QBTimeEditBoxChangeCallback change;
    } callback;
};

SvLocal SvString
QBTimeEditBoxComboBoxPrepareTime(void *target, SvWidget combobox, SvGenericObject value)
{
    QBTimeEditBox self = ((SvWidget) target)->prv;

    int timePoint = SvValueGetInteger((SvValue) value);
    struct tm tm = {
        .tm_min = timePoint % 60,
        .tm_hour = timePoint / 60,
    };
    if ( self->callback.prepare ) {
        SvString ret = NULL;
        if( (ret = self->callback.prepare(self->callback.target, target, value)) ) {
            return ret;
        }
    }
    char buf[100];
    strftime(buf, sizeof(buf), QBTimeFormatGetCurrent()->timeEditBox, &tm);
    return SvStringCreate(buf, NULL);
}


SvLocal bool
QBTimeEditBoxComboBoxInputTime(void *target, SvWidget combobox, SvInputEvent e)
{
    const int key = e->ch;
    QBTimeEditBox self = (QBTimeEditBox) (((SvWidget) target)->prv);

    SvValue value = (SvValue)QBComboBoxGetValue(combobox);
    if (self->callback.input) {
        if (self->callback.input(self->callback.target, target, e)) {
            return true;
        }
    }

    if (key < '0' || key > '9') {
        return false;
    }
    int digit = key - '0';
    const int TimeMax = 24 * 60 - 1;
    int TimeValue = self->inputPos ? SvValueGetInteger(value) : 0;
    int mul[4] = { 10*60, 60, 10, 1 };
    int max[4] = { 2, 9, 5, 9 };

    if ( !self->inputPos && digit > 2 ) {
        self->inputPos++;
    }

    if (digit > max[self->inputPos]) {
        return true;
    }

    TimeValue = TimeValue + digit * mul[self->inputPos];
    if (TimeValue > TimeMax) {
        return true;
    }

    self->inputPos = (self->inputPos + 1) % 4;

    value = SvValueCreateWithInteger(TimeValue, NULL);
    QBComboBoxSetValue(combobox, (SvGenericObject) value);
    SVRELEASE(value);

    return true;
}

SvLocal int
QBTimeEditBoxComboBoxChangeTime(void * target, SvWidget combobox,
    SvGenericObject value, int key)
{
    QBTimeEditBox self = (QBTimeEditBox) (((SvWidget) target)->prv);
    if ( self->callback.change ) {
        int ret = self->callback.change(self->callback.target, target, value, key);
        if ( ret ) {
            return ret;
        }
    }
    self->inputPos = 0;
    int timePoint = SvValueGetInteger((SvValue)value);
    timePoint += key == QBKEY_RIGHT ? self->timeStep : -1;
    return timePoint / self->timeStep;
}


void
QBTimeEditBoxSetFocus(SvWidget w)
{
    QBTimeEditBox self = (QBTimeEditBox) w->prv;
    svWidgetSetFocus(self->comboBox);
}

bool
QBTimeEditBoxIsFocused(SvWidget w)
{
    QBTimeEditBox self = (QBTimeEditBox) w->prv;
    return svWidgetIsFocused(self->comboBox);
}

int
QBTimeEditBoxGetMinutes(SvWidget w)
{
    QBTimeEditBox self = (QBTimeEditBox) w->prv;
    SvValue value = (SvValue)QBComboBoxGetValue(self->comboBox);
    return SvValueGetInteger(value);
}

void
QBTimeEditBoxSetMinutes(SvWidget w, int m)
{
    QBTimeEditBox self = (QBTimeEditBox) w->prv;
    SvValue value = SvValueCreateWithInteger(m, NULL);
    QBComboBoxSetValue(self->comboBox, (SvGenericObject)value);
    SVRELEASE(value);
}

void
QBTimeEditBoxSetTime(SvWidget w, time_t t)
{
    struct tm tm;
    SvTimeBreakDown(SvTimeConstruct(t, 0), true, &tm);
    QBTimeEditBoxSetMinutes(w, tm.tm_hour * 60 + tm.tm_min);
}

void
QBTimeEditBoxSetCallbacks(SvWidget w, void *target, QBComboBoxPrepareDataCallback prepare,
    QBComboBoxInputCallback input, QBComboBoxChangeCallback change)
 {
    if ( !target ) {
        return;
    }
    QBTimeEditBox self = (QBTimeEditBox)w->prv;
    self->callback.target = target;
    self->callback.prepare = prepare;
    self->callback.input = input;
    self->callback.change = change;
 }

SvLocal SvWidget
QBTimeEditBoxNewInteral(SvApplication app, const char *widgetName, bool useRatio, unsigned int parentWidth, unsigned int parentHeight)
{
    SvWidget w = useRatio ? svSettingsWidgetCreateWithRatio(app, widgetName, parentWidth, parentHeight) : svSettingsWidgetCreate(app, widgetName);
    QBTimeEditBox prv = calloc(sizeof(*prv), 1);

    w->prv = prv;
    char nameBuf[1024];
    snprintf(nameBuf, sizeof(nameBuf), "%s.comboBox", widgetName);
    if (useRatio) {
        prv->comboBox = QBComboBoxNewFromRatio(app, nameBuf, w->width, w->height);
    } else {
        prv->comboBox = QBComboBoxNewFromSM(app, nameBuf);
    }
    svWidgetSetFocusable(prv->comboBox, true);
    QBComboBoxCallbacks cb = { QBTimeEditBoxComboBoxPrepareTime, QBTimeEditBoxComboBoxInputTime, QBTimeEditBoxComboBoxChangeTime, NULL, NULL, NULL };
    QBComboBoxSetCallbacks(prv->comboBox, w, cb);

    SvArray values = SvArrayCreate(NULL);
    prv->timeStep = 5;
    for( int idx = 0; idx < 24*60; idx += prv->timeStep ) {
        SvGenericObject v =(SvGenericObject)SvValueCreateWithInteger(idx, NULL);
        SvArrayAddObject(values, v);
        SVRELEASE(v);
    }
    QBComboBoxSetContent(prv->comboBox, values);
    SVTESTRELEASE(values);

    if (useRatio) {
        svSettingsWidgetAttachWithRatio(w, prv->comboBox, nameBuf, 1);
    } else {
        svSettingsWidgetAttach(w, prv->comboBox, nameBuf, 1);
    }

    return w;
}

SvWidget
QBTimeEditBoxNew(SvApplication app, const char *widgetName)
{
    return QBTimeEditBoxNewInteral(app, widgetName, false, 0, 0);
}

SvWidget
QBTimeEditBoxNewFromRatio(SvApplication app, const char *widgetName, unsigned int parentWidth, unsigned int parentHeight)
{
    return QBTimeEditBoxNewInteral(app, widgetName, true, parentWidth, parentHeight);
}
