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

#ifndef QBWATCHDOGSERVICE_H_
#define QBWATCHDOGSERVICE_H_

/**
 * @file watchdog.h QBWatchdog class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <fibers/c/fibers.h>
#include <SvCore/SvErrorInfo.h>

/**Max watchdog timeout in seconds which can be set*/
#define MAX_WATCHDOG_TIMEOUT            300
/**Timeout in second to reboot after call @c QBWatchdogReboot*/
#define WATCHDOG_REBOOT_TIMEOUT         2
/**Longer timeout (in seconds) to allow application to close itself properly*/
#define WATCHDOG_LONG_TIMEOUT_SEC       10
/**Factor how fast feed watchdog, 1000 is the same as watchdog timeout, 500 two times faster*/
#define WATCHDOG_FEED_FACTOR_TIMEOUT    (1000 / 2)

typedef struct QBWatchdog_t* QBWatchdog;

/**
 * Create a QBWatchdog object.
 *
 * @param[out] errorOut error info
 * @return @c QBWatchdog object, @c NULL if failed
 **/
QBWatchdog QBWatchdogCreate(SvErrorInfo *errorOut);

/**
 * Start watchdog and set timeout.
 *
 * @param[in] self watchdog instance
 * @param[in] scheduler scheduler instance
 * @param[in] timeout number of seconds without feeding watchdog
 * @param[out] errorOut error info
 **/
void QBWatchdogStart(QBWatchdog self, SvScheduler scheduler, unsigned timeout, SvErrorInfo *errorOut);

/**
 * Stop watchdog.
 *
 * @param[in] self watchdog instance
 * @param[out] errorOut error info
 **/
void QBWatchdogStop(QBWatchdog self, SvErrorInfo *errorOut);

/**
 * Set min period of feeding the watchdog.
 *
 * This function configures watchdog to require a call to
 * QBWatchdogContinue() at least once each @c timeout seconds.
 * When this condition is not met, device is automatically
 * restarted.
 *
 * @note watchdog is feed in this method after set timeout
 * @note timeout must be in range from 0 to MAX_WATCHDOG_TIMEOUT
 *
 * @param[in] self watchdog instance
 * @param[in] timeout number of seconds of feeding watchdog
 * @param[out] errorOut error info
 **/
void QBWatchdogSetTimeout(QBWatchdog self, unsigned timeout, SvErrorInfo *errorOut);

/**
 * Feed the watchdog.
 *
 * This method should be call at least once each @c timeout seconds to avoid reboot
 *
 * @param[in] self watchdog instance
 * @param[out] errorOut error info
 **/
void QBWatchdogContinue(QBWatchdog self, SvErrorInfo *errorOut);

/**
 * Sets the watchdog to reboot system after default number of seconds.
 *
 * @param[in] self object handle
 * @param[out] errorOut error info
 **/
void QBWatchdogReboot(QBWatchdog self, SvErrorInfo *errorOut);

/**
 * Sets the watchdog to reboot system after given numer of seconds.
 *
 * The reboot order cannot be cancelled using functions QBWatchdogStop or QBWatchdogDestroy.
 * Only call of QBWatchdogContinue can renew the reboot timeout.
 *
 * @param[in] self object handle
 * @param[in] timeout number of seconds to reboot
 * @param[out] errorOut error info
 **/
void QBWatchdogRebootAfterTimeout(QBWatchdog self, unsigned timeout, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif
