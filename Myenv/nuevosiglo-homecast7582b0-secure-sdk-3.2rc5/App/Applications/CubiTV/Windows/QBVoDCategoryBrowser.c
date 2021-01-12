/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBVoDCategoryBrowser.h"

#include <ctype.h>

#include <CUIT/Core/app.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <QBInput/QBInputCodes.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <SWL/icon.h>
#include <SWL/clock.h>
#include <QBOSK/QBOSKKey.h>
#include <QBOSK/QBOSK.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentSeeAll.h>
#include <QBContentManager/QBContentStub.h>
#include <QBContentManager/QBContentProvider.h>
#include <Windows/QBVoDCarousel.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/QBTitle.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBOSKPane.h>
#include <DataModels/loadingProxy.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBListModel.h>
#include <QBDataModel3/QBListProxy.h>
#include <Utils/value.h>
#include <Utils/authenticators.h>
#include <Utils/QBVODUtils.h>
#include <stringUtils.h>
#include <Logic/VoDLogic.h>
#include <libintl.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBVerticalMenu.h>
#include <main.h>
#include <Logic/GUILogic.h>
#include <QBContentManager/QBLoadingStub.h>

struct QBVoDCategoryController_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBTextRenderer renderer;

    QBXMBItemConstructor itemConstructor;

    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    QBFrameConstructor* disabledFocus;

    SvRID iconRID, searchIconRID;
    QBVoDCategoryBrowserContext browserCtx;
};
typedef struct QBVoDCategoryController_ *QBVoDCategoryController;

struct QBVoDCategoryBrowserContext_ {
    struct QBWindowContext_t super_;

    AppGlobals appGlobals;
    SvGenericObject path;
    SvGenericObject provider;
    SvGenericObject dataSource;
    SvString serviceId;
    SvString serviceName;
    SvString parentCategory;
    bool pcProtectedCategory;
    bool isRootCategory;

    int position;
    QBVoDLogic vodLogic;
};

#define COVERS_CNT 6

struct QBVoDCategoryBrowser_ {
    struct SvObject_ super_;

    QBVoDCategoryBrowserContext ctx;

    SvTimerId timer;
    double showCoversDelay;

    SvWidget window;
    SvWidget menu;
    SvWidget logoIcon;
    SvWidget covers[COVERS_CNT];
    SvWidget clock;
    SvWidget parentCategory;

    SvWidget parentalPopup;
    SvGenericObject protectedCategoryPath;
    bool protectedCategoryLeaf;

    SvRID searchRID;
    SvRID seeallRID;
    SvRID emptyRID;

    QBContextMenu   contextMenu;
    QBContentSearch contentSearch;
    SvGenericObject contentSearchPath;

    QBContentCategory selectedCategory;
};
typedef struct QBVoDCategoryBrowser_ *QBVoDCategoryBrowser;

// ======================= QBVoDCategoryItemController ========================

SvLocal QBXMBItem
QBVoDCategoryControllerCreateItemData(QBVoDCategoryController self,
                                      SvGenericObject node_,
                                      SvApplication app)
{
    QBXMBItem item = QBXMBItemCreate();

    item->caption = SVSTRING("????");
    item->loadingRID = self->iconRID;
    SvString titleIconURI = NULL;

    if (SvObjectIsInstanceOf(node_, QBContentSearch_getType())) {
        item->caption = SvStringCreate(gettext("Search"), NULL);
        item->iconRID = self->searchIconRID;
    } else if (SvObjectIsInstanceOf(node_, QBContentSeeAll_getType())) {
        item->caption = SvStringCreate(gettext("See all"), NULL);
        item->iconRID = self->searchIconRID;
    } else if (SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        QBContentCategory node = (QBContentCategory) node_;
        SvString title = SvValueTryGetString((SvValue) QBContentCategoryGetAttribute(node, SVSTRING("shortTitle")));
        if (!title)
            title = QBContentCategoryGetTitle(node);

        item->caption = SVRETAIN(title);
        titleIconURI = SvValueTryGetString((SvValue) QBContentCategoryGetAttribute(node, SVSTRING("titleIconURI")));

    } else if (SvObjectIsInstanceOf(node_, QBContentStub_getType()) || SvObjectIsInstanceOf(node_, QBLoadingStub_getType())) {
        item->caption = SvStringCreate(gettext("Please wait, loading…"), NULL);
    } else if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        titleIconURI = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("icon"));
        item->caption = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("caption"));
        SVTESTRETAIN(item->caption);
    }

    item->iconURI.URI = SVTESTRETAIN(titleIconURI);

    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    item->disabledFocus = SVTESTRETAIN(self->disabledFocus);

    return item;
}

