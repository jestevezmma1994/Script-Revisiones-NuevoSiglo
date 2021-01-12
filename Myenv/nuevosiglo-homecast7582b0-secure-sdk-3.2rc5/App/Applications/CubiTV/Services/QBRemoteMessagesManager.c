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

#include "QBRemoteMessagesManager.h"

#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <QBApplicationController.h>
#include <QBInput/QBInputCodes.h>
#include <QBInput/QBInputEvent.h>
#include <QBInput/QBInputFilter.h>
#include <QBInput/QBInputQueue.h>
#include <QBInput/QBInputService.h>
#include <SWL/label.h>
#include <Services/core/QBPushReceiver.h>
#include <Services/QBStandbyAgent.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvDeque.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <Windows/standby.h>
#include <fibers/c/fibers.h>
#include <main.h>
#include <settings.h>
#include <stdbool.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "QBRemoteMessagesManagerLogLevel", "");

#define log_error(fmt, ...) do { if (env_log_level() >= 0) { SvLogError(COLBEG() "QBRemoteMessagesManager: " fmt COLEND_COL(red), ##__VA_ARGS__); }; } while (0)
#define log_warning(fmt, ...) do { if (env_log_level() >= 0) { SvLogWarning(COLBEG() "QBRemoteMessagesManager: " fmt COLEND_COL(green), ##__VA_ARGS__); }; } while (0)
#define log_debug(fmt, ...) do { if (env_log_level() >= 1) { SvLogNotice(COLBEG() "QBRemoteMessagesManager: " fmt COLEND_COL(green), ##__VA_ARGS__); }; } while (0)

#define REMOTE_MESSAGE_TYPE "RM"
#define REMOTE_COMMAND      "show_alert"

#define POPUP_HINT_TEXT             "Press EXIT to hide message"
#define POPUP_AUTOHIDE_TIMEOUT_SEC  60

/**
 * Remote Messagess Manager class
 **/

struct QBRemoteMessagesManagerMessagePopup_ {
    struct SvObject_ super_;

    QBGlobalWindow window;

    SvWidget label;
    SvWidget hint;
};

typedef struct QBRemoteMessagesManagerMessagePopup_ *QBRemoteMessagesManagerMessagePopup;

struct QBRemoteMessagesManagerMessageData_ {
    struct SvObject_ super_;

    SvString boxType;
    SvString text;

    bool autoHide;
    bool allowExit;

    int timer;
};

typedef struct QBRemoteMessagesManagerMessageData_ *QBRemoteMessagesManagerMessageData;

struct QBRemoteMessagesManager_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    bool started;
    SvDeque messages;

    QBGlobalWindow currentMessageWindow;
    int currentPopupTimeoutSec;
    bool currentPopupAllowExit;
    bool currentPopupAllowAutoHide;
    bool isCurrentPopShown;

    QBRemoteMessagesManagerMessagePopup softPopup;
    QBRemoteMessagesManagerMessagePopup redPopup;
    QBRemoteMessagesManagerMessagePopup bluePopup;

    SvFiber fiber;
    SvFiberTimer timer;
};

// private methods
SvLocal void
QBRemoteMessagesManagerMessagePopup__dtor__(void *self_)
{
    QBRemoteMessagesManagerMessagePopup self = (QBRemoteMessagesManagerMessagePopup) self_;

    SVRELEASE(self->window);
}

SvLocal SvType
QBRemoteMessagesManagerMessagePopup_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRemoteMessagesManagerMessagePopup__dtor__
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRemoteMessagesManagerMessagePopup", sizeof(struct QBRemoteMessagesManagerMessagePopup_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
QBRemoteMessagesManagerCreateMessagePopup(QBRemoteMessagesManager self, QBRemoteMessagesManagerMessagePopup messagePopup, const char *popupType)
{
    char *wname;
    asprintf(&wname, "RemoteMessagesManager.%s", popupType);
    SvWindow window = svSettingsWidgetCreate(self->appGlobals->res, wname);
    free(wname);

    asprintf(&wname, "RemoteMessagesManager.%s.Background", popupType);
    SvWidget background = svSettingsWidgetCreate(self->appGlobals->res, wname);
    svSettingsWidgetAttach(window, background, wname, 1);
    free(wname);

    asprintf(&wname, "RemoteMessagesManager.%s.Text", popupType);
    messagePopup->label = svLabelNewFromSM(self->appGlobals->res, wname);
    svSettingsWidgetAttach(window, messagePopup->label, wname, 1);
    free(wname);

    asprintf(&wname, "RemoteMessagesManager.%s.Hint", popupType);
    messagePopup->hint = svLabelNewFromSM(self->appGlobals->res, wname);
    svSettingsWidgetAttach(window, messagePopup->hint, wname, 1);
    free(wname);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    asprintf(&wname, "RemoteMessagesManager.%s", popupType);
    QBGlobalWindowInit(globalWindow, window, SVSTRING("RemoteMessagesManager"));
    free(wname);
    messagePopup->window = globalWindow;
}

