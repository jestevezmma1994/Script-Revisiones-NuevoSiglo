/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#include <Services/QBCallerIdManager.h>

#include <libintl.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvWeakList.h>
#include <SvFoundation/SvHashTable.h>
#include <QBInput/QBInputCodes.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/QBFrame.h>
#include <SWL/label.h>
#include <SWL/move.h>
#include <SvJSON/SvJSONParse.h>
#include <safeFiles.h>
#include <QBStringUtils.h>
#include <QBConf.h>
#include <SvHTTPClient/SvHTTPRequest.h>
#include <SvHTTPClient/SvHTTPClientListener.h>
#include <SvHTTPClient/SvHTTPInfoListener.h>
#include <SvHTTPClient/SvHTTPClientError.h>
#include <Services/core/QBPushReceiver.h>
#include <Services/core/volume.h>
#include <Windows/newtv.h>
#include <QBWidgets/QBDialog.h>
#include <main.h>
#include <stdlib.h>
#include <stdbool.h>

#define log_debug(fmt, ...)  if(0) SvLogInfo(COLBEG() "QBCallerIdManager: %s:%d " fmt  COLEND_COL(blue), __func__,__LINE__,##__VA_ARGS__)
#define log_error(fmt, ...)  if(1) SvLogError(COLBEG() "QBCallerIdManager: %s:%d " fmt  COLEND_COL(red), __func__,__LINE__,##__VA_ARGS__)

#define CALLID_DEFAULT_TIMEOUT  15  // [s]
#define CALLID_MAX_TIMEOUT      120 // [s]

#define VOICEMAIL_REQUEST_TIMEOUT   30  // [s]

#define FAILURE_DIALOG_TIMEOUT      3   // [s]

#define REMOTE_MESSAGE_TYPE "caller_id"
#define REMOTE_COMMAND      "call_data"

typedef enum QBCallerIdManagerState_t {
    QBCallerIdManagerState_Disabled,
    QBCallerIdManagerState_Enabled
} QBCallerIdManagerState;

typedef enum QBCallerIdManagerBannerState_t {
    QBCallerIdManagerBannerState_Hidden,
    QBCallerIdManagerBannerState_Hiding,
    QBCallerIdManagerBannerState_Shown,
    QBCallerIdManagerBannerState_Showing,
    QBCallerIdManagerBannerState_Cancelled
} QBCallerIdManagerBannerState;

struct QBCallerIdManagerCallData_t {
    struct SvObject_ super_;

    SvString callID;
    SvURL voicemail;
    SvString line;
    SvString callerName;
    SvString number;
};
typedef struct QBCallerIdManagerCallData_t* QBCallerIdManagerCallData;

struct QBCallerIdManager_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    QBGlobalWindow statusWindow;

    QBCallerIdManagerState state;
    int bannerTimeout;

    SvWidget callingMessage;
    SvWidget lineMessage;

    SvWeakList listeners;

    SvFiber statusFiber;
    SvFiberTimer statusTimer;

    double effectDuration;
    SvEffectId showEffectId;

    QBCallerIdManagerBannerState bannerState;

    bool started;

    QBCallerIdManagerCallData currentCallData;
    QBCallerIdManagerCallData newCallData;

    SvHTTPRequest voicemailHttpRequest;

    SvString historyFilename;
    SvArray callsHistory;
    QBJSONSerializer JSONserializer;
    unsigned historySize;

    SvWidget failureDialog;
    SvWidgetId failureDialogId;

    SvFiber failureFiber;
    SvFiberTimer failureTimer;
};

//----------------------------------------------------------------
SvLocal void
QBCallerIdManagerUpdateHistory(QBCallerIdManager self);

SvLocal void
QBCallerIdManagerNotifyListeners(QBCallerIdManager self);

SvLocal void
QBCallerIdManagerDroppingToVoicemailFailed(QBCallerIdManager self);

//----------------------------------------------------------------
SvLocal void
QBCallerIdManagerStopVoicemailRequest(QBCallerIdManager self)
{
    if (!self->voicemailHttpRequest) {
        return;
    }

    SvHTTPRequestCancel(self->voicemailHttpRequest, NULL);
    SVRELEASE(self->voicemailHttpRequest);
    self->voicemailHttpRequest = NULL;
}

