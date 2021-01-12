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

#include "QBSystemInformationMenuItemChoice.h"

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLog.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <QBShellCmds.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <settings.h>
#include <SWL/label.h>
#include <QBApplicationController.h>
#include <QBWindowContext.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <ContextMenus/QBContainerPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <Logic/UpgradeLogic.h>
#include <main.h>
#include <Services/core/cableModemMonitor.h>
#include <Windows/QBDiagnosticsWindow.h>
#include <Windows/QBHelpInfoWindow.h>
#include <Windows/QBSubscriberInfoWindow.h>
#include "QBConfigurationMenuUtils.h"
#include <QBWidgets/QBQRCodeInfo.h>
#include <Windows/QBNetworkSettingsWindow.h>
#include <Services/QBProvidersControllerService.h>
#include <Logic/InitLogic.h>

struct QBSystemInformationMenuItemChoice_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBActiveTree tree;

    QBActiveTreeNode middlewareStatusNode;

    struct {
        QBContextMenu ctx;
        SvWidget licenseLabel;
    } licenseSidemenu;

    QBConfigurationMenuRestrictedNodesHandler restrictedNodesHandler;
};


SvLocal void QBSystemInformationMenuLicenseOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBSystemInformationMenuItemChoice self = self_;

    svWidgetDestroy(self->licenseSidemenu.licenseLabel);
    self->licenseSidemenu.licenseLabel = NULL;
}

SvLocal void QBSystemInformationMenuLicenseOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBSystemInformationMenuItemChoice self = self_;

    svSettingsPushComponent("Configuration.settings");

    self->licenseSidemenu.licenseLabel = svLabelNewFromSM(self->appGlobals->res, "LicenseLabel");
    svSettingsWidgetAttach(frame, self->licenseSidemenu.licenseLabel, svWidgetGetName(self->licenseSidemenu.licenseLabel), 0);

    svSettingsPopComponent();
}

SvLocal void QBSystemInformationMenuLicenseSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBSystemInformationMenuItemChoice self = self_;
    svWidgetSetFocus(self->licenseSidemenu.licenseLabel);
}

SvLocal void QBSystemInformationMenuLicenseSideMenu(QBSystemInformationMenuItemChoice self)
{
    SVTESTRELEASE(self->licenseSidemenu.ctx);
    self->licenseSidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings",
                                                                self->appGlobals->controller,
                                                                self->appGlobals->res,
                                                                SVSTRING("SideMenu"));

    static struct QBContainerPaneCallbacks_t callbacks = {
        .onHide    = QBSystemInformationMenuLicenseOnHide,
        .onShow    = QBSystemInformationMenuLicenseOnShow,
        .setActive = QBSystemInformationMenuLicenseSetActive
    };

    QBContainerPane container = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    QBContainerPaneInit(container, self->appGlobals->res, self->licenseSidemenu.ctx, 1,
                        SVSTRING("BasicPane"), &callbacks, self);

    QBContextMenuShow(self->licenseSidemenu.ctx);
    QBContextMenuPushPane(self->licenseSidemenu.ctx, (SvObject) container);
    SVRELEASE(container);
}

SvLocal bool
QBSystemInformationMenuItemNodeHandleNode(QBSystemInformationMenuItemChoice self,
                                          QBActiveTreeNode node,
                                          SvObject nodePath)
{
    bool handled = false;
    SvString id = QBActiveTreeNodeGetID(node);
    if (id) {
        if (SvStringEqualToCString(id, "NetworkStatus")) {
            if (self->appGlobals->cableModem)
                QBCableModemMonitorPoll(self->appGlobals->cableModem);
        } else if (SvStringEqualToCString(id, "LicenseInformation")) {
            QBSystemInformationMenuLicenseSideMenu(self);
            handled = true;
        } else if (SvStringEqualToCString(id, "PushLogs")) {
            unsigned cookie = rand() % 10000;

            SvString subcaption = SvStringCreateWithFormat("Logs with cookie %i", cookie);
            QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvObject) subcaption);
            SVRELEASE(subcaption);

            SvObject parentPath = SvObjectCopy(nodePath, NULL);
            SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
            QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->tree, getIterator, parentPath, 0);
            size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
            QBActiveTreePropagateNodesChange(self->tree, parentPath, 0, nodesCount, NULL);
            SVRELEASE(parentPath);

            char *command;
            asprintf(&command, "push_logs.sh %i", cookie);
            QBShellExec(command);
            free(command);

            handled = true;
        } else if (SvStringEqualToCString(id, "RestoreDefaults")) {
            QBInitFactoryResetAndStopApplication(self->appGlobals);
            QBWatchdogRebootAfterTimeout(self->appGlobals->watchdog, WATCHDOG_LONG_TIMEOUT_SEC, NULL);
            handled = true;
        }
    }

    return handled;
}

