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

#include "QBListProxyFilter.h"
#include <QBDataModel3/QBListProxy.h>
#include <QBDataModel3/QBListModelListener.h>
#include <limits.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBListModel.h>
#include <QBDataModel3/QBListDataSource.h>

#define INVALID_IDX ((size_t) -1)

/**
 * The filter class for filtering only
 * specific elements from the list.
 * The filter can be used to filter a specific object,
 * a specific object type (first ocurence) or both.
 *
 **/
struct QBListProxyFilter_ {
    struct QBListDataSource_ super_;    ///< superclass
    QBListProxy list;                   ///< source list that is filtered by this filter
    SvObject filterObject;              ///< object that should be present in the result if present in source.
    size_t filterIndex;                 ///< index of filterObject element in the source list (if found)
    SvType searchType;                  ///< type of element that should also be present in the result if present in source.
    size_t searchIndex;                 ///< index of object of type equal to searchType in the source list (if found)
};


// QBListModel virtual methods

SvLocal size_t
QBListProxyFilterGetLength(SvObject self_)
{
    QBListProxyFilter self = (QBListProxyFilter) self_;
    size_t count = 0;
    if (self->filterIndex != INVALID_IDX) {
        ++count;
    }
    if (self->searchIndex != INVALID_IDX) {
        ++count;
    }
    return count;
}

SvLocal SvObject
QBListProxyFilterGetObject(SvObject self_,
                           size_t idx)
{
    QBListProxyFilter self = (QBListProxyFilter) self_;
    if (idx == 0) {
        if (self->searchIndex == INVALID_IDX) {
            idx = 1;
        } else {
            return SvInvokeInterface(QBListModel, (SvObject) self->list, getObject, self->searchIndex);
        }
    }

    if (idx == 1) {
        return self->filterIndex != INVALID_IDX ? SvInvokeInterface(QBListModel, (SvObject) self->list, getObject, self->filterIndex) : NULL;
    }
    return NULL;
}

// QBListModelListener virtual methods

SvLocal void
QBListProxyFilterItemsAdded(SvObject self_,
                            size_t first, size_t count)
{
    QBListProxyFilter self = (QBListProxyFilter) self_;
    bool objectAdded = false;
    bool searchAdded = false;
    for (size_t idx = first; idx < first + count; ++idx) {
        SvObject added = SvInvokeInterface(QBListModel, (SvObject) self->list, getObject, idx);
        if (added == self->filterObject) {
            self->filterIndex = idx;
            objectAdded = true;
        } else if (self->searchIndex == INVALID_IDX && SvObjectIsInstanceOf(added, self->searchType)) {
            self->searchIndex = idx;
            searchAdded = true;
        }
    }
    if (objectAdded || searchAdded) {
        const size_t addedCount = objectAdded && searchAdded ? 2 : 1;
        const size_t startIdx = searchAdded || self->searchIndex == INVALID_IDX ? 0 : 1;
        SvIterator i = QBDataSourceIterator((QBDataSource) self);
        SvObject listener;
        while ((listener = SvIteratorGetNext(&i))) {
            SvInvokeInterface(QBListModelListener, listener, itemsAdded, startIdx, addedCount);
        }
    }
}

SvLocal void
QBListProxyFilterItemsRemoved(SvObject self_,
                              size_t first, size_t count)
{
    QBListProxyFilter self = (QBListProxyFilter) self_;
    bool objectRemoved = false;
    bool searchRemoved = false;
    if (self->filterIndex >= first && self->filterIndex < first + count) {
        self->filterIndex = INVALID_IDX;
        objectRemoved = true;
    }
    if (self->searchIndex >= first && self->searchIndex < first + count) {
        self->searchIndex = INVALID_IDX;
        searchRemoved = true;
    }
    if (objectRemoved || searchRemoved) {
        const size_t removedCount = objectRemoved && searchRemoved ? 2 : 1;
        const size_t startIdx = searchRemoved || self->searchIndex == INVALID_IDX ? 0 : 1;
        SvIterator i = QBDataSourceIterator((QBDataSource) self);
        SvObject listener;
        while ((listener = SvIteratorGetNext(&i))) {
            SvInvokeInterface(QBListModelListener, listener, itemsRemoved, startIdx, removedCount);
        }
    }
}

