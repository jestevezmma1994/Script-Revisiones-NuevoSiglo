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

// This file should be named QBWizardContext.c

#include "wizard.h"
#include "wizardInternal.h"
#include <main.h>
#include <QBConf.h>
#include <QBPlatformHAL/QBPlatformFrontPanel.h>
#include <settings.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvData.h>
#include <SvFoundation/SvString.h>
#include <SvJSON/SvJSONParse.h>
#include <QBAppKit/QBObserver.h>
#include "wizard/QBWizard.h"
#include "wizard/QBWizardOption.h"
#include "wizard/QBWizardBinaryOption.h"
#include "wizard/QBWizardNumericSubOption.h"

#define log_error(FMT, ...) do{ SvLogError("QBWizardContext " FMT, ##__VA_ARGS__); }while(0)

SvLocal void QBWizardContextDestroy(void *self_);
SvLocal void QBWizardContextCreateWindow(QBWindowContext self_, SvApplication app);
SvLocal void QBWizardContextReinitializeWindow(QBWindowContext self_, SvArray itemList);
SvLocal void QBWizardContextDestroyWindow(QBWindowContext self_);
SvLocal void QBWizardContextHybridModeConfigChanged(SvObject self_, SvObject observedObject, SvObject arg);

SvLocal SvType QBWizardContext_getType(void)
{
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_             = {
            .destroy        = QBWizardContextDestroy
        },
        .reinitializeWindow = QBWizardContextReinitializeWindow,
        .createWindow       = QBWizardContextCreateWindow,
        .destroyWindow      = QBWizardContextDestroyWindow
    };
    static const struct QBObserver_ observerMethods = {
        .observedObjectUpdated = QBWizardContextHybridModeConfigChanged,
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBWizardContext",
                            sizeof(struct QBWizardContext_s),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            QBObserver_getInterface(), &observerMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBWizardContextDestroy(void *self_)
{
    QBWizardContext self = self_;
    SVRELEASE(self->sharedData);
}

SvLocal void QBWizardContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBWizardContext self = (QBWizardContext) self_;
    QBPlatformShowOnFrontPanel("Installation");
    svSettingsPushComponent("Wizard.settings");
    SvWindow window = QBWizardWindowCreate(self, app);
    svSettingsPopComponent();
    self->super_.window = window;
    QBWizardLogicAddListener(self->appGlobals->wizardLogic, (SvGenericObject) self);
    if (self->appGlobals->qbMWConfigMonitor && self->appGlobals->qbMWConfigHandler) {
        QBMWConfigHandlerAddListener(self->appGlobals->qbMWConfigHandler, (SvObject) self);
        QBMWConfigMonitorStart(self->appGlobals->qbMWConfigMonitor);
    }
}

SvLocal void QBWizardContextReinitializeWindow(QBWindowContext self_, SvArray itemList)
{
    QBWizardContext self = (QBWizardContext) self_;
    svSettingsPushComponent("Wizard.settings");
    QBWizardWindowReinitialize(self->super_.window, itemList);
    svSettingsPopComponent();
}

