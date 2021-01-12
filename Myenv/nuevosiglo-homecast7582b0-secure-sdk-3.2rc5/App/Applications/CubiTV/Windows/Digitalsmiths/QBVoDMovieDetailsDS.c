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

#include <Windows/Digitalsmiths/QBVoDMovieDetailsDS.h>

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
#include <ContextMenus/QBInnov8onLoadablePane.h>
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
#include <QBInput/Filters/QBLongKeyPressFilter.h>
#include <QBInput/QBInputService.h>
#include <QBWindowContext.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SWL/QBFrame.h>
#include <SWL/move.h>
#include <Utils/authenticators.h>
#include <Utils/QBContentInfo.h>
#include <Utils/QBMovieUtils.h>
#include <Utils/value.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <Widgets/QBMovieInfo.h>
#include <Widgets/QBRatingWidget.h>
#include <Widgets/QBTitle.h>
#include <Widgets/authDialog.h>
#include <Widgets/SideMenu.h>
#include <Windows/pvrplayer.h>
#include <main.h>
#include <SWL/icon.h>
#include <player_hints/http_input.h>
#include <settings.h>
#include <stdbool.h>
#include <Widgets/QBTVPreview.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Logic/timeFormat.h>
#include <QBStringUtils.h>
#include <Services/QBAccessController/QBAccessManagerListener.h>
#include <Utils/QBVODUtils.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuController.h>
#include <XMB2/XMBVerticalMenu.h>
#include <XMB2/XMBIcon.h>
#include <QBMenu/QBMenu.h>
#include <Widgets/QBWidgetsStrip.h>
#include <QBMWClient/QBMWClientEventReporter.h>
#include <SvCore/SvEnv.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "QBVoDMovieDetailsLogLevel", "");

#define log_error(fmt, ...)   do { if (env_log_level() >= 0) SvLogError(COLBEG() "[%s] " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#define log_state(fmt, ...)   do { if (env_log_level() >= 1) SvLogNotice(COLBEG() "[%s] " fmt COLEND_COL(cyan), __func__, ## __VA_ARGS__); } while (0)
#define log_debug(fmt, ...)   do { if (env_log_level() >= 2) SvLogNotice("[%s] " fmt, __func__, ## __VA_ARGS__); } while (0)

typedef struct QBVoDMovieDetailsDS_t* QBVoDMovieDetailsDS;

/// basic elements of the window
struct QBVoDMovieDetailsDSBasicElements_s {
    SvWidget title;             ///< movie title
    SvWidget cover;             ///< movie cover
    SvWidget slider;            ///< widget used for sliding between episodes and recommendations
    SvWidget sliderCover;       ///< widget covering the carousel slided up
    SvWidget recommendedCarousel;   ///< recommendations carousel
    SvWidget episodesCarousel;  ///< series episodes carousel
    SvWidget castMenu;          ///< menu with cast member images
    SvWidget buttonsMenu;       ///< menu with myList button and trailer button
    SvWidget likeMenu;          ///< menu with like and dislike buttons
    SvWidget movieInfo;         ///< bar presenting movie metadata
    SvWidget description;       ///< movie description
    SvWidget backgroundImage;   ///< image displayed as background under the description
    SvWidget backgroundImageTint; ///< image tint that increases readability of text displayed on image

    SvBitmap listStartMarker;   ///< optional listm start marker for recommendations carousel

    int recommendedCarouseInitialFocusColumn; ///< column of the recommendations carousel that will be focussed first

    int sliderOriginalOffset;   ///< vertical offset of the slider when it is not slided
    int sliderSlidedOffset;     ///< vertical offset of the slider when it it slided

    bool usesMovieInfo;         ///< flag deciding if the movie info should be displayed
};

/// action to perform after parental control pin verification
typedef enum {
    QBVoDMovieDetailsDSParentalControlCheck_trailer = 0,  ///< play trailer
    QBVoDMovieDetailsDSParentalControlCheck_movieDetails, ///< show movie details (when recommended movie is blocked)
    QBVoDMovieDetailsDSParentalControlCheck_showSideMenu, ///< show the rent/play side menu
    QBVoDMovieDetailsDSParentalControlCheck_unlockRecommended ///< unlock the recommended carousel (adult content covers)
} QBVoDMovieDetailsDSParentalControlCheck;

/// foccused part of window - used when returning to window to focus the same element as when leaving the window
typedef enum {
    QBVoDMovieDetailsDSFocussedPart_none,           ///< no specific focus set
    QBVoDMovieDetailsDSFocussedPart_cast,           ///< cast menu was focused
    QBVoDMovieDetailsDSFocussedPart_episodes,       ///< episodes carousel was focused
    QBVoDMovieDetailsDSFocussedPart_recommended     ///< recommendations carousel was focused
} QBVoDMovieDetailsDSFocussedPart;

/// type of object displayed by the window
typedef enum {
    QBVoDMovieDetailsDSObjectType_movie = 0,    ///< movie
    QBVoDMovieDetailsDSObjectType_series,       ///< tv series object
    QBVoDMovieDetailsDSObjectType_episode,      ///< single episode of a series
    QBVoDMovieDetailsDSObjectType_person        ///< person (actor)
} QBVoDMovieDetailsDSObjectType;

/// context of details window
struct QBVoDMovieDetailsDSContext_t {
    struct QBWindowContext_t super_;            ///< superclass

    QBVoDMovieDetailsDS movieDetails;           ///< window handle

    SvDBRawObject object;                       ///< object displayed in the window
    QBVoDMovieDetailsDSObjectType objectType;   ///< type of displayed object
    QBContentProvider externalProvider;                  ///< provider that provided data for displayed object
    SvString serviceId;                         ///< id of the VoD service
    SvString serviceName;                       ///< name of the VoD service
    AppGlobals appGlobals;                      ///< application global data
    QBContentTree recommendationsTree;          ///< data tree for recommendations
    QBContentTree castTree;                     ///< data tree for cast members
    QBContentTree episodesTree;                 ///< data tree for series episodes
    QBContentTree detailsTree;                  ///< data tree for person details
    SvArray dataTrees;                          ///< data trees for providers that append data to current object (background image, user interaction data etc).
    SvHashTable dataProviders;                  ///< data sources to providers mapping
    QBContentProvider recommendedProvider;                   ///< data provider for recommendations carousel
    QBContentProvider castProvider;                      ///< data provider for cast members
    QBContentProvider detailsProvider;                   ///< data provider for details of a person
    QBContentProvider episodesProvider;                   ///< data provider for season episodes
    QBContentProvider userDataProvider;                  ///< data provider for user interaction details (like status, my list status)
    QBContentProvider seriesDefaultEpisodeImageProvider; ///< data provider for default episode image used on episodes without an episode specific image
    QBContentProvider backgroundImageProvider;           ///< data provider for movie background image
    SvObject eventReporter;                     ///< event reporter for reporting user interactions (like/dislike etc.)
    SvObject ownerTree;                         ///< external tree that the view is mounted in (for example main menu tree)
    QBVoDLogic vodLogic;                        ///< logic handle
    size_t selectedSeasonIndex;                 ///< season of the series that was selected in the episodes carousel
    size_t seasonsCount;                        ///< number of seasons in a series
    QBInnov8onFetchService mwFetchService;      ///< MW data fetching service
    bool doneLoadingOffers;                     ///< flag informing if offers data has been fetched from MW
    QBVoDMovieDetailsDSFocussedPart focusedPart; ///< part of the UI that is focused
    int focusedIndex;                           ///< index of the focus in the currently focused part if UI
    QBLongKeyPressFilter exitInputFilter;       ///< filter for handling 3sec exit button
    int castMaxCount;                           ///< max number of cast members
    bool defaultEpisodeImageQueried;            ///< flag informing if the default episode image was present in the object from the beginning of this context existence
};
typedef struct QBVoDMovieDetailsDSContext_t *QBVoDMovieDetailsDSContext;

/// slot of recommendations/episode carousel
typedef struct QBVoDMovieDetailsDSCarouselSlot_t {
    unsigned int settingsCtx;           ///< settings context id
    SvWidget box;                       ///< box under the movie image
    SvWidget icon;                      ///< image representing movie/episode
    SvWidget label;                     ///< label with the movie/episode title
    SvWidget labelBackground;           ///< background beneath the label

    SvObject obj;                       ///< object displayed in the slot
    char *widgetName;                   ///< name of widget in the settings file
    QBVoDMovieDetailsDS movieDetails;   ///< movie details window handle
} *QBVoDMovieDetailsDSCarouselSlot;

/// movie details window class
struct QBVoDMovieDetailsDS_t {
    struct SvObject_ super_;       ///< superclass
    QBVoDMovieDetailsDSContext ctx; ///< context handle

    unsigned int settingsCtx;       ///< settings context id

    SvObject recommendedDataSource;     ///< recommendations data source
    SvObject episodesDataSource;    ///< series episodes data source
    SvObject castDataSource;        ///< cast members data source
    SvObject buttonsDataSource;     ///< add to my list and trailer button data source
    SvObject likeDataSource;        ///< like/dislike buttons data source

    struct QBVoDMovieDetailsDSBasicElements_s basicElements; ///< basic widgets of the window

    SvWidget details;               ///< details widget (on the right of the cover)

    struct {
        SvWidget cover;             ///< focus frame
        SvWidget playImage;         ///< focus play button
    } focus;                        ///< cover focus widget

    struct {
        SvWidget title;             ///< movie/episode title or actor name
        SvWidget description;       ///< description
        SvWidget info;              ///< movie info (ratings, actors etc.)
    } movieDetails;                 ///< side menu with focused movie/episode/actpr details (for example recommended one)

    SvDBRawObject activeChildElement;   ///< focused child element (recommended movie, series episode or an actor)

    SvWidget parentalPopup;             ///< PC popup for blocked content

    SvObject contentSideMenu;           ///< side menu with options (Rent, Play, More info, Play Trailer etc.)
};

/// Movie cover struct
typedef struct QBVoDMovieDetailsDSCover_t {
    SvWidget w;         ///< main widget
    SvWidget cover;     ///< cover image
    int focusXOffset;   ///< x offset for the focus frame
    int focusYOffset;   ///< y offset for the focus frame

    QBVoDMovieDetailsDS movieDetails;   ///< movie details window handle
} *QBVoDMovieDetailsDSCover;

/// carousel widget
typedef struct QBVoDMovieDetailsDSCarousel_ {
    SvWidget w;                 ///< main widget of the carousel
    SvWidget carouselWidget;    ///< the carousel widget
    SvWidget title;             ///< title above carousel
    SvWidget titleFocus;        ///< focus wdget for the title
    SvWidget focus;             ///< focus widget for the carousel
    int xOffset;                ///< x offset of the carousel
    int yOffset;                ///< y offest of the carousel
    int titleFocusXOffset;      ///< x offest of the title focus
    int titleFocusYOffset;      ///< y offset of the title focus
    bool isVisible;             ///< true if carousel is visible and false otherwise

    QBVoDMovieDetailsDS movieDetails;   ///< movie details window handle
    QBContentProvider carouselProvider;          ///< carousel data provider
    int activeElementIndex;             ///< index of the element that is active
} *QBVoDMovieDetailsDSCarousel;

/// buttons menu
typedef struct QBVoDMovieDetailsDSMenu_ {
    SvWidget w;                 ///< main widget
    SvWidget menuWidget;        ///< menu widget
    SvWidget title;             ///< menu title
    SvWidget label;             ///< optional label under the menu (for example actor name under image)
    SvWidget labelBackground;   ///< background for optional label
    int xOffset;                ///< x offset of the menu
    int yOffset;                ///< y offset of the menu
    bool isVisible;             ///< true if menu is visible and false otherwise
    int activeElementIndex;     ///< index of focused element

    QBVoDMovieDetailsDS movieDetails;   ///< movie details window handle
} *QBVoDMovieDetailsDSMenu;

SvLocal SvType
QBVoDMovieDetailsDS_getType(void);
SvLocal void
QBVoDMovieDetailsDSFillSideDetails(QBVoDMovieDetailsDS self);
SvLocal SvObject
QBVoDMovieDetailsDSCreateRecommendedDataSource(SvObject tree, SvBitmap listStartMarker);
SvLocal void
QBVoDDetailsShowSideMenu(QBVoDMovieDetailsDS self);
SvLocal SvObject
QBVoDMovieDetailsDSContextCreateContentInfo(QBWindowContext ctx_);
SvLocal SvWidget
QBVoDMovieDetailsDSCarouselCreate(QBVoDMovieDetailsDS movieDetails,
                                  SvApplication app,
                                  SvString widgetName_,
                                  QBTextRenderer renderer,
                                  SvString title,
                                  QBContentProvider provider,
                                  SvUserEventHandler userEventHandler);
SvLocal void
QBVoDMovieDetailsDSFillMovieInfo(QBVoDMovieDetailsDS self);
SvLocal void
QBVoDDetailsSetUpButtonsMenu(QBVoDMovieDetailsDS self);
SvLocal bool
QBVoDMovieDetailsDSContextObjectIsBlocked(QBVoDMovieDetailsDSContext self, SvObject object);
SvLocal void
QBVoDMovieDetailsDSContextRefreshRecommended(QBVoDMovieDetailsDSContext self);
SvLocal void
QBVoDMovieDetailsDSSetBackground(QBVoDMovieDetailsDS self);

SvLocal void
QBVoDMovieDetailsDSInnov8onCallback(void *self_, QBInnov8onFetchService service, SvObject product)
{
    QBVoDMovieDetailsDSContext self = (QBVoDMovieDetailsDSContext) self_;
    self->doneLoadingOffers = true;

    if (self->movieDetails && self->movieDetails->basicElements.movieInfo && self->movieDetails->basicElements.usesMovieInfo) {
        QBVoDMovieDetailsDSFillMovieInfo(self->movieDetails);
    }
    if (self->movieDetails && self->movieDetails->buttonsDataSource) {
        QBVoDDetailsSetUpButtonsMenu(self->movieDetails);
    }

    SVTESTRELEASE(self->mwFetchService);
    self->mwFetchService = NULL;
}

/**
 * this function pulls data from MW.
 * It is used because data from DigitalSmiths does not have all the information needed
 * to buy and play a movie.
 **/
SvLocal void
QBVoDMovieDetailsDSPullMWData(QBVoDMovieDetailsDSContext self)
{
    SVTESTRELEASE(self->mwFetchService);
    self->mwFetchService = QBInnov8onFetchServiceCreate(self->appGlobals->middlewareManager, QBVoDMovieDetailsDSInnov8onCallback, self);
    //set the object that will recieve the data fetched from MW
    QBInnov8onFetchServiceSetRefreshObject(self->mwFetchService, true);
    QBInnov8onFetchServiceStart(self->mwFetchService, (SvObject) self->object, self->appGlobals->scheduler);
}

SvLocal void
QBVoDMovieDetailsDSPlayTrailer(QBVoDMovieDetailsDS self, SvDBRawObject movie, double position)
{
    if (!movie)
        return;

    const char* attributeName = "trailer";

    SvString URI = QBMovieUtilsGetAttr(movie, attributeName, NULL);

    if (!URI)
        return;

    SvString proxyURI = NULL;

    SvString coverURI = NULL;

    SvContent c = NULL;

    c = SvContentCreateFromCString(SvStringCString(URI), NULL);

    SvValue titleV = (SvValue) SvDBRawObjectGetAttrValue(movie, "title");
    SvString title = NULL;
    if (!titleV)
        titleV = (SvValue) SvDBRawObjectGetAttrValue(movie, "name");
    if (titleV && SvObjectIsInstanceOf((SvObject) titleV, SvValue_getType()) && SvValueIsString(titleV))
        title = SvValueGetString(titleV);

    if (title) {
        SvContentMetaDataSetStringProperty(SvContentGetMetaData(c),
                                           SVSTRING(SV_PLAYER_META__TITLE),
                                           title);
    }
    coverURI = QBMovieUtilsGetAttr(movie, "thumbnail", NULL);

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

    SvObject contentInfo = QBVoDMovieDetailsDSContextCreateContentInfo((QBWindowContext) self->ctx);
    QBPVRPlayerContextSetContentData(pvrPlayer, contentInfo);
    SVRELEASE(contentInfo);

    QBPVRPlayerContextSetTitle(pvrPlayer, self->ctx->serviceName);
    QBPVRPlayerContextSetVodMetaData(pvrPlayer, movie);
    QBPVRPlayerContextSetStartPosition(pvrPlayer, position);
    QBPVRPlayerContextUseBookmarks(pvrPlayer, false);
    QBPVRPlayerContextSetAuthenticated(pvrPlayer);
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

    QBPVRPlayerContextSetContent(pvrPlayer, c);

    SVTESTRELEASE(c);
    if (self->ctx->eventReporter) {
        SvInvokeInterface(QBMWClientEventReporter, self->ctx->eventReporter, sendEvent, self->ctx->appGlobals->scheduler, QBMWClientEventReporter_playTrailer, NULL);
    }
    QBApplicationControllerPushContext(self->ctx->appGlobals->controller, pvrPlayer);
    SVRELEASE(pvrPlayer);
}

SvLocal void
QBVoDMovieDetailsDSShowChildDetail(QBVoDMovieDetailsDS self, SvDBRawObject object, QBContentProvider provider, QBVoDMovieDetailsDSFocussedPart focusedPart)
{
    QBVoDMovieDetailsDSContext ctx = self->ctx;
    ctx->focusedPart = focusedPart;
    QBVoDMovieDetailsDSCarousel carousel = NULL;
    QBVoDMovieDetailsDSMenu menu = NULL;
    switch (focusedPart) {
        case QBVoDMovieDetailsDSFocussedPart_recommended:
            carousel = self->basicElements.recommendedCarousel->prv;
            ctx->focusedIndex = carousel->activeElementIndex;
            break;
        case QBVoDMovieDetailsDSFocussedPart_episodes:
            carousel = self->basicElements.episodesCarousel->prv;
            ctx->focusedIndex = carousel->activeElementIndex;
            break;
        case QBVoDMovieDetailsDSFocussedPart_cast:
            menu = self->basicElements.castMenu->prv;
            ctx->focusedIndex = menu->activeElementIndex;
            break;
        default:
            ctx->focusedIndex = 0;
    }
    QBWindowContext movieDetailsCtx = QBVoDMovieDetailsDSContextCreate(ctx->appGlobals,
                                                                       object,
                                                                       provider,
                                                                       ctx->serviceId,
                                                                       ctx->serviceName,
                                                                       ctx->ownerTree);
    QBApplicationControllerPushContext(ctx->appGlobals->controller, movieDetailsCtx);
    SVRELEASE(movieDetailsCtx);
}