SvLocal void
QBListProxyFilterItemsChanged(SvObject self_,
                              size_t first, size_t count)
{
    QBListProxyFilter self = (QBListProxyFilter) self_;
    bool objectChanged = false;
    bool searchChanged = false;
    if (self->filterIndex >= first && self->filterIndex < first + count) {
        objectChanged = true;
    }
    if (self->searchIndex >= first && self->searchIndex < first + count) {
        searchChanged = true;
    }

    if (objectChanged || searchChanged) {
        const size_t changedCount = objectChanged && searchChanged ? 2 : 1;
        const size_t startIdx = searchChanged || self->searchIndex == INVALID_IDX ? 0 : 1;
        SvIterator i = QBDataSourceIterator((QBDataSource) self);
        SvObject listener;
        while ((listener = SvIteratorGetNext(&i))) {
            SvInvokeInterface(QBListModelListener, listener, itemsChanged, startIdx, changedCount);
        }
    }
}

SvLocal void
QBListProxyFilterItemsReordered(SvObject self_,
                                size_t first, size_t count)
{
    QBListProxyFilter self = (QBListProxyFilter) self_;
    if (self->filterIndex >= first && self->filterIndex < first + count) {
        for (size_t idx = first; idx < first + count; ++idx) {
            SvObject moved = SvInvokeInterface(QBListModel, (SvObject) self->list, getObject, idx);
            if (moved == self->filterObject) {
                self->filterIndex = idx;
                break;
            }
        }
    }

    if (self->searchIndex >= first && self->searchIndex < first + count) {
        for (size_t idx = first; idx < first + count; ++idx) {
            SvObject moved = SvInvokeInterface(QBListModel, (SvObject) self->list, getObject, idx);
            if (SvObjectIsInstanceOf(moved, self->searchType)) {
                self->searchIndex = idx;
                break;
            }
        }
    }
}

// public API

SvLocal void
QBListProxyFilterDestroy(void *self_)
{
    QBListProxyFilter self = self_;

    SVTESTRELEASE(self->list);
}

SvType
QBListProxyFilter_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBListProxyFilterDestroy
    };
    static const struct QBListModelListener_t listListenerMethods = {
        .itemsAdded     = QBListProxyFilterItemsAdded,
        .itemsRemoved   = QBListProxyFilterItemsRemoved,
        .itemsChanged   = QBListProxyFilterItemsChanged,
        .itemsReordered = QBListProxyFilterItemsReordered
    };
    static const struct QBListModel_ listMethods = {
        .getLength = QBListProxyFilterGetLength,
        .getObject = QBListProxyFilterGetObject
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBListProxyFilter",
                            sizeof(struct QBListProxyFilter_),
                            QBListDataSource_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBListModelListener_getInterface(), &listListenerMethods,
                            QBListModel_getInterface(), &listMethods,
                            NULL);
    }

    return type;
}

QBListProxyFilter
QBListProxyFilterCreate(QBListProxy list,
                        SvObject filterObject,
                        SvType searchType,
                        SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBListProxyFilter self = NULL;

    if (!list) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL list handle passed");
        goto fini;
    }

    self = (QBListProxyFilter) SvTypeAllocateInstance(QBListProxyFilter_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "can't allocate QBListProxyFilter");
    } else if (!QBListDataSourceInit((QBListDataSource) self, &error)) {
        SVRELEASE(self);
        self = NULL;
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error, "can't allocate QBListProxyFilter");
    } else {
        self->list = SVRETAIN(list);
        self->filterObject = filterObject;
        self->filterIndex = INVALID_IDX;
        self->searchIndex = INVALID_IDX;
        self->searchType = searchType;
        SvInvokeInterface(QBDataModel, list, addListener, (SvObject) self, NULL);
        size_t listLen = SvInvokeInterface(QBListModel, (SvObject) list, getLength);
        for (size_t idx = 0; idx < listLen; ++idx) {
            SvObject checked = SvInvokeInterface(QBListModel, (SvObject) self->list, getObject, idx);
            if (checked == self->filterObject) {
                self->filterIndex = idx;
            } else if (self->searchIndex == INVALID_IDX && SvObjectIsInstanceOf(checked, self->searchType)) {
                self->searchIndex = idx;
            }
        }
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

SvObject
QBListProxyFilterGetTreeNode(QBListProxyFilter self,
                             size_t idx,
                             SvObject *path)
{
    assert(self);
    if (idx == 0) {
        if (self->searchIndex == INVALID_IDX) {
            idx = 1;
        } else {
            return QBListProxyGetTreeNode(self->list, self->searchIndex, path);
        }
    }

    if (idx == 1 && self->filterIndex != INVALID_IDX) {
        return QBListProxyGetTreeNode(self->list, self->filterIndex, path);
    }
    return NULL;
}
