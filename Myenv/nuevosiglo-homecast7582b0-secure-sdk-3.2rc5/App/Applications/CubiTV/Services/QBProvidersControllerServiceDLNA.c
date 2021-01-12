/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBProvidersControllerServiceDLNA.h"
#include <stdlib.h>
#include <main.h>
#include <settings.h>
#include <Menus/menuchoice.h>
#include <QBConf.h>
#include "QBProvidersControllerService.h"
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreePath.h>
#include <Menus/DLNA/dlnaCarouselItemController.h>
#include <QBContentManager/QBDLNAProvider.h>
#include <QBMenu/QBMainMenuInterface.h>

#define QB_DLNA_DISCOVERY_DELAY 1000

struct QBProvidersControllerServiceDLNA_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    SvHashTable dlnaMenuNodes;      // "DLNA" -> menu node ID
    SvHashTable dlnaMounts;         // menu node ID -> array of QBCarouselMenuItemService

    SvObject submenuFactory;
};

typedef struct QBProvidersControllerServiceDLNA_t *QBProvidersControllerServiceDLNA;

SvLocal void
QBProvidersControllerServiceDLNAServiceChosen(QBProvidersControllerServiceDLNA self, bool chosen)
{
    SvString dlnaMenuNodeID = (SvString) SvHashTableFind(self->dlnaMenuNodes, (SvGenericObject) SVSTRING("DLNA"));
    SvArray mounts = dlnaMenuNodeID ? (SvArray) SvHashTableFind(self->dlnaMounts, (SvGenericObject) dlnaMenuNodeID) : NULL;
    if (!mounts || SvArrayCount(mounts) == 0)
        return;

    QBCarouselMenuItemService dlnaMenu = (QBCarouselMenuItemService) SvArrayObjectAtIndex(mounts, 0);
    SvGenericObject provider = QBCarouselMenuGetProvider(dlnaMenu);
    if (provider) {
        if (chosen) {
            QBDLNAProviderPerformDiscoveryAfter(provider, QB_DLNA_DISCOVERY_DELAY);
        } else {
            QBDLNAProviderCancelDiscovery(provider);
        }
    }
}

SvLocal void
QBProvidersControllerServiceMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBProvidersControllerServiceDLNA self = (QBProvidersControllerServiceDLNA) self_;
    SvGenericObject parentPath = SvObjectCopy(nodePath_, NULL);
    int truncateLength = 1 - SvInvokeInterface(QBTreePath, parentPath, getLength);
    if (truncateLength) {
        SvInvokeInterface(QBTreePath, parentPath, truncate, truncateLength);
    }

    SvString dlnaMenuNodeID = (SvString) SvHashTableFind(self->dlnaMenuNodes, (SvGenericObject) SVSTRING("DLNA"));
    SvGenericObject node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, parentPath);
    SvString nodeID = (SvString) QBActiveTreeNodeGetID((QBActiveTreeNode) node);
    if (SvObjectEquals((SvObject) nodeID, (SvObject) dlnaMenuNodeID)) {
        QBProvidersControllerServiceDLNAServiceChosen(self, true);
    } else {
        QBProvidersControllerServiceDLNAServiceChosen(self, false);
    }

    SVRELEASE(parentPath);
}

SvLocal void
QBProvidersControllerServiceDLNAAddMenuNode(SvObject self_)
{
    QBProvidersControllerServiceDLNA self = (QBProvidersControllerServiceDLNA) self_;
    QBTreeIterator iter = QBActiveTreeCreatePostOrderTreeIterator(self->appGlobals->menuTree);
    QBActiveTreeNode menuNode = NULL;

    while ((menuNode = (QBActiveTreeNode) QBTreeIteratorGetNextNode(&iter))) {
        SvString vodServiceType = (SvString) QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("VODServiceType"));
        if (vodServiceType && SvObjectIsInstanceOf((SvObject) vodServiceType, SvString_getType()) &&
            SvStringEqualToCString(vodServiceType, "DLNA"))
        {
            SvString menuNodeID = QBActiveTreeNodeGetID(menuNode);
            SvHashTableInsert(self->dlnaMenuNodes, (SvGenericObject) SVSTRING("DLNA"), (SvGenericObject) menuNodeID);
        }
    }
}

