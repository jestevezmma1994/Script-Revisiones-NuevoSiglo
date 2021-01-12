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

#ifndef QB_FIXED_VARIANT_CALCULATOR_H_
#define QB_FIXED_VARIANT_CALCULATOR_H_

/**
 * @file QBFixedVariantCalculator.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBVariantCalculator.h"

#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBFixedVariantCalculator QBFixedVariantCalculator class
 * @ingroup CubiTV
 * @{
 **/

/**
 * QBFixedVariantCalculator class.
 *
 * @class QBFixedVariantCalculator
 * @extends SvObject
 **/
typedef struct QBFixedVariantCalculator_ *QBFixedVariantCalculator;

/**
 * Create QBFixedVariantCalculator.
 *
 * @param[in]  variant  result of calculator
 * @param[out] errorOut error info
 *
 * @return created QBFixedVariantCalculator, @c NULL in case of error
 **/
QBFixedVariantCalculator
QBFixedVariantCalculatorCreate(int variant, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif // QB_FIXED_VARIANT_CALCULATOR_H_
