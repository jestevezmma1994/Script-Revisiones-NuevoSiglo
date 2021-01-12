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

/* App/Applications/CubiTV/Windows/wizard.h */
/* This file should be named QBWizardContext.h */

#ifndef QBWIZARDCONTEXT_H_
#define QBWIZARDCONTEXT_H_

/**
 * @file wizard.h
 * @brief Wizard Context Class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <QBWindowContext.h>
#include <main_decl.h>

/**
 * @defgroup QBWizardContext Firstboot wizard
 * @ingroup CubiTV_windows
 * @{
 *
 * Firstboot wizard allows setting up some options before the main application
 * starts.
 *
 * Options are loaded from JSON config file into @ref QBWizardOption objects
 * and are presented using @ref QBComboBox for each one.
 *
 * There may be more than one screen with options. Each option is assigned to a
 * screen on which it should appear. Each screen is an instance of
 * QBWizardContext with shared data about options and values selected by user.
 *
 * There may be multiple modes, each having its own configuration file. When the
 * user changes mode, then all options are reloaded and widgets are recreated.
 * If modes are configured to have the same options on the first screen, then on
 * mode change the widgets appear to stay untouched.
 *
 * Special options:
 *
 * - "lang" - valueIds of this option must be valid locale accepted by
 *            setlocale. Changing this option changes the language of the
 *            application immitdiately.
 * - "mode" - valueIds must be valid mode names. Changing this option causes
 *            mode change as described above.
 * - "tuner" and "standard" - These options are coupled with QBTuner
 *            module. Value ids of "tuner" option must be a valid tuner names
 *            and value ids of "standard" must be a valid name returned by
 *            QBTunerStandardToString. When tuner option is changed, the
 *            standard option is updated, so that it has only values that are
 *            relevant to selected tuner and are listed in options config file.
 * - "channels" and "epg" - Channel list and EPG can be obtained from two
 *            soures: dvb or middleware (they can be used simoultaneously).
 *            There might be implemented support for many middlewares, but only
 *            one can be used at the same time (you can't have channels from
 *            one middleware and EPG from another). Value ids of both options
 *            are expected to be "dvb", "mwname" or "dvb mwname". When the user
 *            changes channels option, the epg option is updated, so that it
 *            has only values that satisfy the mentioned above criteria and are
 *            listed in options config file.
 */

/**
 * Create wizard window context.
 *
 * This method creates context for the first screen of the wizard. It also
 * loads config from file into QBWizardOption objects.
 *
 * @param[in]   appGlobals  appGlobals handle
 * @return      QBWizardContext handle
 */
QBWindowContext QBWizardContextCreate(AppGlobals appGlobals);

/**
 * @}
 */

#endif /* QBWIZARDCONTEXT_H_ */
