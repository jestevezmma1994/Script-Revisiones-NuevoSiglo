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

#include <Windows/QBVoDGridBrowser.h>

#include <libintl.h>
#include <QBCarousel/QBVerticalCarousel.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <QBContentManager/Innov8onUtils.h>
#include <Windows/pvrplayer.h>
#include <Widgets/QBSimpleDialog.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <Widgets/QBMovieInfo.h>
#include <QBWidgets/QBGrid.h>
#include <QBWidgets/QBSearchProgressDialog.h>
#include <Widgets/authDialog.h>
#include <Widgets/infoIcons.h>
#include <Widgets/trailerPreview.h>
#include <Widgets/QBVoDGrid.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBInnov8onLoadablePane.h>
#include <ContextMenus/QBInnov8onExtendedInfoPane.h>
#include <ContextMenus/QBOSKPane.h>
#include <QBPlayerControllers/QBVoDAnyPlayerController.h>
#include <QBPlayerControllers/QBPlaylistController.h>
#include <Middlewares/Innov8on/innov8onTransaction.h>
#include <Logic/AnyPlayerLogic.h>
#include <Utils/authenticators.h>
#include <Utils/QBMovieUtils.h>
#include <Utils/QBContentInfo.h>
#include <Utils/QBVODUtils.h>
#include <Utils/vod/QBVoDSubtrees.h>
#include <player_hints/http_input.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentSeeAll.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentCategoryListener.h>
#include <QBContentManager/QBContentProvider.h>
#include <Services/core/QBMiddlewareManager.h>
#include <SvPlayerKit/SvContent.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <QBOSK/QBOSKKey.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenu.h>
#include <Windows/QBVoDMovieDetails.h>
#include <Windows/Digitalsmiths/QBVoDMovieDetailsDS.h>
#include <Windows/tv/adWindow.h>
#include <Widgets/QBRatingWidget.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBListProxy.h>
#include <QBInput/QBInputCodes.h>
#include <QBInput/Filters/QBLongKeyPressFilter.h>
#include <QBInput/QBInputService.h>
#include <Widgets/movieInfo.h>
#include <QBConf.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/QBFrame.h>
#include <SWL/icon.h>
#include <main.h>
#include <Logic/GUILogic.h>
#include <SvCore/SvEnv.h>
#include <Services/QBAccessController/QBAccessManagerListener.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBContentManager/QBContentManager.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBRangeTree.h>
#include <Services/core/QBContentManagers.h>
#include <ctype.h>
#include <Utils/boldUtils.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "QBVoDGridBrowserLogLevel", "");

#define log_error(fmt, ...)   do { if (env_log_level() >= 0) SvLogError(COLBEG() "[%s] " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#define log_warning(fmt, ...) do { if (env_log_level() >= 0) SvLogWarning(COLBEG() "[%s] " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
#define log_state(fmt, ...)   do { if (env_log_level() >= 1) SvLogNotice(COLBEG() "[%s] " fmt COLEND_COL(cyan), __func__, ## __VA_ARGS__); } while (0)
#define log_debug(fmt, ...)   do { if (env_log_level() >= 2) SvLogNotice("[%s] " fmt, __func__, ## __VA_ARGS__); } while (0)

#define MAX_NUM_BUTTONS 5

typedef enum {
    QBVoDGridBrowserMode_movies,
    QBVoDGridBrowserMode_search
} QBVodGridBrowserMode;

typedef enum {
    QBVodGridBrowserContextType_default,        ///< context for displaying main VoD data tree
    QBVodGridBrowserContextType_search,         ///< context for displaying search results
    QBVodGridBrowserContextType_watched,        ///< context tor displaying watched list contents
    QBVodGridBrowserContextType_recommended,    ///< context tor displaying recommendations
    QBVodGridBrowserContextType_myList          ///< context for displaying my list contents
} QBVodGridBrowserContextType;

/**
 * Enum determine action after successful PC verification.
 **/
typedef enum {
    QBVoDGridBrowserParentalControlCheck_movieDetailsWindow, ///< Show movie details window
    QBVoDGridBrowserParentalControlCheck_movieDetailsSideMenu, ///< Show side menu with description
    QBVoDGridBrowserParentalControlCheck_optionsSideMenu ///< Show option side menu
} QBVoDGridBrowserParentalControlCheck;

struct QBVoDGridBrowserContext_ {
    struct QBWindowContext_t super_;

    AppGlobals appGlobals;
    SvObject path;
    SvObject originalPath;
    SvObject filterNode;
    QBContentProvider provider;
    SvObject dataSource;
    SvString serviceId;
    SvString serviceName;
    bool isSearchStarted;
    bool searchContentAvailable; ///< True when search content is not empty

    SvObject tree;

    int lastMenuPos;
    int lastMenuItemPos;
    QBVodGridBrowserMode lastMode; ///< Stored mode of VOD grid browser
    SvSet unlockedGridItems;    ///< Set with categories which have been unlocked.
    bool isCategoriesView;          ///< True when browser contains only categories (first view in vod4).

    SvArray childProviders; ///< slave providers not part of the main VoD data tree (like search provider for innov8on)
    QBContentSearch contentSearch; ///< search category associated with the context
    bool focusKeyboard; ///< flag telling if the OSK keyboard should be displayed when creating window (useful for search window)
    QBVoDGridBrowserSettings settings; ///< Settings that decide on grid behaviour when user interacts with it
    QBContentManagerType contentManagerType; ///< Type of content manager providing data to the view
    QBVodGridBrowserContextType contextType; ///< Type of view displayed in the context
    QBVoDSubtrees subtreesContainer; ///< subtrees containing data for tabs (which is not included in the main VoD tree)
    QBLongKeyPressFilter exitInputFilter; ///< input filter for handling 3sec exit button hold
    SvObject eventReporter; ///< event reporter for reporting events associated with a movie (lay, rent etc.)

    bool hasButtonMyList; // AMERELES MyList y Hide FakeCategory
};

typedef struct {
    SvWidget caption;
    SvWidget active;
    SvWidget focus;
    SvWidget icon;
} QBVodGridBrowserButton;

typedef struct QBVoDGridBrowser_ *QBVoDGridBrowser;

struct QBVodGridBrowserNavigationBar_ {
    SvWidget w;

    SvWidget buttonMovies;      ///< button for returning to the main VoD view
    SvWidget buttonSearch;      ///< button for entering search
    SvWidget buttonWatched;     ///< button for displaying watchlist
    SvWidget buttonMyList;      ///< button for displaying my list
    SvWidget buttonRecommended; ///< button for displaying recommendations

    SvWidget focusQueue[MAX_NUM_BUTTONS]; ///< helper array for handling focus order of navigation bar
    ssize_t focusQueueIndex;    ///< index of focused button

    QBVoDGridBrowser info;      ///< grid widget
};
typedef struct QBVodGridBrowserNavigationBar_* QBVodGridBrowserNavigationBar;

struct QBVoDGridBrowser_ {
    struct SvObject_ super_;

    QBVoDGridBrowserContext ctx;
    SvWidget window;

    SvWidget title;

    SvWidget details;

    SvWidget grid;

    struct {
        SvWidget title;
        SvWidget description;
        SvWidget info;
    } movieDetails;

    SvTimerId timer;

    SvDBRawObject highlightedProduct;
    QBContentCategory categoryOfHighlightedProduct;
    QBContentCategory highlightedCategory;

    unsigned int settingsContext;

    SvWidget navigationBar;

    QBContextMenu contextMenu;

    SvWidget searchRow;
    int searchRowIdx;
    bool searchResultsAvailable;

    SvWidget parentalPopup;
    QBSearchProgressDialog searchProgressDialog; ///< dialog shown during search and in case of lack of results
    
    bool searchProgressDialogVisible; // AMERELES Fix en la navegación del menú de la derecha

    double showDetailsDelay;

    QBVodGridBrowserMode mode;

    QBAdWindow adWindow;

    bool isCurrentItemLocked;
    bool isCurrentCategoryLocked;

    SvObject contentSideMenu;
    
    // AMERELES MyList y Hide FakeCategory
    int searchRowIdxOffset;
    SvWidget tileToHideMyList;
    SvWidget noResultsPopup;
};

SvLocal void QBVoDGridBrowserSelectMovies(QBVoDGridBrowser self, bool immediately, bool navigateToLastMenuItem);
SvLocal void QBVoDGridBrowserSelectSearch(QBVoDGridBrowser self);
SvLocal void QBVoDGridBrowserShowMovieDetails(QBVoDGridBrowser self);
SvLocal void QBVoDGridBrowserHideMovieDetails(QBVoDGridBrowser self);
SvLocal QBWindowContext
QBVoDGridBrowserContextCreateImpl(AppGlobals appGlobals, SvObject path,
                                  QBContentProvider provider, SvString serviceId,
                                  SvString serviceName, SvObject tree,
                                  SvSet unlockedCategories,
                                  QBVoDGridBrowserSettings settings, bool isCategoriesView,
                                  QBVoDSubtrees subtreesContainer, QBVodGridBrowserContextType contextType);
SvLocal void
QBVoDGridBrowserMenuSetFocus(QBVoDGridBrowser self);
SvLocal void
QBVoDGridBrowserContextInit(QBVoDGridBrowserContext self, SvObject tree, SvObject path, QBContentProvider provider, bool isCategoriesView, QBVodGridBrowserContextType contextType);
SvLocal void
QBVoDGridBrowserSearchMenuShow(QBVoDGridBrowser self);

void
QBVoDGridBrowserContextReset(QBVoDGridBrowserContext self)
{
    self->settings->defaultButtonIndex = 0;
}

SvLocal void
QBVoDGridBrowserSettings__dtor__(void* self_)
{
    QBVoDGridBrowserSettings self = (QBVoDGridBrowserSettings) self_;
    SVTESTRELEASE(self->settingsFile);
}

SvLocal SvType
QBVoDGridBrowserSettings_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDGridBrowserSettings__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDGridBrowserSettings",
                            sizeof(struct QBVoDGridBrowserSettings_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}


QBVoDGridBrowserSettings
QBVoDGridBrowserSettingsCreate(void)
{
    return (QBVoDGridBrowserSettings) SvTypeAllocateInstance(QBVoDGridBrowserSettings_getType(), NULL);
}

/**
 * stop all providers asociated with the window
 **/
SvLocal void
QBVoDGridBrowserContextStopProviders(QBVoDGridBrowserContext self)
{
    QBContentProviderStop(self->provider);
    if (self->childProviders) {
        for (size_t i = 0; i < SvArrayGetCount(self->childProviders); i++) {
            QBContentProvider provider = (QBContentProvider) SvArrayGetObjectAtIndex(self->childProviders, i);
            QBContentProviderStop(provider);
        }
    }

    QBVoDSubtree subTree = QBVoDSubtreesGetSubtree(self->subtreesContainer, self->contextType);
    if (subTree) {
        QBVoDSubtreeStopProviders(subTree);
    }
    log_debug("Stopped providers");
}

/**
 * find an object identifying a provider given its type and/or id
 **/
SvLocal SvObject
QBVoDGridBrowserContextGetProviderObject(QBVoDGridBrowserContext ctx, const char* type, const char* id)
{
    SvObject searchObj = NULL;
    QBContentManagersService contentManagers = (QBContentManagersService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                      SVSTRING("QBContentManagersService"));
    SvObject contentManager = SVTESTRETAIN(QBContentManagersServiceFindFirst(contentManagers,
                                                                             ctx->contentManagerType));
    if (!contentManager) {
        log_error("Can't find a contentManager - cannot create search provider");
        goto out;
    }

    SvObject serviceMonitor = QBContentManagersServiceGetServiceMonitor(contentManagers,
                                                                        contentManager);
    if (!serviceMonitor) {
        log_error("Can't find serviceMonitor - cannot create search provider");
        goto out;
    }

    SvObject obj = NULL;
    ssize_t n = SvInvokeInterface(QBListModel, serviceMonitor, getLength) - 1;
    while (n >= 0) {
        obj = SvInvokeInterface(QBListModel, serviceMonitor, getObject, (size_t) n);
        if (SvObjectIsInstanceOf(obj, SvDBRawObject_getType())) {
            SvString typeStr = SvDBObjectGetType((SvDBObject) obj);
            SvValue idVal = SvDBObjectGetID((SvDBObject) obj);
            SvString idStr = idVal && SvValueIsString(idVal) ? SvValueGetString(idVal) : NULL;
            if (type && id && typeStr && SvStringEqualToCString(typeStr, type) && idStr && SvStringEqualToCString(idStr, id)) {
                log_debug("found provider identifying object with type: %s and id: %s", type, id);
                searchObj = SVRETAIN(obj);
                break;
            } else if (id && !type && idStr && SvStringEqualToCString(idStr, id)) {
                log_debug("found provider identifying object with id: %s", id);
                searchObj = SVRETAIN(obj);
                break;
            } else if (type && !id && typeStr && SvStringEqualToCString(typeStr, type)) {
                log_debug("found provider identifying object with type: %s", type);
                searchObj = SVRETAIN(obj);
                break;
            }
        }
        n--;
    }

out:
    SVTESTRELEASE(contentManager);
    return searchObj;
}

/**
 * create provider of a given type and/or id and set it as data provider for a given category
 **/
SvLocal SvObject
QBVoDGridBrowserContextAddProviderToCategory(QBVoDGridBrowserContext ctx,
                                             QBContentCategory category,
                                             const char* providerType,
                                             const char* providerId)
{
    QBContentManagersService contentManagers = (QBContentManagersService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                      SVSTRING("QBContentManagersService"));
    SvObject contentManager = QBContentManagersServiceFindFirst(contentManagers, ctx->contentManagerType);
    if (!contentManager) {
        log_error("Can't find a contentManager - cannot create provider of type: %s and id: %s",
                  (providerType ? providerType : "NULL"),
                  (providerId ? providerId : "NULL"));
        return NULL;
    }

    SvObject providerIdentifyingObj = QBVoDGridBrowserContextGetProviderObject(ctx, providerType, providerId);
    if (!providerIdentifyingObj) {
        log_error("Can't find provider of type: %s and id: %s",
                  (providerType ? providerType : "NULL"),
                  (providerId ? providerId : "NULL"));
        return NULL;
    }

    if (SvObjectIsInstanceOf(providerIdentifyingObj, SvDBRawObject_getType())) {
        //some providers need a parent to be constructed correctly (for example innov8on search provider)
        SvDBRawObject rawObject = (SvDBRawObject) providerIdentifyingObj;
        SvDBRawObjectSetAttrValue(rawObject, "parentProvider", (SvObject) ctx->provider);
    }

    QBContentTreePath providerPath = QBContentTreePathCreate((QBContentCategory) category, NULL);
    QBContentTree tree = QBContentCategoryTakeTree(category);
    SvObject provider = SvInvokeInterface(QBContentManager, contentManager, createContentProvider,
                                          tree, providerPath, providerIdentifyingObj, NULL);
    if (!provider) {
        log_error("contentManager cannot create provider for identyfying object of type: %s, providerType: %s, providerId :%s",
                  SvObjectGetTypeName(providerIdentifyingObj),
                  (providerType ? providerType : "NULL"),
                  (providerId ? providerId : "NULL"));
    }
    log_debug("provider added to category: %s", SvStringCString(QBContentCategoryGetName(category)));
    SVRELEASE(tree);
    SVRELEASE(providerPath);
    SVRELEASE(providerIdentifyingObj);
    return provider;
}

