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

/*
Known hacks/omissions:
- Does not refresh the same content in all providers when refreshing it upon showing sidemenu

Nice to have:
- Fade in/out of focus when moving between cover/trailer/carousel
- Adverts
*/

#include <Windows/QBVoDMovieDetails.h>

#include <libintl.h>
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBListProxy.h>
#include <QBDataModel3/QBListModelListener.h>
#include <Windows/QBVoDGridBrowser.h>
#include <SWL/icon.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/widget.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBInnov8onExtendedInfoPane.h>
#include <ContextMenus/QBContentSideMenu.h>
#include <ContextMenus/QBListPane.h>
#include <QBPlayerControllers/QBPlaylistController.h>
#include <QBPlayerControllers/QBVoDAnyPlayerController.h>
#include <Middlewares/Innov8on/innov8onTransaction.h>
#include <Middlewares/productPurchase.h>
#include <QBCarousel/QBVerticalCarousel.h>
#include <QBContentManager/Innov8on/Innov8onProviderRequestPluginGetRecommendations.h>
#include <QBContentManager/Innov8onUtils.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentCategoryListener.h>
#include <QBContentManager/QBContentManager.h>
#include <QBContentManager/QBContentAction.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBInput/QBInputCodes.h>
#include <QBWindowContext.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SWL/QBFrame.h>
#include <Utils/authenticators.h>
#include <Utils/QBContentInfo.h>
#include <Utils/QBMovieUtils.h>
#include <Utils/value.h>
#include <Widgets/QBMovieInfo.h>
#include <Widgets/QBRatingWidget.h>
#include <Widgets/QBTitle.h>
#include <Widgets/authDialog.h>
#include <Widgets/trailerPreview.h>
#include <Widgets/SideMenu.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <Windows/pvrplayer.h>
#include <Logic/VoDMovieDetailsLogic.h>
#include <main.h>
#include <SWL/icon.h>
#include <player_hints/http_input.h>
#include <settings.h>
#include <stdbool.h>
#include <Widgets/QBTVPreview.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Logic/timeFormat.h>
#include <QBStringUtils.h>
#include <Logic/GUILogic.h>
#include <Services/QBAccessController/QBAccessManagerListener.h>
#include <Utils/QBVODUtils.h>
#include <QBAppKit/Utils/commonUtils.h>

typedef enum {
    QBVoDMovieDetailsParentalControlCheck_trailerPreview = 0,
    QBVoDMovieDetailsParentalControlCheck_movieDetails,
    QBVoDMovieDetailsParentalControlCheck_showSideMenu
} QBVoDMovieDetailsParentalControlCheck;

typedef enum {
    QBVoDMovieDetailsState_unknown = 0,
    QBVoDMovieDetailsState_loading,
    QBVoDMovieDetailsState_noresults,
    QBVoDMovieDetailsState_loaded,
} QBVoDMovieDetailsState;

struct QBVoDMovieDetailsContext_t {
    struct QBWindowContext_t super_;

    QBVoDMovieDetails movieDetails;

    SvWidget title;
    SvDBRawObject object;
    SvString titleString;
    QBContentCategory category;
    SvGenericObject externalProvider;
    SvString serviceId;
    SvString serviceName;
    AppGlobals appGlobals;
    SvHashTable actions;
    QBContentTree tree;
    SvGenericObject provider;
    SvString actionName;
    QBContentAction action;
    SvObject ownerTree;
    QBVoDMovieDetailsState state;
    QBVoDLogic vodLogic;
};
typedef struct QBVoDMovieDetailsContext_t *QBVoDMovieDetailsContext;

typedef struct QBVoDMovieDetailsPaneItemConstructor_t* QBVoDMovieDetailsPaneItemConstructor;

struct QBVoDMovieDetailsSimilarCarouselSlot_t {
    unsigned int settingsCtx;
    SvWidget box;
    SvWidget icon;
    SvWidget label;
    SvWidget labelBackground;

    SvGenericObject obj;
    char *widgetName;
    QBVoDMovieDetails movieDetails;
};

struct QBVoDMovieDetails_t {
    struct SvObject_ super_;

    QBVoDMovieDetailsContext ctx;

    unsigned int settingsCtx;

    SvGenericObject similarDataSource;

    struct QBVoDMovieDetailsBasicElements_s basicElements;

    SvWidget animation;

    SvWidget details;

    struct {
        SvWidget cover;
        SvWidget trailerPreview;
    } focus;

    struct {
        SvWidget title;
        SvWidget description;
        SvWidget info;
        SvWidget objectDescription;
        double showDetailsDelay;
        SvTimerId timer;
        bool shown;
    } movieDetails;

    SvDBRawObject similarCarouselActiveElement;
    int similarCarouselActiveElementIndex;

    SvWidget parentalPopup;

    QBContextMenu contentSideMenu;

    bool isTrailerAvailable;

    QBVoDMovieDetailsLogic vodMovieDetailsLogic;
};

struct QBVoDMovieDetailsPaneItemConstructor_t {
    struct SvObject_ super_;
    QBXMBItemConstructor itemConstructor;
    QBFrameConstructor* focus;
};

typedef struct QBVoDMovieDetailsSimilarCarouselSlot_t* QBVoDMovieDetailsSimilarCarouselSlot;

struct QBVoDMovieDetailsTrailerPreview_t {
    QBVoDMovieDetails movieDetails;
    SvWidget w;
    SvWidget trailerPreview;
    int focusXOffset;
    int focusYOffset;
};
typedef struct QBVoDMovieDetailsTrailerPreview_t* QBVoDMovieDetailsTrailerPreview;

struct QBVoDMovieDetailsCover_t {
    SvWidget w;
    SvWidget cover;
    int focusXOffset;
    int focusYOffset;

    QBVoDMovieDetails movieDetails;
};
typedef struct QBVoDMovieDetailsCover_t* QBVoDMovieDetailsCover;

struct QBVoDMovieDetailsSimilarCarousel_t {
    SvWidget w;
    SvWidget similarCarousel;
    SvWidget title;
    SvWidget focus;
    int xOffset;
    int yOffset;
    bool isVisible;

    QBVoDMovieDetails movieDetails;
};
typedef struct QBVoDMovieDetailsSimilarCarousel_t* QBVoDMovieDetailsSimilarCarousel;

SvLocal SvType QBVoDMovieDetails_getType(void);
SvLocal void QBVoDMovieDetailsSideDetaisShow(QBVoDMovieDetails self);
SvLocal void QBVoDMovieDetailsSideDetailsHide(QBVoDMovieDetails self);
SvLocal void QBVoDMovieDetailsFillSideDetails(QBVoDMovieDetails self);
SvLocal SvWidget QBVoDMovieDetailsTrailerPreviewCreate(QBVoDMovieDetails movieDetails, AppGlobals appGlobals, const char* widgetName);
SvLocal SvGenericObject QBVoDMovieDetailsCreateSimilarDataSource(SvGenericObject tree, SvBitmap listStartMarker);
SvLocal void QBVoDDetailsShowSideMenu(QBVoDMovieDetails self);

