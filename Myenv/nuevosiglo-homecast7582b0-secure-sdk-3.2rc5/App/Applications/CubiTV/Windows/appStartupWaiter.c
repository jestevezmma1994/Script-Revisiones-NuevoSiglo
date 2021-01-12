/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connecwion with a service bureau,
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

#include "appStartupWaiter.h"

#include <libintl.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/label.h>
#include <SWL/anim.h>
#include <Services/QBMWConfigMonitor.h>
#include <main.h>
#include <Widgets/customerLogo.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBWidgets/QBDialog.h>
#include <Utils/appType.h>
#include <Logic/AppStartupWaiterLogic.h>
#include <Logic/GUILogic.h>
#include <SvFoundation/SvArray.h>

struct QBAppStartupWaiterContext_t {
    struct QBWindowContext_t super_;
    AppGlobals appGlobals;
    SvArray plugins;
};

struct QBAppStartupWaiterInfo_t {
    SvWidget window;
    SvWidget logo;
    SvWidget animation;
    SvWidget dialog;
    SvWidget dialogLabel;
    SvWidget dialogButton;
    QBAppStartupWaiterContext ctx;
    SvFiber fiber;
    SvFiberTimer timer;
    SvFiber timeoutFiber;
    SvFiberTimer timeoutTimer;
    QBAppStartupWaiterLogic logic;
};
typedef struct QBAppStartupWaiterInfo_t *QBAppStartupWaiterInfo;


SvLocal void QBAppStartupWaiterStopTimeout(QBAppStartupWaiterInfo self);

SvLocal void QBAppStartupWaiterPluginStop(QBAppStartupWaiterInfo self)
{
    if (self->fiber)
        SvFiberDestroy(self->fiber);
    self->fiber = NULL;
    self->timer = NULL;

    // timeout might be not stopped when we exit from application very fast
    if (self->timeoutTimer)
        QBAppStartupWaiterStopTimeout(self);
}

SvLocal void QBAppStartupWaiterStopTimeout(QBAppStartupWaiterInfo self)
{
    if (self->timeoutTimer)
        SvFiberEventDeactivate(self->timeoutTimer);
    if (self->timeoutFiber) {
        SvFiberDeactivate(self->timeoutFiber);
        SvFiberDestroy(self->timeoutFiber);
    }
    self->timeoutFiber = NULL;
    self->timeoutTimer = NULL;
    QBAppStartupWaiterPluginStop(self);
}

#define DEFAULT_FAIL_MESSAGE_TAG    "EmptyChannelList"

SvLocal void QBChannelWaiterTimeoutDialog(QBAppStartupWaiterInfo self)
{
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject view = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
    if (!view) {
        return;
    }

    QBAppStartupWaiterStopTimeout(self);

    if (!self->dialog) {
        return;
    }

    QBDialogSetTitle(self->dialog, gettext("Error"));

    svSettingsPushComponent("AppStartupWaiter.settings");

    const char* message = NULL;
    SvString failMessageTag = QBChannelScanningLogicGetMWTimeoutMessageTag(self->ctx->appGlobals->channelScanningLogic);

    char buffer[64];
    if (failMessageTag) {
        snprintf(buffer, sizeof(buffer), "failMessage_%s", SvStringCString(failMessageTag));
    } else {
        snprintf(buffer, sizeof(buffer), "failMessage_%s", DEFAULT_FAIL_MESSAGE_TAG);
    }
    message = gettext(svSettingsGetString("Dialog", buffer));

    svLabelSetText(self->dialogLabel, message);

    if( self->animation ) {
        QBDialogRemoveItem(self->dialog, SVSTRING("animation"));
        self->animation = 0;
    }

    if( !self->dialogButton ) {
        self->dialogButton = QBDialogAddButton(self->dialog, SVSTRING("button"), gettext("Proceed"), 1);
        svWidgetSetFocus(self->dialogButton);
    }

    svSettingsPopComponent();
}

SvLocal void QBAppStartupWaiterStep(void *self_)
{
    QBAppStartupWaiterInfo self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    int pluginsNumber = SvArrayGetCount(self->ctx->plugins);
    for (int i = pluginsNumber; i; i--) {
        SvObject plugin = SvArrayGetObjectAtIndex(self->ctx->plugins, i - 1);
        if (SvInvokeInterface(QBAppStartupWaiterPlugin, plugin, hasFinished)) {
            SvInvokeInterface(QBAppStartupWaiterPlugin, plugin, stop);
            SvArrayRemoveObjectAtIndex(self->ctx->plugins, i - 1);
        }

    }

    bool switchContext = false;
    if (!SvArrayGetCount(self->ctx->plugins)) {
        if (QBAppTypeIsDVB() || QBAppTypeIsHybrid() || QBAppTypeIsSAT()) {
            switchContext = true;
        } else if (!self->ctx->appGlobals->qbMWConfigMonitor || QBMWConfigMonitorHasConfiguration(self->ctx->appGlobals->qbMWConfigMonitor)) {
            switchContext = true;
        }
    }

    if (switchContext) {
        QBAppStartupWaiterStopTimeout(self);
        if (self->dialog) {
            QBDialogBreak(self->dialog);
            self->dialog = NULL;
        }
        QBApplicationControllerPopContext(self->ctx->appGlobals->controller);
    } else {
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(250));
    }
}

