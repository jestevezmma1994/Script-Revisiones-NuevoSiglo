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

#include "hints.h"

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvValue.h>
#include <fibers/c/fibers.h>
#include <QBInput/QBInputService.h>
#include <SvJSON/SvJSONClassRegistry.h>
#include <SvJSON/SvJSONParse.h>
#include <CUIT/Core/event.h>
#include <SWL/xmlpage.h>
#include <SWL/fade.h>
#include <safeFiles.h>
#include <settings.h>
#include <string.h>

#define log_error(FMT, ...) SvLogError("QBHint :: " FMT , ##__VA_ARGS__)

struct QBHint_t {
    struct SvObject_ super_;
    int displayDuration;    //how long should be displayed
    int displayDelay;       //how long should it wait before being displayed
    SvString type;
    SvString widgetDescription;
};

struct QBHintDisplayer_t {
    struct SvObject_ super_;
    SvScheduler scheduler;
    QBApplicationController controller;
    SvApplication app;

    SvArray currentHints;
    size_t currentHint;

    enum {
        HINT_NONE,
        HINT_WAITING,
        HINT_SHOWN,
    } currentHintState;

    int betweenHintDelay;

    QBGlobalWindow currentHintWindow;

    SvFiber fiber;
    SvFiberTimer timer;

    int windowWidth;
    int windowHeight;
    int hintHeight;
    SvFont font;
    int fontSize;
    SvColor fontColor;
    SvColor bgColor;
    SvBitmap bgImage;

    SvHashTable hintWindows;

    QBInputWatcher inputWatcher;
};

struct QBHintManager_t {
    struct SvObject_ super_;
    QBHintDisplayer currentDisplayer;
    SvArray currentHintsNames;
    SvHashTable hints;
    bool running;
};

struct QBHintWindow_t {
    struct QBGlobalWindow_t super_;
    SvEffectId hideEffectId;
    QBApplicationController controller;
    QBHintDisplayer hintDisplayer;
};
typedef struct QBHintWindow_t* QBHintWindow;

SvLocal void QBHintDisplayerShowHint(QBHintDisplayer self, QBHint hint);
SvLocal void QBHintDisplayerHideHint(QBHintDisplayer self);
SvLocal void QBHintDisplayerRemoveHintWindow(QBHintDisplayer self, QBHintWindow hintWindow);

SvLocal void QBHint__dtor__(void *self_)
{
    QBHint self = self_;
    SVRELEASE(self->type);
    SVRELEASE(self->widgetDescription);
}

SvLocal SvType QBHint_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBHint__dtor__
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBHint",
                            sizeof(struct QBHint_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal void QBHintDisplayerNoInputTick(SvGenericObject self_, QBInputWatcher inputWatcher)
{
    QBHintDisplayer self = (QBHintDisplayer) self_;
    if(self->fiber)
        SvFiberActivate(self->fiber);
}

SvLocal void QBHintDisplayer__dtor__(void *self_)
{
    QBHintDisplayer self = self_;

    if (self->fiber)
        SvFiberDestroy(self->fiber);

    SVRELEASE(self->hintWindows);
    SVRELEASE(self->currentHints);

    SVRELEASE(self->font);
    SVTESTRELEASE(self->bgImage);
    SVRELEASE(self->inputWatcher);
}


SvLocal SvType QBHintDisplayer_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBHintDisplayer__dtor__
    };
    static SvType type = NULL;
    static const struct QBInputWatcherListener_t inputMethods = {
        .tick = QBHintDisplayerNoInputTick,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBHintDisplayer",
                            sizeof(struct QBHintDisplayer_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBInputWatcherListener_getInterface(), &inputMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBHintManager__dtor__(void *self_)
{
    QBHintManager self = self_;
    SVTESTRELEASE(self->currentDisplayer);
    SVRELEASE(self->currentHintsNames);
    SVRELEASE(self->hints);
}

SvType QBHintManager_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBHintManager__dtor__
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBHintManager",
                            sizeof(struct QBHintManager_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal SvType QBHintWindow_getType(void)
{
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBHintWindow",
                            sizeof(struct QBHintWindow_t),
                            QBGlobalWindow_getType(), &type,
                            NULL);
    }
    return type;
}

SvLocal SvGenericObject QBHintCreateFromHashTable(SvGenericObject helper_, SvString className, SvHashTable desc, SvErrorInfo *errorOut)
{
    SvValue displayDuration = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("displayDuration"));
    SvValue displayDelay = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("displayDelay"));
    SvValue type = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("type"));
    SvValue widgetDescription = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("widgetDescription"));

    if (!displayDuration || !SvObjectIsInstanceOf((SvObject) displayDuration, SvValue_getType()) || !SvValueIsInteger(displayDuration)) {
        log_error("displayDuration is wrong");
        return NULL;
    }

    if (!displayDelay || !SvObjectIsInstanceOf((SvObject) displayDelay, SvValue_getType()) || !SvValueIsInteger(displayDelay)) {
        log_error("displayDelay is wrong");
        return NULL;
    }

    if (!type || !SvObjectIsInstanceOf((SvObject) type, SvValue_getType()) || !SvValueIsString(type)) {
        log_error("type is wrong");
        return NULL;
    }

    if (!widgetDescription || !SvObjectIsInstanceOf((SvObject) widgetDescription, SvValue_getType()) || !SvValueIsString(widgetDescription)) {
        log_error("widgetDescription is wrong");
        return NULL;
    }

    QBHint hint = (QBHint) SvTypeAllocateInstance(QBHint_getType(), NULL);
    hint->displayDuration = SvValueGetInteger(displayDuration);
    hint->displayDelay = SvValueGetInteger(displayDelay);
    hint->type = SVRETAIN(SvValueGetString(type));
    hint->widgetDescription = SVRETAIN(SvValueGetString(widgetDescription));

    return (SvGenericObject) hint;
}

