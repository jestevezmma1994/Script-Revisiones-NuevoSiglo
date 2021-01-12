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

#include <ContextMenus/QBCall2Action/QBCall2ActionContext.h>
#include <ContextMenus/QBCall2Action/QBCall2ActionPane.h>
#include <ContextMenus/QBCall2Action/QBCall2ActionItemController.h>
#include <ContextMenus/QBCall2Action/QBCall2ActionHandler.h>
#include <ContextMenus/QBCall2Action/log.h>
#include <ContextMenus/QBContextMenu.h>
#include <Widgets/QBLoadableDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Utils/value.h>
#include <main.h>
#include <QBCall2ActionClient/QBCall2ActionProvider.h>
#include <QBCall2ActionClient/QBCall2ActionStateListener.h>
#include <QBCall2ActionClient/QBCall2ActionTree.h>
#include <QBContentManager/QBRangeTree.h>
#include <QBContentManager/QBContentTree.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBTreePathMap.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/event.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <SvDataBucket2/SvDBObject.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <fibers/c/fibers.h>
#include <settings.h>

#define TIMEOUT 15 * 1000

struct QBCall2ActionContext_ {
    struct SvObject_ super;

    AppGlobals appGlobals;

    QBContextMenu ctxMenu;
    QBCall2ActionPane pane;

    SvArray callStateStack;
    QBCall2ActionProvider provider;
    SvObject itemController;
    SvObject path;
    QBTreePathMap handlersPathMap;

    bool needPopup;
    SvHashTable popupParams;
    SvWidget dialog;

    SvFiber fiber;
    SvFiberTimer timer;

    SvObject contentTree;
    SvObject product;
    QBContentCategory category;

    void * callbackData;
    QBCall2ActionContextOnCloseCallback callback;

    bool started;
};

SvLocal void
QBCall2ActionContextRemoveCallStateFromStack(QBCall2ActionContext self,
                                             QBCall2ActionState callState);

SvLocal void
QBCall2ActionContextDestroy(void * self_)
{
    QBCall2ActionContext self = (QBCall2ActionContext) self_;

    if (self->started)
        QBCall2ActionContextStop(self, NULL);

    SVTESTRELEASE(self->ctxMenu);

    SVRELEASE(self->callStateStack);

    SVRELEASE(self->path);
    SVTESTRELEASE(self->provider);
    SVTESTRELEASE(self->itemController);
    SVTESTRELEASE(self->pane);
    self->pane = NULL;
    SVRELEASE(self->contentTree);
    SVTESTRELEASE(self->product);
    SVTESTRELEASE(self->category);
    SVTESTRELEASE(self->popupParams);
    SVRELEASE(self->handlersPathMap);
    SvFiberDestroy(self->fiber);
}

SvLocal void
QBCall2ActionContextAddCallStateToStack(QBCall2ActionContext self,
                                        QBCall2ActionState callState)
{
    log_fun();
    QBCall2ActionStateSetListener(callState, (SvObject) self, NULL);
    SvArrayAddObject(self->callStateStack, (SvObject) callState);
}

SvLocal void
QBCall2ActionContextRemoveCallStateFromStack(QBCall2ActionContext self,
                                             QBCall2ActionState callState)
{
    log_fun();
    QBCall2ActionStateSetListener(callState, NULL, NULL);
    SvArrayRemoveObject(self->callStateStack, (SvObject) callState);
}

SvLocal void
QBCall2ActionContextPopupCallback(void *self_, SvWidget dialog, SvString buttonTag,
                                  unsigned keyCode)
{
    log_fun();
    QBCall2ActionContext self = (QBCall2ActionContext) self_;
    if (!self->dialog || !self->started)
        return;
    self->dialog = NULL;
    if (self->callback && self->callbackData)
        self->callback(self->callbackData);
}

SvLocal void
QBCall2ActionContextCreatePopupAndArmTimer(QBCall2ActionContext self)
{
    log_fun();
    svSettingsPushComponent("LoadableDialog.settings");
    self->dialog = QBLoadableDialogCreate(self->appGlobals, NULL, NULL);
    SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(TIMEOUT));
    svSettingsPopComponent();
    QBDialogRun(self->dialog, self, QBCall2ActionContextPopupCallback);
}


