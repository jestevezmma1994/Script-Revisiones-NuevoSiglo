/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "SatelliteSetup.h"

#include <CUIT/Core/app.h>
#include <CUIT/Core/effect.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBSatelliteSetupEditorPane.h>
#include <Hints/hints.h>
#include <Logic/InitLogic.h>
#include <Logic/SatelliteSetupLogic.h>
#include <Logic/TunerLogic.h>
#include <QBApplicationController.h>
#include <QBApplicationControllerTypes.h>
#include <QBConf.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBDVBSatelliteDescriptor.h> // IWYU pragma: keep
#include <QBDVBSatellitesDB.h>
#include <QBDataModel3/QBActiveArray.h>
#include <QBDataModel3/QBListModelListener.h>
#include <QBInput/QBInputCodes.h>
#include <QBInput/QBInputEvent.h>
#include <QBLocalWindowManager.h>
#include <QBTuner.h> // IWYU pragma: keep
#include <QBTunerLNBConfig.h>
#include <QBTunerMonitor.h>
#include <QBTunerTypes.h>
#include <QBWindowContextInputEventHandler.h>
#include <Services/core/QBTextRenderer.h>
#include <SWL/QBFrame.h>
#include <SWL/button.h>
#include <SWL/events.h>
#include <SWL/gauge.h>
#include <SWL/label.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <Widgets/QBTitle.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/customerLogo.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenu.h>
#include <XMB2/XMBTypes.h>
#include <XMB2/XMBVerticalMenu.h>
#include <assert.h>
#include <libintl.h>
#include <main.h>
#include <main_decl.h>
#include <settings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sv_tuner.h>
#include <sys/types.h>
#include <tunerReserver.h>
#include <Logic/GUILogic.h>
#include <QBAppKit/QBGlobalStorage.h>

#define SATELLITE_HINT_DESCRIPTION "satelite_hint"

typedef struct SatelliteSetupContext_ {
    struct QBWindowContext_t super_;

    SvApplication app;
    QBDVBSatellitesDB satellitesDB;
    QBDVBSatelliteDescriptor selectedSatellite;
    bool singleSatelliteMode;

    QBTunerResv *resv;
} *SatelliteSetupContext;

typedef enum {
    SatelliteSetupConfig_satellite,
    SatelliteSetupConfig_lnb,
    SatelliteSetupConfig_toneSwitch,
    SatelliteSetupConfig_diSEqCPort,
    SatelliteSetupConfig_count,
} SatelliteSetupConfigType;

typedef enum {
    SatelliteSetupInfo_transponders,
    SatelliteSetupInfo_freqRange,
    SatelliteSetupInfo_lof,
    SatelliteSetupInfo_count,
} SatelliteSetupInfoType;

typedef struct SatelliteSetupItemController_ {
    struct SvObject_ super_;

    SatelliteSetupConfigType config;
    QBFrameConstructor* focus;
    QBXMBItemConstructor itemConstructor;
    QBTextRenderer textRenderer;
} *SatelliteSetupItemController;

struct SatelliteSetup_ {
    struct SvObject_ super_;

    SatelliteSetupContext ctx;
    SvWidget window;
    SvWidget logo;

    struct {
        SvWidget widget;
        SvArray values;
        int selectedItem;
        int defaultItem;
    } configs[SatelliteSetupConfig_count];

    struct SatelliteSideMenu_t {
        QBDVBSatelliteDescriptor focusedSatellite;
        QBContextMenu ctx;
    } sidemenu;

    struct QBTunerLNBParams LNB;

    SvWidget infos[SatelliteSetupInfo_count];
    SvWidget signalStrength;
    SvWidget signalQuality;

    SvWidget saveBtn;

    int selectedConfig;
    int focusedConfig;

    QBLocalWindow dropDown;
    SvWidgetId dropDownNotify;
    QBActiveArray dropDownDataSource;

    SvWidget xmbMenu;

    QBFrameConstructor* focus;
    QBXMBItemConstructor itemConstructor;
    QBTextRenderer textRenderer;
};
typedef struct SatelliteSetup_ *SatelliteSetup;

SvLocal SvType
SatelliteSetupItemController_getType(void);

SvLocal SatelliteSetupItemController
SatelliteSetupItemControllerCreate(SatelliteSetup self, SatelliteSetupConfigType config)
{
    SatelliteSetupItemController controller = (SatelliteSetupItemController) SvTypeAllocateInstance(SatelliteSetupItemController_getType(), NULL);

    controller->config = config;
    controller->textRenderer = self->textRenderer;
    controller->itemConstructor = SVRETAIN(self->itemConstructor);
    controller->focus = SVRETAIN(self->focus);

    return controller;
}

typedef SvString (* SatelliteSetupButtonFormatValue)(void *self_, SvWidget button, SvObject value);
typedef void (* SatelliteSetupButtonClicked)(void *self_, SvWidget button);
typedef void (* SatelliteSetupButtonFocusChanged)(void *self_, SvWidget button, bool isFocused);

typedef struct SatelliteSetupButton_ {
    SvWidget choiceIndicatorActive, choiceIndicatorInactive;
    SvWidget label;

    SvWidget inactiveFrame, activeFrame;
    bool disabled;

    SatelliteSetupButtonFormatValue formatValue;
    SatelliteSetupButtonClicked selected;
    SatelliteSetupButtonFocusChanged focusChanged;
    void *callbackData;
} *SatelliteSetupButton;

SvLocal bool
SatelliteSetupButtonInputEh(SvWidget w, SvInputEvent ev)
{
    SatelliteSetupButton self = w->prv;

    if (ev->ch == QBKEY_ENTER) {
        if (self->selected) {
            self->selected(self->callbackData, w);
        }

        return true;
    }
    return false;
}

SvLocal void
SatelliteSetupButtonFocusEh(SvWidget w, SvFocusEvent fev)
{
    SatelliteSetupButton self = w->prv;

    if (fev->kind == SvFocusEventKind_GET) {
        svWidgetSetHidden(self->activeFrame, false);
        svWidgetSetHidden(self->inactiveFrame, true);
    } else if (fev->kind == SvFocusEventKind_LOST) {
        svWidgetSetHidden(self->activeFrame, true);
        svWidgetSetHidden(self->inactiveFrame, false);
    }

    if (self->focusChanged)
        self->focusChanged(self->callbackData, w, svWidgetIsHidden(self->activeFrame) == false);

    svWidgetForceRender(w);
}

SvLocal void
SatelliteSetupButtonClean(SvApplication app, void *self_)
{
    SatelliteSetupButton self = (SatelliteSetupButton) self_;

    free(self);
}

#define MAX_NAME 1024

SvLocal SvWidget
SatelliteSetupButtonNew(SvApplication app, const char *widgetName)
{
    SatelliteSetupButton self = NULL;
    SvWidget w = NULL;
    char buf[MAX_NAME];

    int line;
    do {
        if (!(w = svSettingsWidgetCreate(app, widgetName))) {
            line = __LINE__;
            break;
        }
        if (!(self = (SatelliteSetupButton) calloc(1, sizeof(struct SatelliteSetupButton_)))) {
            line = __LINE__;
            break;
        }

        w->prv = self;
        w->clean = SatelliteSetupButtonClean;
        svWidgetSetFocusable(w, true);
        svWidgetSetFocusEventHandler(w, SatelliteSetupButtonFocusEh);
        svWidgetSetInputEventHandler(w, SatelliteSetupButtonInputEh);

        snprintf(buf, MAX_NAME, "%s.Active", widgetName);
        if (!(self->activeFrame = QBFrameCreateFromSM(app, buf))) {
            line = __LINE__;
            break;
        }
        svSettingsWidgetAttach(w, self->activeFrame, buf, 10);
        svWidgetSetHidden(self->activeFrame, true);

        snprintf(buf, MAX_NAME, "%s.Inactive", widgetName);
        if (!(self->inactiveFrame = QBFrameCreateFromSM(app, buf))) {
            line = __LINE__;
            break;
        }
        svSettingsWidgetAttach(w, self->inactiveFrame, buf, 10);

        snprintf(buf, MAX_NAME, "%s.Label", widgetName);
        if (!(self->label = svLabelNewFromSM(app, buf))) {
            line = __LINE__;
            break;
        }
        svSettingsWidgetAttach(w, self->label, buf, 10);

        snprintf(buf, MAX_NAME, "%s.ChoiceIndicatorActive", widgetName);
        if (!(self->choiceIndicatorActive = svSettingsWidgetCreate(app, buf))) {
            line = __LINE__;
            break;
        }
        svSettingsWidgetAttach(w, self->choiceIndicatorActive, buf, 10);

        snprintf(buf, MAX_NAME, "%s.ChoiceIndicatorInactive", widgetName);
        if (!(self->choiceIndicatorInactive = svSettingsWidgetCreate(app, buf))) {
            line = __LINE__;
            break;
        }
        svSettingsWidgetAttach(w, self->choiceIndicatorInactive, buf, 10);
        svWidgetSetHidden(self->choiceIndicatorInactive, true);

        goto out;
    } while (false);
    SvLogError("%s %d: error", __func__, line);
    if (w) {
        svWidgetDestroy(w);
        w = NULL;
    }

out:
    return w;
}

