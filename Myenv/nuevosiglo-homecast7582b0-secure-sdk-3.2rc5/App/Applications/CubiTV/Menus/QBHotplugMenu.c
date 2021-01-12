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

#include "QBHotplugMenu.h"

#include <libintl.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <QBDataModel3/QBTreeModel.h>
#include <Windows/newtv.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <settings.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Services/QBPVRMounter.h>
#include <Services/QBPVODStorage.h>
#include "menuchoice.h"
#include "QBHotplugMenuController.h"
#include <Menus/QBFileBrowserMenu.h>
#include <Widgets/confirmationDialog.h>
#include <Widgets/QBUSBPVRDialog.h>
#include <Windows/mainmenu.h>
#include <Hints/hintsGetter.h>
#include <Utils/authenticators.h>
#include <QBUSBPerfTester.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <QBMenu/QBMenu.h>
#include <main.h>

#define USB_PERF_TESTS_PATH "/usr/local/share/CubiTV/Configurations/usbPerfTests"

struct QBHotplugMenuHandler_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBActiveTree menuTree;
    QBActiveTreeNode currentNode;
    QBPVRMounter mounter;
    QBUSBPerfTester tester;

    SvWidget menuBar;

    SvWeakReference USBInfoDialog;
    SvString formatPartition;
    SvString usbTestDevId;

    struct QBHotplugMenuHandlerSideMenu_t {
        QBContextMenu ctx;
    } sidemenu;
};
typedef struct QBHotplugMenuHandler_t *QBHotplugMenuHandler;

SvLocal void
QBHotplugMenuHandlerDestroy(void *self_)
{
    QBHotplugMenuHandler self = self_;
    SVRELEASE(self->menuTree);
    SVRELEASE(self->sidemenu.ctx);
    SVTESTRELEASE(self->currentNode);
    SVTESTRELEASE(self->USBInfoDialog);
    SVTESTRELEASE(self->formatPartition);
    SVTESTRELEASE(self->tester);
    SVTESTRELEASE(self->usbTestDevId);
}

SvLocal void QBHotplugMenuUSBSpeedTestStart(QBHotplugMenuHandler self, SvString diskId)
{
    SVTESTRELEASE(self->tester);
    self->tester = QBUSBPerfTesterCreate(diskId, QBPVRMounterGetDataDev(self->mounter), SVSTRING(USB_PERF_TESTS_PATH));
    QBUSBPerfTesterAddListener(self->tester, (SvGenericObject) self);
    QBUSBPVRDialog USBDialog = (QBUSBPVRDialog) SvWeakReferenceTakeReferredObject(self->USBInfoDialog);
    QBUSBPVRDialogRegisterSpeedTest(USBDialog, self->tester);
    SVRELEASE(USBDialog);
    QBUSBPerfTesterStart(self->tester);
}

SvLocal QBUSBPVRDialog QBHotplugMenuGetQBUSBPVRDialog(QBHotplugMenuHandler self)
{
    QBUSBPVRDialog usbDialog = NULL;
    if (self->USBInfoDialog) {
        usbDialog = (QBUSBPVRDialog) SvWeakReferenceTakeReferredObject(self->USBInfoDialog);
    }
    if (!usbDialog) {
        SVTESTRELEASE(self->USBInfoDialog);
        self->USBInfoDialog = SvWeakReferenceCreate((SvObject) QBUSBPVRDialogCreate(self->appGlobals), NULL);
        usbDialog = (QBUSBPVRDialog) SvWeakReferenceTakeReferredObject(self->USBInfoDialog);
        if (self->formatPartition) {
            QBUSBPVRDialogSetOnlyOneParitionFormat(usbDialog);
        }
    }
    return SVAUTORELEASE(usbDialog);
}

