/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include "QBScrollView.h"

#include <limits.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvWeakList.h>
#include <QBDataModel3/QBListModel.h>
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBListModelListener.h>
#include <CUIT/Core/widget.h>
#include <SWL/rangelistener.h>
#include <SWL/viewport.h>
#include <settings.h>

#define CONTENT_ROWS_EXCESS 3

SvInterface
QBScrollViewItemController_getInterface(void)
{
    static SvInterface interface = NULL;
    SvErrorInfo error = NULL;

    if (unlikely(!interface)) {
        SvInterfaceCreateManaged("QBScrollViewItemController",
                                 sizeof(struct QBScrollViewItemController_t),
                                 NULL, &interface, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            abort();
        }
    }

    return interface;
}

struct QBScrollView_t {
    struct SvObject_ super_;

    SvApplication app;

    SvWidget viewport;
    SvWidget content;
    SvWidget slideSurf;

    int itemWidth;
    int itemMinHeight;
    int itemSpacing;

    int lastYOffset;

    SvWidget *items;
    int itemsCount;
    int topItemIdx;

    SvGenericObject dataSource;
    SvGenericObject itemController;

    SvWeakList rangeListeners;
};

inline SvLocal int
QBScrollViewGetSourceSize(QBScrollView self)
{
    if (!self->dataSource)
        return 0;

    return SvInvokeInterface(QBListModel, self->dataSource, getLength);
}

inline SvLocal SvGenericObject
QBScrollViewGetObjectAtIndex(QBScrollView self, int idx)
{
    if (!self->dataSource)
        return NULL;

    return SvInvokeInterface(QBListModel, self->dataSource, getObject, idx);
}

SvLocal void
QBScrollViewRearrangeItems(QBScrollView self)
{
    int yOff = self->items[self->topItemIdx % self->itemsCount]->off_y;
    for (int i = 0; i < self->itemsCount; ++i) {
        int idx = (self->topItemIdx + i) % self->itemsCount;
        self->items[idx]->off_y = yOff;
        yOff += self->items[idx]->height + self->itemSpacing;
    }

    int height = self->viewport->height;
    for (int i = 0; i < self->itemsCount; ++i) {
        int idx = (self->topItemIdx + i) % self->itemsCount;
        if (!svWidgetIsHidden(self->items[idx])) {
            int newHeight = self->items[idx]->off_y + self->items[idx]->height;
            if (newHeight > height) {
                height = newHeight;
            }
        }
    }
    self->content->height = height;

    svViewPortContentsSizeChanged(self->viewport);
}

SvLocal void
QBScrollViewFillItem(QBScrollView self, int idx, int objIdx)
{
    SvGenericObject obj = NULL;

    if (objIdx >= QBScrollViewGetSourceSize(self)) {
        svWidgetSetHidden(self->items[idx], true);
    } else {
        svWidgetSetHidden(self->items[idx], false);
        obj = QBScrollViewGetObjectAtIndex(self, objIdx);
    }

    SvInvokeInterface(QBScrollViewItemController, self->itemController,
                      setObject, self->items[idx], obj);
    if (self->items[idx]->height < self->itemMinHeight)
        self->items[idx]->height = self->itemMinHeight;
}

SvLocal void
QBScrollViewCreateItems(QBScrollView self)
{
    int yOff = 0;
    for (int i = 0; i < self->itemsCount; ++i) {
        if (self->items[i]) {
            svWidgetDestroy(self->items[i]);
            self->items[i] = NULL;
        }
        self->items[i] = SvInvokeInterface(QBScrollViewItemController,
                                           self->itemController, createItem,
                                           self->app, self->itemWidth,
                                           self->itemMinHeight);
        if (self->items[i]) {
            svWidgetAttach(self->slideSurf, self->items[i], 0, yOff + self->itemSpacing / 2, 1);
            yOff += self->items[i]->height + self->itemSpacing;
            QBScrollViewFillItem(self, i, i);
        }
    }

    self->topItemIdx = 0;

    svViewPortAddRangeListener(self->viewport, (SvGenericObject) self,
                               SvViewPortRangeOrientation_VERTICAL);

    QBScrollViewRearrangeItems(self);
}

SvWidget
QBScrollViewGetViewport(SvWidget w)
{
    QBScrollView self = w->prv;
    return self->viewport;
}

void
QBScrollViewSetItemController(SvWidget w, SvGenericObject itemController,
                              SvErrorInfo *errorOut)
{
    QBScrollView self = w->prv;
    SvErrorInfo error = NULL;

    if (!itemController || !SvObjectIsImplementationOf(itemController, QBScrollViewItemController_getInterface())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, NULL);
        goto err;
    }

    SVTESTRELEASE(self->itemController);
    self->itemController = SVRETAIN(itemController);

    if (self->dataSource)
        QBScrollViewCreateItems(self);

err:
    SvErrorInfoPropagate(error, errorOut);
}

