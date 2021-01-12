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

#include "loadingProxy.h"
#include <stdbool.h>
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvInterface.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBListModel.h>
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBDataSource.h>
#include <QBDataModel3/QBTreeModelListener.h>
#include <QBDataModel3/QBListModelListener.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentCategoryListener.h>
#include <QBContentManager/QBContentTree.h>
#include <QBContentManager/QBLoadingStub.h>


struct QBLoadingProxy_ {
    struct QBDataSource_t super_;
    SvGenericObject tree;
    SvGenericObject path;
    QBLoadingStub loadingStub;
    unsigned numberOfElementsBeforeLoadingStub;
    bool removingLoadingStub;

    /*
     * Boolean self->showLoadingStub informs if there is no content available or we still
     * waiting for information from a middleware
     */
    bool showLoadingStub;
};

SvLocal size_t QBLoadingProxyGetLength(SvGenericObject self_)
{
    QBLoadingProxy self = (QBLoadingProxy) self_;
    if(self->showLoadingStub || self->removingLoadingStub)
        return self->numberOfElementsBeforeLoadingStub + 1; // All elements plus "Loading" stub

    QBTreeIterator it = SvInvokeInterface(QBTreeModel, self->tree, getIterator, self->path, 0);
    return QBTreeIteratorGetNodesCount(&it);
}

SvLocal SvGenericObject QBLoadingProxyGetObject(SvGenericObject self_, size_t idx)
{
    QBLoadingProxy self = (QBLoadingProxy) self_;
    if(self->showLoadingStub) {
        if(idx == self->numberOfElementsBeforeLoadingStub)
            return (SvGenericObject) self->loadingStub;
        else if (idx < self->numberOfElementsBeforeLoadingStub) {
            QBTreeIterator it = SvInvokeInterface(QBTreeModel, self->tree, getIterator, self->path, idx);
            return QBTreeIteratorGetNextNode(&it);
        } else {
            return NULL;
        }
    } else if (self->removingLoadingStub && idx > self->numberOfElementsBeforeLoadingStub) {
        return NULL;
    } else {
        QBTreeIterator it = SvInvokeInterface(QBTreeModel, self->tree, getIterator, self->path, idx);
        return QBTreeIteratorGetNextNode(&it);
    }
}

SvLocal void QBLoadingProxyTreeItemsAdded(SvGenericObject self_, SvGenericObject path,
                                         size_t first, size_t count)
{
    QBLoadingProxy self = (QBLoadingProxy) self_;

    if (!SvObjectEquals(self->path, path))
        return;

    if(self->showLoadingStub) {
        self->showLoadingStub = false;

        /*
         * If first items have been added, then we need to report that first item has changed
         * (from loading to first 'real' item) and later inform that more items were added.
         * The boolean self->removingLoadingStub is used to report consistent data between
         * 'changed' and 'added'. Anyone after reciving 'changed' might call getLength/getObject,
         * so in order for them to be consistent loadingProxy is in a special state.
         */
        self->removingLoadingStub = true;

        // Update element from a loading proxy place
        SvIterator it = QBDataSourceIterator((QBDataSource) self);
        SvObject listener;
        while((listener = SvIteratorGetNext(&it))) {
            SvInvokeInterface(QBListModelListener, listener, itemsChanged, self->numberOfElementsBeforeLoadingStub, 1);
        }
        self->removingLoadingStub = false;

        if(count > 1) {
            it = QBDataSourceIterator((QBDataSource) self);
            while((listener = SvIteratorGetNext(&it))) {
                SvInvokeInterface(QBListModelListener, listener, itemsAdded,
                                  self->numberOfElementsBeforeLoadingStub + 1,
                                  count - self->numberOfElementsBeforeLoadingStub - 1);
            }
        }

    } else {
        SvIterator it = QBDataSourceIterator((QBDataSource) self);
        SvObject listener;
        while((listener = SvIteratorGetNext(&it))) {
            SvInvokeInterface(QBListModelListener, listener, itemsAdded, first, count);
        }
    }
}

SvLocal void QBLoadingProxyTreeItemsRemoved(SvGenericObject self_, SvGenericObject path,
                                            size_t first, size_t count)
{
    QBLoadingProxy self = (QBLoadingProxy) self_;

    if (!SvObjectEquals(self->path, path))
        return;

    QBTreeIterator i = SvInvokeInterface(QBTreeModel, self->tree, getIterator, self->path, 0);
    if ((QBTreeIteratorGetNodesCount(&i) - self->numberOfElementsBeforeLoadingStub) == 0) {
        // there isn't more items left, LoadingProxyStub will be used to show that there is no content

        SvIterator it = QBDataSourceIterator((QBDataSource) self);
        SvGenericObject listener;
        while ((listener = SvIteratorGetNext(&it))) {
            /*
             * 'showLoadingStub' is set before removing element to guarantee that all calls to getObject/getLength
             * will return a proper value (i.e. LoadingProxyStub and length = 1)
             */
            self->showLoadingStub = true;
            self->loadingStub->isLoading = false;

            // remove all items except one for a loading proxy
            SvInvokeInterface(QBListModelListener, listener, itemsRemoved,
                    self->numberOfElementsBeforeLoadingStub + 1, count - self->numberOfElementsBeforeLoadingStub - 1);

            // below operation will replace a previous element with a LoadingProxyStub
            SvInvokeInterface(QBListModelListener, listener, itemsChanged, self->numberOfElementsBeforeLoadingStub, 1);
        }
    } else {
        SvIterator it = QBDataSourceIterator((QBDataSource) self);
        SvGenericObject listener;
        while ((listener = SvIteratorGetNext(&it))) {
            SvInvokeInterface(QBListModelListener, listener, itemsRemoved, first, count);
        }
    }
}

