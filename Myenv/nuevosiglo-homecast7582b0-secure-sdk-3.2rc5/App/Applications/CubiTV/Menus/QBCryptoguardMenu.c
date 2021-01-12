/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBCryptoguardMenu.h"
#include <QBCASCommonCryptoguardGetMMI.h>
#include <SvFoundation/SvObject.h>
#include <main.h>
#include <QBCAS.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <libintl.h>

#include <QBMenu/QBMenuEventHandler.h>
#include <QBMenu/QBMenu.h>
#include <settings.h>

#define log_fun(fmt, ...) do { if (0) { SvLogNotice(COLBEG() "QBCryptoguardMenu :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)
#define log_debug(fmt, ...) do { if (0) { SvLogNotice(COLBEG() "QBCryptoguardMenu :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } } while (0)
#define log_state(fmt, ...) do { if (1) { SvLogNotice(COLBEG() "QBCryptoguardMenu :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } } while (0)
#define log_error(fmt, ...) do { if (1) { SvLogError(COLBEG() "QBCryptoguardMenu :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } } while (0)

//----------------QBCryptoguardMenu-------------

typedef struct QBCryptoguardMenu_s* QBCryptoguardMenu;

struct QBCryptoguardMenu_s {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    QBActiveTree mainTree;
    QBActiveTreeNode parentNode;
    QBActiveTreeNode cardStateNode;
    QBActiveTreeNode currentNode;

    SvString iconPath;

    QBCASSmartcardState smartcardState;
    struct QBCASCallbacks_s casCallbacks;
    QBCASCmd pendingCmd;
};

SvLocal void QBCryptoguardMenuRemovePleaseWaitNode(QBCryptoguardMenu self, QBActiveTreeNode parent_node);
SvLocal void QBCryptoguardMenuAddNoItemsNode(QBCryptoguardMenu self, QBActiveTreeNode currentNode);

SvLocal void QBCryptoguardMenuRefresh(QBCryptoguardMenu self)
{
    log_fun();
    SvObject parentPath = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, SVSTRING("Cryptoguard2"));
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->mainTree, getIterator, parentPath, 0);
    size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
    QBActiveTreePropagateNodesChange(self->mainTree, parentPath, 0, nodesCount, NULL);
    SVRELEASE(parentPath);
}

SvLocal SvString QBCryptoguardMenuGetFirstNode(QBCryptoguardMenu self, SvString menu, size_t *offset, int *level)
{
    log_fun();

    const char *start = SvStringCString(menu) + *offset;
    const char *ptr = start;
    SvString label = NULL;
    *level = -1;

    ssize_t remainingLenth = SvStringGetLength(menu) - *offset;
    log_debug("remaining length (%zu)", remainingLenth);

    if (remainingLenth < 0) {
        log_error("offset (%zu) cannot be greater then menu string size (%zu)", *offset, SvStringGetLength(menu));
        return NULL;
    }

    if (remainingLenth == 0) {
        log_debug("We parsed all");
        return NULL;
    }

    if (remainingLenth < 5) {
        log_error("Node should have at least 5 characters, received (%zd), node (%s)", remainingLenth, ptr);
        return NULL;
    }

    log_debug("first character = (%c)", *ptr);

    if (*ptr == 'N') {
        log_debug("found N");
        ptr++;
        // next characters should be '00-'
        if (strncmp(ptr, "00-", 3)) {
            goto finished;
        }
    } else if (*ptr == 'S') {
        log_debug("found S");
        ptr++;

        // get 'level'
        char level_str[3];
        memcpy(level_str, ptr, 2);
        level_str[2] = '\0';

        char *end = NULL;
        *level = strtoul(level_str, &end, 10);
        if ((end - level_str) != 2) {
            log_error("Cannot get level from string (%s)", level_str);
            goto finished;
        }
    } else {
        log_error("Cannot parse string (%s)", start);
        goto finished;
    }

    // get label
    ptr += 3;
    char *tmp = strchr(ptr, '\n');
    if (tmp) {
        size_t string_size = (tmp - ptr);
        label = SvStringCreateWithCStringAndLength(ptr, string_size, NULL);
        ptr = tmp + 1;
        log_debug("level(%d) : label (%s)", *level, SvStringCString(label));
    } else {
        log_error("Cannot parse string (%s)", start);
    }

finished:
    *offset = ptr - SvStringCString(menu);
    return label;
}

