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

#include "socialMediaCarouselItemChoice.h"

#include <libintl.h>
#include <main.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/icon.h>
#include <SWL/label.h>
#include <SWL/viewport.h>
#include <QBOSK/QBOSKKey.h>
#include <QBOSK/QBOSK.h>
#include <QBCarousel/QBCarousel.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBOSKPane.h>
#include <ContextMenus/QBListPane.h>
#include <Menus/menuchoice.h>
#include <Utils/value.h>
#include <QBContentManager/Innov8onProvider.h>
#include <QBContentManager/QBContentStub.h>
#include <QBWidgets/QBGrid.h>
#include <Widgets/XMBCarousel.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <Widgets/SideMenu.h>
#include <Widgets/QBScrollView.h>
#include <Windows/imagebrowser.h>
#include <Windows/imagebrowser/QBSocialMediaImageBrowserController.h>
#include <DataModels/loadingProxy.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBContentManager/QBLoadingStub.h>

#include "QBSocialMediaRemoteAction.h"

typedef enum {
    QBSocialMediaCredentials_login,
    QBSocialMediaCredentials_password
} QBSocialMediaCredentials;

struct QBMoreInfo_t {
    QBTextRenderer textRenderer;
    SvWidget viewport;
    SvWidget grid;
    QBAsyncLabelConstructor label;
    QBAsyncLabelConstructor text;
};
typedef struct QBMoreInfo_t *QBMoreInfo;

struct QBSocialMediaCommentsMenuItem_t {
    SvWidget title;
    SvWidget subcaption;
    SvWidget text;
    SvWidget likeIcon;
    SvWidget likes;
};
typedef struct QBSocialMediaCommentsMenuItem_t *QBSocialMediaCommentsMenuItem;

struct QBSocialMediaCommentsMenuItemController_t {
    struct SvObject_ super_;

    QBSocialMediaCarouselMenuChoice ctx;
};
typedef struct QBSocialMediaCommentsMenuItemController_t *QBSocialMediaCommentsMenuItemController;

struct QBSocialMediaCarouselMenuChoice_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    struct QBCarouselSideMenu_t {
        QBContextMenu ctx;
    } sidemenu;

    SvGenericObject selected;

    SvString serviceName;

    SvGenericObject path;

    int serviceId;
    SvString login;
    SvString password;
    QBSocialMediaCredentials credentials;

    SvWidget moreInfo;
    SvWidget comments;

    QBSocialMediaRemoteAction remoteAction;
    SvHashTable               actions;
};

//

inline SvLocal SvHashTable
getProductData(SvDBRawObject product)
{
    SvHashTable details = NULL;
    details = (SvHashTable) SvDBRawObjectGetAttrValue(product, "presentation_details");
    SVAUTOSTRING(dataKey, "data");
    return (SvHashTable) SvHashTableFind(details, (SvGenericObject) dataKey);
}

inline SvLocal SvHashTable
getProductActions(SvDBRawObject product)
{
    SvHashTable details = NULL;
    details = (SvHashTable) SvDBRawObjectGetAttrValue(product, "presentation_details");
    SVAUTOSTRING(actionsKey, "actions");
    return (SvHashTable) SvHashTableFind(details, (SvGenericObject) actionsKey);
}

// QBMoreInfo

SvLocal SvWidget
QBMoreInfoGetViewport(SvWidget w)
{
    QBMoreInfo prv = w->prv;
    return prv->viewport;
}

SvLocal void
QBMoreInfoAddItem(SvWidget w, SvString label, SvString text)
{
    QBMoreInfo prv = w->prv;
    SvWidget asyncLabel;

    asyncLabel = QBGridAddAsyncLabelFromConstructor(prv->grid, prv->label);
    QBAsyncLabelSetText(asyncLabel, label);

    asyncLabel = QBGridAddAsyncLabelFromConstructor(prv->grid, prv->text);
    QBAsyncLabelSetText(asyncLabel, text);
}

SvLocal void
QBMoreInfoAddItems(SvWidget w, SvArray items)
{
    // QBMoreInfo prv = w->prv;
    SvIterator it = SvArrayIterator(items);
    SvHashTable item = NULL;
    SvValue labelV = NULL, textV = NULL;
    SvString label = NULL, text = NULL;
    SVAUTOSTRING(labelKey, "label");
    SVAUTOSTRING(textKey, "text");
    while ((item = (SvHashTable) SvIteratorGetNext(&it))) {
        labelV = (SvValue) SvHashTableFind(item, (SvGenericObject) labelKey);
        textV = (SvValue) SvHashTableFind(item, (SvGenericObject) textKey);
        label = SvValueGetString(labelV);
        text = SvValueGetString(textV);
        QBMoreInfoAddItem(w, label, text);
    }
}

SvLocal void
QBMoreInfoReset(SvWidget w)
{
    QBMoreInfo prv = w->prv;
    QBGridReset(prv->grid);
}

SvLocal void
QBMoreInfoCleanup(SvApplication app, void *prv_)
{
    QBMoreInfo prv = prv_;

    SVRELEASE(prv->label);
    SVRELEASE(prv->text);

    free(prv);
}