/**
 * create a child QBContentTree for storing data of a tab (like My list, watched, recommended, search)
 **/
SvLocal QBContentTree
QBVoDGridBrowserContextCreateChildTree(const char* rootName)
{
    SvString rootId = SvStringCreate(rootName, NULL);
    QBContentTree childTree = QBContentTreeCreate(rootId, NULL);
    QBContentCategory rootCategory = QBContentTreeGetRootCategory(childTree);
    SVRELEASE(rootId);

    QBRangeTree tree = QBRangeTreeCreate();
    QBContentCategorySetItemsSource(rootCategory, (SvObject) tree);
    QBRangeTreeSetRangeSize(tree, 30);
    SVRELEASE(tree);

    return childTree;
}

/**
 * add a child category with given name and empty message to the parent category.
 **/
SvLocal QBContentCategory
QBVoDGridBrowserContextAddChildCategory(QBContentCategory parentCategory, const char* name, const char* noResultsMessage)
{
    SvString categoryName = SvStringCreate(name, NULL);
    QBContentCategory retCategory = QBContentCategoryCreate(categoryName, categoryName, parentCategory, NULL, NULL);
    QBContentCategoryAddStaticItem(parentCategory, (SvObject) retCategory);
    SVRELEASE(categoryName);
    if (noResultsMessage) {
        SvString noresultsString = SvStringCreate(noResultsMessage, NULL);
        QBContentCategorySetAttribute(retCategory, SVSTRING("noResultsMessage"), (SvObject) noresultsString);
        SVRELEASE(noresultsString);
    }
    return retCategory;
}

/**
 * create a child context of a given type, and start all providers associated with that type
 **/
SvLocal QBWindowContext
QBVoDGridBrowserContextCreateChildContext(QBVoDGridBrowserContext ctx, QBVodGridBrowserContextType type)
{
    QBWindowContext childCtx = NULL;
    QBVoDSubtree subTree = QBVoDSubtreesGetSubtree(ctx->subtreesContainer, type);
    if (!subTree) {
        log_error("cannot create child context - there is no data to display in it");
        return NULL;
    }

    QBContentTree childTree = SVRETAIN((QBContentTree) QBVodSubtreeGetTree(subTree));
    //if the rootPath is specified use it, and if not use NULL to connect the grid browser to the root of the tree
    SvObject path = SVTESTRETAIN(QBVodSubtreeGetData(subTree, SVSTRING("rootPath")));
    size_t providersCount = QBVodSubtreeGetProvidersCount(subTree);
    for (size_t i = 0; i < providersCount; ++i) {
        QBContentProvider childProvider = (QBContentProvider) QBVodSubtreeGetProvider(subTree, i);
        QBContentProviderStart(childProvider, ctx->appGlobals->scheduler);
    }
    if (providersCount > 0) {
        QBContentProvider mainProvider = (QBContentProvider) QBVodSubtreeGetProvider(subTree, 0);
        childCtx = QBVoDGridBrowserContextCreateImpl(ctx->appGlobals, path,
                                                     mainProvider, ctx->serviceId,
                                                     ctx->serviceName, (SvObject) childTree,
                                                     ctx->unlockedGridItems,
                                                     ctx->settings, false, ctx->subtreesContainer,
                                                     type);
        if (childCtx) {
            log_debug("created child context of type %d", type);
        }
    }
    SVTESTRELEASE(path);
    SVTESTRELEASE(childTree);
    return childCtx;
}

/**
 * create a child category and a provider for fetching and presenting "vod most popular"
 **/
SvLocal SvObject
QBVoDGridBrowserContextCreateVodMostPopularProvider(QBVoDGridBrowserContext ctx, QBContentTree childTree)
{
    QBContentCategory rootCategory = QBContentTreeGetRootCategory(childTree);
    QBContentCategory recommendedCategory = QBVoDGridBrowserContextAddChildCategory(rootCategory, gettext("What People are Watching"), NULL);
    SvObject recommendedProvider = QBVoDGridBrowserContextAddProviderToCategory(ctx, recommendedCategory, "vodMostPopular", NULL);
    SVRELEASE(recommendedCategory);
    return recommendedProvider;
}

/**
 * create a tree of data and appropriate providers for presenting search results and
 * "vod most popular" below the list contents
 **/
SvLocal void
QBVoDGridBrowserContextCreateSearchSubtree(QBVoDGridBrowserContext ctx)
{
    log_debug("creating search subtree");
    //Create tree for search data
    QBContentTree childTree = QBVoDGridBrowserContextCreateChildTree("ROOT:search");
    QBContentCategory rootCategory = QBContentTreeGetRootCategory(childTree);

    //Create search category
    SvString searchName = SvStringCreate(gettext("Search"), NULL);
    QBContentSearch searchCategory = QBContentSearchCreate(searchName, rootCategory, NULL, NULL);
    QBContentCategorySetLoadingState((QBContentCategory) searchCategory, QBContentCategoryLoadingState_unknown, NULL);
    QBContentCategoryAddStaticItem(rootCategory, (SvObject) searchCategory);
    SvString noresultsString = SvStringCreate(gettext("No results.\nPlease enter new search criteria"), NULL);
    QBContentCategorySetAttribute((QBContentCategory) searchCategory, SVSTRING("noResultsMessage"), (SvObject) noresultsString);
    SVRELEASE(noresultsString);
    SVRELEASE(searchName);

    SvObject searchProvider = QBVoDGridBrowserContextAddProviderToCategory(ctx, (QBContentCategory) searchCategory, "search", NULL);
    if (!searchProvider) {
        log_error("contentManager cannot create search provider");
        goto out;
    }


    QBVoDSubtree subTree = QBVoDSubtreeCreate((SvObject) childTree);
    QBVoDSubtreeAddProvider(subTree, searchProvider);
    SVRELEASE(searchProvider);

    SvObject vodMostPopularProvider = QBVoDGridBrowserContextCreateVodMostPopularProvider(ctx, childTree);
    if (vodMostPopularProvider) {
        log_debug("adding vod most popular row to search subtree");
        QBVoDSubtreeAddProvider(subTree, vodMostPopularProvider);
        SVRELEASE(vodMostPopularProvider);
    }

    QBVodSubtreeSetData(subTree, SVSTRING("searchCategory"), (SvObject) searchCategory);
    QBVoDSubtreesSetSubtree(ctx->subtreesContainer, QBVodGridBrowserContextType_search, subTree);
    SVRELEASE(subTree);
    log_state("search subtree created");
out:
    SVRELEASE(childTree);
    SVRELEASE(searchCategory);
}

/**
 * create a tree of data and appropriate providers for presenting list contents and
 * "vod most popular" below the list contents
 **/
SvLocal void
QBVoDGridBrowserContextCreateListSubtree(QBVoDGridBrowserContext ctx, QBVodGridBrowserContextType typeOfContext)
{
    const char* providerType = NULL;
    const char* categoryName = NULL;
    const char* emptyMessage = NULL;

    switch (typeOfContext) {
        case QBVodGridBrowserContextType_watched:
            providerType = "watched";
            categoryName = gettext("Watched");
            emptyMessage = gettext("You haven't watched anything yet.\nPlease see what content other users are watching.");
            break;
        case QBVodGridBrowserContextType_myList:
            providerType = "myList";
            categoryName = gettext("My List");
            emptyMessage = gettext("You don’t have any content in your List yet.\nPlease see what content other users are watching.");
            break;
        default:
            break;
    }
    log_debug("creating %s list subtree", providerType);

    if (providerType == NULL) {
        log_error("no provider type for context of type %d", typeOfContext);
        return;
    }

    QBContentTree childTree = QBVoDGridBrowserContextCreateChildTree("ROOT:list");
    QBContentCategory rootCategory = QBContentTreeGetRootCategory(childTree);
    QBContentCategory listCategory = QBVoDGridBrowserContextAddChildCategory(rootCategory, categoryName, emptyMessage);


    SvObject listProvider = QBVoDGridBrowserContextAddProviderToCategory(ctx, listCategory, providerType, NULL);
    SVRELEASE(listCategory);

    if (!listProvider) {
        log_error("Can't create list provider");
        goto out;
    }

    QBVoDSubtree subTree = QBVoDSubtreeCreate((SvObject) childTree);
    QBVoDSubtreeAddProvider(subTree, listProvider);
    SVRELEASE(listProvider);

    SvObject vodMostPopularProvider = QBVoDGridBrowserContextCreateVodMostPopularProvider(ctx, childTree);
    if (vodMostPopularProvider) {
        log_debug("adding vod most popular row to %s list subtree", providerType);
        QBVoDSubtreeAddProvider(subTree, vodMostPopularProvider);
        SVRELEASE(vodMostPopularProvider);
    }

    QBVoDSubtreesSetSubtree(ctx->subtreesContainer, typeOfContext, subTree);
    SVRELEASE(subTree);
    log_state("%s list subtree created", providerType);
out:
    SVRELEASE(childTree);
}

/**
 * create a tree of data and appropriate provider for presenting recommendations
 **/
SvLocal void
QBVoDGridBrowserContextCreateRecommendedSubtree(QBVoDGridBrowserContext ctx)
{
    //Create tree for data
    log_debug("creating recommended subtree");
    QBContentTree childTree = QBVoDGridBrowserContextCreateChildTree("ROOT:recommended");
    QBContentCategory rootCategory = QBContentTreeGetRootCategory(childTree);

    SvObject recommendedProvider = QBVoDGridBrowserContextAddProviderToCategory(ctx, rootCategory, "recommended", NULL);

    if (!recommendedProvider) {
        log_error("Can't create recommended provider");
        goto out;
    }

    QBVoDSubtree subTree = QBVoDSubtreeCreate((SvObject) childTree);
    QBVoDSubtreeAddProvider(subTree, recommendedProvider);
    SVRELEASE(recommendedProvider);
    QBVoDSubtreesSetSubtree(ctx->subtreesContainer, QBVodGridBrowserContextType_recommended, subTree);
    SVRELEASE(subTree);
    log_state("recommended subtree created");
out:
    SVRELEASE(childTree);
}

/**
 * show a new QBVoDGridBrowserContext with search window
 **/
SvLocal void
QBVoDGridBrowserContextShowSearchWindow(QBVoDGridBrowserContext ctx)
{
    log_debug("showing search window");
    QBVoDGridBrowserContextStopProviders(ctx);
    QBVoDSubtree subTree = QBVoDSubtreesGetSubtree(ctx->subtreesContainer, QBVodGridBrowserContextType_search);
    if (unlikely(!subTree)) {
        QBVoDGridBrowserContextCreateSearchSubtree(ctx);
        subTree = QBVoDSubtreesGetSubtree(ctx->subtreesContainer, QBVodGridBrowserContextType_search);
        if (!subTree) {
            log_error("Can't create search subtree - unable to perform search");
            return;
        }
    } else {
        log_state("search subtree already exist");
    }
    QBWindowContext childCtx = QBVoDGridBrowserContextCreateChildContext(ctx, QBVodGridBrowserContextType_search);
    if (childCtx) {
        QBVoDGridBrowserContext child = (QBVoDGridBrowserContext) childCtx;
        child->contentSearch = SVTESTRETAIN((QBContentSearch) QBVodSubtreeGetData(subTree, SVSTRING("searchCategory")));
        child->focusKeyboard = true;

        QBApplicationControllerPushContext(ctx->appGlobals->controller, childCtx);
        SVRELEASE(childCtx);
        log_state("showed search window");
    }
}

/**
 * show a new QBVoDGridBrowserContext with a my list or watched list window (depends on typeOfContext)
 **/
SvLocal void
QBVoDGridBrowserContextShowListWindow(QBVoDGridBrowserContext ctx, QBVodGridBrowserContextType typeOfContext)
{
    log_debug("showing list window of type %d", typeOfContext);
    QBVoDGridBrowserContextStopProviders(ctx);
    QBVoDSubtree subTree = QBVoDSubtreesGetSubtree(ctx->subtreesContainer, typeOfContext);
    if (unlikely(!subTree)) {
        QBVoDGridBrowserContextCreateListSubtree(ctx, typeOfContext);
        subTree = QBVoDSubtreesGetSubtree(ctx->subtreesContainer, typeOfContext);
        if (!subTree) {
            log_error("Can't create list subtree - unable to display list contents");
            return;
        }
    } else {
        log_state("list %d subtree already exist", typeOfContext);
    }
    QBWindowContext childCtx = QBVoDGridBrowserContextCreateChildContext(ctx, typeOfContext);
    if (childCtx) {
        QBApplicationControllerPushContext(ctx->appGlobals->controller, childCtx);
        SVRELEASE(childCtx);
        log_state("showed list window of type %d", typeOfContext);
    }
}

/**
 * show a new QBVoDGridBrowserContext with recommendations
 **/
SvLocal void
QBVoDGridBrowserContextShowRecommendedWindow(QBVoDGridBrowserContext ctx)
{
    log_debug("showing recommended window");
    QBVoDGridBrowserContextStopProviders(ctx);
    QBVoDSubtree subTree = QBVoDSubtreesGetSubtree(ctx->subtreesContainer, QBVodGridBrowserContextType_recommended);
    if (unlikely(!subTree)) {
        QBVoDGridBrowserContextCreateRecommendedSubtree(ctx);
        subTree = QBVoDSubtreesGetSubtree(ctx->subtreesContainer, QBVodGridBrowserContextType_recommended);
        if (!subTree) {
            log_error("Can't create recommended subtree - unable display recommendations");
            return;
        }
    } else {
        log_state("recommended subtree already exist");
    }
    QBWindowContext childCtx = QBVoDGridBrowserContextCreateChildContext(ctx, QBVodGridBrowserContextType_recommended);
    if (childCtx) {
        QBApplicationControllerPushContext(ctx->appGlobals->controller, childCtx);
        SVRELEASE(childCtx);
        log_state("showed recommended window");
    }
}

/**
 * show a new QBVoDGridBrowserContext with the default window (top level categories or content of the category)
 **/
SvLocal void
QBVoDGridBrowserContextShowDefaultWindow(QBVoDGridBrowserContext ctx)
{
    log_debug("showing default VOD window");
    QBVoDGridBrowserContextStopProviders(ctx);
    QBWindowContext childCtx = QBVoDGridBrowserContextCreateChildContext(ctx, QBVodGridBrowserContextType_default);
    if (childCtx) {
        QBApplicationControllerPushContext(ctx->appGlobals->controller, childCtx);
        SVRELEASE(childCtx);
        log_state("showed default VOD window");
    }
}

SvLocal void
QBVoDGridBrowserSetupProgressDialog(QBVoDGridBrowser self)
{
    SvErrorInfo error = NULL;
    svSettingsPushComponent("VoDGrid.settings");
    self->searchProgressDialog = QBSearchProgressDialogCreate(self->ctx->appGlobals->res, self->ctx->appGlobals->controller, &error);
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
    self->searchProgressDialogVisible = true; // AMERELES Fix en la navegación del menú de la derecha
    return;
err:
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
}

SvLocal void
QBVoDGridBrowserOSKKeyTyped(void *self_,
                            QBOSKPane pane,
                            SvString input,
                            unsigned int layout,
                            QBOSKKey key)
{
    QBVoDGridBrowser self = self_;
    AppGlobals appGlobals = self->ctx->appGlobals;

    if (input && SvStringLength(input) > 0) {
        if (key->type == QBOSKKeyType_enter) {
            const char* begin = SvStringCString(input);
            const char* end = begin + SvStringLength(input) - 1;
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

            QBContentSearchStartNewSearch(self->ctx->contentSearch, keyword, appGlobals->scheduler);

            SVRELEASE(keyword);

            QBContextMenuHide(self->contextMenu, false);
            self->ctx->isSearchStarted = true;
            if (!self->ctx->settings->skipSearchDialog) {
                QBVoDGridBrowserSetupProgressDialog(self);
            }
        }
    }
}