SvLocal void
QBCallerIdManagerCallData__dtor__(void *self_)
{
    QBCallerIdManagerCallData self = self_;
    SVRELEASE(self->callID);
    SVTESTRELEASE(self->voicemail);
    SVTESTRELEASE(self->line);
    SVTESTRELEASE(self->callerName);
    SVRELEASE(self->number);
}

SvLocal SvType
QBCallerIdManagerCallData_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBCallerIdManagerCallData__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBCallerIdManagerCallData",
                            sizeof(struct QBCallerIdManagerCallData_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
QBCallerIdManagerDestroy(void *self_)
{
    QBCallerIdManager self = self_;

    SVTESTRELEASE(self->statusWindow);
    SVTESTRELEASE(self->currentCallData);
    SVTESTRELEASE(self->newCallData);

    if (self->statusFiber) {
        SvFiberDestroy(self->statusFiber);
        self->statusFiber = NULL;
    }

    if (self->failureFiber) {
        SvFiberDestroy(self->failureFiber);
        self->failureFiber = NULL;
    }

    SVRELEASE(self->callsHistory);
    SVRELEASE(self->historyFilename);
    SVRELEASE(self->JSONserializer);
    SVRELEASE(self->listeners);
}

SvLocal void
QBCallerIdManagerUpdateStateFromConfig(QBCallerIdManager self)
{
    const char* CALLID_STATE = QBConfigGet("CALLID_STATE");
    if (CALLID_STATE) {
        if (!strncasecmp(CALLID_STATE, "enabled", 7)) {
            self->state = QBCallerIdManagerState_Enabled;
        } else {
            self->state = QBCallerIdManagerState_Disabled;
        }
    }
}

SvLocal void
QBCallerIdManagerUpdateTimeoutFromConfig(QBCallerIdManager self)
{
    const char* CALLID_TIMEOUT = QBConfigGet("CALLID_TIMEOUT");
    if (CALLID_TIMEOUT) {
        long int timeout = strtol(CALLID_TIMEOUT, NULL, 10);
        if ((timeout > 0) && (timeout < CALLID_MAX_TIMEOUT)) {
            self->bannerTimeout = timeout;
        }
    }
}

SvLocal void
QBCallerIdManagerStartEffect(QBCallerIdManager self)
{
    SvWidget w = self->statusWindow->window;

    if (self->showEffectId > 0) {
        svAppCancelEffect(self->appGlobals->res, self->showEffectId, SV_EFFECT_FINISH);
    }

    SvEffect showEffect = svEffectMoveNew(w, w->off_x , w->off_y + w->height * ((self->bannerState == QBCallerIdManagerBannerState_Showing) ? -1 : 1),
                                          self->effectDuration, SV_EFFECT_MOVE_LINEAR, 0);

    if (showEffect) {
        self->showEffectId = svAppRegisterEffect(self->appGlobals->res, showEffect);
    }
    svWidgetForceRender(w);
}

SvLocal void
QBCallerIdManagerPrepareStatus(QBCallerIdManager self)
{
    char* callingStr;
    if (self->currentCallData->callerName) {
        asprintf(&callingStr, "%s: %s %s", gettext("Calling"), SvStringCString(self->currentCallData->callerName),
                                                               SvStringCString(self->currentCallData->number));
    } else {
        asprintf(&callingStr, "%s: %s", gettext("Calling"), SvStringCString(self->currentCallData->number));
    }
    svLabelSetText(self->callingMessage, callingStr);
    free(callingStr);

    if (self->currentCallData->line) {
        char* lineStr;
        asprintf(&lineStr, "%s: %s", gettext("Line"), SvStringCString(self->currentCallData->line));
        svLabelSetText(self->lineMessage, lineStr);
        free(lineStr);
    } else {
        svLabelSetText(self->lineMessage, "");
    }

    unsigned int height = 0;
    svAppGetDimensions(self->appGlobals->res, NULL, &height);
    self->statusWindow->window->off_y = height;
}

SvLocal void
QBCallerIdManagerSwitchToNewStatus(QBCallerIdManager self)
{
    self->currentCallData = self->newCallData;
    self->newCallData = NULL;
    QBCallerIdManagerPrepareStatus(self);
    self->bannerState = QBCallerIdManagerBannerState_Showing;
    QBCallerIdManagerStartEffect(self);
    SvFiberTimerActivateAfter(self->statusTimer, SvTimeConstruct(self->bannerTimeout, 0));
}

SvLocal void
QBCallerIdManagerHideOSD(QBCallerIdManager self)
{
    if (QBTVOSDIsVisible(self->appGlobals->newTV)) {
        QBTVOSDHide(self->appGlobals->newTV, true, true);
    }
    QBTVOSDLock(self->appGlobals->newTV, true, true);

    QBVolumeWidgetHideGUI(self->appGlobals->volumeWidget);
}

SvLocal void
QBCallerIdManagerUpdateCalls(QBCallerIdManager self)
{
    if (self->bannerState == QBCallerIdManagerBannerState_Showing) {
        self->bannerState = QBCallerIdManagerBannerState_Shown;
        if (!self->newCallData) {   // if there is a new call we should start hiding current banner
            return;
        }
    }

    if (self->bannerState == QBCallerIdManagerBannerState_Hidden) {    // single new call
        log_debug("Show call status");
        QBCallerIdManagerHideOSD(self);
        QBApplicationControllerAddGlobalWindow(self->appGlobals->controller, self->statusWindow);
        QBCallerIdManagerSwitchToNewStatus(self);
        return;
    }

    if ( (self->bannerState == QBCallerIdManagerBannerState_Shown) ||
         (self->bannerState == QBCallerIdManagerBannerState_Cancelled) ) {      // start hiding
        log_debug("Start hiding call status");
        self->bannerState = QBCallerIdManagerBannerState_Hiding;
        QBCallerIdManagerStartEffect(self);
        return;
    }

    // hiding ended
    log_debug("Call status hiding ended");
    self->bannerState = QBCallerIdManagerBannerState_Hidden;

    SVRELEASE(self->currentCallData);
    self->currentCallData = NULL;

    if (!self->newCallData) {
        QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, self->statusWindow);
        QBTVOSDLock(self->appGlobals->newTV, false, false);
        QBVolumeWidgetEnableGUI(self->appGlobals->volumeWidget);
        return;
    }

    log_debug("Show new call status");
    QBCallerIdManagerSwitchToNewStatus(self);
}

