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

#include "QBLoadablePane.h"
#include "QBLoadablePaneListener.h"
#include <QBWidgets/QBWaitAnimation.h>
#include <main.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvObject.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/anim.h>


SvLocal void
QBLoadablePaneCleanUp(QBLoadablePane self)
{
    SVRELEASE(self->waitPane);
    svWidgetDestroy(self->waitWidget);
    SVTESTRELEASE(self->listener);
    if (self->fiber) {
        SvFiberEventDeactivate(self->timer);
        SvFiberDestroy(self->fiber);
    }
}

SvLocal void
QBLoadablePaneDestroy(void *self_)
{
    QBLoadablePane self = self_;
    QBLoadablePaneCleanUp(self);
}

SvLocal void
QBLoadablePaneSetStateAndNotifyListener(QBLoadablePane self, QBLoadablePaneState state)
{
    self->state = state;
    if (self->listener) {
        SvObject listener = SvWeakReferenceTakeReferredObject(self->listener);
        if (!listener)
            return;
        SvInvokeInterface(QBLoadablePaneListener, listener, stateChanged, self, state);
        SVRELEASE(listener);
    }
}

SvLocal void
QBLoadablePaneShow(SvObject self_)
{
    QBLoadablePane self = (QBLoadablePane) self_;
    if (self->state == QBLoadablePaneStateCreated ||
        self->state == QBLoadablePaneStateStarted ||
        self->state == QBLoadablePaneStateShown)
        return;

    SvInvokeInterface(QBContextMenuPane, self->waitPane, show);
}

SvLocal void
QBLoadablePaneHide(SvObject self_, bool immediately)
{
    QBLoadablePane self = (QBLoadablePane) self_;
    if (self->state != QBLoadablePaneStateShown &&
        self->state != QBLoadablePaneStateStarted &&
        self->state != QBLoadablePaneStateTimeOut)
        return;
    SvInvokeInterface(QBContextMenuPane, self->waitPane, hide, immediately);
}

SvLocal void
QBLoadablePaneSetActive(SvObject self_)
{
    QBLoadablePane self = (QBLoadablePane) self_;
    if (self->state != QBLoadablePaneStateShown &&
        self->state != QBLoadablePaneStateStarted)
        return;
    SvInvokeInterface(QBContextMenuPane, self->waitPane, setActive);
}

SvLocal bool
QBLoadablePaneHandleInputEvent(SvObject self_,
                               SvObject src,
                               SvInputEvent e)
{
    return false;
}

SvType
QBLoadablePane_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBLoadablePaneDestroy
    };

    static const struct QBContextMenuPane_ methods = {
        .show             = QBLoadablePaneShow,
        .hide             = QBLoadablePaneHide,
        .setActive        = QBLoadablePaneSetActive,
        .handleInputEvent = QBLoadablePaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBLoadablePane", sizeof(struct QBLoadablePane_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &vtable,
                            QBContextMenuPane_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvLocal void
QBLoadablePaneWaitOnShow(void *ptr,
                         QBContainerPane pane,
                         SvWidget frame)
{
    QBLoadablePane self = ptr;
    svSettingsRestoreContext(self->settingsCtx);
    svSettingsWidgetAttach(frame, self->waitWidget, "WaitAnimation", 1);
    QBLoadablePaneSetStateAndNotifyListener(self, QBLoadablePaneStateShown);
    if (self->useTimeOut) {
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(self->timeOutMs));
        QBLoadablePaneSetStateAndNotifyListener(self, QBLoadablePaneStateStarted);
    }
    svSettingsPopComponent();
}

SvLocal void
QBLoadablePaneWaitSetActive(void *ptr,
                            QBContainerPane pane,
                            SvWidget frame)
{
    QBLoadablePane self = ptr;
    svWidgetSetFocus(self->waitWidget);
}

