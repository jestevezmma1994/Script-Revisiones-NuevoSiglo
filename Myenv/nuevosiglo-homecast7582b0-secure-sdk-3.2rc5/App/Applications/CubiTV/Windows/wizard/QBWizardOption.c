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

#include "QBWizardOption.h"
#include "QBWizardOptionInternal.h"
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <SvJSON/SvJSONClassRegistry.h>

#define log_error(FMT, ...) do{ SvLogError("QBWizardOption " FMT, ##__VA_ARGS__); }while(0)

SvLocal void* QBWizardOptionCopy(void *self_, SvErrorInfo *errorOut);
SvLocal void QBWizardOption__dtor__(void *self_);

SvHidden SvType QBWizardOption_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .copy    = QBWizardOptionCopy,
        .destroy = QBWizardOption__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBWizardOption",
                            sizeof(struct QBWizardOption_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void* QBWizardOptionCopy(void *self_, SvErrorInfo *errorOut)
{
    QBWizardOption self = (QBWizardOption) self_;
    SvErrorInfo error = NULL;

    QBWizardOption copy = (QBWizardOption) SvTypeAllocateInstance(QBWizardOption_getType(), &error);
    if (!copy)
        goto out;

    copy->id = SVTESTRETAIN(self->id);
    copy->caption = SVTESTRETAIN(self->caption);
    copy->description = SVTESTRETAIN(self->description);
    copy->idToCaption = SVTESTRETAIN(self->idToCaption);
    copy->defaultValueId = SVTESTRETAIN(self->defaultValueId);
    copy->screenNumber = self->screenNumber;

out:
    SvErrorInfoPropagate(error, errorOut);

    return copy;
}

SvLocal void QBWizardOption__dtor__(void *self_)
{
    QBWizardOption self = self_;
    SVTESTRELEASE(self->id);
    SVTESTRELEASE(self->caption);
    SVTESTRELEASE(self->description);
    SVTESTRELEASE(self->idToCaption);
    SVTESTRELEASE(self->defaultValueId);
}

SvLocal SvGenericObject QBWizardOptionLoad(SvGenericObject helper, SvString className, SvHashTable desc, SvErrorInfo *errorOut);

SvHidden void QBWizardOptionRegisterJSONHelper(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    SvJSONClass wizardOptionHelper = SvJSONClassCreate(NULL, QBWizardOptionLoad, &error);
    if (unlikely(error))
        goto fini;

    SvJSONRegisterClassHelper(SVSTRING("QBWizardOption"), (SvGenericObject) wizardOptionHelper, &error);
    if (unlikely(error))
        goto fini;

    SVRELEASE(wizardOptionHelper);

fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal SvGenericObject QBWizardOptionLoad(SvGenericObject helper, SvString className, SvHashTable desc, SvErrorInfo *errorOut)
{
    assert(desc);
    SvErrorInfo error = NULL;

    QBWizardOption self = (QBWizardOption) SvTypeAllocateInstance(QBWizardOption_getType(), NULL);

    QBWizardOptionInitFromJSONDesc(self, desc, &error);
    if (unlikely(error)) {
        SVRELEASE(self);
        self = NULL;
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidArgument, error, "loading data from desc failed");
        goto fini;
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
    return (SvGenericObject) self;
}

SvLocal void QBWizardOptionLoadIdFromJSONDesc(QBWizardOption self, SvHashTable desc, SvErrorInfo *errorOut);
SvLocal void QBWizardOptionLoadCaptionFromJSONDesc(QBWizardOption self, SvHashTable desc, SvErrorInfo *errorOut);
SvLocal void QBWizardOptionLoadIdToCaptionFromJSONDesc(QBWizardOption self, SvHashTable desc, SvErrorInfo *errorOut);
SvLocal void QBWizardOptionLoadDefaultValueIdFromJSONDesc(QBWizardOption self, SvHashTable desc);
SvLocal void QBWizardOptionLoadScreenNumberFromJSONDesc(QBWizardOption self, SvHashTable desc);
SvLocal void QBWizardOptionLoadDescriptionFromJSONDesc(QBWizardOption self, SvHashTable desc);

SvHidden void QBWizardOptionInitFromJSONDesc(QBWizardOption self, SvHashTable desc, SvErrorInfo *errorOut)
{
    assert(self); assert(desc);
    SvErrorInfo error = NULL;

    QBWizardOptionLoadIdFromJSONDesc(self, desc, &error);
    if (unlikely(error))
        goto fini;

    QBWizardOptionLoadCaptionFromJSONDesc(self, desc, &error);
    if (unlikely(error))
        goto fini;

    QBWizardOptionLoadIdToCaptionFromJSONDesc(self, desc, &error);
    if (unlikely(error))
        goto fini;

    QBWizardOptionLoadDefaultValueIdFromJSONDesc(self, desc);
    QBWizardOptionLoadScreenNumberFromJSONDesc(self, desc);
    QBWizardOptionLoadDescriptionFromJSONDesc(self, desc);

fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void QBWizardOptionLoadIdFromJSONDesc(QBWizardOption self, SvHashTable desc, SvErrorInfo *errorOut)
{
    assert(self); assert(!self->id); assert(desc);
    SvErrorInfo error = NULL;

    SvValue idVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("id"));

    if (unlikely(!idVal || !SvObjectIsInstanceOf((SvObject) idVal, SvValue_getType()) || !SvValueIsString(idVal))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_notFound, "id string not found");
        goto fini;
    }

    self->id = SVRETAIN(SvValueGetString(idVal));

fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void QBWizardOptionLoadCaptionFromJSONDesc(QBWizardOption self, SvHashTable desc, SvErrorInfo *errorOut)
{
    assert(self); assert(!self->caption); assert(desc);
    SvErrorInfo error = NULL;

    SvValue captionVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("caption"));

    if (unlikely(!captionVal || !SvObjectIsInstanceOf((SvObject) captionVal, SvValue_getType()) || !SvValueIsString(captionVal))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_notFound, "caption string not found");
        goto fini;
    }

    self->caption = SVRETAIN(SvValueGetString(captionVal));

fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void QBWizardOptionLoadDescriptionFromJSONDesc(QBWizardOption self, SvHashTable desc)
{
    assert(self); assert(!self->description); assert(desc);

    SvValue descriptionVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("description"));
    if (!descriptionVal)
        return;

    if (unlikely(!SvObjectIsInstanceOf((SvObject) descriptionVal, SvValue_getType()) || !SvValueIsString(descriptionVal))) {
        log_error("description is not a string");
        return;
    }

    self->description = SVRETAIN(SvValueGetString(descriptionVal));
}