SvLocal void
QBCallerIdManagerStatusStep(void *self_)
{
    QBCallerIdManager self = self_;

    SvFiberDeactivate(self->statusFiber);
    SvFiberEventDeactivate(self->statusTimer);
    QBCallerIdManagerUpdateCalls(self);
}

SvLocal void
QBCallerIdManagerEffectEventHandler(SvWidget w, SvEffectEvent e)
{
    QBCallerIdManager self = (QBCallerIdManager)w->prv;
    if (e->id == self->showEffectId) {
        self->showEffectId = 0;
        if ( (self->bannerState == QBCallerIdManagerBannerState_Hiding) ||
             (self->bannerState == QBCallerIdManagerBannerState_Showing) ) {
            QBCallerIdManagerUpdateCalls(self);
        }
        return;
    }
}

SvLocal QBCallerIdManagerCallData
QBCallerIdManagerPrepareCallData(QBCallerIdManager self, SvHashTable args)
{
    SvValue callIDVal = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("call_id"));
    if (!callIDVal || !SvValueIsString(callIDVal)) {
        log_error("Remote command - wrong format (call_id)");
        return NULL;
    }

    SvValue voicemailVal = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("voicemail"));
    if (voicemailVal) {
        if (SvValueIsNULL(voicemailVal)) {
            // can be null
            voicemailVal = NULL;
        } else if (!SvValueIsString(voicemailVal)) {
            log_error("Remote command - wrong format (voicemail %d)", SvValueGetType(voicemailVal));
            return NULL;
        }
    }

    SvValue lineVal = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("line"));
    if (lineVal) {
        if (SvValueIsNULL(lineVal)) {
            // can be null
            lineVal = NULL;
        } else if (!SvValueIsString(lineVal)) {
            log_error("Remote command - wrong format (line)");
            return NULL;
        }
    }

    SvHashTable callerHT = (SvHashTable) SvHashTableFind(args, (SvGenericObject) SVSTRING("caller"));
    if (!SvObjectIsInstanceOf((SvGenericObject) callerHT, SvHashTable_getType())) {
        log_error("Remote command - wrong format (caller)");
        return NULL;
    }

    SvValue nameVal = (SvValue) SvHashTableFind(callerHT, (SvGenericObject) SVSTRING("name"));
    if (nameVal) {
        if (SvValueIsNULL(nameVal)) {
            // can be null
            nameVal = NULL;
        } else if (!SvValueIsString(nameVal)) {
            log_error("Remote command - wrong format (caller name)");
            return NULL;
        }
    }

    SvValue numberVal = (SvValue) SvHashTableFind(callerHT, (SvGenericObject) SVSTRING("number"));
    if (!numberVal || !SvValueIsString(numberVal)) {
        log_error("Remote command - wrong format (caller number)");
        return NULL;
    }

    QBCallerIdManagerCallData callData = (QBCallerIdManagerCallData) SvTypeAllocateInstance(QBCallerIdManagerCallData_getType(), NULL);

    callData->callID = SVRETAIN(SvValueGetString(callIDVal));

    if (voicemailVal) {
        callData->voicemail = SvURLCreateWithString(SvValueGetString(voicemailVal), NULL);
    }

    if (lineVal) {
        callData->line = SVRETAIN(SvValueGetString(lineVal));
    }

    if (nameVal) {
        callData->callerName = SVRETAIN(SvValueGetString(nameVal));
    }

    callData->number = SVRETAIN(SvValueGetString(numberVal));

    return callData;
}

