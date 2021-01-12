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

#include "QBVoDSubtrees.h"
#include <QBContentManager/QBContentProvider.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvInterface.h>

/**
 * The class representing a data tree and all
 * providers associated with that data.
 * This class is used for caching data, so that it
 * can be reused.
 **/
struct QBVoDSubtree_ {
    struct SvObject_ super; ///< superclass
    SvObject tree;          ///< cached tree of data
    SvArray providers;      ///< providers associated with the tree
    SvHashTable data;       ///< additional data associated with the tree.
};

SvLocal void
QBVoDSubtree__dtor__(void* self_)
{
    QBVoDSubtree self = (QBVoDSubtree) self_;
    SVRELEASE(self->tree);
    SVRELEASE(self->providers);
    SVTESTRELEASE(self->data);
}

SvLocal SvType
QBVoDSubtree_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDSubtree__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDSubtree",
                            sizeof(struct QBVoDSubtree_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBVoDSubtree
QBVoDSubtreeCreate(SvObject tree)
{
    QBVoDSubtree self = (QBVoDSubtree) SvTypeAllocateInstance(QBVoDSubtree_getType(), NULL);
    self->tree = SVRETAIN(tree);
    self->providers = SvArrayCreate(NULL);
    return self;
}

void
QBVoDSubtreeAddProvider(QBVoDSubtree self, SvObject provider)
{
    SvArrayAddObject(self->providers, provider);
}

void
QBVoDSubtreeStopProviders(QBVoDSubtree self)
{
    for (size_t i = 0; i < SvArrayGetCount(self->providers); i++) {
        QBContentProvider provider = (QBContentProvider) SvArrayGetObjectAtIndex(self->providers, i);
        QBContentProviderStop(provider);
    }
}

SvObject
QBVodSubtreeGetTree(QBVoDSubtree self)
{
    return self->tree;
}

size_t
QBVodSubtreeGetProvidersCount(QBVoDSubtree self)
{
    return SvArrayGetCount(self->providers);
}

SvObject
QBVodSubtreeGetProvider(QBVoDSubtree self, size_t idx)
{
    return SvArrayGetObjectAtIndex(self->providers, idx);
}

void
QBVodSubtreeSetData(QBVoDSubtree self, SvString key, SvObject data)
{
    if (!self->data) {
        self->data = SvHashTableCreate(11, NULL);
    }
    SvHashTableInsert(self->data, (SvObject) key, data);
}

SvObject
QBVodSubtreeGetData(QBVoDSubtree self, SvString key)
{
    if (self->data) {
        return SvHashTableFind(self->data, (SvObject) key);
    }
    return NULL;
}

struct QBVoDSubtrees_ {
    struct SvObject_ super;
    SvHashTable subtrees;
};

SvLocal void
QBVoDSubtrees__dtor__(void* self_)
{
    QBVoDSubtrees self = (QBVoDSubtrees) self_;
    SVRELEASE(self->subtrees);
}

SvType
QBVoDSubtrees_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDSubtrees__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDSubtrees",
                            sizeof(struct QBVoDSubtrees_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBVoDSubtrees
QBVoDSubtreesCreate(void)
{
    QBVoDSubtrees self = (QBVoDSubtrees) SvTypeAllocateInstance(QBVoDSubtrees_getType(), NULL);
    self->subtrees = SvHashTableCreate(11, NULL);
    return self;
}

QBVoDSubtree
QBVoDSubtreesGetSubtree(QBVoDSubtrees self, int id)
{
    SvValue key = SvValueCreateWithInteger(id, NULL);
    QBVoDSubtree ret = (QBVoDSubtree) SvHashTableFind(self->subtrees, (SvObject) key);
    SVRELEASE(key);
    return ret;
}

void
QBVoDSubtreesSetSubtree(QBVoDSubtrees self, int id, QBVoDSubtree subtree)
{
    SvValue key = SvValueCreateWithInteger(id, NULL);
    SvHashTableInsert(self->subtrees, (SvObject) key, (SvObject) subtree);
    SVRELEASE(key);
}
