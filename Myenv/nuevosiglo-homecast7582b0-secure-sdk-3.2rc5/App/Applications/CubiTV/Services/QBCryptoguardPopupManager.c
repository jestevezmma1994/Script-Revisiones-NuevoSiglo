/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBCryptoguardPopupManager.h"
#include <QBCASCommonCryptoguardOSDListener.h>
#include <QBCASCommonCryptoguardSTBDisplayMessage.h>
#include <QBCASCommonCryptoguardOSDMessage.h>
#include <QBCASCommonCryptoguardFingerprintListener.h>
#include <QBCASCommonCryptoguardFingerprint.h>

#include <QBCAS.h>
#include <Services/QBCASManager.h>
#include <player_events/decryption.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvEnv.h>
#include <main.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/types.h>
#include <SWL/QBFrame.h>
#include <Widgets/QBLongTextDialog.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <fibers/c/fibers.h>
#include <QBUTF8Utils.h>
#include <CAGE/Text/SvTextRender.h>
#include <SvGfxHAL/SvGfxUtils.h>
#include <SvGfxHAL/SvGfxObject.h>
#include <SvGfxHAL/SvGfxEngine.h>
#include <QBPlatformHAL/QBPlatformOutput.h>

#include <libintl.h>
#include <stdbool.h>

#if SV_LOG_LEVEL > 0

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 2, "QBCryptoguardPopupManger", "");

#define log_fun(fmt, ...) do { if (env_log_level() >= 4) { SvLogNotice(COLBEG() "QBCryptoguardPopupManger :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)
#define log_debug(fmt, ...) do { if (env_log_level() >= 3) { SvLogNotice(COLBEG() "QBCryptoguardPopupManger :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)
#define log_state(fmt, ...) do { if (env_log_level() >= 2) { SvLogNotice(COLBEG() "QBCryptoguardPopupManger :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } } while (0)
#define log_error(fmt, ...) do { if (env_log_level() >= 1) { SvLogError(COLBEG() "QBCryptoguardPopupManger :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } } while (0)

#else

#define log_fun(fmt, ...)
#define log_debug(fmt, ...)
#define log_state(fmt, ...)
#define log_error(fmt, ...)

#endif

static const Sv2DRect MINIMAL_FP_SIZE = { .width = 0, .height = 12 };

#define FINGERPRINT_COMPENSATION_MARGIN_PERCENT 10

struct QBCryptoguardPopupManager_s {
    struct SvObject_ super;

    QBCASSmartcardState smartcardState;
    struct QBCASCallbacks_s casCallbacks;

    AppGlobals appGlobals;
    bool FTA;
    int disabledWindows;

    QBGlobalWindow globalWindow;
    SvWidget frame;
    SvWidget label;
    SvWidget STBDisplayMessageDialog;
    SvWidget OSDMessageDialog;

    struct {
        SvFont font;
        int fontSize;

        unsigned int hdHandle;
        unsigned int sdHandle;

        SvBitmap bmp;
        int x;
        int y;
    } fingerprint;

    SvFiber fiber;
    SvFiberTimer OSDMessageTimer;
    SvFiberTimer STBDisplayMessageTimer;
    SvFiberTimer fingerprintTimer;

    QBCASCommonCryptoguardOSDMessage OSDMessage;
    QBCASCommonCryptoguardSTBDisplayMessage STBDisplayMessage;
};

SvLocal void QBCryptoguardPopupManagerDestroyOSDMessageDialog(QBCryptoguardPopupManager self)
{
    log_fun();

    if (self->OSDMessageDialog) {
        QBDialogBreak(self->OSDMessageDialog);
        self->OSDMessageDialog = NULL;
    }
}

SvLocal void QBCryptoguardPopupManagerDestroySTBDisplayMessageDialog(QBCryptoguardPopupManager self)
{
    log_fun();

    if (self->STBDisplayMessageDialog) {
        svWidgetDestroy(self->STBDisplayMessageDialog);
        self->STBDisplayMessageDialog = NULL;
    }
}

