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

/* App/Applications/CubiTV/Windows/wizardInternal.h */
// This file should be named QBWizardContextInternal.h

#ifndef QBWIZARDCONTEXTINTERNAL_H_
#define QBWIZARDCONTEXTINTERNAL_H_

/**
 * @file wizardInternal.h
 * @brief Wizard Context Package API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <main_decl.h>
#include <QBWindowContext.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvValue.h>

#include "wizard/QBWizardOption.h"

/**
 * @addtogroup QBWizardContext
 * @{
 */

typedef struct QBWizardContextSharedData_s* QBWizardContextSharedData;

/**
 * Context of single Wizard screen.
 *
 * Wizard is a sequence of screens with various options. Every screen is
 * managed by an instance of QBWizardContext.
 *
 * There are 3 sets of config values: currentMode, allModes and default.
 * - currentMode contains values selected by the user or in the current mode.
 *   Values are initialized with option->defaultValueId.
 * - allModes contains the same data as currentMode plus values selected by the
 *   user for options that were in other modes.
 * - default containt values for options that should be user for options not
 *   shown to the user.
 *
 * This allows us to handle situation where one option is available in one mode
 * and not in the other. When the user selects some options and then changes
 * the mode, values for options that disappeared from menu are taken from
 * defaultValues set. But when he changes mode back, previously selected values
 * are restored from allModesValues.
 */
struct QBWizardContext_s {
    struct QBWindowContext_t super_; ///< super class
    AppGlobals appGlobals; ///< AppGlobals handle
    unsigned screenNumber; ///< number of current screen

    bool modeComboboxEnabled; ///< set if hybrid mode combobox should be display in wizard
    bool isPollPopupSet; ///< set when PollPopup was required

    QBWizardContextSharedData sharedData; ///< data shared between all instances
};

typedef struct QBWizardContext_s* QBWizardContext;


/**
 * Data shared between all instances of QBWizardContext
 */
struct QBWizardContextSharedData_s {
    struct SvObject_ super_; ///< super class

    /**
     * Array of QBWizardOption. Contains all options obtained from config
     * file. If loading fails it's empty, not NULL.
    */
    SvArray currentModeOptions;

    unsigned allScreens; ///< number of all screens in the wizard

    struct QBWizardContextValues {
        /**
         * Maps option id to value id. See \ref QBWizardOption for details.
         * key: (SvString) QBWizardOption->id
         * value: (SvString) valueId
         */
        SvHashTable idToOption;
        /**
         * Maps option id to value of suboption.
         * key: (SvString) QBWizardOption->id
         * value: (SvValue)
         */
        SvHashTable idToSubOption;
    } allModesValues, ///< Values of all options in all modes.
      currentModeValues, ///< Values of options available in current mode.
      defaultValues; ///< Default values for options not specified in currentModeOptions.
};


/**
 * Create next wizard screen's context.
 *
 * @param[in]   prevContext Context of previous screen, which shares allOptions, idToOption and idToSuboption with the new one.
 * @return                  QBWizardContext handle
 */
SvHidden QBWindowContext QBWizardContextCreateNextScreen(QBWizardContext prevContext);

/**
 * Reloads allOptions, updates allScreens, and currentModeValues.
 *
 * @param[in]   self    QBWizardContext handle
 */
SvHidden void QBWizardContextModeChanged(QBWizardContext self);

/**
 * Select options that belong to the current screen
 *
 * @param[in]   self    QBWizardContext handle
 * @return              Array of QBWizardOption
 */
SvHidden SvArray QBWizardContextSelectOptionsOfCurrentScreen(QBWizardContext self);

/**
 * Find option with given id.
 *
 * @param[in]   self        QBWizardContext handle
 * @param[in]   optionId    id of an option
 * @return                  option, or NULL if not found.
 */
SvHidden QBWizardOption QBWizardContextGetOptionById(QBWizardContext self, SvString optionId);

/**
 * Store options value in context.
 *
 * @param[in]   self        QBWizardContext handle
 * @param[in]   optionId    QBWizardOption->id
 * @param[in]   valueId     value id
 */
SvHidden void QBWizardContextSetOptionValueId(QBWizardContext self, SvString optionId, SvString valueId);

/**
 * Get options value stored in context.
 *
 * @param[in]   self        QBWizardContext handle
 * @param[in]   optionId    QBWizardOption->id
 * @return                  value id
 */
SvHidden SvString QBWizardContextGetOptionValueId(QBWizardContext self, SvString optionId);

/**
 * Get options value stored in context, or default if not found.
 *
 * @param[in]   self        QBWizardContext handle
 * @param[in]   optionId    QBWizardOption->id
 * @return                  value id
 */
SvHidden SvString QBWizardContextGetOptionValueIdOrDefault(QBWizardContext self, SvString optionId);

/**
 * Store sub-options value in context.
 *
 * @param[in]   self        QBWizardContext handle
 * @param[in]   optionId    QBWizardOption->id
 * @param[in]   value       value
 */
SvHidden void QBWizardContextSetSubOptionValue(QBWizardContext self, SvString optionId, SvValue value);

/**
 * Get sub-options value from context.
 *
 * @param[in]   self        QBWizardContext handle
 * @param[in]   optionId    QBWizardOption->id
 * @return                  value
 */
SvHidden SvValue QBWizardContextGetSubOptionValue(QBWizardContext self, SvString optionId);

/**
 * Get sub-options value from context, or default if not found.
 *
 * @param[in]   self        QBWizardContext handle
 * @param[in]   optionId    QBWizardOption->id
 * @return                  value
 */
SvHidden SvValue QBWizardContextGetSubOptionValueOrDefault(QBWizardContext self, SvString optionId);

/**
 * Remove sub-options value from context.
 *
 * @param[in]   self        QBWizardContext handle
 * @param[in]   optionId    QBWizardOption->id
 */
SvHidden void QBWizardContextRemoveSubOptionValue(QBWizardContext self, SvString optionId);

/**
 * Creates new idToOption hash table, which contains values selected by the
 * user and defaults for options that weren't shown.
 *
 * @param[in]   self        QBWizardContext handle
 * @return                  new idToOption hash table
 */
SvHidden SvHashTable QBWizardContextCreateIdToOptionComplementedWithDefaults(QBWizardContext self);

/**
 * Creates new idToSubOption hash table, which contains values selected by the
 * user and defaults for options that weren't shown.
 *
 * @param[in]   self        QBWizardContext hanlde
 * @return                  new idToSubOption hash table
 */
SvHidden SvHashTable QBWizardContextCreateIdToSubOptionComplementedWithDefaults(QBWizardContext self);

/**
 * @}
 */

#endif /* QBWIZARDCONTEXTINTERNAL_H_ */