void
QBScrollViewConnectToSource(SvWidget w, SvGenericObject dataSource)
{
    QBScrollView self = w->prv;

    if (self->dataSource) {
        SvInvokeInterface(QBDataModel, self->dataSource, removeListener,
                          (SvGenericObject) self, NULL);
        SVRELEASE(self->dataSource);
        self->dataSource = NULL;
    }

    if (dataSource) {
        self->dataSource = SVRETAIN(dataSource);
        SvInvokeInterface(QBDataModel, self->dataSource, addListener,
                          (SvGenericObject) self, NULL);

        if (self->itemController)
            QBScrollViewCreateItems(self);
    }
}

void
QBScrollViewAddRangeListener(SvWidget w,
                             SvGenericObject rangeListener,
                             SvErrorInfo *errorOut)
{
    QBScrollView self = w->prv;
    SvErrorInfo error = NULL;

    if (!rangeListener || !SvObjectIsImplementationOf(rangeListener, SvRangeListener_getInterface())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "invalid range listener handle passed");
    } else if (!self->rangeListeners && !(self->rangeListeners = SvWeakListCreate(NULL))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "can't create SvWeakList");
    } else {
        SvWeakListPushBack(self->rangeListeners, rangeListener, &error);
    }

    SvErrorInfoPropagate(error, errorOut);
}

void
QBScrollViewRemoveRangeListener(SvWidget w,
                                SvGenericObject rangeListener,
                                SvErrorInfo *errorOut)
{
    QBScrollView self = w->prv;
    SvErrorInfo error = NULL;

    if (!rangeListener) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "invalid range listener handle passed");
    } else if (!SvWeakListRemoveObject(self->rangeListeners, rangeListener)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_notFound,
                                  "range listener not found");
    }

    SvErrorInfoPropagate(error, errorOut);
}

// QBListModelListener virtual methods

SvLocal void
QBScrollViewItemsAdded(SvGenericObject self_, size_t idx, size_t count)
{
    QBScrollView self = (QBScrollView) self_;
    int first, last;

    if (self->topItemIdx > (int) idx + (int) count || self->topItemIdx + self->itemsCount < (int) idx) {
        QBScrollViewRearrangeItems(self);
        return;
    }

    if (self->topItemIdx > (int) idx)
        first = self->topItemIdx;
    else
        first = idx;

    if (self->topItemIdx + self->itemsCount < (int) idx + (int) count)
        last = self->topItemIdx + self->itemsCount;
    else
        last = idx + count;

    for (int i = first; i < last; ++i)
        QBScrollViewFillItem(self, i % self->itemsCount, i);
    QBScrollViewRearrangeItems(self);

    svWidgetForceRender(self->content);
}

SvLocal void
QBScrollViewItemsRemoved(SvGenericObject self_, size_t idx, size_t count)
{
    QBScrollView self = (QBScrollView) self_;
    QBScrollViewRearrangeItems(self);

    svWidgetForceRender(self->content);
}

SvLocal void
QBScrollViewItemsChanged(SvGenericObject self_, size_t idx, size_t count)
{
    QBScrollView self = (QBScrollView) self_;
    int first, last;

    if (self->topItemIdx > (int) idx + (int) count || self->topItemIdx + self->itemsCount < (int) idx) {
        QBScrollViewRearrangeItems(self);
        return;
    }

    if (self->topItemIdx > (int) idx)
        first = self->topItemIdx;
    else
        first = idx;

    if (self->topItemIdx + self->itemsCount < (int) idx + (int) count)
        last = self->topItemIdx + self->itemsCount;
    else
        last = idx + count;

    for (int i = first; i < last; ++i)
        QBScrollViewFillItem(self, i % self->itemsCount, i);
    QBScrollViewRearrangeItems(self);

    svWidgetForceRender(self->content);
}

SvLocal void
QBScrollViewItemsReordered(SvGenericObject self_, size_t idx, size_t count)
{

}

// SvRangeListener virtual methods

SvLocal void
QBScrollViewActiveRangeChanged(SvGenericObject self_, size_t lower, size_t upper)
{
    QBScrollView self = (QBScrollView) self_;
    int xPos, yPos;
    svViewPortGetContentsPosition(self->viewport, &xPos, &yPos);

    if (yPos == self->lastYOffset)
        return;

    int currentItemIdx = self->topItemIdx % self->itemsCount;
    int nextItemIdx = (self->topItemIdx + 1) % self->itemsCount;
    int prevItemIdx = (self->topItemIdx + self->itemsCount - 1) % self->itemsCount;
    SvWidget currentItem = self->items[currentItemIdx];
    SvWidget nextItem = self->items[nextItemIdx];
    SvWidget prevItem = self->items[prevItemIdx];

    if (yPos > self->lastYOffset) {
        if ((int) lower >= nextItem->off_y) {
            currentItem->off_y = prevItem->off_y + prevItem->height + self->itemSpacing;
            QBScrollViewFillItem(self, currentItemIdx, self->topItemIdx + self->itemsCount);
            self->topItemIdx++;
            QBScrollViewRearrangeItems(self);
        }
    } else {
        if ((int) lower < currentItem->off_y + currentItem->height && self->topItemIdx != 0) {
            prevItem->off_y = currentItem->off_y - prevItem->height - self->itemSpacing;
            if (prevItem->off_y < 0) {
                prevItem->off_y = 0;
            }
            QBScrollViewFillItem(self, prevItemIdx, self->topItemIdx - 1);
            self->topItemIdx--;
            QBScrollViewRearrangeItems(self);
        }
    }
    self->lastYOffset = yPos;

    if (!self->rangeListeners)
        return;

    SvGenericObject listener;
    SvIterator iter = SvWeakListIterator(self->rangeListeners);
    while ((listener = SvIteratorGetNext(&iter)))
        SvInvokeInterface(SvRangeListener, listener,
                          activeRangeChanged, lower, upper);
}