SvLocal void
QBCallerIdManagerPushDataReceived(SvGenericObject self_,
                                  SvString type,
                                  SvGenericObject data_)
{
    QBCallerIdManager self = (QBCallerIdManager) self_;

    log_debug("Got push data");

    if (self->state == QBCallerIdManagerState_Disabled) {
        return;
    }

    SvHashTable data = (SvHashTable) data_;
    if (!SvObjectEquals((SvGenericObject) type, (SvGenericObject) SVSTRING(REMOTE_MESSAGE_TYPE))) {
        return;
    }

    SvValue cmdVal = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("command"));
    if (!cmdVal || !SvValueIsString(cmdVal)) {
        return;
    }

    SvString cmd = SvValueGetString(cmdVal);
    if (!SvObjectEquals((SvGenericObject) cmd, (SvGenericObject) SVSTRING(REMOTE_COMMAND))) {
        log_error("Unsupported command");
        return;
    }

    SvHashTable argsHT = (SvHashTable) SvHashTableFind(data, (SvGenericObject) SVSTRING("args"));
    if (!SvObjectIsInstanceOf((SvGenericObject) argsHT, SvHashTable_getType())) {
        log_error("Remote command %s - wrong format", REMOTE_MESSAGE_TYPE);
        return;
    }

    log_debug("Push data OK");

    SVTESTRELEASE(self->newCallData);
    self->newCallData = QBCallerIdManagerPrepareCallData(self, argsHT);
    if (self->newCallData) {
        QBCallerIdManagerUpdateHistory(self);
        QBCallerIdManagerUpdateCalls(self);
        QBCallerIdManagerNotifyListeners(self);
    }
}

SvLocal void
QBCallerIdManagerPushStatusChanged(SvObject self_,
                                   QBPushReceiverStatus status_)
{
}

SvLocal void
QBCallerIdManagerConfigChanged(SvGenericObject self_, const char *key, const char *value)
{
    QBCallerIdManager self = (QBCallerIdManager)self_;

    if (key && !strncmp(key, "CALLID_STATE", 12)) {
        QBCallerIdManagerUpdateStateFromConfig(self);
    } else if (key && !strncmp(key, "CALLID_TIMEOUT", 14)) {
        QBCallerIdManagerUpdateTimeoutFromConfig(self);
    }
}

// SvHTTPClientListener callbacks:
SvLocal void QBCallerIdManagerHeaderReceived(SvGenericObject o, SvHTTPRequest r, const char * h, const char * v) {}
SvLocal void QBCallerIdManagerDataChunkReceived(SvGenericObject self_, SvHTTPRequest req, off_t offset, const uint8_t * buffer, size_t len) {}
SvLocal void QBCallerIdManagerTransferCancelled(SvGenericObject self_, SvHTTPRequest r) {}

SvLocal void QBCallerIdManagerTransferFinished(SvGenericObject self_, SvHTTPRequest req, off_t total)
{
    QBCallerIdManager self = (QBCallerIdManager)self_;
    log_debug("Call dropped to voicemail.");

    SVRELEASE(self->voicemailHttpRequest);
    self->voicemailHttpRequest = NULL;
}

SvLocal void
QBCallerIdManagerErrorOccured(SvGenericObject self_, SvHTTPRequest req, int httpErrno)
{
    QBCallerIdManager self = (QBCallerIdManager)self_;
    log_error("Dropping to voicemail failed ! (http error=%d)", httpErrno);
    QBCallerIdManagerStopVoicemailRequest(self);
    QBCallerIdManagerDroppingToVoicemailFailed(self);
}

