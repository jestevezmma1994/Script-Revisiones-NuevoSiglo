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

#include "QBLatensPopupManager.h"
#include <QBLatens.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Utils/authenticators.h>
#include <QBApplicationController.h>
#include <Services/QBCASManager.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvEnv.h>
#include <SvFoundation/SvDeque.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/QBFrame.h>
#include <SWL/label.h>
#include <SWL/button.h>
#include <SWL/events.h>
#include <CAGE/Text/SvFont.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Core/SvSurface.h>
#include <SvGfxHAL/SvGfxUtils.h>
#include <TranslationMerger.h>
#include <main.h>
#include <libintl.h>

#if SV_LOG_LEVEL > 0
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 4, "QBLatensLogLevel", "");
    #define log_error(fmt, ...) do { if (env_log_level() >= 1) SvLogError(COLBEG() "QBLatensPopupManager :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
    #define log_deep(fmt, ...) do { if (env_log_level() >= 4) SvLogNotice(COLBEG() "QBLatensPopupManager :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } while (0)
#else
    #define log_error(fmt, ...)
    #define log_deep(fmt, ...)
#endif

typedef enum {
    QBLatensPopupManagerScreenType_state = 0,
    QBLatensPopupManagerScreenType_message,
    QBLatensPopupManagerScreenType_overlay,
    QBLatensPopupManagerScreenType_cnt,
} QBLatensPopupManagerScreenType;

struct QBLatensPopupManager_s {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvHashTable screenHandleToScreen;

    SvDeque stbStateScreenStack;
    SvDeque remoteMessageScreenStack;
    SvDeque overlayScreenStack;

    SvWeakList listeners;

    int settingsCtx;
    int fontSizeMultiplier;
    int fontBaseSize;
};

SvLocal void QBLatensPopupManagerContentChanged(SvObject self_)
{
}

SvLocal void QBLatensPopupManagerStart(SvObject self_)
{
}

SvLocal void QBLatensPopupManagerStop(SvObject self_)
{
}

SvLocal void QBLatensPopupManagerVisabilityChanged(SvObject self_, bool hidden)
{
}

SvLocal void QBLatensPopupManagerPlaybackEvent(SvObject self_,
                                               SvString name, void *arg)
{
}

SvLocal QBLatensPopupManagerScreenType QBLatensPopupManagerScreenTypeGetNextLevel(QBLatensPopupManagerScreenType level)
{
    switch (level) {
        case QBLatensPopupManagerScreenType_state:
            return QBLatensPopupManagerScreenType_message;
        case QBLatensPopupManagerScreenType_message:
            return QBLatensPopupManagerScreenType_overlay;
        default:
            return QBLatensPopupManagerScreenType_cnt;
    }
}

SvLocal SvDeque QBLatensPopupManagerScreenTypeToWindowStack(QBLatensPopupManager self, QBLatensPopupManagerScreenType screenType)
{
    switch (screenType) {
        case QBLatensPopupManagerScreenType_state:
            return self->stbStateScreenStack;
        case QBLatensPopupManagerScreenType_message:
            return self->remoteMessageScreenStack;
        case QBLatensPopupManagerScreenType_overlay:
            return self->overlayScreenStack;
        default:
            log_error("unknown screenType: %d", screenType);
            return NULL;
    }
}

SvLocal void QBLatensPopoupManagerHideScreen(QBLatensPopupManager self,
                                             QBLatensPopupManagerScreenType screenType)
{
    SvDeque windowStack = QBLatensPopupManagerScreenTypeToWindowStack(self, screenType);
    if (!windowStack) {
        return;
    }

    log_deep("screenType = %d", screenType);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvDequeFront(windowStack);
    if (globalWindow) {
        QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, globalWindow);
    }
}

