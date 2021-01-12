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

#include "QBTransponderEditPane.h"

#include <CUIT/Core/app.h>
#include <CUIT/Core/effect.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <ContextMenus/QBContainerPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <main_decl.h>
#include <QBDVBSatelliteDescriptor.h>
#include <Services/core/QBTextRenderer.h>
#include <SWL/button.h>
#include <sv_tuner.h> // IWYU pragma: keep
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBPeerEvent.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <Utils/translations.h>
#include <Widgets/QBFrequencyInput.h>
#include <Widgets/QBNumericInput.h>
#include <QBWidgets/QBComboBox.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <assert.h>
#include <libintl.h>
#include <main.h>
#include <settings.h>
#include <stdio.h>
#include <stdlib.h>


struct QBTransponderEditPane_ {
    struct SvObject_ super_;

    SvApplication app;

    QBContainerPane container;

    int level;
    int settingsCtx;

    struct QBTunerProperties tunerProps;

    QBBasicTransponderData transponderData;

    SvWidget frame;

    struct {
        SvWidget freq;
        SvWidget symbolRate;
        SvWidget polarization;
        SvWidget modulation;
    } inputs;
    SvWidget saveBtn;

    struct {
        QBTransponderEditCallback fun;
        void *data;
    } callback;
};

// ------------------------- QBPolarizationInput ------------------------------
SvLocal SvString
QBPolarizationInputPrepare(void *self_, SvWidget combobox, SvGenericObject value)
{
    int val = SvValueGetInteger((SvValue) value);
    switch (val) {
        case QBTunerPolarization_Vertical:
            return SvStringCreate(gettext("Vertical"), NULL);
        case QBTunerPolarization_Horizontal:
            return SvStringCreate(gettext("Horizontal"), NULL);
        default:
            return SvStringCreate(gettext("Any"), NULL);
    }
}

SvLocal SvWidget
QBPolarizationInputCreate(SvApplication app,
                          const char *widgetName,
                          void *prv,
                          SvGenericObject initial)
{
    SvWidget box = QBComboBoxNewFromSM(app, widgetName);
    QBComboBoxCallbacks cb = { QBPolarizationInputPrepare, NULL, NULL, NULL, NULL, NULL };
    QBComboBoxSetCallbacks(box, NULL, cb);

    struct QBTunerProperties *tunerProps = prv;
    SvArray values = SvArrayCreateWithCapacity(2, NULL);

    for (int i = 0; i < tunerProps->allowed.polarization_cnt; ++i) {
        SvValue value = SvValueCreateWithInteger(tunerProps->allowed.polarization_tab[i], NULL);
        SvArrayAddObject(values, (SvGenericObject) value);
        SVRELEASE(value);
    }

    QBComboBoxSetContent(box, values);
    SVRELEASE(values);

    if (initial) {
        int idx = SvArrayIndexOfObject(values, (SvGenericObject) initial);
        QBComboBoxSetPosition(box, idx);
    }

    return box;
}

// -------------------------- QBModulationInput -------------------------------

SvLocal SvString
QBModulationInputPrepare(void *self_, SvWidget combobox, SvGenericObject value)
{
    int val = SvValueGetInteger((SvValue) value);
    return SvStringCreate(QBTunerModulationToTranslatedString(val), NULL);
}

SvLocal SvWidget
QBModulationInputCreate(SvApplication app,
                        const char *widgetName,
                        void *prv,
                        SvGenericObject initial)
{
    SvWidget box = QBComboBoxNewFromSM(app, widgetName);
    QBComboBoxCallbacks cb = { QBModulationInputPrepare, NULL, NULL, NULL, NULL, NULL };
    QBComboBoxSetCallbacks(box, prv, cb);

    struct QBTunerProperties *tunerProps = prv;
    SvArray values = SvArrayCreateWithCapacity(3, NULL);

    for (int i = 0; i < tunerProps->allowed.modulation_cnt; ++i) {
        SvValue value = SvValueCreateWithInteger(tunerProps->allowed.modulation_tab[i], NULL);
        SvArrayAddObject(values, (SvGenericObject) value);
        SVRELEASE(value);
    }

    QBComboBoxSetContent(box, values);
    SVRELEASE(values);

    if (initial) {
        int idx = SvArrayIndexOfObject(values, initial);
        QBComboBoxSetPosition(box, idx);
    }

    return box;
}

typedef SvWidget (*QBTransponderEditInputConstructor)(SvApplication app,
                                                      const char *widgetName,
                                                      void *prv,
                                                      SvGenericObject initial);

