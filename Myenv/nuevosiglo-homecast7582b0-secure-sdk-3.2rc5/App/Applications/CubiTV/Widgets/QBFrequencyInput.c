/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBFrequencyInput.h"

#include <CUIT/Core/app.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/widget.h>
#include <QBConf.h> // IWYU pragma: keep
#include <QBInput/QBInputCodes.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBTunerTypes.h>
#include <Services/core/QBStandardRastersManager.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvDataBucket2/SvDBObject.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <QBWidgets/QBComboBox.h>
#include <assert.h>
#include <libintl.h> // IWYU pragma: keep
#include <main.h>
#include <main_decl.h>
#include <settings.h>
#include <stdio.h>
#include <stdlib.h>
#include <sv_tuner.h>

typedef struct QBFrequencyInput_* QBFrequencyInput;

struct QBFrequencyInput_ {
    struct QBTunerProperties tunerProps;
    QBChannelScanningConf conf;

    SvWidget frequencyInput;
    SvWidget w;
    bool valueIsFinished;

    int value;
    int frequencyJumpValue;
    int frequencyMaxStandardRasterValue;
    int frequencyMinStandardRasterValue;

    bool useHalfFrequencies;
};


SvLocal SvArray QBFrequencyInputFrequencyArray(int freqMin, int freqMax, int bandwidth)
{
    SvArray values = SvArrayCreate(NULL);
    int freq;
    for (freq = freqMin; freq <= freqMax; freq += bandwidth * 1000) {
        SvValue value = SvValueCreateWithInteger(freq, NULL);
        SvArrayAddObject(values, (SvObject) value);
        SVRELEASE(value);
    }
    return values;
}

SvLocal SvString QBFrequencyInputComboBoxPrepareFreq(void *self_, SvWidget combobox, SvObject value)
{
    QBFrequencyInput self = self_;
    int freq = SvValueGetInteger((SvValue) value);
    SvString freqStr;

    if (self->useHalfFrequencies) {
        freqStr = SvStringCreateWithFormat(gettext("%i.%iMHz"), freq / 1000, (freq % 1000) / 100);
    } else {
        freqStr = SvStringCreateWithFormat(gettext("%dMHz"), freq / 1000);
    }

    return freqStr;
}

SvLocal int QBFrequencyInputComboBoxChangeFrequency(void *target, SvWidget combobox, SvObject value, int key)
{
    QBFrequencyInput self = target;

    const int freqMin = self->frequencyMinStandardRasterValue;
    const int freqMax = self->frequencyMaxStandardRasterValue;

    int freq = SvValueGetInteger((SvValue) value);
    if (key == QBKEY_LEFT || key == 0) {
        if (freq <= freqMin) {
            return 0;
        }
        if ((freq - freqMin) % (self->frequencyJumpValue * 1000) == 0)
            freq -= self->frequencyJumpValue * 1000;
        else
            freq -= (freq - freqMin) % (self->frequencyJumpValue * 1000);
    } else {
        if (freq >= freqMax)
            return -1;
        else
            if (freq < freqMin)
                return 0;
            freq += self->frequencyJumpValue * 1000 - (freq - freqMin) % (self->frequencyJumpValue * 1000);
    }

    return (freq - freqMin) / (self->frequencyJumpValue * 1000);
}

SvLocal bool QBFrequencyInputComboBoxInputFreq(void *target, SvWidget combobox, SvInputEvent e)
{
    const int key = e->ch;
    QBFrequencyInput self = target;

    if (key < '0' || key > '9')
        return false;

    const int unit = self->useHalfFrequencies ? 100 : 1000;


    int digit = (key - '0') * unit;

    int freqValue = self->valueIsFinished ? 0 : self->value;

    freqValue = freqValue * 10 + digit;
    if (freqValue == 0 && digit == 0)
        return true;

    //If user types anything except of 5, just assume they wanted to have .0
    if (unit == 100 && digit != 500 && freqValue >= 100 * 1000) {
        freqValue -= digit;
    }

    if (freqValue > self->frequencyMaxStandardRasterValue) {
        freqValue = digit;
    }

    SvValue value = SvValueCreateWithInteger(freqValue, NULL);
    QBComboBoxSetValue(combobox, (SvObject) value);
    SVRELEASE(value);

    self->valueIsFinished = false;

    return true;
}