SvLocal void
SatelliteSetupButtonSetObject(SvWidget w, SvObject obj)
{
    SatelliteSetupButton self = (SatelliteSetupButton) w->prv;

    if (self->formatValue) {
        SvString text = self->formatValue(self->callbackData, w, obj);
        if (text) {
            svLabelSetText(self->label, SvStringCString(text));
            SVRELEASE(text);
        }
    }
}

SvLocal void
SatelliteSetupButtonSetText(SvWidget w, const char *text)
{
    if (!w)
        return;

    SatelliteSetupButton self = (SatelliteSetupButton) w->prv;

    svLabelSetText(self->label, text);
}

SvLocal void
SatelliteSetupButtonSetDisabled(SvWidget w, bool disabled)
{
    if (!w)
        return;

    SatelliteSetupButton self = (SatelliteSetupButton) w->prv;

    if (disabled == self->disabled)
        return;

    self->disabled = disabled;

    if (disabled) {
        svWidgetSetFocusable(w, false);
        svWidgetSetHidden(self->choiceIndicatorActive, true);
        svWidgetSetHidden(self->choiceIndicatorInactive, false);
    } else {
        svWidgetSetFocusable(w, true);
        svWidgetSetHidden(self->choiceIndicatorActive, false);
        svWidgetSetHidden(self->choiceIndicatorInactive, true);
    }
}

SvLocal void
SatelliteSetupButtonSetCallbacks(SvWidget w, void *callbackData,
                                 SatelliteSetupButtonFormatValue format,
                                 SatelliteSetupButtonClicked selected,
                                 SatelliteSetupButtonFocusChanged focusChanged)
{
    SatelliteSetupButton self = (SatelliteSetupButton) w->prv;

    self->formatValue = format;
    self->selected = selected;
    self->focusChanged = focusChanged;
    self->callbackData = callbackData;
}

SvLocal void
SatelliteSetupSaveConfig(SatelliteSetup self)
{
    if (self->LNB.type == QBTunerLNBType_unknown)
        return;

    SvString satelliteID = QBDVBSatelliteDescriptorGetID(self->ctx->selectedSatellite);
    QBTunerLNBConfig cfg = QBTunerGetLNBConfigByID(satelliteID);

    if (!cfg)
        cfg = QBTunerLNBConfigCreate(satelliteID, self->LNB.type, NULL);
    else
        SVRETAIN(cfg);

    cfg->LNB = self->LNB;
    cfg->LNB.enable_22kHz_tone = false;
    if (!self->LNB.dual_band) {
        SvValue v = (SvValue) SvArrayAt(self->configs[SatelliteSetupConfig_toneSwitch].values, self->configs[SatelliteSetupConfig_toneSwitch].selectedItem);
        if (v && SvValueGetInteger(v) > 0)
            cfg->LNB.enable_22kHz_tone = true;
    }

    SvValue v = (SvValue) SvArrayAt(self->configs[SatelliteSetupConfig_diSEqCPort].values, self->configs[SatelliteSetupConfig_diSEqCPort].selectedItem);
    if (v && SvValueGetInteger(v) > 0)
        cfg->LNB.DiSEqC_port_number = SvValueGetInteger(v) - 1;
    else
        cfg->LNB.DiSEqC_port_number = -1;

    QBTunerLNBConfigSave(cfg, NULL);
    SVRELEASE(cfg);

    for (int i = 0; i < SatelliteSetupConfig_count; i++)
        self->configs[i].defaultItem = self->configs[i].selectedItem;
}

SvLocal void
SatelliteSetupConfigChanged(SatelliteSetup self)
{
    sv_tuner_untune(0);

    SatelliteSetupSaveConfig(self);
    QBTunerLNBConfig cfg = QBTunerGetLNBConfigByID(QBDVBSatelliteDescriptorGetID(self->ctx->selectedSatellite));
    struct QBTunerParams tunerParams;
    const struct QBTunerParams *mux = QBDVBSatelliteDescriptorGetFirstAvailableTransponder(self->ctx->selectedSatellite, &(cfg->LNB));
    if (mux == NULL) // No transponders defined
        return;

    tunerParams = *mux;
    strncpy(tunerParams.mux_id.satelliteID, SvStringCString(cfg->ID), sizeof(tunerParams.mux_id.satelliteID));
    sv_tuner_tune(0, &tunerParams);
}

SvLocal unsigned int
SatelliteSetupSelectInitialSatellite(void *self_,
                                     SvArray satellites)
{
    unsigned int selected = 0;
    const char *defSatID = QBConfigGet("DEFAULTSATELLITE");
    if (defSatID) {
        QBDVBSatelliteDescriptor desc = NULL;
        unsigned int i = 0;
        for (i = 0; i < SvArrayCount(satellites); i++) {
            desc = (QBDVBSatelliteDescriptor) SvArrayAt(satellites, i);
            if (SvStringEqualToCString(QBDVBSatelliteDescriptorGetID(desc), defSatID))
                return i;
        }
    }
    size_t i, cnt = SvArrayCount(satellites);
    QBDVBSatelliteDescriptor sat;
    int minDiff = INT_MAX;

    for (i = 0; i < cnt; i++) {
        sat = (QBDVBSatelliteDescriptor) SvArrayObjectAtIndex(satellites, i);
        int pos = QBDVBSatelliteDescriptorGetPosition(sat);
        pos = (pos > 1800) ? pos - 3600 : pos;
        if (abs(pos) < minDiff) {
            minDiff = abs(pos);
            selected = i;
        }
    }

    return selected;
}

SvLocal SvString
SatelliteSetupFormatValue_(SatelliteSetupConfigType config, SvObject value)
{
    if (config == SatelliteSetupConfig_satellite) {
        SvString name = QBDVBSatelliteDescriptorGetName((QBDVBSatelliteDescriptor) value);
        return SVRETAIN(name);
    }

    if (config == SatelliteSetupConfig_lnb) {
        QBTunerLNBType t = SvValueGetInteger((SvValue) value);
        const char *typeName = NULL;
        if (t == QBTunerLNBType_Ku_universal)
            typeName = gettext("Ku-band Universal");
        else if (t == QBTunerLNBType_Ku_low)
            typeName = gettext("Ku-band");
        else if (t == QBTunerLNBType_Ku_BSS)
            typeName = gettext("Ku-band BSS");
        else if (t == QBTunerLNBType_Ku_FSS)
            typeName = gettext("Ku-band FSS");
        else if (t == QBTunerLNBType_C)
            typeName = gettext("C-band");
        else
            typeName = gettext("Custom LNB");
        return SvStringCreate(typeName, NULL);
    }

    if (config == SatelliteSetupConfig_toneSwitch) {
        int option = SvValueGetInteger((SvValue) value);
        const char *optionName = NULL;
        if (option == 0)
            optionName = gettext("Off");
        else if (option == 1)
            optionName = gettext("On");
        else
            optionName = gettext("Band Select");
        return SvStringCreate(optionName, NULL);
    }

    if (config == SatelliteSetupConfig_diSEqCPort) {
        int portNumber = SvValueGetInteger((SvValue) value);
        if (portNumber > 0)
            return SvStringCreateWithFormat(gettext("DiSEqC Port %c"), 'A' + (portNumber - 1));
        return SvStringCreate(gettext("No DiSEqC"), NULL);
    }

    return NULL;
}