SvLocal void
QBRemoteMessagesManagerCreateMessagePopups(QBRemoteMessagesManager self)
{
    self->softPopup = (QBRemoteMessagesManagerMessagePopup) SvTypeAllocateInstance(QBRemoteMessagesManagerMessagePopup_getType(), NULL);
    self->redPopup = (QBRemoteMessagesManagerMessagePopup) SvTypeAllocateInstance(QBRemoteMessagesManagerMessagePopup_getType(), NULL);
    self->bluePopup = (QBRemoteMessagesManagerMessagePopup) SvTypeAllocateInstance(QBRemoteMessagesManagerMessagePopup_getType(), NULL);

    QBRemoteMessagesManagerCreateMessagePopup(self, self->softPopup, "SoftPopup");
    QBRemoteMessagesManagerCreateMessagePopup(self, self->redPopup, "RedPopup");
    QBRemoteMessagesManagerCreateMessagePopup(self, self->bluePopup, "BluePopup");
}

SvLocal void
QBRemoteMessagesManagerMessageData__dtor__(void *self_)
{
    QBRemoteMessagesManagerMessageData self = (QBRemoteMessagesManagerMessageData) self_;

    SVRELEASE(self->boxType);
    SVRELEASE(self->text);
}

SvLocal SvType
QBRemoteMessagesManagerMessageData_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRemoteMessagesManagerMessageData__dtor__
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRemoteMessagesManagerMessageData", sizeof(struct QBRemoteMessagesManagerMessageData_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
QBRemoteMessagesManagerPrepareNewMessageWindow(QBRemoteMessagesManager self, QBRemoteMessagesManagerMessageData message)
{
    log_debug("Prepare new message window");
    SvString boxType = message->boxType;
    SvString text = message->text;

    self->currentPopupTimeoutSec = message->timer;
    self->currentPopupAllowExit = message->allowExit;
    self->currentPopupAllowAutoHide = message->autoHide;

    // If we received remote message with disabled both options: autohide and allow exit,
    // then we show this message with enabled option autohide and timeout set to 60 second.
    if (!self->currentPopupAllowExit && !self->currentPopupAllowAutoHide) {
        self->currentPopupTimeoutSec = POPUP_AUTOHIDE_TIMEOUT_SEC;
        self->currentPopupAllowAutoHide = true;
    }

    if (SvStringEqualToCString(boxType, "soft_box")) {
        svLabelSetText(self->softPopup->label, SvStringCString(text));
        if (self->currentPopupAllowExit) {
            svLabelSetText(self->softPopup->hint, POPUP_HINT_TEXT);
        } else {
            svLabelSetText(self->softPopup->hint, "");
        }
        self->currentMessageWindow = SVRETAIN(self->softPopup->window);
    } else if (SvStringEqualToCString(boxType, "red_box")) {
        svLabelSetText(self->redPopup->label, SvStringCString(text));
        if (self->currentPopupAllowExit) {
            svLabelSetText(self->redPopup->hint, POPUP_HINT_TEXT);
        } else {
            svLabelSetText(self->redPopup->hint, "");
        }
        self->currentMessageWindow = SVRETAIN(self->redPopup->window);
    } else if (SvStringEqualToCString(boxType, "blue_box")) {
        svLabelSetText(self->bluePopup->label, SvStringCString(text));
        if (self->currentPopupAllowExit) {
            svLabelSetText(self->bluePopup->hint, POPUP_HINT_TEXT);
        } else {
            svLabelSetText(self->bluePopup->hint, "");
        }
        self->currentMessageWindow = SVRETAIN(self->bluePopup->window);
    }
}

