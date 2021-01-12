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

#include "QBWizard.h"
#include <libintl.h>
#include <Logic/GUILogic.h>
#include <Logic/TunerLogic.h>
#include <Logic/WizardLogic.h>
#include <main.h>
#include <QBInput/QBInputCodes.h>
#include <QBTuner.h>
#include <QBConf.h>
#include <SvFoundation/SvData.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/button.h>
#include <SWL/events.h>
#include <SWL/label.h>
#include <QBWidgets/QBComboBox.h>
#include <Widgets/customerLogo.h>
#include <Widgets/QBTitle.h>
#include <QBWidgets/QBDialog.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <Services/QBMWConfigHandler.h>
#include <fibers/c/fibers.h>
#include "QBWizardOption.h"
#include "QBWizardBinaryOption.h"
#include "QBWizardNumericSubOption.h"
#include "../wizardInternal.h"


#define log_error(FMT, ...) do{ SvLogError("QBWizard " FMT, ##__VA_ARGS__); }while(0)
#define log_warning(FMT, ...) do{ SvLogWarning("QBWizard " FMT, ##__VA_ARGS__); }while(0)
#define WIZARD_WAIT_DIALOG_DELAY_SEC 2

/**
 * QBWizard holds all widgets of one wizard screen and handles their events.
 *
 * Each @ref QBWizardOption is presented as a label displaying caption and
 * combobox with values. If it's @ref QBWizardBinaryOption it has optional
 * widget for sub-option.
 *
 * Options are displayed in rows. Placement of widgets is determinated by the
 * offset variables.
 *
 * QBWizard is not SvGenericObject.
 */
struct QBWizard_s {
    QBWizardContext ctx; ///< weak_ptr
    SvWindow window;

    /**
     * Array of QBWizardOption which are present on this screen.
     * If there was error in loading options by QBWizardContext it's empty, but not NULL.
     */
    SvArray options;

    bool useDescriptions; ///< True for extra space under each option filled with description.
    int xDescriptionOffset; ///< Horizontal offset of descriptions.
    int yDescriptionOffset; ///< Vertical offset of descriptions.
    int descriptionHeight; ///< Description height.
    int xLabelOffset; ///< Horizontal offset of labels.
    int xComboBoxOffset; ///< Horizontal offset of comboboxes.
    int xSubOptionOffset; ///< Horizontal offset of sub-option widgets.
    int yOffset; ///< Vertical offset of first row.
    /**
     * Distance between rows.
     * Vertical offset of widgets in any row is yOffset + row_number * yOffsetJump.
     */
    int yOffsetJump;

    SvWidget logo; ///< Logo handle.
    SvWidget go; ///< Next/Go button handle.
    SvWidget title; ///< Title handle.

    ///@{
    /**
     * This is an array that has size equal to the size of options.
     * Widgets at given index are related to option at the same index in options.
     */
    SvWidget *labels;
    SvWidget *comboboxes;
    SvWidget *subOptionControls;
    SvWidget *descriptions;
    ///@}

    SvWidget dialog;
    SvFiber dialogFiber;
    SvFiberTimer dialogFiberTimer;

    bool shouldNumericalInputsRestart; ///< "true" causes all numerical controls to start new number when they regain focus
};
typedef struct QBWizard_s* QBWizard;


// ==== INTERNAL HEADERS ====
// === window callbacks ===

/**
 * Input event handler.
 *
 * If back button was pressed pops context and returns to previous wizard screen or else calls @QBWizardLogicHandleInput.
 *
 * @param[in]   window  window handle
 * @param[in]   e       event handle
 * @return      handler status
 */
SvLocal bool QBWizardWindowInputEventHandler(SvWidget window, SvInputEvent e);

/**
 * User event handler.
 *
 * Handles pressing wizard->go. If it's last screen in wizard calls QBWizardLogic or else creates and pushes next screen context.
 *
 * @param[in]   window  window handle
 * @param[in]   src     id of calling widget
 * @param[in]   e       event handle
 * @return      handler status
 */
SvLocal void QBWizardWindowUserEventHandler(SvWidget window, SvWidgetId src, SvUserEvent e);

/**
 * Callback for window->clean.
 *
 * Destroys QBWizard object.
 *
 * @param[in]   app     ignored
 * @param[in]   self_   QBWizard handle
 */
SvLocal void QBWizardWindowClean(SvApplication app, void *self_);

// === wizard ===

//SvLocal QBWizard QBWizardCreateWithContext(QBWizardContext ctx);

SvLocal void QBWizardDestroy(QBWizard self);

/**
 * Loads options from ctx->allOptions where screenNumber == ctx->screenNumber
 */
SvLocal void QBWizardLoadOptions(QBWizard self);

SvLocal void QBWizardCreateLogo(QBWizard self);
SvLocal void QBWizardCreateDialogWizard(QBWizard self);
SvLocal void QBWizardCreateTitle(QBWizard self);
SvLocal void QBWizardCreateGreeting(QBWizard self);
SvLocal void QBWizardCreateHeaders(QBWizard self);
SvLocal void QBWizardCreateOptionsWidgets(QBWizard self);
SvLocal void QBWizardWindowShowHybridModePollPopup(void* window_);
SvLocal void QBWizardWindowHideHybridModePollPopup(SvWindow window);

SvLocal void QBWizardRefreshAllWidgets(QBWizard self);
SvLocal void QBWizardRefreshLogo(QBWizard self);
SvLocal void QBWizardModeChanged(void *self_, SvWidget combobox, SvGenericObject modeId_);

// ==== PUBLIC FUNCTIONS ====

SvHidden SvWindow QBWizardWindowCreate(QBWizardContext ctx, SvApplication app)
{
    SvWidget window = QBGUILogicCreateBackgroundWidget(ctx->appGlobals->guiLogic, "Wizard.Window", NULL);
    svWidgetSetInputEventHandler(window, QBWizardWindowInputEventHandler);
    svWidgetSetUserEventHandler(window, QBWizardWindowUserEventHandler);
    window->clean = QBWizardWindowClean;

    QBWizard self = calloc(1, sizeof(struct QBWizard_s));
    window->prv = self;
    self->ctx = ctx;
    self->options = SvArrayCreate(NULL);
    self->window = window;

    self->xDescriptionOffset = svSettingsGetInteger("Wizard.DescriptionLabel", "xOffset", 0);
    self->yDescriptionOffset = svSettingsGetInteger("Wizard.DescriptionLabel", "yOffset", 0);
    self->descriptionHeight = svSettingsGetInteger("Wizard.DescriptionLabel", "height", 64);
    self->xLabelOffset = svSettingsGetInteger("Wizard", "xLabelOffset", 0);
    self->xComboBoxOffset = svSettingsGetInteger("Wizard", "xComboBoxOffset", 0);
    self->xSubOptionOffset = svSettingsGetInteger("Wizard", "xSubOptionOffset", 0);
    self->yOffset = svSettingsGetInteger("Wizard", "yOffset", 0);
    self->yOffsetJump = svSettingsGetInteger("Wizard", "yOffsetJump", 0);

    QBWizardLoadOptions(self);
    QBWizardCreateLogo(self);
    QBWizardCreateTitle(self);
    QBWizardCreateGreeting(self);
    QBWizardCreateHeaders(self);
    QBWizardCreateOptionsWidgets(self);
    QBWizardCreateDialogWizard(self);
    if (self->ctx->appGlobals->qbMWConfigHandler && self->ctx->appGlobals->qbMWConfigMonitor && !ctx->isPollPopupSet) {
        ctx->isPollPopupSet = true;
        self->dialogFiber = SvFiberCreate(self->ctx->appGlobals->scheduler, NULL, "QBWizardHybridModePollPopup", QBWizardWindowShowHybridModePollPopup, (void*)self->window);
        self->dialogFiberTimer = SvFiberTimerCreate(self->dialogFiber);
        SvFiberTimerActivateAfter(self->dialogFiberTimer, SvTimeConstruct(WIZARD_WAIT_DIALOG_DELAY_SEC, 0));
    }

    svWidgetSetFocus(self->go);

    self->shouldNumericalInputsRestart = true;

    return window;
}

