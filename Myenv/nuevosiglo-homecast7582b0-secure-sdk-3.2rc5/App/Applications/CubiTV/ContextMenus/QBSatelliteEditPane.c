/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBSatelliteEditPane.h"

#include <CUIT/Core/app.h>
#include <CUIT/Core/effect.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <ContextMenus/QBContainerPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBOSKPane.h>
#include <main_decl.h>
#include <QBDVBSatelliteDescriptor.h>
#include <QBOSK/QBOSK.h>
#include <QBOSK/QBOSKKey.h>
#include <Services/core/QBTextRenderer.h>
#include <SWL/button.h>
#include <SWL/input.h>
#include <SWL/label.h>
#include <QBWidgets/QBComboBox.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBPeerEvent.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <Widgets/QBNumericInput.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <assert.h>
#include <libintl.h>
#include <main.h>
#include <settings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    QBSatelliteEditPaneOSKValidatorStat_OK,
    QBSatelliteEditPaneOSKValidatorStat_EMPTY,
    QBSatelliteEditPaneOSKValidatorStat_EXISTS
} QBSatelliteEditPaneOSKValidatorStat;

struct QBSatelliteEditPane_ {
    struct SvObject_ super_;

    SvApplication app;

    QBContainerPane container;
    QBContextMenu ctxMenu;

    int level;
    int settingsCtx;

    SvArray satellitesList;
    QBDVBSatelliteDescriptor editedSatellite;
    QBBasicSatelliteData satelliteData;

    SvWidget frame;

    struct {
        SvWidget name;
        SvWidget degrees;
        SvWidget hemisphere;
    } inputs;
    SvWidget saveButton;

    struct {
        QBSatelliteEditCallback fun;
        void *data;
    } callback;
};

int QBSatelliteEditPaneGetLevel(QBSatelliteEditPane self)
{
    return self->level;
}

// -------------------------- QBBasicSatelliteData ----------------------------

SvLocal QBBasicSatelliteData
QBBasicSatelliteDataCreate(void)
{
    QBBasicSatelliteData satelliteData = calloc(1, sizeof(QBBasicSatelliteData_));
    if (!satelliteData)
        return NULL;

    satelliteData->deg = -1;
    return satelliteData;
}

SvLocal void
QBBasicSatelliteDataDestroy(QBBasicSatelliteData satelliteData)
{
    SVTESTRELEASE(satelliteData->name);
    SVTESTRELEASE(satelliteData->hemisphere);
    free(satelliteData);
}

SvLocal QBBasicSatelliteData
QBBasicSatelliteDataFromDescriptor(QBDVBSatelliteDescriptor desc)
{
    SvString satelliteName = QBDVBSatelliteDescriptorGetName(desc);
    if (!satelliteName)
        return NULL;

    const char *satelliteNamePtr = SvStringCString(satelliteName);

    // Should be safe cause SvString is always NULL terminated
    char* nameSep = strrchr(satelliteNamePtr, '(');
    if (!nameSep)
        return NULL;

    QBBasicSatelliteData satelliteData = QBBasicSatelliteDataCreate();
    size_t len = nameSep - satelliteNamePtr - 1; // Count space before bracket
    satelliteData->name = SvStringCreateSubString(satelliteName, 0, len, NULL);

    int position = QBDVBSatelliteDescriptorGetPosition(desc);
    if (position <= 1800) {
        satelliteData->hemisphere = SVSTRING("E");
        satelliteData->deg = position;
    } else {
        satelliteData->hemisphere = SVSTRING("W");
        satelliteData->deg = 3600 - position;
    }

    return satelliteData;
}

// -----------------------------QBNameInput -----------------------------------

SvLocal SvWidget
QBNameInputCreate(SvApplication app,
                  const char *widgetName,
                  SvGenericObject initial)
{
    SvWidget input = svInputNewFromSM(app, widgetName, SvInputMode_text, 32);

    if (initial)
        svInputSetText(input, SvStringCString((SvString) initial));

    return input;
}

// ------------------------------ QBHemInput ----------------------------------

SvLocal SvString
QBHemInputComboBoxPrepareVal(void *self_, SvWidget combobox, SvGenericObject value)
{
    return SvStringCreateWithFormat("%s", SvStringCString((SvString) value));
}

