/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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

#include "innov8onCarouselItemChoice.h"

#include <QBCarousel/QBCarousel.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentSeeAll.h>
#include <QBContentManager/QBContentTreePath.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <QBContentManager/QBContentProviderListener.h>
#include <QBContentManager/Innov8onUtils.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBOSK/QBOSKKey.h>
#include <settings.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBVoDMenu.h>
#include <ContextMenus/QBCall2Action/QBCall2ActionContext.h>
#include <ContextMenus/QBOSKPane.h>
#include <Menus/menuchoice.h>
#include <Utils/authenticators.h>
#include <Utils/QBVODUtils.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/XMBCarousel.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <QBWidgets/QBGrid.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <QBWidgets/QBSearchProgressDialog.h>
#include <Widgets/authDialog.h>
#include <Widgets/movieInfo.h>
#include <Windows/mainmenu.h>
#include <Windows/pvrplayer.h>
#include <Windows/imagebrowser.h>
#include <Logic/AnyPlayerLogic.h>
#include <Logic/PurchaseLogic.h>
#include <Logic/GUILogic.h>
#include <player_hints/http_input.h>
#include <QBPlayerControllers/QBVoDAnyPlayerController.h>
#include <QBPlayerControllers/QBPlaylistController.h>
#include <QBPlayerControllers/QBPlaybackStateController.h>
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include <ctype.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <SWL/button.h>
#include <stdbool.h>
#include <main.h>
#include <libintl.h>

struct QBInnov8onCarouselMenuChoice_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    SvObject provider;
    SvObject tree;

    SvWidget parentalPopup;
    SvObject protectedCategoryPath;
    bool protectedCategoryLeaf;

    struct QBCarouselSideMenu_t {
        QBContextMenu ctx;
    } sidemenu;
    QBVoDMenu vodMenu;

    SvObject selected;

    SvString serviceName;
    SvString serviceId;

    QBContentSearch contentSearch;
    SvObject contentSearchPath;
    SvObject path;

    SvString serviceType;
    SvString serviceLayout;

    bool call2Action;
    SvObject serverInfo;
    QBCall2ActionContext call2ActionCtx;

    bool isListener;
    QBSearchProgressDialog searchProgressDialog;
};

SvLocal void
QBCarouselMenuOSKKeyTyped(void *self_, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key);

SvLocal bool
QBInnov8onCarouselSearchMenuShow(QBInnov8onCarouselMenuChoice self)
{
    SvErrorInfo error = NULL;
    SvString keyword;
    QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), &error);
    if (error) {
        goto err;
    }
    svSettingsPushComponent("OSKPane.settings");
    QBOSKPaneInit(oskPane, self->appGlobals->scheduler, self->sidemenu.ctx, 1, SVSTRING("OSKPane"), QBCarouselMenuOSKKeyTyped, self, &error);
    svSettingsPopComponent();
    if (error) {
        goto err;
    }
    keyword = QBContentSearchGetKeyword(self->contentSearch);
    if (keyword) {
        QBOSKPaneSetRawInput(oskPane, SvStringCString(keyword));
    }
    QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) oskPane);
    QBContextMenuShow(self->sidemenu.ctx);
    SVRELEASE(oskPane);

    return true;
err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    SVRELEASE(oskPane);
    return false;
}