SvLocal SvValue QBFrequencyInputComboBoxGetProperFreq(QBFrequencyInput self, SvWidget combobox, SvValue userValue)
{
    if (!userValue)
        return NULL;

    const int freqMin = self->frequencyMinStandardRasterValue;
    const int freqMax = self->frequencyMaxStandardRasterValue;

    int value = SvValueGetInteger(userValue);

    if (value < freqMin) {
        return SvValueCreateWithInteger(freqMin, NULL);
    }
    if (value > freqMax) {
        return SvValueCreateWithInteger(freqMax, NULL);
    }

    return SVRETAIN(userValue);
}

SvLocal void QBFrequencyInputComboBoxCurrentFreq(void *target, SvWidget combobox, SvGenericObject value)
{
    QBFrequencyInput self = target;
    int val = SvValueGetInteger((SvValue) value);
    self->value = val;
}

SvLocal void QBFrequencyInputComboBoxOnFinishFreq(void *target, SvWidget combobox, SvObject selectedValue, SvObject userValue)
{
    QBFrequencyInput self = target;
    SvObject properUserValue = (SvObject) QBFrequencyInputComboBoxGetProperFreq(self, combobox, (SvValue) userValue);

    if (properUserValue && properUserValue != userValue && QBComboBoxIsUserValue(combobox)) {
        QBComboBoxSetValue(combobox, properUserValue);
    }

    SVTESTRELEASE(properUserValue);

    self->valueIsFinished = true;
}

SvLocal void QBFrequencyInputSetRasters(QBFrequencyInput self, SvObject rasters)
{
    QBStandardRastersParams standardRastersParams = NULL;
    QBTunerType tunerType = QBTunerType_unknown;
    sv_tuner_get_type(0, &tunerType);
    // recognize whether rasters were passed into function directly, or in an array
    if (rasters && SvObjectIsInstanceOf(rasters, SvArray_getType()) && SvArrayCount((SvArray) rasters)) {
        rasters = SvArrayObjectAtIndex((SvArray) rasters, 0);
    }
    if (rasters && SvObjectIsInstanceOf(rasters, QBStandardRastersParams_getType())) {
        standardRastersParams = (QBStandardRastersParams) rasters;
    }
    if (standardRastersParams) {
        self->frequencyMinStandardRasterValue = (self->tunerProps.allowed.freq_min < standardRastersParams->freq_min)
                                              ? standardRastersParams->freq_min : self->tunerProps.allowed.freq_min;
        self->frequencyMaxStandardRasterValue = (self->tunerProps.allowed.freq_max > standardRastersParams->freq_max)
                                              ? standardRastersParams->freq_max : self->tunerProps.allowed.freq_max;
        self->useHalfFrequencies = standardRastersParams->useHalfFrequencies;
    } else {
        SvLogNotice("self->tunerProps.allowed.freq_min %d, self->tunerProps.allowed.freq_max %d",
                    self->tunerProps.allowed.freq_min, self->tunerProps.allowed.freq_max);
        self->frequencyMinStandardRasterValue = self->tunerProps.allowed.freq_min;
        self->frequencyMaxStandardRasterValue = self->tunerProps.allowed.freq_max;
        if (tunerType == QBTunerType_ter) {
            self->useHalfFrequencies = true;
        } else {
            self->useHalfFrequencies = false;
        }
    }
}

