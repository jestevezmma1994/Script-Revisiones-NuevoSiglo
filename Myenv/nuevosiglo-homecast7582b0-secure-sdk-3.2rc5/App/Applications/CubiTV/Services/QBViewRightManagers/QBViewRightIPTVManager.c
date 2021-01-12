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

#include <Services/QBViewRightManagers/QBViewRightIPTVManager.h>
#include <QBViewRightIPTVListener.h>
#include <QBViewRightIPTVInterface.h>
#include <QBViewRightIPTVFingerprint.h>
#include <QBViewRightIPTVUserNotifications.h>
#include <QBViewRightIPTVUserMessage.h>
#include <QBViewRightIPTVParentalControlRequest.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBTimeDateMonitor.h>
#include <Services/QBParentalControl/QBParentalControlListener.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvDeque.h>
#include <SvFoundation/SvArray.h>
#include <CUIT/Core/types.h>
#include <fibers/c/fibers.h>
#include <time.h>
#include <libintl.h>
#include <SvCore/SvCommonDefs.h>
#include <player_events/decryption.h>
#include <QBCAS.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvEnv.h>
#include <settings.h>
#include <QBViewport.h>
#include <Widgets/authDialog.h>
#include <Utils/authenticators.h>
#include <QBWidgets/QBDialog.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <SWL/icon.h>
#include <main.h>
#include <Services/QBCASManager.h>
#include <QBPlatformHAL/QBPlatformOutput.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 2, "QBViewRightIPTVService", "");

#define log_debug(fmt, ...) if (env_log_level() >= 5) { SvLogNotice(COLBEG() "QBViewRightIPTVService :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); }
#define log_fun(fmt, ...)  if (env_log_level() >= 4) { SvLogNotice(COLBEG() "QBViewRightIPTVService :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); }
#define log_info(fmt, ...)  if (env_log_level() >= 3) { SvLogNotice(COLBEG() "QBViewRightIPTVService :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); }
#define log_state(fmt, ...)  if (env_log_level() >= 2) { SvLogNotice(COLBEG() "QBViewRightIPTVService :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); }
#define log_warn(fmt, ...) if (env_log_level() >= 1) { SvLogError(COLBEG() "QBViewRightIPTVService :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); }
#define log_error(fmt, ...) if (env_log_level() >= 1) { SvLogError(COLBEG() "QBViewRightIPTVService :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); }

typedef struct QBViewRightIPTVService_s *QBViewRightIPTVService;
typedef struct QBViewRightIPTVServiceFingerprint_s *QBViewRightIPTVServiceFingerprint;

struct QBViewRightIPTVServiceFingerprint_s {
    struct SvObject_ super_;
    SvWidget widget;
    SvFiberCmd cmd;
    QBViewRightIPTVService service;
};

struct QBViewRightIPTVService_s {
    struct SvObject_ super_;
    SvWeakReference viewRightIPTVRef;
    AppGlobals appGlobals;
    SvArray fingerprints;
    SvWidget pinDialog;
    SvWidget mmiDialog;
    QBGlobalWindow fingerPrintWindow;
    int fingerPrintCounter;
    SvDeque scheduledPopups;
    QBPCList ratingList;
    QBParentalControlState pcState;
    QBViewRightIPTVChannelStatus channelStatus;

    bool isProperTime;
    bool audioVideoBlocked;

    struct {
        SvWidget dialog;
        SvFiberCmd dialogTimer;
    } userMessage;
    QBViewRightIPTVParentalControlRequest request;
    int settingsCtx;
};

SvLocal void QBViewRightIPTVServiceFingerprintDestroy(void *self_);
SvLocal void QBViewRightIPTVServiceDestroy(void *self_);
SvLocal void QBViewRightIPTVServiceStart(SvObject);
SvLocal void QBViewRightIPTVServiceStop(SvObject);
SvLocal void QBViewRightIPTVServiceVisabilityChanged(SvObject, bool);
SvLocal void QBViewRightIPTVServiceContentChanged(SvObject);
SvLocal void QBViewRightIPTVServicePlaybackEvent(SvObject, SvString name, void *arg);
SvLocal void QBViewRightIPTVServiceRatingChanged(SvObject, QBPCList);
SvLocal void QBViewRightIPTVServicePCStateChanged(SvObject, QBParentalControlState state);
SvLocal void QBViewRightIPTVServicePCLockedTimeTaskAdded(SvObject self_, QBParentalControlLockedTimeTask task);
SvLocal void QBViewRightIPTVServicePCLockedTimeTaskRemoved(SvObject self_, QBParentalControlLockedTimeTask task);
SvLocal void QBViewRightIPTVServiceRemovePopups(QBViewRightIPTVService self);
SvLocal void QBViewRightIPTVServiceRemoveFingerprints(QBViewRightIPTVService self);
SvLocal void QBViewRightIPTVServiceRemoveFingerprint(QBViewRightIPTVService self, QBViewRightIPTVServiceFingerprint fingerprint);
SvLocal void QBViewRightIPTVServiceHandleUserMessage(SvObject self_, QBViewRightIPTVUserMessage userMessage);
SvLocal void QBViewRightIPTVServiceHandleFingerprint(SvObject self_, QBViewRightIPTVFingerprint fingerprint);
SvLocal void QBViewRightIPTVServiceHandleCopyControl(SvObject self_, QBViewRightIPTVCopyControl copyControl);
SvLocal void QBViewRightIPTVServiceHandleChannelStatus(SvObject self_, QBViewRightIPTVChannelStatusNotification channelStatus);
SvLocal void QBViewRightIPTVServiceParentalControlRequest(SvObject self_, QBViewRightIPTVParentalControlRequest request);
SvLocal void QBViewRightIPTVServiceParentalStateChanged(SvObject self_, QBViewRightIPTVState state, int errorCode);
SvLocal void QBViewRightIPTVServiceHandleMessage(QBViewRightIPTVService self, SvGenericObject obj);
SvLocal void QBViewRightIPTVServiceGotActualTimeAndDate(SvObject self_, bool firstTime, QBTimeDateUpdateSource source);

