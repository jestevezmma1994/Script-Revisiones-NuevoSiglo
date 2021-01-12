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

#include "countdownDialog.h"

#include <settings.h>
#include <CUIT/Core/event.h>
#include <SWL/label.h>
#include <SWL/button.h>
#include <QBWidgets/QBDialog.h>
#include <main.h>

struct QBCountdownDialog_ {
    SvWidget dialog;
    SvWidget helper;
    SvWidget timerLabel;
    SvWidget okButton;
    SvWidget cancelButton;

    SvString panelTag;

    SvTimerId timerID;
    int timeout;
    int delay;

    SvString timeoutLabelTag;
    int timeoutLabelPosition;

    const char *okMsg;
    const char *cancelMsg;
    QBCountdownDialogSetTimeoutLabel timeoutLabelGetter;
};

SvLocal void
QBCountdownDialogSetButtonCaptions(QBCountdownDialog self)
{
    char *buff;

    if (self->delay == 0) {
        if (self->okButton)
            svButtonSetCaption(self->okButton, self->okMsg);
        if (self->cancelButton)
            svButtonSetCaption(self->cancelButton, self->cancelMsg);
        return;
    }
    if (self->okButton) {
        asprintf(&buff, "%s (%d)", self->okMsg, self->delay);
        svButtonSetCaption(self->okButton, buff);
        free(buff);
    }

    if (self->cancelButton) {
        asprintf(&buff, "%s (%d)", self->cancelMsg, self->delay);
        svButtonSetCaption(self->cancelButton, buff);
        free(buff);
    }
}

SvLocal bool
QBCountdownDialogNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBCountdownDialog self = ptr;

    if (self->dialog) {
        if (!buttonTag || self->delay > 0)
            return true;
        if (self->timerID)
            svAppTimerStop(dialog->app, self->timerID);
        self->timerID = 0;
        QBDialogBreak(self->dialog);
        self->dialog = NULL;

        return true;
    }

    return false;
}

SvLocal void
QBCountdownDialogTimerEventHandler(SvWidget w, SvTimerEvent ev)
{
    QBCountdownDialog self = w->prv;
    if (ev->id != self->timerID)
        return;

    if (self->timeout > 0) {
        if (--self->timeout == 0) {
            if (self->timerID)
                svAppTimerStop(w->app, self->timerID);
            QBDialogBreak(self->dialog);
            return;
        }
        SvString label = self->timeoutLabelGetter(self->timeout);
        if (label) {
            if (!self->timerLabel)
                self->timerLabel = QBDialogAddLabel(self->dialog, self->panelTag, SvStringCString(label), self->timeoutLabelTag, self->timeoutLabelPosition);
            else
                svLabelSetText(self->timerLabel, SvStringCString(label));

            SVRELEASE(label);
        }
    }

    if (self->delay > 0) {
        --self->delay;
        QBCountdownDialogSetButtonCaptions(self);
    }
}

SvLocal void
QBCountdownDialogClean(SvApplication app,
                     void *self_)
{
    QBCountdownDialog self = self_;

    SVRELEASE(self->panelTag);
    SVRELEASE(self->timeoutLabelTag);
    if (self->timerID)
        svAppTimerStop(app, self->timerID);
    free(self);
}

void
QBCountdownDialogSetTimeout(QBCountdownDialog self, int timeout)
{
    if (!self) {
        SvLogError("%s: NULL argument passed", __FUNCTION__);
        return;
    }

    self->timeout = timeout;
}

int
QBCountdownDialogGetTimeout(QBCountdownDialog self)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __FUNCTION__);
        return -1;
    }

    return self->timeout;
}

void
QBCountdownDialogSetDelay(QBCountdownDialog self, int delay)
{
    if (!self) {
        SvLogError("%s: NULL argument passed", __FUNCTION__);
        return;
    }

    self->delay = delay;
}

int
QBCountdownDialogGetDelay(QBCountdownDialog self)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __FUNCTION__);
        return -1;
    }

    return self->delay;
}

QBCountdownDialog
QBCountdownDialogCreate(SvApplication res,
                        QBApplicationController controller,
                        const char *okMsg,
                        const char *cancelMsg,
                        bool okFocused,
                        QBCountdownDialogSetTimeoutLabel timeoutLabelGetter)
{
    if (!res || !controller || !okMsg || !timeoutLabelGetter) {
        SvLogError("%s: NULL argument passed", __FUNCTION__);
        return NULL;
    }

    QBCountdownDialog self = calloc(1, sizeof(struct QBCountdownDialog_));
    self->okMsg = okMsg;
    self->cancelMsg = cancelMsg;

    self->helper = svWidgetCreate(res, 0, 0);
    self->helper->prv = self;
    svWidgetSetTimerEventHandler(self->helper, QBCountdownDialogTimerEventHandler);
    self->helper->clean = QBCountdownDialogClean;
    self->timeoutLabelGetter = timeoutLabelGetter;

    self->timerID = svAppTimerStart(res, self->helper, 1, false);
    self->timeout = -1;
    self->delay = 0;
    QBDialogParameters params = {
        .app        = res,
        .controller = controller,
        .widgetName = "CountdownDialog",
        .ownerId    = svWidgetGetId(self->helper),
    };
    self->dialog = QBDialogGlobalNew(&params, SVSTRING("CountdownDialog"));
    svWidgetAttach(self->dialog, self->helper, 0, 0, 0);
    self->panelTag = SvStringCreate("content", NULL);
    QBDialogAddPanel(self->dialog, self->panelTag, NULL, 1);
    QBDialogSetNotificationCallback(self->dialog, self, QBCountdownDialogNotificationCallback);

    self->okButton = QBDialogAddButton(self->dialog, SVSTRING("OK-button"), okMsg, 1);
    if (cancelMsg)
        self->cancelButton = QBDialogAddButton(self->dialog, SVSTRING("cancel-button"), cancelMsg, 2);

    if (okFocused || !self->cancelButton)
        svWidgetSetFocus(self->okButton);
    else
        svWidgetSetFocus(self->cancelButton);

    self->timeoutLabelTag = SVSTRING("timeout-message");

    return self;
}

void
QBCountdownDialogSetTitle(QBCountdownDialog self, char *title)
{
    if (!self || !title) {
        SvLogError("%s: NULL argument passed", __FUNCTION__);
        return;
    }

    QBDialogSetTitle(self->dialog, title);
}

SvWidget
QBCountdownDialogGetDialog(QBCountdownDialog self)
{
    if (!self) {
        SvLogError("%s: NULL argument passed", __FUNCTION__);
        return NULL;
    }

    return self->dialog;
}

SvWidget
QBCountdownDialogAddLabel(QBCountdownDialog self, char *text, SvString tag, unsigned int position)
{
    if (!self || !text || !tag) {
        SvLogError("%s: NULL argument passed", __FUNCTION__);
        return NULL;
    }

    return QBDialogAddLabel(self->dialog, self->panelTag, text, tag, position);
}

void
QBCountdownDialogAddTimerLabel(QBCountdownDialog self, SvString tag, unsigned int position)
{
    if (!self || !tag) {
        SvLogError("%s: NULL argument passed", __FUNCTION__);
        return;
    }

    SVTESTRELEASE(self->timeoutLabelTag);
    self->timeoutLabelTag = SVRETAIN(tag);
    self->timeoutLabelPosition = position;
}
