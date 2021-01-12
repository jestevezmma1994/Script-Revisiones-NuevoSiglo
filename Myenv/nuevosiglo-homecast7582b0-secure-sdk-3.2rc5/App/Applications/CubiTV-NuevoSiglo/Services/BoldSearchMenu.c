

#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBOSKPane.h>
#include <ctype.h>
#include <Menus/menuchoice.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBMenu/QBMenu.h>
#include <settings.h>
#include <Windows/QBVoDGridBrowser.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBItemController.h>
#include <Utils/boldUtils.h>

#include "BoldSearchMenu.h"


struct BoldSearchMenuHandler_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    
    QBActiveTree menuTree;
    
    struct QBCarouselSideMenu_t {
        QBContextMenu ctx;
    } sidemenu;
};
typedef struct BoldSearchMenuHandler_t* BoldSearchMenuHandler;


SvLocal void
BoldSearchMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath, int position)
{
    BoldSearchMenuHandler self = (BoldSearchMenuHandler) self_;
    
    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        SvString id = QBActiveTreeNodeGetID((QBActiveTreeNode) node_);
        if(id)
        {
            QBCarouselMenuItemService menu = (QBCarouselMenuItemService) 
                QBProvidersControllerServiceGetServiceByID(self->appGlobals->providersController, SVSTRING("VOD"), getBoldSearchServiceID());
            if (!menu || !QBCarouselMenuPushContext(menu, QBCarouselMenuGetOwnerTree(menu)))
            {
                SvInvokeInterface(QBMainMenu, self->appGlobals->main, switchToNode, SVSTRING("VOD"));
            }
            else
            {
                self->appGlobals->vodGridBrowserInitAsSearch = true;
            }
        }
        return;
    }
}

SvLocal void
BoldSearchMenuHandlerDestroy(void *self_)
{
    BoldSearchMenuHandler self = self_;

    SVRELEASE(self->menuTree);
}

SvLocal SvType
BoldSearchMenuHandler_getType(void)
{
    static struct QBMenuChoice_t menuMethods = {
        .choosen = BoldSearchMenuChoosen
    };
    static const struct SvObjectVTable_ objectVTable =
    {
        .destroy = BoldSearchMenuHandlerDestroy
    };
    static SvType myType = NULL;

    if (unlikely(!myType))
    {
        SvTypeCreateManaged("BoldSearchMenu",
                sizeof(struct BoldSearchMenuHandler_t),
                SvObject_getType(), &myType,
                QBMenuChoice_getInterface(), &menuMethods,
                SvObject_getType(), &objectVTable,
                NULL);
    }
    return myType;
}

void BoldSearchMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvGenericObject path = NULL;
    QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("SEARCHBOLD"), &path);
    if(!path)
        return;
    
    BoldSearchMenuHandler handler;
    handler = (BoldSearchMenuHandler) SvTypeAllocateInstance(BoldSearchMenuHandler_getType(), NULL);
    handler->appGlobals = appGlobals;
    handler->menuTree = SVRETAIN(appGlobals->menuTree);
    
    handler->sidemenu.ctx =
        QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller,
                                        appGlobals->res, SVSTRING("SideMenu"));

    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) handler, NULL);
    QBTreePathMapInsert(pathMap, path, (SvGenericObject) handler, NULL);
    SVRELEASE(handler);
}