SvLocal SvString
SatelliteSetupFormatValue(void *self_,
                          SvWidget button,
                          SvObject value)
{
    SatelliteSetup self = self_;
    SatelliteSetupConfigType i;

    for (i = 0; i < SatelliteSetupConfig_count; i++) {
        if (button == self->configs[i].widget) {
            break;
        }
    }

    if (likely(i != SatelliteSetupConfig_count)) {
        return SatelliteSetupFormatValue_(i, value);
    }

    return NULL;
}

SvLocal SvWidget SatelliteSetupItemControllerCreateItem(SvObject self_, SvObject node_, SvObject path, SvApplication app, XMBMenuState initialState)
{
    SatelliteSetupItemController self = (SatelliteSetupItemController) self_;

    QBXMBItem item = QBXMBItemCreate();
    item->focus = SVRETAIN(self->focus);

    item->caption = SatelliteSetupFormatValue_(self->config, node_);

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);
    return ret;
}

SvLocal void SatelliteSetupItemControllerSetItemState(SvObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    SatelliteSetupItemController self = (SatelliteSetupItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void
SatelliteSetupDropDownDestroy(SvApplication app, void *prv_)
{
    SatelliteSetup self = (SatelliteSetup) prv_;

    SVTESTRELEASE(self->dropDownDataSource);
    self->dropDownDataSource = NULL;
}

SvLocal void
SatelliteSetupOnChangePosition(void *w_, const XMBMenuEvent data);

SvLocal QBLocalWindow
SatelliteSetupCreateDropDown(SatelliteSetup self,
                             const char *menuName)
{
    SvWidget window, verticalMenu, bg;
    SvIterator it;
    QBLocalWindow dropDown = NULL;
    SatelliteSetupItemController controller = NULL;

    it = SvArrayIterator(self->configs[self->selectedConfig].values);
    self->dropDownDataSource = QBActiveArrayCreateWithValues(&it, NULL);

    controller = SatelliteSetupItemControllerCreate(self, self->selectedConfig);

    window = svWidgetCreateBitmap(self->window->app, self->window->app->width, self->window->app->height, NULL);

    svSettingsPushComponent("SatelliteSetup.settings");

    verticalMenu = XMBVerticalMenuNew(self->window->app, menuName, 0);

    char tmp[strlen(menuName) + 4];
    snprintf(tmp, sizeof(tmp), "%s.Bg", menuName);
    bg = QBFrameCreateFromSM(self->window->app, tmp);

    svSettingsWidgetAttach(window, verticalMenu, menuName, 10);

    svSettingsPopComponent();

    XMBVerticalMenuConnectToDataSource(verticalMenu, (SvObject) self->dropDownDataSource, (SvObject) controller, NULL);
    XMBVerticalMenuSetPosition(verticalMenu, self->configs[self->selectedConfig].selectedItem, 1, NULL);
    XMBVerticalMenuSetBG(verticalMenu, bg);
    XMBVerticalMenuSetNotificationTarget(verticalMenu, self->window, SatelliteSetupOnChangePosition);
    self->dropDownNotify = svWidgetGetId(verticalMenu);
    self->xmbMenu = verticalMenu;

    svWidgetSetFocusable(verticalMenu, true);
    svWidgetSetFocus(verticalMenu);

    dropDown = (QBLocalWindow) SvTypeAllocateInstance(QBLocalWindow_getType(), NULL);
    QBLocalWindowInit(dropDown, window, QBLocalWindowTypeFocusable);

    dropDown->window->prv = self;
    dropDown->window->clean = SatelliteSetupDropDownDestroy;

    SVRELEASE(controller);

    return dropDown;
}

SvLocal void
SatelliteSetupDropDownCleanUp(SatelliteSetup self)
{
    svWidgetDestroy(self->dropDown->window);
    SVRELEASE(self->dropDown);
    self->dropDown = NULL;
}

SvLocal void
SatelliteSetupDropDownHideCleanUp(SatelliteSetup self)
{
    svWidgetSetHidden(self->configs[self->selectedConfig].widget, false);
    QBApplicationController controller = (QBApplicationController) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                               SVSTRING("QBApplicationController"));
    QBApplicationControllerRemoveLocalWindow(controller, self->dropDown);
    SatelliteSetupDropDownCleanUp(self);
}

SvLocal void
SatelliteSetupShowDropDown(SatelliteSetup self)
{
    QBApplicationController controller = (QBApplicationController) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                               SVSTRING("QBApplicationController"));
    svWidgetSetHidden(self->configs[self->selectedConfig].widget, true);
    QBApplicationControllerAddLocalWindow(controller, self->dropDown);
}

SvLocal int
SatelliteSetupShowEditorPane(SatelliteSetup self, QBDVBSatelliteDescriptor satellite)
{
    int idx = SvArrayIndexOfObject(self->configs[SatelliteSetupConfig_satellite].values, (SvObject) satellite);
    QBSatelliteSetupEditorPane satelliteSetupEditorPane =
        QBSatelliteSetupEditorPaneCreateFromSettings("SatelliteSetupEditorPane.settings",
                                                     self->ctx->app, self->sidemenu.ctx,
                                                     SVSTRING("SatelliteSetupEditorPane"),
                                                     self->configs[SatelliteSetupConfig_satellite].values,
                                                     idx, self->ctx->singleSatelliteMode, NULL);
    if (!satelliteSetupEditorPane)
        return -1;

    QBSatelliteSetupEditorPaneSetListener(satelliteSetupEditorPane, (SvObject) self);
    QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) satelliteSetupEditorPane);
    QBContextMenuShow(self->sidemenu.ctx);

    SVRELEASE(satelliteSetupEditorPane);

    return 0;
}

SvLocal void SatelliteSetupFocusChanged(void *self_, SvWidget button, bool isFocused)
{
    SatelliteSetup self = (SatelliteSetup) self_;

    if (!isFocused)
        return;

    int i;
    for (i = 0; i < SatelliteSetupConfig_count; ++i) {
        if (button == self->configs[i].widget) {
            self->focusedConfig = i;
            break;
        }
    }
}

SvLocal void
SatelliteSetupButtonSelected(void *self_,
                             SvWidget button)
{
    SatelliteSetup self = (SatelliteSetup) self_;
    const char *menuName = NULL;
    int i;

    self->selectedConfig = -1;
    for (i = 0; i < SatelliteSetupConfig_count; ++i) {
        if (button == self->configs[i].widget) {
            break;
        }
    }

    switch (i) {
        case SatelliteSetupConfig_satellite:
            menuName = "SatelliteMenu";
            break;
        case SatelliteSetupConfig_lnb:
            menuName = "LNBTypeMenu";
            break;
        case SatelliteSetupConfig_diSEqCPort:
            menuName = "DiSEqCPortMenu";
            break;
        case SatelliteSetupConfig_toneSwitch:
            menuName = "ToneSwitchMenu";
            break;
        default:
            return;
    }

    self->selectedConfig = i;
    self->dropDown = SatelliteSetupCreateDropDown(self, menuName);
    SatelliteSetupShowDropDown(self);
}

SvLocal void
SatelliteSetupChangeValue(SatelliteSetup self, SatelliteSetupConfigType config, size_t idx)
{
    if (!self->configs[config].widget)
        return;

    SvObject value = SvArrayAt(self->configs[config].values, idx);
    SatelliteSetupButtonSetObject(self->configs[config].widget, value);
    self->configs[config].selectedItem = (int) idx;
}

