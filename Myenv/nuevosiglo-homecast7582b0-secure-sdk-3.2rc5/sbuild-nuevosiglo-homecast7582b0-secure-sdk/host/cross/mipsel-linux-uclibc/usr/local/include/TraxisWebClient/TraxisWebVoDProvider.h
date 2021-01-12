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

#ifndef TRAXIS_WEB_VOD_PROVIDER_H_
#define TRAXIS_WEB_VOD_PROVIDER_H_

/**
 * @file TraxisWebVoDProvider.h
 * @brief Traxis.Web VoD provider class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdlib.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBContentManager/QBContentTree.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBSearch/QBSearchStatus.h>
#include <TraxisWebClient/TraxisWebSessionManager.h>


/**
 * @defgroup TraxisWebVoDProvider Traxis.Web VoD provider class
 * @ingroup TraxisWebClient
 * @{
 *
 * @image html TraxisWebVoDProvider.png
 **/

typedef void (*TraxisWebVoDProviderCategoryCallback)(SvObject cookie, SvObject category);

/**
 * Category Filter states of the Traxis.Web VoD provider.
 **/
typedef enum {
    /// filter disabled
    TraxisWebVoDFilterDisabled,
    /// filter with false value
    TraxisWebVoDFilterFalse,
    ///filter with true value
    TraxisWebVoDFilterTrue,
} TraxisWebVoDFilterState;

/**
 * Category filter configuration of the Traxis.Web VoD provider.
 **/
typedef struct TraxisWebVoDProviderFilterConfig_s {
    /// @c TraxisWebVoDFilterState value controlling filtering titles based on IsPreview attribute
    TraxisWebVoDFilterState previewTitlesFilter;
    /// @c TraxisWebVoDFilterState value controlling filtering titles based on IsViewableOnCPE attribute
    TraxisWebVoDFilterState viewableTitlesFilter;
    /// @c TraxisWebVoDFilterState value controlling filtering titles based on IsVisible attribute
    TraxisWebVoDFilterState visibleTitlesFilter;
    /// @c TraxisWebVoDFilterState value controlling filtering titles based on IsFeature attribute
    TraxisWebVoDFilterState featureTitlesFilter;
} TraxisWebVoDProviderFilterConfig;

/**
 * Configuration of the Traxis.Web VoD provider.
 **/
typedef struct TraxisWebVoDProviderConfig_s {
    /// minimal refresh of assets and categories period in seconds, @c -1 in case of error
    unsigned int minRefreshPeriod;

    /// @c true to enable virtual "My Rentals" category
    bool enableMyRentals;
    /// @c true to enable virtual "Search" category
    bool enableSearch;
    /// @c true to enable virtual "Search" category in each category
    bool enableSearchInEachCategory;
    /// @c true to enable virtual "Search" in side menu
    bool enableSearchInSideMenu;
    /// @c true to enable "Coupons" category
    bool enableCoupons;
    /// @c true to enable "Favorites" category
    bool enableFavorites;

    /// @c true to ignore TVOD products
    bool ignoreTransactionProducts;
    /// @c true to ignore SVOD products
    bool ignoreSubscriptionProducts;

    /// action which should be taken after pushing ENTER button
    SvString onEnterButton;
    /// action which should be taken after pushing OPTION button
    SvString onOptionButton;

    /// @c true to ignore TVOD products in "Favorites" category
    bool ignoreTransactionProductsOnFavorites;
    /// @c true to ignore SVOD products in "Favorites" category
    bool ignoreSubscriptionProductsOnFavorites;
    /// @c true to ignore FVOD products in "Favorites" category
    bool ignoreFreeProductsOnFavorites;

    /// category filter config
    TraxisWebVoDProviderFilterConfig filterConfig;
    /// search category filter config
    TraxisWebVoDProviderFilterConfig searchFilterConfig;
    /// my rentals category filter config
    TraxisWebVoDProviderFilterConfig myRentalsFilterConfig;

    /// max number of assets in cache, @c 0 for no limit
    unsigned int maxCachedAssets;
    /// max number of search results, must be less than @a maxCachedAssets
    unsigned int maxSearchResults;

    /// limit enableSearchInEachCategory to certain amount of levels
    unsigned int searchInEachCategoryDepth;
} TraxisWebVoDProviderConfig;

/**
 * Traxis.Web VoD provider class.
 * @class TraxisWebVoDProvider
 * @extends QBContentProvider
 **/
typedef struct TraxisWebVoDProvider_ *TraxisWebVoDProvider;

/**
 * TraxisWebVoDSearchProvider http query error states used for handling common returned errors.
 **/
typedef enum {
    /// bad query
    TraxisWebVoDSearchProviderError_BadQuery = 400,
    /// unauthorized access
    TraxisWebVoDSearchProviderError_UnauthorizedAccess = 401,
    /// resource not found
    TraxisWebVoDSearchProviderError_ResourceNotFound = 404,
    /// query custom data exceeds allowed configured size
    TraxisWebVoDSearchProviderError_AllowedSizeExceeded = 494,
    /// internal server error
    TraxisWebVoDSearchProviderError_InternalServerError = 500
} TraxisWebVoDSearchProviderError;