SvLocal void QBCryptoguardPopupManagerRemoveOSDMessageDialog(QBCryptoguardPopupManager self)
{
    log_fun();
    SvFiberEventDeactivate(self->OSDMessageTimer);

    QBCryptoguardPopupManagerDestroyOSDMessageDialog(self);

    SVTESTRELEASE(self->OSDMessage);
    self->OSDMessage = NULL;
}

SvLocal void QBCryptoguardPopupManagerRemoveSTBDisplayMessageDialog(QBCryptoguardPopupManager self)
{
    log_fun();
    SvFiberEventDeactivate(self->STBDisplayMessageTimer);

    QBCryptoguardPopupManagerDestroySTBDisplayMessageDialog(self);

    SVTESTRELEASE(self->STBDisplayMessage);
    self->STBDisplayMessage = NULL;
}

SvLocal void QBCryptoguardPopupManagerSTBDisplayMessageDialogCallbackOnPressedButton(void *target, SvWidget w)
{
    log_fun();
    QBCryptoguardPopupManager self = target;

    if (self->STBDisplayMessageDialog == w) {
        QBCryptoguardPopupManagerRemoveSTBDisplayMessageDialog(self);
    }
}

SvLocal void QBCryptoguardPopupManagerShowSTBDisplayMessageDialog(QBCryptoguardPopupManager self, SvString label, bool withButton)
{
    log_fun();
    svSettingsPushComponent("CryptoguardPopupManager.settings");
    self->STBDisplayMessageDialog = QBLongTextDialogCreate(self->appGlobals, "CryptoguardPopupManagerSTBDialog", label, withButton, NULL);
    svSettingsPopComponent();

    if (self->STBDisplayMessageDialog) {
        QBLongTextDialogSetCallbackOnPressedButton(self->STBDisplayMessageDialog, self, QBCryptoguardPopupManagerSTBDisplayMessageDialogCallbackOnPressedButton);
    } else {
        log_error("Cannot create dialog for message (%s)", SvStringCString(label));
    }
}

SvLocal bool QBCryptoguardPopupManagerOSDMessageDialogNotificationCallback(void *ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    log_fun();
    QBCryptoguardPopupManager self = ptr;

    if (self->OSDMessageDialog == dialog) {
        QBCryptoguardPopupManagerRemoveOSDMessageDialog(self);
        return true;
    }

    return false;
}

SvLocal void QBCryptoguardPopupManagerShowOSDMessageDialog(QBCryptoguardPopupManager self, SvString label, bool withButton)
{
    log_fun();
    svSettingsPushComponent("CryptoguardPopupManager.settings");
    QBDialogParameters dialogParams = {
        .app = self->appGlobals->res,
        .controller = self->appGlobals->controller,
        .ownerId = 0,
        .widgetName = "Dialog",
    };
    self->OSDMessageDialog = QBDialogGlobalNew(&dialogParams, SVSTRING("CryptoguardPopupManagerOSDDialog"));

    QBDialogSetNotificationCallback(self->OSDMessageDialog, self, QBCryptoguardPopupManagerOSDMessageDialogNotificationCallback);
    QBDialogAddPanel(self->OSDMessageDialog, SVSTRING("content"), NULL, 1);
    QBDialogAddLabel(self->OSDMessageDialog, SVSTRING("content"), SvStringCString(label), SVSTRING("message"), 1);
    QBDialogSetTitle(self->OSDMessageDialog, gettext("Message"));
    if (withButton) {
        QBDialogAddButton(self->OSDMessageDialog, SVSTRING("OK-button"), gettext("OK"), 1);
    }

    svSettingsPopComponent();
    QBDialogRun(self->OSDMessageDialog, NULL, NULL);
}

