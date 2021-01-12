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

#include <ContextMenus/QBCall2Action/QBCall2ActionPane.h>
#include <ContextMenus/QBCall2Action/QBCall2ActionHandler.h>
#include <ContextMenus/QBCall2Action/log.h>
#include <ContextMenus/QBLoadablePane.h>
#include <ContextMenus/QBLoadablePaneListener.h>
#include <Menus/menuchoice.h>
#include <main.h>
#include <QBCall2ActionClient/QBCall2ActionProvider.h>
#include <QBCall2ActionClient/QBCall2ActionTreeNode.h>
#include <QBDataModel3/QBTreeModelListener.h>
#include <QBDataModel3/QBTreePathMap.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBDataModel.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <settings.h>

struct QBCall2ActionPane_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    QBContextMenu ctxMenu;
    SvArray panes;
    bool loadingPaneActive;
    int level;

    size_t timeOutMs;
    bool timeOutExpired;

    void * callbackData;
    QBCall2ActionPaneTimeOutCallback callback;

    QBCall2ActionProvider provider;

    SvWeakReference controller;
    QBTreePathMap itemControllersPathMap;
    SvObject currentItemConstructor;
};

SvLocal void
QBCall2ActionPaneDestroy(void * self_)
{
    QBCall2ActionPane self = (QBCall2ActionPane) self_;
    SVRELEASE(self->panes);
    SVRELEASE(self->ctxMenu);
    SVRELEASE(self->provider);
    SVTESTRELEASE(self->controller);
    SVRELEASE(self->itemControllersPathMap);
}

SvLocal QBBasicPane
QBCall2ActionPaneCreateLevel(QBCall2ActionPane self, int level, SvErrorInfo * errorOut);

SvLocal void
QBCall2ActionPaneAddOptionsFromIterator(QBCall2ActionPane self, SvObject path, size_t count, size_t first, int level);

SvLocal void
QBCall2ActionPanePopAllPanes(QBCall2ActionPane self);

SvLocal void
QBCall2ActionPaneNodesAdded(SvObject self_, SvObject path, size_t first, size_t count)
{
    assert(self_ != NULL);
    QBCall2ActionPane self = (QBCall2ActionPane) self_;

    if (count == 0) {
        QBCall2ActionPanePopLevel(self, NULL);
        return;
    }

    if (self->timeOutExpired)
        return;

    int level = (int) SvInvokeInterface(QBTreePath, path, getLength) - 1;
    if (level <= self->level) {
        if (self->loadingPaneActive) {
            self->loadingPaneActive = false;
            SvObject pane = (SvObject) QBCall2ActionPaneCreateLevel(self, self->level, NULL);
            SvObject handler = QBTreePathMapFind(self->itemControllersPathMap, path);
            QBBasicPaneSetItemConstructor((QBBasicPane) pane, handler);
            QBContextMenuSwitchPane(self->ctxMenu, pane);
            SvArrayRemoveLastObject(self->panes);
            SvArrayAddObject(self->panes, pane);
            SVRELEASE(pane);
        }
        QBCall2ActionPaneAddOptionsFromIterator(self, path, count, first, (int) level);
        if (self->controller) {
            SvObject handler = SvWeakReferenceTakeReferredObject(self->controller);
            if (handler) {
                SvInvokeInterface(QBCall2ActionHandler, handler, onNewLevel, path, NULL);
                SVRELEASE(handler);
            }
        }
    }
}

SvLocal void
QBCall2ActionPaneNodesRemoved(SvObject self_, SvObject path, size_t first, size_t count)
{
    SvLogError("Method not supported");
}

SvLocal void
QBCall2ActionPaneNodesChanged(SvObject self_, SvObject path, size_t first, size_t count)
{
    SvLogError("Method not supported");
}

SvLocal void
QBCall2ActionPaneShow(SvObject self_)
{
    assert(self_ != NULL);
}