SvLocal SvType QBViewRightIPTVServiceFingerprint_getType(void)
{
    log_fun();
    static SvType type = NULL;

    static const struct SvObjectVTable_ virtualMethods = {
        .destroy = QBViewRightIPTVServiceFingerprintDestroy,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBViewRightIPTVServiceFingerprint", sizeof(struct QBViewRightIPTVServiceFingerprint_s),
                            SvObject_getType(), &type,
                            SvObject_getType(), &virtualMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBViewRightIPTVServiceFingerprintDone(void *arg, int is_done)
{
    log_fun();
    QBViewRightIPTVServiceFingerprint self = (QBViewRightIPTVServiceFingerprint) arg;

    self->cmd = NULL;
    if (is_done) {
        // Method will remove fingerprintHandle from array which might be the only owner of it.
        // In that case fingerprintHandle (self) will be destroyed.
        QBViewRightIPTVServiceRemoveFingerprint(self->service, self);
    }
}

SvLocal QBViewRightIPTVServiceFingerprint QBViewRightIPTVServiceFingerprintCreate(QBViewRightIPTVService service, SvWidget widget, int durationMs)
{
    log_fun();
    QBViewRightIPTVServiceFingerprint self = (QBViewRightIPTVServiceFingerprint) SvTypeAllocateInstance(QBViewRightIPTVServiceFingerprint_getType(), NULL);
    self->service = SVRETAIN(service);
    self->widget = widget;
    self->cmd = SvFiberCmdCreate(durationMs, self, QBViewRightIPTVServiceFingerprintDone, 1);

    return self;
}

SvLocal void QBViewRightIPTVServiceFingerprintDestroy(void *self_)
{
    log_fun();
    QBViewRightIPTVServiceFingerprint self = (QBViewRightIPTVServiceFingerprint) self_;
    SVRELEASE(self->service);

    svWidgetDetach(self->widget);
    svWidgetDestroy(self->widget);

    if (self->cmd) {
        SvFiberCmdDestroy(self->cmd);
    }
}

SvLocal SvType QBViewRightIPTVService_getType(void)
{
    static SvType type = NULL;

    static const struct QBParentalControlListener_s parentalControlMethods = {
        .ratingChanged         = QBViewRightIPTVServiceRatingChanged,
        .stateChanged          = QBViewRightIPTVServicePCStateChanged,
        .lockedTimeTaskAdded   = QBViewRightIPTVServicePCLockedTimeTaskAdded,
        .lockedTimeTaskRemoved = QBViewRightIPTVServicePCLockedTimeTaskRemoved
    };

    static const struct QBCASPopupManager_ popupMethods = {
        .playbackEvent      = QBViewRightIPTVServicePlaybackEvent,
        .contentChanged     = QBViewRightIPTVServiceContentChanged,
        .visibilityChanged  = QBViewRightIPTVServiceVisabilityChanged,
        .start              = QBViewRightIPTVServiceStart,
        .stop               = QBViewRightIPTVServiceStop
    };

    static const struct QBViewRightIPTVListener_s viewRightIPTVMethods = {
        .stateChanged           = QBViewRightIPTVServiceParentalStateChanged,
        .userMessage            = QBViewRightIPTVServiceHandleUserMessage,
        .fingerprint            = QBViewRightIPTVServiceHandleFingerprint,
        .copyControl            = QBViewRightIPTVServiceHandleCopyControl,
        .parentalControlRequest = QBViewRightIPTVServiceParentalControlRequest,
        .channelStatus          = QBViewRightIPTVServiceHandleChannelStatus,
    };

    static const struct QBTimeDateMonitorListener_ timeDateMonitorListenerMethods = {
        .systemTimeSet = QBViewRightIPTVServiceGotActualTimeAndDate
    };

    static const struct SvObjectVTable_ virtualMethods = {
        .destroy = QBViewRightIPTVServiceDestroy,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBViewRightIPTVService", sizeof(struct QBViewRightIPTVService_s),
                            SvObject_getType(), &type,
                            SvObject_getType(), &virtualMethods,
                            QBCASPopupManager_getInterface(), &popupMethods,
                            QBParentalControlListener_getInterface(), &parentalControlMethods,
                            QBViewRightIPTVListener_getInterface(), &viewRightIPTVMethods,
                            QBTimeDateMonitorListener_getInterface(), &timeDateMonitorListenerMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBViewRightIPTVServiceDestroy(void *self_)
{
    log_fun();
    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;
    SVRELEASE(self->fingerprints);
    SVRELEASE(self->viewRightIPTVRef);
    SVRELEASE(self->scheduledPopups);
    SVRELEASE(self->fingerPrintWindow);
    assert(self->pinDialog == NULL);
    assert(self->mmiDialog == NULL);
    assert(self->userMessage.dialog == NULL);
}

SvObject QBViewRightIPTVServiceCreate(AppGlobals appGlobals)
{
    log_fun();
    QBViewRightIPTVService self = (QBViewRightIPTVService) SvTypeAllocateInstance(QBViewRightIPTVService_getType(), NULL);
    self->appGlobals = appGlobals;
    self->fingerprints = SvArrayCreate(NULL);
    self->scheduledPopups = SvDequeCreate(NULL);

    svSettingsPushComponent("ViewRightIPTVService.settings");
    self->settingsCtx = svSettingsSaveContext();
    svSettingsPopComponent();

    SvWidget window = svWidgetCreate(appGlobals->res, appGlobals->res->width, appGlobals->res->height);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, window, SVSTRING("ViewRightIPTVFingerprint"));

    self->fingerPrintWindow = globalWindow;

    return (SvObject) self;
}

SvLocal void QBViewRightIPTVServiceStart(SvObject self_)
{
    log_fun();
    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;
    SvObject viewRightIPTV = QBCASGetInstance();

    self->viewRightIPTVRef = SvWeakReferenceCreateWithObject(viewRightIPTV, NULL);
    QBParentalControlAddListener(self->appGlobals->pc, (SvObject) self);

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorAddListener(timeDateMonitor, (SvObject) self, NULL);

    if (!SvObjectIsImplementationOf(viewRightIPTV, QBViewRightIPTVInterface_getInterface())) {
        log_error("This CAS is not ViewRightIPTV!");
        abort();
    }

    SvInvokeInterface(QBViewRightIPTVInterface, viewRightIPTV, addListener, (SvObject) self);
}

SvLocal void QBViewRightIPTVServiceRemoveFingerprints(QBViewRightIPTVService self)
{
    log_fun();

    QBViewRightIPTVServiceFingerprint fingerprintHandle;
    while ((fingerprintHandle = (QBViewRightIPTVServiceFingerprint) SvArrayGetLastObject(self->fingerprints))) {
        QBViewRightIPTVServiceRemoveFingerprint(self, fingerprintHandle);
    }
}

SvLocal void QBViewRightIPTVServiceStop(SvObject self_)
{
    log_fun();
    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;
    SvDequeRemoveAllObjects(self->scheduledPopups);
    QBViewRightIPTVServiceRemovePopups(self);

    SvObject viewRightIPTV = SvWeakReferenceTakeReferredObject(self->viewRightIPTVRef);
    SvInvokeInterface(QBViewRightIPTVInterface, viewRightIPTV, removeListener, (SvObject) self);
    SVRELEASE(viewRightIPTV);

    QBParentalControlRemoveListener(self->appGlobals->pc, (SvObject) self);

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorRemoveListener(timeDateMonitor, (SvObject) self, NULL);
}

SvLocal void QBViewRightIPTVServiceGotActualTimeAndDate(SvObject self_, bool firstTime, QBTimeDateUpdateSource source)
{
    log_fun();

    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;

    if (self->isProperTime) {
        return;
    }

    self->isProperTime = true;

    SvObject viewRightIPTV = SvWeakReferenceTakeReferredObject(self->viewRightIPTVRef);
    SvInvokeInterface(QBViewRightIPTVInterface, viewRightIPTV, gotActualTimeAndDate);
    SVRELEASE(viewRightIPTV);
}

SvLocal void QBViewRightIPTVServiceUpdateRatingLevel(QBViewRightIPTVService self, QBParentalControlState state, QBPCList ratingList)
{
    int rating = 255;

    if (state == QBParentalControlState_On && ratingList) {
        SvErrorInfo error = NULL;
        SvArray vrIPTVratings = QBPCListCreateLockedRatingsListByStandard(ratingList, QBPCRatingStandard_ViewRightIPTV, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            return;
        }
        if (vrIPTVratings) {
            // because of the global type of ordering the ratings, the lowest rating is the last in the ratingLevel
            QBPCRating ratingLevel = (QBPCRating) SvArrayGetLastObject(vrIPTVratings);
            rating = QBPCRatingGetRatingId(ratingLevel, &error);
            if (error) {
                SVRELEASE(vrIPTVratings);
                SvErrorInfoWriteLogMessage(error);
                SvErrorInfoDestroy(error);
                return;
            }
            SVRELEASE(vrIPTVratings);
        }
    }

    QBViewRightIPTV viewRightIPTV = (QBViewRightIPTV) SvWeakReferenceTakeReferredObject(self->viewRightIPTVRef);
    SvInvokeInterface(QBViewRightIPTVInterface, viewRightIPTV, parentalControlSetRatingLevel, rating);

    SVRELEASE(viewRightIPTV);
}

SvLocal void QBViewRightIPTVServicePCStateChanged(SvObject self_, QBParentalControlState state)
{
    log_fun("State = %d", state);
    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;
    self->pcState = state;
    QBViewRightIPTVServiceUpdateRatingLevel(self, self->pcState, self->ratingList);
}

SvLocal void QBViewRightIPTVServiceRatingChanged(SvObject self_, QBPCList ratingList)
{
    log_fun();

    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;
    self->ratingList = ratingList;
    QBViewRightIPTVServiceUpdateRatingLevel(self, self->pcState, self->ratingList);
}

SvLocal void QBViewRightIPTVServicePCLockedTimeTaskAdded(SvObject self_, QBParentalControlLockedTimeTask task)
{
}

SvLocal void QBViewRightIPTVServicePCLockedTimeTaskRemoved(SvObject self_, QBParentalControlLockedTimeTask task)
{
}

SvLocal int QBViewRightIPTVServiceFingerprintGetHorizontalPosition(QBViewRightIPTVFingerprintPositionHorizontal horizontal)
{
    switch (horizontal) {
        case QBViewRightIPTVFingerprintPositionHorizontal_Left:
            return svSettingsGetInteger("QBViewRightIPTVFingerprint", "leftPos", 0);
        case QBViewRightIPTVFingerprintPositionHorizontal_Centered:
            return svSettingsGetInteger("QBViewRightIPTVFingerprint", "centerHorizontalPos", 0);
        case QBViewRightIPTVFingerprintPositionHorizontal_Right:
            return svSettingsGetInteger("QBViewRightIPTVFingerprint", "rightPos", 0);
        default:
            return 0;
    }
}

SvLocal int QBViewRightIPTVServiceFingerprintGetVerticalPosition(QBViewRightIPTVFingerprintPositionVertical vertical)
{
    switch (vertical) {
        case QBViewRightIPTVFingerprintPositionVeritical_Top:
            return svSettingsGetInteger("QBViewRightIPTVFingerprint", "topPos", 0);
        case QBViewRightIPTVFingerprintPositionVeritical_Centered:
            return svSettingsGetInteger("QBViewRightIPTVFingerprint", "centerVerticalPos", 0);
        case QBViewRightIPTVFingerprintPositionVeritical_Bottom:
            return svSettingsGetInteger("QBViewRightIPTVFingerprint", "bottomPos", 0);
        default:
            return 0;
    }
}

SvLocal void QBViewRightIPTVServiceHandleMessage(QBViewRightIPTVService self, SvGenericObject obj)
{
    if (SvObjectIsInstanceOf(obj, QBViewRightIPTVParentalControlRequest_getType())) {
        log_state("Got parental control request");
        QBViewRightIPTVServiceParentalControlRequest((SvObject) self, (QBViewRightIPTVParentalControlRequest) obj);
    } else if (SvObjectIsInstanceOf(obj, QBViewRightIPTVChannelStatusNotification_getType())) {
        log_state("Got channel status");
        QBViewRightIPTVServiceHandleChannelStatus((SvObject) self, (QBViewRightIPTVChannelStatusNotification) obj);
    } else if (SvObjectIsInstanceOf(obj, QBViewRightIPTVUserMessage_getType())) {
        log_state("Got user message");
        QBViewRightIPTVServiceHandleUserMessage((SvObject) self, (QBViewRightIPTVUserMessage) obj);
    }
}

SvLocal void QBViewRightIPTVServiceShowFingerprint(QBViewRightIPTVService self, QBViewRightIPTVFingerprint fingerprint)
{
    log_fun();

    QBViewRightIPTVFingerprintType type = QBViewRightIPTVFingerprintGetType(fingerprint);
    unsigned int duration = QBViewRightIPTVFingerprintGetDuration(fingerprint);

    SvString caption = NULL;
    SvWidget logo = NULL;

    switch (type) {
        case QBViewRightIPTVFingerprintType_Test:
            caption = SvStringCreateWithFormat("%s", "Test is test");
            break;
        case QBViewRightIPTVFingerprintType_Logo:;
            svSettingsRestoreContext(self->settingsCtx);
            logo = svIconNew(self->appGlobals->res, "QBViewRightIPTVFingerprint.Logo");
            svSettingsPopComponent();
            break;
        case QBViewRightIPTVFingerprintType_Version:;
            caption = SVRETAIN(QBViewRightIPTVFingerprintGetParsedLibraryVersion(fingerprint));
            break;
        case QBViewRightIPTVFingerprintType_UniqueDeviceIdentifier:;
            caption = SVRETAIN(QBViewRightIPTVFingerprintGetUniqueId(fingerprint));
            break;
        case QBViewRightIPTVFingerprintType_LocalTime:;
            caption = SVRETAIN(QBViewRightIPTVFingerprintGetLocalTime(fingerprint));
            break;
        default:
            log_error("Unsupported fingerprint type");
            return;
    }
    svSettingsRestoreContext(self->settingsCtx);

    SvWidget dialog = NULL;
    if (caption) {
        dialog = QBAsyncLabelNew(self->appGlobals->res, "QBViewRightIPTVFingerprint.Label", self->appGlobals->textRenderer);
        QBAsyncLabelSetText(dialog, caption);
        SVRELEASE(caption);
    } else if (logo) {
        dialog = logo;
    } else {
        log_error("There is nothing (no caption or logo) to be shown at the fingerprint.");
        return;
    }

    QBViewRightIPTVFingerprintPositionHorizontal horizontal;
    QBViewRightIPTVFingerprintPositionVertical vertical;

    QBViewRightIPTVFingerprintGetPosition(fingerprint, &horizontal, &vertical);

    dialog->off_x = QBViewRightIPTVServiceFingerprintGetHorizontalPosition(horizontal);
    dialog->off_y = QBViewRightIPTVServiceFingerprintGetVerticalPosition(vertical);
    int quadrantVerticalSize = svSettingsGetInteger("QBViewRightIPTVFingerprint", "quadrantVerSize", 50);
    int quadrantHorizontalSize = svSettingsGetInteger("QBViewRightIPTVFingerprint", "quadrantHorSize", 50);

    dialog->off_x += (rand() % quadrantHorizontalSize) - quadrantHorizontalSize / 2 - dialog->width / 2;
    dialog->off_y += (rand() % quadrantVerticalSize) - quadrantVerticalSize / 2 - dialog->height / 2;
    svSettingsPopComponent();

    svWidgetAttach(self->fingerPrintWindow->window, dialog, dialog->off_x, dialog->off_y, 0);

    QBViewRightIPTVServiceFingerprint fingerprintHandle = QBViewRightIPTVServiceFingerprintCreate(self, dialog, duration * 1000);
    SvArrayAddObject(self->fingerprints, (SvObject) fingerprintHandle);
    SVRELEASE(fingerprintHandle);

    if (!self->fingerPrintCounter) {
        QBApplicationControllerAddGlobalWindow(self->appGlobals->controller, self->fingerPrintWindow);
    }

    self->fingerPrintCounter++;
}

SvLocal void QBViewRightIPTVServiceParentalControlPINCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    log_fun();
    QBViewRightIPTVService self = self_;
    QBViewRightIPTVParentalControlRequest request = self->request;
    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        QBViewRightIPTVParentalControlRequestSetAccessAllowed(request, true);
        SvObject viewRightIPTV = SvWeakReferenceTakeReferredObject(self->viewRightIPTVRef);
        SvInvokeInterface(QBViewRightIPTVInterface, viewRightIPTV, parentalControlRequestDone, request);
        SVRELEASE(viewRightIPTV);
        SVRELEASE(self->request);
        self->request = NULL;
    } else {
        QBViewRightIPTVParentalControlRequestSetAccessAllowed(request, false);
    }
    self->pinDialog = NULL;

    if (SvDequeGetCount(self->scheduledPopups)) {
        SvObject message = SvDequeTakeBack(self->scheduledPopups);
        QBViewRightIPTVServiceHandleMessage(self, message);
        SVRELEASE(message);
    }
}

SvLocal void QBViewRightIPTVServiceUserMessageTimerCallback(void* arg, int is_done)
{
    QBViewRightIPTVService self = arg;
    if (is_done) {
        self->userMessage.dialogTimer = NULL;
        QBDialogBreak(self->userMessage.dialog);
    }
}

SvLocal void QBViewRightIPTVServiceUserMessageCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    log_fun();

    QBViewRightIPTVService self = self_;
    self->userMessage.dialog = NULL;
    if (self->userMessage.dialogTimer)
        SvFiberCmdDestroy(self->userMessage.dialogTimer);

    self->userMessage.dialogTimer = NULL;

    if (SvDequeGetCount(self->scheduledPopups)) {
        SvObject message = SvDequeTakeBack(self->scheduledPopups);
        QBViewRightIPTVServiceHandleMessage(self, message);
        SVRELEASE(message);
    }
}

