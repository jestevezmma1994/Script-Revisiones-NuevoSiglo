/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBViewRightPopupManager.h"

#include <libintl.h>
#include <QBViewRight.h>
#include <Logic/QBViewRightDVBLogic.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <player_events/decryption.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/QBFrame.h>
#include <SWL/label.h>
#include <SWL/button.h>
#include <SWL/events.h>
#include <QBApplicationController.h>
#include <CAGE/Text/SvFont.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Core/SvSurface.h>
#include <SvGfxHAL/SvGfxUtils.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Utils/authenticators.h>
#include <Services/QBCASManager.h>
#include <Services/QBViewRightManagers/QBViewRightDVBPVRManager.h>
#include <Services/QBAccessController/ViewRightAccessPlugin.h>
#include <main.h>
#include <QBCAS.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBViewRightPopupManager"
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, moduleName "LogLevel", "");

    #define log_fun(fmt, ...)   do { if (env_log_level() >= 6) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_debug(fmt, ...) do { if (env_log_level() >= 5) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_deep(fmt, ...)  do { if (env_log_level() >= 4) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_info(fmt, ...)  do { if (env_log_level() >= 3) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_state(fmt, ...) do { if (env_log_level() >= 2) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...) do { if (env_log_level() >= 1) SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
 #else
    #define log_fun(fmt, ...)
    #define log_debug(fmt, ...)
    #define log_deep(fmt, ...)
    #define log_info(fmt, ...)
    #define log_state(fmt, ...)
    #define log_error(fmt, ...)
#endif

#define QBVIEWRIGHT_PIN_TIME_TICKET 20

typedef struct QBViewRightOnScreenMessage_s* QBViewRightOnScreenMessage;
typedef struct QBViewRightPinDialog_s* QBViewRightPinDialog;
typedef struct QBViewRightMMINotification_s* QBViewRightMMINotification;

SvLocal void QBViewRightPopupManagerShow(SvGenericObject self_, QBViewRightOSM osm);
SvLocal void QBViewRightPopupManagerShowSmartcardMMINotification(SvGenericObject self_, QBViewRightSmartcardMMINotificationType type);
SvLocal void QBViewRightPopupManagerHandleDescramblingMMINotification(SvGenericObject self_, QBViewRightDescramblingMMINotificationType type);
SvLocal void QBViewRightPopupManagerPinCheck(SvGenericObject self_, int sessionId, QBViewRightCheckPinSource pinSource, QBViewRightPinType pinType, QBViewRightPinTextSelector textSelector);
SvLocal void QBViewRightPopupManagerPinOrder(SvGenericObject self_, QBViewRightCheckPinSource pinSource, QBViewRightPinType pinType, SvString availableCredit, SvString costOfEvent);
SvLocal void QBViewRightPopupManager__dtor__(void *self_);
SvLocal SvWidget QBViewRightPopupManagerCreatePopup(QBViewRightPopupManager self, QBViewRightOSM osm);
SvLocal void QBViewRightPopupManagerSwitchStarted(SvGenericObject self_, QBWindowContext from, QBWindowContext to);
SvLocal void QBViewRightPopupManagerSwitchEnded(SvGenericObject self_, QBWindowContext from, QBWindowContext to);
SvLocal void QBViewRightPopupManagerUpdate(QBViewRightPopupManager self);
SvLocal void QBViewRightPopupManagerRemoveWindows(QBViewRightPopupManager self);
SvLocal void QBViewRightPopupManagerHoldWindows(QBViewRightPopupManager self);
SvLocal void QBViewRightPopupManagerStart(SvGenericObject self_);
SvLocal void QBViewRightPopupManagerStop(SvGenericObject self_);

SvLocal void  QBViewRightOnScreenMessageUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e);
SvLocal void QBViewRightOnScreenMessageCreate(QBViewRightPopupManager self);
SvLocal void QBViewRightOnScreenMessageStop(QBViewRightPopupManager self);
SvLocal void QBViewRightOnScreenMessageClean(SvApplication app, void *self_);
SvLocal void QBViewRightOnScreenMessageDestroy(QBViewRightPopupManager self);
SvLocal void QBViewRightOnScreenMessageShow(QBViewRightPopupManager self);
SvLocal void QBViewRightOnScreenMessageHide(QBViewRightPopupManager self);
SvLocal void QBViewRightOnScreenMessageStep(void *self_);

SvLocal void QBViewRightPopupPinDialogCreate(QBViewRightPopupManager self);
SvLocal void QBViewRightPopupPinDialogInit(QBViewRightPopupManager self, QBViewRightPinType pinType, SvString text);
SvLocal void QBViewRightPopupPinDialogDestroy(QBViewRightPopupManager self);
SvLocal void QBViewRightPopupManagerPINCallback(void *self_, SvWidget dlg, SvString ret, unsigned key);
SvLocal void QBViewRightPopupPinDialogStep(void *self_);
SvLocal void QBViewRightPopupPinDialogHide(QBViewRightPopupManager self, bool force);
SvLocal void QBViewRightPopupPinDialogRemove(QBViewRightPopupManager self);
SvLocal void QBViewRightPopupPinDialogStop(QBViewRightPopupManager self);
SvLocal void QBViewRightPopupPinDialogRefresh(QBViewRightPopupManager self);

SvLocal void QBViewRightPopupManagerPlaybackEvent(SvGenericObject self, SvString name, void *arg);
SvLocal void QBViewRightPopupManagerContentChanged(SvGenericObject self);
SvLocal void QBViewRightPopupManagerVisabilityChanged(SvGenericObject self, bool hidden);

SvLocal void QBViewRightMMINofiticationCreate(QBViewRightPopupManager self);
SvLocal SvWidget QBViewRightMMINotificationBuild(QBViewRightPopupManager self, const char* text, bool isError);
SvLocal void QBViewRightMMINotificationStop(QBViewRightPopupManager self);
SvLocal void QBViewRightMMINotificationDestroy(QBViewRightPopupManager self);
SvLocal void QBViewRightMMINotificationHide(QBViewRightPopupManager self);
SvLocal void QBViewRightMMINotificationResume(QBViewRightPopupManager self);
SvLocal void QBViewRightMMINotificationHold(QBViewRightPopupManager self);