SvLocal void
QBInnov8onCarouselCancelCallback(SvObject self_, QBSearchProgressDialog searchProgressDialog)
{
    SvErrorInfo error = NULL;
    QBInnov8onCarouselMenuChoice self = (QBInnov8onCarouselMenuChoice) self_;
    QBSearchProgressDialogHide(searchProgressDialog, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
    SvObject provider = QBContentCategoryTakeProvider((QBContentCategory) self->contentSearch);
    SvInvokeVirtual(QBContentProvider, (QBContentProvider) provider, stop);
    SvInvokeVirtual(QBContentProvider, (QBContentProvider) provider, clear);
    SVTESTRELEASE(provider);
    QBInnov8onCarouselSearchMenuShow(self);
}

SvLocal void
QBInnov8onCarouselOkCallback(SvObject self_, QBSearchProgressDialog searchProgressDialog)
{
    SvErrorInfo error = NULL;
    QBInnov8onCarouselMenuChoice self = (QBInnov8onCarouselMenuChoice) self_;
    QBSearchProgressDialogHide(searchProgressDialog, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
    QBInnov8onCarouselSearchMenuShow(self);
}

SvLocal void
QBInnov8onCarouselSetAsProvidersListener(QBInnov8onCarouselMenuChoice self)
{
    self->isListener = true;
    SvObject provider = QBContentCategoryTakeProvider((QBContentCategory) self->contentSearch);
    SvInvokeVirtual(QBContentProvider, (QBContentProvider) provider, addListener, (SvObject) self);
    SVTESTRELEASE(provider);
}

SvLocal void
QBInnov8onCarouselUnsetAsProvidersListener(QBInnov8onCarouselMenuChoice self)
{
    self->isListener = false;
    if (!self->contentSearch) {
        return;
    }
    SvObject provider = QBContentCategoryTakeProvider((QBContentCategory) self->contentSearch);
    if (provider) {
        SvInvokeVirtual(QBContentProvider, (QBContentProvider) provider, removeListener, (SvObject) self);
        SVRELEASE(provider);
    }
}

SvLocal void
QBInnov8onCarouselSearchStarted(SvObject self_, QBContentCategory category)
{
    SvErrorInfo error = NULL;
    QBInnov8onCarouselMenuChoice self = (QBInnov8onCarouselMenuChoice) self_;
    QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_progressState, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
}

SvLocal void
QBInnov8onCarouselMenuDisplayNoResultsPopup(QBInnov8onCarouselMenuChoice self)
{
    SvErrorInfo error = NULL;
    QBInnov8onCarouselUnsetAsProvidersListener(self);
    QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_noResultState, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
}

SvLocal void
QBInnov8onCarouselMenuDisplaySubMenuWithResults(QBInnov8onCarouselMenuChoice self)
{
    SvErrorInfo error = NULL;
    QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_finishedState, &error);
    if (error) {
        goto err;
    }
    QBSearchProgressDialogHide(self->searchProgressDialog, &error);
    if (error) {
        goto err;
    }
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
    QBInnov8onCarouselUnsetAsProvidersListener(self);

    SvObject path = self->contentSearchPath;
    QBTreeIterator tit = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, path, 0);

    path = QBTreeIteratorGetCurrentNodePath(&tit);

    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvInvokeInterface(QBMenu, menuBar->prv, setPosition, path, NULL);

    QBContextMenuHide(self->sidemenu.ctx, false);
    SVTESTRELEASE(self->contentSearch);
    SVTESTRELEASE(self->contentSearchPath);
    self->contentSearch = NULL;
    self->contentSearchPath = NULL;
    return;
err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
}

SvLocal void
QBInnov8onCarouselSearchingEnded(SvObject self_, QBContentCategory category)
{
    size_t size = SvInvokeInterface(QBListModel, category->items, getLength);
    QBInnov8onCarouselMenuChoice self = (QBInnov8onCarouselMenuChoice) self_;
    if (size > 0) {
        QBInnov8onCarouselMenuDisplaySubMenuWithResults(self);
    } else {
        QBInnov8onCarouselMenuDisplayNoResultsPopup(self);
    }
}

SvLocal void
QBInnov8onCarouselErrorOccured(SvObject self, SvErrorInfo errorInfo)
{
    QBInnov8onCarouselMenuDisplayNoResultsPopup((QBInnov8onCarouselMenuChoice) self);
}

SvLocal void
QBCarouselSideMenuClose(void *self_, QBContextMenu menu)
{
    QBInnov8onCarouselMenuChoice self = self_;
    SVTESTRELEASE(self->sidemenu.ctx);
    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBCarouselSideMenuClose, self);
}

SvLocal void
QBInnov8onCarouselMenuChoiceC2AOnCloseCallback(void *ptr)
{
    QBInnov8onCarouselMenuChoice self = ptr;
    if (self->call2ActionCtx) {
        QBCall2ActionContextStop(self->call2ActionCtx, NULL);
        SVRELEASE(self->call2ActionCtx);
        self->call2ActionCtx = NULL;
    }
}

