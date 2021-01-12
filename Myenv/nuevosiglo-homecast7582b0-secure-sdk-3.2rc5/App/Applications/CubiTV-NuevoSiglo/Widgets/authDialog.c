/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#include "authDialog.h"

#include <libintl.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/event.h>
#include <settings.h>
#include <SWL/label.h>
#include <QBWidgets/QBPINInput.h>
#include <QBWidgets/QBDialog.h>
#include <main.h>
#include <Utils/authenticators.h>
#include <Services/QBAccessController/QBAccessManager.h>
#include <Services/QBAccessController/Innov8onAccessPlugin.h>
#include <Services/QBAccessController/ConaxAccessPlugin.h>
#include <Services/QBParentalControl/QBParentalControl.h>


#define log_debug(fmt, ...)  if(0) SvLogNotice(COLBEG() "QBAuthDialog: %s:%d " fmt  COLEND_COL(yellow), __func__,__LINE__,##__VA_ARGS__)

struct QBAuthDialog_t {
    SvWidget dialog;
    SvWidget pinEntry;
    SvWidget disablePCButton;
    SvWidget okButton;
    SvWidget cancelButton;
    SvWidget helper;
    SvWidget message;
    SvWidget infoLabel;
    SvWidget waitLabel;

    SvGenericObject authenticator;
    SvString domain;

    SvTimerId waitTimer;
    SvTimerId downcounterTimer;
    int waitTicks;
    int minutesLeft;
    bool mandatory;
    bool showAdditionalInfo;

    const char *title;
    const char *invalidPin;
    const char *blockedPin;
    const char *rejectedPin;
    const char *notAuthorized;
    const char *wait;
    const char *PINEntryTitle;
    char *initialMessage;

    AppGlobals appGlobals;
    bool needAuth;
    unsigned int settingsCtx;

    /** If entering pin is disabled - it happens when we user should get only info about wrong pin without possibility to re-enter it */
    bool enteringPinDisabled;
    bool disablePCChosen;
    bool disablePCOption;
};
typedef struct QBAuthDialog_t* QBAuthDialog;

//TODO - implement reinitializable in case the dialog is run as a global window
SvInterface QBAuthDialogAuthenticator_getInterface(void)
{
    static SvInterface interface = NULL;
    if(!interface) {
        SvInterfaceCreateManaged("QBAuthDialogAuthenticator", sizeof(struct QBAuthDialogAuthenticator_t),
                                 NULL, &interface, NULL);
    }
    return interface;
}

SvLocal void QBAuthDialog__dtor__(SvApplication app, void *ptr)
{
    QBAuthDialog self = ptr;
    if (self->downcounterTimer)
        svAppTimerStop(app, self->downcounterTimer);
    if (self->waitTimer)
        svAppTimerStop(app, self->waitTimer);
    self->downcounterTimer = 0;
    SVTESTRELEASE(self->domain);
    SVRELEASE(self->authenticator);
    if (self->initialMessage) {
        free(self->initialMessage);
        self->initialMessage = NULL;
    }

    free(self);
}

SvLocal void QBAuthDialogCheck(SvWidget w){
    QBAuthDialog self = w->prv;
    if (!self->pinEntry) {
        QBDialogSetExitValue(self->dialog, SVSTRING("cancel-button"), 0);
        SvWidget dialog = self->dialog;
        self->dialog = NULL;
        QBDialogBreak(dialog);
        return;
    } else if(strlen(QBPINInputGetValue(self->pinEntry)) != 4) {
        QBDialogSetExitValue(self->dialog, NULL, 0);
        return;
    }
    self->waitTimer = svAppTimerStart(w->app, self->helper, 0.4, false);
    svWidgetSetFocusable(self->helper, true);
    svWidgetSetFocus(self->helper);
    svWidgetSetFocusable(self->helper, false);
    svWidgetSetFocusable(self->pinEntry, false);
    if(self->okButton)
        svWidgetSetFocusable(self->okButton, false);
    if(self->cancelButton)
        svWidgetSetFocusable(self->cancelButton, false);
    if (self->disablePCButton) {
        svWidgetSetFocusable(self->disablePCButton, false);
    }

    SvInvokeInterface(QBAuthDialogAuthenticator, self->authenticator, checkPin, self->helper, QBPINInputGetValue(self->pinEntry));
}