SvLocal QBActiveTreeNode QBCryptoguardCreateActiveTreeNode(QBCryptoguardMenu self, int level, SvString caption)
{
    QBActiveTreeNode node = QBActiveTreeNodeCreate(NULL, NULL, NULL);

    QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvObject) caption);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("icon"), (SvObject) self->iconPath);

    SvValue levelVar = SvValueCreateWithInteger(level, NULL);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("level"), (SvObject) levelVar);
    SVRELEASE(levelVar);

    return node;
}

SvLocal QBActiveTreeNode QBCryptoguardMenuCreateNodeFromMenuString(QBCryptoguardMenu self, SvString menu, size_t *offset)
{
    log_fun();
    QBActiveTreeNode node = NULL;
    int subnodeLevel = 0;

    SvString label = QBCryptoguardMenuGetFirstNode(self, menu, offset, &subnodeLevel);
    if (label) {
        node = QBCryptoguardCreateActiveTreeNode(self, subnodeLevel, label);
        SVRELEASE(label);
    }

    return node;
}

SvLocal size_t QBCryptoguardMenuParseSubMenu(QBCryptoguardMenu self, uint8_t currentLevel, SvString menu, QBActiveTreeNode parent)
{
    log_fun();
    size_t offset = 0;
    QBActiveTreeNode node = NULL;
    size_t nodesCnt = 0;

    while ((node = QBCryptoguardMenuCreateNodeFromMenuString(self, menu, &offset))) {
        QBActiveTreeNodeAddSubTree(parent, node, NULL);
        SVRELEASE(node);
        nodesCnt++;
    }

    return nodesCnt;
}

SvLocal void QBCryptoguardMenuGetMMICallback(SvObject self_, uint8_t level, SvString menu, int retCode)
{
    log_fun();
    QBCryptoguardMenu self = (QBCryptoguardMenu) self_;

    // mark current command as finished
    self->pendingCmd = NULL;

    if (retCode < 0) {
        log_error("Cannot get MMI for level (%u)", level);
        QBCryptoguardMenuAddNoItemsNode(self, self->currentNode);
    } else {
        log_debug("Menu[%u]\n%s", level, SvStringCString(menu));
        size_t nodesCnt = QBCryptoguardMenuParseSubMenu(self, level, menu, self->currentNode);
        if (nodesCnt == 0) {
            QBCryptoguardMenuAddNoItemsNode(self, self->currentNode);
        }
    }

    QBCryptoguardMenuRemovePleaseWaitNode(self, self->currentNode);
    QBCryptoguardMenuRefresh(self);
}


SvLocal SvString QBCryptoguardMenuCreateCardStateString(QBCryptoguardMenu self, QBCASSmartcardState smartcardState)
{
    SvString mainCaption = NULL;

    switch (smartcardState) {
        case QBCASSmartcardState_removed:
            log_debug("Card removed");
            mainCaption = SvStringCreate(gettext("Card removed"), NULL);
            break;
        case QBCASSmartcardState_incorrect:
        case QBCASSmartcardState_incorrect_ex:
            log_debug("Card incorrect");
            mainCaption = SvStringCreate(gettext("Card incorrect"), NULL);
            break;
        case QBCASSmartcardState_problem:
            log_debug("Card problem");
            mainCaption = SvStringCreate(gettext("Card problem"), NULL);
            break;
        case QBCASSmartcardState_inserted:
            log_debug("Card inserted");
            mainCaption = SvStringCreate(gettext("Card inserted"), NULL);
            break;
        case QBCASSmartcardState_correct:
            log_debug("Card correct");
            mainCaption = SvStringCreate(gettext("Card correct"), NULL);
            break;
        default:
            log_error("Unknown card state, (%d)", smartcardState);
            mainCaption = SvStringCreate(gettext("Unknown card state"), NULL);
    }

    return mainCaption;
}

