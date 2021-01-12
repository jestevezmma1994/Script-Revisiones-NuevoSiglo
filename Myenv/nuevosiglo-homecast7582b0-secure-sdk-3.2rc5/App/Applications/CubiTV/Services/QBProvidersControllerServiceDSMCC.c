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

#include "QBProvidersControllerServiceDSMCC.h"
#include "QBProvidersControllerService.h"

#include <main.h>
#include <Menus/DSMCC/DSMCCCarouselItemController.h>
#include <Menus/menuchoice.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDSMCC/QBDSMCCFeedProvider.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvType.h>


struct QBProvidersControllerServiceDSMCC_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    SvHashTable dsmccMenuNodes;      // "DSMCC" -> menu node ID
    SvHashTable dsmccMounts;         // menu node ID -> array of QBCarouselMenuItemService

    SvObject submenuFactory;
};

typedef struct QBProvidersControllerServiceDSMCC_ *QBProvidersControllerServiceDSMCC;

SvLocal void
QBProvidersControllerServiceDSMCCDestroy(void *self_)
{
    QBProvidersControllerServiceDSMCC self = self_;

    SVRELEASE(self->dsmccMenuNodes);
    SVRELEASE(self->dsmccMounts);

    SVTESTRELEASE(self->submenuFactory);
}

SvLocal void QBProvidersControllerServiceDSMCCServiceChosen(QBProvidersControllerServiceDSMCC self, bool chosen)
{
    SvString dsmccMenuNodeID = (SvString) SvHashTableFind(self->dsmccMenuNodes, (SvObject) SVSTRING("DSMCC"));
    SvArray mounts = dsmccMenuNodeID ? (SvArray) SvHashTableFind(self->dsmccMounts, (SvObject) dsmccMenuNodeID) : NULL;
    if (!mounts || SvArrayGetCount(mounts) == 0)
        return;

    QBCarouselMenuItemService dsmccMenu = (QBCarouselMenuItemService) SvArrayObjectAtIndex(mounts, 0);
    QBDSMCCFeedProvider provider = (QBDSMCCFeedProvider) QBCarouselMenuGetProvider(dsmccMenu);
    if (provider) {
        if (chosen) {
            QBContentProviderStart((QBContentProvider) provider, self->appGlobals->scheduler);
            QBContentProviderRefresh((QBContentProvider) provider, NULL, 0, false, false);
        } else {
            QBContentProviderStop((QBContentProvider) provider);
            QBContentProviderClear((QBContentProvider) provider);
        }
    }
}

SvLocal void QBProvidersControllerServiceMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBProvidersControllerServiceDSMCC self = (QBProvidersControllerServiceDSMCC) self_;

    SvObject parentPath = SvObjectCopy(nodePath_, NULL);
    int truncateLength = 1 - SvInvokeInterface(QBTreePath, parentPath, getLength);
    if (truncateLength) {
        SvInvokeInterface(QBTreePath, parentPath, truncate, truncateLength);
    }

    SvString dsmccMenuNodeID = (SvString) SvHashTableFind(self->dsmccMenuNodes, (SvObject) SVSTRING("DSMCC"));
    SvObject node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, parentPath);
    SvString nodeID = (SvString) QBActiveTreeNodeGetID((QBActiveTreeNode) node);
    if (SvObjectEquals((SvObject) nodeID, (SvObject) dsmccMenuNodeID)) {
        QBProvidersControllerServiceDSMCCServiceChosen(self, true);
    } else {
        QBProvidersControllerServiceDSMCCServiceChosen(self, false);
    }

    SVRELEASE(parentPath);
}


SvLocal void QBProvidersControllerServiceDSMCCStart(SvObject self_)
{
    QBProvidersControllerServiceDSMCC self = (QBProvidersControllerServiceDSMCC) self_;

    SvString nodeID = (SvString) SvHashTableFind(self->dsmccMenuNodes, (SvObject) SVSTRING("DSMCC"));
    if (!nodeID || !QBActiveTreeFindNode(self->appGlobals->menuTree, nodeID)) {
        SvLogWarning("%s(): can't find DSMCC menu node!", __func__);
        return;
    }

    QBCarouselMenuItemService dsmccMenu = NULL;
    if (!self->submenuFactory) {
        dsmccMenu = QBDSMCCCarouselMenuCreate(self->appGlobals, nodeID);
    } else {
        dsmccMenu = SvInvokeInterface(QBSubmenuFactory, self->submenuFactory, createSubmenu, NULL);
    }

    if (!dsmccMenu) {
        SvLogError("%s(): can't create carousel menu for DSMCC service!", __func__);
        return;
    }

    struct QBProvidersControllerServiceMountParams_t params = {
        .carouselMenu = dsmccMenu,
        .menuNodeID   = nodeID,
        .mounts       = self->dsmccMounts,
        .isUnfoldable = true,
    };
    QBProvidersControllerServiceMountService(self->appGlobals->providersController, &params);
    SVRELEASE(dsmccMenu);
    SvInvokeInterface(QBMainMenu, self->appGlobals->main, addGlobalHandler, (SvObject) self);
}