void
QBVoDMovieDetailsPlayTrailer(QBVoDMovieDetails self, SvDBRawObject movie, double position)
{
    if (!movie)
        return;

    const char* attributeName = "trailer";

    SvString URI = QBMovieUtilsGetAttr(movie, attributeName, NULL);

    if (!URI)
        return;

    SvString proxyURI = NULL;

    SvString coverURI = NULL;

    QBMediaFileType type = QBAppKitURIToMediaFileType(URI, NULL);
    SvContent c = NULL;

    if (type != QBMediaFileType_audioPlaylist) {
        c = SvContentCreateFromCString(SvStringCString(URI), NULL);

        SvValue titleV = (SvValue) SvDBRawObjectGetAttrValue(movie, "title");
        SvString title = NULL;
        if (!titleV)
            titleV = (SvValue) SvDBRawObjectGetAttrValue(movie, "name");
        if (titleV)
            title = SvValueGetString(titleV);

        if (title) {
            SvContentMetaDataSetStringProperty(SvContentGetMetaData(c),
                                               SVSTRING(SV_PLAYER_META__TITLE),
                                               title);
        }
        coverURI = QBMovieUtilsGetAttr(movie, "thumbnail", NULL);
    }

    if (proxyURI)
        SvContentMetaDataSetStringProperty(SvContentGetHints(c),
                                           SVSTRING(PLAYER_HINT__HTTP_INPUT_PROXY),
                                           proxyURI);

    QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->ctx->appGlobals);
    SvValue propertyKey = SvValueCreateWithString(SVSTRING("VoD"), NULL);
    SvValue propertyValue = SvValueCreateWithString(SVSTRING("Innov8on"), NULL);
    QBAnyPlayerLogicSetProperty(anyPlayerLogic, propertyKey, propertyValue);
    SVRELEASE(propertyKey);
    SVRELEASE(propertyValue);
    if (URI)
        QBAnyPlayerLogicSetVodId(anyPlayerLogic, URI);

    QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->ctx->appGlobals,
                                                         anyPlayerLogic);

    SvGenericObject contentInfo = QBVoDMovieDetailsContextCreateContentInfo((QBWindowContext) self->ctx);
    QBPVRPlayerContextSetContentData(pvrPlayer, contentInfo);
    SVRELEASE(contentInfo);

    QBPVRPlayerContextSetTitle(pvrPlayer, self->ctx->serviceName);
    QBPVRPlayerContextSetVodMetaData(pvrPlayer, movie);
    QBPVRPlayerContextSetStartPosition(pvrPlayer, position);
    QBPVRPlayerContextUseBookmarks(pvrPlayer, false);
    if (coverURI)
        QBPVRPlayerContextSetCoverURI(pvrPlayer, coverURI);

    SvValue playerFeaturesV = NULL;
    playerFeaturesV = (SvValue) SvDBRawObjectGetAttrValue(movie, "playerFeatures");
    if (playerFeaturesV) {
        const char *playerFeatures = SvStringCString(SvValueGetString(playerFeaturesV));
        while (playerFeatures && *playerFeatures) {
            if (!strncmp(playerFeatures, "disableAll", strlen("disableAll"))) {
                playerFeatures += strlen("disableAll");
                QBAnyPlayerLogicToggleAll(anyPlayerLogic, false);
            } else if (!strncmp(playerFeatures, "loop", strlen("loop"))) {
                playerFeatures += strlen("loop");
                QBAnyPlayerLogicToggleLoop(anyPlayerLogic, true);
            } else
                break;

            if (*playerFeatures)
                playerFeatures++;   //skip ','
        }
    }
    SVRELEASE(anyPlayerLogic);

    switch (type) {
        case QBMediaFileType_audio:
            QBPVRPlayerContextSetMusic(pvrPlayer, c);
            break;
        case QBMediaFileType_video:
            QBPVRPlayerContextSetContent(pvrPlayer, c);
            break;
        case QBMediaFileType_audioPlaylist:
            QBPVRPlayerContextSetMusicPlaylist(pvrPlayer, URI);
            break;
        case QBMediaFileType_unknown:
            /** TODO: Add handling of QBMediaFileType_unknown case */
            QBPVRPlayerContextSetContent(pvrPlayer, c);
            break;
        default:
            break;
    }
    SVTESTRELEASE(c);
    QBApplicationControllerPushContext(self->ctx->appGlobals->controller, pvrPlayer);
    SVRELEASE(pvrPlayer);
}

SvLocal void
QBVoDMovieDetailsCheckParentalControlPINCallbackMovieDetails(void *ptr, SvWidget dlg,
                                                                     SvString ret, unsigned key)
{
    QBVoDMovieDetails self = ptr;

    self->parentalPopup = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        QBVoDMovieDetailsContext ctx = self->ctx;
        SvObject category = SvInvokeInterface(QBTreeModel, ctx->tree, getNode, NULL);
        QBWindowContext movieDetailsCtx = QBVoDMovieDetailsContextCreate(ctx->appGlobals, (QBContentCategory) category, self->similarCarouselActiveElement, ctx->provider, ctx->serviceId, ctx->serviceName, (SvObject) ctx->tree);
        QBApplicationControllerPushContext(ctx->appGlobals->controller, movieDetailsCtx);
        SVRELEASE(movieDetailsCtx);
    }
}

SvLocal void
QBVoDMovieDetailsCheckParentalControlPINCallbackTrailerPreview(void *ptr, SvWidget dlg,
                                                                       SvString ret, unsigned key)
{
    QBVoDMovieDetails self = ptr;
    QBVoDMovieDetailsTrailerPreview trailerPreview = self->basicElements.trailerPreview->prv;

    self->parentalPopup = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        QBTrailerPreviewPlayPausePlayback(trailerPreview->trailerPreview);
    }
}

SvLocal void
QBVoDMovieDetailsCheckParentalControlPINCallbackShowSideMenu(void *ptr, SvWidget dlg,
                                                             SvString ret, unsigned key)
{
    QBVoDMovieDetails self = ptr;

    if (!ret || !SvStringEqualToCString(ret, "OK-button"))
        return;

    svSettingsRestoreContext(self->settingsCtx);
    if (QBVoDMovieDetailsLogicShowContentMenu(self->vodMovieDetailsLogic))
        goto finish;

    QBContentSideMenu contentSideMenu = (QBContentSideMenu) self->contentSideMenu;
    if (QBContextMenuIsShown((QBContextMenu) contentSideMenu))
        goto finish;

    SvGenericObject contentInfo = QBVoDMovieDetailsContextCreateContentInfo((QBWindowContext) self->ctx);
    QBContentSideMenuSetContentInfo(contentSideMenu, contentInfo);
    QBContextMenuShow((QBContextMenu) contentSideMenu);
    SVRELEASE(contentInfo);

finish:
    svSettingsPopComponent();
}

SvLocal void
QBVoDMovieDetailsCheckParentalControlPIN(QBVoDMovieDetails self, QBVoDMovieDetailsParentalControlCheck checkType)
{
    svSettingsPushComponent("ParentalControl.settings");
    AppGlobals appGlobals = self->ctx->appGlobals;
    SvGenericObject authenticator = NULL;
    authenticator = QBAuthenticateViaAccessManager(appGlobals->scheduler,
                                                   appGlobals->accessMgr,
                                                   SVSTRING("PC_VOD"));
    SvWidget master = NULL;
    SvWidget dialog = QBAuthDialogCreate(appGlobals, authenticator,
                                         gettext("Authentication required"),
                                         0, true, NULL, &master);

    self->parentalPopup = dialog;
    if (checkType == QBVoDMovieDetailsParentalControlCheck_movieDetails)
        QBDialogRun(dialog, self, QBVoDMovieDetailsCheckParentalControlPINCallbackMovieDetails);
    if (checkType == QBVoDMovieDetailsParentalControlCheck_trailerPreview)
        QBDialogRun(dialog, self, QBVoDMovieDetailsCheckParentalControlPINCallbackTrailerPreview);
    if (checkType == QBVoDMovieDetailsParentalControlCheck_showSideMenu)
        QBDialogRun(dialog, self, QBVoDMovieDetailsCheckParentalControlPINCallbackShowSideMenu);
    svSettingsPopComponent();
}

SvLocal bool
QBVoDMovieDetailsCheckParentalControl(QBVoDMovieDetails self, SvDBRawObject movie)
{
    if (QBAccessManagerGetAuthenticationStatus(self->ctx->appGlobals->accessMgr, SVSTRING("PC_VOD"), NULL) == QBAuthStatus_OK)
        return true;

    if (QBVoDUtilsIsAdult((SvObject) self->ctx->object))
        return false;

    SvString parentalControl = QBMovieUtilsGetAttr(movie, "ratings", NULL);
    bool allowed = false;

    if (!parentalControl || QBParentalControlRatingAllowed(self->ctx->appGlobals->pc, parentalControl))
        allowed = true;
    return allowed;
}

SvLocal void QBVoDMovieDetailsCoverFocusEventHandler(SvWidget w, SvFocusEvent e)
{
    QBVoDMovieDetailsCover self = w->prv;

    switch (e->kind) {
        case SvFocusEventKind_GET:
            svWidgetAttach(self->w, self->movieDetails->focus.cover, self->focusXOffset, self->focusYOffset, 2);
            break;
        case SvFocusEventKind_LOST:
            svWidgetDetach(self->movieDetails->focus.cover);
            break;
        default:
            SvLogWarning("%s() : unexpected focus event kind [%d]", __func__, e->kind);
            return;
    }
}

SvLocal bool QBVoDMovieDetailsCoverInputEventHandler(SvWidget w, SvInputEvent e)
{
    bool ret = false;
    QBVoDMovieDetailsCover self = w->prv;
    if (e->ch == QBKEY_ENTER) {
        QBVoDDetailsShowSideMenu(self->movieDetails);
        ret = true;
    } else if (e->ch == QBKEY_RIGHT) {
        if (self->movieDetails->isTrailerAvailable)
            svWidgetSetFocus(self->movieDetails->basicElements.trailerPreview);
        else
            QBVoDDetailsShowSideMenu(self->movieDetails);
        ret = true;
    } else {
        ret = QBVoDMovieDetailsLogicHandleCoverInputEvent(self->movieDetails->vodMovieDetailsLogic, e, (SvGenericObject) self->movieDetails->ctx);
        if (!ret)
            ret = QBVoDLogicInputHandler(self->movieDetails->ctx->vodLogic, e, (SvGenericObject) self->movieDetails->ctx);
    }

    return ret;
}