SvLocal int
QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(QBVoDGridBrowser self)
{
    int pos = 0;
    if (self->searchRowIdx >= 0)
    {
        //pos = self->searchRowIdx + 1;
        pos = self->searchRowIdx + 1 + self->searchRowIdxOffset; // AMERELES MyList y Hide FakeCategory
    }

    return pos;
}

SvLocal void
QBVoDGridBrowserSearchMenuShow(QBVoDGridBrowser self)
{
    if (!self->ctx->contentSearch) {
        QBVoDGridBrowserSelectMovies(self, false, true);
        return;
    }

    QBOSKPane oskPane = (QBOSKPane)
        SvTypeAllocateInstance(QBOSKPane_getType(), NULL);

    svSettingsPushComponent("OSKPane.settings");
    SvErrorInfo error = NULL;
    QBOSKPaneInit(oskPane, self->ctx->appGlobals->scheduler,
                  self->contextMenu, 1, SVSTRING("OSKPane"),
                  QBVoDGridBrowserOSKKeyTyped, self, &error);
    svSettingsPopComponent();
    if (!error) {
        SvString keyword = QBContentSearchGetKeyword(self->ctx->contentSearch);
        if (keyword)
            QBOSKPaneSetRawInput(oskPane, SvStringCString(keyword));

        QBContextMenuPushPane(self->contextMenu, (SvObject) oskPane);
        QBContextMenuShow(self->contextMenu);
    } else {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVRELEASE(oskPane);
}

SvLocal void
QBVoDGridBrowserNavigationBarButtonFocusEventHandler(SvWidget w, SvFocusEvent e)
{
    QBVodGridBrowserButton *buttonComponents = w->prv;
    switch (e->kind) {
        case SvFocusEventKind_GET:
            svWidgetSetHidden(buttonComponents->focus, false);
            break;
        case SvFocusEventKind_LOST:
            svWidgetSetHidden(buttonComponents->focus, true);
            break;
        default:
            SvLogWarning("%s() : unexpected focus event kind [%d]",
                         __func__, e->kind);
            return;
    }
}

SvLocal void
QBVoDGridBrowserNavigationBarButtonComponentsCreate(SvApplication app, QBTextRenderer textRenderer, SvWidget w, const char* text, bool canBeDisabled, bool disabled)
{
    char *buf;
    QBVodGridBrowserButton* buttonComponents = (QBVodGridBrowserButton*)malloc(sizeof(QBVodGridBrowserButton));
    w->prv = buttonComponents;
    svWidgetSetFocusEventHandler(w, QBVoDGridBrowserNavigationBarButtonFocusEventHandler);
    svWidgetSetFocusable(w, true);

    if (!canBeDisabled || (canBeDisabled && !disabled))
        asprintf(&buf, "%s.Caption", svWidgetGetName(w));
    else
        asprintf(&buf, "%s.CaptionDisabled", svWidgetGetName(w));
    buttonComponents->caption = QBAsyncLabelNew(app, buf, textRenderer);
    if (buttonComponents->caption) {
        QBAsyncLabelSetCText(buttonComponents->caption, text);
        svSettingsWidgetAttach(w, buttonComponents->caption, buf, 3);
    }
    free(buf);

    asprintf(&buf, "%s.Active", svWidgetGetName(w));
    if (svSettingsIsWidgetDefined(buf)) {
        buttonComponents->active = QBFrameCreateFromSM(app, buf);
        if (buttonComponents->active)
            svSettingsWidgetAttach(w, buttonComponents->active, buf, 2);
    } else {
        buttonComponents->active = NULL;
    }
    free(buf);

    asprintf(&buf, "%s.Focus", svWidgetGetName(w));
    buttonComponents->focus = QBFrameCreateFromSM(app, buf);
    if (buttonComponents->focus)
        svSettingsWidgetAttach(w, buttonComponents->focus, buf, 1);
    free(buf);
    svWidgetSetHidden(buttonComponents->focus, true);

    asprintf(&buf, "%s.Icon", svWidgetGetName(w));
    if (svSettingsIsWidgetDefined(buf))
    {
        buttonComponents->icon = svSettingsWidgetCreate(app, buf);
        if (buttonComponents->icon)
            svSettingsWidgetAttach(w, buttonComponents->icon, buf, 1);
    }
    free(buf);
}

SvLocal void
QBVoDGridBrowserNavigationBarUpdateHighlight(QBVodGridBrowserNavigationBar navigationBar)
{
    bool searchInNewTab = navigationBar->info->ctx->settings->searchInNewTab;
    QBVodGridBrowserContextType currentContextType = navigationBar->info->ctx->contextType;
    QBVodGridBrowserMode currentMode = navigationBar->info->mode;
    QBVodGridBrowserButton *buttonComponents = NULL;
    if (navigationBar->buttonMovies) {
        buttonComponents = navigationBar->buttonMovies->prv;
        if (buttonComponents->active) {
            if (!searchInNewTab) {
                svWidgetSetHidden(buttonComponents->active, currentMode != QBVoDGridBrowserMode_movies);
            } else {
                svWidgetSetHidden(buttonComponents->active, currentContextType != QBVodGridBrowserContextType_default);
            }
        }
    }

    if (navigationBar->buttonSearch) {
        buttonComponents = navigationBar->buttonSearch->prv;
        if (buttonComponents->active) {
            if (!searchInNewTab) {
                svWidgetSetHidden(buttonComponents->active, currentMode != QBVoDGridBrowserMode_search);
            } else {
                svWidgetSetHidden(buttonComponents->active, currentContextType != QBVodGridBrowserContextType_search);
            }
        }
    }

    if (navigationBar->buttonMyList) {
        buttonComponents = navigationBar->buttonMyList->prv;
        if (buttonComponents->active) {
            svWidgetSetHidden(buttonComponents->active, currentContextType != QBVodGridBrowserContextType_myList);
        }
    }

    if (navigationBar->buttonWatched) {
        buttonComponents = navigationBar->buttonWatched->prv;
        if (buttonComponents->active) {
            svWidgetSetHidden(buttonComponents->active, currentContextType != QBVodGridBrowserContextType_watched);
        }
    }

    if (navigationBar->buttonRecommended) {
        buttonComponents = navigationBar->buttonRecommended->prv;
        if (buttonComponents->active) {
            svWidgetSetHidden(buttonComponents->active, currentContextType != QBVodGridBrowserContextType_recommended);
        }
    }
}

SvLocal void
QBVoDGridBrowserMenuSetFocus(QBVoDGridBrowser self)
{
    svWidgetSetFocus(self->grid);
}

SvLocal void
QBVoDGridBrowserContextResetSavedPosition(QBVoDGridBrowserContext self)
{
    self->lastMenuPos = -1;
    self->lastMenuItemPos = -1;
}

SvLocal void
QBVoDGridBrowserSelectMovies(QBVoDGridBrowser self, bool immediately, bool navigateToLastMenuItem)
{
    QBVodGridBrowserNavigationBar navigationBar = self->navigationBar->prv;

    int firstNonspecialCategoryIdx = QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(self);

    if (navigationBar->info->ctx->settings->searchInNewTab) {
        self->mode = QBVoDGridBrowserMode_movies;
    }

    QBVoDGridBrowserNavigationBarUpdateHighlight(navigationBar);

    QBVoDGridBrowserMenuSetFocus(self);

    // go to last selected category only if it is not of a special type
    if (!navigateToLastMenuItem || self->ctx->lastMenuPos < 0 || self->ctx->lastMenuPos == self->searchRowIdx) {
        QBVoDGridSetPosition(self->grid, firstNonspecialCategoryIdx, self->ctx->lastMenuItemPos, immediately);
    } else if (self->ctx->lastMenuPos >= 0 && self->ctx->lastMenuItemPos >= 0) {
        QBVoDGridSetPosition(self->grid, self->ctx->lastMenuPos, self->ctx->lastMenuItemPos, true);
    }
}

SvLocal void QBVoDGridBrowserFocusNavigation(QBVodGridBrowserNavigationBar navigationBar, SvWidget specificButton)
{
    if (specificButton) {
        for (ssize_t i = 0; i < MAX_NUM_BUTTONS && navigationBar->focusQueue[i] != NULL; ++i) {
            if (navigationBar->focusQueue[i] == specificButton) {
                navigationBar->info->ctx->settings->defaultButtonIndex = i;
                navigationBar->focusQueueIndex = i;
            }
        }
    }
    ssize_t focusQueueIndex = navigationBar->info->ctx->settings->defaultButtonIndex;
    if (focusQueueIndex >= 0 && navigationBar->focusQueue[focusQueueIndex] != NULL) {
        log_debug("focus button %zu", focusQueueIndex);
        svWidgetSetFocus(navigationBar->focusQueue[focusQueueIndex]);
    } else if (navigationBar->focusQueue[0] != NULL) {
        log_error("focus button 0 - index out of range");
        svWidgetSetFocus(navigationBar->focusQueue[0]);
        navigationBar->info->ctx->settings->defaultButtonIndex = 0;
    }
}

SvLocal bool QBVoDGridBrowserFocusNavigationNext(QBVodGridBrowserNavigationBar navigationBar)
{
    ssize_t focusQueueIndex = navigationBar->info->ctx->settings->defaultButtonIndex;
    log_debug("focus next button. cur focus index %zu", focusQueueIndex);
    if (focusQueueIndex + 1 < MAX_NUM_BUTTONS && navigationBar->focusQueue[focusQueueIndex + 1]) {
        focusQueueIndex += 1;
        navigationBar->info->ctx->settings->defaultButtonIndex = focusQueueIndex;
        svWidgetSetFocus(navigationBar->focusQueue[focusQueueIndex]);
        return true;
    }
    return false;
}

SvLocal bool QBVoDGridBrowserFocusNavigationPrev(QBVodGridBrowserNavigationBar navigationBar)
{
    log_debug("focus prev button. cur focus index %zu", navigationBar->info->ctx->settings->defaultButtonIndex);
    if (navigationBar->info->ctx->settings->defaultButtonIndex > 0) {
        navigationBar->info->ctx->settings->defaultButtonIndex -= 1;
        svWidgetSetFocus(navigationBar->focusQueue[navigationBar->info->ctx->settings->defaultButtonIndex]);
        return true;
    }
    return false;
}


SvLocal void
QBVoDGridBrowserSelectSearch(QBVoDGridBrowser self)
{
    QBVodGridBrowserNavigationBar navigationBar = self->navigationBar->prv;

    self->mode = QBVoDGridBrowserMode_search;
    self->ctx->contextType = QBVodGridBrowserContextType_default; // AMERELES MyList y Hide FakeCategory

    QBVoDGridBrowserNavigationBarUpdateHighlight(navigationBar);
    QBVoDGridBrowserFocusNavigation(navigationBar, navigationBar->buttonSearch);

    QBVoDGridBrowserSearchMenuShow(self);
}

SvLocal void
QBVoDGridBrowserApplyHighlightedProduct(QBVoDGridBrowser self)
{
    SvValue val = NULL;

    val = (SvValue) (self->highlightedProduct ? SvDBRawObjectGetAttrValue(self->highlightedProduct, "name") : 0);
    if (self->highlightedProduct && !val) {
        val = (SvValue) SvDBRawObjectGetAttrValue(self->highlightedProduct, "title");
    }
    if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType())
        && SvValueIsString(val)) {
        QBAsyncLabelSetText(self->movieDetails.title, SvValueGetString(val));
    } else {
        QBAsyncLabelSetText(self->movieDetails.title, NULL);
    }

    svSettingsRestoreContext(self->settingsContext);
    val = (SvValue) (self->highlightedProduct ? SvDBRawObjectGetAttrValue(self->highlightedProduct, "description") : 0);
    if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType())
        && SvValueIsString(val)) {
        QBAsyncLabelSetText(self->movieDetails.description, SvValueGetString(val));
        self->movieDetails.info->off_y = svSettingsGetInteger(svWidgetGetName(self->movieDetails.info), "yOffset", 0);
    } else {
        QBAsyncLabelSetText(self->movieDetails.description, NULL);
        self->movieDetails.info->off_y = self->movieDetails.description->off_y;
    }

    svWidgetSetHidden(self->movieDetails.info, !self->highlightedProduct);
    QBMovieInfoSetObject(self->movieDetails.info, (SvObject) self->highlightedProduct);
    svSettingsPopComponent();

    svWidgetSetHidden(self->details, !(self->highlightedProduct && !self->isCurrentItemLocked));

    // AMERELES BEGIN #2316 Categorización del título VoD
    bool isRental = SvStringEqualToCString(getPackageOfferType(self->highlightedProduct), "rental");
    BoldWidgetHide(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.CategoriaIcon"), isRental);
    BoldWidgetHide(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Categoria"), isRental);
    BoldWidgetHide(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.PriceIcon"), !isRental);
    BoldWidgetHide(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.PricePromoIcon"), !isRental);
    BoldWidgetHide(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Price"), !isRental);
    
    svSettingsPushComponent("VoDGrid.settings");
    if (isRental)
    {
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.CategoriaIcon"), SvStringCreate("MovieDetails.InfoRentals.CategoriaIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Categoria"), SvStringCreate("MovieDetails.InfoRentals.Categoria", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.PriceIcon"), SvStringCreate("MovieDetails.InfoRentals.PriceIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.PricePromoIcon"), SvStringCreate("MovieDetails.InfoRentals.PricePromoIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Price"), SvStringCreate("MovieDetails.InfoRentals.Price", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.GenreIcon"), SvStringCreate("MovieDetails.InfoRentals.GenreIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Genre"), SvStringCreate("MovieDetails.InfoRentals.Genre", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.CountryIcon"), SvStringCreate("MovieDetails.InfoRentals.CountryIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Country"), SvStringCreate("MovieDetails.InfoRentals.Country", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.DirectorIcon"), SvStringCreate("MovieDetails.InfoRentals.DirectorIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Director"), SvStringCreate("MovieDetails.InfoRentals.Director", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.CastIcon"), SvStringCreate("MovieDetails.InfoRentals.CastIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Cast"), SvStringCreate("MovieDetails.InfoRentals.Cast", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.RatingsIcon"), SvStringCreate("MovieDetails.InfoRentals.RatingsIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Ratings"), SvStringCreate("MovieDetails.InfoRentals.Ratings", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.RuntimeIcon"), SvStringCreate("MovieDetails.InfoRentals.RuntimeIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Runtime"), SvStringCreate("MovieDetails.InfoRentals.Runtime", NULL));
    }
    else
    {
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.CategoriaIcon"), SvStringCreate("MovieDetails.Info.CategoriaIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Categoria"), SvStringCreate("MovieDetails.Info.Categoria", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.PriceIcon"), SvStringCreate("MovieDetails.Info.PriceIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.PricePromoIcon"), SvStringCreate("MovieDetails.Info.PricePromoIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Price"), SvStringCreate("MovieDetails.Info.Price", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.GenreIcon"), SvStringCreate("MovieDetails.Info.GenreIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Genre"), SvStringCreate("MovieDetails.Info.Genre", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.CountryIcon"), SvStringCreate("MovieDetails.Info.CountryIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Country"), SvStringCreate("MovieDetails.Info.Country", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.DirectorIcon"), SvStringCreate("MovieDetails.Info.DirectorIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Director"), SvStringCreate("MovieDetails.Info.Director", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.CastIcon"), SvStringCreate("MovieDetails.Info.CastIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Cast"), SvStringCreate("MovieDetails.Info.Cast", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.RatingsIcon"), SvStringCreate("MovieDetails.Info.RatingsIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Ratings"), SvStringCreate("MovieDetails.Info.Ratings", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.RuntimeIcon"), SvStringCreate("MovieDetails.Info.RuntimeIcon", NULL));
        BoldWidgetMove(getSvWidgetChild(self->movieDetails.info, "MovieDetails.Info.Runtime"), SvStringCreate("MovieDetails.Info.Runtime", NULL));
    }
    svSettingsPopComponent();
    // AMERELES END #2316 Categorización del título VoD

    
}