SvLocal SvString QBCryptoguardPopupManagerCreateCardStateString(QBCryptoguardPopupManager self, QBCASSmartcardState smartcardState)
{
    log_fun();
    SvString smartcardStateString = NULL;

    switch (smartcardState) {
        case QBCASSmartcardState_removed:
            log_debug("Card removed");
            smartcardStateString = SvStringCreate(gettext("Card removed"), NULL);
            break;
        case QBCASSmartcardState_incorrect:
        case QBCASSmartcardState_incorrect_ex:
            log_debug("Card incorrect");
            smartcardStateString = SvStringCreate(gettext("Card incorrect"), NULL);
            break;
        case QBCASSmartcardState_problem:
            log_debug("Card problem");
            smartcardStateString = SvStringCreate(gettext("Card problem"), NULL);
            break;
        case QBCASSmartcardState_inserted:
            log_debug("Card inserted");
            smartcardStateString = SvStringCreate(gettext("Card inserted"), NULL);
            break;
        case QBCASSmartcardState_correct:
            log_debug("Card correct");
            smartcardStateString = SvStringCreate(gettext("Card correct"), NULL);
            break;
        default:
            log_error("Unknown card state, (%d)", smartcardState);
            smartcardStateString = SvStringCreate(gettext("Unknown card state"), NULL);
    }

    return smartcardStateString;
}

SvLocal void QBCryptoguardPopupManagerShowLabel(QBCryptoguardPopupManager self, SvString label)
{
    log_fun();
    QBAsyncLabelSetText(self->label, label);
    QBApplicationControllerAddGlobalWindow(self->appGlobals->controller, self->globalWindow);
}

SvLocal void QBCryptoguardPopupManagerHideLabel(QBCryptoguardPopupManager self)
{
    log_fun();
    if (self->globalWindow && self->appGlobals->controller) {
        QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, self->globalWindow);
    }
}

SvLocal void QBCryptoguardPopupManagerPrepareFingerprint(QBCryptoguardPopupManager self, QBCASCommonCryptoguardFingerprint fingerprint)
{
    log_fun();

    SvColor color = COLOR(((fingerprint->fontColor >> 16) & 0xFF), ((fingerprint->fontColor >> 8) & 0xFF), (fingerprint->fontColor & 0xFF), ((fingerprint->fontColor >> 24) & 0xFF));
    SvFontSetSize(self->fingerprint.font, self->fingerprint.fontSize, NULL);
    SvBitmap textBmp = SvBitmapCreateWithText(SvStringCString(fingerprint->text), self->fingerprint.font, color, NULL);

    SvColor bgColor = COLOR(((fingerprint->backgroundColor >> 16) & 0xFF), ((fingerprint->backgroundColor >> 8) & 0xFF), (fingerprint->backgroundColor & 0xFF), ((fingerprint->backgroundColor >> 24) & 0xFF));

    // create background bitmap which has one pixel margin
    SvBitmap bmp = SvBitmapCreateAndFill(textBmp->width + 2, textBmp->height + 2, SV_CS_BGRA, bgColor);

    SvBitmapBlend(bmp, 1, 1, textBmp, NULL, ALPHA_SOLID, NULL);
    SVRELEASE(textBmp);

    SvGfxBlitBitmap(bmp, NULL);
    SvBitmapBlank(bmp, NULL);

    self->fingerprint.bmp = bmp;

    // destination coordinates are in PAL
    // CryptoGuard recommended to compensate fingerprint position for overscan
    uint32_t xOffset = 720 * FINGERPRINT_COMPENSATION_MARGIN_PERCENT / 100;
    uint32_t yOffset = 576 * FINGERPRINT_COMPENSATION_MARGIN_PERCENT / 100;
    uint32_t compensatedWidth = 720 - 2 * xOffset;
    uint32_t compensatedHeight = 576 - 2 * yOffset;

    self->fingerprint.x = xOffset + compensatedWidth * fingerprint->xPosition / 16;
    self->fingerprint.y = yOffset + compensatedHeight * fingerprint->yPosition / 16;
    log_debug("x(%d), y(%d)", self->fingerprint.x, self->fingerprint.y);
}

SvLocal void QBCryptoguardPopupManagerFingerprintFillGfxObject(QBCryptoguardPopupManager self, struct SvGfxObject_s *gfxObject)
{
    log_fun();
    SvBitmap bmp = self->fingerprint.bmp;

    gfxObject->src = bmp->hwSurface;
    gfxObject->color = 0xffffffff; // neutral value for color multiplication
    gfxObject->blend = true;
    gfxObject->ldim = 0;
    gfxObject->dest_rect = Sv2DRectCreate(self->fingerprint.x, self->fingerprint.y, bmp->width, bmp->height);;
    gfxObject->src_rect = Sv2DRectCreate(0, 0, bmp->width, bmp->height);
}

