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

#include "dlnaCarouselItemChoice.h"

#include <libintl.h>
#include <QBCarousel/QBCarousel.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentTreePath.h>
#include <QBOSK/QBOSKKey.h>
#include <QBOSK/QBOSK.h>
#include <settings.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBOSKPane.h>
#include <Menus/menuchoice.h>
#include <Utils/authenticators.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/XMBCarousel.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <Utils/QBMovieUtils.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <XMB2/XMBMenuBar.h>
#include <QBWidgets/QBGrid.h>
#include <Widgets/authDialog.h>
#include <Widgets/eventISMovieInfo.h>
#include <Windows/mainmenu.h>
#include <Windows/pvrplayer.h>
#include <Windows/imagebrowser.h>
#include <SvDataBucket2/SvDBReference.h>
#include <Logic/AnyPlayerLogic.h>
#include <player_hints/http_input.h>
#include <QBPlayerControllers/QBVoDAnyPlayerController.h>
#include <QBPlayerControllers/QBPlaybackStateController.h>
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include <ctype.h>
#include "Windows/imagebrowser/QBDLNAImageBrowserController.h"
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <main.h>
#include <QBAppKit/Utils/commonUtils.h>



struct QBDLNACarouselMenuChoice_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    struct QBCarouselSideMenu_t {
        QBContextMenu ctx;
    } sidemenu;

    SvGenericObject selected;
    SvString selectedOptionID;

    SvString serviceName;

    QBContentSearch contentSearch;
    SvGenericObject contentSearchPath;
    SvGenericObject path;
};

SvLocal void
QBCarouselPlayContent(QBDLNACarouselMenuChoice self, SvDBRawObject movie)
{
    if (!movie) {
        return;
    }

    bool lastPosition = SvStringEqualToCString(self->selectedOptionID, "Continue");
    bool playTrailer = SvStringEqualToCString(self->selectedOptionID, "Play Trailer");

    const char* attributeName = NULL;
    attributeName = "mediaURI";

    SvString URI = QBMovieUtilsGetAttr(movie, attributeName, NULL);
    if (!URI)
        return;

    SvString proxyURI = QBMovieUtilsGetAttr(movie, "proxyURI", NULL);

    QBMediaFileType type = QBMediaFileType_unknown;
    SvValue typeV = (SvValue) SvDBRawObjectGetAttrValue(movie, "metadata");
    SvString typeStr = (typeV && SvValueIsString(typeV)) ? SvValueGetString(typeV) : NULL;
    if (typeStr) {
        if (!strcmp(SvStringCString(typeStr), "audio"))
            type = QBMediaFileType_audio;
        else if (!strcmp(SvStringCString(typeStr), "video"))
            type = QBMediaFileType_video;
        else if (!strcmp(SvStringCString(typeStr), "playlist"))
            type = QBMediaFileType_audioPlaylist;
    }

    if (QBMediaFileType_unknown == type)
        type = QBAppKitURIToMediaFileType(URI, NULL);

    SvContent c = NULL;
    if (URI)
        SvLogNotice("URI = %s", SvStringCString(URI));

    SvString coverURI = NULL;
    if (type != QBMediaFileType_audioPlaylist) {
        c = SvContentCreateFromCString(SvStringCString(URI), NULL);
        SvValue titleV = (SvValue) SvDBRawObjectGetAttrValue(movie, "title");
        SvString title = NULL;
        if (titleV) {
            title = SvValueGetString(titleV);
        }
        if (title)
            SvContentMetaDataSetStringProperty(SvContentGetMetaData(c), SVSTRING(SV_PLAYER_META__TITLE), title);
        coverURI = QBMovieUtilsGetAttr(movie, "thumbnail", NULL);
    }

    if (proxyURI) {
        SvContentMetaDataSetStringProperty(SvContentGetHints(c), SVSTRING(PLAYER_HINT__HTTP_INPUT_PROXY), proxyURI);
    }

    QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->appGlobals);
    if (URI && !playTrailer && self->appGlobals->bookmarkManager) {
        QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, URI);
        if (!bookmark)
            bookmark = QBBookmarkManagerSetLastPositionBookmark(self->appGlobals->bookmarkManager, URI, 0.0, QBBookmarkType_Generic);

        if (bookmark)
            QBAnyPlayerLogicSetLastPositionBookmark(anyPlayerLogic, bookmark);
    }

    QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals, anyPlayerLogic);

    if (self->appGlobals->remoteControl) {
        QBPlaybackStateController playbackStateController = QBRemoteControlCreatePlaybackStateController(self->appGlobals->remoteControl, NULL);
        QBAnyPlayerLogicSetPlaybackStateController(anyPlayerLogic, (SvGenericObject) playbackStateController);
        SVRELEASE(playbackStateController);
    }

    QBPVRPlayerContextSetTitle(pvrPlayer, self->serviceName);
    QBPVRPlayerContextUseBookmarks(pvrPlayer, lastPosition);
    QBPVRPlayerContextSetVodMetaData(pvrPlayer, movie);
    if (coverURI) {
        QBPVRPlayerContextSetCoverURI(pvrPlayer, coverURI);
    }

    SvValue playerFeaturesV = NULL;
    SvDBReference ref = (SvDBReference) SvDBRawObjectGetAttrValue(movie, "metadata");
    SvDBRawObject metaData = (SvDBRawObject) (SvDBReferenceGetReferredObject(ref));
    playerFeaturesV = (SvValue) SvDBRawObjectGetAttrValue(metaData, "playerFeatures");
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
    QBApplicationControllerPushContext(self->appGlobals->controller, pvrPlayer);
    SVRELEASE(pvrPlayer);
}