SvLocal void QBAuthDialogOnPIN(void *ptr, SvWidgetId sender, bool isEnter, const char *pin)
{
    QBAuthDialog self = (QBAuthDialog) ptr;
    if (self->pinEntry) {
        if(self->okButton)
            svWidgetSetFocus(self->okButton);
        else
            QBAuthDialogCheck(self->helper);
        svWidgetForceRender(self->helper);
    }
}

SvLocal void QBAuthDialogSetToBaseMode(QBAuthDialog self)
{
    if (!self->dialog)
        return;

    QBDialogRemoveItem(self->dialog, SVSTRING("info-label"));

    svSettingsRestoreContext(self->settingsCtx);
    if (!self->mandatory)
        self->cancelButton = QBDialogAddButton(self->dialog, SVSTRING("cancel-button"), gettext("Cancel"), 2);

    if (self->disablePCOption) {
        self->disablePCButton = QBDialogAddButton(self->dialog, SVSTRING("disablePC-button"), gettext("Disable PC"), 3);
    }

    QBDialogAlignButtons(self->dialog);

    self->message = QBDialogAddLabel(self->dialog, SVSTRING("content"), self->initialMessage, SVSTRING("message"), 0);
    //create PIN input field
    QBAuthParams params;
    SvInvokeInterface(QBAuthDialogAuthenticator, self->authenticator, checkAuthParams, &params);

    int width = svSettingsGetInteger("Dialog.InputField", "width", -1);
    int height = svSettingsGetInteger("Dialog.InputField", "height", -1);

    SvWidget caption, input;
    SvWidget PinField = svWidgetCreateBitmap(self->appGlobals->res, width, height, NULL);
    caption = svLabelNewFromSM(self->appGlobals->res, "Dialog.InputField.Caption");
    svLabelSetText(caption, self->PINEntryTitle);

    static struct QBPINInputCallbacks_t callbacks_qbinput = {
        .onPIN = QBAuthDialogOnPIN
    };

    input = QBPINInputNew(self->appGlobals->res, "Dialog.InputField.PINInput", params.password.maxLength,
                        &callbacks_qbinput, self, NULL);

    int captionYOffset = svSettingsGetInteger("Dialog.InputField.Caption", "yOffset", -1);
    int pinYOffset = svSettingsGetInteger("Dialog.InputField.PINInput", "yOffset", -1);
    int captionXOffset = -1;
    int pinXOffset = -1;
    int spacing = svSettingsGetInteger("Dialog.InputField", "spacing", 0);

    if (caption->width < (PinField->width / 2 - input->width / 2)) {
        pinXOffset = svSettingsGetInteger("Dialog.InputField.PINInput", "xOffset", -1);
        captionXOffset = pinXOffset - caption->width - spacing / 2;
    } else {
        captionXOffset = 0;
        pinXOffset = caption->width + spacing;
        int panelSpacing = svSettingsGetInteger("Dialog", "panelSpacing", 0);
        int overlap = (pinXOffset + input->width) - (self->dialog->width - 2*panelSpacing);
        // don't allow input widget to be pushed outside of dialogs viewport
        if (overlap > 0) {
            pinXOffset = self->dialog->width - input->width - 2*panelSpacing;
            captionXOffset = -overlap;
        }
    }
    svWidgetAttach(PinField, caption, captionXOffset, captionYOffset, 1);
    svWidgetAttach(PinField, input, pinXOffset, pinYOffset, 1);

    QBDialogAddWidget(self->dialog, SVSTRING("content"), PinField, SVSTRING("PIN-entry"), 3, NULL);
    self->pinEntry = input;
    svWidgetSetFocus(self->pinEntry);
    svSettingsPopComponent();
}