struct QBViewRightOnScreenMessage_s {
    SvFiber fiber;
    SvFiberTimer showTimer;

    SvTime startedTime;
    SvTime totalTimeShown;
    SvTime duration;

    bool isHold;
    QBGlobalWindow window;
    SvWidget label;
    SvWidget button;
    SvWidget currentDialog;

    int buttonXOffset;
    int buttonYOffset;
};

struct QBViewRightMMINotification_s {
    SvFiber fiber;
    SvFiberTimer showTimer;

    QBGlobalWindow window;
    SvWidget label;
    SvWidget currentDialog;
    uint duration;
    bool isScNotficationActive;
};

struct QBViewRightPinDialog_s {
    SvWidget dialog;
    QBViewRightPinType pinType; /**< Pin type related to current PIN dialog */
    SvWidget helper;            /**< PIN dialog widget helper */
    SvFiber fiber;
    SvFiberTimer hideTimer;
};

struct QBViewRightPopupManager_s {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    bool visible;
    bool FTA;
    QBViewRightDescramblingMMINotificationType descramblingState;

    QBViewRightOSM queuedOSM;

    bool disabledWindows;

    struct QBViewRightOnScreenMessage_s onScreenMessage;
    struct QBViewRightPinDialog_s pinDialog;
    struct QBViewRightMMINotification_s notification;
    QBGlobalWindow currentOSMWindow;
    SvWidget currentPinDialog;
};

SvLocal SvType QBViewRightPopupManager_getType(void)
{
    log_fun();
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBViewRightPopupManager__dtor__
    };
    static SvType type = NULL;

    static const struct QBViewRightOSMListener_s methods = {
        .show                            = QBViewRightPopupManagerShow,
        .checkPin                        = QBViewRightPopupManagerPinCheck,
        .orderPin                        = QBViewRightPopupManagerPinOrder,
        .showSmartcardMMINotification    = QBViewRightPopupManagerShowSmartcardMMINotification,
        .showDescramblingMMINotification = QBViewRightPopupManagerHandleDescramblingMMINotification,
    };

    static const struct QBContextSwitcherListener_t switcherMethods = {
        .started = QBViewRightPopupManagerSwitchStarted,
        .ended   = QBViewRightPopupManagerSwitchEnded
    };

    static const struct QBCASPopupManager_ popupMethods = {
        .playbackEvent      = QBViewRightPopupManagerPlaybackEvent,
        .contentChanged     = QBViewRightPopupManagerContentChanged,
        .visibilityChanged  = QBViewRightPopupManagerVisabilityChanged,
        .start              = QBViewRightPopupManagerStart,
        .stop               = QBViewRightPopupManagerStop
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBViewRightPopupManager",
                            sizeof(struct QBViewRightPopupManager_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBViewRightOSMListener_getInterface(), &methods,
                            QBContextSwitcherListener_getInterface(), &switcherMethods,
                            QBCASPopupManager_getInterface(), &popupMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBViewRightPopupManagerSwitchStarted(SvGenericObject self_, QBWindowContext from, QBWindowContext to)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;
    self->visible = false;
    QBViewRightCanShowOSM(QBViewRightGetInstance(), false);
    QBViewRightPopupManagerHoldWindows(self);
}

SvLocal void QBViewRightPopupManagerSwitchEnded(SvGenericObject self_, QBWindowContext from, QBWindowContext to)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;

    if (QBApplicationControllerCanAddGlobalWindowWithName(self->appGlobals->controller, SVSTRING("ViewRightManagerPINPopup")) ||
        QBApplicationControllerCanAddGlobalWindowWithName(self->appGlobals->controller, SVSTRING("ViewRightPopup"))) {
        self->visible = true;
        QBViewRightCanShowOSM(QBViewRightGetInstance(), true);
    } else if (QBApplicationControllerCanAddGlobalWindowWithName(self->appGlobals->controller, SVSTRING("ViewRightTrigger"))) {
        QBViewRightCanShowOSM(QBViewRightGetInstance(), true);
    } else {
        QBViewRightCanShowOSM(QBViewRightGetInstance(), false);
    }
    QBViewRightPopupManagerUpdate(self);
}

SvLocal void QBViewRightOnScreenMessageHold(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightOnScreenMessage um = &self->onScreenMessage;
    if (!um->isHold) {
        QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, self->currentOSMWindow);
        um->totalTimeShown = SvTimeAdd(um->totalTimeShown, SvTimeSub(SvTimeGet(), um->startedTime));
        um->isHold = true;
        SvFiberEventDeactivate(um->showTimer);
    }
}

SvLocal void QBViewRightOnScreenMessageResume(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightOnScreenMessage um = &self->onScreenMessage;
    if (um->isHold) {
        um->isHold = false;
        QBApplicationControllerAddGlobalWindow(self->appGlobals->controller, self->currentOSMWindow);
        um->startedTime = SvTimeGet();
        if (SvTimeCmp(um->duration, SvTimeGetZero())) {
            if (SvTimeCmp(um->duration, um->totalTimeShown) >= 0) {
                SvFiberTimerActivateAfter(um->showTimer, SvTimeSub(um->duration, um->totalTimeShown));
            } else {
                SvFiberActivate(um->fiber);
            }
        }
    }
}

SvLocal void QBViewRightPopupManagerUpdate(QBViewRightPopupManager self)
{
    log_fun();
    if (self->disabledWindows || !self->visible) {
        return;
    }

    if (!self->FTA) {
        if (self->notification.currentDialog) {
            QBViewRightMMINotificationResume(self);
        }
    }

    //Pin Dialog has priority
    if (self->currentPinDialog) {
        QBDialogShow(self->currentPinDialog, false);
        return;
    }
    //Run pin dialog if it is prepared.
    if (!self->currentPinDialog && self->pinDialog.dialog) {
        self->currentPinDialog = self->pinDialog.dialog;
        QBDialogRun(self->currentPinDialog, self,
                    QBViewRightPopupManagerPINCallback);
        return;
    }

    //Resume previous dialog
    if (self->currentOSMWindow) {
        QBViewRightOnScreenMessageResume(self);
        return;
    }

    //Show new dialog
    if (self->onScreenMessage.currentDialog) {
        QBViewRightOnScreenMessageShow(self);
        return;
    }
}