SvWidget QBVoDMovieDetailsCoverCreate(QBVoDMovieDetails movieDetails, SvApplication app, const char *widgetName)
{
    QBVoDMovieDetailsCover self = calloc(1, sizeof(*self));
    self->w = svSettingsWidgetCreate(app, widgetName);
    self->w->prv = self;
    svWidgetSetFocusEventHandler(self->w, QBVoDMovieDetailsCoverFocusEventHandler);
    svWidgetSetFocusable(self->w, true);
    svWidgetSetInputEventHandler(self->w, QBVoDMovieDetailsCoverInputEventHandler);
    self->movieDetails = movieDetails;

    char *buf;
    asprintf(&buf, "%s.Image", widgetName);
    self->cover = svIconNew(app, buf);
    svSettingsWidgetAttach(self->w, self->cover, buf, 1);
    free(buf);

    asprintf(&buf, "%s.Focus", widgetName);
    self->movieDetails->focus.cover = QBFrameCreateFromSM(app, buf);
    self->focusXOffset = svSettingsGetInteger(buf, "xOffset", 0);
    self->focusYOffset = svSettingsGetInteger(buf, "yOffset", 0);
    free(buf);

    return self->w;
}

SvLocal void QBVoDMovieDetailsSimilarCarouselUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e)
{
    QBVoDMovieDetailsSimilarCarousel self = w->prv;

    if (svWidgetGetId(self->similarCarousel) == src) {
        if (!self->isVisible) {
            return;
        }
        if (e->code == SV_EVENT_QB_CAROUSEL_NOTIFICATION) {
            QBVerticalCarouselNotification notify = e->prv;
            if (notify->type == QB_CAROUSEL_NOTIFICATION_ACTIVE_ELEMENT) {
                SVTESTRELEASE(self->movieDetails->similarCarouselActiveElement);
                self->movieDetails->similarCarouselActiveElement = NULL;
                QBVoDMovieDetailsSideDetailsHide(self->movieDetails);
                if (SvObjectIsInstanceOf(notify->dataObject, SvDBRawObject_getType())) {
                    self->movieDetails->similarCarouselActiveElement = SVTESTRETAIN(notify->dataObject);
                    self->movieDetails->similarCarouselActiveElementIndex = notify->dataIndex;
                    QBVoDMovieDetailsLogicSetActiveElement(self->movieDetails->vodMovieDetailsLogic, self->movieDetails->similarCarouselActiveElement);
                    if (QBVerticalCarouselIsFocused(self->similarCarousel))
                        QBVoDMovieDetailsSideDetaisShow(self->movieDetails);
                }
            } else if (notify->type == QB_CAROUSEL_NOTIFICATION_SELECTED_ELEMENT) {
                if (notify->dataObject && SvObjectIsInstanceOf(notify->dataObject, SvDBRawObject_getType())) {
                    QBVoDMovieDetailsContext ctx = self->movieDetails->ctx;
                    SvDBRawObject obj;
                    if (!(obj = QBVoDMovieDetailsLogicGetAdaptedObject(self->movieDetails->vodMovieDetailsLogic, (SvDBRawObject) notify->dataObject)))
                        obj = (SvDBRawObject) notify->dataObject;

                    const SvDBRawObject currentMovie = self->movieDetails->ctx->object;
                    const SvDBRawObject recommendedMovie = self->movieDetails->similarCarouselActiveElement;
                    QBVoDUtilsLogCurrentAndRecommendedMovieDetails(currentMovie, recommendedMovie);

                    if(QBVoDMovieDetailsCheckParentalControl(self->movieDetails, obj)) {
                        SvObject category = SvInvokeInterface(QBTreeModel, ctx->tree, getNode, NULL);
                        QBWindowContext movieDetailsCtx = QBVoDMovieDetailsContextCreate(ctx->appGlobals, (QBContentCategory) category, obj, ctx->provider, ctx->serviceId, ctx->serviceName, (SvObject) ctx->tree);
                        QBApplicationControllerPushContext(ctx->appGlobals->controller, movieDetailsCtx);
                        SVRELEASE(movieDetailsCtx);
                    } else {
                        QBVoDMovieDetailsCheckParentalControlPIN(self->movieDetails, QBVoDMovieDetailsParentalControlCheck_movieDetails);
                    }
                } else {
                    QBVoDMovieDetailsContext ctx = self->movieDetails->ctx;
                    QBWindowContext windowCtx = QBVoDLogicCreateContextForObject(ctx->vodLogic,
                            (SvGenericObject) ctx->tree, ctx->provider, NULL,
                            NULL, notify->dataObject);
                    if (windowCtx) {
                        QBApplicationControllerPushContext(self->movieDetails->ctx->appGlobals->controller, windowCtx);
                        SVRELEASE(windowCtx);
                    }
                }
            }
        }
        return;
    }
}

SvLocal bool QBVoDMovieDetailsShouldBeBlocked(QBVoDMovieDetails self)
{
    if (QBAccessManagerGetAuthenticationStatus(self->ctx->appGlobals->accessMgr, SVSTRING("PC_VOD"), NULL) == QBAuthStatus_OK)
        return false;

    if (!QBParentalControlLogicAdultIsBlocked(self->ctx->appGlobals->parentalControlLogic))
        return false;

    return QBVoDUtilsIsAdult((SvObject) self->ctx->object);
}

void QBVoDMovieDetailsFillCover(SvWidget w, SvString attrType_, SvString attrName_, SvString attrNameVal_)
{
    if (w && w->prv) {
        QBVoDMovieDetailsCover cover = (QBVoDMovieDetailsCover) w->prv;
        if (cover->movieDetails) {
            QBVoDMovieDetails details = cover->movieDetails;
            if (!QBVoDMovieDetailsShouldBeBlocked(details)) {
                QBMovieUtilsSetThumbnailIntoIcon(cover->cover, (SvObject) cover->movieDetails->ctx->object, SvStringCString(attrType_), SvStringCString(attrName_), SvStringCString(attrNameVal_), 1);
            } else {
                svIconSetBitmap(cover->cover, 1, svSettingsGetBitmap("QBVoDMovieDetails.Cover.Image", "bgLocked"));
                svIconSwitch(cover->cover, 1, 0, 0.0);
            }
        }
    }
}

SvLocal void QBVoDMovieDetailsFillMovieInfo(QBVoDMovieDetails self)
{
    if (self->basicElements.movieInfo && !QBVoDMovieDetailsShouldBeBlocked(self))
        QBMovieInfoSetObject(self->basicElements.movieInfo, (SvGenericObject) self->ctx->object);
}

void QBVoDMovieDetailsFillDescription(QBVoDMovieDetails self, SvObject object)
{
    if (!self->basicElements.description || !SvObjectIsInstanceOf(object, SvDBRawObject_getType()))
        return;

    if (QBVoDMovieDetailsShouldBeBlocked(self)) {
        QBAsyncLabelSetText(self->basicElements.description, QBParentalControlGetBlockedMovieDescription(self->ctx->appGlobals->pc));
    } else {
        SvString description = QBMovieUtilsGetAttr((SvDBRawObject) object, "description", NULL);
        if (description) {
            QBAsyncLabelSetText(self->basicElements.description, description);
        }
    }
}

SvLocal void QBVoDMovieDetailsFillTrailerPreview(QBVoDMovieDetails self)
{
    if (!QBVoDMovieDetailsShouldBeBlocked(self)) {
        SvString trailerURL = QBMovieUtilsGetAttr(self->ctx->object, "trailer", NULL);
        if (trailerURL && SvStringLength(trailerURL) > 0) {
            self->basicElements.trailerPreview = QBVoDMovieDetailsTrailerPreviewCreate(self, self->ctx->appGlobals, "QBVoDMovieDetails.TrailerPreview");
            svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.trailerPreview, "QBVoDMovieDetails.TrailerPreview", 1);
            QBVoDMovieDetailsTrailerPreview trailerPreview = self->basicElements.trailerPreview->prv;
            QBTrailerPreviewSetMedia(trailerPreview->trailerPreview, trailerURL);
            self->isTrailerAvailable = true;
        }
    }
}

SvLocal void QBVoDMovieDetailsFillTitle(QBVoDMovieDetails self)
{
    if (self->basicElements.title) {
        if (!QBVoDMovieDetailsShouldBeBlocked(self)) {
            SvString title = QBMovieUtilsGetAttr(self->ctx->object, "name", SVSTRING(""));
            QBAsyncLabelSetText(self->basicElements.title, title);
        } else {
            QBAsyncLabelSetText(self->basicElements.title, QBParentalControlGetBlockedMovieTitle(self->ctx->appGlobals->pc));
        }
    }
}

SvLocal void QBVoDMovieDetailsSimilarCarouselSetDefaultPosition(QBVoDMovieDetails self)
{
    QBVoDMovieDetailsSimilarCarousel similarCarousel = self->basicElements.similarCarousel->prv;
    int len = SvInvokeInterface(QBListModel, self->similarDataSource, getLength);

    QBVerticalCarouselSetActive(similarCarousel->similarCarousel, (len > self->basicElements.similarCarouseInitialFocusColumn) ? self->basicElements.similarCarouseInitialFocusColumn : (len - 1));
}

