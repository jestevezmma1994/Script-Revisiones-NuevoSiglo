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

/* App/Applications/CubiTV/Windows/wizard/QBWizardOption.h */

#ifndef QBWIZARDOPTION_H_
#define QBWIZARDOPTION_H_

/**
 * @file QBWizardOption.h
 * @brief QBWizardOption class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvString.h>

/**
 * @defgroup QBWizardOption Wizard option
 * @ingroup QBWizardContext
 * @{
 */

/**
 * Wizard option
 *
 * This class is used to store config obtained from JSON file.
 *
 * example JSON:
 * {
 *     "__jsonclass__": [
 *         "QBWizardOption",
 *         []
 *     ],
 *     "i18n_gettext": [ "caption", "automatic", "manual" ],
 *     "id": "scan",
 *     "screenNumber": 1,
 *     "caption": "Scan Type",
 *     "description": "Option description",
 *     "default": "automatic",
 *     "ids": {
 *         "automatic": "Automatic",
 *         "manual": "Manual"
 *     }
 * }
 */
struct QBWizardOption_s {
    struct SvObject_ super_; ///< super class
    SvString id; ///< Arbitrary string uniquely identifying option
    SvString caption; ///< Translatable string describing option to the user
    SvString description; ///< Option description. May be NULL.

    /**
     * Possible values for this option.
     * - key: (SvString) Arbitrary string uniquely (within this option) identifying value (valueId for short)
     * - value: (SvString) Translatable string describing value to the user
     */
    SvHashTable idToCaption;
    SvString defaultValueId; ///< id of default value. May be NULL.
    unsigned screenNumber; ///< Number of screen this option should appear on. Default: 0.
};
typedef struct QBWizardOption_s* QBWizardOption;

/**
 * @return Runtime type identification object representing QBWizardOption class.
 */
SvHidden SvType QBWizardOption_getType(void);

/**
 * Register JSON helper.
 *
 * JSON helper supports creating QBWizardOption objects from data from JSON parser.
 *
 * @param[out]  errorOut    error info
 */
SvHidden void QBWizardOptionRegisterJSONHelper(SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* QBWIZARDOPTION_H_ */