/**
 * Create new @ref SvHashTable with the same keys as in given @ref SvHashTable and values converted to SvStrings.
 * Values which aren't SvString are reported as errors and not inserted to returned HashTable.
 *
 * @param[in]   idToCaptionVals     map SvString to SvValue
 * @return                          map SvString to SvString
 */
SvLocal SvHashTable QBWizardOptionCreateIdToCaptionFromJSONVals(SvHashTable idToCaptionVals);

SvLocal void QBWizardOptionLoadIdToCaptionFromJSONDesc(QBWizardOption self, SvHashTable desc, SvErrorInfo *errorOut)
{
    assert(self); assert(!self->idToCaption); assert(desc);
    SvErrorInfo error = NULL;

    SvHashTable idToCaptionVals = (SvHashTable) SvHashTableFind(desc, (SvObject) SVSTRING("ids"));

    if (unlikely(!idToCaptionVals || !SvObjectIsInstanceOf((SvObject) idToCaptionVals, SvHashTable_getType()) || !SvHashTableCount(idToCaptionVals))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_notFound, "ids object not found");
        goto fini;
    }

    self->idToCaption = QBWizardOptionCreateIdToCaptionFromJSONVals(idToCaptionVals);

fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal SvHashTable QBWizardOptionCreateIdToCaptionFromJSONVals(SvHashTable idToCaptionVals)
{
    assert(idToCaptionVals);

    SvHashTable idToCaption = SvHashTableCreate(20, NULL);

    SvIterator it = SvHashTableKeysIterator(idToCaptionVals);
    SvString itId;
    while ((itId = (SvString) SvIteratorGetNext(&it))) {
        if (unlikely(!SvObjectIsInstanceOf((SvObject) itId, SvString_getType()))) {
            log_error("JSON key isn't SvString");
            continue;
        }
        SvValue itCaption = (SvValue) SvHashTableFind(idToCaptionVals, (SvObject) itId);
        if (unlikely(!SvObjectIsInstanceOf((SvObject) itCaption, SvValue_getType()) || !SvValueIsString(itCaption))) {
            log_error("ids content is bad");
            continue;
        }
        SvHashTableInsert(idToCaption, (SvObject) itId, (SvObject) SvValueGetString(itCaption));
    }

    return idToCaption;
}

SvLocal void QBWizardOptionLoadDefaultValueIdFromJSONDesc(QBWizardOption self, SvHashTable desc)
{
    assert(self); assert(!self->defaultValueId); assert(desc);

    SvValue defaultValueIdVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("default"));

    if (!defaultValueIdVal)
        return;

    if (unlikely(!SvObjectIsInstanceOf((SvObject) defaultValueIdVal, SvValue_getType()) || !SvValueIsString(defaultValueIdVal))) {
        log_error("default is not a string");
        return;
    }

    SvString defaultValueId = SvValueGetString(defaultValueIdVal);

    if (unlikely(!SvHashTableFind(self->idToCaption, (SvObject) defaultValueId))) {
        log_error("default value not found in ids");
        return;
    }

    self->defaultValueId = SVRETAIN(defaultValueId);
}

SvLocal void QBWizardOptionLoadScreenNumberFromJSONDesc(QBWizardOption self, SvHashTable desc)
{
    assert(self); assert(desc);

    SvValue screenNumberVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("screenNumber"));

    // If not found or bad defaults to 0
    if (screenNumberVal) {
        if (unlikely(!SvObjectIsInstanceOf((SvObject) screenNumberVal, SvValue_getType()) || !SvValueIsInteger(screenNumberVal))) {
            log_error("screenNumber is not integer");
            screenNumberVal = NULL;
        } else if (unlikely(SvValueGetInteger(screenNumberVal) < 0)) {
            log_error("screenNumber is negative");
            screenNumberVal = NULL;
        }
    }

    self->screenNumber = screenNumberVal ? SvValueGetInteger(screenNumberVal) : 0;
}
