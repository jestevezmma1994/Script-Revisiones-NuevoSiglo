/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_NETWORK_PROBLEM_DIALOG_H_
#define QB_NETWORK_PROBLEM_DIALOG_H_

/**
 * @file networkProblemDialog.h Network Problem Dialog API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "countdownDialog.h"

#include <Services/core/QBNetworkWatcher.h>
#include <QBApplicationController.h>
#include <CUIT/Core/types.h>

/**
 * Create Network Problem dialog.
 *
 * @param[in] res            application handle
 * @param[in] controller     application controller handle
 * @return                   newly created dialog or NULL in case of error
 **/
extern QBCountdownDialog
QBNetworkProblemDialogCreate(SvApplication res,
                             QBApplicationController controller);

/**
 * Set diagnosis enum type to Network Problem dialog.
 * The diagnosis will be displayed in a human-readable form.
 *
 * @param[in] self           network problem dialog handle
 * @param[in] diagnosis      diagnosis to be displayed as a description
 * @return                   newly created dialog or NULL in case of error
 **/
extern void
QBNetworkProblemDialogSetDiagnosis(QBCountdownDialog self, QBNetworkWatcherOverallDiagnosis diagnosis);


#endif
