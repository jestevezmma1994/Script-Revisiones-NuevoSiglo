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

#include "QBSystemSettingsMenuItemChoice.h"

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
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvPlayerKit/SvTimeRange.h>
#include <ContextMenus/QBContextMenu.h>
#include <init.h>
#include <Logic/AudioTrackLogic.h>
#include <Logic/UpgradeLogic.h>
#include <main.h>
#include <Services/core/playlistManager.h>
#include <Utils/appType.h>
#include <Windows/QBNetworkSettingsWindow.h>
#include <Services/QBRCUPairingService.h>
#include <Windows/QBRCUPairingWindow.h>
#include "QBConfigurationMenuUtils.h"


struct QBSystemSettingsMenuItemChoice_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    QBActiveTree tree;

    struct {
        QBActiveTreeNode node;
        bool hidden;
    } selectTimeZoneMenu;

    struct {
        QBActiveTreeNode analogNode;
        QBActiveTreeNode digitalNode;
        bool hidden;
    } selectClosedCaptionsMenu;

    QBConfigurationMenuRestrictedNodesHandler restrictedNodesHandler;

    QBContextMenu sideMenuCtx;
};

SvLocal void QBConfigurationMenuUpdateVODServicesDialogCreate(void *self_);

SvLocal void QBSystemSettingsMenuShowSelectTimeZoneMenu(QBSystemSettingsMenuItemChoice self)
{
    if (self->selectTimeZoneMenu.hidden && self->selectTimeZoneMenu.node) {
        QBActiveTreeNode parentNode = QBActiveTreeFindNode(self->tree, SVSTRING("Time Zone"));
        QBActiveTreeNodeAddSubTree(parentNode, self->selectTimeZoneMenu.node, NULL);
    }
    self->selectTimeZoneMenu.hidden = false;

    const char* manualTZ = QBConfigGet("TIMEZONE");
    if (manualTZ) {
        setenv("TZ", manualTZ, 1);
        tzset();
    }
}

SvLocal void QBSystemSettingsMenuHideSelectTimeZoneMenu(QBSystemSettingsMenuItemChoice self)
{
    if (!self->selectTimeZoneMenu.hidden && self->selectTimeZoneMenu.node) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(self->selectTimeZoneMenu.node), self->selectTimeZoneMenu.node, NULL);
    }
    self->selectTimeZoneMenu.hidden = true;
}

SvLocal void QBSystemSettingsMenuShowSelectClosedCaptionsMenu(QBSystemSettingsMenuItemChoice self)
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

SvLocal void QBSystemSettingsMenuHideSelectClosedCaptionsMenu(QBSystemSettingsMenuItemChoice self)
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


SvLocal void
QBSystemSettingsMenuItemRunRCUPairing(QBSystemSettingsMenuItemChoice self)
{
    QBRCUPairingService rcuPairingService = (QBRCUPairingService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                              SVSTRING("QBRCUPairingService"));
    QBRCUPairingContext rcuPairingContext = QBRCUPairingContextCreate();
    QBApplicationControllerPushContext(self->appGlobals->controller, (QBWindowContext) rcuPairingContext);
    const int PAIRING_FROM_MENU_ATTEMPTS_COUNT = 2;
    QBRCUPairingServiceStartPairing(rcuPairingService, PAIRING_FROM_MENU_ATTEMPTS_COUNT);

    SVRELEASE(rcuPairingContext);
}

