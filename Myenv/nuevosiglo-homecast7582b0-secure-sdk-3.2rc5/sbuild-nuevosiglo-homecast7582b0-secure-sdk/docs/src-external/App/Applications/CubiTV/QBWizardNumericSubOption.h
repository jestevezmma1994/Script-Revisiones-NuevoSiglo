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

/* App/Applications/CubiTV/Windows/wizard/QBWizardNumericSubOptionOption.h */

#ifndef QBWIZARDNUMERICSUBOPTION_H_
#define QBWIZARDNUMERICSUBOPTION_H_

/**
 * @file QBWizardNumericSubOption.h
 * @brief QBWizardNumericSubOption class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>

/**
 * @defgroup QBWizardNumericSubOption Wizard numeric sub-option
 * @ingroup QBWizardContext
 * @{
 */

/**
 * Numeric sub-option for QBWizardBinaryOption.
 *
 * Value -1 is equivalent to binary option being disabled.
 */
struct QBWizardNumericSubOption_s {
    struct SvObject_ super_; ///< super class
    SvValue defaultValue; ///< it's type is SvValueType_integer, default value: -1
    int numericalMax; ///< maximum value this option accepts, default: INT_MAX
};
typedef struct QBWizardNumericSubOption_s* QBWizardNumericSubOption;

/**
 * @return Runtime type identification object representing QBWizardNumericSubOption class.
 */
SvHidden SvType QBWizardNumericSubOption_getType(void);

/**
 * Register JSON helper.
 *
 * JSON helper supports creating QBWizardNumericSubOption objects from data from JSON parser.
 *
 * @param[out]  errorOut    error info
 */
SvHidden void QBWizardNumericSubOptionRegisterJSONHelper(SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* QBWIZARDNUMERICSUBOPTION_H_ */
