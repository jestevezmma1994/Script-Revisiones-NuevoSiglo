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

#include "QBChannelInstallationMenuItemChoice.h"

#include <libintl.h>

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLog.h>
#include <QBConfig.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <QBApplicationController.h>
#include <QBWindowContext.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <Services/core/QBChannelScanningConfManager.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <main.h>
#include <Windows/channelscanning.h>
#include <Windows/SatelliteSetup.h>

#include "QBConfigurationMenuUtils.h"


struct QBChannelInstallationMenuItemChoice_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    QBActiveTree tree;

    QBConfigurationMenuRestrictedNodesHandler restrictedNodesHandler;
};

SvLocal bool
QBChannelInstallationMenuItemNodeApply(void *self_,
                                       SvObject node_,
                                       SvObject nodePath_)
{
    if (!SvObjectIsInstanceOf((SvObject) node_, QBActiveTreeNode_getType()))
        return false;

    bool handled = false;
    QBChannelInstallationMenuItemChoice self = (QBChannelInstallationMenuItemChoice) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    SvString action = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("action"));
    SvString optVal = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationValue"));

    if (action) {
        if (SvStringEqualToCString(action, "Channel scanning")) {
            SvString type = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("type"));
            QBWindowContext ctx = QBChannelScanningContextCreate(self->appGlobals, type);

            SvString scanType = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("scanType"));
            if (scanType) {
                QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                              SVSTRING("QBChannelScanningConfManager"));
                QBChannelScanningConf conf = QBChannelScanningConfManagerGetConf(channelScanningConf, scanType);
                if (conf)
                    QBChannelScanningContextLoadConf(ctx, conf);
            }

            QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
            SVRELEASE(ctx);
            handled = true;
        } else if (SvStringEqualToCString(action, "Satellite setup")) {
            QBWindowContext ctx = SatelliteSetupContextCreate(self->appGlobals->res);
            if (ctx) {
                QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
                SVRELEASE(ctx);
            } else {
                SvLogWarning("CubiTV: satellite support is non-functional!");
            }
            handled = true;
        }
    }

    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode((QBActiveTreeNode) node);
    if (!parent)
        return handled;

    SvString optName = (SvString) QBActiveTreeNodeGetAttribute(parent, SVSTRING("configurationName"));
    if (!optName || !optVal)
        return handled;

    handled = true;
    QBConfigSet(SvStringCString(optName), SvStringCString(optVal));
    QBConfigSave();
    SvObject parentPath = SvObjectCopy(nodePath_, NULL);
    SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->tree,
                                            getIterator, parentPath, 0);
    size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
    QBActiveTreePropagateNodesChange(self->tree, parentPath, 0, nodesCount, NULL);
    SVRELEASE(parentPath);

    return handled;
}

SvLocal bool
QBChannelInstallationMenuItemNodeSelected(SvObject self_,
                                          SvObject node_,
                                          SvObject nodePath_)
{
    QBChannelInstallationMenuItemChoice self = (QBChannelInstallationMenuItemChoice) self_;
    bool handled = QBConfigurationMenuHandleItemNodeSelection(self->restrictedNodesHandler, node_, nodePath_);
    if (!handled)
        handled = QBChannelInstallationMenuItemNodeApply((void *) self_, node_, nodePath_);

    return handled;
}

SvLocal void
QBChannelInstallationMenuItemChoiceDestroy(void *self_)
{
    QBChannelInstallationMenuItemChoice self = self_;

    SVRELEASE(self->tree);
    SVTESTRELEASE(self->restrictedNodesHandler);
}

SvLocal SvType
QBChannelInstallationMenuItemChoice_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBChannelInstallationMenuItemChoiceDestroy
    };

    static const struct QBMenuEventHandler_ eventHandlerMethods = {
        .nodeSelected = QBChannelInstallationMenuItemNodeSelected,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBChannelInstallationMenuItemChoice",
                            sizeof(struct QBChannelInstallationMenuItemChoice_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &eventHandlerMethods,
                            NULL);
    }

    return type;
}

QBChannelInstallationMenuItemChoice
QBChannelInstallationMenuItemChoiceCreate(AppGlobals appGlobals, SvWidget menu, QBActiveTree tree, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    if (!tree) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL tree passed");
        goto out;
    }

    QBChannelInstallationMenuItemChoice self = (QBChannelInstallationMenuItemChoice)
        SvTypeAllocateInstance(QBChannelInstallationMenuItemChoice_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBMainMenuItemController");
        goto out;
    }

    self->appGlobals = appGlobals;
    self->tree = SVRETAIN(tree);

    self->restrictedNodesHandler =
        QBConfigurationMenuRestrictedNodesHandlerCreate(appGlobals, (void *) self,
                                                        QBChannelInstallationMenuItemNodeApply,
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