SvLocal SvWidget
QBVoDCategoryControllerCreateItem(SvGenericObject self_,
                                  SvGenericObject node_,
                                  SvGenericObject path,
                                  SvApplication app,
                                  XMBMenuState initialState)
{
    QBVoDCategoryController self = (QBVoDCategoryController) self_;

    QBXMBItem item = QBVoDCategoryControllerCreateItemData(self, node_, app);
    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);

    return ret;
}

SvLocal void
QBVoDCategoryControllerSetItemState(SvGenericObject self_,
                                    SvWidget item_,
                                    XMBMenuState state,
                                    bool isFocused)
{
    QBVoDCategoryController self = (QBVoDCategoryController) self_;

    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void
QBVoDCategoryController__dtor__(void *self_)
{
    QBVoDCategoryController self = self_;

    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVTESTRELEASE(self->disabledFocus);
}

SvLocal SvType
QBVoDCategoryController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDCategoryController__dtor__
    };
    static SvType type = NULL;

    static const struct XMBItemController_t controllerMethods = {
        .createItem = QBVoDCategoryControllerCreateItem,
        .setItemState = QBVoDCategoryControllerSetItemState
    };

    if (!type) {
        SvTypeCreateManaged("QBVoDCategoryController",
                            sizeof(struct QBVoDCategoryController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controllerMethods,
                            NULL);
    }

    return type;
}

SvLocal QBVoDCategoryController
QBVoDCategoryControllerCreate(QBVoDCategoryBrowserContext ctx)
{
    AppGlobals appGlobals = ctx->appGlobals;
    QBVoDCategoryController self = (QBVoDCategoryController)
        SvTypeAllocateInstance(QBVoDCategoryController_getType(), NULL);
    if (!self) {
        SvLogError("%s() : no memory", __func__);
        return NULL;
    }

    self->browserCtx = ctx;
    self->appGlobals = appGlobals;
    self->itemConstructor = QBXMBItemConstructorCreate("CategoryName", appGlobals->textRenderer);

    self->iconRID = svSettingsGetResourceID("CategoryName", "icon");
    self->searchIconRID = svSettingsGetResourceID("CategoryName", "searchIcon");
    self->focus = QBFrameConstructorFromSM("CategoryName.focus");
    if (svSettingsIsWidgetDefined("CategoryName.inactiveFocus"))
        self->inactiveFocus = QBFrameConstructorFromSM("CategoryName.inactiveFocus");
    if (svSettingsIsWidgetDefined("CategoryName.disabledFocus"))
        self->disabledFocus = QBFrameConstructorFromSM("CategoryName.disabledFocus");

    self->renderer = appGlobals->textRenderer;

    return self;
}

// ========================== QBVoDCategoryBrowser ============================

/**
 * XXX: HACK! Remove it when middleware will support actions.
 */
SvLocal void
QBVoDCategoryBrowserGetContentSearch(QBVoDCategoryBrowser self,
                                     QBContentSearch *search,
                                     SvGenericObject *path)
{
    SvGenericObject ds = self->ctx->dataSource;
    int len = SvInvokeInterface(QBListModel, ds, getLength);
    for (int i = 0; i < len && i < 2; ++i) {
        SvGenericObject obj = SvInvokeInterface(QBListModel, ds, getObject, i);
        if (SvObjectIsInstanceOf(obj, QBContentSearch_getType())) {
            if (search)
                *search = (QBContentSearch) obj;
            if (path)
                QBListProxyGetTreeNode((QBListProxy) ds, i, path);
            return;
        }
    }
    self->contentSearch = NULL;
    self->contentSearchPath = NULL;
}

/**
 * XXX: HACK! Remove it when middleware will support actions.
 */