SvLocal void QBCryptoguardMenuHandleSmartcardState(QBCryptoguardMenu self)
{
    log_fun();

    // all subnodes should be removed while new card state is received
    QBActiveTreeNodeRemoveSubTrees(self->parentNode, NULL);

    if (self->smartcardState == QBCASSmartcardState_correct) {
        self->currentNode = self->parentNode;
        // Call get MMI with level 0
        QBCASCommonCryptoguardGetMMI((QBCASCommonCryptoguard) QBCASGetInstance(), QBCryptoguardMenuGetMMICallback, (SvObject) self, &self->pendingCmd, 0);
    } else {
        // show current card status
        SvString label = QBCryptoguardMenuCreateCardStateString(self, self->smartcardState);
        SVTESTRELEASE(self->cardStateNode);
        self->cardStateNode = QBCryptoguardCreateActiveTreeNode(self, -1, label);
        SVRELEASE(label);

        QBActiveTreeNodeAddSubTree(self->parentNode, self->cardStateNode, NULL);

        self->currentNode = self->cardStateNode;
    }

    QBCryptoguardMenuRefresh(self);
}

SvLocal void QBCryptoguardMenuSmartcardCallback(void* target, QBCASSmartcardState state)
{
    log_fun();
    QBCryptoguardMenu self = (QBCryptoguardMenu) target;
    self->smartcardState = state;

    QBCryptoguardMenuHandleSmartcardState(self);
}

SvLocal void QBCryptoguardMenuAddNoItemsNode(QBCryptoguardMenu self, QBActiveTreeNode currentNode)
{
    log_fun();
    SvString label = SvStringCreate(gettext("No Items"), NULL);
    QBActiveTreeNode node = QBCryptoguardCreateActiveTreeNode(self, -1, label);
    SVRELEASE(label);

    QBActiveTreeNodeSetAttribute(node, SVSTRING("noItems"), (SvObject) SVSTRING("noItems"));
    QBActiveTreeNodeAddSubTree(currentNode, node, NULL);

    SVRELEASE(node);
}

SvLocal void QBCryptoguardMenuAddPleaseWait(QBCryptoguardMenu self, QBActiveTreeNode currentNode)
{
    log_fun();
    SvString label = SvStringCreate(gettext("Please wait..."), NULL);
    QBActiveTreeNode node = QBCryptoguardCreateActiveTreeNode(self, -1, label);
    SVRELEASE(label);

    QBActiveTreeNodeSetAttribute(node, SVSTRING("pleaseWait"), (SvObject) SVSTRING("pleaseWait"));
    QBActiveTreeNodeAddSubTree(currentNode, node, NULL);

    SVRELEASE(node);
}

SvLocal void QBCryptoguardMenuRemovePleaseWaitNode(QBCryptoguardMenu self, QBActiveTreeNode parent_node)
{
    log_fun();
    SvIterator iter = QBActiveTreeNodeChildNodesIterator(parent_node);
    QBActiveTreeNode first = NULL;

    while ((first = (QBActiveTreeNode) SvIteratorGetNext(&iter))) {
        if (QBActiveTreeNodeGetAttribute(first, SVSTRING("pleaseWait"))) {
            QBActiveTreeNodeRemoveSubTree(parent_node, first, NULL);
            log_debug("'pleaseWait' removed from node list");
        }
    }
}