SvLocal void
QBRemoteMessagesManagerStep(void *self_)
{
    QBRemoteMessagesManager self = (QBRemoteMessagesManager) self_;

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    self->currentPopupTimeoutSec = 0;

    if (self->currentMessageWindow) {
        log_debug("Remove current message window");
        QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, self->currentMessageWindow);
        SVRELEASE(self->currentMessageWindow);
        self->currentMessageWindow = NULL;
        self->currentPopupAllowExit = false;
        self->currentPopupAllowAutoHide = false;
        self->isCurrentPopShown = false;
    }

    int count = SvDequeCount(self->messages);
    if (count == 0) {
        return;
    }

    QBRemoteMessagesManagerMessageData message = (QBRemoteMessagesManagerMessageData) SvDequeTakeBack(self->messages);

    log_debug("New message: boxType: %s, allowExit: %d, autoHide: %d, timer: %d \n text: %s", SvStringCString(message->boxType), message->allowExit,
              message->autoHide, message->timer, SvStringCString(message->text));

    QBRemoteMessagesManagerPrepareNewMessageWindow(self, message);
    QBApplicationControllerAddGlobalWindow(self->appGlobals->controller, self->currentMessageWindow);
    self->isCurrentPopShown = true;
    log_debug("Add new message window");

    SVRELEASE(message);

    if (self->currentPopupAllowExit && !self->currentPopupAllowAutoHide) {
        return;
    }

    log_debug("Start processing new message after %d seconds", self->currentPopupTimeoutSec);
    SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(self->currentPopupTimeoutSec * 1000));
}

SvLocal void
QBRemoteMessagesManagerAddMessageToQueue(QBRemoteMessagesManager self, QBRemoteMessagesManagerMessageData message)
{
    bool standby = false;
    QBStandbyStateData standbyData = QBStandbyAgentGetCurrentDataState(self->appGlobals->standbyAgent);
    if (standbyData && QBStandbyStateDataGetState(standbyData) != QBStandbyState_on) {
        standby = true;
    }

    if (standby) {
        if (message->autoHide) {
            log_debug("Add message to queue");
            SvDequePushFront(self->messages, (SvObject) message, NULL);
        }
    } else {
        log_debug("Add message to queue");
        SvDequePushFront(self->messages, (SvObject) message, NULL);
    }

    if (!standby && (SvDequeCount(self->messages) == 1) && self->isCurrentPopShown == false) {
        log_debug("Start processing first message");
        SvFiberActivate(self->fiber);
    }
}

SvLocal QBRemoteMessagesManagerMessageData
QBRemoteMessagesManageCreateMessageData(QBRemoteMessagesManager self, SvHashTable args)
{
    SvValue autoHideVal = (SvValue) SvHashTableFind(args, (SvObject) SVSTRING("auto_hide"));
    if (!autoHideVal || !SvObjectIsInstanceOf((SvObject) autoHideVal, SvValue_getType()) || !SvValueIsBoolean(autoHideVal)) {
        log_error("Remote command: %s - wrong format (auto_hide)", REMOTE_COMMAND);
        return NULL;
    }

    SvValue boxTypeVal = (SvValue) SvHashTableFind(args, (SvObject) SVSTRING("box_type"));
    if (!boxTypeVal || !SvObjectIsInstanceOf((SvObject) boxTypeVal, SvValue_getType()) || !SvValueIsString(boxTypeVal)) {
        log_error("Remote command: %s - wrong format (box_type)", REMOTE_COMMAND);
        return NULL;
    }

    SvValue textVal = (SvValue) SvHashTableFind(args, (SvObject) SVSTRING("text"));
    if (!textVal || !SvObjectIsInstanceOf((SvObject) textVal, SvValue_getType()) || !SvValueIsString(textVal)) {
        log_error("Remote command: %s - wrong format (text)", REMOTE_COMMAND);
        return NULL;
    }

    SvValue allowExitVal = (SvValue) SvHashTableFind(args, (SvObject) SVSTRING("allow_exit"));
    if (!allowExitVal || !SvObjectIsInstanceOf((SvObject) allowExitVal, SvValue_getType()) || !SvValueIsBoolean(allowExitVal)) {
        log_error("Remote command: %s - wrong format (allow_exit)", REMOTE_COMMAND);
        return NULL;
    }

    SvValue timerVal = (SvValue) SvHashTableFind(args, (SvObject) SVSTRING("timer"));
    if (!timerVal || !SvObjectIsInstanceOf((SvObject) timerVal, SvValue_getType())) {
        log_error("Remote command: %s - wrong format (timer)", REMOTE_COMMAND);
        return NULL;
    }

    QBRemoteMessagesManagerMessageData messageData = (QBRemoteMessagesManagerMessageData) SvTypeAllocateInstance(QBRemoteMessagesManagerMessageData_getType(), NULL);

    messageData->autoHide = SvValueGetBoolean(autoHideVal);
    messageData->boxType = SVRETAIN(SvValueGetString(boxTypeVal));
    messageData->text = SVRETAIN(SvValueGetString(textVal));
    messageData->allowExit = SvValueGetBoolean(allowExitVal);

    int timer = -1;
    bool validValue = true;
    if (SvValueIsInteger(timerVal)) {
        timer = SvValueGetInteger(timerVal);
    } else if (SvValueIsString(timerVal)) {
        char *delim;
        timer = strtol(SvStringCString(SvValueGetString(timerVal)), &delim, 10);
        if (*delim != '\0') {
            validValue = false;
        }
    }

    if (timer != -1 && validValue) {
        messageData->timer = timer;
    } else {
        log_debug("Remote command: %s - invalid value (timer)", REMOTE_COMMAND);
        SVRELEASE(messageData);
        return NULL;
    }

    return messageData;
}

