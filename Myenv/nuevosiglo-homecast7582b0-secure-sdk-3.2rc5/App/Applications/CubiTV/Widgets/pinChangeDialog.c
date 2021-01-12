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

#include "pinChangeDialog.h"

#include <libintl.h>
#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/event.h>
#include <settings.h>
#include <SWL/label.h>
#include <Services/QBAccessController/QBAccessManager.h>
#include <QBWidgets/QBPINInput.h>
#include <QBWidgets/QBDialog.h>
#include <QBConf.h>
#include <main.h>

struct QBPinChangeDialog_t {
   SvWidget helper;

   QBAccessManager accessMgr;
   SvString accountID;
   SvString domain;

   SvWidget dialog;
   SvWidget infoLabel;
   SvWidget PINEntry;
   SvWidget NewPIN1Entry;
   SvWidget NewPIN2Entry;
   SvWidget OKButton;
   SvWidget waitLabel;

   unsigned int waitTicks;
   int minutesLeft;
   SvTimerId waitTimer;
   SvTimerId downcounterTimer;

   bool changedPin;
   bool showAdditionalInfo;

   const char *pinsDontMatch;
   const char *oldPinInvalid;
   const char *pinChangeFailed;
   const char *pinChangeSuccessful;
   const char *wait;
   const char *oldPinFieldIncorrect;
   const char *newPinField1Incorrect;
   const char *newPinField2Incorrect;

   bool oldPinRequired;

   char *widgetName;

   char *newPin;

   int magic;

   AppGlobals appGlobals;

   unsigned int settingsCtx;

   bool baseMode;
};
typedef struct QBPinChangeDialog_t* QBPinChangeDialog;

SvLocal void QBPinChangeDialogTimerEventHandler(SvWidget helper, SvTimerEvent ev);

SvLocal unsigned int magicTab[] = {QBKEY_GREEN, QBKEY_YELLOW, QBKEY_GREEN, QBKEY_RED, QBKEY_YELLOW};
SvLocal int magicLen = sizeof(magicTab) / sizeof(magicTab[0]);

//TODO - implement reinitializable in case the dialog is run as a global window
SvLocal void QBPinChangeDialogClean(SvApplication app, void *ptr)
{
    QBPinChangeDialog self = ptr;
    if(self->waitTimer)
        svAppTimerStop(app, self->waitTimer);
    SVTESTRELEASE(self->domain);
    SVTESTRELEASE(self->accountID);
    free(self->widgetName);
    free(self->newPin);
    free(self);
}

SvLocal void QBPinChangeSuccessful(QBPinChangeDialog self)
{
    self->newPin = strdup(QBPINInputGetValue(self->NewPIN1Entry));

    QBDialogRemoveItem(self->dialog, SVSTRING("cancel-button"));
    QBDialogRemoveItem(self->dialog, SVSTRING("info-label"));
    QBDialogRemoveItem(self->dialog, SVSTRING("wait-label"));
    if (self->PINEntry)
        QBDialogRemoveItem(self->dialog, SVSTRING("PIN-field"));
    QBDialogRemoveItem(self->dialog, SVSTRING("NewPIN1-field"));
    QBDialogRemoveItem(self->dialog, SVSTRING("NewPIN2-field"));
    self->infoLabel = QBDialogAddLabel(self->dialog, SVSTRING("content"),
                                  gettext(self->pinChangeSuccessful), SVSTRING("info-label"), 1);
    if (self->OKButton) {
        svWidgetSetFocusable(self->OKButton, true);
        svWidgetSetFocus(self->OKButton);
    }

    self->changedPin = true;
}

SvLocal void
QBPinChangeDialogOnPIN(void *ptr, SvWidgetId sender, bool isEnter, const char *pin)
{
    QBPinChangeDialog self = (QBPinChangeDialog) ptr;

    if (sender == svWidgetGetId(self->NewPIN1Entry)) {
        svWidgetSetFocus(self->NewPIN2Entry);
    } else if (sender == svWidgetGetId(self->NewPIN2Entry)) {
        svWidgetSetFocus(self->OKButton);
    } else if (sender == svWidgetGetId(self->PINEntry)) {
        svWidgetSetFocus(self->NewPIN1Entry);
    }
}

