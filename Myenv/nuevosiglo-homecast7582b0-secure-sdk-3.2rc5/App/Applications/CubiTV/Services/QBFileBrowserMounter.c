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

#include "QBFileBrowserMounter.h"

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLog.h>
#include <Services/core/hotplugMounts.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvWeakReference.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <main.h>

SvInterface
QBFileBrowserMounterHandler_getInterface(void)
{
    static SvInterface interface = NULL;
    SvErrorInfo error = NULL;

    if (!interface) {
        SvInterfaceCreateManaged("QBFileBrowserMounterHandler",
                                 sizeof(struct QBFileBrowserMounterHandler_),
                                 NULL, &interface, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            abort();
        }
    }

    return interface;
}


struct QBFileBrowserMounter_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    SvWeakReference handler;
};

SvLocal void QBFileBrowserMounterDiskAdded(SvGenericObject self_, SvGenericObject path, QBDiskInfo disk)
{
}

SvLocal void QBFileBrowserMounterPartitionRemoved(SvGenericObject self_, SvGenericObject path, SvString remId, SvString diskId)
{
    QBFileBrowserMounter self = (QBFileBrowserMounter) self_;
    if (!self->handler) {
        SvLogWarning("%s: no handler registered", __func__);
        return;
    }

    if (!remId || !diskId)
        return;

    SvObject handler = NULL;
    if (!(handler = SvWeakReferenceTakeReferredObject(self->handler))) {
        SvLogError("%s: SvWeakReferenceTakeReferredObject(self->handler) failed", __func__);
        return;
    }

    if (!SvObjectEquals((SvObject) remId, (SvObject) diskId)) {
        QBActiveTreeNode node = QBActiveTreeFindNodeByPath(self->appGlobals->menuTree, path);
        QBPartitionInfo partInfo = (QBPartitionInfo) QBActiveTreeNodeGetAttribute(node, SVSTRING("QBPartitionInfo"));
        if (partInfo && partInfo->rootNode) {
            SvWidget menu = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
            QBTreePathMap pathMap = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getPathMap);
            SvInvokeInterface(QBFileBrowserMounterHandler, handler, menuUnregister, menu, self->appGlobals->menuTree, pathMap, path);
        }
    }

    SVRELEASE(handler);
}

SvLocal void QBFileBrowserMounterPartitionAdded(SvGenericObject self_, SvGenericObject path, QBDiskInfo disk, QBPartitionInfo part)
{
    QBFileBrowserMounter self = (QBFileBrowserMounter) self_;
    if (!self->handler) {
        SvLogWarning("%s: no handler registered", __func__);
        return;
    }

    SvObject handler = NULL;
    if (!(handler = SvWeakReferenceTakeReferredObject(self->handler))) {
        SvLogError("%s: SvWeakReferenceTakeReferredObject(self->handler) failed", __func__);
        return;
    }

    if (part && part->rootNode) {
        SvWidget menu = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
        QBTreePathMap pathMap = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getPathMap);
        SvInvokeInterface(QBFileBrowserMounterHandler, handler, menuRegister, menu, self->appGlobals->menuTree, pathMap, path, part->rootNode);
    }

    SVRELEASE(handler);
}

SvLocal void QBFileBrowserMounter__dtor__(void *self_)
{
    QBFileBrowserMounter self = self_;
    SVTESTRELEASE(self->handler);
}

SvLocal SvType QBFileBrowserMounter_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBFileBrowserMounter__dtor__
    };
    static SvType type = NULL;

    static const struct QBHotplugMountAgentListener_t hotplugMethods = {
        .partitionAdded = QBFileBrowserMounterPartitionAdded,
        .partitionRemoved = QBFileBrowserMounterPartitionRemoved,
        .diskAdded = QBFileBrowserMounterDiskAdded,
        .diskRemoved = QBFileBrowserMounterPartitionRemoved
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBFileBrowserMounter",
                            sizeof(struct QBFileBrowserMounter_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBHotplugMountAgentListener_getInterface(), &hotplugMethods,
                            NULL);
    }
    return type;
}

QBFileBrowserMounter QBFileBrowserMounterCreate(void)
{
    QBFileBrowserMounter self = (QBFileBrowserMounter) SvTypeAllocateInstance(QBFileBrowserMounter_getType(), NULL);
    return self;
}

void QBFileBrowserMounterStart(QBFileBrowserMounter self, AppGlobals appGlobals)
{
    if (!appGlobals)
        return;

    self->appGlobals = appGlobals;
    if (appGlobals->hotplugMountAgent)
        QBHotplugMountAgentAddListener(appGlobals->hotplugMountAgent, (SvGenericObject)self);
}

void QBFileBrowserMounterStop(QBFileBrowserMounter self)
{
    if (!self->appGlobals || !self->appGlobals->hotplugMountAgent)
        return;

    QBHotplugMountAgentRemoveListener(self->appGlobals->hotplugMountAgent, (SvGenericObject)self);
    self->appGlobals = NULL;
}

void QBFileBrowserMounterRegisterHandler(QBFileBrowserMounter self, SvObject handler, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    if (self->handler) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "handler already registered");
        goto out;
    }
    if (!handler) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL handler handle passed");
        goto out;
    }
    if (!SvObjectIsImplementationOf(handler, QBFileBrowserMounterHandler_getInterface())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "handler doesn't implement QBFileBrowserMounterHandler interface");
        goto out;
    }

    self->handler = SvWeakReferenceCreate(handler, &error);
    if (!self->handler) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvWeakReferenceCreate() for handler failed");
    }

out:
    SvErrorInfoPropagate(error, errorOut);
}