SvLocal void QBHintWindowEffectEventHandler(SvWidget w, SvEffectEvent e)
{
    QBHintWindow self = w->prv;
    if(e->id != self->hideEffectId)
        return;

    QBApplicationControllerRemoveGlobalWindow(self->controller, (QBGlobalWindow) self);

    QBHintDisplayerRemoveHintWindow(self->hintDisplayer, self);
}

SvLocal void QBHintWindowClean(SvApplication app, void *self_)
{
}

SvLocal void QBHintDisplayerShowHint(QBHintDisplayer self, QBHint hint)
{
    QBHintWindow hintWindow = (QBHintWindow) SvTypeAllocateInstance(QBHintWindow_getType(), NULL);
    SvWidget window = svWidgetCreateBitmap(self->app, self->windowWidth, self->windowHeight, NULL);
    QBGlobalWindowInit((QBGlobalWindow) hintWindow, window, SVSTRING("Hint"));

    SVTESTRELEASE(self->currentHintWindow);
    self->currentHintWindow = SVRETAIN(hintWindow);

    QBApplicationControllerAddGlobalWindow(self->controller, self->currentHintWindow);

    SvWidget widget = svXmlPageMemoryNew(self->app, SV_XMLPAGE_WIDGET_BG, self->font, self->fontSize, self->fontColor, self->bgColor, self->windowWidth, self->hintHeight, SvStringCString(hint->widgetDescription), SV_XMLPAGE_HORIZONTAL, false);

    if (widget) {
        svWidgetAttach(window, widget, 0, window->height - widget->height, 0);
    }

    hintWindow->controller = self->controller;
    hintWindow->hintDisplayer = self;
    window->prv = hintWindow;
    svWidgetSetEffectEventHandler(window, QBHintWindowEffectEventHandler);
    window->clean = QBHintWindowClean;
    window->tree_alpha = ALPHA_TRANSPARENT;

    SvEffect effect = svEffectFadeNew(hintWindow->super_.window, ALPHA_SOLID, true, 0.2, SV_EFFECT_FADE_LOGARITHM);
    svAppRegisterEffect(self->app, effect);

    SvHashTableInsert(self->hintWindows, (SvGenericObject) hintWindow, (SvGenericObject) hintWindow);
    SVRELEASE(hintWindow);

    QBInputWatcherRemoveListener(self->inputWatcher, (SvGenericObject) self);
}

SvLocal void QBHintDisplayerHideHint(QBHintDisplayer self)
{
    if (self->currentHintWindow) {
        QBHintWindow hintWindow = (QBHintWindow) self->currentHintWindow;
        SvEffect effect = svEffectFadeNew(hintWindow->super_.window, ALPHA_TRANSPARENT, true, 0.2, SV_EFFECT_FADE_LOGARITHM);
        hintWindow->hideEffectId = svAppRegisterEffect(self->app, effect);

        SVRELEASE(self->currentHintWindow);
        self->currentHintWindow = NULL;
    }
}