SvLocal void
QBCarouselSideMenuHide(QBDLNACarouselMenuChoice self){
    QBContextMenuHide(self->sidemenu.ctx, false);
    SVTESTRELEASE(self->selected);
    self->selected = NULL;
}

SvLocal void
QBCarouselSideMenuPlay_(QBDLNACarouselMenuChoice self)
{
    SvDBRawObject movie = (SvDBRawObject) self->selected;

    QBCarouselPlayContent(self, movie);
    QBCarouselSideMenuHide(self);
}

SvLocal void
QBCarouselSideMenuPlayLastPosition(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBDLNACarouselMenuChoice self = self_;
    SVTESTRELEASE(self->selectedOptionID);
    self->selectedOptionID = SVRETAIN(id);
    QBCarouselSideMenuPlay_(self);
}

SvLocal void QBCarouselSideMenuClose(void *self_, QBContextMenu menu)
{
    QBDLNACarouselMenuChoice self = self_;
    SVTESTRELEASE(self->sidemenu.ctx);
    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBCarouselSideMenuClose, self);
}

SvLocal bool
QBCarouselHasLastPosition(QBDLNACarouselMenuChoice self, SvDBRawObject movie)
{
    SvValue productIdV = (SvValue) SvDBRawObjectGetAttrValue(movie, "mediaURI");
    if (!productIdV || !SvValueIsString(productIdV))
        return false;

    SvString productId = SvValueGetString(productIdV);
    return QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, productId) != NULL;
}

SvLocal void
QBCarouselSideMenuPlay(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBDLNACarouselMenuChoice self = self_;
    SVTESTRELEASE(self->selectedOptionID);
    self->selectedOptionID = SVRETAIN(id);
    QBCarouselSideMenuPlay_(self);
}

SvLocal bool
supports_bookmarks(SvGenericObject product)
{
    if (product && SvObjectIsInstanceOf((SvObject) product, SvDBRawObject_getType())) {
        SvString type = QBMovieUtilsGetAttr((SvDBRawObject) product, "type", NULL);
        if (!type)
            return false;

        return (SvStringEqualToCString(type, "video") || SvStringEqualToCString(type, "audio"));
    }
    return false;
}

