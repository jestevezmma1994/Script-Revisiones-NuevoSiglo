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

#include "QBAppExtensions.h"

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>

struct QBAppExtensions_ {
    struct SvObject_ super_; ///< super class

    SvHashTable extensions;
};

SvLocal void
QBAppExtensionsDestroy(void *ptr)
{
    QBAppExtensions self = ptr;

    SVTESTRELEASE(self->extensions);
}

SvLocal SvType
QBAppExtensions_getType(void)
{
    static SvType type = NULL;
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAppExtensionsDestroy
    };

    if (!type) {
        SvTypeCreateManaged("QBAppExtensions",
                            sizeof(struct QBAppExtensions_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBAppExtensions
QBAppExtensionsCreate(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBAppExtensions self = (QBAppExtensions) SvTypeAllocateInstance(QBAppExtensions_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBAppExtensions");
        goto err1;
    }

    self->extensions = SvHashTableCreate(43, &error);
    if (!self->extensions) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvHashTableCreate() failed");
        goto err2;
    }

    return self;
err2:
    SVRELEASE(self);
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

void
QBAppExtensionsAdd(QBAppExtensions self, SvValue id, SvObject extension)
{
    if (SvHashTableFind(self->extensions, (SvObject) id)) {
        SvLogError("%s(): extension with given id already added", __func__);
        return;
    }
    SvHashTableInsert(self->extensions, (SvObject) id, extension);
}

void
QBAppExtensionsRemove(QBAppExtensions self, SvValue id)
{
    SvHashTableRemove(self->extensions, (SvObject) id);
}

SvObject
QBAppExtensionsGet(QBAppExtensions self, SvValue id)
{
    return SvHashTableFind(self->extensions, (SvObject) id);
}
