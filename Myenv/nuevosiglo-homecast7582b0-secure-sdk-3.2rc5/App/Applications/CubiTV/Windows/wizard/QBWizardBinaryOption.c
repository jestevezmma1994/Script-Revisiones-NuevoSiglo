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

#include "QBWizardBinaryOption.h"
#include "QBWizardOptionInternal.h"
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvType.h>
#include <SvJSON/SvJSONClassRegistry.h>
#include "QBWizardNumericSubOption.h"

#define log_error(FMT, ...) do{ SvLogError("QBWizardBinaryOption " FMT, ##__VA_ARGS__); }while(0)


SvLocal void QBWizardBinaryOptionDestroy(void *self_);

SvHidden SvType QBWizardBinaryOption_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBWizardBinaryOptionDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBWizardBinaryOption",
                            sizeof(struct QBWizardBinaryOption_s),
                            QBWizardOption_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void QBWizardBinaryOptionDestroy(void *self_)
{
    QBWizardBinaryOption self = self_;
    SVTESTRELEASE(self->subOption);
}

SvLocal SvGenericObject QBWizardBinaryOptionLoad(SvGenericObject helper, SvString className, SvHashTable desc, SvErrorInfo *errorOut);

SvHidden void QBWizardBinaryOptionRegisterJSONHelper(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    SvJSONClass wizardBinaryOptionHelper = SvJSONClassCreate(NULL, QBWizardBinaryOptionLoad, &error);
    if (unlikely(error))
        goto fini;

    SvJSONRegisterClassHelper(SVSTRING("QBWizardBinaryOption"), (SvGenericObject) wizardBinaryOptionHelper, &error);
    if (unlikely(error))
        goto fini;

    SVRELEASE(wizardBinaryOptionHelper);

fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void QBWizardBinaryOptionInitFromJSONDesc(QBWizardBinaryOption self, SvHashTable desc, SvErrorInfo *errorOut);

SvLocal SvGenericObject QBWizardBinaryOptionLoad(SvGenericObject helper, SvString className, SvHashTable desc, SvErrorInfo *errorOut)
{
    assert(desc);
    SvErrorInfo error = NULL;

    QBWizardBinaryOption self = (QBWizardBinaryOption) SvTypeAllocateInstance(QBWizardBinaryOption_getType(), NULL);

    QBWizardBinaryOptionInitFromJSONDesc(self, desc, &error);
    if (unlikely(error)) {
        SVRELEASE(self);
        self = NULL;
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidArgument, error, "failed loading options from JSON");
        goto fini;
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
    return (SvGenericObject) self;
}

SvLocal void QBWizardBinaryOptionLoadSubOptionFromJSONDesc(QBWizardBinaryOption self, SvHashTable desc);

SvLocal void QBWizardBinaryOptionInitFromJSONDesc(QBWizardBinaryOption self, SvHashTable desc, SvErrorInfo *errorOut)
{
    assert(self); assert(desc);
    SvErrorInfo error = NULL;

    QBWizardOptionInitFromJSONDesc((QBWizardOption) self, desc, &error);
    if (unlikely(error))
        goto fini;

    QBWizardBinaryOptionLoadSubOptionFromJSONDesc(self, desc);

fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void QBWizardBinaryOptionLoadSubOptionFromJSONDesc(QBWizardBinaryOption self, SvHashTable desc)
{
    assert(self); assert(!self->subOption); assert(desc);

    // subOption is optional
    SvGenericObject subOptionVal = (SvGenericObject) SvHashTableFind(desc, (SvGenericObject) SVSTRING("subOption"));

    if (subOptionVal && unlikely(!SvObjectIsInstanceOf(subOptionVal, QBWizardNumericSubOption_getType()))) {
        log_error("subOptionVal is bad");
        subOptionVal = NULL;
    }

    self->subOption = SVTESTRETAIN(subOptionVal);
}
