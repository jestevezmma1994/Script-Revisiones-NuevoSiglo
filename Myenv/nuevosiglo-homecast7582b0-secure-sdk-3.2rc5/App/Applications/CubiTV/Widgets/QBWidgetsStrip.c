/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2009-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBWidgetsStrip.h"
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvCommonDefs.h>
#include <QBDataModel3/QBListModel.h>
#include <QBDataModel3/QBListModelListener.h>
#include <QBDataModel3/QBDataModel.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/app.h>
#include <SWL/QBFrame.h>
#include <XMB2/XMBItemController.h>
#include <settings.h>
#include <QBInput/QBInputCodes.h>

typedef struct QBWidgetsStrip_ {
    struct SvObject_ super_;                ///< superclass
    SvWidget strip;                         ///< main widget handle
    SvApplication app;                      ///< application handle for creating widgets
    SvGenericObject dataSource;             ///< source of data for creating child widgets
    SvGenericObject itemController;         ///< item controller used to create child widgets

    int maxWidgets;                         ///< max number of widgets a strip can present
    int spacing;                            ///< horizontal spacing between child widgets
    int focusXOffset;                       ///< x offset of focus frame from the focused child x coordinate
    int focusYOffset;                       ///< y offset of focus frame from the focused child y coordinate
    int focusPosition;                      ///< index of currently focused child widget
    SvWidget focus;                         ///< focus frame widget
    bool isFocused;                         ///< flag telling if the focus belongs to strip wodget
    QBWidgetsStripFocusPolicy focusPolicy;  ///< focus behaviour policy executed when strip gains focus

    SvWidget* childWidgets;                 ///< array of child widgets
    size_t* childIndexes;                   ///< mapping of child widget index to data source index
    size_t childWidgetsCount;               ///< number of child widgets in a strip
    SvWidgetId notificationTarget;          ///< widget that will be nottified of user events.
} *QBWidgetsStrip;

SvLocal void
QBWidgetsStripDestroy(void* self_)
{
    QBWidgetsStrip self = (QBWidgetsStrip) self_;
    free(self->childWidgets);
    free(self->childIndexes);
    if (self->focus && !self->isFocused) {
        svWidgetDestroy(self->focus);
    }
    if (self->dataSource) {
        SvInvokeInterface(QBDataModel, self->dataSource,
                          removeListener, (SvGenericObject) self, NULL);
        SVRELEASE(self->dataSource);
    }
    SVTESTRELEASE(self->itemController);
}

SvLocal void
QBWidgetsStripUpdateItems(QBWidgetsStrip self)
{
    for (size_t i = 0; i < self->childWidgetsCount; ++i) {
        if (self->childWidgets[i]) {
            svWidgetDetach(self->childWidgets[i]);
            svWidgetDestroy(self->childWidgets[i]);
        }
    }
    free(self->childWidgets);
    self->childWidgets = NULL;
    free(self->childIndexes);
    self->childIndexes = NULL;
    self->childWidgetsCount = 0;

    if (!self->dataSource) {
        return;
    }

    size_t dataLen = SvInvokeInterface(QBListModel, self->dataSource, getLength);
    if (self->maxWidgets >= 0 && self->maxWidgets < (int) dataLen) {
        dataLen = (size_t) self->maxWidgets;
    }
    if (dataLen == 0) {
        return;
    }
    self->childWidgets = calloc(dataLen, sizeof(SvWidget));
    self->childIndexes = calloc(dataLen, sizeof(size_t));
    int nextX = self->focusXOffset < 0 ? -self->focusXOffset : 0;
    int yLevel = self->focusYOffset < 0 ? -self->focusYOffset : 0;
    if (!self->itemController) {
        return;
    }
    for (size_t i = 0; i < dataLen; ++i) {
        SvObject node = SvInvokeInterface(QBListModel, self->dataSource, getObject, i);
        SvWidget child = SvInvokeInterface(XMBItemController, self->itemController, createItem, node, NULL, self->app, XMBMenuState_normal);
        if (child) {
            svWidgetAttach(self->strip, child, nextX, yLevel, 1);
            nextX += child->width + self->spacing;
            self->childWidgets[self->childWidgetsCount] = child;
            self->childIndexes[self->childWidgetsCount] = i;
            ++self->childWidgetsCount;
        }
    }
}


extern void
QBWidgetsStripRefreshItems(SvWidget strip)
{
    QBWidgetsStrip self = strip->prv;
    if (self->dataSource && SvInvokeInterface(QBListModel, self->dataSource, getLength) > 0) {
        QBWidgetsStripUpdateItems(self);
    }
}

// QBListModelListener virtual methods

SvLocal void
QBWidgetsStripItemsAdded(SvGenericObject self_, size_t idx, size_t count)
{
    QBWidgetsStrip self = (QBWidgetsStrip) self_;
    QBWidgetsStripUpdateItems(self);
}