SvLocal bool QBUserInterfaceMenuItemNodeHandleNode(QBSystemSettingsMenuItemChoice self, QBActiveTreeNode node)
{
    bool handled = false;
    SvString id = QBActiveTreeNodeGetID(node);
    if (id) {
        if (SvStringEqualToCString(id, "RestoreDefaults")) {
            QBInitFactoryResetAndStopApplication(self->appGlobals);
            QBWatchdogRebootAfterTimeout(self->appGlobals->watchdog, WATCHDOG_LONG_TIMEOUT_SEC, NULL);
            handled = true;
        } else if (SvStringEqualToCString(id, "TRX")) {
            if (!QBAppTypeIsHybrid()) {
                QBConfigSet("HYBRIDMODE", "HYBRID");
                QBConfigSet("USE_CABLE_MODEM", "NO");
                QBConfigSave();

                QBWatchdogReboot(self->appGlobals->watchdog, NULL);
            }
            handled = true;
        } else if (SvStringEqualToCString(id, "DVB")) {
            if (!QBAppTypeIsDVB()) {
                QBConfigSet("HYBRIDMODE", "DVB");
                QBConfigSet("USE_CABLE_MODEM", "NO");
                QBConfigSave();

                QBWatchdogReboot(self->appGlobals->watchdog, NULL);
            }
            handled = true;
        } else if (SvStringEqualToCString(id, "RSTR")) {
            SvLogError("%s():%d User reboot : call QBInitStopApplication()", __func__, __LINE__);
            QBInitStopApplication(self->appGlobals, true, "restart from menu");
            SvLogError("QBSystemSettingsMenu: RestartApplication : call QBInitStopApplication");
            QBWatchdogReboot(self->appGlobals->watchdog, NULL);
            handled = true;
        } else if (SvStringEqualToCString(id, "FormatHardDisk")) {
            FILE *f = fopen("/tmp/formatHardDisk", "w");
            if (f)
                fclose(f);
            f = fopen("/etc/vod/formatHardDisk", "w");
            if (f)
                fclose(f);
            f = fopen("/etc/vod/forceInternalDiskFormat", "w");
            if (f)
                fclose(f);

            SvLogError("QBSystemSettingsMenu: FormatHardDisk : call QBInitStopApplication");
            QBInitStopApplication(self->appGlobals, true, "format hdd");
            handled = true;
        } else if (SvStringEqualToCString(id, "CC_MANUAL")) {
            QBSystemSettingsMenuShowSelectClosedCaptionsMenu(self);
        } else if (SvStringEqualToCString(id, "CC_OFF") || SvStringEqualToCString(id, "CC_AUTOMATIC")) {
            QBSystemSettingsMenuHideSelectClosedCaptionsMenu(self);
        } else if (SvStringEqualToCString(id, "StartRCUPairing")) {
            QBSystemSettingsMenuItemRunRCUPairing(self);
            handled = true;
        }
    }

    return handled;
}

SvLocal bool
QBUserInterfaceMenuItemNodeHandleAction(QBSystemSettingsMenuItemChoice self, QBActiveTreeNode node, SvString optVal)
{
    bool handled = false;
    SvString action = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("action"));
    AudioTrackLogic audioTrackLogic =
        (AudioTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("AudioTrackLogic"));
    if (action) {
        if (SvStringEqualToCString(action, "Change audio track type")) {
            AudioTrackLogicSetupAudioTrack(audioTrackLogic);
            handled = true;
        } else if (SvStringEqualToCString(action, "Change audio language")) {
            AudioTrackLogicSetupAudioTrack(audioTrackLogic);
            handled = true;
        } else if (SvStringEqualToCString(action, "Configure upgrade logic")) {
            QBUpgradeLogicReconfigure(self->appGlobals->upgradeLogic);
            handled = true;
        } else if (SvStringEqualToCString(action, "Change time zone") && optVal) {
            setenv("TZ", SvStringCString(optVal), 1);
            tzset();
            handled = true;
        } else if (SvStringEqualToCString(action, "Reboot")) {
            QBWatchdogReboot(self->appGlobals->watchdog, NULL);
            handled = true;
        } else if (SvStringEqualToCString(action, "Enable automatic time zone")) {
            QBSystemSettingsMenuHideSelectTimeZoneMenu(self);
            handled = true;
        } else if (SvStringEqualToCString(action, "Disable automatic time zone")) {
            QBSystemSettingsMenuShowSelectTimeZoneMenu(self);
            handled = true;
        } else if (SvStringEqualToCString(action, "Network Setup")) {
            QBWindowContext ctx = QBNetworkSettingsContextCreate(self->appGlobals->res,
                                                                 self->appGlobals->scheduler,
                                                                 self->appGlobals->initLogic,
                                                                 self->appGlobals->controller,
                                                                 self->appGlobals->textRenderer,
                                                                 self->appGlobals->networkMonitor,
                                                                 false);
            QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
            SVRELEASE(ctx);
        } else if (SvStringEqualToCString(action, "Update VOD Services")) {
            // BEGIN AMERELES Update VoD Services
            QBProvidersControllerServiceUpdateVODServices(self->appGlobals->providersController);
            QBConfigurationMenuUpdateVODServicesDialogCreate(self);
            BoldInitLogicUpdateLogos(self->appGlobals->initLogic);
            // END AMERELES Update VoD Services
        }
    }

    return handled;
}