SvLocal void QBHotplugPaneUSBSpeedTestConfirmationDialogCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBHotplugMenuHandler self = self_;
    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        QBPVRMounterAddListener(self->mounter, (SvGenericObject) self);
        SVTESTRELEASE(self->usbTestDevId);
        self->usbTestDevId = QBActiveTreeNodeGetID(self->currentNode);
        if (!self->usbTestDevId) {
            return;
        }
        SVRETAIN(self->usbTestDevId);
        QBUSBPVRDialog USBDialog = SVRETAIN(QBHotplugMenuGetQBUSBPVRDialog(self));
        SvWidget wUSBDialog = QBUSBPVRDialogGetDialog(USBDialog);
        SVRELEASE(USBDialog);
        QBDialogRun(wUSBDialog, NULL, NULL);
        QBPVRLogicSetRecLimit(self->appGlobals->pvrLogic, 0);
        QBPVRMounterEnablePVRFileSystemMounting(self->mounter, false);
        QBPVRMounterFormat(self->mounter, self->usbTestDevId, NULL);
    }
}

SvLocal void
QBHotplugPaneFormatOK(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBHotplugMenuHandler self = (QBHotplugMenuHandler) self_;

    svSettingsPushComponent("QBUSBPVRDialog.settings");
    QBConfirmationDialogParams_t params = {
        .title                 = gettext(svSettingsGetString("ConfirmationDialog", "title")),
        .message               = gettext(svSettingsGetString("ConfirmationDialog", "message")),
        .local                 = true,
        .focusOK               = true,
        .isCancelButtonVisible = true
    };
    svSettingsPopComponent();
    if (self->appGlobals->pvrLogic && QBPVRLogicIsSpeedTestEnabled(self->appGlobals->pvrLogic) && !self->formatPartition) {
        SvWidget dialog = QBConfirmationDialogCreate(self->appGlobals->res, &params);
        QBDialogRun(dialog, self, QBHotplugPaneUSBSpeedTestConfirmationDialogCallback);
    } else {
        QBUSBPVRDialog usbDialogObject = SVRETAIN(QBHotplugMenuGetQBUSBPVRDialog(self));
        SvWidget usbDialog = QBUSBPVRDialogGetDialog(usbDialogObject);
        SVRELEASE(usbDialogObject);
        QBDialogRun(usbDialog, NULL, NULL);
        QBPVRMounterFormat(self->mounter, QBActiveTreeNodeGetID(self->currentNode), self->formatPartition);
        SVTESTRELEASE(self->formatPartition);
        self->formatPartition = NULL;
    }

    QBContextMenuHide(self->sidemenu.ctx, false);
    if (!self->currentNode)
        return;
}

SvLocal void
QBHotplugPaneFormatCancel(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBHotplugMenuHandler self = (QBHotplugMenuHandler) self_;

    SVTESTRELEASE(self->formatPartition);
    self->formatPartition = NULL;
    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal void
QBHotplugPaneFormatShowConfirmationPane(QBHotplugMenuHandler self)
{
    if (!self->currentNode) {
        return;
    }
    svSettingsPushComponent("FormatConfirmPane.settings");
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 2, SVSTRING("FormatConfirmPane"));
    const char *title = svSettingsGetString("FormatConfirmPane.Title", "text");
    svSettingsPopComponent();

    SvString name = SvStringCreate(gettext(title), NULL);
    QBBasicPaneSetTitle(options, name);
    SVRELEASE(name);
    name = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("Cancel"), name, QBHotplugPaneFormatCancel, self);
    SVRELEASE(name);
    name = SvStringCreate(gettext("OK"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("OK"), name, QBHotplugPaneFormatOK, self);
    SVRELEASE(name);

    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
    QBContextMenuShow(self->sidemenu.ctx);

    SVRELEASE(options);
}

SvLocal void
QBHotplugPaneFormatPINCallback(void *ptr, SvWidget dlg, SvString ret, unsigned key)
{
    QBHotplugMenuHandler self = ptr;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        QBHotplugPaneFormatShowConfirmationPane(self);
    } else {
        QBContextMenuHide(self->sidemenu.ctx, false);
        SVTESTRELEASE(self->formatPartition);
        self->formatPartition = NULL;
    }
}

