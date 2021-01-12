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

#ifndef TRAXIS_WEB_VOD_TRANSACTIONS_H_
#define TRAXIS_WEB_VOD_TRANSACTIONS_H_

/**
 * @file TraxisWebVoDTransactions.h
 * @brief Traxis.Web VoD transactions
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <TraxisWebClient/TraxisWebVoDProvider.h>


/**
 * @defgroup TraxisWebVoDTransactions Traxis.Web VoD transactions
 * @ingroup TraxisWebClient
 * @{
 **/

/**
 * Traxis.Web VoD 'GetOffer' transaction class.
 *
 * This class performs Traxis.Web VoD 'GetOffer' transaction,
 * which contains of two requests to the Traxis.Web service:
 * first one gets the list of feature contents for given title,
 * second one gets the list of products, including offer prices,
 * for the feature content.
 *
 * It implements @ref QBContentMgrTransaction.
 *
 * @class TraxisWebVoDGetOfferTransaction
 * @extends SvObject
 **/
typedef struct TraxisWebVoDGetOfferTransaction_ *TraxisWebVoDGetOfferTransaction;

/**
 * Get runtime type identification object representing
 * type of Traxis.Web VoD 'GetOffer' transaction class.
 *
 * @return Traxis.Web VoD 'GetOffer' transaction class
 **/
extern SvType
TraxisWebVoDGetOfferTransaction_getType(void);

/**
 * Create Traxis.Web VoD 'GetOffer' transaction.
 *
 * @memberof TraxisWebVoDGetOfferTransaction
 *
 * @param[in] provider  Traxis.Web provider handle
 * @param[in] title     handle to a title object to get product offer for
 * @param[out] errorOut error info
 * @return              created transaction, @c NULL in case of error
 **/
extern TraxisWebVoDGetOfferTransaction
TraxisWebVoDGetOfferTransactionCreate(TraxisWebVoDProvider provider,
                                      SvDBRawObject title,
                                      SvErrorInfo *errorOut);

/**
 * Get title object.
 *
 * @memberof TraxisWebVoDGetOfferTransaction
 *
 * @param[in] self      Traxis.Web VoD 'GetOffer' transaction handle
 * @param[out] errorOut error info
 * @return              handle to the title object this transaction has
 *                      been created with, @c NULL in case of error
 **/
extern SvDBRawObject
TraxisWebVoDGetOfferTransactionGetTitle(TraxisWebVoDGetOfferTransaction self,
                                        SvErrorInfo *errorOut);

/**
 * Get an array of contents for the title.
 *
 * @memberof TraxisWebVoDGetOfferTransaction
 *
 * @param[in] self      Traxis.Web VoD 'GetOffer' transaction handle
 * @param[out] errorOut error info
 * @return              handle to an array of contents, @c NULL in case of error
 **/
extern SvImmutableArray
TraxisWebVoDGetOfferTransactionGetContents(TraxisWebVoDGetOfferTransaction self,
                                           SvErrorInfo *errorOut);

/**
 * Get feature content found by 'GetOffer' transaction.
 *
 * @memberof TraxisWebVoDGetOfferTransaction
 *
 * @param[in] self      Traxis.Web VoD 'GetOffer' transaction handle
 * @param[out] errorOut error info
 * @return              feature content object handle, @c NULL in case of error
 **/
extern SvDBRawObject
TraxisWebVoDGetOfferTransactionGetFeatureContent(TraxisWebVoDGetOfferTransaction self,
                                                 SvErrorInfo *errorOut);

/**
 * Get an array of products (including offer prices) for feature content.
 *
 * @memberof TraxisWebVoDGetOfferTransaction
 *
 * @param[in] self      Traxis.Web VoD 'GetOffer' transaction handle
 * @param[out] errorOut error info
 * @return              handle to an array of products, @c NULL in case of error
 **/
extern SvImmutableArray
TraxisWebVoDGetOfferTransactionGetProducts(TraxisWebVoDGetOfferTransaction self,
                                           SvErrorInfo *errorOut);


/**
 * Traxis.Web VoD 'Purchase' transaction class.
 *
 * This class performs Traxis.Web VoD 'Purchase' transaction.
 * It implements @ref QBContentMgrTransaction.
 *
 * @class TraxisWebVoDPurchaseTransaction
 * @extends SvObject
 **/