SvLocal void
QBCall2ActionPaneHide(SvObject self_, bool immediately)
{
    assert(self_ != NULL);
}

SvLocal void
QBCall2ActionPaneSetActive(SvObject self_)
{
    assert(self_ != NULL);
}

SvLocal bool
QBCall2ActionPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvLocal void
QBCall2ActionPanePopLastPane(QBCall2ActionPane self)
{
    assert(self != NULL);

    QBContextMenuPopPane(self->ctxMenu);
    SvArrayRemoveLastObject(self->panes);
    self->level--;
}

SvLocal void
QBCall2ActionPanePopAllPanes(QBCall2ActionPane self)
{
    assert(self != NULL);

    if (self->loadingPaneActive)
        self->loadingPaneActive = false;

    while (self->level >= 0)
        QBCall2ActionPanePopLastPane(self);
}

SvLocal void
QBCall2ActionPaneStateChanged(SvObject self_,
                              QBLoadablePane pane,
                              QBLoadablePaneState state)
{
    QBCall2ActionPane self = (QBCall2ActionPane) self_;

    if (state == QBLoadablePaneStateTimeOut) {
        log_debug("TimeOut!");
        self->timeOutExpired = true;
        if (self->callback && self->callbackData)
            self->callback(self->callbackData, self);
    }
}

SvLocal SvType
QBCall2ActionPane_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBCall2ActionPaneDestroy
    };

    static const struct QBTreeModelListener_t treeMethods = {
        .nodesAdded   = QBCall2ActionPaneNodesAdded,
        .nodesRemoved = QBCall2ActionPaneNodesRemoved,
        .nodesChanged = QBCall2ActionPaneNodesChanged
    };

    static const struct QBContextMenuPane_ methods = {
        .show             = QBCall2ActionPaneShow,
        .hide             = QBCall2ActionPaneHide,
        .setActive        = QBCall2ActionPaneSetActive,
        .handleInputEvent = QBCall2ActionPaneHandleInputEvent
    };

    static const struct QBLoadablePaneListener_ paneMethods = {
        .stateChanged = QBCall2ActionPaneStateChanged
    };

    if (!type) {
        SvTypeCreateManaged("QBCall2ActionPane", sizeof(struct QBCall2ActionPane_),
                            SvObject_getType(), &type, SvObject_getType(), &vtable,
                            QBTreeModelListener_getInterface(), &treeMethods,
                            QBContextMenuPane_getInterface(), &methods,
                            QBLoadablePaneListener_getInterface(), &paneMethods,
                            NULL);
    }
    return type;
}

SvLocal void
QBCall2ActionPaneCallback(void * self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    assert(self_ != NULL);
    assert(item != NULL);

    QBCall2ActionPane self = (QBCall2ActionPane) self_;
    SVAUTOSTRING(pathStr, "path");
    SvObject path = SvHashTableFind(item->metadata, (SvObject) pathStr);
    SVAUTOSTRING(nodeStr, "node");
    SvObject node = SvHashTableFind(item->metadata, (SvObject) nodeStr);
    if (self->controller) {
        SvObject handler = SvWeakReferenceTakeReferredObject(self->controller);
        if (handler) {
            SvInvokeInterface(QBCall2ActionHandler, handler, onOptionSelected, path, node);
            SVRELEASE(handler);
        }
    }
}

SvLocal QBBasicPane
QBCall2ActionPaneCreateLevel(QBCall2ActionPane self, int level, SvErrorInfo * errorOut)
{
    assert(self != NULL);
    assert(level >= 0);

    SvErrorInfo error = NULL;

    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane pane = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "SvTypeAllocateInstance call failed.");
        goto err;
    }
    QBBasicPaneInit(pane, self->appGlobals->res, self->appGlobals->scheduler,
                    self->appGlobals->textRenderer, self->ctxMenu, level + 1,
                    SVSTRING("BasicPane"));

    svSettingsPopComponent();
    return pane;
err:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