SvLocal SvWidget
QBHemInputCreate(SvApplication app, const char *widgetName, SvGenericObject initial)
{
    SvWidget box = QBComboBoxNewFromSM(app, widgetName);
    QBComboBoxCallbacks cb = { QBHemInputComboBoxPrepareVal, NULL, NULL, NULL, NULL, NULL };
    QBComboBoxSetCallbacks(box, box, cb);

    SvArray values = SvArrayCreateWithCapacity(2, NULL);

    SvArrayAddObject(values, (SvGenericObject) SVSTRING("W"));
    SvArrayAddObject(values, (SvGenericObject) SVSTRING("E"));

    QBComboBoxSetContent(box, values);
    SVRELEASE(values);

    if (initial) {
        int idx = SvArrayIndexOfObject(values, initial);
        QBComboBoxSetPosition(box, idx);
    }

    return box;
}

// ----------------------------------------------------------------------------

typedef SvWidget (*QBSatelliteEditInputConstructor)(SvApplication app,
                                                    const char *widgetName,
                                                    SvGenericObject initial);

SvLocal SvWidget
createInput(SvApplication app,
            const char *widgetName,
            SvWidget *input,
            QBSatelliteEditInputConstructor inputConstructor,
            SvGenericObject initialValue,
            QBTextRenderer textRenderer)
{
    SvWidget container = svSettingsWidgetCreate(app, widgetName);

    char name[64] = { 0 };
    SvWidget w = NULL;

    snprintf(name, 64, "%s.desc", widgetName);
    w = QBAsyncLabelNew(app, name, textRenderer);
    svSettingsWidgetAttach(container, w, svWidgetGetName(w), 1);

    snprintf(name, 64, "%s.input", widgetName);
    w = inputConstructor(app, name, initialValue);
    svSettingsWidgetAttach(container, w, svWidgetGetName(w), 1);

    if (input)
        *input = w;

    return container;
}

// ----------------------------------------------------------------------------

SvLocal void
QBSatelliteEditPaneContainerOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBSatelliteEditPane self = self_;

    svWidgetDetach(self->frame);
}

SvLocal void
QBSatelliteEditPaneContainerOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBSatelliteEditPane self = self_;

    svWidgetAttach(frame, self->frame, 0, 0, 0);
}

SvLocal void
QBSatelliteEditPaneContainerSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBSatelliteEditPane self = self_;

    svWidgetSetFocus(self->inputs.name);
}

SvLocal void
QBSatelliteEditPaneContainerOnDestroy(void *self_, QBContainerPane pane)
{
    QBSatelliteEditPane self = self_;

    svWidgetDestroy(self->frame);
    self->frame = NULL;
}

SvLocal QBSatelliteEditPaneOSKValidatorStat
QBSatelliteEditPaneValidateSatelliteName(QBSatelliteEditPane self, SvString newName)
{
    size_t newNameLen = SvStringLength(newName);
    if (newNameLen == 0)
        return QBSatelliteEditPaneOSKValidatorStat_EMPTY;

    if (self->satelliteData->name) {
        bool nameChanged = !SvObjectEquals((SvObject) self->satelliteData->name, (SvObject) newName);
        if (!nameChanged)
            return QBSatelliteEditPaneOSKValidatorStat_OK;
    }

    QBDVBSatelliteDescriptor desc = NULL;
    SvIterator it = SvArrayIterator(self->satellitesList);
    while ((desc = (QBDVBSatelliteDescriptor) SvIteratorGetNext(&it))) {
        SvString name = QBDVBSatelliteDescriptorGetName(desc);
        const char *satelliteNamePtr = SvStringCString(name);
        char* nameSep = strrchr(satelliteNamePtr, '(');
        assert(nameSep != NULL);
        size_t len = nameSep - satelliteNamePtr - 1; // Count space before bracket
        if (newNameLen != len)
            continue;
        if (strncmp(SvStringCString(newName), satelliteNamePtr, len) == 0)
            break;
    }

    if (desc)
        return QBSatelliteEditPaneOSKValidatorStat_EXISTS;

    return QBSatelliteEditPaneOSKValidatorStat_OK;
}


