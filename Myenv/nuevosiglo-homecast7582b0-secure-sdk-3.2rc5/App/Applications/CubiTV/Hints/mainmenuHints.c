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

#include "mainmenuHints.h"
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeModelListener.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <Menus/menuchoice.h>
#include <Hints/hintsGetter.h>

struct QBMainMenuHints_t {
    struct SvObject_ super_;
    QBHintManager hintsManager;
    SvGenericObject tree;
    QBTreePathMap pathMap;

    SvGenericObject parentPath;
    SvGenericObject path;

    SvString lastSetHints;

    bool started;
};

SvLocal void QBMainMenuHintsItemChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBMainMenuHints self = (QBMainMenuHints) self_;

    SVTESTRELEASE(self->parentPath);

    if (!SvObjectEquals(self->path, nodePath_)) {
        SVTESTRELEASE(self->path);
        self->path = SVTESTRETAIN(nodePath_);
    }

    if (SvInvokeInterface(QBTreePath, nodePath_, getLength) > 1) {
        self->parentPath = SvObjectCopy(nodePath_, NULL);
        SvInvokeInterface(QBTreePath, self->parentPath, truncate, -1);
    } else {
        self->parentPath = SVRETAIN(nodePath_);
    }

    SvGenericObject path = SvObjectCopy(nodePath_, NULL);

    SvString hints = NULL;
    SvGenericObject handler = QBTreePathMapFind(self->pathMap, path);
    if (handler && SvObjectIsImplementationOf(handler, QBHintsGetter_getInterface())) {
        hints = SvInvokeInterface(QBHintsGetter, handler, getHintsForPath, path);
    } else {
        //Iterate upwards from current node in order to find any node that has hints
        while(SvInvokeInterface(QBTreePath, path, getLength)) {
            SvObject hintNode = SvInvokeInterface(QBTreeModel, self->tree, getNode, path);
            SvInvokeInterface(QBTreePath, path, truncate, -1);

            if (!hintNode || !SvObjectIsInstanceOf(hintNode, QBActiveTreeNode_getType()))
                continue;

            QBActiveTreeNode node = (QBActiveTreeNode) hintNode;

            hints = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("hints"));
            if (!hints)
                continue;
            break;
        }
    }

    if (hints) {
        if (!self->lastSetHints || !SvObjectEquals((SvObject) self->lastSetHints, (SvObject) hints)) {
            SVTESTRELEASE(self->lastSetHints);
            self->lastSetHints = SVRETAIN(hints);
            if (self->started)
                QBHintManagerSetHintsFromString(self->hintsManager, hints);
        }
        goto out;
    }

    //If none found, then reset hints
    if(self->started)
        QBHintManagerSetHintsFromString(self->hintsManager, NULL);
    SVTESTRELEASE(self->lastSetHints);
    self->lastSetHints = NULL;

out:
    SVRELEASE(path);
}

SvLocal void QBMainMenuHintsTreeIgnore(SvGenericObject self_, SvGenericObject path, size_t first, size_t count)
{
}

SvLocal void QBMainMenuHintsTreeChanged(SvGenericObject self_, SvGenericObject path, size_t first, size_t count)
{
    QBMainMenuHints self = (QBMainMenuHints) self_;
    if (!self->parentPath)
        return;

    if (SvObjectEquals(path, self->parentPath))
        QBMainMenuHintsItemChoosen((SvObject) self, NULL, self->path, 0);
}

SvLocal void QBMainMenuHints__dtor__(void *self_)
{
    QBMainMenuHints self = self_;
    SVRELEASE(self->tree);
    SVRELEASE(self->hintsManager);
    SVTESTRELEASE(self->parentPath);
    SVTESTRELEASE(self->path);
    SVTESTRELEASE(self->lastSetHints);
}

SvLocal SvType QBMainMenuHints_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMainMenuHints__dtor__
    };
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBMainMenuHintsItemChoosen,
    };

    static const struct QBTreeModelListener_t treeMethods = {
        .nodesAdded = QBMainMenuHintsTreeIgnore,
        .nodesRemoved = QBMainMenuHintsTreeIgnore,
        .nodesChanged = QBMainMenuHintsTreeChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMainMenuHints",
                            sizeof(struct QBMainMenuHints_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBTreeModelListener_getInterface(), &treeMethods,
                            NULL);
    }

    return type;
}

QBMainMenuHints QBMainMenuHintsCreate(QBHintManager hintsManager, QBTreePathMap pathMap, SvGenericObject tree)
{
    QBMainMenuHints self = (QBMainMenuHints) SvTypeAllocateInstance(QBMainMenuHints_getType(), NULL);
    self->hintsManager = SVRETAIN(hintsManager);
    self->tree = SVRETAIN(tree);
    self->pathMap = pathMap;
    return self;
}

void QBMainMenuHintsStart(QBMainMenuHints self)
{
    self->started = true;
    if(self->lastSetHints)
        QBHintManagerSetHintsFromString(self->hintsManager, self->lastSetHints);

    SvInvokeInterface(QBDataModel, self->tree, addListener, (SvGenericObject) self, NULL);
}

void QBMainMenuHintsStop(QBMainMenuHints self)
{
    self->started = false;
    QBHintManagerSetHintsFromString(self->hintsManager, NULL);
}