SvLocal SvType
QBCallerIdManager_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBCallerIdManagerDestroy
    };
    static SvType type = NULL;

    static const struct QBPushReceiverListener_t pushReceiverMethods = {
        .dataReceived = QBCallerIdManagerPushDataReceived,
        .statusChanged = QBCallerIdManagerPushStatusChanged
    };

    static const struct QBConfigListener_t configMethods = {
        .changed = QBCallerIdManagerConfigChanged
    };

    static const struct SvHTTPClientListener_ httpClientListenerMethods = {
        .headerReceived = QBCallerIdManagerHeaderReceived,
        .dataChunkReceived = QBCallerIdManagerDataChunkReceived,
        .transferFinished = QBCallerIdManagerTransferFinished,
        .errorOccured = QBCallerIdManagerErrorOccured,
        .transferCancelled = QBCallerIdManagerTransferCancelled
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBCallerIdManager",
                            sizeof(struct QBCallerIdManager_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPushReceiverListener_getInterface(), &pushReceiverMethods,
                            QBConfigListener_getInterface(), &configMethods,
                            SvHTTPClientListener_getInterface(), &httpClientListenerMethods,
                            NULL);
    }

    return type;
}

SvLocal void
QBCallerIdManagerClean(SvApplication app, void *ptr)
{
    QBCallerIdManager self = ptr;
    if (self->showEffectId > 0) {
        svAppCancelEffect(app, self->showEffectId, SV_EFFECT_FINISH);
    }
}

SvLocal void
QBCallerIdManagerFailureDialogStep(void *self_)
{
    QBCallerIdManager self = self_;

    SvFiberDeactivate(self->failureFiber);
    SvFiberEventDeactivate(self->failureTimer);

    QBDialogBreak(self->failureDialog);
    self->failureDialogId = UINT_MAX;
}

SvLocal void
QBCallerIdManagerDroppingToVoicemailFailed(QBCallerIdManager self)
{
    if (self->failureDialogId != UINT_MAX) {
        return;
    }

    svSettingsPushComponent("Dialog.settings");
    QBDialogParameters params = {
        .app        = self->appGlobals->res,
        .controller = self->appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0,
    };
    self->failureDialog = QBDialogGlobalNew(&params, SVSTRING("CallerIdFailurePopup"));
    self->failureDialogId = svWidgetGetId(self->failureDialog);
    svSettingsPopComponent();
    svSettingsPushComponent("CallerID.settings");
    QBDialogAddPanel(self->failureDialog, SVSTRING("content"), NULL, 1);
    QBDialogSetTitle(self->failureDialog, gettext(svSettingsGetString("FailurePopup", "title")));
    QBDialogAddLabel(self->failureDialog, SVSTRING("content"), gettext(svSettingsGetString("FailurePopup", "info")), SVSTRING("info"), 1);
    svSettingsPopComponent();
    QBDialogRun(self->failureDialog, NULL, NULL);
    SvFiberTimerActivateAfter(self->failureTimer, SvTimeConstruct(FAILURE_DIALOG_TIMEOUT, 0));
}

SvLocal void
QBCallerIdManagerCancelCallStatus(QBCallerIdManager self)
{
    SvFiberDeactivate(self->statusFiber);
    SvFiberEventDeactivate(self->statusTimer);
    self->bannerState = QBCallerIdManagerBannerState_Cancelled;
    QBCallerIdManagerUpdateCalls(self);
}