SvLocal void
QBHotplugPaneFormatPIN(QBHotplugMenuHandler self)
{
    AppGlobals appGlobals = self->appGlobals;
    SvGenericObject authenticator = NULL;
    authenticator = QBAuthenticateViaAccessManager(appGlobals->scheduler,
                                                   appGlobals->accessMgr,
                                                   SVSTRING("PC_MENU"));
    SvWidget master = NULL;
    svSettingsPushComponent("ParentalControl.settings");
    SvWidget dialog = QBAuthDialogCreate(appGlobals, authenticator,
                                         gettext("Authentication required"),
                                         0, true, NULL, &master);
    svSettingsPopComponent();


    QBDialogRun(dialog, (void *) self, QBHotplugPaneFormatPINCallback);
}

SvLocal void
QBHotplugPaneFormat(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBHotplugMenuHandler self = (QBHotplugMenuHandler) self_;
    SvValue parentalControlVal = NULL;
    bool parentalControl = false;
    if (item->metadata) {
        parentalControlVal = (SvValue) SvHashTableFind(item->metadata, (SvObject) SVSTRING("parentalControl"));
        if (parentalControlVal && SvObjectIsInstanceOf((SvObject) parentalControlVal, SvValue_getType()) && SvValueIsBoolean(parentalControlVal)) {
            parentalControl = SvValueGetBoolean(parentalControlVal);
        }
    }

    SvString partitionId = (SvString) SvHashTableFind(item->metadata, (SvGenericObject) SVSTRING("partitionId"));
    self->formatPartition = SVTESTRETAIN(partitionId);

    if (parentalControl) {
        QBHotplugPaneFormatPIN(self);
    } else {
        QBHotplugPaneFormatShowConfirmationPane(self);
    }
}

SvLocal void
QBHotplugPanePVRStop(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBHotplugMenuHandler self = (QBHotplugMenuHandler) self_;

    QBContextMenuHide(self->sidemenu.ctx, false);
    if (!self->currentNode)
        return;

    QBPVRMounterUnmount(self->mounter, QBActiveTreeNodeGetID(self->currentNode));

    if (self->appGlobals->pvodStorage)
        QBPVODStorageDisconnect(self->appGlobals->pvodStorage);
}

SvLocal void
QBHotplugPanePVRUse(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBHotplugMenuHandler self = (QBHotplugMenuHandler) self_;

    QBContextMenuHide(self->sidemenu.ctx, false);
    if (!self->currentNode)
        return;

    QBPVRMounterMount(self->mounter, QBActiveTreeNodeGetID(self->currentNode));

    if (self->appGlobals->pvodStorage)
        QBPVODStorageReconnect(self->appGlobals->pvodStorage);
}

SvLocal void
QBHotplugMenuCreatePVRPane(QBHotplugMenuHandler self, QBBasicPane options)
{
    SvString id = QBActiveTreeNodeGetID(self->currentNode);
    QBDiskStatus diskStat = QBPVRMounterGetDiskStatus(self->mounter, id);
    bool pvrUse = QBPVRMounterIsPVRUsed(self->mounter, id);
    if (pvrUse) {
        if (diskStat->otherPartitions && SvArrayCount(diskStat->otherPartitions)) {
            QBBasicPaneAddOption(options, SVSTRING("stop"), NULL, QBHotplugPanePVRStop, self);
        } else {
            if (diskStat->pvr && diskStat->ts)
                QBBasicPaneAddOption(options, SVSTRING("stop pvr/ts"), NULL, QBHotplugPanePVRStop, self);
            else if (diskStat->ts)
                QBBasicPaneAddOption(options, SVSTRING("stop ts"), NULL, QBHotplugPanePVRStop, self);
            else
                QBBasicPaneAddOption(options, SVSTRING("stop pvr"), NULL, QBHotplugPanePVRStop, self);
        }
    } else {
        if ((diskStat->otherPartitions && SvArrayCount(diskStat->otherPartitions)) ||
            diskStat->pvr || diskStat->ts) {
            //If the disk is formatted by us
            if (diskStat->pvr && diskStat->ts)
                QBBasicPaneAddOption(options, SVSTRING("use pvr/ts"), NULL, QBHotplugPanePVRUse, self);
            else if (diskStat->ts)
                QBBasicPaneAddOption(options, SVSTRING("use ts"), NULL, QBHotplugPanePVRUse, self);
            else
                QBBasicPaneAddOption(options, SVSTRING("use pvr"), NULL, QBHotplugPanePVRUse, self);


            bool otherParitionAdded = false;
            if (diskStat->otherPartitions) {
                SvIterator iter = SvArrayIterator(diskStat->otherPartitions);
                SvString partitionId;
                while ((partitionId = (SvString) SvIteratorGetNext(&iter))) {
                    SvString menuOptionId = SvStringCreateWithFormat("format %s", SvStringCString(partitionId));
                    QBBasicPaneItem option = QBBasicPaneAddOption(options, menuOptionId, NULL, QBHotplugPaneFormat, self);
                    if (option) {
                        SvHashTableInsert(option->metadata, (SvGenericObject) SVSTRING("partitionId"), (SvGenericObject) partitionId);
                        otherParitionAdded = true;
                    }

                    SVRELEASE(menuOptionId);
                }
            }

            if (diskStat->pvr || diskStat->ts) {
                if (otherParitionAdded) {
                    QBBasicPaneItem option = QBBasicPaneAddOption(options, SVSTRING("format pvr/ts"), NULL, QBHotplugPaneFormat, self);
                    if (option)
                        SvHashTableInsert(option->metadata, (SvGenericObject) SVSTRING("partitionId"), (SvGenericObject) SVSTRING("pvr/ts"));
                    QBBasicPaneAddOption(options, SVSTRING("format all"), NULL, QBHotplugPaneFormat, self);
                } else {
                    QBBasicPaneAddOption(options, SVSTRING("format"), NULL, QBHotplugPaneFormat, self);
                }
            } else if (otherParitionAdded) {
                QBBasicPaneAddOption(options, SVSTRING("format all"), NULL, QBHotplugPaneFormat, self);
            }
        } else {
            QBBasicPaneAddOption(options, SVSTRING("format all"), NULL, QBHotplugPaneFormat, self);
        }
    }
}