QBViewRightPopupManager QBViewRightPopupManagerCreate(AppGlobals appGlobals)
{
    log_fun();
    SvErrorInfo error = NULL;
    QBViewRightPopupManager self = (QBViewRightPopupManager) SvTypeAllocateInstance(QBViewRightPopupManager_getType(), NULL);

    self->appGlobals = appGlobals;
    self->queuedOSM = NULL;
    self->FTA = true;
    QBViewRightOnScreenMessageCreate(self);
    QBViewRightMMINofiticationCreate(self);
    QBViewRightPopupPinDialogCreate(self);

    if (unlikely(!QBViewRightGetInstance())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "There is no ViewRight manager");
        goto fini;
    }
    QBApplicationControllerAddListener(appGlobals->controller, (SvGenericObject) self);
    QBViewRightOsmListenerAdd(QBViewRightGetInstance(), (SvGenericObject) self);

fini:
    if (unlikely(error)) {
        SVTESTRELEASE(self);
        self = NULL;
    }
    return self;
}
/**
 * Removes all windows
 */
SvLocal void QBViewRightPopupManagerRemoveWindows(QBViewRightPopupManager self)
{
    log_fun();
    if (self->currentPinDialog) {
        QBViewRightPopupPinDialogHide(self, true);
    } else if (self->pinDialog.dialog) {
        QBViewRightPopupPinDialogRemove(self);
    }

    if (self->currentOSMWindow) {
        QBViewRightOnScreenMessageHold(self);
    }

    if (self->notification.currentDialog) {
        QBViewRightMMINotificationHide(self);
    }
}
/**
 * Hold all windows.
 */
SvLocal void QBViewRightPopupManagerHoldWindows(QBViewRightPopupManager self)
{
    log_fun();
    if (self->currentPinDialog) {
        QBDialogHide(self->currentPinDialog, true, false);
    }

    if (self->currentOSMWindow) {
        QBViewRightOnScreenMessageHold(self);
    }
    if (self->notification.currentDialog) {
        QBViewRightMMINotificationHold(self);
    }
}

SvLocal void QBViewRightPopupManagerContentChanged(SvGenericObject self_)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;
    self->FTA = true;
    self->descramblingState = QBViewRightDescramblingMMINotificationType_OK;

    QBViewRightPopupManagerRemoveWindows(self);
}

SvLocal void QBViewRightPopupManagerSmartcardState(void* target, QBCASSmartcardState state)
{
    log_fun();

    QBViewRightPopupManager self = (QBViewRightPopupManager) target;

    QBViewRight viewRight = (QBViewRight) QBCASGetInstance();

    // Parental control PIN for smartcard should be removed when smartcard is removed.
    if (!QBViewRightIsSmartcardEnabled(viewRight)) {
        if (self->pinDialog.dialog && self->pinDialog.pinType != QBViewRightPinType_NonScPin) {
            log_state("Smartcard has been removed. Hide SC Pin popup");
            QBViewRightPopupPinDialogHide(self, true);
        }
    }
}

static const struct QBCASCallbacks_s callbacks = {
    .smartcard_state = QBViewRightPopupManagerSmartcardState,
};

SvLocal void QBViewRightPopupManagerStart(SvGenericObject self_)
{
    log_fun();

    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, &callbacks, self_, "QBViewRightPopupManager");
}

SvLocal void QBViewRightPopupManagerStop(SvGenericObject self_)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;

    SvInvokeInterface(QBCAS, QBCASGetInstance(), removeCallbacks, &callbacks, self);
    QBViewRightPopupPinDialogStop(self);
    QBViewRightOnScreenMessageStop(self);
    QBViewRightMMINotificationStop(self);
}

SvLocal void QBViewRightPopupManagerShow(SvGenericObject self_, QBViewRightOSM osm)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;
    QBViewRightOSMGetMarkupText(osm);

    log_state("Received OnScreenMessage : ");
    log_state("    - duration = %u", QBViewRightOSMGetDuration(osm));
    log_state("    - size (%d, %d)", QBViewRightOSMGetHeight(osm), QBViewRightOSMGetWidth(osm));
    log_state("    - pos (%d, %d)", QBViewRightOSMGetXOffset(osm), QBViewRightOSMGetYOffset(osm));
    log_state("    - textAlign = %d", QBViewRightOSMGetTextAlign(osm));
    log_state("    - anchor = %d", QBViewRightOSMGetAnchor(osm));

    SvString text = QBViewRightOSMGetMarkupText(osm);
    if (text) {
        log_state("    - text = %s", SvStringGetCString(text));
    }

    QBViewRightOnScreenMessage um = &self->onScreenMessage;
    SVTESTRELEASE(self->queuedOSM);
    self->queuedOSM = SVRETAIN(osm);
    SvFiberActivate(um->fiber);
}

SvLocal const char* QBViewRightGetSmartcardMMIMessage(QBViewRightPopupManager self, QBViewRightSmartcardMMINotificationType type)
{
    log_fun();

    const char *smartcardText = "";

    switch (type) {
        case QBViewRightSmartcardMMINotificationType_OK:
            smartcardText = gettext("Smartcard ok");
            break;
        case QBViewRightSmartcardMMINotificationType_SmartcardNotPresent:
            smartcardText = gettext("Smartcard not present");
            break;
        case QBViewRightSmartcardMMINotificationType_HardwareError:
            smartcardText = gettext("Smartcard hardware error");
            break;
        case QBViewRightSmartcardMMINotificationType_SmartcardRejected:
            smartcardText = gettext("Smartcard rejected");
            break;
        case QBViewRightSmartcardMMINotificationType_NonSmartcardMode:
            smartcardText = gettext("Non smartcard mode");
            break;
        case QBViewRightSmartcardMMINotificationType_NonSmartcardModeWithCard:
            smartcardText = gettext("Non smartcard mode with card");
            break;
        case QBViewRightSmartcardMMINotificationType_UpdateRequired:
            smartcardText = gettext("Update required");
            break;
        default:
            smartcardText = gettext("Internal error. Unknown state");
            break;
    }

    return smartcardText;
}

