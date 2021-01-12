/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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


#include "QBConaxPopupManager.h"

#include <libintl.h>
#include <player_events/decryption.h>
#include <QBApplicationController.h>
#include <QBSmartcard2Interface.h>
#include <QBICSmartcardUserText.h>
#include <QBICSmartcardOrderInfo.h>
#include <QBICSmartcardAcceptViewing.h>
#include <QBICSmartcardTokenDebit.h>
#include <QBICSmartcardSessionDescription.h>
#include <QBCAS.h>
#include <settings.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/widget.h>
#include <SWL/QBFrame.h>
#include <SWL/button.h>
#include <SWL/events.h>
#include <main.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBTimeDateMonitor.h>
#include <Utils/authenticators.h>
#include <Services/QBAccessController/ConaxAccessPlugin.h>
#include <Services/QBCASManager.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>
#include <TranslationMerger.h>
#include <QBInput/QBInputCodes.h>
#include <QBConaxPullEMMNotificationListener.h>
#include <QBSecureLogManager.h>
#include <QBJSONUtils.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_debug_user_message, 0, "CUMDB", "");

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "ConaxPopupLogLevel", "");
#define log_error(fmt, ...) do { if (1) { SvLogError(COLBEG() "Popup : " fmt COLEND_COL(red), ## __VA_ARGS__); } } while (0)
#define log_info(fmt, ...) do { if (env_log_level() >= 1) { SvLogNotice(COLBEG() "Popup : " fmt COLEND_COL(green), ## __VA_ARGS__); } } while (0)

typedef enum {
    QBConaxPopupType_smartcardNotifications,
    QBConaxPopupType_orderInfo,
    QBConaxPopupType_decryptionNotification,
    QBConaxPopupType_emmPullNotification,
    QBConaxPopupType_userMessages,
    QBConaxPopupType_maturityRating,
    QBConaxPopupType_acceptViewing,
    QBConaxPopupType_tokenDebit,
    QBConaxPopupType_popupNone,
    QBConaxPopupType_versionMismatch,
} QBConaxPopupType;

struct QBConaxPopupManager_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    struct QBSmartcardNotifications_t {
        QBCASSmartcardState state;
        struct {
            bool is_active;
            QBCASSmartcardState state;
        } special_state;
        SvFiber fiber;
        SvFiberTimer timer;
        bool active;
        QBGlobalWindow window;
        SvWidget label;
    } smartcardNotifications;

    struct QBDecryptionNotifications_t {
        bool hasDecryptionError;
        struct player_event_decryption_failed_s decryptionError;
        bool hasNoAccessEvent;
        int noAccessEvent;
        bool hasNoneSupportedCAIDFoundEvent;
        bool active;
        QBGlobalWindow window;
        SvWidget label;
    } decryptionNotifications;

    struct QBEmmPullNotifications_ {
        int noAccessCode;
        bool active;
        QBGlobalWindow window;
        SvWidget label;
    } emmPullNotifications;

    struct QBUserMessages_t {
        SvFiber fiber;
        SvFiberTimer timer;
        SvFiberTimer showTimer;

        QBICSmartcardUserText* userText;

        SvArray queuedUserTexts;

        bool active;
        bool withConfirmation;
        QBGlobalWindow window;
        SvWidget popup;
        SvWidget label;
        SvWidget button;

        SvTime start;
        int buttonXOffset;
        int buttonYOffset;
    } userMessages;

    struct QBOrderInfo_t {
        bool active;
        QBGlobalWindow window;
        SvWidget popup;
        SvWidget label;
        SvString orderPrice;
        SvString orderLabel;
        SvString message;
    } orderInfo;

    struct QBMaturityRating_t {
        bool active;
        bool cancelled;
        SvWidget dialog;
        int current;
        int wanted;
        QBICSmartcardSessionDescription sessionDesc;
    } maturityRating;

    struct QBAcceptViewing_t {
        bool active;
        bool cancelled;
        SvWidget dialog;
        QBICSmartcardAcceptViewing* acceptViewing;
        QBICSmartcardSessionDescription sessionDesc;
    } acceptViewing;

    struct QBTokenDebit_t {
        bool activePINCheck;
        bool activeInfo;
        bool cancelled;
        QBGlobalWindow window;
        SvWidget popup;
        SvWidget label;
        SvWidget dialog;
        QBICSmartcardTokenDebit* tokenDebit;
        QBICSmartcardSessionDescription sessionDesc;
    } tokenDebit;

    struct QBConaxDriverVersion_t {
        bool active;
        QBGlobalWindow window;
        SvWidget popup;
        SvWidget label;
        bool version_mismatch;
        bool conax_version_mismatch; ///< true if driver conax version not supported
        QBICSmartcardOptionConaxVersion current_conax_version; ///< conax version (i.e. 5.0a, 6.0)
    } conaxDriverVersion;

    QBGlobalWindow globalWindow;
    SvWidget dialog;

    QBConaxPopupType shownPopup;

    SvApplication app;
    QBApplicationController ctr;

    bool disabledWindows;
    bool visible;
    bool FTA;
    bool isProperTime;
};

typedef struct QBSmartcardNotifications_t* QBSmartcardNotifications;
typedef struct QBDecryptionNotifications_t* QBDecryptionNotifications;
typedef struct QBEmmPullNotifications_* QBEmmPullNotifications;
typedef struct QBUserMessages_t* QBUserMessages;
typedef struct QBOrderInfo_t* QBOrderInfo;
typedef struct QBMaturityRating_t* QBMaturityRating;
typedef struct QBAcceptViewing_t* QBAcceptViewing;
typedef struct QBTokenDebit_t* QBTokenDebit;
typedef struct QBConaxDriverVersion_t* QBConaxDriverVersion;

SvLocal void QBConaxPopupManagerCancelAllAccessInfo(QBConaxPopupManager self);
SvLocal void QBUserMessagesMarkText(QBConaxPopupManager self, QBICSmartcardMarkUserTextType type);
SvLocal void QBConaxPopupManagerPlaybackEvent(SvGenericObject self, SvString name, void *arg);
SvLocal void QBConaxPopupManagerContentChanged(SvGenericObject self);
SvLocal void QBConaxPopupManagerVisabilityChanged(SvGenericObject self, bool hidden);
SvLocal void QBConaxPopupManagerStart(SvGenericObject self);
SvLocal void QBConaxPopupManagerStop(SvGenericObject self);

SvLocal void QBConaxPopupManagerDumpUserMessagesState(QBConaxPopupManager self)
{
    QBUserMessages um = &self->userMessages;
    log_info("Current user text is %p", um->userText);
    log_info("Active=%i", um->active);
    log_info("fiber events active? : timer=%i, show timer=%i", SvFiberEventIsActive(um->timer), SvFiberEventIsActive(um->showTimer));
    log_info("Time of start %i (nowdiffms=%d)", SvTimeToMs(um->start), SvTimeToMs(SvTimeSub(SvTimeGet(), um->start)));
    if (um->userText) {
        log_info("Internal data structures of current user text");
        log_info("Slot=%i, start_time=%lu, d=%i, i=%i, cnt=%i", um->userText->slot, um->userText->start_time, um->userText->duration, um->userText->interval, um->userText->display_counter);
        log_info("Text=[%s]", SvStringCString(um->userText->text));
    }
    log_info("--------------------------------");
}

SvLocal void QBConaxPopupManagerRemoveWindows(QBConaxPopupManager self, bool keepUserMessageOnScreen)
{
    if (self->shownPopup != QBConaxPopupType_popupNone) {
        if (self->shownPopup == QBConaxPopupType_userMessages) {
            if (keepUserMessageOnScreen) {
                // we should keep current user text on the screen
                return;
            } else {
                QBUserMessagesMarkText(self, QBICSmartcardMarkUserTextType_shown);
            }
        }

        if (self->globalWindow)
            QBApplicationControllerRemoveGlobalWindow(self->ctr, self->globalWindow);
        else if (self->dialog)
            QBDialogBreak(self->dialog);
        self->globalWindow = NULL;
        self->dialog = NULL;
    }
    self->shownPopup = QBConaxPopupType_popupNone;
}

SvLocal void QBConaxPupupManagerClearAllPinDialogs(QBConaxPopupManager self)
{
    self->maturityRating.cancelled = false;
    self->maturityRating.current = -1;
    self->maturityRating.wanted = -1;
    QBICSmartcardClearSessionDecryption(&self->maturityRating.sessionDesc);
    SVTESTRELEASE(self->acceptViewing.acceptViewing);
    self->acceptViewing.acceptViewing = NULL;
    self->acceptViewing.cancelled = false;
    QBICSmartcardClearSessionDecryption(&self->acceptViewing.sessionDesc);
    SVTESTRELEASE(self->tokenDebit.tokenDebit);
    self->tokenDebit.tokenDebit = NULL;
    self->tokenDebit.cancelled = false;
    QBICSmartcardClearSessionDecryption(&self->tokenDebit.sessionDesc);
}

SvLocal void QBConaxPopupManagerReset(QBConaxPopupManager self)
{
    QBConaxPopupManagerRemoveWindows(self,
                                     (self->conaxDriverVersion.current_conax_version == QBICSmartcardOption_conaxVersion__6_0) ||
                                     (self->conaxDriverVersion.current_conax_version == QBICSmartcardOption_conaxVersion__6_0_enhanced_pvr));

    QBConaxPopupManagerCancelAllAccessInfo(self);
    QBConaxPupupManagerClearAllPinDialogs(self);
    self->emmPullNotifications.active = false;
    self->FTA = true;
    self->shownPopup = QBConaxPopupType_popupNone;
    log_info(" %s", __func__);
    QBConaxPopupManagerDumpUserMessagesState(self);
}