SvLocal void
QBVoDGridBrowserHideMovieDetails(QBVoDGridBrowser self)
{
    if (self->timer)
        svAppTimerStop(self->ctx->super_.window->app, self->timer);
    self->timer = 0;

    svWidgetSetHidden(self->details, true);
}

SvLocal void
QBVoDGridBrowserShowMovieDetails(QBVoDGridBrowser self)
{
    if (self->timer)
        svAppTimerStop(self->ctx->super_.window->app, self->timer);
    self->timer = 0;

    if (self->highlightedProduct && svWidgetIsFocused(self->grid))
        self->timer = svAppTimerStart(self->ctx->super_.window->app, self->ctx->super_.window, self->showDetailsDelay, true);
}

SvLocal void
QBVoDGridBrowserUpdateMenuPosition(QBVoDGridBrowser self)
{
    int selectedRowIdx;
    QBVoDGridGetPosition(self->grid, &selectedRowIdx, NULL);
    // sync grid position with current browse mode
    //if ((self->searchRowIdx >= 0 && selectedRowIdx == self->searchRowIdx) && !self->searchResultsAvailable) {
    // AMERELES MyList y Hide FakeCategory
    if ((self->searchRowIdx >= 0 && selectedRowIdx == self->searchRowIdx) && !self->searchResultsAvailable && self->ctx->contextType != QBVodGridBrowserContextType_myList) {
        QBVoDGridBrowserSelectMovies(self, false, true);
    }

    QBVoDGridBrowserNavigationBarUpdateHighlight(self->navigationBar->prv);
}

SvLocal void
QBVoDGridBrowserShowMovieDetailsWindow(QBVoDGridBrowser self)
{
    QBVoDGridGetPosition(self->grid, &self->ctx->lastMenuPos, &self->ctx->lastMenuItemPos);
    self->ctx->lastMode = self->mode;
    QBWindowContext ctx = NULL;
    if (self->ctx->settings->detailsScreen == QBVoDGridBrowserDetailsScreen_Default) {
        // BEGIN AMERELES #2131 tracking de navegación VOD en STBs híbridos
        SvString categoryName = SvStringCreate("", NULL);
        if (self->categoryOfHighlightedProduct)
        {
            categoryName = QBContentCategoryGetName(self->categoryOfHighlightedProduct);
        }
        
        ctx = QBVoDMovieDetailsContextCreate(self->ctx->appGlobals, self->categoryOfHighlightedProduct, self->highlightedProduct, (SvObject) self->ctx->provider, self->ctx->serviceId, self->ctx->serviceName, (SvObject) self->ctx->tree, categoryName, SVSTRING(""));
        // END AMERELES #2131 tracking de navegación VOD en STBs híbridos
        //ctx = QBVoDMovieDetailsContextCreate(self->ctx->appGlobals, self->categoryOfHighlightedProduct, self->highlightedProduct, (SvObject) self->ctx->provider, self->ctx->serviceId, self->ctx->serviceName, (SvObject) self->ctx->tree);
    } else if (self->ctx->settings->detailsScreen == QBVoDGridBrowserDetailsScreen_ColumbusDS) {
        ctx = QBVoDMovieDetailsDSContextCreate(self->ctx->appGlobals, self->highlightedProduct, self->ctx->provider, self->ctx->serviceId, self->ctx->serviceName, (SvObject) self->ctx->tree);
    }

    if (ctx) {
        QBApplicationControllerPushContext(self->ctx->appGlobals->controller, ctx);
        SVRELEASE(ctx);
    }
}

SvLocal SvObject
QBVoDGridBrowserCreateContentInfo(QBVoDGridBrowser self)
{
    QBVoDGridBrowserContext ctx = self->ctx;

    if (ctx->settings->refreshObject) {
        SvObject cubiId = SvDBRawObjectGetAttrValue(self->highlightedProduct, "cubiware.id");
        if (cubiId && SvObjectIsInstanceOf(cubiId, SvValue_getType())) {
            SvDBRawObjectSetDomainId(self->highlightedProduct, SvDBRawObjectDomain_Cubiware, (SvValue) cubiId);
        }
    }
    SvObject contentInfo = QBContentInfoCreate(ctx->appGlobals, ctx->serviceId, ctx->serviceName, (SvObject) self->highlightedProduct, self->categoryOfHighlightedProduct, (SvObject) ctx->provider, ctx->tree);
    return contentInfo;
}

/**
 * this function will create an event reporter for a movie
 * and pass it to the side menu, so that the menu can report
 * events when user performs actions.
 **/
SvLocal void
QBVoDGridBrowserSetEventReporter(QBVoDGridBrowser self, QBContentSideMenu contentSideMenu)
{
    SVTESTRELEASE(self->ctx->eventReporter);
    self->ctx->eventReporter = NULL;
    SvObject contentManager = QBContentProviderGetManager(self->ctx->provider);
    if (contentManager) {
        //get actions for highlighted movie
        SvArray actions = SvInvokeInterface(QBContentManager, contentManager, constructActionsForContent, (SvObject) self->highlightedProduct, NULL);

        if (actions) {
            SvIterator it = SvArrayIterator(actions);
            QBContentAction act;

            while ((act = (QBContentAction) SvIteratorGetNext(&it))) {
                // if there is an action for reporting events
                if (QBContentActionGetType(act, NULL) == QBContentActionType_relatedContent &&
                    SvStringEqualToCString(QBContentActionGetName(act, NULL), "reportEvent")) {
                    // create event reporter
                    SvObject eventReporter = SvInvokeInterface(QBContentManager, contentManager, createActionRequest,
                                                               (SvObject) self->highlightedProduct, act, NULL, NULL);
                    if (eventReporter && SvObjectIsImplementationOf(eventReporter, QBMWClientEventReporter_getInterface())) {
                        // apply the event filter for handling watched event (see QBEventReporterWatchedEventFilter docs for more details)
                        QBEventReporterWatchedEventFilter filter = QBEventReporterWatchedEventFilterCreate(self->ctx->appGlobals->bookmarkManager,
                                                                                   self->highlightedProduct,
                                                                                   eventReporter,
                                                                                   NULL);
                        // set the event reporter in the side menu
                        QBContentSideMenuSetEventReporter(contentSideMenu, (SvObject) filter);
                        self->ctx->eventReporter = SVRETAIN(filter);
                        SVRELEASE(filter);
                    }
                    SVTESTRELEASE(eventReporter);
                }
            }
            SVRELEASE(actions);
        }
    }
}

SvLocal void
QBVoDGridBrowserShowSideMenu(QBVoDGridBrowser self)
{
    QBContentSideMenu contentSideMenu = (QBContentSideMenu) self->contentSideMenu;
    QBContentSideMenuSetImmediateAction(contentSideMenu, QBContentSideMenuImmediateAction_none);

    svSettingsRestoreContext(self->settingsContext);
    if (QBContextMenuIsShown((QBContextMenu) contentSideMenu))
        goto finish;

    SvObject contentInfo = QBVoDGridBrowserCreateContentInfo(self);
    QBVoDGridBrowserSetEventReporter(self, contentSideMenu);
    
    // BEGIN AMERELES #2131 tracking de navegación VOD en STBs híbridos
    SvString categoryName = SvStringCreate("", NULL);
    if (self->categoryOfHighlightedProduct)
    {
        categoryName = QBContentCategoryGetName(self->categoryOfHighlightedProduct);
    }
    
    QBContentSideMenuSetContentInfo(contentSideMenu, contentInfo, categoryName, SVSTRING(""));
    // END AMERELES #2131 tracking de navegación VOD en STBs híbridos
    //QBContentSideMenuSetContentInfo(contentSideMenu, contentInfo);
    QBContextMenuShow((QBContextMenu) self->contentSideMenu);
    SVRELEASE(contentInfo);

finish:
    svSettingsPopComponent();
}

SvLocal void
QBVoDGridBrowserShowCategoryWindow(QBVoDGridBrowser self)
{
    QBContentCategory highlihtedParent = QBContentCategoryGetParent(self->highlightedCategory);
    SvObject foundPath = NULL;
    QBTreeIterator childIter = SvInvokeInterface(QBTreeModel, self->ctx->tree, getIterator, self->ctx->path, 0);
    size_t count = QBTreeIteratorGetNodesCount(&childIter);
    for (size_t i = 0; i < count; ++i) {
        SvObject path = QBTreeIteratorGetCurrentNodePath(&childIter);
        SvObject node = QBTreeIteratorGetNextNode(&childIter);
        foundPath = node == (SvObject) self->highlightedCategory ? path : foundPath;
        if (node == (SvObject) highlihtedParent) {
            childIter = SvInvokeInterface(QBTreeModel, self->ctx->tree, getIterator, path, 0);
            count = QBTreeIteratorGetNodesCount(&childIter);
            i = (size_t) -1;
        }
    }
    if (foundPath) {
        if (self->highlightedCategory) {
            QBVoDUtilsLogMovieCategory(QBContentCategoryGetId(self->highlightedCategory),
                                       QBContentCategoryGetName(self->highlightedCategory),
                                       QBContentCategoryGetLevel(self->highlightedCategory));
        }

        QBVoDGridGetPosition(self->grid, &self->ctx->lastMenuPos, &self->ctx->lastMenuItemPos);
        QBWindowContext ctx = QBVoDGridBrowserContextCreateImpl(self->ctx->appGlobals, foundPath,
                                                                self->ctx->provider, self->ctx->serviceId,
                                                                self->ctx->serviceName, self->ctx->tree,
                                                                self->ctx->unlockedGridItems,
                                                                self->ctx->settings, false, self->ctx->subtreesContainer,
                                                                self->ctx->contextType);
        QBApplicationControllerPushContext(self->ctx->appGlobals->controller, ctx);
        SVRELEASE(ctx);
    }
}

SvLocal void
QBVoDGridBrowserCheckParentalControlPINCallbackMovieDetails(void *ptr, SvWidget dlg,
                                                            SvString ret, unsigned key)
{
    QBVoDGridBrowser self = ptr;

    self->parentalPopup = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        if (self->highlightedProduct) {
            QBVoDGridUnlockCurrentItem(self->grid);
        } else if (self->highlightedCategory) {
            QBVoDGridUnlockSelectedCategory(self->grid);
        }
        self->isCurrentCategoryLocked = false;
        self->isCurrentItemLocked = false;

        QBVoDGridBrowserShowMovieDetailsWindow(self);
    }
}

SvLocal void
QBVoDGridBrowserCheckParentalControlPINCallbackAdultMovieDetails(void *ptr, SvWidget dlg,
                                                                 SvString ret, unsigned key)
{
    QBVoDGridBrowser self = ptr;

    self->parentalPopup = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        if (self->highlightedProduct) {
            QBVoDGridUnlockCurrentItem(self->grid);
            QBVoDGridBrowserShowMovieDetails(self);
        } else if (self->highlightedCategory) {
            QBVoDGridUnlockSelectedCategory(self->grid);
        }
        self->isCurrentCategoryLocked = false;
        self->isCurrentItemLocked = false;
    }
}

SvLocal void
QBVoDGridBrowserCheckParentalControlPINCallbackSideMenu(void *ptr, SvWidget dlg,
                                                        SvString ret, unsigned key)
{
    QBVoDGridBrowser self = ptr;

    self->parentalPopup = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        QBVoDGridUnlockCurrentItem(self->grid);
        self->isCurrentCategoryLocked = false;
        self->isCurrentItemLocked = false;
        QBVoDGridBrowserShowSideMenu(self);
    }
}

SvLocal void
QBVoDGridBrowserCheckParentalControlPIN(QBVoDGridBrowser self, QBVoDGridBrowserParentalControlCheck action)
{
    svSettingsRestoreContext(self->settingsContext);
    AppGlobals appGlobals = self->ctx->appGlobals;
    SvObject authenticator = NULL;
    authenticator = QBAuthenticateViaAccessManager(appGlobals->scheduler,
                                                   appGlobals->accessMgr,
                                                   SVSTRING("PC_VOD"));

    svSettingsPushComponent("ParentalControl.settings");
    SvWidget master = NULL;
    SvWidget dialog = QBAuthDialogCreate(appGlobals, authenticator,
                                         gettext("Authentication required"),
                                         0, true, NULL, &master);
    svSettingsPopComponent();

    self->parentalPopup = dialog;

    switch (action) {
        case QBVoDGridBrowserParentalControlCheck_movieDetailsSideMenu:
            QBDialogRun(dialog, self, QBVoDGridBrowserCheckParentalControlPINCallbackAdultMovieDetails);
            break;
        case QBVoDGridBrowserParentalControlCheck_movieDetailsWindow:
            QBDialogRun(dialog, self, QBVoDGridBrowserCheckParentalControlPINCallbackMovieDetails);
            break;
        case QBVoDGridBrowserParentalControlCheck_optionsSideMenu:
            QBDialogRun(dialog, self, QBVoDGridBrowserCheckParentalControlPINCallbackSideMenu);
            break;
        default:
            break;
    }

    svSettingsPopComponent();
}