SvLocal SvWidget
QBMoreInfoCreate(SvApplication app, QBTextRenderer textRenderer,
                 const char* widgetName)
{
    QBMoreInfo prv = calloc(1, sizeof(struct QBMoreInfo_t));
    SvWidget w = svSettingsWidgetCreate(app, widgetName);
    w->prv = prv;
    svWidgetSetName(w, widgetName);
    w->clean = QBMoreInfoCleanup;

    prv->textRenderer = textRenderer;

    char *tmp = NULL;

    asprintf(&tmp, "%s.viewport", widgetName);
    prv->viewport = svViewPortNew(app, tmp);
    svSettingsWidgetAttach(w, prv->viewport, tmp, 1);
    free(tmp);

    asprintf(&tmp, "%s.grid", widgetName);
    prv->grid = QBGridCreate(app, tmp);
    free(tmp);

    asprintf(&tmp, "%s.label", widgetName);
    prv->label = QBAsyncLabelConstructorCreate(app, tmp, textRenderer, NULL);
    free(tmp);

    asprintf(&tmp, "%s.text", widgetName);
    prv->text = QBAsyncLabelConstructorCreate(app, tmp, textRenderer, NULL);
    free(tmp);

    svViewPortSetContents(prv->viewport, prv->grid);
    svViewPortSetContentsPosition(prv->viewport, 0, 0, true);

    return w;
}

// QBSocialMediaCommentsMenuItemController

SvLocal SvWidget
QBSocialMediaCommentsMenuItemControllerCreateItem(SvGenericObject self_,
                                                  SvApplication app,
                                                  int width, int minHeight)
{
    svSettingsPushComponent("ActionsPane.settings");

    SvWidget item = svWidgetCreate(app, width, minHeight);
    //SvWidget item = svWidgetCreateWithColor(app, width, minHeight, 0x00ff00ff);
    QBSocialMediaCommentsMenuItem prv = calloc(1, sizeof(struct QBSocialMediaCommentsMenuItem_t));
    item->prv = prv;

    //prv->title = QBAsyncLabelNew(app, "Comment.title");
    prv->title = svLabelNewFromSM(app, "Comment.title");
    svSettingsWidgetAttach(item, prv->title, "Comment.title", 1);

    //prv->subcaption = QBAsyncLabelNew(app, "Comment.subcaption");
    prv->subcaption = svLabelNewFromSM(app, "Comment.subcaption");
    svSettingsWidgetAttach(item, prv->subcaption, "Comment.subcaption", 1);

    //prv->text = QBAsyncLabelNew(app, "Comment.text");
    prv->text = svLabelNewFromSM(app, "Comment.text");
    svSettingsWidgetAttach(item, prv->text, "Comment.text", 1);

    //prv->likes = QBAsyncLabelNew(app, "Comment.likes");
    prv->likes = svLabelNewFromSM(app, "Comment.likes");
    svSettingsWidgetAttach(item, prv->likes, "Comment.likes", 1);

    prv->likeIcon = svIconNew(app, "Comment.likeIcon");
    svSettingsWidgetAttach(item, prv->likeIcon, "Comment.likeIcon", 1);

    svSettingsPopComponent();

    return item;
}