/*
 * As for now user messages say that they were shown regardless on whether they were
 * actualy on screen (a more important popup might have covered it).
 * If user message has to be shown 'sincerely', then after each Update which takes
 * user messages from screen it's state should be reseted as in Reset()
 * and user messages step shouldn't try to be shown if any more important popup
 * is on screen.
 */

SvLocal void QBMaturityRatingPINCallback(void *ptr, SvWidget dlg, SvString ret, unsigned key);
SvLocal void QBMaturityRatingCreate(AppGlobals appGlobals, QBConaxPopupManager self);
SvLocal void QBAcceptViewingPINCallback(void *ptr, SvWidget dlg, SvString ret, unsigned key);
SvLocal void QBAcceptViewingCreate(AppGlobals appGlobals, QBConaxPopupManager self);
SvLocal void QBTokenDebitCreatePopup(AppGlobals appGlobals, QBConaxPopupManager self);
SvLocal void QBTokenDebitPINCallback(void *ptr, SvWidget dlg, SvString ret, unsigned key);
SvLocal void QBTokenDebitAsyncLabelCallback(void *ptr, SvWidget label);

SvLocal void QBSmartcardNotificationsWidgetUpdate(QBConaxPopupManager self);
SvLocal char *QBTokenDebitCreateNoTokenString(QBConaxPopupManager self);
SvLocal SvString QBOrderInfoCreateString(QBConaxPopupManager self, SvString label, SvString price);
static void QBUserMessage_update_label(QBUserMessages um, bool force);
SvLocal void QBDecryptionNotificationsWidgetUpdate(QBConaxPopupManager self);
SvLocal char *QBConaxDriverVersionCreateString(QBConaxPopupManager self, SvString currentVersion, SvString supportedVersions, QBICSmartcardOptionConaxVersion currentConaxVersion);

SvLocal void QBConaxPopupManagerPositionItems(SvWidget window, SvWidget popup, SvWidget label, SvWidget button)
{
    int neededHeight = label->off_y + label->height + label->off_y; //offset from top, height, offset from bottom
    if (button) {
        button->off_y = neededHeight;
        neededHeight += button->height * 2; //offset from label, height
    }

    // popup is child od window so we have to change children height and parrent height
    if (window->height != neededHeight) {
        window->height = neededHeight;
        QBFrameSetHeight(popup, neededHeight);
        svWidgetForceRender(window);
    }
}

SvLocal void QBConaxPopupManagerFillWindow(QBConaxPopupManager self, QBGlobalWindow global)
{
    if (global == self->smartcardNotifications.window) {
        QBSmartcardNotificationsWidgetUpdate(self);
    } else if (global == self->tokenDebit.window) {
        QBTokenDebit td = &self->tokenDebit;
        if (td->activeInfo) {
            char *message = QBTokenDebitCreateNoTokenString(self);
            QBAsyncLabelSetCText(td->label, message);
            QBAsyncLabelSetCallback(td->label, QBTokenDebitAsyncLabelCallback, td);
            free(message);
        }
    } else if (global == self->userMessages.window) {
        QBUserMessages um = &self->userMessages;
        QBUserMessage_update_label(um, true);
    } else if (global == self->decryptionNotifications.window) {
        QBDecryptionNotificationsWidgetUpdate(self);
    }
}

SvLocal void QBConaxPopupManagerCancelAllAccessInfo(QBConaxPopupManager self)
{
    self->decryptionNotifications.active = false;
    self->orderInfo.active = false;
    self->maturityRating.active = false;
    self->acceptViewing.active = false;
    self->tokenDebit.activePINCheck = false;
    self->tokenDebit.activeInfo = false;
}

SvLocal void QBConaxPopupManagerUpdate(QBConaxPopupManager self)
{
    if (!self->visible || self->disabledWindows)
        return;

    log_info(" start %s (fta=%d)", __func__, self->FTA);
    QBConaxPopupManagerDumpUserMessagesState(self);

    QBConaxPopupType wantedPopup = QBConaxPopupType_popupNone;
    QBGlobalWindow wantedWindow = NULL;
    SvWidget wantedDialog = NULL;
    QBDialogCallback wantedCallback = NULL;
    /*
     * This statement decides of hierarchy of popups.
     */
    if (self->FTA) {
        if (self->conaxDriverVersion.active) {
            wantedPopup = QBConaxPopupType_versionMismatch;
            wantedWindow = self->conaxDriverVersion.window;
        } else if (self->userMessages.active) {
            wantedPopup = QBConaxPopupType_userMessages;
            wantedWindow = self->userMessages.window;
        }
    } else {
        if (self->conaxDriverVersion.active) {
            wantedPopup = QBConaxPopupType_versionMismatch;
            wantedWindow = self->conaxDriverVersion.window;
        } else if (self->smartcardNotifications.active) {
            wantedPopup = QBConaxPopupType_smartcardNotifications;
            wantedWindow = self->smartcardNotifications.window;
            svWidgetSetHidden(self->smartcardNotifications.window->window, false);
        } else if (self->maturityRating.active) {
            QBMaturityRatingCreate(self->appGlobals, self);
            wantedDialog = self->maturityRating.dialog;
            wantedPopup = QBConaxPopupType_maturityRating;
            wantedCallback = QBMaturityRatingPINCallback;
        } else if (self->acceptViewing.active) {
            QBAcceptViewingCreate(self->appGlobals, self);
            wantedDialog = self->acceptViewing.dialog;
            wantedPopup = QBConaxPopupType_acceptViewing;
            wantedCallback = QBAcceptViewingPINCallback;
        } else if (self->tokenDebit.activePINCheck) {
            QBTokenDebitCreatePopup(self->appGlobals, self);
            wantedDialog = self->tokenDebit.dialog;
            wantedPopup = QBConaxPopupType_tokenDebit;
            wantedCallback = QBTokenDebitPINCallback;
        } else if (self->tokenDebit.activeInfo) {
            wantedPopup = QBConaxPopupType_tokenDebit;
            wantedWindow = self->tokenDebit.window;
            svWidgetSetHidden(self->tokenDebit.window->window, false);
        } else if (self->orderInfo.active) {
            wantedPopup = QBConaxPopupType_orderInfo;
            wantedWindow = self->orderInfo.window;
            svWidgetSetHidden(self->orderInfo.window->window, false);
        } else if (self->userMessages.active) {
            wantedPopup = QBConaxPopupType_userMessages;
            wantedWindow = self->userMessages.window;
        } else if (self->emmPullNotifications.active) {
            wantedPopup = QBConaxPopupType_emmPullNotification;
            wantedWindow = self->emmPullNotifications.window;
        } else if (self->decryptionNotifications.active) {
            wantedPopup = QBConaxPopupType_decryptionNotification;
            wantedWindow = self->decryptionNotifications.window;
        }
    }
    log_info(" current popup=%i, wanted popup=%i, wanted window=%p", self->shownPopup, wantedPopup, wantedWindow);
    log_info("QBConaxPopupManagerUpdate(%d) : self->shownPopup=%d, userMessages : active=%d",
             __LINE__, self->shownPopup, self->userMessages.active);

    if (self->shownPopup == wantedPopup) {
        /*
         * noop
         */
    } else if (wantedPopup == QBConaxPopupType_popupNone) {
        if (self->globalWindow)
            QBApplicationControllerRemoveGlobalWindow(self->ctr, self->globalWindow);
        else if (self->dialog) {
            QBDialogBreak(self->dialog);
        }
        self->globalWindow = NULL;
        self->dialog = NULL;
        self->shownPopup = QBConaxPopupType_popupNone;
    } else {
        if (self->dialog) {
            QBDialogBreak(self->dialog);
        } else if (self->globalWindow) {
            QBApplicationControllerRemoveGlobalWindow(self->ctr, self->globalWindow);
        }
        self->globalWindow = wantedWindow;
        self->dialog = wantedDialog;

        QBConaxPopupManagerFillWindow(self, self->globalWindow);

        if (self->dialog) {
            QBDialogRun(wantedDialog, SVRETAIN(self), wantedCallback);
        } else {
            QBApplicationControllerAddGlobalWindow(self->ctr, self->globalWindow);
        }
        /*
         * If current user message is during showing sequence, but it is prempted now, then treat
         * it as if the user message was shown for full time.
         */
        if (self->shownPopup == QBConaxPopupType_userMessages && self->userMessages.active) {
            QBUserMessagesMarkText(self, QBICSmartcardMarkUserTextType_shown);
        }
        self->shownPopup = wantedPopup;
    }
    log_info(" end %s", __func__);
    QBConaxPopupManagerDumpUserMessagesState(self);
}

SvLocal SvGenericObject QBConaxPopupManagerCreateAuthenticator(AppGlobals appGlobals, const QBICSmartcardSessionDescription *sessionDesc, const char *conaxDomain)
{
    SvString domain = SvStringCreate(conaxDomain, NULL);
    SvValue sessionID = SvValueCreateWithInteger(sessionDesc->sessionId, NULL);

    SvGenericObject authenticator = QBAuthenticateViaAccessManagerWithContext(appGlobals->scheduler, appGlobals->accessMgr, domain, (SvGenericObject) sessionID);
    SVRELEASE(domain);
    SVRELEASE(sessionID);

    return authenticator;
}

/*
 *-------------
 * TOKEN DEBIT
 *-------------
 */

SvLocal void QBTokenDebitPINCallback(void *self_, SvWidget dlg, SvString ret, unsigned key)
{
    QBConaxPopupManager self = self_;
    QBTokenDebit td = &self->tokenDebit;
    //see comments for maturity rating
    self->dialog = NULL;
    td->dialog = NULL;
    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        td->activePINCheck = false;
        QBConaxPopupManagerUpdate(self);
    } else if (key == QBKEY_BACK || (ret && SvStringEqualToCString(ret, "cancel-button"))) {
        // QBAuthDialog is cancelled only if key == QBKEY_BACK or ret == "cancel-button".
        td->cancelled = true;
    }
    SVRELEASE(self);
}

