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

#include "QBProvidersControllerServiceTraxis.h"
#include <stdlib.h>
#include <main.h>
#include <settings.h>
#include <Menus/menuchoice.h>
#include <QBConf.h>
#include "QBProvidersControllerService.h"
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBMenu/QBMainMenuInterface.h>

struct QBProvidersControllerServiceTraxis_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    SvHashTable traxisWebMenuNodes; // Traxis.Web catalog name -> menu node ID
    SvHashTable traxisWebMounts;    // menu node ID -> array of QBCarouselMenuItemService
};

typedef struct QBProvidersControllerServiceTraxis_t *QBProvidersControllerServiceTraxis;

SvLocal void
QBProvidersControllerServiceTraxisAddMenuNode(SvObject self_)
{
    QBProvidersControllerServiceTraxis self = (QBProvidersControllerServiceTraxis) self_;
    QBTreeIterator iter = QBActiveTreeCreatePostOrderTreeIterator(self->appGlobals->menuTree);
    QBActiveTreeNode menuNode = NULL;

    const char *configVODServiceType = QBConfigGet("PROVIDERS.VOD");
    const bool configVODServiceTypeIsTraxis = (configVODServiceType && strcmp(configVODServiceType, "traxis") == 0);
    if (!configVODServiceTypeIsTraxis)
        return;
    while ((menuNode = (QBActiveTreeNode) QBTreeIteratorGetNextNode(&iter))) {
        SvString vodServiceType = (SvString) QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("VODServiceType"));
        if (vodServiceType && SvObjectIsInstanceOf((SvObject) vodServiceType, SvString_getType())) {
            const bool vodServiceTypeIsConfig = SvStringEqualToCString(vodServiceType, "CONFIG");
            if (SvStringEqualToCString(vodServiceType, "traxis") || vodServiceTypeIsConfig) {
                SvString menuNodeID = QBActiveTreeNodeGetID(menuNode);
                SvString traxisWebCatalog = (SvString) QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("TraxisWebCatalog"));
                if (!traxisWebCatalog)
                    traxisWebCatalog = SVSTRING("");
                SvHashTableInsert(self->traxisWebMenuNodes, (SvGenericObject) traxisWebCatalog, (SvGenericObject) menuNodeID);
            }
        }
    }
}

SvLocal void
QBProvidersControllerServiceTraxisSetLanguage(QBProvidersControllerServiceTraxis self)
{
    const char *locale = QBConfigGet("LANG");
    if (locale) {
        SvString langCode = SvStringCreateWithCStringAndLength(locale, 2, NULL);
        TraxisWebSessionManagerSetLanguage(self->appGlobals->traxisWebSessionManager, langCode, NULL);
        SVRELEASE(langCode);
    }
}

SvLocal void
QBProvidersControllerServiceTraxisConfigChanged(SvObject self_, const char *key, const char *value)
{
    QBProvidersControllerServiceTraxis self = (QBProvidersControllerServiceTraxis) self_;
    QBProvidersControllerServiceTraxisSetLanguage(self);
}

SvLocal void
QBProvidersControllerServiceTraxisMountNode(QBProvidersControllerServiceTraxis self, QBActiveTreeNode node, SvString traxisWebCatalog, SvString nodeID)
{
    // pass all Traxis-related attributes to provider
    SvHashTable attrs = SvHashTableCreate(11, NULL);
    SvString attrName;
    SvIterator attrsIter = QBActiveTreeNodeAttributesIterator(node);
    while ((attrName = (SvString) SvIteratorGetNext(&attrsIter))) {
        if (strncmp(SvStringCString(attrName), "TraxisWeb", 9) == 0) {
            SvHashTableInsert(attrs, (SvGenericObject) attrName, QBActiveTreeNodeGetAttribute(node, attrName));
        } else if (strncmp(SvStringCString(attrName), "QBItemChoiceAction", 18) == 0) {
            SvHashTableInsert(attrs, (SvGenericObject) attrName, QBActiveTreeNodeGetAttribute(node, attrName));
        }
    }

    QBCarouselMenuItemService traxisWebMenu = QBEventISCarouselMenuCreate(self->appGlobals, traxisWebCatalog, attrs);
    SVRELEASE(attrs);

    if (traxisWebMenu) {
        struct QBProvidersControllerServiceMountParams_t params = {
            .carouselMenu = traxisWebMenu,
            .menuNodeID   = nodeID,
            .mounts       = self->traxisWebMounts,
            .isUnfoldable = true,
        };
        QBProvidersControllerServiceMountService(self->appGlobals->providersController, &params);
        SVRELEASE(traxisWebMenu);
        QBProvidersControllerServiceTraxisSetLanguage(self);
    } else {
        SvLogError("%s(): can't create carousel menu for Traxis.Web service!", __func__);
    }
}

