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

#include "QBLogicFactory.h"
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvArray.h>


/**
 * QBLogicFactory class internals.
 **/
struct QBLogicFactory_ {
    /// super class
    struct SvObject_ super_;
    /// registered implementations (SvType instances)
    SvArray registeredClasses;
};


/**
 * Logic factory singleton instance.
 * @internal
 **/
static volatile QBLogicFactory instance = NULL;


SvLocal void
QBLogicFactoryDestroy(void *self_)
{
    QBLogicFactory self = self_;
    SVRELEASE(self->registeredClasses);
}

SvLocal SvType
QBLogicFactory_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBLogicFactoryDestroy
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBLogicFactory",
                            sizeof(struct QBLogicFactory_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBLogicFactory
QBLogicFactoryGetInstance(void)
{
    if (unlikely(!instance)) {
        QBLogicFactory factory = (QBLogicFactory) SvTypeAllocateInstance(QBLogicFactory_getType(), NULL);
        assert(factory != NULL);
        factory->registeredClasses = SvArrayCreate(NULL);
        assert(factory->registeredClasses != NULL);

        // setup singleton pointer atomically
        long int v = (intptr_t) factory;
        volatile long int *p = (long int *)((void *) &instance);
        if (SvCAS(p, 0, v) != 0) {
            // other thread was first
            SVRELEASE(factory);
        }
    }

    return instance;
}

SvLocal SvDestructor void
QBLogicFactoryDeinit(void)
{
    QBLogicFactory tmp = instance;
    instance = NULL;
    SVTESTRELEASE(tmp);
}

void
QBLogicFactoryRegisterClass(QBLogicFactory self,
                            SvType logicClass,
                            SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (unlikely(!self)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
        goto fini;
    }

    if (unlikely(!logicClass)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL class passed");
        goto fini;
    }

    if (unlikely(SvTypeIsAbstract(logicClass))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "class %s is abstract", SvTypeGetName(logicClass));
        goto fini;
    }

    size_t count = SvArrayGetCount(self->registeredClasses);
    for (size_t i = 0; i < count; i++) {
        SvType registeredClass = (SvType) SvArrayGetObjectAtIndex(self->registeredClasses, i);
        if (SvTypeIsSuperTypeOf(logicClass, registeredClass)) {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                      "a subclass %s of %s is already registered",
                                      SvTypeGetName(registeredClass), SvTypeGetName(logicClass));
            goto fini;
        }
    }

    bool registered = false;
    for (size_t i = 0; i < count;) {
        SvType registeredClass = (SvType) SvArrayGetObjectAtIndex(self->registeredClasses, i);
        if (SvTypeIsSuperTypeOf(registeredClass, logicClass)) {
            if (!registered) {
                // first parent class found, replace it (we are more specialized)
                SvArrayReplaceObjectAtIndexWithObject(self->registeredClasses, i, (SvObject) logicClass);
                registered = true;
                i++;
            } else {
                // consecutive parent class found, remove it
                SvArrayRemoveObjectAtIndex(self->registeredClasses, i);
                --count;
            }
        } else {
            i++;
        }
    }

    if (!registered) {
        SvArrayAddObject(self->registeredClasses, (SvObject) logicClass);
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvType
QBLogicFactoryFindImplementationOf(QBLogicFactory self,
                                   SvType parentClass,
                                   SvErrorInfo *errorOut)
{
    SvType implementingClass = NULL;
    SvErrorInfo error = NULL;

    if (unlikely(!self)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
        goto fini;
    }

    if (unlikely(!parentClass)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL parent class passed");
        goto fini;
    }

    SvIterator i = SvArrayGetIterator(self->registeredClasses);
    SvType t;
    while ((t = (SvType) SvIteratorGetNext(&i))) {
        if (SvTypeIsSuperTypeOf(parentClass, t)) {
            if (implementingClass) {
                SvLogCritical("%s(): found multiple implementations of %s: %s and %s",
                              __func__, SvTypeGetName(parentClass),
                              SvTypeGetName(implementingClass), SvTypeGetName(t));
                abort();
            }
            implementingClass = t;
        }
    }

    if (!implementingClass)
        implementingClass = parentClass;

fini:
    SvErrorInfoPropagate(error, errorOut);
    return implementingClass;
}
