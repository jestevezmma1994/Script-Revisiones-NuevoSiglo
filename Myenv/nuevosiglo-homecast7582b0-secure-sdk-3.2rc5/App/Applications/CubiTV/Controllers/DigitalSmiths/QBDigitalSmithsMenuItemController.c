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

#include "QBDigitalSmithsMenuItemController.h"

#include <Menus/menuchoice.h>
#include <Menus/carouselMenuPrivate.h>
#include <Utils/value.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <QBWindowContext.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBMenuController.h>
#include <XMB2/XMBItemController.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentProvider.h>
#include <SvDataBucket2/SvDBReference.h>
#include <settings.h>
#include <QBResourceManager/rb.h>
#include <QBResourceManager/SvRBBitmap.h>
#include <QBResourceManager/QBResourceManager.h>
#include <QBResourceManager/SvRBObject.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <libintl.h>
#include <main.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvEnv.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeModel.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, "QBDigitalSmithsMenuItemControllerLogLevel", "");

#define log_error(FMT, ...)                                   SvLogError(COLBEG()  "QBDigitalSmithsMenuItemController :: %s(): " FMT COLEND_COL(red), __func__, ## __VA_ARGS__)
#define log_state(FMT, ...)  do { if (env_log_level() >= 1) { SvLogNotice(COLBEG()  "QBDigitalSmithsMenuItemController :: %s(): " FMT COLEND_COL(cyan), __func__, ## __VA_ARGS__); } } while (0)
#define log_debug(FMT, ...)  do { if (env_log_level() >= 2) { SvLogNotice(COLBEG()  "QBDigitalSmithsMenuItemController :: %s(): " FMT COLEND_COL(cyan), __func__, ## __VA_ARGS__); } } while (0)

typedef struct QBDigitalSmithsMenu_ {
    struct QBCarouselMenuItemService_ super_;
} *QBDigitalSmithsMenu;

struct QBDigitalSmithsMenuItemController_ {
    struct SvObject_ super_;

    SvScheduler scheduler; ///< scheduler used to start data provider
    QBGUILogic guiLogic; ///< loginc used to create VoD context
    QBApplicationController controller; ///< application controller for pushing VoD context

    SvBitmap icon; ///< icon of the menu item
    QBFrameConstructor* focus; ///< manu item focus widget
    QBFrameConstructor* inactiveFocus; ///< menu item inactive focus widget
    QBXMBItemConstructor itemConstructor; ///< constructor used to produce XMB menu items

    QBContentProvider provider; ///< root data provider to be started when menu item is selected

    QBResourceManager resourceManager; ///< resource manager handle (used for icons)
    SvRID iconRID; ///< item icon resource id

    SvString serviceName; ///< name of the VoD service
    SvString serviceId; ///< id of the VoD service
    SvString serviceLayout; ///< layout of the VoD service
};


SvLocal void
QBDigitalSmithsMenuItemControllerDestroy(void *ptr)
{
    QBDigitalSmithsMenuItemController self = ptr;
    SVTESTRELEASE(self->icon);
    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVTESTRELEASE(self->serviceName);
    SVTESTRELEASE(self->serviceId);
    SVTESTRELEASE(self->serviceLayout);
    if (self->itemConstructor)
        QBXMBItemConstructorDestroy(self->itemConstructor);
    if (self->provider) {
        QBContentProviderStop(self->provider);
        SVRELEASE(self->provider);
    }
    SVTESTRELEASE(self->resourceManager);
}

SvLocal void
QBDigitalSmithsMenuItemControllerChoosen(SvGenericObject self_, SvGenericObject node, SvGenericObject nodePath_, int position)
{
}

SvLocal bool
QBDigitalSmithsMenuItemControllerSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath)
{
    QBDigitalSmithsMenuItemController self = (QBDigitalSmithsMenuItemController) self_;
    SvObject path = SVTESTRETAIN(nodePath);

    QBContentProviderStart(self->provider, self->scheduler);
    QBWindowContext ctx =
        QBGUILogicPrepareVoDContext(self->guiLogic,
                                    self->serviceLayout, path, (SvObject) self->provider,
                                    self->serviceId, self->serviceName, false, NULL);
    SVTESTRELEASE(path);
    if (ctx) {
        log_debug("showing VoD context %p", ctx);
        QBApplicationControllerPushContext(self->controller, ctx);
        SVRELEASE(ctx);
    } else {
        log_error("Unable to create context for menu node");
        return false;
    }

    return true;
}

SvLocal SvWidget
QBDigitalSmithsMenuItemControllerCreateSubMenu(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app)
{
    return NULL;
}