SvLocal void
SatelliteSetupCreateToneSwitchOptions(SatelliteSetup self, QBTunerLNBType type)
{
    SvArray toneSwitchOptions = SvArrayCreateWithCapacity(2, NULL);
    SvValue tmp = NULL;
    if (self->LNB.dual_band) {
        tmp = SvValueCreateWithInteger(-1, NULL);
        SvArrayAddObject(toneSwitchOptions, (SvObject) tmp);
        SVRELEASE(tmp);
        SatelliteSetupButtonSetDisabled(self->configs[SatelliteSetupConfig_toneSwitch].widget, true);
    } else {
        tmp = SvValueCreateWithInteger(0, NULL);
        SvArrayAddObject(toneSwitchOptions, (SvObject) tmp);
        SVRELEASE(tmp);
        tmp = SvValueCreateWithInteger(1, NULL);
        SvArrayAddObject(toneSwitchOptions, (SvObject) tmp);
        SVRELEASE(tmp);
        SatelliteSetupButtonSetDisabled(self->configs[SatelliteSetupConfig_toneSwitch].widget, false);
    }

    self->configs[SatelliteSetupConfig_toneSwitch].values = toneSwitchOptions;
    QBTunerLNBConfig cfg = QBTunerGetLNBConfigByID(QBDVBSatelliteDescriptorGetID(self->ctx->selectedSatellite));

    int toneSwitchDefault = 0;
    if (cfg && type == cfg->LNB.type && !cfg->LNB.dual_band && cfg->LNB.enable_22kHz_tone)
        toneSwitchDefault = 1;

    self->configs[SatelliteSetupConfig_toneSwitch].defaultItem = toneSwitchDefault;
    SatelliteSetupChangeValue(self, SatelliteSetupConfig_toneSwitch, toneSwitchDefault);
}

SvLocal void
SatelliteSetupChangeLNB(SatelliteSetup self, size_t idx)
{
    QBTunerLNBType type;
    char *buffer;

    SvValue value = (SvValue) SvArrayAt(self->configs[SatelliteSetupConfig_lnb].values, idx);

    SatelliteSetupChangeValue(self, SatelliteSetupConfig_lnb, idx);

    SVTESTRELEASE(self->configs[SatelliteSetupConfig_toneSwitch].values);
    self->configs[SatelliteSetupConfig_toneSwitch].values = NULL;

    type = SvValueGetInteger((SvValue) value);
    if (QBTunerGetLNBParams(type, &(self->LNB)) < 0) {
        self->LNB.type = QBTunerLNBType_unknown;
        asprintf(&buffer, gettext("%d out of %d"), 0, 0);
        svLabelSetText(self->infos[SatelliteSetupInfo_transponders], buffer);
        free(buffer);
        svLabelSetText(self->infos[SatelliteSetupInfo_freqRange], gettext("(unavailable)"));
        svLabelSetText(self->infos[SatelliteSetupInfo_lof], gettext("(unavailable)"));
        SatelliteSetupButtonSetText(self->configs[SatelliteSetupConfig_toneSwitch].widget, "");
        SatelliteSetupButtonSetDisabled(self->configs[SatelliteSetupConfig_toneSwitch].widget, true);
        return;
    }

    int availableCnt = QBDVBSatelliteDescriptorGetAvailableTranspondersCount(self->ctx->selectedSatellite, &(self->LNB));
    int totalCnt = QBDVBSatelliteDescriptorGetEnabledTranspondersCount(self->ctx->selectedSatellite);
    asprintf(&buffer, gettext("%d out of %d"), availableCnt, totalCnt);
    svLabelSetText(self->infos[SatelliteSetupInfo_transponders], buffer);
    free(buffer);

    const char *freqUnit = gettext("GHz");
    int freq_min = self->LNB.bands[0].freq_min;
    int freq_max = self->LNB.dual_band ? self->LNB.bands[1].freq_max : self->LNB.bands[0].freq_max;
    asprintf(&buffer, gettext("%.2f %s - %.2f %s"), (double) freq_min * 1e-6, freqUnit, (double) freq_max * 1e-6, freqUnit);
    svLabelSetText(self->infos[SatelliteSetupInfo_freqRange], buffer);
    free(buffer);

    asprintf(&buffer, "%.2f %s", (double) self->LNB.bands[0].local_osc_freq * 1e-6, freqUnit);
    char *oldbuf = buffer;
    if (self->LNB.dual_band) {
        asprintf(&buffer, "%s / %.2f %s", buffer, (double) self->LNB.bands[1].local_osc_freq * 1e-6, freqUnit);
        free(oldbuf);
    }
    svLabelSetText(self->infos[SatelliteSetupInfo_lof], buffer);
    free(buffer);

    SatelliteSetupCreateToneSwitchOptions(self, type);
}


SvLocal void
SatelliteSetupChangeSatellite(SatelliteSetup self, size_t idx)
{
    QBDVBSatelliteDescriptor satellite = (QBDVBSatelliteDescriptor) SvArrayAt(self->configs[SatelliteSetupConfig_satellite].values, idx);
    SvArray LNBTypes = SvArrayCreateWithCapacity(QBTunerLNBType_cnt + 1, NULL);
    struct QBTunerLNBParams params;
    int newLNBPos = 0, newDiSEqCPos = 0;
    unsigned int i;

    SatelliteSetupChangeValue(self, SatelliteSetupConfig_satellite, idx);

    SVTESTRELEASE(self->ctx->selectedSatellite);
    self->ctx->selectedSatellite = SVRETAIN(satellite);

    // If no transponders defined all LNB types are available
    ssize_t transpondersCnt = QBDVBSatelliteDescriptorGetTranspondersCount(satellite);

    // check what LNB types can be used with this satellite
    for (i = 1; i < QBTunerLNBType_cnt; i++) {
        if (QBTunerGetLNBParams(i, &params) < 0) {
            continue;
        }
        if (transpondersCnt > 0 && QBDVBSatelliteDescriptorGetAvailableTranspondersCount(satellite, &params) <= 0) {
            // won't tune to any transponders using this LNB
            continue;
        }
        SvValue tmp = SvValueCreateWithInteger(i, NULL);
        SvArrayAddObject(LNBTypes, (SvObject) tmp);
        SVRELEASE(tmp);
    }

    if (SvArrayCount(LNBTypes) == 0) {
        SvValue tmp = SvValueCreateWithInteger(-1, NULL);
        SvArrayAddObject(LNBTypes, (SvObject) tmp);
        SVRELEASE(tmp);
    }

    SVTESTRELEASE(self->configs[SatelliteSetupConfig_lnb].values);
    self->configs[SatelliteSetupConfig_lnb].values = LNBTypes;

    QBTunerLNBConfig cfg = QBTunerGetLNBConfigByID(QBDVBSatelliteDescriptorGetID(satellite));
    if (!cfg) {
        self->configs[SatelliteSetupConfig_satellite].defaultItem = -1;
        // this satellite hasn't been configured before
    } else {
        self->configs[SatelliteSetupConfig_satellite].defaultItem = idx;
        // select previously configured LNB type
        for (i = 0; i < SvArrayCount(LNBTypes); i++) {
            SvValue v = (SvValue) SvArrayObjectAtIndex(LNBTypes, i);
            if ((int) cfg->LNB.type == SvValueGetInteger(v)) {
                newLNBPos = i;
                break;
            }
        }
        newDiSEqCPos = cfg->LNB.DiSEqC_port_number + 1;
    }

    // select previously configured LNB type
    self->configs[SatelliteSetupConfig_lnb].defaultItem = newLNBPos;
    SatelliteSetupChangeLNB(self, newLNBPos);

    // select previously configured DiSEqC port number
    self->configs[SatelliteSetupConfig_diSEqCPort].defaultItem = newDiSEqCPos;
    SatelliteSetupChangeValue(self, SatelliteSetupConfig_diSEqCPort, newDiSEqCPos);
}

SvLocal void
SatelliteSetupSetDiSEqCPortForAllSatellites(SatelliteSetup self)
{
    SvValue disecqPortValue = (SvValue) SvArrayAt(self->configs[SatelliteSetupConfig_diSEqCPort].values, self->configs[SatelliteSetupConfig_diSEqCPort].selectedItem);
    if (!disecqPortValue)
        return;

    int disecqPort = SvValueGetInteger(disecqPortValue);
    if (disecqPort > 0)
        disecqPort--;

    SvIterator it = SvArrayGetIterator(self->configs[SatelliteSetupConfig_satellite].values);
    QBDVBSatelliteDescriptor satellite = NULL;

    while ((satellite = (QBDVBSatelliteDescriptor) SvIteratorGetNext(&it))) {
        QBTunerLNBConfig cfg = QBTunerGetLNBConfigByID(QBDVBSatelliteDescriptorGetID(satellite));
        if (cfg) {
            SVRETAIN(cfg);
            cfg->LNB.DiSEqC_port_number = disecqPort;
            QBTunerLNBConfigSave(cfg, NULL);
            SVRELEASE(cfg);
        }
    }
}