void QBWizardWindowShowHybridModePollPopup(void* window_)
{
    SvWindow window = (SvWindow) window_;
    assert(window);
    QBWizard self = (QBWizard) window->prv;

    SvFiberDeactivate(self->dialogFiber);
    SvFiberEventDeactivate(self->dialogFiberTimer);

    svSettingsPushComponent("Dialog.settings");
    QBDialogParameters params = {
        .app        = self->ctx->appGlobals->res,
        .controller = self->ctx->appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(window),
    };
    self->dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    svSettingsPopComponent();
    svSettingsPushComponent("WizardDialog.settings");
    QBDialogAddPanel(self->dialog, SVSTRING("content"), NULL, 1);

    QBDialogSetTitle(self->dialog, gettext(svSettingsGetString("WizardPopup", "title")));

    SvApplication app = self->ctx->appGlobals->res;
    SvWidget animation = QBWaitAnimationCreate(app, "WizardPopup.animation");

    if (animation)
        QBDialogAddWidget(self->dialog, SVSTRING("content"), animation, SVSTRING("animation"), 1, "WizardPopup.animation");
    svSettingsPopComponent();

    QBDialogRun(self->dialog, self, NULL);
}

SvLocal void QBWizardWindowHideHybridModePollPopup(SvWindow window)
{
    QBWizard self = (QBWizard) window->prv;
    if (self->dialogFiber)
        SvFiberDeactivate(self->dialogFiber);
    if (self->dialogFiberTimer)
        SvFiberEventDeactivate(self->dialogFiberTimer);
    if (self && self->dialog) {
        QBDialogBreak(self->dialog);
        self->dialog = NULL;
    }
}

SvHidden void QBWizardWindowReinitialize(SvWindow window, SvArray itemList)
{
    assert(window);
    QBWizard self = (QBWizard) window->prv;

    if (itemList) {
        if (SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("CustomerLogo")))
            QBWizardRefreshLogo(self);
    } else {
        QBWizardRefreshAllWidgets(self);
    }
    const char *currentMode = QBConfigGet("HYBRIDMODE");
    SvString modeString = SvStringCreate(currentMode, NULL);
    QBWizardContextSetOptionValueId(self->ctx, SVSTRING("mode"), modeString);
    QBWizardModeChanged(self, NULL, (SvObject) modeString);
    QBWizardWindowHideHybridModePollPopup(window);
    SVRELEASE(modeString);
}

// ==== PRIVATE FUNCTIONS ====
// === window callbacks ===

SvLocal bool QBWizardWindowInputEventHandler(SvWidget window, SvInputEvent e)
{
    assert(window); assert(e);
    QBWizard self = window->prv;

    switch (e->ch) {
        case QBKEY_BACK:
            QBApplicationControllerPopContext(self->ctx->appGlobals->controller);
            return true;
        default:
            break;
    }

    if (svWidgetInputEventHandler(window, e))
        return true;

    return QBWizardLogicHandleInput(self->ctx->appGlobals->wizardLogic, &e->fullInfo);
}

SvLocal void QBWizardWindowUserEventHandler(SvWidget window, SvWidgetId src, SvUserEvent e)
{
    assert(window); assert(src); assert(e);
    QBWizard self = window->prv;
    assert(self->ctx);
    QBWizardContext ctx = self->ctx;
    assert(self->go);

    if (e->code != SV_EVENT_BUTTON_PUSHED || src != svWidgetGetId(self->go)) {
        return;
    }

    if (ctx->screenNumber == ctx->sharedData->allScreens) {
        SvHashTable idToOption = QBWizardContextCreateIdToOptionComplementedWithDefaults(ctx);
        SvHashTable idToSubOption = QBWizardContextCreateIdToSubOptionComplementedWithDefaults(ctx);
        QBWizardLogicSetOption(self->ctx->appGlobals->wizardLogic, idToOption, idToSubOption);
        SVRELEASE(idToOption);
        SVRELEASE(idToSubOption);
    } else {
        QBWindowContext nextCtx = QBWizardContextCreateNextScreen(ctx);
        QBApplicationControllerPushContext(self->ctx->appGlobals->controller, nextCtx);
        SVRELEASE(nextCtx);
    }
}

SvLocal void QBWizardWindowClean(SvApplication app, void *self_)
{
    assert(self_);
    QBWizard self = self_;

    QBWizardDestroy(self);
}

// === wizard ===
// == construction ==

SvLocal void QBWizardAllocateArraysForWidgets(QBWizard self);

SvLocal void QBWizardLoadOptions(QBWizard self)
{
    assert(self); assert(self->options);

    SVTESTRELEASE(self->options);
    self->options = QBWizardContextSelectOptionsOfCurrentScreen(self->ctx);

    QBWizardAllocateArraysForWidgets(self);
}

SvLocal void QBWizardAllocateArraysForWidgets(QBWizard self)
{
    assert(self); assert(self->options);
    assert(!self->comboboxes); assert(!self->labels); assert(!self->subOptionControls);
    assert(!self->descriptions);

    size_t optionsCount = SvArrayCount(self->options);
    self->comboboxes = calloc(optionsCount, sizeof(SvWidget));
    self->labels = calloc(optionsCount, sizeof(SvWidget));
    self->subOptionControls = calloc(optionsCount, sizeof(SvWidget));
    self->descriptions = calloc(optionsCount, sizeof(SvWidget));
}

SvLocal void QBWizardCreateLogo(QBWizard self)
{
    assert(self); assert(!self->logo);

    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(self->ctx->appGlobals->initLogic);
    if (!logoWidgetName) {
        return;
    }
    self->logo = QBCustomerLogoAttach(self->window, logoWidgetName, 1);
}

SvLocal void QBWizardSetGoButtonCaption(QBWizard self);

SvLocal void QBWizardCreateDialogWizard(QBWizard self)
{
    assert(self); assert(!self->go);

    self->go = svButtonNewFromSM(self->ctx->appGlobals->res, "Wizard.Go", NULL, 0, svWidgetGetId(self->window));
    QBWizardSetGoButtonCaption(self);

    svSettingsWidgetAttach(self->window, self->go, "Wizard.Go", 1);
}

SvLocal void QBWizardSetGoButtonCaption(QBWizard self)
{
    assert(self); assert(self->go);
    QBWizardContext ctx = self->ctx;

    const char *caption = gettext(svSettingsGetString("Wizard.Go", (ctx->screenNumber == ctx->sharedData->allScreens ? "captionFinish" : "captionNext")));
    svButtonSetCaption(self->go, caption);
}

SvLocal void QBWizardSetTitleText(QBWizard self);

SvLocal void QBWizardCreateTitle(QBWizard self)
{
    assert(self); assert(!self->title);

    self->title = QBTitleNew(self->ctx->appGlobals->res, "Title", self->ctx->appGlobals->textRenderer);
    QBWizardSetTitleText(self);
    svSettingsWidgetAttach(self->window, self->title, "Title", 1);
}

SvLocal void QBWizardSetTitleText(QBWizard self)
{
    assert(self); assert(self->title);
    QBWizardContext ctx = self->ctx;

    {
        SvString titleStr = SvStringCreate(gettext("First Time Installation"), NULL);
        QBTitleSetText(self->title, titleStr, 0);
        SVRELEASE(titleStr);
    }
    {
        SvString titleStr = SvStringCreateWithFormat(gettext("Step %d out of %d"),
                                                     ctx->screenNumber + 1,
                                                     ctx->sharedData->allScreens + 1);
        QBTitleSetText(self->title, titleStr, 1);
        SVRELEASE(titleStr);
    }
}

SvLocal void QBWizardCreateGreeting(QBWizard self)
{
    assert(self);

    if (!svSettingsIsWidgetDefined("Wizard.Greeting"))
        return;

    SvWidget greeting = svLabelNewFromSM(self->ctx->appGlobals->res, "Wizard.Greeting");
    svSettingsWidgetAttach(self->window, greeting, "Wizard.Greeting", 1);
}

SvLocal void QBWizardCreateHeaders(QBWizard self)
{
    assert(self);

    for (unsigned char i = 1; i < UCHAR_MAX; i++) {
        char *widgetName;
        if (asprintf(&widgetName, "Wizard.Header%u", i) == -1)
            break;
        if (!svSettingsIsWidgetDefined(widgetName)) {
            free(widgetName);
            break;
        }
        SvWidget header = svLabelNewFromSM(self->ctx->appGlobals->res, widgetName);
        svSettingsWidgetAttach(self->window, header, widgetName, 1);
        free(widgetName);
    }
}