SvLocal void
QBVoDCategoryBrowserGetContentSeeAll(QBVoDCategoryBrowser self,
                                     SvGenericObject *seeAll,
                                     SvGenericObject *path)
{
    SvGenericObject ds = self->ctx->dataSource;
    int len = SvInvokeInterface(QBListModel, ds, getLength);
    for (int i = 0; i < len && i < 2; ++i) {
        SvGenericObject obj = SvInvokeInterface(QBListModel, ds, getObject, i);
        if (SvObjectIsInstanceOf(obj, QBContentSeeAll_getType())) {
            if (seeAll)
                *seeAll= obj;
            if (path)
                QBListProxyGetTreeNode((QBListProxy) ds, i, path);
            return;
        }
    }
    if (seeAll)
        *seeAll = NULL;
    if (path)
        *path = NULL;
}

SvLocal void
QBVoDCategoryBrowserSideMenuClose(void *self_, QBContextMenu menu)
{
    QBVoDCategoryBrowser self = self_;
    SVTESTRELEASE(self->contextMenu);
    self->contextMenu = NULL;
}

SvLocal void
QBVoDCategoryBrowserCreateSideMenu(QBVoDCategoryBrowser self)
{
    self->contextMenu =
        QBContextMenuCreateFromSettings("ContextMenu.settings",
                                        self->ctx->appGlobals->controller,
                                        self->ctx->appGlobals->res,
                                        SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->contextMenu,
                              QBVoDCategoryBrowserSideMenuClose, self);
}

SvLocal void
QBVoDCategoryBrowserStartTimer(QBVoDCategoryBrowser self)
{
    if (self->timer) {
        svAppTimerStop(self->window->app, self->timer);
        self->timer = 0;
    }

    self->timer = svAppTimerStart(self->window->app, self->window,
                                  self->showCoversDelay, 1);
}

SvLocal void
QBVoDCategoryBrowserPushContext(QBVoDCategoryBrowser self,
                                bool leaf,
                                SvGenericObject path)
{
    AppGlobals appGlobals = self->ctx->appGlobals;
    QBWindowContext ctx;

    if (leaf) {
        ctx = QBVoDCarouselContextCreate(appGlobals, path, self->ctx->provider,
                                         self->ctx->serviceId, self->ctx->serviceName,
                                         self->ctx->pcProtectedCategory, false);
    } else {
        ctx = QBVoDCategoryBrowserContextCreate(appGlobals, path, self->ctx->provider,
                                                self->ctx->serviceId, self->ctx->serviceName,
                                                self->ctx->pcProtectedCategory,
                                                false);
    }

    if (ctx) {
        QBContentProviderStart((QBContentProvider) self->ctx->provider,
                               appGlobals->scheduler);
        QBApplicationControllerPushContext(appGlobals->controller, ctx);
        SVRELEASE(ctx);
    } else {
        SvLogError("QBVoDCategoryBrowser : No context");
    }
}

SvLocal void QBVoDCategoryBrowserCheckCategoryParentalControlPINCallback(void *self_, SvWidget dlg, SvString ret, unsigned key)
{
    QBVoDCategoryBrowser self = self_;

    if (!self->parentalPopup || !self->protectedCategoryPath) {
        SVTESTRELEASE(self->protectedCategoryPath);
        self->protectedCategoryPath = NULL;
        self->parentalPopup = NULL;
        return;
    }
    self->parentalPopup = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        // enter protected category
        QBVoDCategoryBrowserPushContext(self, self->protectedCategoryLeaf, self->protectedCategoryPath);
    }
    SVRELEASE(self->protectedCategoryPath);
    self->protectedCategoryPath = NULL;
}

SvLocal bool
QBVoDCategoryShouldBeBlocked(AppGlobals appGlobals, QBContentCategory product)
{
    if (QBAccessManagerGetAuthenticationStatus(appGlobals->accessMgr, SVSTRING("PC_VOD"), NULL) == QBAuthStatus_OK)
        return false;

    if (!QBParentalControlLogicAdultIsBlocked(appGlobals->parentalControlLogic))
        return false;

    return QBVoDUtilsIsAdult((SvObject) product);
}