SvLocal void QBProvidersControllerServiceDSMCCStop(SvObject self_)
{
    QBProvidersControllerServiceDSMCC self = (QBProvidersControllerServiceDSMCC) self_;
    QBProvidersControllerServiceDestroyServices(self->appGlobals->providersController,
                                                self->dsmccMounts, self->dsmccMenuNodes, NULL);
}

SvLocal void QBProvidersControllerServiceDSMCCAddMenuNode(SvObject self_)
{
    QBProvidersControllerServiceDSMCC self = (QBProvidersControllerServiceDSMCC) self_;

    QBTreeIterator iter = QBActiveTreeCreatePostOrderTreeIterator(self->appGlobals->menuTree);
    QBActiveTreeNode menuNode = NULL;

    while ((menuNode = (QBActiveTreeNode) QBTreeIteratorGetNextNode(&iter))) {
        SvObject vodServiceType = QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("VODServiceType"));
        if (vodServiceType && SvObjectIsInstanceOf(vodServiceType, SvString_getType()) &&
            SvStringEqualToCString((SvString) vodServiceType, "DSMCC")) {
            SvString menuNodeID = QBActiveTreeNodeGetID(menuNode);
            SvHashTableInsert(self->dsmccMenuNodes, (SvObject) SVSTRING("DSMCC"), (SvObject) menuNodeID);
        }
    }
}

SvLocal void QBProvidersControllerServiceDSMCCReinitialize(SvObject self_, SvArray itemList)
{
}

SvLocal enum QBProvidersControllerServiceType QBProvidersControllerServiceDSMCCType(SvObject self_)
{
    return QBProvidersControllerService_DSMCC;
}

SvLocal SvArray QBProvidersControllerServiceDSMCCGetServices(SvObject self_, SvString tag)
{
    QBProvidersControllerServiceDSMCC self = (QBProvidersControllerServiceDSMCC) self_;
    return (SvArray) SvHashTableFind(self->dsmccMounts, (SvObject) tag);
}

SvLocal void QBProvidersControllerServiceDSMCCRegisterSubmenuFactory(SvObject self_, SvString id, SvObject submenuFactory)
{
    QBProvidersControllerServiceDSMCC self = (QBProvidersControllerServiceDSMCC) self_;

    SVTESTRELEASE(self->submenuFactory);
    self->submenuFactory = SVRETAIN(submenuFactory);
}


SvLocal SvType
QBProvidersControllerServiceDSMCC_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        static const struct SvObjectVTable_ objectVTable = {
            .destroy = QBProvidersControllerServiceDSMCCDestroy
        };
        static const struct QBMenuChoice_t menuMethods = {
            .choosen = QBProvidersControllerServiceMenuChoosen,
        };
        static const struct QBProvidersControllerServiceChild_t serviceMethods = {
            .start                  = QBProvidersControllerServiceDSMCCStart,
            .stop                   = QBProvidersControllerServiceDSMCCStop,
            .addNode                = QBProvidersControllerServiceDSMCCAddMenuNode,
            .reinitialize           = QBProvidersControllerServiceDSMCCReinitialize,
            .getType                = QBProvidersControllerServiceDSMCCType,
            .getServices            = QBProvidersControllerServiceDSMCCGetServices,
            .registerSubmenuFactory = QBProvidersControllerServiceDSMCCRegisterSubmenuFactory,
        };

        SvTypeCreateManaged("QBProvidersControllerServiceDSMCC",
                            sizeof(struct QBProvidersControllerServiceDSMCC_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBProvidersControllerServiceChild_getInterface(), &serviceMethods,
                            NULL);
    }
    return type;
}

SvObject
QBProvidersControllerServiceDSMCCCreate(AppGlobals appGlobals)
{
    QBProvidersControllerServiceDSMCC self = NULL;

    SvHashTable dsmccMenuNodes = NULL;
    SvHashTable dsmccMounts = NULL;

    // create members

    dsmccMenuNodes = SvHashTableCreate(3, NULL);
    if (!dsmccMenuNodes) {
        goto fini;
    }

    dsmccMounts = SvHashTableCreate(3, NULL);
    if (!dsmccMounts) {
        goto fini;
    }

    // create self

    self = (QBProvidersControllerServiceDSMCC) SvTypeAllocateInstance(QBProvidersControllerServiceDSMCC_getType(), NULL);
    if (!self) {
        goto fini;
    }

    // attach members to self

    self->appGlobals = appGlobals;
    self->dsmccMenuNodes = SVRETAIN(dsmccMenuNodes);
    self->dsmccMounts = SVRETAIN(dsmccMounts);

fini:
    SVTESTRELEASE(dsmccMenuNodes);
    SVTESTRELEASE(dsmccMounts);

    return (SvObject) self;
}
