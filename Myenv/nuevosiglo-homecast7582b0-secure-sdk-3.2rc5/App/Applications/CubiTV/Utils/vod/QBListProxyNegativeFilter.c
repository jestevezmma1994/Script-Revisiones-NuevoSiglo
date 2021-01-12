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

#include "QBListProxyNegativeFilter.h"
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
 * Class for filtering out the first occurrence of element of specific type from the source list.
 **/
struct QBListProxyNegativeFilter_ {
    struct QBListDataSource_ super_;    ///< superclass
    QBListProxy list;                   ///< source list
    SvType filterType;                  ///< type of element that should be filtered out
    size_t filterIndex;                 ///< index of element that was filtered out (if found in source)
};


// QBListModel virtual methods

SvLocal size_t
QBListProxyNegativeFilterGetLength(SvObject self_)
{
    QBListProxyNegativeFilter self = (QBListProxyNegativeFilter) self_;
    size_t originLength = SvInvokeInterface(QBListModel, self->list, getLength);
    return self->filterIndex == INVALID_IDX ? originLength : originLength - 1;
}

SvLocal SvObject
QBListProxyNegativeFilterGetObject(SvObject self_,
                                   size_t idx)
{
    QBListProxyNegativeFilter self = (QBListProxyNegativeFilter) self_;
    if (self->filterIndex != INVALID_IDX && idx >= self->filterIndex) {
        ++idx;
    }
    return SvInvokeInterface(QBListModel, self->list, getObject, idx);
}

// QBListModelListener virtual methods

SvLocal void
QBListProxyNegativeFilterItemsAdded(SvObject self_,
                                    size_t first, size_t count)
{
    QBListProxyNegativeFilter self = (QBListProxyNegativeFilter) self_;
    bool filteredFound = false;
    for (size_t idx = first; idx < first + count; ++idx) {
        SvObject added = SvInvokeInterface(QBListModel, (SvObject) self->list, getObject, idx);
        if (self->filterIndex == INVALID_IDX && SvObjectIsInstanceOf(added, self->filterType)) {
            self->filterIndex = idx;
            filteredFound = true;
        }
    }
    if (!filteredFound && self->filterIndex != INVALID_IDX && first > self->filterIndex) {
        --first;
    }
    if (filteredFound) {
        --count;
    }
    if (count == 0) {
        return;
    }
    SvIterator i = QBDataSourceIterator((QBDataSource) self);
    SvObject listener;
    while ((listener = SvIteratorGetNext(&i))) {
        SvInvokeInterface(QBListModelListener, listener, itemsAdded, first, count);
    }
}

SvLocal void
QBListProxyNegativeFilterItemsRemoved(SvObject self_,
                                      size_t first, size_t count)
{
    QBListProxyNegativeFilter self = (QBListProxyNegativeFilter) self_;
    if (self->filterIndex != INVALID_IDX && self->filterIndex >= first && self->filterIndex < first + count) {
        self->filterIndex = INVALID_IDX;
        --count;
    } else if (self->filterIndex != INVALID_IDX && self->filterIndex < first) {
        --first;
    }
    if (count == 0) {
        return;
    }
    SvIterator i = QBDataSourceIterator((QBDataSource) self);
    SvObject listener;
    while ((listener = SvIteratorGetNext(&i))) {
        SvInvokeInterface(QBListModelListener, listener, itemsRemoved, first, count);
    }
}

SvLocal void
QBListProxyNegativeFilterItemsChanged(SvObject self_,
                                      size_t first, size_t count)
{
    QBListProxyNegativeFilter self = (QBListProxyNegativeFilter) self_;
    if (self->filterIndex != INVALID_IDX && self->filterIndex >= first && self->filterIndex < first + count) {
        --count;
    } else if (self->filterIndex != INVALID_IDX && self->filterIndex < first) {
        --first;
    }
    if (count == 0) {
        return;
    }
    SvIterator i = QBDataSourceIterator((QBDataSource) self);
    SvObject listener;
    while ((listener = SvIteratorGetNext(&i))) {
        SvInvokeInterface(QBListModelListener, listener, itemsChanged, first, count);
    }
}

SvLocal void
QBListProxyNegativeFilterItemsReordered(SvObject self_,
                                        size_t first, size_t count)
{
    QBListProxyNegativeFilter self = (QBListProxyNegativeFilter) self_;
    if (self->filterIndex != INVALID_IDX && self->filterIndex >= first && self->filterIndex < first + count) {
        --count;
    } else if (self->filterIndex != INVALID_IDX && self->filterIndex < first) {
        --first;
    }
    if (count == 0) {
        return;
    }
    SvIterator i = QBDataSourceIterator((QBDataSource) self);
    SvObject listener;
    while ((listener = SvIteratorGetNext(&i))) {
        SvInvokeInterface(QBListModelListener, listener, itemsReordered, first, count);
    }
}

// public API

SvLocal void
QBListProxyNegativeFilterDestroy(void *self_)
{
    QBListProxyNegativeFilter self = self_;

    SVTESTRELEASE(self->list);
}

SvType
QBListProxyNegativeFilter_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBListProxyNegativeFilterDestroy
    };
    static const struct QBListModelListener_t listListenerMethods = {
        .itemsAdded     = QBListProxyNegativeFilterItemsAdded,
        .itemsRemoved   = QBListProxyNegativeFilterItemsRemoved,
        .itemsChanged   = QBListProxyNegativeFilterItemsChanged,
        .itemsReordered = QBListProxyNegativeFilterItemsReordered
    };
    static const struct QBListModel_ listMethods = {
        .getLength = QBListProxyNegativeFilterGetLength,
        .getObject = QBListProxyNegativeFilterGetObject
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBListProxyNegativeFilter",
                            sizeof(struct QBListProxyNegativeFilter_),
                            QBListDataSource_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBListModelListener_getInterface(), &listListenerMethods,
                            QBListModel_getInterface(), &listMethods,
                            NULL);
    }

    return type;
}

QBListProxyNegativeFilter
QBListProxyNegativeFilterCreate(QBListProxy list,
                                SvType filterType,
                                SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBListProxyNegativeFilter self = NULL;

    if (!list) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL list handle passed");
        goto fini;
    }

    self = (QBListProxyNegativeFilter) SvTypeAllocateInstance(QBListProxyNegativeFilter_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "can't allocate QBListProxyNegativeFilter");
    } else if (!QBListDataSourceInit((QBListDataSource) self, &error)) {
        SVRELEASE(self);
        self = NULL;
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error, "can't allocate QBListProxyNegativeFilter");
    } else {
        self->list = SVRETAIN(list);
        self->filterIndex = INVALID_IDX;
        self->filterType = filterType;
        SvInvokeInterface(QBDataModel, list, addListener, (SvObject) self, NULL);
        size_t listLen = SvInvokeInterface(QBListModel, (SvObject) list, getLength);
        for (size_t idx = 0; idx < listLen; ++idx) {
            SvObject checked = SvInvokeInterface(QBListModel, (SvObject) self->list, getObject, idx);
            if (self->filterIndex == INVALID_IDX && SvObjectIsInstanceOf(checked, self->filterType)) {
                self->filterIndex = idx;
            }
        }
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

SvObject
QBListProxyNegativeFilterGetTreeNode(QBListProxyNegativeFilter self,
                                     size_t idx,
                                     SvObject *path)
{
    if (self->filterIndex != INVALID_IDX && idx >= self->filterIndex) {
        ++idx;
    }
    return QBListProxyGetTreeNode(self->list, idx, path);
}