SvLocal void QBCryptoguardPopupManagerFingerprintAddRemove(QBCryptoguardPopupManager self, bool add)
{
    log_fun();
    int HDOutputIDGfx = -1;
    int SDOutputIDGfx = -1;

    int HDOutputID = QBPlatformFindOutput(QBOutputType_unknown, NULL, QBOutputCapability_HD, true, false);
    if (HDOutputID >= 0) {
        QBVideoOutputConfig config;
        int ret = QBPlatformGetOutputConfig(HDOutputID, &config);
        if (ret != 0) {
            log_error("Cannot get output config for outputID %d", HDOutputID);
            return;
        }
        if ((int) config.masterGfxID == HDOutputID) {
            HDOutputIDGfx = config.masterGfxID;
        }
    }

    int SDOutputID = QBPlatformFindOutput(QBOutputType_unknown, NULL, QBOutputCapability_SD, true, false);
    if (SDOutputID >= 0) {
        QBVideoOutputConfig config;
        int ret = QBPlatformGetOutputConfig(SDOutputID, &config);
        if (ret != 0) {
            log_error("Cannot get output config for outputID %d", SDOutputID);
            return;
        }
        if ((int) config.masterGfxID == SDOutputID) {
            SDOutputIDGfx = config.masterGfxID;
        }
    }

    if (add) {
        struct SvGfxObject_s object = {
            .object_id = 0,
        };
        QBCryptoguardPopupManagerFingerprintFillGfxObject(self, &object);

        if (HDOutputIDGfx >= 0) {
            SvGfxEngineAddImageOnVideo(HDOutputIDGfx, &object, &MINIMAL_FP_SIZE, SvGfxOnVideoQueueObjectType_default, &self->fingerprint.hdHandle);
        }
        if (SDOutputIDGfx >= 0) {
            SvGfxEngineAddImageOnVideo(SDOutputIDGfx, &object, &MINIMAL_FP_SIZE, SvGfxOnVideoQueueObjectType_default, &self->fingerprint.sdHandle);
        }
    } else {
        if (HDOutputIDGfx >= 0) {
            SvGfxEngineRemoveImageFromVideo(HDOutputIDGfx, self->fingerprint.hdHandle);
        }
        if (SDOutputIDGfx >= 0) {
            SvGfxEngineRemoveImageFromVideo(SDOutputIDGfx, self->fingerprint.sdHandle);
        }
    }
}

SvLocal void QBCryptoguardPopupManagerShowFingerprint(QBCryptoguardPopupManager self, QBCASCommonCryptoguardFingerprint fingerprint)
{
    log_fun();
    QBCryptoguardPopupManagerPrepareFingerprint(self, fingerprint);
    QBCryptoguardPopupManagerFingerprintAddRemove(self, true);
}

SvLocal void QBCryptoguardPopupManagerHideFingerprint(QBCryptoguardPopupManager self)
{
    log_fun();
    if (self->fingerprint.bmp) {
        SvFiberEventDeactivate(self->fingerprintTimer);
        QBCryptoguardPopupManagerFingerprintAddRemove(self, false);
        SVRELEASE(self->fingerprint.bmp);
        self->fingerprint.bmp = NULL;
    }
}

SvLocal void QBCryptoguardPopupManagerUpdate(QBCryptoguardPopupManager self)
{
    log_fun();

    if (!self->FTA) {
        if (self->smartcardState == QBCASSmartcardState_correct) {
            QBCryptoguardPopupManagerHideLabel(self);
        } else {
            // show current card status
            SvString label = QBCryptoguardPopupManagerCreateCardStateString(self, self->smartcardState);
            QBCryptoguardPopupManagerShowLabel(self, label);
            SVRELEASE(label);
        }
    } else {
        // Only 'card status' and OSD message is removed here, STB messages should not be removed while switching channels
        // CryptoGuard requires not to remove  STB messages, but does not say anything about the others.
        // We assume that OSD messages can be removed during channel switching/leaving standby mode only if they have specified
        // (non-zero) timeout.
        QBCryptoguardPopupManagerHideLabel(self);
        if (self->OSDMessage && self->OSDMessage->duration) {
            QBCryptoguardPopupManagerRemoveOSDMessageDialog(self);
        }
    }
}