SvLocal void QBViewRightPopupManagerShowSmartcardMMINotification(SvGenericObject self_, QBViewRightSmartcardMMINotificationType type)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;

    const char *stateInfo = QBViewRightGetSmartcardMMIMessage(self, type);

    log_state("Received Smartcard MMI notification");
    log_state("    - type = %d", type);
    if (stateInfo) {
        log_state("    - stateInfo = %s", stateInfo);
    }

    // This notfication should not remove card notifcations
    if (type == QBViewRightSmartcardMMINotificationType_NonSmartcardModeWithCard) {
        return;
    }

    if (self->notification.currentDialog) {
        QBViewRightMMINotificationHide(self);
    }

    SvString notificationStr = QBViewRightDVBLogicCreateMMINotificationString(type, stateInfo);
    if (!notificationStr) {
        return;
    }

    bool isError = false;
    switch (type) {
        case QBViewRightSmartcardMMINotificationType_SmartcardNotPresent:
        case QBViewRightSmartcardMMINotificationType_HardwareError:
        case QBViewRightSmartcardMMINotificationType_SmartcardRejected:
            isError = true;
            break;
        default:
            break;
    }

    QBViewRightMMINotificationBuild(self, SvStringCString(notificationStr), isError);
    SVRELEASE(notificationStr);
    self->notification.isScNotficationActive = true;

    if (self->notification.duration != 0) {
        SvFiberTimerActivateAfter(self->notification.showTimer, SvTimeConstruct(self->notification.duration, 0));
    }
    QBViewRightPopupManagerUpdate(self);
}

SvLocal const char* QBViewGetDescramblingMMIMessage(QBViewRightPopupManager self, QBViewRightDescramblingMMINotificationType type)
{
    log_fun();

    const char *messageText = "";

    switch (type) {
        case QBViewRightDescramblingMMINotificationType_OK:
        case QBViewRightDescramblingMMINotificationType_AuthorisationRequired:
            break;
        case QBViewRightDescramblingMMINotificationType_NoPairing:
            messageText = gettext("No access (No pairing)");
            break;
        case QBViewRightDescramblingMMINotificationType_Error:
        default:
            messageText = gettext("No access");
            break;
    }

    return messageText;
}

SvLocal void QBViewRightPopupManagerUpdateDescramblingMMINotification(QBViewRightPopupManager self, QBViewRightDescramblingMMINotificationType type)
{
    log_fun();

    if (self->notification.currentDialog) {
        QBViewRightMMINotificationHide(self);
    }

    if (type == QBViewRightDescramblingMMINotificationType_OK || type == QBViewRightDescramblingMMINotificationType_AuthorisationRequired) {
        return;
    }

    const char *stateInfo = QBViewGetDescramblingMMIMessage(self, type);
    QBViewRightMMINotificationBuild(self, stateInfo, (type == QBViewRightDescramblingMMINotificationType_Error));
    if (self->notification.duration != 0) {
        SvFiberTimerActivateAfter(self->notification.showTimer, SvTimeConstruct(self->notification.duration, 0));
    }
    QBViewRightPopupManagerUpdate(self);
}

SvLocal void QBViewRightPopupManagerHandleDescramblingMMINotification(SvGenericObject self_, QBViewRightDescramblingMMINotificationType type)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;

    const char *stateInfo = QBViewGetDescramblingMMIMessage(self, type);

    log_state("Received descrambling MMI notification");
    log_state("    - type = %d", type);
    if (stateInfo) {
        log_state("    - stateInfo = %s", stateInfo);
    }

    self->descramblingState = type;

    // Smartcard notfication has priorty
    if (self->notification.isScNotficationActive) {
        return;
    }

    QBViewRightPopupManagerUpdateDescramblingMMINotification(self, self->descramblingState);
}

SvLocal void QBViewRightPopupCalculatePosition(QBViewRightOSM osm, SvWidget w)
{
    log_fun();
    QBViewRightOSMAnchor anchor = QBViewRightOSMGetAnchor(osm);
    int xOff = 0, yOff = 0;
    uint width = w->width;
    uint height = w->height;

    uint xAnchor = QBViewRightOSMGetXOffset(osm);
    uint yAnchor = QBViewRightOSMGetYOffset(osm);
    switch (anchor) {
        case QBViewRightOSMAnchor_LeftTop:
            xOff = xAnchor;
            yOff = yAnchor;
            break;
        case QBViewRightOSMAnchor_LeftCenter:
            xOff = xAnchor;
            yOff = yAnchor - (height >> 1);
            break;
        case QBViewRightOSMAnchor_LeftBottom:
            xOff = xAnchor;
            yOff = yAnchor - height;
            break;
        case QBViewRightOSMAnchor_CenterTop:
            xOff = xAnchor - (width >> 1);
            yOff = yAnchor;
            break;
        case QBViewRightOSMAnchor_Center:
            xOff = xAnchor - (width >> 1);
            yOff = yAnchor - (height >> 1);
            break;
        case QBViewRightOSMAnchor_CenterBottom:
            xOff = xAnchor - (width >> 1);
            yOff = yAnchor - height;
            break;
        case QBViewRightOSMAnchor_RightTop:
            xOff = xAnchor - (width);
            yOff = yAnchor;
            break;
        case QBViewRightOSMAnchor_RightCenter:
            xOff = xAnchor - (width);
            yOff = yAnchor - (height >> 1);
            break;
        case QBViewRightOSMAnchor_RightBottom:
            xOff = xAnchor - (width);
            yOff = yAnchor - (height);
            break;
        default:
            log_error("Invalid anchor value");
            break;
    }
    w->off_x = xOff;
    w->off_y = yOff;
}

