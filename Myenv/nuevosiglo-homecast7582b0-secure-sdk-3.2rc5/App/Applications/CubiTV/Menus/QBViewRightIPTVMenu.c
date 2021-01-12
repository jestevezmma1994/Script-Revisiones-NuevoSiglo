/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include "QBViewRightIPTVMenu.h"
#include <SvFoundation/SvObject.h>
#include <main.h>
#include <QBCAS.h>
#include <QBViewRightIPTVInfo.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <libintl.h>

#define log_fun(fmt, ...) if (0) { SvLogNotice(COLBEG() "QBViewRightIPTVMenu :: %s " fmt COLEND_COL(green), __func__, ##__VA_ARGS__); }
#define log_state(fmt, ...) if (1) { SvLogNotice(COLBEG() "QBViewRightIPTVMenu :: %s " fmt COLEND_COL(blue), __func__, ##__VA_ARGS__); }
#define log_info(fmt, ...) if (1) { SvLogNotice("QBViewRightIPTVMenu :: %s " fmt, __func__, ##__VA_ARGS__); }
#define log_error(fmt, ...) if (1) { SvLogError(COLBEG() "QBViewRightIPTVMenu :: %s " fmt COLEND_COL(red), __func__, ##__VA_ARGS__); }

//----------------QBViewRightIPTVMenu-------------

typedef struct QBViewRightIPTVMenu_s* QBViewRightIPTVMenu;

struct QBViewRightIPTVMenu_s {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBActiveTree  mainTree;
    SvWidget  menuBar;
    QBCASSmartcardState smardCardState;

    QBActiveTreeNode caSysId;
    QBActiveTreeNode uniqueId;
    QBActiveTreeNode libVersion;
    QBActiveTreeNode connectivity;
};

SvLocal void QBViewRightIPTVMenuUpdate(QBViewRightIPTVMenu self);
SvLocal void QBViewRightIPTVMenuDestroy(void *self_);

SvLocal void QBViewRightIPTVMenuSmartcardCallback(void* target, QBCASSmartcardState state);

static const struct QBCASCallbacks_s casCallbacks = {
  .smartcard_state  = &QBViewRightIPTVMenuSmartcardCallback,
};

SvLocal SvType QBViewRightIPTVMenu_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBViewRightIPTVMenuDestroy
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBViewRightIPTVMenu",
                            sizeof(struct QBViewRightIPTVMenu_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void QBViewRightIPTVMenuDestroy(void *self_)
{
    log_fun();
    QBViewRightIPTVMenu self = (QBViewRightIPTVMenu) self_;
    SvInvokeInterface(QBCAS, QBCASGetInstance(), removeCallbacks, (QBCASCallbacks)&casCallbacks, self);
}

SvLocal void QBViewRightIPTVMenuSmartcardCallback(void* target, QBCASSmartcardState state)
{
    log_fun();
    QBViewRightIPTVMenu self = (QBViewRightIPTVMenu) target;
    self->smardCardState = state;
    QBViewRightIPTVMenuUpdate(self);
}

SvLocal QBViewRightIPTVMenu QBViewRightIPTVMenuCreate(SvErrorInfo *errorOut)
{
    log_fun();
    SvErrorInfo error = NULL;
    QBViewRightIPTVMenu self = (QBViewRightIPTVMenu) SvTypeAllocateInstance(QBViewRightIPTVMenu_getType(), &error);

    SvErrorInfoPropagate(error, errorOut);
    return self;
}

SvLocal void QBViewRightIPTVMenuRefresh(QBViewRightIPTVMenu self)
{
    log_fun();
    SvObject parentPath = NULL;
    QBActiveTreeFindNodeByID(self->appGlobals->menuTree, SVSTRING("ViewRightIPTV"), &parentPath);
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->mainTree, getIterator, parentPath, 0);
    size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
    QBActiveTreePropagateNodesChange(self->mainTree, parentPath, 0, nodesCount, NULL);
}

SvLocal void QBViewRightIPTVMenuCallbackInfo(void* self_, const QBCASCmd cmd, int status, QBCASInfo info)
{
    log_fun();
    QBViewRightIPTVMenu self = (QBViewRightIPTVMenu) self_;
    QBViewRightIPTVInfo viewRightInfo = (QBViewRightIPTVInfo) info;

    SvString connectivity = NULL;

    switch(viewRightInfo->state) {
    case QBViewRightIPTVState_Created:
    case QBViewRightIPTVState_Initialization:
        connectivity = SvStringCreate(gettext("Connecting..."), NULL);
        break;
    case QBViewRightIPTVState_Online:
        connectivity = SvStringCreate(gettext("Connected"), NULL);
        break;
    case QBViewRightIPTVState_Offline:
        connectivity = SvStringCreate(gettext("Offline"), NULL);
        break;
    case QBViewRightIPTVState_Closed:
        connectivity = SvStringCreateWithFormat(gettext("Inactive"));
        break;
    case QBViewRightIPTVState_Error:
        connectivity = SvStringCreateWithFormat(gettext("Error: %d"), viewRightInfo->errorCode);
        break;
    default:
        connectivity = SvStringCreateWithFormat(gettext("Error: %d"), viewRightInfo->errorCode);
        break;
    }

    QBActiveTreeNodeSetAttribute(self->connectivity, SVSTRING("subcaption"), (SvObject) connectivity);
    QBActiveTreeNodeSetAttribute(self->uniqueId, SVSTRING("subcaption"), (SvObject) info->card_number_complete);
    QBActiveTreeNodeSetAttribute(self->libVersion, SVSTRING("subcaption"), (SvObject) info->sw_ver);
    SvString caSysId = SvStringCreateWithFormat("0x%X", info->ca_sys_id);
    QBActiveTreeNodeSetAttribute(self->caSysId, SVSTRING("subcaption"), (SvObject) caSysId);
    SVRELEASE(caSysId);
    SVRELEASE(connectivity);

    QBViewRightIPTVMenuRefresh(self);
}

SvLocal void QBViewRightIPTVMenuUpdate(QBViewRightIPTVMenu self)
{
    log_fun();
    SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, QBViewRightIPTVMenuCallbackInfo, self, NULL);
}

void QBViewRightIPTVMenuRegister(QBTreePathMap pathMap, AppGlobals appGlobals)
{
    log_fun();
    SvObject path = NULL;
    QBActiveTreeNode node = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("ViewRightIPTV"), &path);

    if (SvInvokeInterface(QBCAS, QBCASGetInstance(), getCASType) != QBCASType_viewrightIPTV) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
        log_state("ViewRightIPTV CAS not present, menu disabled");
        return;
    }

    if (!path)
        return;

    QBViewRightIPTVMenu self = QBViewRightIPTVMenuCreate(NULL);
    self->appGlobals = appGlobals;
    self->mainTree = appGlobals->menuTree;

    self->libVersion = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightIPTVLibraryVersion"));
    self->uniqueId = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightIPTVUniqueId"));
    self->caSysId = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightIPTVCaSysId"));
    self->connectivity = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightIPTVConnectivity"));

    QBTreePathMapInsert(pathMap, path, (SvObject) self, NULL);

    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks)&casCallbacks, self, "QBViewRightIPTVMenu");
    QBViewRightIPTVMenuUpdate(self);

    SVRELEASE(self);
}