SvLocal char* QBTokenDebitCreateString(QBConaxPopupManager self, const char *str)
{
    QBTokenDebit td = &self->tokenDebit;
    char *message;

    asprintf(&message,
             gettext("Token access\n"
                     "%s\n"
                     "%d token(s)%s\n"
                     "Purse balance: %s %d tokens\n"
                     "%s"),
             td->tokenDebit->label ? SvStringCString(td->tokenDebit->label) : "",
             td->tokenDebit->event_cost, td->tokenDebit->is_ppm ? gettext(" / minute") : "",
             SvStringCString(td->tokenDebit->purse->label), td->tokenDebit->purse->balance,
             str);

    return message;
}

SvLocal char* QBTokenDebitCreateNoTokenString(QBConaxPopupManager self)
{
    return QBTokenDebitCreateString(self, gettext("Not enough tokens - please recharge your card"));
}

SvLocal void QBTokenDebitCreatePopup(AppGlobals appGlobals, QBConaxPopupManager self)
{
    QBTokenDebit td = &self->tokenDebit;
    if (td->dialog)
        return;

    SvGenericObject authenticator = QBConaxPopupManagerCreateAuthenticator(appGlobals, &td->sessionDesc, "ConaxTokenDebit");

    char *message = QBTokenDebitCreateString(self, gettext("Enter CA PIN to accept"));
    svSettingsPushComponent("ConaxPopupManager.settings");
    td->dialog = QBAuthDialogCreate(appGlobals, authenticator, gettext("Authentication required"), message, false, SVSTRING("ConaxPopupManagerPINPopup"), NULL);
    svSettingsPopComponent();
    free(message);
}

SvLocal bool QBTokenDebitInputEventHandler(SvWidget w, SvInputEvent e)
{
    if (e->ch != QBKEY_BACK)
        return false;

    QBConaxPopupManager self = w->prv;
    QBTokenDebit td = &self->tokenDebit;

    if (td->activeInfo) {
        log_info("Hidding token debit info because user pressed exit");
        svWidgetSetHidden(w, true);
        return true;
    }

    return false;
}

SvLocal void QBTokenDebitCreate(AppGlobals appGlobals, QBConaxPopupManager self)
{
    QBTokenDebit td = &self->tokenDebit;

    SvWidget popup = QBFrameCreateFromSM(appGlobals->res, "ConaxPopupManager.SimplePopup");
    SvWidget window = svWidgetCreate(appGlobals->res, popup->width, popup->height);
    svWidgetAttach(window, popup, 0, 0, 0);
    SvWidget label = QBAsyncLabelNew(appGlobals->res, "ConaxPopupManager.SimplePopup.Title", appGlobals->textRenderer);
    svSettingsWidgetAttach(window, label, "ConaxPopupManager.SimplePopup.Title", 1);

    svWidgetSetInputEventHandler(window, QBTokenDebitInputEventHandler);
    window->prv = self;
    svWidgetSetFocusable(window, true);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, window, SVSTRING("ConaxPopupManager"));
    globalWindow->focusable = true;

    td->window = globalWindow;
    td->popup = popup;
    td->label = label;
}

SvLocal void QBTokenDebitDestroy(QBConaxPopupManager self)
{
    QBTokenDebit td = &self->tokenDebit;
    SVTESTRELEASE(td->tokenDebit);
    if (td->dialog)
        QBDialogBreak(td->dialog);

    struct QBGlobalWindow_t* window = td->window;
    window->window->prv = NULL;

    SVRELEASE(td->window);
}

/*
 *----------------
 * ACCEPT VIEWING
 *----------------
 */

SvLocal void QBAcceptViewingPINCallback(void *self_, SvWidget dlg, SvString ret, unsigned key)
{
    QBConaxPopupManager self = self_;
    QBAcceptViewing av = &self->acceptViewing;
    //see comments for maturity rating
    self->dialog = NULL;
    av->dialog = NULL;
    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        av->active = false;
        QBConaxPopupManagerUpdate(self);
    } else if (key == QBKEY_BACK || (ret && SvStringEqualToCString(ret, "cancel-button"))) {
        // QBAuthDialog is cancelled only if key == QBKEY_BACK or ret == "cancel-button".
        av->cancelled = true;
    }
    SVRELEASE(self);
}

SvLocal void QBAcceptViewingCreate(AppGlobals appGlobals, QBConaxPopupManager self)
{
    QBAcceptViewing av = &self->acceptViewing;
    if (av->dialog) {
        return;
    }

    SvGenericObject authenticator = QBConaxPopupManagerCreateAuthenticator(appGlobals, &av->sessionDesc, "ConaxAcceptViewing");

    char *message;
    asprintf(&message, gettext("Start viewing\n%s\nMinutes viewing time left: %i\nEnter CA PIN to start viewing:"),
             SvStringCString(av->acceptViewing->label), av->acceptViewing->minutes_left);
    svSettingsPushComponent("ConaxPopupManager.settings");
    av->dialog = QBAuthDialogCreate(appGlobals, authenticator, gettext("Authentication required"), message, false, SVSTRING("ConaxPopupManagerPINPopup"), NULL);
    svSettingsPopComponent();
    free(message);
}

SvLocal void QBAcceptViewingDestroy(QBConaxPopupManager self)
{
    QBAcceptViewing av = &self->acceptViewing;
    SVTESTRELEASE(av->acceptViewing);
    if (av->dialog)
        QBDialogBreak(av->dialog);
}

/*
 *----------------------
 * MATURITY HIGH RATING
 *----------------------
 */

SvLocal void QBMaturityRatingPINCallback(void *ptr, SvWidget dlg, SvString ret, unsigned key)
{
    QBConaxPopupManager self = ptr;
    QBMaturityRating mr = &self->maturityRating;
    /*
     * The popup will be automaticaly removed, so we need to forget about it.
     */
    self->dialog = NULL;
    mr->dialog = NULL;
    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        mr->active = false;
        /*
         * It is called here because if the if is false then it means that popup manager
         * has called break by itself, so there is no need to update.
         * So, we call update only if user has typed correct pin.
         */
        QBConaxPopupManagerUpdate(self);
    } else if (key == QBKEY_BACK || (ret && SvStringEqualToCString(ret, "cancel-button"))) {
        /*
         * QBAuthDialog is cancelled only if key == QBKEY_BACK or ret == "cancel-button".
         */
        mr->cancelled = true;
    }
    SVRELEASE(self);
}

SvLocal const char* QBMaturityRatingSmartcardMaturityRatingToText(int rating)
{
    switch (rating) {
        case 0x1:
            return "G";
        case 0x2:
            return "PG";
        case 0x4:
            return "A";
        case 0x8:
            return "X";
        case 0x9:
            return "XXX";
    }

    return gettext("unknown");
}

SvLocal void QBMaturityRatingCreate(AppGlobals appGlobals, QBConaxPopupManager self)
{
    QBMaturityRating mr = &self->maturityRating;
    if (mr->dialog)
        return;

    SvGenericObject authenticator = QBConaxPopupManagerCreateAuthenticator(appGlobals, &mr->sessionDesc, "ConaxMaturity");

    char *message;
    asprintf(&message, gettext("Maturity Lock\n\nProgram rating: %s\nCard level: %s\nEnter CA PIN to unlock"),
             QBMaturityRatingSmartcardMaturityRatingToText(mr->wanted),
             QBMaturityRatingSmartcardMaturityRatingToText(mr->current));
    svSettingsPushComponent("ConaxPopupManager.settings");
    mr->dialog = QBAuthDialogCreate(appGlobals, authenticator, gettext("Authentication required"), message, false, SVSTRING("ConaxPopupManagerPINPopup"), NULL);
    svSettingsPopComponent();
    free(message);
}

SvLocal void QBMaturityRatingDestroy(QBConaxPopupManager self)
{
    QBMaturityRating mr = &self->maturityRating;
    if (mr->dialog)
        QBDialogBreak(mr->dialog);
}

/*
 *---------------
 * USER MESSAGES
 *---------------
 */

SvLocal void QBUserMessagesMarkText(QBConaxPopupManager self, QBICSmartcardMarkUserTextType type)
{
    log_info("Marking user text");
    QBUserMessages um = &self->userMessages;
    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), markUserText, um->userText, type);
    QBUserMessage_update_label(um, false); // debug only -deffer
    if (um->userText->display_counter < um->userText->repetition) {
        /*
         * No matter when we mark, next step is at start + interval - duration
         */
        SvTime timeOfNext = SvTimeAdd(um->start, SvTimeConstruct(um->userText->interval, 0));
        SvTime timeUntilNext = SvTimeSub(timeOfNext, SvTimeGet());
        log_info("User text is alive, will be shown again in %i", SvTimeToMs(timeUntilNext) / 1000);
        SvFiberTimerActivateAfter(um->showTimer, timeUntilNext);
    } else {
        SvFiberEventDeactivate(um->showTimer);
        SvArrayRemoveObjectIdenticalTo(um->queuedUserTexts, (SvGenericObject) um->userText);
        SVRELEASE(um->userText);
        um->userText = NULL;
        log_info("Dropping current user message");
    }
    um->active = false;
}

SvLocal void QBUserMessageAsyncLabelCallback(void *ptr, SvWidget label)
{
    log_info(" %s", __func__);
    QBUserMessages um = (QBUserMessages) ptr;
    QBConaxPopupManagerPositionItems(um->window->window, um->popup, label, um->withConfirmation ? um->button : NULL);
}