SvLocal int
QBCallerIdManagerDropToVoicemail(QBCallerIdManager self)
{
    SvErrorInfo errorInfo = NULL;
    SvHTTPRequest httpRequest = NULL;

    httpRequest = (SvHTTPRequest) SvTypeAllocateInstance(SvHTTPRequest_getType(), NULL);
    if (!SvHTTPRequestInit(httpRequest, NULL, SvHTTPRequestMethod_PUT, SvStringCString(SvURLString(self->currentCallData->voicemail)), &errorInfo)) {
        SVRELEASE(httpRequest);
        if (errorInfo) {
            SvErrorInfoWriteLogMessage(errorInfo);
            SvErrorInfoDestroy(errorInfo);
        }
        return -1;
    }

    self->voicemailHttpRequest = httpRequest;
    SvHTTPRequestSetTimeout(self->voicemailHttpRequest, VOICEMAIL_REQUEST_TIMEOUT, NULL);

    char* dataBuf;
    asprintf(&dataBuf, "{\n\t\"call_id\": \"%s\"\n}", SvStringCString(self->currentCallData->callID));

    SvHTTPRequestSetOutgoingData(self->voicemailHttpRequest, SVSTRING("application/json"),
                                 (unsigned char *) dataBuf,
                                 strlen(dataBuf), NULL);
    free(dataBuf);

    SvHTTPRequestSetUserAgent(self->voicemailHttpRequest, SVSTRING("SvHTTPClient"), NULL);
    SvHTTPRequestSetIOPolicy(self->voicemailHttpRequest, false, NULL);
    SvHTTPRequestSetListener(self->voicemailHttpRequest, (SvGenericObject)self, false, NULL);
    SvHTTPRequestSetSSLParams(self->voicemailHttpRequest, self->appGlobals->sslParams, NULL);
    SvHTTPRequestStart(self->voicemailHttpRequest, NULL);

    return 0;
}

SvLocal bool
QBCallerIdManagerInputEventHandler(SvWidget w, SvInputEvent ev)
{
    QBCallerIdManager self = (QBCallerIdManager)w->prv;

    switch (ev->ch) {
        case QBKEY_BACK:
            log_debug("QBKEY_BACK (EXIT) pressed");
            if (self->bannerState == QBCallerIdManagerBannerState_Shown) {
                QBCallerIdManagerCancelCallStatus(self);
            }
            break;
        case QBKEY_GREEN:
            log_debug("QBKEY_GREEN pressed");

            if (self->voicemailHttpRequest) {   // wait on current request status
                break;
            }

            if ( (self->bannerState == QBCallerIdManagerBannerState_Showing) ||
                 (self->bannerState == QBCallerIdManagerBannerState_Shown) ) {
                QBCallerIdManagerCancelCallStatus(self);
            }

            if (!self->currentCallData->voicemail) {
                log_error("No voicemail url !");
                QBCallerIdManagerDroppingToVoicemailFailed(self);
                break;
            }

            if (QBCallerIdManagerDropToVoicemail(self) < 0) {
                log_error("Dropping to voicemail failed !");
                QBCallerIdManagerDroppingToVoicemailFailed(self);
            }
            break;
        default:
            return false;
    }

    return true;
}

SvLocal void
QBCallerIdManagerFocusEventHandler(SvWidget w, SvFocusEvent ev)
{
    QBCallerIdManager self = (QBCallerIdManager) w->prv;

    if ( (self->bannerState == QBCallerIdManagerBannerState_Showing) ||
         (self->bannerState == QBCallerIdManagerBannerState_Shown) ) {
        QBCallerIdManagerHideOSD(self);
    }
}

SvLocal void
QBCallerIdManagerCreateStatus(QBCallerIdManager self)
{
    SvWindow bar = svSettingsWidgetCreate(self->appGlobals->res, "CallerIdStatus.Bar");
    bar->off_x = svSettingsGetInteger("CallerIdStatus.Bar", "xOffset", 0);
    bar->off_y = svSettingsGetInteger("CallerIdStatus.Bar", "yOffset", 0);

    SvWidget callingIconWidget = svSettingsWidgetCreate(self->appGlobals->res, "CallerIdStatus.Phone");

    self->callingMessage = svLabelNewFromSM(self->appGlobals->res, "CallerIdStatus.CallingMessage");
    self->lineMessage = svLabelNewFromSM(self->appGlobals->res, "CallerIdStatus.LineMessage");

    SvWidget hintMessage = svLabelNewFromSM(self->appGlobals->res, "CallerIdStatus.Hint");

    svSettingsWidgetAttach(bar, callingIconWidget, "CallerIdStatus.Phone", 1);
    svSettingsWidgetAttach(bar, self->callingMessage, "CallerIdStatus.CallingMessage", 1);
    svSettingsWidgetAttach(bar, self->lineMessage, "CallerIdStatus.LineMessage", 1);
    svSettingsWidgetAttach(bar, hintMessage, "CallerIdStatus.Hint", 1);

    self->effectDuration = svSettingsGetDouble("CallerIdStatus", "duration", 0.0);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, bar, SVSTRING("CallerIdStatus"));
    self->statusWindow = globalWindow;

    self->statusWindow->window->prv = self;
    svWidgetSetInputEventHandler(self->statusWindow->window, QBCallerIdManagerInputEventHandler);
    svWidgetSetFocusEventHandler(self->statusWindow->window, QBCallerIdManagerFocusEventHandler);
    svWidgetSetFocusable(self->statusWindow->window, true);
    self->statusWindow->focusable = true;

    self->statusWindow->window->clean = QBCallerIdManagerClean;
    svWidgetSetEffectEventHandler(self->statusWindow->window, QBCallerIdManagerEffectEventHandler);

    self->showEffectId = 0;
    self->bannerState = QBCallerIdManagerBannerState_Hidden;
}

