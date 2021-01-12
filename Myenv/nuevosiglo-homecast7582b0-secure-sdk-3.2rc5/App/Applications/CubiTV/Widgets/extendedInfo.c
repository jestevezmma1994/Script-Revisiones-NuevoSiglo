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

#include "extendedInfo.h"

#include <libintl.h>
#include <SvCore/SvTime.h>
#include <QBInput/QBInputCodes.h>
#include <SWL/viewport.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <Widgets/QBScrollBar.h>
#include <settings.h>
#include <QBWidgets/QBGrid.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Windows/newtv.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <Windows/newtvguide.h>
#include <Utils/QBEventUtils.h>
#include <Logic/timeFormat.h>
#include <Logic/EventsLogic.h>
#include <main.h>
#include <QBPCRatings/QBPCUtils.h>

struct QBExtendedInfo_t {
    SvWidget text, scrollBar, viewPort, title, time;
    QBTextRenderer renderer;

    SvFont font;
    SvColor textColor;
    int fontSize;

    SvArray events;
    int pos, maxTextWidth;
    SvGenericObject content;
    SvDBRawObject vodMetaData;

    int timeOffY, scrollBarOffY, viewportOffY;
    int viewportHeight;

    AppGlobals appGlobals;
    SvWidget grid;
};
typedef struct QBExtendedInfo_t* QBExtendedInfo;

SvLocal void
QBExtendedInfoCleanupContents(QBExtendedInfo prv)
{
    SVTESTRELEASE(prv->events);
    SVTESTRELEASE(prv->content);
    SVTESTRELEASE(prv->vodMetaData);
    prv->events = NULL;
    prv->content = NULL;
    prv->vodMetaData = NULL;
}

SvLocal void
QBExtendedInfoCleanup(SvApplication app, void *prv_){
    QBExtendedInfo prv = prv_;
    QBExtendedInfoCleanupContents(prv);
    free(prv_);
}

SvLocal bool
QBExtendedInfoInputEventHandler(SvWidget w, SvInputEvent event)
{
    QBExtendedInfo prv = w->prv;

    switch(event->ch){
        case QBKEY_RIGHT:
            if(prv->events) {
                int n = SvArrayCount(prv->events);
                if(prv->pos < n-1) {
                    QBExtendedInfoSetPosition(w, prv->pos + 1, false);
                    break;
                }
                if(n>1)
                    break;
            }
            return false;
        case QBKEY_LEFT:
            if(prv->events) {
                if(prv->pos) {
                    QBExtendedInfoSetPosition(w, prv->pos - 1, false);
                    break;
                }
                if(SvArrayCount(prv->events)>1)
                    break;
            }
            return false;
        case QBKEY_DOWN:
        case QBKEY_UP:
            break;
        default:
            return false;
    }
    return true;
}

void
QBExtendedInfoSetText(SvWidget w, SvString title, SvString text){
    QBExtendedInfo prv = w->prv;
    if(!text)
        text = SVSTRING("");
    if(!title)
        title = SVSTRING("");

    QBAsyncLabelSetText(prv->title, title);
    QBAsyncLabelSetText(prv->time, SVSTRING(""));
    QBAsyncLabelSetText(prv->text, text);
}

void
QBExtendedInfoSetPosition(SvWidget w, int position, bool force) {
    QBExtendedInfo prv = w->prv;
    if (!prv->events)
        return;

    if(SvArrayCount(prv->events)){
        prv->pos = position;
        QBExtendedInfoSetTextByEvent(w, (SvEPGEvent)SvArrayAt(prv->events, position), force);
    }else{
        prv->pos = 0;
        QBExtendedInfoSetText(w, NULL, NULL);
    }
}

void
QBExtendedInfoSetEvents(SvWidget w, SvArray events){
    QBExtendedInfo prv = w->prv;
    QBExtendedInfoCleanupContents(prv);
    prv->events = SVTESTRETAIN(events);
    prv->pos = 0;
    QBExtendedInfoSetPosition(w, 0, false);
}

void
QBExtendedInfoRefreshEvents(SvWidget w, SvArray events){
    QBExtendedInfo prv = w->prv;
    SVTESTRELEASE(prv->events);
    prv->events = SVTESTRETAIN(events);
    ssize_t pos = prv->pos;
    size_t eventsCount = SvArrayCount(events);
    if (pos < 0)
        pos = 0;
    else if ((size_t) pos >= eventsCount)
        pos = eventsCount - 1;
    if (!eventsCount) {
        SVTESTRELEASE(prv->content);
        prv->content = NULL;
    }
    QBExtendedInfoSetPosition(w, pos, false);
}