SvLocal void QBDLNACarouselMenuChoiceSetupPane(QBDLNACarouselMenuChoice self, SvDBRawObject product, QBBasicPane pane)
{
    if (!self || !product || !pane) {
        SvLogWarning("%s(): NULL parameter passed! (self=%p, product=%p, pane=%p)", __func__, self, product, pane);
        return;
    }

    if (supports_bookmarks(self->selected)) {
        SvString playOption = NULL;
        if(QBCarouselHasLastPosition(self, product)){
            SvString option = SvStringCreate(gettext("Continue"), NULL);
            QBBasicPaneAddOption(pane, SVSTRING("Continue"), option, QBCarouselSideMenuPlayLastPosition, self);
            SVRELEASE(option);
            playOption = SvStringCreate(gettext("Play from the beginning"), NULL);
        } else {
            playOption = SvStringCreate(gettext("Play"), NULL);
        }
        QBBasicPaneAddOption(pane, SVSTRING("play"), playOption, QBCarouselSideMenuPlay, self);
        SVRELEASE(playOption);
    } else {
        SvString option = SvStringCreate(gettext("Play"), NULL);
        QBBasicPaneAddOption(pane, SVSTRING("Play"), option, QBCarouselSideMenuPlay, self);
        SVRELEASE(option);
    }
}

SvLocal void
QBCarouselMenuChoosen(SvGenericObject self_, SvGenericObject node, SvGenericObject nodePath_, int position)
{
    QBDLNACarouselMenuChoice self = (QBDLNACarouselMenuChoice) self_;
    SVTESTRELEASE(self->path);
    self->path = SVTESTRETAIN(nodePath_);
    if (SvObjectIsInstanceOf(node, SvDBRawObject_getType())) {
        SvDBRawObject product = (SvDBRawObject) node;
        SVTESTRELEASE(self->selected);
        self->selected = SVRETAIN(node);
        SvString URI = QBMovieUtilsGetAttr(product, "mediaURI", NULL);
        SvString source = QBMovieUtilsGetAttr(product, "source", NULL);

        if (URI || source){
            SvString type = NULL;
            product = (SvDBRawObject) self->selected;
            if (product && SvObjectIsInstanceOf((SvObject) self->selected, SvDBRawObject_getType())) {
                type = QBMovieUtilsGetAttr(product, "type", NULL);
            }

            if (type && SvStringEqualToCString(type, "image")) {
                SvGenericObject parentPath = SvObjectCopy(nodePath_, NULL);
                SvInvokeInterface(QBTreePath, parentPath, truncate, -1);

                QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, parentPath, 0);
                int nodePosition = 0;
                do {
                    if (SvObjectEquals(QBTreeIteratorGetCurrentNodePath(&iter), nodePath_))
                        break;
                    nodePosition++;
                } while (QBTreeIteratorGetNextNode(&iter));

                QBDLNAImageBrowserController controller = QBDLNAImageBrowserControllerCreate(self->appGlobals, NULL);
                QBWindowContext imageBrowser = QBImageBrowserContextCreate(self->appGlobals, parentPath, nodePosition, (SvObject) controller, false, -1);
                QBApplicationControllerPushContext(self->appGlobals->controller, imageBrowser);
                SVRELEASE(parentPath);
                SVRELEASE(imageBrowser);
                SVRELEASE(controller);
            } else {
                self = (QBDLNACarouselMenuChoice) self_;
                SVTESTRELEASE(self->selectedOptionID);
                self->selectedOptionID = SVRETAIN(nodePath_);

                svSettingsPushComponent("BasicPane.settings");
                QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
                QBBasicPaneInit(options, self->appGlobals->res,
                                self->appGlobals->scheduler,
                                self->appGlobals->textRenderer,
                                self->sidemenu.ctx, 1, SVSTRING("BasicPane"));

                QBDLNACarouselMenuChoiceSetupPane(self, product, options);

                QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
                QBContextMenuShow(self->sidemenu.ctx);
                SVRELEASE(options);
                svSettingsPopComponent();
            }
        }
    }
}