SvLocal void QBCryptoguardPopupManagerStep(void *self_)
{
    log_fun();
    QBCryptoguardPopupManager self = (QBCryptoguardPopupManager) self_;
    SvFiberDeactivate(self->fiber);

    if (SvFiberEventIsActive(self->OSDMessageTimer)) {
        // OSD message displayed
        QBCryptoguardPopupManagerRemoveOSDMessageDialog(self);
    }

    if (SvFiberEventIsActive(self->STBDisplayMessageTimer)) {
        // STB Display message displayed
        QBCryptoguardPopupManagerRemoveSTBDisplayMessageDialog(self);
    }

    if (SvFiberEventIsActive(self->fingerprintTimer)) {
        QBCryptoguardPopupManagerHideFingerprint(self);
    }
}

SvLocal void QBCryptoguardPopupManagerSmartcardCallback(void* target, QBCASSmartcardState state)
{
    log_fun();
    QBCryptoguardPopupManager self = (QBCryptoguardPopupManager) target;
    self->smartcardState = state;
    QBCryptoguardPopupManagerUpdate(self);
}

SvLocal void QBCryptoguardPopupManagerDestroy(void *self_)
{
    log_fun();
    QBCryptoguardPopupManager self = self_;

    QBCryptoguardPopupManagerHideLabel(self);
    QBCryptoguardPopupManagerRemoveOSDMessageDialog(self);
    QBCryptoguardPopupManagerRemoveSTBDisplayMessageDialog(self);
    QBCryptoguardPopupManagerHideFingerprint(self);

    SvFiberDestroy(self->fiber);
    SVRELEASE(self->globalWindow);
    SVRELEASE(self->fingerprint.font);
}

// -------------------- QBCASPopupManager (callbacks start) --------------------
SvLocal void QBCryptoguardPopupManagerCASPopupManagerPlaybackEvent(SvObject self_, SvString name, void *arg)
{
    log_fun();
    QBCryptoguardPopupManager self = (QBCryptoguardPopupManager) self_;
    if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_FAILED)) {
    } else if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_OK)) {
    } else if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_ON)) {
        self->FTA = false;
    } else if (SvStringEqualToCString(name, PLAYER_EVENT_DECRYPTION_OFF)) {
        self->FTA = true;
    }

    QBCryptoguardPopupManagerUpdate(self);
}

SvLocal void QBCryptoguardPopupManagerCASPopupManagerContentChanged(SvObject self_)
{
    log_fun();
    QBCryptoguardPopupManager self = (QBCryptoguardPopupManager) self_;
    self->FTA = true;
    QBCryptoguardPopupManagerUpdate(self);
}

SvLocal void QBCryptoguardPopupManagerCASPopupManagerStart(SvObject self_)
{
    log_fun();
    QBCryptoguardPopupManager self = (QBCryptoguardPopupManager) self_;
    self->smartcardState = SvInvokeInterface(QBCAS, QBCASGetInstance(), getState);

    self->casCallbacks.smartcard_state = &QBCryptoguardPopupManagerSmartcardCallback;
    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, &self->casCallbacks, self, "QBCryptoguardPopupManager");

    QBCASCommonCryptoguardOSDListenerAdd((QBCASCommonCryptoguard) QBCASGetInstance(), (SvObject) self);
    QBCASCommonCryptoguardFingerprintListenerAdd((QBCASCommonCryptoguard) QBCASGetInstance(), (SvObject) self);

    QBCryptoguardPopupManagerUpdate(self);
}