SvLocal void QBWizardContextDestroyWindow(QBWindowContext self_)
{
    QBWizardContext self = (QBWizardContext) self_;
    QBPlatformShowOnFrontPanel(" ");
    if (self->appGlobals->qbMWConfigMonitor && self->appGlobals->qbMWConfigHandler)
        QBMWConfigHandlerRemoveListener(self->appGlobals->qbMWConfigHandler, (SvObject) self);
    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal void QBWizardContextHybridModeConfigChanged(SvObject self_, SvObject observedObject, SvObject arg)
{
    QBWizardContext self = (QBWizardContext) self_;
    SvValue msg = (SvValue) arg;
    if (SvValueGetInteger(msg) == CONFIG_STATE_NOREAD) {
        if (!self->modeComboboxEnabled) {
            self->modeComboboxEnabled = true;
            QBWizardWindowReinitialize(self->super_.window, self->sharedData->currentModeOptions);
        }
    } else {
        QBWizardWindowReinitialize(self->super_.window, self->sharedData->currentModeOptions);
    }
}

SvLocal void QBWizardContextSharedData__dtor__(void *self_);

SvLocal SvType QBWizardContextSharedData_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBWizardContextSharedData__dtor__
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBWizardContextSharedData",
                            sizeof(struct QBWizardContextSharedData_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal void QBWizardContextSharedData__dtor__(void *self_)
{
    QBWizardContextSharedData self = self_;
    SVRELEASE(self->currentModeOptions);
    SVRELEASE(self->allModesValues.idToOption);
    SVRELEASE(self->allModesValues.idToSubOption);
    SVRELEASE(self->currentModeValues.idToOption);
    SVRELEASE(self->currentModeValues.idToSubOption);
    SVRELEASE(self->defaultValues.idToOption);
    SVRELEASE(self->defaultValues.idToSubOption);
}

SvLocal QBWizardContextSharedData QBWizardContextSharedDataCreate(void)
{
    QBWizardContextSharedData self = (QBWizardContextSharedData) SvTypeAllocateInstance(QBWizardContextSharedData_getType(), NULL);
    self->currentModeOptions = SvArrayCreate(NULL);
    self->allModesValues.idToOption = SvHashTableCreate(10, NULL);
    self->allModesValues.idToSubOption = SvHashTableCreate(10, NULL);
    self->currentModeValues.idToOption = SvHashTableCreate(10, NULL);
    self->currentModeValues.idToSubOption = SvHashTableCreate(10, NULL);
    self->defaultValues.idToOption = SvHashTableCreate(10, NULL);
    self->defaultValues.idToSubOption = SvHashTableCreate(10, NULL);
    return self;
}

SvLocal void QBWizardContextRegisterJSONHelpers(SvErrorInfo *errorOut);

/**
 * Clear currentModeOptions and fill it with valid options loaded from file.
 *
 * If there's fatal error it's reported to error log and array is left empty.
 * If there's single invalid option, it's omited and reported to error log.
 */
SvLocal void QBWizardContextLoadOptionsFromConfigFile(QBWizardContext self);

SvLocal void QBWizardContextFillDefaultValues(QBWizardContext self);
SvLocal void QBWizardContextFillCurrentModeValues(QBWizardContext self);

QBWindowContext QBWizardContextCreate(AppGlobals appGlobals)
{
    SvErrorInfo error = NULL;
    QBWizardContext self = NULL;

    if (unlikely(!appGlobals)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "NULL appGlobals passed");
        goto fini;
    }

    QBWizardContextRegisterJSONHelpers(&error);
    if (unlikely(error))
        goto fini;

    self = (QBWizardContext) SvTypeAllocateInstance(QBWizardContext_getType(), &error);
    if (unlikely(!self))
        goto fini;

    self->appGlobals = appGlobals;
    self->screenNumber = 0;
    self->modeComboboxEnabled = false;
    self->isPollPopupSet = false;

    self->sharedData = QBWizardContextSharedDataCreate();
    QBWizardContextLoadOptionsFromConfigFile(self);
    self->sharedData->allScreens = 0;
    QBWizardContextFillDefaultValues(self);
    QBWizardContextFillCurrentModeValues(self);

    {
        SvString lang = SvStringCreate(QBConfigGet("LANG"), NULL);
        QBWizardContextSetOptionValueId(self, SVSTRING("lang"), lang);
        SVRELEASE(lang);
    }

    self->super_.window = NULL;

fini:
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    return (QBWindowContext) self;
}

SvLocal void QBWizardContextRegisterJSONHelpers(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    QBWizardOptionRegisterJSONHelper(&error);
    if (unlikely(error))
        goto fini;

    QBWizardBinaryOptionRegisterJSONHelper(&error);
    if (unlikely(error))
        goto fini;

    QBWizardNumericSubOptionRegisterJSONHelper(&error);

fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void QBWizardContextFillDefaultValues(QBWizardContext self)
{
    assert(self);
    struct QBWizardContextValues *defaultValues = &(self->sharedData->defaultValues);

    const char* const defaultKeysAndValues[][2] = {
        {"PROVIDERS.CHANNELS.DVB", "enabled"},
        {"PROVIDERS.EPG.DVB", "eit"},
        {"PROVIDERS.CHANNELS.IP", "unusedOption"},
        {"PROVIDERS.EPG.IP", "unusedOption"},
        {"PROVIDERS.NPVR", "unusedOption"},
        {"PROVIDERS.CUTV", "unusedOption"},
        {"PROVIDERS.VOD", "cubimw"},
        {"PROVIDERS.WEBSERVICES", "cubimw"}
    };
    // For every key, use value from QBConfig or, if not found, from array.
    for (size_t i = 0; i < sizeof(defaultKeysAndValues)/sizeof(defaultKeysAndValues[0]); ++i) {
        SvString key = SvStringCreate(defaultKeysAndValues[i][0], NULL);
        SvString value = NULL;
        {
            const char* const confValue = QBConfigGet(SvStringCString(key));
            if (confValue)
                value = SvStringCreate(confValue, NULL);
            else
                value = SvStringCreate(defaultKeysAndValues[i][1], NULL);
        }

        SvHashTableInsert(defaultValues->idToOption, (SvGenericObject) key, (SvGenericObject) value);
        SVRELEASE(key);
        SVRELEASE(value);
    }
}

SvLocal void QBWizardContextFillCurrentModeValues(QBWizardContext self)
{
    assert(self);

    /*
    for option in currentModeOptions
            if option has default value
                allModesValues[option->id] = currentModeValues[option->id] = option->defaultValueId
            else
                allModesValues[option->id] = currentModeValues[option->id] = first key of option->idToCaption
    */


    SvIterator it = SvArrayIterator(self->sharedData->currentModeOptions);
    QBWizardOption option;
    while ((option = (QBWizardOption) SvIteratorGetNext(&it))) {
        if (option->defaultValueId) {
            QBWizardContextSetOptionValueId(self, option->id, option->defaultValueId);
        } else {
            SvIterator itID = SvHashTableKeysIterator(option->idToCaption);
            SvString key = (SvString) SvIteratorGetNext(&itID);
            QBWizardContextSetOptionValueId(self, option->id, key);
        }
        if (SvObjectIsInstanceOf((SvObject) option, QBWizardBinaryOption_getType())) {
            QBWizardBinaryOption binaryOption = (QBWizardBinaryOption) option;
            if (SvObjectIsInstanceOf(binaryOption->subOption, QBWizardNumericSubOption_getType()))
                QBWizardContextSetSubOptionValue(self, option->id, ((QBWizardNumericSubOption) binaryOption->subOption)->defaultValue);
        }
    }
}

SvHidden QBWindowContext QBWizardContextCreateNextScreen(QBWizardContext prevContext)
{
    assert(prevContext);

    QBWizardContext self = (QBWizardContext) SvTypeAllocateInstance(QBWizardContext_getType(), NULL);
    self->appGlobals = prevContext->appGlobals;
    self->screenNumber = prevContext->screenNumber + 1;
    self->sharedData = SVRETAIN(prevContext->sharedData);
    self->modeComboboxEnabled = prevContext->modeComboboxEnabled;
    self->isPollPopupSet = true;
    QBWizardLogicAddListener(self->appGlobals->wizardLogic, (SvGenericObject) self);
    return (QBWindowContext) self;
}

SvLocal void QBWizardContextLoadOptionsFromConfigFile(QBWizardContext self)
{
    assert(self);
    SvErrorInfo error = NULL;
    SvString optionFileName = NULL;
    SvArray parsedData = NULL;
    SvData buffer = NULL;

    SvArrayRemoveAllObjects(self->sharedData->currentModeOptions);

    optionFileName = QBWizardLogicGetOptionFileName(self->appGlobals->wizardLogic); // Contrary to its name, it creates a new string.

    buffer = SvDataCreateFromFile(SvStringCString(optionFileName), 32768, &error);
    if (unlikely(!buffer))
        goto fini;

    parsedData = (SvArray) SvJSONParseData(SvDataBytes(buffer), SvDataLength(buffer), true, &error);
    if (unlikely(!parsedData))
        goto fini;

    if (unlikely(!SvObjectIsInstanceOf((SvObject) parsedData, SvArray_getType()))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "content loaded from %s is not a list", SvStringCString(optionFileName));
        SVRELEASE(parsedData);
        parsedData = NULL;
        goto fini;
    }

    {
        SvIterator it = SvArrayIterator(parsedData);
        int i = 0;
        SvGenericObject object;
        while ((object = SvIteratorGetNext(&it))) {
            if (SvObjectIsInstanceOf(object, QBWizardOption_getType())) {
                object = (SvObject) QBWizardLogicCreateWizardOption(self->appGlobals->wizardLogic, (QBWizardOption) object);
                if (object) {
                    SvArrayAddObject(self->sharedData->currentModeOptions, object);
                    SVRELEASE(object);
                }
            } else {
                log_error("object on position %d is not a QBWizardOption", i);
            }
            ++i;
        }
    }

fini:
    SVTESTRELEASE(buffer);
    SVTESTRELEASE(parsedData);
    SVTESTRELEASE(optionFileName);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
}

SvHidden void QBWizardContextModeChanged(QBWizardContext self)
{
    assert(self);

    QBWizardContextLoadOptionsFromConfigFile(self);

    self->sharedData->allScreens = 0;

    SvHashTableRemoveAllObjects(self->sharedData->currentModeValues.idToOption);
    SvHashTableRemoveAllObjects(self->sharedData->currentModeValues.idToSubOption);
    QBWizardContextFillCurrentModeValues(self);
}

SvHidden SvArray QBWizardContextSelectOptionsOfCurrentScreen(QBWizardContext self)
{
    assert(self);
    SvArray options = SvArrayCreate(NULL);
    SvIterator it = SvArrayIterator(self->sharedData->currentModeOptions);
    QBWizardOption option;
    while ((option = (QBWizardOption) SvIteratorGetNext(&it)))
        // Option 'mode' is displayed only when hybrid mode is not read from middleware.
        if ((option->screenNumber == self->screenNumber) && ((!SvStringEqualToCString(option->id, "mode") || !self->appGlobals->qbMWConfigHandler) || self->modeComboboxEnabled))
            SvArrayAddObject(options, (SvGenericObject) option);
    return options;
}

SvHidden QBWizardOption QBWizardContextGetOptionById(QBWizardContext self, SvString optionId)
{
    assert(self); assert(optionId);
    SvIterator it = SvArrayIterator(self->sharedData->currentModeOptions);
    QBWizardOption option = NULL;
    while ((option = (QBWizardOption) SvIteratorGetNext(&it))) {
        if (SvObjectEquals((SvObject) option->id, (SvObject) optionId))
            break;
    }
    return option;
}

SvHidden void QBWizardContextSetOptionValueId(QBWizardContext self, SvString optionId, SvString valueId)
{
    assert(self); assert(optionId); assert(valueId);
    SvHashTableInsert(self->sharedData->allModesValues.idToOption, (SvGenericObject) optionId, (SvGenericObject) valueId);
    SvHashTableInsert(self->sharedData->currentModeValues.idToOption, (SvGenericObject) optionId, (SvGenericObject) valueId);
}

SvHidden SvString QBWizardContextGetOptionValueId(QBWizardContext self, SvString optionId)
{
    assert(self); assert(optionId);
    return (SvString) SvHashTableFind(self->sharedData->currentModeValues.idToOption, (SvGenericObject) optionId);
}

SvHidden SvString QBWizardContextGetOptionValueIdOrDefault(QBWizardContext self, SvString optionId)
{
    assert(self); assert(optionId);
    SvString valueId = QBWizardContextGetOptionValueId(self, optionId);
    if (!valueId)
        valueId = (SvString) SvHashTableFind(self->sharedData->defaultValues.idToOption, (SvGenericObject) optionId);
    return valueId; // still may be NULL
}

SvHidden void QBWizardContextSetSubOptionValue(QBWizardContext self, SvString optionId, SvValue value)
{
    assert(self); assert(optionId); assert(value);
    SvHashTableInsert(self->sharedData->allModesValues.idToSubOption, (SvGenericObject) optionId, (SvGenericObject) value);
    SvHashTableInsert(self->sharedData->currentModeValues.idToSubOption, (SvGenericObject) optionId, (SvGenericObject) value);
}

SvHidden SvValue QBWizardContextGetSubOptionValue(QBWizardContext self, SvString optionId)
{
    assert(self); assert(optionId);
    return (SvValue) SvHashTableFind(self->sharedData->currentModeValues.idToSubOption, (SvGenericObject) optionId);
}

SvHidden SvValue QBWizardContextGetSubOptionValueOrDefault(QBWizardContext self, SvString optionId)
{
    assert(self); assert(optionId);
    SvValue value = QBWizardContextGetSubOptionValue(self, optionId);
    if (!value)
        value = (SvValue) SvHashTableFind(self->sharedData->defaultValues.idToSubOption, (SvGenericObject) optionId);
    return value; // still may be NULL
}

SvHidden void QBWizardContextRemoveSubOptionValue(QBWizardContext self, SvString optionId)
{
    assert(self); assert(optionId);
    SvHashTableRemove(self->sharedData->allModesValues.idToSubOption, (SvGenericObject) optionId);
    SvHashTableRemove(self->sharedData->currentModeValues.idToSubOption, (SvGenericObject) optionId);
}

SvLocal void QBWizardFillNonExistingValues(SvHashTable values, SvHashTable additionalValues);

SvHidden SvHashTable QBWizardContextCreateIdToOptionComplementedWithDefaults(QBWizardContext self)
{
    assert(self);
    SvHashTable idToOption = (SvHashTable) SvObjectCopy((SvGenericObject) self->sharedData->currentModeValues.idToOption, NULL);
    QBWizardFillNonExistingValues(idToOption, self->sharedData->defaultValues.idToOption);
    return idToOption;
}

SvHidden SvHashTable QBWizardContextCreateIdToSubOptionComplementedWithDefaults(QBWizardContext self)
{
    assert(self);
    SvHashTable idToSubOption = (SvHashTable) SvObjectCopy((SvGenericObject) self->sharedData->currentModeValues.idToSubOption, NULL);
    QBWizardFillNonExistingValues(idToSubOption, self->sharedData->defaultValues.idToSubOption);
    return idToSubOption;
}

SvLocal void QBWizardFillNonExistingValues(SvHashTable values, SvHashTable additionalValues)
{
    assert(values); assert(additionalValues);
    SvIterator it = SvHashTableKeysIterator(additionalValues);
    SvGenericObject key;
    while ((key = SvIteratorGetNext(&it))) {
        if (!SvHashTableFind(values, key)) {
            SvGenericObject value = SvHashTableFind(additionalValues, key);
            SvHashTableInsert(values, key, value);
        }
    }
}
