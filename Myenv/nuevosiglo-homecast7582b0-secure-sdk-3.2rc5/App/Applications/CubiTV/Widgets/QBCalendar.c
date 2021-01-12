/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

/*
 * QBCalendar.c
 *
 *  Created on: Feb 7, 2011
 *      Author: Rafał Duszyński
 */

#include "QBCalendar.h"
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvTime.h>
#include <SvCore/QBDate.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvFoundation/SvArray.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/QBUIEvent.h>
#include <QBInput/QBInputCodes.h>
#include <settings.h>
#include <SWL/QBFrame.h>
#include <QBOSK/QBOSKMap.h>
#include <QBOSK/QBOSK.h>
#include <QBOSK/QBOSKEvents.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <string.h>
#include <assert.h>


typedef struct QBCalendar_ *QBCalendar;
struct QBCalendar_ {
    struct SvObject_ super_;

    SvWidget calendar;
    SvWidget dateLabel;
    SvString labelTimeFormat;
    SvWidget selectorOSK;
    SvBitmap sundayBg;

    QBOSKRenderer renderer;
    QBOSKMap map;

    struct tm startDate;
    struct tm endDate;
    struct tm currentDate;
    int firstDay;
    int lastDay;
};


SvType QBCalendarDateSelectedEvent_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBCalendarDateSelectedEvent",
                            sizeof(struct QBCalendarDateSelectedEvent_),
                            QBUIEvent_getType(),
                            &type,
                            NULL);
    }

    return type;
}

SvLocal void QBCalendarHandlePeerEvent(SvObject self_, QBPeerEvent event_, SvObject sender);

SvLocal void QBCalendarDestroy(void *self_)
{
    QBCalendar self = self_;

    assert(!self->calendar);

    SVTESTRELEASE(self->sundayBg);
    SVRELEASE(self->labelTimeFormat);
    SVTESTRELEASE(self->map);
    SVTESTRELEASE(self->renderer);
}