SvLocal void QBVoDMovieDetailsShowSimilarCarousel(QBVoDMovieDetails self)
{
    QBVoDMovieDetailsSimilarCarousel similarCarousel = self->basicElements.similarCarousel->prv;

    similarCarousel->isVisible = true;
    svWidgetAttach(self->ctx->super_.window, similarCarousel->w, similarCarousel->xOffset, similarCarousel->yOffset, 1);
}

SvLocal void QBVoDMovieDetailsHideSimilarCarousel(QBVoDMovieDetails self)
{
    QBVoDMovieDetailsSimilarCarousel similarCarousel = self->basicElements.similarCarousel->prv;

    similarCarousel->isVisible = false;
    svWidgetDetach(similarCarousel->w);
}

SvLocal SvWidget
QBVoDMovieDetailsCreateAnimation(SvApplication app, const char *widgetName)
{
    SvWidget animation = QBWaitAnimationCreate(app, widgetName);
    return animation;
}

SvLocal void QBVoDMovieDetailsEnsureSimilarCarousel(QBVoDMovieDetails self)
{
    if (!self->basicElements.similarCarousel)
        return;
    QBVoDMovieDetailsSimilarCarousel similarCarousel = self->basicElements.similarCarousel->prv;
    if (!similarCarousel)
        return;
    int len = SvInvokeInterface(QBListModel, self->similarDataSource, getLength);

    if (similarCarousel->isVisible && len > 1)
        return;

    if (len > 1) {
        QBVoDMovieDetailsShowSimilarCarousel(self);
        QBVoDMovieDetailsSimilarCarouselSetDefaultPosition(self);
    } else {
        QBVoDMovieDetailsHideSimilarCarousel(self);
    }
}


SvLocal void QBVoDMovieDetailsSideDetaisShow(QBVoDMovieDetails self)
{
    if (self->movieDetails.timer)
        svAppTimerStop(self->ctx->super_.window->app, self->movieDetails.timer);

    if (self->similarCarouselActiveElement)
        self->movieDetails.timer = svAppTimerStart(self->ctx->super_.window->app, self->ctx->super_.window, self->movieDetails.showDetailsDelay, 1);
    else
        self->movieDetails.timer = 0;
}

SvLocal void QBVoDMovieDetailsSideDetailsHide(QBVoDMovieDetails self)
{
    if (self->movieDetails.timer) {
        svAppTimerStop(self->ctx->super_.window->app, self->movieDetails.timer);
        self->movieDetails.timer = 0;
    }

    if (!QBVoDMovieDetailsLogicHideObjectDetails(self->vodMovieDetailsLogic))
        svWidgetSetHidden(self->details, true);
}

SvWidget QBVoDMovieDetailsSimilarCarouselCreate(QBVoDMovieDetails movieDetails, SvApplication app, SvString widgetName_, QBTextRenderer renderer, SvString title)
{
    QBVoDMovieDetailsSimilarCarousel self = calloc(1, sizeof(*self));

    self->movieDetails = movieDetails;

    const char* widgetName = SvStringCString(widgetName_);
    self->w = svSettingsWidgetCreate(app, widgetName);
    self->w->prv = self;
    svWidgetSetUserEventHandler(self->w, QBVoDMovieDetailsSimilarCarouselUserEventHandler);
    self->xOffset = svSettingsGetInteger(widgetName, "xOffset", 0);
    self->yOffset = svSettingsGetInteger(widgetName, "yOffset", 0);

    char *buf;
    asprintf(&buf, "%s.Carousel", widgetName);
    self->similarCarousel = QBVerticalCarouselNewFromSM(app, buf);
    QBVerticalCarouselSetNotificationTarget(self->similarCarousel, svWidgetGetId(self->w));
    svSettingsWidgetAttach(self->w, self->similarCarousel, buf, 1);
    free(buf);
    asprintf(&buf, "%s.Carousel.Focus", widgetName);
    self->focus = QBFrameCreateFromSM(app, buf);
    free(buf);
    asprintf(&buf, "%s.Title", widgetName);
    self->title = QBAsyncLabelNew(app, buf, renderer);
    QBAsyncLabelSetText(self->title, title);
    svSettingsWidgetAttach(self->w, self->title, buf, 10);
    free(buf);

    return self->w;
}

SvLocal void QBVoDMovieDetailsTrailerPreviewPlay(QBVoDMovieDetailsTrailerPreview self)
{
    if (QBVoDMovieDetailsCheckParentalControl(self->movieDetails, self->movieDetails->ctx->object)) {
        QBTrailerPreviewPlayPausePlayback(self->trailerPreview);
    } else {
        QBVoDMovieDetailsCheckParentalControlPIN(self->movieDetails, QBVoDMovieDetailsParentalControlCheck_trailerPreview);
    }
}

SvLocal bool QBVoDMovieDetailsTrailerPreviewInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBVoDMovieDetailsTrailerPreview self = w->prv;
    bool ret = false;

    if (e->ch == QBKEY_ENTER) {
        if (!QBTrailerPreviewIsPlaying(self->trailerPreview)) {
            QBVoDMovieDetailsTrailerPreviewPlay(self);
        } else {
            double currentPosition = QBTrailerPreviewGetCurrentPosition(self->trailerPreview);
            QBTrailerPreviewSetMedia(self->trailerPreview, NULL);
            QBVoDMovieDetailsPlayTrailer(self->movieDetails, self->movieDetails->ctx->object, currentPosition);
        }
        ret = true;
    } else if (e->ch == QBKEY_LEFT) {
        svWidgetSetFocus(self->movieDetails->basicElements.cover);
        ret = true;
    } else if (e->ch == QBKEY_RIGHT) {
        QBVoDDetailsShowSideMenu(self->movieDetails);
        ret = true;
    } else {
        ret = QBVoDLogicInputHandler(self->movieDetails->ctx->vodLogic, e, (SvGenericObject) self->movieDetails->ctx);
    }

    return ret;
}

SvLocal void QBVoDMovieDetailsTrailerPreviewFocusEventHandler(SvWidget w, SvFocusEvent e)
{
    QBVoDMovieDetailsTrailerPreview self = w->prv;

    switch (e->kind) {
        case SvFocusEventKind_GET:
            svWidgetAttach(self->w, self->movieDetails->focus.trailerPreview, self->focusXOffset, self->focusYOffset, 2);
            break;
        case SvFocusEventKind_LOST:
            svWidgetDetach(self->movieDetails->focus.trailerPreview);
            break;
        default:
            SvLogWarning("%s() : unexpected focus event kind [%d]", __func__, e->kind);
            return;
    }
}

SvLocal SvWidget QBVoDMovieDetailsTrailerPreviewCreate(QBVoDMovieDetails movieDetails, AppGlobals appGlobals, const char* widgetName)
{
    QBVoDMovieDetailsTrailerPreview self = calloc(1, sizeof(*self));
    self->movieDetails = movieDetails;

    SvApplication app = appGlobals->res;
    SvWidget w = svSettingsWidgetCreate(app, widgetName);
    self->w = w;
    w->prv = self;
    svWidgetSetFocusable(w, true);
    svWidgetSetFocusEventHandler(w, QBVoDMovieDetailsTrailerPreviewFocusEventHandler);
    svWidgetSetInputEventHandler(w, QBVoDMovieDetailsTrailerPreviewInputEventHandler);

    char *buf;
    asprintf(&buf, "%s.Preview", widgetName);
    self->trailerPreview = QBTrailerPreviewNew(appGlobals, buf);
    svSettingsWidgetAttach(w, self->trailerPreview, buf, 1);
    free(buf);

    asprintf(&buf, "%s.Focus", widgetName);
    self->movieDetails->focus.trailerPreview = QBFrameCreateFromSM(app, buf);
    self->focusXOffset = svSettingsGetInteger(buf, "xOffset", 0);
    self->focusYOffset = svSettingsGetInteger(buf, "yOffset", 0);
    free(buf);

    return w;
}

SvLocal void QBVoDMovieDetailsSimilarCarouselSlotClean(SvApplication app, void *self_)
{
    QBVoDMovieDetailsSimilarCarouselSlot self = self_;
    free(self->widgetName);
    SVTESTRELEASE(self->obj);
    free(self);
}

SvLocal void
QBVoDMovieDetailsSimilarCarouselSlotDestroyIcon(SvWidget w)
{
    QBVoDMovieDetailsSimilarCarouselSlot self = w->prv;
    if (self->icon) {
        svWidgetDestroy(self->icon);
        self->icon = NULL;
    }
}