/**
 * Get runtime type identification object representing
 * type of Traxis.Web VoD provider class.
 *
 * @return Traxis.Web VoD provider class
 **/
extern SvType
TraxisWebVoDProvider_getType(void);

/**
 * Create Traxis.Web VoD provider.
 *
 * @memberof TraxisWebVoDProvider
 *
 * @param[in] manager   Traxis.Web session manager handle
 * @param[in] tree      handle to a content tree to put VoD content in
 * @param[in] catalogName VoD catalog name
 * @param[in] config    configuration for provider's behavior
 * @param[out] errorOut error info
 * @return              created provider, @c NULL in case of error
 **/
extern TraxisWebVoDProvider
TraxisWebVoDProviderCreate(TraxisWebSessionManager manager,
                           QBContentTree tree,
                           SvString catalogName,
                           const TraxisWebVoDProviderConfig* config,
                           SvErrorInfo *errorOut);

/**
 * Get configuration of the VoD provider.
 *
 * @memberof TraxisWebVoDProvider
 *
 * @param[in] self      Traxis.Web VoD provider handle
 * @return              provider's configuration
 **/
extern const TraxisWebVoDProviderConfig*
TraxisWebVoDProviderGetConfig(TraxisWebVoDProvider self);

/**
 * Setup sort order for categories and titles.
 *
 * @memberof TraxisWebVoDProvider
 *
 * @param[in] self      Traxis.Web VoD provider handle
 * @param[in] categoriesSortProps comma-separated list of properties
 *                      to sort categories by, @c NULL to use defaults
 * @param[in] sortCategoriesDesc @c true to sort categories descending,
 *                      @c false to sort ascending
 * @param[in] titlesSortProps comma-separated list of properties
 *                      to sort titles by, @c NULL to use defaults
 * @param[in] sortTitlesDesc @c true to sort titles descending,
 *                      @c false to sort ascending
 * @param[out] errorOut error info
 **/
extern void
TraxisWebVoDProviderSetSortOrder(TraxisWebVoDProvider self,
                                 const char *categoriesSortProps,
                                 bool sortCategoriesDesc,
                                 const char *titlesSortProps,
                                 bool sortTitlesDesc,
                                 SvErrorInfo *errorOut);

/**
 * Get Traxis.Web session manager used by VoD provider.
 *
 * @memberof TraxisWebVoDProvider
 *
 * @param[in] self      Traxis.Web VoD provider handle
 * @return              session manager handle
 **/
extern TraxisWebSessionManager
TraxisWebVoDProviderGetSessionManager(TraxisWebVoDProvider self);

/**
 * Registers a callback that is called after a category is received and before
 * it is propagated further. It can be used to adopt categories in a
 * client specific way.
 *
 * @param[in] self      Traxis.Web VoD provider handle
 * @param[in] callback  callback called whe a category arrives
 * @param[in] cookie    first argument to callback used to store callback specific data
 **/
extern void
TraxisWebVoDProviderRegisterCategoryCallback(SvObject self, TraxisWebVoDProviderCategoryCallback callback, SvObject cookie);

/**
 * Force Reload all categories and assets
 * (sending requests to Traxis is triggered by user moving between VoD Titles)
 *
 * @memberof TraxisWebVoDProvider
 *
 * @param[in] self      Traxis.Web VoD provider handle
 **/
extern void
TraxisWebVoDProviderForceReloadEverything(TraxisWebVoDProvider self);

/**
 * Force reload one specific category
 *
 * @memberof TraxisWebVoDProvider
 *
 * @param[in] self      Traxis.Web VoD provider handle
 * @param[in] category  Traxis.Web VoD category handle
 */
extern void
TraxisWebVoDProviderReloadCategory(TraxisWebVoDProvider self, QBContentCategory category);

/**
 *  Set search callback.
 *
 *  @memberof TraxisWebVoDProvider
 *
 *  @param[in] self             Traxis.Web VoD provider handle
 *  @param[in] callbackData     Callback object handle
 *  @param[in] callback         QBSearchStatusUpdate function pointer handle
 */
extern void
TraxisWebVoDProviderSetSearchCallback(TraxisWebVoDProvider self,
                                      void *callbackData,
                                      QBSearchStatusUpdate callback);

/**
 * Verify if category is "Favorites" category.
 *
 * @param[in] self      Traxis.Web VoD provider handle
 * @param[in] category  QBContentCategory handle
 * @return              @c true if category is the virtual "Favorites" category, @c false, otherwise
 **/
extern bool
TraxisWebVoDProviderIsFavoritesCategory(TraxisWebVoDProvider self, QBContentCategory category);

/**
 * Force reload "Favorites" category.
 *
 * @param[in] self      Traxis.Web VoD provider handle
 **/
extern void
TraxisWebVoDProviderReloadFavorites(TraxisWebVoDProvider self);

/**
 * Return root name.
 *
 * @param[in] self Traxis.Web VoD provider handle
 * @return         root name
 */
extern SvString
TraxisWebVoDProviderGetRootName(TraxisWebVoDProvider self);

/**
 * @}
 **/


#endif