SvLocal void
QBSocialMediaCommentsMenuItemControllerSetObject(SvGenericObject self_,
                                                 SvWidget item_,
                                                 SvGenericObject object)
{
    QBSocialMediaCommentsMenuItemController self = (QBSocialMediaCommentsMenuItemController)self_;
    QBSocialMediaCommentsMenuItem prv = item_->prv;

    if (SvObjectIsInstanceOf(object, SvDBRawObject_getType())) {
        SvDBRawObject comment = (SvDBRawObject) object;
        SvValue titleV , textV , subcaptionV , likesV, likeIconV;
        svWidgetSetHidden(prv->subcaption, false);
        svWidgetSetHidden(prv->likes, false);
        svWidgetSetHidden(prv->likeIcon, false);
        svWidgetSetHidden(prv->text, false);

        SvHashTable data = getProductData(comment);

        SVAUTOSTRING(titleKey, "title");
        titleV = (SvValue) SvHashTableFind(data, (SvGenericObject) titleKey);
        //QBAsyncLabelSetText(prv->title, SvValueGetString(titleV));
        svLabelSetText(prv->title, SvValueGetStringAsCString(titleV, NULL));

        SVAUTOSTRING(subcaptionKey, "subcaption");
        subcaptionV = (SvValue) SvHashTableFind(data, (SvGenericObject) subcaptionKey);
        //QBAsyncLabelSetText(prv->subcaption, SvValueGetString(subcaptionV));
        svLabelSetText(prv->subcaption, SvValueGetStringAsCString(subcaptionV, NULL));

        SVAUTOSTRING(likesKey, "likes_counter");
        likesV = (SvValue) SvHashTableFind(data, (SvGenericObject) likesKey);
        //QBAsyncLabelSetText(prv->likes, SvValueGetString(likesV));
        svLabelSetText(prv->likes, SvValueGetStringAsCString(likesV, NULL));

        SVAUTOSTRING(likeIconKey, "like_icon");
        likeIconV = (SvValue) SvHashTableFind(data, (SvGenericObject) likeIconKey);
        SvString icon = SvValueGetString(likeIconV);
        SvURI buf = NULL;
        SvString thumbnailsPrefixUrl = QBMiddlewareManagerGetThumbnailsPrefixUrl(self->ctx->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        if (thumbnailsPrefixUrl) {
            SvURI base = SvURICreateWithString(thumbnailsPrefixUrl, NULL);
            buf = SvURICreateWithBaseURIAndPath(base, icon, NULL);
            SVTESTRELEASE(base);
        } else {
            buf = SvURICreateWithString(icon, NULL);
            if (!SvURIIsAbsolute(buf)) {
                SvLogError("%s(): path %s is not absolute, cannot load image properly.", __func__, SvStringGetCString(icon));
            }
        }
        svIconSetBitmapFromURI(prv->likeIcon, 0, SvStringGetCString(SvURIPath(buf)));
        SVTESTRELEASE(buf);

        SVAUTOSTRING(textKey, "text");
        textV = (SvValue) SvHashTableFind(data, (SvGenericObject) textKey);
        //QBAsyncLabelSetText(prv->text, SvValueGetString(textV));
        svLabelSetText(prv->text, SvValueGetStringAsCString(textV, NULL));

        item_->height = prv->text->off_y + prv->text->height;
    } else if (SvObjectIsInstanceOf(object, QBLoadingStub_getType()) ||
               SvObjectIsInstanceOf(object, QBContentStub_getType())) {
        SvString loading = SvStringCreate(gettext("Please wait, loading…"), NULL);
        //QBAsyncLabelSetText(prv->title, loading);
        svLabelSetText(prv->title, SvStringCString(loading));
        svWidgetSetHidden(prv->subcaption, true);
        svWidgetSetHidden(prv->likes, true);
        svWidgetSetHidden(prv->likeIcon, true);
        svWidgetSetHidden(prv->text, true);
        item_->height = prv->title->off_y + prv->title->height;
        SVRELEASE(loading);
    }
}

SvLocal void
QBSocialMediaCommentsMenuItemControllerDestroy(void *self_)
{
    QBSocialMediaCommentsMenuItemController self = self_;
    SVRELEASE(self->ctx);
}

SvLocal SvType
QBSocialMediaCommentsMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSocialMediaCommentsMenuItemControllerDestroy
    };
    static SvType type = NULL;
    static const struct QBScrollViewItemController_t controllerMethods = {
        .createItem = QBSocialMediaCommentsMenuItemControllerCreateItem,
        .setObject = QBSocialMediaCommentsMenuItemControllerSetObject,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSocialMediaCommentsMenuItemController",
                            sizeof(struct QBSocialMediaCommentsMenuItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBScrollViewItemController_getInterface(), &controllerMethods,
                            NULL);
    }

    return type;
}

SvLocal QBSocialMediaCommentsMenuItemController
QBSocialMediaCommentsMenuItemControllerCreate(QBSocialMediaCarouselMenuChoice ctx)
{
    QBSocialMediaCommentsMenuItemController self = NULL;

    self = (QBSocialMediaCommentsMenuItemController)
            SvTypeAllocateInstance(QBSocialMediaCommentsMenuItemController_getType(), NULL);

    if (!self) {
        SvLogError("%s() : out of memory", __func__);
        return NULL;
    }

    self->ctx = SVRETAIN(ctx);

    return self;
}

// QBSocialMediaCarouselMenuChoice

SvLocal void
QBCarouselSideMenuClose(void *self_, QBContextMenu menu)
{
    QBSocialMediaCarouselMenuChoice self = self_;
    SVTESTRELEASE(self->sidemenu.ctx);
    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBCarouselSideMenuClose, self);
}

SvLocal void
QBSocialMediaCarouselMenuMoreInfoOnHide(void *self_,
                                        QBContainerPane pane,
                                        SvWidget frame)
{
    QBSocialMediaCarouselMenuChoice self = self_;

    QBSocialMediaRemoteActionCancelAction(self->remoteAction);

    svWidgetDestroy(self->moreInfo);
    self->moreInfo = NULL;
}

SvLocal void
QBSocialMediaCarouselMenuMoreInfoOnShow(void *self_,
                                        QBContainerPane pane,
                                        SvWidget frame)
{
    QBSocialMediaCarouselMenuChoice self = self_;

    int level = QBContainerPaneGetLevel(pane);
    SideMenuSetFrameWidth(self->sidemenu.ctx->sideMenu, level, self->moreInfo->width, false);
    frame->height = self->moreInfo->height;
    frame->width = self->moreInfo->width;
    svWidgetAttach(frame, self->moreInfo, self->moreInfo->off_x, self->moreInfo->off_y, 0);
}

SvLocal void
QBSocialMediaCarouselMenuMoreInfoSetActive(void *self_,
                                           QBContainerPane pane,
                                           SvWidget frame)
{
    QBSocialMediaCarouselMenuChoice self = self_;
    svWidgetSetFocus(QBMoreInfoGetViewport(self->moreInfo));
}