SvLocal void
QBVoDMovieDetailsSimilarCarouselSlotSetObject(SvWidget w, SvGenericObject object)
{
    QBVoDMovieDetailsSimilarCarouselSlot self = w->prv;
    if (SvObjectEquals(self->obj, object))
        return;

    QBVoDMovieDetailsSimilarCarouselSlotDestroyIcon(w);

    SVTESTRETAIN(object);
    SVTESTRELEASE(self->obj);
    self->obj = object;

    svSettingsRestoreContext(self->settingsCtx);

    self->icon = svIconNew(w->app, self->widgetName);
    svIconSetBitmap(self->icon, 1, svSettingsGetBitmap(self->widgetName, "bgLocked"));
    svWidgetAttach(self->box, self->icon, 0, 0, 1);

    char *labelName = NULL;
    asprintf(&labelName, "%s.Label", self->widgetName);
    if (svSettingsIsWidgetDefined(labelName)) {
        if (self->label) {
            svWidgetDestroy(self->label);
            self->label = NULL;
        }

        if (self->labelBackground) {
            svWidgetDestroy(self->labelBackground);
            self->labelBackground = NULL;
        }

        char *labelBackgroundName = NULL;
        asprintf(&labelBackgroundName, "%s.Background", labelName);
        self->labelBackground = svSettingsWidgetCreate(w->app, labelBackgroundName);
        svSettingsWidgetAttach(self->box, self->labelBackground, labelBackgroundName, 1);
        svWidgetSetHidden(self->labelBackground, true);
        free(labelBackgroundName);

        self->label = QBAsyncLabelNew(w->app, labelName, self->movieDetails->ctx->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->box, self->label, labelName, 1);
    }
    free(labelName);
    if (self->label && self->obj) {
        if (SvObjectIsInstanceOf(self->obj, SvDBRawObject_getType())) {
            SvValue val = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) self->obj, "name");
            if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsString(val)) {
                QBAsyncLabelSetText(self->label, SvValueGetString(val));
                svWidgetSetHidden(self->labelBackground, false);
            } else {
                QBAsyncLabelSetText(self->label, NULL);
                svWidgetSetHidden(self->labelBackground, true);
            }
        }
    }


    svSettingsPopComponent();

    if (!object) {
        return;
    }

    if (SvObjectIsInstanceOf(object, SvBitmap_getType())) {
        svIconSetBitmap(self->icon, 2, (SvBitmap) object);
        svIconSwitch(self->icon, 2, 0, 0);
        return;
    }

    if (!QBVoDMovieDetailsLogicSimilarCarouselSlotSetObject(self->movieDetails->vodMovieDetailsLogic, object, w, self->icon)) {
        if (SvObjectIsInstanceOf(object, SvDBRawObject_getType()))
            QBMovieUtilsSetThumbnailIntoIcon(self->icon, (SvObject) object, "covers", "cover_name", "grid", 1);
    }
}

SvLocal SvWidget
QBVoDMovieDetailsSimilarCarouselSlotCreate(QBVoDMovieDetails movieDetails, SvApplication app, const char *widgetName)
{
    QBVoDMovieDetailsSimilarCarouselSlot self = calloc(1, sizeof(struct QBVoDMovieDetailsSimilarCarouselSlot_t));
    self->box = svWidgetCreateBitmap(app, 0, 0, NULL);
    self->box->prv = self;
    self->box->clean = QBVoDMovieDetailsSimilarCarouselSlotClean;
    self->settingsCtx = svSettingsSaveContext();
    self->widgetName = strdup(widgetName);
    self->box->width = svSettingsGetInteger(widgetName, "width", 0);
    self->box->height = svSettingsGetInteger(widgetName, "height", 0);
    self->movieDetails = movieDetails;
    self->label = NULL;
    self->labelBackground = NULL;

    return self->box;
}

SvLocal void QBVoDMovieDetailsFillSimilarCarousel(QBVoDMovieDetails self, const char *widgetName)
{
    QBVoDMovieDetailsSimilarCarousel similarCarousel = self->basicElements.similarCarousel->prv;
    SvWidget carousel = similarCarousel->similarCarousel;

    SvGenericObject category = SvInvokeInterface(QBTreeModel, self->ctx->tree, getNode, NULL);
    if (category && SvObjectIsInstanceOf(category, QBContentCategory_getType())) {
        QBContentCategoryAddListener((QBContentCategory) category, (SvGenericObject) self, NULL);
        QBContentCategorySetTitle((QBContentCategory) category, self->ctx->titleString, NULL);
    }

    int count = QBVerticalCarouselGetSlotCount(carousel);
    char *widgetCarouselSlotBuffer;
    asprintf(&widgetCarouselSlotBuffer, "%s.Carousel.Slot", widgetName);

    for (int i = 0; i < count; ++i) {
        QBVerticalCarouselSetContentAtIndex(carousel, i, QBVoDMovieDetailsSimilarCarouselSlotCreate(self, self->ctx->appGlobals->res, widgetCarouselSlotBuffer));
        QBVerticalCarouselSetDataHandler(carousel, i, QBVoDMovieDetailsSimilarCarouselSlotSetObject);
    }

    free(widgetCarouselSlotBuffer);
    similarCarousel->isVisible = false;

    QBVerticalCarouselSetFocusWidget(carousel, similarCarousel->focus);
    QBVerticalCarouselConnectToSource(carousel, self->similarDataSource, NULL);
}

SvLocal void QBVoDMovieDetailsFillSideDetails(QBVoDMovieDetails self)
{
    SvString val = NULL;
    int movieDetailsOffset = -1;

    if (self->movieDetails.title) {
        svWidgetDetach(self->movieDetails.title);
        svWidgetDestroy(self->movieDetails.title);
        self->movieDetails.title = NULL;
    }

    if (self->movieDetails.description) {
        svWidgetDetach(self->movieDetails.description);
        svWidgetDestroy(self->movieDetails.description);
        self->movieDetails.description = NULL;
    }

    if (self->movieDetails.info) {
        svWidgetDetach(self->movieDetails.info);
        svWidgetDestroy(self->movieDetails.info);
        self->movieDetails.info = NULL;
    }

    svSettingsRestoreContext(self->settingsCtx);

    val = QBMovieUtilsGetAttr(self->similarCarouselActiveElement, "name", NULL);
    if (val) {
        self->movieDetails.title = QBAsyncLabelNew(self->ctx->appGlobals->res, "MovieDetails.Title", self->ctx->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->details, self->movieDetails.title, "MovieDetails.Title", 1);
        QBAsyncLabelSetText(self->movieDetails.title, val);
    }

    val = QBMovieUtilsGetAttr(self->similarCarouselActiveElement, "description", NULL);
    if (val) {
        self->movieDetails.description = QBAsyncLabelNew(self->ctx->appGlobals->res, "MovieDetails.Description", self->ctx->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->details, self->movieDetails.description, "MovieDetails.Description", 1);

        QBAsyncLabelSetText(self->movieDetails.description, val);
    } else {
        movieDetailsOffset = svSettingsGetInteger("MovieDetails.Description", "yOffset", 0);
    }

    if (self->similarCarouselActiveElement) {
        self->movieDetails.info = QBMovieInfoCreate(self->ctx->appGlobals->res, "MovieDetails.Info", self->ctx->appGlobals->textRenderer);
        if (movieDetailsOffset >= 0)
            self->movieDetails.info->off_y = movieDetailsOffset;
        svSettingsWidgetAttach(self->details, self->movieDetails.info, "MovieDetails.Info", 1);
        QBMovieInfoSetObject(self->movieDetails.info, (SvGenericObject) self->similarCarouselActiveElement);
    }

    svSettingsPopComponent();
}

SvLocal bool QBVoDMovieDetailsInputEventHandler(SvWidget window, SvInputEvent e)
{
    QBVoDMovieDetails self = window->prv;
    if (!self->ctx->provider)
        return false;
    QBVoDMovieDetailsSimilarCarousel similarCarousel = self->basicElements.similarCarousel->prv;

    bool ret = QBVoDMovieDetailsLogicHandleInputEvent(self->ctx->movieDetails->vodMovieDetailsLogic, e, (SvGenericObject) self->ctx->movieDetails->ctx);
    if (ret) {
        return true;
    } else if (e->ch == QBKEY_FUNCTION) {
        QBVoDMovieDetailsCover cover = (QBVoDMovieDetailsCover) self->basicElements.cover->prv;
        QBVoDDetailsShowSideMenu(cover->movieDetails);
        return true;
    } else if (e->ch == QBKEY_UP && similarCarousel->isVisible && QBVerticalCarouselIsFocused(similarCarousel->similarCarousel)) {
        svWidgetSetFocus(self->basicElements.cover);
        QBVoDMovieDetailsSideDetailsHide(self);
        return true;
    } else if (e->ch == QBKEY_DOWN && similarCarousel->isVisible && !QBVerticalCarouselIsFocused(similarCarousel->similarCarousel)) {
        svWidgetSetFocus(similarCarousel->similarCarousel);
        QBVoDMovieDetailsSideDetaisShow(self);
        return true;
    }

    return false;
}

SvLocal void QBVoDMovieDetailsTimerEventHandler(SvWidget window, SvTimerEvent e)
{
    QBVoDMovieDetails self = window->prv;
    if (e->id == self->movieDetails.timer) {
        self->movieDetails.timer = 0;
        self->movieDetails.shown = true;

        if (!QBVoDMovieDetailsLogicShowObjectDetails(self->vodMovieDetailsLogic)) {
            QBVoDMovieDetailsFillSideDetails(self);
            svWidgetSetHidden(self->details, false);
        }

        return;
    }
}