SvLocal void QBViewRightIPTVServiceMMINotificationCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    log_fun();
    QBViewRightIPTVService self = self_;
    self->mmiDialog = NULL;
}

SvLocal void QBViewRightIPTVServiceShowMMINotification(QBViewRightIPTVService self, SvString caption)
{
    log_fun();

    svSettingsRestoreContext(self->settingsCtx);
    QBDialogParameters params = {
        .app        = self->appGlobals->res,
        .controller = self->appGlobals->controller,
        .widgetName = "QBViewRightIPTVMMI",
        .ownerId    = 0,
    };
    SvWidget dialog = QBDialogGlobalNew(&params, SVSTRING("ViewRightIPTVMMI"));

    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    if (caption) {
        QBDialogAddLabel(dialog, SVSTRING("content"), SvStringCString(caption), SVSTRING("caption"), 0);
    }

    if (self->mmiDialog)
        QBDialogBreak(self->mmiDialog);
    self->mmiDialog = dialog;
    svSettingsPopComponent();

    QBDialogRun(self->mmiDialog, self, QBViewRightIPTVServiceMMINotificationCallback);
}

SvLocal void QBViewRightIPTVServiceBlockAudioVideo(QBViewRightIPTVService self, bool blockAudioVideo)
{
    log_fun();

    if(self->audioVideoBlocked == blockAudioVideo) {
        return;
    }

    self->audioVideoBlocked = blockAudioVideo;

    QBViewportBlockAudio(QBViewportGet(), self->audioVideoBlocked);
    QBViewportBlockVideo(QBViewportGet(), SVSTRING("QBViewRightIPTVService"), QBViewportBlockVideoReason_unknown, self->audioVideoBlocked);
}

