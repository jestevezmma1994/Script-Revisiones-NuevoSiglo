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

#include "QBUserInterfaceMenuItemChoice.h"

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvTime.h>
#include <QBConfig.h>
#include <QBShellCmds.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBApplicationController.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvPlayerKit/SvTimeRange.h>
#include <TraxisWebClient/TraxisWebSessionManager.h>
#include <init.h>
#include <Logic/InitLogic.h>
#include <main.h>
#include <Services/core/playlistManager.h>
#include <Services/QBProvidersControllerService.h>
#include "QBConfigurationMenuUtils.h"

struct QBUserInterfaceMenuItemChoice_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    QBActiveTree tree;

    QBConfigurationMenuRestrictedNodesHandler restrictedNodesHandler;

    // AMERELES [#3144] Reordenamiento y limpieza de opciones de configuración
    struct {
        QBActiveTreeNode analogNode;
        QBActiveTreeNode digitalNode;
        bool hidden;
    } selectClosedCaptionsMenu;
};

// BEGIN AMERELES [#3144] Reordenamiento y limpieza de opciones de configuración
SvLocal void QBUserInterfaceMenuShowSelectClosedCaptionsMenu(QBUserInterfaceMenuItemChoice self)
{
    if (!self->selectClosedCaptionsMenu.hidden)
        return;

    QBActiveTreeNode parentNode = QBActiveTreeFindNode(self->tree, SVSTRING("CC"));
    
    if (self->selectClosedCaptionsMenu.analogNode) {
        QBActiveTreeNodeAddSubTree(parentNode, self->selectClosedCaptionsMenu.analogNode, NULL);
    }

    if (self->selectClosedCaptionsMenu.digitalNode) {
        QBActiveTreeNodeAddSubTree(parentNode, self->selectClosedCaptionsMenu.digitalNode, NULL);
    }

    self->selectClosedCaptionsMenu.hidden = false;
}

SvLocal void QBUserInterfaceMenuHideSelectClosedCaptionsMenu(QBUserInterfaceMenuItemChoice self)
{
    if (self->selectClosedCaptionsMenu.hidden)
        return;

    if (self->selectClosedCaptionsMenu.analogNode) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(self->selectClosedCaptionsMenu.analogNode),
                                      self->selectClosedCaptionsMenu.analogNode, NULL);
    }
    if (self->selectClosedCaptionsMenu.digitalNode) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(self->selectClosedCaptionsMenu.digitalNode),
                                      self->selectClosedCaptionsMenu.digitalNode, NULL);
    }
    self->selectClosedCaptionsMenu.hidden = true;
}
// END AMERELES [#3144] Reordenamiento y limpieza de opciones de configuración

SvLocal bool
QBUserInterfaceMenuItemNodeApply(void *self_,
                                 SvObject node_,
                                 SvObject nodePath_)
{
    if (!SvObjectIsInstanceOf((SvObject) node_, QBActiveTreeNode_getType()))
        return false;

    bool handled = false;
    QBUserInterfaceMenuItemChoice self = (QBUserInterfaceMenuItemChoice) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    SvString optVal = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationValue"));

    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode((QBActiveTreeNode) node);
    if (!parent)
        return handled;

    SvString parentId = QBActiveTreeNodeGetID(parent);
    if (parentId) {
        SvString id = QBActiveTreeNodeGetID(node); // AMERELES [#3144] Reordenamiento y limpieza de opciones de configuración
        const char* qlang = QBConfigGet("LANG");
        if (optVal && SvStringEqualToCString(parentId, "MLNG") &&
            qlang && !SvStringEqualToCString(optVal, qlang)) {
            QBInitLogicSetLocale(self->appGlobals->initLogic, optVal);
            QBApplicationControllerReinitializeWindows(self->appGlobals->controller, NULL);

            // update epg
            SvTimeRange timeRange;
            time_t t = SvTimeNow();
            SvTimeRangeInit(&timeRange, t - 100, t + 100);

            QBServiceRegistry registry = QBServiceRegistryGetInstance();
            QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
            SvObject playlist = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
            int count = SvInvokeInterface(SvEPGChannelView, playlist, getCount);

            SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                 SVSTRING("SvEPGManager"));
            int idx = 0;
            for (idx = 0; idx < count; ++idx) {
                SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, playlist, getByIndex, idx);
                SvEPGManagerPropagateEventsChange(epgManager, SvTVChannelGetID(channel), &timeRange, NULL);
            }
            handled = true;
        }
        // BEGIN AMERELES [#3144] Reordenamiento y limpieza de opciones de configuración
        else if (SvStringEqualToCString(id, "CC_MANUAL")) {
            QBUserInterfaceMenuShowSelectClosedCaptionsMenu(self);
            handled = true;
        } else if (SvStringEqualToCString(id, "CC_OFF") || SvStringEqualToCString(id, "CC_AUTOMATIC")) {
            QBUserInterfaceMenuHideSelectClosedCaptionsMenu(self);
            handled = true;
        }
        // END AMERELES [#3144] Reordenamiento y limpieza de opciones de configuración
    }

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
QBUserInterfaceMenuItemNodeSelected(SvObject self_,
                                    SvObject node_,
                                    SvObject nodePath_)
{
    QBUserInterfaceMenuItemChoice self = (QBUserInterfaceMenuItemChoice) self_;
    bool handled = QBConfigurationMenuHandleItemNodeSelection(self->restrictedNodesHandler, node_, nodePath_);
    if (!handled)
        handled = QBUserInterfaceMenuItemNodeApply((void *) self_, node_, nodePath_);

    return handled;
}