SvLocal void
SatelliteSetupOnChangePosition(void *w_, const XMBMenuEvent data)
{
    SvWidget w = (SvWidget) w_;
    SatelliteSetup self = (SatelliteSetup) w->prv;

    if (self->selectedConfig == SatelliteSetupConfig_satellite) {
        if (data->position < SvArrayCount(self->configs[SatelliteSetupConfig_satellite].values))
            self->sidemenu.focusedSatellite = (QBDVBSatelliteDescriptor) SvArrayAt(self->configs[SatelliteSetupConfig_satellite].values, data->position);
    }

    if (data->clicked) {
        SatelliteSetupDropDownHideCleanUp(self);

        if (self->selectedConfig == SatelliteSetupConfig_satellite) {
            SatelliteSetupChangeSatellite(self, data->position);
        } else if (self->selectedConfig == SatelliteSetupConfig_lnb) {
            SatelliteSetupChangeLNB(self, data->position);
        } else {
            SatelliteSetupChangeValue(self, (SatelliteSetupConfigType) self->selectedConfig, (size_t) data->position);
        }

        QBSatelliteSetupLogic satelliteSetupLogic = (QBSatelliteSetupLogic) QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBSatelliteSetupLogic_getType(), NULL);
        if (QBSatelliteSetupLogicIsSetDiSEqCPortForAllSatellites(satelliteSetupLogic) &&
            self->selectedConfig == SatelliteSetupConfig_diSEqCPort) {
            SatelliteSetupSetDiSEqCPortForAllSatellites(self);
        }

        SatelliteSetupConfigChanged(self);
    }
}


SvLocal bool
SatelliteSetupInputEventHandlerImpl(SatelliteSetup self, const QBInputEvent* ev)
{
    if (ev->type != QBInputEventType_keyTyped) {
        return false;
    }

    QBSatelliteSetupLogic satelliteSetupLogic = (QBSatelliteSetupLogic) QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBSatelliteSetupLogic_getType(), NULL);
    if (QBSatelliteSetupLogicInputEventHandler(satelliteSetupLogic, ev))
        return true;

    unsigned keyCode = ev->u.key.code;
    if (keyCode == QBKEY_BACK) {
        if (self->dropDown) {
            SatelliteSetupDropDownHideCleanUp(self);
            return true;
        }
    } else if (keyCode == QBKEY_FUNCTION) {
        if (self->dropDown) {
            SatelliteSetupDropDownHideCleanUp(self);
        }
        QBDVBSatelliteDescriptor editedSatellite = !self->dropDown ? self->ctx->selectedSatellite : self->sidemenu.focusedSatellite;
        SatelliteSetupShowEditorPane(self, editedSatellite);
        return true;
    }

    return false;
}

SvLocal bool
SatelliteSetupInputEventHandler(SvWidget w,
                                SvInputEvent ev)
{
    SatelliteSetup self = w->prv;

    if (SatelliteSetupInputEventHandlerImpl(self, &ev->fullInfo))
        return true;

    return svWidgetInputEventHandler(w, ev);
}

SvLocal void
SatelliteSetupContextTunerStateChanged(SvObject self_, int tunerNum)
{
    if (tunerNum != 0)
        return;
    struct sv_tuner_state* tuner = sv_tuner_get(tunerNum);
    struct QBTunerStatus status;
    sv_tuner_get_status(tuner, &status);
    SatelliteSetup setup = (SatelliteSetup) (((SatelliteSetupContext) self_)->super_.window->prv);
    if (status.full_lock) {
        svGaugeSetValue(setup->signalQuality, status.signal_quality);
        svGaugeSetValue(setup->signalStrength, status.signal_strength);
    } else {
        svGaugeSetValue(setup->signalQuality, 0);
        svGaugeSetValue(setup->signalStrength, 0);
    }
}

SvLocal void
SatelliteSetupUserEventHandler(SvWidget w,
                               SvWidgetId src,
                               SvUserEvent ev)
{
    SatelliteSetup self = w->prv;

    if (ev->code == SV_EVENT_BUTTON_PUSHED && src == svWidgetGetId(self->saveBtn)) {
        SatelliteSetupSaveConfig(self);
        SvString satelliteID = QBDVBSatelliteDescriptorGetID(self->ctx->selectedSatellite);
        QBConfigSet("DEFAULTSATELLITE", SvStringCString(satelliteID));
        QBConfigSave();
        QBApplicationController controller = (QBApplicationController) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                   SVSTRING("QBApplicationController"));
        QBApplicationControllerPopContext(controller);
        return;
    }
}

SvLocal void
SatelliteSetupSatelliteAdded(SvObject self_,
                             size_t idx, size_t count)
{
    SatelliteSetup setup = (SatelliteSetup) self_;

    if (setup->dropDown) {
        SatelliteSetupDropDownHideCleanUp(setup);
    }

    SatelliteSetupChangeSatellite(setup, idx);
    SatelliteSetupConfigChanged(setup);
    setup->sidemenu.focusedSatellite = (QBDVBSatelliteDescriptor) SvArrayAt(setup->configs[SatelliteSetupConfig_satellite].values, idx);
}

SvLocal void
SatelliteSetupSatelliteRemoved(SvObject self_,
                               size_t idx, size_t count)
{
    SatelliteSetup setup = (SatelliteSetup) self_;

    if (setup->dropDown) {
        QBActiveArray satList = setup->dropDownDataSource;
        QBActiveArrayRemoveObjectAtIndex(satList, idx, NULL);
    }

    size_t satCnt = SvArrayCount(setup->configs[SatelliteSetupConfig_satellite].values);
    assert(satCnt != 0); // Can't remove all satellites

    int newIdx = satCnt > idx ? idx : idx - 1;
    if (setup->configs[SatelliteSetupConfig_satellite].selectedItem > (int) idx) {
        setup->configs[SatelliteSetupConfig_satellite].selectedItem -= 1;
        setup->configs[SatelliteSetupConfig_satellite].defaultItem -= 1;
    } else if (setup->configs[SatelliteSetupConfig_satellite].selectedItem == (int) idx) {
        setup->sidemenu.focusedSatellite = (QBDVBSatelliteDescriptor) SvArrayAt(setup->configs[SatelliteSetupConfig_satellite].values, newIdx);
        SatelliteSetupChangeSatellite(setup, newIdx);
    }
    setup->sidemenu.focusedSatellite = (QBDVBSatelliteDescriptor) SvArrayAt(setup->configs[SatelliteSetupConfig_satellite].values, newIdx);
}

SvLocal void
SatelliteSetupSatelliteChanged(SvObject self_,
                               size_t idx, size_t count)
{
    SatelliteSetup setup = (SatelliteSetup) self_;

    if (setup->dropDown) {
        QBActiveArray satList = setup->dropDownDataSource;

        bool positionChanged = !SvObjectEquals((SvObject) setup->sidemenu.focusedSatellite, (SvObject) QBActiveArrayObjectAtIndex(satList, idx));
        if (positionChanged) {
            size_t oldIdx = QBActiveArrayIndexOfObject(satList, (SvObject) setup->sidemenu.focusedSatellite, NULL);
            QBActiveArrayMoveObject(satList, oldIdx, idx, NULL);
            XMBVerticalMenuSetPosition(setup->xmbMenu, idx, true, NULL);
        }
        QBActiveArraySetObjectAtIndex(satList, idx, (SvObject) setup->sidemenu.focusedSatellite, NULL);
    }

    if (SvObjectEquals((SvObject) setup->ctx->selectedSatellite, (SvObject) setup->sidemenu.focusedSatellite)) {
        SatelliteSetupChangeValue(setup, SatelliteSetupConfig_satellite, idx);
        SatelliteSetupChangeLNB(setup, setup->configs[SatelliteSetupConfig_lnb].defaultItem);
        SvString satelliteID = QBDVBSatelliteDescriptorGetID(setup->ctx->selectedSatellite);
        QBConfigSet("DEFAULTSATELLITE", SvStringCString(satelliteID));
        QBConfigSave();

        setup->configs[SatelliteSetupConfig_satellite].defaultItem = idx;
    }
}