SvLocal void QBViewRightPopupMoveToVisibleArea(SvWidget w, int maxX, int maxY)
{
    log_fun();

    if (w->off_x < 0) {
        w->off_x = 0;
    }

    if (w->off_y < 0) {
        w->off_y = 0;
    }

    if (w->off_x + w->width > maxX) {
        w->off_x = maxX - w->width;
    }

    if (w->off_y + w->height > maxY) {
        w->off_y = maxY - w->height;
    }
}

SvLocal SvWidget QBViewRightPopupManagerCreatePopup(QBViewRightPopupManager self, QBViewRightOSM osm)
{
    log_fun();
    QBViewRightOnScreenMessage um = &self->onScreenMessage;
    AppGlobals appGlobals = self->appGlobals;
    int width = 0;
    int height = 0;
    int frameWidth = 0;
    int frameHeight = 0;
    int buttonSpace = 0;

    SvString message = QBViewRightOSMGetMarkupText(osm);
    if (!message) {
        return NULL;
    }

    svSettingsPushComponent("ViewRightPopup.settings");
    struct SvLabelParams labelParams = {
        .app             = appGlobals->res,
        .isMarkup        = true,
        .isMultiline     = true,
        .value           = SvStringCString(message),
        .font            = svSettingsCreateFont("ViewRightOSM.Label", "font"),
        .obliqueFont     = svSettingsCreateFont("ViewRightOSM.Label", "italicFont"),
        .boldFont        = svSettingsCreateFont("ViewRightOSM.Label", "boldFont"),
        .boldObliqueFont = svSettingsCreateFont("ViewRightOSM.Label", "boldItalicFont"),
        .fontSize        = svSettingsGetInteger("ViewRightOSM.Label", "fontSize", 30),
        .fontColor       = svSettingsGetColor("ViewRightOSM.Label", "textColor", 0),
        .maxLines        = svSettingsGetInteger("ViewRightOSM.Label", "maxLines", 10),
    };

    switch (QBViewRightOSMGetTextAlign(osm)) {
        case QBViewRightOSMTextAlign_Left:
            labelParams.align = SvLabelAlignment_LEFT;
            break;
        case QBViewRightOSMTextAlign_Center:
            labelParams.align = SvLabelAlignment_CENTER;
            break;
        case QBViewRightOSMTextAlign_Right:
            labelParams.align = SvLabelAlignment_RIGHT;
            break;
        default:
            labelParams.align = SvLabelAlignment_CENTER;
            break;
    }

    if (QBViewRightOSMUseGivenSize(osm)) {
        labelParams.width = QBViewRightOSMGetWidth(osm);
        labelParams.height = QBViewRightOSMGetHeight(osm);
    } else {
        labelParams.width = svSettingsGetInteger("ViewRightOSM.Label", "width", 0);
        labelParams.height = svSettingsGetInteger("ViewRightOSM.Label", "height", 0);
    }

    if (QBViewRightOSMIsRemovable(osm)) {
        um->button = svButtonNewFromSM(appGlobals->res, "ViewRightOSM.Button", "OK", 1, svWidgetGetId(um->window->window));
    } else {
        um->button = NULL;
    }
    SvBitmap bg = svSettingsGetBitmap("ViewRightOSM", "bg");

    frameWidth = svSettingsGetInteger("ViewRightOSM", "width", 0);
    int minimalWidth = svSettingsGetInteger("ViewRightOSM", "minimalWidth", 0);
    frameHeight = svSettingsGetInteger("ViewRightOSM", "height", 0);
    buttonSpace = svSettingsGetInteger("ViewRightOSM", "buttonSpace", frameHeight);

    svSettingsPopComponent();

    if (!QBViewRightOSMUseGivenSize(osm)) {
        um->label = svLabelAutoSize(&labelParams);
    } else {
        um->label = svLabelNewWithCaption(&labelParams);
    }

    SVTESTRELEASE(labelParams.font);
    SVTESTRELEASE(labelParams.obliqueFont);
    SVTESTRELEASE(labelParams.boldFont);
    SVTESTRELEASE(labelParams.boldObliqueFont);

    if (!(um->label)) {
        return NULL;
    }
    if (!(um->button))
        height = um->label->height;
    else
        height = um->label->height + um->button->height;
    width = um->label->width + frameWidth * 2;
    height += frameHeight * 2 + buttonSpace;

    if (width < minimalWidth) {
        width = minimalWidth;
    }

    QBFrameConstructData params = {
        .width = width,
        .height = height,
        .bitmap = QBViewRightOSMHasBackground(osm) ? bg : NULL,
    };

    um->currentDialog = QBFrameCreate(appGlobals->res, &params);
    QBViewRightPopupCalculatePosition(osm, um->currentDialog);

    svWidgetAttach(um->currentDialog, um->label, (width - um->label->width) / 2, frameHeight, 1);
    if (um->button) {
        svWidgetAttach(um->currentDialog, um->button, (width - um->button->width) / 2, height - um->button->height - frameHeight, 1);
    }

    QBViewRightPopupMoveToVisibleArea(um->currentDialog, um->window->window->width, um->window->window->height);
    svWidgetAttach(um->window->window, um->currentDialog, um->currentDialog->off_x, um->currentDialog->off_y, 1);

    if (um->button) {
        svWidgetSetFocus(um->button);
        um->window->focusable = true;
    }

    um->duration = SvTimeConstruct(QBViewRightOSMGetDuration(self->queuedOSM), 0);

    return um->currentDialog;
}

SvLocal void QBViewRightPopupManager__dtor__(void *self_)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;
    QBViewRightOnScreenMessageDestroy(self);
    QBViewRightMMINotificationDestroy(self);
    QBViewRightPopupPinDialogDestroy(self);
    SVTESTRELEASE(self->queuedOSM);
}

SvLocal void QBViewRightOnScreenMessageUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e)
{
    log_fun();
    if (e->code != SV_EVENT_BUTTON_PUSHED)
        return;

    QBViewRightPopupManager self = w->prv;
    QBViewRightOnScreenMessage um = &self->onScreenMessage;
    if (um->currentDialog) {
        QBViewRightOnScreenMessageHide(self);
    }
    SvFiberEventDeactivate(um->showTimer);
}

SvLocal SvWidget QBViewRightMMINotificationBuild(QBViewRightPopupManager self, const char *text, bool isError)
{
    log_fun();
    QBViewRightMMINotification notification = &self->notification;
    AppGlobals appGlobals = self->appGlobals;

    svSettingsPushComponent("ViewRightPopup.settings");
    SvBitmap bg = svSettingsGetBitmap("ViewRightMMINotification", "bg");

    int width = svSettingsGetInteger("ViewRightMMINotification", "width", 0);
    int height = svSettingsGetInteger("ViewRightMMINotification", "height", 0);
    int off_x = svSettingsGetInteger("ViewRightMMINotification", "xOffset", 0);
    int off_y = svSettingsGetInteger("ViewRightMMINotification", "yOffset", 0);

    uint duration = 0;
    if (isError) {
        duration = svSettingsGetInteger("ViewRightMMINotification", "errorDuration", 10);
    } else {
        duration = svSettingsGetInteger("ViewRightMMINotification", "infoDuration", 10);
    }

    notification->label = svLabelNewFromSM(self->appGlobals->res, "ViewRightMMINotification.Label");
    svLabelSetText(notification->label, text);
    svSettingsPopComponent();

    QBFrameConstructData params = {
        .width = width,
        .height = height,
        .bitmap = bg,
    };
    notification->currentDialog = QBFrameCreate(appGlobals->res, &params);
    notification->duration = duration;

    svWidgetAttach(notification->currentDialog, notification->label, 0, (height - notification->label->height) / 2, 1);
    svWidgetAttach(notification->window->window, notification->currentDialog, off_x, off_y, 1);

    return notification->currentDialog;
}

SvLocal void QBViewRightMMINotificationStep(void *self_)
{
    log_fun();
    QBViewRightPopupManager self = self_;
    QBViewRightMMINotification notyfication = &self->notification;

    SvFiberDeactivate(notyfication->fiber);

    if (SvFiberEventIsActive(notyfication->showTimer)) {
        SvFiberEventDeactivate(notyfication->showTimer);
        QBViewRightPopupManagerUpdateDescramblingMMINotification(self, self->descramblingState);
    }
}

SvLocal void QBViewRightMMINofiticationCreate(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightMMINotification notification = &self->notification;
    AppGlobals appGlobals = self->appGlobals;
    notification->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "ViewRightMMINotification", QBViewRightMMINotificationStep, self);
    notification->showTimer = SvFiberTimerCreate(notification->fiber);

    svSettingsPushComponent("ViewRightPopup.settings");
    SvWidget window = svSettingsWidgetCreate(appGlobals->res, "ViewRightPopupWindow");
    svSettingsPopComponent();
    window->prv = self;
    window->clean = QBViewRightOnScreenMessageClean;

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, window, SVSTRING("ViewRightPopup"));

    notification->window = globalWindow;
}

SvLocal void QBViewRightMMINotificationResume(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightMMINotification notification = &self->notification;
    if (notification->currentDialog)
        QBApplicationControllerAddGlobalWindow(self->appGlobals->controller, notification->window);
}

SvLocal void QBViewRightMMINotificationHold(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightMMINotification notification = &self->notification;
    if (notification->currentDialog)
        QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, notification->window);
}

SvLocal void QBViewRightMMINotificationHide(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightMMINotification notification = &self->notification;
    QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, notification->window);
    SvFiberEventDeactivate(notification->showTimer);
    svWidgetDetach(notification->currentDialog);
    svWidgetDestroy(notification->currentDialog);
    notification->currentDialog = NULL;
    self->notification.isScNotficationActive = false;
}


SvLocal void QBViewRightMMINotificationStop(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightMMINotification notification = &self->notification;
    SvFiberDeactivate(notification->fiber);
    SvFiberEventDeactivate(notification->showTimer);
}

SvLocal void QBViewRightMMINotificationDestroy(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightMMINotification notification = &self->notification;
    SvFiberDestroy(notification->fiber);
    SVRELEASE(notification->window);
}

SvLocal void QBViewRightOnScreenMessageCreate(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightOnScreenMessage um = &self->onScreenMessage;
    AppGlobals appGlobals = self->appGlobals;
    um->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "ViewRightUserMessages", QBViewRightOnScreenMessageStep, self);
    um->showTimer = SvFiberTimerCreate(um->fiber);

    svSettingsPushComponent("ViewRightPopup.settings");
    SvWidget window = svSettingsWidgetCreate(appGlobals->res, "ViewRightPopupWindow");
    svSettingsPopComponent();
    window->prv = self;
    window->clean = QBViewRightOnScreenMessageClean;
    svWidgetSetUserEventHandler(window, QBViewRightOnScreenMessageUserEventHandler);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, window, SVSTRING("ViewRightPopup"));

    um->window = globalWindow;
}

SvLocal void QBViewRightOnScreenMessageShow(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightOnScreenMessage um = &self->onScreenMessage;

    if (!um->currentDialog)
        return;
    um->startedTime = SvTimeGet();
    um->totalTimeShown = SvTimeConstruct(0, 0);

    if (SvTimeCmp(um->duration, SvTimeGetZero())) {
        SvFiberTimerActivateAfter(um->showTimer, um->duration);
    } else {
        // deactivate previously set timer, when duration is set to 0 we should show it until next message occur
        SvFiberEventDeactivate(um->showTimer);
    }
    self->currentOSMWindow = self->onScreenMessage.window;
    QBApplicationControllerAddGlobalWindow(self->appGlobals->controller, self->currentOSMWindow);
}