SvLocal void
QBCall2ActionContextSetupResultPopup(QBCall2ActionContext self, QBCall2ActionState state,
                                     SvErrorInfo * errorOut)
{
    log_fun();
    SvErrorInfo error = NULL;
    bool success = QBCall2ActionStateIsSuccess(state, &error);
    if (error)
        goto fini;

    SvString title = NULL;
    SvString text = NULL;

    if (success) {
        if (self->popupParams) {
            SVAUTOSTRING(titleStr, "title");
            SvValue titleVal = (SvValue) SvHashTableFind(self->popupParams, (SvObject) titleStr);
            if (titleVal) {
                title = SvValueTryGetString(titleVal);
                if (!title) {
                    error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                              "Popup title should be SvValueType_string");
                    goto fini;
                }
            }
            SVAUTOSTRING(textStr, "text");
            SvValue textVal = (SvValue) SvHashTableFind(self->popupParams, (SvObject) textStr);
            if (textVal) {
                text = SvValueTryGetString(textVal);
                if (!text) {
                    error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                              "Popup text should be SvValueType_String");
                    goto fini;
                }
            }
        }
        QBLoadableDialogSetData(self->dialog, title, text);
    } else {
        text = QBCall2ActionStateGetFailureCause(state, &error);
        if (error)
            goto fini;
        title = SvStringCreate("Failed", NULL);
        QBLoadableDialogSetData(self->dialog, title, text);
        SVRELEASE(title);
    }
fini:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBCall2ActionContextRefreshCategory(QBCall2ActionContext self, QBContentCategory category)
{
    log_fun();
    log_debug("Refreshing category with id:'%s'",
              SvStringCString(QBContentCategoryGetId(category)));
    SvObject provider = QBContentCategoryTakeProvider(category);
    QBContentProviderRefresh((QBContentProvider) provider, category, 0, true, false);
    SVRELEASE(provider);
}

SvLocal void
QBCall2ActionContextRefreshService(QBCall2ActionContext self)
{
    log_fun();
    QBContentCategory tmp = NULL;
    QBContentCategory parentCategory = self->category;
    while ((tmp = QBContentCategoryGetParent(parentCategory)))
        parentCategory = tmp;

    QBCall2ActionContextRefreshCategory(self, parentCategory);
}

SvLocal void
QBCall2ActionContextRefreshAllCategories(QBCall2ActionContext self, SvArray array, SvErrorInfo * errorOut)
{
    log_fun();
    SvObject category = NULL;
    SvString categoryId = NULL;
    SvErrorInfo error = NULL;
    SvValue idVal = NULL;
    SvIterator iter = SvArrayIterator(array);
    while ((idVal = (SvValue) SvIteratorGetNext(&iter))) {
        categoryId = SvValueTryGetString(idVal);
        if (!categoryId) {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                      "Category id should be SvValueType_string");
            goto err;
        }
        log_debug("Found categoryId:'%s'", SvStringCString(categoryId));
        category = QBContentTreeFindCategoryById((QBContentTree) self->contentTree,
                                                 categoryId, &error);
        if (category) {
            QBCall2ActionContextRefreshCategory(self, (QBContentCategory) category);
        } else if (error) {
            goto err;
        } else {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                      "There's no category with given id: %s",
                                      SvStringCString(categoryId));
            goto err;
        }
    }
err:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBCall2ActionContextStateChanged(SvObject self_, QBCall2ActionState state)
{
    log_fun();
    QBCall2ActionContext self = (QBCall2ActionContext) self_;

    SvErrorInfo error = NULL;

    ssize_t callIdx = 0;
    if ((callIdx = SvArrayFindObject(self->callStateStack, (SvObject) state)) == -1)
        return;

    /// If callState is the last call we can change popup
    if (self->needPopup && (callIdx == (ssize_t) SvArrayCount(self->callStateStack) - 1)) {
        self->needPopup = false;
        SvFiberEventDeactivate(self->timer);
        QBCall2ActionContextSetupResultPopup(self, state, &error);
        if (error)
            goto fini;
    }

    SvHashTable refreshTable = (SvHashTable) QBCall2ActionStateGetRefreshTable(state, &error);
    if (!refreshTable)
        goto fini;

    SVAUTOSTRING(actionStr, "actionID");
    SvValue actionId = (SvValue) SvHashTableFind(refreshTable, (SvObject) actionStr);
    if (actionId && SvValueIsString(actionId)) {
        QBCall2ActionProviderRefreshAction(self->provider, actionId, &error);
        if (error)
            goto fini;
    }
    SVAUTOSTRING(serviceStr, "service");
    SvValue serviceVal = (SvValue) SvHashTableFind(refreshTable, (SvObject) serviceStr);
    if (serviceVal && self->category) {
        /// We clear rootCategory
        QBCall2ActionContextRefreshService(self);
    } else {
        bool currentCatRefreshed = false;

        SVAUTOSTRING(productIDs, "productIDs");
        SvArray productIdArray = (SvArray) SvHashTableFind(refreshTable, (SvObject) productIDs);
        /*
         * We can refresh only current product because we know his category id.
         * Product can be find in a few categories. There is no need to refresh all these categories.
         */
        if (productIdArray && self->product && self->category) {
            SvObject productId = (SvObject) SvDBObjectGetID((SvDBObject) self->product);
            if (SvArrayContainsObject(productIdArray, productId)) {
                QBCall2ActionContextRefreshCategory(self, self->category);
                currentCatRefreshed = true;
            }
        }

        SVAUTOSTRING(categoryIDs, "categoryIDs");
        SvArray categoryIdArray = (SvArray) SvHashTableFind(refreshTable, (SvObject) categoryIDs);
        if (categoryIdArray) {
            /// There's no need to refresh current category twice,
            if (currentCatRefreshed && self->category) {
                SvObject currentCatId = (SvObject) QBContentCategoryGetId(self->category);
                SvArrayRemoveObject(categoryIdArray, currentCatId);
            }
            log_debug("Found categoryIDs table to refresh.");
            QBCall2ActionContextRefreshAllCategories(self, categoryIdArray, &error);
        }
    }
fini:
    QBCall2ActionContextRemoveCallStateFromStack(self, state);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
}

SvLocal bool
QBCall2ActionContextOnNewLevel(SvObject self_, SvObject path, SvArray options)
{
    log_fun();
    QBCall2ActionContext self = (QBCall2ActionContext) self_;
    SvObject handler = QBTreePathMapFind(self->handlersPathMap, path);
    if (handler && SvInvokeInterface(QBCall2ActionHandler, handler, onNewLevel, path, options))
        return true;
    return false;
}

SvLocal void
QBCall2ActionContextExecuteAction(QBCall2ActionContext self, SvObject option)
{
    log_fun();
    QBCall2ActionState callState = QBCall2ActionProviderExecuteAction(self->provider,
                                                                      option, NULL, NULL);
    if (callState)
        QBCall2ActionContextAddCallStateToStack(self, callState);
}

SvLocal bool
QBCall2ActionContextResolveActionByType(QBCall2ActionContext self,
                                        QBCall2ActionTreeNodeType type,
                                        SvObject action, SvErrorInfo * errorOut)
{
    log_fun();
    assert(self != NULL);
    assert(action != NULL);

    SvErrorInfo error = NULL;
    switch (type) {
        case QBCall2ActionTreeNodeTypeMultichoice:
            QBCall2ActionContextExecuteAction(self, action);
            break;
        case QBCall2ActionTreeNodeTypeNormal:
            QBCall2ActionContextExecuteAction(self, action);
            QBCall2ActionPanePushLevel(self->pane, NULL);
            break;
        case QBCall2ActionTreeNodeTypeFinal:
            QBCall2ActionContextExecuteAction(self, action);
            QBCall2ActionContextCreatePopupAndArmTimer(self);
            break;
        case QBCall2ActionTreeNodeTypeFinalAsynchronous:
            QBCall2ActionProviderExecuteAction(self->provider, action, NULL, NULL);
            QBCall2ActionPanePopAllLevels(self->pane, NULL);
            break;
        case QBCall2ActionTreeNodeTypeInvalid:
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                      "Unknown action type.");
            SvErrorInfoPropagate(error, errorOut);
            return false;
    }
    return true;
}

