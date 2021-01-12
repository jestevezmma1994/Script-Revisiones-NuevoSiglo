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

#ifndef QB_PLATFORM_WATCHDOG_H_
#define QB_PLATFORM_WATCHDOG_H_

/**
 * @file QBPlatformWatchdog.h Watchdog control API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformWatchdog Watchdog control
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Set min period of feeding the watchdog.
 *
 * This function configures watchdog to require a call to
 * QBPlatformWatchdogContinue() at least once each @a n seconds.
 * When this condition is not met, device is automatically
 * restarted.
 *
 * @param[in] n         max time between calls to QBPlatformWatchdogContinue()
 **/
extern void QBPlatformWatchdogSetTimeout(int n);

/**
 * Activate the watchdog.
 **/
extern void QBPlatformWatchdogStart(void);

/**
 * Deactivate and close the watchdog.
 **/
extern void QBPlatformWatchdogStop(void);

/**
 * Close the watchdog without deactivatinig it.
 **/
extern void QBPlatformWatchdogCloseWithoutDeactivation(void);

/**
 * Feed the watchdog, see QBPlatformWatchdogSetTimeout().
 **/
extern void QBPlatformWatchdogContinue(void);

/**
 * Called if a error occurs in watchdog functionality.
 * @param[in] error     description
 **/
extern void QBPlatformWatchdogReportError(SvString error);

/**
 * Returns list of all errors reported since the previous use of this
 * function reported by QBPlatformWatchdogReportError().
 * @return SvArray of all errors
 **/
extern SvArray QBPlatformWatchdogGetErrors(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