SvLocal void
QBVoDCategoryBrowserCheckContextAndPush(QBVoDCategoryBrowser self,
                                bool leaf,
                                SvGenericObject path)
{
    if (!QBVoDCategoryShouldBeBlocked(self->ctx->appGlobals, self->selectedCategory)) {
        QBVoDCategoryBrowserPushContext(self, leaf, path);
        return;    // Allowed
    }

    self->ctx->pcProtectedCategory = true;
    self->protectedCategoryLeaf = leaf;
    SVTESTRELEASE(self->protectedCategoryPath);
    self->protectedCategoryPath = SVRETAIN(path);

    SvGenericObject authenticator = QBAuthenticateViaAccessManager(self->ctx->appGlobals->scheduler, self->ctx->appGlobals->accessMgr, SVSTRING("PC_VOD"));
    SvWidget master = NULL;
    svSettingsPushComponent("ParentalControl.settings");
    SvWidget dialog = QBAuthDialogCreate(self->ctx->appGlobals, authenticator, gettext("Authentication required"), gettext("Please enter parental control PIN to enter this category"), true, NULL, &master);
    svSettingsPopComponent();
    self->parentalPopup = dialog;
    QBDialogRun(dialog, self, QBVoDCategoryBrowserCheckCategoryParentalControlPINCallback);
}

SvLocal void
QBVoDCategoryBrowserOSKKeyTyped(void *self_,
                                QBOSKPane pane,
                                SvString input,
                                unsigned int layout,
                                QBOSKKey key)
{
    QBVoDCategoryBrowser self = self_;
    AppGlobals appGlobals = self->ctx->appGlobals;

    if (key->type == QBOSKKeyType_enter) {
        if (input && SvStringLength(input) > 0 && self->contentSearch) {
            int len = SvStringLength(input) + 1;
            char *stripped = malloc(len * sizeof(char));
            QBStringUtilsStripWhitespace(stripped, len, SvStringCString(input));
            SvString keyword = SvStringCreate(stripped, NULL);
            free(stripped);

            QBContentSearchStartNewSearch(self->contentSearch, keyword,
                                          appGlobals->scheduler);
            SVRELEASE(keyword);
            QBContextMenuHide(self->contextMenu, false);

            QBVoDCategoryBrowserCheckContextAndPush(self, true, self->contentSearchPath);

            SVTESTRELEASE(self->contentSearch);
            SVTESTRELEASE(self->contentSearchPath);
            self->contentSearch = NULL;
            self->contentSearchPath = NULL;
        }
    }
}

SvLocal void
QBVoDCategoryBrowserSeeAll(void *self_,
                           SvString id,
                           QBBasicPane pane,
                           QBBasicPaneItem item)
{
    QBVoDCategoryBrowser self = self_;

    SvGenericObject path;
    QBVoDCategoryBrowserGetContentSeeAll(self, NULL, &path);
    if (path)
        QBVoDCategoryBrowserCheckContextAndPush(self, true, path);
    else
        SvLogError("%s :: Can't open 'See from all'.", __func__);
}

inline SvLocal void
QBVoDCategoryBrowserHideCovers(QBVoDCategoryBrowser self)
{
    for (int i = 0; i < COVERS_CNT; ++i) {
        svWidgetSetHidden(self->covers[i], true);
        svIconSetBitmapFromRID(self->covers[i], 0, SV_RID_INVALID);
    }
}

SvLocal void
QBVoDCategoryBrowserFillCovers(QBVoDCategoryBrowser self)
{
    if (!self->selectedCategory) {
        return;
    }

    bool checkAdult = true;
    if (QBAccessManagerGetAuthenticationStatus(self->ctx->appGlobals->accessMgr, SVSTRING("PC_VOD"), NULL) == QBAuthStatus_OK)
        checkAdult = false;

    if (!QBParentalControlLogicAdultIsBlocked(self->ctx->appGlobals->parentalControlLogic))
        checkAdult = false;

    // Do not show covers from adult category when we are not authorized and PC is enabled.
    if (checkAdult && QBVoDUtilsIsAdult((SvObject) self->selectedCategory))
        return;

    // If we are not authorized and PC is enabled we shoud check each cover before showing it even if category isn't adult.
    SvArray covers = QBVODUtilsCreateArrayOfCategoryCovers(self->ctx->appGlobals, self->selectedCategory, checkAdult);
    if (!covers)
        return;

    const size_t len = SvArrayCount(covers);
    int coverIdx = 0;
    for (size_t i = 0; i < len && i < COVERS_CNT; ++i) {
        SvValue urlV = (SvValue) SvArrayObjectAtIndex(covers, i);
        SvString url = SvValueTryGetString(urlV);
        if (url) {
            svWidgetSetHidden(self->covers[coverIdx], false);
            svIconSetBitmapFromURI(self->covers[coverIdx], 0, SvStringCString(url));
            ++coverIdx;
        }
    }
    for (size_t i = coverIdx; i < COVERS_CNT; ++i) {
        svWidgetSetHidden(self->covers[i], true);
    }

    SVRELEASE(covers);
}