SvLocal bool
QBCall2ActionContextOnOptionSelected(SvObject self_, SvObject path, SvObject option)
{
    log_fun();
    assert(self_ != NULL);
    assert(path != NULL);
    assert(option != NULL);

    SvErrorInfo error = NULL;
    if (!SvObjectIsInstanceOf(option, QBCall2ActionTreeNode_getType())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Parameter 'option' should have QBCall2ActionTreeNode type.");
        goto err;
    }

    QBCall2ActionContext self = (QBCall2ActionContext) self_;
    SvObject handler = QBTreePathMapFind(self->handlersPathMap, path);
    if (handler &&
        SvInvokeInterface(QBCall2ActionHandler, handler, onOptionSelected, path, option))
        return true;

    log_info("No handlers found. Executing default flow.");

    SvHashTable response = (SvHashTable) QBCall2ActionTreeNodeGetAttrValue((QBCall2ActionTreeNode) option,
                                                                           "response", NULL);

    if (!response || !SvObjectIsInstanceOf((SvObject) response, SvHashTable_getType())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Action response should have SvHashTable type.");
        goto err;
    }

    SVTESTRELEASE(self->popupParams);
    self->needPopup = false;
    SVAUTOSTRING(popup, "popup");
    if ((self->popupParams = (SvHashTable) SvHashTableFind(response, (SvObject) popup))) {
        if (!SvObjectIsInstanceOf((SvObject) self->popupParams, SvHashTable_getType())) {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                      "Popup table should have SvHashTable type.");
            goto err;
        }
        SVRETAIN(self->popupParams);
        self->needPopup = true;
    }

    QBCall2ActionTreeNodeType type = QBCall2ActionTreeNodeGetType((QBCall2ActionTreeNode) option, NULL);
    if (QBCall2ActionContextResolveActionByType(self, type, option, &error))
        return true;
err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    return false;
}

SvLocal SvType
QBCall2ActionContext_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBCall2ActionContextDestroy
    };

    static const struct QBCall2ActionStateListener_ stateMethods = {
        .stateChanged = QBCall2ActionContextStateChanged
    };

    static const struct QBCall2ActionHandler_ methods = {
        .onNewLevel       = QBCall2ActionContextOnNewLevel,
        .onOptionSelected = QBCall2ActionContextOnOptionSelected,
    };

    if (!type) {
        SvTypeCreateManaged("QBCall2ActionContext", sizeof(struct QBCall2ActionContext_),
                            SvObject_getType(), &type, SvObject_getType(), &vtable,
                            QBCall2ActionStateListener_getInterface(), &stateMethods,
                            QBCall2ActionHandler_getInterface(), &methods, NULL);
    }
    return type;
}

SvLocal bool
QBCall2ActionContextPaneInputEventHandler(void * self_, SvObject src, SvInputEvent e)
{
    log_fun();
    QBCall2ActionContext self = (QBCall2ActionContext) self_;

    if ((e->ch == QBKEY_LEFT || e->ch == QBKEY_BACK) && QBCall2ActionPanePopLevel(self->pane, NULL))
        return true;

    return false;
}

bool
QBCall2ActionContextAddOption(QBCall2ActionContext self, SvString id, SvString caption, SvErrorInfo * errorOut)
{
    log_fun();
    SvErrorInfo error = NULL;
    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed. (self=%p)", self);
        goto err;
    }
    if (QBCall2ActionPaneAddOption(self->pane, id, caption, &error))
        return true;
err:
    SvErrorInfoPropagate(error, errorOut);
    return false;
}

bool
QBCall2ActionContextAddAction(QBCall2ActionContext self, QBCall2ActionTreeNode action,
                              QBBasicPaneItemCallback callback, SvObject callbackData,
                              SvErrorInfo * errorOut)
{
    log_fun();
    SvErrorInfo error = NULL;
    if (!self || !action || !callbackData) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed. (self=%p, action=%p, callbackData=%p)",
                                  self, action, callbackData);
        goto err;
    }
    if (QBCall2ActionPaneAddAction(self->pane, action, callback, callbackData, &error))
        return true;
err:
    SvErrorInfoPropagate(error, errorOut);
    return false;
}

bool
QBCall2ActionContextSetItemController(QBCall2ActionContext self, SvObject controller,
                                      SvObject path, SvErrorInfo * errorOut)
{
    log_fun();
    SvErrorInfo error = NULL;
    if (!self || !path) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed. (self=%p, path=%p)", self, path);
        goto err;
    } else if (SvObjectIsInstanceOf(path, QBCall2ActionTreePath_getType())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Path has incorrect type");
        goto err;
    }

    if (QBCall2ActionPaneSetItemController(self->pane, path, controller, &error))
        return true;
err:
    SvErrorInfoPropagate(error, errorOut);
    return false;
}

bool
QBCall2ActionContextSetHandler(QBCall2ActionContext self, SvObject handler,
                               SvObject path, SvErrorInfo * errorOut)
{
    log_fun();
    SvErrorInfo error = NULL;
    if (!self || !path) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed.(self=%p, path=%p)", self, path);
        goto err;
    } else if (SvObjectIsInstanceOf(path, QBCall2ActionTreePath_getType())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Path has incorrect type");
        goto err;
    }

    if (handler) {
        QBTreePathMapInsert(self->handlersPathMap, path, handler, NULL);
    } else {
        if (!QBTreePathMapFind(self->handlersPathMap, path))
            return false;
        QBTreePathMapRemove(self->handlersPathMap, path, NULL);
    }
    return true;