SvLocal void QBWizardUpdateOption(QBWizard self, SvString optionId);

SvLocal void QBWizardCreateLabel(QBWizard self, QBWizardOption option, size_t idx);
SvLocal void QBWizardCreateCombobox(QBWizard self, QBWizardOption option, size_t idx);
SvLocal bool QBWizardUseDescriptionForOptions(QBWizard self);
SvLocal bool QBWizardCreateDescription(QBWizard self, QBWizardOption option, size_t idx);
SvLocal void QBWizardCreateNumericSubOptionControl(QBWizard self, QBWizardBinaryOption option, size_t idx);
// other suboption types here

SvLocal void QBWizardCreateOptionsWidgets(QBWizard self)
{
    assert(self); assert(self->options); assert(self->comboboxes);

    SvSet idsOfOptionsWithDependencies = SvSetCreate(7, NULL);
    SvSetAddElement(idsOfOptionsWithDependencies, (SvGenericObject) SVSTRING("standard"));
    SvSetAddElement(idsOfOptionsWithDependencies, (SvGenericObject) SVSTRING("PROVIDERS.EPG.DVB"));
    SvSetAddElement(idsOfOptionsWithDependencies, (SvGenericObject) SVSTRING("PROVIDERS.CHANNELS.IP"));
    SvSetAddElement(idsOfOptionsWithDependencies, (SvGenericObject) SVSTRING("PROVIDERS.EPG.IP"));
    SvSetAddElement(idsOfOptionsWithDependencies, (SvGenericObject) SVSTRING("PROVIDERS.NPVR"));
    SvSetAddElement(idsOfOptionsWithDependencies, (SvGenericObject) SVSTRING("PROVIDERS.CUTV"));

    // array, not set, because it's important to keep them in order they appear
    SvArray idsOfFoundOptionsWithDependencies = SvArrayCreate(NULL);

    self->useDescriptions = QBWizardUseDescriptionForOptions(self);
    for(size_t i = 0; i < SvArrayCount(self->options); ++i) {
        QBWizardOption option = (QBWizardOption) SvArrayObjectAtIndex(self->options, i);

        QBWizardCreateLabel(self, option, i);
        QBWizardCreateCombobox(self, option, i);
        if (self->useDescriptions) {
            QBWizardCreateDescription(self, option, i);
        }

        if (SvObjectIsInstanceOf((SvObject) option, QBWizardBinaryOption_getType())) {
            QBWizardBinaryOption binaryOption = (QBWizardBinaryOption) option;

            if (binaryOption->subOption && SvStringEqualToCString(QBWizardContextGetOptionValueId(self->ctx, option->id), "enabled")) {
                if (SvObjectIsInstanceOf(binaryOption->subOption, QBWizardNumericSubOption_getType())) {
                    QBWizardCreateNumericSubOptionControl(self, binaryOption, i);
                }
                // other suboption types here
            }
        }

        if (SvSetContainsElement(idsOfOptionsWithDependencies, (SvGenericObject) option->id))
            SvArrayAddObject(idsOfFoundOptionsWithDependencies, (SvGenericObject) option->id);
    }

    SVRELEASE(idsOfOptionsWithDependencies);

    // Tweaks for options that depend on each other.
    {
        SvIterator it = SvArrayIterator(idsOfFoundOptionsWithDependencies);
        SvString optionId;
        while ((optionId = (SvString) SvIteratorGetNext(&it)))
            QBWizardUpdateOption(self, optionId);
    }

    SVRELEASE(idsOfFoundOptionsWithDependencies);
}

SvLocal void QBWizardCreateLabel(QBWizard self, QBWizardOption option, size_t idx)
{
    assert(self); assert(option);

    SvWidget label = svLabelNewFromSM(self->ctx->appGlobals->res, "Wizard.Label");
    svLabelSetText(label, gettext(SvStringCString(option->caption)));
    svWidgetAttach(self->window, label, self->xLabelOffset,
                   self->yOffset + idx * self->yOffsetJump +
                   (self->useDescriptions ? idx * self->descriptionHeight : 0), 1);

    self->labels[idx] = label;
}

SvLocal bool QBWizardUseDescriptionForOptions(QBWizard self)
{
    // check if description is properly defined in settings
    SvWidget description = svLabelNewFromSM(self->ctx->appGlobals->res, "Wizard.DescriptionLabel");
    if (!description)
        return false;
    svWidgetDestroy(description);

    // check if at least one description text is defined
    for(size_t i = 0; i < SvArrayCount(self->options); ++i) {
        QBWizardOption option = (QBWizardOption) SvArrayObjectAtIndex(self->options, i);
        if (option->description)
            return true;
    }

    return false;
}

SvLocal bool QBWizardCreateDescription(QBWizard self, QBWizardOption option, size_t idx)
{
    assert(self); assert(option);

    if (!option->description)
        return false;

    SvWidget description = svLabelNewFromSM(self->ctx->appGlobals->res, "Wizard.DescriptionLabel");
    if (!description)
        return false;

    svLabelSetText(description, gettext(SvStringCString(option->description)));
    svWidgetAttach(self->window, description, self->xDescriptionOffset,
                   self->yDescriptionOffset + idx * self->yOffsetJump +
                   (self->useDescriptions ? idx * self->descriptionHeight : 0), 1);

    self->descriptions[idx] = description;

    return true;
}

SvLocal void QBWizardLNBChanged(void *self_, SvWidget combobox, SvGenericObject valueId_);
SvLocal void QBWizardLNBComboboxFocusFinished(void *target, SvWidget combobox, SvGenericObject selectedValue, SvGenericObject userValue);
SvLocal void QBWizardLNBComboboxFocusStarted(void *target, SvWidget combobox, SvGenericObject selectedValue);
SvLocal SvString QBWizardComboboxPrepareOption(void *self_, SvWidget combobox, SvGenericObject valueId_);
SvLocal int QBWizardComboboxChange(void *self_, SvWidget combobox, SvGenericObject value, int key);
SvLocal void QBWizardLanguageChanged(void *self_, SvWidget combobox, SvGenericObject languageId_);
SvLocal void QBWizardTunerChanged(void *self_, SvWidget combobox, SvGenericObject tunerId_);
SvLocal void QBWizardProviderChanged(void *self_, SvWidget combobox, SvGenericObject newValueId_);

SvLocal bool QBWizardBinaryOptionWithSubOptionComboboxInputHandler(void *self_, SvWidget combobox, SvInputEvent e);
SvLocal void QBWizardBinaryOptionWithSubOptionChanged(void *self_, SvWidget combobox, SvGenericObject valueId_);

SvLocal void QBWizardGenericComboboxChanged(void *self_, SvWidget combobox, SvGenericObject valueId_);

/**
 * Compare ids.
 * @param[in] _prv unused
 * @param[in] id1 SvString
 * @param[in] id2 SvString
 * @return    strcmp(id1, id2) with rule that "unusedOption" is less than everything
 */
SvLocal int QBWizardCompareIds(void *_prv, SvGenericObject id1, SvGenericObject id2);

