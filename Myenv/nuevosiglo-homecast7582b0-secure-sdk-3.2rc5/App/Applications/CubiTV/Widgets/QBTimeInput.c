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

#include "QBTimeInput.h"

#include <SvFoundation/SvValue.h>
#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <settings.h>
#include <Windows/newtv.h>
#include <QBWidgets/QBComboBox.h>
#include <Logic/timeFormat.h>

struct QBTimeInput_t {
    SvWidget w;
    SvWidget comboBox;
    int timeStep;
    int inputPos;
};
typedef struct QBTimeInput_t *QBTimeInput;

SvLocal SvString QBTimeInputComboBoxPrepareTime(void *self_, SvWidget combobox, SvGenericObject value)
{
    int timePoint = SvValueGetInteger((SvValue) value);
    struct tm tm = {
        .tm_min = timePoint % 60,
        .tm_hour = timePoint / 60,
    };
    char buf[100];
    strftime(buf, sizeof(buf), QBTimeFormatGetCurrent()->recordingEditPaneTime, &tm);
    return SvStringCreate(buf, NULL);
}


SvLocal bool QBTimeInputComboBoxInputTime(void *target, SvWidget combobox, SvInputEvent e)
{
    const int key = e->ch;
    QBTimeInput self = target;
    if (key < '0' || key > '9')
        return false;
    int digit = key - '0';

    const int TimeMax = 24 * 60 - 1;

    SvValue value = (SvValue)QBComboBoxGetValue(combobox);
    int TimeValue = self->inputPos ? SvValueGetInteger(value) : 0;
    int mul[4] = {10*60, 60, 10, 1};
    int max[4] = {2, 9, 5, 9};

    if(!self->inputPos && digit > 2)
        self->inputPos++;

    if(digit > max[self->inputPos])
        return true;

    TimeValue = TimeValue + digit * mul[self->inputPos];
    if (TimeValue > TimeMax)
        return true;

    self->inputPos = (self->inputPos + 1) % 4;

    value = SvValueCreateWithInteger(TimeValue, NULL);
    QBComboBoxSetValue(combobox, (SvGenericObject) value);
    SVRELEASE(value);

    return true;
}

SvLocal int QBTimeInputComboBoxChangeTime(void * target, SvWidget combobox, SvGenericObject value, int key)
{
    QBTimeInput self = target;
    self->inputPos = 0;
    int timePoint = SvValueGetInteger((SvValue)value);
    timePoint += key == QBKEY_RIGHT ? self->timeStep : -1;
    return timePoint / self->timeStep;
}


void
QBTimeInputSetFocus(SvWidget w)
{
    QBTimeInput prv = (QBTimeInput)w->prv;
    svWidgetSetFocus(prv->comboBox);
}


SvWidget
QBTimeInputNew(SvApplication app, unsigned int resolution, const char *widgetName)
{
    SvWidget w = svSettingsWidgetCreate(app, widgetName);
    QBTimeInput prv = calloc(sizeof(*prv), 1);

    w->prv = prv;
    char nameBuf[1024];
    snprintf(nameBuf, sizeof(nameBuf), "%s.comboBox", widgetName);
    prv->comboBox = QBComboBoxNewFromSM(app, nameBuf);
    svWidgetSetFocusable(prv->comboBox, true);
    QBComboBoxCallbacks cb = { QBTimeInputComboBoxPrepareTime, QBTimeInputComboBoxInputTime, QBTimeInputComboBoxChangeTime, NULL, NULL, NULL };
    QBComboBoxSetCallbacks(prv->comboBox, prv, cb);

    SvArray values = SvArrayCreate(NULL);
    int idx=0;
    prv->timeStep = resolution;
    for(idx=0; idx<24*60; idx+=prv->timeStep) {
        SvGenericObject v =(SvGenericObject)SvValueCreateWithInteger(idx, NULL);
        SvArrayAddObject(values, v);
        SVRELEASE(v);
    }
    QBComboBoxSetContent(prv->comboBox, values);
    SVTESTRELEASE(values);

    svSettingsWidgetAttach(w, prv->comboBox, nameBuf, 1);

    return w;
}

int
QBTimeEditGetMinutes(SvWidget w)
{
    QBTimeInput prv = (QBTimeInput)w->prv;
    SvValue value = (SvValue)QBComboBoxGetValue(prv->comboBox);
    return SvValueGetInteger(value);
}

void
QBTimeEditSetMinutes(SvWidget w, int m)
{
    QBTimeInput prv = (QBTimeInput)w->prv;
    SvValue value = SvValueCreateWithInteger(m, NULL);
    QBComboBoxSetValue(prv->comboBox, (SvGenericObject)value);
    SVRELEASE(value);
}

void
QBTimeEditSetTime(SvWidget w, time_t t)
{
    struct tm tm;
    SvTimeBreakDown(SvTimeConstruct(t, 0), true, &tm);
    QBTimeEditSetMinutes(w, tm.tm_hour * 60 + tm.tm_min);
}