SvLocal void
QBPinChangeDialogSetToBaseMode(QBPinChangeDialog self)
{
    if (!self->dialog || self->baseMode)
        return;

    QBDialogRemoveItem(self->dialog, SVSTRING("info-label"));

    SvWidget okButton;
    QBAuthParams params;

    QBAccessManagerGetAuthParamsByDomain(self->appGlobals->accessMgr, &params, self->domain, NULL);

    const char* pinEntryTitle[3];
    svSettingsRestoreContext(self->settingsCtx);
    pinEntryTitle[0] = gettext(svSettingsGetString(self->widgetName, "oldPinEntryTitle"));
    pinEntryTitle[1] = gettext(svSettingsGetString(self->widgetName, "newPIN1EntryTitle"));
    pinEntryTitle[2] = gettext(svSettingsGetString(self->widgetName, "newPIN2EntryTitle"));

    svSettingsPushComponent("PINChangeDialog.settings");

    QBDialogAddPanel(self->dialog, SVSTRING("content"), NULL, 1);

    int width = svSettingsGetInteger("Dialog.InputField", "width", -1);
    int height = svSettingsGetInteger("Dialog.InputField", "height", -1);

    static struct QBPINInputCallbacks_t callbacks_qbinput = {
        .onPIN = QBPinChangeDialogOnPIN,
    };

    SvWidget field[3];
    SvWidget caption[3];
    SvWidget input[3];
    int longestInput = -1;
    for (int i = 0; i < 3; i++) {
        if (i == 0 && !self->oldPinRequired) {
            field[i] = NULL;
            caption[i] = NULL;
            input[i] = NULL;
            continue;
        }

        field[i] = svWidgetCreateBitmap(self->appGlobals->res, width, height, NULL);
        caption[i] = svLabelNewFromSM(self->appGlobals->res, "Dialog.InputField.Caption");
        svLabelSetText(caption[i], pinEntryTitle[i]);
        if (caption[i]->width > longestInput)
            longestInput = caption[i]->width;

        input[i] = QBPINInputNew(self->appGlobals->res, "Dialog.InputField.PINInput", params.password.maxLength, &callbacks_qbinput, self, NULL);
    }

    int captionYOffset = svSettingsGetInteger("Dialog.InputField.Caption", "yOffset", -1);
    int pinYOffset = svSettingsGetInteger("Dialog.InputField.PINInput", "yOffset", -1);
    int pinXOffset = -1;
    int spacing = svSettingsGetInteger("Dialog.InputField", "spacing", 0);

    int firstFieldIdx = 0;
    if (!field[firstFieldIdx])
        ++firstFieldIdx;
    if (longestInput < field[firstFieldIdx]->width / 2) {
        pinXOffset = field[firstFieldIdx]->width / 2 + spacing / 2;
        for (int i = 0; i < 3; i++) {
            if (field[i]) {
                svWidgetAttach(field[i], caption[i], field[firstFieldIdx]->width / 2 - caption[i]->width - spacing / 2, captionYOffset, 1);
                svWidgetAttach(field[i], input[i], pinXOffset, pinYOffset, 1);
            }
        }
    } else {
        pinXOffset = longestInput + spacing;
        for (int i = 0; i < 3; i++) {
            if (field[i]) {
                svWidgetAttach(field[i], caption[i], 0, captionYOffset, 1);
                svWidgetAttach(field[i], input[i], pinXOffset, pinYOffset, 1);
            }
        }
    }

    if (self->oldPinRequired)
        QBDialogAddWidget(self->dialog, SVSTRING("content"), field[0], SVSTRING("PIN-field"), 3, NULL);
    QBDialogAddWidget(self->dialog, SVSTRING("content"), field[1], SVSTRING("NewPIN1-field"), 3, NULL);
    QBDialogAddWidget(self->dialog, SVSTRING("content"), field[2], SVSTRING("NewPIN2-field"), 3, NULL);

    if (self->oldPinRequired)
        self->PINEntry = input[0];
    self->NewPIN1Entry = input[1];
    self->NewPIN2Entry = input[2];

    svSettingsPopComponent();

    okButton = QBDialogAddButton(self->dialog, SVSTRING("OK-button"), "OK", 1);
    QBDialogAddButton(self->dialog, SVSTRING("cancel-button"), gettext("Cancel"), 2);

    self->OKButton = okButton;

    if (self->PINEntry)
        svWidgetSetFocus(self->PINEntry);
    else
        svWidgetSetFocus(self->NewPIN1Entry);

    svSettingsPopComponent();
    self->baseMode = true;
}