SvLocal bool
QBSystemSettingsMenuItemNodeApply(void *self_,
                                  SvObject node_,
                                  SvObject nodePath_)
{
    if (!SvObjectIsInstanceOf((SvObject) node_, QBActiveTreeNode_getType()))
        return false;

    bool handled = false;
    QBSystemSettingsMenuItemChoice self = (QBSystemSettingsMenuItemChoice) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    if (QBUserInterfaceMenuItemNodeHandleNode(self, node))
        handled = true;

    SvString optVal = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationValue"));
    if (QBUserInterfaceMenuItemNodeHandleAction(self, node, optVal))
        handled = true;


    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode((QBActiveTreeNode) node);
    if (!parent)
        return handled;

    SvString parentId = QBActiveTreeNodeGetID(parent);
    if (parentId) {
        if (SvStringEqualToCString(parentId, "SLNG") || SvStringEqualToCString(parentId, "SSNG")) {
            //update epg
            SvTimeRange timeRange;
            time_t t = SvTimeNow();
            SvTimeRangeInit(&timeRange, t - 100, t + 100);

            QBServiceRegistry registry = QBServiceRegistryGetInstance();
            QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
            SvObject playlist = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
            int count = SvInvokeInterface(SvEPGChannelView, playlist, getCount);
            int idx = 0;
            SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                 SVSTRING("SvEPGManager"));
            for (idx = 0; idx < count; ++idx) {
                SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, playlist, getByIndex, idx);
                SvEPGManagerPropagateEventsChange(epgManager, SvTVChannelGetID(channel), &timeRange, NULL);
            }
        }
    }

    SvString optName = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationName"));
    if (!optName)
        optName = (SvString) QBActiveTreeNodeGetAttribute(parent, SVSTRING("configurationName"));

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
QBSystemSettingsMenuItemNodeSelected(SvObject self_,
                                     SvObject node_,
                                     SvObject nodePath_)
{
    QBSystemSettingsMenuItemChoice self = (QBSystemSettingsMenuItemChoice) self_;
    bool handled = QBConfigurationMenuHandleItemNodeSelection(self->restrictedNodesHandler, node_, nodePath_);
    if (!handled)
        handled = QBSystemSettingsMenuItemNodeApply((void *) self_, node_, nodePath_);

    return handled;
}

SvLocal void
QBSystemSettingsMenuConfigChanged(SvObject self_, const char *key, const char *value)
{
    QBSystemSettingsMenuItemChoice self = (QBSystemSettingsMenuItemChoice) self_;

    if (key && !strcmp(key, "STANDBYTIMERTIMEOUT")) {
        QBActiveTreeNode node = QBActiveTreeFindNode(self->tree, SVSTRING("StandByTimerTimeout"));
        SvObject path = QBActiveTreeCreateNodePath(self->tree, QBActiveTreeNodeGetID(node));
        size_t count = QBActiveTreeNodeGetChildNodesCount(node);
        QBActiveTreePropagateNodesChange(self->tree, path, 0, count, NULL);
        SVRELEASE(path);
    }
}

SvLocal void
QBSystemSettingsMenuItemChoiceDestroy(void *self_)
{
    QBSystemSettingsMenuItemChoice self = self_;
    QBConfigRemoveListener((SvObject) self, "STANDBYTIMERTIMEOUT");

    SVRELEASE(self->sideMenuCtx);
    SVTESTRELEASE(self->selectTimeZoneMenu.node);
    SVTESTRELEASE(self->selectClosedCaptionsMenu.analogNode);
    SVTESTRELEASE(self->selectClosedCaptionsMenu.digitalNode);
    SVRELEASE(self->tree);

    SVTESTRELEASE(self->restrictedNodesHandler);
}

SvLocal SvType
QBSystemSettingsMenuItemChoice_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSystemSettingsMenuItemChoiceDestroy
    };

    static const struct QBMenuEventHandler_ eventHandlerMethods = {
        .nodeSelected = QBSystemSettingsMenuItemNodeSelected,
    };

    static const struct QBConfigListener_t configListenerMethods = {
        .changed = QBSystemSettingsMenuConfigChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSystemSettingsMenuItemChoice",
                            sizeof(struct QBSystemSettingsMenuItemChoice_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &eventHandlerMethods,
                            QBConfigListener_getInterface(), &configListenerMethods,
                            NULL);
    }

    return type;
}