SvLocal void
QBWidgetsStripItemsRemoved(SvGenericObject self_, size_t idx, size_t count)
{
    QBWidgetsStrip self = (QBWidgetsStrip) self_;
    QBWidgetsStripUpdateItems(self);
}

SvLocal void
QBWidgetsStripItemsChanged(SvGenericObject self_, size_t idx, size_t count)
{
    QBWidgetsStrip self = (QBWidgetsStrip) self_;
    QBWidgetsStripUpdateItems(self);
}

SvLocal void
QBWidgetsStripItemsReordered(SvGenericObject self_, size_t idx, size_t count)
{
    QBWidgetsStrip self = (QBWidgetsStrip) self_;
    QBWidgetsStripUpdateItems(self);
}


SvLocal SvType
QBWidgetsStrip_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBWidgetsStripDestroy
    };
    static const struct QBListModelListener_t methods = {
        .itemsAdded     = QBWidgetsStripItemsAdded,
        .itemsRemoved   = QBWidgetsStripItemsRemoved,
        .itemsChanged   = QBWidgetsStripItemsChanged,
        .itemsReordered = QBWidgetsStripItemsReordered
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBWidgetsStrip",
                            sizeof(struct QBWidgetsStrip_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBListModelListener_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvLocal void
QBWidgetsStripCleanup(SvApplication app, void *self_)
{
    QBWidgetsStrip self = self_;
    SVRELEASE(self);
}

SvLocal void
QBWidgetsStripNotificationCleanup(void *ptr)
{
    QBWidgetsStripNotification notify = ptr;
    SVTESTRELEASE(notify->dataObject);
    free(ptr);
};

/**
 * function for sending user events notifications of changing state of the widget strip
 *
 * @param [in] self             widget strip handle
 * @param [in] type             type of event to send
 **/
SvLocal void
QBWidgetsStripSendNotification(QBWidgetsStrip self, QBWidgetsStripNotificationType type)
{
    int dataIndex = self->focusPosition >= 0 ? (int) self->childIndexes[self->focusPosition] : self->focusPosition;

    QBWidgetsStripNotification notify = calloc(1, sizeof(struct QBWidgetsStripNotification_));
    notify->type = type;
    notify->dataIndex = dataIndex;
    notify->widgetIndex = self->focusPosition;
    notify->childWidget = self->childWidgets[self->focusPosition];

    if (self->dataSource && dataIndex >= 0) {
        SvGenericObject obj = SvInvokeInterface(QBListModel, self->dataSource, getObject, (size_t) dataIndex);
        notify->dataObject = SVTESTRETAIN(obj);
    }
    svAppPostEvent(self->strip->app, self->strip, svUserEventCreate(QB_WIDGETS_STRIP_NOTIFICATION, self->notificationTarget, notify, QBWidgetsStripNotificationCleanup));
}

SvLocal bool
QBWidgetsStripSetFocus(QBWidgetsStrip self, int position)
{
    if (self->childWidgetsCount == 0 || position >= (int) self->childWidgetsCount || position < 0) {
        return false;
    }
    self->focusPosition = position;
    int xOffset = self->childWidgets[self->focusPosition]->off_x;
    int yOffset = self->childWidgets[self->focusPosition]->off_y;
    int width = self->childWidgets[self->focusPosition]->width;
    svWidgetDetach(self->focus);
    QBFrameSetWidth(self->focus, width - 2 * self->focusXOffset);
    svWidgetAttach(self->strip, self->focus, xOffset + self->focusXOffset, yOffset + self->focusYOffset, 2);
    self->isFocused = true;
    QBWidgetsStripSendNotification(self, QB_WIDGETS_STRIP_NOTIFICATION_ACTIVE_ELEMENT);
    return true;
}

SvLocal bool
QBWidgetsStripInputEventHandler(SvWidget strip, SvInputEvent event)
{
    QBWidgetsStrip self = strip->prv;
    if (event->ch == QBKEY_RIGHT) {
        return QBWidgetsStripSetFocus(self, self->focusPosition + 1);
    } else if (event->ch == QBKEY_LEFT) {
        return QBWidgetsStripSetFocus(self, self->focusPosition - 1);
    } else if (event->ch == QBKEY_ENTER) {
        QBWidgetsStripSendNotification(self, QB_WIDGETS_STRIP_NOTIFICATION_SELECTED_ELEMENT);
        return true;
    }

    return 0;
}

SvLocal void
QBWidgetsStripFocusEventHandler(SvWidget strip, SvFocusEvent event)
{
    QBWidgetsStrip self = strip->prv;

    if (event->kind == SvFocusEventKind_GET) {
        if (self->focusPolicy == QBWidgetsStripFocusPolicy_FocusFirst) {
            QBWidgetsStripSetFocus(self, 0);
            return;
        } else if (self->focusPolicy == QBWidgetsStripFocusPolicy_FocusFirst) {
            QBWidgetsStripSetFocus(self, self->childWidgetsCount - 1);
            return;
        } else {
            if (self->focusPosition > 0 && self->focusPosition < (int) self->childWidgetsCount) {
                QBWidgetsStripSetFocus(self, self->focusPosition);
                return;
            } else {
                QBWidgetsStripSetFocus(self, 0);
                return;
            }
        }
    } else {
        svWidgetDetach(self->focus);
        self->isFocused = false;
        QBWidgetsStripSendNotification(self, QB_WIDGETS_STRIP_NOTIFICATION_FOCUS_LOST);
    }
}

SvWidget
QBWidgetsStripNew(SvApplication app,
                  const char *name,
                  SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    QBWidgetsStrip self = (QBWidgetsStrip) SvTypeAllocateInstance(QBWidgetsStrip_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "can't allocate QBWidgetsStrip");
        goto err;
    }
    if (!(self->strip = svSettingsWidgetCreate(app, name))) {
        goto err;
    }
    self->app = app;
    self->strip->prv = self;
    self->strip->clean = QBWidgetsStripCleanup;
    svWidgetSetInputEventHandler(self->strip, QBWidgetsStripInputEventHandler);
    svWidgetSetFocusEventHandler(self->strip, QBWidgetsStripFocusEventHandler);

    self->spacing = svSettingsGetInteger(name, "itemSpacing", 0);
    self->maxWidgets = svSettingsGetInteger(name, "maxWidgets", -1);
    self->focusXOffset = svSettingsGetInteger(name, "focusXOffset", -1);
    self->focusYOffset = svSettingsGetInteger(name, "focusYOffset", -1);
    self->focusPosition = -1;
    self->childWidgets = NULL;
    self->childIndexes = NULL;
    self->childWidgetsCount = 0;

    char *buf;
    asprintf(&buf, "%s.Focus", name);
    self->focus = QBFrameCreateFromSM(app, buf);
    self->focusPolicy = QBWidgetsStripFocusPolicy_FocusFirst;
    free(buf);

    return self->strip;

err:
    if (self) {
        if (self->strip)
            svWidgetDestroy(self->strip);
        else
            SVRELEASE(self);
    }
    if (!error) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "invalid settings for widget %s", name);
    }
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