SvLocal void
QBUserInterfaceMenuItemChoiceDestroy(void *self_)
{
    QBUserInterfaceMenuItemChoice self = self_;

    // AMERELES [#3144] Reordenamiento y limpieza de opciones de configuración
    SVTESTRELEASE(self->selectClosedCaptionsMenu.analogNode);
    SVTESTRELEASE(self->selectClosedCaptionsMenu.digitalNode);

    SVRELEASE(self->tree);
    SVTESTRELEASE(self->restrictedNodesHandler);
}

SvLocal SvType
QBUserInterfaceMenuItemChoice_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBUserInterfaceMenuItemChoiceDestroy
    };

    static const struct QBMenuEventHandler_ eventHandlerMethods = {
        .nodeSelected = QBUserInterfaceMenuItemNodeSelected,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBUserInterfaceMenuItemChoice",
                            sizeof(struct QBUserInterfaceMenuItemChoice_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &eventHandlerMethods,
                            NULL);
    }

    return type;
}

QBUserInterfaceMenuItemChoice
QBUserInterfaceMenuItemChoiceCreate(AppGlobals appGlobals, SvWidget menu, QBActiveTree tree, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    if (!tree) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL tree passed");
        goto out;
    }

    QBUserInterfaceMenuItemChoice self = (QBUserInterfaceMenuItemChoice) SvTypeAllocateInstance(QBUserInterfaceMenuItemChoice_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBMainMenuItemController");
        goto out;
    }
    self->appGlobals = appGlobals;
    self->tree = SVRETAIN(tree);

    self->restrictedNodesHandler =
        QBConfigurationMenuRestrictedNodesHandlerCreate(appGlobals, (void *) self,
                                                        QBUserInterfaceMenuItemNodeApply,
                                                        menu, tree, &error);
    if (!self->restrictedNodesHandler) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBConfigurationMenuRestrictedNodesHandler");
        SVRELEASE(self);
        goto out;
    }

    // BEGIN AMERELES [#3144] Reordenamiento y limpieza de opciones de configuración
    const char *ccChannelMode = QBConfigGet("CLOSED_CAPTION_CHANNEL_MODE");
    bool hide = true;
    if (!ccChannelMode || !strcmp(ccChannelMode, "Manual"))
        hide = false;
    
    QBActiveTreeNode parentNode = QBActiveTreeFindNode(self->tree, SVSTRING("CC"));
    self->selectClosedCaptionsMenu.analogNode = SVTESTRETAIN(QBActiveTreeNodeFindChildByID(parentNode, SVSTRING("ACC")));
    self->selectClosedCaptionsMenu.digitalNode = SVTESTRETAIN(QBActiveTreeNodeFindChildByID(parentNode, SVSTRING("DCC")));
    
    if (hide)
        QBUserInterfaceMenuHideSelectClosedCaptionsMenu(self);
    // END AMERELES [#3144] Reordenamiento y limpieza de opciones de configuración

    return self;
out:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}