/**
 * Get metadata that can be stored under different attribute names depending on the context (E.g. Traxis and Innov8on metadata are different).
 * NOTE: list of attribute names must be NULL terminated!
 * usage: get_from_meta(meta, "name", "title", NULL)
 */
SvLocal SvGenericObject
get_from_meta(SvDBRawObject meta, const char *attrName1, ...)
{
    va_list ap;
    va_start(ap, attrName1);

    SvGenericObject val = NULL;
    const char *attrName = attrName1;
    while (!val && attrName) {
        val = SvDBRawObjectGetAttrValue(meta, attrName);
        attrName = va_arg(ap, const char*);
    }

    va_end(ap);

    return val;
}

SvLocal
void QBExtendedInfoAddRatingsInfoToText(QBExtendedInfo self, SvValue ratingV, SvStringBuffer buf)
{
    if (!ratingV || !buf)
        return;

    SvErrorInfo error = NULL;
    QBPCList rating = QBPCListCreate(&error);
    if (error)
        goto err;
    QBPCListLockRatingsFromString(rating, SvValueGetString(ratingV),
            &error);
    if (error) {
        SVRELEASE(rating);
        goto err;
    }
    SvString ratingStr = QBParentalControlLogicCreateDescriptionStringFromPCList(self->appGlobals->parentalControlLogic,
                                                                                 rating,
                                                                                 &error);
    if (error) {
        SVRELEASE(rating);
        goto err;
    }
    if (ratingStr) {
        SvStringBufferAppendFormatted(buf, NULL, " (%s)", SvStringCString(ratingStr));
        SVRELEASE(ratingStr);
    }
    SVRELEASE(rating);
err:
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
}

bool
QBExtendedInfoSetTextByVODMetaData(SvWidget w, SvDBRawObject vodMetaData){

    QBExtendedInfo prv = w->prv;
    if (!vodMetaData) {
        QBExtendedInfoCleanupContents(prv);
        QBExtendedInfoSetText(w, NULL, NULL);
        return true;
    }

    SvString title = NULL;
    SvString description = NULL;
    SvStringBuffer buf = SvStringBufferCreate(NULL);

    SVRETAIN(vodMetaData);
    QBExtendedInfoCleanupContents(prv);
    prv->vodMetaData = vodMetaData;

    SvValue titleV = (SvValue) get_from_meta(vodMetaData, "name", "Name", NULL);
    if (titleV)
        SvStringBufferAppendFormatted(buf, NULL, "%s", SvValueGetStringAsCString(titleV, NULL));

    SvValue descV = (SvValue) get_from_meta(vodMetaData, "description", "Description", NULL);
    if (descV)
        description = SvValueGetString(descV);

    SvValue ratingV = (SvValue) get_from_meta(vodMetaData, "ratings", "Ratings", NULL);
    QBExtendedInfoAddRatingsInfoToText(prv, ratingV, buf);

    title = SvStringBufferCreateContentsString(buf, NULL);
    SVRELEASE(buf);
    QBExtendedInfoSetText(w, title, description);
    SVTESTRELEASE(title);

    return (!titleV && !descV);
}

void
QBExtendedInfoReinitialize(SvWidget w) {
    if (!w)
        return;

    QBExtendedInfo prv = w->prv;
    size_t pos = prv->pos;

    if (prv->events || prv->content)
        QBExtendedInfoSetTextByEvent(w, (SvEPGEvent)SvArrayAt(prv->events, pos), true);
    else if (prv->vodMetaData)
        QBExtendedInfoSetTextByVODMetaData(w, prv->vodMetaData);
}