SvLocal void QBCryptoguardPopupManagerCASPopupManagerStop(SvObject self_)
{
    log_fun();
    QBCryptoguardPopupManager self = (QBCryptoguardPopupManager) self_;

    QBCryptoguardPopupManagerHideLabel(self);
    QBCryptoguardPopupManagerRemoveSTBDisplayMessageDialog(self);
    QBCryptoguardPopupManagerRemoveOSDMessageDialog(self);
    QBCryptoguardPopupManagerHideFingerprint(self);

    QBCASCommonCryptoguard casInstance = (QBCASCommonCryptoguard) QBCASGetInstance();
    if (casInstance) {
        QBCASCommonCryptoguardFingerprintListenerRemove(casInstance, (SvObject) self);
        QBCASCommonCryptoguardOSDListenerRemove(casInstance, (SvObject) self);
        SvInvokeInterface(QBCAS, casInstance, removeCallbacks, &self->casCallbacks, self);
    }
}

SvLocal void QBCryptoguardPopupManagerCASPopupManagerVisabilityChanged(SvObject self_, bool hidden)
{
    // we don't want to hide OSD message anytime
}

// -------------------- QBCASPopupManager (callbacks end) --------------------

SvLocal void QBCryptoguardPopupManagerDisplayMessages(QBCryptoguardPopupManager self)
{
    log_fun();

    // OSD message should be always on top on STB Display message

    QBCryptoguardPopupManagerDestroySTBDisplayMessageDialog(self);
    QBCryptoguardPopupManagerDestroyOSDMessageDialog(self);

    if (self->STBDisplayMessage) {
        QBCryptoguardPopupManagerShowSTBDisplayMessageDialog(self, self->STBDisplayMessage->message, !self->STBDisplayMessage->forced);
    }

    if (self->OSDMessage) {
        // if OSD duration is equal zero it means that this message won't be hidden automatically, but could be removed manually
        //
        // On 02.07.2015 15:11, Kent Johansson wrote that all OSD messages should be cancelable
        QBCryptoguardPopupManagerShowOSDMessageDialog(self, self->OSDMessage->message, true /*(self->OSDMessage->duration == 0)*/);
    }
}

SvLocal void QBCryptoguardPopupManagerSTBDisplayMessage(SvObject self_, int8_t sessionId, QBCASCommonCryptoguardSTBDisplayMessage message)
{
    log_fun();
    QBCryptoguardPopupManager self = (QBCryptoguardPopupManager) self_;

    log_state("STB Display Message Received, sessionId (%d), forced (%s), duration (%u) ms, message (%s)",
              sessionId, (message->forced) ? "true" : "false", message->duration, SvStringCString(message->message));

    QBCryptoguardPopupManagerRemoveSTBDisplayMessageDialog(self);

    SVTESTRELEASE(self->STBDisplayMessage);
    self->STBDisplayMessage = SVRETAIN(message);
    QBCryptoguardPopupManagerDisplayMessages(self);

    SvFiberTimerActivateAfter(self->STBDisplayMessageTimer, SvTimeConstruct(0, message->duration * 1000));
}

SvLocal void QBCryptoguardPopupManagerOSDMessage(SvObject self_, int8_t sessionId, QBCASCommonCryptoguardOSDMessage message)
{
    log_fun();
    QBCryptoguardPopupManager self = (QBCryptoguardPopupManager) self_;

    log_state("OSD Message Received, sessionId (%d), duration (%u) ms, message (%s)",
              sessionId, message->duration, SvStringCString(message->message));

    QBCryptoguardPopupManagerRemoveOSDMessageDialog(self);

    SVTESTRELEASE(self->OSDMessage);
    self->OSDMessage = SVRETAIN(message);
    QBCryptoguardPopupManagerDisplayMessages(self);

    // if OSD duration is equal zero it means that this message won't be hidden automatically, but could be removed manually
    if (message->duration != 0) {
        SvFiberTimerActivateAfter(self->OSDMessageTimer, SvTimeConstruct(0, message->duration * 1000));
    }
}