err:
    SvErrorInfoPropagate(error, errorOut);
    return false;
}

SvLocal SvString
QBCall2ActionContextInitActionDataFromNode(QBCall2ActionContext self, SvObject node,
                                           SvErrorInfo * errorOut)
{
    log_fun();
    assert(self != NULL);

    SvErrorInfo error = NULL;
    SvString id = NULL;
    if (node && SvObjectIsInstanceOf(node, SvDBRawObject_getType())) {
        log_info("Found object with type: SvDBRawObject_getType");
        self->product = SVRETAIN(node);
        SvValue idValue = SvDBObjectGetID((SvDBObject) node);
        if (idValue && SvValueIsString(idValue)) {
            id = SvValueGetString(idValue);
        } else {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                      "Product id should be SvValueType_string");
            goto err;
        }
        SvObject tmpPath = SvObjectCopy(self->path, NULL);
        SvInvokeInterface(QBTreePath, tmpPath, truncate, -1);
        SvObject parent = SvInvokeInterface(QBTreeModel, (SvObject) self->appGlobals->menuTree,
                                            getNode, tmpPath);
        SVRELEASE(tmpPath);
        if (SvObjectIsInstanceOf(parent, QBContentCategory_getType())) {
            self->category = (QBContentCategory) SVRETAIN(parent);
        } else {
            log_debug("Product parent should have QBContentCategory type.");
        }
    } else if (SvObjectIsInstanceOf(node, QBContentCategory_getType())) {
        log_info("Found object with type: QBContentCategory_getType");
        id = QBContentCategoryGetId((QBContentCategory) node);
        self->category = (QBContentCategory) SVRETAIN(node);
    } else {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Node has incorrect type.");
        goto err;
    }
    return id;
err:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

SvLocal void
QBCall2ActionContextSetPaneItemController(QBCall2ActionContext self, QBCall2ActionTree tree,
                                          SvErrorInfo * errorOut)
{
    log_fun();
    assert(self != NULL);
    assert(tree != NULL);

    SvErrorInfo error = NULL;

    QBCall2ActionTreeNode rootNode = QBCall2ActionTreeGetRootNode(tree, &error);
    if (error)
        goto err;
    SvValue rootId = QBCall2ActionTreeNodeGetId(rootNode, &error);
    if (error)
        goto err;

    QBCall2ActionTreePath npath = NULL;
    if (!QBCall2ActionTreeFindNodeById(tree, rootId, &npath, &error))
        goto err;
    if (!QBCall2ActionPaneSetItemController(self->pane, (SvObject) npath, self->itemController, &error))
        goto err;
    SVRELEASE(npath);
err:
    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void
QBCall2ActionContextDialogTimeOutStep(void *self_)
{
    log_fun();
    QBCall2ActionContext self = self_;
    if (!self->dialog)
        return;

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    QBLoadableDialogSetData(self->dialog, SVSTRING("Aborted"), SVSTRING("Time out expired"));
    QBCall2ActionState callState = (QBCall2ActionState) SvArrayLastObject(self->callStateStack);
    if (callState)
        QBCall2ActionContextRemoveCallStateFromStack(self, callState);
}

SvLocal void
QBCall2ActionContextTimeOutCallback(void * self_, QBCall2ActionPane pane)
{
    log_fun();
    QBCall2ActionContext self = self_;
    QBDialogParameters params = {
        .app        = self->appGlobals->res,
        .controller = self->appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0
    };
    self->dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    QBDialogSetTitle(self->dialog, "Time out expired");
    SvWidget button = QBDialogAddButton(self->dialog, SVSTRING("OK-button"), "Hide", 1);
    svWidgetSetFocus(button);
    QBDialogRun(self->dialog, self, QBCall2ActionContextPopupCallback);
}

QBCall2ActionContext
QBCall2ActionContextCreate(AppGlobals appGlobals, SvObject contentTree, SvObject path,
                           SvString serviceId, SvObject serverInfo, SvErrorInfo * errorOut)
{
    log_fun();
    SvErrorInfo error = NULL;
    QBCall2ActionContext self = NULL;
    if (!appGlobals || !path || !contentTree || !serverInfo) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "NULL argument passed. "
                                  "(appGlobals=%p, path=%p, contentTree=%p, serverInfo=%p)",
                                  appGlobals, path, contentTree, serverInfo);
        goto err;
    }

    if (!(self = (QBCall2ActionContext) SvTypeAllocateInstance(QBCall2ActionContext_getType(), &error)))
        goto err;

    self->path = SVRETAIN(path);
    self->appGlobals = appGlobals;
    self->contentTree = SVRETAIN(contentTree);
    self->handlersPathMap = QBTreePathMapCreate(&error);
    self->callStateStack = SvArrayCreate(NULL);
    self->fiber = SvFiberCreate(appGlobals->scheduler, NULL, "QBCall2ActionContextStep",
                                QBCall2ActionContextDialogTimeOutStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    SvObject node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, self->path);

    SvString id = QBCall2ActionContextInitActionDataFromNode(self, node, &error);
    if (!id)
        goto err;

    struct QBCall2ActionProviderParams_ params = {
        .serviceId  = serviceId,
        .productId  = id,
        .serverInfo = serverInfo
    };
    if (!(self->provider = QBCall2ActionProviderCreate(self->appGlobals->middlewareManager, &params, &error)))
        goto err;

    QBCall2ActionTree tree = QBCall2ActionProviderGetTree(self->provider);

    self->ctxMenu = QBContextMenuCreateFromSettings("ContextMenu.settings", self->appGlobals->controller,
                                                    self->appGlobals->res, SVSTRING("SideMenu"));
    QBContextMenuSetInputEventHandler(self->ctxMenu, self, QBCall2ActionContextPaneInputEventHandler);

    svSettingsPushComponent("LoadableBasicPane.settings");

    self->itemController = QBCall2ActionItemControllerCreate(tree, self->appGlobals, SVSTRING("BasicPane"));
    if (!(self->pane = QBCall2ActionPaneCreate(self->provider, self->appGlobals, self->ctxMenu,
                                               TIMEOUT, &error)))
        goto err;

    if (!QBCall2ActionPaneSetTimeOutCallback(self->pane, QBCall2ActionContextTimeOutCallback,
                                             (SvObject) self, &error))
        goto err;

    if (!QBCall2ActionPaneSetController(self->pane, (SvObject) self, &error))
        goto err;

    svSettingsPopComponent();

    QBCall2ActionContextSetPaneItemController(self, tree, &error);

    return self;
