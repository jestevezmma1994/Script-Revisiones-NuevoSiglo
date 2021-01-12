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

#include "QBLogWindowManager.h"

#include <stdlib.h> // free

#if SV_LOG_LEVEL > 0

#include <QBGlobalWindowManager.h>
#include <QBApplicationController.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/label.h>
#include <main.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvLogQueue.h>

#define log_state(fmt, ...) do { if (0) { SvLogNotice( COLBEG() " QBLogQueue :: " fmt COLEND_COL(green), ##__VA_ARGS__); } } while (0)

#define QB_LOG_QUEUE_FIBER_DELAY_MS  200

SvLocal SvWidget qbLogQueueNew(SvApplication app, const char* widgetName);
SvLocal void qbLogQueueAddLog(SvWidget w, const SvLogQueueElement* log);
SvLocal void qbLogQueueStep(void *arg);



struct QBLogWindowManager_s
{
    struct QBGlobalWindow_t super_;
    AppGlobals appGlobals;
    SvWidget logQueue;
    SvFiber fiber;
    SvFiberTimer fiberTimer;
};


SvLocal void QBLogWindowManagerDestroy(void *ptr)
{
    log_state("%s()", __func__);
    QBLogWindowManager self = (QBLogWindowManager) ptr;
    QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, (QBGlobalWindow) self);
}

SvLocal SvType QBLogWindowManager_getType(void)
{
    static const struct QBWindowVTable_ vtable = {
        .super_      = {
            .destroy = QBLogWindowManagerDestroy
        }
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBLogWindowManager",
                            sizeof(struct QBLogWindowManager_s),
                            QBGlobalWindow_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            NULL);
    }
    return type;
}

SvLocal void QBLogWindowManagerFakeClean(SvApplication app, void *ptr)
{
    // all child widgets are always attached, so automatically destroyed
}

QBGlobalWindow QBLogWindowManagerCreate(AppGlobals appGlobals)
{
    SvApplication app = appGlobals->res;

    QBLogWindowManager self = (QBLogWindowManager) SvTypeAllocateInstance(QBLogWindowManager_getType(), NULL);
    QBGlobalWindow super = (QBGlobalWindow) self;

    svSettingsPushComponent("logWindow.settings");
    {
        SvWindow window = svSettingsWidgetCreate(app, "logWindow");
        QBGlobalWindowInit(super, window, SVSTRING("QBLogWindowManager"));
        SvWidget logQueue = qbLogQueueNew(app, "logqueue");
        svSettingsWidgetAttach(super->window, logQueue, "logqueue", 10000);
        self->logQueue = logQueue;
    };
    svSettingsPopComponent();

    super->window->prv = self;
    super->window->clean = QBLogWindowManagerFakeClean;
    self->appGlobals = appGlobals;

    QBApplicationControllerAddGlobalWindow(self->appGlobals->controller, (QBGlobalWindow)self);

    return (QBGlobalWindow) self;
}

void QBLogWindowManagerStart(QBGlobalWindow self_)
{
    QBLogWindowManager self = (QBLogWindowManager) self_;
    if (self && !self->fiber) {
        self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBLogWindowManager", &qbLogQueueStep, self);
        self->fiberTimer = SvFiberTimerCreate(self->fiber);
        SvFiberActivate(self->fiber);
    }
}

void QBLogWindowManagerStop(QBGlobalWindow self_)
{
    QBLogWindowManager self = (QBLogWindowManager) self_;
    if (self && self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
    }
}

void QBLogWindowManagerAddLog(QBGlobalWindow self_, char* text)
{
    QBLogWindowManager self = (QBLogWindowManager) self_;
    if (self && self->logQueue) {
        SvLogQueueElement log;
        log.message = text;
        log.time = SvTimeGet();
        qbLogQueueAddLog(self->logQueue, &log);
    }
}


// **** WIDGET QBLogQueue ****

struct QBLogQueue_s
{
    SvWidget *queue; ///< queue of queue elements (labels)
    SvTime *queueTime; ///< time of a label when it was added to queue
    int queueCount;  ///< number of current show labels
    int maxQueueSize; ///< max number of elements of queue

    int heightSum; ///< height position to attach new widget

    int spacing;
    SvTime elementDuration; ///< how long element of queue should be visible (sec)
    int level; ///< level for labels
    SvBitmap background; ///< label background
};
typedef struct QBLogQueue_s* QBLogQueue;

SvLocal void
qbLogQueueShiftQueue(SvWidget widget)
{
    QBLogQueue self = (QBLogQueue) widget->prv;

    if (!self->queueCount)
        return;

    SvWidget w = self->queue[0];
    assert(self->heightSum >= 0);
    self->queueTime[0] = SvTimeGetZero();
    svWidgetDetach(w);
    svWidgetDestroy(w);
    self->queue[0] = NULL;

    self->heightSum = 0;

    int i;
    for (i = 1; i < self->queueCount; ++i) {
        w = self->queue[i-1] = self->queue[i];
        self->queueTime[i-1] = self->queueTime[i];
        assert(w);
        svWidgetDetach(w);
        svWidgetAttach(widget, w, 0, self->heightSum, self->level);
        self->heightSum += w->height;
        self->heightSum += self->spacing;
    }

    self->queue[self->queueCount-1] = NULL;
    self->queueTime[self->queueCount-1] = SvTimeGetZero();
    --self->queueCount;
}