SvLocal void
QBHotplugMenuCreateNoPVRPane(QBHotplugMenuHandler self, QBBasicPane options)
{
    QBBasicPaneAddOption(options, SVSTRING("format all"), NULL, QBHotplugPaneFormat, self);
}

SvLocal void
QBHotplugMenuContextChoosen(SvGenericObject self_, SvGenericObject nodePath)
{
    QBHotplugMenuHandler self = (QBHotplugMenuHandler) self_;
    SvGenericObject node_;
    SvString ID = NULL;
    node_ = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath);
    if (!node_)
        return;

    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        SVTESTRELEASE(self->currentNode);
        self->currentNode = SVRETAIN((QBActiveTreeNode) node_);
        QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(self->currentNode);
        ID = QBActiveTreeNodeGetID(parent);
        if (ID && !strcmp(SvStringCString(ID), "HotplugMounts")) {
            if (self->mounter) {
                QBDiskStatus diskStat = QBPVRMounterGetDiskStatus(self->mounter, QBActiveTreeNodeGetID(self->currentNode));
                if (QBPVRLogicIsDiskPVRCompatible(diskStat->disk)) { // create pane for PVR compatible disks only
                    QBPVRMounterState status = QBPVRMounterGetState(self->mounter, QBActiveTreeNodeGetID(self->currentNode));
                    svSettingsPushComponent("HotplugOptionsPane.settings");
                    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
                    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 1, SVSTRING("HotplugOptionsPane"));
                    QBBasicPaneLoadOptionsFromFile(options, SVSTRING("QBHotplugMenu.json"));

                    svSettingsPopComponent();
                    if (QBPVRMounterIsBusy(self->mounter)) {
                        SVRELEASE(options);
                        return;
                    } else if (QBPVRMounterIsFormatted(self->mounter, QBActiveTreeNodeGetID(self->currentNode))) {
                        QBHotplugMenuCreatePVRPane(self, options);
                    } else if (status == QBPVRMounterState_disk_incompatible || status == QBPVRMounterState_error_formatting || status == QBPVRMounterState_error_mounting || status == QBPVRMounterState_error_unmounting) {
                        QBHotplugMenuCreateNoPVRPane(self, options);
                    } else {
                        SVRELEASE(options);
                        return;
                    }
                    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
                    QBContextMenuShow(self->sidemenu.ctx);
                    SVRELEASE(options);
                }
            }
        }
    }
}