SvLocal void
QBVoDMovieDetailsDSCheckParentalControlPINCallbackMovieDetails(void *ptr, SvWidget dlg,
                                                               SvString ret, unsigned key)
{
    QBVoDMovieDetailsDS self = ptr;

    self->parentalPopup = NULL;

    if (self->activeChildElement && ret && SvStringEqualToCString(ret, "OK-button")) {
        QBVoDMovieDetailsDSContext ctx = self->ctx;
        QBVoDMovieDetailsDSShowChildDetail(self, self->activeChildElement, ctx->recommendedProvider, QBVoDMovieDetailsDSFocussedPart_recommended);
    }
}

SvLocal void
QBVoDMovieDetailsDSCheckParentalControlPINCallbackTrailer(void *ptr, SvWidget dlg,
                                                          SvString ret, unsigned key)
{
    QBVoDMovieDetailsDS self = ptr;

    self->parentalPopup = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        QBVoDMovieDetailsDSPlayTrailer(self, self->ctx->object, 0.0);
    }
}

SvLocal void
QBVoDMovieDetailsDSCheckParentalControlPINCallbackShowSideMenu(void *ptr, SvWidget dlg,
                                                               SvString ret, unsigned key)
{
    QBVoDMovieDetailsDS self = ptr;

    if (!ret || !SvStringEqualToCString(ret, "OK-button"))
        return;

    svSettingsRestoreContext(self->settingsCtx);

    QBContentSideMenu contentSideMenu = (QBContentSideMenu) self->contentSideMenu;
    if (QBContextMenuIsShown((QBContextMenu) contentSideMenu))
        goto finish;

    SvObject contentInfo = QBVoDMovieDetailsDSContextCreateContentInfo((QBWindowContext) self->ctx);
    QBContentSideMenuSetContentInfo(contentSideMenu, contentInfo);
    QBContextMenuShow((QBContextMenu) contentSideMenu);
    SVRELEASE(contentInfo);

finish:
    svSettingsPopComponent();
}

SvLocal void
QBVoDMovieDetailsDSCheckParentalControlPINCallbackUnlockRecommended(void *ptr, SvWidget dlg,
                                                                    SvString ret, unsigned key)
{
    QBVoDMovieDetailsDS self = ptr;

    if (!ret || !SvStringEqualToCString(ret, "OK-button"))
        return;

    QBVoDMovieDetailsDSContextRefreshRecommended(self->ctx);
}

SvLocal void
QBVoDMovieDetailsDSCheckParentalControlPIN(QBVoDMovieDetailsDS self, QBVoDMovieDetailsDSParentalControlCheck checkType)
{
    svSettingsPushComponent("ParentalControl.settings");
    AppGlobals appGlobals = self->ctx->appGlobals;
    SvObject authenticator = NULL;
    authenticator = QBAuthenticateViaAccessManager(appGlobals->scheduler,
                                                   appGlobals->accessMgr,
                                                   SVSTRING("PC_VOD"));
    SvWidget master = NULL;
    SvWidget dialog = QBAuthDialogCreate(appGlobals, authenticator,
                                         gettext("Authentication required"),
                                         0, true, NULL, &master);

    self->parentalPopup = dialog;
    if (checkType == QBVoDMovieDetailsDSParentalControlCheck_movieDetails)
        QBDialogRun(dialog, self, QBVoDMovieDetailsDSCheckParentalControlPINCallbackMovieDetails);
    if (checkType == QBVoDMovieDetailsDSParentalControlCheck_trailer)
        QBDialogRun(dialog, self, QBVoDMovieDetailsDSCheckParentalControlPINCallbackTrailer);
    if (checkType == QBVoDMovieDetailsDSParentalControlCheck_showSideMenu)
        QBDialogRun(dialog, self, QBVoDMovieDetailsDSCheckParentalControlPINCallbackShowSideMenu);
    if (checkType == QBVoDMovieDetailsDSParentalControlCheck_unlockRecommended)
        QBDialogRun(dialog, self, QBVoDMovieDetailsDSCheckParentalControlPINCallbackUnlockRecommended);
    svSettingsPopComponent();
}

SvLocal bool
QBVoDMovieDetailsDSCheckParentalControl(QBVoDMovieDetailsDS self, SvDBRawObject movie)
{
    if (QBAccessManagerGetAuthenticationStatus(self->ctx->appGlobals->accessMgr, SVSTRING("PC_VOD"), NULL) == QBAuthStatus_OK)
        return true;

    if (QBParentalControlLogicAdultIsBlocked(self->ctx->appGlobals->parentalControlLogic) && QBVoDUtilsIsAdult((SvObject) movie))
        return false;

    SvString parentalControl = QBMovieUtilsGetAttr(movie, "ratings", NULL);
    bool allowed = false;

    if (!parentalControl || QBParentalControlRatingAllowed(self->ctx->appGlobals->pc, parentalControl))
        allowed = true;
    return allowed;
}

SvLocal void QBVoDMovieDetailsDSPlayTrailerCheck(QBVoDMovieDetailsDS self)
{
    if (QBVoDMovieDetailsDSCheckParentalControl(self, self->ctx->object)) {
        QBVoDMovieDetailsDSPlayTrailer(self, self->ctx->object, 0.0);
    } else {
        QBVoDMovieDetailsDSCheckParentalControlPIN(self, QBVoDMovieDetailsDSParentalControlCheck_trailer);
    }
}

SvLocal void QBVoDMovieDetailsDSCoverFocusEventHandler(SvWidget w, SvFocusEvent e)
{
    QBVoDMovieDetailsDSCover self = w->prv;
    if (e->kind == SvFocusEventKind_GET) {
        svWidgetAttach(self->w, self->movieDetails->focus.cover, self->focusXOffset, self->focusYOffset, 2);
        if (self->movieDetails->focus.playImage) {
            if (self->movieDetails->ctx->objectType != QBVoDMovieDetailsDSObjectType_person) {
                svWidgetAttach(self->w, self->movieDetails->focus.playImage, self->focusXOffset, self->focusYOffset, 3);
            }
        }
    } else {
        svWidgetDetach(self->movieDetails->focus.cover);
        if (self->movieDetails->focus.playImage) {
            svWidgetDetach(self->movieDetails->focus.playImage);
        }
    }
}

SvLocal bool QBVoDMovieDetailsDSCoverInputEventHandler(SvWidget w, SvInputEvent e)
{
    bool ret = false;
    QBVoDMovieDetailsDSCover self = w->prv;
    if (self->movieDetails->ctx->objectType == QBVoDMovieDetailsDSObjectType_person) {
        return false;
    }
    if (e->ch == QBKEY_ENTER) {
        QBVoDDetailsShowSideMenu(self->movieDetails);
        ret = true;
    }

    return ret;
}

SvLocal SvWidget QBVoDMovieDetailsDSCoverCreate(QBVoDMovieDetailsDS movieDetails, SvApplication app, const char *widgetName)
{
    QBVoDMovieDetailsDSCover self = calloc(1, sizeof(*self));
    self->w = svSettingsWidgetCreate(app, widgetName);
    self->w->prv = self;
    svWidgetSetFocusEventHandler(self->w, QBVoDMovieDetailsDSCoverFocusEventHandler);
    svWidgetSetInputEventHandler(self->w, QBVoDMovieDetailsDSCoverInputEventHandler);
    svWidgetSetFocusable(self->w, true);
    self->movieDetails = movieDetails;

    char *buf;
    asprintf(&buf, "%s.Image", widgetName);
    self->cover = svIconNew(app, buf);

    if (movieDetails->ctx->objectType == QBVoDMovieDetailsDSObjectType_person) {
        svIconSetBitmap(self->cover, 0, svSettingsGetBitmap(buf, "bgPerson"));
    }

    svSettingsWidgetAttach(self->w, self->cover, buf, 1);
    free(buf);

    asprintf(&buf, "%s.Focus", widgetName);
    self->movieDetails->focus.cover = QBFrameCreateFromSM(app, buf);
    self->focusXOffset = svSettingsGetInteger(buf, "xOffset", 0);
    self->focusYOffset = svSettingsGetInteger(buf, "yOffset", 0);
    free(buf);

    asprintf(&buf, "%s.PlayImage", widgetName);
    self->movieDetails->focus.playImage = svIconNew(app, buf);
    free(buf);

    return self->w;
}

SvLocal bool QBVoDMovieDetailsDSSetCarouselActiveElement(QBVoDMovieDetailsDS self, SvObject object, SvWidget originWidget)
{
    bool set = false;
    if (svWidgetIsFocused(originWidget)) {
        SVTESTRELEASE(self->activeChildElement);
        self->activeChildElement = NULL;
        if (SvObjectIsInstanceOf(object, SvDBRawObject_getType())) {
            self->activeChildElement = SVTESTRETAIN(object);
            set = true;
            if (!svWidgetIsHidden(self->details)) {
                bool isBlocked = QBVoDMovieDetailsDSContextObjectIsBlocked(self->ctx, (SvObject) self->activeChildElement);
                if (isBlocked) {
                    svWidgetSetHidden(self->details, true);
                } else {
                    QBVoDMovieDetailsDSFillSideDetails(self);
                }
            }
        }
    }

    return set;
}

SvLocal void QBVoDMovieDetailsDSCarouselUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e)
{
    QBVoDMovieDetailsDSCarousel self = w->prv;

    if (svWidgetGetId(self->carouselWidget) == src) {
        if (!self->isVisible) {
            return;
        }
        if (e->code == SV_EVENT_QB_CAROUSEL_NOTIFICATION) {
            QBVerticalCarouselNotification notify = e->prv;
            if (notify->type == QB_CAROUSEL_NOTIFICATION_ACTIVE_ELEMENT) {
                QBVoDMovieDetailsDSSetCarouselActiveElement(self->movieDetails, notify->dataObject, self->carouselWidget);
                self->activeElementIndex = notify->dataIndex;
            } else if (notify->type == QB_CAROUSEL_NOTIFICATION_SELECTED_ELEMENT) {
                if (notify->dataObject && SvObjectIsInstanceOf(notify->dataObject, SvDBRawObject_getType())) {
                    SvDBRawObject obj;
                    obj = (SvDBRawObject) notify->dataObject;

                    if (QBVoDMovieDetailsDSContextObjectIsBlocked(self->movieDetails->ctx, (SvObject) obj)) {
                        QBVoDMovieDetailsDSCheckParentalControlPIN(self->movieDetails, QBVoDMovieDetailsDSParentalControlCheck_unlockRecommended);
                    } else if (QBVoDMovieDetailsDSCheckParentalControl(self->movieDetails, obj)) {
                        QBVoDMovieDetailsDSFocussedPart focussedPart = QBVoDMovieDetailsDSFocussedPart_recommended;
                        if (self->movieDetails->basicElements.episodesCarousel && self == self->movieDetails->basicElements.episodesCarousel->prv) {
                            focussedPart = QBVoDMovieDetailsDSFocussedPart_episodes;
                        }
                        QBVoDMovieDetailsDSShowChildDetail(self->movieDetails, obj, self->carouselProvider, focussedPart);
                    } else {
                        QBVoDMovieDetailsDSCheckParentalControlPIN(self->movieDetails, QBVoDMovieDetailsDSParentalControlCheck_movieDetails);
                    }
                } else {
                    QBVoDMovieDetailsDSContext ctx = self->movieDetails->ctx;
                    QBWindowContext windowCtx = QBVoDLogicCreateContextForObject(ctx->vodLogic,
                                                                                 (SvObject) ctx->recommendationsTree,
                                                                                 (SvObject) self->carouselProvider,
                                                                                 NULL,
                                                                                 NULL,
                                                                                 notify->dataObject);
                    if (windowCtx) {
                        QBApplicationControllerPushContext(self->movieDetails->ctx->appGlobals->controller, windowCtx);
                        SVRELEASE(windowCtx);
                    }
                }
            }
        }
    }
}

SvLocal void QBVoDMovieDetailsDSEpisodesCarouselUpdateSeasonNumber(QBVoDMovieDetailsDSCarousel self, SvDBRawObject activeChildElement)
{
    if (activeChildElement) {
        SvValue seasonVal = (SvValue) SvDBRawObjectGetAttrValue(activeChildElement, "seasonNumber");
        if (seasonVal && SvObjectIsInstanceOf((SvObject) seasonVal, SvValue_getType()) && SvValueIsInteger(seasonVal)) {
            size_t seasonNumber = (size_t) SvValueGetInteger(seasonVal);
            self->movieDetails->ctx->selectedSeasonIndex = seasonNumber - 1;
            SvString seasonName = SvStringCreateWithFormat(gettext("Season: %zu"), seasonNumber);
            QBAsyncLabelSetText(self->title, seasonName);
            SVRELEASE(seasonName);
        }
    }
}

SvLocal void QBVoDMovieDetailsDSEpisodesCarouselUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e)
{
    QBVoDMovieDetailsDSCarouselUserEventHandler(w, src, e);

    QBVoDMovieDetailsDSCarousel self = w->prv;
    QBVoDMovieDetailsDSEpisodesCarouselUpdateSeasonNumber(self, self->movieDetails->activeChildElement);
}

//============================= XMB mneu item controller ==========================

/// item controller used for creating items in WidgetStrips.
typedef struct QBVoDMovieDetailsDSHorizontalMenuController_ {
    struct SvObject_ super_;    ///< superclass

    AppGlobals appGlobals;      ///< global data handle
    unsigned int settingsCtx;   ///< settings file context

    SvString widgetName;        ///< name of the parent widget in the settings file
} *QBVoDMovieDetailsDSHorizontalMenuController;

SvLocal void
QBVoDMovieDetailsDSHorizontalMenuController__dtor__(void* self_)
{
    QBVoDMovieDetailsDSHorizontalMenuController self = (QBVoDMovieDetailsDSHorizontalMenuController) self_;
    SVRELEASE(self->widgetName);
}

/**
 *  create the widget that will be embeded in the WidgetStrip given a node of data.
 **/
SvLocal SvWidget
QBVoDMovieDetailsDSHorizontalMenuControllerCreateItem(SvObject self_, SvObject node_, SvObject path, SvApplication app, XMBMenuState initialState)
{
    QBVoDMovieDetailsDSHorizontalMenuController self = (QBVoDMovieDetailsDSHorizontalMenuController) self_;

    SvString thumbnail = NULL;
    if (SvObjectIsInstanceOf(node_, SvDBRawObject_getType())) {
        thumbnail = QBMovieUtilsGetAttr((SvDBRawObject) node_, "thumbnail", NULL);
        if (!thumbnail) {
            return NULL;
        }
    }

    svSettingsRestoreContext(self->settingsCtx);

    char* buf;
    if (SvObjectIsInstanceOf(node_, SvString_getType())) {
        asprintf(&buf, "%s.Strip.Slot%s", SvStringCString(self->widgetName), SvStringCString((SvString) node_));
    } else {
        asprintf(&buf, "%s.Strip.Slot", SvStringCString(self->widgetName));
    }

    SvWidget icon = svIconNew(app, buf);

    if (thumbnail) {
        svIconSetBitmapFromURI(icon, 1, SvStringCString(thumbnail));
        if (!svIconIsBitmapAvailable(icon, 1))
            svIconSwitch(icon, 0, 0, 0.0);
        svIconSwitch(icon, 1, 0, -1.0);
    }
    free(buf);

    svSettingsPopComponent();

    return icon;
}

SvLocal void
QBVoDMovieDetailsDSHorizontalMenuControllerSetItemState(SvObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
}

SvLocal SvType
QBVoDMovieDetailsDSHorizontalMenuController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDMovieDetailsDSHorizontalMenuController__dtor__
    };
    static SvType type = NULL;

    static const struct XMBItemController_t controller_methods = {
        .createItem   = QBVoDMovieDetailsDSHorizontalMenuControllerCreateItem,
        .setItemState = QBVoDMovieDetailsDSHorizontalMenuControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDMovieDetailsDSHorizontalMenuController",
                            sizeof(struct QBVoDMovieDetailsDSHorizontalMenuController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            NULL);
    }

    return type;
}

SvLocal QBVoDMovieDetailsDSHorizontalMenuController
QBVoDMovieDetailsDSHorizontalMenuControllerCreate(AppGlobals appGlobals, const char *widgetName)
{
    QBVoDMovieDetailsDSHorizontalMenuController itemControler = (QBVoDMovieDetailsDSHorizontalMenuController) SvTypeAllocateInstance(
        QBVoDMovieDetailsDSHorizontalMenuController_getType(), NULL);

    itemControler->appGlobals = appGlobals;
    itemControler->settingsCtx = svSettingsSaveContext();
    itemControler->widgetName = SvStringCreate(widgetName, NULL);

    return itemControler;
}

//============================= END - XMB mneu item controller ==========================

SvLocal bool
QBVoDDetailsGetBoolAttribute(SvDBRawObject object, const char* name)
{
    if (!object) {
        return false;
    }
    SvValue retVal = (SvValue) SvDBRawObjectGetAttrValue(object, name);
    if (!retVal || !SvObjectIsInstanceOf((SvObject) retVal, SvValue_getType())) {
        return false;
    }
    if (SvValueIsBoolean(retVal)) {
        return SvValueGetBoolean(retVal);
    } else if (SvValueIsInteger(retVal)) {
        return (bool) SvValueGetInteger(retVal);
    } else if (SvValueIsDouble(retVal)) {
        return SvValueGetDouble(retVal) > 0.0;
    }
    return false;
}

SvLocal void
QBVoDDetailsSetBoolAttribute(SvDBRawObject object, const char* name, bool val)
{
    if (!object) {
        return;
    }
    SvValue valVal = SvValueCreateWithBoolean(val, NULL);
    SvDBRawObjectSetAttrValue(object, name, (SvObject) valVal);
    SVRELEASE(valVal);
}