SvLocal void
SatelliteSetupSatellitesReordered(SvObject self_,
                                  size_t idx, size_t count)
{
    SatelliteSetup setup = (SatelliteSetup) self_;

    if (setup->focusedConfig != SatelliteSetupConfig_satellite)
        return;

    int selectedSatIdx = SvArrayIndexOfObject(setup->configs[SatelliteSetupConfig_satellite].values, (SvObject) setup->ctx->selectedSatellite);
    setup->configs[SatelliteSetupConfig_satellite].selectedItem = selectedSatIdx;
    setup->configs[SatelliteSetupConfig_satellite].defaultItem = selectedSatIdx;

    if (setup->dropDown) {
        SatelliteSetupDropDownHideCleanUp(setup);
        setup->dropDown = SatelliteSetupCreateDropDown(setup, "SatelliteMenu");
        SatelliteSetupShowDropDown(setup);
        unsigned int newPos = SvArrayIndexOfObject(setup->configs[SatelliteSetupConfig_satellite].values, (SvObject) setup->sidemenu.focusedSatellite);
        XMBVerticalMenuSetPosition(setup->xmbMenu, newPos, true, NULL);
    }
}

SvLocal void
SatelliteSetup__dtor__(void *self_)
{
    SatelliteSetup self = (SatelliteSetup) self_;

    if (self->dropDown)
        SatelliteSetupDropDownCleanUp(self);

    SVTESTRELEASE(self->sidemenu.ctx);
    for (int i = 0; i < SatelliteSetupConfig_count; i++)
        SVTESTRELEASE(self->configs[i].values);

    SVRELEASE(self->focus);
    SVRELEASE(self->itemConstructor);
}

SvLocal SvType
SatelliteSetup_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = SatelliteSetup__dtor__
    };
    static SvType type = NULL;

    static const struct QBListModelListener_t listMethods = {
        .itemsAdded     = SatelliteSetupSatelliteAdded,
        .itemsRemoved   = SatelliteSetupSatelliteRemoved,
        .itemsChanged   = SatelliteSetupSatelliteChanged,
        .itemsReordered = SatelliteSetupSatellitesReordered
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("SatelliteSetup",
                            sizeof(struct SatelliteSetup_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBListModelListener_getInterface(), &listMethods,
                            NULL);
    }

    return type;
}

SvLocal void
SatelliteSetupClean(SvApplication app, void *self_)
{
    SatelliteSetup setup = (SatelliteSetup) self_;
    SVRELEASE(setup);
}

SvLocal SvWidget
SatelliteSetupCreateWindow(SatelliteSetup self, SvApplication app)
{
    SvWidget label, tmp;

    svSettingsPushComponent("SatelliteSetup.settings");

    QBGUILogic guiLogic = (QBGUILogic) QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBGUILogic_getType(), NULL);
    SvWidget window = QBGUILogicCreateBackgroundWidget(guiLogic, "Window", NULL);
    window->prv = self;
    svWidgetSetInputEventHandler(window, SatelliteSetupInputEventHandler);
    svWidgetSetUserEventHandler(window, SatelliteSetupUserEventHandler);
    window->clean = SatelliteSetupClean;
    svWidgetSetName(window, "SatelliteSetup");

    self->window = window;

    QBTextRenderer textRenderer = (QBTextRenderer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTextRenderer"));
    SvWidget title = QBTitleNew(app, "Title", textRenderer);
    svSettingsWidgetAttach(window, title, "Title", 1);
    SvString titleStr = SvStringCreate(gettext("Satellite Setup"), NULL);
    QBTitleSetText(title, titleStr, 0);
    SVRELEASE(titleStr);

    QBInitLogic initLogic = (QBInitLogic) QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBInitLogic_getType(), NULL);
    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(initLogic);
    if (logoWidgetName)
        self->logo = QBCustomerLogoAttach(window, logoWidgetName, 1);

    tmp = QBFrameCreateFromSM(app, "Frame1");
    svSettingsWidgetAttach(window, tmp, "Frame1", 1);

    tmp = QBFrameCreateFromSM(app, "Frame2");
    svSettingsWidgetAttach(window, tmp, "Frame2", 1);

    label = svLabelNewFromSM(app, "SatelliteInfoLabel");
    svSettingsWidgetAttach(window, label, svWidgetGetName(label), 1);

    label = svLabelNewFromSM(app, "LNBSetupLabel");
    svSettingsWidgetAttach(window, label, svWidgetGetName(label), 1);

    if (svSettingsIsWidgetDefined("SatelliteLabel")) {
        label = svLabelNewFromSM(app, "SatelliteLabel");
    }
    svSettingsWidgetAttach(window, label, svWidgetGetName(label), 1);

    if (svSettingsIsWidgetDefined("SatelliteBox")) {
        self->configs[SatelliteSetupConfig_satellite].widget = SatelliteSetupButtonNew(app, "SatelliteBox");
        assert(self->configs[SatelliteSetupConfig_satellite].widget != NULL);
        svSettingsWidgetAttach(window, self->configs[SatelliteSetupConfig_satellite].widget, svWidgetGetName(self->configs[SatelliteSetupConfig_satellite].widget), 1);
        SatelliteSetupButtonSetCallbacks(self->configs[SatelliteSetupConfig_satellite].widget, self, SatelliteSetupFormatValue, SatelliteSetupButtonSelected, SatelliteSetupFocusChanged);
    }
    if (self->ctx->singleSatelliteMode) {
        SatelliteSetupButtonSetDisabled(self->configs[SatelliteSetupConfig_satellite].widget, true);
    }

    if (svSettingsIsWidgetDefined("LNBTypeLabel")) {
        label = svLabelNewFromSM(app, "LNBTypeLabel");
        svSettingsWidgetAttach(window, label, svWidgetGetName(label), 1);
        self->configs[SatelliteSetupConfig_lnb].widget = SatelliteSetupButtonNew(app, "LNBTypeBox");
        assert(self->configs[SatelliteSetupConfig_lnb].widget != NULL);
        svSettingsWidgetAttach(window, self->configs[SatelliteSetupConfig_lnb].widget, svWidgetGetName(self->configs[SatelliteSetupConfig_lnb].widget), 1);
        SatelliteSetupButtonSetCallbacks(self->configs[SatelliteSetupConfig_lnb].widget, self, SatelliteSetupFormatValue, SatelliteSetupButtonSelected, SatelliteSetupFocusChanged);
    }

    label = svLabelNewFromSM(app, "TranspondersCountLabel");
    svSettingsWidgetAttach(window, label, svWidgetGetName(label), 1);
    self->infos[SatelliteSetupInfo_transponders] = svLabelNewFromSM(app, "TranspondersCountField");
    svSettingsWidgetAttach(window, self->infos[SatelliteSetupInfo_transponders], svWidgetGetName(self->infos[SatelliteSetupInfo_transponders]), 1);

    label = svLabelNewFromSM(app, "FreqRangeLabel");
    svSettingsWidgetAttach(window, label, svWidgetGetName(label), 1);
    self->infos[SatelliteSetupInfo_freqRange] = svLabelNewFromSM(app, "FreqRangeField");
    svSettingsWidgetAttach(window, self->infos[SatelliteSetupInfo_freqRange], svWidgetGetName(self->infos[SatelliteSetupInfo_freqRange]), 1);

    label = svLabelNewFromSM(app, "LOFLabel");
    svSettingsWidgetAttach(window, label, svWidgetGetName(label), 1);
    self->infos[SatelliteSetupInfo_lof] = svLabelNewFromSM(app, "LOFField");
    svSettingsWidgetAttach(window, self->infos[SatelliteSetupInfo_lof], svWidgetGetName(self->infos[SatelliteSetupInfo_lof]), 1);

    if (svSettingsIsWidgetDefined("ToneSwitchLabel")) {
        label = svLabelNewFromSM(app, "ToneSwitchLabel");
        svSettingsWidgetAttach(window, label, svWidgetGetName(label), 1);
    }

    if (svSettingsIsWidgetDefined("ToneSwitchBox")) {
        self->configs[SatelliteSetupConfig_toneSwitch].widget = SatelliteSetupButtonNew(app, "ToneSwitchBox");
        assert(self->configs[SatelliteSetupConfig_toneSwitch].widget != NULL);
        svSettingsWidgetAttach(window, self->configs[SatelliteSetupConfig_toneSwitch].widget, svWidgetGetName(self->configs[SatelliteSetupConfig_toneSwitch].widget), 1);
        SatelliteSetupButtonSetCallbacks(self->configs[SatelliteSetupConfig_toneSwitch].widget, self, SatelliteSetupFormatValue, SatelliteSetupButtonSelected, SatelliteSetupFocusChanged);
    }

    label = svLabelNewFromSM(app, "DiSEqCPortLabel");
    svSettingsWidgetAttach(window, label, svWidgetGetName(label), 1);
    self->configs[SatelliteSetupConfig_diSEqCPort].widget = SatelliteSetupButtonNew(app, "DiSEqCPortBox");
    assert(self->configs[SatelliteSetupConfig_diSEqCPort].widget != NULL);
    svSettingsWidgetAttach(window, self->configs[SatelliteSetupConfig_diSEqCPort].widget, svWidgetGetName(self->configs[SatelliteSetupConfig_diSEqCPort].widget), 1);
    SatelliteSetupButtonSetCallbacks(self->configs[SatelliteSetupConfig_diSEqCPort].widget, self, SatelliteSetupFormatValue, SatelliteSetupButtonSelected, SatelliteSetupFocusChanged);

    self->focusedConfig = -1;

    self->saveBtn = svButtonNewFromSM(app, "SaveButton", gettext("Done"), 1, svWidgetGetId(window));
    svSettingsWidgetAttach(window, self->saveBtn, svWidgetGetName(self->saveBtn), 1);

    self->focus = QBFrameConstructorFromSM("MenuItem.focus");
    self->textRenderer = textRenderer;
    self->itemConstructor = QBXMBItemConstructorCreate("MenuItem", self->textRenderer);

    SvWidget statusStrengthTitle = svLabelNewFromSM(app, "Strength.Title");
    SvWidget statusQualityTitle = svLabelNewFromSM(app, "Quality.Title");
    self->signalStrength = svGaugeNewFromSM(app, "Strength.Gauge", SV_GAUGE_HORIZONTAL, SV_GAUGE_STYLE_BASIC, 0, 100, 0);
    self->signalQuality = svGaugeNewFromSM(app, "Quality.Gauge", SV_GAUGE_HORIZONTAL, SV_GAUGE_STYLE_BASIC, 0, 100, 0);

    svSettingsWidgetAttach(window, statusStrengthTitle, "Strength.Title", 0);
    svSettingsWidgetAttach(window, statusQualityTitle, "Quality.Title", 0);
    svSettingsWidgetAttach(window, self->signalStrength, "Strength.Gauge", 0);
    svSettingsWidgetAttach(window, self->signalQuality, "Quality.Gauge", 0);

    svSettingsPopComponent();

    return window;
}

