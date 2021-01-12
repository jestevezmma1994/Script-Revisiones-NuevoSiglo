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

#include "npvrDialog.h"
#include <QBWidgets/QBDialog.h>
#include <main.h>
#include <NPvr/QBnPVRProviderRequest.h>

#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/widget.h>
#include <SWL/button.h>
#include <settings.h>
#include <libintl.h>
#include <assert.h>

struct QBNPvrDialog_s
{
    struct SvObject_ super_;

    SvWidget dialog;
    SvWidget button;
    SvWidget helper;

    QBnPVRProviderRequest npvrRequest;
};
typedef struct QBNPvrDialog_s* QBNPvrDialog;

SvLocal void QBNPvrDialog_hide(QBNPvrDialog self)
{
    SvWidget dialog = self->dialog;
    self->dialog = NULL;
    QBDialogBreak(dialog);
}

SvLocal void QBNPvrDialog_recording_set_popup(QBNPvrDialog self, const char *titleStr, const char *bodyStr)
{
    SvWidget dialog = self->dialog;
    QBDialogSetTitle(dialog, titleStr);
    QBDialogAddLabel(dialog, SVSTRING("content"), bodyStr, SVSTRING("message"), 0);

    svButtonSetCaption(self->button, gettext("OK"));
}

SvLocal void QBNPvrDialog_recording_failed(QBNPvrDialog self, QBnPVRProviderRequest request)
{
    QBnPVRProviderRequestError error = QBnPVRProviderRequestError_none;
    QBnPVRProviderRequestException exception = QBnPVRProviderRequestException_none;
    const char* titleStr = gettext("Failed to schedule nPVR recording");

    if (request) {
        error = QBnPVRProviderRequestGetError(request);
        exception = QBnPVRProviderRequestGetException(request);
    } else {
        error = QBnPVRProviderRequestError_communication;
    }

    const char* bodyStr = NULL;
    switch (error) {
    case QBnPVRProviderRequestError_quota:
        bodyStr = gettext("Out of quota on nPVR server.");
        break;
    case QBnPVRProviderRequestError_rejected:
        bodyStr = gettext("nPVR server rejected recording request.");
        break;
    case QBnPVRProviderRequestError_communication:
        bodyStr = gettext("Failed to contact nPVR server.");
        break;
    default:
        if (request && exception != QBnPVRProviderRequestException_none) {
            bodyStr = QBnPVRProviderRequestGetExceptionMessage(request);
        }

        if (!bodyStr) {
            bodyStr = gettext("Unknown nPVR error.");
        }
        break;
    }

    QBNPvrDialog_recording_set_popup(self, titleStr, bodyStr);
}

SvLocal void QBNPvrDialog_recording_canceled(QBNPvrDialog self)
{
    const char* titleStr = gettext("Failed to schedule nPVR recording");
    const char* bodyStr = gettext("nPVR request has been canceled");

    QBNPvrDialog_recording_set_popup(self, titleStr, bodyStr);
}

SvLocal void QBNPvrDialogRequestFinishedWithError(QBNPvrDialog self)
{
    const char* titleStr = gettext("Server error");
    const char* bodyStr = gettext("Invalid server response");

    QBNPvrDialog_recording_set_popup(self, titleStr, bodyStr);
}

SvLocal void QBNPvrDialogRecordingsDeleteFailed(QBNPvrDialog self, QBnPVRProviderRequestError reason)
{
    const char* titleStr = gettext("Failed to delete nPVR recordings");
    const char* bodyStr = gettext("Unknown nPVR error.");

    QBNPvrDialog_recording_set_popup(self, titleStr, bodyStr);
}

SvLocal void QBNPvrDialogDirectoryDeleteFailed(QBNPvrDialog self, QBnPVRProviderRequest request)
{
    const char* titleStr = gettext("Failed to delete nPVR directory");
    const char* bodyStr = QBnPVRProviderRequestGetExceptionMessage(request);

    if (!bodyStr) {
        bodyStr = gettext("Unknown nPVR error.");
    }

    QBNPvrDialog_recording_set_popup(self, titleStr, bodyStr);
}

SvLocal void QBNPvrDialogRequestFailed(QBNPvrDialog self, QBnPVRProviderRequest request)
{
    const char* titleStr = gettext("nPVR action failed");
    const char* bodyStr = QBnPVRProviderRequestGetExceptionMessage(request);

    if (!bodyStr) {
        bodyStr = gettext("Unknown nPVR error.");
    }

    QBNPvrDialog_recording_set_popup(self, titleStr, bodyStr);
}

SvLocal bool QBNPvrDialogNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBNPvrDialog self = ptr;
    if (self->dialog) {
        if (!buttonTag)
            return false;
        QBNPvrDialog_hide(self);
        return true;
    }

    return false;
}