SvLocal void QBVoDMovieDetailsClean(SvApplication app, void *self_)
{
    QBVoDMovieDetails self = self_;
    SVRELEASE(self);
}

SvLocal void QBVoDMovieDetailsContextDestroy(void *self_)
{
    QBVoDMovieDetailsContext self = self_;
    SVTESTRELEASE(self->titleString);
    SVTESTRELEASE(self->object);
    SVTESTRELEASE(self->category);
    SVTESTRELEASE(self->serviceId);
    SVRELEASE(self->serviceName);
    SVRELEASE(self->externalProvider);
    SVTESTRELEASE(self->tree);
    SVTESTRELEASE(self->provider);
    SVTESTRELEASE(self->actionName);
    SVRELEASE(self->vodLogic);
    SVTESTRELEASE(self->actions);
    SVTESTRELEASE(self->action);
}

SvLocal void QBVoDMovieDetailsContextReinitializeWindow(QBWindowContext self_, SvArray itemList)
{
    QBVoDMovieDetailsContext self = (QBVoDMovieDetailsContext) self_;

    if (self->movieDetails) {
        svSettingsRestoreContext(self->movieDetails->settingsCtx);
        QBVoDMovieDetailsFillMovieInfo(self->movieDetails);
        QBVoDMovieDetailsFillDescription(self->movieDetails, (SvObject) self->object);
        QBVoDMovieDetailsFillTitle(self->movieDetails);
        svSettingsPopComponent();
    }
}

SvLocal SvGenericObject QBVoDMovieDetailsCreateSimilarDataSource(SvGenericObject tree, SvBitmap listStartMarker)
{
    SvGenericObject similarDataSource = NULL;
    if (listStartMarker) {
        QBActiveArray staticItems = QBActiveArrayCreate(1, NULL);
        QBActiveArrayAddObject(staticItems, (SvGenericObject) listStartMarker, NULL);
        similarDataSource = (SvGenericObject) QBListProxyCreateWithStaticItems(tree, NULL /*root*/, (SvGenericObject) staticItems, NULL);
        SVRELEASE(staticItems);
    } else {
        similarDataSource = (SvGenericObject) QBListProxyCreate(tree, NULL /*root*/, NULL);
    }
    return similarDataSource;
}

SvLocal void QBVoDDetailsSetUpCarousel(QBVoDMovieDetails self)
{
    self->similarDataSource = QBVoDMovieDetailsCreateSimilarDataSource((SvGenericObject) self->ctx->tree, self->basicElements.listStartMarker);
    QBVoDMovieDetailsFillSimilarCarousel(self, svWidgetGetName(self->basicElements.similarCarousel));
}

SvLocal void
QBVoDMovieDetailsSetSimilarCarousel(QBVoDMovieDetails self)
{
    if (!self->basicElements.usesSimilarCarousel)
        return;

    if (!self->basicElements.similarCarousel) {
        self->basicElements.similarCarousel = QBVoDMovieDetailsSimilarCarouselCreate(self, self->ctx->appGlobals->res,
                SVSTRING("QBVoDMovieDetails.SimilarCarousel"), self->ctx->appGlobals->textRenderer, self->ctx->actionName);
        self->basicElements.similarCarouseInitialFocusColumn = svSettingsGetInteger("QBVoDMovieDetails.SimilarCarousel.Carousel", "initialFocusColumn", 3);
    }

    if (!self->basicElements.listStartMarker) {
        self->basicElements.listStartMarker = svSettingsGetBitmap("QBVoDMovieDetails.SimilarCarousel.Carousel.Slot", "bgListStartMarker");
    }
}

SvLocal void QBVoDMovieDetailsCreateRemainingWidgets(QBVoDMovieDetails self)
{
    if (!self->basicElements.cover) {
        self->basicElements.cover = QBVoDMovieDetailsCoverCreate(self, self->ctx->appGlobals->res, "QBVoDMovieDetails.Cover");
        svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.cover, "QBVoDMovieDetails.Cover", 1);
        QBVoDMovieDetailsFillCover(self->basicElements.cover, SVSTRING("covers"), SVSTRING("cover_name"), SVSTRING("movie_details"));
    }

    if (!self->basicElements.title) {
        self->basicElements.title = QBAsyncLabelNew(self->ctx->appGlobals->res, "QBVoDMovieDetails.Title", self->ctx->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.title, "QBVoDMovieDetails.Title", 1);
        QBVoDMovieDetailsFillTitle(self);
    }

    if (!self->basicElements.movieInfo && self->basicElements.usesMovieInfo) {
        self->basicElements.movieInfo = QBMovieInfoCreate(self->ctx->appGlobals->res, "QBVoDMovieDetails.MovieInfo", self->ctx->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.movieInfo, "QBVoDMovieDetails.MovieInfo", 1);
        QBVoDMovieDetailsFillMovieInfo(self);
    }

    if (!self->basicElements.trailerPreview && self->basicElements.usesTrailerPreview)
        QBVoDMovieDetailsFillTrailerPreview(self);

    if (!self->basicElements.description) {
        self->basicElements.description = QBAsyncLabelNew(self->ctx->appGlobals->res, "QBVoDMovieDetails.Description", self->ctx->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.description, "QBVoDMovieDetails.Description", 1);
        QBVoDMovieDetailsFillDescription(self, (SvObject) self->ctx->object);
    }

    QBVoDMovieDetailsSetSimilarCarousel(self);
}

SvLocal void QBVoDMovieDetailsContextCreateWindow(QBWindowContext ctx_, SvApplication app)
{
    QBVoDMovieDetailsContext ctx = (QBVoDMovieDetailsContext) ctx_;
    QBVoDMovieDetails self = (QBVoDMovieDetails) SvTypeAllocateInstance(QBVoDMovieDetails_getType(), NULL);
    self->ctx = ctx;

    svSettingsPushComponent("QBVoDMovieDetails.settings");
    self->settingsCtx = svSettingsSaveContext();

    SvWidget window = QBGUILogicCreateBackgroundWidget(ctx->appGlobals->guiLogic, "QBVoDMovieDetails.Window", NULL);
    svWidgetSetName(window, "QBVoDMovieDetails");

    svWidgetSetInputEventHandler(window, QBVoDMovieDetailsInputEventHandler);
    svWidgetSetTimerEventHandler(window, QBVoDMovieDetailsTimerEventHandler);
    window->clean = QBVoDMovieDetailsClean;
    window->prv = self;
    ctx->super_.window = window;

    SVTESTRELEASE(self->ctx->actionName);
    self->ctx->actionName = NULL;
    if (self->ctx->action) {
        self->ctx->actionName = QBMovieUtilsMapString(QBContentActionGetName(self->ctx->action, NULL));
    }

    self->vodMovieDetailsLogic = QBVoDMovieDetailsLogicNew(ctx->appGlobals, self, (SvGenericObject) self->ctx->object, self->ctx->actions, self->ctx->action);

    QBVoDMovieDetailsLogicBasicElementsInit(&self->basicElements);
    QBVoDMovieDetailsLogicCreateWidgets(self->vodMovieDetailsLogic, app, window, (SvGenericObject) self->ctx->object, &self->basicElements);
    QBVoDMovieDetailsCreateRemainingWidgets(self);
    svWidgetSetFocus(self->basicElements.cover);

    if (self->basicElements.similarCarousel)
        QBVoDDetailsSetUpCarousel(self);

    self->details = svSettingsWidgetCreate(app, "MovieDetails");
    svSettingsWidgetAttach(window, self->details, "MovieDetails", 4);
    svWidgetSetHidden(self->details, true);
    self->movieDetails.showDetailsDelay = svSettingsGetDouble("MovieDetails", "showDelay", 0);
    self->movieDetails.title = NULL;
    self->movieDetails.description = NULL;
    self->movieDetails.info = NULL;

    if (!(self->contentSideMenu = SVTESTRETAIN(QBVoDMovieDetailsLogicCreateContentMenu(self->vodMovieDetailsLogic)))) {
        self->contentSideMenu = (QBContextMenu) QBContentSideMenuCreate(ctx->appGlobals, SVSTRING("QBVoDMovieMenu.json"), SVSTRING("PC_VOD"));
        QBGUILogicContentSideMenuRegisterCallbacks(self->ctx->appGlobals->guiLogic, (SvObject) self->contentSideMenu);
    }

    svSettingsPopComponent();

    if (self->ctx->provider) {
        QBContentProviderStart((QBContentProvider) self->ctx->provider, self->ctx->appGlobals->scheduler);
        SvInvokeInterface(QBDataModel, self->similarDataSource, addListener, (SvGenericObject) self, NULL);
        QBVoDMovieDetailsEnsureSimilarCarousel(self);
    }

    QBProvidersControllerServiceAddListener(ctx->appGlobals->providersController, (SvObject) self, QBProvidersControllerService_INNOV8ON);
    QBAccessManagerAddListener(ctx->appGlobals->accessMgr, (SvObject) self, NULL);

    self->ctx->movieDetails = self;

    if (self->isTrailerAvailable && QBVoDMovieDetailsLogicIsInstantTrailerPreview(self->vodMovieDetailsLogic)) {
        QBVoDMovieDetailsTrailerPreviewPlay(self->basicElements.trailerPreview->prv);
    }

    QBApplicationControllerAddListener(ctx->appGlobals->controller, (SvObject) ctx);
}