SvLocal void
QBRemoteMessagesManagerPushDataReceived(SvObject self_,
                                        SvString type,
                                        SvObject data_)
{
    QBRemoteMessagesManager self = (QBRemoteMessagesManager) self_;
    SvHashTable data = (SvHashTable) data_;

    if (!SvObjectEquals((SvObject) type, (SvObject) SVSTRING(REMOTE_MESSAGE_TYPE))) {
        return;
    }

    SvValue cmdVal = (SvValue) SvHashTableFind(data, (SvObject) SVSTRING("command"));
    if (!cmdVal || !SvValueIsString(cmdVal)) {
        return;
    }

    SvString cmd = SvValueGetString(cmdVal);
    if (!SvObjectEquals((SvObject) cmd, (SvObject) SVSTRING(REMOTE_COMMAND))) {
        log_error("Unsupported command");
        return;
    }

    SvHashTable args = (SvHashTable) SvHashTableFind(data, (SvObject) SVSTRING("args"));
    if (!args || !SvObjectIsInstanceOf((SvObject) args, SvHashTable_getType())) {
        log_error("Remote command: %s - wrong format", REMOTE_COMMAND);
        return;
    }

    QBRemoteMessagesManagerMessageData newMessage = QBRemoteMessagesManageCreateMessageData(self, args);
    if (newMessage) {
        log_debug("Received new message");
        QBRemoteMessagesManagerAddMessageToQueue(self, newMessage);
    }
    SVTESTRELEASE(newMessage);
}

SvLocal void
QBRemoteMessagesManagerPushStatusChanged(SvObject self_,
                                         QBPushReceiverStatus status)
{
}

SvLocal void
QBRemoteMessagesManagerSwitchStarted(SvObject self_, QBWindowContext from, QBWindowContext to)
{
    QBRemoteMessagesManager self = (QBRemoteMessagesManager) self_;
    if (from && SvObjectIsInstanceOf((SvObject) from, QBStandbyContext_getType())) {
        if (SvDequeCount(self->messages) != 0) {
            SvFiberActivate(self->fiber);
            log_debug("Leaving standby - start processing messages");
        }
    }
}

SvLocal void
QBRemoteMessagesManagerSwitchEnded(SvObject self_, QBWindowContext from, QBWindowContext to)
{
}

SvLocal SvString
QBRemoteMessagesManagerGetName(SvObject self_)
{
    return SVSTRING("QBRemoteMessagesManager");
}

SvLocal unsigned int
QBRemoteMessagesManagerProcessEvents(SvObject self_, QBInputQueue outQueue, QBInputQueue inQueue)
{
    QBRemoteMessagesManager self = (QBRemoteMessagesManager) self_;
    QBInputEvent ev;

    for (;;) {
        ev = QBInputQueueGet(inQueue);
        if (ev.type == QBInputEventType_keyTyped) {
            if (ev.u.key.code == QBKEY_BACK) {
                if (self->currentMessageWindow && self->currentPopupAllowExit) {
                    SvFiberActivate(self->fiber);
                    log_debug("EXIT button is pressed - start removing current message");
                    break;
                }
            }
        }

        if (ev.type != QBInputEventType_invalid) {
            QBInputQueuePut(outQueue, ev);
        } else {
            break;
        }
    }

    return 0;
}