SvLocal void
QBPinChangeDialogSetToBlockedMode(QBPinChangeDialog self)
{
    if (self->PINEntry) {
        QBDialogRemoveItem(self->dialog, SVSTRING("PIN-field"));
        self->PINEntry = NULL;
    }
    if (self->NewPIN1Entry) {
        QBDialogRemoveItem(self->dialog, SVSTRING("NewPIN1-field"));
        self->NewPIN1Entry = NULL;
    }
    if (self->NewPIN2Entry) {
        QBDialogRemoveItem(self->dialog, SVSTRING("NewPIN2-field"));
        self->NewPIN2Entry = NULL;
    }
    QBDialogRemoveItem(self->dialog, SVSTRING("cancel-button"));

    int minutesLeft = QBAccessManagerGetBlockPeriodLeft(self->appGlobals->accessMgr) / 60;
    if (minutesLeft != self->minutesLeft || !self->infoLabel) {
        self->minutesLeft = minutesLeft;
        SvString infoMsg = SvStringCreateWithFormat(ngettext("PIN authentication blocked for %d minute", "PIN authentication blocked for %d minutes", 1 + minutesLeft), 1 + minutesLeft);
        SvString msg = self->showAdditionalInfo ? SvStringCreateWithFormat("%s %s", self->oldPinInvalid, SvStringCString(infoMsg)) : SVRETAIN(infoMsg);
        self->infoLabel = QBDialogAddLabel(self->dialog, SVSTRING("content"), SvStringCString(msg), SVSTRING("info-label"), 1);
        SVRELEASE(infoMsg);
        SVRELEASE(msg);
    }
    self->baseMode = false;
}

SvLocal void QBPinChangeDialogFailed(QBPinChangeDialog self, QBPasswordStatus status)
{
    if (self->infoLabel)
        QBDialogRemoveItem(self->dialog, SVSTRING("info-label"));

    const char *errorMessage = NULL;

    if (status == QBPasswordStatus_invalid)
        errorMessage = self->oldPinInvalid;
    else
        errorMessage = self->pinChangeFailed;

    int attemptsLeft = QBAccessManagerGetPinAttemptsLeft(self->appGlobals->accessMgr, self->domain);
    if (attemptsLeft == 0) {
        QBPinChangeDialogSetToBlockedMode(self);
    } else {
        SvString msg = NULL;
        if (attemptsLeft > 0) {
            SvString attemptsLeftMsg = SvStringCreateWithFormat(ngettext("%d attempt left.", "%d attempts left", attemptsLeft), attemptsLeft);
            msg = SvStringCreateWithFormat("%s %s", gettext(errorMessage), SvStringCString(attemptsLeftMsg));
            SVRELEASE(attemptsLeftMsg);
        } else {
            msg = SvStringCreate(gettext(errorMessage), NULL);
        }
        self->infoLabel = QBDialogAddLabel(self->dialog, SVSTRING("content"),
                                           SvStringCString(msg), SVSTRING("info-label"), 1);
        SVRELEASE(msg);
    }

    if (attemptsLeft >= 0)
        self->downcounterTimer = svAppTimerStart(self->helper->app, self->helper, 1, false);

    if (self->waitLabel) {
       QBDialogRemoveItem(self->dialog, SVSTRING("wait-label"));
       self->waitLabel = NULL;
    }
    if (self->PINEntry) {
        QBPINInputClear(self->PINEntry);
        svWidgetSetFocusable(self->PINEntry, true);
        svWidgetSetFocus(self->PINEntry);
    }
    if (self->NewPIN1Entry) {
        QBPINInputClear(self->NewPIN1Entry);
        svWidgetSetFocusable(self->NewPIN1Entry, true);
    }
    if (self->NewPIN2Entry) {
        QBPINInputClear(self->NewPIN2Entry);
        svWidgetSetFocusable(self->NewPIN2Entry, true);
    }
    if (self->OKButton) {
        svWidgetSetFocusable(self->OKButton, true);
        if (!self->PINEntry)
            svWidgetSetFocus(self->OKButton);
    }
    QBDialogSetExitValue(self->dialog, NULL, 0);

}