SvLocal void QBLatensPopupManagerShowScreen(QBLatensPopupManager self, QBLatensPopupManagerScreenType screenType)
{
    SvDeque windowStack = QBLatensPopupManagerScreenTypeToWindowStack(self, screenType);

    QBLatensPopupManagerScreenType nextLevel = QBLatensPopupManagerScreenTypeGetNextLevel(screenType);

    if (nextLevel != QBLatensPopupManagerScreenType_cnt) {
        QBLatensPopoupManagerHideScreen(self, nextLevel);
    }

    log_deep("screenType = %d", screenType);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvDequeFront(windowStack);
    if (globalWindow) {
        QBApplicationControllerAddGlobalWindow(self->appGlobals->controller, globalWindow);
    }

    if (nextLevel != QBLatensPopupManagerScreenType_cnt) {
        QBLatensPopupManagerShowScreen(self, nextLevel);
    }
}

SvLocal int QBLatensPopoupManagerRemoveScreen(QBLatensPopupManager self,
                                              QBLatensPopupManagerScreenType screenType, SvValue screenHandleVal)
{
    SvDeque windowStack = QBLatensPopupManagerScreenTypeToWindowStack(self, screenType);
    if (!windowStack) {
        log_error("Window stack not found");
        return -1;
    }

    log_deep("screenType = %d, screenHandle = %d", screenType, SvValueGetInteger(screenHandleVal));

    int retScreenHandle = -1;

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvHashTableFind(self->screenHandleToScreen, (SvObject) screenHandleVal);
    if (globalWindow) {
        SvDequeRemove(windowStack, (SvObject) globalWindow);

        QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, globalWindow);
        retScreenHandle = SvValueGetInteger(screenHandleVal);
        SvHashTableRemove(self->screenHandleToScreen, (SvObject) screenHandleVal);
    }

    QBLatensPopupManagerShowScreen(self, screenType);

    return retScreenHandle;
}

SvLocal void QBLatensRemoteMessageScreenUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e)
{
    if (e->code != SV_EVENT_BUTTON_PUSHED) {
        return;
    }

    SvValue screenHandleVal = (SvValue) w->prv;
    int screenHandle = SvValueGetInteger(screenHandleVal);

    QBLatens latens = QBLatensGetInstance();
    QBLatensRemoveRemoteMessageScreen(latens, screenHandle);
    SVRELEASE(screenHandleVal);
}


static const char * QBLatensStateMessages[] = {
    gettext_noop("SSM Download"),
    gettext_noop("Not subscribed"),
    gettext_noop("FTA Blocking - Block"),
    gettext_noop("Not authorized"),
    gettext_noop("FTA Blocking - Ent Based"),
};

SvLocal void QBLatensPopupManagerDisplayStateScreen(SvObject self_,
                                                    SvValue screenHandle, QBLatensOSDTextType textType, SvString channel, bool blockPlayback)
{
    QBLatensPopupManager self = (QBLatensPopupManager) self_;
    SvString text = NULL;
    if (textType < QBLatensOSDTextType_max) {
        text = SvStringCreate(gettext(QBLatensStateMessages[textType]), NULL);
    }

    log_deep("screenHandle = %d, text = %s, channel = %s, blockPlayback = %d", SvValueGetInteger(screenHandle),
             text ? SvStringCString(text) : "(NULL)", channel ? SvStringCString(channel) : "(NULL)", blockPlayback);

    svSettingsRestoreContext(self->settingsCtx);
    SvWidget window = QBFrameCreateFromSM(self->appGlobals->res, "LatensPopupManager.SimplePopup");
    SvWidget label = svLabelNewFromSM(self->appGlobals->res, "LatensPopupManager.SimplePopup.Title");
    svLabelSetText(label, text ? SvStringCString(text) : gettext("Unknown message"));
    SVTESTRELEASE(text);

    svSettingsWidgetAttach(window, label, "LatensPopupManager.SimplePopup.Title", 1);
    svSettingsPopComponent();

    QBLatensPopoupManagerHideScreen(self, QBLatensPopupManagerScreenType_state);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    SvString name = SvStringCreateWithFormat("%d", SvValueGetInteger(screenHandle));
    QBGlobalWindowInit(globalWindow, window, SVSTRING("LatensStateScreen"));
    SVRELEASE(name);
    SvDequePushFront(self->stbStateScreenStack, (SvObject) globalWindow, NULL);
    SVRELEASE(globalWindow);

    QBLatensPopupManagerShowScreen(self, QBLatensPopupManagerScreenType_state);
    globalWindow->focusable = false;

    SvHashTableInsert(self->screenHandleToScreen, (SvObject) screenHandle, (SvObject) globalWindow);
}

