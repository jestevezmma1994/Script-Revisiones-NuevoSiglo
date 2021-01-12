/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBEnvMenuHandler.h"

#include <string.h>

#include <SvCore/SvEnv.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvValue.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <QBMenu/QBMenu.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <main.h>


struct QBEnvMenuHandler_s
{
    struct SvObject_ super_;

    AppGlobals  appGlobals;
    QBActiveTree  mainTree;
    SvWidget  menuBar;

    SvGenericObject  path;
    QBActiveTreeNode  root_node;
};
typedef struct QBEnvMenuHandler_s* QBEnvMenuHandler;

struct QBEnvMenuData_s {
    struct SvObject_ super_;
    int idx;
    int initial;
    int currentVal;
    int selected;       // used only for next value selection on ENTER key
};
typedef struct QBEnvMenuData_s* QBEnvMenuData;

#define MAX_NODE_NAME_LEN       30
#define MAX_NODE_SUBCAP_LEN     50

SvLocal SvType QBEnvMenuData_getType(void)
{
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBEnvMenuData",
                        sizeof(struct QBEnvMenuData_s),
                        SvObject_getType(), &type,
                        NULL);
    }

    return type;
}

SvLocal SvString prepareSubcaption(QBEnvMenuHandler self,
                                   const SvEnvStruct* envData,
                                   int newVal,
                                   int initial)
{
    char val[MAX_NODE_SUBCAP_LEN];
    snprintf(val, sizeof(val), "%d [", newVal);
    int idx = strlen(val);

    if (envData->valType == SvEnv_Val_Type_NONE) {
        if (initial == envData->defaultVal) {
            if (initial == 0)
                snprintf(&val[idx], sizeof(val) - idx, "0 , 1]");
            else
                snprintf(&val[idx], sizeof(val) - idx, "0 , %d]", initial);
        } else if (envData->defaultVal == 0) {
            snprintf(&val[idx], sizeof(val) - idx, "0 , %d]", initial);
        } else {
            snprintf(&val[idx], sizeof(val) - idx, "0 , %d, %d]", envData->defaultVal, initial);
        }
    } else if (envData->valType == SvEnv_Val_Type_DEFAULTS) {
        int x;
        for (x = 0; x < envData->valTabSize - 1; x++) {
            snprintf(&val[idx], sizeof(val) - idx, "%d, ", envData->valTab[x]);
            idx = strlen(val);
        }
        snprintf(&val[idx], sizeof(val) - idx, "%d]", envData->valTab[x]);
    } else if (envData->valType == SvEnv_Val_Type_MIN_MAX) {
        snprintf(&val[idx], sizeof(val) - idx, "%d...%d]", envData->valTab[0], envData->valTab[1]);
    }

    return SvStringCreate(val, NULL);
}

SvLocal void propagateChanges(QBEnvMenuHandler self)
{
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, self->path, 0);
    size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
    QBActiveTreePropagateNodesChange(self->mainTree, self->path, 0, nodesCount, NULL);
}

SvLocal void updateEnvNode(QBEnvMenuHandler self,
                           QBActiveTreeNode node,
                           const SvEnvStruct* envData,
                           int idx,
                           int newVal,
                           int initial)
{
    SvEnvSet(idx, newVal);

    SvString valString = prepareSubcaption(self, envData, newVal, initial);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvGenericObject)valString);
    SVRELEASE(valString);

    propagateChanges(self);
}

SvLocal void shortModuleName(const char* src, char* name, int len)
{
    const char* ptr = strrchr(src, '/');
    if (!ptr)
        ptr = src;
    else
        ptr++;

    name[0] = ' ';      // to designate module name in the nodes IDs list
    len--;

    int idx=1;
    len--;
    while (*ptr != 0 && *ptr !='.') {
        name[idx++] = *ptr;
        if (idx >= len)
            break;
        ptr++;
    }
    name[idx] = 0;
}