SvLocal void QBPinChangeDialogChangePin(QBPinChangeDialog self)
{
   QBPasswordStatus status;
   SvString passwd, newPasswd;

   if (!self->NewPIN1Entry || !self->NewPIN2Entry) {
      QBDialogSetExitValue(self->dialog, SVSTRING("cancel-button"), 0);
      SvWidget dialog = self->dialog;
      self->dialog = NULL;
      QBDialogBreak(dialog);
      return;
   }

   const char *PIN1 = QBPINInputGetValue(self->NewPIN1Entry);
   const char *PIN2 = QBPINInputGetValue(self->NewPIN2Entry);

   const char* incorrectField = NULL;
   if (self->PINEntry && strlen(QBPINInputGetValue(self->PINEntry)) != 4) {
       incorrectField = self->oldPinFieldIncorrect;
       svWidgetSetFocus(self->PINEntry);
   } else if (strlen(PIN1) != 4) {
       incorrectField = self->newPinField1Incorrect;
       svWidgetSetFocus(self->NewPIN1Entry);
   } else if (strlen(PIN2) != 4) {
       incorrectField = self->newPinField2Incorrect;
       svWidgetSetFocus(self->NewPIN2Entry);
   } else if (strcmp(PIN1, PIN2) != 0) {
       incorrectField = self->pinsDontMatch;
       svWidgetSetFocus(self->PINEntry);
       QBPINInputClear(self->NewPIN1Entry);
       QBPINInputClear(self->NewPIN2Entry);
       QBPINInputClear(self->PINEntry);
   }
   if (incorrectField) {
       self->infoLabel = QBDialogAddLabel(self->dialog, SVSTRING("content"),
               gettext(incorrectField), SVSTRING("info-label"), 1);
       return;
   }

   svWidgetSetFocusable(self->helper, true);
   svWidgetSetFocus(self->helper);
   svWidgetSetFocusable(self->helper, false);
   if (self->PINEntry) svWidgetSetFocusable(self->PINEntry, false);
   if (self->NewPIN1Entry) svWidgetSetFocusable(self->NewPIN1Entry, false);
   if (self->NewPIN2Entry) svWidgetSetFocusable(self->NewPIN2Entry, false);
   svWidgetSetFocusable(self->OKButton, true);

   if ((SvObjectEquals((SvObject) self->domain, (SvObject) SVSTRING("PC")) || SvObjectEquals((SvObject) self->domain, (SvObject) SVSTRING("PC_MENU"))) &&
       self->magic == magicLen ) {
       QBConfigSet("PC_PIN", PIN1);
       QBConfigSave();
       status = QBPasswordStatus_OK;
   } else if (!self->oldPinRequired) {
       newPasswd = SvStringCreate(PIN1, NULL);
       status = QBAccessManagerResetPassword(self->accessMgr, self->domain, newPasswd, NULL);
       SVRELEASE(newPasswd);
   } else {
       passwd = SvStringCreate(QBPINInputGetValue(self->PINEntry), NULL);
       newPasswd = SvStringCreate(PIN1, NULL);
       status = QBAccessManagerChangePassword(self->accessMgr, self->domain, passwd, newPasswd, NULL);
       SVRELEASE(passwd);
       SVRELEASE(newPasswd);
   }

   if (status == QBPasswordStatus_OK) {
       QBPinChangeSuccessful(self);
   } else if (status == QBPasswordStatus_inProgress) {
      // FIXME: add real animation!!!
      if (!self->waitLabel) {
         self->waitLabel = QBDialogAddLabel(self->dialog, SVSTRING("content"),
                                            gettext(self->wait),
                                            SVSTRING("wait-label"), 6);
      }
      if (self->infoLabel) {
         QBDialogRemoveItem(self->dialog, SVSTRING("info-label"));
         self->infoLabel = NULL;
      }
      self->waitTicks = 0;
      self->waitTimer = svAppTimerStart(self->helper->app, self->helper, 0.4, false);
   } else {
      QBPinChangeDialogFailed(self, status);
   }
}

