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

#include "QBOSKPane.h"
#include <QBConf.h>
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvStringBuffer.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <QBResourceManager/QBResourceManager.h>
#include <QBResourceManager/SvRBLocator.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <QBUTF8Utils.h>
#include <QBOSK/QBOSKParser.h>
#include <QBOSK/QBOSKMap.h>
#include <QBOSK/QBOSKRenderer.h>
#include <QBOSK/QBOSKEvents.h>
#include <SWL/label.h>
#include <SWL/input.h>
#include <Widgets/SideMenu.h>
#include <safeFiles.h>
#include <assert.h>


struct QBOSKPane_t {
    struct SvObject_ super_;
    QBContextMenu contextMenu;
    SvWidget sideMenu;

    int level;
    size_t maxChars;

    SvWidget osk;
    QBOSKRenderer oskRenderer;
    QBOSKMap oskMap;
    SvString oskLayoutID;
    SvStringBuffer oskBuffer;
    SvWidget title;
    SvWidget input;
    SvWidget frame;

    bool shown;

    unsigned int settingsContext;

    QBOSKPaneKeyTyped callback;
    void *callbackData;

    SvString widgetName;
    SvString titleName;
    SvString inputName;

    SvString oskLastLayoutPath;

    int frameWidth;
};

SvLocal void QBOSKPaneDestroy(void *self_)
{
    QBOSKPane self = self_;

    if (self->frame) {
        svWidgetDetach(self->frame);
        svWidgetDestroy(self->frame);
    }

    QBOSKRendererStop(self->oskRenderer, NULL);
    SVRELEASE(self->oskRenderer);
    SVRELEASE(self->oskMap);
    SVTESTRELEASE(self->oskLayoutID);
    SVRELEASE(self->oskBuffer);
    SVRELEASE(self->oskLastLayoutPath);

    SVRELEASE(self->widgetName);
    SVRELEASE(self->titleName);
    SVRELEASE(self->inputName);
}

SvLocal void QBOSKPaneShow(SvGenericObject self_)
{
    QBOSKPane self = (QBOSKPane) self_;
    if(self->shown)
        return;

    self->shown = true;

    svSettingsRestoreContext(self->settingsContext);
    SvWidget frame = SideMenuGetFrame(self->sideMenu, self->level);
    svWidgetAttach(frame, self->frame, 0, 0, 0);
    int newWidth = svSettingsGetInteger(SvStringCString(self->widgetName), "boxWidth", 0);
    self->frameWidth = SideMenuGetFrameWidth(self->sideMenu, self->level);
    SideMenuSetFrameWidth(self->sideMenu, self->level, newWidth, false);
    svSettingsPopComponent();
    QBContextMenuSetDepth(self->contextMenu, self->level, false);
}

SvLocal void QBOSKPaneHide(SvGenericObject self_, bool immediately)
{
    QBOSKPane self = (QBOSKPane) self_;
    if(!self->shown)
        return;
    self->shown = false;
    svWidgetDetach(self->frame);
    SideMenuSetFrameWidth(self->sideMenu, self->level, self->frameWidth, immediately);

    QBContextMenuSetDepth(self->contextMenu, self->level - 1, immediately);
}

SvLocal void QBOSKPaneSetActive(SvGenericObject self_)
{
    QBOSKPane self = (QBOSKPane) self_;
    svWidgetSetFocus(self->osk);
}

SvLocal bool QBOSKPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvLocal void QBOSKPaneSaveLastLayout(QBOSKPane self)
{
    if (self->oskLayoutID)
        QBBufferToFile(SvStringCString(self->oskLastLayoutPath), SvStringCString(self->oskLayoutID));
}

SvLocal SvString QBOSKPaneGetLastLayout(QBOSKPane self)
{
    char *buffer = NULL;
    QBFileToBuffer(SvStringCString(self->oskLastLayoutPath), &buffer);
    SvString oskLayoutID = SvStringCreate(buffer, NULL);
    free(buffer);
    return oskLayoutID;
}

SvLocal void QBOSKPaneHandlePeerEvent(SvObject self_, QBPeerEvent event, SvObject sender);

