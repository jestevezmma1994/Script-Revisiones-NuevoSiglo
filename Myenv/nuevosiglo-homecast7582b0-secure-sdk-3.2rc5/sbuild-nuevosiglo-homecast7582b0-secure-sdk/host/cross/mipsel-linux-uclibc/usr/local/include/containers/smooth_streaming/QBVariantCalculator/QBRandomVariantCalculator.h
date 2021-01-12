/*****************************************************************************
** TiVo Poland Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2019 TiVo Poland Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by TiVo Poland Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from TiVo Poland Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** TiVo Poland Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_RANDOM_VARIANT_CALCULATOR_H_
#define QB_RANDOM_VARIANT_CALCULATOR_H_

/**
 * @file QBRandomVariantCalculator.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBVariantCalculator.h"
#include "QBVariantProvider.h"

#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBRandomVariantCalculator QBRandomVariantCalculator class
 * @ingroup CubiTV
 * @{
 **/

/**
 * QBRandomVariantCalculator class.
 *
 * @class QBRandomVariantCalculator
 * @extends SvObject
 **/
typedef struct QBRandomVariantCalculator_ *QBRandomVariantCalculator;

/**
 * Create QBRandomVariantCalculator.
 *
 * @param[in]  variantProvider object that implements QBVariantProviderIface.
 *                             It is caller responsibility to ensure that variantProvider
 *                             exists thought QBRandomVariantCalculator lifetime
 * @param[out] errorOut        error info
 *
 * @return created QBRandomVariantCalculator, @c NULL in case of error
 **/
QBRandomVariantCalculator
QBRandomVariantCalculatorCreate(SvObject variantProvider, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif // QB_RANDOM_VARIANT_CALCULATOR_H_