SvLocal void QBAuthDialogSetToBlockedMode(QBAuthDialog self)
{
    if (self->message) {
        QBDialogRemoveItem(self->dialog, SVSTRING("message"));
        self->message = NULL;
    }
    if (self->cancelButton) {
        QBDialogRemoveItem(self->dialog, SVSTRING("cancel-button"));
        self->cancelButton = NULL;
    }

    if (self->disablePCButton) {
        QBDialogRemoveItem(self->dialog, SVSTRING("disablePC-button"));
        self->disablePCButton = NULL;
    }

    if (self->pinEntry) {
        QBDialogRemoveItem(self->dialog, SVSTRING("PIN-entry"));
        self->pinEntry = NULL;
    }
    int minutesLeft = QBAccessManagerGetBlockPeriodLeft(self->appGlobals->accessMgr) / 60;
    if (minutesLeft != self->minutesLeft || !self->infoLabel) {
        self->minutesLeft = minutesLeft;
        SvString infoMsg = SvStringCreateWithFormat(ngettext("PIN authentication blocked for %d minute", "PIN authentication blocked for %d minutes", 1 + minutesLeft), 1 + minutesLeft);
        SvString msg = self->showAdditionalInfo ? SvStringCreateWithFormat("%s %s", self->invalidPin, SvStringCString(infoMsg)) : SVRETAIN(infoMsg);
        self->infoLabel = QBDialogAddLabel(self->dialog, SVSTRING("content"), SvStringCString(msg), SVSTRING("info-label"), 1);
        SVRELEASE(infoMsg);
        SVRELEASE(msg);
    }
}

SvLocal void QBAuthDialogPinCheckFailed(QBAuthDialog self, QBAuthStatus status)
{
    if (self->infoLabel)
        QBDialogRemoveItem(self->dialog, SVSTRING("info-label"));

    if (self->okButton)
        svWidgetSetFocusable(self->okButton, true);
    if (self->cancelButton)
        svWidgetSetFocusable(self->cancelButton, true);
    if (self->disablePCButton) {
        svWidgetSetFocusable(self->disablePCButton, true);
        self->disablePCChosen = false;
    }

    if (status == QBAuthStatus_invalid) {
        int attemptsLeft = QBAccessManagerGetPinAttemptsLeft(self->appGlobals->accessMgr, self->domain);
        if (attemptsLeft == 0) {
            if (self->okButton)
                svWidgetSetFocus(self->okButton);
            QBAuthDialogSetToBlockedMode(self);
        } else {
            SvString invalidPinMessage = NULL;
            if (attemptsLeft > 0) {
                SvString attemptsLeftMsg = SvStringCreateWithFormat(ngettext("%d attempt left.", "%d attempts left", attemptsLeft), attemptsLeft);
                invalidPinMessage = SvStringCreateWithFormat("%s %s", self->invalidPin, attemptsLeftMsg ? SvStringCString(attemptsLeftMsg) : "");
                SVTESTRELEASE(attemptsLeftMsg);
            } else {
                invalidPinMessage = SvStringCreate(self->invalidPin, NULL);
            }
            self->infoLabel = QBDialogAddLabel(self->dialog, SVSTRING("content"), SvStringCString(invalidPinMessage), SVSTRING("info-label"), 1);
            SVRELEASE(invalidPinMessage);
        }

        if (attemptsLeft >= 0)
            self->downcounterTimer = svAppTimerStart(self->appGlobals->res, self->helper, 1, false);
    } else if (status == QBAuthStatus_failed) {
        self->infoLabel = QBDialogAddLabel(self->dialog, SVSTRING("content"), self->invalidPin, SVSTRING("info-label"), 1);
    }

    if (self->waitLabel) {
       QBDialogRemoveItem(self->dialog, SVSTRING("wait-label"));
       self->waitLabel = NULL;
    }
    if (self->pinEntry) {
        QBPINInputClear(self->pinEntry);
        svWidgetSetFocusable(self->pinEntry, true);
    }
    if (self->pinEntry)
        svWidgetSetFocus(self->pinEntry);
    QBDialogSetExitValue(self->dialog, NULL, 0);
    svAppTimerStop(self->helper->app, self->waitTimer);
    self->waitTimer = 0;
}