SvLocal void
QBSatelliteEditPaneOSKKeyPressed(void *self_,
                                 QBOSKPane pane,
                                 SvString input,
                                 unsigned int layout,
                                 QBOSKKey key)
{
    QBSatelliteEditPane self = self_;

    bool disableSave = true;

    if (key->type == QBOSKKeyType_enter) {
        SvWidget title = QBOSKPaneGetTitle(pane);
        switch (QBSatelliteEditPaneValidateSatelliteName(self, input)) {
            case QBSatelliteEditPaneOSKValidatorStat_EMPTY:
                svLabelSetText(title, gettext("Satellite name cannot be empty"));
                break;
            case QBSatelliteEditPaneOSKValidatorStat_EXISTS:
                svLabelSetText(title, gettext("Satellite with given name already exists"));
                break;
            case QBSatelliteEditPaneOSKValidatorStat_OK:
                SVTESTRELEASE(self->satelliteData->name);
                self->satelliteData->name = SVRETAIN(input);

                QBContextMenuPopPane(self->ctxMenu);
                disableSave = false;
                break;
        }
    } else if (key->type == QBOSKKeyType_default || key->type == QBOSKKeyType_backspace) {
        SvWidget title = QBOSKPaneGetTitle(pane);
        if (SvStringLength(input) == 0) {
            svLabelSetText(title, gettext("Satellite name cannot be empty"));
        } else {
            svLabelSetText(title, "");
            disableSave = false;
        }
    }
    svButtonSetDisabled(self->saveButton, disableSave);
}

SvLocal void
QBSatelliteEditPaneHandlePeerEvent(SvObject self_, QBPeerEvent event, SvObject sender)
{
    QBSatelliteEditPane self = (QBSatelliteEditPane) self_;

    if (SvObjectIsInstanceOf((SvObject) event, SvInputSelectedEvent_getType()) && sender == self->inputs.name->prv) {
        {
            QBOSKPane osk = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);

            svSettingsRestoreContext(self->settingsCtx);
            SvErrorInfo error = NULL;
            QBOSKPaneInit(osk, SvSchedulerGet(), self->ctxMenu,
                          self->level + 1, SVSTRING("SatelliteEdit.Name.OSK"),
                          QBSatelliteEditPaneOSKKeyPressed, self, &error);
            svSettingsPopComponent();
            if (!error) {
                QBOSKPaneSetInputWidget(osk, self->inputs.name);
                QBContextMenuPushPane(self->ctxMenu, (SvGenericObject) osk);
            } else {
                SvErrorInfoWriteLogMessage(error);
                SvErrorInfoDestroy(error);
            }
            SVRELEASE(osk);

            return;
        }
    }

    if (SvObjectIsInstanceOf((SvObject) event, SvButtonPushedEvent_getType()) && sender == self->saveButton->prv) {
        self->satelliteData->deg = QBNumericInputGetValue(self->inputs.degrees);
        SvString hemisphere = (SvString) QBComboBoxGetValue(self->inputs.hemisphere);
        SVTESTRETAIN(hemisphere);
        SVTESTRELEASE(self->satelliteData->hemisphere);
        self->satelliteData->hemisphere = hemisphere;

        self->callback.fun(self->callback.data, self, self->satelliteData);

        return;
    }
}

SvLocal void
QBSatelliteEditPaneFakeClean(SvApplication app, void *prv)
{
}

SvLocal void
QBSatelliteEditPaneShow(SvGenericObject self_)
{
    QBSatelliteEditPane self = (QBSatelliteEditPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->container, show);
}

SvLocal void
QBSatelliteEditPaneHide(SvGenericObject self_, bool immediately)
{
    QBSatelliteEditPane self = (QBSatelliteEditPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->container, hide, immediately);
}

SvLocal void
QBSatelliteEditPaneSetActive(SvGenericObject self_)
{
    QBSatelliteEditPane self = (QBSatelliteEditPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->container, setActive);
}

SvLocal bool
QBSatelliteEditPaneHandleInputEvent(SvObject self_,
                                    SvObject src,
                                    SvInputEvent e)
{
    return false;
}

SvLocal void
QBSatelliteEditPaneDestroy(void *self_)
{
    QBSatelliteEditPane self = self_;

    SVRELEASE(self->container);
    SVTESTRELEASE(self->editedSatellite);
    SVRELEASE(self->satellitesList);

    QBBasicSatelliteDataDestroy(self->satelliteData);
    svWidgetDestroy(self->frame);
}

SvLocal SvType
QBSatelliteEditPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSatelliteEditPaneDestroy
    };
    static const struct QBPeerEventReceiver_ eventReceiverMethods = {
        .handleEvent = QBSatelliteEditPaneHandlePeerEvent
    };
    static const struct QBContextMenuPane_ paneMethods = {
        .show             = QBSatelliteEditPaneShow,
        .hide             = QBSatelliteEditPaneHide,
        .setActive        = QBSatelliteEditPaneSetActive,
        .handleInputEvent = QBSatelliteEditPaneHandleInputEvent
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBSatelliteEditPane",
                            sizeof(struct QBSatelliteEditPane_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            QBContextMenuPane_getInterface(), &paneMethods,
                            NULL);
    }

    return type;
}