SvLocal void
qbLogQueueShiftOnTime(SvWidget widget)
{
    QBLogQueue self = (QBLogQueue) widget->prv;

    SvTime now = SvTimeGet();
    SvTime minStartTime = SvTimeSub(now, self->elementDuration);

    while (self->queueCount > 0) {
        if (SvTimeCmp(self->queueTime[0], minStartTime) >= 0)
            break;
        qbLogQueueShiftQueue(widget);
    }
}

#define SINGLE_QUERY_SIZE  1

SvLocal
bool qbLogQueueUpdate(SvWidget w)
{
    for (;;)
    {
        SvLogQueueElement  queue[SINGLE_QUERY_SIZE];
        int queueCount = SvLogQueueMultiPop(queue, SINGLE_QUERY_SIZE);
        log_state("%s() count = %d", __func__, queueCount);
        if (queueCount == 0)
            return true;

        int i;
        for (i = 0; i < queueCount; ++i)
            qbLogQueueAddLog(w, &queue[i]);

        if (SvFiberTimePassed())
            return false;
    }
}

SvLocal
void qbLoqQueueForceShiftOnSize(SvWidget w)
{
    QBLogQueue self = (QBLogQueue) w->prv;

    // we shift queue until the last element of the queue is finally visible
    while (self->heightSum > w->height && self->queueCount > 1) {
        log_state("%s() heightSum  = %d, widget height size = %d - force shift", __func__, self->heightSum, w->height);
        qbLogQueueShiftQueue(w);
    }
}

SvLocal void
qbLogQueueCleanUp(SvApplication app, void* ptr)
{
    log_state("%s()",__func__);

    QBLogQueue self = (QBLogQueue) ptr;
    SVTESTRELEASE(self->background);
    free(self->queue);
    self->queue = NULL;
    free(self->queueTime);
    self->queueTime = NULL;
    free(self);
    self = NULL;
}

SvLocal SvWidget
qbLogQueueNew(SvApplication app, const char* widgetName)
{
    log_state("%s()",__func__);
    SvWidget widget = NULL;
    QBLogQueue self = calloc(sizeof(struct QBLogQueue_s),1);

    if (!self)
        return NULL;

    widget = svSettingsWidgetCreate(app, widgetName);

    if (!widget) {
        SvLogError("%s() create widget: %s fail", __func__, widgetName);
        free(self);
        return NULL;
    }

    widget->prv = self;
    widget->clean = qbLogQueueCleanUp;
    svWidgetSetFocusable(widget, false);

    self->queueCount = 0;
    self->maxQueueSize = svSettingsGetInteger(widgetName, "maxQueueSize",20);
    int durationSec = svSettingsGetInteger(widgetName, "elementTime", 5);
    self->elementDuration = SvTimeConstruct(durationSec, 0);
    self->level = 1000;
    self->queue = calloc(sizeof(SvWidget), self->maxQueueSize);
    self->queueTime = calloc(sizeof(SvTime), self->maxQueueSize);
    self->background = SVRETAIN(svSettingsGetBitmap(widgetName, "background"));
    self->spacing = svSettingsGetInteger(widgetName, "spacing", 0);

    return widget;
}

SvLocal SvString
qbLogQueueCreateMessage(const SvLogQueueElement* log)
{
    SvTime now = SvTimeConstruct(SvTimeNow(), 0);
    struct tm  tm_time;
    SvTimeBreakDown(now, true, &tm_time);

    char buf[16];
    strftime(buf, 16, "%H:%M:%S", &tm_time);

    int ms = SvTimeToMs(log->time);
    char buf2[16];
    snprintf(buf2, sizeof(buf2), "%3d.%03d", (ms / 1000) % 1000, ms % 1000);

    SvString message = SvStringCreateWithFormat("%s | %s | %s", buf, buf2, log->message);
    return message;
}

SvLocal void
qbLogQueueAddLog(SvWidget w, const SvLogQueueElement* log)
{
    if (!w) {
        free(log->message);
        return;
    };

    QBLogQueue self = w->prv;

    SvString message = qbLogQueueCreateMessage(log);

    svSettingsPushComponent("logWindow.settings");
    SvWidget label = svLabelNewFromSM(w->app, "label");

    svLabelSetText(label, SvStringCString(message));
    SVRELEASE(message);

    if (self->queueCount == self->maxQueueSize)
        qbLogQueueShiftQueue(w);

    svWidgetSetBitmap(label, self->background);
    svWidgetAttach(w, label, 0, self->heightSum, self->level);
    self->heightSum += label->height;
    self->queue[self->queueCount] = label;
    self->queueTime[self->queueCount] = log->time;
    ++self->queueCount;

    qbLoqQueueForceShiftOnSize(w);

    svSettingsPopComponent();

    free(log->message);
}

SvLocal
void qbLogQueueStep(void *arg)
{
    QBLogWindowManager self = (QBLogWindowManager) arg;

    qbLogQueueShiftOnTime(self->logQueue);

    bool finished = qbLogQueueUpdate(self->logQueue);
    if (!finished)
        return;

    SvFiberDeactivate(self->fiber);
    SvFiberTimerActivateAfter(self->fiberTimer, SvTimeFromMs(QB_LOG_QUEUE_FIBER_DELAY_MS));
}

#else // #if SV_LOG_LEVEL > 0

QBGlobalWindow QBLogWindowManagerCreate(AppGlobals appGlobals)
{
    return NULL;
}

void QBLogWindowManagerStart(QBGlobalWindow self)
{
}

void QBLogWindowManagerStop(QBGlobalWindow self)
{
}

void QBLogWindowManagerAddLog(QBGlobalWindow self_, char* text)
{
    free(text);
}

#endif // #if SV_LOG_LEVEL > 0