SvLocal void
QBVoDGridBrowserHandlePeerEvent(SvObject self_, QBPeerEvent event_, SvObject sender)
{
    QBVoDGridBrowser self = (QBVoDGridBrowser) self_;

    if (!self->window || sender != self->grid->prv)
        return;

    if (SvObjectIsInstanceOf((SvObject) event_, QBVoDGridItemSelectedEvent_getType())) {
        QBVoDGridEvent event = (QBVoDGridEvent) event_;
        self->categoryOfHighlightedProduct = (QBContentCategory) event->category;

        log_debug("event->isLocked: %d  event->isAdult: %d , hihghlightedProduct: %p", event->isLocked, event->isAdult, self->highlightedProduct);
        if (!self->highlightedProduct && !self->highlightedCategory) {
            if (self->ctx->contextType == QBVodGridBrowserContextType_search && self->ctx->contentSearch) {
                QBVoDGridBrowserSearchMenuShow(self);
            }
            return;
        }

        if (event->isLocked) {
            if (event->isAdult) {
                QBVoDGridBrowserCheckParentalControlPIN(self, QBVoDGridBrowserParentalControlCheck_movieDetailsSideMenu);
            } else if (self->highlightedProduct) {
                SvString parentalControl = QBMovieUtilsGetAttr(self->highlightedProduct, "ratings", NULL);
                if (!parentalControl || QBParentalControlRatingAllowed(self->ctx->appGlobals->pc, parentalControl)) {
                    QBVoDGridBrowserShowMovieDetailsWindow(self);
                } else {
                    QBVoDGridBrowserCheckParentalControlPIN(self, QBVoDGridBrowserParentalControlCheck_movieDetailsWindow);
                }
            } else if (self->highlightedCategory) {
                SvString parentalControl = NULL;
                SvValue parentalControlVal = (SvValue) QBContentCategoryGetAttribute(self->highlightedCategory, SVSTRING("ratings"));
                if (parentalControlVal && SvValueIsString(parentalControlVal)) {
                    parentalControl = SvValueGetString(parentalControlVal);
                }

                if (!parentalControl || QBParentalControlRatingAllowed(self->ctx->appGlobals->pc, parentalControl)) {
                    QBVoDGridBrowserShowCategoryWindow(self);
                } else {
                    QBVoDGridBrowserCheckParentalControlPIN(self, QBVoDGridBrowserParentalControlCheck_movieDetailsWindow);
                }
            }
        } else if (self->highlightedProduct) {
            QBVoDGridBrowserShowMovieDetailsWindow(self);
        } else if (self->highlightedCategory) {
            QBVoDGridBrowserShowCategoryWindow(self);
        }
    } else if (SvObjectIsInstanceOf((SvObject) event_, QBVoDGridItemFocusedEvent_getType())) {
        QBVoDGridEvent event = (QBVoDGridEvent) event_;
        self->categoryOfHighlightedProduct = (QBContentCategory) event->category;
        SvDBRawObject newHighlight = SvObjectIsInstanceOf(event->object, SvDBRawObject_getType()) ? (SvDBRawObject) event->object : NULL;
        SVTESTRETAIN(newHighlight);
        SVTESTRELEASE(self->highlightedProduct);
        self->highlightedProduct = NULL;
        log_debug("event->isLocked: %d  event->isAdult: %d ", event->isLocked, event->isAdult);
        self->isCurrentItemLocked = event->isLocked && QBVoDUtilsIsAdult((SvObject) newHighlight);
        self->isCurrentCategoryLocked = event->isLocked;

        QBVoDGridBrowserApplyHighlightedProduct(self);

        self->highlightedProduct = newHighlight;

        if (!self->isCurrentItemLocked)
            QBVoDGridBrowserShowMovieDetails(self);

        QBContentCategory category = SvObjectIsInstanceOf(event->object, QBContentCategory_getType()) ? (QBContentCategory) event->object : NULL;
        SVTESTRETAIN(category);
        SVTESTRELEASE(self->highlightedCategory);
        self->highlightedCategory = category;

        // BEGIN AMERELES MyList y Hide FakeCategory
        int menuPosition;
        QBVoDGridGetPosition(self->grid, &menuPosition, NULL);
        
        int firstNormalMenuItemRowIdx = QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(self);
        if (menuPosition > firstNormalMenuItemRowIdx - 1)
        {
            self->ctx->contextType = QBVodGridBrowserContextType_default; // GOES FROM myList TO default
            QBVoDGridSetScrollRange(self->grid, firstNormalMenuItemRowIdx, -1);
        }
        
        if (menuPosition > firstNormalMenuItemRowIdx)
        {
            // If its beyong the first normal raw, hide the tile
            svWidgetSetHidden(self->tileToHideMyList, true);
        }
        else if (menuPosition == 0)
        {
            // If it is on search row, show tile to hide my_rentals row
            svSettingsRestoreContext(self->settingsContext);
            BoldWidgetSetProperties(self->tileToHideMyList, SVSTRING("VerticalMenu.TileToHideMyListSearch"));
            svSettingsPopComponent();
            svWidgetSetHidden(self->tileToHideMyList, false);
        }
        else
        {
            // If its first normal row or rental row, show tide to hide unused rows
            svSettingsRestoreContext(self->settingsContext);
            BoldWidgetSetProperties(self->tileToHideMyList, SVSTRING("VerticalMenu.TileToHideMyList"));
            svSettingsPopComponent();
            svWidgetSetHidden(self->tileToHideMyList, false);
        }
        
        if (self->mode != QBVoDGridBrowserMode_search && 
            self->ctx->contextType != QBVodGridBrowserContextType_myList &&
            (menuPosition < firstNormalMenuItemRowIdx))
        {
            // If row is not search and not my list and position is less that first normal row, go to normal row
            QBVoDGridSetPosition(self->grid, firstNormalMenuItemRowIdx, -1, true);
        }
        // END AMERELES MyList y Hide FakeCategory
    }
}

SvLocal void
QBVoDGridBrowserTimerEventHandler(SvWidget w, SvTimerEvent e)
{
    QBVoDGridBrowser self = w->prv;


    if (e->id != self->timer)
        return;

    self->timer = 0;

    QBVoDGridBrowserApplyHighlightedProduct(self);
}

SvLocal void
QBVoDGridBrowserHandleOptionKey(QBVoDGridBrowser self)
{
    // If there is no highlighted product it means that we have a category.
    if (!self->highlightedProduct)
        return;

    if (self->isCurrentCategoryLocked) {
        SvString parentalControl = QBMovieUtilsGetAttr(self->highlightedProduct, "ratings", NULL);
        if (!parentalControl || !QBParentalControlRatingAllowed(self->ctx->appGlobals->pc, parentalControl)) {
            QBVoDGridBrowserCheckParentalControlPIN(self, QBVoDGridBrowserParentalControlCheck_optionsSideMenu);
            return;
        } else if (QBVoDUtilsIsAdult((SvObject) self->highlightedProduct)) {
            QBVoDGridBrowserCheckParentalControlPIN(self, QBVoDGridBrowserParentalControlCheck_optionsSideMenu);
            return;
        }
    }
    QBVoDGridBrowserShowSideMenu(self);
}

SvLocal void
QBVoDGridBrowserEnterIntoSearch(QBVodGridBrowserNavigationBar navigationBar)
{
    if (navigationBar->info->ctx->settings->searchInNewTab && navigationBar->info->ctx->contextType != QBVodGridBrowserContextType_search) {
        QBVoDGridBrowserContextShowSearchWindow(navigationBar->info->ctx);
    } else {
        QBVoDGridBrowserSelectSearch(navigationBar->info);
    }
}

SvLocal void
QBVoDGridBrowserShowWatchedWindow(QBVoDGridBrowserContext ctx)
{
    QBVoDGridBrowserContextShowListWindow(ctx,
                                          QBVodGridBrowserContextType_watched);
}

SvLocal void
QBVoDGridBrowserShowMyListWindow(QBVoDGridBrowserContext ctx)
{
    QBVoDGridBrowserContextShowListWindow(ctx,
                                          QBVodGridBrowserContextType_myList);
}

SvLocal bool
QBVoDGridBrowserInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBVoDGridBrowser self = w->prv;
    QBVodGridBrowserNavigationBar navigationBar = self->navigationBar->prv;

    if (self->ctx->settings->enableKeyShortcuts) {
        if (e->ch == QBKEY_VOD) {
            QBVODUtilsPopVodContexts(self->ctx->appGlobals->controller);
            return true;
        }

        if (e->ch == QBKEY_RED) {
            if (self->ctx->contextType != QBVodGridBrowserContextType_watched) {
                QBVoDGridBrowserShowWatchedWindow(self->ctx);
            }
            return true;
        }

        if (e->ch == QBKEY_GREEN) {
            if (self->ctx->contextType != QBVodGridBrowserContextType_myList) {
                QBVoDGridBrowserShowMyListWindow(self->ctx);
            }
            return true;
        }

        if (e->ch == QBKEY_YELLOW) {
            if (self->ctx->contextType != QBVodGridBrowserContextType_recommended) {
                QBVoDGridBrowserContextShowRecommendedWindow(self->ctx);
            }
            return true;
        }

        if (e->ch == QBKEY_INFO) {
            if (self->ctx->contextType != QBVodGridBrowserContextType_search) {
                QBVoDGridBrowserContextShowSearchWindow(self->ctx);
            }
            return true;
        }
    }
    
    // BEGIN AMERELES Fix en la navegación del menú de la derecha
    if (e->ch == QBKEY_BACK)
    {
        if (self->contextMenu && QBContextMenuIsShown(self->contextMenu))
        {
            QBContextMenuHide(self->contextMenu, false);
            return true;
        }
        else if (self->searchProgressDialog  && self->searchProgressDialogVisible)
        {
            svAppPostEvent(self->ctx->appGlobals->res, NULL, svInputEventCreate(QBKEY_ENTER));
            return true;
        }
    }
    // END AMERELES Fix en la navegación del menú de la derecha

    if(!svWidgetIsFocused(self->grid))
        return false;

    int menuPosition;
    int menuLen = SvInvokeInterface(QBListModel, self->ctx->dataSource, getLength);

    QBVoDGridGetPosition(self->grid, &menuPosition, NULL);

    if (self->ctx->settings->enableKeyShortcuts && (e->ch == QBKEY_PLAY || e->ch == QBKEY_PLAYPAUSE)) {
        QBVoDGridBrowserShowSideMenu(self);
        QBContentSideMenu contentSideMenu = (QBContentSideMenu) self->contentSideMenu;
        QBContentSideMenuSetImmediateAction(contentSideMenu, QBContentSideMenuImmediateAction_rentOrPlay);
        return true;
    }

    if (e->ch == QBKEY_VOD) {
        QBApplicationControllerPopContext(self->ctx->appGlobals->controller);
        return true;
    }

    if (e->ch == QBKEY_PGUP) {
        if (menuPosition >= 0 && (menuPosition == QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(self))) {
            QBVoDGridBrowserSelectMovies(self, false, false);
            QBVoDGridSetPosition(self->grid, menuLen - 1, -1, false);
        }
        return true;
    }

    if (e->ch == QBKEY_UP) {
        QBVoDGridBrowserContextResetSavedPosition(self->ctx);

        QBVoDGridBrowserFocusNavigation(navigationBar, NULL);

        return true;
    }

    if (e->ch == QBKEY_FUNCTION) {
        if (!self->ctx->settings->useSideMenu) {
            QBVoDGridGetPosition(self->grid, &self->ctx->lastMenuPos, &self->ctx->lastMenuItemPos);
            QBVoDGridBrowserFocusNavigation(navigationBar, NULL);
        } else {
            QBVoDGridBrowserHandleOptionKey(self);
        }
        return true;
    }

    if (e->ch == QBKEY_SEARCH) {
        QBVoDGridBrowserEnterIntoSearch(navigationBar);

        return true;
    }

    return false;
}

SvLocal void
QBVoDGridBrowserClean(SvApplication app, void *prv_)
{
    QBVoDGridBrowser self = prv_;
    self->window = NULL;
    QBInputServiceRemoveFilter((SvObject) self->ctx->exitInputFilter, NULL);
    SVRELEASE(self);
}

SvLocal void
QBVoDGridBrowserServiceNotificationNoop(SvObject self_, SvString serviceId)
{
}

SvLocal void
QBVoDGridBrowserServiceRemoved(SvObject self_, SvString serviceId)
{
    QBVoDGridBrowser self = (QBVoDGridBrowser) self_;
    if (SvStringEqualToCString(self->ctx->serviceId, SvStringCString(serviceId))) {
        QBApplicationControllerSwitchToRoot(self->ctx->appGlobals->controller);
        QBApplicationControllerPushContext(self->ctx->appGlobals->controller, self->ctx->appGlobals->main);
    }
}

SvLocal void
QBVoDGridBrowserCheckSearchResult(QBVoDGridBrowser self, QBContentCategory category)
{
    self->searchResultsAvailable = false;

    SvErrorInfo error = NULL;
    if (!self->ctx->settings->searchInNewTab && self->searchRowIdx < 0) {
        QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_noResultState, &error);
        if (error) {
            goto err;
        }
        return;
    }

    size_t itemsCnt = QBContentCategoryGetLength(category) - QBContentCategoryGetStaticCount(category);
    if (!self->ctx->settings->searchInNewTab && itemsCnt > 0) {
        self->searchResultsAvailable = true;
        QBVoDGridBrowserMenuSetFocus(self);

        svSettingsRestoreContext(self->settingsContext);
        size_t initialFocusColumn = (size_t) svSettingsGetInteger("VerticalMenu.Item.Carousel", "initialFocusColumn", 4);
        size_t columnToFocus = (itemsCnt > initialFocusColumn) ? initialFocusColumn : itemsCnt;
        svSettingsPopComponent();

        QBVoDGridSetSearchResultAvailability(self->grid, self->searchResultsAvailable);
        QBVoDGridSetPosition(self->grid, self->searchRowIdx, (int) columnToFocus, false);
        QBVoDGridSetScrollRange(self->grid, self->searchRowIdx, -1);
        QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_finishedState, &error);
        if (error) {
            goto err;
        }
        QBSearchProgressDialogHide(self->searchProgressDialog, &error);
        self->searchProgressDialogVisible = false; // AMERELES Fix en la navegación del menú de la derecha
        if (error) {
            goto err;
        }
    } else if (itemsCnt == 0) {
        int firstNormalMenuItemRowIdx = QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(self);
        QBVoDGridSetSearchResultAvailability(self->grid, self->searchResultsAvailable);
        QBVoDGridSetScrollRange(self->grid, firstNormalMenuItemRowIdx, -1);
        QBVoDGridSetPosition(self->grid, firstNormalMenuItemRowIdx, -1, false);

        QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_noResultState, &error);
        if (error) {
            goto err;
        }
    }
    return;
err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
}

SvLocal void
QBVoDGridBrowserMiddlewareDataChanged(SvObject self_,
                                      QBMiddlewareManagerType middlewareType)
{
    QBVoDGridBrowser self = (QBVoDGridBrowser) self_;

    if (middlewareType != QBMiddlewareManagerType_Innov8on)
        QBApplicationControllerSwitchToRoot(self->ctx->appGlobals->controller);
}

SvLocal void
QBVoDGridBrowserCategoryLoadingStateChanged(SvObject self_, QBContentCategory category, QBContentCategoryLoadingState previousState, QBContentCategoryLoadingState currentState)
{
    QBVoDGridBrowser self = (QBVoDGridBrowser) self_;

    if (SvObjectIsInstanceOf((SvObject) category, QBContentSearch_getType())) {
        if (!self->ctx->contentSearch || !self->ctx->isSearchStarted)
            return;
        QBContentCategoryLoadingState loadingState = QBContentCategoryGetLoadingState(category);
        int results = QBContentCategoryGetLength(category) - QBContentCategoryGetStaticCount(category);
        if (loadingState != QBContentCategoryLoadingState_active || results) {
            self->ctx->isSearchStarted = false;
            QBVoDGridBrowserCheckSearchResult(self, category);
            self->ctx->searchContentAvailable = true;
        }
    }
}

SvLocal
void QBVoDGridBrowserAdServiceCurrentAdChanged(SvObject self_)
{
    QBVoDGridBrowser self = (QBVoDGridBrowser) self_;
    SvString settingsFile = SVSTRING("VoDGrid.settings");
    if (self->ctx->settings->settingsFile) {
        settingsFile = self->ctx->settings->settingsFile;
    }
    SvWidget ad = QBAdServiceGetAd(self->ctx->appGlobals->adService, SvStringCString(settingsFile));
    QBAdWindowSetAd(self->adWindow, ad);
    QBAdWindowShow(self->adWindow);
}

SvLocal void
QBVoDGridBrowserOkCallback(SvObject self_, QBSearchProgressDialog searchProgressDialog)
{
    SvErrorInfo error = NULL;
    QBVoDGridBrowser self = (QBVoDGridBrowser) self_;
    QBSearchProgressDialogHide(searchProgressDialog, &error);
    self->searchProgressDialogVisible = false; // AMERELES Fix en la navegación del menú de la derecha
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
    QBVoDGridBrowserSearchMenuShow(self);
}