SvLocal void QBHintDisplayerRemoveHintWindow(QBHintDisplayer self, QBHintWindow hintWindow)
{
    SvHashTableRemove(self->hintWindows, (SvGenericObject) hintWindow);
}

SvLocal void QBHintDisplayerStep(void *self_)
{
    QBHintDisplayer self = self_;

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);


    if (self->currentHint >= SvArrayCount(self->currentHints)) {
        self->currentHintState = HINT_NONE;
        return;
    }


    QBHint hint = (QBHint) SvArrayObjectAtIndex(self->currentHints, self->currentHint);

    if (self->currentHint == 0 && self->currentHintState == HINT_NONE) {
        SvTime lastEventTime = QBInputServiceGetLastEventTime(NULL);
        if (SvTimeToMs(SvTimeSub(SvTimeGet(), lastEventTime)) < hint->displayDelay * 1000) {
            QBInputWatcherAddListener(self->inputWatcher, (SvGenericObject) self, hint->displayDelay);
            return;
        }
    }

    switch (self->currentHintState) {
        case HINT_WAITING:
            QBHintDisplayerShowHint(self, hint);
            if (hint->displayDuration > 0)
                SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(1000 * hint->displayDuration));
            self->currentHintState = HINT_SHOWN;
            return;
        case HINT_SHOWN:
            QBHintDisplayerHideHint(self);
            self->currentHint++;
            SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(1000 * self->betweenHintDelay));
            self->currentHintState = HINT_NONE;
            return;
        case HINT_NONE:
            if(self->currentHint)
                SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(1000 * hint->displayDelay));
            else
                SvFiberActivate(self->fiber);
            self->currentHintState = HINT_WAITING;
            return;
    }
}