SvLocal void
QBVoDMovieDetailsDSLiked(QBVoDMovieDetailsDS movieDetails, QBMWClientEventReporterState state)
{
    if (state == QBMWClientEventReporterState_finished) {
        QBActiveArray array = (QBActiveArray) movieDetails->likeDataSource;
        SvString buttonId = SvStringCreate("LikeButtonLit", NULL);
        QBActiveArraySetObjectAtIndex(array, 0, (SvObject) buttonId, NULL);
        SVRELEASE(buttonId);

        buttonId = SvStringCreate("DislikeButton", NULL);
        QBActiveArraySetObjectAtIndex(array, 1, (SvObject) buttonId, NULL);
        SVRELEASE(buttonId);

        QBVoDDetailsSetBoolAttribute(movieDetails->ctx->object, "likedByUser()", true);
        QBVoDDetailsSetBoolAttribute(movieDetails->ctx->object, "dislikedByUser()", false);
    } else if (state == QBMWClientEventReporterState_error) {
        log_error("Unable to Like event");
    }
}

SvLocal void
QBVoDMovieDetailsDSDisliked(QBVoDMovieDetailsDS movieDetails, QBMWClientEventReporterState state)
{
    if (state == QBMWClientEventReporterState_finished) {
        QBActiveArray array = (QBActiveArray) movieDetails->likeDataSource;
        SvString buttonId = SvStringCreate("LikeButton", NULL);
        QBActiveArraySetObjectAtIndex(array, 0, (SvObject) buttonId, NULL);
        SVRELEASE(buttonId);

        buttonId = SvStringCreate("DislikeButtonLit", NULL);
        QBActiveArraySetObjectAtIndex(array, 1, (SvObject) buttonId, NULL);
        SVRELEASE(buttonId);

        QBVoDDetailsSetBoolAttribute(movieDetails->ctx->object, "likedByUser()", false);
        QBVoDDetailsSetBoolAttribute(movieDetails->ctx->object, "dislikedByUser()", true);
    } else if (state == QBMWClientEventReporterState_error) {
        log_error("Unable to Dislike event");
    }
}

SvLocal void
QBVoDMovieDetailsDSClearedLike(QBVoDMovieDetailsDS movieDetails, QBMWClientEventReporterState state)
{
    if (state == QBMWClientEventReporterState_finished) {
        QBActiveArray array = (QBActiveArray) movieDetails->likeDataSource;
        SvString buttonId = SvStringCreate("LikeButton", NULL);
        QBActiveArraySetObjectAtIndex(array, 0, (SvObject) buttonId, NULL);
        SVRELEASE(buttonId);

        buttonId = SvStringCreate("DislikeButton", NULL);
        QBActiveArraySetObjectAtIndex(array, 1, (SvObject) buttonId, NULL);
        SVRELEASE(buttonId);

        QBVoDDetailsSetBoolAttribute(movieDetails->ctx->object, "likedByUser()", false);
        QBVoDDetailsSetBoolAttribute(movieDetails->ctx->object, "dislikedByUser()", false);
    } else if (state == QBMWClientEventReporterState_error) {
        log_error("Unable to clear Like/Dislike event");
    }
}

SvLocal void
QBVoDMovieDetailsDSAddedToMyList(QBVoDMovieDetailsDS movieDetails, QBMWClientEventReporterState state)
{
    if (state == QBMWClientEventReporterState_finished) {
        QBActiveArray array = (QBActiveArray) movieDetails->buttonsDataSource;
        SvString litButton = SvStringCreate("MyListButtonLit", NULL);
        QBActiveArraySetObjectAtIndex(array, 0, (SvObject) litButton, NULL);
        SVRELEASE(litButton);

        QBVoDDetailsSetBoolAttribute(movieDetails->ctx->object, "inUserList(myList)", true);
    } else {
        log_error("Unable to add to My List");
    }
}

SvLocal void
QBVoDMovieDetailsDSRemovedFromMyList(QBVoDMovieDetailsDS movieDetails, QBMWClientEventReporterState state)
{
    if (state == QBMWClientEventReporterState_finished) {
        QBActiveArray array = (QBActiveArray) movieDetails->buttonsDataSource;
        SvString litButton = SvStringCreate("MyListButton", NULL);
        QBActiveArraySetObjectAtIndex(array, 0, (SvObject) litButton, NULL);
        SVRELEASE(litButton);

        QBVoDDetailsSetBoolAttribute(movieDetails->ctx->object, "inUserList(myList)", false);
    } else {
        log_error("Unable to remove from My List");
    }
}


SvLocal void
QBVoDMovieDetailsDSContextRequestStateChanged(SvObject self_,
                                              SvObject req,
                                              QBMWClientEventReporterEventType eventType,
                                              QBMWClientEventReporterState state)
{
    QBVoDMovieDetailsDSContext self = (QBVoDMovieDetailsDSContext) self_;
    if (!self->movieDetails) {
        return;
    }

    if (eventType == QBMWClientEventReporter_like) {
        QBVoDMovieDetailsDSLiked(self->movieDetails, state);
    } else if (eventType == QBMWClientEventReporter_dislike) {
        QBVoDMovieDetailsDSDisliked(self->movieDetails, state);
    } else if (eventType == QBMWClientEventReporter_clearLike) {
        QBVoDMovieDetailsDSClearedLike(self->movieDetails, state);
    } else if (eventType == QBMWClientEventReporter_addToMyList) {
        QBVoDMovieDetailsDSAddedToMyList(self->movieDetails, state);
    } else if (eventType == QBMWClientEventReporter_removeFromMyList) {
        QBVoDMovieDetailsDSRemovedFromMyList(self->movieDetails, state);
    }
}

SvLocal void
QBVoDMovieDetailsDSAddToMyList(QBVoDMovieDetailsDS movieDetails)
{
    if (movieDetails->ctx->eventReporter) {
        SvInvokeInterface(QBMWClientEventReporter, movieDetails->ctx->eventReporter, sendEvent, movieDetails->ctx->appGlobals->scheduler, QBMWClientEventReporter_addToMyList, NULL);
    }
}

SvLocal void
QBVoDMovieDetailsDSRemoveFromMyList(QBVoDMovieDetailsDS movieDetails)
{
    if (movieDetails->ctx->eventReporter) {
        SvInvokeInterface(QBMWClientEventReporter, movieDetails->ctx->eventReporter, sendEvent, movieDetails->ctx->appGlobals->scheduler, QBMWClientEventReporter_removeFromMyList, NULL);
    }
}

SvLocal void
QBVoDMovieDetailsDSLike(QBVoDMovieDetailsDS movieDetails)
{
    if (movieDetails->ctx->eventReporter) {
        SvInvokeInterface(QBMWClientEventReporter, movieDetails->ctx->eventReporter, sendEvent, movieDetails->ctx->appGlobals->scheduler, QBMWClientEventReporter_like, NULL);
    }
}

SvLocal void
QBVoDMovieDetailsDSDislike(QBVoDMovieDetailsDS movieDetails)
{
    if (movieDetails->ctx->eventReporter) {
        SvInvokeInterface(QBMWClientEventReporter, movieDetails->ctx->eventReporter, sendEvent, movieDetails->ctx->appGlobals->scheduler, QBMWClientEventReporter_dislike, NULL);
    }
}

SvLocal void
QBVoDMovieDetailsDSClearLikeStatus(QBVoDMovieDetailsDS movieDetails)
{
    if (movieDetails->ctx->eventReporter) {
        SvInvokeInterface(QBMWClientEventReporter, movieDetails->ctx->eventReporter, sendEvent, movieDetails->ctx->appGlobals->scheduler, QBMWClientEventReporter_clearLike, NULL);
    }
}

SvLocal void QBVoDMovieDetailsDSMenuSetLabelText(QBVoDMovieDetailsDSMenu self, SvWidget widget, SvString text)
{
    if (text && widget) {
        QBVoDMovieDetailsDSContext ctx = self->movieDetails->ctx;
        int x_offset = widget->off_x + widget->width / 2;

        static const char* labelName = "QBVoDMovieDetails.CastMenu.ActorLabel";
        static const char* labelBackgroundName = "QBVoDMovieDetails.CastMenu.ActorLabel.Background";

        svSettingsRestoreContext(self->movieDetails->settingsCtx);
        if (self->label != NULL) {
            svWidgetDetach(self->label);
            if (self->labelBackground) {
                svWidgetDetach(self->labelBackground);
            }
        } else {
            self->label = QBAsyncLabelNew(ctx->appGlobals->res, labelName, ctx->appGlobals->textRenderer);

            if (svSettingsIsWidgetDefined(labelBackgroundName)) {
                self->labelBackground = svSettingsWidgetCreate(ctx->appGlobals->res, labelBackgroundName);
            }
        }

        int labelXOffset = svSettingsGetInteger(labelName, "xOffset", 0);
        int labelYOffset = svSettingsGetInteger(labelName, "yOffset", 0);
        int backgroundXOffset = 0;
        int backgroundYOffset = 0;
        if (self->labelBackground) {
            backgroundXOffset = svSettingsGetInteger(labelBackgroundName, "xOffset", 0);
            backgroundYOffset = svSettingsGetInteger(labelBackgroundName, "yOffset", 0);
        }

        svSettingsPopComponent();

        if (self->labelBackground) {
            svWidgetAttach(self->w, self->labelBackground, (x_offset - self->labelBackground->width / 2) + backgroundXOffset, backgroundYOffset, 1);
        }
        QBAsyncLabelSetText(self->label, text);
        svWidgetAttach(self->w, self->label, (x_offset - self->label->width / 2) + labelXOffset, labelYOffset, 2);
    } else {
        if (self->label) {
            svWidgetDetach(self->label);
            svWidgetDestroy(self->label);
            self->label = NULL;
        }
        if (self->labelBackground) {
            svWidgetDetach(self->labelBackground);
            svWidgetDestroy(self->labelBackground);
            self->labelBackground = NULL;
        }
    }
}

SvLocal void QBVoDMovieDetailsDSHorizontalMenuUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e)
{
    QBVoDMovieDetailsDSMenu self = w->prv;

    if (svWidgetGetId(self->menuWidget) == src) {
        if (!self->isVisible) {
            return;
        }
        if (e->code == QB_WIDGETS_STRIP_NOTIFICATION) {
            QBWidgetsStripNotification notify = e->prv;
            if (notify->type == QB_WIDGETS_STRIP_NOTIFICATION_SELECTED_ELEMENT) {
                if (notify->dataObject && SvObjectIsInstanceOf(notify->dataObject, SvDBRawObject_getType())) {
                    QBVoDMovieDetailsDSContext ctx = self->movieDetails->ctx;
                    SvDBRawObject obj;
                    obj = (SvDBRawObject) notify->dataObject;
                    self->activeElementIndex = notify->widgetIndex;

                    if (QBVoDMovieDetailsDSCheckParentalControl(self->movieDetails, obj)) {
                        QBVoDMovieDetailsDSShowChildDetail(self->movieDetails, obj, ctx->castProvider, QBVoDMovieDetailsDSFocussedPart_cast);
                    } else {
                        QBVoDMovieDetailsDSCheckParentalControlPIN(self->movieDetails, QBVoDMovieDetailsDSParentalControlCheck_movieDetails);
                    }
                } else {
                    if (notify->dataObject && SvObjectIsInstanceOf(notify->dataObject, SvString_getType())) {
                        SvString option = (SvString) notify->dataObject;
                        if (SvStringEqualToCString(option, "MyListButton")) {
                            QBVoDMovieDetailsDSAddToMyList(self->movieDetails);
                        } else if (SvStringEqualToCString(option, "MyListButtonLit")) {
                            QBVoDMovieDetailsDSRemoveFromMyList(self->movieDetails);
                        } else if (SvStringEqualToCString(option, "TrailerButton")) {
                            QBVoDMovieDetailsDSPlayTrailerCheck(self->movieDetails);
                        } else if (SvStringEqualToCString(option, "LikeButton")) {
                            QBVoDMovieDetailsDSLike(self->movieDetails);
                        } else if (SvStringEqualToCString(option, "DislikeButton")) {
                            QBVoDMovieDetailsDSDislike(self->movieDetails);
                        } else if (SvStringEqualToCString(option, "DislikeButtonLit")
                                   || SvStringEqualToCString(option, "LikeButtonLit")) {
                            QBVoDMovieDetailsDSClearLikeStatus(self->movieDetails);
                        }
                    }
                }
            } else if (notify->type == QB_WIDGETS_STRIP_NOTIFICATION_ACTIVE_ELEMENT) {
                if (self == self->movieDetails->basicElements.castMenu->prv && notify->dataObject && SvObjectIsInstanceOf(notify->dataObject, SvDBRawObject_getType())) {
                    if (QBVoDMovieDetailsDSSetCarouselActiveElement(self->movieDetails, notify->dataObject, self->menuWidget)) {
                        self->activeElementIndex = notify->widgetIndex;
                    }
                    SvString name = QBMovieUtilsGetAttr((SvDBRawObject) notify->dataObject, "name", NULL);
                    QBVoDMovieDetailsDSMenuSetLabelText(self, notify->childWidget, name);
                }
            } else if (notify->type == QB_WIDGETS_STRIP_NOTIFICATION_FOCUS_LOST) {
                if (self == self->movieDetails->basicElements.castMenu->prv) {
                    QBVoDMovieDetailsDSMenuSetLabelText(self, notify->childWidget, NULL);
                }
            }
        }
    }
}

SvLocal SvWidget
QBVoDMovieDetailsDSHorizontalMenuCreate(QBVoDMovieDetailsDS movieDetails, SvApplication app, SvString widgetName_, QBTextRenderer renderer, SvString title)
{
    QBVoDMovieDetailsDSMenu self = calloc(1, sizeof(*self));

    self->movieDetails = movieDetails;

    const char* widgetName = SvStringCString(widgetName_);
    self->w = svSettingsWidgetCreate(app, widgetName);
    self->w->prv = self;
    svWidgetSetUserEventHandler(self->w, QBVoDMovieDetailsDSHorizontalMenuUserEventHandler);
    self->xOffset = svSettingsGetInteger(widgetName, "xOffset", 0);
    self->yOffset = svSettingsGetInteger(widgetName, "yOffset", 0);

    char *buf;
    asprintf(&buf, "%s.Strip", widgetName);
    self->menuWidget = QBWidgetsStripNew(app, buf, NULL);
    QBWidgetsStripSetNotificationTarget(self->menuWidget, svWidgetGetId(self->w));
    svSettingsWidgetAttach(self->w, self->menuWidget, buf, 1);
    free(buf);
    svWidgetSetFocusable(self->menuWidget, true);

    asprintf(&buf, "%s.Title", widgetName);
    self->title = QBAsyncLabelNew(app, buf, renderer);
    QBAsyncLabelSetText(self->title, title);
    svSettingsWidgetAttach(self->w, self->title, buf, 10);
    free(buf);

    return self->w;
}

SvLocal void QBVoDMovieDetailsStopDataProviders(QBVoDMovieDetailsDSContext self)
{
    if (self->dataProviders) {
        SvIterator iter = SvHashTableValuesIterator(self->dataProviders);
        QBContentProvider provider = NULL;
        while ((provider = (QBContentProvider) SvIteratorGetNext(&iter))) {
            QBContentProviderStop(provider);
        }
        SVRELEASE(self->dataProviders);
        self->dataProviders = NULL;
    }
    SVTESTRELEASE(self->dataTrees);
    self->dataTrees = NULL;
}

SvLocal SvObject
QBVoDMovieDetailsDSContextCreateActionProvider(QBVoDMovieDetailsDSContext self, SvObject contentManager, SvDBObject object, QBContentAction action)
{
    if (!self->dataTrees) {
        self->dataTrees = SvArrayCreate(NULL);
    }
    if (!self->dataProviders) {
        self->dataProviders = SvHashTableCreate(11, NULL);
    }
    SvString productId = SvValueGetString(SvDBObjectGetID((SvDBObject) object));
    QBContentTree dataTree = QBContentTreeCreate(productId, NULL);
    SvArrayAddObject(self->dataTrees, (SvObject) dataTree);
    SvObject provider = SvInvokeInterface(QBContentManager, contentManager, createActionContentProvider,
                                          dataTree, (SvObject) self->externalProvider, (SvObject) object, action, NULL);
    SvObject dataSource = (SvObject) QBListProxyCreate((SvObject) dataTree, NULL /*root*/, NULL);
    SvHashTableInsert(self->dataProviders, dataSource, provider);
    if (self->movieDetails) {
        SvInvokeInterface(QBDataModel, dataSource, addListener, (SvObject) self->movieDetails, NULL);
    }

    SVRELEASE(dataSource);
    SVRELEASE(dataTree);
    return provider;
}

SvLocal void
QBVoDMovieDetailsDSItemsChanged(SvObject self_, SvObject dataSource, size_t start, size_t offset);

SvLocal void
QBVoDMovieDetailsDSContextConnectDataSources(QBVoDMovieDetailsDSContext self)
{
    if (!self->movieDetails || !self->dataProviders) {
        return;
    }
    SvIterator iter = SvHashTableKeysIterator(self->dataProviders);
    SvObject dataSource = NULL;
    while ((dataSource = SvIteratorGetNext(&iter)) != NULL) {
        SvInvokeInterface(QBDataModel, dataSource, addListener, (SvObject) self->movieDetails, NULL);
        int size = SvInvokeInterface(QBListModel, dataSource, getLength);
        if (size != 0) {
            QBVoDMovieDetailsDSItemsChanged((SvObject) self->movieDetails, dataSource, 0, size);
        }
    }
}

SvLocal void
QBVoDMovieDetailsDSContextDisconnectDataSources(QBVoDMovieDetailsDSContext self)
{
    if (!self->movieDetails || !self->dataProviders) {
        return;
    }
    SvIterator iter = SvHashTableKeysIterator(self->dataProviders);
    SvObject dataSource = NULL;
    while ((dataSource = SvIteratorGetNext(&iter)) != NULL) {
        SvInvokeInterface(QBDataModel, dataSource, removeListener, (SvObject) self->movieDetails, NULL);
    }
}