SvLocal void
QBSocialMediaCarouselMenuShowMoreInfo(QBSocialMediaCarouselMenuChoice self, int level)
{
    AppGlobals appGlobals = self->appGlobals;
    if (self->moreInfo)
        svWidgetDestroy(self->moreInfo);
    self->moreInfo = QBMoreInfoCreate(appGlobals->res, appGlobals->textRenderer,
                                      "MoreInfo");

    static struct QBContainerPaneCallbacks_t callbacks = {
        .onHide    = QBSocialMediaCarouselMenuMoreInfoOnHide,
        .onShow    = QBSocialMediaCarouselMenuMoreInfoOnShow,
        .setActive = QBSocialMediaCarouselMenuMoreInfoSetActive
    };
    QBContainerPane options = (QBContainerPane)
        SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    QBContainerPaneInit(options, appGlobals->res, self->sidemenu.ctx, level,
                        SVSTRING("BasicPane"), &callbacks, self);
    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
    SVRELEASE(options);
}

SvLocal void
QBSocialMediaCarouselMenuLocalMoreInfoCallback(void *self_,
                                               SvString id,
                                               QBBasicPane pane,
                                               QBBasicPaneItem item)
{
    QBSocialMediaCarouselMenuChoice self = self_;
    SvDBRawObject product = (SvDBRawObject) self->selected;

    SvValue titleV = NULL, textV = NULL;

    SvHashTable data = getProductData(product);

    SVAUTOSTRING(titleKey, "title");
    SVAUTOSTRING(textKey, "text");
    titleV = (SvValue) SvHashTableFind(data, (SvGenericObject) titleKey);
    textV = (SvValue) SvHashTableFind(data, (SvGenericObject) textKey);

    SvString title = SvValueTryGetString(titleV);
    SvString text = SvValueTryGetString(textV);

    int level = QBBasicPaneGetLevel(pane);
    svSettingsPushComponent("ActionsPane.settings");
    QBSocialMediaCarouselMenuShowMoreInfo(self, level + 1);
    QBMoreInfoAddItem(self->moreInfo, title, text);
    svSettingsPopComponent();
}

SvLocal void
QBSocialMediaCarouselMenuShowFullscreen(QBSocialMediaCarouselMenuChoice self)
{
    AppGlobals appGlobals = self->appGlobals;

    SvGenericObject parentPath = SvObjectCopy(self->path, NULL);
    SvInvokeInterface(QBTreePath, parentPath, truncate, -1);

    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, appGlobals->menuTree,
                                            getIterator, parentPath, 0);
    int position = 0;
    do {
        if (SvObjectEquals(QBTreeIteratorGetCurrentNodePath(&iter), self->path))
            break;
        position++;
    } while (QBTreeIteratorGetNextNode(&iter));

    QBSocialMediaImageBrowserController controller = NULL;
    QBWindowContext imageBrowser = NULL;

    controller = QBSocialMediaImageBrowserControllerCreate(appGlobals, NULL);
    imageBrowser = QBImageBrowserContextCreate(appGlobals, parentPath, position,
                                               (SvGenericObject) controller, false, -1);
    QBApplicationControllerPushContext(appGlobals->controller, imageBrowser);
    SVRELEASE(parentPath);
    SVRELEASE(imageBrowser);
    SVRELEASE(controller);
}


SvLocal void
QBSocialMediaCarouselMenuCreateLocalActions(QBSocialMediaCarouselMenuChoice self,
                                            QBBasicPane actionsPane)
{
    SvDBRawObject product = (SvDBRawObject) self->selected;
    SvString name = NULL;
    SvValue schemeV = NULL;
    SvString schemeS = NULL;

    schemeV = (SvValue) SvDBRawObjectGetAttrValue(product, "presentation_scheme");
    schemeS = SvValueTryGetString(schemeV);
    if (!schemeS)
        return;
    const char *scheme = SvStringCString(schemeS);

    if (!strncmp(scheme, "ITEM_DETAILS", 12)) {
        name = SvStringCreate(gettext("More info"), NULL);
        QBBasicPaneAddOption(actionsPane, SVSTRING("MORE_INFO"), name,
                             QBSocialMediaCarouselMenuLocalMoreInfoCallback, self);
        SVRELEASE(name);
    }

    //if (!strcmp(scheme, "PICTURE")) {
    //    name = SvStringCreate(gettext("Fullscreen"), NULL);
    //    QBBasicPaneAddOption(actionsPane, SVSTRING("FULLSCREEN"), name,
    //                         QBSocialMediaCarouselMenuFullscreenCallback, self);
    //    SVRELEASE(name);
    //}
}

SvLocal void
QBSocialMediaCarouselMenuDoLikeActionCallback(void *self_,
                                              SvHashTable action,
                                              SvGenericObject result)
{
//    QBSocialMediaCarouselMenuChoice self = self_;
//    SvDBRawObject product = (SvDBRawObject) result;
}

SvLocal void
QBSocialMediaCarouselMenuDoLikeAction(void *self_,
                                      SvString id,
                                      QBBasicPane pane,
                                      QBBasicPaneItem item)
{
    QBSocialMediaCarouselMenuChoice self = self_;
    SvHashTable action = NULL;
    action = (SvHashTable) SvHashTableFind(self->actions, (SvGenericObject) id);

    QBSocialMediaRemoteActionDoAction(self->remoteAction, action, self->serviceId,
                                      QBSocialMediaCarouselMenuDoLikeActionCallback, self);

}