SvLocal QBBasicPaneItem
QBCall2ActionPaneAddOptionWithNode(QBBasicPane pane, QBCall2ActionTreeNode node, QBBasicPaneItemCallback callback,
                                   SvObject callbackData, SvErrorInfo * errorOut)
{
    assert(pane != NULL);
    assert(node != NULL);

    SvErrorInfo error = NULL;
    SvString id = NULL;
    SvValue idValue = NULL;
    SvString caption = NULL;

    SvValue value = QBCall2ActionTreeNodeGetAttrValue(node, "caption", NULL);
    if (!value) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Action haven't got caption attribute");
        goto err;
    }
    caption = SvValueGetString(value);
    idValue = QBCall2ActionTreeNodeGetId(node, &error);
    if (!idValue)
        goto err;
    id = SvValueGetString(idValue);
    return QBBasicPaneAddOption(pane, id, caption, callback, callbackData);
err:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

SvLocal void
QBCall2ActionPaneAddOptionsFromIterator(QBCall2ActionPane self, SvObject path, size_t count, size_t first, int level)
{
    assert(self != NULL);
    assert(path != NULL);
    assert(count > 0);
    assert(level >= 0);

    size_t i = 0;
    SvErrorInfo error = NULL;
    QBCall2ActionTreeNode node = NULL;
    QBBasicPaneItem item = NULL;

    QBTreeIterator nodeIter = SvInvokeInterface(QBTreeModel, self->provider, getIterator, path, first);
    QBTreeIterator pathIter = SvInvokeInterface(QBTreeModel, self->provider, getIterator, path, first);
    QBBasicPane pane = (QBBasicPane) SvArrayAt(self->panes, level);

    while ((node = (QBCall2ActionTreeNode) QBTreeIteratorGetNextNode(&nodeIter)) && i < count) {
        if ((item = QBCall2ActionPaneAddOptionWithNode(pane, node, QBCall2ActionPaneCallback, (SvObject) self, &error))) {
            SvObject newPath = QBTreeIteratorGetCurrentNodePath(&pathIter);
            QBTreeIteratorGetNextNode(&pathIter);
            SvHashTableInsert(item->metadata, (SvObject) SVSTRING("path"), newPath);
            SvHashTableInsert(item->metadata, (SvObject) SVSTRING("node"), (SvObject) node);
            SVRELEASE(newPath);
        } else {
            goto err;
        }
        i++;
    }
    return;
err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
}

bool
QBCall2ActionPaneSetTimeOutCallback(QBCall2ActionPane self, QBCall2ActionPaneTimeOutCallback callback,
                                    SvObject callbackData, SvErrorInfo * errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed. (self=%p)", self);
        SvErrorInfoPropagate(error, errorOut);
        return false;
    }

    self->callback = callback;
    self->callbackData = callbackData;
    return true;
}

bool
QBCall2ActionPaneSetItemController(QBCall2ActionPane self, SvObject path, SvObject itemController, SvErrorInfo * errorOut)
{
    SvErrorInfo error = NULL;

    if (!self || !path) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed. (self=%p, path=%p)", self, path);
        SvErrorInfoPropagate(error, errorOut);
        return false;
    }

    if (itemController) {
        QBTreePathMapInsert(self->itemControllersPathMap, path, itemController, NULL);
    } else {
        if (!QBTreePathMapFind(self->itemControllersPathMap, path))
            return false;
        QBTreePathMapRemove(self->itemControllersPathMap, path, NULL);
    }
    return true;
}

bool
QBCall2ActionPaneSetController(QBCall2ActionPane self, SvObject controller, SvErrorInfo * errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed.");
        SvErrorInfoPropagate(error, errorOut);
        return false;
    }

    SVTESTRELEASE(self->controller);
    self->controller = controller ? SvWeakReferenceCreate(controller, NULL) : NULL;
    return true;
}