SvLocal void
QBCallerIdManagerNotifyListeners(QBCallerIdManager self)
{
    SvIterator it = SvWeakListIterator(self->listeners);
    SvGenericObject listener = NULL;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBCallerIdManagerListener, listener, callsDataChanged);
    }
}

SvLocal void
QBCallerIdManagerUpdateHistory(QBCallerIdManager self)
{
    SvHashTable newCall = SvHashTableCreate(5, NULL);

    char dateTimeBuffer[32];
    QBStringFromDateTime(dateTimeBuffer, SvTimeGetCurrentTime(), QBDateTimeFormat_ISO8601, sizeof(dateTimeBuffer), true);

    SvString dateTimeString = SvStringCreate(dateTimeBuffer, NULL);
    SvValue dateTimeVal = SvValueCreateWithString(dateTimeString, NULL);
    SvHashTableInsert(newCall, (SvGenericObject) SVSTRING("dateTime"), (SvGenericObject) dateTimeVal);
    SVRELEASE(dateTimeVal);
    SVRELEASE(dateTimeString);

    if (self->newCallData->callerName) {
        SvValue callerNameVal = SvValueCreateWithString(self->newCallData->callerName, NULL);
        SvHashTableInsert(newCall, (SvGenericObject) SVSTRING("callerName"), (SvGenericObject) callerNameVal);
        SVRELEASE(callerNameVal);
    }

    SvValue numberVal = SvValueCreateWithString(self->newCallData->number, NULL);
    SvHashTableInsert(newCall, (SvGenericObject) SVSTRING("number"), (SvGenericObject) numberVal);
    SVRELEASE(numberVal);

    if (self->newCallData->line) {
        SvValue lineVal = SvValueCreateWithString(self->newCallData->line, NULL);
        SvHashTableInsert(newCall, (SvGenericObject) SVSTRING("line"), (SvGenericObject) lineVal);
        SVRELEASE(lineVal);
    }

    SvArrayInsertObjectAtIndex(self->callsHistory, (SvGenericObject) newCall, 0);
    SVRELEASE(newCall);

    if (SvArrayCount(self->callsHistory) > self->historySize) {
        SvArrayRemoveLastObject(self->callsHistory);
    }

    QBJSONSerializerAddJob(self->JSONserializer, (SvGenericObject) self->callsHistory, self->historyFilename);
}

SvLocal void
QBCallerIdManagerLoadHistory(QBCallerIdManager self)
{
    char *buffer = NULL;
    QBFileToBuffer(SvStringCString(self->historyFilename), &buffer);
    if (!buffer) {
        return;
    }

    SvArray callsData = (SvArray) SvJSONParseString(buffer, false, NULL);
    if (!callsData || !SvObjectIsInstanceOf((SvGenericObject) callsData, SvArray_getType())) {
        goto fini;
    }

    SvHashTable ht = (SvHashTable) SvArrayAt(callsData, 0);
    if (!ht || !SvObjectIsInstanceOf((SvGenericObject) ht, SvHashTable_getType())) {
        goto fini;
    }

    self->callsHistory = SVRETAIN(callsData);

fini:
    free(buffer);
    SVTESTRELEASE(callsData);
}