SvLocal void QBWizardCreateCombobox(QBWizard self, QBWizardOption option, size_t idx)
{
    assert(self); assert(option);

    SvWidget combobox = QBComboBoxNewFromSM(self->ctx->appGlobals->res, "Wizard.ComboBox");
    svWidgetSetFocusable(combobox, true);
    svWidgetAttach(self->window, combobox, self->xComboBoxOffset,
                   self->yOffset + idx * self->yOffsetJump +
                   (self->useDescriptions ? idx * self->descriptionHeight : 0), 1);

    // set combobox content
    {
        SvArray ids = SvArrayCreate(NULL);

        if (SvObjectIsInstanceOf((SvObject) option, QBWizardBinaryOption_getType())) {
            // we know what are the options and we need them in specific order
            SvArrayAddObject(ids, (SvObject) SVSTRING("disabled"));
            SvArrayAddObject(ids, (SvObject) SVSTRING("enabled"));
        } else if (SvStringEqualToCString(option->id, "lang")) {
            SvIterator it = SvHashTableKeysIterator(option->idToCaption);
            SvArrayAddObjects(ids, &it);
            SvArraySortWithCompareFn(ids, QBWizardCompareIds, NULL);

            // Use the language from qbconf, so it won't be overriden when mode changes.
            SvString lang = SvStringCreate(QBConfigGet("LANG"), NULL);
            QBWizardContextSetOptionValueId(self->ctx, option->id, lang);
            SVRELEASE(lang);
        } else if (SvStringEqualToCString(option->id, "mode")) {
            const int tunerCount = QBTunerLogicGetTunerCount(self->ctx->appGlobals->tunerLogic);
            if (tunerCount == 0) {
                SvArrayAddObject(ids, (SvObject) SVSTRING("IP"));
                if (!SvHashTableFind(option->idToCaption, (SvObject) SVSTRING("IP"))) {
                    log_warning("No available tuners and no IP mode option. Forcing IP mode.");
                    QBConfigSet("HYBRIDMODE", "IP");
                    QBConfigSave();
                }
            } else {
                SvIterator it = SvHashTableKeysIterator(option->idToCaption);
                SvArrayAddObjects(ids, &it);
                SvArraySortWithCompareFn(ids, QBWizardCompareIds, NULL);
            }

        } else if (SvStringEqualToCString(option->id, "tuner")) {

            const struct QBTunerConfigSet* configSets = NULL;
            int configSetCnt = 0;
            QBTunerGetConfig("native", NULL, NULL, &configSets, &configSetCnt);
            int s;
            for (s=0; s<configSetCnt; ++s) {
                SvString name = SvStringCreate(configSets[s].name, NULL);
                if (SvHashTableFind(option->idToCaption, (SvGenericObject) name)) {
                    SvArrayAddObject(ids, (SvGenericObject)name);
                }
                SVRELEASE(name);
            }
            if (!configSetCnt)
                SvArrayAddObject(ids, (SvGenericObject) SVSTRING("unusedOption"));

        } else {
            SvIterator it = SvHashTableKeysIterator(option->idToCaption);
            SvArrayAddObjects(ids, &it);
            SvArraySortWithCompareFn(ids, QBWizardCompareIds, NULL);
        }

        QBComboBoxSetContent(combobox, ids);
        SVRELEASE(ids);
    }

    // set combobox position
    {
        SvString valueId = QBWizardContextGetOptionValueId(self->ctx, option->id);

        ssize_t valueIdx = SvArrayIndexOfObject(QBComboBoxGetContent(combobox), (SvGenericObject) valueId);
        if (valueIdx < 0)
            valueIdx = 0;

        QBComboBoxSetPosition(combobox, valueIdx);

        // if the value may have changed, then save the current one.
        if (valueIdx == 0) {
            valueId = (SvString) QBComboBoxGetValue(combobox);
            QBWizardContextSetOptionValueId(self->ctx, option->id, valueId);
        }
    }

    // set combobox callbacks
    {
        if (SvObjectGetType((SvGenericObject) option) == QBWizardBinaryOption_getType() && ((QBWizardBinaryOption) option)->subOption) {
            QBComboBoxCallbacks cb = { QBWizardComboboxPrepareOption, QBWizardBinaryOptionWithSubOptionComboboxInputHandler, NULL, QBWizardBinaryOptionWithSubOptionChanged, NULL, NULL };
            QBComboBoxSetCallbacks(combobox, self, cb);
        } else if (SvStringEqualToCString(option->id, "lang")) {
            QBComboBoxCallbacks cb = { QBWizardComboboxPrepareOption, NULL, NULL, QBWizardLanguageChanged, NULL, NULL };
            QBComboBoxSetCallbacks(combobox, self, cb);
        } else if (SvStringEqualToCString(option->id, "mode")) {
            QBComboBoxCallbacks cb = { QBWizardComboboxPrepareOption, NULL, NULL, QBWizardModeChanged, NULL, NULL };
            QBComboBoxSetCallbacks(combobox, self, cb);
        } else if (SvStringEqualToCString(option->id, "tuner")) {
            QBComboBoxCallbacks cb = { QBWizardComboboxPrepareOption, NULL, NULL, QBWizardTunerChanged, NULL, NULL };
            QBComboBoxSetCallbacks(combobox, self, cb);
        } else if (SvStringEqualToCString(option->id, "TUNERS_LIMIT")) {
            QBComboBoxCallbacks cb = { QBWizardComboboxPrepareOption, NULL, NULL, QBWizardLNBChanged, QBWizardLNBComboboxFocusStarted, QBWizardLNBComboboxFocusFinished };
            QBComboBoxSetCallbacks(combobox, self, cb);
        } else if (SvStringEqualToCString(option->id, "PROVIDERS.CHANNELS.DVB") ||
                   SvStringEqualToCString(option->id, "PROVIDERS.EPG.DVB") ||
                   SvStringEqualToCString(option->id, "PROVIDERS.CHANNELS.IP") ||
                   SvStringEqualToCString(option->id, "PROVIDERS.EPG.IP") ||
                   SvStringEqualToCString(option->id, "PROVIDERS.NPVR")) {
            QBComboBoxCallbacks cb = { QBWizardComboboxPrepareOption, NULL, QBWizardComboboxChange, QBWizardProviderChanged, NULL, NULL };
            QBComboBoxSetCallbacks(combobox, self, cb);
        } else {
            QBComboBoxCallbacks cb = { QBWizardComboboxPrepareOption, NULL, QBWizardComboboxChange, QBWizardGenericComboboxChanged, NULL, NULL };
            QBComboBoxSetCallbacks(combobox, self, cb);
        }
    }

    self->comboboxes[idx] = combobox;
    QBComboBoxReinitialize(combobox); // display value
}

SvLocal int QBWizardCompareIds(void *_prv, SvGenericObject _id1, SvGenericObject _id2)
{
    SvString id1 = (SvString) _id1;
    SvString id2 = (SvString) _id2;
    // "unusedOption" is less than everything
    bool id1IsUnused = SvStringEqualToCString(id1, "unusedOption");
    bool id2IsUnused = SvStringEqualToCString(id2, "unusedOption");

    if (id1IsUnused && id2IsUnused)
        return 0;
    if (id1IsUnused)
        return -1;
    if (id2IsUnused)
        return 1;
    return strcmp(SvStringCString(id1), SvStringCString(id2));
}

/**
 * Get index of widget in collection.
 *
 * @param[in]   self        QBWizard handle
 * @param[in]   collection  self->comboboxes or self->subOptionControls
 * @param[in]   widget      widget handle
 * @return                  index of widget in collection or -1 if not found
 */
SvLocal ssize_t QBWizardGetIndexOfWidgetInCollection(QBWizard self, SvWidget* collection, SvWidget widget);

/**
 * Get option related to given combobox.
 *
 * @param[in]   self        QBWizard handle
 * @param[in]   combobox    ComboBox widget handle
 * @return                  option related to combobox
 */
SvLocal QBWizardOption QBWizardGetComboboxOption(QBWizard self, SvWidget combobox)
{
    assert(self); assert(self->comboboxes); assert(self->options); assert(combobox);

    ssize_t optionIdx = QBWizardGetIndexOfWidgetInCollection(self, self->comboboxes, combobox);
    assert(optionIdx >= 0);

    QBWizardOption option = (QBWizardOption) SvArrayObjectAtIndex(self->options, optionIdx);
    assert(option);

    return option;
}

SvLocal SvString QBWizardComboboxPrepareOption(void *self_, SvWidget combobox, SvGenericObject valueId_)
{
    assert(self_); assert(combobox); assert(valueId_);
    QBWizard self = self_;
    SvString valueId = (SvString) valueId_;

    QBWizardOption option = QBWizardGetComboboxOption(self, combobox);
    assert(option);

    SvString caption = (SvString) SvHashTableFind(option->idToCaption, (SvGenericObject) valueId);
    return SvStringCreate(gettext(SvStringCString(caption)), NULL);
}