SvLocal void
QBProvidersControllerServiceDLNAStart(SvObject self_)
{
    QBProvidersControllerServiceDLNA self = (QBProvidersControllerServiceDLNA) self_;

    SvString nodeID = (SvString) SvHashTableFind(self->dlnaMenuNodes, (SvGenericObject) SVSTRING("DLNA"));
    if (!nodeID || !QBActiveTreeFindNode(self->appGlobals->menuTree, nodeID)) {
        SvLogWarning("%s(): can't find DLNA menu node!", __func__);
        return;
    }

    QBCarouselMenuItemService dlnaMenu = NULL;
    if (!self->submenuFactory)
        dlnaMenu = QBDLNACarouselMenuCreate(self->appGlobals);
    else
        dlnaMenu = SvInvokeInterface(QBSubmenuFactory, self->submenuFactory, createSubmenu, NULL);

    if (dlnaMenu) {
        struct QBProvidersControllerServiceMountParams_t params = {
            .carouselMenu = dlnaMenu,
            .menuNodeID   = nodeID,
            .mounts       = self->dlnaMounts,
            .isUnfoldable = true,
        };
        QBProvidersControllerServiceMountService(self->appGlobals->providersController, &params);
        SVRELEASE(dlnaMenu);
        SvInvokeInterface(QBMainMenu, self->appGlobals->main, addGlobalHandler, (SvObject) self);
    } else {
        SvLogError("%s(): can't create carousel menu for DLNA service!", __func__);
    }
}

SvLocal void
QBProvidersControllerServiceDLNAStop(SvObject self_)
{
    QBProvidersControllerServiceDLNA self = (QBProvidersControllerServiceDLNA) self_;

    QBProvidersControllerServiceDestroyServices(self->appGlobals->providersController,
                                                self->dlnaMounts, self->dlnaMenuNodes, NULL);
}

SvLocal void
QBProvidersControllerServiceDLNAReinitialize(SvObject self_, SvArray itemList)
{
}

SvLocal QBProvidersControllerServiceType
QBProvidersControllerServiceDLNAType(SvObject self_)
{
    return QBProvidersControllerService_DLNA;
}

SvLocal SvArray
QBProvidersControllerServiceDLNAGetServices(SvObject self_, SvString tag)
{
    QBProvidersControllerServiceDLNA self = (QBProvidersControllerServiceDLNA) self_;
    return (SvArray) SvHashTableFind(self->dlnaMounts, (SvGenericObject) tag);
}

SvLocal void
QBProvidersControllerServiceDLNARegisterSubmenuFactory(SvObject self_, SvString id, SvObject submenuFactory)
{
    QBProvidersControllerServiceDLNA self = (QBProvidersControllerServiceDLNA) self_;
    SVTESTRELEASE(self->submenuFactory);
    self->submenuFactory = SVRETAIN(submenuFactory);
}

SvLocal void
QBProvidersControllerServiceDLNA__dtor__(void *self_)
{
    QBProvidersControllerServiceDLNA self = self_;

    SVRELEASE(self->dlnaMenuNodes);
    SVRELEASE(self->dlnaMounts);
    SVTESTRELEASE(self->submenuFactory);
}

SvLocal SvType
QBProvidersControllerServiceDLNA_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        static const struct SvObjectVTable_ objectVTable = {
            .destroy = QBProvidersControllerServiceDLNA__dtor__
        };
        static const struct QBMenuChoice_t menuMethods = {
            .choosen = QBProvidersControllerServiceMenuChoosen,
        };
        static const struct QBProvidersControllerServiceChild_t serviceMethods = {
            .start                      = QBProvidersControllerServiceDLNAStart,
            .stop                       = QBProvidersControllerServiceDLNAStop,
            .addNode                    = QBProvidersControllerServiceDLNAAddMenuNode,
            .reinitialize               = QBProvidersControllerServiceDLNAReinitialize,
            .getType                    = QBProvidersControllerServiceDLNAType,
            .getServices                = QBProvidersControllerServiceDLNAGetServices,
            .registerSubmenuFactory     = QBProvidersControllerServiceDLNARegisterSubmenuFactory,
        };

        SvTypeCreateManaged("QBProvidersControllerServiceDLNA",
                            sizeof(struct QBProvidersControllerServiceDLNA_t),
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
QBProvidersControllerServiceDLNACreate(AppGlobals appGlobals)
{
    QBProvidersControllerServiceDLNA self;

    self = (QBProvidersControllerServiceDLNA) SvTypeAllocateInstance(QBProvidersControllerServiceDLNA_getType(), NULL);
    if (!self)
        return NULL;

    self->appGlobals = appGlobals;
    self->dlnaMenuNodes = SvHashTableCreate(3, NULL);
    self->dlnaMounts = SvHashTableCreate(3, NULL);

    return (SvObject)self;
}