SvLocal void
QBVoDGridBrowserCancelCallback(SvObject self_, QBSearchProgressDialog searchProgressDialog)
{
    SvErrorInfo error = NULL;
    QBVoDGridBrowser self = (QBVoDGridBrowser) self_;
    QBSearchProgressDialogHide(searchProgressDialog, &error);
    self->searchProgressDialogVisible = false; // AMERELES Fix en la navegación del menú de la derecha
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
    self->ctx->isSearchStarted = false;
    self->ctx->searchContentAvailable = false;
    QBVoDGridBrowserSearchMenuShow(self);
}

SvLocal void
QBVoDGridBrowserDestroy(void *self_)
{
    QBVoDGridBrowser self = (QBVoDGridBrowser) self_;

    assert(!self->window);

    if (self->parentalPopup) {
        QBDialogBreak(self->parentalPopup);
        self->parentalPopup = NULL;
    }

    // AMERELES MyList y Hide FakeCategory
    if (self->noResultsPopup) {
        QBDialogBreak(self->noResultsPopup);
        self->noResultsPopup = NULL;
    }

    if (self->contextMenu) {
        QBContextMenuSetCallbacks(self->contextMenu, NULL, NULL);
        QBContextMenuHide(self->contextMenu, false);
        SVRELEASE(self->contextMenu);
    }

    SVTESTRELEASE(self->highlightedProduct);
    SVTESTRELEASE(self->highlightedCategory);
    SVRELEASE(self->contentSideMenu);
    SVTESTRELEASE(self->searchProgressDialog);
}

SvLocal SvType
QBVoDGridBrowser_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDGridBrowserDestroy
    };
    static const struct QBPeerEventReceiver_ eventReceiverMethods = {
        .handleEvent = QBVoDGridBrowserHandlePeerEvent
    };
    static const struct QBContentCategoryListener_ categoryListenerMethods = {
        .loadingStateChanged = QBVoDGridBrowserCategoryLoadingStateChanged,
    };
    static const struct QBProvidersControllerServiceListener_t provdersControllerServiceMethods = {
        .serviceAdded    = QBVoDGridBrowserServiceNotificationNoop,
        .serviceRemoved  = QBVoDGridBrowserServiceRemoved,
        .serviceModified = QBVoDGridBrowserServiceNotificationNoop
    };
    static const struct QBMiddlewareManagerListener_t middlewareListenerMethods = {
        .middlewareDataChanged = QBVoDGridBrowserMiddlewareDataChanged
    };
    static const struct QBAdServiceListener_t adServiceListenerMethods = {
        .currentAdChanged = QBVoDGridBrowserAdServiceCurrentAdChanged
    };
    static const struct QBSearchProgressDialogListener_ searchProgressDialogListener = {
        .onOkPressed     = QBVoDGridBrowserOkCallback,
        .onCancelPressed = QBVoDGridBrowserCancelCallback
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDGridBrowser",
                            sizeof(struct QBVoDGridBrowser_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            QBContentCategoryListener_getInterface(), &categoryListenerMethods,
                            QBProvidersControllerServiceListener_getInterface(), &provdersControllerServiceMethods,
                            QBMiddlewareManagerListener_getInterface(), &middlewareListenerMethods,
                            QBAdServiceListener_getInterface(), &adServiceListenerMethods,
                            QBSearchProgressDialogListener_getInterface(), &searchProgressDialogListener,
                            NULL);
    }

    return type;
}

SvLocal void
QBVoDGridBrowserSideMenuClose(void *self_, QBContextMenu menu)
{
    QBVoDGridBrowser self = self_;
    SVTESTRELEASE(self->contextMenu);
    self->contextMenu =
        QBContextMenuCreateFromSettings("ContextMenu.settings",
                                        self->ctx->appGlobals->controller,
                                        self->ctx->appGlobals->res,
                                        SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->contextMenu,
                              QBVoDGridBrowserSideMenuClose, self);
}

// BEGIN AMERELES MyList y Hide FakeCategory
SvLocal void
QBVoDGridBrowserMyRentalsNoResultDialogCallback(void *self_, SvWidget dialog, SvString buttonTag,
                                                unsigned keyCode)
{
    QBVoDGridBrowser self = (QBVoDGridBrowser) self_;
    self->noResultsPopup = NULL;
    if(buttonTag && SvStringEqualToCString(buttonTag, "OK-button"))
    {
        int firstNormalMenuItemRowIdx = QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(self);
        QBVoDGridSetScrollRange(self->grid, firstNormalMenuItemRowIdx, -1);
        QBVoDGridSetPosition(self->grid, firstNormalMenuItemRowIdx, -1, false);
    }
}

SvLocal SvWidget
QBVoDGridBrowserCreatePopup(AppGlobals appGlobals, const char *title,
                            const char *message)
{
    svSettingsPushComponent("VoDGrid.settings");
    
    
    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0
    };
    SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    QBDialogAddLabel(dialog, SVSTRING("content"), message, SVSTRING("message"), 0);
    QBDialogSetTitle(dialog, title);
    QBDialogAddButton(dialog, SVSTRING("OK-button"), gettext("OK"), 1);
    svSettingsPopComponent();
    return dialog;
}

SvLocal void
QBVoDMovieDetailsSetupNoResultDialog(QBVoDGridBrowser self)
{
    if (self->noResultsPopup)
        return;

    self->noResultsPopup = QBVoDGridBrowserCreatePopup(self->ctx->appGlobals, "", gettext("No hay contenidos agregados a Mi lista."));
    QBDialogRun(self->noResultsPopup, self, QBVoDGridBrowserMyRentalsNoResultDialogCallback);
    self->noResultsPopup->off_y = self->noResultsPopup->off_y - 20;
}
// END AMERELES MyList y Hide FakeCategory

SvLocal bool
QBVoDGridBrowserNavigationBarInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBVodGridBrowserNavigationBar navigationBar = w->prv;

    QBVoDGridBrowserHideMovieDetails(navigationBar->info);

    if (e->ch == QBKEY_UP)
        return true;

    if (e->ch == QBKEY_DOWN || e->ch == QBKEY_BACK) {
        // BEGIN AMERELES MyList y Hide FakeCategory
        if (navigationBar->info->ctx->contextType == QBVodGridBrowserContextType_myList) {
            int menuPosition;
            QBVoDGridGetPosition(navigationBar->info->grid, &menuPosition, NULL);
            if (menuPosition == 1) {
                QBVoDGridBrowserSelectMovies(navigationBar->info, false, true);
                int firstNormalMenuItemRowIdx = QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(navigationBar->info);
                QBVoDGridSetScrollRange(navigationBar->info->grid, firstNormalMenuItemRowIdx - 1, -1);
                QBVoDGridSetPosition(navigationBar->info->grid, firstNormalMenuItemRowIdx - 1, -1, true);
                QBVoDGridBrowserShowMovieDetails(navigationBar->info);
                return true;
            }
        }
        // END AMERELES MyList y Hide FakeCategory
        if (navigationBar->info->mode == QBVoDGridBrowserMode_movies) {
            QBVoDGridBrowserSelectMovies(navigationBar->info, false, true);
            QBVoDGridBrowserShowMovieDetails(navigationBar->info);
        } else if (navigationBar->info->mode == QBVoDGridBrowserMode_search) {
            int menuPosition;
            QBVoDGridGetPosition(navigationBar->info->grid, &menuPosition, NULL);
            if (menuPosition >= 0 && menuPosition != navigationBar->info->searchRowIdx) {
                QBVoDGridBrowserSelectMovies(navigationBar->info, false, false);
                QBVoDGridBrowserShowMovieDetails(navigationBar->info);
            } else {
                QBVoDGridBrowserMenuSetFocus(navigationBar->info);
                if (navigationBar->info->searchRowIdx >= 0) {
                    if (navigationBar->info->ctx->contentSearch) {
                        size_t len = QBContentCategoryGetLength((QBContentCategory) navigationBar->info->ctx->contentSearch);
                        if (len > 0) {
                            QBVoDGridSetPosition(navigationBar->info->grid, navigationBar->info->searchRowIdx, -1, false);
                            QBVoDGridBrowserShowMovieDetails(navigationBar->info);
                        } else {
                            QBVoDGridBrowserSelectMovies(navigationBar->info, false, false);
                        }
                    }
                }
            }
        }
        return true;
    }

    if (e->ch == QBKEY_ENTER) {
        if (navigationBar->buttonMovies && svWidgetIsFocused(navigationBar->buttonMovies)) {
            if (navigationBar->info->ctx->settings->moviesButtonMimicsVodButton) {
                bool popped = QBVODUtilsPopVodContexts(navigationBar->info->ctx->appGlobals->controller);
                if (!popped) {
                    QBVoDGridBrowserSelectMovies(navigationBar->info, false, true);
                    QBVoDGridBrowserShowMovieDetails(navigationBar->info);
                }
                return true;
            }

            bool differentTreeOrPath = false;
            if (navigationBar->info->ctx->settings->moveToRootOnMoviesButton) {
                QBVoDSubtree subTree = QBVoDSubtreesGetSubtree(navigationBar->info->ctx->subtreesContainer, QBVodGridBrowserContextType_default);
                if (!subTree) {
                    differentTreeOrPath = true;
                }
                SvObject childTree = SVRETAIN(QBVodSubtreeGetTree(subTree));
                SvObject path = SVTESTRETAIN(QBVodSubtreeGetData(subTree, SVSTRING("rootPath")));
                if (!SvObjectEquals(childTree, navigationBar->info->ctx->tree) || !SvObjectEquals(path, navigationBar->info->ctx->path)) {
                    differentTreeOrPath = true;
                }
                SVTESTRELEASE(path);
                SVTESTRELEASE(childTree);
            }

            if (navigationBar->info->ctx->contextType != QBVodGridBrowserContextType_default || differentTreeOrPath) {
                QBVoDGridBrowserContextShowDefaultWindow(navigationBar->info->ctx);
                return true;
            } else {
                QBVoDGridBrowserSelectMovies(navigationBar->info, false, true);
                QBVoDGridBrowserShowMovieDetails(navigationBar->info);
                return true;
            }
        }

        if (navigationBar->buttonSearch && svWidgetIsFocused(navigationBar->buttonSearch)) {
            QBVoDGridBrowserEnterIntoSearch(navigationBar);
            return true;
        }

        if (navigationBar->buttonWatched && svWidgetIsFocused(navigationBar->buttonWatched)) {
            if (navigationBar->info->ctx->contextType != QBVodGridBrowserContextType_watched) {
                QBVoDGridBrowserShowWatchedWindow(navigationBar->info->ctx);
                return true;
            } else {
                QBVoDGridBrowserSelectMovies(navigationBar->info, false, true);
                QBVoDGridBrowserShowMovieDetails(navigationBar->info);
                return true;
            }
        }
        if (navigationBar->buttonMyList && svWidgetIsFocused(navigationBar->buttonMyList)) {
            // BEGIN AMERELES MyList y Hide FakeCategory
            /*
            if (navigationBar->info->ctx->contextType != QBVodGridBrowserContextType_myList) {
                QBVoDGridBrowserShowMyListWindow(navigationBar->info->ctx);
            */
                navigationBar->info->ctx->contextType = QBVodGridBrowserContextType_myList;
                QBVoDGridBrowserSelectMovies(navigationBar->info, false, true);
                int firstNormalMenuItemRowIdx = QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(navigationBar->info) - 1;
                QBVoDGridSetScrollRange(navigationBar->info->grid, firstNormalMenuItemRowIdx, -1);
                QBVoDGridSetPosition(navigationBar->info->grid, firstNormalMenuItemRowIdx, -1, false);
                QBVoDGridBrowserShowMovieDetails(navigationBar->info);
                
                int categoryLength = QBVoDGridActiveRowGetLength(navigationBar->info->grid);
                if (categoryLength <= 0)
                {
                    QBVoDMovieDetailsSetupNoResultDialog(navigationBar->info);
                }
                
                return true;
            /*
            } else {
                QBVoDGridBrowserSelectMovies(navigationBar->info, false, true);
                QBVoDGridBrowserShowMovieDetails(navigationBar->info);
                return true;
            }
            */
            // END AMERELES MyList y Hide FakeCategory
        }
        if (navigationBar->buttonRecommended && svWidgetIsFocused(navigationBar->buttonRecommended)) {
            if (navigationBar->info->ctx->contextType != QBVodGridBrowserContextType_recommended) {
                QBVoDGridBrowserContextShowRecommendedWindow(navigationBar->info->ctx);
                return true;
            } else {
                QBVoDGridBrowserSelectMovies(navigationBar->info, false, true);
                QBVoDGridBrowserShowMovieDetails(navigationBar->info);
                return true;
            }
        }
    } else if (e->ch == QBKEY_RIGHT) {
        if (QBVoDGridBrowserFocusNavigationNext(navigationBar)) {
            return true;
        }
    } else if (e->ch == QBKEY_LEFT) {
        if (QBVoDGridBrowserFocusNavigationPrev(navigationBar)) {
            return true;
        }
    }
    return false;
}

SvLocal void
QBVoDGridBrowserItemCreated(SvObject owner_, QBVoDGridMenuItemInfo menuItemInfo)
{
    QBVoDGridBrowser self = (QBVoDGridBrowser) owner_;

    if (self->ctx->settings->searchInNewTab && self->ctx->contextType == QBVodGridBrowserContextType_search) {
        return;
    }

    SvObject category = menuItemInfo->category;
    if (!category) {
        return;
    }

    // BEGIN AMERELES MyList y Hide FakeCategory
    SvString categoryName = QBContentCategoryGetName((QBContentCategory) category);
    if (SvStringEqualToCString(categoryName, "Otros") ||
         SvStringEqualToCString(categoryName, "Capítulos y Temporadas"))
    {
        self->searchRowIdxOffset = 2;
    }
    // END AMERELES MyList y Hide FakeCategory

    if (!SvObjectIsInstanceOf(category, QBContentSearch_getType())) {
        return;
    }
    else
    {
        // BEGIN AMERELES MyList y Hide FakeCategory
        self->searchRow = menuItemInfo->item;
        self->searchRowIdx = menuItemInfo->idx;
        QBContentCategoryAddListener((QBContentCategory) category, (SvObject) self, NULL);
        svWidgetSetHidden(menuItemInfo->item, self->mode != QBVoDGridBrowserMode_search);
        // END AMERELES MyList y Hide FakeCategory
    }

    if (!self->searchResultsAvailable) {
        //QBVoDGridSetScrollRange(self->grid, QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(self), -1);
        // BEGIN AMERELES MyList y Hide FakeCategory
        if (self->ctx->contextType == QBVodGridBrowserContextType_myList)
        {
            QBVoDGridSetScrollRange(self->grid, QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(self) - 1, -1);
        }
        else
        {
            QBVoDGridSetScrollRange(self->grid, QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(self), -1);
        }
        // END AMERELES MyList y Hide FakeCategory
    }
}

SvLocal void
QBVoDGridBrowserCategoryStateChanged(SvObject owner_, QBVoDGridState newState)
{
    QBVoDGridBrowser self = (QBVoDGridBrowser) owner_;

    if (self->ctx->settings->showPopupInEmptyCategory && newState == QBVoDGridState_noresults) {
        SvErrorInfo error = NULL;
        QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_noResultState, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            SVTESTRELEASE(self->searchProgressDialog);
            self->searchProgressDialog = NULL;
        }
    }
}