SvLocal int QBLatensPopupManagerGetRecalculatedFontSize(QBLatensPopupManager self, unsigned char fontSize)
{
    return self->fontBaseSize + self->fontSizeMultiplier * fontSize;
}

SvLocal SvColor QBLatensPopupManagerGetRecalculatedColor(unsigned long color, unsigned char transparency)
{
    unsigned long colorRGB_ = color << 8;
    unsigned char transparencyScaled = (transparency * ALPHA_SOLID) / 100;
    return COLOR(R(colorRGB_), G(colorRGB_), B(colorRGB_), ALPHA_SOLID - transparencyScaled);
}

SvLocal void QBLatensPopupManagerDisplayRemoteMessageScreen(
    SvObject self_, SvValue screenHandle, SvString text,
    SvString channel, unsigned long textColour,
    unsigned long backgroundColour, unsigned char textTransparency,
    unsigned char backgroundTransparency, unsigned char fontSize,
    unsigned char forced)
{
    QBLatensPopupManager self = (QBLatensPopupManager) self_;

    log_deep("screenHandle = %d, text = %s, channel = %s, fontSize = %i, forced = %i", SvValueGetInteger(screenHandle),
             text ? SvStringCString(text) : "(NULL)", channel ? SvStringCString(channel) : "(NULL)", fontSize, forced);

    SvColor backroundColorRGB = QBLatensPopupManagerGetRecalculatedColor(backgroundColour, backgroundTransparency);

    svSettingsRestoreContext(self->settingsCtx);

    SvWidget label = svLabelNewFromSM(self->appGlobals->res, "LatensPopupManager.SimplePopup.Title");


    svLabelSetFontSize(label, QBLatensPopupManagerGetRecalculatedFontSize(self, fontSize));
    svLabelSetText(label, SvStringCString(text));

    SvColor textColorRGB = QBLatensPopupManagerGetRecalculatedColor(textColour, textTransparency);
    svLabelSetTextColor(label, textColorRGB, 0.0);

    int buttonHeight = svSettingsGetInteger("LatensPopupManager.SimplePopup.Button", "height", 0);
    int windowWidth = svSettingsGetInteger("LatensPopupManager.SimplePopup", "width", 0);
    int windowHeight = svSettingsGetInteger("LatensPopupManager.SimplePopup", "height", 0);
    int buttonOffsetY = svSettingsGetInteger("LatensPopupManager.SimplePopup.Button", "yOffset", 0);
    int labelOffsetY = svSettingsGetInteger("LatensPopupManager.SimplePopup.Title", "yOffset", 0);

    // multiply by 2 because we need margin before and after label
    if (windowHeight < labelOffsetY * 2 + svLabelGetHeight(label))
        windowHeight = labelOffsetY * 2 + svLabelGetHeight(label);

    if (!forced)
        windowHeight += buttonHeight + buttonOffsetY;

    SvWidget parentWindow = svWidgetCreateWithColor(self->appGlobals->res, self->appGlobals->res->width, self->appGlobals->res->height, 0);
    SvWidget window = svWidgetCreateWithColor(self->appGlobals->res, windowWidth, windowHeight, backroundColorRGB);
    svSettingsWidgetAttach(parentWindow, window, "LatensPopupManager.SimplePopup", 1);
    svSettingsWidgetAttach(window, label, "LatensPopupManager.SimplePopup.Title", 1);

    QBLatensPopoupManagerHideScreen(self, QBLatensPopupManagerScreenType_message);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    SvString name = SvStringCreateWithFormat("%d", SvValueGetInteger(screenHandle));
    QBGlobalWindowInit(globalWindow, parentWindow, name);
    SVRELEASE(name);
    globalWindow->focusable = true;

    SvDequePushFront(self->remoteMessageScreenStack, (SvObject) globalWindow, NULL);
    SVRELEASE(globalWindow);

    QBLatensPopupManagerShowScreen(self, QBLatensPopupManagerScreenType_message);

    if (!forced) {
        SvWidget button = svButtonNewFromSM(self->appGlobals->res, "LatensPopupManager.SimplePopup.Button", "OK", 1, svWidgetGetId(parentWindow));
        svWidgetAttach(window, button, (windowWidth - button->width) / 2, windowHeight - button->height - buttonOffsetY, 1);
        svWidgetSetUserEventHandler(parentWindow, QBLatensRemoteMessageScreenUserEventHandler);
        parentWindow->prv = SVRETAIN(screenHandle);
        svWidgetSetFocus(button);
    }

    svSettingsPopComponent();

    SvHashTableInsert(self->screenHandleToScreen, (SvObject) screenHandle, (SvObject) globalWindow);
}