SvLocal void QBVoDMovieDetailsFetchActorDetails(QBVoDMovieDetailsDSContext self)
{
    int len = SvInvokeInterface(QBListModel, self->movieDetails->castDataSource, getLength);
    len = (self->castMaxCount != 0 && len > self->castMaxCount) ? self->castMaxCount : len;
    for (int i = 0; i < len; ++i) {
        SvDBRawObject actor = (SvDBRawObject) SvInvokeInterface(QBListModel, self->movieDetails->castDataSource, getObject, i);
        if (!actor || !SvObjectIsInstanceOf((SvObject) actor, SvDBRawObject_getType()) || QBVoDDetailsGetBoolAttribute(actor, "detailsFetched")) {
            continue;
        }
        SvObject contentManager = QBContentProviderGetManager(self->castProvider);
        if (contentManager) {
            SvArray actions = SvInvokeInterface(QBContentManager, contentManager, constructActionsForContent, (SvObject) actor, NULL);
            if (actions) {
                SvIterator it = SvArrayIterator(actions);
                QBContentAction detailsAction = NULL;
                QBContentAction act = NULL;
                while ((act = (QBContentAction) SvIteratorGetNext(&it))) {
                    if (QBContentActionGetType(act, NULL) == QBContentActionType_relatedContent) {
                        if (!detailsAction && SvStringEqualToCString(QBContentActionGetName(act, NULL), "personDetails")) {
                            detailsAction = act;
                        }
                    }
                }
                if (detailsAction) {
                    QBContentProvider provider = (QBContentProvider) QBVoDMovieDetailsDSContextCreateActionProvider(self, contentManager, (SvDBObject) actor, detailsAction);
                    QBContentProviderRefresh(provider, NULL, 0, true, false);
                    QBContentProviderStart(provider, self->appGlobals->scheduler);
                    SVRELEASE(provider);
                }
                SVRELEASE(actions);
            }
        }
    }
}

SvLocal void QBVoDMovieDetailsDSShowCastMenu(QBVoDMovieDetailsDS self)
{
    QBVoDMovieDetailsDSMenu castMenu = self->basicElements.castMenu->prv;

    castMenu->isVisible = true;
    svWidgetAttach(self->ctx->super_.window, castMenu->w, castMenu->xOffset, castMenu->yOffset, 3);

    QBVoDMovieDetailsFetchActorDetails(self->ctx);
}

SvLocal void QBVoDMovieDetailsDSHideCastMenu(QBVoDMovieDetailsDS self)
{
    QBVoDMovieDetailsDSMenu castMenu = self->basicElements.castMenu->prv;

    castMenu->isVisible = false;
    svWidgetDetach(castMenu->w);
}

SvLocal bool QBVoDMovieDetailsDSShouldBeBlocked(QBVoDMovieDetailsDSContext self)
{
    if (QBAccessManagerGetAuthenticationStatus(self->appGlobals->accessMgr, SVSTRING("PC_VOD"), NULL) == QBAuthStatus_OK)
        return false;

    if (!QBParentalControlLogicAdultIsBlocked(self->appGlobals->parentalControlLogic))
        return false;

    return QBVoDUtilsIsAdult((SvObject) self->object);
}

/**
 * Make sure that cast menu is shown if there are any cast members to display.
 **/
SvLocal void QBVoDMovieDetailsDSEnsureCastMenu(QBVoDMovieDetailsDS self)
{
    if (!self->basicElements.castMenu)
        return;
    QBVoDMovieDetailsDSMenu castMenu = self->basicElements.castMenu->prv;
    if (!castMenu)
        return;

    int len = SvInvokeInterface(QBListModel, self->castDataSource, getLength);

    if (castMenu->isVisible && len >= 1) {
        log_debug("showing cast members");
        QBWidgetsStripRefreshItems(castMenu->menuWidget);
        if (QBVoDMovieDetailsDSShouldBeBlocked(self->ctx)) {
            log_debug("hiding cast members due to Parental Control settings");
            QBVoDMovieDetailsDSHideCastMenu(self);
        }
        return;
    }

    if (QBVoDMovieDetailsDSShouldBeBlocked(self->ctx)) {
        log_debug("hiding cast members due to Parental Control settings");
        QBVoDMovieDetailsDSHideCastMenu(self);
    } else if (len >= 1) {
        QBVoDMovieDetailsDSShowCastMenu(self);
    } else {
        QBVoDMovieDetailsDSHideCastMenu(self);
    }
}

SvLocal void QBVoDMovieDetailsDSFillCastMenu(QBVoDMovieDetailsDS self, const char *widgetName)
{
    QBVoDMovieDetailsDSMenu castMenu = self->basicElements.castMenu->prv;
    SvWidget menu = castMenu->menuWidget;

    SvObject controller = (SvObject)
                          QBVoDMovieDetailsDSHorizontalMenuControllerCreate(self->ctx->appGlobals, widgetName);

    QBWidgetsStripConnectToDataSource(menu, self->castDataSource, controller, NULL);
    SVRELEASE(controller);
}

SvLocal void QBVoDDetailsSetUpCastMenu(QBVoDMovieDetailsDS self)
{
    self->castDataSource = (SvObject) QBListProxyCreate((SvObject) self->ctx->castTree, NULL /*root*/, NULL);
    QBVoDMovieDetailsDSFillCastMenu(self, svWidgetGetName(self->basicElements.castMenu));
}

SvLocal void
QBVoDMovieDetailsDSSetCastMenu(QBVoDMovieDetailsDS self)
{
    if (!self->basicElements.castMenu) {
        self->basicElements.castMenu = QBVoDMovieDetailsDSHorizontalMenuCreate(self, self->ctx->appGlobals->res,
                                                                               SVSTRING("QBVoDMovieDetails.CastMenu"), self->ctx->appGlobals->textRenderer, SVSTRING("Cast"));
    }
}

SvLocal void QBVoDMovieDetailsDSContextRefreshRecommended(QBVoDMovieDetailsDSContext self)
{
    if (self->movieDetails && self->movieDetails->recommendedDataSource && self->movieDetails->basicElements.recommendedCarousel) {
        QBVoDMovieDetailsDSCarousel recommendedCarousel = self->movieDetails->basicElements.recommendedCarousel->prv;
        size_t len = SvInvokeInterface(QBListModel, self->movieDetails->recommendedDataSource, getLength);
        SvInvokeInterface(QBListModelListener, recommendedCarousel->carouselWidget->prv, itemsChanged, 0, len);
    }
}

SvLocal bool QBVoDMovieDetailsDSContextObjectIsBlocked(QBVoDMovieDetailsDSContext self, SvObject object)
{
    if (QBAccessManagerGetAuthenticationStatus(self->appGlobals->accessMgr, SVSTRING("PC_VOD"), NULL) == QBAuthStatus_OK)
        return false;
    return QBParentalControlLogicAdultIsBlocked(self->appGlobals->parentalControlLogic) && QBVoDUtilsIsAdult(object);
}

SvLocal void QBVoDMovieDetailsDSFillCover(SvWidget w, SvString attrType_, SvString attrName_, SvString attrNameVal_)
{
    if (w && w->prv) {
        QBVoDMovieDetailsDSCover cover = (QBVoDMovieDetailsDSCover) w->prv;
        if (cover->movieDetails) {
            QBVoDMovieDetailsDS details = cover->movieDetails;
            if (!QBVoDMovieDetailsDSShouldBeBlocked(details->ctx)) {
                log_debug("setting cover");
                QBMovieUtilsSetThumbnailIntoIcon(cover->cover, (SvObject) cover->movieDetails->ctx->object, SvStringCString(attrType_), SvStringCString(attrName_), SvStringCString(attrNameVal_), 1);
            } else {
                log_debug("blocking cover due to Parental Control settings");
                svIconSetBitmap(cover->cover, 1, svSettingsGetBitmap("QBVoDMovieDetails.Cover.Image", "bgLocked"));
                svIconSwitch(cover->cover, 1, 0, 0.0);
            }
        }
    }
}

SvLocal void QBVoDMovieDetailsDSFillMovieInfo(QBVoDMovieDetailsDS self)
{
    if (self->basicElements.movieInfo && !QBVoDMovieDetailsDSShouldBeBlocked(self->ctx)) {
        svWidgetSetHidden(self->basicElements.movieInfo, false);
        QBMovieInfoSetObjectLoadingState(self->basicElements.movieInfo, !self->ctx->doneLoadingOffers);
        log_debug("refreshing movie info");
        QBMovieInfoSetObject(self->basicElements.movieInfo, (SvObject) self->ctx->object);
    } else if (self->basicElements.movieInfo) {
        log_debug("hiding movie info due to Parental Controll settings");
        svWidgetSetHidden(self->basicElements.movieInfo, true);
    }
}

SvLocal void QBVoDMovieDetailsDSFillDescription(QBVoDMovieDetailsDS self)
{
    if (self->basicElements.description) {
        if (!QBVoDMovieDetailsDSShouldBeBlocked(self->ctx)) {
            SvString description = SVTESTRETAIN(QBMovieUtilsGetAttr(self->ctx->object, "description", NULL));
            if (!description) {
                if (self->ctx->objectType == QBVoDMovieDetailsDSObjectType_person) {
                    SvStringBuffer descriptionBuffer = SvStringBufferCreate(NULL);
                    SvString birthDate = QBMovieUtilsGetAttr(self->ctx->object, "birthDate", NULL);
                    if (birthDate) {
                        //xgettext:c-format
                        SvStringBufferAppendFormatted(descriptionBuffer, NULL, gettext("Birth date: %s\n"), SvStringCString(birthDate));
                    }
                    SvString birthPlace = QBMovieUtilsGetAttr(self->ctx->object, "birthPlace", NULL);
                    if (birthPlace) {
                        //xgettext:c-format
                        SvStringBufferAppendFormatted(descriptionBuffer, NULL, gettext("Birth place: %s\n"), SvStringCString(birthPlace));
                    }
                    description = SvStringBufferCreateContentsString(descriptionBuffer, NULL);
                    SVRELEASE(descriptionBuffer);
                }
            }
            if (description) {
                log_debug("setting description");
                QBAsyncLabelSetText(self->basicElements.description, description);
                SVRELEASE(description);
            }
        } else {
            log_debug("blocking description due to Parental Controll settings");
            QBAsyncLabelSetText(self->basicElements.description, QBParentalControlGetBlockedMovieDescription(self->ctx->appGlobals->pc));
        }
    }
}

SvLocal void QBVoDMovieDetailsDSFillTitle(QBVoDMovieDetailsDS self)
{
    if (self->basicElements.title) {
        if (!QBVoDMovieDetailsDSShouldBeBlocked(self->ctx)) {
            SvString episodeTitle = QBMovieUtilsGetAttr(self->ctx->object, "episodeTitle", NULL);
            int episodeNum = -1;
            SvValue episodeVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) self->ctx->object, "episodeNumber");
            if (episodeVal && SvObjectIsInstanceOf((SvObject) episodeVal, SvValue_getType()) && SvValueIsInteger(episodeVal)) {
                episodeNum = SvValueGetInteger(episodeVal);
            }
            int releaseYear = -1;
            SvValue releaseYearVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) self->ctx->object, "year");
            if (releaseYearVal && SvObjectIsInstanceOf((SvObject) releaseYearVal, SvValue_getType()) && SvValueIsInteger(releaseYearVal)) {
                releaseYear = SvValueGetInteger(releaseYearVal);
            }
            SvString title = QBMovieUtilsGetAttr(self->ctx->object, "title", NULL);
            if (!title) {
                title = QBMovieUtilsGetAttr(self->ctx->object, "name", NULL);
            }
            if (title) {
                SvString fullTitle = NULL;
                if (episodeNum >= 0 || episodeTitle) {
                    SvStringBuffer titleBuffer = SvStringBufferCreate(NULL);
                    SvStringBufferAppendString(titleBuffer, title, NULL);
                    if (episodeNum) {
                        //xgettext:c-format
                        SvStringBufferAppendFormatted(titleBuffer, NULL, gettext(" ep. %d"), episodeNum);
                    }
                    if (episodeTitle) {
                        SvStringBufferAppendFormatted(titleBuffer, NULL, " - %s", SvStringCString(episodeTitle));
                    }
                    fullTitle = SvStringBufferCreateContentsString(titleBuffer, NULL);
                    SVRELEASE(titleBuffer);
                } else if (releaseYear >= 0) {
                    SvStringBuffer titleBuffer = SvStringBufferCreate(NULL);
                    SvStringBufferAppendFormatted(titleBuffer, NULL, "%s (%d)", SvStringCString(title), releaseYear);
                    fullTitle = SvStringBufferCreateContentsString(titleBuffer, NULL);
                    SVRELEASE(titleBuffer);
                } else {
                    fullTitle = SVRETAIN(title);
                }
                log_debug("setting title");
                QBAsyncLabelSetText(self->basicElements.title, fullTitle);
                SVRELEASE(fullTitle);
            }
        } else {
            log_debug("blocking title due to Parental Controll settings");
            QBAsyncLabelSetText(self->basicElements.title, QBParentalControlGetBlockedMovieTitle(self->ctx->appGlobals->pc));
        }
    }
}

SvLocal void QBVoDMovieDetailsDSCarouselSetDefaultPosition(QBVoDMovieDetailsDS self)
{
    QBVoDMovieDetailsDSCarousel recommendedCarousel = self->basicElements.recommendedCarousel->prv;
    int len = SvInvokeInterface(QBListModel, self->recommendedDataSource, getLength);

    QBVerticalCarouselSetActive(recommendedCarousel->carouselWidget, (len > self->basicElements.recommendedCarouseInitialFocusColumn) ? self->basicElements.recommendedCarouseInitialFocusColumn : (len - 1));
    if (self->ctx->objectType == QBVoDMovieDetailsDSObjectType_person) {
        recommendedCarousel = self->basicElements.recommendedCarousel->prv;
        if (recommendedCarousel->isVisible) {
            svWidgetSetFocus(recommendedCarousel->carouselWidget);
        }
    }
}

SvLocal void QBVoDMovieDetailsDSShowRecommendedCarousel(QBVoDMovieDetailsDS self)
{
    QBVoDMovieDetailsDSCarousel recommendedCarousel = self->basicElements.recommendedCarousel->prv;

    recommendedCarousel->isVisible = true;
    svWidgetAttach(self->ctx->movieDetails->basicElements.slider, recommendedCarousel->w, recommendedCarousel->xOffset, recommendedCarousel->yOffset, 1);
}

SvLocal void QBVoDMovieDetailsDSHideRecommendedCarousel(QBVoDMovieDetailsDS self)
{
    QBVoDMovieDetailsDSCarousel recommendedCarousel = self->basicElements.recommendedCarousel->prv;

    recommendedCarousel->isVisible = false;
    svWidgetDetach(recommendedCarousel->w);
}

/**
 * Make sure that recommended carousel is shown if there are any cast recommended movies to display.
 **/
SvLocal void QBVoDMovieDetailsDSEnsureRecommendedCarousel(QBVoDMovieDetailsDS self)
{
    if (!self->basicElements.recommendedCarousel)
        return;
    QBVoDMovieDetailsDSCarousel recommendedCarousel = self->basicElements.recommendedCarousel->prv;
    if (!recommendedCarousel)
        return;
    int len = SvInvokeInterface(QBListModel, self->recommendedDataSource, getLength);

    if (recommendedCarousel->isVisible && len >= 1)
        return;

    if (len >= 1) {
        log_debug("showing recommended carousel");
        QBVoDMovieDetailsDSShowRecommendedCarousel(self);
        QBVoDMovieDetailsDSCarouselSetDefaultPosition(self);
    } else {
        QBVoDMovieDetailsDSHideRecommendedCarousel(self);
    }
}

SvLocal void QBVoDMovieDetailsDSShowButtonsMenu(QBVoDMovieDetailsDS self)
{
    if (self->ctx->objectType == QBVoDMovieDetailsDSObjectType_person) {
        return;
    }

    QBVoDMovieDetailsDSMenu buttonsMenu = self->basicElements.buttonsMenu->prv;
    int len = SvInvokeInterface(QBListModel, self->buttonsDataSource, getLength);
    bool isVisible = (len != 0) && !QBVoDMovieDetailsDSShouldBeBlocked(self->ctx);
    if (isVisible && !buttonsMenu->isVisible) {
        log_debug("showing buttons menu");
        svWidgetAttach(self->ctx->super_.window, buttonsMenu->w, buttonsMenu->xOffset, buttonsMenu->yOffset, 3);
    } else if (!isVisible && buttonsMenu->isVisible) {
        log_debug("hiding buttons menu");
        svWidgetDetach(buttonsMenu->w);
    }
    buttonsMenu->isVisible = isVisible;
}

SvLocal void QBVoDMovieDetailsDSFillButtonsMenu(QBVoDMovieDetailsDS self, const char *widgetName)
{
    QBVoDMovieDetailsDSMenu buttonsMenu = self->basicElements.buttonsMenu->prv;
    SvWidget menu = buttonsMenu->menuWidget;

    SvObject controller = (SvObject)
                          QBVoDMovieDetailsDSHorizontalMenuControllerCreate(self->ctx->appGlobals, widgetName);

    QBWidgetsStripConnectToDataSource(menu, self->buttonsDataSource, controller, NULL);
    SVRELEASE(controller);
}

SvLocal void QBVoDDetailsSetUpButtonsMenu(QBVoDMovieDetailsDS self)
{
    bool onMyList = QBVoDDetailsGetBoolAttribute(self->ctx->object, "inUserList(myList)");

    QBActiveArray array = (QBActiveArray) self->buttonsDataSource;
    if (array) {
        QBActiveArrayRemoveAllObjects(array, NULL);
    } else {
        array = QBActiveArrayCreate(2, NULL);
        self->buttonsDataSource = (SvObject) array;
    }
    SvString obj = NULL;
    if (self->ctx->objectType != QBVoDMovieDetailsDSObjectType_episode) {
        if (onMyList) {
            obj = SvStringCreate("MyListButtonLit", NULL);
        } else {
            obj = SvStringCreate("MyListButton", NULL);
        }
        QBActiveArrayAddObject(array, (SvObject) obj, NULL);
        SVRELEASE(obj);
    }

    SvObject trailer = SvDBRawObjectGetAttrValue(self->ctx->object, "trailer");
    if (trailer) {
        obj = SvStringCreate("TrailerButton", NULL);
        QBActiveArrayAddObject(array, (SvObject) obj, NULL);
        SVRELEASE(obj);
    }
    QBVoDMovieDetailsDSShowButtonsMenu(self);
}

SvLocal void
QBVoDMovieDetailsDSSetButtonsMenu(QBVoDMovieDetailsDS self)
{
    if (!self->basicElements.buttonsMenu) {
        self->basicElements.buttonsMenu = QBVoDMovieDetailsDSHorizontalMenuCreate(self, self->ctx->appGlobals->res,
                                                                                  SVSTRING("QBVoDMovieDetails.ButtonsMenu"), self->ctx->appGlobals->textRenderer, SVSTRING(""));
    }
}