SvLocal void QBHotplugMenuHandlerDiskAdded(SvGenericObject self_, SvGenericObject path, QBDiskInfo disk)
{
}

SvLocal void QBHotplugMenuHandlerPartitionRemoved(SvGenericObject self_, SvGenericObject path, SvString remId, SvString diskId)
{
    QBHotplugMenuHandler self = (QBHotplugMenuHandler) self_;

    if (self->usbTestDevId && SvObjectEquals((SvObject) remId, (SvObject) self->usbTestDevId)) {
        QBPVRMounterRemoveListener(self->mounter, (SvGenericObject) self);
        QBPVRMounterEnablePVRFileSystemMounting(self->mounter, true);
        SVRELEASE(self->usbTestDevId);
        self->usbTestDevId = NULL;
        if (self->tester)
            QBUSBPerfTesterStop(self->tester);
    }

    if (!self->currentNode || !remId)
        return;
    if (!strcmp(SvStringCString(remId), SvStringCString(QBActiveTreeNodeGetID(self->currentNode)))) {
        QBContextMenuHide(self->sidemenu.ctx, false);
        SVRELEASE(self->currentNode);
        self->currentNode = NULL;
    }
}

SvLocal void QBHotplugMenuHandlerPartitionAdded(SvGenericObject self_, SvGenericObject path, QBDiskInfo disk, QBPartitionInfo part)
{
}

SvLocal bool QBHotplugMenuNodeSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath)
{
    QBHotplugMenuHandler self = (QBHotplugMenuHandler) self_;
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return false;

    if (SvStringEqualToCString(QBActiveTreeNodeGetID((QBActiveTreeNode) node_), "HotplugMounts")) {
        return false;
    }

    if (!QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("QBPartitionInfo"))) {
        if (self->mounter) {
            QBPVRMounterState state = QBPVRMounterGetState(self->mounter, QBActiveTreeNodeGetID((QBActiveTreeNode) node_));

            if (state == QBPVRMounterState_disk_incompatible || state == QBPVRMounterState_no_state)
                return false;
            else
                return true;
        } else {
            return false;
        }
    }

    return false;
}

SvLocal SvString QBHotplugMenuGetHintsForPath(SvGenericObject self_, SvGenericObject path)
{
    QBHotplugMenuHandler self = (QBHotplugMenuHandler) self_;

    SvString nodeId = QBActiveTreeNodeGetID(QBActiveTreeFindNodeByPath(self->menuTree, path));
    if (!self->mounter || QBPVRMounterIsBusy(self->mounter) || (nodeId && SvStringEqualToCString(nodeId, "HotplugMounts")))
        return NULL;
    return SVSTRING("hotplug_hint");
}

SvLocal void QBHotplugMenuMounterResult(SvGenericObject self_, QBDiskStatus disk)
{
    QBHotplugMenuHandler self = (QBHotplugMenuHandler) self_;

    if (disk && disk->disk->devId && SvObjectEquals((SvObject) self->usbTestDevId, (SvObject) disk->disk->devId)) {
        if (disk->state == QBPVRMounterState_disk_mounted) {
            QBPVRMounterRemoveListener(self->mounter, self_);
            QBHotplugMenuUSBSpeedTestStart(self, disk->disk->devId);
        } else if (disk->state == QBPVRMounterState_disk_compatible) {
            QBPVRMounterMount(self->mounter, disk->disk->devId);
        } else if (disk->state == QBPVRMounterState_disk_incompatible) {
            QBPVRMounterMount(self->mounter, disk->disk->devId);
        }
    }
}

SvLocal void QBHotplugMenuUSBTestResultGathered(SvGenericObject self_, QBUSBPerfTestResult result)
{
}

SvLocal void QBHotplugMenuUSBTestStarted(SvGenericObject self_, QBUSBPerfTest test)
{
}

