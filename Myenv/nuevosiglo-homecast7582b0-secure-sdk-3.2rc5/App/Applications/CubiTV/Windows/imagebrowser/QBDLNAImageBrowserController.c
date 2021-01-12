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

#include "QBDLNAImageBrowserController.h"

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBResourceManager/SvRBBitmap.h>
#include <QBResourceManager/QBResourceManager.h>
#include <QBResourceManager/rb.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBContentManager/QBContentStub.h>
#include "main.h"
#include "Windows/imagebrowser.h"


struct QBDLNAImageBrowserController_ {
    struct SvObject_ super_;
    QBResourceManager resourceManager;
    AppGlobals appGlobals;
};

SvLocal bool QBDLNAImageBrowserControllerIsNodeImage(QBDLNAImageBrowserController self, SvGenericObject node)
{
    if (!node) {
        return false;
    } else if (SvObjectIsInstanceOf(node, QBContentStub_getType())) {
        return false;
    } else if (!SvObjectIsInstanceOf(node, SvDBRawObject_getType())) {
        return false;
    }

    SvValue typeV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) node, "type");
    if (typeV && SvValueIsString(typeV)) {
        SvString type = SvValueGetString((SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) node, "type"));
        if (!strcmp(SvStringCString(type), "image"))
            return true;
    }
    return false;
}

SvLocal SvRID QBDLNAImageBrowserControllerGetRID(SvGenericObject self_, SvGenericObject nodePath)
{
    QBDLNAImageBrowserController  self = (QBDLNAImageBrowserController) self_;
    SvGenericObject               entry = NULL;

    if (!nodePath)
        return SV_RID_INVALID;

    entry = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath);

    if (!QBDLNAImageBrowserControllerIsNodeImage(self, entry))
        return SV_RID_INVALID;


    SvString URI = SvValueGetString((SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) entry, "mediaURI"));
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
    return rid;
}

SvLocal void QBDLNAImageBrowserControllerDestroy(void *self_)
{
    QBDLNAImageBrowserController self = self_;
    SVTESTRELEASE(self->resourceManager);
}

SvLocal SvType QBDLNAImageBrowserController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDLNAImageBrowserControllerDestroy
    };
    static const struct QBImageBrowserController_t controllerMethods = {
        .getRIDFromPath = QBDLNAImageBrowserControllerGetRID
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDLNAImageBrowserController",
                            sizeof(struct QBDLNAImageBrowserController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBImageBrowserController_getInterface(), &controllerMethods,
                            NULL);
    }

    return type;
}

QBDLNAImageBrowserController QBDLNAImageBrowserControllerCreate(AppGlobals appGlobals, SvErrorInfo *errorOut)
{
    QBDLNAImageBrowserController  self;
    SvErrorInfo                 error = NULL;

    self = (QBDLNAImageBrowserController) SvTypeAllocateInstance(QBDLNAImageBrowserController_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "can't allocate QBDLNAImageBrowserController instance");
    } else {
        self->appGlobals = appGlobals;

        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        self->resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));
        SVRETAIN(self->resourceManager);
    }

    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}