SvLocal bool
QBPinChangeDialogNotification(void * ptr, SvWidget dialog,
                                   SvString buttonTag,
                                   unsigned keyCode)
{
   QBPinChangeDialog self = ptr;

   if (self->dialog) {
      if (buttonTag) {
         if (SvStringEqualToCString(buttonTag, "OK-button")) {
            if(self->changedPin) {
                self->dialog = NULL;
                QBDialogBreak(dialog);
            } else
                QBPinChangeDialogChangePin(self);
         } else if (SvStringEqualToCString(buttonTag, "cancel-button")) {
             self->dialog = NULL;
             QBDialogBreak(dialog);
         }
      } else {
          if(keyCode == QBKEY_BACK) {
              self->dialog = NULL;
              QBDialogBreak(dialog);
          }
          if(!self->changedPin && self->magic < magicLen && magicTab[self->magic] == keyCode) {
              self->magic++;
              if(self->magic == magicLen) {
                  QBPinChangeDialogChangePin(self);
              }
          } else {
              self->magic = 0;
          }

      }
      return true;
   }

   return false;
}

SvLocal void
QBPinChangeDialogTimerEventHandler(SvWidget helper,
                                    SvTimerEvent ev)
{
   QBPinChangeDialog self = helper->prv;

   if (!self->dialog)
      return;

   if (ev->id == self->waitTimer) {
      QBPasswordStatus status = QBAccessManagerCheckPasswordChange(self->accessMgr, self->domain, NULL);
      if (status == QBPasswordStatus_inProgress)
          return;
      if(status == QBPasswordStatus_OK) {
          QBPinChangeSuccessful(self);
      } else if(status == QBPasswordStatus_invalid || status == QBPasswordStatus_failed) {
         QBPinChangeDialogFailed(self, status);
      }
      svAppTimerStop(helper->app, self->waitTimer);
      self->waitTimer = 0;
      return;
   } else if (ev->id == self->downcounterTimer) {
       if (QBAccessManagerGetBlockPeriodLeft(self->appGlobals->accessMgr) > 0 &&
               QBAccessManagerGetPinAttemptsLeft(self->appGlobals->accessMgr, self->domain) == 0) {
           QBPinChangeDialogSetToBlockedMode(self);
      } else if (QBAccessManagerGetBlockPeriodLeft(self->appGlobals->accessMgr) <= 0){//if (QBAccessManagerGetPinAttemptsLeft(self->appGlobals->accessMgr) > 0) {
          svAppTimerStop(self->appGlobals->res, self->downcounterTimer);
          self->downcounterTimer = 0;
          self->showAdditionalInfo = true;
          self->minutesLeft = -1;
          if (self->infoLabel)
              QBDialogRemoveItem(self->dialog, SVSTRING("info-label"));
          QBPinChangeDialogSetToBaseMode(self);
      }
      return;
   }
}