SvLocal SvType QBCalendar_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBCalendarDestroy
    };
    static const struct QBPeerEventReceiver_ eventReceiverMethods = {
        .handleEvent = QBCalendarHandlePeerEvent
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBCalendar",
                            sizeof(struct QBCalendar_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBCalendarCleanup(SvApplication app, void *self_)
{
    QBCalendar self = self_;

    self->calendar = NULL;
    SVRELEASE(self);
}

SvLocal int QBCalendarGetNumberOfDays(int year, int month)
{
    static const int numbers[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    year += 1900;

    if (month == 1)
        if ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0))
            return 29;
    return numbers[month];
}

SvLocal int QBCalendarGetDayOfWeek(int year, int month, int day)
{
    struct tm temptm = {
        .tm_year = year,
        .tm_mon = month,
        .tm_mday = day,
    };

    mktime(&temptm); // updates temptm.tm_wday

    if (temptm.tm_wday == 0)
        return 6;
    else
        return temptm.tm_wday - 1;
}

SvLocal int QBCalendarCompareDate(struct tm *first, struct tm *second)
{
    if (first->tm_year == 0 || second->tm_year == 0)
        return 0;

    if (first->tm_year < second->tm_year)
        return -1;
    if (first->tm_year > second->tm_year)
        return 1;

    if (first->tm_mon < second->tm_mon)
        return -1;
    if (first->tm_mon > second->tm_mon)
        return 1;

    if (first->tm_mday < second->tm_mday)
        return -1;
    if (first->tm_mday > second->tm_mday)
        return 1;

    return 0;
}

SvLocal bool QBCalendarDatePrev(QBCalendar self, struct tm *newDate)
{
    if (self->currentDate.tm_mon == 0) {
        newDate->tm_year = self->currentDate.tm_year - 1;
        newDate->tm_mon = 11;
    } else {
        newDate->tm_year = self->currentDate.tm_year;
        newDate->tm_mon = self->currentDate.tm_mon - 1;
    }
    newDate->tm_mday = QBCalendarGetNumberOfDays(newDate->tm_year, newDate->tm_mon);

    if (QBCalendarCompareDate(newDate, &self->startDate) >= 0)
        return true;
    else
        return false;
}

SvLocal bool QBCalendarDateNext(QBCalendar self, struct tm *newDate)
{
    if (self->currentDate.tm_mon == 11) {
        newDate->tm_year = self->currentDate.tm_year + 1;
        newDate->tm_mon = 0;
    } else {
        newDate->tm_year = self->currentDate.tm_year;
        newDate->tm_mon = self->currentDate.tm_mon + 1;
    }
    newDate->tm_mday = 1;

    if (QBCalendarCompareDate(newDate, &self->endDate) <= 0)
        return true;
    else
        return false;
}

SvLocal bool QBCalendarIsKeyInRange(QBCalendar self, int keyIndex)
{
    struct tm keyDate;
    memcpy(&keyDate, &self->currentDate, sizeof(struct tm));
    keyDate.tm_mday = keyIndex - self->firstDay + 1;

    if (QBCalendarCompareDate(&keyDate, &self->startDate) >= 0 && QBCalendarCompareDate(&keyDate, &self->endDate) <= 0)
        return true;
    else
        return false;
}

SvLocal void QBCalendarNotify(QBCalendar self, QBDate selectedDate)
{
    QBCalendarDateSelectedEvent event = (QBCalendarDateSelectedEvent) SvTypeAllocateInstance(QBCalendarDateSelectedEvent_getType(), NULL);
    if (event) {
        event->selectedDate = selectedDate;
        QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
        QBEventBusPostEvent(eventBus, (QBPeerEvent) event, (SvObject) self, NULL);
        SVRELEASE(event);
    }
}

SvLocal bool QBCalendarInputEventHandler(SvWidget calendar, SvInputEvent ev)
{
    QBCalendar self = calendar->prv;
    SvString keyID = QBOSKGetFocusedKeyID(self->selectorOSK);
    int currentKey = atoi(SvStringCString(keyID));
    struct tm newDate;

    memset(&newDate, 0, sizeof(newDate));

    if (ev->ch == QBKEY_UP) {
        if (QBCalendarDatePrev(self, &newDate))
            QBCalendarSetCurrentDate(calendar, &newDate, NULL, NULL);
        return true;
    } else if (ev->ch == QBKEY_DOWN) {
        if (QBCalendarDateNext(self, &newDate))
            QBCalendarSetCurrentDate(calendar, &newDate, NULL, NULL);
        return true;
    } else if (ev->ch == QBKEY_LEFT) {
        if (currentKey == self->firstDay) {
            if (QBCalendarDatePrev(self, &newDate))
                QBCalendarSetCurrentDate(calendar, &newDate, NULL, NULL);
        } else if (QBCalendarIsKeyInRange(self, currentKey - 1)) {
            SvString newKey = SvStringCreateWithFormat("%i", currentKey - 1);
            QBOSKSetFocusToKey(self->selectorOSK, newKey);
            SVRELEASE(newKey);
        }
        return true;
    } else if (ev->ch == QBKEY_RIGHT) {
        if (currentKey == self->lastDay) {
            if (QBCalendarDateNext(self, &newDate))
                QBCalendarSetCurrentDate(calendar, &newDate, NULL, NULL);
        } else if (QBCalendarIsKeyInRange(self, currentKey + 1)) {
            SvString newKey = SvStringCreateWithFormat("%i", currentKey + 1);
            QBOSKSetFocusToKey(self->selectorOSK, newKey);
            SVRELEASE(newKey);
        }
        return true;
    }

    return false;
}

SvLocal void QBCalendarHandlePeerEvent(SvObject self_, QBPeerEvent event_, SvObject sender)
{
    QBCalendar self = (QBCalendar) self_;

    if (SvObjectIsInstanceOf((SvObject) event_, QBOSKKeyPressedEvent_getType())) {
        QBOSKKeyPressedEvent event = (QBOSKKeyPressedEvent) event_;

        if (!event->super_.key)
            return;

        SvString keyID = event->super_.key->ID;
        int currentKey = atoi(SvStringCString(keyID));
        QBDate selectedDate = {
            .mday = currentKey - self->firstDay + 1,
            .mon  = self->currentDate.tm_mon,
            .year = self->currentDate.tm_year + 1900
        };
        QBCalendarNotify(self, selectedDate);
    }
}

SvLocal void QBCalendarFocusEventHandler(SvWidget w, SvFocusEvent ev)
{
    QBCalendar self = w->prv;
    if (ev->kind == SvFocusEventKind_GET) {
        svWidgetSetFocus(self->selectorOSK);
        return;
    }
}

SvLocal QBOSKKey QBCalendarKeyCreate(QBCalendar self, int keyNum, int keyCaption, bool enabled, SvBitmap altBg)
{
    SvString key = SvStringCreateWithFormat("%i", keyNum);
    if (!key)
        return NULL;
    QBOSKKey currentKey = QBOSKKeyCreate(key, 1, NULL);
    SVRELEASE(key);
    if (!currentKey)
        return NULL;
    if (altBg)
        currentKey->keyBg = SVRETAIN(altBg);
    currentKey->variants[0].disabled = !enabled;
    currentKey->variants[0].position.x = keyNum % 7;
    currentKey->variants[0].position.y = keyNum / 7;
    currentKey->variants[0].value = enabled ? SvStringCreateWithFormat("%i", keyCaption) : SvStringCreate("0", NULL);
    currentKey->variants[0].caption = SvStringCreateWithFormat("%i", keyCaption);

    return currentKey;
}

SvLocal QBOSKMap QBCalendarCreateMap(QBCalendar self, SvErrorInfo *errorOut)
{
    QBOSKMap map = NULL;
    SvArray layouts = NULL;
    SvString layout = SvStringCreate("basic", NULL);
    int total = 0;
    int curDays, lastMonthFrom;
    QBOSKKey currentKey = NULL;
    SvErrorInfo error = NULL;

    if (!layout) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory, "Allocation error");
        goto errout;
    }

    layouts = SvArrayCreateWithCapacity(1, NULL);

    if (!layouts) {
        SVRELEASE(layout);
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory, "Allocation error");
        goto errout;
    }

    SvArrayAddObject(layouts, (SvGenericObject) layout);
    SVRELEASE(layout);

    map = QBOSKMapCreate(layout, (SvImmutableArray) layouts, &error);
    if (error)
        goto errout;
    SVRELEASE(layouts);

    self->firstDay = QBCalendarGetDayOfWeek(self->currentDate.tm_year, self->currentDate.tm_mon, 1);
    self->lastDay = QBCalendarGetNumberOfDays(self->currentDate.tm_year, self->currentDate.tm_mon) + self->firstDay - 1;

    if (self->currentDate.tm_mon == 0)
        lastMonthFrom = QBCalendarGetNumberOfDays(self->currentDate.tm_year - 1, 11) - self->firstDay + 1;
    else
        lastMonthFrom = QBCalendarGetNumberOfDays(self->currentDate.tm_year, self->currentDate.tm_mon - 1) - self->firstDay + 1;

    curDays = QBCalendarGetNumberOfDays(self->currentDate.tm_year, self->currentDate.tm_mon);

    while (total < self->firstDay) {
        currentKey = QBCalendarKeyCreate(self, total, lastMonthFrom + total, false, NULL);
        QBOSKMapAddKey(map, currentKey, &error);
        if (error)
            goto errout;
        SVRELEASE(currentKey);
        total++;
    }
    while (total < (self->firstDay + curDays)) {
        if ((total % 7) == 6)
            currentKey = QBCalendarKeyCreate(self, total, total - self->firstDay + 1, QBCalendarIsKeyInRange(self, total), self->sundayBg);
        else
            currentKey = QBCalendarKeyCreate(self, total, total - self->firstDay + 1, QBCalendarIsKeyInRange(self, total), NULL);
        QBOSKMapAddKey(map, currentKey, &error);
        if (error)
            goto errout;
        SVRELEASE(currentKey);
        total++;
    }
    while (total < 42) {
        currentKey = QBCalendarKeyCreate(self, total, total - self->firstDay - curDays + 1, false, NULL);
        QBOSKMapAddKey(map, currentKey, &error);
        if (error)
            goto errout;
        SVRELEASE(currentKey);
        total++;
    }

    return map;