SvLocal bool QBCryptoguardMenuNodeSelected(SvObject self_, SvObject node_, SvObject nodePath_)
{
    log_fun();
    QBCryptoguardMenu self = (QBCryptoguardMenu) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    int level = -1;

    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        return false;
    }

    SvValue levelVal = (SvValue) QBActiveTreeNodeGetAttribute(node, SVSTRING("level"));
    if (levelVal) {
        level = SvValueGetInteger(levelVal);
    }

    log_debug("node (%s), level (%d)", SvStringCString((SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"))), level);

    // level < 0 means that current node does not have subnodes
    if (level < 0) {
        return false;
    }

    if (self->pendingCmd) {
        // cancel previous command if pending
        SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->pendingCmd);
        self->pendingCmd = NULL;
    }

    self->currentNode = node;

    QBActiveTreeNodeRemoveSubTrees(self->currentNode, NULL);
    QBCryptoguardMenuAddPleaseWait(self, self->currentNode);
    QBCASCommonCryptoguardGetMMI((QBCASCommonCryptoguard) QBCASGetInstance(), QBCryptoguardMenuGetMMICallback, (SvObject) self, &self->pendingCmd, (uint8_t) level);

    return false;
}

SvLocal void QBCryptoguardMenuDestroy(void * self_)
{
    log_fun();
    QBCryptoguardMenu self = (QBCryptoguardMenu) self_;

    SVTESTRELEASE(self->cardStateNode);

    SvObject casInstance = QBCASGetInstance();
    if (casInstance) {
        if (self->pendingCmd) {
            SvInvokeInterface(QBCAS, casInstance, cancelCommand, self->pendingCmd);
        }
        SvInvokeInterface(QBCAS, casInstance, removeCallbacks, &self->casCallbacks, self);
    }
    SVTESTRELEASE(self->iconPath);
    SVRELEASE(self->parentNode);
}

SvLocal SvType QBCryptoguardMenu_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBCryptoguardMenuDestroy,
    };

    static const struct QBMenuEventHandler_ selectMethods = {
        .nodeSelected = QBCryptoguardMenuNodeSelected,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBCryptoguardMenu", sizeof(struct QBCryptoguardMenu_s),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &selectMethods,
                            NULL);
    }

    return type;
}

SvLocal QBCryptoguardMenu QBCryptoguardMenuCreate(AppGlobals appGlobals, QBActiveTreeNode parentNode, SvErrorInfo *errorOut)
{
    log_fun();
    SvErrorInfo error = NULL;
    QBCryptoguardMenu self = (QBCryptoguardMenu) SvTypeAllocateInstance(QBCryptoguardMenu_getType(), &error);

    self->appGlobals = appGlobals;
    self->mainTree = appGlobals->menuTree;
    self->parentNode = SVRETAIN(parentNode);

    svSettingsPushComponent("CryptoguardMenu.settings");
    self->iconPath = SvStringCreate(svSettingsGetString("IconPaths", "iconPath"), NULL);
    svSettingsPopComponent();

    self->smartcardState = SvInvokeInterface(QBCAS, QBCASGetInstance(), getState);

    self->casCallbacks.smartcard_state = &QBCryptoguardMenuSmartcardCallback;
    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, &self->casCallbacks, self, "QBCryptoguardMenu");

    SvErrorInfoPropagate(error, errorOut);
    return self;
}

void QBCryptoguardMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    log_fun();
    QBActiveTreeNode node = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("Cryptoguard2"));
    SvObject path = node ? QBActiveTreeCreateNodePath(appGlobals->menuTree, QBActiveTreeNodeGetID(node)) : NULL;

    if (!SvObjectIsInstanceOf(QBCASGetInstance(), QBCASCommonCryptoguard_getType())) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
        log_state("Cryptoguard CAS not present, menu disabled");
        SVTESTRELEASE(path);
        return;
    }

    if (!path) {
        return;
    }

    QBCryptoguardMenu self = QBCryptoguardMenuCreate(appGlobals, node, NULL);
    if (!self) {
        log_error("Cannot create QBCryptoguardMenu");
        SVRELEASE(path);
        return;
    }

    QBTreePathMapInsert(pathMap, path, (SvObject) self, NULL);
    SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, path, (SvObject) self, NULL);
    QBCryptoguardMenuHandleSmartcardState(self);

    SVRELEASE(self);
    SVRELEASE(path);
}