SvLocal void QBWizardSaveOptionComboboxNewValueToContext(QBWizard self, SvWidget combobox, SvString valueId)
{
    assert(self); assert(combobox); assert(valueId);

    QBWizardOption option = QBWizardGetComboboxOption(self, combobox);
    assert(option);

    QBWizardContextSetOptionValueId(self->ctx, option->id, valueId);
}

SvLocal int QBWizardComboboxChange(void *self_, SvWidget combobox, SvGenericObject value, int key)
{
    assert(combobox); assert(value);

    SvArray content = QBComboBoxGetContent(combobox);
    ssize_t oldValueIdx = SvArrayIndexOfObject(content, value);
    if (oldValueIdx < 0)
        oldValueIdx = 0;
    return oldValueIdx;
}

SvLocal void QBWizardRefreshTranslatableWidgets(QBWizard self);

SvLocal void QBWizardLanguageChanged(void *self_, SvWidget combobox, SvGenericObject languageId_)
{
    assert(self_); assert(combobox); assert(languageId_);
    QBWizard self = self_;
    SvString languageId = (SvString) languageId_;

    QBWizardSaveOptionComboboxNewValueToContext(self, combobox, languageId);
    QBInitLogicSetLocale(self->ctx->appGlobals->initLogic, languageId);
    QBConfigSet("LANG", SvStringCString(languageId));
    QBConfigSave();

    svSettingsPushComponent("Wizard.settings");

    QBWizardRefreshTranslatableWidgets(self);

    svSettingsPopComponent();
}

SvLocal void QBWizardLNBChanged(void *self_, SvWidget combobox, SvGenericObject valueId_)
{
    assert(self_); assert(combobox); assert(valueId_);
    QBWizard self = self_;
    if (self->ctx->appGlobals->hintManager == NULL) {
        SvLogWarning("hintManager is not initialized");
        return;
    }

    SvString valueId = (SvString) valueId_;
    QBWizardSaveOptionComboboxNewValueToContext(self, combobox, valueId);
    if (SvStringEqualToCString(valueId, "1")) {
        QBHintManagerSetHintsFromString(self->ctx->appGlobals->hintManager, SVSTRING("LNB_hint_one"));
    } else if (SvStringEqualToCString(valueId, "2")) {
        QBHintManagerSetHintsFromString(self->ctx->appGlobals->hintManager, SVSTRING("LNB_hint_both"));
    }
}

SvLocal void QBWizardLNBComboboxFocusStarted(void *target, SvWidget combobox, SvGenericObject selectedValue)
{
    assert(target); assert(combobox); assert(selectedValue);
    QBWizard self = target;
    if (self->ctx->appGlobals->hintManager == NULL) {
        SvLogWarning("hintManager is not initialized");
        return;
    }

    if (SvStringEqualToCString((SvString) selectedValue, "1")) {
        QBHintManagerSetHintsFromString(self->ctx->appGlobals->hintManager, SVSTRING("LNB_hint_one"));
    } else if (SvStringEqualToCString((SvString) selectedValue, "2")) {
        QBHintManagerSetHintsFromString(self->ctx->appGlobals->hintManager, SVSTRING("LNB_hint_both"));
    }
}

SvLocal void QBWizardLNBComboboxFocusFinished(void *target, SvWidget combobox, SvGenericObject selectedValue, SvGenericObject userValue)
{
    assert(target); assert(combobox); assert(selectedValue);
    QBWizard self = target;
    if (self->ctx->appGlobals->hintManager == NULL) {
        SvLogWarning("hintManager is not initialized");
        return;
    }
    QBHintManagerSetHintsFromString(self->ctx->appGlobals->hintManager, NULL);
}

SvLocal void QBWizardReloadOptionsAndRecreateWidgets(QBWizard self);

SvLocal void QBWizardModeChanged(void *self_, SvWidget combobox, SvGenericObject modeId_)
{
    assert(self_); assert(modeId_);
    QBWizard self = self_;
    SvString modeId = (SvString) modeId_;
    if (combobox)
        QBWizardSaveOptionComboboxNewValueToContext(self, combobox, modeId);

    svSettingsPushComponent("Wizard.settings");

    QBWizardLogicModeChanged(self->ctx->appGlobals->wizardLogic,
                             self->ctx->sharedData->currentModeValues.idToOption,
                             self->ctx->sharedData->currentModeValues.idToSubOption);
    QBWizardContextModeChanged(self->ctx);
    QBWizardSetGoButtonCaption(self);
    QBWizardSetTitleText(self);
    QBWizardReloadOptionsAndRecreateWidgets(self);

    svSettingsPopComponent();
}

SvLocal void QBWizardTunerChanged(void *self_, SvWidget combobox, SvGenericObject tunerId_)
{
    assert(self_); assert(combobox); assert(tunerId_);
    QBWizard self = self_;
    SvString tunerId = (SvString) tunerId_;

    QBWizardSaveOptionComboboxNewValueToContext(self, combobox, tunerId);

    QBWizardUpdateOption(self, SVSTRING("standard"));
}

SvLocal void QBWizardProviderChanged(void *self_, SvWidget combobox, SvGenericObject newValueId_)
{
    assert(self_); assert(combobox); assert(newValueId_);
    QBWizard self = self_;
    SvString newValueId = (SvString) newValueId_;

    QBWizardOption option = QBWizardGetComboboxOption(self, combobox);
    assert(option);

    {
        SvString oldValueId = QBWizardContextGetOptionValueId(self->ctx, option->id);
        assert(oldValueId);

        if (SvObjectEquals(newValueId_, (SvObject) oldValueId))
            return;
    }

    QBWizardContextSetOptionValueId(self->ctx, option->id, newValueId);

    if (SvStringEqualToCString(option->id, "PROVIDERS.CHANNELS.DVB")) {
        goto channelsProvidersDVBChanged;
    } else if (SvStringEqualToCString(option->id, "PROVIDERS.EPG.DVB")) {
        goto epgProviderDVBChanged;
    } else if (SvStringEqualToCString(option->id, "PROVIDERS.CHANNELS.IP")) {
        goto channelsProviderIPChanged;
    } else if (SvStringEqualToCString(option->id, "PROVIDERS.EPG.IP")) {
        QBWizardUpdateOption(self, SVSTRING("PROVIDERS.EPG.DVB"));
        goto epgProviderIPChanged;
    } else if (SvStringEqualToCString(option->id, "PROVIDERS.NPVR")) {
        goto npvrProviderChanged;
    } else {
        log_error("Provider %s not supported", SvStringCString(option->id));
        return;
    }

channelsProvidersDVBChanged:
    QBWizardUpdateOption(self, SVSTRING("PROVIDERS.EPG.DVB"));
epgProviderDVBChanged:
    QBWizardUpdateOption(self, SVSTRING("PROVIDERS.CHANNELS.IP"));
channelsProviderIPChanged:
    QBWizardUpdateOption(self, SVSTRING("PROVIDERS.EPG.IP"));
epgProviderIPChanged:
    QBWizardUpdateOption(self, SVSTRING("PROVIDERS.NPVR"));
npvrProviderChanged:
    QBWizardUpdateOption(self, SVSTRING("PROVIDERS.CUTV"));
}

SvLocal bool QBWizardBinaryOptionWithSubOptionComboboxInputHandler(void *self_, SvWidget combobox, SvInputEvent e)
{
    assert(self_); assert(combobox); assert(e);

    bool retVal = false;

    if (e->ch == QBKEY_RIGHT && SvStringEqualToCString((SvString) QBComboBoxGetValue(combobox), "enabled")) {
        // pass event to the window so it can switch focus to suboption input
        retVal = svWidgetInputEventHandler(svWidgetGetParent(combobox), e);
    }

    return retVal;
}