SvLocal void QBVoDMovieDetailsDSShowLikeMenu(QBVoDMovieDetailsDS self)
{
    if (self->ctx->objectType == QBVoDMovieDetailsDSObjectType_person) {
        return;
    }

    QBVoDMovieDetailsDSMenu likeMenu = self->basicElements.likeMenu->prv;

    int len = SvInvokeInterface(QBListModel, self->likeDataSource, getLength);
    bool isVisible = (len != 0) && !QBVoDMovieDetailsDSShouldBeBlocked(self->ctx);
    if (isVisible && !likeMenu->isVisible) {
        log_debug("showing like/dislike menu");
        svWidgetAttach(self->ctx->super_.window, likeMenu->w, likeMenu->xOffset, likeMenu->yOffset, 3);
    } else if (!isVisible && likeMenu->isVisible) {
        log_debug("hiding like/dislike menu");
        svWidgetDetach(likeMenu->w);
    }
    likeMenu->isVisible = isVisible;
}

SvLocal void QBVoDMovieDetailsDSFillLikeMenu(QBVoDMovieDetailsDS self, const char *widgetName)
{
    QBVoDMovieDetailsDSMenu likeMenu = self->basicElements.likeMenu->prv;
    SvWidget menu = likeMenu->menuWidget;

    SvObject controller = (SvObject)
                          QBVoDMovieDetailsDSHorizontalMenuControllerCreate(self->ctx->appGlobals, widgetName);

    QBWidgetsStripConnectToDataSource(menu, self->likeDataSource, controller, NULL);
    SVRELEASE(controller);
}

SvLocal void QBVoDDetailsSetUpLikeMenu(QBVoDMovieDetailsDS self)
{
    bool liked = QBVoDDetailsGetBoolAttribute(self->ctx->object, "likedByUser()");
    bool disliked = QBVoDDetailsGetBoolAttribute(self->ctx->object, "dislikedByUser()");

    QBActiveArray array = (QBActiveArray) self->likeDataSource;
    if (array) {
        QBActiveArrayRemoveAllObjects(array, NULL);
    } else {
        array = QBActiveArrayCreate(2, NULL);
        self->likeDataSource = (SvObject) array;
    }
    SvString obj = NULL;
    if (liked) {
        obj = SvStringCreate("LikeButtonLit", NULL);
    } else {
        obj = SvStringCreate("LikeButton", NULL);
    }
    QBActiveArrayAddObject(array, (SvObject) obj, NULL);
    SVRELEASE(obj);

    if (disliked) {
        obj = SvStringCreate("DislikeButtonLit", NULL);
    } else {
        obj = SvStringCreate("DislikeButton", NULL);
    }
    QBActiveArrayAddObject(array, (SvObject) obj, NULL);
    SVRELEASE(obj);

    QBVoDMovieDetailsDSShowLikeMenu(self);
}

SvLocal void
QBVoDMovieDetailsDSSetLikeMenu(QBVoDMovieDetailsDS self)
{
    if (!self->basicElements.likeMenu) {
        self->basicElements.likeMenu = QBVoDMovieDetailsDSHorizontalMenuCreate(self, self->ctx->appGlobals->res,
                                                                               SVSTRING("QBVoDMovieDetails.LikeButtonsMenu"), self->ctx->appGlobals->textRenderer, SVSTRING(""));
    }
}

SvLocal void QBVoDMovieDetailsDSSideDetailsShow(QBVoDMovieDetailsDS self)
{
    bool isBlocked = QBVoDMovieDetailsDSContextObjectIsBlocked(self->ctx, (SvObject) self->activeChildElement);
    if (self->activeChildElement && !isBlocked) {
        QBVoDMovieDetailsDSFillSideDetails(self);
        svWidgetSetHidden(self->details, false);
    } else {
        svWidgetSetHidden(self->details, true);
    }
}

SvLocal void QBVoDMovieDetailsDSSideDetailsHide(QBVoDMovieDetailsDS self)
{
    SVTESTRELEASE(self->activeChildElement);
    self->activeChildElement = NULL;
    svWidgetSetHidden(self->details, true);
}

SvLocal void QBVoDMovieDetailsDSEpisodesTitleFocusEventHandler(SvWidget w, SvFocusEvent e)
{
    SvWidget parent = svWidgetGetParent(w);
    if (!parent) {
        return;
    }
    QBVoDMovieDetailsDSCarousel self = parent->prv;
    if (e->kind == SvFocusEventKind_GET) {
        svWidgetAttach(self->w, self->titleFocus, self->titleFocusXOffset, self->titleFocusYOffset, 8);
    } else {
        svWidgetDetach(self->titleFocus);
    }
}

SvLocal SvWidget
QBVoDMovieDetailsDSCarouselSlotCreate(QBVoDMovieDetailsDS movieDetails, SvApplication app, const char *widgetName);
SvLocal void
QBVoDMovieDetailsDSCarouselSlotSetObject(SvWidget w, SvObject object);

SvLocal void QBVoDMovieDetailsDSShowEpisodesCarousel(QBVoDMovieDetailsDS self)
{
    QBVoDMovieDetailsDSCarousel episodesCarousel = self->basicElements.episodesCarousel->prv;

    episodesCarousel->isVisible = true;
    svWidgetAttach(self->ctx->movieDetails->basicElements.slider, episodesCarousel->w, episodesCarousel->xOffset, episodesCarousel->yOffset, 1);
}

SvLocal void QBVoDMovieDetailsDSFillEpisodesCarousel(QBVoDMovieDetailsDS self, const char *widgetName)
{
    svSettingsRestoreContext(self->settingsCtx);
    QBVoDMovieDetailsDSCarousel episodesCarousel = self->basicElements.episodesCarousel->prv;
    SvWidget carousel = episodesCarousel->carouselWidget;

    int count = QBVerticalCarouselGetSlotCount(carousel);
    char *widgetCarouselSlotBuffer;
    asprintf(&widgetCarouselSlotBuffer, "%s.Carousel.Slot", widgetName);

    for (int i = 0; i < count; ++i) {
        QBVerticalCarouselSetContentAtIndex(carousel, i, QBVoDMovieDetailsDSCarouselSlotCreate(self, self->ctx->appGlobals->res, widgetCarouselSlotBuffer));
        QBVerticalCarouselSetDataHandler(carousel, i, QBVoDMovieDetailsDSCarouselSlotSetObject);
    }

    free(widgetCarouselSlotBuffer);
    episodesCarousel->isVisible = false;

    QBVerticalCarouselSetFocusWidget(carousel, episodesCarousel->focus);
    QBVerticalCarouselConnectToSource(carousel, self->episodesDataSource, NULL);
    svSettingsPopComponent();
}

SvLocal void QBVoDMovieDetailsDSSliderSlideDown(QBVoDMovieDetailsDS self, bool immediate)
{
    if (immediate) {
        self->basicElements.slider->off_y = self->basicElements.sliderSlidedOffset;
        svWidgetForceRender(self->basicElements.slider);
        return;
    } else {
        SvEffect move = svEffectMoveNew(self->basicElements.slider,
                                        self->basicElements.slider->off_x,
                                        self->basicElements.sliderSlidedOffset,
                                        0.5,
                                        SV_EFFECT_MOVE_LOGARITHM,
                                        false);
        svEffectSetNotificationTarget(move, self->basicElements.slider);
        svAppRegisterEffect(self->basicElements.slider->app, move);
    }
}

SvLocal void QBVoDMovieDetailsDSSliderSlideUp(QBVoDMovieDetailsDS self, bool immediate)
{
    if (immediate) {
        self->basicElements.slider->off_y = self->basicElements.sliderOriginalOffset;
        svWidgetForceRender(self->basicElements.slider);
        return;
    } else {
        SvEffect move = svEffectMoveNew(self->basicElements.slider,
                                        self->basicElements.slider->off_x,
                                        self->basicElements.sliderOriginalOffset,
                                        0.5,
                                        SV_EFFECT_MOVE_LOGARITHM,
                                        false);
        svEffectSetNotificationTarget(move, self->basicElements.slider);
        svAppRegisterEffect(self->basicElements.slider->app, move);
    }
}

/**
 * Make sure that episodes carousel is shown if there are any episodes to display.
 **/
SvLocal void QBVoDMovieDetailsDSEnsureEpisodesCarousel(QBVoDMovieDetailsDS self)
{
    if (!self->basicElements.episodesCarousel) {
        return;
    }
    if (!self->episodesDataSource) {
        return;
    }
    QBVoDMovieDetailsDSCarousel episodesCarousel = self->basicElements.episodesCarousel->prv;
    if (!episodesCarousel)
        return;

    if (episodesCarousel->isVisible)
        return;

    log_debug("showing episodes carousel");
    QBVoDMovieDetailsDSShowEpisodesCarousel(self);
}

SvLocal void
QBVoDMovieDetailsDSSetEpisodesCarousel(QBVoDMovieDetailsDS self)
{
    SvString contentType = QBMovieUtilsGetAttr(self->ctx->object, "collectionType", NULL);
    if (!contentType || !SvStringEqualToCString(contentType, "series")) {
        QBVoDMovieDetailsDSSliderSlideDown(self, true);
        return;
    }

    if (!self->basicElements.episodesCarousel) {
        self->basicElements.episodesCarousel = QBVoDMovieDetailsDSCarouselCreate(self,
                                                                                 self->ctx->appGlobals->res,
                                                                                 SVSTRING("QBVoDMovieDetails.Slider.EpisodesCarousel"),
                                                                                 self->ctx->appGlobals->textRenderer,
                                                                                 SVSTRING(""),
                                                                                 self->ctx->episodesProvider,
                                                                                 QBVoDMovieDetailsDSEpisodesCarouselUserEventHandler);
    }
}

SvLocal bool QBVoDMovieDetailsDSEpisodesTitleInputEventHandler(SvWidget w, SvInputEvent e)
{
    bool ret = false;
    SvWidget parent = svWidgetGetParent(w);
    if (!parent) {
        return false;
    }
    QBVoDMovieDetailsDSCarousel carousel = parent->prv;
    QBVoDMovieDetailsDS self = carousel->movieDetails;
    if (e->ch == QBKEY_RIGHT && self->ctx->seasonsCount > 0) {
        size_t episodesCount = SvInvokeInterface(QBListModel, self->episodesDataSource, getLength);
        for (size_t i = 1; i < episodesCount; i++) {
            size_t episodeIndex = (carousel->activeElementIndex + i) % episodesCount;
            SvDBRawObject episode = (SvDBRawObject) SvInvokeInterface(QBListModel, self->episodesDataSource, getObject, episodeIndex);
            if (episode && SvObjectIsInstanceOf((SvObject) episode, SvDBRawObject_getType())) {
                SvValue seasonVal = (SvValue) SvDBRawObjectGetAttrValue(episode, "seasonNumber");
                if (seasonVal && SvObjectIsInstanceOf((SvObject) seasonVal, SvValue_getType()) && SvValueIsInteger(seasonVal)) {
                    size_t seasonNumber = (size_t) SvValueGetInteger(seasonVal);
                    if (seasonNumber != self->ctx->selectedSeasonIndex + 1) {
                        QBVerticalCarouselSetActive(carousel->carouselWidget, episodeIndex);
                        QBVoDMovieDetailsDSEpisodesCarouselUpdateSeasonNumber(carousel, episode);
                        break;
                    }
                }
            }
        }
        ret = true;
    } else if (e->ch == QBKEY_LEFT && self->ctx->seasonsCount > 0) {
        size_t episodesCount = SvInvokeInterface(QBListModel, self->episodesDataSource, getLength);
        int prevSeason = -1;
        size_t firstEpisodeIndex = 0;
        SvDBRawObject firstEpisode = NULL;
        for (size_t i = 1; i <= episodesCount; i++) {
            size_t episodeIndex = (episodesCount + carousel->activeElementIndex - i) % episodesCount;
            SvDBRawObject episode = (SvDBRawObject) SvInvokeInterface(QBListModel, self->episodesDataSource, getObject, episodeIndex);
            if (episode && SvObjectIsInstanceOf((SvObject) episode, SvDBRawObject_getType())) {
                SvValue seasonVal = (SvValue) SvDBRawObjectGetAttrValue(episode, "seasonNumber");
                if (seasonVal && SvObjectIsInstanceOf((SvObject) seasonVal, SvValue_getType()) && SvValueIsInteger(seasonVal)) {
                    size_t seasonNumber = (size_t) SvValueGetInteger(seasonVal);
                    if (prevSeason < 0 && seasonNumber != self->ctx->selectedSeasonIndex + 1) {
                        prevSeason = seasonNumber;
                        firstEpisodeIndex = episodeIndex;
                        firstEpisode = episode;
                    } else if (prevSeason >= 0 && seasonNumber == (size_t) prevSeason) {
                        firstEpisodeIndex = episodeIndex;
                        firstEpisode = episode;
                    } else if (prevSeason >= 0) {
                        QBVerticalCarouselSetActive(carousel->carouselWidget, firstEpisodeIndex);
                        QBVoDMovieDetailsDSEpisodesCarouselUpdateSeasonNumber(carousel, firstEpisode);
                        break;
                    }
                }
            }
        }
        ret = true;
    }

    return ret;
}

SvLocal SvWidget
QBVoDMovieDetailsDSCarouselCreate(QBVoDMovieDetailsDS movieDetails,
                                  SvApplication app,
                                  SvString widgetName_,
                                  QBTextRenderer renderer,
                                  SvString title,
                                  QBContentProvider provider,
                                  SvUserEventHandler userEventHandler)
{
    QBVoDMovieDetailsDSCarousel self = calloc(1, sizeof(*self));
    self->carouselProvider = provider;

    self->movieDetails = movieDetails;

    const char* widgetName = SvStringCString(widgetName_);
    log_debug("creating carousel widget: %s", widgetName);
    self->w = svSettingsWidgetCreate(app, widgetName);
    self->w->prv = self;
    svWidgetSetUserEventHandler(self->w, userEventHandler);
    self->xOffset = svSettingsGetInteger(widgetName, "xOffset", 0);
    self->yOffset = svSettingsGetInteger(widgetName, "yOffset", 0);

    char *buf;
    asprintf(&buf, "%s.Carousel", widgetName);
    self->carouselWidget = QBVerticalCarouselNewFromSM(app, buf);
    QBVerticalCarouselSetNotificationTarget(self->carouselWidget, svWidgetGetId(self->w));
    svSettingsWidgetAttach(self->w, self->carouselWidget, buf, 1);
    free(buf);
    asprintf(&buf, "%s.Carousel.Focus", widgetName);
    self->focus = QBFrameCreateFromSM(app, buf);
    free(buf);
    asprintf(&buf, "%s.Title", widgetName);
    self->title = QBAsyncLabelNew(app, buf, renderer);
    QBAsyncLabelSetText(self->title, title);
    svSettingsWidgetAttach(self->w, self->title, buf, 10);
    free(buf);

    asprintf(&buf, "%s.TitleFocus", widgetName);
    if (svSettingsIsWidgetDefined(buf)) {
        self->titleFocus = svSettingsWidgetCreate(app, buf);
        svWidgetSetFocusable(self->title, true);
        svWidgetSetFocusEventHandler(self->title, QBVoDMovieDetailsDSEpisodesTitleFocusEventHandler);
        svWidgetSetInputEventHandler(self->title, QBVoDMovieDetailsDSEpisodesTitleInputEventHandler);
        self->titleFocusXOffset = svSettingsGetInteger(buf, "xOffset", 0);
        self->titleFocusYOffset = svSettingsGetInteger(buf, "yOffset", 0);
        svWidgetSetFocus(self->title);
    }
    free(buf);

    return self->w;
}

SvLocal void QBVoDMovieDetailsDSCarouselSlotClean(SvApplication app, void *self_)
{
    QBVoDMovieDetailsDSCarouselSlot self = self_;
    free(self->widgetName);
    SVTESTRELEASE(self->obj);
    free(self);
}

SvLocal void
QBVoDMovieDetailsDSCarouselSlotDestroyIcon(SvWidget w)
{
    QBVoDMovieDetailsDSCarouselSlot self = w->prv;
    if (self->icon) {
        svWidgetDestroy(self->icon);
        self->icon = NULL;
    }
}

SvLocal void
QBVoDMovieDetailsDSCarouselSlotSetObject(SvWidget w, SvObject object)
{
    QBVoDMovieDetailsDSCarouselSlot self = w->prv;

    QBVoDMovieDetailsDSCarouselSlotDestroyIcon(w);

    SVTESTRETAIN(object);
    SVTESTRELEASE(self->obj);
    self->obj = object;

    bool isBlocked = QBVoDMovieDetailsDSContextObjectIsBlocked(self->movieDetails->ctx, object);

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
        if (isBlocked) {
            SvString blockedString = SvStringCreate(gettext("Title blocked"), NULL);
            QBAsyncLabelSetText(self->label, blockedString);
            svWidgetSetHidden(self->labelBackground, false);
            SVRELEASE(blockedString);
        } else if (SvObjectIsInstanceOf(self->obj, SvDBRawObject_getType())) {
            int episodeNum = -1;
            SvValue episodeVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) self->obj, "episodeNumber");
            if (episodeVal && SvObjectIsInstanceOf((SvObject) episodeVal, SvValue_getType()) && SvValueIsInteger(episodeVal)) {
                episodeNum = SvValueGetInteger(episodeVal);
            }
            SvString val = QBMovieUtilsGetAttr((SvDBRawObject) self->obj, "title", NULL);
            if (!val) {
                val = QBMovieUtilsGetAttr((SvDBRawObject) self->obj, "name", NULL);
            }
            if (val && episodeNum >= 0) {
                //xgettext:c-format
                SvString episodeName = SvStringCreateWithFormat(gettext("%s ep. %d"), SvStringCString(val), episodeNum);
                QBAsyncLabelSetText(self->label, episodeName);
                svWidgetSetHidden(self->labelBackground, false);
                SVRELEASE(episodeName);
            } else if (val) {
                QBAsyncLabelSetText(self->label, val);
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

    if (isBlocked) {
        svIconSwitch(self->icon, 1, 0, 0);
    } else if (SvObjectIsInstanceOf(object, SvDBRawObject_getType())) {
        SvString objectType = QBMovieUtilsGetAttr((SvDBRawObject) object, "objectType", NULL);
        if (objectType && SvStringEqualToCString(objectType, "Episode")) {
            SvString thumbnail = QBMovieUtilsGetAttr((SvDBRawObject) object, "thumbnail", NULL);
            if (!thumbnail) {
                SvDBRawObjectSetAttrValue((SvDBRawObject) object,
                                          "thumbnail",
                                          SvDBRawObjectGetAttrValue(self->movieDetails->ctx->object, "defaultEpisodeImage"));
            }
        }
        QBMovieUtilsSetThumbnailIntoIcon(self->icon, (SvObject) object, "covers", "cover_name", "grid", 1);
    }
}

