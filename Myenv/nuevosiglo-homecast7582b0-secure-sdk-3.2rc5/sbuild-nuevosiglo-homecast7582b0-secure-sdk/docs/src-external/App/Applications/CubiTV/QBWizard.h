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

/* App/Applications/CubiTV/Windows/wizard/QBWizard.h */

#ifndef QBWIZARD_H_
#define QBWIZARD_H_

/**
 * @file QBWizard.h
 * @brief QBWizard class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <CUIT/Core/types.h>
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvArray.h>

#include "../wizardInternal.h"


/**
 * @defgroup QBWizard Wizard window
 * @ingroup QBWizardContext
 * @{
 */

/**
 * Create wizard window.
 *
 * @param[in] ctx   QBWizardContext handle
 * @param[in] app   CUIT application handle
 * @return          wizard window handle or NULL in case of error
 */
SvHidden SvWindow QBWizardWindowCreate(QBWizardContext ctx, SvApplication app);

/**
 * Reinitialize wizard window.
 *
 * @param[in] window    wizard window handle
 * @param[in] itemList  array of SvStrings describing items to reinitialize
 *                      (currently supported is only "CustomerLogo") or NULL to
 *                      reinitialize everything.
 */
SvHidden void QBWizardWindowReinitialize(SvWindow window, SvArray itemList);

/**
 * @}
 */

#endif /* QBWIZARD_H_ */