SvLocal void
QBLoadablePaneWaitOnHide(void *ptr,
                         QBContainerPane pane,
                         SvWidget frame)
{
    QBLoadablePane self = ptr;
    if (self->state == QBLoadablePaneStateStarted) {
        SvFiberEventDeactivate(self->timer);
        QBLoadablePaneSetStateAndNotifyListener(self, QBLoadablePaneStateStopped);
    }
    svWidgetDetach(self->waitWidget);
    QBLoadablePaneSetStateAndNotifyListener(self, QBLoadablePaneStateHidden);
}

SvLocal void
QBLoadablePaneTimeOutStep(void * self_)
{
    QBLoadablePane self = (QBLoadablePane) self_;
    SvFiberEventDeactivate(self->timer);

    if (self->state != QBLoadablePaneStateStarted)
        return;
    SvLogError("%s(): Time out!", __func__);
    QBLoadablePaneSetStateAndNotifyListener(self, QBLoadablePaneStateTimeOut);
}

void
QBLoadablePaneInit(QBLoadablePane self,
                   QBContextMenu ctxMenu,
                   int level, int timeOutMs,
                   AppGlobals appGlobals,
                   SvErrorInfo * errorOut)
{
    SvErrorInfo error = NULL;
    if (!ctxMenu || level < 1 || timeOutMs < 0 || !appGlobals) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed. (ctxMenu=%p, level=%d, appGlobals=%p)",
                                  ctxMenu, level, appGlobals);
        goto err;
    }
    if (self->state == QBLoadablePaneStateInitialized)
        QBLoadablePaneCleanUp(self);

    self->settingsCtx = svSettingsSaveContext();
    self->timeOutMs = timeOutMs;
    if (self->timeOutMs > 0)
        self->useTimeOut = true;

    self->waitPane = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    static struct QBContainerPaneCallbacks_t callbacks = {
        .onShow    = QBLoadablePaneWaitOnShow,
        .onHide    = QBLoadablePaneWaitOnHide,
        .setActive = QBLoadablePaneWaitSetActive,
    };
    SvApplication app = appGlobals->res;
    QBContainerPaneInit(self->waitPane, app, ctxMenu, level, SVSTRING("WaitPane"), &callbacks, self);

    self->waitWidget = QBWaitAnimationCreate(app, "WaitAnimation");
    if (!self->waitWidget) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState, "Creating WaitAnimation failed.");
        goto err;
    }
    self->fiber = SvFiberCreate(appGlobals->scheduler, NULL, "QBLoadablePaneStep", QBLoadablePaneTimeOutStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    svWidgetSetFocusable(self->waitWidget, true);
    QBLoadablePaneSetStateAndNotifyListener(self, QBLoadablePaneStateInitialized);
err:
    SvErrorInfoPropagate(error, errorOut);
}

QBLoadablePane
QBLoadablePaneCreate(QBContextMenu ctxMenu,
                     int level,
                     int timeOutMs,
                     AppGlobals appGlobals,
                     SvErrorInfo * errorOut)
{
    SvErrorInfo error = NULL;
    if (!ctxMenu || level < 1 || !appGlobals) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed. (ctxMenu=%p, level=%d, appGlobals=%p)",
                                  ctxMenu, level, appGlobals);
        goto err;
    }
    QBLoadablePane self = (QBLoadablePane) SvTypeAllocateInstance(QBLoadablePane_getType(), NULL);
    QBLoadablePaneInit(self, ctxMenu, level, timeOutMs, appGlobals, &error);
    if (error) {
        SVTESTRELEASE(self);
        goto err;
    }
    return self;
err:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

bool
QBLoadablePaneSetStateListener(QBLoadablePane self,
                               SvObject listener,
                               SvErrorInfo * errorOut)
{
    SvErrorInfo error = NULL;
    if (!self || !listener) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed");
        goto err;
    }

    if (!SvObjectIsImplementationOf(listener, QBLoadablePaneListener_getInterface())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "listener needs to implement "
                                  "QBLoadablePaneStateListener interface");
        goto err;
    }

    self->listener = SvWeakReferenceCreateWithObject(listener, NULL);
    return true;
err:
    SvErrorInfoPropagate(error, errorOut);
    return false;
}