SvLocal void QBViewRightIPTVServiceHandleChannelStatus(SvObject self_, QBViewRightIPTVChannelStatusNotification channelStatus)
{
    log_fun();

    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;

    QBViewRightIPTVChannelStatus status = QBViewRightIPTVChannelStatusNotificationGetStatus(channelStatus);

    SvString caption = NULL;

    self->channelStatus = status;

    if (self->mmiDialog) {
        QBDialogBreak(self->mmiDialog);
    }

    if (status != QBViewRightIPTVChannelStatus_PinRequired && self->pinDialog) {
        QBDialogBreak(self->pinDialog);
    }

    svSettingsRestoreContext(self->settingsCtx);

    bool blockAudioVideo = true;
    switch (status) {
        case QBViewRightIPTVChannelStatus_OK:
            blockAudioVideo = false;
            break;
        case QBViewRightIPTVChannelStatus_KeyNotAvailable:
            caption = SvStringCreateWithFormat("%s", svSettingsGetString("QBViewRightIPTVMMI", "keyNotAvailableStr"));
            break;
        case QBViewRightIPTVChannelStatus_PinRequired:
            break;
        case QBViewRightIPTVChannelStatus_Blackout:
            caption = SvStringCreateWithFormat("%s", svSettingsGetString("QBViewRightIPTVMMI", "blackoutStr"));
            break;
        case QBViewRightIPTVChannelStatus_Other:
            caption = SvStringCreateWithFormat("%s", svSettingsGetString("QBViewRightIPTVMMI", "otherStr"));
            break;
        case QBViewRightIPTVChannelStatus_Unknown:
            caption = SvStringCreateWithFormat("%s", svSettingsGetString("QBViewRightIPTVMMI", "unknownStr"));
            break;
        default:
            log_error("Not handled channel status = %d", status);
            break;
    }

    QBViewRightIPTVServiceBlockAudioVideo(self, blockAudioVideo);

    svSettingsPopComponent();

    if (caption) {
        QBViewRightIPTVServiceShowMMINotification(self, caption);
        SVRELEASE(caption);
    }
}

