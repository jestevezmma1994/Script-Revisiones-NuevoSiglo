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

#include "QBHotplugNotifier.h"

#include <libintl.h>

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvHashTable.h>
#include <fibers/c/fibers.h>
#include <Services/core/hotplugMounts.h>
#include <main.h>
#include <Widgets/confirmationDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Windows/mainmenu.h>
#include <QBShellCmds.h>
#include <QBMenu/QBMainMenuInterface.h>

#define QBNOTIFIER_ACTIVE_TIMER_AFTER 20000
#define QBNOTIFIER_OBJECT_NAME "QBHotplugUSBNotifierPopup"
#define QBNOTIFIER_DISKS_COUNT 10

typedef enum {
    QBHotplugNotifierState_no_state,
    QBHotplugNotifierState_formatting

} QBHotplugNotifierState;

struct QBHotplugNotifier_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvFiber fiber;
    SvFiberTimer timer;
    QBDiskInfo disk;
    SvHashTable disksStatus;
    SvWidget popup;
    int newFreshDisksCount;
    bool showPopup;
};

SvLocal void
QBHotplugNotifier__dtor__(void *self_)
{
    QBHotplugNotifier self = self_;
    if ( self->fiber ) {
        SvFiberDestroy(self->fiber);
    }
    if ( self->popup ) {
        QBDialogBreak(self->popup);
    }
    SVTESTRELEASE( self->disksStatus );
}

SvLocal bool
QBHotplugNotifierHasDisk(SvGenericObject self_, SvString diskId)
{
    QBHotplugNotifier self = (QBHotplugNotifier) self_;
    if ( !diskId || !self->disksStatus ) {
        return false;
    }
    return ( SvHashTableFind(self->disksStatus, (SvGenericObject) diskId) != NULL ? true : false );
}

SvLocal bool
QBHotplugNotifierAddDisk(SvGenericObject self_, SvString diskId, QBHotplugNotifierState state)
{
    QBHotplugNotifier self = (QBHotplugNotifier) self_;

    if (!diskId || !self->disksStatus) {
        SvLogError("%s(): invalid arguments!", __func__);
        return false;
    }
    if (QBHotplugNotifierHasDisk(self_, diskId)) {
        return false;
    }

    SvGenericObject val = (SvGenericObject) SvValueCreateWithInteger(state, NULL);
    if (val) {
        SvHashTableInsert(self->disksStatus, (SvGenericObject) diskId, val);
        SVRELEASE(val);
        return true;
    }

    return false;
}

SvLocal bool
QBHotplugNotifierRemoveDisk(SvGenericObject self_, SvString diskId)
{
    QBHotplugNotifier self = (QBHotplugNotifier) self_;

    if (!diskId || !QBHotplugNotifierHasDisk(self_, diskId)) {
        return false;
    }

    if (SvHashTableFind(self->disksStatus, (SvGenericObject) diskId)) {
        SvHashTableRemove(self->disksStatus, (SvGenericObject) diskId);
        return true;
    }

    return false;
}

SvLocal bool
QBHotplugNotifierUpdateDisk(SvGenericObject self_, SvString diskId,
    QBHotplugNotifierState state)
{
    QBHotplugNotifier self = (QBHotplugNotifier) self_;
    if ( !diskId || !QBHotplugNotifierHasDisk(self_, diskId) ) {
        return false;
    }

    SvGenericObject item = NULL;
    if ( (item = (SvGenericObject) SvStringCreate(SvStringCString(diskId), NULL)) ) {
        SvGenericObject val = (SvGenericObject) SvValueCreateWithInteger(state, NULL);
        if ( val ) {
            SvHashTableRemove(self->disksStatus, item);
            SvHashTableInsert(self->disksStatus, item, val);
            SVRELEASE(item);
            SVRELEASE(val);
            return true;
        }
        SVRELEASE(item);
    }
    return false;
}