SvLocal void
QBScrollViewRangeChanged(SvGenericObject self_, size_t lower, size_t upper)
{
    QBScrollView self = (QBScrollView) self_;
    SvGenericObject listener;

    if (!self->rangeListeners)
        return;

    SvIterator iter = SvWeakListIterator(self->rangeListeners);
    while ((listener = SvIteratorGetNext(&iter)))
        SvInvokeInterface(SvRangeListener, listener,
                          rangeChanged, lower, upper);
}

SvLocal void
QBScrollViewClean(SvApplication app, void *ptr)
{
    QBScrollView self = ptr;

    SVRELEASE(self);
}

SvLocal void
QBScrollView__dtor__(void *self_)
{
    QBScrollView self = self_;

    free(self->items);

    SVTESTRELEASE(self->dataSource);
    SVTESTRELEASE(self->itemController);
    SVTESTRELEASE(self->rangeListeners);
}

SvLocal SvType
QBScrollView_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBScrollView__dtor__
    };
    static SvType type = NULL;
    static const struct QBListModelListener_t methods = {
        .itemsAdded     = QBScrollViewItemsAdded,
        .itemsRemoved   = QBScrollViewItemsRemoved,
        .itemsChanged   = QBScrollViewItemsChanged,
        .itemsReordered = QBScrollViewItemsReordered
    };
    static const struct SvRangeListener_t listenerMethods =  {
        .rangeChanged       = QBScrollViewRangeChanged,
        .activeRangeChanged = QBScrollViewActiveRangeChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBScrollView",
                            sizeof(struct QBScrollView_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBListModelListener_getInterface(), &methods,
                            SvRangeListener_getInterface(), &listenerMethods,
                            NULL);
    }

    return type;
}

SvWidget
QBScrollViewCreate(SvApplication app, const char* widgetName, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBScrollView self = NULL;

    self = (QBScrollView) SvTypeAllocateInstance(QBScrollView_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "can't allocate QBScrollView");
        goto err;
    }

    if (!widgetName || !svSettingsIsWidgetDefined(widgetName)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Wrong widget name '%s' passed.", widgetName);
        goto err;
    }

    SvWidget w = svSettingsWidgetCreate(app, widgetName);
    w->prv = self;
    w->clean = QBScrollViewClean;

    self->itemWidth = svSettingsGetInteger(widgetName, "itemWidth", -1);
    self->itemMinHeight = svSettingsGetInteger(widgetName, "itemMinHeight", -1);

    if (self->itemWidth <= 0 || self->itemMinHeight <= 0) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Wrong settings provided");
        goto err;
    }
    self->itemSpacing = svSettingsGetInteger(widgetName, "itemSpacing", 0);

    self->content = svWidgetCreate(app, self->itemWidth, USHRT_MAX / 2);
    if (!self->content) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory, "Widget");
        goto err;
    }

    self->slideSurf = svWidgetCreate(app, self->itemWidth, USHRT_MAX / 2);
    if (!self->slideSurf) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory, "Widget");
        svWidgetDestroy(self->content);
        self->content = NULL;
        goto err;
    }

    svWidgetAttach(self->content, self->slideSurf, 0, 0, 1);

    char *buf = NULL;
    asprintf(&buf, "%s.viewport", widgetName);
    self->viewport = svViewPortNew(app, buf);
    free(buf);

    svSettingsWidgetAttach(w, self->viewport, widgetName, 1);
    svViewPortSetContents(self->viewport, self->content);
    svViewPortContentsSizeChanged(self->viewport);
    svViewPortSetContentsPosition(self->viewport, 0, 0, true);


    self->itemsCount = (self->viewport->height / self->itemMinHeight)
        + CONTENT_ROWS_EXCESS;

    self->items = calloc(self->itemsCount, sizeof(SvWidget));
    if (!self->items) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "can't allocate QBScrollView items");
        goto err;
    }

    self->app = app;
    self->topItemIdx = 0;

    self->rangeListeners = NULL;
    self->lastYOffset = 0;

    return w;

err:
    if (self) {
        if (self->viewport)
            svWidgetDestroy(self->viewport);
        else
            SVRELEASE(self);
    }
    if (!error) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "invalid settings for widget %s", widgetName);
    }
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}