SvLocal void QBViewRightIPTVServiceParentalStateChanged(SvObject self_, QBViewRightIPTVState state, int errorCode)
{
}

SvLocal void QBViewRightIPTVServiceParentalControlRequest(SvObject self_, QBViewRightIPTVParentalControlRequest request)
{
    log_fun();

    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;

    if (self->pinDialog || self->userMessage.dialog) {
        return;
    }

    SvGenericObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, SVSTRING("PC"));

    svSettingsPushComponent("ParentalControl.settings");
    self->pinDialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"), gettext("Parental control PIN required (CA)"), false, SVSTRING("ViewRightIPTVManagerPINPopup"), NULL);
    self->request = SVRETAIN(request);
    svSettingsPopComponent();

    QBDialogRun(self->pinDialog, self, QBViewRightIPTVServiceParentalControlPINCallback);
}

SvLocal void QBViewRightIPTVServiceHandleUserMessage(SvObject self_, QBViewRightIPTVUserMessage userMessage)
{
    log_fun();

    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;

    if (self->pinDialog || self->userMessage.dialog) {
        SvDequePushFront(self->scheduledPopups, (SvObject) userMessage, NULL);
        return;
    }

    svSettingsRestoreContext(self->settingsCtx);

    QBDialogParameters params = {
        .app        = self->appGlobals->res,
        .controller = self->appGlobals->controller,
        .widgetName = "QBViewRightIPTVUserMessage",
        .ownerId    = 0,
    };
    SvWidget dialog = QBDialogGlobalNew(&params, SVSTRING("ViewRightIPTVUserMessage"));
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    SvString caption = QBViewRightIPTVUserMessageGetText(userMessage);
    QBDialogAddLabel(dialog, SVSTRING("content"), SvStringCString(caption), SVSTRING("caption"), 0);

    QBViewRightIPTVUserMessageType type = QBViewRightIPTVUserMessageGetType(userMessage);

    if (type != QBViewRightIPTVUserMessageType_Fixed) {
        QBDialogAddButton(dialog, SVSTRING("button"), gettext("OK"), 1);
    }

    svSettingsPopComponent();
    SvTime duration = QBViewRightIPTVUserMessageGetDuration(userMessage);

    if (SvTimeCmp(duration, SvTimeConstruct(0, 0))) {
        self->userMessage.dialogTimer = SvFiberCmdCreate(SvTimeGetSeconds(duration) * 1000, self, QBViewRightIPTVServiceUserMessageTimerCallback, 0);
    } else {
        self->userMessage.dialogTimer = NULL;
    }

    self->userMessage.dialog = dialog;
    QBDialogRun(self->userMessage.dialog, self, QBViewRightIPTVServiceUserMessageCallback);
}

