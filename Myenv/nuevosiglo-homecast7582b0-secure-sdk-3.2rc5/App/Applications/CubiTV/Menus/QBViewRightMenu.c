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

#include "QBViewRightMenu.h"

#include <libintl.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBCAS.h>
#include <QBViewRightInfo.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <main.h>


#define log_debug(fmt, ...) if (0) { SvLogNotice(COLBEG() "QBViewRightMenu :: %s " fmt COLEND_COL(green), __func__, ##__VA_ARGS__); }
#define log_state(fmt, ...) if (1) { SvLogNotice(COLBEG() "QBViewRightMenu :: %s " fmt COLEND_COL(blue), __func__, ##__VA_ARGS__); }
#define log_error(fmt, ...) if (1) { SvLogError(COLBEG() "QBViewRightMenu :: %s " fmt COLEND_COL(red), __func__, ##__VA_ARGS__); }


//----------------QBViewRightMenu-------------

typedef struct QBViewRightMenu_s* QBViewRightMenu;

struct QBViewRightMenu_s {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvWidget  menuBar;

    QBActiveTreeNode mode;
    QBActiveTreeNode serialNo;
    QBActiveTreeNode chipId;
    QBActiveTreeNode libVersion;
    QBActiveTreeNode libDate;
    QBActiveTreeNode pairing;
    QBActiveTreeNode purse;
    QBActiveTreeNode purseEmpty;
    SvArray walletNodes;
    SvString purseIcon;
};

SvLocal bool QBViewRightMenuSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_);
SvLocal void QBViewRightMenuUpdate(QBViewRightMenu self);
SvLocal void QBViewRightMenuDestroy(void *self_);

SvLocal void QBViewRightMenuSmartcardCallback(void* target, QBCASSmartcardState state);

static const struct QBCASCallbacks_s casCallbacks = {
  .smartcard_state  = &QBViewRightMenuSmartcardCallback,
};

SvLocal SvType QBViewRightMenu_getType(void) {
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBViewRightMenuDestroy
    };

    static SvType type = NULL;

    static const struct QBMenuEventHandler_ selectMethods = {
      .nodeSelected = QBViewRightMenuSelected
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBViewRightMenu",
                            sizeof(struct QBViewRightMenu_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &selectMethods,
                NULL);
    }

    return type;
}

SvLocal void QBViewRightMenuDestroy(void *self_)
{
    QBViewRightMenu self = (QBViewRightMenu) self_;
    SVRELEASE(self->walletNodes);
    SvInvokeInterface(QBCAS, QBCASGetInstance(), removeCallbacks, (QBCASCallbacks)&casCallbacks, self);
}

SvLocal QBViewRightMenu QBViewRightMenuCreate(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBViewRightMenu self = (QBViewRightMenu) SvTypeAllocateInstance(QBViewRightMenu_getType(), &error);
    if (unlikely(error))
        goto fini;
fini:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

SvLocal bool QBViewRightMenuSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_)
{
    log_debug();
    QBViewRightMenu self = (QBViewRightMenu) self_;
    QBViewRightMenuUpdate(self);
    return false;
}

SvLocal void QBViewRightMenuRefresh(QBViewRightMenu self)
{
    SvObject parentPath = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, SVSTRING("ViewRight"));
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, parentPath, 0);
    size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
    QBActiveTreePropagateNodesChange(self->appGlobals->menuTree, parentPath, 0, nodesCount, NULL);
    SVRELEASE(parentPath);
}

SvLocal SvString QBViewRightMenuModeStringCreate(QBViewRightMode mode)
{
    const char * modeCStr = NULL;

    switch (mode) {
    case QBViewRightMode_smartcard:
        modeCStr = gettext("Smartcard");
        break;
    case QBViewRightMode_nonsmartcard:
        modeCStr = gettext("Non smartcard");
        break;
    case QBViewRightMode_notPresent:
        modeCStr = gettext("Not present");
        break;
    case QBViewRightMode_error:
        modeCStr = gettext("Error");
        break;
    default:
        modeCStr = gettext("Error");
        break;
    }
    return SvStringCreate(modeCStr, NULL);
}

SvLocal SvString QBViewRightMenuPairingStringCreate(QBViewRightPairingState state)
{

    const char * modeCStr = NULL;

    switch (state) {
    case QBViewRightPairingState_NoPairing:
        modeCStr = gettext("No pairing");
        break;
    case QBViewRightPairingState_PairingBad:
        modeCStr = gettext("Pairing bad");
        break;
    case QBViewRightPairingState_PairingInProgress:
        modeCStr = gettext("In progress");
        break;
    case QBViewRightPairingState_PairingOk:
        modeCStr = gettext("Pairing Ok");
        break;
    default:
        modeCStr = gettext("Error");
        break;
    }
    return SvStringCreate(modeCStr,NULL);
}

