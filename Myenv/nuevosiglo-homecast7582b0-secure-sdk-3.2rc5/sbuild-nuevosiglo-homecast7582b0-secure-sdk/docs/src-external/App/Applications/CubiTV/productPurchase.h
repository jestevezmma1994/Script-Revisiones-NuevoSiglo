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

#ifndef PRODUCTPURCHASE_H_
#define PRODUCTPURCHASE_H_

/**
 * @file productPurchase.h
 * @brief Traxis/Innov8on transactions support
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvGenericObject.h>
#include <CUIT/Core/types.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <main_decl.h>

/**
 * @defgroup QBProductPurchase Product purchase class
 * @ingroup CubiTV_middleware_support
 * @{
 **/

typedef struct QBProductPurchase_t *QBProductPurchase;

/**
 * Create purchase transaction
 *
 * @param[in] transaction object implementing QBContentMgrTransaction interface
 * @param[in] product object representing purchased product
 * @param[in] appGlobals
 * @return new purchase transaction
 **/
QBProductPurchase
QBProductPurchaseNew(SvGenericObject transaction,
                     SvGenericObject product,
                     AppGlobals appGlobals);

/**
 * Set listener to be notified on transaction status changes
 *
 * @param[in] self   product purchase handle
 * @param[in] listener  object implementing QBContentMgrTransactionListener interface
 **/
void
QBProductPurchaseSetListener(QBProductPurchase self,
                             SvGenericObject listener);

/**
 * Returns handle to product that is being purchased.
 *
 * @param[in] self  product purchase handle
 * @return product being purchased
 **/
SvDBRawObject
QBProductPurchaseGetProduct(QBProductPurchase self);

/**
 * Cancels the purchase flow, but does not guarantee that the product
 * was not purchased, if called too late.
 *
 * @param[in] self  product purchase handle
 **/
void
QBProductPurchaseCancel(QBProductPurchase self);

/**
 * Creates an error dialog based on appropriate settings. The error information
 * must be provided.
 *
 * @param[in] appGlobals    CubiTV application
 * @param[in] title         title of error dialog
 * @param[in] message       message of error dialog
 * @return QBDialog instance
 **/
SvWidget
QBProductPurchaseCreateErrorDialog(AppGlobals appGlobals, const char *title, const char *message);

/**
 * @}
 **/

#endif /* PRODUCTPURCHASE_H_ */