SvLocal void QBViewRightIPTVServiceRemoveFingerprint(QBViewRightIPTVService self, QBViewRightIPTVServiceFingerprint fingerprintHandle)
{
    log_fun();

    self->fingerPrintCounter--;
    if (!self->fingerPrintCounter) {
        QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, self->fingerPrintWindow);
    }

    SvArrayRemoveObject(self->fingerprints, (SvObject) fingerprintHandle);
}

SvLocal void QBViewRightIPTVServiceHandleFingerprint(SvObject self_, QBViewRightIPTVFingerprint fingerprint)
{
    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;

    QBViewRightIPTVFingerprintPositionHorizontal horizontalPos;
    QBViewRightIPTVFingerprintPositionVertical verticalPos;
    QBViewRightIPTVFingerprintGetPosition(fingerprint, &horizontalPos, &verticalPos);
    QBViewRightIPTVFingerprintType type = QBViewRightIPTVFingerprintGetType(fingerprint);
    SvString version = QBViewRightIPTVFingerprintGetParsedLibraryVersion(fingerprint);
    SvString uniqueId = QBViewRightIPTVFingerprintGetUniqueId(fingerprint);
    SvString localTime = QBViewRightIPTVFingerprintGetLocalTime(fingerprint);

    log_state("Fingerprint:");
    log_state("   - duration %d sec", QBViewRightIPTVFingerprintGetDuration(fingerprint));
    log_state("   - horizontal position : %s", horizontalPos == QBViewRightIPTVFingerprintPositionHorizontal_Left ? "Left" : horizontalPos == QBViewRightIPTVFingerprintPositionHorizontal_Right ? "Right" : "Centered");
    log_state("   - vertical position : %s", verticalPos == QBViewRightIPTVFingerprintPositionVeritical_Bottom ? "Bottom" : verticalPos == QBViewRightIPTVFingerprintPositionVeritical_Top ? "Top" : "Centered");
    log_state("   - type : %s",
              type == QBViewRightIPTVFingerprintType_Test ? "Test" : type == QBViewRightIPTVFingerprintType_Logo ? "Logo" : type == QBViewRightIPTVFingerprintType_UniqueDeviceIdentifier ? "Unique device identifier" : type == QBViewRightIPTVFingerprintType_LocalTime ? "Local time" : type == QBViewRightIPTVFingerprintType_Version ? "Version" : type == QBViewRightIPTVFingerprintType_Unknown ? "Unknown" : "Unknown");
    if (version)
        log_state("   - version : %s", SvStringCString(version));
    if (uniqueId)
        log_state("   - uniqueId : %s", SvStringCString(uniqueId));
    if (localTime) {
        log_state("   - localTime : %s", SvStringCString(localTime));
    }

    if (QBViewRightIPTVFingerprintGetDuration(fingerprint) == 0) {
        log_state("Special fingerprint with duration equals to 0.");
        log_state("Clear fingerprints");

        QBViewRightIPTVServiceRemoveFingerprints(self);
    } else {
        QBViewRightIPTVServiceShowFingerprint(self, fingerprint);
    }
}