QBCallerIdManager
QBCallerIdManagerCreate(AppGlobals appGlobals, SvString historyFilename, QBJSONSerializer JSONserializer, int historySize)
{
    QBCallerIdManager self = (QBCallerIdManager) SvTypeAllocateInstance(QBCallerIdManager_getType(), NULL);

    if (!self) {
        log_error("failed");
        return NULL;
    }

    self->appGlobals = appGlobals;
    self->historyFilename = SVRETAIN(historyFilename);
    self->JSONserializer = SVRETAIN(JSONserializer);
    self->historySize = historySize;

    self->state = QBCallerIdManagerState_Disabled;
    QBCallerIdManagerUpdateStateFromConfig(self);

    self->bannerTimeout = CALLID_DEFAULT_TIMEOUT;
    QBCallerIdManagerUpdateTimeoutFromConfig(self);

    svSettingsPushComponent("CallerID.settings");
    QBCallerIdManagerCreateStatus(self);
    svSettingsPopComponent();

    self->failureDialogId = UINT_MAX;

    self->listeners = SvWeakListCreate(NULL);

    self->statusFiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBCallerIdManagerStatusStep", QBCallerIdManagerStatusStep, self);
    self->statusTimer = SvFiberTimerCreate(self->statusFiber);

    self->failureFiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBCallerIdManagerFailureDialogStep", QBCallerIdManagerFailureDialogStep, self);
    self->failureTimer = SvFiberTimerCreate(self->failureFiber);

    QBCallerIdManagerLoadHistory(self);
    if (!self->callsHistory) {
        self->callsHistory = SvArrayCreate(NULL);
    }

    return self;
}

void
QBCallerIdManagerStart(QBCallerIdManager self,
                       SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        log_error("failed. NULL self passed.");
        return;
    }

    if (self->started) {
        return;
    }

    if (!self->appGlobals->pushReceiver) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                          "pushReceiver doesn't exist");
        goto fini;
    }

    QBPushReceiverAddListener(self->appGlobals->pushReceiver, (SvGenericObject) self, SVSTRING(REMOTE_MESSAGE_TYPE), &error);
    QBConfigAddListener((SvGenericObject) self, "CALLID_STATE");
    QBConfigAddListener((SvGenericObject) self, "CALLID_TIMEOUT");

    self->started = true;

fini:
    SvErrorInfoPropagate(error, errorOut);
}

void
QBCallerIdManagerStop(QBCallerIdManager self,
                      SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        log_error("failed. NULL self passed.");
        return;
    }

    if (!self->started) {
        return;
    }

    if (!self->appGlobals->pushReceiver) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                          "pushReceiver doesn't exist");
        goto fini;
    }

    QBCallerIdManagerStopVoicemailRequest(self);

    QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, self->statusWindow);
    QBPushReceiverRemoveListener(self->appGlobals->pushReceiver, (SvGenericObject) self, &error);
    QBConfigRemoveListener((SvGenericObject) self, "CALLID_STATE");
    QBConfigRemoveListener((SvGenericObject) self, "CALLID_TIMEOUT");

    SvFiberEventDeactivate(self->statusTimer);
    SvFiberEventDeactivate(self->failureTimer);

    self->started = false;

fini:
    SvErrorInfoPropagate(error, errorOut);
}

void
QBCallerIdManagerAddListener(QBCallerIdManager self, SvGenericObject listener)
{
    if (!self || !listener) {
        log_error("failed. NULL passed.");
        return;
    }

    if (SvObjectIsImplementationOf(listener, QBCallerIdManagerListener_getInterface())) {
        SvWeakListPushBack(self->listeners, listener, NULL);
    } else {
        SvLogWarning("%s(): listener %p of type %s doesn't implement QBCallerIdManagerListener interface.", __func__, listener, listener ? SvObjectGetTypeName(listener) : "--");
    }
}

void
QBCallerIdManagerRemoveListener(QBCallerIdManager self, SvGenericObject listener)
{
    if (!self || !listener) {
        log_error("failed. NULL passed.");
        return;
    }

    if (SvObjectIsImplementationOf(listener, QBCallerIdManagerListener_getInterface())) {
        SvWeakListRemoveObject(self->listeners, listener);
    } else {
        SvLogWarning("%s(): listener %p of type %s doesn't implement QBCallerIdManagerListener interface.", __func__, listener, listener ? SvObjectGetTypeName(listener) : "--");
    }
}

SvArray
QBCallerIdManagerGetCallsHistory(QBCallerIdManager self)
{
    if (!self) {
        log_error("failed. NULL self passed.");
        return NULL;
    }

    return self->callsHistory;
}

int
QBCallerIdManagerGetHistorySize(QBCallerIdManager self)
{
    if (!self) {
        log_error("failed. NULL self passed.");
        return -1;
    }

    return self->historySize;
}

SvInterface
QBCallerIdManagerListener_getInterface(void)
{
    static SvInterface interface = NULL;
    if (!interface) {
        SvInterfaceCreateManaged("QBCallerIdManagerListener",
                                 sizeof(struct QBCallerIdManagerListener_t),
                                 NULL, &interface, NULL);
    }
    return interface;
}


