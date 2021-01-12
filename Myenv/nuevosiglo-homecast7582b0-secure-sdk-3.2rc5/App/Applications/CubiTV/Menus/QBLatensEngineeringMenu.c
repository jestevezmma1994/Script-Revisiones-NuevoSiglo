/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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
#include "QBLatensEngineeringMenu.h"

#include <QBCAS.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <QBLatens.h>
#include <QBLatensInfo.h>
#include <main.h>
#include <SvFoundation/SvInterface.h>

#define log_state(fmt, ...)  do { SvLogNotice(COLBEG() "QBLatensEngineeringMenu :: %s " fmt COLEND_COL(cyan), __func__, ## __VA_ARGS__); } while (0)
#define log_error(fmt, ...)  do { SvLogError(COLBEG() "QBLatensEngineeringMenu :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)

//----------------QBLatensEngineeringMenu-------------
typedef struct QBLatensEngineeringMenu_s* QBLatensEngineeringMenu;

struct QBLatensEngineeringMenu_s {
    struct SvObject_ super;
    AppGlobals appGlobals;
    QBActiveTree mainTree;
    SvWidget menuBar;

    QBActiveTreeNode CASVersion;
    QBActiveTreeNode LatensSSM;
    QBActiveTreeNode LatensSTBID;
    QBActiveTreeNode LatensCheckCode;
    QBActiveTreeNode LatensVCID;
    QBActiveTreeNode LatensBootLoader;
};

SvLocal bool QBLatensEngineeringMenuSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_);
SvLocal void QBLatensEngineeringMenuUpdate(QBLatensEngineeringMenu self);
SvLocal void QBLatensEngineeringMenuDestroy(void * self_);

SvLocal void QBLatensEngineeringMenuSmartcardCallback(void* target, QBCASSmartcardState state);

static const struct QBCASCallbacks_s casCallbacks = {
    .smartcard_state = &QBLatensEngineeringMenuSmartcardCallback,
};

SvLocal SvType QBLatensEngineeringMenu_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBLatensEngineeringMenuDestroy
    };

    static const struct QBMenuEventHandler_ selectMethods = {
        .nodeSelected = QBLatensEngineeringMenuSelected
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBLatensEngineeringMenu", sizeof(struct QBLatensEngineeringMenu_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            QBMenuEventHandler_getInterface(), &selectMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBLatensEngineeringMenuDestroy(void * self_)
{
    QBLatensEngineeringMenu self = (QBLatensEngineeringMenu) self_;
    SvInvokeInterface(QBCAS, QBCASGetInstance(), removeCallbacks, (QBCASCallbacks) & casCallbacks, self);
}

SvLocal QBLatensEngineeringMenu QBLatensEngineeringMenuCreate(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBLatensEngineeringMenu self = (QBLatensEngineeringMenu) SvTypeAllocateInstance(QBLatensEngineeringMenu_getType(), &error);
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

SvLocal bool QBLatensEngineeringMenuSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_)
{
    QBLatensEngineeringMenu self = (QBLatensEngineeringMenu) self_;
    QBLatensEngineeringMenuUpdate(self);
    return false;
}

SvLocal void QBLatensEngineeringMenuSmartcardCallback(void* target, QBCASSmartcardState state)
{
    QBLatensEngineeringMenu self = (QBLatensEngineeringMenu) target;
    QBLatensEngineeringMenuUpdate(self);
}

SvLocal void QBLatensEngineeringMenuRefresh(QBLatensEngineeringMenu self)
{
    SvObject parentPath = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, SVSTRING("LatensEngineering"));
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->mainTree, getIterator, parentPath, 0);
    size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
    QBActiveTreePropagateNodesChange(self->mainTree, parentPath, 0, nodesCount, NULL);
    SVRELEASE(parentPath);
}

SvLocal void QBLatensEngineeringMenuCallbackInfo(void* self_, const QBCASCmd cmd, int status, QBCASInfo info)
{
    QBLatensEngineeringMenu self = (QBLatensEngineeringMenu) self_;

    QBLatensInfo latensInfo = (QBLatensInfo) info;
    QBActiveTreeNodeSetAttribute(self->CASVersion, SVSTRING("subcaption"), (SvGenericObject) latensInfo->CASVersion);
    QBActiveTreeNodeSetAttribute(self->LatensSSM, SVSTRING("subcaption"), (SvGenericObject) latensInfo->LatensSSM);
    QBActiveTreeNodeSetAttribute(self->LatensSTBID, SVSTRING("subcaption"), (SvGenericObject) latensInfo->LatensSTBID);
    QBActiveTreeNodeSetAttribute(self->LatensCheckCode, SVSTRING("subcaption"), (SvGenericObject) latensInfo->LatensCheckCode);
    QBActiveTreeNodeSetAttribute(self->LatensVCID, SVSTRING("subcaption"), (SvGenericObject) latensInfo->LatensVCID);
    QBActiveTreeNodeSetAttribute(self->LatensBootLoader, SVSTRING("subcaption"), (SvGenericObject) latensInfo->LatensBootLoader);

    QBLatensEngineeringMenuRefresh(self);
}

SvLocal void QBLatensEngineeringMenuUpdate(QBLatensEngineeringMenu self)
{
    SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, &QBLatensEngineeringMenuCallbackInfo, self, NULL);
}

void QBLatensEngineeringMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    QBActiveTreeNode node = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("LatensEngineering"));
    SvObject path = node ? QBActiveTreeCreateNodePath(appGlobals->menuTree, QBActiveTreeNodeGetID(node)) : NULL;

    if (!SvObjectIsInstanceOf(QBCASGetInstance(), QBLatens_getType()) && node) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
        log_state("Latens CAS not present, menu disabled");
        SVTESTRELEASE(path);
        return;
    }

    if (!path)
        return;

    QBLatensEngineeringMenu self = QBLatensEngineeringMenuCreate(NULL);
    self->appGlobals = appGlobals;
    self->mainTree = appGlobals->menuTree;
    self->menuBar = menuBar;

    self->CASVersion = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("LatensCASVersion"));
    self->LatensSSM = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("LatensSSM"));
    self->LatensSTBID = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("LatensSTBID"));
    self->LatensCheckCode = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("LatensCheckCode"));
    self->LatensVCID = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("LatensVCID"));
    self->LatensBootLoader = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("LatensBootLoader"));

    QBTreePathMapInsert(pathMap, path, (SvGenericObject) self, NULL);

    SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, path, (SvObject) self, NULL);
    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks) & casCallbacks, self, "QBLatensEngineeringMenu");
    QBLatensEngineeringMenuUpdate(self);

    SVRELEASE(self);
    SVRELEASE(path);
}