SvLocal QBContentProtectionCGMSMode QBViewRightIPTVServiceMap2DataformatCGMS(QBViewRightIPTVCopyControlEMI emi)
{
    switch (emi) {
        case QBViewRightIPTVCopyControlEMI_CopyingPermited:
            return QBContentProtectionCGMSMode_copyFreely;
        case QBViewRightIPTVCopyControlEMI_NoFurtherCopying:
            return QBContentProtectionCGMSMode_copyNoMore;
        case QBViewRightIPTVCopyControlEMI_OneGenerationCopyPermited:
            return QBContentProtectionCGMSMode_copyOnce;
        case QBViewRightIPTVCopyControlEMI_CopyingProhibited:
            return QBContentProtectionCGMSMode_copyNever;
        default:
            abort();
            break;
    }
}

SvLocal QBContentProtectionMacrovisionMode QBViewRightIPTVServiceMap2DataformatMacrovision(QBViewRightIPTVCopyControlAPS aps)
{
    switch (aps) {
        case QBViewRightIPTVCopyControlAPS_Disabled:
            return QBContentProtectionMacrovisionMode_disabled;
        case QBViewRightIPTVCopyControlAPS_AGConSplitBurstOff:
            return QBContentProtectionMacrovisionMode_agcOnly;
        case QBViewRightIPTVCopyControlAPS_AGConSplitBurst2:
            return QBContentProtectionMacrovisionMode_agc2Lines;
        case QBViewRightIPTVCopyControlAPS_AGConSplitBurst4:
            return QBContentProtectionMacrovisionMode_agc4Lines;
        default:
            abort();
            break;
    }
}

SvLocal bool QBViewRightIPTVServiceIsDigitalOutput(const QBVideoOutputConfig *cfg)
{
    if (cfg->type == QBOutputType_unknown || cfg->type == QBOutputType_other) {
        log_error("Unknown output type %u!", cfg->type);
    }
    return cfg->type == QBOutputType_HDMI || cfg->type == QBOutputType_DVI;
}

SvLocal inline bool QBViewRightIPTVServiceIsAnalogOutput(const QBVideoOutputConfig *cfg)
{
    return !QBViewRightIPTVServiceIsDigitalOutput(cfg);
}

SvLocal void QBViewRightIPTVServiceHandleDigitalOutput(unsigned outputIndex, const QBVideoOutputConfig *cfg, bool enableHDCP)
{
    // handle HDCP
    if (cfg->capabilities & QBOutputCapability_HDCP) {
        QBContentProtection HDCPlevel = enableHDCP ? QBContentProtection_required : QBContentProtection_disable;

        bool HDCPstatus = !QBPlatformSetHDCP(outputIndex, HDCPlevel, QBPlatformOutputPriority_content);
        if (!HDCPstatus) {
            log_warn("Failed to setup HDCP on output %u, will try to mute it", outputIndex);
        }
        // mute only if failed to enable HDCP, unmute always
        QBPlatformOutputMute(outputIndex, enableHDCP && !HDCPstatus, QBPlatformOutputPriority_content);
    } else {
        // HDCP not supported, mute ouptut if HDCP on output was to be enabled, unmute otherwise
        QBPlatformOutputMute(outputIndex, enableHDCP, QBPlatformOutputPriority_content);
    }
}