SvLocal bool QBAppStartupWaiterNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBAppStartupWaiterInfo cwi = ptr;
    AppGlobals appGlobals = cwi->ctx->appGlobals;

    if (cwi->dialog ) {
        if (!buttonTag)
            return false;
        QBDialogBreak(cwi->dialog);
        cwi->dialog = NULL;

        QBApplicationControllerPopContext(appGlobals->controller);
        return true;
   }
   return false;
}

SvLocal bool QBAppStartupWaiterInputEventHandler(SvWidget w, SvInputEvent ev)
{
    QBAppStartupWaiterInfo cwi = w->prv;
    return QBAppStartupWaiterLogicInputHandler(cwi->logic, ev);
}

SvLocal void QBAppStartupWaiterSetupWaitingDialog(QBAppStartupWaiterInfo cwi)
{
    const char *dialogWidgetName = "Dialog";

    svSettingsPushComponent("AppStartupWaiter.settings");

    if (!svSettingsIsWidgetDefined(dialogWidgetName)) {
        svSettingsPopComponent();
        return;
    }

    AppGlobals appGlobals = cwi->ctx->appGlobals;
    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = dialogWidgetName,
        .ownerId    = svWidgetGetId(cwi->window),
    };
    SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    QBDialogSetTitle(dialog, gettext(svSettingsGetString(dialogWidgetName, "title")));

    QBDialogSetNotificationCallback(dialog, cwi, QBAppStartupWaiterNotificationCallback);
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);

    cwi->dialogLabel = QBDialogAddLabel(dialog, SVSTRING("content"), gettext(svSettingsGetString(dialogWidgetName, "msg")), SVSTRING("label"), 2);

    SvWidget animation = svSettingsWidgetCreate(appGlobals->res, "Animation");
    float duration = svSettingsGetDouble("Animation", "stepDuration", 0.2f);
    int bmpCnt = svSettingsGetInteger("Animation", "steps", 0);
    if(bmpCnt > 0) {
        SvBitmap* bitmaps = calloc(bmpCnt, sizeof(SvBitmap));
        int i;
        for(i = 0; i < bmpCnt; i++) {
            char *frameName;
            asprintf(&frameName, "frame%i", i);
            bitmaps[i] = SVRETAIN(svSettingsGetBitmap("Animation", frameName));
            free(frameName);
        }
        SvEffect effect = svEffectAnimNew(animation, bitmaps, bmpCnt, SV_EFFECT_ANIM_FORWARD, SV_ANIM_LOOP_INFINITE, duration);
        for(i = 0; i < bmpCnt; i++)
            SVRELEASE(bitmaps[i]);
        free(bitmaps);
        svAppRegisterEffect(appGlobals->res, effect);
    }

    QBDialogAddWidget(dialog, SVSTRING("content"), animation, SVSTRING("animation"), 3, "Dialog.Animation");
    cwi->animation = animation;
    cwi->dialog = dialog;

    svSettingsPopComponent();

    QBDialogRun(cwi->dialog, NULL, NULL);
}

SvLocal void QBAppStartupWaiterTimeoutStep(void *self_)
{
    QBAppStartupWaiterInfo self = self_;
    QBAppStartupWaiterStopTimeout(self);

    QBChannelWaiterTimeoutDialog(self);
}



SvLocal void
QBAppStartupWaiterCleanup( SvApplication app, void* ptr )
{
    QBAppStartupWaiterInfo prv = ptr;
    QBAppStartupWaiterPluginStop(prv);
    SVRELEASE(prv->logic);
    if (prv->timer)
        SvFiberEventDeactivate(prv->timer);
    if (prv->fiber)
        SvFiberDeactivate(prv->fiber);
    if (prv->timeoutTimer)
        SvFiberEventDeactivate(prv->timeoutTimer);
    if (prv->timeoutFiber)
        SvFiberDeactivate(prv->timeoutFiber);
    free(ptr);
}


