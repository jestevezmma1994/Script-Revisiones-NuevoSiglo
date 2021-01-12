/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBDiskPVRDirectorySerializer.h"

#include "QBDiskPVRDirectory.h"
#include <QBPVRDirectory.h>
#include <SvJSON/SvJSONClassHelper.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>

struct QBDiskPVRDirectorySerializer_ {
    struct SvObject_ super_;            ///< super type

    SvWeakReference parent;

    bool addSearchToDirectories;
    QBPVRSearchProviderTest searchProviderTest;
};

SvLocal
void QBDiskPVRDirectorySerializerDestroy(void *self_)
{
    QBDiskPVRDirectorySerializer self = (QBDiskPVRDirectorySerializer) self_;

    SVTESTRELEASE(self->parent);
}

SvLocal void
QBDiskPVRDirectorySerializerSerialize(SvObject helper_,
                                      SvObject obj,
                                      SvHashTable desc,
                                      SvErrorInfo *errorOut)
{
    QBDiskPVRDirectorySerialize((QBDiskPVRDirectory) obj, desc, errorOut);
}

SvLocal SvObject
QBDiskPVRDirectorySerializerCreateObject(SvObject helper_,
                                         SvString className,
                                         SvHashTable desc,
                                         SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBDiskPVRDirectorySerializer self = (QBDiskPVRDirectorySerializer) helper_;

    QBContentCategory parent = (QBContentCategory) SvWeakReferenceTakeReferredObject(self->parent);
    if (!parent) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "SvWeakReferenceTakeReferredObject() failed");
        goto err;
    }
    QBPVRDirectory dir = (QBPVRDirectory) QBDiskPVRDirectoryCreateWithDesc(parent,
                                                                           desc,
                                                                           self->addSearchToDirectories,
                                                                           self->searchProviderTest,
                                                                           &error);
    SVRELEASE(parent);
    if (!dir) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBDiskPVRDirectoryCreateWithDesc() failed");
        goto err;
    }

    return (SvObject) dir;
err:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

SvLocal SvType
QBDiskPVRDirectorySerializer_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDiskPVRDirectorySerializerDestroy,
    };

    static const struct SvJSONClassHelper_ helperMethods = {
        .serialize    = QBDiskPVRDirectorySerializerSerialize,
        .createObject = QBDiskPVRDirectorySerializerCreateObject,
    };

    if (!type) {
        SvTypeCreateManaged("QBDiskPVRDirectorySerializer",
                            sizeof(struct QBDiskPVRDirectorySerializer_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            SvJSONClassHelper_getInterface(), &helperMethods,
                            NULL);
    }

    return type;
}

QBDiskPVRDirectorySerializer
QBDiskPVRDirectorySerializerCreate(QBContentCategory parent,
                                   bool addSearchToDirectories,
                                   QBPVRSearchProviderTest searchProviderTest,
                                   SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!parent) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL parent passed");
        goto err1;
    }

    QBDiskPVRDirectorySerializer self = (QBDiskPVRDirectorySerializer)
            SvTypeAllocateInstance(QBDiskPVRDirectorySerializer_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error,
                                           "can't allocate QBDiskPVRDirectorySerializer");
        goto err1;
    }

    self->parent = SvWeakReferenceCreate((SvObject) parent, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvWeakReferenceCreate() failed");
        goto err2;
    }

    self->addSearchToDirectories = addSearchToDirectories;
    self->searchProviderTest = searchProviderTest;

    return self;
err2:
    SVRELEASE(self);
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}