SvLocal void QBCarouselMenuOSKKeyTyped(void *self_, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBDLNACarouselMenuChoice self = (QBDLNACarouselMenuChoice) self_;

    if (key->type == QBOSKKeyType_enter) {
        if (input && SvStringLength(input) > 0) {
            const char* begin = SvStringCString(input);
            const char* end = begin + SvStringLength(input) - 1;
            while(*begin != '\0' && isspace(*begin))
                begin++;
            if(begin > end)
                return;
            while(end > begin && isspace(*end))
                end--;
            char* final = calloc(end - begin + 2, sizeof(char));
            strncpy(final, begin, end - begin + 1);
            SvString keyword = SvStringCreate(final, NULL);
            free(final);

            SvGenericObject path = self->contentSearchPath;
            QBContentSearchStartNewSearch(self->contentSearch, keyword, self->appGlobals->scheduler);
            SVRELEASE(keyword);
            QBTreeIterator tit = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, path, 0);

            path = QBTreeIteratorGetCurrentNodePath(&tit);

            if (path) {
                SvWidget menuBar = SvInvokeInterface(QBMainMenu, (SvObject) self->appGlobals->main, getMenu);
                SvInvokeInterface(QBMenu, menuBar->prv, setPosition, path, NULL);
            }

            QBContextMenuHide(self->sidemenu.ctx, false);

            SVTESTRELEASE(self->contentSearch);
            SVTESTRELEASE(self->contentSearchPath);
            self->contentSearch = NULL;
            self->contentSearchPath = NULL;

        }
    }
}


SvLocal bool
QBCarouselMenuNodeSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath)
{
    QBDLNACarouselMenuChoice self = (QBDLNACarouselMenuChoice) self_;

    if (SvObjectIsInstanceOf(node_, QBContentSearch_getType())) {
        SvString keyword;
        QBContentSearch search = (QBContentSearch) node_;
        QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);

        SVTESTRELEASE(self->contentSearch);
        SVTESTRELEASE(self->contentSearchPath);
        self->contentSearch = SVRETAIN(search);
        self->contentSearchPath = SvObjectCopy(nodePath, NULL);

        svSettingsPushComponent("OSKPane.settings");
        SvErrorInfo error = NULL;
        QBOSKPaneInit(oskPane, self->appGlobals->scheduler, self->sidemenu.ctx, 1, SVSTRING("OSKPane"), QBCarouselMenuOSKKeyTyped, self, &error);
        svSettingsPopComponent();
        if (!error) {
            keyword = QBContentSearchGetKeyword(search);
            if (keyword)
                QBOSKPaneSetRawInput(oskPane, SvStringCString(keyword));

            QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) oskPane);
            QBContextMenuShow(self->sidemenu.ctx);
            SVRELEASE(oskPane);

            return true;
        } else {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            SVRELEASE(oskPane);

            return false;
        }
    }

    return false;
}


SvLocal void
QBDLNACarouselMenuChoiceDestroy(void *self_)
{
    QBDLNACarouselMenuChoice self = self_;

    if (self->sidemenu.ctx) {
        QBContextMenuSetCallbacks(self->sidemenu.ctx, NULL, NULL);
        QBContextMenuHide(self->sidemenu.ctx, false);
        SVRELEASE(self->sidemenu.ctx);
    }

    SVTESTRELEASE(self->selected);

    SVTESTRELEASE(self->contentSearch);
    SVTESTRELEASE(self->contentSearchPath);
    SVTESTRELEASE(self->path);

    SVTESTRELEASE(self->serviceName);
}

SvLocal SvType
QBDLNACarouselMenuChoice_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDLNACarouselMenuChoiceDestroy
    };
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBCarouselMenuChoosen
    };

    static const struct QBMenuEventHandler_ selectedMethods = {
        .nodeSelected = QBCarouselMenuNodeSelected,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDLNACarouselMenuChoice",
                            sizeof(struct QBDLNACarouselMenuChoice_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBMenuEventHandler_getInterface(), &selectedMethods,
                            NULL);
    }

    return type;
}

QBDLNACarouselMenuChoice
QBDLNACarouselMenuChoiceNew(AppGlobals appGlobals, SvString serviceName)
{
    QBDLNACarouselMenuChoice self = (QBDLNACarouselMenuChoice) SvTypeAllocateInstance(QBDLNACarouselMenuChoice_getType(), NULL);

    self->appGlobals = appGlobals;
    self->serviceName = SVRETAIN(serviceName);

    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBCarouselSideMenuClose, self);

    return self;
}