SvLocal void QBViewRightOnScreenMessageHide(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightOnScreenMessage um = &self->onScreenMessage;
    QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, self->currentOSMWindow);
    svWidgetDetach(um->currentDialog);
    svWidgetDestroy(um->currentDialog);
    um->currentDialog = NULL;
    self->currentOSMWindow = NULL;
}

SvLocal void QBViewRightOnScreenMessageStop(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightOnScreenMessage um = &self->onScreenMessage;
    SvFiberDeactivate(um->fiber);
    SvFiberEventDeactivate(um->showTimer);
}

SvLocal void QBViewRightOnScreenMessageDestroy(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightOnScreenMessage um = &self->onScreenMessage;
    SvFiberDestroy(um->fiber);
    SVRELEASE(um->window);
}

SvLocal void QBViewRightOnScreenMessageStep(void *self_)
{
    log_fun();
    QBViewRightPopupManager self = self_;
    QBViewRightOnScreenMessage um = &self->onScreenMessage;

    SvFiberDeactivate(um->fiber);

    if (SvFiberEventIsActive(um->showTimer)) {
        SvFiberEventDeactivate(um->showTimer);
        if (um->currentDialog) {
            QBViewRightOnScreenMessageHide(self);
        }
    }

    if (self->pinDialog.dialog) {
        return;
    }

    if (self->visible && self->queuedOSM) {
        if (um->currentDialog) {
            QBViewRightOnScreenMessageHide(self);
        }
        QBViewRightPopupManagerCreatePopup(self, self->queuedOSM);
        QBViewRightPopupManagerUpdate(self);

        SVRELEASE(self->queuedOSM);
        self->queuedOSM = NULL;
    }
}

SvLocal void QBViewRightOnScreenMessageClean(SvApplication app, void *self_)
{
    log_fun();
}

SvLocal void  QBViewRightPopupManagerPINCallback(void *self_, SvWidget dlg, SvString ret, unsigned key)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;
    if (self->currentOSMWindow)
        QBViewRightOnScreenMessageResume(self);
    self->pinDialog.dialog = NULL;
    self->currentPinDialog = NULL;
    if (self->queuedOSM) {
        SvFiberActivate(self->onScreenMessage.fiber);
    }
}

SvLocal void QBViewRightPopupManagerPinDialogShow(QBViewRightPopupManager self, QBViewRightPinType pinType, SvString text, bool persistent)
{
    log_fun();
    log_deep("text = '%s'", SvStringCString(text));

    if (self->pinDialog.dialog) {
        if (self->pinDialog.pinType == pinType && !QBAuthDialogIsEnteringPinDisabled(self->pinDialog.helper)) {
            // PIN dialog hasn't changed
            goto fini;
        }
        // PIN dialog has changed - hiding current dialog
        QBViewRightPopupPinDialogHide(self, true);
    }
    QBViewRightPopupPinDialogInit(self, pinType, text);
    if (self->currentOSMWindow)
        QBViewRightOnScreenMessageHold(self);

    QBViewRightPopupManagerUpdate(self);

fini:
    if (!persistent) {
        QBViewRightPopupPinDialogRefresh(self);
    }
}

SvLocal SvString QBViewRightPopupManagerCreateTextFromSelector(QBViewRightPinTextSelector textSelector)
{
    log_fun();
    const char * text = NULL;
    switch (textSelector) {
        case QBViewRightPinTextSelector_ParentalControl:
            text = gettext("Parental control PIN");
            break;
        case QBViewRightPinTextSelector_IPPV:
            text = gettext("IPPV PIN");
            break;
        case QBViewRightPinTextSelector_ParentalControlNonSmartCard:
            text = gettext("Parental control PIN (Non smartcard)");
            break;
        case QBViewRightPinTextSelector_ResumingEvent:
            text = gettext("Resume event pin");
            break;
        case QBViewRightPinTextSelector_SelectEvent:
            text = gettext("Select event");
            break;
        default:
            return NULL;
    }
    return SvStringCreate(text, NULL);
}

SvLocal void QBViewRightPopupManagerPinCheck(SvGenericObject self_, int sessionId, QBViewRightCheckPinSource pinSource, QBViewRightPinType pinType, QBViewRightPinTextSelector textSelector)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;

    SvString text = QBViewRightPopupManagerCreateTextFromSelector(textSelector);
    if (!text) {
        log_error("Cannot create text for textSelector (%d)", textSelector);
        return;
    }

    log_state("Received pin check event : ");
    log_state("    - pinType = %d", pinType);
    if (text) {
        log_state("    - text = %s", SvStringGetCString(text));
    }
    log_state("    - pinSource = %d", pinSource);
    log_state("    - sessionId = %d", sessionId);

    switch (pinSource) {
        case QBViewRightCheckPinSource_livePlayback:
            // In case of live playback ask user for PIN. Popup has to be hidden after crypto period.
            QBViewRightPopupManagerPinDialogShow(self, pinType, text, false);
            break;
        case QBViewRightCheckPinSource_pvrPlayback:
            // In case of pvr playback ask user for PIN. Popup has to stay on screen.
            QBViewRightPopupManagerPinDialogShow(self, pinType, text, true);
            break;
        case QBViewRightCheckPinSource_pvrRecord:
            // Mark that this recording has parental control event. Ongoing recordings with parental control event can not be played.
            QBViewRightDVBPVRManagerMarkParentalControlledEvent((QBViewRightDVBPVRManager) self->appGlobals->casPVRManager, sessionId);
            // In case of pvr record try to use cached PIN for the user.
            if (self->appGlobals->casPVRManager) {
                QBViewRightDVBPVRManagerEnterCachedPin((QBViewRightDVBPVRManager) self->appGlobals->casPVRManager, pinType, textSelector);
            } else {
                log_error("ViewRightDVB pvr manager is not present.");
            }
            break;
        default:
            break;
    }

    SVRELEASE(text);
}