SvLocal void
QBInnov8onCarouselMenuChoiceContextChoosen(SvObject self_, SvObject nodePath_)
{
    QBInnov8onCarouselMenuChoice self = (QBInnov8onCarouselMenuChoice) self_;
    SvErrorInfo error = NULL;
    if (self->call2Action) {
        SvObject node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath_);
        SvValue call2ActionVal = NULL;
        if (SvObjectIsInstanceOf(node, QBContentCategory_getType())) {
            SVAUTOSTRING(attribute, "call2action");
            call2ActionVal = (SvValue) QBContentCategoryGetAttribute((QBContentCategory) node, attribute);
        } else if (SvObjectIsInstanceOf(node, SvDBRawObject_getType())) {
            call2ActionVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) node, "call2action");
        }
        if (call2ActionVal && SvValueIsBoolean(call2ActionVal) && SvValueGetBoolean(call2ActionVal)) {
            SVTESTRELEASE(self->call2ActionCtx);
            self->call2ActionCtx = QBCall2ActionContextCreate(self->appGlobals, self->tree, nodePath_,
                                                              self->serviceId, self->serverInfo, &error);
            if (!self->call2ActionCtx && error) {
                goto err;
            }

            if (!QBCall2ActionContextSetOnCloseCallback(self->call2ActionCtx, self_,
                                                        &QBInnov8onCarouselMenuChoiceC2AOnCloseCallback, &error)) {
                goto err;
            }

            if (!QBCall2ActionContextStart(self->call2ActionCtx, &error)) {
                goto err;
            }
        }
    }
    return;
err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
}

SvLocal void
QBInnov8onCarouselMenuChoiceChoosen(SvObject self_, SvObject node, SvObject nodePath_, int position)
{
    QBInnov8onCarouselMenuChoice self = (QBInnov8onCarouselMenuChoice) self_;
    SVTESTRELEASE(self->path);
    self->path = SVTESTRETAIN(nodePath_);
    if (SvObjectIsInstanceOf(node, SvDBRawObject_getType())) {
        SvDBRawObject product = (SvDBRawObject) node;
        SVTESTRELEASE(self->selected);
        self->selected = SVRETAIN(node);

        if (self->vodMenu) {
            QBVoDMenuStop(self->vodMenu);
            SVRELEASE(self->vodMenu);
            // set to NULL because we can leave this function - see if under
            self->vodMenu = NULL;
        }

        if (QBMainMenuLogicHandleMenuChoice(self->appGlobals->mainLogic, product))
            return;

        self->vodMenu = QBVoDMenuCreate(self->appGlobals, self->provider, self->serviceId, self->serviceName, SVSTRING("PC_MENU"));
        QBVoDMenuStart(self->vodMenu, product, self->path);
    }
}

SvLocal bool
QBInnov8onCarouselMenuPushContext(QBInnov8onCarouselMenuChoice self,
                                  SvObject path,
                                  bool leaf,
                                  bool allreadyAuthenticated)
{
    if (SvStringEqualToCString(self->serviceType, "VOD") ||
        SvStringEqualToCString(self->serviceType, "Catchup Channel")) {
        QBContentProviderStart((QBContentProvider) self->provider, self->appGlobals->scheduler);
        QBWindowContext ctx =
            QBGUILogicPrepareVoDContext(self->appGlobals->guiLogic,
                                        self->serviceLayout, path, self->provider,
                                        self->serviceId, self->serviceName, leaf, NULL);
        if (ctx) {
            QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
            SVRELEASE(ctx);
        } else {
            return false;
        }

        return true;
    }

    return false;
}

SvLocal void
QBCarouselMenuOSKKeyTyped(void *self_, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    SvErrorInfo error = NULL;
    QBInnov8onCarouselMenuChoice self = (QBInnov8onCarouselMenuChoice) self_;

    if (key->type == QBOSKKeyType_enter) {
        if (input && SvStringLength(input) > 0) {
            const char *begin = SvStringCString(input);
            const char *end = begin + SvStringLength(input) - 1;
            while (*begin != '\0' && isspace(*begin))
                begin++;
            if (begin > end)
                return;
            while (end > begin && isspace(*end))
                end--;
            char *final = calloc(end - begin + 2, sizeof(char));
            strncpy(final, begin, end - begin + 1);
            SvString keyword = SvStringCreate(final, NULL);
            free(final);

            QBContextMenuHide(self->sidemenu.ctx, false);
            if (!QBInnov8onCarouselMenuPushContext(self, self->contentSearchPath, true, false)) {
                svSettingsPushComponent("searchResults.settings");
                self->searchProgressDialog = QBSearchProgressDialogCreate(self->appGlobals->res, self->appGlobals->controller, &error);
                svSettingsPopComponent();
                if (error) {
                    goto err;
                }
                QBSearchProgressDialogSetListener(self->searchProgressDialog, (SvObject) self, &error);
                if (error) {
                    goto err;
                }
                QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_progressState, &error);
                if (error) {
                    goto err;
                }
                QBSearchProgressDialogShow(self->searchProgressDialog, &error);
                if (error) {
                    goto err;
                }
                QBInnov8onCarouselSetAsProvidersListener(self);
            }
            QBContentSearchStartNewSearch(self->contentSearch, keyword, self->appGlobals->scheduler);
            SVRELEASE(keyword);
        }
    }
    return;