static void QBUserMessage_update_label(QBUserMessages um, bool force)
{
    bool do_debug = env_debug_user_message(); // change this to debug message repetition count

    if (!force && !do_debug)
        return;
    if (do_debug) {
        // for debugging only: to see which repetition of the text is being displayed
        char buf[1024];
        snprintf(buf, sizeof(buf), "d=%d, i=%d, cnt=%d/%d, [%s]",
                 um->userText->duration, um->userText->interval, um->userText->display_counter + 1, um->userText->repetition,
                 SvStringCString(um->userText->text));
        QBAsyncLabelSetCText(um->label, buf);
    } else {
        QBAsyncLabelSetText(um->label, um->userText->text);
    };
    QBAsyncLabelSetCallback(um->label, QBUserMessageAsyncLabelCallback, um);
}

SvLocal void QBUserMessagesRemoveMessage(QBConaxPopupManager self, int slot)
{
    QBUserMessages um = &self->userMessages;

    SvIterator it = SvArrayIterator(um->queuedUserTexts);
    QBICSmartcardUserText* oldUserText;
    while ((oldUserText = (QBICSmartcardUserText *) SvIteratorGetNext(&it))) {
        if (oldUserText->slot == slot) {
            log_info("Removing user message at slot %i", slot);
            SvArrayRemoveObjectIdenticalTo(um->queuedUserTexts, (SvGenericObject) oldUserText);
            if (um->userText && um->userText->slot == slot) {
                log_info("The removed user message was current, so we will take it from screen");
                SvFiberEventDeactivate(um->showTimer);
                um->active = false;
                SVRELEASE(um->userText);
                um->userText = NULL;
                QBConaxPopupManagerUpdate(self);
            }
            break;
        }
    }
}

SvLocal void QBUserMessagesStep(void *self_)
{
    QBConaxPopupManager self = self_;
    QBUserMessages um = &self->userMessages;

    log_info(" start %s", __func__);
    QBConaxPopupManagerDumpUserMessagesState(self);

    SvFiberDeactivate(um->fiber);
    SvFiberEventDeactivate(um->timer);

    /*
     * This fiber is active if we have an user text that we want to show
     * and as for now there are no obstacles to do that
     */
    if (SvFiberEventIsActive(um->showTimer)) {
        SvFiberEventDeactivate(um->showTimer);
        /*
         * A small state machine, if um->active is true it means that we have
         * a user message that needs to be shown and is being shown on screen.
         * So it means that we need to hide it.
         */
        if (um->active) {
            /*
             * Popup is shown, now what?
             */
            QBUserMessagesMarkText(self, QBICSmartcardMarkUserTextType_shown);
            QBConaxPopupManagerUpdate(self);
        } else {
            /*
             * Popup is hidden, now what?
             */
            if (!self->visible || (self->shownPopup != QBConaxPopupType_popupNone && self->shownPopup != QBConaxPopupType_decryptionNotification)) {
                if (self->conaxDriverVersion.current_conax_version == QBICSmartcardOption_conaxVersion__6_0 ||
                    self->conaxDriverVersion.current_conax_version == QBICSmartcardOption_conaxVersion__6_0_enhanced_pvr) {
                    // we cannot show user text on the screen now
                    // it should be marked as dropped
                    um->start = SvTimeGet();
                    QBUserMessagesMarkText(self, QBICSmartcardMarkUserTextType_dropped);
                } else {
                    /*
                     * This text will be shown later because we have self->userMessages.userText
                     * and this fiber will tick again
                     */
                    log_info("User text is hidden, but cannot show, will show sometime later");
                    self->userMessages.active = false;
                }
            } else if (self->shownPopup == QBConaxPopupType_popupNone) {
                um->active = true;
                um->start = SvTimeGet();
                log_info("User text is being shown for %i", um->userText->duration);
                SvFiberTimerActivateAfter(um->showTimer, SvTimeConstruct(um->userText->duration, 0));
                QBConaxPopupManagerUpdate(self);
            }
        }
    } else if (SvArrayCount(um->queuedUserTexts)) {
        time_t now = SvTimeNow();

        SvIterator it = SvArrayIterator(um->queuedUserTexts);
        QBICSmartcardUserText* userText = um->userText;
        log_info("Searching for user text to show");
        log_info("Do i have currently an user text? %p", userText);
        if (userText) {
            log_info("Checking if it should be shown, it is now %u, user text was shown at %u, and the interval is %u",
                     SvTimeToMs(SvTimeGet()) / 1000, SvTimeToMs(um->start) / 1000, um->userText->interval);
            if (SvTimeToMs(SvTimeSub(SvTimeGet(), um->start)) / 1000 < um->userText->interval)
                userText = NULL;
        } else {
            while ((userText = (QBICSmartcardUserText *) SvIteratorGetNext(&it))) {
                // have to wait for a proper time while scheduled user text
                if ((self->isProperTime && now >= userText->start_time) || userText->start_time == 0) {
                    break;
                }
            }
        }

        if (userText) {
            log_info("Found user text to show, it is now %lu and it has start time at %lu", now, userText->start_time);
            if (um->userText != userText)
                um->userText = SVRETAIN(userText);

            um->withConfirmation = userText->user_ack_allowed;
            if (um->withConfirmation) {
                if (!svWidgetIsAttached(um->button))
                    svWidgetAttach(um->window->window, um->button, um->buttonXOffset, um->buttonYOffset, 2);
                svWidgetSetFocus(um->button);
                um->window->focusable = true;
            } else {
                svWidgetDetach(um->button);
                um->window->focusable = false;
            }

            um->start = SvTimeGet();
            QBUserMessage_update_label(um, true);

            if (!self->visible || (self->shownPopup != QBConaxPopupType_popupNone && self->shownPopup != QBConaxPopupType_decryptionNotification)) {
                if (self->conaxDriverVersion.current_conax_version == QBICSmartcardOption_conaxVersion__6_0 ||
                    self->conaxDriverVersion.current_conax_version == QBICSmartcardOption_conaxVersion__6_0_enhanced_pvr) {
                    // we cannot show user text on the screen now
                    // it should be marked as dropped
                    QBUserMessagesMarkText(self, QBICSmartcardMarkUserTextType_dropped);
                } else {
                    // wait while we will be visible again
                }
                um->active = false;
            } else {
                // will be shown now
                SvFiberTimerActivateAfter(um->showTimer, SvTimeConstruct(um->userText->duration, 0));
                um->active = true;
                QBConaxPopupManagerUpdate(self);
            }
        }
    }

    if (SvArrayCount(um->queuedUserTexts))
        SvFiberTimerActivateAfter(um->timer, SvTimeConstruct(1, 0));
    log_info(" end %s", __func__);
    QBConaxPopupManagerDumpUserMessagesState(self);
}

SvLocal void QBUserMessagesClean(SvApplication app, void *self_)
{
}

SvLocal void QBUserMessagesUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e)
{
    if (e->code != SV_EVENT_BUTTON_PUSHED)
        return;

    QBConaxPopupManager self = w->prv;
    QBUserMessages um = &self->userMessages;

    log_info("Marking user text because user pressed OK");
    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), markUserText, um->userText, QBICSmartcardMarkUserTextType_shown);
    SvArrayRemoveObjectIdenticalTo(um->queuedUserTexts, (SvGenericObject) um->userText);
    SVRELEASE(um->userText);
    um->userText = NULL;
    SvFiberEventDeactivate(um->showTimer);
    log_info("Dropping current user message");
    um->active = false;

    QBConaxPopupManagerUpdate(self);
}

SvLocal void QBUserMessagesCreate(AppGlobals appGlobals, QBConaxPopupManager self)
{
    QBUserMessages um = &self->userMessages;
    um->fiber = SvFiberCreate(appGlobals->scheduler, NULL, "UserMessagess", QBUserMessagesStep, self);
    um->timer = SvFiberTimerCreate(um->fiber);
    um->showTimer = SvFiberTimerCreate(um->fiber);
    um->queuedUserTexts = SvArrayCreate(NULL);

    SvWidget popup = QBFrameCreateFromSM(appGlobals->res, "ConaxPopupManager.SimplePopup");
    SvWidget window = svWidgetCreate(appGlobals->res, popup->width, popup->height);
    svWidgetAttach(window, popup, 0, 0, 0);
    svWidgetSetName(window, "ConaxPopupManager");
    um->popup = popup;

    window->prv = self;
    window->clean = QBUserMessagesClean;
    svWidgetSetUserEventHandler(window, QBUserMessagesUserEventHandler);
    SvWidget label = QBAsyncLabelNew(appGlobals->res, "ConaxPopupManager.SimplePopup.Title", appGlobals->textRenderer);
    SvWidget button = svButtonNewFromSM(appGlobals->res, "ConaxPopupManager.SimplePopup.Button", "Dismiss", 1, svWidgetGetId(window));
    svSettingsWidgetAttach(window, label, "ConaxPopupManager.SimplePopup.Title", 1);
    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, window, SVSTRING("ConaxPopupManager"));
    um->window = globalWindow;
    um->label = label;
    um->button = button;
    um->buttonXOffset = svSettingsGetInteger("ConaxPopupManager.SimplePopup.Button", "xOffset", 0);
    um->buttonYOffset = svSettingsGetInteger("ConaxPopupManager.SimplePopup.Button", "yOffset", 0);
}

SvLocal void QBUserMessagesStop(QBConaxPopupManager self)
{
    QBUserMessages um = &self->userMessages;
    SvFiberDeactivate(um->fiber);
    SvFiberEventDeactivate(um->timer);
    SvFiberEventDeactivate(um->showTimer);
}

SvLocal void QBUserMessagesDestroy(QBConaxPopupManager self)
{
    QBUserMessages um = &self->userMessages;
    SvFiberDestroy(um->fiber);
    svWidgetDestroy(um->button);
    SVRELEASE(um->queuedUserTexts);
    SVRELEASE(um->window);
    SVTESTRELEASE(um->userText);
}