void
QBExtendedInfoSetTextByEvent(SvWidget w, SvEPGEvent event, bool force)
{
    if (!w)
        return;

    QBExtendedInfo prv = w->prv;

    if (!force && SvObjectEquals((SvObject) event, prv->content)) {
        return;
    }

    SVTESTRELEASE(prv->content);
    prv->content = SVTESTRETAIN(event);

    SvString description = NULL;
    SvString title = NULL;
    SvString sDurationTime = NULL;
    if (event) {
        SvStringBuffer buf = SvStringBufferCreate(NULL);
        SvStringBuffer titleBuf = SvStringBufferCreate(NULL);
        const char* categoryName = SvEPGEventGetCategoryName(event);
        if (categoryName) {
            SvStringBufferAppendFormatted(buf, NULL, "%s\n\n", gettext(categoryName));
        }

        SvEPGEventDesc desc = QBEventUtilsGetDescFromEvent(prv->appGlobals->eventsLogic, prv->appGlobals->langPreferences, event);
        if (desc) {
            SvString descStr = desc->description ? desc->description : desc->shortDescription;
            if (descStr)
                SvStringBufferAppendFormatted(buf, NULL, "%s", SvStringCString(descStr));

            if (likely(desc->title)) {
                SvStringBufferAppendString(titleBuf, desc->title, NULL);
            }
            SvString ratingStr = EventsLogicPCRatingToString(prv->appGlobals->eventsLogic, event->PC);
            if (ratingStr) {
                SvStringBufferAppendFormatted(titleBuf, NULL, " (%s)", SvStringCString(ratingStr));
                SVRELEASE(ratingStr);
            }
        }
        description = SvStringBufferCreateContentsString(buf, NULL);
        title = SvStringBufferCreateContentsString(titleBuf, NULL);
        SVRELEASE(buf);
        SVRELEASE(titleBuf);

        sDurationTime = SvEPGEventGetDurationTime(event, QBTimeFormatGetCurrent()->extendedInfoTime);
    }

    QBExtendedInfoSetText(w, title, description);
    QBAsyncLabelSetText(prv->time, sDurationTime);

    SVTESTRELEASE(sDurationTime);
    SVTESTRELEASE(description);
    SVTESTRELEASE(title);

    svViewPortSetContentsPosition(prv->viewPort, 0, 0, true);
}

void QBExtendedInfoSetFocus(SvWidget w){
    QBExtendedInfo prv = w->prv;
    svWidgetSetFocus(prv->viewPort);
}

void QBExtendedInfoResetPosition(SvWidget w){
    QBExtendedInfo prv = w->prv;
    svViewPortSetContentsPosition(prv->viewPort, 0, 0, true);
}

SvLocal void QBExtendedInfoResizeCallback(SvWidget grid, void *_prv)
{
    QBExtendedInfo prv = (QBExtendedInfo)_prv;
    svViewPortContentsSizeChanged(prv->viewPort);
}

SvWidget QBExtendedInfoNew(SvApplication app, const char *name, AppGlobals appGlobals)
{
    SvWidget w = svSettingsWidgetCreate(app, name);
    QBExtendedInfo prv = calloc(1, sizeof(struct QBExtendedInfo_t));
    QBTextRenderer renderer = appGlobals->textRenderer;
    char buf[128];
    w->prv = prv;
    svWidgetSetInputEventHandler(w, QBExtendedInfoInputEventHandler);
    w->clean = QBExtendedInfoCleanup;

    snprintf(buf, sizeof(buf), "%s.%s", name, "ScrollBar");
    prv->scrollBar = QBScrollBarNew(app, buf);
    svSettingsWidgetAttach(w, prv->scrollBar, buf, 1);
    prv->appGlobals = appGlobals;

    prv->grid = QBGridCreate(app, "Grid");

    snprintf(buf, sizeof(buf), "%s.%s", name, "Title");
    prv->title = QBGridAddAsyncLabel(prv->grid, buf, renderer);

    snprintf(buf, sizeof(buf), "%s.%s", name, "Time");
    prv->time = QBGridAddAsyncLabel(prv->grid, buf, renderer);

    snprintf(buf, sizeof(buf), "%s.%s", name, "Text");
    prv->text = QBGridAddAsyncLabel(prv->grid, buf, renderer);

    snprintf(buf, sizeof(buf), "%s.%s", name, "ViewPort");
    prv->viewPort = svViewPortNew(app, buf);
    svSettingsWidgetAttach(w, prv->viewPort, buf, 2);

    svViewPortSetContents(prv->viewPort, prv->grid);
    svViewPortAddRangeListener(prv->viewPort,
            QBScrollBarGetRangeListener(prv->scrollBar),
            SvViewPortRangeOrientation_VERTICAL);

    QBExtendedInfoSetText(w, NULL, NULL);

    svWidgetSetFocusable(prv->viewPort, true);

    QBGridSetResizeCallback(prv->grid, QBExtendedInfoResizeCallback, w->prv);
    return w;
}

void QBExtendedInfoStartScrollingVertically(SvWidget w, bool downDirection){
    QBExtendedInfo prv = w->prv;
    svViewPortStartScrollingVertically(prv->viewPort, downDirection);
}

void QBExtendedInfoStopScrollingVertically(SvWidget w){
    QBExtendedInfo prv = w->prv;
    svViewPortStopScrollingVertically(prv->viewPort);
}

bool QBExtendedInfoCanBeScrolledVertically(SvWidget w, bool downDirection){
    QBExtendedInfo prv = w->prv;
    return (svViewPortCanBeScrolledVertically(prv->viewPort, downDirection));
}