SvLocal void QBLatensPopupManagerDisplayIdOverlay(SvObject self_,
                                                  SvValue screenHandle, SvString channel, unsigned long textColour,
                                                  unsigned long backgroundColour, unsigned char X_Pos,
                                                  unsigned char Y_Pos, unsigned char chRandom,
                                                  unsigned char textTransparency, unsigned char backgroundTransparency,
                                                  unsigned char fontSize, unsigned char covert, SvString text)
{
    QBLatensPopupManager self = (QBLatensPopupManager) self_;

    log_deep("screenHandle = %d, channel = %s, fontSize = %i, covert = %i", SvValueGetInteger(screenHandle),
             channel ? SvStringCString(channel) : "(NULL)", fontSize, covert);

    svSettingsRestoreContext(self->settingsCtx);
    SvWidget window = svWidgetCreateWithColor(self->appGlobals->res, self->appGlobals->res->width, self->appGlobals->res->height, 0);

    SvColor backroundColorRGB = QBLatensPopupManagerGetRecalculatedColor(backgroundColour, backgroundTransparency);

    int windowWidth = svSettingsGetInteger("LatensPopupManager.SimplePopup", "width", 0);
    int windowHeight = svSettingsGetInteger("LatensPopupManager.SimplePopup", "height", 0);
    SvWidget background = svWidgetCreateWithColor(self->appGlobals->res, windowWidth, windowHeight, backroundColorRGB);
    svWidgetAttach(window, background, X_Pos * self->appGlobals->res->width / 100,
                   self->appGlobals->res->height - background->height - Y_Pos * self->appGlobals->res->height / 100, 1);

    SvWidget label = svLabelNewFromSM(self->appGlobals->res, "LatensPopupManager.SimplePopup.Title");
    svLabelSetFontSize(label, QBLatensPopupManagerGetRecalculatedFontSize(self, fontSize));
    svLabelSetText(label, SvStringCString(text));
    SvColor textColorRGB = QBLatensPopupManagerGetRecalculatedColor(textColour, textTransparency);
    svLabelSetTextColor(label, textColorRGB, 0.0);

    svWidgetAttach(background, label, (background->width - label->width) / 2, (background->height - label->height) / 2, 1);
    svSettingsPopComponent();

    QBLatensPopoupManagerHideScreen(self, QBLatensPopupManagerScreenType_overlay);

    QBGlobalWindow globalWindow = (QBGlobalWindow) SvTypeAllocateInstance(QBGlobalWindow_getType(), NULL);
    SvString name = SvStringCreateWithFormat("%d", SvValueGetInteger(screenHandle));
    QBGlobalWindowInit(globalWindow, window, name);
    SVRELEASE(name);
    SvDequePushFront(self->overlayScreenStack, (SvObject) globalWindow, NULL);
    SVRELEASE(globalWindow);

    QBLatensPopupManagerShowScreen(self, QBLatensPopupManagerScreenType_overlay);
    SvHashTableInsert(self->screenHandleToScreen, (SvObject) screenHandle, (SvObject) globalWindow);
}

