/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef INNOV8ON_UTILS_H_
#define INNOV8ON_UTILS_H_

#include <SvDataBucket2/SvDBRawObject.h>

/**
 * Checks if product is entitled according to Middleware server. (Non-empty 'expires_at' field means we have entitlement)
 *  @note Date provided in expires_at field is not checked in any way! MW is supposed to set expires_at only for entitled objects, STB relies
 *  on MW for entitlement checking. We shouldn't rely on local STB date anyway.
 *
 * @param[in] product product to check for entitlement
 * @return                  true/false
 **/
bool
QBInnov8onProductIsEntitled(SvDBRawObject product);

/**
 * Checks if the offer can be bought be the user.
 * @param[in] offer         offer to check
 * @return true iff offer can be bought
 **/
bool
QBInnov8onOfferIsAvailable(SvDBRawObject offer);

#endif