/*
 *--------------------------
 * DECRYPTION NOTIFICATIONS
 *--------------------------
 */

SvLocal char* QBDecryptionNotificationsErrorCodeToString(int error_code)
{
    //Hooray
    static const char* noAccessEvents[] = {
        NULL,
        gettext_noop("No access (1)"),
        gettext_noop("No access (2)"),
        gettext_noop("No access (3)"),
        gettext_noop("No access (4)"),
        gettext_noop("No access (5)"),
        gettext_noop("No access (6)"),
        gettext_noop("No access (7)"),
        gettext_noop("No access (8)"),
        gettext_noop("No access (9)"),
        gettext_noop("No access (10)"),
        gettext_noop("No access (11)"),
        gettext_noop("No access (12)"),
    };


    char *caption = NULL;
    if (error_code < 0 || error_code >= (int) (sizeof(noAccessEvents) / sizeof(noAccessEvents[0])) || !noAccessEvents[error_code]) {
        asprintf(&caption, gettext("No access (%d)"), error_code);
    } else {
        caption = strdup(gettext(noAccessEvents[error_code]));
    }

    return caption;
}

SvLocal void QBDecryptionNotificationsWidgetUpdate(QBConaxPopupManager self)
{
    QBDecryptionNotifications dn = &self->decryptionNotifications;
    char *caption = NULL;

    if (dn->hasNoneSupportedCAIDFoundEvent) {
        svSettingsPushComponent("ConaxPopupManager.settings");
        caption = strdup(gettext(svSettingsGetString("NoneSupportedCAIDFoundPopup", "text")));
        svSettingsPopComponent();
    } else if (dn->hasDecryptionError) {
        if (dn->decryptionError.error_code == 0xFF)
            caption = strdup(gettext("No access"));
        else {
            caption = QBDecryptionNotificationsErrorCodeToString(dn->decryptionError.error_code);
        }
    } else if (dn->hasNoAccessEvent) {
        if (dn->noAccessEvent >= 0)
            caption = QBDecryptionNotificationsErrorCodeToString(dn->noAccessEvent);
        else {
            switch ((QBICSmartcardNoAccessEvent) dn->noAccessEvent) {
                case QBICSmartcardNoAccessEvent_geographical_blackout:
                    caption = strdup(gettext("No access - geographical blackout"));
                    break;
                case QBICSmartcardNoAccessEvent_no_access_to_network:
                    caption = strdup(gettext("No access - network block"));
                    break;
            }
        }
    }

    if (caption) {
        QBAsyncLabelSetCText(dn->label, caption);
        char* escapedMsg = QBStringCreateJSONEscapedString(caption);
        QBSecureLogEvent("QBConaxPopupManager", "ErrorShown.ConaxDecryption.NoAccess", "JSON:{\"description\":\"%s\"}", escapedMsg);
        free(escapedMsg);
        free(caption);
    }
}

SvLocal void QBDecryptionNotificationsUpdate(QBConaxPopupManager self)
{
    QBDecryptionNotifications dn = &self->decryptionNotifications;

    QBConaxPopupManagerCancelAllAccessInfo(self);

    dn->active = dn->hasDecryptionError || dn->hasNoAccessEvent || dn->hasNoneSupportedCAIDFoundEvent;

    QBDecryptionNotificationsWidgetUpdate(self);

    QBConaxPopupManagerUpdate(self);
}

SvLocal void QBDecryptionNotificationsCreate(AppGlobals appGlobals, QBConaxPopupManager self)
{
    QBDecryptionNotifications dn = &self->decryptionNotifications;
    SvWidget window = QBFrameCreateFromSM(appGlobals->res, "ConaxPopupManager.SimplePopup");
    dn->label = QBAsyncLabelNew(appGlobals->res, "ConaxPopupManager.SimplePopup.Title", appGlobals->textRenderer);
    svSettingsWidgetAttach(window, dn->label, "ConaxPopupManager.SimplePopup.Title", 1);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, window, SVSTRING("ConaxPopupManager"));
    dn->window = globalWindow;
}

SvLocal void QBDecryptionNotificationsDestroy(QBConaxPopupManager self)
{
    SVRELEASE(self->decryptionNotifications.window);
}

/*
 *--------------------------
 * EMM PULL NOTIFICATIONS
 *--------------------------
 */

SvLocal void QBConaxPopupManagerEmmPullNotifyNoAccess(SvGenericObject self_, int code)
{
    QBConaxPopupManager self = (QBConaxPopupManager) self_;
    QBEmmPullNotifications en = &self->emmPullNotifications;
    en->active = true;
    en->noAccessCode = code;
    char *caption = NULL;
    asprintf(&caption, gettext("No access (%d)"), en->noAccessCode);
    QBAsyncLabelSetCText(en->label, caption);
    free(caption);
    QBConaxPopupManagerUpdate(self);
}

SvLocal void QBConaxPopupManagerEmmPullClear(SvGenericObject self_)
{
    QBConaxPopupManager self = (QBConaxPopupManager) self_;
    QBEmmPullNotifications en = &self->emmPullNotifications;
    en->active = false;
    en->noAccessCode = -1;
    QBAsyncLabelSetCText(en->label, "");
    QBConaxPopupManagerUpdate(self);
}

SvLocal void QBEmmPullNotificationsCreate(AppGlobals appGlobals, QBConaxPopupManager self)
{
    QBEmmPullNotifications en = &self->emmPullNotifications;
    SvWidget window = QBFrameCreateFromSM(appGlobals->res, "ConaxPopupManager.SimplePopup");
    en->label = QBAsyncLabelNew(appGlobals->res, "ConaxPopupManager.SimplePopup.Title", appGlobals->textRenderer);
    svSettingsWidgetAttach(window, en->label, "ConaxPopupManager.SimplePopup.Title", 1);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, window, SVSTRING("ConaxPopupManager"));
    en->window = globalWindow;
}

SvLocal void QBEmmPullNotificationsDestroy(QBConaxPopupManager self)
{
    SVRELEASE(self->emmPullNotifications.window);
}

/*
 *--------------------
 * SMARTCARD STATUS
 *--------------------
 */

SvLocal void QBSmartcardNotificationsWidgetUpdate(QBConaxPopupManager self)
{
    QBSmartcardNotifications sn = &self->smartcardNotifications;

    QBCASSmartcardState state = sn->state;
    if (sn->special_state.is_active) {
        state = sn->special_state.state;
    };

    if (state == QBCASSmartcardState_correct) {
        return;
    }

    char *caption = NULL;

    switch (state) {
        case QBCASSmartcardState_removed:
            caption = strdup(gettext("No card inserted"));
            break;
        case QBCASSmartcardState_inserted:
            caption = strdup(gettext("Card inserted"));
            break;
        case QBCASSmartcardState_correct:
            caption = strdup(gettext("Card is correct"));
            break;
        case QBCASSmartcardState_incorrect_ex:
            caption = strdup(gettext("Incorrect Conax card")); /// TODO: card-provider specific message here
            break;
        case QBCASSmartcardState_incorrect:
            caption = strdup(gettext("Incorrect card"));
            break;
        case QBCASSmartcardState_problem:
            caption = strdup(gettext("Card problem - check card"));
            break;
    }

    QBAsyncLabelSetCText(sn->label, caption);
    free(caption);
}


SvLocal void QBSmartcardNotificationsUpdate(QBConaxPopupManager self)
{
    QBSmartcardNotifications sn = &self->smartcardNotifications;

    QBCASSmartcardState state = sn->state;
    if (sn->special_state.is_active) {
        state = sn->special_state.state;
    };

    if (state == QBCASSmartcardState_correct) {
        sn->active = false;
        QBConaxPopupManagerUpdate(self);
        return;
    }


    sn->active = true;
    QBSmartcardNotificationsWidgetUpdate(self);

    QBConaxPopupManagerUpdate(self);
}

SvLocal void QBSmartcardNotificationsStep(void *self_)
{
    QBConaxPopupManager self = self_;
    QBSmartcardNotifications sn = &self->smartcardNotifications;

    log_info("---------- QBSmartcardNotificationsStep : special=%d", sn->special_state.is_active);

    SvFiberDeactivate(sn->fiber);
    SvFiberEventDeactivate(sn->timer);

    /// Was the timer caused by the expiration of "special_state"?
    if (sn->special_state.is_active)
        sn->special_state.is_active = false;

    QBCASSmartcardState state = sn->state;
    if (state == QBCASSmartcardState_inserted ||
        state == QBCASSmartcardState_correct) {
        log_info("---------- QBSmartcardNotificationsStep : state=%d -> disabling", state);
        sn->active = false;
        QBConaxPopupManagerUpdate(self);
        return;
    };

    QBSmartcardNotificationsUpdate(self);
}


SvLocal bool QBSmartcardNotificationsInputEventHandler(SvWidget w, SvInputEvent e)
{
    if (e->ch != QBKEY_BACK)
        return false;

    QBConaxPopupManager self = w->prv;
    QBSmartcardNotifications sn = &self->smartcardNotifications;

    if (sn->state == QBCASSmartcardState_incorrect
        || sn->state == QBCASSmartcardState_problem) {
        log_info("Hidding smartcard notification because user pressed exit");

        svWidgetSetHidden(w, true);

        return true;
    }

    return false;
}