SvLocal void QBFrequencyInputLoadRasters(QBFrequencyInput self)
{
    QBTunerType tunerType = QBTunerType_unknown;
    sv_tuner_get_type(0, &tunerType);
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBStandardRastersManager standardRastersManager = (QBStandardRastersManager)
        QBGlobalStorageGetItem(globalStorage, QBStandardRastersManager_getType(), NULL);
    SvArray standardRastersParamsArray = NULL;
    if (standardRastersManager && self->conf && self->conf->useStandardRasters && self->conf->forceUseStandardRasters) {
        QBTunerStandard standard = QBTunerStandard_unknown;
        if (self->tunerProps.allowed.standard_cnt) {
            const char* standardString = QBConfigGet("SCANSTANDARD");
            if (standardString) {
                standard = QBTunerStandardFromString(standardString);
            }
        }
        QBStandardRastersConfig rastersConfig = QBStandardRastersManagerGetConfig(standardRastersManager);
        standardRastersParamsArray = QBStandardRastersConfigCreateParams(rastersConfig,
                                                                       tunerType,
                                                                       standard,
                                                                       self->frequencyJumpValue);
    }
    QBFrequencyInputSetRasters(self, (SvObject) standardRastersParamsArray);
    SVTESTRELEASE(standardRastersParamsArray);
}

SvLocal void QBFrequencyInputAdjustValue(QBFrequencyInput self)
{
    SvArray values = QBFrequencyInputFrequencyArray(self->frequencyMinStandardRasterValue,
                                                    self->frequencyMaxStandardRasterValue,
                                                    self->frequencyJumpValue);

    bool previousIsUserValue = QBComboBoxIsUserValue(self->frequencyInput);
    SvValue previousValue = (SvValue) SVTESTRETAIN(QBComboBoxGetValue(self->frequencyInput));
    int previousPosition = QBComboBoxGetPosition(self->frequencyInput);

    int previousFreq = -1;
    if (previousValue)
        previousFreq = SvValueGetInteger(previousValue);

    QBComboBoxSetContent(self->frequencyInput, values);

    if (!previousIsUserValue) {
        if (previousPosition == 0)
            QBComboBoxSetPosition(self->frequencyInput, 0);
        else
            QBComboBoxSetPosition(self->frequencyInput, (previousFreq - self->frequencyMinStandardRasterValue + self->frequencyJumpValue * 500) / (self->frequencyJumpValue * 1000));
    } else if (previousValue) {
        QBComboBoxSetValue(self->frequencyInput, (SvObject) previousValue);
    }

    SVTESTRELEASE(previousValue);

    SVRELEASE(values);
}

void QBFrequencyInputUpdateRange(SvWidget w, int bandwidth)
{
    QBFrequencyInput self = (QBFrequencyInput) w->prv;
    assert(bandwidth > 0);
    self->frequencyJumpValue = bandwidth;
    QBTunerType tunerType = QBTunerType_unknown;
    sv_tuner_get_type(0, &tunerType);

    // frequency jump doesn't make sense on satellite
    if (tunerType == QBTunerType_sat)
        return;

    QBFrequencyInputLoadRasters(self);
    QBFrequencyInputAdjustValue(self);
}

SvLocal void QBFrequencyInputSetControlledValue(QBFrequencyInput self, int value)
{
    SvArray values = NULL;
    values = QBFrequencyInputFrequencyArray(self->frequencyMinStandardRasterValue,
                                            self->frequencyMaxStandardRasterValue,
                                            self->frequencyJumpValue);


    QBComboBoxSetContent(self->frequencyInput, values);
    SVRELEASE(values);
    if ((value < self->frequencyMinStandardRasterValue) || (value > self->frequencyMaxStandardRasterValue)) {
        SvValue freqMinValue = SvValueCreateWithInteger(value, NULL);
        QBComboBoxSetValue(self->frequencyInput, (SvObject) freqMinValue);
        SVRELEASE(freqMinValue);
    } else {
        // check if we're in raster
        if (((value - self->frequencyMinStandardRasterValue) % (self->frequencyJumpValue * 1000)) == 0) {
            QBComboBoxSetPosition(self->frequencyInput, (value - self->frequencyMinStandardRasterValue) / (self->frequencyJumpValue * 1000));
        } else {
            SvValue freqMinValue = SvValueCreateWithInteger(value, NULL);
            QBComboBoxSetValue(self->frequencyInput, (SvObject) freqMinValue);
            SVRELEASE(freqMinValue);
        }
    }
}