bool
QBCall2ActionPanePushLevel(QBCall2ActionPane self, SvErrorInfo * errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed.");
        goto err;
    }
    self->level++;
    QBLoadablePane pane = QBLoadablePaneCreate(self->ctxMenu, self->level + 1,
                                               self->timeOutMs, self->appGlobals, &error);
    if (!pane)
        goto err;

    QBLoadablePaneSetStateListener(pane, (SvObject) self, NULL);

    self->loadingPaneActive = true;
    QBContextMenuPushPane(self->ctxMenu, (SvObject) pane);
    SvArrayAddObject(self->panes, (SvObject) pane);
    SVRELEASE(pane);
    return true;
err:
    SvErrorInfoPropagate(error, errorOut);
    return false;
}

bool
QBCall2ActionPanePopLevel(QBCall2ActionPane self, SvErrorInfo * errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed.");
        SvErrorInfoPropagate(error, errorOut);
        return false;
    }
    if (self->loadingPaneActive)
        self->loadingPaneActive = false;
    QBCall2ActionPanePopLastPane(self);
    return true;
}

bool
QBCall2ActionPanePopAllLevels(QBCall2ActionPane self, SvErrorInfo * errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed.");
        SvErrorInfoPropagate(error, errorOut);
        return false;
    }
    QBCall2ActionPanePopAllPanes(self);
    return true;
}

bool
QBCall2ActionPaneAddOption(QBCall2ActionPane self, SvString id, SvString caption, SvErrorInfo * errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed. (self=%p)", self);
        SvErrorInfoPropagate(error, errorOut);
        return false;
    }
    log_debug("Adding option.");

    QBBasicPane pane = (QBBasicPane) SvArrayAt(self->panes, self->level);
    QBBasicPaneAddOption(pane, id, caption, QBCall2ActionPaneCallback, self);
    return true;
}


bool
QBCall2ActionPaneAddAction(QBCall2ActionPane self, QBCall2ActionTreeNode action, QBBasicPaneItemCallback callback,
                           SvObject callbackData, SvErrorInfo * errorOut)
{
    SvErrorInfo error = NULL;

    if (!self || !action) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed. (self=%p, action=%p)",
                                  self, action);
        goto err;
    }

    log_debug("Adding action.");

    QBBasicPane pane = (QBBasicPane) SvArrayAt(self->panes, self->level);
    if (QBCall2ActionPaneAddOptionWithNode(pane, action, callback, callbackData, &error))
        return true;
err:
    SvErrorInfoPropagate(error, errorOut);
    return false;
}

QBCall2ActionPane
QBCall2ActionPaneCreate(QBCall2ActionProvider provider, AppGlobals appGlobals,
                        QBContextMenu ctxMenu, size_t timeOutMs, SvErrorInfo * errorOut)
{
    SvErrorInfo error = NULL;
    QBCall2ActionPane self = NULL;

    log_debug("Creating Call2ActionPane.");

    if (!provider || !appGlobals || !ctxMenu) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed. (provider=%p, appGlobals=%p, ctxMennu=%p)",
                                  provider, appGlobals, ctxMenu);
        goto err;
    }

    self = (QBCall2ActionPane) SvTypeAllocateInstance(QBCall2ActionPane_getType(), &error);
    if (error)
        goto err;

    self->level = -1;
    self->ctxMenu = SVRETAIN(ctxMenu);
    self->appGlobals = appGlobals;
    self->provider = SVRETAIN(provider);
    self->timeOutMs = timeOutMs;
    self->itemControllersPathMap = QBTreePathMapCreate(&error);
    if (error)
        goto err;

    self->panes = SvArrayCreateWithCapacity(3, NULL);

    SvInvokeInterface(QBDataModel, (SvObject) self->provider, addListener, (SvObject) self, &error);
    if (error)
        goto err;

    self->loadingPaneActive = false;
    return self;

err:
    SvErrorInfoPropagate(error, errorOut);
    SVTESTRELEASE(self);
    return NULL;
}
