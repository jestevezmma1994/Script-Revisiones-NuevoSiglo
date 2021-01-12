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

#include "QBReattachableActiveTreeNodeWrapper.h"

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvWeakReference.h>

struct QBReattachableActiveTreeNodeWrapper_ {
    struct SvObject_ super_; ///< super class

    SvWeakReference tree;

    QBActiveTreeNode node;
    SvWeakReference parentNode;
    size_t idx;
    bool removed;
};

SvLocal void
QBReattachableActiveTreeNodeWrapperDestroy(void *ptr)
{
    QBReattachableActiveTreeNodeWrapper self = ptr;

    SVRELEASE(self->tree);
    SVRELEASE(self->node);
    SVRELEASE(self->parentNode);
}

SvLocal SvType
QBReattachableActiveTreeNodeWrapper_getType(void)
{
    static SvType type = NULL;
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBReattachableActiveTreeNodeWrapperDestroy
    };

    if (!type) {
        SvTypeCreateManaged("QBReattachableActiveTreeNodeWrapper",
                            sizeof(struct QBReattachableActiveTreeNodeWrapper_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBReattachableActiveTreeNodeWrapper
QBReattachableActiveTreeNodeWrapperCreate(QBActiveTree tree,
                                          SvString nodeId,
                                          SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBReattachableActiveTreeNodeWrapper self = NULL;
    if (!tree) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "NULL tree passed");
        goto out;
    }
    if (!nodeId) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "NULL nodeId passed");
        goto out;
    }
    QBActiveTreeNode node = QBActiveTreeFindNode(tree, nodeId);
    if (!node) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "node with id %s not found", SvStringCString(nodeId));
        goto out;
    }

    self = (QBReattachableActiveTreeNodeWrapper) SvTypeAllocateInstance(QBReattachableActiveTreeNodeWrapper_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBReattachableActiveTreeNodeWrapper");
        goto out;
    }

    self->tree = SvWeakReferenceCreate((SvObject) tree, NULL);
    self->node = SVRETAIN(node);
    self->idx = QBActiveTreeNodeGetChildNodeIndex(QBActiveTreeNodeGetParentNode(node), node);
    self->parentNode = SvWeakReferenceCreate((SvObject) QBActiveTreeNodeGetParentNode(node), NULL);

out:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

void
QBReattachableActiveTreeNodeWrapperRemove(QBReattachableActiveTreeNodeWrapper self)
{
    if (!self->removed) {
        QBActiveTree tree = (QBActiveTree) SvWeakReferenceTakeReferredObject(self->tree);
        QBActiveTreeRemoveSubTree(tree, self->node, NULL);
        SVRELEASE(tree);
        self->removed = true;
    }
}

void
QBReattachableActiveTreeNodeWrapperInsert(QBReattachableActiveTreeNodeWrapper self)
{
    if (self->removed) {
        QBActiveTree tree = (QBActiveTree) SvWeakReferenceTakeReferredObject(self->tree);
        QBActiveTreeNode parentNode = (QBActiveTreeNode) SvWeakReferenceTakeReferredObject(self->parentNode);
        QBActiveTreeInsertSubTree(tree, parentNode, self->node, self->idx, NULL);
        SVRELEASE(parentNode);
        SVRELEASE(tree);
        self->removed = false;
    }
}