SvLocal void QBWizardBinaryOptionWithSubOptionChanged(void *self_, SvWidget combobox, SvGenericObject valueId_)
{
    assert(self_); assert(combobox); assert(valueId_);
    QBWizard self = self_;
    assert(self->options); assert(self->comboboxes); assert(self->subOptionControls);
    SvString valueId = (SvString) valueId_;

    ssize_t optionIdx = QBWizardGetIndexOfWidgetInCollection(self, self->comboboxes, combobox);
    assert(optionIdx >= 0);

    QBWizardBinaryOption option = (QBWizardBinaryOption) SvArrayObjectAtIndex(self->options, optionIdx);
    assert(option);

    QBWizardSaveOptionComboboxNewValueToContext(self, combobox, valueId);

    SvWidget subOptionControl = self->subOptionControls[optionIdx];
    bool optionEnabled = SvStringEqualToCString(valueId, "enabled");
    if (optionEnabled) {
        assert(subOptionControl == NULL);

        svSettingsPushComponent("Wizard.settings");
        if (SvObjectIsInstanceOf((SvObject) option->subOption, QBWizardNumericSubOption_getType())) {
            QBWizardCreateNumericSubOptionControl(self, option, optionIdx);
        }
        // other suboption types here
        svSettingsPopComponent();
    } else {
        assert(subOptionControl != NULL);

        QBWizardContextRemoveSubOptionValue(self->ctx, option->super_.id);

        svWidgetDetach(subOptionControl);
        svWidgetDestroy(subOptionControl);
        self->subOptionControls[optionIdx] = NULL;
    }
}

SvLocal void QBWizardGenericComboboxChanged(void *self_, SvWidget combobox, SvGenericObject valueId_)
{
    assert(self_); assert(combobox); assert(valueId_);
    QBWizard self = self_;
    SvString valueId = (SvString) valueId_;

    QBWizardSaveOptionComboboxNewValueToContext(self, combobox, valueId);
}

SvLocal SvWidget QBWizardCreateSubOptionControl(QBWizard self, size_t idx);

SvLocal SvString QBWizardComboboxPrepareNumericSubOption(void *self_, SvWidget combobox, SvGenericObject value_);
SvLocal bool QBWizardNumericSubOptionComboboxInputHandler(void *self_, SvWidget combobox, SvInputEvent e);
SvLocal void QBWizardNumericSubOptionControlFinished(void *self_, SvWidget combobox, SvGenericObject selectedValue, SvGenericObject userValue);

SvLocal void QBWizardCreateNumericSubOptionControl(QBWizard self, QBWizardBinaryOption option, size_t idx)
{
    assert(self); assert(option); assert(option->subOption);
    QBWizardNumericSubOption subOption = (QBWizardNumericSubOption) option->subOption;

    SvWidget control = QBWizardCreateSubOptionControl(self, idx);

    // combobox needs content array even if it's unused
    {
        SvArray subOptionContent = SvArrayCreate(NULL);

        SvGenericObject whatever = SvTypeAllocateInstance(SvObject_getType(), NULL);
        SvArrayAddObject(subOptionContent, whatever);
        SVRELEASE(whatever);

        QBComboBoxSetContent(control, subOptionContent);
        SVRELEASE(subOptionContent);
    }

    SvValue value = QBWizardContextGetSubOptionValue(self->ctx, option->super_.id);
    if (!value)
        value = subOption->defaultValue;

    QBComboBoxSetValue(control, (SvGenericObject) value);

    QBComboBoxCallbacks cb = { QBWizardComboboxPrepareNumericSubOption, QBWizardNumericSubOptionComboboxInputHandler,
                               NULL, NULL, NULL, QBWizardNumericSubOptionControlFinished };
    QBComboBoxSetCallbacks(control, self, cb);

    self->subOptionControls[idx] = control;
    QBComboBoxReinitialize(control);
}

// other suboption types here

SvLocal SvWidget QBWizardCreateSubOptionControl(QBWizard self, size_t idx)
{
    assert(self);

    SvWidget control = QBComboBoxNewFromSM(self->ctx->appGlobals->res, "Wizard.Box");
    assert(control);
    svWidgetSetFocusable(control, true);
    svWidgetAttach(self->window, control, self->xSubOptionOffset,
                   self->yOffset + idx * self->yOffsetJump +
                   (self->useDescriptions ? idx * self->descriptionHeight : 0), 1);

    return control;
}

SvLocal SvString QBWizardComboboxPrepareNumericSubOption(void *self_, SvWidget combobox, SvGenericObject value_)
{
    assert(value_);
    SvValue value = (SvValue) value_;
    int numericValue = SvValueGetInteger(value);

    return numericValue < 0
            ? SvStringCreate("---", NULL)
            : SvStringCreateWithFormat("%d", numericValue);
}

/**
 * Get BinaryOption related to given subOptionControl.
 * @param[in]   self                QBWizard handle
 * @param[in]   subOptionControl    suboption widget handle
 * @return                          binary option related to given suboption widget.
 */
SvLocal QBWizardBinaryOption QBWizardGetSubOptionControlsBinaryOption(QBWizard self, SvWidget subOptionControl);

SvLocal bool QBWizardNumericSubOptionComboboxInputHandler(void *self_, SvWidget combobox, SvInputEvent e)
{
    assert(self_); assert(combobox); assert(e);
    QBWizard self = self_;
    assert(self->options); assert(self->subOptionControls);

    const int key = e->ch;

    if (key < '0' || key > '9') {
        return false;
    }

    int digit = (key - '0');

    QBWizardBinaryOption option = QBWizardGetSubOptionControlsBinaryOption(self, combobox);
    QBWizardNumericSubOption subOption = (QBWizardNumericSubOption) option->subOption;

    int newValue;
    if (self->shouldNumericalInputsRestart) {
        newValue = digit;
        self->shouldNumericalInputsRestart = false;
    } else {
        int oldValue = SvValueGetInteger((SvValue) QBComboBoxGetValue(combobox));
        if (unlikely(oldValue < 0))
            oldValue = 0;
        newValue = oldValue * 10 + digit;
        if (newValue > subOption->numericalMax) {
            newValue = digit;
        }
    }

    SvValue value = SvValueCreateWithInteger(newValue, NULL);
    QBComboBoxSetValue(combobox, (SvGenericObject) value);
    SVRELEASE(value);

    return true;
}

SvLocal void QBWizardSaveSubOptionControlNewValueToContext(QBWizard self, SvWidget subOptionControl, SvValue value);

SvLocal void QBWizardNumericSubOptionControlFinished(void *self_, SvWidget combobox, SvGenericObject selectedValue, SvGenericObject userValue_)
{
    assert(self_); assert(userValue_);
    QBWizard self = (QBWizard) self_;
    SvValue value = (SvValue) userValue_;

    if (SvValueGetInteger(value) >= 0)
        QBWizardSaveSubOptionControlNewValueToContext(self, combobox, value);

    self->shouldNumericalInputsRestart = true;
}

// other suboption type callbacks here

SvLocal void QBWizardSaveSubOptionControlNewValueToContext(QBWizard self, SvWidget subOptionControl, SvValue value)
{
    assert(self); assert(subOptionControl); assert(value);

    QBWizardOption option = (QBWizardOption) QBWizardGetSubOptionControlsBinaryOption(self, subOptionControl);
    assert(option);

    QBWizardContextSetSubOptionValue(self->ctx, option->id, value);
}

SvLocal QBWizardBinaryOption QBWizardGetSubOptionControlsBinaryOption(QBWizard self, SvWidget subOptionControl)
{
    assert(self); assert(self->options); assert(self->subOptionControls); assert(subOptionControl);

    ssize_t optionIdx = QBWizardGetIndexOfWidgetInCollection(self, self->subOptionControls, subOptionControl);
    assert(optionIdx >= 0);

    QBWizardBinaryOption option = (QBWizardBinaryOption) SvArrayObjectAtIndex(self->options, optionIdx);
    assert(option);

    return option;
}

SvLocal ssize_t QBWizardGetIndexOfWidgetInCollection(QBWizard self, SvWidget* collection, SvWidget widget)
{
    assert(self); assert(self->options); assert(collection); assert(widget);

    for (size_t i = 0; i < SvArrayCount(self->options); ++i) {
        if (collection[i] == widget) {
            return i;
        }
    }
    return -1;
}

// == desctuction ==

SvLocal void QBWizardDeallocateArraysForWidgets(QBWizard self);

SvLocal void QBWizardDestroy(QBWizard self)
{
    assert(self); assert(self->options);

    SVRELEASE(self->options);
    QBWizardDeallocateArraysForWidgets(self);
    free(self);
}


