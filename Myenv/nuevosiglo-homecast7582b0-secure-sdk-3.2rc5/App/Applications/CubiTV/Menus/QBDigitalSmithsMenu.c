/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBDigitalSmithsMenu.h"

#include "Controllers/DigitalSmiths/QBDigitalSmithsMenuItemController.h"
#include <Menus/carouselMenuPrivate.h>
#include "Services/core/QBContentManagers.h"
#include <Utils/value.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <QBContentManager/QBContentManager.h>
#include <QBContentManager/QBContentProvider.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <main.h>
#include <libintl.h>
#include <QBAppKit/QBServiceRegistry.h>

typedef struct QBDigitalSmithsMenuFactory_ {
    struct SvObject_ super_;
    SvString rootCategoryName;
    QBMWClientMenuParams mwVoDParams;
} *QBDigitalSmithsMenuFactory;

typedef struct QBDigitalSmithsCarouselMenu_ {
    struct QBCarouselMenuItemService_ super_;
} *QBDigitalSmithsCarouselMenu;

SvLocal SvType
QBDigitalSmithsCarouselMenu_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDigitalSmithsCarouselMenu",
                            sizeof(struct QBDigitalSmithsCarouselMenu_),
                            QBCarouselMenuItemService_getType(), &type,
                            NULL);
    }

    return type;
}

SvLocal QBCarouselMenuItemService
QBDigitalSmithsMenuCreateSubmenu(SvObject self_, SvObject params_)
{
    QBDigitalSmithsMenuFactory self = (QBDigitalSmithsMenuFactory) self_;
    QBMWClientMenuParams params = (QBMWClientMenuParams) params_;
    if (!params->serviceID) {
        SvLogError("%s() : Can't create menu without service ID!", __func__);
        return NULL;
    }

    QBContentManagersService contentManagers = (QBContentManagersService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                      SVSTRING("QBContentManagersService"));
    SvObject contentManager = QBContentManagersServiceFindFirst(contentManagers,
                                                                QBContentManagerType_mwClient);
    if (!contentManager) {
        SvLogError("%s() : Can't find a contentManager", __func__);
        return NULL;
    }

    SVRETAIN(contentManager);
    SvObject serviceMonitor = QBContentManagersServiceGetServiceMonitor(contentManagers,
                                                                        contentManager);
    SvObject obj = NULL;
    ssize_t n = SvInvokeInterface(QBListModel, serviceMonitor, getLength) - 1;
    while (n >= 0) {
        obj = SvInvokeInterface(QBListModel, serviceMonitor, getObject, (size_t) n);
        if (SvObjectIsInstanceOf(obj, SvDBRawObject_getType())) {
            SvObject value = SvDBRawObjectGetAttrValue((SvDBRawObject) obj, "tag");
            SvString tag = SvValueTryGetString((SvValue) value);
            if (SvObjectEquals((SvObject) tag, (SvObject) params->serviceID))
                break;
        }
        n--;
    }

    if (n < 0) {
        SvLogError("%s(): There's no tag service with given serviceID: %s", __func__,
                   SvStringCString(params->serviceID));
        SVRELEASE(contentManager);
        return NULL;
    }

    SvString rootId = SvStringCreateWithFormat("ROOT:%s", SvStringCString(params->serviceID));

    QBContentTree tree = NULL;
    if (self->rootCategoryName) {
        tree = QBContentTreeCreate(rootId, NULL);
        SvString rootContentName = SvStringCreate(gettext("Categories"), NULL);
        QBContentTreeAddContentRootCategory(tree, rootContentName, NULL);
        SVRELEASE(rootContentName);
    } else {
        tree = QBContentTreeCreate(rootId, NULL);
    }

    SVRELEASE(rootId);

    SvObject provider = SvInvokeInterface(QBContentManager, contentManager, createContentProvider,
                                          tree, NULL, obj, NULL);

    QBCarouselMenuPushSettingsComponent(params->serviceName, params->serviceType);

    QBDigitalSmithsMenuItemController
        itemController = QBDigitalSmithsMenuItemControllerCreate(params->appGlobals, (QBContentProvider) provider,
                                                                 params->serviceID,
                                                                 SVSTRING("VoD"),
                                                                 params->serviceType,
                                                                 self->mwVoDParams ? self->mwVoDParams->serviceLayout : params->serviceLayout);

    QBCarouselMenuItemService itemInfo = (QBCarouselMenuItemService)
                                         SvTypeAllocateInstance(QBDigitalSmithsCarouselMenu_getType(), NULL);

    struct QBCarouselMenuInitParams_s initParams = {
        .serviceName    = (self->mwVoDParams && self->mwVoDParams->serviceName) ? self->mwVoDParams->serviceName : params->serviceName,
        .serviceType    = params->serviceType,
        .serviceID      = params->serviceID,
        .serviceLayout  = (self->mwVoDParams && self->mwVoDParams->serviceLayout) ? self->mwVoDParams->serviceLayout : params->serviceLayout,
        .provider       = provider,
        .itemController = (SvObject) itemController,
        .itemChoice     = (SvObject) itemController,
        .contentTree    = (SvObject) tree,
    };

    QBCarouselMenuInitWithParams(itemInfo, params->appGlobals, &initParams);

    SVRELEASE(contentManager);
    SVRELEASE(provider);
    SVRELEASE(itemController);
    SVRELEASE(tree);

    return itemInfo;
}


SvLocal void
QBDigitalSmithsMenuFactoryDestroy(void *self_)
{
    QBDigitalSmithsMenuFactory self = (QBDigitalSmithsMenuFactory) self_;
    SVTESTRELEASE(self->rootCategoryName);
    SVTESTRELEASE(self->mwVoDParams);
}

SvLocal SvType
QBDigitalSmithsMenuFactory_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBDigitalSmithsMenuFactoryDestroy
    };

    static const struct QBSubmenuFactory_ methods = {
        .createSubmenu = QBDigitalSmithsMenuCreateSubmenu
    };

    if (!type) {
        SvTypeCreateManaged("QBDigitalSmithsMenuFactory",
                            sizeof(struct QBDigitalSmithsMenuFactory_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &vtable,
                            QBSubmenuFactory_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvObject
QBDigitalSmithsMenuFactoryCreate(SvString rootCategoryName, QBMWClientMenuParams mwVoDParams, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBDigitalSmithsMenuFactory self = (QBDigitalSmithsMenuFactory) SvTypeAllocateInstance(QBDigitalSmithsMenuFactory_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "can't allocate QBDigitalSmithsMenuFactory");
    } else {
        self->rootCategoryName = SVTESTRETAIN(rootCategoryName);
        self->mwVoDParams = SVTESTRETAIN(mwVoDParams);
    }

    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : (SvObject) self;
}