SvLocal void
QBSocialMediaCarouselMenuDoMoreActionCallback(void *self_,
                                              SvHashTable action,
                                              SvGenericObject result)
{
    QBSocialMediaCarouselMenuChoice self = self_;

    QBMoreInfoReset(self->moreInfo);

    if (!result) {
        return;
    } else if (SvObjectIsInstanceOf(result, SvDBRawObject_getType())) {
        SvDBRawObject product = (SvDBRawObject) result;

        SvHashTable pDetails = (SvHashTable)
            SvDBRawObjectGetAttrValue(product, "presentation_details");
        if (!pDetails)
            return;

        SvArray details = (SvArray)
            SvHashTableFind(pDetails, (SvGenericObject) SVSTRING("details"));
        if (!details)
            return;

        QBMoreInfoAddItems(self->moreInfo, details);
    } else if (SvObjectIsInstanceOf(result, QBLoadingStub_getType())) {
        SvString loading = SvStringCreate(gettext("Please wait, loading…"), NULL);
        QBMoreInfoAddItem(self->moreInfo, loading, NULL);
        SVRELEASE(loading);
    }
}

SvLocal void
QBSocialMediaCarouselMenuDoMoreAction(void *self_,
                                      SvString id,
                                      QBBasicPane pane,
                                      QBBasicPaneItem item)
{
    QBSocialMediaCarouselMenuChoice self = self_;
    SvHashTable action = NULL;
    action = (SvHashTable) SvHashTableFind(self->actions, (SvGenericObject) id);

    int level = QBBasicPaneGetLevel(pane);
    svSettingsPushComponent("ActionsPane.settings");
    QBSocialMediaCarouselMenuShowMoreInfo(self, level + 1);
    svSettingsPopComponent();

    QBSocialMediaRemoteActionDoAction(self->remoteAction, action, self->serviceId,
                                      QBSocialMediaCarouselMenuDoMoreActionCallback, self);
}

SvLocal void
QBSocialMediaCommentsOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBSocialMediaCarouselMenuChoice self = self_;

    QBSocialMediaRemoteActionCancelAction(self->remoteAction);

    svWidgetDestroy(self->comments);
    self->comments = NULL;
}

SvLocal void
QBSocialMediaCommentsOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBSocialMediaCarouselMenuChoice self = self_;

    SideMenuSetFrameWidth(self->sidemenu.ctx->sideMenu, 2, self->comments->width, false);
    frame->height = self->comments->height;
    frame->width = self->comments->width;
    svWidgetAttach(frame, self->comments, self->comments->off_x, self->comments->off_y, 0);
}

SvLocal void
QBSocialMediaCommentsSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBSocialMediaCarouselMenuChoice self = self_;
    svWidgetSetFocus(QBScrollViewGetViewport(self->comments));
}

SvLocal void
QBSocialMediaCarouselMenuDoCommentsActionCallback(void *self_,
                                                  SvHashTable action,
                                                  SvGenericObject result)
{
    QBSocialMediaCarouselMenuChoice self = self_;
    AppGlobals appGlobals = self->appGlobals;

    static struct QBContainerPaneCallbacks_t callbacks = {
        .onHide    = QBSocialMediaCommentsOnHide,
        .onShow    = QBSocialMediaCommentsOnShow,
        .setActive = QBSocialMediaCommentsSetActive
    };

    svSettingsPushComponent("ActionsPane.settings");
    if (self->comments)
        svWidgetDestroy(self->comments);
    self->comments = QBScrollViewCreate(appGlobals->res, "ScrollView", NULL);

    QBSocialMediaCommentsMenuItemController itemController =
        QBSocialMediaCommentsMenuItemControllerCreate(self);

    QBScrollViewSetItemController(self->comments, (SvGenericObject) itemController, NULL);
    SVRELEASE(itemController);
    QBScrollViewConnectToSource(self->comments, result);

    QBContainerPane options = (QBContainerPane)
        SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    QBContainerPaneInit(options, appGlobals->res, self->sidemenu.ctx, 2,
                        SVSTRING("BasicPane"), &callbacks, self);

    svSettingsPopComponent();

    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
    SVRELEASE(options);
}

SvLocal void
QBSocialMediaCarouselMenuDoCommentsAction(void *self_,
                                          SvString id,
                                          QBBasicPane pane,
                                          QBBasicPaneItem item)
{
    QBSocialMediaCarouselMenuChoice self = self_;
    SvHashTable action = NULL;
    action = (SvHashTable) SvHashTableFind(self->actions, (SvGenericObject) id);

    QBSocialMediaRemoteActionDoAction(self->remoteAction, action, self->serviceId,
                                      QBSocialMediaCarouselMenuDoCommentsActionCallback, self);
}