SvLocal void QBSmartcardNotificationsCreate(AppGlobals appGlobals, QBConaxPopupManager self)
{
    QBSmartcardNotifications sn = &self->smartcardNotifications;
    sn->fiber = SvFiberCreate(appGlobals->scheduler, NULL, "SmartcardNotifications", QBSmartcardNotificationsStep, self);
    sn->timer = SvFiberTimerCreate(sn->fiber);

    SvWidget popup = QBFrameCreateFromSM(appGlobals->res, "ConaxPopupManager.SimplePopup");
    SvWidget window = svWidgetCreate(appGlobals->res, popup->width, popup->height);
    svWidgetAttach(window, popup, 0, 0, 0);

    svWidgetSetInputEventHandler(window, QBSmartcardNotificationsInputEventHandler);
    window->prv = self;
    svWidgetSetFocusable(window, true);

    sn->label = QBAsyncLabelNew(appGlobals->res, "ConaxPopupManager.SimplePopup.Title", appGlobals->textRenderer);
    svSettingsWidgetAttach(window, sn->label, "ConaxPopupManager.SimplePopup.Title", 1);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, window, SVSTRING("ConaxPopupManager"));
    globalWindow->focusable = true;
    sn->window = globalWindow;
}

SvLocal void QBSmartcardNotificationsStop(QBConaxPopupManager self)
{
    QBSmartcardNotifications sn = &self->smartcardNotifications;
    SvFiberDeactivate(sn->fiber);
    SvFiberEventDeactivate(sn->timer);
}

SvLocal void QBSmartcardNotificationsDestroy(QBConaxPopupManager self)
{
    QBSmartcardNotifications sn = &self->smartcardNotifications;
    SvFiberDestroy(sn->fiber);
    struct QBGlobalWindow_t* window = (struct QBGlobalWindow_t *) sn->window;
    window->window->prv = NULL;
    SVRELEASE(sn->window);
}

/*
 *------------
 * ORDER INFO
 *------------
 */

SvLocal SvString QBOrderInfoCreateString(QBConaxPopupManager self, SvString label, SvString price)
{
    return SvStringCreateWithFormat(gettext("Ordering\n%s \%s\nPlease contact your operator for further information."), SvStringCString(label), SvStringCString(price));
}

SvLocal bool QBOrderInfoInputEventHandler(SvWidget w, SvInputEvent e)
{
    if (e->ch != QBKEY_BACK)
        return false;

    QBConaxPopupManager self = w->prv;
    QBOrderInfo oi = &self->orderInfo;

    if (oi->active) {
        log_info("Hidding order info because user pressed exit");
        svWidgetSetHidden(w, true);
        return true;
    }

    return false;
}

SvLocal void QBOrderInfoAsyncLabelCallback(void *ptr, SvWidget label)
{
    log_info(" %s", __func__);
    QBOrderInfo oi = (QBOrderInfo) ptr;
    QBConaxPopupManagerPositionItems(oi->window->window, oi->popup, label, NULL);
}

SvLocal void QBOrderInfoCreate(AppGlobals appGlobals, QBConaxPopupManager self)
{
    QBOrderInfo oi = &self->orderInfo;

    SvWidget popup = QBFrameCreateFromSM(appGlobals->res, "ConaxPopupManager.SimplePopup");
    SvWidget window = svWidgetCreate(appGlobals->res, popup->width, popup->height);
    svWidgetAttach(window, popup, 0, 0, 0);
    oi->popup = popup;

    oi->label = QBAsyncLabelNew(appGlobals->res, "ConaxPopupManager.SimplePopup.Title", appGlobals->textRenderer);
    svSettingsWidgetAttach(window, oi->label, "ConaxPopupManager.SimplePopup.Title", 1);

    svWidgetSetInputEventHandler(window, QBOrderInfoInputEventHandler);
    window->prv = self;
    svWidgetSetFocusable(window, true);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, window, SVSTRING("ConaxPopupManager"));
    globalWindow->focusable = true;
    oi->window = globalWindow;
}

SvLocal void QBOrderInfoDestroy(QBConaxPopupManager self)
{
    QBOrderInfo oi = &self->orderInfo;

    struct QBGlobalWindow_t* window = oi->window;
    window->window->prv = NULL;

    SVRELEASE(self->orderInfo.window);
    SVTESTRELEASE(self->orderInfo.orderLabel);
    SVTESTRELEASE(self->orderInfo.orderPrice);
    SVTESTRELEASE(self->orderInfo.message);
}

/*
 *------------
 * DRIVER VERSION
 *------------
 */

SvLocal char *QBConaxDriverVersionCreateString(QBConaxPopupManager self, SvString currentVersion, SvString supportedVersions, QBICSmartcardOptionConaxVersion currentConaxVersion)
{
    char *message = NULL;
    asprintf(&message, "!! Conax Driver Version Mismatch !!\nCurrent API Version: %s; Supported API Version(s): %s; Current Cx Version: %s",
             SvStringCString(currentVersion), SvStringCString(supportedVersions), QBICSmartcardConaxVersionToString(currentConaxVersion));

    return message;
}

SvLocal void QBConaxDriverVersionCreate(AppGlobals appGlobals, QBConaxPopupManager self)
{
    log_info("%s", __func__);
    QBConaxDriverVersion dv = &self->conaxDriverVersion;

    SvWidget popup = QBFrameCreateFromSM(appGlobals->res, "ConaxPopupManager.SimplePopup");
    SvWidget window = svWidgetCreate(appGlobals->res, popup->width, popup->height);
    svWidgetAttach(window, popup, 0, 0, 0);
    dv->popup = popup;

    dv->label = QBAsyncLabelNew(appGlobals->res, "ConaxPopupManager.SimplePopup.Title", appGlobals->textRenderer);
    svSettingsWidgetAttach(window, dv->label, "ConaxPopupManager.SimplePopup.Title", 1);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, window, SVSTRING("ConaxPopupManager"));
    dv->window = globalWindow;
}

SvLocal void QBConaxDriverVersionDestroy(QBConaxPopupManager self)
{
    SVRELEASE(self->conaxDriverVersion.window);
}

/*
 *---------------------
 * CONAX POPUP MANAGER
 *---------------------
 */

SvLocal void QBConaxPopupManagerSmartcardState(void *self_, QBCASSmartcardState state)
{
    QBConaxPopupManager self = self_;
    struct QBSmartcardNotifications_t* sn = &self->smartcardNotifications;
    sn->state = state;

    if (sn->special_state.is_active && state != QBCASSmartcardState_correct) {
        /// Special state is no longer valid.
        sn->special_state.is_active = false;
        SvFiberEventDeactivate(sn->timer);
    };

    if (!sn->special_state.is_active) {
        /// Rules for specific states.
        if (state == QBCASSmartcardState_inserted) {
            SvFiberTimerActivateAfter(sn->timer, SvTimeFromMs(2 * 1000));
        } else {
            SvFiberEventDeactivate(sn->timer);
        };
    };

    if (state == QBCASSmartcardState_removed) {
        // remove all popus when card is removed
        QBConaxPopupManagerCancelAllAccessInfo(self);
    }

    QBSmartcardNotificationsUpdate(self);
}

SvLocal void QBConaxPopupManagerSmartcardStateReport(void *self_)
{
    QBConaxPopupManager self = self_;

    /// Special state notifications are not needed at conax 6.0.
    if (self->conaxDriverVersion.current_conax_version == QBICSmartcardOption_conaxVersion__6_0 ||
        self->conaxDriverVersion.current_conax_version == QBICSmartcardOption_conaxVersion__6_0_enhanced_pvr) {
        return;
    }

    struct QBSmartcardNotifications_t* sn = &self->smartcardNotifications;

    /// Special state needs to be shown for 5-10 seconds.
    sn->special_state.state = sn->state;
    sn->special_state.is_active = true;
    SvFiberDeactivate(sn->fiber);
    SvFiberTimerActivateAfter(sn->timer, SvTimeFromMs(8 * 1000));

    QBSmartcardNotificationsUpdate(self);
}

SvLocal void QBConaxPopupManagerNoAccessEvent(void *self_, const QBICSmartcardSessionDescription *sessionDesc, int event)
{
    // we are interested in all messages from our session and from non-session input (sessionId == -1)
    if (sessionDesc->sessionId != -1 && sessionDesc->sessionType != QBCASSessionType_playback) {
        return;
    }

    QBConaxPopupManager self = self_;
    QBDecryptionNotifications dn = &self->decryptionNotifications;
    dn->noAccessEvent = event;
    dn->hasDecryptionError = false;
    dn->hasNoAccessEvent = true;
    QBDecryptionNotificationsUpdate(self);
}

SvLocal void QBConaxPopupManagerUserText(void *self_, const QBICSmartcardSessionDescription *sessionDesc, SvGenericObject userText_)
{
    QBConaxPopupManager self = self_;
    QBUserMessages um = &self->userMessages;

    // we are interested in all user text from our session and from text from non-session input (sessionId == -1)
    if (sessionDesc->sessionId != -1 && sessionDesc->sessionType != QBCASSessionType_playback) {
        return;
    }

    QBICSmartcardUserText* userText = (QBICSmartcardUserText *) userText_;
    if (userText->display_counter >= userText->repetition) {
        log_info("Dropping user text as it has been shown enough times");
        return;
    }

    // interval duration should not be set to '0', some QBConax2 modules return here value '0' in case of no 'interval' parameter from smartcard
    if (userText->interval == 0) {
        userText->interval = userText->duration;
    }

    // We cannot drop any user text if we don't have a valid time yet
    if (self->isProperTime && userText->start_time != 0 && userText->start_time + userText->interval * userText->repetition < SvTimeNow()) {
        log_info("Dropping user text as it should've been shown way in the past");
        return;
    }
    QBUserMessagesRemoveMessage(self, userText->slot);

    SvArrayAddObject(um->queuedUserTexts, userText_);
    SvFiberActivate(um->fiber);

    log_info(" %s", __func__);
    QBConaxPopupManagerDumpUserMessagesState(self);
}

