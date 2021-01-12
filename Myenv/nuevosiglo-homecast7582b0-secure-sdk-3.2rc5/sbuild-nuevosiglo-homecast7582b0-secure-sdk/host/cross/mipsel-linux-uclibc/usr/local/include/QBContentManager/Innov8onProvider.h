/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

/* App/Libraries/QBContentManager/Innov8onProvider.h */

#ifndef INNOV8ON_PROVIDER_H_
#define INNOV8ON_PROVIDER_H_

/**
 * @file Innov8onProvider.h
 * @brief Innov8on provider class API
 **/

/**
 * @defgroup Innov8onProvider Innov8on provider class
 * @ingroup Innov8on
 * @{
 **/

#include <stdlib.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <QBContentManager/QBContentTree.h>
#include <QBContentManager/Innov8onProviderParams.h>


/**
 * Innov8on provider class.
 * Innvo8on Middleware provides tree-like data structure with 'categories' and
 * 'assets'. Each non-leaf object is a category, all leafs are assets.
 * Single provider obtains contents of a single category, in order to obtain
 * contents of a subcategory a new instance is created.
 *
 * The data is provided as a QBContentTree.
 *
 * Due to the fact that the tree may be large, or that we want to
 * reduce MW load the tree is obtained on demand and cached. This means that
 * provider downloads only what is needed and not the whole tree. Also,
 * data is cached in order to reduce amount of requests to the MW.
 *
 **/
typedef struct Innov8onProvider_ *Innov8onProvider;

// must match categoryType attribute as returned by the middleware
#define INNOV8ON_CATEGORY_TYPE__MY_RENTALS (SVSTRING("myRentals"))

typedef enum {
    Innov8onServerResponse_Success = 200,
    Innov8onServerResponse_SuccessForFree = 210,
    Innov8onServerResponse_FailedCMSConnection = 4300,
    Innov8onServerResponse_FailedCMSTimeout = 4301,
    Innov8onServerResponse_FailedGenericError = 4302,
    Innov8onServerResponse_FailedNotEnaughCredits = 4303,
    Innov8onServerResponse_FailedProductNotExist = 4304,
    Innov8onServerResponse_FailedInvalidProduct = 4305,
    Innov8onServerResponse_FailedInvalidSmartCardID = 4306,
    Innov8onServerResponse_FailedSmartCardNotOnList = 4307,
    Innov8onServerResponse_FailedAlreadyOrderedInMW = 69,
    Innov8onServerResponse_FailedAlreadyOrderedInCMS = 4308,
    Innov8onServerResponse_FailedInactiveSubscription = 4309,
    Innov8onServerResponse_FailedExpiringSoon = 4310,
    Innov8onServerResponse_FailedVerifyOfferWithDevice = 4311
} Innov8onServerResponse;

/**
 * Get runtime type identification object representing
 * type of Innov8on provider class.
 * @return Innov8on provider runtime type identification object
 **/
SvType
Innov8onProvider_getType(void);

/**
 * Create Innov8on provider.
 *
 * @param[in] tree          handle to a content tree to put VoD content in
 * @param[in] params        provider parameters
 * @param[in] serviceID     MW service ID
 * @param[in] categoryID    ID of first category (@c NULL to default)
 * @param[in] searchable    show (or don't) Search option in menu
 * @param[in] allVisible    show (or don't) See All From option in menu
 * @param[out] errorOut     error info
 * @return                  created provider, @c NULL in case of error
 **/
Innov8onProvider
Innov8onProviderCreate(QBContentTree tree,
                       Innov8onProviderParams params,
                       SvString serviceID,
                       SvString categoryID,
                       bool searchable,
                       bool allVisible,
                       SvErrorInfo *errorOut);


/**
 * Returns server info from provider init params.
 * @param[in] self          provider handle
 * @param[out] errorOut      error info
 * @return XML RPC server info.
 **/
SvXMLRPCServerInfo
Innov8onProviderGetServerInfo(Innov8onProvider self, SvErrorInfo *errorOut);

/**
 * Returns params from Innov8onProviderCreate().
 *
 * @param[in] self          provider handle
 * @param[out] errorOut      error info
 * @return provder params
 **/
Innov8onProviderParams
Innov8onProviderGetParams(Innov8onProvider self, SvErrorInfo *errorOut);

/**
 * Requests refreshing of cached object. Objects may be available under multiple
 * categories, and the object must be refreshed in all of them.
 *
 * @param[in] self          provider handle
 * @param[in] path          array of paths to parent categories
 * @param[in] obj           object to be refreshed
 **/
void
Innov8onProviderRefreshObject(Innov8onProvider self, SvArray path, SvDBRawObject obj);

/**
 * Returns path to the category whose contents this provider obtains.
 * @param[in] self          provider handle
 * @param[out] errorOut      error info
 * @return path to the category whose contents this provider obtains.
 **/
SvObject
Innov8onProviderGetRootContentPath(Innov8onProvider self,
                                   SvErrorInfo *errorOut);

/**
 * Returns whether the category whose contents tyhis provider obtains is searchable.
 * @param[in] self          provider handle
 * @return returns true iff category whose contents tyhis provider obtains is searchable.
 **/
bool
Innov8onProviderIsSearchable(Innov8onProvider self);

/**
 * Invalidate cached assets for all categories of given type.
 *
 * @param[in] self
 * @param[in] categoryType  must match Middleware category attribute 'categoryType'
 */
void
Innov8onProviderInvalidateCategory(Innov8onProvider self, SvString categoryType);

void
Innov8onProviderEnableSearch(Innov8onProvider self, bool enabled);

void
Innov8onProviderEnableAllVisible(Innov8onProvider self, bool enabled);

/**
 * Prepend each path with Middleware address
 *
 * @param[in] self           provider handle
 * @param[in] obj            object to be adapted
 * @return                   return adapted object
 */
SvDBRawObject
Innov8onProviderAdaptObject(Innov8onProvider self, SvDBRawObject obj);




/**
 * Make URI(s) in a given field of a database absolute.
 *
 * Checks if under field of a database is a string with absolute URI - if only relative,
 * prepends prefix from provided URI.
 *
 * @param[in] obj            database
 * @param[in] field          name of checked field
 * @param[in] prefix         error
 * @return                   modified database
 **/
SvDBRawObject
Innov8onProviderAdaptField(SvDBRawObject obj, const char *field, SvString prefix);

/**
 * Get error code returned by MW as string.
 *
 * @param[in] errorCode   code of error
 * @return    SvString with error Innov8onProviderSetNeedSuccessCode description, must be released
 */
SvString
Innov8onProviderCreateMessageFromErrorCode(const int errorCode);

/**
 * Set flag which indicates if success code should be provided by MW.
 *
 * @param[in] self                 this provider
 * @param[in] needSuccessCode      value to be set
 */
void
Innov8onProviderSetNeedSuccessCode(Innov8onProvider self, bool needSuccessCode);

/**
 * Returns whether provider needs success code
 *
 * @param[in] self          provider handle
 * @return returns true if flag indicating if provider needs success code is set to true, false otherwise
 **/
bool
Innov8onProviderGetNeedSuccessCode(SvObject self);

/**
 * @}
 **/

#endif