SvLocal void
QBVoDCategoryBrowserSetLogoIcon(QBVoDCategoryBrowser self, SvGenericObject selected)
{
    if (SvObjectIsInstanceOf(selected, QBContentSeeAll_getType())) {
        svWidgetSetHidden(self->logoIcon, false);
        svIconSetBitmapFromRID(self->logoIcon, 0, self->seeallRID);
    } else if (SvObjectIsInstanceOf(selected, QBContentSearch_getType())) {
        svWidgetSetHidden(self->logoIcon, false);
        svIconSetBitmapFromRID(self->logoIcon, 0, self->searchRID);
    } else if (SvObjectIsInstanceOf(selected, QBContentCategory_getType())) {
        QBContentCategory category = (QBContentCategory) selected;

        SVRETAIN(category);
        SVTESTRELEASE(self->selectedCategory);
        self->selectedCategory = category;
        QBVoDCategoryBrowserStartTimer(self);

        SvValue logoV = (SvValue)
            QBContentCategoryGetAttribute(category, SVSTRING("thumbnail"));
        SvString logo = SvValueTryGetString(logoV);
        if (logo) {
            svWidgetSetHidden(self->logoIcon, false);
            svIconSetBitmapFromURI(self->logoIcon, 0, SvStringCString(logo));
        } else {
            svWidgetSetHidden(self->logoIcon, true);
        }
    } else {
        svWidgetSetHidden(self->logoIcon, true);
    }
}

SvLocal void
QBVoDCategoryBrowserDisplayCategory(QBVoDCategoryBrowser self)
{
    QBVoDCategoryBrowserHideCovers(self);

    QBVoDCategoryBrowserContext ctx = self->ctx;

    int dsLength = SvInvokeInterface(QBListModel, ctx->dataSource, getLength);
    if (dsLength <= ctx->position)
       ctx->position = dsLength - 1;
    else if (ctx->position < 0)
        ctx->position = 0;

    SvGenericObject selected = SvInvokeInterface(QBListModel, ctx->dataSource, getObject, ctx->position);
    if (!SvObjectIsInstanceOf(selected, QBContentCategory_getType())) {
        svIconSwitch(self->logoIcon, 0, 0, -1.0);
        return;
    }

    QBContentCategory category = (QBContentCategory) selected;

    SVRETAIN(category);
    SVTESTRELEASE(self->selectedCategory);
    self->selectedCategory = category;

    QBVoDCategoryBrowserSetLogoIcon(self, selected);
}

SvLocal bool
QBVoDCategoryBrowserInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBVoDCategoryBrowser self = w->prv;
    AppGlobals appGlobals = self->ctx->appGlobals;

    if (e->ch == QBKEY_FUNCTION) {
        QBVoDCategoryBrowserCreateSideMenu(self);
        svSettingsPushComponent("BasicPane.settings");

        QBBasicPane options = (QBBasicPane)
            SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
        QBBasicPaneInit(options, appGlobals->res, appGlobals->scheduler,
                        appGlobals->textRenderer, self->contextMenu,
                        1, SVSTRING("BasicPane"));

        svSettingsPopComponent();

        SvString option = SvStringCreate(gettext("Search"), NULL);
        svSettingsPushComponent("OSKPane.settings");
        SVTESTRELEASE(self->contentSearch);
        SVTESTRELEASE(self->contentSearchPath);
        QBVoDCategoryBrowserGetContentSearch(self, &self->contentSearch,
                                             &self->contentSearchPath);
        SVTESTRETAIN(self->contentSearch);
        SVTESTRETAIN(self->contentSearchPath);
        QBBasicPaneAddOptionWithOSK(options, SVSTRING("Search"), option,
                                    SVSTRING("OSKPane"),
                                    QBVoDCategoryBrowserOSKKeyTyped, self);
        svSettingsPopComponent();
        SVRELEASE(option);

        option = SvStringCreate(gettext("See all"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("See all"), option,
                             QBVoDCategoryBrowserSeeAll, self);
        SVRELEASE(option);

        QBContextMenuPushPane(self->contextMenu, (SvGenericObject) options);
        QBContextMenuShow(self->contextMenu);
        SVRELEASE(options);
        return true;
    }
    return false;
}

