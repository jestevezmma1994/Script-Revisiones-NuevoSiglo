/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCONAX_TIMESHIFT_MONITOR_H_
#define QBCONAX_TIMESHIFT_MONITOR_H_

#include <fibers/c/fibers.h>

/**
 * @file QBConaxTimeshiftMonitor.h
 * @brief Conax timeshift monitor
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBConaxTimeshiftMonitor Conax timeshift monitor
 * @ingroup CubiTV_services
 * @{
 **/

/**
 * QBConaxTimeshiftMonitor class.
 * It is responsible for monitoring timeshift.
 */
typedef struct QBConaxTimeshiftMonitor_s *QBConaxTimeshiftMonitor;

/**
 * Start Conax timeshift monitor.
 *
 * @param[in] self instance of @link QBConaxTimeshiftMonitor @endlink
 **/
extern void QBConaxTimeshiftMonitorStart(QBConaxTimeshiftMonitor self);

/**
 * Stop Conax timeshift monitor.
 *
 * @param[in] self instance of @link QBConaxTimeshiftMonitor @endlink
 **/
extern void QBConaxTimeshiftMonitorStop(QBConaxTimeshiftMonitor self);

/**
 * Creates Conax timeshift monitor
 * @param[in] scheduler scheduler
 * @return new Conax timeshift monitor instance
 **/
extern QBConaxTimeshiftMonitor QBConaxTimeshiftMonitorCreate(SvScheduler scheduler);

/**
 * @}
 **/

#endif // QBCONAX_TIMESHIFT_MONITOR_H_