SvLocal void QBAppStartupWaiterContextCreateWindow(QBWindowContext ctx_, SvApplication app)
{
    QBAppStartupWaiterContext ctx = (QBAppStartupWaiterContext) ctx_;
    QBWindowContext wctx = (QBWindowContext) ctx;

    svSettingsPushComponent("AppStartupWaiter.settings");

    SvWidget window = QBGUILogicCreateBackgroundWidget(ctx->appGlobals->guiLogic, "AppStartupWaiterWindow", NULL);

    int channelWaitTime = 0;

    if (!QBInitLogicGetAppStartupWaiterTimeout(ctx->appGlobals->initLogic, &channelWaitTime)) {
        channelWaitTime = svSettingsGetInteger("AppStartupWaiterWindow", "waitTime", 20);
    }

    SvWidget windowTitle = svLabelNewFromSM(app, "AppStartupWaiterWindow.Title");

    svSettingsWidgetAttach(window, windowTitle, "AppStartupWaiterWindow.Title", 1);

    if (svSettingsIsWidgetDefined("AppStartupWaiterWindow.Text")) {
        SvWidget windowText = svLabelNewFromSM(app, "AppStartupWaiterWindow.Text");
        svSettingsWidgetAttach(window, windowText, "AppStartupWaiterWindow.Text", 1);
    }

    bool showLogo = svSettingsGetBoolean("AppStartupWaiterWindow", "showLogo", true);

    svSettingsPopComponent();

    QBAppStartupWaiterInfo self = calloc(1, sizeof(struct QBAppStartupWaiterInfo_t));
    self->ctx = ctx;
    self->window = window;
    self->logic = QBAppStartupWaiterLogicCreate(NULL);
    window->prv = self;
    svWidgetSetInputEventHandler(window, QBAppStartupWaiterInputEventHandler);
    window->clean = QBAppStartupWaiterCleanup;

    wctx->window = window;

    if (showLogo) {
        SvString logoWidgetName = QBInitLogicGetLogoWidgetName(ctx->appGlobals->initLogic);
        if (logoWidgetName)
            self->logo = QBCustomerLogoAttach(ctx->super_.window, logoWidgetName, 1);
    }

    QBAppStartupWaiterSetupWaitingDialog(self);

    self->fiber = SvFiberCreate(SvSchedulerGet(), NULL, "QBAppStartupWaiter", QBAppStartupWaiterStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(250));

    if (channelWaitTime != -1) {
        self->timeoutFiber = SvFiberCreate(SvSchedulerGet(), NULL,
                                           "QBAppStartupWaiterTimeout",
                                           QBAppStartupWaiterTimeoutStep, self);
        SvFiberSetPriority(self->timeoutFiber, 50);
        self->timeoutTimer = SvFiberTimerCreate(self->timeoutFiber);
        SvFiberTimerActivateAfter(self->timeoutTimer, SvTimeFromMs(channelWaitTime * 1000));
    } else {
        self->timeoutFiber = NULL;
        self->timeoutTimer = NULL;
    }

    SvIterator it = SvArrayGetIterator(ctx->plugins);
    SvObject plugin;
    while ((plugin = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBAppStartupWaiterPlugin, plugin, start);
    }
}

SvLocal void QBAppStartupWaiterContextDestroyWindow(QBWindowContext self_)
{
    QBAppStartupWaiterContext self = (QBAppStartupWaiterContext) self_;

    SvIterator it = SvArrayGetIterator(self->plugins);
    SvObject plugin;
    while ((plugin = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBAppStartupWaiterPlugin, plugin, stop);
    }
    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal void QBAppStartupWaiterContextReinitializeWindow(QBWindowContext self_, SvArray requests)
{

}

SvLocal void
QBAppStartupWaiterContextDestroy(void *self_)
{
    QBAppStartupWaiterContext self = self_;

    SVRELEASE(self->plugins);
}

SvLocal SvType QBAppStartupWaiterContext_getType(void)
{
    static SvType type = NULL;
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_        = {
            .destroy   = QBAppStartupWaiterContextDestroy
        },
        .createWindow  = QBAppStartupWaiterContextCreateWindow,
        .destroyWindow = QBAppStartupWaiterContextDestroyWindow,
        .reinitializeWindow = QBAppStartupWaiterContextReinitializeWindow
    };

    if (!type) {
        SvTypeCreateManaged("QBAppStartupWaiterContext",
                            sizeof(struct QBAppStartupWaiterContext_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            NULL);
    }

    return type;
}

QBWindowContext
QBAppStartupWaiterContextCreate(AppGlobals appGlobals)
{
    QBAppStartupWaiterContext ctx = (QBAppStartupWaiterContext)
        SvTypeAllocateInstance(QBAppStartupWaiterContext_getType(), NULL);

    ctx->appGlobals = appGlobals;
    ctx->plugins = SvArrayCreate(NULL);

    return (QBWindowContext) ctx;
}

void QBAppStartupWaiterContextAddPlugin(QBAppStartupWaiterContext self,
                                   SvGenericObject plugin)
{
    SvArrayAddObject(self->plugins, plugin);
}

SvInterface
QBAppStartupWaiterPlugin_getInterface(void)
{
   static SvInterface interface = NULL;
   SvErrorInfo error = NULL;

   if (!interface) {
      SvInterfaceCreateManaged("QBAppStartupWaiterPlugin",
                               sizeof(struct QBAppStartupWaiterPlugin_),
                               NULL, &interface, &error);
      if (error) {
         SvErrorInfoWriteLogMessage(error);
         SvErrorInfoDestroy(error);
         abort();
      }
   }

   return interface;
}