SvLocal QBHotplugNotifierState
QBHotplugNotifierGetDiskStatus(SvGenericObject self_, SvString diskId)
{
    QBHotplugNotifier self = (QBHotplugNotifier) self_;
    SvGenericObject item = NULL;
    QBHotplugNotifierState ret = QBHotplugNotifierState_no_state;

    if ( (item = (SvGenericObject) SvStringCreate(SvStringCString(diskId), NULL)) ) {
        SvValue val = (SvValue) SvHashTableFind(self->disksStatus, item);
        if ( val ) {
            ret = SvValueGetInteger(val);
        }
        SVRELEASE(item);
    }
    return ret;
}

SvLocal void
QBHotplugNotifierDiskFoundPopupCallback(void *self_, SvWidget dialog,
    SvString buttonTag, unsigned keyCode)
{
    QBHotplugNotifier self = (QBHotplugNotifier) self_;
    self->popup = NULL;

    if( buttonTag && self->disk && self->disk->devId ) {
        if (SvStringEqualToCString(buttonTag, "OK-button")) {
            SvInvokeInterface(QBMainMenu, self->appGlobals->main,
                              switchToNode, self->disk->devId);
        }
    }
    SVTESTRELEASE(self->disk);
    self->disk = 0;
}

SvLocal void
QBHotplugNotifierShowDiskFoundPopup(SvGenericObject self_, QBDiskInfo disk)
{
    QBHotplugNotifier self = (QBHotplugNotifier) self_;
    if (!self || self->popup ) {
        return;
    }
    if ( !QBApplicationControllerCanAddGlobalWindowWithName(self->appGlobals->controller,
            SVSTRING(QBNOTIFIER_OBJECT_NAME)) ) {
        return;
    }

    char* msg = 0;
    if ( !asprintf(&msg, gettext("A new disk %s %s has been found.\nClick OK to browse available disks."),
        (disk->vendor ? SvStringCString(disk->vendor) : ""),
        (disk->model ? SvStringCString(disk->model) : "")) ) {
        return;
    }

    self->disk = SVTESTRETAIN(disk);
    QBConfirmationDialogParams_t params =
    {
        .title = gettext("Information"),
        .message = msg,
        .local = false,
        .globalName = SVSTRING(QBNOTIFIER_OBJECT_NAME),
        .focusOK = false,
        .isCancelButtonVisible = true
    };
    if ( (self->popup = QBConfirmationDialogCreate(self->appGlobals->res, &params)) ) {
        QBDialogRun(self->popup, self, QBHotplugNotifierDiskFoundPopupCallback);
    }
    free(msg);
}

SvLocal void QBHotplugNotifierMounterResult(SvGenericObject self_, QBDiskStatus disk)
{
    if ( disk && disk->disk && disk->disk->devId ) {
        if ( disk->state == QBPVRMounterState_disk_formatting ) {
            QBHotplugNotifierUpdateDisk(self_, disk->disk->devId, QBHotplugNotifierState_formatting);
        }
        if ( (disk->state == QBPVRMounterState_disk_mounted || disk->state == QBPVRMounterState_error_formatting)
            && QBHotplugNotifierHasDisk(self_, disk->disk->devId)
            && QBHotplugNotifierGetDiskStatus(self_, disk->disk->devId) == QBHotplugNotifierState_formatting ) {
            QBHotplugNotifierUpdateDisk(self_, disk->disk->devId, QBHotplugNotifierState_no_state);
        }
    }
}

SvLocal void QBHotplugNotifierDiskAdded(SvGenericObject self_, SvGenericObject path, QBDiskInfo disk)
{
}

SvLocal void QBHotplugNotifierPartitionAdded(SvGenericObject self_,
    SvGenericObject path, QBDiskInfo disk, QBPartitionInfo part)
{
    QBHotplugNotifier self = (QBHotplugNotifier) self_;
    if ( disk && !part && disk->devId ) {
        if ( !QBHotplugNotifierHasDisk(self_, disk->devId) ) {
            if ( !self->popup ) {
                self->newFreshDisksCount = 1;
                if ( self->showPopup ) {
                    QBHotplugNotifierShowDiskFoundPopup(self_, disk);
                }
            } else {
                ++self->newFreshDisksCount;
            }
            QBHotplugNotifierAddDisk(self_, disk->devId, QBHotplugNotifierState_no_state);
        }
    }
}

