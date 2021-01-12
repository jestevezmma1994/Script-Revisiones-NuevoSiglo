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

#include "screensaver.h"

#include <QBConf.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <Widgets/QBAnimatedBackground.h>
#include <Widgets/customerLogo.h>
#include <Windows/mainmenu.h>
#include <main.h>


struct QBScreensaverContext_t {
    struct QBWindowContext_t super_;
    AppGlobals appGlobals;
    bool enabled;
};
typedef struct QBScreensaverContext_t* QBScreensaverContext;

struct QBScreensaver_t {
    SvWidget background;
    QBScreensaverContext ctx;
};
typedef struct QBScreensaver_t* QBScreensaver;
SvLocal bool QBScreensaverInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBScreensaver self = w->prv;

    if (e->ch != 0)
        QBApplicationControllerPopContext(self->ctx->appGlobals->controller);

    return true;
}

SvLocal void QBScreensaverClean(SvApplication app, void *self_)
{
    QBScreensaver self = self_;
    free(self);
}

SvLocal void QBScreensaverContextReinitializeWindow(QBWindowContext self_, SvArray itemList)
{
    QBScreensaver info = self_->window->prv;
    QBAnimatedBackgroundReinitialize(info->background, itemList);
}

SvLocal void QBScreensaverContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBScreensaverContext self = (QBScreensaverContext) self_;
    svSettingsPushComponent("Screensaver.settings");
    SvWidget window = svSettingsWidgetCreate(app, "Screensaver");
    SvWidget background = QBAnimatedBackgroundCreate(app, NULL, QBAnimatedBackgroundType_default, self->appGlobals->initLogic);
    svWidgetAttach(window, background, 0, 0, 1);

    svSettingsPopComponent();

    QBScreensaver info = calloc(1, sizeof(struct QBScreensaver_t));
    info->background = background;
    info->ctx = self;

    window->prv = info;
    window->clean = QBScreensaverClean;
    svWidgetSetInputEventHandler(window, QBScreensaverInputEventHandler);

    self->super_.window = window;
    QBAnimatedBackgroundShow(background);
}

SvLocal void QBScreensaverContextDestroyWindow(QBWindowContext self_)
{
    if (self_->window) {
        svWidgetDestroy(self_->window);
        self_->window = NULL;
    }
}

SvLocal void QBScreensaverContextNoInput(SvGenericObject self_, QBInputWatcher inputWatcher)
{
    QBScreensaverContext self = (QBScreensaverContext) self_;
    if(self->enabled)
        QBApplicationControllerPushContext(self->appGlobals->controller, (QBWindowContext) self);
}

SvLocal void QBScreensaverContextSwitch(SvObject self_, QBWindowContext from, QBWindowContext to)
{
    QBScreensaverContext self = (QBScreensaverContext) self_;
    self->enabled = SvObjectIsInstanceOf((SvObject) to, QBMainMenuContext_getType());
}

SvLocal int QBScreensaverGetDelaySeconds(const char *value)
{
    if(!value)
        return 0;
    char unit = value[strlen(value)-1];
    int seconds = atoi(value);

    if(unit != 'S') {
        seconds *= 60;
    }
    return seconds;
}

SvLocal void QBScreensaverConfigChanged(SvGenericObject self_, const char *key, const char *value)
{
    if(!value)
        return;

    QBScreensaverContext self = (QBScreensaverContext) self_;

    int delay = QBScreensaverGetDelaySeconds(value);
    if(delay == 0) {
        QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, self_);
    } else
        QBInputWatcherAddListener(self->appGlobals->inputWatcher, self_, delay);
}

SvLocal void QBScreensaverContextDestroy(void *self_)
{
}

SvLocal SvType QBScreensaverContext_getType(void)
{
    static SvType type = NULL;

    static const struct QBWindowContextVTable_ contextVTable = {
        .super_             = {
            .destroy        = QBScreensaverContextDestroy,
        },
        .reinitializeWindow = QBScreensaverContextReinitializeWindow,
        .createWindow       = QBScreensaverContextCreateWindow,
        .destroyWindow      = QBScreensaverContextDestroyWindow
    };

    static const struct QBInputWatcherListener_t inputMethods = {
        .tick = QBScreensaverContextNoInput
    };

    static const struct QBContextSwitcherListener_t switchMethods = {
        .started = QBScreensaverContextSwitch,
        .ended = QBScreensaverContextSwitch,
    };

    static const struct QBConfigListener_t configMethods = {
        .changed = QBScreensaverConfigChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBScreensaverContext",
                            sizeof(struct QBScreensaverContext_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            QBInputWatcherListener_getInterface(), &inputMethods,
                            QBContextSwitcherListener_getInterface(), &switchMethods,
                            QBConfigListener_getInterface(), &configMethods,
                            NULL);
    }

    return type;
}

QBWindowContext QBScreensaverContextCreate(AppGlobals appGlobals)
{
    QBScreensaverContext self = (QBScreensaverContext) SvTypeAllocateInstance(QBScreensaverContext_getType(), NULL);
    self->appGlobals = appGlobals;

    QBApplicationControllerAddListener(self->appGlobals->controller, (SvGenericObject) self);
    const char* delayStr = QBConfigGet("SCREENSAVERTIMEOUT");
    int delay = QBScreensaverGetDelaySeconds(delayStr);
    if(delay)
        QBInputWatcherAddListener(self->appGlobals->inputWatcher, (SvGenericObject) self, delay);
    QBConfigAddListener((SvGenericObject) self, "SCREENSAVERTIMEOUT");

    return (QBWindowContext) self;
}