SvLocal void QBVoDMovieDetailsContextDestroyWindow(QBWindowContext self_)
{
    QBVoDMovieDetailsContext self = (QBVoDMovieDetailsContext) self_;
    QBApplicationControllerRemoveListener(self->appGlobals->controller, (SvObject) self);
    QBVoDMovieDetails info = self->super_.window->prv;

    QBAccessManagerRemoveListener(self->appGlobals->accessMgr, (SvObject) info, NULL);
    if (info->ctx->provider)
        QBContentProviderStop((QBContentProvider) info->ctx->provider);

    svWidgetDetach(info->focus.cover);
    if (info->focus.trailerPreview)
        svWidgetDetach(info->focus.trailerPreview);

    svWidgetDestroy(info->focus.cover);
    if (info->focus.trailerPreview)
        svWidgetDestroy(info->focus.trailerPreview);

    if (info->basicElements.similarCarousel) {
        QBVoDMovieDetailsSimilarCarousel similarCarousel = info->basicElements.similarCarousel->prv;
        svWidgetDestroy(similarCarousel->focus);
        svWidgetDetach(similarCarousel->w);
        svWidgetDestroy(similarCarousel->w);
    }

    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal bool QBVoDMovieDetailsContextInputEventHandler(SvGenericObject self_, const QBInputEvent* e)
{
    return false;
}

SvLocal void
QBVoDMovieDetailsContextSwitchStarted(SvObject ctx_, QBWindowContext from, QBWindowContext to)
{
    QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_disabled, NULL);
}

SvLocal void
QBVoDMovieDetailsContextSwitchEnded(SvObject ctx_, QBWindowContext from, QBWindowContext to)
{
}


SvType QBVoDMovieDetailsContext_getType(void)
{
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_             = {
            .destroy        = QBVoDMovieDetailsContextDestroy
        },
        .reinitializeWindow = QBVoDMovieDetailsContextReinitializeWindow,
        .createWindow       = QBVoDMovieDetailsContextCreateWindow,
        .destroyWindow      = QBVoDMovieDetailsContextDestroyWindow
    };
    static const struct QBWindowContextInputEventHandler_t input = {
        .handleInputEvent = QBVoDMovieDetailsContextInputEventHandler
    };
    static const struct QBContextSwitcherListener_t switchMethods = {
        .started = QBVoDMovieDetailsContextSwitchStarted,
        .ended   = QBVoDMovieDetailsContextSwitchEnded
    };

    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBVoDMovieDetailsContext",
                            sizeof(struct QBVoDMovieDetailsContext_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            QBWindowContextInputEventHandler_getInterface(), &input,
                            QBContextSwitcherListener_getInterface(), &switchMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBVoDMovieDetailsItemsChanged(SvGenericObject self_, size_t start, size_t offset)
{
    QBVoDMovieDetails self = (QBVoDMovieDetails) self_;
    QBVoDMovieDetailsEnsureSimilarCarousel(self);
}

SvLocal void QBVoDDetailsShowSideMenu(QBVoDMovieDetails self)
{
    if (QBVoDMovieDetailsShouldBeBlocked(self)) {
        QBVoDMovieDetailsCheckParentalControlPIN(self, QBVoDMovieDetailsParentalControlCheck_showSideMenu);
        return;
    }

    svSettingsRestoreContext(self->settingsCtx);
    if (QBVoDMovieDetailsLogicShowContentMenu(self->vodMovieDetailsLogic))
        goto finish;

    if (QBContextMenuIsShown(self->contentSideMenu))
        goto finish;

    SvGenericObject contentInfo = QBVoDMovieDetailsContextCreateContentInfo((QBWindowContext) self->ctx);
    QBContentSideMenuSetContentInfo((QBContentSideMenu) self->contentSideMenu, contentInfo);
    SVRELEASE(contentInfo);
    QBContextMenuShow(self->contentSideMenu);

finish:
    svSettingsPopComponent();
}

SvLocal void
QBVoDMovieDetailsContextCheckState(QBVoDMovieDetails self, QBContentCategory category)
{

    //SvGenericObject firstItem = NULL;
    QBVoDMovieDetailsState newState = self->ctx->state;

    if (!SvObjectIsInstanceOf((SvObject) category, QBContentCategory_getType()))
        return;
    QBContentCategoryLoadingState categoryState = QBContentCategoryGetLoadingState(category);
    size_t itemsCnt = 0;

    itemsCnt = SvInvokeInterface(QBListModel, self->similarDataSource, getLength);

    if (self->animation) {
        svWidgetDetach(self->animation);
        svWidgetDestroy(self->animation);
        self->animation = NULL;
    }


    if (self->ctx->state != QBVoDMovieDetailsState_loaded) {
        //if (itemsCnt > 1)
          //  firstItem = SvInvokeInterface(QBListModel, self->similarDataSource, getObject, 1);

        if (true) {
            if (categoryState == QBContentCategoryLoadingState_idle || itemsCnt > 1) {
                newState = QBVoDMovieDetailsState_loaded;
            } else if (categoryState == QBContentCategoryLoadingState_initial || categoryState == QBContentCategoryLoadingState_active) {
                newState = QBVoDMovieDetailsState_loading;
                svSettingsRestoreContext(self->settingsCtx);
                self->animation = QBVoDMovieDetailsCreateAnimation(self->ctx->appGlobals->res, "QBVoDMovieDetails.Animation");
                svSettingsWidgetAttach(self->ctx->super_.window, self->animation, "QBVoDMovieDetails.Animation", 3);
                svSettingsPopComponent();
            } else {
                newState = QBVoDMovieDetailsState_noresults;
            }
        }
    }

    //SvLogNotice("%s, %s, len: %d, categoryState: %d, itemstate: %d, newstate: %d", __func__, QBContentCategoryGetName(self->category)->text, (int)itemsCnt, categoryState, self->state, newState);

    if (newState != self->ctx->state) {
        if (newState == QBVoDMovieDetailsState_loaded && itemsCnt > 1 && self->basicElements.similarCarousel)
            QBVoDMovieDetailsSimilarCarouselSetDefaultPosition(self);
        self->ctx->state = newState;
    }
}

SvLocal void
QBVoDMovieDetailsCategoryStateChanged(SvGenericObject self_,
                                      QBContentCategory category,
                                      QBContentCategoryLoadingState previousState,
                                      QBContentCategoryLoadingState currentState)
{
    QBVoDMovieDetails self = (QBVoDMovieDetails) self_;

    QBVoDMovieDetailsContextCheckState(self, category);
}

SvLocal void
QBVoDMovieDetailsServiceNotificationNoop(SvGenericObject self_, SvString serviceId)
{
}

SvLocal void
QBVoDMovieDetailsServiceRemoved(SvGenericObject self_, SvString serviceId)
{
    QBVoDMovieDetails self = (QBVoDMovieDetails) self_;
    if (SvStringEqualToCString(self->ctx->serviceId, SvStringCString(serviceId))) {
        QBApplicationControllerSwitchToRoot(self->ctx->appGlobals->controller);
        QBApplicationControllerPushContext(self->ctx->appGlobals->controller, self->ctx->appGlobals->main);
    }
}

SvLocal void
QBVoDMovieDetailsMiddlewareDataChanged(SvGenericObject self_,
                                      QBMiddlewareManagerType middlewareType)
{
    QBVoDMovieDetails self = (QBVoDMovieDetails) self_;

    if (middlewareType != QBMiddlewareManagerType_Innov8on) {
        return;
    }

    if (!QBMiddlewareManagerGetId(self->ctx->appGlobals->middlewareManager, middlewareType)) {
        QBApplicationControllerSwitchToRoot(self->ctx->appGlobals->controller);
        QBApplicationControllerPushContext(self->ctx->appGlobals->controller, self->ctx->appGlobals->main);
    }
}

SvLocal void QBVoDMovieDetails__dtor__(void *self_)
{
    QBVoDMovieDetails self = self_;

    self->ctx->movieDetails = NULL;
    SVRELEASE(self->contentSideMenu);
    SVTESTRELEASE(self->similarDataSource);
    SVTESTRELEASE(self->similarCarouselActiveElement);
    SVRELEASE(self->vodMovieDetailsLogic);

}

SvLocal void
QBVoDMovieDetailsAuthenticationChanged(SvObject self_, QBAccessManager manager, SvString domainName)
{
    QBVoDMovieDetails self = (QBVoDMovieDetails) self_;

    if (!SvStringEqualToCString(domainName, "PC_VOD")) {
        return;
    }

    svSettingsRestoreContext(self->settingsCtx);
    QBVoDMovieDetailsFillMovieInfo(self);
    QBVoDMovieDetailsFillDescription(self, (SvObject) self->ctx->object);
    QBVoDMovieDetailsFillTitle(self);
    QBVoDMovieDetailsFillCover(self->basicElements.cover, SVSTRING("covers"), SVSTRING("cover_name"), SVSTRING("movie_details"));
    if (!self->basicElements.trailerPreview && self->basicElements.usesTrailerPreview)
        QBVoDMovieDetailsFillTrailerPreview(self);
    svSettingsPopComponent();
}

SvLocal SvType QBVoDMovieDetails_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDMovieDetails__dtor__
    };
    static SvType type = NULL;

    static const struct QBContentCategoryListener_ categoryListenerMethods = {
        .loadingStateChanged = QBVoDMovieDetailsCategoryStateChanged,
    };

    static const struct QBListModelListener_t listModelListener = {
        .itemsAdded = QBVoDMovieDetailsItemsChanged,
        .itemsRemoved = QBVoDMovieDetailsItemsChanged,
        .itemsChanged = QBVoDMovieDetailsItemsChanged,
        .itemsReordered = QBVoDMovieDetailsItemsChanged,
    };

    static const struct QBProvidersControllerServiceListener_t providersControllerServiceMethods = {
        .serviceAdded = QBVoDMovieDetailsServiceNotificationNoop,
        .serviceRemoved = QBVoDMovieDetailsServiceRemoved,
        .serviceModified = QBVoDMovieDetailsServiceNotificationNoop
    };

    static const struct QBMiddlewareManagerListener_t middlewareListenerMethods = {
        .middlewareDataChanged = QBVoDMovieDetailsMiddlewareDataChanged
    };

    static const struct QBAccessManagerListener_t authenticationListenerMethods = {
        .authenticationChanged = QBVoDMovieDetailsAuthenticationChanged
    };

    if (!type) {
        SvTypeCreateManaged("QBVoDMovieDetails",
                            sizeof(struct QBVoDMovieDetails_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBListModelListener_getInterface(), &listModelListener,
                            QBContentCategoryListener_getInterface(), &categoryListenerMethods,
                            QBProvidersControllerServiceListener_getInterface(), &providersControllerServiceMethods,
                            QBMiddlewareManagerListener_getInterface(), &middlewareListenerMethods,
                            QBAccessManagerListener_getInterface(), &authenticationListenerMethods,
                            NULL);
    }

    return type;
}