SvLocal void QBHotplugNotifierPartitionRemoved(SvObject self_,
                                               SvObject path, SvString remId, SvString diskId)
{
    QBHotplugNotifier self = (QBHotplugNotifier) self_;

    if (diskId && remId && SvObjectEquals((SvObject) remId, (SvObject) diskId)) {
        QBHotplugNotifierRemoveDisk(self_, diskId);
    }
    if (diskId) {
        QBPVRMounterState currDiskStatus = self->appGlobals->PVRMounter ? QBPVRMounterGetState(self->appGlobals->PVRMounter, diskId) : QBPVRMounterState_no_state;
        if (currDiskStatus != QBPVRMounterState_disk_formatting) {
            if (self->popup && self->newFreshDisksCount > 0) {
                --self->newFreshDisksCount;
            } else if (!self->popup && self->newFreshDisksCount > 0) {
                self->newFreshDisksCount = 0;
            }
        }
        if (self->popup && self->newFreshDisksCount == 0) {
            QBDialogBreak(self->popup);
        }
    }
}

SvLocal SvType
QBHotplugNotifier_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBHotplugNotifier__dtor__
    };
    static const struct QBHotplugMountAgentListener_t hotplugMethods = {
        .partitionAdded = QBHotplugNotifierPartitionAdded,
        .partitionRemoved = QBHotplugNotifierPartitionRemoved,
        .diskAdded = QBHotplugNotifierDiskAdded,
        .diskRemoved = QBHotplugNotifierPartitionRemoved
    };

    static const struct QBPVRMounterListener_t pvrMounter = {
        .mounterResultReceived = QBHotplugNotifierMounterResult
    };

    static SvType type = NULL;
    if ( unlikely(!type) ) {
        SvTypeCreateManaged("QBHotplugNotifier",
                            sizeof(struct QBHotplugNotifier_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBHotplugMountAgentListener_getInterface(), &hotplugMethods,
            QBPVRMounterListener_getInterface(), &pvrMounter,
            NULL);
    }
    return type;
}

SvLocal void
QBHotplugNotifierStep(void *self_)
{
    QBHotplugNotifier self = (QBHotplugNotifier) self_;
    self->showPopup = true;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
}

QBHotplugNotifier
QBHotplugNotifierCreate(AppGlobals appGlobals)
{
    QBHotplugNotifier self = (QBHotplugNotifier) SvTypeAllocateInstance(QBHotplugNotifier_getType(), 0);
    self->appGlobals = appGlobals;
    self->showPopup = false;
    self->fiber = NULL;
    self->timer = NULL;
    self->disksStatus = SvHashTableCreate(QBNOTIFIER_DISKS_COUNT, NULL);
    return self;
}

void
QBHotplugNotifierStart(QBHotplugNotifier self)
{
    self->newFreshDisksCount = 0;

    if ( self->fiber == NULL ) {
        self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBHotplugNotifierFiber",
            QBHotplugNotifierStep, self);
    }
    if ( self->timer == NULL ) {
        self->timer = SvFiberTimerCreate(self->fiber);
    }
    if ( self->timer != NULL ) {
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(QBNOTIFIER_ACTIVE_TIMER_AFTER));
    }
    if ( self->appGlobals && self->appGlobals->hotplugMountAgent ) {
        QBHotplugMountAgentRemoveListener(self->appGlobals->hotplugMountAgent, (SvGenericObject)self);
        QBHotplugMountAgentAddListener(self->appGlobals->hotplugMountAgent, (SvGenericObject)self);
    }
    if ( self->appGlobals && self->appGlobals->PVRMounter ) {
        QBPVRMounterRemoveListener(self->appGlobals->PVRMounter, (SvGenericObject)self);
        QBPVRMounterAddListener(self->appGlobals->PVRMounter, (SvGenericObject)self);
    }
}

void
QBHotplugNotifierStop(QBHotplugNotifier self)
{
    if ( self->fiber ) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->timer = NULL;
    }
    if ( self->appGlobals->hotplugMountAgent ) {
        QBHotplugMountAgentRemoveListener(self->appGlobals->hotplugMountAgent, (SvGenericObject)self);
    }
    if ( self->appGlobals->PVRMounter ) {
        QBPVRMounterRemoveListener(self->appGlobals->PVRMounter, (SvGenericObject)self);
    }
}