SvLocal void QBConaxPopupManagerOrderInfo(void *self_, const QBICSmartcardSessionDescription *sessionDesc, SvGenericObject orderInfo_)
{
    QBConaxPopupManager self = self_;
    QBICSmartcardOrderInfo* orderInfo = (QBICSmartcardOrderInfo *) orderInfo_;

    // filter out uninterested sessions
    if (sessionDesc->sessionType != QBCASSessionType_playback) {
        return;
    }

    SvString message = QBOrderInfoCreateString(self, orderInfo->label, orderInfo->price);

    QBOrderInfo oi = &self->orderInfo;
    SVTESTRELEASE(oi->orderLabel);
    SVTESTRELEASE(oi->orderPrice);
    oi->orderLabel = SVRETAIN(orderInfo->label);
    oi->orderPrice = SVRETAIN(orderInfo->price);

    QBConaxPopupManagerCancelAllAccessInfo(self);
    if (oi->message && !SvObjectEquals((SvObject) oi->message, (SvObject) message)) {
        // new label occurred, update popup
        QBConaxPopupManagerUpdate(self);
    }

    QBAsyncLabelSetText(oi->label, message);
    QBAsyncLabelSetCallback(oi->label, QBOrderInfoAsyncLabelCallback, oi);
    SVTESTRELEASE(oi->message);
    oi->message = message;
    oi->active = true;
    QBConaxPopupManagerUpdate(self);
}

SvLocal void QBConaxPopupManagerHighRating(void *self_, const QBICSmartcardSessionDescription *sessionDesc, int current, int needed)
{
    QBConaxPopupManager self = self_;

    // filter out uninterested sessions
    if (sessionDesc->sessionType != QBCASSessionType_playback) {
        return;
    }

    QBMaturityRating mr = &self->maturityRating;
    QBConaxPopupManagerCancelAllAccessInfo(self);

    // if message was cancelled we want to show it if new notification was occurred
    if (mr->wanted != needed || mr->current != current ||
        mr->sessionDesc.sessionId != sessionDesc->sessionId ||
        mr->cancelled) {
        mr->cancelled = false;
        // replace old dialog with a new one
        QBConaxPopupManagerUpdate(self);
    }

    mr->active = true;
    mr->wanted = needed;
    mr->current = current;
    mr->sessionDesc = *sessionDesc;
    QBConaxPopupManagerUpdate(self);
}

SvLocal bool QBConaxPopupManagerIsNewAcceptViewing(QBConaxPopupManager self, QBICSmartcardAcceptViewing *newAcceptViewing, const QBICSmartcardSessionDescription *sessionDesc)
{
    if (self->acceptViewing.acceptViewing) {
        QBICSmartcardAcceptViewing *oldAcceptViewing = self->acceptViewing.acceptViewing;

        if (SvObjectEquals((SvObject) oldAcceptViewing->label, (SvObject) newAcceptViewing->label) &&
            oldAcceptViewing->minutes_left == newAcceptViewing->minutes_left &&
            oldAcceptViewing->product_ref == newAcceptViewing->product_ref &&
            self->acceptViewing.sessionDesc.sessionId == sessionDesc->sessionId) {
            // this is the same accept viewing
            return false;
        }
    }

    return true;
}

SvLocal void QBConaxPopupManagerAcceptViewing(void *self_, const QBICSmartcardSessionDescription *sessionDesc, SvGenericObject acceptViewing)
{
    QBConaxPopupManager self = self_;

    // filter out uninterested sessions
    if (sessionDesc->sessionType != QBCASSessionType_playback) {
        return;
    }

    QBAcceptViewing av = &self->acceptViewing;
    QBConaxPopupManagerCancelAllAccessInfo(self);

    bool isNew = QBConaxPopupManagerIsNewAcceptViewing(self, (QBICSmartcardAcceptViewing *) acceptViewing, sessionDesc);

    if (isNew) {
        // replace old dialog
        av->cancelled = false;
        QBConaxPopupManagerUpdate(self);
    } else if (av->cancelled) {
        // dialog was cancelled do not show it again
        QBConaxPopupManagerUpdate(self);
        return;
    }

    av->active = true;
    SVTESTRELEASE(av->acceptViewing);
    av->acceptViewing = SVRETAIN(acceptViewing);
    av->sessionDesc = *sessionDesc;
    QBConaxPopupManagerUpdate(self);
}

SvLocal bool QBConaxPopupManagerIsNewTokenDebit(QBConaxPopupManager self, QBICSmartcardTokenDebit* new, const QBICSmartcardSessionDescription *sessionDesc)
{
    if (self->tokenDebit.tokenDebit) {
        QBICSmartcardTokenDebit *old = self->tokenDebit.tokenDebit;

        if (old->event_cost == new->event_cost &&
            old->event_tag == new->event_tag &&
            old->has_enough_tokens == new->has_enough_tokens &&
            old->is_ppm == new->is_ppm &&
            SvObjectEquals((SvObject) old->label, (SvObject) new->label) &&
            old->purse->balance == new->purse->balance &&
            SvObjectEquals((SvObject) old->purse->label, (SvObject) new->purse->label) &&
            old->purse->purse_ref == new->purse->purse_ref &&
            self->tokenDebit.sessionDesc.sessionId == sessionDesc->sessionId) {
            // this is the same token debit
            return false;
        }
    }

    return true;
}

SvLocal void QBTokenDebitAsyncLabelCallback(void *ptr, SvWidget label)
{
    log_info(" %s", __func__);
    QBTokenDebit td = (QBTokenDebit) ptr;
    QBConaxPopupManagerPositionItems(td->window->window, td->popup, label, NULL);
}

SvLocal void QBConaxPopupManagerTokenDebit(void *self_, const QBICSmartcardSessionDescription *sessionDesc, SvGenericObject tokenDebit_)
{
    QBConaxPopupManager self = self_;

    // filter out uninterested sessions
    if (sessionDesc->sessionType != QBCASSessionType_playback) {
        return;
    }

    QBTokenDebit td = &self->tokenDebit;
    QBICSmartcardTokenDebit* tokenDebit = (QBICSmartcardTokenDebit *) tokenDebit_;

    bool isNew = QBConaxPopupManagerIsNewTokenDebit(self, tokenDebit, sessionDesc);

    SVTESTRELEASE(td->tokenDebit);
    td->tokenDebit = SVRETAIN(tokenDebit);

    QBConaxPopupManagerCancelAllAccessInfo(self);

    if (isNew) {
        // replace old dialog
        td->cancelled = false;
        QBConaxPopupManagerUpdate(self);
    } else if (td->cancelled) {
        // dialog was cancelled do not show it again
        QBConaxPopupManagerUpdate(self);
        return;
    }

    td->sessionDesc = *sessionDesc;

    if (tokenDebit->has_enough_tokens) {
        td->activePINCheck = true;
        td->activeInfo = false;
    } else {
        char *message = QBTokenDebitCreateNoTokenString(self);
        QBAsyncLabelSetCText(td->label, message);
        QBAsyncLabelSetCallback(td->label, QBTokenDebitAsyncLabelCallback, td);
        free(message);
        td->activePINCheck = false;
        td->activeInfo = true;
    }

    QBConaxPopupManagerUpdate(self);
}

SvLocal void QBConaxPopupManagerDriverVersion(QBConaxPopupManager self, QBICSmartcardDriverVersion* driverVersion)
{
    QBConaxDriverVersion conax_driver_version = &self->conaxDriverVersion;
    // we want to show this popup when driver version mismatch occurred
    conax_driver_version->active = driverVersion->versionMismatch || driverVersion->conaxVersionMismatch;
    conax_driver_version->version_mismatch = driverVersion->versionMismatch;
    conax_driver_version->conax_version_mismatch = driverVersion->conaxVersionMismatch;
    conax_driver_version->current_conax_version = driverVersion->currentConaxVersion;

    char *message = QBConaxDriverVersionCreateString(self, driverVersion->currentVersion, driverVersion->supportedVersion, driverVersion->currentConaxVersion);
    QBAsyncLabelSetCText(conax_driver_version->label, message);
    if (conax_driver_version->active) {
        log_error("%s", message);
    } else {
        log_info("Conax driver version OK");
    }
    free(message);

    QBConaxPopupManagerUpdate(self);
}

SvLocal void QBConaxPopupManagerDriverVersionCallback(void *self_, SvGenericObject driverVersion)
{
    QBConaxPopupManagerDriverVersion((QBConaxPopupManager) self_, (QBICSmartcardDriverVersion *) driverVersion);
}

SvLocal void QBConaxPopupManagerMessageRemoved(void *self_, int slot)
{
    QBConaxPopupManager self = self_;
    QBUserMessagesRemoveMessage(self, slot);
}

SvLocal void QBConaxPopupManagerContentChanged(SvGenericObject self_)
{
    QBConaxPopupManager self = (QBConaxPopupManager) self_;
    QBConaxPopupManagerReset(self);
    QBConaxPopupManagerUpdate(self);
}

SvLocal void QBConaxPopupManager__dtor__(void *self_)
{
    QBConaxPopupManager self = self_;
    QBConaxPopupManagerReset(self);
    QBSmartcardNotificationsDestroy(self);
    QBDecryptionNotificationsDestroy(self);
    QBEmmPullNotificationsDestroy(self);
    QBUserMessagesDestroy(self);
    QBOrderInfoDestroy(self);
    QBMaturityRatingDestroy(self);
    QBAcceptViewingDestroy(self);
    QBTokenDebitDestroy(self);
    QBConaxDriverVersionDestroy(self);
}

SvLocal void QBConaxPopupManagerSwitchStarted(SvGenericObject self_, QBWindowContext from, QBWindowContext to)
{
    QBConaxPopupManager self = (QBConaxPopupManager) self_;
    QBConaxPopupManagerRemoveWindows(self, false);
    QBConaxPopupManagerUpdate(self);
    self->visible = false;
}