err:
    SVTESTRELEASE(self);
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

bool
QBCall2ActionContextStart(QBCall2ActionContext self, SvErrorInfo * errorOut)
{
    log_fun();
    SvErrorInfo error = NULL;
    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "NULL argument passed.");
        goto err;
    }

    if (self->started) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Context is already started.");
        goto err;
    }

    if (!QBCall2ActionProviderStart(self->provider, &error))
        goto err;

    if (!QBCall2ActionPanePushLevel(self->pane, &error))
        goto err;

    QBContextMenuShow(self->ctxMenu);

    self->started = true;

    return true;
err:
    SvErrorInfoPropagate(error, errorOut);
    return false;
}

bool
QBCall2ActionContextStop(QBCall2ActionContext self, SvErrorInfo * errorOut)
{
    log_fun();
    SvErrorInfo error = NULL;
    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "NULL argument passed.");
        goto err;
    }

    if (!self->started) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Can't stop context. It wasn't started already.");
        goto err;
    }

    if (!QBCall2ActionProviderStop(self->provider, &error))
        goto err;

    SvIterator iter = SvArrayIterator(self->callStateStack);
    QBCall2ActionState state = NULL;
    while ((state = (QBCall2ActionState) SvIteratorGetNext(&iter)))
        QBCall2ActionContextRemoveCallStateFromStack(self, state);

    if (!QBCall2ActionPanePopAllLevels(self->pane, &error))
        goto err;

    QBContextMenuHide(self->ctxMenu, true);

    if (self->dialog)
        QBDialogHide(self->dialog, true, true);

    self->started = false;

    return true;
err:
    SvErrorInfoPropagate(error, errorOut);
    return false;
}

bool
QBCall2ActionContextSetOnCloseCallback(QBCall2ActionContext self, void * ptr,
                                       QBCall2ActionContextOnCloseCallback callback, SvErrorInfo * errorOut)
{
    log_fun();
    SvErrorInfo error = NULL;
    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "NULL argument passed.");
        goto err;
    }

    self->callbackData = ptr;
    self->callback = callback;
    return true;
err:
    SvErrorInfoPropagate(error, errorOut);
    return false;
}