SvLocal void QBLoadingProxyTreeItemsChanged(SvGenericObject self_, SvGenericObject path,
                                              size_t first, size_t count)
{
    QBLoadingProxy self = (QBLoadingProxy) self_;

    if (!SvObjectEquals(self->path, path))
        return;

    SvIterator it = QBDataSourceIterator((QBDataSource) self);
    SvGenericObject listener;
    while((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBListModelListener, listener, itemsChanged, first, count);
    }
}

SvLocal void QBLoadingProxyCategoryStateChanged(SvGenericObject self_,
                                                QBContentCategory category,
                                                QBContentCategoryLoadingState previousState,
                                                QBContentCategoryLoadingState currentState)
{
    QBLoadingProxy self = (QBLoadingProxy) self_;

    if (currentState == QBContentCategoryLoadingState_idle) {
        // set isLoading flag to false to inform that information from middleware was received
        self->loadingStub->isLoading = false;

        QBTreeIterator i = SvInvokeInterface(QBTreeModel, self->tree, getIterator, self->path, 0);
        if ((QBTreeIteratorGetNodesCount(&i) - self->numberOfElementsBeforeLoadingStub) == 0) {
            self->showLoadingStub = true;
            // loading finished and category is still empty, so our stub should
            // change from 'please wait, loading...' to 'no items'
            SvIterator j = QBDataSourceIterator((QBDataSource) self);
            SvGenericObject listener;
            while ((listener = SvIteratorGetNext(&j))) {
                SvInvokeInterface(QBListModelListener, listener, itemsChanged, self->numberOfElementsBeforeLoadingStub, 1);
            }
        }
    }
}

SvLocal void QBLoadingProxyDestroy(void *self_)
{
    QBLoadingProxy self = self_;
    SVRELEASE(self->tree);
    SVTESTRELEASE(self->path);
    SVRELEASE(self->loadingStub);
}

SvLocal SvType QBLoadingProxy_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBLoadingProxyDestroy
    };
    static SvType type = NULL;
    static const struct QBListModel_t listMethods = {
        .getLength = QBLoadingProxyGetLength,
        .getObject = QBLoadingProxyGetObject
    };
    static const struct QBTreeModelListener_t treeListenerMethods = {
        .nodesAdded = QBLoadingProxyTreeItemsAdded,
        .nodesRemoved = QBLoadingProxyTreeItemsRemoved,
        .nodesChanged = QBLoadingProxyTreeItemsChanged
    };
    static const struct QBContentCategoryListener_ categoryListenerMethods = {
        .loadingStateChanged = QBLoadingProxyCategoryStateChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBLoadingProxy",
                            sizeof(struct QBLoadingProxy_),
                            QBDataSource_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBListModel_getInterface(), &listMethods,
                            QBTreeModelListener_getInterface(), &treeListenerMethods,
                            QBContentCategoryListener_getInterface(), &categoryListenerMethods,
                            NULL);
    }

    return type;
}

QBLoadingProxy QBLoadingProxyCreate(SvGenericObject tree, SvGenericObject path)
{
    SvGenericObject node = SvInvokeInterface(QBTreeModel, tree, getNode, path);
    return QBLoadingProxyCreateForNode(tree, path, node);
}

QBLoadingProxy QBLoadingProxyCreateForNode(SvGenericObject tree, SvGenericObject path, SvObject node)
{
    QBLoadingProxy self = (QBLoadingProxy) SvTypeAllocateInstance(QBLoadingProxy_getType(), NULL);
    QBDataSourceInit((QBDataSource) self, QBListModelListener_getInterface(), NULL);
    self->tree = SVRETAIN(tree);
    self->path = SVTESTRETAIN(path);
    self->loadingStub = (QBLoadingStub) SvTypeAllocateInstance(QBLoadingStub_getType(), NULL);
    self->loadingStub->isLoading = true;
    self->showLoadingStub = true;

    SvInvokeInterface(QBDataModel, self->tree, addListener, (SvGenericObject) self, NULL);
    QBContentCategory category = NULL;
    if (node) {
        if (SvObjectIsInstanceOf(node, QBContentCategory_getType())) {
            category = (QBContentCategory) node;
        } else if ((SvObjectIsInstanceOf(node, QBActiveTreeNode_getType())) &&
                   (SvObjectIsInstanceOf(tree, QBActiveTree_getType()))) {
            SvObject mountedTree = QBActiveTreeGetMountedSubTree((QBActiveTree) tree, path);
            if (mountedTree && (SvObjectIsInstanceOf(mountedTree, QBContentTree_getType()))) {
                category = QBContentTreeGetRootCategory((QBContentTree) mountedTree);
            }
        }
    }

    if (category) {
        QBContentCategoryAddListener(category, (SvGenericObject) self, NULL);

        if ((QBContentCategoryGetLength(category) - QBContentCategoryGetStaticCount(category)) != 0) {
            // items already in content category, LoadingProxyStub shouldn't be shown
            self->showLoadingStub = false;
        } else if (QBContentCategoryGetLoadingState(category) == QBContentCategoryLoadingState_idle) {
            // we already know that this category is empty
            self->loadingStub->isLoading = false;
        }
        self->numberOfElementsBeforeLoadingStub = QBContentCategoryGetStaticCount(category);

        // Forced to load the content
        SvInvokeInterface(QBTreeModel, self->tree, getIterator, self->path, 0);
    } else {
        QBTreeIterator it = SvInvokeInterface(QBTreeModel, self->tree, getIterator, self->path, 0);
        size_t len = QBTreeIteratorGetNodesCount(&it);
        if (len > 0)
            self->showLoadingStub = false;
    }

    return self;
}