SvLocal void
SatelliteSetupSetSatellites(SatelliteSetup self,
                            SatelliteSetupContext ctx)
{
    // setup satellites list
    SvArray satellites = NULL;
    if (!ctx->singleSatelliteMode) {
        SvImmutableArray src = QBDVBSatellitesDBGetSatellites(ctx->satellitesDB);
        assert(src);
        size_t cnt = SvImmutableArrayCount(src);
        satellites = SvArrayCreateWithCapacity(cnt, NULL);
        SvIterator satIter = SvImmutableArrayIterator(src);
        SvArrayAddObjects(satellites, &satIter);
        SVRELEASE(src);

        const char *sortType = QBConfigGet("SATSORTTYPE");
        if (sortType && strcmp(sortType, "position") == 0)
            QBDVBSatelliteDescriptorSortByPosition(satellites);
        else
            QBDVBSatelliteDescriptorSortByName(satellites);
    } else {
        satellites = SvArrayCreateWithCapacity(1, NULL);
        SvArrayAddObject(satellites, (SvObject) ctx->selectedSatellite);
    }
    self->configs[SatelliteSetupConfig_satellite].values = satellites;
    // select satellite closest to prime meridian
    unsigned int idx = SatelliteSetupSelectInitialSatellite(self, satellites);
    self->sidemenu.focusedSatellite = (QBDVBSatelliteDescriptor) SvArrayAt(self->configs[SatelliteSetupConfig_satellite].values, idx);
    SatelliteSetupChangeSatellite(self, idx);
}

SvLocal SvWidget
SatelliteSetupNew(SvApplication app,
                  SatelliteSetupContext ctx)
{
    SatelliteSetup self;

    self = (SatelliteSetup) SvTypeAllocateInstance(SatelliteSetup_getType(), NULL);
    self->ctx = ctx;

    SvWidget window = SatelliteSetupCreateWindow(self, app);

    self->LNB.type = QBTunerLNBType_unknown;

    // setup DiSEqC switch ports list
    SvArray ports = SvArrayCreateWithCapacity(5, NULL);
    SvIterator portsIter = SvAutoIterator("iiiii", -1, 1, 2, 3, 4);
    SvArrayAddObjects(ports, &portsIter);
    self->configs[SatelliteSetupConfig_diSEqCPort].values = ports;

    SatelliteSetupSetSatellites(self, ctx);
    QBHintManager hintManager = (QBHintManager) QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBHintManager_getType(), NULL);
    if (hintManager)
        QBHintManagerSetHintsFromString(hintManager, SVSTRING(SATELLITE_HINT_DESCRIPTION));
    SatelliteSetupConfigChanged(self);

    QBApplicationController controller = (QBApplicationController) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                               SVSTRING("QBApplicationController"));
    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", controller, ctx->app, SVSTRING("SideMenu"));

    return window;
}

SvLocal SvWidget
SatelliteSetupReservationInfoNew(SvApplication app,
                                 SatelliteSetupContext ctx)
{
    SvWidget label;

    svSettingsPushComponent("SatelliteSetup.settings");

    QBTextRenderer textRenderer = (QBTextRenderer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTextRenderer"));
    QBGUILogic guiLogic = (QBGUILogic) QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBGUILogic_getType(), NULL);
    SvWidget window = QBGUILogicCreateBackgroundWidget(guiLogic, "Window", NULL);

    SvWidget title = QBTitleNew(app, "Title", textRenderer);
    svSettingsWidgetAttach(window, title, "Title", 1);
    SvString tmp = SvStringCreate(gettext("Satellite Setup"), NULL);
    QBTitleSetText(title, tmp, 0);
    SVRELEASE(tmp);

    label = svLabelNewFromSM(app, "ReservationInfo");
    svSettingsWidgetAttach(window, label, svWidgetGetName(label), 1);

    return window;
}
// SatelliteSetupContext