SvLocal void
QBVoDCategoryBrowserTimerEventHandler(SvWidget w, SvTimerEvent e)
{
    QBVoDCategoryBrowser self = w->prv;

    if (e->id == self->timer) {
        self->timer = 0;
        QBVoDCategoryBrowserFillCovers(self);
    }
}

SvLocal void
QBVoDCategoryBrowserClean(SvApplication app, void *self_)
{
    QBVoDCategoryBrowser self = self_;

    SVRELEASE(self);
}

SvLocal void
QBVoDCategoryBrowser__dtor__(void *self_)
{
    QBVoDCategoryBrowser self = self_;

    if (self->contextMenu) {
        QBContextMenuSetCallbacks(self->contextMenu, NULL, NULL);
        QBContextMenuHide(self->contextMenu, true);
        SVRELEASE(self->contextMenu);
    }
    SVTESTRELEASE(self->selectedCategory);
    SVTESTRELEASE(self->contentSearch);
    SVTESTRELEASE(self->contentSearchPath);
    SVTESTRELEASE(self->protectedCategoryPath);
}

SvLocal void
QBVoDCategoryBrowserServiceNotificationNoop(SvGenericObject self_, SvString serviceId)
{
}

SvLocal void
QBVoDCategoryBrowserServiceRemoved(SvGenericObject self_, SvString serviceId)
{
    QBVoDCategoryBrowser self = (QBVoDCategoryBrowser) self_;
    if (SvStringEqualToCString(self->ctx->serviceId, SvStringCString(serviceId))) {
        QBApplicationControllerSwitchToRoot(self->ctx->appGlobals->controller);
        QBApplicationControllerPushContext(self->ctx->appGlobals->controller, self->ctx->appGlobals->main);
    }
}

SvLocal SvType
QBVoDCategoryBrowser_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDCategoryBrowser__dtor__
    };

    static const struct QBProvidersControllerServiceListener_t providersControllerServiceMethods = {
        .serviceAdded    = QBVoDCategoryBrowserServiceNotificationNoop,
        .serviceRemoved  = QBVoDCategoryBrowserServiceRemoved,
        .serviceModified = QBVoDCategoryBrowserServiceNotificationNoop
    };

    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBVoDCategoryBrowser",
                            sizeof(struct QBVoDCategoryBrowser_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBProvidersControllerServiceListener_getInterface(), &providersControllerServiceMethods,
                            NULL);
    }

    return type;
}

SvLocal void
QBVoDCategoryBrowserOnChangePosition(void *w_, const XMBMenuEvent data)
{
    QBVoDCategoryBrowser self = ((SvWidget) w_)->prv;

    self->ctx->position = data->position;

    SvGenericObject selected =
        SvInvokeInterface(QBListModel, self->ctx->dataSource, getObject, data->position);
    if (!selected)
        return;

    if (data->clicked) {
        SvGenericObject path;
        QBListProxyGetTreeNode((QBListProxy) self->ctx->dataSource,
                data->position, &path);

        if (SvObjectIsInstanceOf(selected, QBContentSearch_getType())) {
            QBContentSearch search = (QBContentSearch) selected;

            QBOSKPane oskPane = (QBOSKPane)
                SvTypeAllocateInstance(QBOSKPane_getType(), NULL);

            SVTESTRELEASE(self->contentSearch);
            SVTESTRELEASE(self->contentSearchPath);
            self->contentSearch = SVRETAIN(search);
            self->contentSearchPath = SvObjectCopy(path, NULL);

            QBVoDCategoryBrowserCreateSideMenu(self);
            svSettingsPushComponent("OSKPane.settings");
            SvErrorInfo error = NULL;
            QBOSKPaneInit(oskPane, self->ctx->appGlobals->scheduler,
                    self->contextMenu, 1, SVSTRING("OSKPane"),
                    QBVoDCategoryBrowserOSKKeyTyped, self, &error);
            svSettingsPopComponent();
            if (!error) {
                SvString keyword = QBContentSearchGetKeyword(search);
                if (keyword)
                    QBOSKPaneSetRawInput(oskPane, SvStringCString(keyword));

                QBContextMenuPushPane(self->contextMenu, (SvGenericObject) oskPane);
                QBContextMenuShow(self->contextMenu);
            } else {
                SvErrorInfoWriteLogMessage(error);
                SvErrorInfoDestroy(error);
            }
            SVRELEASE(oskPane);
        } else if (SvObjectIsInstanceOf(selected, QBContentSeeAll_getType())) {
            QBVoDCategoryBrowserPushContext(self, true, path);
        } else if (SvObjectIsInstanceOf(selected, QBContentCategory_getType())) {
            QBContentCategory category = (QBContentCategory) selected;

            bool leaf = false;
            SvValue leafV = (SvValue) QBContentCategoryGetAttribute(category, SVSTRING("leaf"));
            if (leafV && SvObjectIsInstanceOf((SvObject) leafV, SvValue_getType()) && SvValueIsInteger(leafV)) {
                leaf = SvValueGetInteger(leafV);
            } else if (leafV && SvObjectIsInstanceOf((SvObject) leafV, SvValue_getType()) && SvValueIsBoolean(leafV)) {
                leaf = SvValueGetBoolean(leafV);
            }
            QBVoDCategoryBrowserCheckContextAndPush(self, leaf, path);
        }
    } else {
        QBVoDCategoryBrowserDisplayCategory(self);
    }
}