SvLocal void QBWizardDeallocateArraysForWidgets(QBWizard self)
{
    assert(self);

    free(self->comboboxes);
    free(self->labels);
    free(self->subOptionControls);
    free(self->descriptions);
    self->comboboxes = NULL;
    self->labels = NULL;
    self->subOptionControls = NULL;
    self->descriptions = NULL;
}

// == updates ==

/**
 * Get index of option.
 *
 * @param   self    QBWizard handle
 * @param   id      option id
 * @return          index of option in self->options which id is same as given id or -1 if not found.
 */
SvLocal ssize_t QBWizardGetOptionIdx(QBWizard self, const char* id);

SvLocal SvArray QBWizardCreateNewValuesForOption(QBWizard self, SvString optionId);

SvLocal void QBWizardUpdateOption(QBWizard self, SvString optionId)
{
    assert(self); assert(optionId);

    SvString oldValue = QBWizardContextGetOptionValueId(self->ctx, optionId);
    if (!oldValue) // if option does not exist in the current mode, just do nothing. Default value will be used by whoever need it.
        return;

    SvArray newValues = QBWizardCreateNewValuesForOption(self, optionId);
    assert(newValues);
    assert(SvArrayCount(newValues) > 0);

    SvArraySortWithCompareFn(newValues, QBWizardCompareIds, NULL);

    if (!SvArrayContainsObject(newValues, (SvGenericObject) oldValue)) {
        // The newValue must be the same that will be picked-up by the combobox. See QBWizardComboboxChange
        SvString newValue = (SvString) SvArrayObjectAtIndex(newValues, 0);
        assert(newValue);
        QBWizardContextSetOptionValueId(self->ctx, optionId, newValue);
    }

    // It may be possible, that the option is available in the current mode, but not on the current screen.

    ssize_t idx = QBWizardGetOptionIdx(self, SvStringCString(optionId));
    if (idx >= 0) {
        SvWidget combobox = self->comboboxes[idx];
        QBComboBoxSetContent(combobox, newValues);

        /// Logic prevents no-epg situation in GUI
        ssize_t conditionIndex = -1;
        if (SvStringEqualToCString(optionId, "PROVIDERS.EPG.DVB")) {
            conditionIndex = QBWizardGetOptionIdx(self, "PROVIDERS.EPG.IP");
        } else if (SvStringEqualToCString(optionId, "PROVIDERS.EPG.IP")) {
            conditionIndex = QBWizardGetOptionIdx(self, "PROVIDERS.EPG.DVB");
        }

        if (conditionIndex >= 0) {
            SvObject valueObj = QBComboBoxGetValue(self->comboboxes[conditionIndex]);
            if (SvObjectIsInstanceOf(valueObj, SvString_getType()) && SvStringEqualToCString((SvString) valueObj, "unusedOption")) {
                // values are sorted above and QBWizardCompareIds sorting functions provides unusedOption at 0 position. So set to 1.
                QBComboBoxSetPosition(self->comboboxes[idx], 1);
            }
        }
    }

    SVRELEASE(newValues);
}

SvLocal SvArray QBWizardCreateNewValuesForScanStandard(QBWizard self);

SvLocal SvArray QBWizardCreateNewValuesForEPGDVB(QBWizard self);
SvLocal SvArray QBWizardCreateNewValuesForChannelsIP(QBWizard self);
SvLocal SvArray QBWizardCreateNewValuesForEPGIP(QBWizard self);
SvLocal SvArray QBWizardCreateNewValuesForNPVR(QBWizard self);
SvLocal SvArray QBWizardCreateNewValuesForCUTV(QBWizard self);

SvLocal SvArray QBWizardCreateNewValuesForOption(QBWizard self, SvString optionId)
{
    assert(self); assert(optionId);

    if (SvStringEqualToCString(optionId, "standard"))
        return QBWizardCreateNewValuesForScanStandard(self);
    else if (SvStringEqualToCString(optionId, "PROVIDERS.EPG.DVB"))
        return QBWizardCreateNewValuesForEPGDVB(self);
    else if (SvStringEqualToCString(optionId, "PROVIDERS.CHANNELS.IP"))
        return QBWizardCreateNewValuesForChannelsIP(self);
    else if (SvStringEqualToCString(optionId, "PROVIDERS.EPG.IP"))
        return QBWizardCreateNewValuesForEPGIP(self);
    else if (SvStringEqualToCString(optionId, "PROVIDERS.NPVR"))
        return QBWizardCreateNewValuesForNPVR(self);
    else if (SvStringEqualToCString(optionId, "PROVIDERS.CUTV"))
        return QBWizardCreateNewValuesForCUTV(self);
    else {
        log_error("%s: option %s not supported", __FUNCTION__, SvStringCString(optionId));
        return NULL;
    }
}

SvLocal SvArray QBWizardCreateNewValuesForScanStandard(QBWizard self)
{
    assert(self);

    SvString tunerId = QBWizardContextGetOptionValueIdOrDefault(self->ctx, SVSTRING("tuner"));

    SvArray newValues = SvArrayCreate(NULL);
    QBWizardOption option = QBWizardContextGetOptionById(self->ctx, SVSTRING("standard"));

    if (!SvStringEqualToCString(tunerId, "unusedOption")) {
        int tunerConfigSetIdx = QBTunerFindConfigSetByName("native", SvStringCString(tunerId));
        const struct QBTunerConfigSet* configSet = QBTunerGetConfigSet("native", tunerConfigSetIdx);
        const struct QBTunerProperties* props = QBTunerGetPropertiesById("native", configSet->tab[0]);

        for (int i=0; i<props->allowed.standard_cnt; ++i) {
            SvString standardId = SvStringCreate(QBTunerStandardToString(props->allowed.standard_tab[i]), NULL);
            if (SvHashTableFind(option->idToCaption, (SvGenericObject) standardId))
                SvArrayAddObject(newValues, (SvGenericObject) standardId);
            SVRELEASE(standardId);
        }
    }
    if (SvArrayCount(newValues) == 0) {
        /// TODO: this option should be somehow disabled, but for now, we just set this fake value -deffer 2012.01.10
        SvArrayAddObject(newValues, (SvGenericObject) SVSTRING("unusedOption"));
    }

    return newValues;
}

SvLocal SvArray QBWizardCreateNewValuesForEPGDVB(QBWizard self)
{
    assert(self);

    SvArray newValues = SvArrayCreate(NULL);
    QBWizardOption option = QBWizardContextGetOptionById(self->ctx, SVSTRING("PROVIDERS.EPG.DVB"));

    SvString channelsProviderDVBId = QBWizardContextGetOptionValueIdOrDefault(self->ctx, SVSTRING("PROVIDERS.CHANNELS.DVB"));
    if (!SvStringEqualToCString(channelsProviderDVBId, "disabled")) {
        SvIterator it = SvHashTableKeysIterator(option->idToCaption);
        SvArrayAddObjects(newValues, &it);
    } else {
        if (SvHashTableFind(option->idToCaption, (SvObject) SVSTRING("unusedOption"))) {
            SvArrayAddObject(newValues, (SvObject) SVSTRING("unusedOption"));
        } else {
            log_error("Not found unusedOption as valueId for PROVIDERS.EPG.DVB, check config");
        }
    }

    return newValues;
}

SvLocal SvArray QBWizardCreateNewValuesForChannelsIP(QBWizard self)
{
    assert(self);

    SvArray newValues = SvArrayCreate(NULL);
    QBWizardOption option = QBWizardContextGetOptionById(self->ctx, SVSTRING("PROVIDERS.CHANNELS.IP"));

    SvIterator it = SvHashTableKeysIterator(option->idToCaption);
    SvString channelsIPProviderId;
    while ((channelsIPProviderId = (SvString) SvIteratorGetNext(&it)))
        SvArrayAddObject(newValues, (SvObject) channelsIPProviderId);

    return newValues;
}