SvLocal void QBAuthDialogPinIncorrect(QBAuthDialog self, bool blocked)
{
    if (self->infoLabel)
        QBDialogRemoveItem(self->dialog, SVSTRING("info-label"));

    if (self->pinEntry) {
        QBDialogRemoveItem(self->dialog, SVSTRING("PIN-entry"));
        self->pinEntry = NULL;
    }

    SvString incorrectPinMessage = NULL;
    if (blocked) {
        incorrectPinMessage = SvStringCreate(self->blockedPin, NULL);
    } else {
        incorrectPinMessage = SvStringCreate(self->rejectedPin, NULL);
    }
    self->infoLabel = QBDialogAddLabel(self->dialog, SVSTRING("content"), SvStringCString(incorrectPinMessage), SVSTRING("info-label"), 1);
    SVRELEASE(incorrectPinMessage);

    self->okButton = QBDialogAddButton(self->dialog, SVSTRING("OK-button"), gettext("OK"), 1);
    svWidgetSetFocus(self->okButton);

    self->enteringPinDisabled = true;

    QBDialogSetExitValue(self->dialog, NULL, 0);
    svAppTimerStop(self->helper->app, self->waitTimer);
    self->waitTimer = 0;
}

void QBAuthDialogNotAuthorized(SvWidget helper)
{
    log_debug();
    QBAuthDialog self = helper->prv;
    if (self->infoLabel) {
        QBDialogRemoveItem(self->dialog, SVSTRING("info-label"));
    }

    if (self->pinEntry) {
        QBDialogRemoveItem(self->dialog, SVSTRING("PIN-entry"));
        self->pinEntry = NULL;
    }

    SvString notAuthorized = SvStringCreate(self->notAuthorized, NULL);

    self->infoLabel = QBDialogAddLabel(self->dialog, SVSTRING("content"), SvStringCString(notAuthorized), SVSTRING("info-label"), 1);
    SVRELEASE(notAuthorized);

    self->okButton = QBDialogAddButton(self->dialog, SVSTRING("OK-button"), gettext("OK"), 1);
    svWidgetSetFocus(self->okButton);

    self->enteringPinDisabled = true;

    QBDialogSetExitValue(self->dialog, NULL, 0);
    svAppTimerStop(self->helper->app, self->waitTimer);
    self->waitTimer = 0;
}

bool QBAuthDialogIsEnteringPinDisabled(SvWidget helper)
{
    QBAuthDialog self = helper->prv;
    return self->enteringPinDisabled;
}


SvLocal bool QBAuthDialogNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBAuthDialog self = ptr;
    if (self->dialog) {
        if (!buttonTag) {
            if (keyCode == QBKEY_BACK) {
                if (!self->mandatory) {
                    SvInvokeInterface(QBAuthDialogAuthenticator, self->authenticator, cancelPinCheck);
                    self->dialog = NULL;
                    QBDialogBreak(dialog);
                } else
                    return false;
            }
        } else {
            if(SvStringEqualToCString(buttonTag, "OK-button")) {
                if (!self->needAuth)
                    QBAuthDialogPinAccepted(self->helper);
                else
                    QBAuthDialogCheck(self->helper);
            } else if(SvStringEqualToCString(buttonTag, "cancel-button")) {
                self->dialog = NULL;
                QBDialogBreak(dialog);
            } else if (SvStringEqualToCString(buttonTag, "disablePC-button")) {
                self->disablePCChosen = true;
                if (!self->needAuth) {
                    QBAuthDialogPinAccepted(self->helper);
                } else {
                    QBAuthDialogCheck(self->helper);
                }
            }
        }
        svWidgetForceRender(self->helper);
        return true;
    }

    return false;
}