errout:
    SVTESTRELEASE(layouts);
    SVTESTRELEASE(currentKey);
    SVTESTRELEASE(map);
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

SvWidget QBCalendarNew(SvApplication app, const char *name, QBTextRenderer txtRenderer, SvErrorInfo *errorOut)
{
    const size_t tmpNameSize = strlen(name) + 30;
    char tmpName[tmpNameSize];
    int i, addOffset;
    int margin, yOffset, xOffset;
    QBCalendar self;
    SvErrorInfo error;
    char weekDay[10];
    struct tm tempTime;
    SvWidget wDayBg;
    SvWidget wDayLabel;
    SvWidget tempWidget;
    const char *timeStr;

    self = (QBCalendar) SvTypeAllocateInstance(QBCalendar_getType(), &error);

    if (error) {
        SvErrorInfoPropagate(error, errorOut);
        return NULL;
    }

    self->sundayBg = SVRETAIN(svSettingsGetBitmap(name, "sundayBg"));
    self->calendar = svSettingsWidgetCreate(app, name);
    self->calendar->prv = self;
    self->calendar->clean = QBCalendarCleanup;
    svWidgetSetInputEventHandler(self->calendar, QBCalendarInputEventHandler);
    svWidgetSetFocusEventHandler(self->calendar, QBCalendarFocusEventHandler);
    svWidgetSetFocusable(self->calendar, true);

    // Boundaries set to prevent time_t from overflowing on 32 bit machines
    self->startDate.tm_year = 71;
    self->startDate.tm_mon = 0;
    self->startDate.tm_mday = 1;

    self->endDate.tm_year = 137;
    self->endDate.tm_mon = 11;
    self->endDate.tm_mday = 31;

    snprintf(tmpName, tmpNameSize, "%s.date", name);
    tempWidget = QBFrameCreateFromSM(app, tmpName);
    svSettingsWidgetAttach(self->calendar, tempWidget, tmpName, 0);

    snprintf(tmpName, tmpNameSize, "%s.date.label", name);
    self->dateLabel = QBAsyncLabelNew(app, tmpName, txtRenderer);
    svSettingsWidgetAttach(tempWidget, self->dateLabel, tmpName, 0);
    timeStr = svSettingsGetString(tmpName, "timeFormat");
    if (timeStr)
        self->labelTimeFormat = SvStringCreate(timeStr, NULL);
    else
        self->labelTimeFormat = SvStringCreate("%B %Y", NULL);

    snprintf(tmpName, tmpNameSize, "%s.dayOfWeek", name);
    margin = svSettingsGetInteger(tmpName, "margin", 8);
    yOffset = svSettingsGetInteger(tmpName, "yOffset", 0);
    xOffset = svSettingsGetInteger(tmpName, "xOffset", 0);
    memset(&tempTime, 0, sizeof(struct tm));
    for(i = 0; i < 7; i++) {
        wDayBg = svSettingsWidgetCreate(app, tmpName);
        addOffset = xOffset + margin + (i * (wDayBg->width + margin));
        svWidgetAttach(self->calendar, wDayBg, addOffset, yOffset, 0);
        wDayLabel = QBAsyncLabelNew(app, tmpName, txtRenderer);
        svWidgetAttach(wDayBg, wDayLabel, 0, 0, 0);
        tempTime.tm_wday = (i + 1) % 7;
        strftime(weekDay, 10, "%a", &tempTime);
        QBAsyncLabelSetCText(wDayLabel, weekDay);
    }

    snprintf(tmpName, tmpNameSize, "%s.OSK", name);
    self->selectorOSK = QBOSKNew(app, tmpName, NULL);
    svSettingsWidgetAttach(self->calendar, self->selectorOSK, tmpName, 0);

    QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
    QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, QBOSKKeyPressedEvent_getType(), self->selectorOSK->prv, NULL);

    return self->calendar;
}