void
QBWidgetsStripConnectToDataSource(SvWidget strip,
                                  SvGenericObject dataSource,
                                  SvGenericObject controller,
                                  SvErrorInfo *errorOut)
{
    QBWidgetsStrip self = strip->prv;
    SvErrorInfo error = NULL;
    if (dataSource) {
        if (!SvObjectIsImplementationOf(dataSource, QBListModel_getInterface())) {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                      "invalid list model handle passed");
            goto fini;
        } else if (!controller
                   || !SvObjectIsImplementationOf(controller, XMBItemController_getInterface())) {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                      "invalid item controller handle passed");
            goto fini;
        }
    }

    if (self->dataSource) {
        SvInvokeInterface(QBDataModel, self->dataSource,
                          removeListener, (SvGenericObject) self, NULL);
        SVRELEASE(self->dataSource);
        self->dataSource = NULL;
        SVRELEASE(self->itemController);
        self->itemController = NULL;
        QBWidgetsStripUpdateItems(self);
    }
    if (!dataSource)
        return;

    SvInvokeInterface(QBDataModel, dataSource,
                      addListener, (SvGenericObject) self, &error);
    if (error)
        goto fini;

    self->dataSource = SVRETAIN(dataSource);
    self->itemController = SVRETAIN(controller);
    self->focusPosition = -1;

    if (SvInvokeInterface(QBListModel, dataSource, getLength) > 0) {
        QBWidgetsStripUpdateItems(self);
    }
fini:
    SvErrorInfoPropagate(error, errorOut);
}

void
QBWidgetsStripConnectSetFocusPolicy(SvWidget strip,
                                    QBWidgetsStripFocusPolicy policy)
{
    QBWidgetsStrip self = strip->prv;
    self->focusPolicy = policy;
}

void
QBWidgetsStripSetActive(SvWidget strip,
                        int idx)
{
    QBWidgetsStrip self = strip->prv;
    self->focusPolicy = QBWidgetsStripFocusPolicy_FocusRecent;
    QBWidgetsStripSetFocus(self, idx);
}

size_t
QBWidgetsStripGetWidgetsCount(SvWidget strip)
{
    QBWidgetsStrip self = strip->prv;
    return self->childWidgetsCount;
}

void
QBWidgetsStripSetNotificationTarget(SvWidget strip, SvWidgetId target)
{
    QBWidgetsStrip self = strip->prv;
    self->notificationTarget = target;
}