err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
}

SvLocal void
QBCarouselMenuCheckCategoryParentalControlPINCallback(void *self_, SvWidget dlg, SvString ret, unsigned key)
{
    QBInnov8onCarouselMenuChoice self = self_;

    if (!self->parentalPopup || !self->protectedCategoryPath) {
        SVTESTRELEASE(self->protectedCategoryPath);
        self->protectedCategoryPath = NULL;
        self->parentalPopup = NULL;
        return;
    }
    self->parentalPopup = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        // enter protected category
        if (self->serviceType && SvStringEqualToCString(self->serviceType, "VOD")) {
            SvObject node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, self->protectedCategoryPath);
            if (node && SvObjectIsInstanceOf(node, QBContentCategory_getType())) {
                QBContentCategory category = (QBContentCategory) node;
                QBVoDUtilsLogMovieCategory(QBContentCategoryGetId(category), QBContentCategoryGetName(category), QBContentCategoryGetLevel(category));
            }
        }

        if (!QBInnov8onCarouselMenuPushContext(self, self->protectedCategoryPath, self->protectedCategoryLeaf, true)) {
            // No context set - Set position in vod1 way
            QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, self->protectedCategoryPath, 0);
            if (QBTreeIteratorGetNodesCount(&iter) > 0) {
                SvObject destPath = QBTreeIteratorGetCurrentNodePath(&iter);
                SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
                SvInvokeInterface(QBMenu, menuBar->prv, setPosition, destPath, NULL);
            }
        }
    }
    SVRELEASE(self->protectedCategoryPath);
    self->protectedCategoryPath = NULL;
}

SvLocal bool
QBCarouselMenuCheckCategoryParentalRating(QBInnov8onCarouselMenuChoice self, QBContentCategory category, SvObject path)
{
    if (!QBVODUtilsCategoryShouldBeBlocked(self->appGlobals, category)) {
        return true;    // Allowed
    }

    // remember path to this node to use it in dialog's callback
    SVTESTRELEASE(self->protectedCategoryPath);
    self->protectedCategoryPath = SVRETAIN(path);

    svSettingsPushComponent("Carousel_VOD.settings");
    SvObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, SVSTRING("PC_VOD"));
    SvWidget master = NULL;
    svSettingsPushComponent("ParentalControl.settings");
    SvWidget dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"), gettext("Please enter parental control PIN to enter this category"), true, NULL, &master);
    svSettingsPopComponent();
    self->parentalPopup = dialog;
    QBDialogRun(dialog, self, QBCarouselMenuCheckCategoryParentalControlPINCallback);
    return false;
}

SvLocal bool
QBCarouselMenuNodeSelected(SvObject self_, SvObject node_, SvObject nodePath)
{
    QBInnov8onCarouselMenuChoice self = (QBInnov8onCarouselMenuChoice) self_;

    if (SvObjectIsInstanceOf(node_, QBContentSearch_getType())) {
        QBContentSearch search = (QBContentSearch) node_;
        SVTESTRELEASE(self->contentSearch);
        SVTESTRELEASE(self->contentSearchPath);
        self->contentSearch = SVRETAIN(search);
        self->contentSearchPath = SvObjectCopy(nodePath, NULL);
        return QBInnov8onCarouselSearchMenuShow(self);
    } else if (SvObjectIsInstanceOf(node_, QBContentSeeAll_getType())) {
        return QBInnov8onCarouselMenuPushContext(self, nodePath, true, false);
    } else if (SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        QBContentCategory category = (QBContentCategory) node_;
        bool leaf = false;
        SvValue leafV = (SvValue)
                        QBContentCategoryGetAttribute(category, SVSTRING("leaf"));
        if (leafV && SvObjectIsInstanceOf((SvObject) leafV, SvValue_getType())
            && SvValueIsInteger(leafV)) {
            leaf = SvValueGetInteger(leafV);
        } else if (leafV && SvObjectIsInstanceOf((SvObject) leafV, SvValue_getType())
                   && SvValueIsBoolean(leafV)) {
            leaf = SvValueGetBoolean(leafV);
        }
        self->protectedCategoryLeaf = leaf;

        if (QBCarouselMenuCheckCategoryParentalRating(self, (QBContentCategory) node_, nodePath)) {
            if (self->serviceType && SvStringEqualToCString(self->serviceType, "VOD")) {
                QBVoDUtilsLogMovieCategory(QBContentCategoryGetId(category), QBContentCategoryGetName(category), QBContentCategoryGetLevel(category));
            }

            return QBInnov8onCarouselMenuPushContext(self, nodePath, leaf, false);
        } else {
            return true;        // True that select_event is consumed. Category not allowed, PIN popup is being shown
        }
    } else if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        return QBInnov8onCarouselMenuPushContext(self, nodePath, false, false);
    }
    return false;
}