SvLocal bool
SatelliteSetupContextReserveTuners(SatelliteSetupContext self)
{
    QBTunerLogic tunerLogic = (QBTunerLogic) QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBTunerLogic_getType(), NULL);
    int tunerCount = QBTunerLogicGetTunerCount(tunerLogic);
    int res;
    QBTunerResv             *resv;

    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    QBTunerReserverStartTransaction(tunerReserver);

    resv = QBTunerResvCreate(SVSTRING("SatelliteCheck"), NULL, NULL);
    resv->params.priority = QBTUNERRESERVER_PRIORITY_SATELLITE_CHECK;
    resv->params.tunerParams = QBTunerGetEmptyParams();
    resv->params.tunerOwnership = true;
    resv->params.allTuners = true;
    resv->params.immediate = false;
    res = QBTunerReserverTryObtain(tunerReserver, resv);
    if (res < 0) {
        SVRELEASE(resv);
        QBTunerReserverEndTransaction(tunerReserver);
        return false;
    }
    for (int i = 0; i < tunerCount; ++i)
        sv_tuner_untune(i);
    QBTunerReserverRelease(tunerReserver, resv);
    SVRELEASE(resv);

    resv = QBTunerResvCreate(SVSTRING("SatelliteSetup"), NULL, NULL);
    resv->params.priority = QBTUNERRESERVER_PRIORITY_SATELLITE_SETUP;
    resv->params.tunerParams = QBTunerGetEmptyParams();
    resv->params.tunerOwnership = true;
    resv->params.immediate = false;
    res = QBTunerReserverTryObtain(tunerReserver, resv);
    if (res < 0) {
        SVRELEASE(resv);
        QBTunerReserverEndTransaction(tunerReserver);
        return false;
    }
    self->resv = resv;

    QBTunerReserverEndTransaction(tunerReserver);
    return true;
}

SvLocal void
SatelliteSetupContextFreeTuners(SatelliteSetupContext self)
{
    if (self->resv == NULL)
        return;

    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    QBTunerReserverStartTransaction(tunerReserver);
    QBTunerReserverRelease(tunerReserver, self->resv);
    SVRELEASE(self->resv);
    self->resv = NULL;
    QBTunerReserverEndTransaction(tunerReserver);
}

SvLocal void
SatelliteSetupContextReinitializeWindow(QBWindowContext self_,
                                        SvArray itemList)
{
    SatelliteSetupContext self = (SatelliteSetupContext) self_;
    if (!self->super_.window) {
        return;
    }
    SatelliteSetup prv = self->super_.window->prv;
    if (!itemList || SvArrayContainsObject(itemList, (SvObject) SVSTRING("CustomerLogo"))) {
        QBInitLogic initLogic = (QBInitLogic) QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBInitLogic_getType(), NULL);
        SvString logoWidgetName = QBInitLogicGetLogoWidgetName(initLogic);
        if (logoWidgetName)
            prv->logo = QBCustomerLogoReplace(prv->logo, self->super_.window, logoWidgetName, 1);
    }
}

SvLocal void
SatelliteSetupContextCreateWindow(QBWindowContext self_,
                                  SvApplication app)
{
    SatelliteSetupContext self = (SatelliteSetupContext) self_;
    if (!SatelliteSetupContextReserveTuners(self))
        self->super_.window = SatelliteSetupReservationInfoNew(app, self);
    else {
        self->super_.window = SatelliteSetupNew(app, self);
        QBTunerMonitor tunerMonitor = (QBTunerMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                   SVSTRING("QBTunerMonitor"));
        if (tunerMonitor)
            QBTunerMonitorAddListener(tunerMonitor, (SvObject) self);
    }
}

SvLocal void
SatelliteSetupContextDestroyWindow(QBWindowContext self_)
{
    SatelliteSetupContext self = (SatelliteSetupContext) self_;
    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
    if (self->resv != NULL && self->selectedSatellite) {
        SvString satelliteID = QBDVBSatelliteDescriptorGetID(self->selectedSatellite);
        QBConfigSet("DEFAULTSATELLITE", SvStringCString(satelliteID));
        QBConfigSave();
    }
    SatelliteSetupContextFreeTuners(self);
    QBHintManager hintManager = (QBHintManager) QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBHintManager_getType(), NULL);
    if (hintManager)
        QBHintManagerSetHintsFromString(hintManager, NULL);

    QBTunerMonitor tunerMonitor = (QBTunerMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                               SVSTRING("QBTunerMonitor"));
    if (tunerMonitor)
        QBTunerMonitorRemoveListener(tunerMonitor, (SvObject) self);
}


SvLocal void
SatelliteSetupContextDestroy(void *self_)
{
    SatelliteSetupContext self = self_;

    SVTESTRELEASE(self->satellitesDB);
    SVTESTRELEASE(self->selectedSatellite);
}

SvLocal void
SatelliteSetupItemController__dtor__(void *self_)
{
    SatelliteSetupItemController self = (SatelliteSetupItemController) self_;

    SVRELEASE(self->focus);
    SVRELEASE(self->itemConstructor);
}

SvLocal SvType
SatelliteSetupItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = SatelliteSetupItemController__dtor__
    };
    static SvType type = NULL;

    static const struct XMBItemController_t methods = {
        .createItem   = SatelliteSetupItemControllerCreateItem,
        .setItemState = SatelliteSetupItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("SatelliteSetupItemController",
                            sizeof(struct SatelliteSetupItemController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvLocal bool
SatelliteSetupContextInputEventHandler(SvObject w, const QBInputEvent* ev)
{
    SatelliteSetupContext ctx = (SatelliteSetupContext) w;
    SatelliteSetup setup = (SatelliteSetup) (ctx->super_.window->prv);
    unsigned keyCode = ev->u.key.code;
    if (keyCode == QBKEY_UP || keyCode == QBKEY_DOWN ||
        keyCode == QBKEY_LEFT || keyCode == QBKEY_RIGHT) {
        return true;
    }

    return SatelliteSetupInputEventHandlerImpl(setup, ev);
}

SvLocal SvType
SatelliteSetupContext_getType(void)
{
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_             = {
            .destroy        = SatelliteSetupContextDestroy
        },
        .reinitializeWindow = SatelliteSetupContextReinitializeWindow,
        .createWindow       = SatelliteSetupContextCreateWindow,
        .destroyWindow      = SatelliteSetupContextDestroyWindow
    };

    static const struct QBWindowContextInputEventHandler_t windowCtxInputMethod = {
        .handleInputEvent = SatelliteSetupContextInputEventHandler
    };

    static const struct QBTunerMonitorListener_t tunerMethods = {
        .tunerStateChanged = SatelliteSetupContextTunerStateChanged
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("SatelliteSetupContext",
                            sizeof(struct SatelliteSetupContext_),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            QBWindowContextInputEventHandler_getInterface(), &windowCtxInputMethod,
                            QBTunerMonitorListener_getInterface(), &tunerMethods,
                            NULL);
    }

    return type;
}

SvLocal SatelliteSetupContext
SatelliteSetupContextInit(SvApplication app, SatelliteSetupContext self)
{
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
                                     QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), NULL);

    if (!self || !satellitesDB)
        return NULL;

    if (!QBDVBSatellitesDBIsLoaded(satellitesDB)) {
        // FIXME: load in progress - we should wait until it is finished!
        SvLogWarning("%s(): satellites DB not loaded yet!", __func__);
        return NULL;
    }

    if (QBDVBSatellitesDBGetSatellitesCount(satellitesDB) <= 0) {
        /*
         * satellites DB is empty (firmware is invalid, because project
         * with satellite support should ALWAYS have this database included),
         * this screen is not functional without satellites DB
         **/
        SvLogWarning("%s(): satellites DB is empty (not available)!", __func__);
        return NULL;
    }

    self->app = app;

    self->satellitesDB = SVRETAIN(satellitesDB);

    return self;
}

QBWindowContext
SatelliteSetupContextCreate(SvApplication app)
{
    SatelliteSetupContext self;

    self = (SatelliteSetupContext) SvTypeAllocateInstance(SatelliteSetupContext_getType(), NULL);

    if (!self || !SatelliteSetupContextInit(app, self)) {
        SVTESTRELEASE(self);
        SvLogError("%s(): Failed to create SatelliteSetupContext", __func__);
        return NULL;
    }

    return (QBWindowContext) self;
}

QBWindowContext
SatelliteSetupContextCreateSingleSatelliteMode(SvApplication app, QBDVBSatelliteDescriptor satellite)
{
    if (!satellite) {
        SvLogError("%s(): Invalid argument, passed NULL satellite", __func__);
        return NULL;
    }

    SatelliteSetupContext self = (SatelliteSetupContext) SatelliteSetupContextCreate(app);

    if (!self) {
        return NULL;
    }

    self->selectedSatellite = SVRETAIN(satellite);
    self->singleSatelliteMode = true;

    return (QBWindowContext) self;
}