SvLocal void QBViewRightPopupManagerPinOrder(SvGenericObject self_, QBViewRightCheckPinSource pinSource, QBViewRightPinType pinType, SvString availableCredit, SvString costOfEvent)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;

    log_state("Received pin order event: ");
    log_state("    - pinType = %d", pinType);
    log_state("    - availableCredit = %s", SvStringGetCString(availableCredit));
    log_state("    - costOfEvent = %s", SvStringGetCString(costOfEvent));
    log_state("    - pinSource = %d", pinSource);

    if (pinSource != QBViewRightCheckPinSource_livePlayback) {
        log_state("Pin order is used only for live playback. Recording will be dropped.");
        QBViewRight viewRight = (QBViewRight) QBCASGetInstance();
        QBViewRightEnterPin(viewRight, NULL, pinType, NULL);
        return;
    }

    const char *pinTypeStr = "";
    if (pinType == QBViewRightPinType_NonScPin) {
        pinTypeStr = gettext("(Non smartcard)");
    }

    SvString text = SvStringCreateWithFormat("%s %s %s %s %s", gettext("Credit:"), SvStringCString(availableCredit), gettext("Cost:"),
                                             SvStringCString(costOfEvent), pinTypeStr);

    QBViewRightPopupManagerPinDialogShow(self, pinType, text, false);

    SVRELEASE(text);
}

SvLocal void QBViewRightPopupPinDialogCreate(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightPinDialog pd = &self->pinDialog;
    pd->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "ViewRightPINDialog", QBViewRightPopupPinDialogStep, self);
    pd->hideTimer = SvFiberTimerCreate(pd->fiber);
}

SvLocal void QBViewRightPopupPinDialogRefresh(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightPinDialog pd = &self->pinDialog;
    SvFiberTimerActivateAfter(pd->hideTimer, SvTimeConstruct(QBVIEWRIGHT_PIN_TIME_TICKET, 0));
}

SvLocal void QBViewRightPopupPinDialogInit(QBViewRightPopupManager self, QBViewRightPinType pinType, SvString text)
{
    log_fun();
    QBViewRightPinDialog pd = &self->pinDialog;
    if (pd->dialog)
        return;

    AppGlobals appGlobals = self->appGlobals;

    const char *message = SvStringCString(text);
    SvString domain;

    switch (pinType) {
        case QBViewRightPinType_ParentalControlPin2:
            domain = SVSTRING("QBViewRightPinType_PC");
            break;
        case QBViewRightPinType_IPPV_PC_1:
            domain = SVSTRING("QBViewRightPinType_IPPV_PC_1");
            break;
        case QBViewRightPinType_STBLockPin:
            domain = SVSTRING("QBViewRightPinType_STBLockPin");
            break;
        case QBViewRightPinType_HomeShopping:
            domain = SVSTRING("QBViewRightPinType_HomeShopping");
            break;
        case QBViewRightPinType_NonScPin:
            domain = SVSTRING("QBViewRightPinType_NonScPin");
            break;
        default:
            log_error("Unknown pin type = 0x%X", pinType);
            return;
    }

    log_state("pinType(%d) -> domain (%s)", pinType, SvStringCString(domain));

    SvGenericObject authenticator = QBAuthenticateViaAccessManager(appGlobals->scheduler, appGlobals->accessMgr, domain);

    svSettingsPushComponent("ViewRightPopup.settings");
    pd->dialog = QBAuthDialogCreate(appGlobals, authenticator, gettext("Authentication required"), message, false, SVSTRING("ViewRightManagerPINPopup"), &pd->helper);
    pd->pinType = pinType;
    svSettingsPopComponent();
}

SvLocal void QBViewRightPopupPinDialogRemove(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightPinDialog pd = &self->pinDialog;
    QBDialogBreak(pd->dialog);
    pd->dialog = NULL;
}

/**
 * Hides pin dialog
 * @param self
 * @param force true means hiding dialog always, false means hiding only if entering PIN is enabled
 */
SvLocal void QBViewRightPopupPinDialogHide(QBViewRightPopupManager self, bool force)
{
    log_fun();
    log_deep("force=%d", force);
    QBViewRightPinDialog pd = &self->pinDialog;
    SvFiberEventDeactivate(pd->hideTimer);
    if (pd->dialog && (force || !QBAuthDialogIsEnteringPinDisabled(pd->helper))) {
        log_deep("Hiding PIN Dialog");
        QBViewRightPopupPinDialogRemove(self);
    }
}

SvLocal void QBViewRightPopupPinDialogStop(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightPinDialog pd = &self->pinDialog;
    SvFiberDeactivate(pd->fiber);
    SvFiberEventDeactivate(pd->hideTimer);
}

SvLocal void QBViewRightPopupPinDialogStep(void *self_)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;
    QBViewRightPinDialog pd = &self->pinDialog;
    SvFiberDeactivate(pd->fiber);
    if (SvFiberEventIsActive(pd->hideTimer)) {
        QBViewRightPopupPinDialogHide(self, false);
    }
}

SvLocal void QBViewRightPopupPinDialogDestroy(QBViewRightPopupManager self)
{
    log_fun();
    QBViewRightPinDialog pd = &self->pinDialog;
    SvFiberDestroy(pd->fiber);
    if (pd->dialog) {
        QBViewRightPopupPinDialogRemove(self);
    }
}

SvLocal void QBViewRightPopupManagerVisabilityChanged(SvGenericObject self_, bool hidden)
{
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;
    self->disabledWindows = hidden;
    if (hidden) {
        QBViewRightPopupManagerHoldWindows(self);
    } else {
        QBViewRightPopupManagerUpdate(self);
    }

}

SvLocal void QBViewRightPopupManagerPlaybackEvent(SvGenericObject self_, SvString name, void *arg)
{
    log_fun();
    QBViewRightPopupManager self = (QBViewRightPopupManager) self_;

    if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_ON)) {
        self->FTA = false;
        QBViewRightPopupManagerUpdate(self);
    } else if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_OFF)) {
        self->FTA = true;
        if (self->notification.currentDialog) {
            QBViewRightMMINotificationHold(self);
        }
        QBViewRightPopupManagerUpdate(self);
    }
}