SvLocal SvWidget
createInput(SvApplication app,
            const char *widgetName,
            SvWidget *input,
            QBTransponderEditInputConstructor inputConstructor,
            void *prv,
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
    w = inputConstructor(app, name, prv, initialValue);
    svSettingsWidgetAttach(container, w, svWidgetGetName(w), 1);

    if (input)
        *input = w;

    return container;
}

SvLocal QBBasicTransponderData
QBBasicTransponderDataCreate(const struct QBTunerProperties *tunerProps)
{
    QBBasicTransponderData transponderData = calloc(sizeof(struct QBBasicTransponderData_), 1);
    if (!transponderData)
        return NULL;

    transponderData->freq = tunerProps->allowed.freq_min;
    transponderData->symbolRate = tunerProps->allowed.symbol_rate_min;
    transponderData->polarization = QBTunerPolarization_Vertical;
    transponderData->modulation = QBTunerModulation_QPSK;

    return transponderData;
}

SvLocal QBBasicTransponderData
QBBasicTransponderDataCreateFromTransponder(QBDVBSatelliteTransponder editedTransponder)
{
    QBBasicTransponderData transponderData = calloc(sizeof(struct QBBasicTransponderData_), 1);
    if (!transponderData)
        return NULL;

    transponderData->freq = editedTransponder->desc.mux_id.freq;
    transponderData->symbolRate = editedTransponder->desc.symbol_rate;
    transponderData->polarization = editedTransponder->desc.polarization;
    transponderData->modulation = editedTransponder->desc.modulation;

    return transponderData;
}

SvLocal void
QBBasicTransponderDataDestroy(QBBasicTransponderData self)
{
    free(self);
}

// ------------------------- QBTransponderEditPane ----------------------------

SvLocal void
QBTransponderEditPaneContainerOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBTransponderEditPane self = self_;

    svWidgetAttach(frame, self->frame, 0, 0, 0);
}

SvLocal void
QBTransponderEditPaneContainerOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBTransponderEditPane self = self_;

    svWidgetDetach(self->frame);
}

SvLocal void
QBTransponderEditPaneContainerSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBTransponderEditPane self = self_;

    svWidgetSetFocus(QBFrequencyInputGetWidget(self->inputs.freq));
}

SvLocal void
QBTransponderEditPaneHandlePeerEvent(SvObject self_, QBPeerEvent event, SvObject sender)
{
    QBTransponderEditPane self = (QBTransponderEditPane) self_;

    if (SvObjectIsInstanceOf((SvObject) event, SvButtonPushedEvent_getType()) && sender == self->saveBtn->prv) {
        SvValue polarization = (SvValue) QBComboBoxGetValue(self->inputs.polarization);
        SvValue modulation = (SvValue) QBComboBoxGetValue(self->inputs.modulation);

        self->transponderData->freq = QBFrequencyInputGetValue(self->inputs.freq);
        self->transponderData->symbolRate = QBNumericInputGetValue(self->inputs.symbolRate);
        self->transponderData->polarization = SvValueGetInteger(polarization);
        self->transponderData->modulation = SvValueGetInteger(modulation);

        self->callback.fun(self->callback.data, self, self->transponderData);

        return;
    }
}

SvLocal void
QBTransponderEditPaneFakeClean(SvApplication app, void *prv)
{
}

SvLocal void
QBTransponderEditPaneShow(SvGenericObject self_)
{
    QBTransponderEditPane self = (QBTransponderEditPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->container, show);
}

SvLocal void
QBTransponderEditPaneHide(SvGenericObject self_, bool immediately)
{
    QBTransponderEditPane self = (QBTransponderEditPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->container, hide, immediately);
}

SvLocal void
QBTransponderEditPaneSetActive(SvGenericObject self_)
{
    QBTransponderEditPane self = (QBTransponderEditPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->container, setActive);
}

SvLocal bool
QBTransponderEditPaneHandleInputEvent(SvObject self_,
                                      SvObject src,
                                      SvInputEvent e)
{
    return false;
}

SvLocal void
QBTransponderEditPaneDestroy(void *self_)
{
    QBTransponderEditPane self = self_;

    SVRELEASE(self->container);
    svWidgetDestroy(self->frame);

    QBBasicTransponderDataDestroy(self->transponderData);
}

SvLocal SvType
QBTransponderEditPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTransponderEditPaneDestroy
    };
    static const struct QBPeerEventReceiver_ eventReceiverMethods = {
        .handleEvent = QBTransponderEditPaneHandlePeerEvent
    };
    static const struct QBContextMenuPane_ paneMethods = {
        .show             = QBTransponderEditPaneShow,
        .hide             = QBTransponderEditPaneHide,
        .setActive        = QBTransponderEditPaneSetActive,
        .handleInputEvent = QBTransponderEditPaneHandleInputEvent
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBTransponderEditPane",
                            sizeof(struct QBTransponderEditPane_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            QBContextMenuPane_getInterface(), &paneMethods,
                            NULL);
    }

    return type;
}