SvLocal void
QBSocialMediaCarouselMenuCreateRemoteAction(QBSocialMediaCarouselMenuChoice self,
                                            QBBasicPane actionsPane,
                                            SvString key,
                                            QBBasicPaneItemCallback callback)
{
    SvHashTable action = NULL;
    SvValue nameV = NULL;
    SvString name = NULL;
    action = (SvHashTable) SvHashTableFind(self->actions, (SvGenericObject) key);
    if (action) {
        SVAUTOSTRING(nameKey, "label");
        nameV = (SvValue) SvHashTableFind(action, (SvGenericObject) nameKey);
        name = SvValueGetString(nameV);

        SvLogNotice("%s() : added option : %s",
                    __func__, name ? SvStringCString(name) : NULL);
        QBBasicPaneAddOption(actionsPane, key, name,
                             callback, self);
    }
}

SvLocal void
QBSocialMediaCarouselMenuCreateRemoteActions(QBSocialMediaCarouselMenuChoice self,
                                             QBBasicPane actionsPane)
{
    SvDBRawObject product = (SvDBRawObject) self->selected;

    SvHashTable actions = getProductActions(product);
    SVTESTRETAIN(actions);
    SVTESTRELEASE(self->actions);
    self->actions = actions;
    if (!self->actions)
        return;

    QBSocialMediaCarouselMenuCreateRemoteAction(self, actionsPane, SVSTRING("like"),
                                                QBSocialMediaCarouselMenuDoLikeAction);
    QBSocialMediaCarouselMenuCreateRemoteAction(self, actionsPane, SVSTRING("more"),
                                                QBSocialMediaCarouselMenuDoMoreAction);
    QBSocialMediaCarouselMenuCreateRemoteAction(self, actionsPane, SVSTRING("comments"),
                                                QBSocialMediaCarouselMenuDoCommentsAction);
}


SvLocal void
QBSocialMediaCarouselMenuShowActions(QBSocialMediaCarouselMenuChoice self, int level)
{
    AppGlobals appGlobals = self->appGlobals;

    svSettingsPushComponent("ActionsPane.settings");

    QBBasicPane actionsPane = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(actionsPane, appGlobals->res, appGlobals->scheduler,
                    appGlobals->textRenderer, self->sidemenu.ctx, level, SVSTRING("BasicPane"));

    QBSocialMediaCarouselMenuCreateLocalActions(self, actionsPane);
    QBSocialMediaCarouselMenuCreateRemoteActions(self, actionsPane);


    svSettingsPopComponent();

    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) actionsPane);
    QBContextMenuShow(self->sidemenu.ctx);
    SVRELEASE(actionsPane);
}

SvLocal void
QBSocialMediaCarouselMenuChoosen(SvGenericObject self_,
                                 SvGenericObject node,
                                 SvGenericObject nodePath_,
                                 int position)
{
    QBSocialMediaCarouselMenuChoice self = (QBSocialMediaCarouselMenuChoice) self_;
    SVTESTRELEASE(self->path);
    self->path = SVTESTRETAIN(nodePath_);
    if (SvObjectIsInstanceOf(node, SvDBRawObject_getType())) {
        SVRETAIN(node);
        SVTESTRELEASE(self->selected);
        self->selected = node;
        SvDBRawObject product = (SvDBRawObject) node;
        SvValue schemeV = NULL;
        schemeV = (SvValue) SvDBRawObjectGetAttrValue(product, "presentation_scheme");
        SvString scheme = SvValueTryGetString(schemeV);

        if (scheme && SvStringEqualToCString(scheme, "PICTURE")) {
            QBSocialMediaCarouselMenuShowFullscreen(self);
        } else if (scheme && SvStringEqualToCString(scheme, "PROFILE")) {
            SvHashTable actions = getProductActions(product);
            SvHashTable action = (SvHashTable)
                SvHashTableFind(actions, (SvGenericObject) SVSTRING("more"));

            QBContextMenuShow(self->sidemenu.ctx);
            svSettingsPushComponent("ActionsPane.settings");
            QBSocialMediaCarouselMenuShowMoreInfo(self, 1);
            svSettingsPopComponent();

            QBSocialMediaRemoteActionDoAction(self->remoteAction, action, self->serviceId,
                                              QBSocialMediaCarouselMenuDoMoreActionCallback, self);
        } else {
            QBSocialMediaCarouselMenuShowActions(self, 1);
        }
    }
}

SvLocal void
QBSocialMediaCarouselMenuOSKKeyTyped(void *self_,
                                     QBOSKPane pane,
                                     SvString input,
                                     unsigned int layout,
                                     QBOSKKey key)
{
    QBSocialMediaCarouselMenuChoice self = self_;

    if (key->type == QBOSKKeyType_enter && SvStringLength(input) > 0) {
        switch (self->credentials) {
            case QBSocialMediaCredentials_login:
                SVTESTRELEASE(self->login);
                self->login = SVRETAIN(input);
                break;
            case QBSocialMediaCredentials_password:
                SVTESTRELEASE(self->password);
                self->password = SVRETAIN(input);
                break;
        }
        QBContextMenuPopPane(self->sidemenu.ctx);
    }
}