SvLocal SvWidget
QBVoDMovieDetailsDSCarouselSlotCreate(QBVoDMovieDetailsDS movieDetails, SvApplication app, const char *widgetName)
{
    QBVoDMovieDetailsDSCarouselSlot self = calloc(1, sizeof(struct QBVoDMovieDetailsDSCarouselSlot_t));
    self->box = svWidgetCreateBitmap(app, 0, 0, NULL);
    self->box->prv = self;
    self->box->clean = QBVoDMovieDetailsDSCarouselSlotClean;
    self->settingsCtx = svSettingsSaveContext();
    self->widgetName = strdup(widgetName);
    self->box->width = svSettingsGetInteger(widgetName, "width", 0);
    self->box->height = svSettingsGetInteger(widgetName, "height", 0);
    self->movieDetails = movieDetails;
    self->label = NULL;
    self->labelBackground = NULL;

    return self->box;
}

SvLocal void QBVoDMovieDetailsDSFillRecommendedCarousel(QBVoDMovieDetailsDS self, const char *widgetName)
{
    QBVoDMovieDetailsDSCarousel recommendedCarousel = self->basicElements.recommendedCarousel->prv;
    SvWidget carousel = recommendedCarousel->carouselWidget;

    int count = QBVerticalCarouselGetSlotCount(carousel);
    char *widgetCarouselSlotBuffer;
    asprintf(&widgetCarouselSlotBuffer, "%s.Carousel.Slot", widgetName);

    for (int i = 0; i < count; ++i) {
        QBVerticalCarouselSetContentAtIndex(carousel, i, QBVoDMovieDetailsDSCarouselSlotCreate(self, self->ctx->appGlobals->res, widgetCarouselSlotBuffer));
        QBVerticalCarouselSetDataHandler(carousel, i, QBVoDMovieDetailsDSCarouselSlotSetObject);
    }

    free(widgetCarouselSlotBuffer);
    recommendedCarousel->isVisible = false;

    QBVerticalCarouselSetFocusWidget(carousel, recommendedCarousel->focus);
    QBVerticalCarouselConnectToSource(carousel, self->recommendedDataSource, NULL);
}

SvLocal void QBVoDMovieDetailsDSFillSideDetails(QBVoDMovieDetailsDS self)
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

    if (!self->activeChildElement) {
        return;
    }

    val = QBMovieUtilsGetAttr(self->activeChildElement, "objectType", NULL);
    bool isPerson = (val && SvStringEqualToCString(val, "Person"));

    svSettingsRestoreContext(self->settingsCtx);

    val = QBMovieUtilsGetAttr(self->activeChildElement, "episodeTitle", NULL);
    if (!val) {
        val = QBMovieUtilsGetAttr(self->activeChildElement, "name", NULL);
    }
    if (!val) {
        val = QBMovieUtilsGetAttr(self->activeChildElement, "title", NULL);
    }
    if (val) {
        self->movieDetails.title = QBAsyncLabelNew(self->ctx->appGlobals->res, "MovieDetails.Title", self->ctx->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->details, self->movieDetails.title, "MovieDetails.Title", 1);
        QBAsyncLabelSetText(self->movieDetails.title, val);
    }

    val = QBMovieUtilsGetAttr(self->activeChildElement, "description", NULL);
    if (val) {
        if (isPerson) {
            self->movieDetails.description = QBAsyncLabelNew(self->ctx->appGlobals->res, "MovieDetails.DescriptionLong", self->ctx->appGlobals->textRenderer);
            svSettingsWidgetAttach(self->details, self->movieDetails.description, "MovieDetails.DescriptionLong", 1);
        } else {
            self->movieDetails.description = QBAsyncLabelNew(self->ctx->appGlobals->res, "MovieDetails.Description", self->ctx->appGlobals->textRenderer);
            svSettingsWidgetAttach(self->details, self->movieDetails.description, "MovieDetails.Description", 1);
        }
        QBAsyncLabelSetText(self->movieDetails.description, val);
    } else {
        movieDetailsOffset = svSettingsGetInteger("MovieDetails.Description", "yOffset", 0);
    }

    if (!isPerson) {
        self->movieDetails.info = QBMovieInfoCreate(self->ctx->appGlobals->res, "MovieDetails.Info", self->ctx->appGlobals->textRenderer);
        if (movieDetailsOffset >= 0)
            self->movieDetails.info->off_y = movieDetailsOffset;
        svSettingsWidgetAttach(self->details, self->movieDetails.info, "MovieDetails.Info", 1);
        QBMovieInfoSetObject(self->movieDetails.info, (SvObject) self->activeChildElement);
    }

    svSettingsPopComponent();
}

SvLocal bool QBVoDMovieDetailsDSInputEventHandler(SvWidget window, SvInputEvent e)
{
    QBVoDMovieDetailsDS self = window->prv;
    QBVoDMovieDetailsDSCarousel recommendedCarousel = self->basicElements.recommendedCarousel->prv;
    QBVoDMovieDetailsDSCarousel episodesCarousel = self->basicElements.episodesCarousel ? self->basicElements.episodesCarousel->prv : NULL;
    QBVoDMovieDetailsDSMenu castMenu = self->basicElements.castMenu->prv;
    QBVoDMovieDetailsDSMenu buttonsMenu = self->basicElements.buttonsMenu->prv;
    QBVoDMovieDetailsDSMenu likeMenu = self->basicElements.likeMenu->prv;
    if (e->ch == QBKEY_INFO) {
        if (svWidgetIsHidden(self->details)) {
            QBVoDMovieDetailsDSSideDetailsShow(self);
        } else {
            svWidgetSetHidden(self->details, true);
        }
        return true;
    }
    if (e->ch == QBKEY_VOD) {
        QBVODUtilsPopVodContexts(self->ctx->appGlobals->controller);
        return true;
    }
    if (self->ctx->objectType == QBVoDMovieDetailsDSObjectType_person) {
        return QBVoDLogicInputHandler(self->ctx->vodLogic, e, (SvObject) self->ctx);
    }

    if (e->ch == QBKEY_FUNCTION) {
        QBVoDMovieDetailsDSCover cover = (QBVoDMovieDetailsDSCover) self->basicElements.cover->prv;
        QBVoDDetailsShowSideMenu(cover->movieDetails);
        return true;
    } else if (e->ch == QBKEY_PLAY || e->ch == QBKEY_PLAYPAUSE) {
        QBVoDMovieDetailsDSCover cover = (QBVoDMovieDetailsDSCover) self->basicElements.cover->prv;
        QBVoDDetailsShowSideMenu(cover->movieDetails);
        QBContentSideMenu contentSideMenu = (QBContentSideMenu) cover->movieDetails->contentSideMenu;
        QBContentSideMenuSetImmediateAction(contentSideMenu, QBContentSideMenuImmediateAction_rentOrPlay);
        return true;
    } else if (e->ch == QBKEY_UP) {
        if (recommendedCarousel->isVisible && QBVerticalCarouselIsFocused(recommendedCarousel->carouselWidget)) {
            if (episodesCarousel && episodesCarousel->isVisible) {
                svWidgetSetFocus(episodesCarousel->carouselWidget);

                QBVoDMovieDetailsDSSliderSlideUp(self, false);

                return true;
            } else if (likeMenu->isVisible && QBWidgetsStripGetWidgetsCount(likeMenu->menuWidget) > 0) {
                QBWidgetsStripConnectSetFocusPolicy(likeMenu->menuWidget, QBWidgetsStripFocusPolicy_FocusRecent);
                svWidgetSetFocus(likeMenu->menuWidget);
                QBVoDMovieDetailsDSSideDetailsHide(self);
                return true;
            } else {
                svWidgetSetFocus(self->basicElements.cover);
                QBVoDMovieDetailsDSSideDetailsHide(self);
                return true;
            }
        } else if (episodesCarousel && QBVerticalCarouselIsFocused(episodesCarousel->carouselWidget)) {
            svWidgetSetFocus(episodesCarousel->title);
            QBVoDMovieDetailsDSSideDetailsHide(self);
            return true;
        } else if (episodesCarousel && svWidgetIsFocused(episodesCarousel->title)) {
            if (likeMenu->isVisible && QBWidgetsStripGetWidgetsCount(likeMenu->menuWidget) > 0) {
                QBWidgetsStripConnectSetFocusPolicy(likeMenu->menuWidget, QBWidgetsStripFocusPolicy_FocusRecent);
                svWidgetSetFocus(likeMenu->menuWidget);
                return true;
            } else {
                svWidgetSetFocus(self->basicElements.cover);
                return true;
            }
        } else if (svWidgetIsFocused(likeMenu->menuWidget)) {
            svWidgetSetFocus(self->basicElements.cover);
            return true;
        }
    } else if (e->ch == QBKEY_DOWN) {
        if (svWidgetIsFocused(self->basicElements.cover) && likeMenu->isVisible && QBWidgetsStripGetWidgetsCount(likeMenu->menuWidget) > 0) {
            QBWidgetsStripConnectSetFocusPolicy(likeMenu->menuWidget, QBWidgetsStripFocusPolicy_FocusRecent);
            svWidgetSetFocus(likeMenu->menuWidget);
            return true;
        } else if ((svWidgetIsFocused(likeMenu->menuWidget)
                    || svWidgetIsFocused(castMenu->menuWidget)
                    || svWidgetIsFocused(buttonsMenu->menuWidget))
                   || svWidgetIsFocused(self->basicElements.cover)) {
            if (episodesCarousel && episodesCarousel->isVisible) {
                svWidgetSetFocus(episodesCarousel->title);
                QBVoDMovieDetailsDSSideDetailsHide(self);
                return true;
            } else if (recommendedCarousel->isVisible) {
                svWidgetSetFocus(recommendedCarousel->carouselWidget);
                return true;
            }
        } else if (episodesCarousel && episodesCarousel->isVisible && svWidgetIsFocused(episodesCarousel->title)) {
            svWidgetSetFocus(episodesCarousel->carouselWidget);
            return true;
        } else if (recommendedCarousel->isVisible && episodesCarousel && QBVerticalCarouselIsFocused(episodesCarousel->carouselWidget)) {
            svWidgetSetFocus(recommendedCarousel->carouselWidget);

            QBVoDMovieDetailsDSSliderSlideDown(self, false);

            return true;
        }
    } else if (e->ch == QBKEY_RIGHT) {
        if (svWidgetIsFocused(self->basicElements.cover)
            || svWidgetIsFocused(likeMenu->menuWidget)) {
            if (castMenu->isVisible && QBWidgetsStripGetWidgetsCount(castMenu->menuWidget) > 0) {
                QBWidgetsStripConnectSetFocusPolicy(castMenu->menuWidget, QBWidgetsStripFocusPolicy_FocusFirst);
                svWidgetSetFocus(castMenu->menuWidget);
                return true;
            } else if (buttonsMenu->isVisible && QBWidgetsStripGetWidgetsCount(buttonsMenu->menuWidget) > 0) {
                QBWidgetsStripConnectSetFocusPolicy(buttonsMenu->menuWidget, QBWidgetsStripFocusPolicy_FocusFirst);
                svWidgetSetFocus(buttonsMenu->menuWidget);
                return true;
            } else {
                QBVoDDetailsShowSideMenu(self);
                return true;
            }
        } else if (svWidgetIsFocused(castMenu->menuWidget)) {
            if (buttonsMenu->isVisible && QBWidgetsStripGetWidgetsCount(buttonsMenu->menuWidget) > 0) {
                QBWidgetsStripConnectSetFocusPolicy(buttonsMenu->menuWidget, QBWidgetsStripFocusPolicy_FocusFirst);
                svWidgetSetFocus(buttonsMenu->menuWidget);
                QBVoDMovieDetailsDSSideDetailsHide(self);
                return true;
            } else {
                QBVoDDetailsShowSideMenu(self);
                svWidgetSetHidden(self->details, true);
                return true;
            }
        } else {
            QBVoDDetailsShowSideMenu(self);
            return true;
        }
    } else if (e->ch == QBKEY_LEFT) {
        if (svWidgetIsFocused(buttonsMenu->menuWidget)) {
            if (castMenu->isVisible && QBWidgetsStripGetWidgetsCount(castMenu->menuWidget) > 0) {
                QBWidgetsStripConnectSetFocusPolicy(castMenu->menuWidget, QBWidgetsStripFocusPolicy_FocusLast);
                svWidgetSetFocus(castMenu->menuWidget);
                return true;
            } else {
                svWidgetSetFocus(self->basicElements.cover);
                return true;
            }
        } else if (svWidgetIsFocused(castMenu->menuWidget)) {
            svWidgetSetFocus(self->basicElements.cover);
            QBVoDMovieDetailsDSSideDetailsHide(self);
            return true;
        }
    } else {
        return QBVoDLogicInputHandler(self->ctx->vodLogic, e, (SvObject) self->ctx);
    }

    return false;
}

SvLocal void QBVoDMovieDetailsDSClean(SvApplication app, void *self_)
{
    QBVoDMovieDetailsDS self = self_;
    QBInputServiceRemoveFilter((SvObject) self->ctx->exitInputFilter, NULL);
    SVRELEASE(self);
}

SvLocal void QBVoDMovieDetailsDSContextDestroy(void *self_)
{
    QBVoDMovieDetailsDSContext self = self_;
    QBVoDMovieDetailsStopDataProviders(self);
    SVTESTRELEASE(self->object);
    SVTESTRELEASE(self->serviceId);
    SVRELEASE(self->serviceName);
    SVRELEASE(self->externalProvider);
    SVTESTRELEASE(self->recommendationsTree);
    SVTESTRELEASE(self->castTree);
    SVTESTRELEASE(self->episodesTree);
    SVTESTRELEASE(self->detailsTree);
    SVTESTRELEASE(self->recommendedProvider);
    SVTESTRELEASE(self->castProvider);
    SVTESTRELEASE(self->detailsProvider);
    SVTESTRELEASE(self->episodesProvider);
    SVTESTRELEASE(self->userDataProvider);
    SVTESTRELEASE(self->backgroundImageProvider);
    SVTESTRELEASE(self->seriesDefaultEpisodeImageProvider);
    SVTESTRELEASE(self->eventReporter);
    SVRELEASE(self->vodLogic);
    SVTESTRELEASE(self->mwFetchService);
    SVRELEASE(self->exitInputFilter);
}

SvLocal void QBVoDMovieDetailsDSHandlePCChange(QBVoDMovieDetailsDS self)
{
    log_state("Parental Control state has changed - refreshing window data");
    svSettingsRestoreContext(self->settingsCtx);
    QBVoDMovieDetailsDSFillMovieInfo(self);
    QBVoDMovieDetailsDSFillDescription(self);
    QBVoDMovieDetailsDSFillTitle(self);
    QBVoDMovieDetailsDSFillCover(self->basicElements.cover, SVSTRING("covers"), SVSTRING("cover_name"), SVSTRING("movie_details"));
    QBVoDMovieDetailsDSContextRefreshRecommended(self->ctx);
    QBVoDMovieDetailsDSShowLikeMenu(self);
    QBVoDMovieDetailsDSShowButtonsMenu(self);
    QBVoDMovieDetailsDSEnsureCastMenu(self);
    QBVoDMovieDetailsDSSetBackground(self);
    svSettingsPopComponent();
}

SvLocal void QBVoDMovieDetailsDSContextReinitializeWindow(QBWindowContext self_, SvArray itemList)
{
    QBVoDMovieDetailsDSContext self = (QBVoDMovieDetailsDSContext) self_;

    if (self->movieDetails) {
        QBVoDMovieDetailsDSHandlePCChange(self->movieDetails);
    }
}

SvLocal SvObject QBVoDMovieDetailsDSCreateRecommendedDataSource(SvObject tree, SvBitmap listStartMarker)
{
    SvObject recommendedDataSource = NULL;
    if (listStartMarker) {
        QBActiveArray staticItems = QBActiveArrayCreate(1, NULL);
        QBActiveArrayAddObject(staticItems, (SvObject) listStartMarker, NULL);
        recommendedDataSource = (SvObject) QBListProxyCreateWithStaticItems(tree, NULL /*root*/, (SvObject) staticItems, NULL);
        SVRELEASE(staticItems);
    } else {
        recommendedDataSource = (SvObject) QBListProxyCreate(tree, NULL /*root*/, NULL);
    }
    return recommendedDataSource;
}

SvLocal void QBVoDDetailsSetUpRecommendedCarousel(QBVoDMovieDetailsDS self)
{
    self->recommendedDataSource = QBVoDMovieDetailsDSCreateRecommendedDataSource((SvObject) self->ctx->recommendationsTree,
                                                                                 self->basicElements.listStartMarker);
    QBVoDMovieDetailsDSFillRecommendedCarousel(self, svWidgetGetName(self->basicElements.recommendedCarousel));
}

SvLocal void
QBVoDMovieDetailsDSSetRecommendedCarousel(QBVoDMovieDetailsDS self)
{
    if (!self->basicElements.recommendedCarousel) {
        if (self->ctx->objectType == QBVoDMovieDetailsDSObjectType_person) {
            SvString name = QBMovieUtilsGetAttr(self->ctx->object, "name", NULL);
            SvString title = SvStringCreateWithFormat(gettext("Movies %s is in..."), name ? SvStringCString(name) : ""); //xgettext:c-format
            self->basicElements.recommendedCarousel = QBVoDMovieDetailsDSCarouselCreate(self,
                                                                                        self->ctx->appGlobals->res,
                                                                                        SVSTRING("QBVoDMovieDetails.Slider.RecommendedCarousel"),
                                                                                        self->ctx->appGlobals->textRenderer,
                                                                                        title,
                                                                                        self->ctx->recommendedProvider,
                                                                                        QBVoDMovieDetailsDSCarouselUserEventHandler);
            SVRELEASE(title);
        } else {
            SvString title = SvStringCreate(gettext("We think you might like..."), NULL);
            self->basicElements.recommendedCarousel = QBVoDMovieDetailsDSCarouselCreate(self,
                                                                                        self->ctx->appGlobals->res,
                                                                                        SVSTRING("QBVoDMovieDetails.Slider.RecommendedCarousel"),
                                                                                        self->ctx->appGlobals->textRenderer,
                                                                                        title,
                                                                                        self->ctx->recommendedProvider,
                                                                                        QBVoDMovieDetailsDSCarouselUserEventHandler);
            SVRELEASE(title);
        }
        self->basicElements.recommendedCarouseInitialFocusColumn = svSettingsGetInteger("QBVoDMovieDetails.Slider.RecommendedCarousel.Carousel", "initialFocusColumn", 3);
    }

    if (!self->basicElements.listStartMarker) {
        self->basicElements.listStartMarker = svSettingsGetBitmap("QBVoDMovieDetails.Slider.RecommendedCarousel.Carousel.Slot", "bgListStartMarker");
    }
}