SvLocal void QBAuthDialogTimerEventHandler(SvWidget w, SvTimerEvent e)
{
    QBAuthDialog self = w->prv;
    if(e->id == self->waitTimer && self->dialog) {
        if (!self->waitLabel) {
            self->waitLabel = QBDialogAddLabel(self->dialog, SVSTRING("content"),
                self->wait,
                SVSTRING("wait-label"), 4);
        }
        if (self->infoLabel) {
            QBDialogRemoveItem(self->dialog, SVSTRING("info-label"));
            self->infoLabel = NULL;
        }

        // FIXME: add real animation!
        char buff[31];
        snprintf(buff, sizeof(buff), "%s", self->wait);
        self->waitTicks++;
        if ((self->waitTicks % 3) == 1)
            strcat(buff, ".");
        else if ((self->waitTicks % 3) == 2)
            strcat(buff, "..");
        svLabelSetText(self->waitLabel, buff);

        svWidgetForceRender(w); // FIXME: is it needed here?
        return;
    } else if (e->id == self->downcounterTimer && self->dialog) {
        if (QBAccessManagerGetBlockPeriodLeft(self->appGlobals->accessMgr) <= 0) {
            svAppTimerStop(self->appGlobals->res, self->downcounterTimer);
            self->downcounterTimer = 0;
            self->showAdditionalInfo = true;
            self->minutesLeft = -1;
            QBAuthDialogSetToBaseMode(self);
        } else if (QBAccessManagerGetPinAttemptsLeft(self->appGlobals->accessMgr, self->domain) == 0) {
            QBAuthDialogSetToBlockedMode(self);
        }

        svWidgetForceRender(w); // FIXME: is it needed here?
        return;
    }
}

void QBAuthDialogPinAccepted(SvWidget helper)
{
    log_debug();
    QBAuthDialog self = helper->prv;
    if (self->disablePCChosen) {
        QBDialogSetExitValue(self->dialog, SVSTRING("disablePC-button"), 0);
        QBParentalControlSetState(self->appGlobals->pc, SVSTRING("OFF"));
        self->disablePCChosen = false;
    }
    QBDialogSetExitValue(self->dialog, SVSTRING("OK-button"), 0);
    SvWidget dialog = self->dialog;
    self->dialog = NULL;
    QBDialogBreak(dialog);
}

void QBAuthDialogPinDeclined(SvWidget helper)
{
    log_debug();
    QBAuthDialog self = helper->prv;
    QBAuthDialogPinCheckFailed(self, QBAuthStatus_invalid);
}

void QBAuthDialogPinRejected(SvWidget helper)
{
    log_debug();
    QBAuthDialog self = helper->prv;
    QBAuthDialogPinIncorrect(self, false);
}

void QBAuthDialogPinBlocked(SvWidget helper)
{
    log_debug();
    QBAuthDialog self = helper->prv;
    QBAuthDialogPinIncorrect(self, true);
}

void QBAuthDialogPinFailed(SvWidget helper)
{
    QBAuthDialog self = helper->prv;
    QBAuthDialogPinCheckFailed(self, QBAuthStatus_failed);
}

void QBAuthDialogSetMessageWidget(SvWidget dialog, SvWidget w)
{
    QBDialogRemoveItem(dialog, SVSTRING("message"));
    QBDialogAddWidget(dialog, SVSTRING("content"), w, SVSTRING("message"), 0, NULL);
}

SvLocal void
QBAuthDialogGetMessageToShow(QBAuthDialog self, SvString domain)
{
    if ( !domain || !self )
        return;

    SvGenericObject plugin = QBAccessManagerGetDomainPlugin(self->appGlobals->accessMgr, domain);
    SvString name = SvInvokeInterface(QBAccessPlugin, plugin, getName);
    if ( name ) {
       if ( svSettingsIsWidgetDefined("AuthenticationDialog") ) {
           char* tag = 0;
           const char* tmp = strstr(SvStringCString(name), "AccessPlugin");
           if ( tmp ) {
               SvString shortName = SvStringCreateSubString(name, 0, SvStringLength(name) - strlen(tmp), 0);
               if ( shortName && asprintf(&tag, "caption%s", SvStringCString(shortName)) ) {
                   const char* text = svSettingsGetString("AuthenticationDialog", tag);
                   if ( text ) {
                       self->initialMessage = strdup(gettext(text));
                   }
                   free(tag);
               }
               SVTESTRELEASE(shortName);
           }
       }
    }
}

