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
/*
 * QBFSImageBrowserController.c
 *
 *  Created on: Dec 9, 2011
 *      Author: Rafał Duszyński
 */

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBResourceManager/SvRBBitmap.h>
#include <QBResourceManager/QBResourceManager.h>
#include <QBResourceManager/rb.h>
#include <QBFSEntry.h>
#include <QBFSFile.h>
#include "QBFSImageBrowserController.h"
#include "Windows/imagebrowser.h"
#include "main.h"

struct QBFSImageBrowserController_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBResourceManager resourceManager;
    QBActiveArray filter;
};

SvLocal SvRID QBFSImageBrowserControllerGetRID(SvGenericObject self_, SvGenericObject nodePath)
{
    QBFSImageBrowserController  self = (QBFSImageBrowserController) self_;
    QBFSEntry                   entry = NULL;

    if (!nodePath)
        return SV_RID_INVALID;

    if (!SvObjectIsInstanceOf(self_, QBFSImageBrowserController_getType()))
        return SV_RID_INVALID;

    entry = (QBFSEntry) SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath);

    if (!entry)
        return SV_RID_INVALID;

    if (!SvObjectIsInstanceOf((SvObject) entry, QBFSEntry_getType()))
        return SV_RID_INVALID;

    if (QBFSEntryGetType(entry) != QBFSEntryType_regularFile)
        return SV_RID_INVALID;

    if (QBFSFileGetFileType((QBFSFile) entry) != QBFSFileType_image)
        return SV_RID_INVALID;

    if (self->filter && QBActiveArrayIndexOfObject(self->filter, (SvObject)entry, NULL) == -1)
        return SV_RID_INVALID;

    SvString URI = QBFSEntryCreateFullPathString(entry);
    if (!URI) {
        return SV_RID_INVALID;
    }
    SvRID rid = SV_RID_INVALID;
    SvRBBitmap bmp = (SvRBBitmap) svRBFindItemByURI(self->resourceManager, SvStringCString(URI));

    if (bmp && SvObjectIsInstanceOf((SvObject) bmp, SvRBBitmap_getType())) {
        rid = SvRBObjectGetID((SvRBObject) bmp);
    } else if ((bmp = SvRBBitmapCreateWithURICString(SvStringCString(URI), SvBitmapType_static))) {
        SvRBBitmapSetAsync(bmp, true);
        rid = svRBAddItem(self->resourceManager, (SvRBObject) bmp, SvRBPolicy_auto);
        SVRELEASE(bmp);
    }
    SVRELEASE(URI);
    return rid;
}

SvLocal void
QBFSImageBrowserControllerDestroy(void *self_)
{
    QBFSImageBrowserController self = (QBFSImageBrowserController) self_;
    SVTESTRELEASE(self->resourceManager);
    SVTESTRELEASE(self->filter);
}

SvType QBFSImageBrowserController_getType(void)
{
    static const struct QBImageBrowserController_t controllerMethods = {
        .getRIDFromPath = QBFSImageBrowserControllerGetRID
    };
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBFSImageBrowserControllerDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBFSImageBrowserController",
                            sizeof(struct QBFSImageBrowserController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBImageBrowserController_getInterface(), &controllerMethods,
                            NULL);
    }

    return type;
}

QBFSImageBrowserController QBFSImageBrowserControllerCreate(AppGlobals appGlobals, QBActiveArray filter, SvErrorInfo *errorOut)
{
    QBFSImageBrowserController  self;
    SvErrorInfo                 error = NULL;

    self = (QBFSImageBrowserController) SvTypeAllocateInstance(QBFSImageBrowserController_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "can't allocate QBFSImageBrowserController instance");
    } else {
        self->appGlobals = appGlobals;
        self->filter = SVTESTRETAIN(filter);

        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        self->resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));
        SVRETAIN(self->resourceManager);
    }

    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}