SvLocal void
QBSocialMediaCarouselMenuShowOSK(QBSocialMediaCarouselMenuChoice self, int level)
{
    svSettingsPushComponent("OSKPane.settings");

    QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(),
                                                           NULL);
    SvErrorInfo error = NULL;
    QBOSKPaneInitLayout(oskPane, self->appGlobals->scheduler,
                        self->sidemenu.ctx, "LoginOSKPane.oskmap",
                        level, SVSTRING("OSKPane"),
                        QBSocialMediaCarouselMenuOSKKeyTyped, self,
                        &error);
    svSettingsPopComponent();
    if (!error) {
        switch (self->credentials) {
            case QBSocialMediaCredentials_login:
                QBOSKPaneSetPasswordMode(oskPane, false, '*');
                if (self->login)
                    QBOSKPaneSetInput(oskPane, self->login);
                break;
            case QBSocialMediaCredentials_password:
                QBOSKPaneSetPasswordMode(oskPane, true, '*');
                if (self->password)
                    QBOSKPaneSetInput(oskPane, self->password);
                break;
        }

        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) oskPane);
    } else {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVRELEASE(oskPane);
}

SvLocal void
QBSocialMediaCarouselMenuEditLoginCallback(void *self_,
                                           SvString id,
                                           QBBasicPane pane,
                                           QBBasicPaneItem item)
{
    QBSocialMediaCarouselMenuChoice self = self_;
    self->credentials = QBSocialMediaCredentials_login;
    int level = QBBasicPaneGetLevel(pane);
    QBSocialMediaCarouselMenuShowOSK(self, level + 1);
}

SvLocal void
QBSocialMediaCarouselMenuEditPasswordCallback(void *self_,
                                              SvString id,
                                              QBBasicPane pane,
                                              QBBasicPaneItem item)
{
    QBSocialMediaCarouselMenuChoice self = self_;
    self->credentials = QBSocialMediaCredentials_password;
    int level = QBBasicPaneGetLevel(pane);
    QBSocialMediaCarouselMenuShowOSK(self, level + 1);
}

