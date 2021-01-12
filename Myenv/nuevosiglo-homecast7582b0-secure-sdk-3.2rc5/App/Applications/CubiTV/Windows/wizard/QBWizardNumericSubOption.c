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

#include "QBWizardNumericSubOption.h"
#include <limits.h>
#include <SvCore/SvErrnoDomain.h>
#include <SvFoundation/SvType.h>
#include <SvJSON/SvJSONClassRegistry.h>

#define log_error(FMT, ...) do{ SvLogError("QBWizardNumericSubOption " FMT, ##__VA_ARGS__); }while(0)


SvLocal void QBWizardNumericSubOption__dtor__(void *self_);

SvHidden SvType QBWizardNumericSubOption_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBWizardNumericSubOption__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBWizardNumericSubOption",
                            sizeof(struct QBWizardNumericSubOption_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void QBWizardNumericSubOption__dtor__(void *self_)
{
    QBWizardNumericSubOption self = self_;
    SVRELEASE(self->defaultValue);
}

SvLocal SvGenericObject QBWizardNumericSubOptionLoad(SvGenericObject helper, SvString className, SvHashTable desc, SvErrorInfo *errorOut);

SvHidden void QBWizardNumericSubOptionRegisterJSONHelper(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    SvJSONClass wizardNumericSubOptionHelper = SvJSONClassCreate(NULL, QBWizardNumericSubOptionLoad, &error);
    if (unlikely(error))
        goto fini;

    SvJSONRegisterClassHelper(SVSTRING("QBWizardNumericSubOption"), (SvGenericObject) wizardNumericSubOptionHelper, &error);
    if (unlikely(error))
        goto fini;

    SVRELEASE(wizardNumericSubOptionHelper);

fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void QBWizardNumericSubOptionInitFromJSONDesc(QBWizardNumericSubOption self, SvHashTable desc);

SvLocal SvGenericObject QBWizardNumericSubOptionLoad(SvGenericObject helper, SvString className, SvHashTable desc, SvErrorInfo *errorOut)
{
    assert(desc);

    QBWizardNumericSubOption self = (QBWizardNumericSubOption) SvTypeAllocateInstance(QBWizardNumericSubOption_getType(), NULL);

    QBWizardNumericSubOptionInitFromJSONDesc(self, desc);

    return (SvGenericObject) self;
}

SvLocal void QBWizardNumericSubOptionLoadNumericalMaxFromJSONDesc(QBWizardNumericSubOption self, SvHashTable desc);
SvLocal void QBWizardNumericSubOptionLoadDefaultValueFromJSONDesc(QBWizardNumericSubOption self, SvHashTable desc);

SvLocal void QBWizardNumericSubOptionInitFromJSONDesc(QBWizardNumericSubOption self, SvHashTable desc)
{
    assert(self); assert(desc);

    QBWizardNumericSubOptionLoadNumericalMaxFromJSONDesc(self, desc);
    QBWizardNumericSubOptionLoadDefaultValueFromJSONDesc(self, desc);
}

SvLocal void QBWizardNumericSubOptionLoadNumericalMaxFromJSONDesc(QBWizardNumericSubOption self, SvHashTable desc)
{
    assert(self); assert(desc);

    SvValue numericalMaxVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("numericalMax"));

    // If not found or bad use INT_MAX

    if (numericalMaxVal && unlikely(!SvObjectIsInstanceOf((SvObject) numericalMaxVal, SvValue_getType()) || !SvValueIsInteger(numericalMaxVal))) {
        log_error("numericalMax is not an integer");
        numericalMaxVal = NULL;
    }

    self->numericalMax = numericalMaxVal ? SvValueGetInteger(numericalMaxVal) : INT_MAX;
}

SvLocal void QBWizardNumericSubOptionLoadDefaultValueFromJSONDesc(QBWizardNumericSubOption self, SvHashTable desc)
{
    assert(self); assert(!self->defaultValue); assert(desc);

    SvValue defaultVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("default"));

    // If not found or bad use -1

    if (defaultVal && unlikely(!SvObjectIsInstanceOf((SvObject) defaultVal, SvValue_getType()) || !SvValueIsInteger(defaultVal))) {
        log_error("default is not an integer");
        defaultVal = NULL;
    }

    self->defaultValue = defaultVal ? SVRETAIN(defaultVal) : SvValueCreateWithInteger(-1, NULL);
}