SvLocal void QBViewRightIPTVServiceHandleAnalogOutput(unsigned outputIndex, const QBVideoOutputConfig *cfg,
                                                      QBViewRightIPTVCopyControlEMI EMImode, QBViewRightIPTVCopyControlAPS APSmode,
                                                      bool DOT)
{
    // handle DOT
    if (DOT) {
        if (QBPlatformOutputMute(outputIndex, true, QBPlatformOutputPriority_content) < 0) {
            log_error("Failed to mute analog output %u!", outputIndex);
        } else {
            // output muted, CGSM/Macrovision settings are irrelevant
            return;
        }
    }

    bool mute = false;

    // handle CGMS
    if (cfg->capabilities & QBOutputCapability_CGMS) {

        bool CGMSstatus = !QBPlatformSetCGMS(outputIndex, QBContentProtection_required, QBViewRightIPTVServiceMap2DataformatCGMS(EMImode),
                                             QBPlatformOutputPriority_content);
        if (!CGMSstatus) {
            mute = EMImode != QBViewRightIPTVCopyControlEMI_CopyingPermited;
            log_warn("Failed to setup CGMS on output %u, will%s try to mute it (EMI mode is %u)", outputIndex, mute ? "": " not", EMImode);
        }
    } else {
        // CGMS not supported, mute ouptut if EMI mode is any but QBViewRightIPTVCopyControlEMI_CopyingPermited
        mute = EMImode != QBViewRightIPTVCopyControlEMI_CopyingPermited;
    }

    // handle Macrovision
    if (cfg->capabilities & QBOutputCapability_Macrovision) {
        QBContentProtectionMacrovisionSettings macrovisionSettings = {
            .mode = QBViewRightIPTVServiceMap2DataformatMacrovision(APSmode)
        };

        bool macrovisionStatus = !QBPlatformSetMacrovision(outputIndex, &macrovisionSettings, QBPlatformOutputPriority_content);
        if (!macrovisionStatus) {
            bool macrovisionMute = APSmode != QBViewRightIPTVCopyControlAPS_Disabled;
            log_warn("Failed to setup Macrovision on output %u, will%s try to mute it (APS mode is %u)",
                     outputIndex, macrovisionMute ? "" : " not", APSmode);
            mute |= macrovisionMute;
        }
    } else {
        // Macrovision not supported, mute ouptut if APS mode is any but QBViewRightIPTVCopyControlAPS_Disabled
        mute |= APSmode != QBViewRightIPTVCopyControlAPS_Disabled;
    }

    // perform mute/unmute
    QBPlatformOutputMute(outputIndex, mute, QBPlatformOutputPriority_content);
}


// reset all outputs state to defaults suitable for FTA content (i.e. all protections disabled)
SvLocal void QBViewRightIPTVServiceResetOutputs(void)
{
    QBVideoOutputConfig cfg = {
        .enabled = false,
    };

    // iterate through all outputs
    for (unsigned i = 0;; i++) {
        if (QBPlatformGetOutputConfig(i, &cfg) < 0) {
            break;      // no more outputs
        }

        if (QBViewRightIPTVServiceIsDigitalOutput(&cfg)) {
            QBViewRightIPTVServiceHandleDigitalOutput(i, &cfg, false);
        } else if (QBViewRightIPTVServiceIsAnalogOutput(&cfg)) {
            QBViewRightIPTVServiceHandleAnalogOutput(i, &cfg,
                                                     QBViewRightIPTVCopyControlEMI_CopyingPermited,
                                                     QBViewRightIPTVCopyControlAPS_Disabled, false);
        }
    }
}

SvLocal void QBViewRightIPTVServiceHandleCopyControl(SvObject self_, QBViewRightIPTVCopyControl copyControl)
{
    // copyControl object properties
    QBViewRightIPTVCopyControlEMI EMImode;
    QBViewRightIPTVCopyControlAPS APSmode;
    bool HDCP, DOT;

    EMImode = QBViewRightIPTVCopyControlGetEMImode(copyControl);
    APSmode = QBViewRightIPTVCopyControlGetAPSmode(copyControl);
    HDCP = QBViewRightIPTVCopyControlGetHDCP(copyControl);
    DOT = QBViewRightIPTVCopyControlGetDOT(copyControl);

    QBVideoOutputConfig cfg = {
        .enabled = false,
    };

    // iterate through all outputs
    for (unsigned i = 0;; i++) {
        if (QBPlatformGetOutputConfig(i, &cfg) < 0) {
            break;      // no more outputs
        }

        if (QBViewRightIPTVServiceIsDigitalOutput(&cfg)) {
            QBViewRightIPTVServiceHandleDigitalOutput(i, &cfg, HDCP);
        } else if (QBViewRightIPTVServiceIsAnalogOutput(&cfg)) {
            QBViewRightIPTVServiceHandleAnalogOutput(i, &cfg, EMImode, APSmode, DOT);
        }
    }
}

SvLocal void QBViewRightIPTVServiceHideWindows(QBViewRightIPTVService self, bool hide)
{
    if (self->pinDialog) {
        if (hide) {
            QBDialogHide(self->pinDialog, false, false);
        } else {
            QBDialogShow(self->pinDialog, false);
        }
    }

    if (self->mmiDialog) {
        svWidgetSetHidden(self->mmiDialog, hide);
    }

    if (self->userMessage.dialog) {
        svWidgetSetHidden(self->userMessage.dialog, hide);
    }
}

SvLocal void QBViewRightIPTVServiceVisabilityChanged(SvGenericObject self_, bool hidden)
{
    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;
    QBViewRightIPTVServiceHideWindows(self, hidden);
}

SvLocal void QBViewRightIPTVServiceRemovePopups(QBViewRightIPTVService self)
{
    log_fun();

    QBViewRightIPTVServiceRemoveFingerprints(self);

    if (self->pinDialog)
        QBDialogBreak(self->pinDialog);

    if (self->mmiDialog)
        QBDialogBreak(self->mmiDialog);

    if (self->userMessage.dialog)
        QBDialogBreak(self->userMessage.dialog);
}

SvLocal void QBViewRightIPTVServiceContentChanged(SvObject self_)
{
    log_fun();
    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;
    SvDequeRemoveAllObjects(self->scheduledPopups);
    QBViewRightIPTVServiceRemovePopups(self);
}

SvLocal void QBViewRightIPTVServicePlaybackEvent(SvObject self_, SvString name, void *arg)
{
    log_fun();

    QBViewRightIPTVService self = (QBViewRightIPTVService) self_;
    QBViewRightIPTVServiceBlockAudioVideo(self, false);

    if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_OFF)) {
        // content is FTA, disable the HDCP protection
        QBViewRightIPTVServiceResetOutputs();
    }
}
