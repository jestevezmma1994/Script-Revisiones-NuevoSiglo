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

#ifndef INNOV8ONTRANSACTION_H_
#define INNOV8ONTRANSACTION_H_

/**
 * @file innov8onTransaction.h
 * @brief Innov8on transaction
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <Middlewares/productPurchase.h>
#include <QBContentManager/Innov8onProviderParams.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <main_decl.h>
#include <stdbool.h>

/**
 * @defgroup QBInnov8onPurchaseTransaction Innov8on purchase transaction class
 * @ingroup CubiTV_middleware_support
 * @{
 *
 * Innov8on purchase transaction.
 **/
typedef struct QBInnov8onPurchaseTransaction_ *QBInnov8onPurchaseTransaction;


/**
 * Innov8on purchase transaction Params
 **/
typedef struct QBInnov8onPurchaseTransactionParams_ {
    /// flag indicating if extended error code in response from MW is needed
    bool retCodeRequired;
    /// flag indicating if price is required for the product being purchased
    bool isPriceRequired;
    /// price for the product
    int price;
} QBInnov8onPurchaseTransactionParams;

/**
 * Create Innov8onTransaction.
 *
 * @param[in]  provider service provider
 * @param[in]  offerId  id of the offer from which trasaction will be created
 * @param[in]  params   parameters to create transaction
 * @param[out] errorOut error output
 *
 * @returns    Newly created object or NULL in case of failure.
 **/
QBInnov8onPurchaseTransaction QBInnov8onPurchaseTransactionCreate(SvObject provider,
                                                                  int offerId,
                                                                  QBInnov8onPurchaseTransactionParams params,
                                                                  SvErrorInfo *errorOut);

/**
 * Set flag to mark if price is requiered and set price itself.
 *
 * @param[in] self            purchase transaction object handle
 * @param[in] price           price for the trasaction
 * @param[in] isPriceRequired flag which indicates if price is required for the transaction
 **/
void QBInnov8onPurchaseTransactionSetPrice(QBInnov8onPurchaseTransaction self, int price, bool isPriceRequired);

/**
 * Create transaction request.
 *
 * @param[in] appGlobals Application globals handle
 * @param[in] self       purchase transaction object handle
 *
 * @returns              Newly created object or NULL in case of failure.
 **/
SvObject QBInnov8onPurchaseTransactionCreateRequest(AppGlobals appGlobals, QBInnov8onPurchaseTransaction self);

/**
 * @}
 **/

#endif /* INNOV8ONTRANSACTION_H_ */
