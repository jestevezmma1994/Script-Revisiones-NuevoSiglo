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


#include "QBLongTextDialog.h"

#include <libintl.h> // gettext
#include <settings.h>

#include <SWL/button.h>
#include <SWL/QBFrame.h>
#include <CUIT/Core/event.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBInput/QBInputCodes.h>
#include <Widgets/QBTextBox.h>

#include <main.h>

typedef struct QBLongTextDialog_ *QBLongTextDialog;
SvLocal void QBLongTextDialogCallCallbackOnPressedButton(QBLongTextDialog self);

struct QBLongTextDialog_ {
    struct SvObject_ super_;

    SvWidget window;
    SvWidget textBox;
    SvWidget button;

    QBGlobalWindow globalWindow;
    QBApplicationController controller;

    QBLongTextCallbackOnPressedButton callback;
    void *callbackTarget;
};

SvLocal void
QBLongTextDialogDestroy(void *self_)
{
    QBLongTextDialog self = self_;
    if (self->globalWindow && self->controller) {
        QBApplicationControllerRemoveGlobalWindow(self->controller, self->globalWindow);
        self->globalWindow->window = NULL;
        SVRELEASE(self->globalWindow);
        self->globalWindow = NULL;
    }
}

SvLocal void
QBLongTextDialogClean(SvApplication app, void *ptr)
{
    QBLongTextDialog self = ptr;
    SVRELEASE(self);
}

SvLocal bool
QBLongTextDialogInputEventHandler(SvWidget w, SvInputEvent ev)
{
    QBLongTextDialog self = w->prv;
    if (svWidgetIsAttached(w))
        return false;

    if (ev->ch == QBKEY_ENTER && self->button) {
        QBLongTextDialogCallCallbackOnPressedButton(self);
        return true;
    }

    svWidgetInputEventHandler(self->textBox, ev);

    return true;
}

SvLocal SvType
QBLongTextDialog_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBLongTextDialogDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBLongTextDialog",
                            sizeof(struct QBLongTextDialog_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvWidget
QBLongTextDialogCreate(AppGlobals appGlobals, const char *widgetName, SvString message, bool withButton, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBLongTextDialog self = NULL;
    SvWidget window = NULL;
    char* subWidgetName = NULL;

    self = (QBLongTextDialog) SvTypeAllocateInstance(QBLongTextDialog_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "Can't allocate QBLongTextDialog widget");
        goto err;
    }

    // creating window
    window = svWidgetCreate(appGlobals->res, appGlobals->res->width, appGlobals->res->height);
    svWidgetSetFocusable(window, true);
    window->prv = self;
    svWidgetSetInputEventHandler(window, QBLongTextDialogInputEventHandler);
    window->clean = QBLongTextDialogClean;
    self->window = window;

    if (!svSettingsIsWidgetDefined(widgetName)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Widget [%s] not found", widgetName);
        goto err;
    }

    // creating frame
    int framePadding = svSettingsGetInteger(widgetName, "framePadding", 0);
    SvWidget frame = QBFrameCreateFromSM(appGlobals->res, widgetName);
    svWidgetAttach(window, frame, 0, 0, 0);
    frame->off_x = (appGlobals->res->width - frame->width) / 2;
    frame->off_y = (appGlobals->res->height - frame->height) / 2;

    // creating button
    int buttonHeight = 0;
    if (withButton) {
        asprintf(&subWidgetName, "%s.Button", widgetName);
        if (!svSettingsIsWidgetDefined(subWidgetName)) {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                      "Widget [%s] not found", subWidgetName);
            goto err;
        }
        self->button = svButtonNewFromSM(appGlobals->res, subWidgetName, gettext("OK"), 0, svWidgetGetId(frame));
        if (!self->button) {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                      "Couldn't create widget [%s]", subWidgetName);
            goto err;
        }
        buttonHeight = svSettingsGetInteger(subWidgetName, "height", 100);
        free(subWidgetName);
        subWidgetName = NULL;
        svWidgetAttach(frame, self->button, 0, 0, 0);
        // inside settings file we set activate image to inactiveBg, so button draws as active
        svWidgetSetFocusable(self->button, false);
    }

    // creating textBox
    asprintf(&subWidgetName, "%s.TextBox", widgetName);
    if (!svSettingsIsWidgetDefined(subWidgetName)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Widget [%s] not found", subWidgetName);
        goto err;
    }
    self->textBox = QBTextBoxCreate(appGlobals->res, subWidgetName, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "Couldn't create widget [%s]", subWidgetName);
        goto err;
    }
    free(subWidgetName);
    subWidgetName = NULL;
    QBTextBoxAddText(self->textBox, message);
    svWidgetAttach(frame, self->textBox, 0, 0, 1);
    self->textBox->off_x = framePadding;
    self->textBox->off_y = framePadding;
    QBTextBoxSetHeight(self->textBox, frame->height - 2 * framePadding - (withButton ? (buttonHeight + framePadding) : 0));

    if (self->button) {
        self->button->off_x = (self->textBox->width - self->button->width) / 2;
        self->button->off_y = self->textBox->height + 2 * framePadding;
    }

    // creating globalWindow
    self->globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    SvString widgetNameStr = SvStringCreate(widgetName, NULL);
    QBGlobalWindowInit(self->globalWindow, window, widgetNameStr);
    SVRELEASE(widgetNameStr);
    self->globalWindow->focusable = true;
    QBApplicationControllerAddGlobalWindow(appGlobals->controller, self->globalWindow);
    self->controller = appGlobals->controller;
    return window;

err:
    free(subWidgetName);
    if (window) {
        svWidgetDestroy(window);
    }
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

void
QBLongTextDialogSetCallbackOnPressedButton(SvWidget w, void *target, QBLongTextCallbackOnPressedButton callback)
{
    QBLongTextDialog self = w->prv;
    self->callback = callback;
    self->callbackTarget = target;
}

SvLocal void
QBLongTextDialogCallCallbackOnPressedButton(QBLongTextDialog self)
{
    if (self->callback) {
        self->callback(self->callbackTarget, self->window);
    }
}