QBTransponderEditPane
QBTransponderEditPaneCreate(QBContextMenu ctxMenu,
                            SvApplication app,
                            int level,
                            QBDVBSatelliteTransponder editedTransponder,
                            QBTransponderEditCallback callback,
                            void *callbackData,
                            SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    static struct QBContainerPaneCallbacks_t newTransponderCallbacks = {
        .onShow    = QBTransponderEditPaneContainerOnShow,
        .onHide    = QBTransponderEditPaneContainerOnHide,
        .setActive = QBTransponderEditPaneContainerSetActive,
    };

    QBTransponderEditPane self = (QBTransponderEditPane)
                                 SvTypeAllocateInstance(QBTransponderEditPane_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBTransponderEditPane");
        goto out;
    }

    self->app = app;
    self->settingsCtx = svSettingsSaveContext();
    self->level = level;
    self->callback.fun = callback;
    self->callback.data = callbackData;

    self->frame = svSettingsWidgetCreate(app, "TransponderEdit");
    self->frame->prv = self;
    self->frame->clean = QBTransponderEditPaneFakeClean;

    sv_tuner_get_properties(0, &self->tunerProps);  //all tuners are the same
    self->transponderData = editedTransponder ? QBBasicTransponderDataCreateFromTransponder(editedTransponder)
                                              : QBBasicTransponderDataCreate(&self->tunerProps);
    QBTextRenderer textRenderer = (QBTextRenderer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTextRenderer"));
    SvWidget w = NULL;
    assert(self->transponderData);

    {
        w = QBAsyncLabelNew(app, "TransponderEdit.Frequency.Description", textRenderer);
        svSettingsWidgetAttach(self->frame, w, svWidgetGetName(w), 1);

        w = QBFrequencyInputNewFromSM(app, "TransponderEdit.Frequency.Input");
        svSettingsWidgetAttach(self->frame, w, svWidgetGetName(w), 10);
        self->inputs.freq = w;
        QBFrequencyInputSetParameters(self->inputs.freq, NULL, NULL,
                                      self->transponderData->freq, 1);
    }

    {
        w = QBAsyncLabelNew(app, "TransponderEdit.SymbolRate.Description", textRenderer);
        svSettingsWidgetAttach(self->frame, w, svWidgetGetName(w), 1);
        w = QBNumericInputNewFromSM(app, "TransponderEdit.SymbolRate.Input");
        svSettingsWidgetAttach(self->frame, w, svWidgetGetName(w), 1);
        self->inputs.symbolRate = w;
        QBNumericInputSetValueAndRange(self->inputs.symbolRate,
                                       self->transponderData->symbolRate,
                                       self->tunerProps.allowed.symbol_rate_min,
                                       self->tunerProps.allowed.symbol_rate_max);
    }

    {
        SvValue initialPolarization = SvValueCreateWithInteger(self->transponderData->polarization, NULL);
        w = createInput(app, "TransponderEdit.Polarization",
                        &self->inputs.polarization, QBPolarizationInputCreate, &self->tunerProps,
                        (SvGenericObject) initialPolarization, textRenderer);
        svSettingsWidgetAttach(self->frame, w, svWidgetGetName(w), 1);
        SVRELEASE(initialPolarization);
    }

    {
        SvValue initialModulation = SvValueCreateWithInteger(self->transponderData->modulation, NULL);
        w = createInput(app, "TransponderEdit.Modulation",
                        &self->inputs.modulation, QBModulationInputCreate, &self->tunerProps,
                        (SvGenericObject) initialModulation, textRenderer);
        svSettingsWidgetAttach(self->frame, w, svWidgetGetName(w), 1);
        SVRELEASE(initialModulation);
    }

    self->saveBtn = svButtonNewFromSM(app, "TransponderEdit.SaveButton", NULL, 0, svWidgetGetId(self->frame));
    svSettingsWidgetAttach(self->frame, self->saveBtn, svWidgetGetName(self->saveBtn), 1);
    QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
    QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, SvButtonPushedEvent_getType(), self->saveBtn->prv, NULL);

    self->container = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    QBContainerPaneInit(self->container, app, ctxMenu, level,
                        SVSTRING("NewTransponderContainer"),
                        &newTransponderCallbacks, self);

out:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}