SvLocal SvWidget
QBDigitalSmithsMenuItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    log_debug("node: %p, path: %p, initialState: %d", node_, path, initialState);
    QBDigitalSmithsMenuItemController self = (QBDigitalSmithsMenuItemController) self_;


    QBXMBItem item = QBXMBItemCreate();

    item->caption = SVSTRING("????");
    item->loadingRID = self->iconRID;
    SvString titleIconURI = NULL;

    if (SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        log_debug("creating node for QBContentCategory");
        QBContentCategory node = (QBContentCategory) node_;
        SvString title = SvValueTryGetString((SvValue) QBContentCategoryGetAttribute(node, SVSTRING("shortTitle")));
        if (!title)
            title = QBContentCategoryGetTitle(node);

        item->caption = SVRETAIN(title);
        titleIconURI = SvValueTryGetString((SvValue) QBContentCategoryGetAttribute(node, SVSTRING("titleIconURI")));
    } else if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType())) {
        log_debug("creating node for SvDBRawObject");
        SvDBRawObject leaf = (SvDBRawObject) node_;
        SvString title = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue(leaf, "name"));
        if (title && !SvStringEqualToCString(title, "")) {
            item->caption = SVRETAIN(title);
        } else {
            SvDBReference parentRef = (SvDBReference) SvDBRawObjectGetAttrValue(leaf, "parent");
            if (parentRef) {
                SvDBRawObject parent = (SvDBRawObject) SvDBReferenceGetReferredObject((SvDBReference) parentRef);
                SvValue nameV = (SvValue) SvDBRawObjectGetAttrValue(parent, "name");
                if (nameV) {
                    const char *name = SvValueGetStringAsCString(nameV, NULL);
                    SvString caption = SvStringCreateWithFormat("%s %s", name, gettext("element"));
                    item->caption = SVRETAIN(caption);
                    SVRELEASE(caption);
                }
            }
        }
        titleIconURI = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue(leaf, "titleIconURI"));
    } else if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        log_debug("creating node for QBActiveTreeNode");
        titleIconURI = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("icon"));
        item->caption = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("caption"));
        SVTESTRETAIN(item->caption);
    }

    if (titleIconURI) {
        SvRBObject bmpObj;
        const char *fileName = SvStringCString(titleIconURI);
        if ((bmpObj = svRBFindItemByURI(self->resourceManager, fileName))) {
            item->iconRID = SvRBObjectGetID(bmpObj);
        } else {
            SvRBBitmap bmp = SvRBBitmapCreateWithURICString(fileName, SvBitmapType_static);
            if (bmp) {
                item->iconRID = svRBAddItem(self->resourceManager, (SvRBObject) bmp, SvRBPolicy_auto);
                SVRELEASE(bmp);
            }
        }
    }

    if (!item->iconRID)
        item->icon = SVRETAIN(self->icon);

    SvWidget ret = NULL;

    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);

    SVRELEASE(item);
    return ret;
}

SvLocal void
QBDigitalSmithsMenuItemControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    log_debug("item: %p, state: %d, isFocused: %s", item_, state, isFocused ? "true" : "false");
    QBDigitalSmithsMenuItemController self = (QBDigitalSmithsMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal SvType
QBDigitalSmithsMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDigitalSmithsMenuItemControllerDestroy
    };
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBDigitalSmithsMenuItemControllerChoosen
    };

    static const struct QBMenuEventHandler_ selectedMethods = {
        .nodeSelected = QBDigitalSmithsMenuItemControllerSelected,
    };

    static const struct XMBMenuController_t menu_controller_methods = {
        .createSubMenu = QBDigitalSmithsMenuItemControllerCreateSubMenu,
    };

    static const struct XMBItemController_t controller_methods = {
        .createItem   = QBDigitalSmithsMenuItemControllerCreateItem,
        .setItemState = QBDigitalSmithsMenuItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDigitalSmithsMenuItemController",
                            sizeof(struct QBDigitalSmithsMenuItemController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBMenuEventHandler_getInterface(), &selectedMethods,
                            XMBItemController_getInterface(), &controller_methods,
                            XMBMenuController_getInterface(), &menu_controller_methods,
                            NULL);
    }

    return type;
}

QBDigitalSmithsMenuItemController
QBDigitalSmithsMenuItemControllerCreate(AppGlobals appGlobals,
                                        QBContentProvider provider,
                                        SvString serviceId,
                                        SvString serviceName,
                                        SvString serviceType,
                                        SvString serviceLayout)
{
    if (!provider) {
        log_error("Can't create item controller without provider!");
        return NULL;
    }

    QBDigitalSmithsMenuItemController self = (QBDigitalSmithsMenuItemController) SvTypeAllocateInstance(QBDigitalSmithsMenuItemController_getType(), NULL);

    svSettingsPushComponent("VODMenu.settings");

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBResourceManager resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));

    self->scheduler = appGlobals->scheduler;
    self->guiLogic = appGlobals->guiLogic;
    self->controller = appGlobals->controller;
    self->resourceManager = SVRETAIN(resourceManager);
    self->provider = SVTESTRETAIN(provider);
    self->serviceName = SVTESTRETAIN(serviceName);
    self->serviceId = SVTESTRETAIN(serviceId);
    self->serviceLayout = SVTESTRETAIN(serviceLayout);

    self->icon = SVRETAIN(svSettingsGetBitmap("MenuItem", "icon"));
    self->iconRID = svSettingsGetResourceID("MenuItem", "icon");
    self->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        self->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    self->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);

    svSettingsPopComponent();

    log_state("created item controller");
    return self;
}