SvLocal void QBHintDisplayerStart(QBHintDisplayer self) {
    self->fiber = SvFiberCreate(self->scheduler, NULL, "QBHintDisplayer", QBHintDisplayerStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    SvFiberActivate(self->fiber);

    self->currentHintState = HINT_NONE;
    self->currentHint = 0;

}

SvLocal void QBHintDisplayerStop(QBHintDisplayer self) {
    QBHintDisplayerHideHint(self);
    SvFiberDestroy(self->fiber);
    self->fiber = NULL;

    SvIterator it = SvHashTableKeysIterator(self->hintWindows);
    QBHintWindow hintWindow;
    while((hintWindow = (QBHintWindow) SvIteratorGetNext(&it))) {
        QBApplicationControllerRemoveGlobalWindow(self->controller, (QBGlobalWindow) hintWindow);
    }
}

SvLocal void QBHintDisplayerAddHint(QBHintDisplayer self, QBHint hint) {
    SvArrayAddObject(self->currentHints, (SvGenericObject) hint);
    if (self->fiber)
        SvFiberActivate(self->fiber);
}

SvLocal void QBHintDisplayerReset(QBHintDisplayer self) {
    SvFiberDeactivate(self->fiber);
    QBHintDisplayerHideHint(self);
    SvArrayRemoveAllObjects(self->currentHints);
    self->currentHint = 0;
    self->currentHintState = HINT_NONE;
}

SvLocal QBHintDisplayer QBHintDisplayerCreate(SvApplication app, QBApplicationController controller, SvScheduler scheduler, QBInputWatcher inputWatcher)
{
    QBHintDisplayer self = (QBHintDisplayer) SvTypeAllocateInstance(QBHintDisplayer_getType(), NULL);
    self->controller = controller;
    self->scheduler = scheduler;
    self->betweenHintDelay = 2;
    self->app = app;
    self->currentHints = SvArrayCreate(NULL);
    self->hintWindows = SvHashTableCreate(10, NULL);
    self->inputWatcher = SVRETAIN(inputWatcher);

    svSettingsPushComponent("Hints.settings");

    self->windowWidth = svSettingsGetInteger("Hint", "width", -1);
    self->windowHeight = svSettingsGetInteger("Hint", "height", -1);
    self->font = svSettingsCreateFont("Hint", "font");
    self->fontSize = svSettingsGetInteger("Hint", "fontSize", -1);
    self->fontColor = svSettingsGetColor("Hint", "fontColor", 0xffffffff);
    self->bgColor = svSettingsGetColor("Hint", "bgColor", -1);
    self->bgImage = SVTESTRETAIN(svSettingsGetBitmap("Hint", "bg"));
    self->hintHeight = svSettingsGetInteger("Hint.Widget", "hintHeight", -1);

    svSettingsPopComponent();

    return self;
}

void QBHintManagerLoadHints(QBHintManager self, SvString hintsDatabaseFilename)
{
    char *buffer;
    QBFileToBuffer(SvStringCString(hintsDatabaseFilename), &buffer);
    if (!buffer) {
        SvLogError("unable to load hints file");
        return;
    }

    SvHashTable hintNameToFilename = (SvHashTable) SvJSONParseString(buffer, true, NULL);
    free(buffer);
    if (!hintNameToFilename) {
        SvLogError("unable to parse hints file");
        return;
    }

    SvIterator keys = SvHashTableKeysIterator(hintNameToFilename);
    SvIterator values = SvHashTableValuesIterator(hintNameToFilename);

    SvHashTableInsertObjects(self->hints, &keys, &values);

    SVRELEASE(hintNameToFilename);

    //Handle notification about adding a hint to displayer if someone tried to add it
    //but manager didn't have the hint
}

void QBHintManagerAddHint(QBHintManager self, SvString hintName)
{
    SvArrayAddObject(self->currentHintsNames, (SvGenericObject) hintName);

    if (self->currentDisplayer) {
        QBHint hint = (QBHint) SvHashTableFind(self->hints, (SvGenericObject) hintName);
        if (hint)
            QBHintDisplayerAddHint(self->currentDisplayer, hint);
    }
}

void QBHintManagerSetHintsFromArray(QBHintManager self, SvArray hints)
{
    if (self->currentDisplayer)
        QBHintDisplayerReset(self->currentDisplayer);

    if (self->currentHintsNames)
        SvArrayRemoveAllObjects(self->currentHintsNames);
    else
        self->currentHintsNames = SvArrayCreate(NULL);

    SvIterator it = SvArrayIterator(hints);
    SvString hint;
    while((hint = (SvString) SvIteratorGetNext(&it)))
        QBHintManagerAddHint(self, hint);

}

void QBHintManagerSetHintsFromString(QBHintManager self, SvString hintsStr)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __FUNCTION__);
        return;
    }

    if (self->currentHintsNames)
        SvArrayRemoveAllObjects(self->currentHintsNames);

    if (self->currentDisplayer)
        QBHintDisplayerReset(self->currentDisplayer);

    if (!hintsStr) {
        QBHintManagerAddHint(self, SVSTRING(""));
        return;
    }

    size_t i;
    size_t start = 0;

    const char *hints = SvStringCString(hintsStr);
    for (i = 0; i < strlen(hints); i++) {
        if (hints[i] == ',') {
            if (i != start) {
                SvString tmpHintName = SvStringCreateSubString(hintsStr, start, i - start, NULL);
                QBHintManagerAddHint(self, tmpHintName);
                SVRELEASE(tmpHintName);
            }
            start = i + 1;
        }
    }
    SvString hintName = SvStringCreateSubString(hintsStr, start, i - start, NULL);
    QBHintManagerAddHint(self, hintName);
    SVRELEASE(hintName);
}

QBHintManager QBHintManagerCreate(SvApplication app, QBApplicationController controller, SvScheduler scheduler, QBInputWatcher inputWatcher)
{
    QBHintManager self = (QBHintManager) SvTypeAllocateInstance(QBHintManager_getType(), NULL);
    self->hints = SvHashTableCreate(100, NULL);
    self->currentHintsNames = SvArrayCreate(NULL);
    self->currentDisplayer = QBHintDisplayerCreate(app, controller, scheduler, inputWatcher);

    SvJSONClass helper = SvJSONClassCreate(NULL, QBHintCreateFromHashTable, NULL);
    SvJSONRegisterClassHelper(SVSTRING("QBHint"), (SvGenericObject) helper, NULL);
    SVRELEASE(helper);

    return self;
}

void QBHintManagerStart(QBHintManager self)
{
    self->running = true;
    if (self->currentDisplayer)
        QBHintDisplayerStart(self->currentDisplayer);
}

void QBHintManagerStop(QBHintManager self)
{
    self->running = false;
    if (self->currentDisplayer)
        QBHintDisplayerStop(self->currentDisplayer);
}