SvLocal void QBConaxPopupManagerSwitchEnded(SvGenericObject self_, QBWindowContext from, QBWindowContext to)
{
    QBConaxPopupManager self = (QBConaxPopupManager) self_;
    if (QBApplicationControllerCanAddGlobalWindowWithName(self->appGlobals->controller, SVSTRING("ConaxPopupManagerPINPopup")) &&
        QBApplicationControllerCanAddGlobalWindowWithName(self->appGlobals->controller, SVSTRING("ConaxPopupManager")))
        self->visible = true;

    QBConaxPopupManagerUpdate(self);
}

SvLocal void QBConaxPopupManagerTimeUpdated(SvGenericObject self_, bool firstTime, QBTimeDateUpdateSource source)
{
    log_info("TimeUpdated, firstTime: %d, source: %d", (int) firstTime, (int) source);
    QBConaxPopupManager self = (QBConaxPopupManager) self_;
    QBUserMessages um = &self->userMessages;
    if (firstTime) {
        self->isProperTime = true;

        // drop all scheduled user text from the past, there were placed into array while there wasn't a proper time
        SvIterator it = SvArrayReverseIterator(um->queuedUserTexts);
        QBICSmartcardUserText* userText = NULL;
        while ((userText = (QBICSmartcardUserText *) SvIteratorGetNext(&it))) {
            if (userText->start_time != 0) {
                while ((userText->start_time + userText->interval * userText->display_counter < SvTimeNow()) &&
                       (userText->display_counter < userText->repetition)) {
                    log_info("Marking user text as dropped, slot: %d", userText->slot);
                    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), markUserText, userText, QBICSmartcardMarkUserTextType_dropped);
                }

                if (userText->display_counter >= userText->repetition) {
                    log_info("Dropping user text as it should've been shown way in the past, slot: %d", userText->slot);
                    SvArrayRemoveObject(um->queuedUserTexts, (SvGenericObject) userText);
                }
            }
        }

        // activate fiber to check if any user text should be shown
        SvFiberActivate(um->fiber);
    }
}

SvLocal SvType QBConaxPopupManager_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBConaxPopupManager__dtor__
    };
    static SvType type = NULL;
    static const struct QBContextSwitcherListener_t switcherMethods = {
        .started = QBConaxPopupManagerSwitchStarted,
        .ended   = QBConaxPopupManagerSwitchEnded
    };
    static const struct QBTimeDateMonitorListener_ timeDateListenerMethods = {
        .systemTimeSet = QBConaxPopupManagerTimeUpdated
    };
    static const struct QBCASPopupManager_ popupMethods = {
        .playbackEvent      = QBConaxPopupManagerPlaybackEvent,
        .contentChanged     = QBConaxPopupManagerContentChanged,
        .visibilityChanged  = QBConaxPopupManagerVisabilityChanged,
        .start              = QBConaxPopupManagerStart,
        .stop               = QBConaxPopupManagerStop
    };

    static const struct QBConaxPullEMMNotificationListener_ emmPullNotificationListener = {
        .notifyNoAccess = &QBConaxPopupManagerEmmPullNotifyNoAccess,
        .clear          = &QBConaxPopupManagerEmmPullClear,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBConaxPopupManager",
                            sizeof(struct QBConaxPopupManager_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextSwitcherListener_getInterface(), &switcherMethods,
                            QBTimeDateMonitorListener_getInterface(), &timeDateListenerMethods,
                            QBCASPopupManager_getInterface(), &popupMethods,
                            QBConaxPullEMMNotificationListener_getInterface(), &emmPullNotificationListener,
                            NULL);
    }

    return type;
}

SvLocal void QBConaxPopupManagerPlaybackEvent(SvGenericObject self_, SvString name, void *arg)
{
    QBConaxPopupManager self = (QBConaxPopupManager) self_;
    QBDecryptionNotifications dn = &self->decryptionNotifications;
    if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_FAILED)) {
        struct player_event_decryption_failed_s* error = arg;
        if (error->external_reason)
            return;
        dn->decryptionError = *((struct player_event_decryption_failed_s *) arg);
        dn->hasDecryptionError = true;
        dn->hasNoAccessEvent = false;
        dn->hasNoneSupportedCAIDFoundEvent = false;
        QBDecryptionNotificationsUpdate(self);
    } else if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_OK)) {
        dn->hasDecryptionError = false;
        dn->hasNoAccessEvent = false;
        dn->hasNoneSupportedCAIDFoundEvent = false;
        self->decryptionNotifications.active = false;
        self->maturityRating.active = false;
        QBConaxPopupManagerUpdate(self);
    } else if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_ON)) {
        self->FTA = false;
        QBConaxPopupManagerUpdate(self);
    } else if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_OFF)) {
        self->FTA = true;
        dn->hasDecryptionError = false;
        dn->hasNoAccessEvent = false;
        dn->hasNoneSupportedCAIDFoundEvent = false;
        QBConaxPopupManagerCancelAllAccessInfo(self);
        QBConaxPopupManagerUpdate(self);
    } else if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_DESCRAMBLING_FAILED)) {
        // dont update this state if NONE_SUPPORTED_CAID_FOUND occurred or NO ACCESS error is displayed
        if (dn->active && (dn->hasNoneSupportedCAIDFoundEvent || dn->hasDecryptionError)) {
            return;
        }
        dn->decryptionError.error_code = 8;
        dn->decryptionError.external_reason = false;
        dn->hasDecryptionError = true;
        dn->hasNoAccessEvent = false;
        dn->hasNoneSupportedCAIDFoundEvent = false;
        QBDecryptionNotificationsUpdate(self);
    } else if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_DESCRAMBLING_OK)) {
        // dont update this state if NONE_SUPPORTED_CAID_FOUND occurred
        if (dn->active && dn->hasNoneSupportedCAIDFoundEvent) {
            return;
        }
        dn->hasDecryptionError = false;
        dn->hasNoAccessEvent = false;
        dn->hasNoneSupportedCAIDFoundEvent = false;
        self->decryptionNotifications.active = false;
        QBConaxPopupManagerUpdate(self);
    } else if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_NONE_SUPPORTED_CAID_FOUND)) {
        dn->hasDecryptionError = false;
        dn->hasNoAccessEvent = false;
        dn->hasNoneSupportedCAIDFoundEvent = true;
        QBDecryptionNotificationsUpdate(self);
    }
}

static struct QBCASCallbacks_s cas_callbacks = {
    .smartcard_state = QBConaxPopupManagerSmartcardState,
};

static struct QBICSmartcardCallbacks_s conax_callbacks = {
    .smartcard_state_report = QBConaxPopupManagerSmartcardStateReport,
    .no_access_event        = QBConaxPopupManagerNoAccessEvent,
    .user_text              = QBConaxPopupManagerUserText,
    .order_info             = QBConaxPopupManagerOrderInfo,
    .high_rating            = QBConaxPopupManagerHighRating,
    .accept_viewing         = QBConaxPopupManagerAcceptViewing,
    .token_debit            = QBConaxPopupManagerTokenDebit,
    .message_removed        = QBConaxPopupManagerMessageRemoved,
    .driver_version         = QBConaxPopupManagerDriverVersionCallback,
};

QBConaxPopupManager QBConaxPopupManagerCreate(AppGlobals appGlobals)
{
    if (!SvObjectIsImplementationOf(QBCASGetInstance(), QBSmartcard2Interface_getInterface())) {
        log_error("Conax Popup Manager should be only created for Conax CAS type");
        return NULL;
    }

    QBConaxPopupManager self = (QBConaxPopupManager) SvTypeAllocateInstance(QBConaxPopupManager_getType(), NULL);

    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, &cas_callbacks, self, "ConaxPopupManager");
    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), addConaxCallbacks, &conax_callbacks, self, "ConaxPopupManager");
    QBApplicationControllerAddListener(appGlobals->controller, (SvGenericObject) self);

    self->app = appGlobals->res;
    self->ctr = appGlobals->controller;
    self->shownPopup = QBConaxPopupType_popupNone;
    self->visible = false;
    self->appGlobals = appGlobals;
    self->FTA = true;

    svSettingsPushComponent("ConaxPopupManager.settings");
    QBSmartcardNotificationsCreate(appGlobals, self);
    QBDecryptionNotificationsCreate(appGlobals, self);
    QBEmmPullNotificationsCreate(appGlobals, self);
    QBUserMessagesCreate(appGlobals, self);
    QBOrderInfoCreate(appGlobals, self);
    QBTokenDebitCreate(appGlobals, self);
    QBConaxDriverVersionCreate(appGlobals, self);
    svSettingsPopComponent();

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorAddListener(timeDateMonitor, (SvObject) self, NULL);

    QBICSmartcardDriverVersion* driver_version = SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getDriverVersion);
    if (driver_version) {
        QBConaxPopupManagerDriverVersion(self, driver_version);
        SVRELEASE(driver_version);
    }

    QBCASSmartcardState state = SvInvokeInterface(QBCAS, QBCASGetInstance(), getState);
    QBConaxPopupManagerSmartcardState(self, state);

    return self;
}

SvLocal void QBConaxPopupManagerStart(SvGenericObject self_)
{
}

SvLocal void QBConaxPopupManagerStop(SvGenericObject self_)
{
    QBConaxPopupManager self = (QBConaxPopupManager) self_;
    QBSmartcardNotificationsStop(self);
    QBUserMessagesStop(self);
    SvInvokeInterface(QBCAS, QBCASGetInstance(), removeCallbacks, &cas_callbacks, self);
    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), removeConaxCallbacks, &conax_callbacks, self);
}

SvLocal void QBConaxPopupManagerVisabilityChanged(SvGenericObject self_, bool hidden)
{
    QBConaxPopupManager self = (QBConaxPopupManager) self_;
    self->disabledWindows = hidden;

    if (self->disabledWindows) {
        QBConaxPopupManagerRemoveWindows(self, false);
    } else {
        QBConaxPopupManagerUpdate(self);
    }
}