SvLocal void
QBVoDCategoryBrowserContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBVoDCategoryBrowserContext ctx = (QBVoDCategoryBrowserContext) self_;

    QBVoDCategoryBrowser self = (QBVoDCategoryBrowser)
        SvTypeAllocateInstance(QBVoDCategoryBrowser_getType(), NULL);
    if (!self) {
        SvLogError("%s() : No memory", __func__);
        return;
    }

    svSettingsPushComponent("VoDCategoryBrowser.settings");

    SvWidget window = QBGUILogicCreateBackgroundWidget(ctx->appGlobals->guiLogic, "Window", NULL);

    window->prv = self;
    window->clean = QBVoDCategoryBrowserClean;
    svWidgetSetInputEventHandler(window, QBVoDCategoryBrowserInputEventHandler);
    svWidgetSetTimerEventHandler(window, QBVoDCategoryBrowserTimerEventHandler);

    self->ctx = ctx;
    ctx->super_.window = window;

    self->window = window;

    self->parentCategory = QBAsyncLabelNew(app, "ParentCategory", ctx->appGlobals->textRenderer);
    svSettingsWidgetAttach(window, self->parentCategory,
                           svWidgetGetName(self->parentCategory), 1);
    QBAsyncLabelSetText(self->parentCategory, ctx->parentCategory);

    self->clock = svClockNew(app, "Clock");
    svSettingsWidgetAttach(window, self->clock, svWidgetGetName(self->clock), 1);

    self->logoIcon = svIconNew(app, "CategoryLogo");
    svSettingsWidgetAttach(window, self->logoIcon, svWidgetGetName(self->logoIcon), 1);
    self->searchRID = svSettingsGetResourceID("CategoryLogo", "searchIcon");
    self->seeallRID = svSettingsGetResourceID("CategoryLogo", "seeallIcon");
    SvGenericObject selected = SvInvokeInterface(QBListModel, ctx->dataSource,
                                                 getObject, 0);
    QBVoDCategoryBrowserSetLogoIcon(self, selected);
    self->emptyRID = svSettingsGetResourceID("CategoryLogo", "empty");
    svIconSetBitmapFromRID(self->logoIcon, 0, self->emptyRID);

    char buf[8];
    for (int i = 0; i < COVERS_CNT; ++i) {
        snprintf(buf, sizeof(buf), "Cover%d", i + 1);
        if (svSettingsIsWidgetDefined(buf)) {
            self->covers[i] = svIconNew(app, buf);
            svSettingsWidgetAttach(window, self->covers[i], svWidgetGetName(self->covers[i]), 1);
        }
    }

    self->menu = XMBVerticalMenuNew(app, "VerticalMenu", NULL);
    svSettingsWidgetAttach(window, self->menu, svWidgetGetName(self->menu), 1);
    XMBVerticalMenuSetNotificationTarget(self->menu, window, QBVoDCategoryBrowserOnChangePosition);
    //XMBVerticalMenuSetBG(self->xmbMenu, bg);
    SvGenericObject controller = (SvGenericObject)
                                 QBVoDCategoryControllerCreate(ctx);
    XMBVerticalMenuConnectToDataSource(self->menu, ctx->dataSource, controller, NULL);
    SVRELEASE(controller);
    svWidgetSetFocusable(self->menu, true);
    svWidgetSetFocus(self->menu);
    XMBVerticalMenuSetPosition(self->menu, ctx->position, true, NULL);

    self->showCoversDelay = svSettingsGetDouble("Window", "showCoversDelay", 0.5);

    svSettingsPopComponent();

    self->contextMenu =
        QBContextMenuCreateFromSettings("ContextMenu.settings",
                                        ctx->appGlobals->controller,
                                        ctx->appGlobals->res,
                                        SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->contextMenu, QBVoDCategoryBrowserSideMenuClose, self);


    QBProvidersControllerServiceAddListener(ctx->appGlobals->providersController, (SvObject) self, QBProvidersControllerService_INNOV8ON);
    QBVoDCategoryBrowserDisplayCategory(self);
}