void QBFrequencyInputSetParameters(SvWidget w, QBStandardRastersParams standardRastersParams, QBChannelScanningConf conf, int value, int bandwidth)
{
    QBFrequencyInput self = (QBFrequencyInput) w->prv;
    QBTunerType tunerType = QBTunerType_unknown;
    sv_tuner_get_type(0, &tunerType);

    self->conf = conf;
    self->frequencyJumpValue = bandwidth;

    QBFrequencyInputSetRasters(self, (SvObject) standardRastersParams);
    QBFrequencyInputSetControlledValue(self, value);
}

SvLocal SvWidget QBFrequencyInputNewFromSettingsType(SvApplication app, const char *widgetName, bool useRatio, unsigned int width, unsigned int height)
{
    SvWidget w = useRatio ? svSettingsWidgetCreateWithRatio(app, widgetName, width, height) : svSettingsWidgetCreate(app, widgetName);
    if (!w) {
        SvLogError("%s() %d", __func__, __LINE__);
        return NULL;
    }
    QBFrequencyInput self = calloc(1, sizeof(struct QBFrequencyInput_));
    self->w = w;
    static const int bufSize = 128;
    char buf[bufSize];

    snprintf(buf, bufSize, "%s.ComboBox", widgetName);

    SvWidget frequencyBox = useRatio ? QBComboBoxNewFromRatio(app, buf, width, height) : QBComboBoxNewFromSM(app, buf);
    if (!frequencyBox) {
        SvLogError("%s() %d", __func__, __LINE__);
        svWidgetDestroy(w);
        free(self);
        return NULL;
    }

    QBComboBoxCallbacks cb = {
        QBFrequencyInputComboBoxPrepareFreq,
        QBFrequencyInputComboBoxInputFreq,
        QBFrequencyInputComboBoxChangeFrequency,
        QBFrequencyInputComboBoxCurrentFreq,
        NULL,
        QBFrequencyInputComboBoxOnFinishFreq
    };
    QBComboBoxSetCallbacks(frequencyBox, self, cb);

    svWidgetSetFocusable(frequencyBox, true);
    svWidgetAttach(w, frequencyBox, 0, 0, 1);

    self->frequencyInput = frequencyBox;
    self->valueIsFinished = true;

    sv_tuner_get_properties(0, &self->tunerProps);  //all tuners are the same

    w->prv = self;

    return w;
}

SvWidget QBFrequencyInputNewFromSM(SvApplication app, const char *widgetName)
{
    return QBFrequencyInputNewFromSettingsType(app, widgetName, false, 0, 0);
}

SvWidget QBFrequencyInputNewFromRatio(SvApplication app, const char *widgetName, unsigned int parentWidth, unsigned int parentHeight)
{
    return QBFrequencyInputNewFromSettingsType(app, widgetName, true, parentWidth, parentHeight);
}

int QBFrequencyInputGetValue(SvWidget w)
{
    assert(w != NULL);
    QBFrequencyInput self = (QBFrequencyInput) w->prv;

    return self->useHalfFrequencies ? self->value : 1000 * (self->value / 1000);
}

SvWidget QBFrequencyInputGetWidget(SvWidget w)
{
    assert(w != NULL);
    QBFrequencyInput self = (QBFrequencyInput) w->prv;

    return self->frequencyInput;
}

void QBFrequencyInputSetValue(SvWidget w, int value)
{
    QBFrequencyInput self = (QBFrequencyInput) w->prv;
    self->value = value;
    SvValue frequencyValue = SvValueCreateWithInteger(value, NULL);
    QBComboBoxSetValue(self->frequencyInput, (SvObject) frequencyValue);
    SVRELEASE(frequencyValue);
}