SvType QBOSKPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBOSKPaneDestroy
    };
    static const struct QBContextMenuPane_ paneMethods = {
        .show             = QBOSKPaneShow,
        .hide             = QBOSKPaneHide,
        .setActive        = QBOSKPaneSetActive,
        .handleInputEvent = QBOSKPaneHandleInputEvent
    };
    static const struct QBPeerEventReceiver_ eventReceiverMethods = {
        .handleEvent = QBOSKPaneHandlePeerEvent
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBOSKPane",
                            sizeof(struct QBOSKPane_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextMenuPane_getInterface(), &paneMethods,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBOSKPaneUpdateTitle(QBOSKPane self)
{
    if (!self->input)
        return;
    SvString val = SvStringBufferCreateContentsString(self->oskBuffer, NULL);
    if(val) {
        svInputSetText(self->input, SvStringCString(val));
        SVRELEASE(val);
    }
}

SvLocal bool
QBOSKPaneInputEventHandler(SvWidget w, SvInputEvent ev)
{
    QBOSKPane self = w->prv;
    return QBContextMenuHandleInput(self->contextMenu, (SvGenericObject) self, ev);
}

SvLocal void QBOSKPaneKeyTypeDefault(QBOSKPane self, QBOSKKey key, int layout)
{
    if (layout < 0)
        return;

    SvString val = key->variants[layout].value;
    if (val) {
        SvString buffer = SvStringBufferCreateContentsString(self->oskBuffer, NULL);
        if ((ssize_t) self->maxChars > QBUTF8StringGetLength(SvStringCString(buffer))) {
            SvStringBufferAppendCString(self->oskBuffer, SvStringCString(val), NULL);
            QBOSKPaneUpdateTitle(self);
        }
        SVRELEASE(buffer);
    }
}

SvLocal void QBOSKPaneKeyTypeBackspace(QBOSKPane self)
{
    if (SvStringBufferLength(self->oskBuffer) > 0) {
        SvString val = SvStringBufferCreateContentsString(self->oskBuffer, NULL);
        const char *cstr = SvStringCString(val);
        const char *prev = NULL;
        for (; *cstr; cstr += QBUTF8StringGetCharLength(cstr))
            prev = cstr;
        SvStringBufferReset(self->oskBuffer);
        SvStringBufferAppendCStringWithLength(self->oskBuffer, SvStringCString(val), SvStringLength(val) - QBUTF8StringGetCharLength(prev), NULL);
        SVRELEASE(val);
        QBOSKPaneUpdateTitle(self);
    }
}

SvLocal void QBOSKPaneKeyTypeLayoutCyclic(QBOSKPane self, QBOSKKey key)
{
    SvImmutableArray layoutArray = QBOSKMapGetLayouts(self->oskMap);
    int size = SvImmutableArrayCount(layoutArray);
    int layoutIndex = QBOSKMapFindLayout(self->oskMap, self->oskLayoutID);
    assert(layoutIndex != -1);
    SvString strID = (SvString) SvImmutableArrayObjectAtIndex(layoutArray,(layoutIndex + 1) % size);
    SVRELEASE(self->oskLayoutID);
    self->oskLayoutID = strID;
    SVRETAIN(self->oskLayoutID);
    QBOSKSelectLayout(self->osk, self->oskLayoutID, NULL);
    QBOSKPaneSaveLastLayout(self);
}

SvLocal void QBOSKPaneHandlePeerEvent(SvObject self_, QBPeerEvent event_, SvObject sender)
{
    QBOSKPane self = (QBOSKPane) self_;

    if (unlikely(!self->osk))
        return;

    if (!SvObjectIsInstanceOf((SvObject) event_, QBOSKKeyPressedEvent_getType()))
        return;

    QBOSKKeyPressedEvent event = (QBOSKKeyPressedEvent) event_;
    QBOSKKey key = event->super_.key;
    if (!key)
        return;

    unsigned int layout = QBOSKMapFindLayout(self->oskMap, self->oskLayoutID);

    if (key->type == QBOSKKeyType_default) {
        QBOSKPaneKeyTypeDefault(self, key, layout);
    } else if (key->type == QBOSKKeyType_backspace) {
        QBOSKPaneKeyTypeBackspace(self);
    } else if (key->type == QBOSKKeyType_layout) {
        QBOSKPaneKeyTypeLayoutCyclic(self, key);
    } else if (key->type == QBOSKKeyType_defaultWithLayout) {
        QBOSKPaneKeyTypeDefault(self, key, event->super_.selectedKeyLayout);
    }

    if (self->callback) {
        SvString val = SvStringBufferCreateContentsString(self->oskBuffer, NULL);
        self->callback(self->callbackData, self, val, layout, key);
        SVRELEASE(val);
    }
}

SvLocal void QBOSKPaneClean(SvApplication app, void *self_)
{
    QBOSKPane self = self_;
    self->frame = NULL;
}

QBOSKPane QBOSKPaneCreateFromSettings(const char *settings, SvScheduler scheduler, QBContextMenu ctxMenu, unsigned int level, SvString widgetName, QBOSKPaneKeyTyped callback, void *callbackData)
{
    svSettingsPushComponent(settings);
    QBOSKPane self = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
    SvErrorInfo error = NULL;
    QBOSKPaneInit(self, scheduler, ctxMenu, level, widgetName, callback, callbackData, &error);
    svSettingsPopComponent();
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        SVRELEASE(self);
        self = NULL;
    }

    return self;
}

void QBOSKPaneInit(QBOSKPane self, SvScheduler scheduler, QBContextMenu ctxMenu, unsigned int level, SvString widgetName, QBOSKPaneKeyTyped callback, void *callbackData, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    SvString OSKPaneFile = SvStringCreate("OSKPane.oskmap", NULL);
    QBOSKPaneInitLayout(self, scheduler, ctxMenu, SvStringCString(OSKPaneFile), level, widgetName, callback, callbackData, &error);
    SVRELEASE(OSKPaneFile);

    SvErrorInfoPropagate(error, errorOut);
}

void QBOSKPaneInitLayout(QBOSKPane self, SvScheduler scheduler, QBContextMenu ctxMenu, const char *file, unsigned int level, SvString widgetName, QBOSKPaneKeyTyped callback, void *callbackData, SvErrorInfo *errorOut)
{
    SvApplication app = ctxMenu->sideMenu->app;

    self->contextMenu = ctxMenu;
    self->sideMenu = ctxMenu->sideMenu;
    self->level = level;
    self->widgetName = SVRETAIN(widgetName);
    self->titleName = SvStringCreateWithFormat("%s.Title", SvStringCString(widgetName));
    self->inputName = SvStringCreateWithFormat("%s.Input", SvStringCString(widgetName));

    self->oskRenderer = QBOSKRendererCreate(NULL);
    self->oskBuffer = SvStringBufferCreate(NULL);

    self->oskLastLayoutPath = SvStringCreate("/etc/vod/app/oskLastLayout", NULL);

    //const char *file = "OSKPane.oskmap";
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBResourceManager resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));
    SvRBLocator locator = QBResourceManagerGetResourceLocator(resourceManager);
    char *mapFilePath = SvRBLocatorFindFile(locator, file);
    if (!mapFilePath) {
        SvLogError("%s: Cannot find language dedicated map xml file", __func__);
        mapFilePath = SvRBLocatorFindFile(locator, "OSKPane.oskmap");
        if (!mapFilePath) {
            SvLogError("%s: Cannot find default map xml file", __func__);
            abort();
        }
    }
    self->oskMap = QBOSKMapCreateFromFile(mapFilePath, NULL);
    if (!self->oskMap) {
        free(mapFilePath);
        mapFilePath = SvRBLocatorFindFile(locator, "OSKPane.oskmap");
        self->oskMap = QBOSKMapCreateFromFile(mapFilePath, NULL);
    }
    free(mapFilePath);

    self->osk = QBOSKNew(app, SvStringCString(widgetName), NULL);

    SvString defaultLayoutID = QBOSKMapGetDefaultLayoutID(self->oskMap);
    self->oskLayoutID = SVTESTRETAIN(defaultLayoutID);

    if (!self->oskLayoutID) {
        self->oskLayoutID = QBOSKPaneGetLastLayout(self);
        if (!self->oskLayoutID || QBOSKMapFindLayout(self->oskMap, self->oskLayoutID) == -1) {
            const char *oskLayoutIDStr = QBConfigGet("LANG");
            SVTESTRELEASE(self->oskLayoutID);
            self->oskLayoutID = oskLayoutIDStr ? SvStringCreate(oskLayoutIDStr, NULL) : NULL;
            SvArray availableLayouts = QBOSKMapGetLayouts(self->oskMap);
            if (self->oskLayoutID && availableLayouts && !SvImmutableArrayContainsObject(availableLayouts, (SvGenericObject) self->oskLayoutID)) {
                SVRELEASE(self->oskLayoutID);
                self->oskLayoutID = (SvString) SvImmutableArrayObjectAtIndex(availableLayouts, 0);
                SVTESTRETAIN(self->oskLayoutID);
            } else {
                self->oskLayoutID = SVSTRING("");
            }
            QBOSKPaneSaveLastLayout(self);
        }
    }

    self->settingsContext = svSettingsSaveContext();
    self->callback = callback;
    self->callbackData = callbackData;

    self->frame = svWidgetCreateBitmap(app, app->width, app->height, NULL);
    svWidgetSetInputEventHandler(self->frame, QBOSKPaneInputEventHandler);
    self->frame->clean = QBOSKPaneClean;
    self->frame->prv = self;

    if(svSettingsIsWidgetDefined(SvStringCString(self->inputName))){
        self->input = svInputNewFromSM(app, SvStringCString(self->inputName), SvInputMode_text, svSettingsGetInteger(SvStringCString(widgetName), "inputWidth", 32));
        svSettingsWidgetAttach(self->frame, self->input, SvStringCString(self->inputName), 0);
    }
    if(svSettingsIsWidgetDefined(SvStringCString(self->titleName))){
        self->title = svLabelNewFromSM(app, SvStringCString(self->titleName));
        svSettingsWidgetAttach(self->frame, self->title, SvStringCString(self->titleName), 0);
    }
    svSettingsWidgetAttach(self->frame, self->osk, SvStringCString(self->widgetName), 0);

    SvErrorInfo error = NULL;
    QBOSKSetKeyboardMap(self->osk, self->oskRenderer, self->oskMap, self->oskLayoutID, &error);
    if (!error) {
        QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
        QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, QBOSKEvent_getType(), self->osk->prv, NULL);
        svWidgetSetFocusable(self->osk, true);
        self->maxChars = svSettingsGetInteger(SvStringCString(widgetName), "maxChars", INT_MAX);

        QBOSKRendererStart(self->oskRenderer, scheduler, NULL);
    }

    SvErrorInfoPropagate(error, errorOut);
}


SvWidget QBOSKPaneGetTitle(QBOSKPane self)
{
    return self->title;
}

void QBOSKPaneSetRawInput(QBOSKPane self, const char * input)
{
    SvStringBufferReset(self->oskBuffer);
    SvStringBufferAppendCString(self->oskBuffer, input, NULL);
    if (self->input)
        svInputSetText(self->input, input);
}

void QBOSKPaneSetInput(QBOSKPane self, SvString input)
{
    SvStringBufferReset(self->oskBuffer);
    SvStringBufferAppendCString(self->oskBuffer, SvStringCString(input), NULL);
    if (self->input)
        svInputSetText(self->input, SvStringCString(input));
}

void QBOSKPaneSetInputWidget(QBOSKPane self, SvWidget input)
{
    self->input = input;
    SvStringBufferReset(self->oskBuffer);
    char *text = svInputGetText(input);
    SvStringBufferAppendCString(self->oskBuffer, text, NULL);
    free(text);
}

void
QBOSKPaneSetPasswordMode(QBOSKPane self, bool passwordMode, unsigned int maskCharCode)
{
    svInputSetPasswordMode(self->input, passwordMode, maskCharCode);
}