QBSystemSettingsMenuItemChoice
QBSystemSettingsMenuItemChoiceCreate(AppGlobals appGlobals, SvWidget menu, QBActiveTree tree, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    if (!tree) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL tree passed");
        goto out;
    }

    QBSystemSettingsMenuItemChoice self = (QBSystemSettingsMenuItemChoice) SvTypeAllocateInstance(QBSystemSettingsMenuItemChoice_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBMainMenuItemController");
        goto out;
    }
    self->appGlobals = appGlobals;
    self->tree = SVRETAIN(tree);

    self->sideMenuCtx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, appGlobals->res,
                                                        SVSTRING("SideMenu"));

    self->restrictedNodesHandler =
        QBConfigurationMenuRestrictedNodesHandlerCreate(appGlobals, (void *) self,
                                                        QBSystemSettingsMenuItemNodeApply,
                                                        menu, tree, &error);
    if (!self->restrictedNodesHandler) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBConfigurationMenuRestrictedNodesHandler");
        SVRELEASE(self);
        goto out;
    }

    QBActiveTreeNode node = QBActiveTreeFindNode(tree, SVSTRING("TimeZone"));
    if (node) {
        self->selectTimeZoneMenu.node = SVRETAIN(node);
        const char *autoTZ = QBConfigGet("AUTOTZ");
        if (!autoTZ || !strcmp(autoTZ, "YES")) {
            QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(self->selectTimeZoneMenu.node), self->selectTimeZoneMenu.node, NULL);
            self->selectTimeZoneMenu.hidden = true;
        } else {
            self->selectTimeZoneMenu.hidden = false;
        }
    }

    const char *ccChannelMode = QBConfigGet("CLOSED_CAPTION_CHANNEL_MODE");
    bool hide = true;
    if (!ccChannelMode || !strcmp(ccChannelMode, "Manual"))
        hide = false;
    self->selectClosedCaptionsMenu.analogNode = SVTESTRETAIN(QBActiveTreeFindNode(tree, SVSTRING("ACC")));
    self->selectClosedCaptionsMenu.digitalNode = SVTESTRETAIN(QBActiveTreeFindNode(tree, SVSTRING("DCC")));
    if (hide)
        QBSystemSettingsMenuHideSelectClosedCaptionsMenu(self);

    if (!QBAppTypeIsPVR() && !self->appGlobals->PVRMounter && !self->appGlobals->npvrAgent) {
        // remove recoding settings node
        node = QBActiveTreeFindNode(tree, SVSTRING("Recording"));
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
    } else {
        if (!QBAppTypeIsPVR()) {
            //remove internal pvr specific nodes
            node = QBActiveTreeFindNode(tree, SVSTRING("FormatHardDisk"));
            QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
        }
        if (!QBAppTypeIsPVR() && !self->appGlobals->PVRMounter) {
            //remove pvr specific nodes
            node = QBActiveTreeFindNode(tree, SVSTRING("InstantTimeshift"));
            QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);

            node = QBActiveTreeFindNode(tree, SVSTRING("PVRStartMargin"));
            QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);

            node = QBActiveTreeFindNode(tree, SVSTRING("PVREndMargin"));
            QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);

            node = QBActiveTreeFindNode(tree, SVSTRING("PVR_QUOTA_AUTOCLEAN"));
            QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);

            node = QBActiveTreeFindNode(tree, SVSTRING("PVR_QUOTA_DEFAULT"));
            QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
        }
        if (!self->appGlobals->npvrAgent) {
            //remove npvr specific nodes
            node = QBActiveTreeFindNode(tree, SVSTRING("NPVRStartMargin"));
            QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);

            node = QBActiveTreeFindNode(tree, SVSTRING("NPVREndMargin"));
            QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
        }
    }

    QBConfigAddListener((SvObject) self, "STANDBYTIMERTIMEOUT");

    return self;
out:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

// BEGIN AMERELES Update VoD Services
SvLocal void QBConfigurationMenuUpdateVODServicesDialogCreate(void *self_)
{
    SvLogNotice("%s() UpdateVODServices feature, strarting animation dialog", __func__);
    
    QBSystemSettingsMenuItemChoice self = self_;
    QBConfigurationMenuShowUpdateVODServicesDialog(self->restrictedNodesHandler);

    QBProvidersControllerServiceSetUpdateVODServicesCount(self->appGlobals->providersController, 0);
}
// END AMERELES Update VoD Services