SvLocal void
QBRemoteMessagesManager__dtor__(void *self_)
{
    QBRemoteMessagesManager self = (QBRemoteMessagesManager) self_;

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
    }

    SVRELEASE(self->softPopup);
    SVRELEASE(self->redPopup);
    SVRELEASE(self->bluePopup);
    SVRELEASE(self->messages);

    SVTESTRELEASE(self->currentMessageWindow);
}

SvLocal SvType
QBRemoteMessagesManager_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRemoteMessagesManager__dtor__
    };

    static const struct QBPushReceiverListener_t pushReceiverMethods = {
        .dataReceived = QBRemoteMessagesManagerPushDataReceived,
        .statusChanged = QBRemoteMessagesManagerPushStatusChanged
    };

    static const struct QBContextSwitcherListener_t switcherMethods = {
        .started = QBRemoteMessagesManagerSwitchStarted,
        .ended   = QBRemoteMessagesManagerSwitchEnded
    };

    static const struct QBInputFilter_ inputFilterMethods = {
        .getName       = QBRemoteMessagesManagerGetName,
        .processEvents = QBRemoteMessagesManagerProcessEvents
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRemoteMessagesManager", sizeof(struct QBRemoteMessagesManager_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBPushReceiverListener_getInterface(), &pushReceiverMethods,
                            QBContextSwitcherListener_getInterface(), &switcherMethods,
                            QBInputFilter_getInterface(), &inputFilterMethods,
                            NULL);
    }

    return type;
}

// public API
QBRemoteMessagesManager
QBRemoteMessagesManagerCreate(AppGlobals appGlobals,
                              SvErrorInfo *errorOut)
{
    QBRemoteMessagesManager self = NULL;
    SvErrorInfo error = NULL;

    if (!appGlobals) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "NULL argument passed");
        goto fini;
    }

    self = (QBRemoteMessagesManager) SvTypeAllocateInstance(QBRemoteMessagesManager_getType(), &error);
    if (!self) {
        if (!error) {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory, "can't allocate QBRemoteMessages");
        }
        goto fini;
    }

    self->appGlobals = appGlobals;
    self->messages = SvDequeCreate(NULL);
    self->currentPopupTimeoutSec = 0;
    self->currentPopupAllowExit = false;
    self->currentPopupAllowAutoHide = false;
    self->isCurrentPopShown = false;

    svSettingsPushComponent("RemoteMessagesManager.settings");
    QBRemoteMessagesManagerCreateMessagePopups(self);
    svSettingsPopComponent();

    log_debug("QBRemoteMessagesManager service is created");

fini:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

void
QBRemoteMessagesManagerStart(QBRemoteMessagesManager self,
                             SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "NULL argument passed");
        goto fini;
    } else if (!self->appGlobals->remoteMessagesManager) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState, "remoteMessagesManager not created");
        goto fini;
    } else if (self->started) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState, "remoteMessagesManager already started");
        goto fini;
    }

    self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBRemoteMessagesManager", QBRemoteMessagesManagerStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);

    QBPushReceiverAddListener(self->appGlobals->pushReceiver, (SvObject) self, SVSTRING(REMOTE_MESSAGE_TYPE), &error);
    QBApplicationControllerAddListener(self->appGlobals->controller, (SvObject) self);
    QBInputServiceAddGlobalFilter((SvObject) self, NULL, NULL);

    self->started = true;

    log_debug("QBRemoteMessagesManager service is started");

fini:
    SvErrorInfoPropagate(error, errorOut);
}

void
QBRemoteMessagesManagerStop(QBRemoteMessagesManager self,
                            SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "NULL argument passed");
        goto fini;
    } else if (!self->appGlobals->remoteMessagesManager) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState, "remoteMessagesManager not created");
        goto fini;
    } else if (!self->started) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState, "remoteMessagesManager not started");
        goto fini;
    }

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->timer = NULL;
    }

    QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, self->currentMessageWindow);
    QBPushReceiverRemoveListener(self->appGlobals->pushReceiver, (SvObject) self, &error);
    QBApplicationControllerRemoveListener(self->appGlobals->controller, (SvObject) self);
    QBInputServiceRemoveFilter((SvObject) self, NULL);

    self->started = false;

    log_debug("QBRemoteMessagesManager service is stopped");

fini:
    SvErrorInfoPropagate(error, errorOut);
}