SvLocal QBActiveTreeNode findNodeByName(QBEnvMenuHandler self, QBActiveTreeNode startNode, SvString name, int* idx)
{
    SvIterator iter = QBActiveTreeNodeChildNodesIterator(startNode);
    QBActiveTreeNode node = NULL;
    if (idx)
        *idx = 0;
    while ((node = (QBActiveTreeNode) SvIteratorGetNext(&iter))) {
        SvString currModuleName = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
        int cmpRes = strcasecmp(SvStringCString(currModuleName), SvStringCString(name));
        if (cmpRes == 0)
            return node;
        if (cmpRes > 0)     // nodes are sorted
            return NULL;

        if (idx)
            *idx += 1;
    }

    return NULL;
}

SvLocal QBActiveTreeNode insertNewModuleNode(QBEnvMenuHandler self, SvString name)
{
    QBActiveTreeNode moduleNode = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBActiveTreeNodeSetAttribute(moduleNode, SVSTRING("caption"), (SvGenericObject)name);

    int idx;
    findNodeByName(self, self->root_node, name, &idx);

    QBActiveTreeNodeInsertChildNode(self->root_node, moduleNode, idx, NULL);
    SVRELEASE(moduleNode);

    return moduleNode;
}

SvLocal void updateSelectedIdx(QBEnvMenuData data, SvEnvStruct* envData)
{
    data->selected = 0;

    // for DEFAULTS type find current val in the defaults array
    if (envData->valType == SvEnv_Val_Type_DEFAULTS) {
        for (; data->selected < envData->valTabSize; data->selected++)
            if (envData->valTab[data->selected] == envData->val)
                break;
    }
}

SvLocal void QBEnvMenuHandlerUpdateData(QBEnvMenuHandler self)
{
    bool dataChanged = false;

    SvIterator iter = QBActiveTreeNodeChildNodesIterator(self->root_node);
    QBActiveTreeNode node = NULL;

    // iterate modules
    while ((node = (QBActiveTreeNode) SvIteratorGetNext(&iter))) {
        SvIterator childIter = QBActiveTreeNodeChildNodesIterator(node);
        QBActiveTreeNode child = NULL;

        // iterate envs
        while ((child = (QBActiveTreeNode) SvIteratorGetNext(&childIter))) {
            QBEnvMenuData nodeData = (QBEnvMenuData) QBActiveTreeNodeGetAttribute(child, SVSTRING("nodeData"));

            SvEnvStruct* envData = SvEnvGetEnvData(nodeData->idx);

            if (envData->val != nodeData->currentVal) {
                dataChanged = true;
                SvString valStr = prepareSubcaption(self, envData, envData->val, nodeData->initial);
                QBActiveTreeNodeSetAttribute(child, SVSTRING("subcaption"), (SvGenericObject)valStr);
                SVRELEASE(valStr);

                nodeData->currentVal = envData->val;
                updateSelectedIdx(nodeData, envData);
            }
        }
    }

    if (dataChanged)
        propagateChanges(self);
}

SvLocal void QBEnvMenuHandlerFill(QBEnvMenuHandler self)
{
    int x;
    for (x = 0; x < SvEnvGetCount(); x++) {
        SvEnvStruct* envData = SvEnvGetEnvData(x);

        char moduleName[MAX_NODE_NAME_LEN];
        shortModuleName(envData->module, moduleName, sizeof(moduleName));
        SvString caption = SvStringCreate(moduleName, NULL);

        QBActiveTreeNode moduleNode = findNodeByName(self, self->root_node, caption, NULL);

        if (!moduleNode)
            moduleNode = insertNewModuleNode(self, caption);

        SVRELEASE(caption);

        SvString name = SvStringCreate(envData->name, NULL);
        SvString valStr = prepareSubcaption(self, envData, envData->val, envData->val);

        QBEnvMenuData nodeData = (QBEnvMenuData) SvTypeAllocateInstance(QBEnvMenuData_getType(), NULL);
        nodeData->idx = envData->idx;
        nodeData->initial = envData->val;
        nodeData->currentVal = envData->val;
        updateSelectedIdx(nodeData, envData);

        QBActiveTreeNode node = QBActiveTreeNodeCreate(NULL, NULL, NULL);
        QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvGenericObject)name);
        QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvGenericObject)valStr);
        QBActiveTreeNodeSetAttribute(node, SVSTRING("nodeData"), (SvGenericObject)nodeData);

        int idx;
        findNodeByName(self, moduleNode, name, &idx);
        QBActiveTreeNodeInsertChildNode(moduleNode, node, idx, NULL);

        SVRELEASE(nodeData);
        SVRELEASE(name);
        SVRELEASE(valStr);
        SVRELEASE(node);
    }
}