SvLocal void QBCryptoguardPopupManagerDisplayFingerprint(SvObject self_, int8_t sessionId, QBCASCommonCryptoguardFingerprint fingerprint)
{
    log_fun();
    QBCryptoguardPopupManager self = (QBCryptoguardPopupManager) self_;

    log_state("Fingerprint Received, sessionId (%d), text (%s), duration (%u) ms, xPosition (%u), yPosition (%u), fontColor (0x%X), backgroundColor (0x%X)",
              sessionId, SvStringCString(fingerprint->text), fingerprint->duration, fingerprint->xPosition, fingerprint->yPosition, fingerprint->fontColor, fingerprint->backgroundColor);

    QBCryptoguardPopupManagerHideFingerprint(self);
    QBCryptoguardPopupManagerShowFingerprint(self, fingerprint);
    SvFiberTimerActivateAfter(self->fingerprintTimer, SvTimeConstruct(0, fingerprint->duration * 1000));
}

SvLocal SvType QBCryptoguardPopupManager_getType(void)
{
    log_fun();
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBCryptoguardPopupManagerDestroy
    };
    static const struct QBCASPopupManager_ popupMethods = {
        .playbackEvent      = QBCryptoguardPopupManagerCASPopupManagerPlaybackEvent,
        .contentChanged     = QBCryptoguardPopupManagerCASPopupManagerContentChanged,
        .visibilityChanged  = QBCryptoguardPopupManagerCASPopupManagerVisabilityChanged,
        .start              = QBCryptoguardPopupManagerCASPopupManagerStart,
        .stop               = QBCryptoguardPopupManagerCASPopupManagerStop
    };

    struct QBCASCommonCryptoguardOSDListener_s cryptoguardOSDMethods = {
        .STBDisplayMessage = QBCryptoguardPopupManagerSTBDisplayMessage,
        .OSDMessage        = QBCryptoguardPopupManagerOSDMessage,

    };

    struct QBCASCommonCryptoguardFingerprintListener_s cryptoguardFingerprint = {
        .displayFingerprint = QBCryptoguardPopupManagerDisplayFingerprint,
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBCryptoguardPopupManager",
                            sizeof(struct QBCryptoguardPopupManager_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBCASPopupManager_getInterface(), &popupMethods,
                            QBCASCommonCryptoguardOSDListener_getInterface(), &cryptoguardOSDMethods,
                            QBCASCommonCryptoguardFingerprintListener_getInterface(), &cryptoguardFingerprint,
                            NULL);
    }

    return type;
}

QBCryptoguardPopupManager QBCryptoguardPopupManagerCreate(AppGlobals appGlobals)
{
    log_fun();
    QBCryptoguardPopupManager self = (QBCryptoguardPopupManager) SvTypeAllocateInstance(QBCryptoguardPopupManager_getType(), NULL);

    self->appGlobals = appGlobals;
    self->FTA = true;
    self->smartcardState = QBCASSmartcardState_removed;

    svSettingsPushComponent("CryptoguardPopupManager.settings");
    SvWidget frame = QBFrameCreateFromSM(appGlobals->res, "CryptoguardPopupManager.SimplePopup");
    SvWidget label = QBAsyncLabelNew(appGlobals->res, "CryptoguardPopupManager.SimplePopup.Title", appGlobals->textRenderer);
    svSettingsWidgetAttach(frame, label, "CryptoguardPopupManager.SimplePopup.Title", 1);

    self->fingerprint.font = svSettingsCreateFont("CryptoguardPopupManager.Fingerprint", "font");
    self->fingerprint.fontSize = svSettingsGetInteger("CryptoguardPopupManager.Fingerprint", "fontSize", 20);

    svSettingsPopComponent();

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    QBGlobalWindowInit(globalWindow, frame, SVSTRING("CryptoguardPopupManager"));
    self->globalWindow = globalWindow;
    self->frame = frame;
    self->label = label;

    self->fiber = SvFiberCreate(appGlobals->scheduler, NULL, "QBCryptoguardPopupManagerStep", QBCryptoguardPopupManagerStep, self);
    self->OSDMessageTimer = SvFiberTimerCreate(self->fiber);
    self->STBDisplayMessageTimer = SvFiberTimerCreate(self->fiber);
    self->fingerprintTimer = SvFiberTimerCreate(self->fiber);

    return self;
}