SvLocal void QBViewRightMenuUpdateWalletSubmenu(QBViewRightMenu self, SvArray wallets, SvArray walletNames)
{
    uint walletNodesCount = SvArrayCount(self->walletNodes);

    if (!wallets ) { //Clear submenu

        if (!QBActiveTreeNodeGetParentNode(self->purseEmpty)) {
            QBActiveTreeNodeAddChildNode(self->purse, self->purseEmpty, NULL);
        }

        for (uint i = 0; i < walletNodesCount; i++) {
            QBActiveTreeNode node = (QBActiveTreeNode) SvArrayLastObject(self->walletNodes);
            SvArrayRemoveLastObject(self->walletNodes);
            QBActiveTreeRemoveSubTree(self->appGlobals->menuTree, node, NULL);
        }

        return;
    }

    if (QBActiveTreeNodeGetParentNode(self->purseEmpty)) {
        QBActiveTreeNodeRemoveChildNode(self->purse, self->purseEmpty);
    }

    uint walletsCount = SvArrayCount(wallets);

    if (walletsCount > walletNodesCount) {    //Add positions in purse submenu.
        for (uint i = 0; i < walletsCount - walletNodesCount; i++) {
            QBActiveTreeNode node = QBActiveTreeNodeCreate(NULL, self->purse, NULL);
            QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvGenericObject) SVSTRING(""));
            QBActiveTreeAddSubTree(self->appGlobals->menuTree, self->purse, node, NULL);
            QBActiveTreeNodeSetAttribute(node, SVSTRING("icon"), (SvGenericObject) self->purseIcon);
            SvArrayAddObject(self->walletNodes, (SvGenericObject) node);
            SVRELEASE(node);
        }
    } else {    //Remove positions in purse submenu
        for (uint i = 0; i < walletNodesCount - walletsCount; i++) {
            QBActiveTreeNode node = (QBActiveTreeNode) SvArrayLastObject(self->walletNodes);
            SvArrayRemoveLastObject(self->walletNodes);
            QBActiveTreeRemoveSubTree(self->appGlobals->menuTree, node, NULL);
        }
    }

    SvIterator iter = SvArrayIterator(wallets);
    SvIterator iterNodes = SvArrayIterator(self->walletNodes);
    SvIterator iterNames = SvArrayIterator(walletNames);
    SvString moneyStr = NULL;
    SvString walletName = NULL;
    while ((moneyStr = (SvString) SvIteratorGetNext(&iter))) {
        QBActiveTreeNode node = (QBActiveTreeNode) SvIteratorGetNext(&iterNodes);
        walletName = (SvString) SvIteratorGetNext(&iterNames);
        QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvGenericObject) walletName);
        QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvGenericObject) moneyStr);
    }
}

SvLocal void QBViewRightMenuCallbackInfo(void* self_, const QBCASCmd cmd, int status, QBCASInfo info)
{
    log_debug();
    QBViewRightMenu self = (QBViewRightMenu) self_;

    QBViewRightInfo vmxInfo = (QBViewRightInfo) info;

    SvString mode = QBViewRightMenuModeStringCreate(vmxInfo->mode);
    QBActiveTreeNodeSetAttribute(self->mode, SVSTRING("subcaption"), (SvGenericObject) mode);
    SVRELEASE(mode);

    SvString pairing = QBViewRightMenuPairingStringCreate(vmxInfo->pairingState);
    QBActiveTreeNodeSetAttribute(self->pairing, SVSTRING("subcaption"), (SvGenericObject) pairing);
    SVRELEASE(pairing);

    QBActiveTreeNodeSetAttribute(self->serialNo, SVSTRING("subcaption"), (SvGenericObject) vmxInfo->base.card_number_complete);
    QBActiveTreeNodeSetAttribute(self->chipId, SVSTRING("subcaption"), (SvGenericObject) vmxInfo->base.chip_id);
    QBActiveTreeNodeSetAttribute(self->libVersion, SVSTRING("subcaption"), (SvGenericObject) vmxInfo->lib_version);
    QBActiveTreeNodeSetAttribute(self->libDate, SVSTRING("subcaption"), (SvGenericObject) vmxInfo->libDate);

    QBViewRightMenuUpdateWalletSubmenu(self, vmxInfo->wallets, vmxInfo->walletNames);

    QBViewRightMenuRefresh(self);
}


SvLocal void QBViewRightMenuUpdate(QBViewRightMenu self)
{
    log_debug();
    SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, &QBViewRightMenuCallbackInfo, self, NULL);
}

SvLocal void QBViewRightMenuSmartcardCallback(void* target, QBCASSmartcardState state)
{
    log_debug("Smartcard state = %d", state);
    QBViewRightMenu self = (QBViewRightMenu) target;
    QBViewRightMenuUpdate(self);
}

void QBViewRightMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    QBActiveTreeNode node = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRight"));

    if (!SvObjectIsInstanceOf(QBCASGetInstance(), QBViewRight_getType())) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
        log_state("ViewRightDVB CAS not present, menu disabled");
        return;
    }

    SvObject path = QBActiveTreeCreateNodePath(appGlobals->menuTree, SVSTRING("ViewRight"));
    if (!path)
        return;

    QBViewRightMenu self = QBViewRightMenuCreate(NULL);
    self->appGlobals = appGlobals;
    self->menuBar = menuBar;

    self->mode = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightMode"));
    self->libVersion = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightLibVersion"));
    self->libDate = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightLibDate"));
    self->serialNo = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightSerialNo"));
    self->chipId =  QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightChipId"));
    self->purse = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightPurse"));
    self->purseEmpty = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightPurseEmpty"));
    self->pairing = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightPairing"));
    self->walletNodes = SvArrayCreate(NULL);
    self->purseIcon = (SvString) QBActiveTreeNodeGetAttribute(self->purse, SVSTRING("icon"));

    QBTreePathMapInsert(pathMap, path, (SvGenericObject) self, NULL);

    SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, path, (SvObject) self, NULL);
    SVRELEASE(path);

    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks) &casCallbacks, self, "QBViewRightMenu");
    QBViewRightMenuUpdate(self);

    SVRELEASE(self);
}