typedef struct TraxisWebVoDPurchaseTransaction_ *TraxisWebVoDPurchaseTransaction;

/**
 * Get runtime type identification object representing
 * type of Traxis.Web VoD 'Purchase' transaction class.
 *
 * @return Traxis.Web VoD 'Purchase' transaction class
 **/
extern SvType
TraxisWebVoDPurchaseTransaction_getType(void);

/**
 * Create Traxis.Web VoD 'Purchase' transaction.
 *
 * @memberof TraxisWebVoDPurchaseTransaction
 *
 * @param[in] provider  Traxis.Web provider handle
 * @param[in] content   handle to a feature content object
 * @param[in] product   handle to a product object to purchase
 * @param[out] errorOut error info
 * @return              created transaction, @c NULL in case of error
 **/
extern TraxisWebVoDPurchaseTransaction
TraxisWebVoDPurchaseTransactionCreate(TraxisWebVoDProvider provider,
                                      SvDBRawObject content,
                                      SvDBRawObject product,
                                      SvErrorInfo *errorOut);

/**
 * Check whether purchase transaction succeeded.
 *
 * @memberof TraxisWebVoDPurchaseTransaction
 *
 * @param[in] self      Traxis.Web VoD 'Purchase' transaction handle
 * @param[out] errorOut error info
 * @return              @c true if product had been purchased,
 *                      @c false otherwise
 **/
extern bool
TraxisWebVoDPurchaseTransactionIsPurchased(TraxisWebVoDPurchaseTransaction self,
                                           SvErrorInfo *errorOut);

/**
 * Get feature content updated after purchase.
 *
 * @memberof TraxisWebVoDPurchaseTransaction
 *
 * @param[in] self      Traxis.Web VoD 'Purchase' transaction handle
 * @param[out] errorOut error info
 * @return              feature content object handle, @c NULL in case of error
 **/
extern SvDBRawObject
TraxisWebVoDPurchaseTransactionGetFeatureContent(TraxisWebVoDPurchaseTransaction self,
                                                 SvErrorInfo *errorOut);

/**
 * Set Coupon's TraxisWeb identifier which will be used to purchase content.
 *
 * @param[in] self          Traxis.Web VoD "Purchase" transaction handler
 * @param[in] couponId      coupon's TraxisWeb identifier
 */
extern void
TraxisWebVoDPurchaseTransactionSetCoupon(TraxisWebVoDPurchaseTransaction self,
                                         SvString couponId);

/**
 * Traxis.Web VoD 'WishList' transaction class.
 *
 * This class performs Traxis.Web VoD 'WishList' transaction.
 * Request must be started by @ref QBContentMgrTransaction interface.
 * If there's a need to start request again, previous request need to be
 * cancelled by @ref QBContentMgrTransaction interface.
 * Starting request without previous cancelletion will end with no effect.
 *
 * @class TraxisWebVoDWishListModificationTransaction
 * @extends SvObject
 * @implements QBContentMgrTransaction
 **/
typedef struct TraxisWebVoDWishListModificationTransaction_ *TraxisWebVoDWishListModificationTransaction;

/**
 * Wishlist actions
 **/
typedef enum TraxisWebVoDWishListAction_e {
    /// add wishlist
    TraxisWebVoDWishListActionAdd = 0,
    /// remove wishlist
    TraxisWebVoDWishListActionRemove,
    /// last action
    TraxisWebVoDWishListActionMax
}TraxisWebVoDWishListAction;

/**
 * Get runtime type identification object representing
 * type of Traxis.Web VoD 'WishList' transaction class.
 *
 * @return Traxis.Web VoD 'WishList' transaction class
 **/
extern SvType
TraxisWebVoDWishListModificationTransaction_getType(void);

/**
 * Create Traxis.Web VoD 'WishList' transaction.
 *
 * @memberof TraxisWebVoDWishListModificationTransaction
 *
 * @param[in] provider  Traxis.Web provider handle
 * @param[in] titleId   object id that attachment to WishList will be changed
 * @param[in] action    enum that identifies request type
 * @param[out] errorOut error info
 * @return              created transaction, @c NULL in case of error
 **/
extern TraxisWebVoDWishListModificationTransaction
TraxisWebVoDWishListModificationTransactionCreate(TraxisWebVoDProvider provider,
                                                  SvString titleId,
                                                  TraxisWebVoDWishListAction action,
                                                  SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