SvLocal int QBLatensPopupManagerRemoveStateScreen(SvObject self_, SvValue screenHandle)
{
    QBLatensPopupManager self = (QBLatensPopupManager) self_;
    return QBLatensPopoupManagerRemoveScreen(self, QBLatensPopupManagerScreenType_state, screenHandle);
}

SvLocal int QBLatensPopupManagerRemoveRemoteMessageScreen(SvObject self_, SvValue screenHandle)
{
    QBLatensPopupManager self = (QBLatensPopupManager) self_;
    return QBLatensPopoupManagerRemoveScreen(self, QBLatensPopupManagerScreenType_message, screenHandle);
}

SvLocal int QBLatensPopupManagerRemoveIdOverlay(SvObject self_, SvValue screenHandle)
{
    QBLatensPopupManager self = (QBLatensPopupManager) self_;
    return QBLatensPopoupManagerRemoveScreen(self, QBLatensPopupManagerScreenType_overlay, screenHandle);
}

SvLocal void QBLatensPopupManagerDestroy(void *self_)
{
    QBLatensPopupManager self = (QBLatensPopupManager) self_;
    SVRELEASE(self->screenHandleToScreen);
    SVRELEASE(self->stbStateScreenStack);
    SVRELEASE(self->remoteMessageScreenStack);
    SVRELEASE(self->overlayScreenStack);
}

SvLocal SvType QBLatensPopupManager_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBLatensPopupManagerDestroy
    };

    static const struct QBCASPopupManager_ popupMethods = {
        .playbackEvent      = QBLatensPopupManagerPlaybackEvent,
        .contentChanged     = QBLatensPopupManagerContentChanged,
        .visibilityChanged  = QBLatensPopupManagerVisabilityChanged,
        .start              = QBLatensPopupManagerStart,
        .stop               = QBLatensPopupManagerStop
    };

    static const struct QBLatensOSDDisplayer_i displayerMethods = {
        .displayStateScreen         = QBLatensPopupManagerDisplayStateScreen,
        .displayRemoteMessageScreen = QBLatensPopupManagerDisplayRemoteMessageScreen,
        .displayIdOverlay           = QBLatensPopupManagerDisplayIdOverlay,
        .removeStateScreen          = QBLatensPopupManagerRemoveStateScreen,
        .removeRemoteMessageScreen  = QBLatensPopupManagerRemoveRemoteMessageScreen,
        .removeIdOverlay            = QBLatensPopupManagerRemoveIdOverlay
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBLatensPopupManager",
                            sizeof(struct QBLatensPopupManager_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            QBCASPopupManager_getInterface(), &popupMethods,
                            QBLatensOSDDisplayer_getInterface(), &displayerMethods, NULL);
    }

    return type;
}

QBLatensPopupManager QBLatensPopupManagerCreate(AppGlobals appGlobals)
{
    QBLatensPopupManager self = (QBLatensPopupManager) SvTypeAllocateInstance(QBLatensPopupManager_getType(), NULL);

    self->appGlobals = appGlobals;
    self->screenHandleToScreen = SvHashTableCreate(11, NULL);
    QBLatens latensCAS = (QBLatens) QBLatensGetInstance();
    QBLatensSetOSDDisplayer(latensCAS, (SvObject) self);

    svSettingsPushComponent("LatensPopupManager.settings");
    self->settingsCtx = svSettingsSaveContext();
    self->stbStateScreenStack = SvDequeCreate(NULL);
    self->remoteMessageScreenStack = SvDequeCreate(NULL);
    self->overlayScreenStack = SvDequeCreate(NULL);
    self->fontSizeMultiplier = svSettingsGetInteger("LatensPopupManager.SimplePopup", "fontSizeMultiplier", 1);
    self->fontBaseSize = svSettingsGetInteger("LatensPopupManager.SimplePopup", "fontBaseSize", 21);
    svSettingsPopComponent();

    return self;
}