QBWindowContext QBVoDMovieDetailsContextCreate(AppGlobals appGlobals, QBContentCategory category, SvDBRawObject object, SvGenericObject provider, SvString serviceId, SvString serviceName, SvObject externalTree)
{
    QBVoDMovieDetailsContext self = (QBVoDMovieDetailsContext) SvTypeAllocateInstance(QBVoDMovieDetailsContext_getType(), NULL);

    self->vodLogic = QBVoDLogicNew(appGlobals);

    self->ownerTree = externalTree;
    if (!self->ownerTree)
        self->ownerTree = (SvObject) appGlobals->menuTree;

    self->appGlobals = appGlobals;
    self->serviceId = SVTESTRETAIN(serviceId);
    self->serviceName = SVRETAIN(serviceName);
    self->externalProvider = SVRETAIN(provider);
    self->category = SVTESTRETAIN(category);
    self->movieDetails = NULL;


    SvString productId = SvValueGetString(SvDBObjectGetID((SvDBObject)object));
    self->tree = QBContentTreeCreate(productId, NULL);
    self->object = SVRETAIN(object);

    SvObject contentManager = QBContentProviderGetManager((QBContentProvider) provider);
    if (contentManager) {
        SvArray actions = SvInvokeInterface(QBContentManager, contentManager, constructActionsForContent, (SvGenericObject) object, NULL);
        if (actions) {

            self->actions = SvHashTableCreate(10, NULL);

            SvIterator it = SvArrayIterator(actions);
            QBContentAction act;
            QBContentAction defaultAction = NULL;

            while ((act = (QBContentAction) SvIteratorGetNext(&it))) {
                if (QBContentActionGetType(act, NULL) == QBContentActionType_relatedContent) {
                    if (!defaultAction || SvStringEqualToCString(QBContentActionGetName(act, NULL), "recommended"))
                        defaultAction = act;
                    SvHashTableInsert(self->actions, (SvGenericObject) QBContentActionGetName(act, NULL), (SvGenericObject) act);
                }
            }

            if (defaultAction) {
                self->provider = SvInvokeInterface(QBContentManager, contentManager, createActionContentProvider,
                                                   self->tree, provider, (SvGenericObject) object, defaultAction, NULL);
                self->action = SVRETAIN(defaultAction);
            } else {
                self->actionName = SVSTRING("");
            }
            SVRELEASE(actions);
        }
    } else {
        Innov8onProviderParams params = (Innov8onProviderParams) SvObjectCopy((SvGenericObject) Innov8onProviderGetParams((Innov8onProvider) self->externalProvider, NULL), NULL);
        SvGenericObject requestPlugin = Innov8onProviderRequestPluginGetRecommendationsCreate();
        Innov8onProviderParamsSetRequestPlugin(params, requestPlugin);
        SVRELEASE(requestPlugin);
        self->provider = (SvGenericObject) Innov8onProviderCreate(self->tree, params, self->serviceId, productId, false, false, NULL);
        SVRELEASE(params);
    }

    self->state = QBVoDMovieDetailsState_unknown;

    return (QBWindowContext) self;
}

SvGenericObject QBVoDMovieDetailsContextCreateContentInfo(QBWindowContext ctx_)
{
    QBVoDMovieDetailsContext self = (QBVoDMovieDetailsContext) ctx_;

    SvGenericObject contentInfo = QBContentInfoCreate(self->appGlobals, self->serviceId, self->serviceName, (SvGenericObject) self->object, self->category, self->externalProvider, self->ownerTree);
    return contentInfo;
}

SvGenericObject QBVoDMovieDetailsContextGetContentSideMenu(QBWindowContext ctx_)
{
    QBVoDMovieDetailsContext self = (QBVoDMovieDetailsContext) ctx_;

    return self->movieDetails ? (SvGenericObject) self->movieDetails->contentSideMenu : NULL;
}

void QBVoDMovieDetailsSetProviderData(QBVoDMovieDetails self, SvGenericObject provider, SvGenericObject tree, SvGenericObject action)
{
    if (self->basicElements.similarCarousel) {
        svWidgetDetach(self->basicElements.similarCarousel);
        svWidgetDestroy(self->basicElements.similarCarousel);
        self->basicElements.similarCarousel = NULL;
    }

    if (self->ctx->provider) {
        QBContentProviderStop((QBContentProvider) self->ctx->provider);
        SVRELEASE(self->ctx->provider);
    }
    self->ctx->provider = SVTESTRETAIN(provider);

    SVTESTRELEASE(self->ctx->tree);
    self->ctx->tree = (QBContentTree) SVTESTRETAIN(tree);

    if (action && SvObjectIsInstanceOf(action, QBContentAction_getType())) {
        SVTESTRELEASE(self->ctx->action);
        SVTESTRELEASE(self->ctx->actionName);
        self->ctx->action = (QBContentAction) SVRETAIN(action);
        self->ctx->actionName = QBMovieUtilsMapString(QBContentActionGetName(self->ctx->action, NULL));
    }

    self->ctx->state = QBVoDMovieDetailsState_unknown;

    svSettingsRestoreContext(self->settingsCtx);

    QBVoDMovieDetailsLogicSetUpCarousel(self->vodMovieDetailsLogic, &self->basicElements);
    QBVoDMovieDetailsSetSimilarCarousel(self);

    if (self->basicElements.similarCarousel && self->ctx->tree) {
        SVRELEASE(self->similarDataSource);
        QBVoDDetailsSetUpCarousel(self);
    }

    svSettingsPopComponent();

    if (self->ctx->provider) {
        QBContentProviderStart((QBContentProvider) self->ctx->provider, self->ctx->appGlobals->scheduler);
        SvInvokeInterface(QBDataModel, self->similarDataSource, addListener, (SvGenericObject) self, NULL);
        QBVoDMovieDetailsEnsureSimilarCarousel(self);
    }
}

QBVoDLogic QBVoDMovieDetailsGetVoDLogic(QBVoDMovieDetails self)
{
    return self->ctx->vodLogic;
}

SvGenericObject QBVoDMovieDetailsGetExternalProvider(QBVoDMovieDetails self)
{
    return self->ctx->externalProvider;
}

void
QBVoDMovieDetailsSetTitleString(QBVoDMovieDetails self, SvString titleString)
{
    SVTESTRELEASE(self->ctx->titleString);
    self->ctx->titleString = SVTESTRETAIN(titleString);
}