QBSatelliteEditPane
QBSatelliteEditPaneCreate(QBContextMenu ctxMenu,
                          SvApplication app,
                          int level,
                          QBDVBSatelliteDescriptor editedSatellite,
                          SvArray satellitesList,
                          QBSatelliteEditCallback callback,
                          void *callbackData,
                          SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    static struct QBContainerPaneCallbacks_t newSatelliteCallbacks = {
        .onHide    = QBSatelliteEditPaneContainerOnHide,
        .onShow    = QBSatelliteEditPaneContainerOnShow,
        .setActive = QBSatelliteEditPaneContainerSetActive,
        .onDestroy = QBSatelliteEditPaneContainerOnDestroy
    };

    QBSatelliteEditPane self = (QBSatelliteEditPane)
                               SvTypeAllocateInstance(QBSatelliteEditPane_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBSatelliteEditPane");
        goto out;
    }


    self->satelliteData = editedSatellite ? QBBasicSatelliteDataFromDescriptor(editedSatellite)
                                          : QBBasicSatelliteDataCreate();

    if (!self->satelliteData) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't create satelite data");
        SVRELEASE(self);
        self = NULL;
        goto out;

    }

    self->ctxMenu = ctxMenu;
    self->app = app;
    self->settingsCtx = svSettingsSaveContext();
    self->level = level;
    self->editedSatellite = SVTESTRETAIN(editedSatellite);
    self->satellitesList = SVRETAIN(satellitesList);
    self->callback.fun = callback;
    self->callback.data = callbackData;

    self->frame = svSettingsWidgetCreate(app, "SatelliteEdit");
    self->frame->prv = self;
    self->frame->clean = QBSatelliteEditPaneFakeClean;

    QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
    QBTextRenderer textRenderer = (QBTextRenderer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTextRenderer"));

    SvWidget w = NULL;

    {
        SvString defaultVal = self->satelliteData->name ? self->satelliteData->name : NULL;
        w = createInput(app, "SatelliteEdit.Name", &self->inputs.name,
                        QBNameInputCreate, (SvGenericObject) defaultVal,
                        textRenderer);
        svSettingsWidgetAttach(self->frame, w, svWidgetGetName(w), 1);
        svInputSetMode(self->inputs.name, SvInputMode_unknown);
        QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, SvInputSelectedEvent_getType(), self->inputs.name->prv, NULL);
    }

    {
        int initialVal = self->satelliteData->deg != -1 ? self->satelliteData->deg : 0;
        w = QBAsyncLabelNew(app, "SatelliteEdit.Degrees.Description", textRenderer);
        svSettingsWidgetAttach(self->frame, w, svWidgetGetName(w), 1);
        w = QBNumericInputNewFromSM(app, "SatelliteEdit.Degrees.Input");
        QBNumericInputSetValueAndRange(w, initialVal, 0, 1800);
        QBNumericInputSetDecimalPartWidth(w, 1);
        svSettingsWidgetAttach(self->frame, w, svWidgetGetName(w), 1);
        self->inputs.degrees = w;
    }

    {
        SvString defaultVal = self->satelliteData->hemisphere ? self->satelliteData->hemisphere : NULL;
        w = createInput(app, "SatelliteEdit.Hemisphere", &self->inputs.hemisphere,
                        QBHemInputCreate, (SvGenericObject) defaultVal, textRenderer);
        svSettingsWidgetAttach(self->frame, w, svWidgetGetName(w), 1);
    }

    self->saveButton = svButtonNewFromSM(app, "SatelliteEdit.SaveButton", NULL, 0, svWidgetGetId(self->frame));
    svSettingsWidgetAttach(self->frame, self->saveButton, svWidgetGetName(self->saveButton), 1);
    if (!self->satelliteData->name)
        svButtonSetDisabled(self->saveButton, true);
    QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, SvButtonPushedEvent_getType(), self->saveButton->prv, NULL);

    self->container = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    QBContainerPaneInit(self->container, app, ctxMenu, level,
                        SVSTRING("NewSatelliteContainer"),
                        &newSatelliteCallbacks, self);

out:
    assert(error || self);
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}