SvWidget QBAuthDialogCreate(AppGlobals appGlobals, SvGenericObject authenticator, const char *title, const char *message,
                            bool local, SvString globalName, SvWidget *helperPtr)
{
    SvWidget dialog = NULL;
    SvWidget helper = NULL;
    SvString domain = authenticator ? SvInvokeInterface(QBAuthDialogAuthenticator, authenticator, getDomain) : NULL;

    QBAuthDialog self = calloc(1, sizeof(struct QBAuthDialog_t));
    self->appGlobals = appGlobals;
    self->invalidPin = NULL;
    if (message) {
        self->initialMessage = strdup(message);
    } else {
        QBAuthDialogGetMessageToShow(self, domain);
    }
    if (svSettingsIsWidgetDefined("AuthenticationDialog")) {
        const char * tmp;
        if ((tmp = svSettingsGetString("AuthenticationDialog", "invalidPin")))
            self->invalidPin = gettext(tmp);
        if ((tmp = svSettingsGetString("AuthenticationDialog", "rejectedPin")))
            self->rejectedPin = gettext(tmp);
        if ((tmp = svSettingsGetString("AuthenticationDialog", "blockedPin")))
            self->blockedPin = gettext(tmp);
        if ((tmp = svSettingsGetString("AuthenticationDialog", "notAuthorized")))
            self->notAuthorized = gettext(tmp);
        if ((tmp = svSettingsGetString("AuthenticationDialog", "title")))
            self->title = gettext(tmp);
        if ((tmp = svSettingsGetString("AuthenticationDialog", "wait")))
            self->wait = gettext(tmp);
        SvString pinEntryTitle = SvStringCreateWithFormat("pinEntryTitle%s", domain ? SvStringCString(domain) : "");
        if ((tmp = svSettingsGetString("AuthenticationDialog", SvStringCString(pinEntryTitle))))
            self->PINEntryTitle = gettext(tmp);
        SVRELEASE(pinEntryTitle);
    }

    if (!self->title)
        self->title = title;
    if (!self->invalidPin)
        self->invalidPin = gettext("Authentication failed. Try again.");
    if (!self->rejectedPin)
        self->rejectedPin = gettext("Authentication failed. Wrong Pin was entered.");
    if (!self->blockedPin)
        self->blockedPin = gettext("Authentication failed - PIN is blocked.");
    if (!self->notAuthorized)
        self->notAuthorized = gettext("Not authorized.");
    if (!self->wait)
        self->wait = gettext("Please wait.");
    if (!self->PINEntryTitle)
        self->PINEntryTitle = gettext("PIN:");

    helper = svWidgetCreateBitmap(appGlobals->res, 0, 0, NULL);
    svWidgetSetTimerEventHandler(helper, QBAuthDialogTimerEventHandler);
    helper->prv = self;
    helper->clean = QBAuthDialog__dtor__;

    if(helperPtr)
        *helperPtr = helper;

    self->helper = helper;
    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(helper),
    };
    if (local) {
        dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    } else {
        dialog = QBDialogGlobalNew(&params, globalName);
    }
    unsigned int globalKeys[] = {QBKEY_BACK, 0};
    QBDialogSetGlobalKeys(dialog, globalKeys);
    svWidgetAttach(dialog, helper, 0, 0, 0);
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);

    QBDialogSetNotificationCallback(dialog, self, QBAuthDialogNotificationCallback);
    QBDialogSetTitle(dialog, self->title);

    self->dialog = dialog;
    self->mandatory = svSettingsGetBoolean("AuthenticationDialog", "mandatory", false);
    self->authenticator = authenticator;
    self->domain = SVTESTRETAIN(domain);

    QBAccessCode code = SvInvokeInterface(QBAuthDialogAuthenticator, self->authenticator, checkAccess, self->helper);
    if(code == QBAccess_authenticationNotRequired)
        self->needAuth = false;
    else
        self->needAuth = true;

    if(!self->mandatory)
        self->okButton = QBDialogAddButton(dialog, SVSTRING("OK-button"), gettext("OK"), 1);

    int secondsLeft = QBAccessManagerGetBlockPeriodLeft(self->appGlobals->accessMgr);
    self->minutesLeft = -1;
    self->settingsCtx = svSettingsSaveContext();

    if (!self->needAuth) {
        self->cancelButton = QBDialogAddButton(dialog, SVSTRING("cancel-button"), gettext("Cancel"), 2);
        self->message = QBDialogAddLabel(self->dialog, SVSTRING("content"), self->initialMessage, SVSTRING("message"), 0);
        svWidgetSetFocus(self->cancelButton);
    } else if (secondsLeft > 0 && QBAccessManagerGetPinAttemptsLeft(self->appGlobals->accessMgr, self->domain) == 0) {
        if (self->okButton)
            svWidgetSetFocus(self->okButton);
        self->showAdditionalInfo = false;
        QBAuthDialogSetToBlockedMode(self);
        self->downcounterTimer = svAppTimerStart(self->appGlobals->res, self->helper, 1, false);
    } else {
        self->showAdditionalInfo = true;
        QBAuthDialogSetToBaseMode(self);
    }

    self->disablePCOption = svSettingsGetBoolean("AuthenticationDialog", "disablePC", false);
    if (self->disablePCOption && (QBParentalControlGetState(self->appGlobals->pc) == QBParentalControlState_On) && self->showAdditionalInfo && !self->mandatory) {
        self->disablePCButton = QBDialogAddButton(dialog, SVSTRING("disablePC-button"), gettext("Disable PC"), 3);
    }

    QBDialogAlignButtons(dialog);

    return dialog;
}