SvLocal void QBVoDMovieDetailsDSCreateRemainingWidgets(QBVoDMovieDetailsDS self, SvApplication app)
{
    if (!self->basicElements.cover) {
        self->basicElements.cover = QBVoDMovieDetailsDSCoverCreate(self, self->ctx->appGlobals->res, "QBVoDMovieDetails.Cover");
        svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.cover, "QBVoDMovieDetails.Cover", 6);
        QBVoDMovieDetailsDSFillCover(self->basicElements.cover, SVSTRING("covers"), SVSTRING("cover_name"), SVSTRING("movie_details"));
    }

    if (!self->basicElements.title) {
        self->basicElements.title = QBAsyncLabelNew(self->ctx->appGlobals->res, "QBVoDMovieDetails.Title", self->ctx->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.title, "QBVoDMovieDetails.Title", 6);
        QBVoDMovieDetailsDSFillTitle(self);
    }

    if (!self->basicElements.movieInfo && self->basicElements.usesMovieInfo) {
        self->basicElements.movieInfo = QBMovieInfoCreate(self->ctx->appGlobals->res, "QBVoDMovieDetails.MovieInfo", self->ctx->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.movieInfo, "QBVoDMovieDetails.MovieInfo", 6);
        QBVoDMovieDetailsDSFillMovieInfo(self);
    }

    if (!self->basicElements.description) {
        self->basicElements.description = QBAsyncLabelNew(self->ctx->appGlobals->res, "QBVoDMovieDetails.Description", self->ctx->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.description, "QBVoDMovieDetails.Description", 6);
        QBVoDMovieDetailsDSFillDescription(self);
    }

    if (!self->basicElements.sliderCover) {
        self->basicElements.sliderCover = svIconNew(app, "QBVoDMovieDetails.SliderCover");
        svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.sliderCover, "QBVoDMovieDetails.SliderCover", 2);
    }

    if (!self->basicElements.slider) {
        self->basicElements.slider = svSettingsWidgetCreate(app, "QBVoDMovieDetails.Slider");
        self->basicElements.sliderOriginalOffset = svSettingsGetInteger("QBVoDMovieDetails.Slider", "yOffset", 0);
        self->basicElements.sliderSlidedOffset = svSettingsGetInteger("QBVoDMovieDetails.Slider", "slidedYOffset", 0);
        svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.slider, "QBVoDMovieDetails.Slider", 1);
    }

    QBVoDMovieDetailsDSSetRecommendedCarousel(self);
    QBVoDMovieDetailsDSSetEpisodesCarousel(self);
    QBVoDMovieDetailsDSSetCastMenu(self);
    QBVoDMovieDetailsDSSetButtonsMenu(self);
    QBVoDMovieDetailsDSSetLikeMenu(self);
}

SvLocal void QBVoDMovieDetailsDSStartEpisodesProvider(QBVoDMovieDetailsDS self)
{
    if (self->ctx->episodesProvider) {
        self->episodesDataSource = (SvObject) QBListProxyCreate((SvObject) self->ctx->episodesTree, NULL /*root*/, NULL);
        QBContentProviderStart(self->ctx->episodesProvider, self->ctx->appGlobals->scheduler);
        SvInvokeInterface(QBDataModel, self->episodesDataSource, addListener, (SvObject) self, NULL);
        QBVoDMovieDetailsDSFillEpisodesCarousel(self, svWidgetGetName(self->basicElements.episodesCarousel));
        QBVoDMovieDetailsDSEnsureEpisodesCarousel(self);
    }
}


SvLocal void QBVoDMovieDetailsDSContextCreateWindow(QBWindowContext ctx_, SvApplication app)
{
    log_state("creating movie details window");
    QBVoDMovieDetailsDSContext ctx = (QBVoDMovieDetailsDSContext) ctx_;
    QBVoDMovieDetailsDS self = (QBVoDMovieDetailsDS) SvTypeAllocateInstance(QBVoDMovieDetailsDS_getType(), NULL);
    self->ctx = ctx;
    self->ctx->movieDetails = self;

    svSettingsPushComponent("QBVoDMovieDetailsDS.settings");
    self->settingsCtx = svSettingsSaveContext();

    SvWidget window = svSettingsWidgetCreate(app, "QBVoDMovieDetails.Window");
    svWidgetSetInputEventHandler(window, QBVoDMovieDetailsDSInputEventHandler);
    window->clean = QBVoDMovieDetailsDSClean;
    window->prv = self;
    ctx->super_.window = window;

    QBInputServiceAddGlobalFilter((SvObject) self->ctx->exitInputFilter, NULL, NULL);

    if (self->ctx->objectType == QBVoDMovieDetailsDSObjectType_person) {
        self->basicElements.usesMovieInfo = false;
    } else {
        self->basicElements.usesMovieInfo = true;
    }

    QBVoDMovieDetailsDSCreateRemainingWidgets(self, app);
    svWidgetSetFocus(self->basicElements.cover);

    if (self->basicElements.recommendedCarousel)
        QBVoDDetailsSetUpRecommendedCarousel(self);

    if (self->basicElements.castMenu)
        QBVoDDetailsSetUpCastMenu(self);

    if (self->basicElements.buttonsMenu) {
        QBVoDDetailsSetUpButtonsMenu(self);
        QBVoDMovieDetailsDSFillButtonsMenu(self, svWidgetGetName(self->basicElements.buttonsMenu));
    }

    if (self->basicElements.likeMenu) {
        QBVoDDetailsSetUpLikeMenu(self);
        QBVoDMovieDetailsDSFillLikeMenu(self, svWidgetGetName(self->basicElements.likeMenu));
    }

    self->details = svSettingsWidgetCreate(app, "MovieDetails");
    svSettingsWidgetAttach(window, self->details, "MovieDetails", 6);
    svWidgetSetHidden(self->details, true);
    self->movieDetails.title = NULL;
    self->movieDetails.description = NULL;
    self->movieDetails.info = NULL;

    self->contentSideMenu = (SvObject) QBContentSideMenuCreate(ctx->appGlobals, SVSTRING("QBVoDMovieMenu.json"), SVSTRING("PC_VOD"));
    QBVoDMovieDetailsDSPullMWData(self->ctx);
    QBGUILogicContentSideMenuRegisterCallbacks(self->ctx->appGlobals->guiLogic, self->contentSideMenu);
    QBContentSideMenuSetEventReporter((QBContentSideMenu) self->contentSideMenu, self->ctx->eventReporter);

    svSettingsPopComponent();

    if (self->ctx->recommendedProvider) {
        QBContentProviderStart(self->ctx->recommendedProvider, self->ctx->appGlobals->scheduler);
        SvInvokeInterface(QBDataModel, self->recommendedDataSource, addListener, (SvObject) self, NULL);
        QBVoDMovieDetailsDSEnsureRecommendedCarousel(self);
    }

    if (self->ctx->castProvider) {
        QBContentProviderStart(self->ctx->castProvider, self->ctx->appGlobals->scheduler);
        SvInvokeInterface(QBDataModel, self->castDataSource, addListener, (SvObject) self, NULL);
        QBVoDMovieDetailsDSEnsureCastMenu(self);
    }

    if (self->ctx->defaultEpisodeImageQueried) {
        log_state("already have default episode image - starting episodes query");
        QBVoDMovieDetailsDSStartEpisodesProvider(self);
    }

    if (self->ctx->userDataProvider) {
        QBContentProviderClear(self->ctx->userDataProvider);
        QBContentProviderRefresh(self->ctx->userDataProvider, NULL, 0, true, false);
        QBContentProviderStart(self->ctx->userDataProvider, self->ctx->appGlobals->scheduler);
    }

    QBProvidersControllerServiceAddListener(ctx->appGlobals->providersController, (SvObject) self, QBProvidersControllerService_INNOV8ON);
    QBProvidersControllerServiceAddListener(ctx->appGlobals->providersController, (SvObject) self, QBProvidersControllerService_MWClient);
    QBAccessManagerAddListener(ctx->appGlobals->accessMgr, (SvObject) self, NULL);

    QBVoDMovieDetailsDSCarousel recommendedCarousel = self->basicElements.recommendedCarousel->prv;
    QBVoDMovieDetailsDSCarousel episodesCarousel = self->basicElements.episodesCarousel ? self->basicElements.episodesCarousel->prv : NULL;
    QBVoDMovieDetailsDSMenu castMenu = self->basicElements.castMenu ? self->basicElements.castMenu->prv : NULL;
    if (ctx->focusedPart == QBVoDMovieDetailsDSFocussedPart_cast && castMenu) {
        svWidgetSetFocus(castMenu->menuWidget);
        QBWidgetsStripSetActive(castMenu->menuWidget, ctx->focusedIndex);
    } else if (ctx->focusedPart == QBVoDMovieDetailsDSFocussedPart_recommended) {
        svWidgetSetFocus(recommendedCarousel->carouselWidget);
        QBVerticalCarouselSetActive(recommendedCarousel->carouselWidget, ctx->focusedIndex);

        QBVoDMovieDetailsDSSliderSlideDown(self, true);
    } else if (ctx->focusedPart == QBVoDMovieDetailsDSFocussedPart_episodes && episodesCarousel) {
        svWidgetSetFocus(episodesCarousel->carouselWidget);
        QBVerticalCarouselSetActive(episodesCarousel->carouselWidget, ctx->focusedIndex);
    }

    QBVoDMovieDetailsDSContextConnectDataSources(ctx);

    if (ctx->eventReporter) {
        //we send watched evnet upon every window creation, because we don't know where we entered the window from.
        //Be it back from the player or from the grid it doesnt matter because thanks to using QBEventReporterWatchedEventFilter
        //we make sure that the watched event is sent only when play occured first and enough content has been played.
        SvInvokeInterface(QBMWClientEventReporter, ctx->eventReporter, sendEvent, ctx->appGlobals->scheduler, QBMWClientEventReporter_watched, NULL);
    }
}

SvLocal void QBVoDMovieDetailsDSContextDestroyWindow(QBWindowContext self_)
{
    log_state("destroying movie details window");
    QBVoDMovieDetailsDSContext self = (QBVoDMovieDetailsDSContext) self_;
    QBVoDMovieDetailsDS info = self->super_.window->prv;

    QBVoDMovieDetailsDSContextDisconnectDataSources(self);

    QBAccessManagerRemoveListener(self->appGlobals->accessMgr, (SvObject) info, NULL);
    if (info->ctx->recommendedProvider)
        QBContentProviderStop(info->ctx->recommendedProvider);

    if (info->ctx->castProvider)
        QBContentProviderStop(info->ctx->castProvider);

    if (info->ctx->detailsProvider)
        QBContentProviderStop(info->ctx->detailsProvider);

    if (info->ctx->episodesProvider)
        QBContentProviderStop(info->ctx->episodesProvider);

    if (info->ctx->userDataProvider)
        QBContentProviderStop(info->ctx->userDataProvider);

    if (info->ctx->backgroundImageProvider)
        QBContentProviderStop(info->ctx->backgroundImageProvider);

    if (info->ctx->seriesDefaultEpisodeImageProvider)
        QBContentProviderStop(info->ctx->seriesDefaultEpisodeImageProvider);

    svWidgetDetach(info->focus.cover);

    if (info->focus.playImage)
        svWidgetDetach(info->focus.playImage);

    svWidgetDestroy(info->focus.cover);

    if (info->focus.playImage)
        svWidgetDestroy(info->focus.playImage);

    if (info->basicElements.recommendedCarousel) {
        QBVoDMovieDetailsDSCarousel recommendedCarousel = info->basicElements.recommendedCarousel->prv;
        svWidgetDestroy(recommendedCarousel->focus);
        svWidgetDetach(recommendedCarousel->w);
        svWidgetDestroy(recommendedCarousel->w);
    }

    if (info->basicElements.episodesCarousel) {
        QBVoDMovieDetailsDSCarousel episodesCarousel = info->basicElements.episodesCarousel->prv;
        svWidgetDestroy(episodesCarousel->focus);
        if (episodesCarousel->titleFocus) {
            svWidgetDetach(episodesCarousel->titleFocus);
            svWidgetDestroy(episodesCarousel->titleFocus);
        }
        svWidgetDetach(episodesCarousel->w);
        svWidgetDestroy(episodesCarousel->w);
    }

    if (info->basicElements.castMenu) {
        QBVoDMovieDetailsDSMenu castMenu = info->basicElements.castMenu->prv;
        svWidgetDetach(castMenu->w);
        svWidgetDestroy(castMenu->w);
    }

    if (info->basicElements.buttonsMenu) {
        QBVoDMovieDetailsDSMenu buttonsMenu = info->basicElements.buttonsMenu->prv;
        svWidgetDetach(buttonsMenu->w);
        svWidgetDestroy(buttonsMenu->w);
    }

    if (info->basicElements.likeMenu) {
        QBVoDMovieDetailsDSMenu likeMenu = info->basicElements.likeMenu->prv;
        svWidgetDetach(likeMenu->w);
        svWidgetDestroy(likeMenu->w);
    }


    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal bool QBVoDMovieDetailsDSContextInputEventHandler(SvObject self_, const QBInputEvent* e)
{
    return false;
}

SvType QBVoDMovieDetailsDSContext_getType(void)
{
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_             = {
            .destroy        = QBVoDMovieDetailsDSContextDestroy
        },
        .reinitializeWindow = QBVoDMovieDetailsDSContextReinitializeWindow,
        .createWindow       = QBVoDMovieDetailsDSContextCreateWindow,
        .destroyWindow      = QBVoDMovieDetailsDSContextDestroyWindow
    };
    static const struct QBWindowContextInputEventHandler_t input = {
        .handleInputEvent = QBVoDMovieDetailsDSContextInputEventHandler
    };
    static const struct QBMWClientEventReporterListener_ reporterListener = {
        .requestStateChanged = QBVoDMovieDetailsDSContextRequestStateChanged
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBVoDMovieDetailsDSContext",
                            sizeof(struct QBVoDMovieDetailsDSContext_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            QBWindowContextInputEventHandler_getInterface(), &input,
                            QBMWClientEventReporterListener_getInterface(), &reporterListener,
                            NULL);
    }

    return type;
}

SvLocal void QBVoDMovieDetailsDSSetBackground(QBVoDMovieDetailsDS self)
{
    if (QBVoDMovieDetailsDSShouldBeBlocked(self->ctx)) {
        if (self->basicElements.backgroundImage) {
            log_debug("blocked content - Hiding background image");
            svWidgetDetach(self->basicElements.backgroundImage);
            svWidgetDestroy(self->basicElements.backgroundImage);
            self->basicElements.backgroundImage = NULL;
        }
        if (self->basicElements.backgroundImageTint) {
            svWidgetDetach(self->basicElements.backgroundImageTint);
            svWidgetDestroy(self->basicElements.backgroundImageTint);
            self->basicElements.backgroundImageTint = NULL;
        }
        return;
    }

    SvString backgroundImage = SVTESTRETAIN(QBMovieUtilsGetAttr(self->ctx->object, "backgroundImage", NULL));
    if (!backgroundImage || self->basicElements.backgroundImage) {
        SVTESTRELEASE(backgroundImage);
        //do nothing if there is no background image or it has already been set.
        return;
    }
    svSettingsRestoreContext(self->settingsCtx);
    log_debug("creating background image");

    self->basicElements.backgroundImage = svIconNew(self->ctx->appGlobals->res, "QBVoDMovieDetails.MovieBackground");
    svIconSetBitmapFromURI(self->basicElements.backgroundImage, 0, SvStringCString(backgroundImage));
    svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.backgroundImage, "QBVoDMovieDetails.MovieBackground", 3);

    if (svSettingsIsWidgetDefined("QBVoDMovieDetails.MovieBackgroundTint")) {
        self->basicElements.backgroundImageTint = svSettingsWidgetCreate(self->ctx->appGlobals->res, "QBVoDMovieDetails.MovieBackgroundTint");
        svSettingsWidgetAttach(self->ctx->super_.window, self->basicElements.backgroundImageTint, "QBVoDMovieDetails.MovieBackgroundTint", 4);
    }

    svSettingsPopComponent();
    SVRELEASE(backgroundImage);
}

SvLocal void QBVoDMovieDetailsDSSetSeasonNumber(QBVoDMovieDetailsDS self)
{
    if (self->episodesDataSource) {
        size_t episodesCount = SvInvokeInterface(QBListModel, self->episodesDataSource, getLength);
        for (size_t i = 0; i < episodesCount; i++) {
            SvDBRawObject episode = (SvDBRawObject) SvInvokeInterface(QBListModel, self->episodesDataSource, getObject, i);
            if (episode && SvObjectIsInstanceOf((SvObject) episode, SvDBRawObject_getType())) {
                SvValue seasonVal = (SvValue) SvDBRawObjectGetAttrValue(episode, "seasonNumber");
                if (seasonVal && SvObjectIsInstanceOf((SvObject) seasonVal, SvValue_getType()) && SvValueIsInteger(seasonVal)) {
                    size_t seasonNumber = (size_t) SvValueGetInteger(seasonVal);
                    if (seasonNumber > self->ctx->seasonsCount) {
                        self->ctx->seasonsCount = seasonNumber;
                    }
                    if (self->basicElements.episodesCarousel) {
                        QBVoDMovieDetailsDSCarousel episodesCarousel = self->basicElements.episodesCarousel->prv;
                        if ((int) i == episodesCarousel->activeElementIndex) {
                            self->ctx->selectedSeasonIndex = seasonNumber - 1;
                            SvString seasonName = SvStringCreateWithFormat(gettext("Season: %zu"), seasonNumber);
                            QBAsyncLabelSetText(episodesCarousel->title, seasonName);
                            SVRELEASE(seasonName);
                        }
                    }
                }
            }
        }
    }
}