SvLocal void
QBVoDCategoryBrowserContextDestroyWindow(QBWindowContext self_)
{
    QBVoDCategoryBrowserContext self = (QBVoDCategoryBrowserContext) self_;


    QBProvidersControllerServiceRemoveListener(self->appGlobals->providersController, self->super_.window->prv, QBProvidersControllerService_INNOV8ON);
    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

// ======================== QBVoDCategoryBrowserContext =======================

bool
QBVoDCategoryBrowserContextIsParentalControlProtected(QBVoDCategoryBrowserContext self)
{
    return self->pcProtectedCategory;
}

SvLocal void
QBVoDCategoryBrowserContextDestroy(void *self_)
{
    QBVoDCategoryBrowserContext self = self_;

    if (self->isRootCategory)
        QBAccessManagerInvalidateAuthentication(self->appGlobals->accessMgr, SVSTRING("PC_VOD"), NULL);

    SVRELEASE(self->path);
    SVRELEASE(self->dataSource);
    SVRELEASE(self->provider);
    SVRELEASE(self->serviceId);
    SVRELEASE(self->serviceName);
    SVRELEASE(self->vodLogic);
    SVTESTRELEASE(self->parentCategory);
}

SvType
QBVoDCategoryBrowserContext_getType(void)
{
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_        = {
            .destroy   = QBVoDCategoryBrowserContextDestroy
        },
        .createWindow  = QBVoDCategoryBrowserContextCreateWindow,
        .destroyWindow = QBVoDCategoryBrowserContextDestroyWindow
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBVoDCategoryBrowserContext",
                            sizeof(struct QBVoDCategoryBrowserContext_),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            NULL);
    }

    return type;
}

QBWindowContext
QBVoDCategoryBrowserContextCreate(AppGlobals appGlobals,
                                  SvGenericObject path,
                                  SvGenericObject provider,
                                  SvString serviceId,
                                  SvString serviceName,
                                  bool pcProtectedCategory,
                                  bool isRootCategory)
{
    QBVoDCategoryBrowserContext self = (QBVoDCategoryBrowserContext)
        SvTypeAllocateInstance(QBVoDCategoryBrowserContext_getType(), NULL);

    self->appGlobals = appGlobals;
    self->path = SVRETAIN(path);

    QBListProxy proxy = QBListProxyCreate((SvGenericObject) appGlobals->menuTree, path, NULL);
    self->dataSource = (SvGenericObject) proxy;
    self->provider = SVRETAIN(provider);
    self->serviceId = SVRETAIN(serviceId);
    self->serviceName = SVRETAIN(serviceName);
    self->vodLogic = QBVoDLogicNew(appGlobals);

    self->pcProtectedCategory = pcProtectedCategory;
    self->isRootCategory = isRootCategory;

    SvGenericObject node = SvInvokeInterface(QBTreeModel, appGlobals->menuTree, getNode, path);
    if (node && SvObjectIsInstanceOf(node, QBContentCategory_getType())) {
        if (QBVODUtilsCategoryShouldBeBlocked(appGlobals, (QBContentCategory) node)) {
            self->pcProtectedCategory = true;
        }

        SvString category = QBContentCategoryGetName((QBContentCategory) node);
        self->parentCategory = SVTESTRETAIN(category);
    } else if (node && SvObjectIsInstanceOf(node, QBActiveTreeNode_getType())) {
        SvString category = (SvString)
            QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node, SVSTRING("caption"));
        self->parentCategory = SVTESTRETAIN(category);
    }

    return (QBWindowContext) self;
}