SvLocal void
QBInnov8onCarouselMenuChoiceDestroy(void *self_)
{
    QBInnov8onCarouselMenuChoice self = self_;

    if (self->isListener) {
        QBInnov8onCarouselUnsetAsProvidersListener(self);
    }

    if (self->sidemenu.ctx) {
        QBContextMenuSetCallbacks(self->sidemenu.ctx, NULL, NULL);
        QBContextMenuHide(self->sidemenu.ctx, false);
        SVRELEASE(self->sidemenu.ctx);
    }

    SVTESTRELEASE(self->vodMenu);

    SVTESTRELEASE(self->selected);

    SVTESTRELEASE(self->provider);
    SVRELEASE(self->tree);

    SVTESTRELEASE(self->call2ActionCtx);

    SVTESTRELEASE(self->contentSearch);
    SVTESTRELEASE(self->contentSearchPath);
    SVTESTRELEASE(self->path);

    SVRELEASE(self->serviceName);
    SVRELEASE(self->serviceId);
    SVTESTRELEASE(self->serverInfo);

    SVRELEASE(self->serviceType);

    SVTESTRELEASE(self->serviceLayout);

    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
}

SvLocal SvType
QBInnov8onCarouselMenuChoice_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBInnov8onCarouselMenuChoiceDestroy
    };

    static const struct QBMenuContextChoice_t menuContextMethods = {
        .contextChoosen = QBInnov8onCarouselMenuChoiceContextChoosen
    };

    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBInnov8onCarouselMenuChoiceChoosen
    };

    static const struct QBMenuEventHandler_ selectedMethods = {
        .nodeSelected = QBCarouselMenuNodeSelected,
    };

    static const struct QBContentProviderListener_ listener = {
        .activityStarted = QBInnov8onCarouselSearchStarted,
        .activityEnded   = QBInnov8onCarouselSearchingEnded,
        .errorOccured    = QBInnov8onCarouselErrorOccured
    };

    static const struct QBSearchProgressDialogListener_ searchProgressDialogListener = {
        .onOkPressed     = QBInnov8onCarouselOkCallback,
        .onCancelPressed = QBInnov8onCarouselCancelCallback
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBInnov8onCarouselMenuChoice",
                            sizeof(struct QBInnov8onCarouselMenuChoice_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuContextChoice_getInterface(), &menuContextMethods,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBMenuEventHandler_getInterface(), &selectedMethods,
                            QBContentProviderListener_getInterface(), &listener,
                            QBSearchProgressDialogListener_getInterface(), &searchProgressDialogListener,
                            NULL);
    }

    return type;
}

QBInnov8onCarouselMenuChoice
QBInnov8onCarouselMenuChoiceNew(AppGlobals appGlobals,
                                SvObject vodProvider,
                                SvObject tree,
                                SvString serviceId,
                                SvString serviceName,
                                SvString serviceType,
                                SvString serviceLayout,
                                bool call2Action,
                                SvObject serverInfo)
{
    QBInnov8onCarouselMenuChoice self = (QBInnov8onCarouselMenuChoice)
                                        SvTypeAllocateInstance(QBInnov8onCarouselMenuChoice_getType(), NULL);

    self->provider = SVTESTRETAIN(vodProvider);
    self->tree = SVRETAIN(tree);
    self->appGlobals = appGlobals;
    self->serviceName = SVRETAIN(serviceName);
    self->serviceId = SVRETAIN(serviceId);
    self->serviceType = SVRETAIN(serviceType);
    self->serviceLayout = SVTESTRETAIN(serviceLayout);
    self->call2Action = call2Action;
    self->serverInfo = SVTESTRETAIN(serverInfo);
    self->searchProgressDialog = NULL;
    self->isListener = false;

    self->sidemenu.ctx =
        QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller,
                                        appGlobals->res, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBCarouselSideMenuClose, self);
    return self;
}

void QBInnov8onCarouselMenuChoiceStop(SvObject self_)
{
    QBInnov8onCarouselMenuChoice self = (QBInnov8onCarouselMenuChoice) self_;
    if (self->vodMenu)
        QBVoDMenuStop(self->vodMenu);
}
