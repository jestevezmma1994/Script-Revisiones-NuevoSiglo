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

#ifndef QB_BUFFER_DURATION_VARIANT_CALCULATOR_H_
#define QB_BUFFER_DURATION_VARIANT_CALCULATOR_H_

/**
 * @file QBBufferDurationVariantCalculator.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBVariantCalculator.h"
#include "QBVariantProvider.h"

#include <SvPlayerKit/QBSmoothStreaming/QBVariantCalculator/QBBufferDurationVariantCalculatorConf.h>

#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBBufferDurationVariantCalculator QBBufferDurationVariantCalculator class
 * @ingroup CubiTV
 * @{
 **/

/**
 * QBBufferDurationVariantCalculator class.
 *
 * @class QBBufferDurationVariantCalculator
 * @extends SvObject
 **/
typedef struct QBBufferDurationVariantCalculator_ *QBBufferDurationVariantCalculator;

/**
 * Create QBBufferDurationVariantCalculator.
 *
 * @param[in]  variantProvider object that implements QBVariantProviderIface.
 *                             It is caller responsibility to ensure that variantProvider
 *                             exists thought QBBufferDurationVariantCalculator lifetime
 * @param[in]  config          bufferDuration config
 * @param[out] errorOut        error info
 * @return created QBBufferDurationVariantCalculator, @c NULL in case of error
 **/
QBBufferDurationVariantCalculator
QBBufferDurationVariantCalculatorCreate(SvObject variantProvider,
                                        QBBufferDurationVariantCalculatorConf config,
                                        SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif // QB_BUFFER_DURATION_VARIANT_CALCULATOR_H_