SvLocal void
QBProvidersControllerServiceTraxisAddNodes(QBProvidersControllerServiceTraxis self)
{
    SvString traxisDeviceID = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Traxis);
    SvIterator it = SvHashTableKeysIterator(self->traxisWebMenuNodes);
    SvString traxisWebCatalog = NULL;
    while ((traxisWebCatalog = (SvString) SvIteratorGetNext(&it))) {
        SvString nodeID = (SvString) SvHashTableFind(self->traxisWebMenuNodes, (SvGenericObject) traxisWebCatalog);
        QBActiveTreeNode node;
        if (!nodeID || !(node = QBActiveTreeFindNode(self->appGlobals->menuTree, nodeID)))
            continue;

        bool anonymousBrowsing = QBActiveTreeNodeGetAttribute(node, SVSTRING("TraxisWebEnableAnonymousBrowsing"));

        if (!anonymousBrowsing) {
            if (traxisDeviceID)
                QBProvidersControllerServiceTraxisMountNode(self, node, traxisWebCatalog, nodeID);
            else {
                QBProvidersControllerServiceDestroyServices(self->appGlobals->providersController,
                                                            self->traxisWebMounts, self->traxisWebMenuNodes, NULL);
            }
         } else {
            SvArray currentMounts = (SvArray) SvHashTableFind(self->traxisWebMounts, (SvGenericObject) nodeID);
            if (!currentMounts || SvArrayCount(currentMounts) == 0) {
                QBProvidersControllerServiceTraxisMountNode(self, node, traxisWebCatalog, nodeID);
                continue;
            }
            SvGenericObject provider = QBCarouselMenuGetProvider((QBCarouselMenuItemService) SvArrayAt(currentMounts, 0));
            QBEventISCarouselMenuForceReload(provider);
        }
    }
}

SvLocal void
QBProvidersControllerServiceTraxisStart(SvObject self_)
{
    QBProvidersControllerServiceTraxis self = (QBProvidersControllerServiceTraxis) self_;

    QBMiddlewareManagerAddListener(self->appGlobals->middlewareManager, (SvGenericObject) self);
    QBConfigAddListener(self_, "LANG");

    QBProvidersControllerServiceTraxisAddNodes(self);
}

SvLocal void
QBProvidersControllerServiceTraxisStop(SvObject self_)
{
    QBProvidersControllerServiceTraxis self = (QBProvidersControllerServiceTraxis) self_;

    QBProvidersControllerServiceDestroyServices(self->appGlobals->providersController,
                                                self->traxisWebMounts, self->traxisWebMenuNodes, NULL);

    QBConfigRemoveListener(self_, "LANG");
    if (self->appGlobals->middlewareManager) {
        QBMiddlewareManagerRemoveListener(self->appGlobals->middlewareManager, (SvGenericObject) self);
    }
}

SvLocal void
QBProvidersControllerServiceTraxisReinitialize(SvObject self_, SvArray itemList)
{
}

SvLocal QBProvidersControllerServiceType
QBProvidersControllerServiceTraxisType(SvObject self_)
{
    return QBProvidersControllerService_TRAXIS;
}

SvLocal SvArray
QBProvidersControllerServiceTraxisGetServices(SvObject self_, SvString tag)
{
    QBProvidersControllerServiceTraxis self = (QBProvidersControllerServiceTraxis) self_;
    return (SvArray) SvHashTableFind(self->traxisWebMounts, (SvGenericObject) tag);
}

SvLocal void
QBProvidersControllerServiceMiddlewareDataChanged(SvGenericObject self_, QBMiddlewareManagerType middlewareType)
{
    QBProvidersControllerServiceTraxis self = (QBProvidersControllerServiceTraxis) self_;

    if (middlewareType == QBMiddlewareManagerType_Traxis)
        QBProvidersControllerServiceTraxisAddNodes(self);
}

SvLocal void
QBProvidersControllerServiceTraxis__dtor__(void *self_)
{
    QBProvidersControllerServiceTraxis self = self_;

    SVRELEASE(self->traxisWebMenuNodes);
    SVRELEASE(self->traxisWebMounts);
}

SvLocal void
QBProvidersControllerServiceTraxisRegisterSubmenuFactory(SvObject self_, SvString id, SvObject submenuFactory)
{
    SvLogError("%s(): not yet implemented", __func__);
}

SvLocal SvType
QBProvidersControllerServiceTraxis_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        static const struct SvObjectVTable_ objectVTable = {
            .destroy = QBProvidersControllerServiceTraxis__dtor__
        };
        static const struct QBProvidersControllerServiceChild_t serviceMethods = {
            .start                      = QBProvidersControllerServiceTraxisStart,
            .stop                       = QBProvidersControllerServiceTraxisStop,
            .addNode                    = QBProvidersControllerServiceTraxisAddMenuNode,
            .reinitialize               = QBProvidersControllerServiceTraxisReinitialize,
            .getType                    = QBProvidersControllerServiceTraxisType,
            .getServices                = QBProvidersControllerServiceTraxisGetServices,
            .registerSubmenuFactory     = QBProvidersControllerServiceTraxisRegisterSubmenuFactory,
        };
        static struct QBMiddlewareManagerListener_t middlewareIdListener = {
            .middlewareDataChanged = QBProvidersControllerServiceMiddlewareDataChanged
        };
        static struct QBConfigListener_t configListener = {
            .changed = QBProvidersControllerServiceTraxisConfigChanged,
        };

        SvTypeCreateManaged("QBProvidersControllerServiceTraxis",
                            sizeof(struct QBProvidersControllerServiceTraxis_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBProvidersControllerServiceChild_getInterface(), &serviceMethods,
                            QBMiddlewareManagerListener_getInterface(), &middlewareIdListener,
                            QBConfigListener_getInterface(), &configListener,
                            NULL);
    }
    return type;
}

SvObject
QBProvidersControllerServiceTraxisCreate(AppGlobals appGlobals)
{
    QBProvidersControllerServiceTraxis self;

    self = (QBProvidersControllerServiceTraxis) SvTypeAllocateInstance(QBProvidersControllerServiceTraxis_getType(), NULL);
    if (!self)
        return NULL;

    self->appGlobals = appGlobals;
    self->traxisWebMenuNodes = SvHashTableCreate(11, NULL);
    self->traxisWebMounts = SvHashTableCreate(11, NULL);

    return (SvObject)self;
}