void QBCalendarSetLimitDates(SvWidget Calendar, struct tm *MinDate, struct tm *MaxDate)
{
    QBCalendar self = Calendar->prv;

    if (MinDate == NULL)
        self->startDate.tm_year = 0;
    else {
        self->startDate.tm_year = MinDate->tm_year;
        self->startDate.tm_mon = MinDate->tm_mon;
        self->startDate.tm_mday = MinDate->tm_mday;
    }

    if (MaxDate == NULL)
        self->endDate.tm_year = 0;
    else {
        self->endDate.tm_year = MaxDate->tm_year;
        self->endDate.tm_mon = MaxDate->tm_mon;
        self->endDate.tm_mday = MaxDate->tm_mday;
    }
}

void QBCalendarSetCurrentDate(SvWidget Calendar, struct tm *CurrentDate, QBOSKRenderer renderer, SvErrorInfo *errorOut)
{
    QBCalendar self = Calendar->prv;
    SvString layoutID = SvStringCreate("basic", NULL);
    SvErrorInfo error = NULL;
    SvString selectedKeyID = NULL;
    SvString todayKeyID = NULL;
    QBOSKMap newMap;
    struct tm today;
    char labelText[30];

    if (!layoutID) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory, "Allocation error");
        goto errout;
    }

    if (CurrentDate) {
        self->currentDate.tm_year = CurrentDate->tm_year;
        self->currentDate.tm_mon = CurrentDate->tm_mon;
        self->currentDate.tm_mday = CurrentDate->tm_mday;
    } else {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "Cannot set empty date");
        goto errout;
    }
    newMap = QBCalendarCreateMap(self, &error);
    if (error)
        goto errout;

    if (renderer) {
        self->renderer = SVRETAIN(renderer);
        QBOSKSetKeyboardMap(self->selectorOSK, renderer, newMap, layoutID, &error);
    } else {
        QBOSKSetKeyboardMap(self->selectorOSK, self->renderer, newMap, layoutID, &error);
    }
    SVRELEASE(layoutID);
    if (error)
        goto errout;
    SVTESTRELEASE(self->map);
    self->map = newMap;
    strftime(labelText, 30, SvStringCString(self->labelTimeFormat), CurrentDate);
    QBAsyncLabelSetCText(self->dateLabel, labelText);
    SvTimeBreakDown(SvTimeConstruct(SvTimeNow(), 0), true, &today);
    if (today.tm_mon == CurrentDate->tm_mon)
        todayKeyID = SvStringCreateWithFormat("%i", self->firstDay + today.tm_mday - 1);

    if (CurrentDate->tm_mday == 0) {
        if (today.tm_mon == CurrentDate->tm_mon)
            selectedKeyID = SvStringCreateWithFormat("%i", self->firstDay + today.tm_mday - 1);
        else
            selectedKeyID = SvStringCreateWithFormat("%i", self->firstDay);
    } else {
        selectedKeyID = SvStringCreateWithFormat("%i", self->firstDay + CurrentDate->tm_mday - 1);
    }
    QBOSKSetFocusToKey(self->selectorOSK, selectedKeyID);
    SVRELEASE(selectedKeyID);

    if (todayKeyID) {
        QBOSKSetKeyActive(self->selectorOSK, todayKeyID, true, &error);
        SVRELEASE(todayKeyID);
        if (error)
            goto errout;
    }
    return;

errout:
    SvErrorInfoPropagate(error, errorOut);
}