// BEGIN AMERELES Update VoD Services
SvLocal void QBConfigurationMenuUpdateVODServicesDialogCreate(void *self_)
{
    SvLogNotice("%s() UpdateVODServices feature, strarting animation dialog", __func__);
    
    QBSystemInformationMenuItemChoice self = self_;
    QBConfigurationMenuShowUpdateVODServicesDialog(self->restrictedNodesHandler);

    QBProvidersControllerServiceSetUpdateVODServicesCount(self->appGlobals->providersController, 0);
}
// END AMERELES Update VoD Services

SvLocal bool
QBSystemInformationMenuItemNodeApply(void *self_,
                                     SvObject node_,
                                     SvObject nodePath)
{
    if (!SvObjectIsInstanceOf((SvObject) node_, QBActiveTreeNode_getType()))
        return false;

    bool handled = false;
    QBSystemInformationMenuItemChoice self = (QBSystemInformationMenuItemChoice) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    if (QBSystemInformationMenuItemNodeHandleNode(self, node, nodePath))
        handled = true;
    SvString action = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("action"));
    if (action) {
        if (SvStringEqualToCString(action, "Check software update")) {
            QBUpgradeLogicCheck(self->appGlobals->upgradeLogic);
            handled = true;
        } else if (SvStringEqualToCString(action, "Diagnostics")) {
            SvString type = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("type"));
            if (type) {
                QBWindowContext ctx = QBDiagnosticsWindowContextCreate(self->appGlobals, type);
                if (ctx) {
                    QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
                    SVRELEASE(ctx);
                }
            }
            handled = true;
        } else if (SvStringEqualToCString(action, "QRCodeInfo")) {
            QBQRCodeInfoRunDialog(self->appGlobals->res, self->appGlobals->controller, self->appGlobals->smartCardMonitor, true, NULL);
            handled = true;
        } else if (SvStringEqualToCString(action, "HelpInfo")) {
            QBWindowContext ctx = QBHelpInfoContextCreate(self->appGlobals->initLogic, self->appGlobals->textRenderer);
            if (ctx) {
                QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
                SVRELEASE(ctx);
            }
            handled = true;
        } else if (SvStringEqualToCString(action, "SubscriberInfo")) {
            QBWindowContext ctx = QBSubscriberInfoContextCreate(self->appGlobals);
            if (ctx) {
                QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
                SVRELEASE(ctx);
            }
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
            QBProvidersControllerServiceUpdateVODServices(self->appGlobals->providersController);
            QBConfigurationMenuUpdateVODServicesDialogCreate(self);
            BoldInitLogicUpdateLogos(self->appGlobals->initLogic);
        }
    }

    // refresh middleware on enter
    if (self->middlewareStatusNode && QBActiveTreeNodeGetParentNode(self->middlewareStatusNode) == node) {
        if (self->appGlobals->customerInfoMonitor)
            CubiwareMWCustomerInfoRefresh(self->appGlobals->customerInfoMonitor);
    }

    return handled;
}

SvLocal bool
QBSystemInformationMenuItemNodeSelected(SvObject self_,
                                        SvObject node_,
                                        SvObject nodePath_)
{
    QBSystemInformationMenuItemChoice self = (QBSystemInformationMenuItemChoice) self_;
    bool handled = QBConfigurationMenuHandleItemNodeSelection(self->restrictedNodesHandler, node_, nodePath_);
    if (!handled)
        handled = QBSystemInformationMenuItemNodeApply((void *) self_, node_, nodePath_);

    return handled;
}

SvLocal void
QBSystemInformationMenuItemChoiceDestroy(void *self_)
{
    QBSystemInformationMenuItemChoice self = self_;

    SVTESTRELEASE(self->middlewareStatusNode);
    SVTESTRELEASE(self->licenseSidemenu.ctx);
    SVRELEASE(self->tree);

    SVTESTRELEASE(self->restrictedNodesHandler);
}

SvLocal SvType
QBSystemInformationMenuItemChoice_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSystemInformationMenuItemChoiceDestroy
    };

    static const struct QBMenuEventHandler_ eventHandlerMethods = {
        .nodeSelected = QBSystemInformationMenuItemNodeSelected,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSystemInformationMenuItemChoice",
                            sizeof(struct QBSystemInformationMenuItemChoice_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &eventHandlerMethods,
                            NULL);
    }

    return type;
}

QBSystemInformationMenuItemChoice
QBSystemInformationMenuItemChoiceCreate(AppGlobals appGlobals, SvWidget menu, QBActiveTree tree, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    if (!tree) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL tree passed");
        goto out;
    }

    QBSystemInformationMenuItemChoice self = (QBSystemInformationMenuItemChoice) SvTypeAllocateInstance(QBSystemInformationMenuItemChoice_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBMainMenuItemController");
        goto out;
    }
    self->appGlobals = appGlobals;
    self->tree = SVRETAIN(tree);

    QBActiveTreeNode node = QBActiveTreeFindNode(tree, SVSTRING("MiddlewareStatus"));
    if (node && appGlobals->customerInfoMonitor) {
        self->middlewareStatusNode = SVRETAIN(node);
    }

    self->restrictedNodesHandler =
        QBConfigurationMenuRestrictedNodesHandlerCreate(appGlobals, (void *) self,
                                                        QBSystemInformationMenuItemNodeApply,
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