// BEGIN AMERELES Support authentication dialog with no-pin requirement (purchase VoD without PIN)
SvWidget QBConfCondAuthDialogCreate(AppGlobals appGlobals, SvGenericObject authenticator, const char *title, const char *message,
                            bool local, SvString globalName, SvWidget *helperPtr, 
                            const char *text_button_ok, const char *text_button_cancel, bool requireAuth, bool focusOK)
{
    SvWidget dialog = NULL;
    SvWidget helper = NULL;
    SvString domain = authenticator ? SvInvokeInterface(QBAuthDialogAuthenticator, authenticator, getDomain) : NULL;

    QBAuthDialog self = calloc(1, sizeof(struct QBAuthDialog_t));
    self->appGlobals = appGlobals;
    self->invalidPin = NULL;
    if (message) {
        self->initialMessage = strdup(message);
    } else {
        QBAuthDialogGetMessageToShow(self, domain);
    }
    if (svSettingsIsWidgetDefined("AuthenticationDialog")) {
        const char * tmp;
        if ((tmp = svSettingsGetString("AuthenticationDialog", "invalidPin")))
            self->invalidPin = gettext(tmp);
        if ((tmp = svSettingsGetString("AuthenticationDialog", "rejectedPin")))
            self->rejectedPin = gettext(tmp);
        if ((tmp = svSettingsGetString("AuthenticationDialog", "blockedPin")))
            self->blockedPin = gettext(tmp);
        if ((tmp = svSettingsGetString("AuthenticationDialog", "notAuthorized")))
            self->notAuthorized = gettext(tmp);
        if ((tmp = svSettingsGetString("AuthenticationDialog", "title")))
            self->title = gettext(tmp);
        if ((tmp = svSettingsGetString("AuthenticationDialog", "wait")))
            self->wait = gettext(tmp);
        SvString pinEntryTitle = SvStringCreateWithFormat("pinEntryTitle%s", domain ? SvStringCString(domain) : "");
        if ((tmp = svSettingsGetString("AuthenticationDialog", SvStringCString(pinEntryTitle))))
            self->PINEntryTitle = gettext(tmp);
        SVRELEASE(pinEntryTitle);
    }

    if (title)
        self->title = title;
    if (!self->invalidPin)
        self->invalidPin = gettext("Authentication failed. Try again.");
    if (!self->rejectedPin)
        self->rejectedPin = gettext("Authentication failed. Wrong Pin was entered.");
    if (!self->blockedPin)
        self->blockedPin = gettext("Authentication failed - PIN is blocked.");
    if (!self->notAuthorized)
        self->notAuthorized = gettext("Not authorized.");
    if (!self->wait)
        self->wait = gettext("Please wait.");
    if (!self->PINEntryTitle)
        self->PINEntryTitle = gettext("PIN:");

    helper = svWidgetCreateBitmap(appGlobals->res, 0, 0, NULL);
    svWidgetSetTimerEventHandler(helper, QBAuthDialogTimerEventHandler);
    helper->prv = self;
    helper->clean = QBAuthDialog__dtor__;

    if(helperPtr)
        *helperPtr = helper;

    self->helper = helper;
    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(helper),
    };
    if (local) {
        dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    } else {
        dialog = QBDialogGlobalNew(&params, globalName);
    }
    unsigned int globalKeys[] = {QBKEY_BACK, 0};
    QBDialogSetGlobalKeys(dialog, globalKeys);
    svWidgetAttach(dialog, helper, 0, 0, 0);
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);

    QBDialogSetNotificationCallback(dialog, self, QBAuthDialogNotificationCallback);
    QBDialogSetTitle(dialog, self->title);

    self->dialog = dialog;
    self->mandatory = svSettingsGetBoolean("AuthenticationDialog", "mandatory", false);
    self->authenticator = authenticator;
    self->domain = SVTESTRETAIN(domain);

    if(requireAuth == false)
    {
        self->needAuth = false;
    }
    else
    {
        QBAccessCode code = SvInvokeInterface(QBAuthDialogAuthenticator, self->authenticator, checkAccess, self->helper);
        if(code == QBAccess_authenticationNotRequired)
            self->needAuth = false;
        else
            self->needAuth = true;
    }

    if(!self->mandatory)
        self->okButton = QBDialogAddButton(dialog, SVSTRING("OK-button"), text_button_ok, 1);

    int secondsLeft = QBAccessManagerGetBlockPeriodLeft(self->appGlobals->accessMgr);
    self->minutesLeft = -1;
    self->settingsCtx = svSettingsSaveContext();

    if (!self->needAuth) {
        self->cancelButton = QBDialogAddButton(dialog, SVSTRING("cancel-button"), text_button_cancel, 2);
        self->message = QBDialogAddLabel(self->dialog, SVSTRING("content"), self->initialMessage, SVSTRING("message"), 0);
        if (focusOK && self->okButton)
            svWidgetSetFocus(self->okButton);
        else
            svWidgetSetFocus(self->cancelButton);
    } else if (secondsLeft > 0 && QBAccessManagerGetPinAttemptsLeft(self->appGlobals->accessMgr, self->domain) == 0) {
        if (self->okButton)
            svWidgetSetFocus(self->okButton);
        self->showAdditionalInfo = false;
        QBAuthDialogSetToBlockedMode(self);
        self->downcounterTimer = svAppTimerStart(self->appGlobals->res, self->helper, 1, false);
    } else {
        self->showAdditionalInfo = true;
        QBAuthDialogSetToBaseMode(self);
    }

    self->disablePCOption = svSettingsGetBoolean("AuthenticationDialog", "disablePC", false);
    if (self->disablePCOption && (QBParentalControlGetState(self->appGlobals->pc) == QBParentalControlState_On) && self->showAdditionalInfo && !self->mandatory) {
        self->disablePCButton = QBDialogAddButton(dialog, SVSTRING("disablePC-button"), gettext("Disable PC"), 3);
    }

    QBDialogAlignButtons(dialog);

    return dialog;
}
// AMERELES END Support authentication dialog with no-pin requirement (purchase VoD without PIN)