SvLocal void
QBVoDGridBrowserItemStateChanged(SvObject owner_, QBVoDGridMenuItemInfo menuItemInfo)
{
    QBVoDGridBrowser self = (QBVoDGridBrowser) owner_;

    if (self->searchRowIdx >= 0 && menuItemInfo->idx == (unsigned int) self->searchRowIdx && !self->searchResultsAvailable) {
        svWidgetSetHidden(menuItemInfo->item, true);
    }

    //if (menuItemInfo->state == QBVoDGridState_loaded || menuItemInfo->state == QBVoDGridState_noresults) {
    // AMERELES MyList y Hide FakeCategory
    if ((menuItemInfo->state == QBVoDGridState_loaded || menuItemInfo->state == QBVoDGridState_noresults) && self->ctx->contextType != QBVodGridBrowserContextType_myList) {
        QBVoDGridBrowserUpdateMenuPosition(self);
    }
}

SvLocal void
QBVoDGridBrowserSearchResultsFocusChanged(SvObject owner_, bool focused)
{
    QBVoDGridBrowser self = (QBVoDGridBrowser) owner_;

    self->mode = (focused ? QBVoDGridBrowserMode_search : QBVoDGridBrowserMode_movies);
    
    // BEGIN AMERELES MyList y Hide FakeCategory
    if (self->mode != QBVoDGridBrowserMode_search)
    {
        if (self->ctx->contextType != QBVodGridBrowserContextType_myList)
        {
            int firstNormalMenuItemRowIdx = QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(self);
            QBVoDGridSetScrollRange(self->grid, firstNormalMenuItemRowIdx, -1);
        }
        else 
        {
            int firstNormalMenuItemRowIdx = QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(self);
            QBVoDGridSetScrollRange(self->grid, firstNormalMenuItemRowIdx - 1, -1);
        }
    }
    else if (self->ctx->contextType != QBVodGridBrowserContextType_myList)
    {
        self->ctx->contextType = QBVodGridBrowserContextType_default;
    }
    // END AMERELES MyList y Hide FakeCategory

    QBVodGridBrowserNavigationBar navigationBar = self->navigationBar->prv;
    QBVoDGridBrowserNavigationBarUpdateHighlight(navigationBar);
}

SvLocal SvWidget
QBVoDGridBrowserNavigationBarCreate(QBVoDGridBrowser info, SvApplication app, const char *widgetName)
{
    QBVodGridBrowserNavigationBar self = calloc(1, sizeof(*self));
    self->w = svSettingsWidgetCreate(app, widgetName);
    self->info = info;
    self->w->prv = self;
    svWidgetSetInputEventHandler(self->w, QBVoDGridBrowserNavigationBarInputEventHandler);

    char *buf;
    asprintf(&buf, "%s.ButtonMovies", svWidgetGetName(self->w));
    ssize_t focusQueueIndex = 0;
    if (svSettingsIsWidgetDefined(buf)) {
        self->buttonMovies = svSettingsWidgetCreate(app, buf);
        svSettingsWidgetAttach(self->w, self->buttonMovies, buf, 4);
        QBVoDGridBrowserNavigationBarButtonComponentsCreate(app, self->info->ctx->appGlobals->textRenderer, self->buttonMovies, gettext("Movies"), false, false);
        self->focusQueue[focusQueueIndex++] = self->buttonMovies;
    }
    free(buf);

    asprintf(&buf, "%s.ButtonWatched", svWidgetGetName(self->w));
    if (svSettingsIsWidgetDefined(buf)) {
        self->buttonWatched = svSettingsWidgetCreate(app, buf);
        svSettingsWidgetAttach(self->w, self->buttonWatched, buf, 4);
        QBVoDGridBrowserNavigationBarButtonComponentsCreate(app, self->info->ctx->appGlobals->textRenderer, self->buttonWatched, gettext("Watched"), false, false);
        self->focusQueue[focusQueueIndex++] = self->buttonWatched;
    }
    free(buf);

    asprintf(&buf, "%s.ButtonMyList", svWidgetGetName(self->w));
    if (svSettingsIsWidgetDefined(buf) && (self->info->ctx->hasButtonMyList)) {
        self->buttonMyList = svSettingsWidgetCreate(app, buf);
        svSettingsWidgetAttach(self->w, self->buttonMyList, buf, 4);
        QBVoDGridBrowserNavigationBarButtonComponentsCreate(app, self->info->ctx->appGlobals->textRenderer, self->buttonMyList, gettext("My List"), false, false);
        self->focusQueue[focusQueueIndex++] = self->buttonMyList;
    }
    free(buf);

    asprintf(&buf, "%s.ButtonRecommended", svWidgetGetName(self->w));
    if (svSettingsIsWidgetDefined(buf)) {
        self->buttonRecommended = svSettingsWidgetCreate(app, buf);
        svSettingsWidgetAttach(self->w, self->buttonRecommended, buf, 4);
        QBVoDGridBrowserNavigationBarButtonComponentsCreate(app, self->info->ctx->appGlobals->textRenderer, self->buttonRecommended, gettext("Recommended"), false, false);
        self->focusQueue[focusQueueIndex++] = self->buttonRecommended;
    }
    free(buf);

    asprintf(&buf, "%s.ButtonSearch", svWidgetGetName(self->w));
    if (svSettingsIsWidgetDefined(buf)) {
        self->buttonSearch = svSettingsWidgetCreate(app, buf);
    }
    if (self->buttonSearch) {
        svSettingsWidgetAttach(self->w, self->buttonSearch, buf, 4);
        bool searchDisabled = false;
        if (self->info->ctx->provider && SvObjectIsInstanceOf((SvObject) self->info->ctx->provider, Innov8onProvider_getType())) {
            searchDisabled = !Innov8onProviderIsSearchable((Innov8onProvider) self->info->ctx->provider);
        }
        QBVoDGridBrowserNavigationBarButtonComponentsCreate(app, self->info->ctx->appGlobals->textRenderer, self->buttonSearch, gettext("Search"), true, searchDisabled);
        self->focusQueue[focusQueueIndex++] = self->buttonSearch;
    }
    free(buf);

    return self->w;
}

SvLocal void
QBVoDGridBrowserContextStartChildProviders(QBVoDGridBrowserContext ctx)
{
    //If the window presents the data from a subtree (myList, watchList, search)
    //then we need to make sure providers are in the started state after the window
    //is created. Otherwise there are scenarios in which pages of data will not be downloaded.
    //Example:
    //1. Open the window with started providers
    //2. enter a movie details (which stops providers)
    //3. exit details and scroll the carousel. If the providers are not started you will see empty pages
    //QBVoDSubtree subTree = QBVoDSubtreesGetSubtree(ctx->subtreesContainer, ctx->contextType);
    // AMERELES MyList y Hide FakeCategory
    QBVoDSubtree subTree = QBVoDSubtreesGetSubtree(ctx->subtreesContainer, QBVodGridBrowserContextType_default);
    if (subTree) {
        size_t count = 0;
        QBContentProvider listProvider = NULL;
        while ((listProvider = (QBContentProvider) SVTESTRETAIN(QBVodSubtreeGetProvider(subTree, count))) != NULL) {
            QBContentProviderStart(listProvider, ctx->appGlobals->scheduler);
            SVRELEASE(listProvider);
            ++count;
        }
    }
}