SvLocal void QBVoDMovieDetailsDSItemsChanged(SvObject self_, SvObject dataSource, size_t start, size_t offset)
{
    log_debug("data changed - refreshing window");
    QBVoDMovieDetailsDS self = (QBVoDMovieDetailsDS) self_;
    QBContentProvider dataProvider = self->ctx->dataProviders ? (QBContentProvider) SvHashTableFind(self->ctx->dataProviders, dataSource) : NULL;

    if (dataProvider && dataProvider == self->ctx->seriesDefaultEpisodeImageProvider) {
        // got data for series default episode image background
        log_state("got data for series default episode image background");
        QBVoDMovieDetailsDSStartEpisodesProvider(self);
    } else if (dataSource == self->recommendedDataSource) {
        // got data for recommendations carousel
        log_state("got data for recommendations carousel");
        QBVoDMovieDetailsDSEnsureRecommendedCarousel(self);
    } else if (dataSource == self->episodesDataSource) {
        // got data for episodes carousel
        log_state("got data for episodes carousel");
        QBVoDMovieDetailsDSEnsureEpisodesCarousel(self);
        QBVoDMovieDetailsDSSetSeasonNumber(self);
    } else if (dataProvider && dataProvider == self->ctx->userDataProvider) {
        // got data about user like/dislike status and user list containment
        log_state("got data about user like/dislike status and user list containment");
        QBVoDDetailsSetUpLikeMenu(self);
        QBVoDDetailsSetUpButtonsMenu(self);
    } else if (dataProvider && dataProvider == self->ctx->backgroundImageProvider) {
        // got data for background image
        log_state("got data for background image");
        QBVoDMovieDetailsDSSetBackground(self);
    } else {
        // got data for top cast member
        log_state("got data for top cast member");
        QBVoDMovieDetailsDSEnsureCastMenu(self);
    }
}

SvLocal void QBVoDDetailsShowSideMenu(QBVoDMovieDetailsDS self)
{
    QBContentSideMenu contentSideMenu = (QBContentSideMenu) self->contentSideMenu;
    QBContentSideMenuSetImmediateAction(contentSideMenu, QBContentSideMenuImmediateAction_none);

    if (QBVoDMovieDetailsDSShouldBeBlocked(self->ctx)) {
        QBVoDMovieDetailsDSCheckParentalControlPIN(self, QBVoDMovieDetailsDSParentalControlCheck_showSideMenu);
        return;
    }

    svSettingsRestoreContext(self->settingsCtx);
    if (QBContextMenuIsShown((QBContextMenu) contentSideMenu))
        goto finish;

    SvObject contentInfo = QBVoDMovieDetailsDSContextCreateContentInfo((QBWindowContext) self->ctx);
    QBContentSideMenuSetContentInfo(contentSideMenu, contentInfo);
    QBContextMenuShow((QBContextMenu) contentSideMenu);
    SVRELEASE(contentInfo);

finish:
    svSettingsPopComponent();
}

SvLocal void
QBVoDMovieDetailsDSServiceNotificationNoop(SvObject self_, SvString serviceId)
{
}

SvLocal void
QBVoDMovieDetailsDSServiceRemoved(SvObject self_, SvString serviceId)
{
    QBVoDMovieDetailsDS self = (QBVoDMovieDetailsDS) self_;
    if (SvStringEqualToCString(self->ctx->serviceId, SvStringCString(serviceId))) {
        QBApplicationControllerSwitchToRoot(self->ctx->appGlobals->controller);
        QBApplicationControllerPushContext(self->ctx->appGlobals->controller, self->ctx->appGlobals->main);
    }
}

SvLocal void
QBVoDMovieDetailsDSMiddlewareDataChanged(SvObject self_,
                                         QBMiddlewareManagerType middlewareType)
{
    QBVoDMovieDetailsDS self = (QBVoDMovieDetailsDS) self_;

    if (middlewareType != QBMiddlewareManagerType_Innov8on) {
        return;
    }

    if (!QBMiddlewareManagerGetId(self->ctx->appGlobals->middlewareManager, middlewareType)) {
        QBApplicationControllerSwitchToRoot(self->ctx->appGlobals->controller);
        QBApplicationControllerPushContext(self->ctx->appGlobals->controller, self->ctx->appGlobals->main);
    }
}

SvLocal void QBVoDMovieDetailsDS__dtor__(void *self_)
{
    QBVoDMovieDetailsDS self = self_;

    self->ctx->movieDetails = NULL;
    SVRELEASE(self->contentSideMenu);
    SVTESTRELEASE(self->recommendedDataSource);
    SVTESTRELEASE(self->castDataSource);
    SVTESTRELEASE(self->buttonsDataSource);
    SVTESTRELEASE(self->likeDataSource);
    SVTESTRELEASE(self->episodesDataSource);
    SVTESTRELEASE(self->activeChildElement);
}

SvLocal void
QBVoDMovieDetailsDSAuthenticationChanged(SvObject self_, QBAccessManager manager, SvString domainName)
{
    QBVoDMovieDetailsDS self = (QBVoDMovieDetailsDS) self_;

    if (!SvStringEqualToCString(domainName, "PC_VOD")) {
        return;
    }

    QBVoDMovieDetailsDSHandlePCChange(self);
}

SvLocal SvType QBVoDMovieDetailsDS_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDMovieDetailsDS__dtor__
    };
    static SvType type = NULL;

    static const struct QBListModelListener2_ listModelListener = {
        .itemsAdded     = QBVoDMovieDetailsDSItemsChanged,
        .itemsRemoved   = QBVoDMovieDetailsDSItemsChanged,
        .itemsChanged   = QBVoDMovieDetailsDSItemsChanged,
        .itemsReordered = QBVoDMovieDetailsDSItemsChanged,
        .itemsReplaced  = QBVoDMovieDetailsDSItemsChanged
    };

    static const struct QBProvidersControllerServiceListener_t providersControllerServiceMethods = {
        .serviceAdded    = QBVoDMovieDetailsDSServiceNotificationNoop,
        .serviceRemoved  = QBVoDMovieDetailsDSServiceRemoved,
        .serviceModified = QBVoDMovieDetailsDSServiceNotificationNoop
    };

    static const struct QBMiddlewareManagerListener_t middlewareListenerMethods = {
        .middlewareDataChanged = QBVoDMovieDetailsDSMiddlewareDataChanged
    };

    static const struct QBAccessManagerListener_t authenticationListenerMethods = {
        .authenticationChanged = QBVoDMovieDetailsDSAuthenticationChanged
    };

    if (!type) {
        SvTypeCreateManaged("QBVoDMovieDetailsDS",
                            sizeof(struct QBVoDMovieDetailsDS_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBListModelListener2_getInterface(), &listModelListener,
                            QBProvidersControllerServiceListener_getInterface(), &providersControllerServiceMethods,
                            QBMiddlewareManagerListener_getInterface(), &middlewareListenerMethods,
                            QBAccessManagerListener_getInterface(), &authenticationListenerMethods,
                            NULL);
    }

    return type;
}

QBWindowContext QBVoDMovieDetailsDSContextCreate(AppGlobals appGlobals, SvDBRawObject object, QBContentProvider extProvider, SvString serviceId, SvString serviceName, SvObject externalTree)
{
    log_state("creating movie details context");
    QBVoDMovieDetailsDSContext self = (QBVoDMovieDetailsDSContext) SvTypeAllocateInstance(QBVoDMovieDetailsDSContext_getType(), NULL);
    svSettingsPushComponent("QBVoDMovieDetailsDS.settings");
    bool useBackground = svSettingsIsWidgetDefined("QBVoDMovieDetails.MovieBackground");
    self->defaultEpisodeImageQueried = QBVoDDetailsGetBoolAttribute(object, "defaultEpisodeImageQueried");
    self->castMaxCount = svSettingsGetInteger("QBVoDMovieDetails.CastMenu.Strip", "maxWidgets", 0);
    svSettingsPopComponent();

    self->vodLogic = QBVoDLogicNew(appGlobals);

    self->ownerTree = externalTree;
    if (!self->ownerTree)
        self->ownerTree = (SvObject) appGlobals->menuTree;

    self->appGlobals = appGlobals;
    self->serviceId = SVTESTRETAIN(serviceId);
    self->serviceName = SVRETAIN(serviceName);
    self->externalProvider = SVRETAIN(extProvider);
    self->movieDetails = NULL;

    SvString productId = SvValueGetString(SvDBObjectGetID((SvDBObject) object));
    self->recommendationsTree = QBContentTreeCreate(productId, NULL);
    self->castTree = QBContentTreeCreate(productId, NULL);
    self->object = SVRETAIN(object);
    self->objectType = QBVoDMovieDetailsDSObjectType_movie;
    SvString type = QBMovieUtilsGetAttr(self->object, "objectType", NULL);
    if (type && SvStringEqualToCString(type, "Series")) {
        self->objectType = QBVoDMovieDetailsDSObjectType_series;
    } else if (type && SvStringEqualToCString(type, "Episode")) {
        self->objectType = QBVoDMovieDetailsDSObjectType_episode;
    } else if (type && SvStringEqualToCString(type, "Person")) {
        self->objectType = QBVoDMovieDetailsDSObjectType_person;
    }
    SvObject cubiId = SvDBRawObjectGetAttrValue(self->object, "cubiware.id");
    if (cubiId && SvObjectIsInstanceOf(cubiId, SvValue_getType())) {
        SvDBRawObjectSetDomainId(self->object, SvDBRawObjectDomain_Cubiware, (SvValue) cubiId);
    } else if (cubiId && SvObjectIsInstanceOf(cubiId, SvString_getType())) {
        SvValue cubiIdVal = SvValueCreateWithString((SvString) cubiId, NULL);
        SvDBRawObjectSetDomainId(self->object, SvDBRawObjectDomain_Cubiware, cubiIdVal);
        SVRELEASE(cubiIdVal);
    }

    SvObject contentManager = QBContentProviderGetManager(self->externalProvider);
    if (contentManager) {
        SvArray actions = SvInvokeInterface(QBContentManager, contentManager, constructActionsForContent, (SvObject) object, NULL);
        if (actions) {
            SvIterator it = SvArrayIterator(actions);
            QBContentAction act;
            QBContentAction recommendedAction = NULL;
            QBContentAction castAction = NULL;
            QBContentAction episodesAction = NULL;
            QBContentAction reportAction = NULL;
            QBContentAction personDetailsAction = NULL;
            QBContentAction userDetailsAction = NULL;
            QBContentAction backgroundImageAction = NULL;
            QBContentAction seriesDefaultEpisodeImageAction = NULL;

            while ((act = (QBContentAction) SvIteratorGetNext(&it))) {
                if (QBContentActionGetType(act, NULL) == QBContentActionType_relatedContent) {
                    if (!recommendedAction &&
                        (SvStringEqualToCString(QBContentActionGetName(act, NULL), "recommended") ||
                         SvStringEqualToCString(QBContentActionGetName(act, NULL), "movies"))) {
                        recommendedAction = act;
                    }
                    if (!castAction && SvStringEqualToCString(QBContentActionGetName(act, NULL), "cast")) {
                        castAction = act;
                    }
                    if (!episodesAction && SvStringEqualToCString(QBContentActionGetName(act, NULL), "content")) {
                        episodesAction = act;
                    }
                    if (!reportAction && SvStringEqualToCString(QBContentActionGetName(act, NULL), "reportEvent")) {
                        reportAction = act;
                    }
                    if (!personDetailsAction && SvStringEqualToCString(QBContentActionGetName(act, NULL), "personDetails")) {
                        personDetailsAction = act;
                    }
                    if (!userDetailsAction && SvStringEqualToCString(QBContentActionGetName(act, NULL), "userInteractionDetails")) {
                        userDetailsAction = act;
                    }
                    if (!backgroundImageAction && SvStringEqualToCString(QBContentActionGetName(act, NULL), "backgroundImage")) {
                        backgroundImageAction = act;
                    }
                    if (!seriesDefaultEpisodeImageAction && SvStringEqualToCString(QBContentActionGetName(act, NULL), "seriesDefaultEpisodeImage")) {
                        seriesDefaultEpisodeImageAction = act;
                    }
                }
            }

            if (castAction) {
                log_state("cast action found - creating provider");
                self->castProvider = (QBContentProvider) SvInvokeInterface(QBContentManager, contentManager, createActionContentProvider,
                                                       self->castTree, (SvObject) self->externalProvider, (SvObject) object, castAction, NULL);
            }
            if (episodesAction) {
                log_state("series episodes action found - creating provider");
                self->episodesTree = QBContentTreeCreate(productId, NULL);
                self->episodesProvider = (QBContentProvider) SvInvokeInterface(QBContentManager, contentManager, createActionContentProvider,
                                                           self->episodesTree, (SvObject) self->externalProvider, (SvObject) object, episodesAction, NULL);
            }
            if (reportAction) {
                log_state("user interaction reporting action found - creating event reporter");
                self->eventReporter = SvInvokeInterface(QBContentManager, contentManager, createActionRequest,
                                                        (SvObject) object, reportAction, NULL, NULL);
                if (self->eventReporter && SvObjectIsImplementationOf(self->eventReporter, QBMWClientEventReporter_getInterface())) {
                    QBEventReporterWatchedEventFilter filter = QBEventReporterWatchedEventFilterCreate(self->appGlobals->bookmarkManager,
                                                                                                       self->object,
                                                                                                       self->eventReporter,
                                                                                                       NULL);
                    SVRELEASE(self->eventReporter);
                    self->eventReporter = (SvObject) filter;
                    SvInvokeInterface(QBMWClientEventReporter, self->eventReporter, setListener, (SvObject) self, NULL);
                    SvInvokeInterface(QBMWClientEventReporter, self->eventReporter, sendEvent, self->appGlobals->scheduler, QBMWClientEventReporter_viewDetails, NULL);
                } else {
                    SVTESTRELEASE(self->eventReporter);
                    self->eventReporter = NULL;
                }
            }
            if (personDetailsAction) {
                log_state("person details action found - creating provider");
                self->detailsTree = QBContentTreeCreate(productId, NULL);
                self->detailsProvider = (QBContentProvider) SvInvokeInterface(QBContentManager, contentManager, createActionContentProvider,
                                                          self->detailsTree, (SvObject) self->externalProvider, (SvObject) object, personDetailsAction, NULL);
                QBContentProviderRefresh(self->detailsProvider, NULL, 0, true, false);
                QBContentProviderStart(self->detailsProvider, self->appGlobals->scheduler);
            }
            if (userDetailsAction) {
                log_state("user interaction details action found - creating provider");
                self->userDataProvider = (QBContentProvider) QBVoDMovieDetailsDSContextCreateActionProvider(self,
                                                                                        contentManager,
                                                                                        (SvDBObject) object,
                                                                                        userDetailsAction);
            }
            if (backgroundImageAction && useBackground) {
                log_state("background image action found - creating provider");
                self->backgroundImageProvider = (QBContentProvider) QBVoDMovieDetailsDSContextCreateActionProvider(self,
                                                                                               contentManager,
                                                                                               (SvDBObject) object,
                                                                                               backgroundImageAction);
                QBContentProviderRefresh(self->backgroundImageProvider, NULL, 0, true, false);
                QBContentProviderStart(self->backgroundImageProvider, self->appGlobals->scheduler);
            }
            if (seriesDefaultEpisodeImageAction && !self->defaultEpisodeImageQueried) {
                log_state("series default image action found - creating provider");
                self->seriesDefaultEpisodeImageProvider = (QBContentProvider) QBVoDMovieDetailsDSContextCreateActionProvider(self,
                                                                                                         contentManager,
                                                                                                         (SvDBObject) object,
                                                                                                         seriesDefaultEpisodeImageAction);
                QBContentProviderRefresh(self->seriesDefaultEpisodeImageProvider, NULL, 0, true, false);
                QBContentProviderStart(self->seriesDefaultEpisodeImageProvider, self->appGlobals->scheduler);
            }

            if (recommendedAction) {
                log_state("recommended action found - creating provider");
                self->recommendedProvider = (QBContentProvider) SvInvokeInterface(QBContentManager, contentManager, createActionContentProvider,
                                                              self->recommendationsTree, (SvObject) self->externalProvider, (SvObject) object,
                                                              recommendedAction, NULL);
            }
            SVRELEASE(actions);
        }
    } else {
        Innov8onProviderParams params = (Innov8onProviderParams) SvObjectCopy((SvObject) Innov8onProviderGetParams((Innov8onProvider) self->externalProvider, NULL), NULL);
        SvObject requestPlugin = Innov8onProviderRequestPluginGetRecommendationsCreate();
        Innov8onProviderParamsSetRequestPlugin(params, requestPlugin);
        SVRELEASE(requestPlugin);
        self->recommendedProvider = (QBContentProvider) Innov8onProviderCreate(self->recommendationsTree, params, self->serviceId, productId,
                                                                      false, false, NULL);
        SVRELEASE(params);
    }

    static const QBLongKeyPressExtendedMapping map[] = {
        { QBKEY_BACK, QBKEY_BACK, QBKEY_VOD, 3 * 1000 * 1000, false },
        { 0,          0,          0,         0,               false }
    };
    self->exitInputFilter = (QBLongKeyPressFilter) SvTypeAllocateInstance(QBLongKeyPressFilter_getType(), NULL);
    QBLongKeyPressFilterInitExtended(self->exitInputFilter, map, NULL);

    return (QBWindowContext) self;
}

SvLocal SvObject
QBVoDMovieDetailsDSContextCreateContentInfo(QBWindowContext ctx_)
{
    QBVoDMovieDetailsDSContext self = (QBVoDMovieDetailsDSContext) ctx_;
    SvDBRawObject object = self->object;
    if (self->movieDetails && self->movieDetails->episodesDataSource && self->objectType == QBVoDMovieDetailsDSObjectType_series) {
        size_t episodesCount = SvInvokeInterface(QBListModel, self->movieDetails->episodesDataSource, getLength);
        if (episodesCount > 0) {
            object = (SvDBRawObject) SvInvokeInterface(QBListModel, self->movieDetails->episodesDataSource, getObject, 0);
        }
    }

    SvObject contentInfo = QBContentInfoCreate(self->appGlobals, self->serviceId, self->serviceName, (SvObject) object, NULL, (SvObject) self->externalProvider, self->ownerTree);
    return contentInfo;
}