SvLocal void
QBNPvrDialogRequestStateChanged(SvGenericObject self_, QBnPVRProviderRequest request)
{
    QBNPvrDialog self = (QBNPvrDialog) self_;

    QBnPVRProviderRequestState state = QBnPVRProviderRequestGetState(request);
    QBnPVRProviderRequestError error = QBnPVRProviderRequestGetError(request);

    if (!self->dialog) {
        // too late dialog is already hidden (probably using Hide button)
        return;
    }

    if (state == QBnPVRProviderRequestState_finished) {
        if (error) {
            QBNPvrDialogRequestFinishedWithError(self);
            return;
        }

        /// TODO: implement a version with a "Success" message + OK button
        QBNPvrDialog_hide(self);
    } else if (state == QBnPVRProviderRequestState_failed) {
        switch (QBnPVRProviderRequestGetType(request)) {
        case QBnPVRProviderRequestType_scheduleRecording:
            QBNPvrDialog_recording_failed(self, request);
            break;
        case QBnPVRProviderRequestType_deleteRecordings:
            QBNPvrDialogRecordingsDeleteFailed(self, error);
            break;
        case QBnPVRProviderRequestType_deleteDirectory:
            QBNPvrDialogDirectoryDeleteFailed(self, request);
            break;
        default:
            QBNPvrDialogRequestFailed(self, request);
            break;
        }
    } else if (state == QBnPVRProviderRequestState_canceled) {
        switch (QBnPVRProviderRequestGetType(request)) {
        case QBnPVRProviderRequestType_scheduleRecording:
            QBNPvrDialog_recording_canceled(self);
            break;
        default:
            QBNPvrDialogRequestFailed(self, request);
            break;
        }
    }
}

SvLocal void
QBNPvrDialogDestroy(void *self_)
{
    QBNPvrDialog self = self_;

    if (self->npvrRequest) {
        QBnPVRProviderRequestRemoveListener(self->npvrRequest, (SvGenericObject) self);
        SVRELEASE(self->npvrRequest);
    }

    if (self->dialog) {
        QBNPvrDialog_hide(self);
    }
}

SvLocal SvType QBNPvrDialog_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNPvrDialogDestroy
    };
    static SvType type = NULL;

    static const struct QBnPVRProviderRequestListener_ nPVRRequestMethods = {
        .stateChanged = QBNPvrDialogRequestStateChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNPvrDialog",
                            sizeof(struct QBNPvrDialog_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBnPVRProviderRequestListener_getInterface(), &nPVRRequestMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBNPvrDialog_helper_cleanup(SvApplication app, void* prv)
{
    QBNPvrDialog self = (QBNPvrDialog) prv;
    SVRELEASE(self);
}

SvWidget QBNPvrDialogCreate(AppGlobals appGlobals, const QBNPvrDialogParams_t* params)
{
    assert(params != NULL);

    QBNPvrDialog self = (QBNPvrDialog) SvTypeAllocateInstance(QBNPvrDialog_getType(), NULL);

    svSettingsPushComponent("Dialog.settings");

    SvWidget helper = svWidgetCreate(appGlobals->res, 0, 0);

    const char* titleStr = gettext("Please wait");
    const char* bodyStr  = gettext("Waiting for nPVR server response...");

    struct QBDialogParameters_ dialParams = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(helper),
    };
    SvWidget dialog = QBDialogLocalNew(&dialParams, QBDialogLocalTypeFocusable);
    svWidgetAttach(dialog, helper, 0, 0, 0);
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    QBDialogSetTitle(dialog, titleStr);
    QBDialogAddLabel(dialog, SVSTRING("content"), bodyStr, SVSTRING("message"), 0);
    QBDialogSetNotificationCallback(dialog, self, QBNPvrDialogNotificationCallback);

    // Until dialog is displayed we don't want to allow
    // the next action with npvr.
    unsigned int keysToCatch[] = { QBKEY_REC, QBKEY_FUNCTION, 0 };
    QBDialogSetKeysToCatch(dialog, keysToCatch);

    svSettingsPopComponent();

    SvWidget button = QBDialogAddButton(dialog, SVSTRING("OK-button"), gettext("Hide"), 1);

    self->dialog = dialog;
    self->helper = helper;
    self->button = button;

    helper->prv = self;
    helper->clean = QBNPvrDialog_helper_cleanup;

    svWidgetSetFocus(button);

    if (params->npvrRequest) {
        self->npvrRequest = SVRETAIN(params->npvrRequest);
        /// register for notifications about "npvrRequest"
        QBnPVRProviderRequestAddListener(self->npvrRequest, (SvGenericObject) self);
    } else {
        /// report error immediately
        QBNPvrDialog_recording_failed(self, NULL);
    }

    return dialog;
}

void
QBNPvrDialogCreateAndShow(AppGlobals appGlobals, QBnPVRProviderRequest request)
{
    if (!request) {
        return;
    }

    /// success or failure : npvrDialog will display it
    const QBNPvrDialogParams_t  dialogParams = {
        .npvrRequest = request,
    };

    SvWidget dialog = QBNPvrDialogCreate(appGlobals, &dialogParams);
    QBDialogRun(dialog, NULL, NULL);
}