SvWidget QBPinChangeDialogCreate(const AppGlobals appGlobals, const char *widgetName, SvString domain, bool oldPinRequired, SvWidget *helperPtr)
{
    SvWidget dlg, helper ;
    QBPinChangeDialog self = calloc(1, sizeof(struct QBPinChangeDialog_t));

    self->appGlobals = appGlobals;
    self->pinsDontMatch = gettext(svSettingsGetString(widgetName, "pinsDontMatch"));
    self->widgetName = strdup(widgetName);
    self->oldPinInvalid = gettext(svSettingsGetString(widgetName, "invalidPin"));
    self->pinChangeFailed = gettext(svSettingsGetString(widgetName, "pinChangeFailed"));
    self->pinChangeSuccessful = gettext(svSettingsGetString(widgetName, "pinChangeSuccessful"));
    self->oldPinFieldIncorrect = gettext(svSettingsGetString(widgetName, "oldPinFieldIncorrect"));
    self->newPinField1Incorrect = gettext(svSettingsGetString(widgetName, "newPinF1Incorrect"));
    self->newPinField2Incorrect = gettext(svSettingsGetString(widgetName, "newPinF2Incorrect"));
    self->wait = gettext(svSettingsGetString(widgetName, "wait"));
    self->oldPinRequired = oldPinRequired;
    const char* msg = gettext(svSettingsGetString(widgetName, "caption"));

    const char* dialogTitle = gettext(svSettingsGetString(widgetName, "title"));
    if (!dialogTitle || !strcmp(dialogTitle, ""))
        dialogTitle = gettext("PIN change");

    helper = svWidgetCreateBitmap(appGlobals->res, 0, 0, NULL);
    helper->prv = self;
    helper->clean = QBPinChangeDialogClean;
    svWidgetSetTimerEventHandler(helper, QBPinChangeDialogTimerEventHandler);

    self->accessMgr = appGlobals->accessMgr;
    self->helper = helper;
    self->domain = SVRETAIN(domain);

    self->settingsCtx = svSettingsSaveContext();
    svSettingsPushComponent("PINChangeDialog.settings");

    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(helper),
    };
    dlg = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);

    svSettingsPopComponent();

    unsigned int globalKeys[] = {QBKEY_BACK, QBKEY_RED, QBKEY_GREEN, QBKEY_YELLOW, 0};
    QBDialogSetGlobalKeys(dlg, globalKeys);
    QBDialogSetNotificationCallback(dlg, self, QBPinChangeDialogNotification);

    self->dialog = dlg;

    svWidgetAttach(dlg, helper, 0, 0, 0);

    QBDialogSetTitle(dlg, dialogTitle);
    QBPinChangeDialogSetToBaseMode(self);
    self->infoLabel = QBDialogAddLabel(self->dialog, SVSTRING("content"),
                                       msg, SVSTRING("info-label"), 1);
    int secondsLeft = QBAccessManagerGetBlockPeriodLeft(self->appGlobals->accessMgr);
    self->minutesLeft = -1;
    if (secondsLeft > 0 && QBAccessManagerGetPinAttemptsLeft(self->appGlobals->accessMgr, self->domain) == 0) {
        self->showAdditionalInfo = false;
        QBPinChangeDialogSetToBlockedMode(self);
        self->downcounterTimer = svAppTimerStart(self->helper->app, self->helper, 1, false);
    } else {
        self->showAdditionalInfo = true;
    }

    if(helperPtr)
        *helperPtr = helper;

    return dlg;
}

const char* QBPinChangeDialogGetNewPin(SvWidget widget)
{
    QBPinChangeDialog self = widget->prv;
    return self->newPin;
}