SvLocal void QBHotplugMenuUSBTestFinished(SvGenericObject self_, SvArray tests, int lastPassedTestId)
{
    QBHotplugMenuHandler self = (QBHotplugMenuHandler) self_;

    if (self->usbTestDevId) {
        if (lastPassedTestId >= 0) {
            QBPVRLogicSetRecLimit(self->appGlobals->pvrLogic, QBPVRLogicGetRecLimitFromUSBPerfTest(self->appGlobals->pvrLogic, (QBUSBPerfTest) SvArrayGetObjectAtIndex(tests, lastPassedTestId)));
        }
        QBPVRMounterEnablePVRFileSystemMounting(self->mounter, true);
        QBPVRMounterFormat(self->mounter, self->usbTestDevId, NULL);
        SVRELEASE(self->usbTestDevId);
        self->usbTestDevId = NULL;
    }
}

SvLocal SvType
QBHotplugMenuHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBHotplugMenuHandlerDestroy
    };
    static const struct QBHotplugMountAgentListener_t hotplugMethods = {
        .partitionAdded = QBHotplugMenuHandlerPartitionAdded,
        .partitionRemoved = QBHotplugMenuHandlerPartitionRemoved,
        .diskAdded = QBHotplugMenuHandlerDiskAdded,
        .diskRemoved = QBHotplugMenuHandlerPartitionRemoved
    };

    static const struct QBMenuContextChoice_t contextMethods = {
        .contextChoosen = QBHotplugMenuContextChoosen
    };

    static const struct QBMenuEventHandler_ selectMethods = {
        .nodeSelected = QBHotplugMenuNodeSelected
    };

    static const struct QBHintsGetter_t hintsMethods = {
        .getHintsForPath = QBHotplugMenuGetHintsForPath
    };

    static const struct QBPVRMounterListener_t pvrMounter = {
        .mounterResultReceived = QBHotplugMenuMounterResult
    };

    static const struct QBUSBPerfTesterListener_t usbTesterListener = {
        .testFinished     = QBHotplugMenuUSBTestResultGathered,
        .testStarted      = QBHotplugMenuUSBTestStarted,
        .allTestsFinished = QBHotplugMenuUSBTestFinished,
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBHotplugMenuHandler",
                            sizeof(struct QBHotplugMenuHandler_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuContextChoice_getInterface(), &contextMethods,
                            QBHotplugMountAgentListener_getInterface(), &hotplugMethods,
                            QBMenuEventHandler_getInterface(), &selectMethods,
                            QBHintsGetter_getInterface(), &hintsMethods,
                            QBPVRMounterListener_getInterface(), &pvrMounter,
                            QBUSBPerfTesterListener_getInterface(), &usbTesterListener,
                            NULL);
    }

    return type;
}

SvLocal void QBHotplugMenuHandlerOnSideMenuClose(void *self_, QBContextMenu ctx)
{
    QBContextMenuHide(ctx, false);
}

void
QBHotplugMenuRegister(SvWidget menuBar,
                      QBTreePathMap pathMap,
                      AppGlobals appGlobals)
{
    QBHotplugMenuHandler handler;

    if (!appGlobals->hotplugMountAgent)
        return;

    SvGenericObject path = QBActiveTreeCreateNodePath(appGlobals->menuTree, SVSTRING("HotplugMounts"));
    if (!path)
        return;

    handler = (QBHotplugMenuHandler) SvTypeAllocateInstance(QBHotplugMenuHandler_getType(), NULL);
    handler->menuBar = menuBar;
    handler->appGlobals = appGlobals;
    handler->menuTree = SVRETAIN(appGlobals->menuTree);
    handler->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));
    if (appGlobals->PVRMounter) {
        handler->mounter = appGlobals->PVRMounter;
    }
    QBContextMenuSetCallbacks(handler->sidemenu.ctx, QBHotplugMenuHandlerOnSideMenuClose, handler);
    QBHotplugMountAgentAddListener(appGlobals->hotplugMountAgent, (SvGenericObject) handler);
    QBTreePathMapInsert(pathMap, path, (SvGenericObject) handler, NULL);
    SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, path, (SvObject) handler, NULL);
    SVRELEASE(handler);

    svSettingsPushComponent("HotplugMenu.settings");
    QBHotplugMenuController hotplugController = QBHotplugMenuControllerCreate(appGlobals);
    svSettingsPopComponent();
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) hotplugController, NULL);
    SVRELEASE(hotplugController);

    SVRELEASE(path);
}
