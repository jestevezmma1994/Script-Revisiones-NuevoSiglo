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

#include "QBTimeZoneMenuItemChoice.h"

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLog.h>
#include <QBConfig.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <main.h>
#include "QBConfigurationMenuUtils.h"


struct QBTimeZoneMenuItemChoice_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    QBActiveTree tree;

    QBConfigurationMenuRestrictedNodesHandler restrictedNodesHandler;
};

SvLocal bool
QBTimeZoneMenuItemNodeApply(void *self_,
                            SvObject node_,
                            SvObject nodePath_)
{
    if (!SvObjectIsInstanceOf((SvObject) node_, QBActiveTreeNode_getType()))
        return false;

    QBTimeZoneMenuItemChoice self = (QBTimeZoneMenuItemChoice) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    SvString optVal = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationValue"));
    SvString optName = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationName"));
    if (!optVal || !optName)
        return false;

    QBConfigSet(SvStringCString(optName), SvStringCString(optVal));
    QBConfigSave();

    SvObject parentPath = SvObjectCopy(nodePath_, NULL);
    SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->tree,
                                            getIterator, parentPath, 0);
    size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
    QBActiveTreePropagateNodesChange(self->tree, parentPath, 0, nodesCount, NULL);
    SVRELEASE(parentPath);

    setenv("TZ", SvStringCString(optVal), 1);
    tzset();

    return true;
}

SvLocal bool
QBTimeZoneMenuItemNodeSelected(SvObject self_,
                               SvObject node_,
                               SvObject nodePath_)
{
    QBTimeZoneMenuItemChoice self = (QBTimeZoneMenuItemChoice) self_;
    bool handled = QBConfigurationMenuHandleItemNodeSelection(self->restrictedNodesHandler, node_, nodePath_);
    if (!handled)
        handled = QBTimeZoneMenuItemNodeApply((void *) self_, node_, nodePath_);

    return handled;
}

SvLocal void
QBTimeZoneMenuItemChoiceDestroy(void *self_)
{
    QBTimeZoneMenuItemChoice self = self_;

    SVRELEASE(self->tree);
    SVTESTRELEASE(self->restrictedNodesHandler);
}

SvLocal SvType
QBTimeZoneMenuItemChoice_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTimeZoneMenuItemChoiceDestroy
    };

    static const struct QBMenuEventHandler_ eventHandlerMethods = {
        .nodeSelected = QBTimeZoneMenuItemNodeSelected,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTimeZoneMenuItemChoice",
                            sizeof(struct QBTimeZoneMenuItemChoice_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &eventHandlerMethods,
                            NULL);
    }

    return type;
}

QBTimeZoneMenuItemChoice
QBTimeZoneMenuItemChoiceCreate(AppGlobals appGlobals, SvWidget menu, QBActiveTree tree, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    if (!tree) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL tree passed");
        goto out;
    }

    QBTimeZoneMenuItemChoice self = (QBTimeZoneMenuItemChoice) SvTypeAllocateInstance(QBTimeZoneMenuItemChoice_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBMainMenuItemController");
        goto out;
    }
    self->appGlobals = appGlobals;
    self->tree = SVRETAIN(tree);

    self->restrictedNodesHandler =
        QBConfigurationMenuRestrictedNodesHandlerCreate(appGlobals, (void *) self,
                                                        QBTimeZoneMenuItemNodeApply,
                                                        menu, tree, &error);
    if (!self->restrictedNodesHandler) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBConfigurationMenuRestrictedNodesHandler");
        SVRELEASE(self);
        goto out;
    }

    return self;
out:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}