SvLocal void
QBSocialMediaCarouselMenuSaveCredentialsCallback(void *self_,
                                                 SvString id,
                                                 QBBasicPane pane,
                                                 QBBasicPaneItem item)
{
    QBSocialMediaCarouselMenuChoice self = self_;

    if (!self->login || !self->password)
        return;

    QBAuthenticationServiceSetupAccount(self->appGlobals->authenticationService,
                                        self->serviceId,
                                        self->login, self->password,
                                        true);

    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal void
QBSocialMediaCarouselMenuSignOffCallback(void *self_,
                                         SvString id,
                                         QBBasicPane pane,
                                         QBBasicPaneItem item)
{
    QBSocialMediaCarouselMenuChoice self = self_;

    if (SvStringEqualToCString(id, "OK")) {
        QBAuthenticationServiceDeleteAccount(self->appGlobals->authenticationService,
                                             self->serviceId);
    }

    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal void
QBSocialMediaCarouselMenuShowCredentials(QBSocialMediaCarouselMenuChoice self, int level)
{
    svSettingsPushComponent("BasicPane.settings");

    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler,
                    self->appGlobals->textRenderer, self->sidemenu.ctx, level, SVSTRING("BasicPane"));

    SvString name = NULL;

    name = SvStringCreate(gettext("Login"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("EDIT_LOGIN"), name,
                         QBSocialMediaCarouselMenuEditLoginCallback, self);
    SVRELEASE(name);

    name = SvStringCreate(gettext("Password"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("EDIT_PASSWORD"), name,
                         QBSocialMediaCarouselMenuEditPasswordCallback, self);
    SVRELEASE(name);

    name = SvStringCreate(gettext("Save"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("SAVE"), name,
                         QBSocialMediaCarouselMenuSaveCredentialsCallback, self);
    SVRELEASE(name);

    if (self->login && self->password) {
        SvArray confirmations = SvArrayCreate(NULL);
        SvArray ids = SvArrayCreate(NULL);

        name = SvStringCreate(gettext("Ok"), NULL);
        SvArrayAddObject(confirmations, (SvGenericObject) name );
        SvArrayAddObject(ids, (SvGenericObject) SVSTRING("OK"));
        SVRELEASE(name);

        name = SvStringCreate(gettext("Cancel"), NULL);
        SvArrayAddObject(confirmations,  (SvGenericObject) name);
        SvArrayAddObject(ids, (SvGenericObject) SVSTRING("CANCEL"));
        SVRELEASE(name);

        name = SvStringCreate(gettext("Sign Off"), NULL);
        QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("SIGN_OFF"),
                                             name, SVSTRING("BasicPane"),
                                             confirmations, ids,
                                             QBSocialMediaCarouselMenuSignOffCallback, self);
        SVRELEASE(name);
        SVRELEASE(confirmations);
        SVRELEASE(ids);
    }

    svSettingsPopComponent();

    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
    QBContextMenuShow(self->sidemenu.ctx);
    SVRELEASE(options);
}

SvLocal void
QBSocialMediaCarouselMenuUpdateCredentials(QBSocialMediaCarouselMenuChoice self)
{
    SvString login = NULL, password = NULL;

    SVTESTRELEASE(self->login);
    SVTESTRELEASE(self->password);
    QBAuthenticationServiceGetCredentials(self->appGlobals->authenticationService,
                                          self->serviceId, &login, &password, NULL);
    self->login = SVTESTRETAIN(login);
    self->password = SVTESTRETAIN(password);
}

SvLocal void
QBSocialMediaCarouselMenuContextChoosen(SvGenericObject self_, SvGenericObject nodePath)
{
    QBSocialMediaCarouselMenuChoice self = (QBSocialMediaCarouselMenuChoice) self_;

    QBActiveTreeNode node = QBActiveTreeFindNodeByPath(self->appGlobals->menuTree, nodePath);
    SvValue requiresLoginV = (SvValue) QBActiveTreeNodeGetAttribute(node, SVSTRING("requiresLogin"));
    bool requiresLogin = false;
    if (requiresLoginV && SvObjectIsInstanceOf((SvObject) requiresLoginV, SvValue_getType())) {
        if (SvValueIsBoolean(requiresLoginV))
            requiresLogin = SvValueGetBoolean(requiresLoginV);
        else if (SvValueIsInteger(requiresLoginV))
            requiresLogin = SvValueGetInteger(requiresLoginV);
    }

    if (requiresLogin) {
        SvValue serviceIdV = (SvValue) QBActiveTreeNodeGetAttribute(node, SVSTRING("serviceId"));
        self->serviceId = atoi(SvValueGetStringAsCString(serviceIdV, NULL));

        QBSocialMediaCarouselMenuUpdateCredentials(self);
        QBSocialMediaCarouselMenuShowCredentials(self, 1);
    }
}

SvLocal bool
QBSocialMediaCarouselMenuNodeSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath)
{
    QBSocialMediaCarouselMenuChoice self = (QBSocialMediaCarouselMenuChoice) self_;

    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        QBActiveTreeNode node = (QBActiveTreeNode) node_;

        SvValue requiresLoginV = (SvValue) QBActiveTreeNodeGetAttribute(node, SVSTRING("requiresLogin"));
        bool requiresLogin = false;
        if (requiresLoginV && SvObjectIsInstanceOf((SvObject) requiresLoginV, SvValue_getType())) {
            if (SvValueIsBoolean(requiresLoginV))
                requiresLogin = SvValueGetBoolean(requiresLoginV);
            else if (SvValueIsInteger(requiresLoginV))
                requiresLogin = SvValueGetInteger(requiresLoginV);
        }

        if (requiresLogin) {
            SvValue serviceIdV = (SvValue) QBActiveTreeNodeGetAttribute(node, SVSTRING("serviceId"));
            self->serviceId = atoi(SvValueGetStringAsCString(serviceIdV, NULL));

            QBSocialMediaCarouselMenuUpdateCredentials(self);
            if (!self->login || !self->password) {
                QBSocialMediaCarouselMenuShowCredentials(self, 1);
                return true;
            }
        }
    }

    return false;
}


SvLocal void
QBSocialMediaCarouselMenuChoiceDestroy(void *self_)
{
    QBSocialMediaCarouselMenuChoice self = self_;

    if (self->sidemenu.ctx){
        QBContextMenuSetCallbacks(self->sidemenu.ctx, NULL, NULL);
        QBContextMenuHide(self->sidemenu.ctx, false);
        SVRELEASE(self->sidemenu.ctx);
    }

    SVTESTRELEASE(self->selected);
    SVTESTRELEASE(self->actions);
    SVTESTRELEASE(self->remoteAction);

    SVTESTRELEASE(self->path);

    SVTESTRELEASE(self->login);
    SVTESTRELEASE(self->password);
    SVTESTRELEASE(self->serviceName);
}

SvLocal SvType
QBSocialMediaCarouselMenuChoice_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSocialMediaCarouselMenuChoiceDestroy
    };
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBSocialMediaCarouselMenuChoosen
    };
    static const struct QBMenuContextChoice_t contextMethods = {
        .contextChoosen = QBSocialMediaCarouselMenuContextChoosen
    };

    static const struct QBMenuEventHandler_ selectedMethods = {
        .nodeSelected = QBSocialMediaCarouselMenuNodeSelected,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSocialMediaCarouselMenuChoice",
                            sizeof(struct QBSocialMediaCarouselMenuChoice_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBMenuContextChoice_getInterface(), &contextMethods,
                            QBMenuEventHandler_getInterface(), &selectedMethods,
                            NULL);
    }

    return type;
}

QBSocialMediaCarouselMenuChoice
QBSocialMediaCarouselMenuChoiceNew(AppGlobals appGlobals, SvString serviceName)
{
    QBSocialMediaCarouselMenuChoice self = (QBSocialMediaCarouselMenuChoice)
        SvTypeAllocateInstance(QBSocialMediaCarouselMenuChoice_getType(), NULL);

    self->appGlobals = appGlobals;
    self->serviceName = SVRETAIN(serviceName);
    self->moreInfo = NULL;
    self->remoteAction = QBSocialMediaRemoteActionCreate(appGlobals);

    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings",
                                                         appGlobals->controller,
                                                         appGlobals->res,
                                                         SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBCarouselSideMenuClose, self);

    return self;
}