SvLocal SvArray QBWizardCreateNewValuesForEPGIP(QBWizard self)
{
    assert(self);

    SvArray newValues = SvArrayCreate(NULL);
    QBWizardOption option = QBWizardContextGetOptionById(self->ctx, SVSTRING("PROVIDERS.EPG.IP"));
    SvString channelsProviderIPId = QBWizardContextGetOptionValueIdOrDefault(self->ctx, SVSTRING("PROVIDERS.CHANNELS.IP"));
    bool channelsIPIsDisabled = SvStringEqualToCString(channelsProviderIPId, "unusedOption");

    if (!channelsIPIsDisabled) {
        SvIterator it = SvHashTableKeysIterator(option->idToCaption);
        SvArrayAddObjects(newValues, &it);
    } else {
        if (SvHashTableFind(option->idToCaption, (SvObject) SVSTRING("unusedOption"))) {
            SvArrayAddObject(newValues, (SvObject) SVSTRING("unusedOption"));
        } else {
            log_error("Not found unusedOption as valueId for PROVIDERS.EPG.IP, check config");
        }
    }

    return newValues;
}

SvLocal SvArray QBWizardCreateNewValuesForNPVR(QBWizard self)
{
    assert(self);

    SvArray newValues = SvArrayCreate(NULL);
    QBWizardOption option = QBWizardContextGetOptionById(self->ctx, SVSTRING("PROVIDERS.NPVR"));
    SvString epgIPProviderId = QBWizardContextGetOptionValueId(self->ctx, SVSTRING("PROVIDERS.EPG.IP"));

    if (epgIPProviderId && !SvStringEqualToCString(epgIPProviderId, "unusedOption")) {
        SvIterator it = SvHashTableKeysIterator(option->idToCaption);
        SvString npvrProviderId;
        while ((npvrProviderId = (SvString) SvIteratorGetNext(&it))) {
            if (SvObjectEquals((SvObject) npvrProviderId, (SvObject) epgIPProviderId) || SvStringEqualToCString(npvrProviderId, "unusedOption"))
                SvArrayAddObject(newValues, (SvObject) npvrProviderId);
        }
    } else {
        if (SvHashTableFind(option->idToCaption, (SvObject) SVSTRING("unusedOption"))) {
            SvArrayAddObject(newValues, (SvObject) SVSTRING("unusedOption"));
        } else {
            log_error("Not found unusedOption as valueId for PROVIDERS.NPVR, check config");
        }
    }

    return newValues;
}

SvLocal SvArray QBWizardCreateNewValuesForCUTV(QBWizard self)
{
    assert(self);

    SvArray newValues = SvArrayCreate(NULL);
    QBWizardOption option = QBWizardContextGetOptionById(self->ctx, SVSTRING("PROVIDERS.CUTV"));
    SvString epgIPProviderId = QBWizardContextGetOptionValueId(self->ctx, SVSTRING("PROVIDERS.EPG.IP"));

    if (epgIPProviderId && !SvStringEqualToCString(epgIPProviderId, "unusedOption")) {
        SvIterator it = SvHashTableKeysIterator(option->idToCaption);
        SvString npvrProviderId;
        while ((npvrProviderId = (SvString) SvIteratorGetNext(&it))) {
            if (SvObjectEquals((SvObject) npvrProviderId, (SvObject) epgIPProviderId) || SvStringEqualToCString(npvrProviderId, "unusedOption"))
                SvArrayAddObject(newValues, (SvObject) npvrProviderId);
        }
    } else {
        if (SvHashTableFind(option->idToCaption, (SvObject) SVSTRING("unusedOption"))) {
            SvArrayAddObject(newValues, (SvObject) SVSTRING("unusedOption"));
        } else {
            log_error("Not found unusedOption as valueId for PROVIDERS.CUTV, check config");
        }
    }

    return newValues;
}

SvLocal ssize_t QBWizardGetOptionIdx(QBWizard self, const char* id)
{
    assert(self); assert(self->options); assert(id);

    size_t idx = 0;
    SvIterator it = SvArrayIterator(self->options);
    QBWizardOption option;
    while ((option = (QBWizardOption) SvIteratorGetNext(&it))) {
        if (SvStringEqualToCString(option->id, id)) {
            return idx;
        }
        ++idx;
    }
    return -1;
}

SvLocal void QBWizardRefreshOptionsWidgets(QBWizard self);

SvLocal void QBWizardRefreshAllWidgets(QBWizard self)
{
    assert(self);

    QBWizardRefreshLogo(self);
    QBWizardSetTitleText(self);
    QBWizardSetGoButtonCaption(self);
    QBWizardRefreshOptionsWidgets(self);
}

SvLocal void QBWizardRefreshLogo(QBWizard self)
{
    assert(self);

    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(self->ctx->appGlobals->initLogic);
    if (logoWidgetName)
        self->logo = QBCustomerLogoReplace(self->logo, self->window, logoWidgetName, 1);
}

SvLocal void QBWizardRefreshTranslatableWidgets(QBWizard self)
{
    assert(self);

    QBWizardSetTitleText(self);
    QBWizardSetGoButtonCaption(self);
    QBWizardRefreshOptionsWidgets(self);
}

SvLocal void QBWizardRefreshOptionsWidgets(QBWizard self)
{
    assert(self); assert(self->options); assert(self->comboboxes); assert(self->labels); assert(self->subOptionControls);
    assert(self->descriptions);

    for(size_t i = 0; i < SvArrayCount(self->options); ++i) {
        QBComboBoxReinitialize(self->comboboxes[i]);
        if (self->subOptionControls[i]) {
            QBComboBoxReinitialize(self->subOptionControls[i]);
        }
        QBWizardOption option = (QBWizardOption) SvArrayObjectAtIndex(self->options, i);
        svLabelSetText(self->labels[i], gettext(SvStringCString(option->caption)));
        if (self->descriptions[i] && option->description) {
            svLabelSetText(self->descriptions[i], gettext(SvStringCString(option->description)));
        }
    }
}

SvLocal size_t QBWizardGetFocusLevel(QBWizard self);
SvLocal void QBWizardDestroyOptionsWidgets(QBWizard self);
SvLocal void QBWizardClearOptions(QBWizard self);
SvLocal void QBWizardTrySetFocusLevel(QBWizard self, size_t focusLevel);

SvLocal void QBWizardReloadOptionsAndRecreateWidgets(QBWizard self)
{
    assert(self);

    size_t focusLevel = QBWizardGetFocusLevel(self);

    QBWizardDestroyOptionsWidgets(self);
    QBWizardClearOptions(self);

    QBWizardLoadOptions(self);
    QBWizardCreateOptionsWidgets(self);

    QBWizardTrySetFocusLevel(self, focusLevel);
}

SvLocal size_t QBWizardGetFocusLevel(QBWizard self)
{
    assert(self); assert(self->options); assert(self->comboboxes);

    size_t focusLevel;
    for (focusLevel = 0; focusLevel < SvArrayCount(self->options); ++focusLevel) {
        if (svWidgetIsFocused(self->comboboxes[focusLevel])) {
            break;
        }
    }
    return focusLevel;
}

SvLocal void QBWizardDestroyOptionsWidgets(QBWizard self)
{
    assert(self); assert(self->options); assert(self->comboboxes); assert(self->labels); assert(self->subOptionControls);
    assert(self->descriptions);

    for(size_t i = 0; i < SvArrayCount(self->options); ++i) {
        svWidgetDetach(self->comboboxes[i]);
        svWidgetDestroy(self->comboboxes[i]);

        svWidgetDetach(self->labels[i]);
        svWidgetDestroy(self->labels[i]);

        if (self->subOptionControls[i]) {
            svWidgetDetach(self->subOptionControls[i]);
            svWidgetDestroy(self->subOptionControls[i]);
        }

        if (self->descriptions[i]) {
            svWidgetDetach(self->descriptions[i]);
            svWidgetDestroy(self->descriptions[i]);
        }
    }
}

SvLocal void QBWizardClearOptions(QBWizard self)
{
    assert(self); assert(self->options);

    QBWizardDeallocateArraysForWidgets(self);
    SvArrayRemoveAllObjects(self->options);
}

SvLocal void QBWizardTrySetFocusLevel(QBWizard self, size_t focusLevel)
{
    assert(self); assert(self->options); assert(self->comboboxes);

    if (SvArrayCount(self->options) == 0)
        return;

    size_t maxFocusLevel = SvArrayCount(self->options) - 1;
    if (focusLevel > maxFocusLevel)
        focusLevel = maxFocusLevel;
    svWidgetSetFocus(self->comboboxes[focusLevel]);
}