SvLocal bool QBEnvMenuSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_)
{
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return false;
    QBEnvMenuHandler self = (QBEnvMenuHandler) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    if (node == self->root_node) {
        QBEnvMenuHandlerUpdateData(self);
    } else if (QBActiveTreeNodeGetChildNodesCount(node) == 0) {
        QBEnvMenuData nodeData = (QBEnvMenuData) QBActiveTreeNodeGetAttribute(node, SVSTRING("nodeData"));
        SvEnvStruct* envData = SvEnvGetEnvData(nodeData->idx);

        if (envData->valType == SvEnv_Val_Type_NONE) {
            int newVal = 0;

            if (envData->val == nodeData->initial) {
                if (envData->defaultVal == 0)
                    newVal = 1;
                else
                    newVal = 0;
            } else if (envData->val == 0) {
                newVal = envData->defaultVal;
            } else {
                newVal = nodeData->initial;
            }

            updateEnvNode(self, node, envData, nodeData->idx, newVal, nodeData->initial);
            return true;
        } else if (envData->valType == SvEnv_Val_Type_DEFAULTS) {
            nodeData->selected++;
            if (nodeData->selected >= envData->valTabSize)
                nodeData->selected = 0;

            updateEnvNode(self, node, envData, nodeData->idx, envData->valTab[nodeData->selected], 0);
            return true;
        } else if (envData->valType == SvEnv_Val_Type_MIN_MAX) {
            int min = envData->valTab[0];
            int max = envData->valTab[1];

            envData->val++;
            if (envData->val > max)
                envData->val = min;

            updateEnvNode(self, node, envData, nodeData->idx, envData->val, 0);
            return true;
        }
    }

    return false;
}

SvLocal bool init_menu_data(QBEnvMenuHandler self, SvString id, AppGlobals appGlobals, QBTreePathMap pathMap)
{
    QBActiveTreeNode node = QBActiveTreeFindNode(appGlobals->menuTree, id);
    SvObject path = node ? QBActiveTreeCreateNodePath(appGlobals->menuTree, id) : NULL;
    if (!node || !path)
        return false;

    self->root_node = SVRETAIN(node);
    self->path = path;

    QBTreePathMapInsert(pathMap, path, (SvObject) self, NULL);
    SvInvokeInterface(QBMenu, self->menuBar->prv, setEventHandlerForPath, path, (SvObject) self, NULL);
    return true;
}

SvLocal void QBEnvMenuHandlerDestroy(void *self_)
{
    QBEnvMenuHandler self = self_;

    SVTESTRELEASE(self->path);
    SVTESTRELEASE(self->root_node);
}

SvLocal SvType QBEnvMenuHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBEnvMenuHandlerDestroy
    };
    static SvType type = NULL;
    static const struct QBMenuEventHandler_ selectMethods = {
            .nodeSelected = QBEnvMenuSelected
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBEnvMenuHandler",
                            sizeof(struct QBEnvMenuHandler_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &selectMethods,
                        NULL);
    }

    return type;
}

void QBEnvMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    QBEnvMenuHandler self = (QBEnvMenuHandler) SvTypeAllocateInstance(QBEnvMenuHandler_getType(), NULL);
    self->appGlobals = appGlobals;
    self->mainTree = appGlobals->menuTree;
    self->menuBar = menuBar;

    bool needed = false;
    needed |= init_menu_data(self, SVSTRING("Envs"), appGlobals, pathMap);

    if (needed)
        QBEnvMenuHandlerFill(self);

    // "self" was inserted in the QBTreePathMap so we can release it
    SVRELEASE(self);
}