SvLocal void
QBVoDGridBrowserContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBVoDGridBrowserContext self = (QBVoDGridBrowserContext) self_;
    // BEGIN AMERELES MyList y Hide FakeCategory
    // refresh data for lists upon every window creation
    //if (self->contextType == QBVodGridBrowserContextType_myList || self->contextType == QBVodGridBrowserContextType_watched) {
    //    QBVoDSubtree subTree = QBVoDSubtreesGetSubtree(self->subtreesContainer, self->contextType);
    //    if (subTree) {
    //        QBContentProvider listProvider = (QBContentProvider) SVTESTRETAIN(QBVodSubtreeGetProvider(subTree, 0));
    //        if (listProvider) {
    //            log_state("Refreshing data for list provider");
    //            QBContentProviderStop(listProvider);
    //            QBContentProviderClear(listProvider);
    //            QBContentProviderRefresh(listProvider, NULL, 0, true, false);
    //            QBContentProviderStart(listProvider, self->appGlobals->scheduler);
    //            SVRELEASE(listProvider);
    //        }
    //    }
    //}
    // END AMERELES MyList y Hide FakeCategory
    
    QBGlobalPlayerControllerStopCurrentController(self->appGlobals->playerController); // AMERELES Sacar DVB cuando se entra en en VoD3

    QBVoDGridBrowserContextStartChildProviders(self);

    QBVoDGridBrowser prv = (QBVoDGridBrowser) SvTypeAllocateInstance(
        QBVoDGridBrowser_getType(), NULL);
    prv->ctx = self;

    SvString settingsFile = SVSTRING("VoDGrid.settings");
    if (self->settings->settingsFile) {
        settingsFile = self->settings->settingsFile;
    }
    svSettingsPushComponent(SvStringCString(settingsFile));
    SvWidget window = prv->window = QBGUILogicCreateBackgroundWidget(self->appGlobals->guiLogic, "Window", NULL);
    window->prv = prv;
    window->clean = QBVoDGridBrowserClean;
    svWidgetSetName(window, "QBVoDGridBrowser");

    prv->settingsContext = svSettingsSaveContext();

    // AMERELES MyList y Hide FakeCategory
    //const char* MYLIST = QBConfigGet("MYLIST");
    //bool myListEnabled = MYLIST && strcmp(MYLIST, "enabled") == 0;
    prv->ctx->hasButtonMyList = SvStringEqualToCString(self->serviceId, "6") /*&& myListEnabled*/;

    prv->navigationBar = QBVoDGridBrowserNavigationBarCreate(prv, app, "NavigationBar");
    if (prv->navigationBar)
        svSettingsWidgetAttach(window, prv->navigationBar, "NavigationBar", 5);

    prv->grid = QBVoDGridCreate(app, "VoDGrid", self->appGlobals);

    prv->searchProgressDialogVisible = false; // AMERELES Fix en la navegación del menú de la derecha

    QBVoDGridSetUnlockedCategories(prv->grid, self->unlockedGridItems);

    QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
    QBEventBusRegisterReceiverForSender(eventBus, (SvObject) prv, QBVoDGridEvent_getType(), prv->grid->prv, NULL);

    QBVoDGridCallbacks gridCallbacks = {
        .QBVoDGridItemCreated               = QBVoDGridBrowserItemCreated,
        .QBVoDGridItemStateChanged          = QBVoDGridBrowserItemStateChanged,
        .QBVoDGridCategoryStateChanged      = QBVoDGridBrowserCategoryStateChanged,
        .QBVoDGridSearchResultsFocusChanged = QBVoDGridBrowserSearchResultsFocusChanged
    };
    QBVoDGridRegisterCallbacks(prv->grid, (SvObject) prv, gridCallbacks);
    svWidgetAttach(window, prv->grid, 0, 0, 3);
    
    // AMERELES MyList y Hide FakeCategory
    prv->tileToHideMyList = QBFrameCreateFromSM(app, "VerticalMenu.TileToHideMyList");
    svSettingsWidgetAttach(prv->grid, prv->tileToHideMyList, "VerticalMenu.TileToHideMyList", 3);
    svWidgetSetHidden(prv->tileToHideMyList, false);

    prv->details = svSettingsWidgetCreate(app, "MovieDetails");

    prv->movieDetails.info = QBMovieInfoCreate(app, "MovieDetails.Info", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(prv->details, prv->movieDetails.info, "MovieDetails.Info", 1);
    svWidgetSetHidden(prv->movieDetails.info, true);

    prv->movieDetails.description = QBAsyncLabelNew(app, "MovieDetails.Description", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(prv->details, prv->movieDetails.description, "MovieDetails.Description", 1);

    prv->movieDetails.title = QBAsyncLabelNew(app, "MovieDetails.Title", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(prv->details, prv->movieDetails.title, "MovieDetails.Title", 1);

    prv->showDetailsDelay = svSettingsGetDouble("MovieDetails", "showDelay", 0);

    svSettingsWidgetAttach(prv->grid, prv->details, "MovieDetails", 4);

    prv->contextMenu =
        QBContextMenuCreateFromSettings("ContextMenu.settings",
                                        self->appGlobals->controller,
                                        self->appGlobals->res,
                                        SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(prv->contextMenu, QBVoDGridBrowserSideMenuClose, prv);

    svWidgetSetHidden(prv->details, true);

    svWidgetSetInputEventHandler(window, QBVoDGridBrowserInputEventHandler);
    svWidgetSetTimerEventHandler(window, QBVoDGridBrowserTimerEventHandler);
    QBInputServiceAddGlobalFilter((SvObject) prv->ctx->exitInputFilter, NULL, NULL);

    prv->adWindow = NULL;
    SvWidget ad = NULL;

    if (svSettingsIsWidgetDefined("AdIcon")) {
        bool useStaticAd = svSettingsGetBoolean("AdIcon", "useStaticAd", false);
        if (useStaticAd) {
            const char* url = QBConfigGet("ADVERT_URL");
            if (url) {
                prv->adWindow = QBAdWindowNew(self->appGlobals);
                ad = svIconNew(self->appGlobals->res, "AdIcon");
                ad->off_x = svSettingsGetInteger("AdIcon", "xOffset", 0);
                ad->off_y = svSettingsGetInteger("AdIcon", "yOffset", 0);
                svIconSetBitmapFromURI(ad, 0, url);
            }
        } else if (self->appGlobals->adService) {
            prv->adWindow = QBAdWindowNew(self->appGlobals);
            QBAdServiceAddListener(self->appGlobals->adService, (SvObject) prv, NULL);
            SvString VoDSettingsFile = SVSTRING("VoDGrid.settings");
            if (self->settings->settingsFile) {
                VoDSettingsFile = self->settings->settingsFile;
            }
            ad = QBAdServiceGetAd(self->appGlobals->adService, SvStringCString(VoDSettingsFile));
        }
    }

    if (prv->adWindow) {
        QBAdWindowSetAd(prv->adWindow, ad);
        QBAdWindowShow(prv->adWindow);
    }

    svSettingsPopComponent();

    self->super_.window = window;

    if (!prv->ctx->searchContentAvailable) {
        prv->searchRow = NULL;
        prv->searchRowIdx = -1;
        prv->searchResultsAvailable = false;
    } else {
        prv->searchResultsAvailable = true;
    }
    
    // AMERELES MyList y Hide FakeCategory
    prv->searchRowIdxOffset = 1;
    
    QBVoDGridSetSearchResultAvailability(prv->grid, prv->searchResultsAvailable);
    //QBVoDGridSetScrollRange(prv->grid, prv->searchRowIdx, -1);
    QBVoDGridSetScrollRange(prv->grid, 1, -1); // AMERELES MyList y Hide FakeCategory

    prv->mode = prv->ctx->lastMode;

    QBProvidersControllerServiceAddListener(self->appGlobals->providersController, (SvObject) prv, QBProvidersControllerService_INNOV8ON);
    QBProvidersControllerServiceAddListener(self->appGlobals->providersController, (SvObject) prv, QBProvidersControllerService_MWClient);
    QBVoDGridConnectToTree(prv->grid, (SvObject) self->tree, self->path, self->filterNode);

    // AMERELES MyList y Hide FakeCategory
    //if (prv->ctx->lastMenuPos < QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(prv) && !prv->searchResultsAvailable) {
    if ((prv->ctx->lastMenuPos < QBVoDGridBrowserGetFirstNormalMenuItemRowIdx(prv) - 1) && !prv->searchResultsAvailable) {
        QBVoDGridBrowserContextResetSavedPosition(self);
    }

    QBVoDGridSetPosition(prv->grid, prv->ctx->lastMenuPos, prv->ctx->lastMenuItemPos, true);

    QBVoDGridBrowserMenuSetFocus(prv);

    prv->ctx->isSearchStarted = false;
    
    // BEGIN AMERELES Buscador Global
    if (self->appGlobals->vodGridBrowserInitAsSearch)
    {
        self->appGlobals->vodGridBrowserInitAsSearch = false;
        QBVoDGridBrowserSelectSearch(prv);
    }
    // END AMERELES Buscador Global

    prv->contentSideMenu = (SvObject) QBContentSideMenuCreate(self->appGlobals, SVSTRING("QBVoDMovieMenu.json"), SVSTRING("PC_VOD"));
    if (self->settings->refreshObject) {
        QBContentSideMenuSetRefreshObject((QBContentSideMenu) prv->contentSideMenu, true);
    }
    QBGUILogicContentSideMenuRegisterCallbacks(self->appGlobals->guiLogic, prv->contentSideMenu);

    if (self->contentSearch) {
        QBContentCategoryAddListener((QBContentCategory) self->contentSearch, (SvObject) prv, NULL);
    }
    if (self->focusKeyboard) {
        QBVoDGridBrowserSearchMenuShow(prv);
        self->focusKeyboard = false;
    }

    QBVodGridBrowserNavigationBar navigationBar = prv->navigationBar->prv;
    QBVoDGridBrowserNavigationBarUpdateHighlight(navigationBar);

    if (self->eventReporter) {
        //we send watched event upon every window creation, because we don't know where we entered the window from.
        //Be it back from the player or from the grid it doesnt matter because thanks to using QBEventReporterWatchedEventFilter
        //we make sure that the watched event is sent only when play occured first and enough content has been played.
        SvInvokeInterface(QBMWClientEventReporter, self->eventReporter, sendEvent, self->appGlobals->scheduler, QBMWClientEventReporter_watched, NULL);
    }
    QBApplicationControllerAddListener(self->appGlobals->controller, (SvObject) self);
}

SvLocal void
QBVoDGridBrowserContextDestroyWindow(QBWindowContext self_)
{
    QBVoDGridBrowserContext self = (QBVoDGridBrowserContext) self_;
    QBVoDGridBrowserContextStopProviders(self);
    QBApplicationControllerRemoveListener(self->appGlobals->controller, (SvObject) self);
    
    QBGlobalPlayerControllerResumeCurrentController(self-> appGlobals->playerController); // AMERELES Sacar DVB cuando se entra en en VoD3

    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal void
QBVoDGridBrowserContextDestroy(void *self_)
{
    QBVoDGridBrowserContext self = self_;
    QBAccessManagerRemoveListener(self->appGlobals->accessMgr, (SvObject) self, NULL);

    SVTESTRELEASE(self->path);
    SVTESTRELEASE(self->originalPath);
    SVTESTRELEASE(self->contentSearch);
    SVTESTRELEASE(self->childProviders);
    SVTESTRELEASE(self->provider);
    SVRELEASE(self->dataSource);
    SVRELEASE(self->serviceId);
    SVRELEASE(self->serviceName);
    SVRELEASE(self->unlockedGridItems);
    SVTESTRELEASE(self->filterNode);
    SVRELEASE(self->settings);
    SVRELEASE(self->subtreesContainer);
    SVRELEASE(self->exitInputFilter);
    SVTESTRELEASE(self->eventReporter);

    if (self->isCategoriesView) {
        log_debug("invalidate authentication");
        QBAccessManagerInvalidateAuthentication(self->appGlobals->accessMgr, SVSTRING("PC_VOD"), NULL);
    }
}

SvLocal void
QBVoDGridBrowserContextAuthenticationChanged(SvObject self_, QBAccessManager manager, SvString domainName)
{
    QBVoDGridBrowserContext self = (QBVoDGridBrowserContext) self_;

    if (!SvStringEqualToCString(domainName, "PC_VOD"))
        return;

    QBAuthStatus code = QBAccessManagerGetAuthenticationStatus(self->appGlobals->accessMgr, SVSTRING("PC_VOD"), NULL);
    log_debug("domainName: %s  code: %d", SvStringCString(domainName), code);

    if (code == QBAuthStatus_inProgress)
        return;

    if (code != QBAuthStatus_OK)
        SvSetRemoveAllElements(self->unlockedGridItems);
}

SvLocal void
QBVoDGridBrowserContextSwitchStarted(SvObject ctx_, QBWindowContext from, QBWindowContext to)
{
    QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_disabled, NULL);
}

SvLocal void
QBVoDGridBrowserContextSwitchEnded(SvObject ctx_, QBWindowContext from, QBWindowContext to)
{
}

SvType
QBVoDGridBrowserContext_getType(void)
{
    static SvType type = NULL;
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_        = {
            .destroy   = QBVoDGridBrowserContextDestroy
        },
        .createWindow  = QBVoDGridBrowserContextCreateWindow,
        .destroyWindow = QBVoDGridBrowserContextDestroyWindow
    };

    static const struct QBAccessManagerListener_t authenticationListenerMethods = {
        .authenticationChanged = QBVoDGridBrowserContextAuthenticationChanged
    };

    static const struct QBContextSwitcherListener_t switchMethods = {
        .started = QBVoDGridBrowserContextSwitchStarted,
        .ended   = QBVoDGridBrowserContextSwitchEnded
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDGridBrowserContext",
                            sizeof(struct QBVoDGridBrowserContext_),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            QBAccessManagerListener_getInterface(), &authenticationListenerMethods,
                            QBContextSwitcherListener_getInterface(), &switchMethods,
                            NULL);
    }

    return type;
}

/**
 * add search category as static item on index 0 to the category
 * passed as parameter.
 **/
SvLocal QBContentSearch
QBVoDGridBrowserAddSearchChildCategory(QBVoDGridBrowserContext self, QBContentCategory parentCategory)
{
    if (!parentCategory) {
        return NULL;
    }

    SVTESTRELEASE(self->contentSearch);
    QBVoDGridBrowser prv = self->super_.window ? self->super_.window->prv : NULL;

    size_t length = QBContentCategoryGetLength(parentCategory);
    for (size_t i = 0; i < length; i++) {
        SvObject obj = QBContentCategoryGetObjectAtIndex(parentCategory, i);
        if (SvObjectIsInstanceOf(obj, QBContentSearch_getType())) {
            log_debug("Search already present in the category - no need to add it");
            self->contentSearch = SVRETAIN((QBContentSearch) obj);
            if (prv) {
                QBContentCategoryAddListener((QBContentCategory) self->contentSearch, (SvObject) prv, NULL);
            }
            return NULL;
        }
    }

    SvString tmp = SvStringCreate(gettext("Search"), NULL);
    QBContentSearch category = QBContentSearchCreate(tmp, parentCategory, NULL, NULL);
    SVRELEASE(tmp);
    log_debug("Adding search as static item on index 0");
    QBContentCategoryPrependStaticItem(parentCategory, (SvObject) category);
    self->contentSearch = SVRETAIN(category);
    if (prv) {
        QBContentCategoryAddListener((QBContentCategory) self->contentSearch, (SvObject) prv, NULL);
    }

    return category;
}

SvLocal QBContentCategory
QBVoDGridBrowserContextGetCategory(SvObject tree, SvObject path)
{
    QBContentCategory category = NULL;
    SvObject node = SvInvokeInterface(QBTreeModel, tree, getNode, path);
    if (node && SvObjectIsInstanceOf(node, QBContentCategory_getType())) {
        category = (QBContentCategory) node;
    } else if (node && SvObjectIsInstanceOf(node, QBActiveTreeNode_getType())) {
        SvObject subTree = QBActiveTreeGetMountedSubTree((QBActiveTree) tree, path);
        node = SvInvokeInterface(QBTreeModel, subTree, getNode, NULL);
        category = (QBContentCategory) node;
    }
    return category;
}

SvLocal void
QBVoDGridBrowserContextInit(QBVoDGridBrowserContext self,
                            SvObject tree,
                            SvObject path,
                            QBContentProvider provider,
                            bool isCategoriesView,
                            QBVodGridBrowserContextType contextType)
{
    SVTESTRELEASE(self->provider);
    self->provider = NULL;
    SVTESTRELEASE(self->filterNode);
    self->filterNode = NULL;
    SVTESTRELEASE(self->path);
    self->path = NULL;
    SVTESTRELEASE(self->dataSource);
    self->dataSource = NULL;

    self->contextType = contextType;
    self->provider = SVTESTRETAIN(provider);
    self->tree = tree ? tree : (SvObject) self->appGlobals->menuTree;
    self->path = path ? SvObjectCopy(path, NULL) : NULL;
    self->isCategoriesView = isCategoriesView;

    QBContentCategory rootCategory = SVTESTRETAIN(QBVoDGridBrowserContextGetCategory(self->tree, self->path));

    bool leafCategory = false;
    if (rootCategory) {
        SvValue leafVal = (SvValue) QBContentCategoryGetAttribute((QBContentCategory) rootCategory, SVSTRING("leaf"));
        if (leafVal && SvValueIsInteger(leafVal)) {
            leafCategory = (bool) SvValueGetInteger(leafVal);
        } else if (leafVal && SvValueIsBoolean(leafVal)) {
            leafCategory = SvValueGetBoolean(leafVal);
        }
    }

    if (leafCategory) {
        self->filterNode = SVTESTRETAIN((SvObject) rootCategory);
        if (self->path && SvObjectIsImplementationOf(self->path, QBTreePath_getInterface())) {
            SvInvokeInterface(QBTreePath, self->path, truncate, -1);
            SvObject node = SvInvokeInterface(QBTreeModel, self->tree, getNode, self->path);
            if (node && SvObjectIsInstanceOf(node, QBContentCategory_getType())) {
                SVTESTRELEASE(rootCategory);
                rootCategory = SVTESTRETAIN((QBContentCategory) node);
            }
        }
    }

    if (!(self->settings->searchInNewTab)) {
        QBContentSearch searchChildCategory = NULL;
        if (self->provider && (searchChildCategory = QBVoDGridBrowserAddSearchChildCategory(self, rootCategory)) != NULL) {
            SvObject searchProvider = QBVoDGridBrowserContextAddProviderToCategory(self, (QBContentCategory) searchChildCategory, "search", NULL);
            if (searchProvider) {
                if (!self->childProviders) {
                    self->childProviders = SvArrayCreateWithCapacity(2, NULL);
                }
                SvArrayAddObject(self->childProviders, searchProvider);
                SVRELEASE(searchProvider);
            }
            SVRELEASE(searchChildCategory);
        }
    }

    SVTESTRELEASE(rootCategory);

    QBListProxy proxy = QBListProxyCreate(self->tree, self->path, NULL);
    self->dataSource = (SvObject) proxy;
    QBVoDGridBrowserContextResetSavedPosition(self);

    if (self->super_.window) {
        QBVoDGridBrowser browser = self->super_.window->prv;
        QBVodGridBrowserNavigationBar navigationBar = browser->navigationBar->prv;
        QBVoDGridBrowserNavigationBarUpdateHighlight(navigationBar);
    }
}

SvLocal QBWindowContext
QBVoDGridBrowserContextCreateImpl(AppGlobals appGlobals, SvObject path,
                                  QBContentProvider provider, SvString serviceId,
                                  SvString serviceName, SvObject tree,
                                  SvSet unlockedCategories,
                                  QBVoDGridBrowserSettings settings, bool isCategoriesView,
                                  QBVoDSubtrees subtreesContainer, QBVodGridBrowserContextType contextType)
{
    QBVoDGridBrowserContext self = (QBVoDGridBrowserContext) SvTypeAllocateInstance(
        QBVoDGridBrowserContext_getType(), NULL);
    if (settings) {
        self->settings = SVRETAIN(settings);
    } else {
        self->settings = QBVoDGridBrowserSettingsCreate();
    }
    self->subtreesContainer = SVRETAIN(subtreesContainer);

    self->contentManagerType = QBContentManagerType_cubiware;
    const char* providerConfigSetting = QBConfigGet("PROVIDERS.VOD");
    if (strcmp(providerConfigSetting, "digitalsmiths") == 0) {
        self->contentManagerType = QBContentManagerType_mwClient;
    }
    self->isSearchStarted = false;
    self->searchContentAvailable = false;
    self->appGlobals = appGlobals;

    self->originalPath = path ? SvObjectCopy(path, NULL) : NULL;
    QBVoDGridBrowserContextInit(self, tree, path, provider, isCategoriesView, contextType);

    self->serviceId = SVRETAIN(serviceId);
    self->serviceName = SVRETAIN(serviceName);

    if (!unlockedCategories) {
        self->unlockedGridItems = SvSetCreate(1, NULL);
    } else {
        //we use an external set of unlocked categories.
        //those categories will not be blocked by parental control.
        self->unlockedGridItems = SVRETAIN(unlockedCategories);
        log_debug("use external set of unlocked categories. Size: %zu", SvSetGetCardinality(unlockedCategories, NULL));
    }

    QBAccessManagerAddListener(self->appGlobals->accessMgr, (SvObject) self, NULL);

    static const QBLongKeyPressExtendedMapping map[] = {
        { QBKEY_BACK, QBKEY_BACK, QBKEY_VOD, 3 * 1000 * 1000, false },
        { 0,          0,          0,         0,               false }
    };
    self->exitInputFilter = (QBLongKeyPressFilter) SvTypeAllocateInstance(QBLongKeyPressFilter_getType(), NULL);
    QBLongKeyPressFilterInitExtended(self->exitInputFilter, map, NULL);

    return (QBWindowContext) self;
}

extern QBWindowContext
QBVoDGridBrowserContextCreate(AppGlobals appGlobals, SvObject path,
                              SvObject provider, SvString serviceId,
                              SvString serviceName, QBActiveTree externalTree,
                              SvSet unlockedCategories,
                              QBVoDGridBrowserSettings settings)
{
    if (!provider || !SvObjectIsInstanceOf(provider, QBContentProvider_getType())) {
        return NULL;
    }
    QBVoDSubtrees subtreesContainer = NULL;
    SvObject tree = externalTree ? (SvObject) externalTree : (SvObject) appGlobals->menuTree;
    subtreesContainer = QBVoDSubtreesCreate();

    QBVoDSubtree subTree = QBVoDSubtreeCreate(tree);
    QBVoDSubtreeAddProvider(subTree, provider);
    if (path) {
        QBVodSubtreeSetData(subTree, SVSTRING("rootPath"), path);
    }
    QBVoDSubtreesSetSubtree(subtreesContainer, QBVodGridBrowserContextType_default, subTree);
    SVRELEASE(subTree);

    QBWindowContext ret = QBVoDGridBrowserContextCreateImpl(appGlobals, path, (QBContentProvider) provider, serviceId,
                                                            serviceName, (SvObject) externalTree,
                                                            unlockedCategories,
                                                            settings, true, subtreesContainer, QBVodGridBrowserContextType_default);
    SVTESTRELEASE(subtreesContainer);
    return ret;
}
