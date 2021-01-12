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

#ifndef QBCONAX_RETENTION_LIMIT_MONITOR_H_
#define QBCONAX_RETENTION_LIMIT_MONITOR_H_

#include <main_decl.h>
#include <QBPVRRecording.h>
#include <QBICSmartcardURIData.h>

#include <SvPlayerManager/SvPlayerTask.h> // TEMP REMOVE ME

/**
 * @file QBConaxRetentionLimitMonitor.h
 * @brief Conax retention limit monitor
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBConaxRetentionLimitMonitor Conax retention limit monitor
 * @ingroup CubiTV_services
 * @{
 **/

/**
 * QBConaxRetentionLimitMonitor class.
 * It is responsible for monitoring currently displayed content.
 * If content does not satisfies retention limit restrictions, it is blocked.
 */
typedef struct QBConaxRetentionLimitMonitor_s *QBConaxRetentionLimitMonitor;

/**
 * Start Conax retention limit monitor.
 *
 * @param[in] self instance of @link QBConaxRetentionLimitMonitor @endlink
 **/
extern void QBConaxRetentionLimitMonitorStart(QBConaxRetentionLimitMonitor self);

/**
 * Stop Conax retention limit monitor.
 *
 * @param[in] self instance of @link QBConaxRetentionLimitMonitor @endlink
 **/
extern void QBConaxRetentionLimitMonitorStop(QBConaxRetentionLimitMonitor self);

/**
 * Creates Conax retention limit monitor
 * @param[in] appGlobals appGlobals
 * @return new Conax retention limit monitor instance
 **/
extern QBConaxRetentionLimitMonitor QBConaxRetentionLimitMonitorCreate(AppGlobals appGlobals);

/**
 * Set current URI data.
 * @param[in] self instance of @link QBConaxRetentionLimitMonitor @endlink
 * @param[in] uriData URI data
 */
extern void QBConaxRetentionLimitMonitorSetURIData(QBConaxRetentionLimitMonitor self, QBICSmartcardURIData uriData);

/**
 * Set current recording, it there is no such one set NULL.
 * @param[in] self instance of @link QBConaxRetentionLimitMonitor @endlink
 * @param[in] rec recording
 */
extern void QBConaxRetentionLimitMonitorSetRecording(QBConaxRetentionLimitMonitor self, QBPVRRecording rec);

/**
 * @}
 **/


#endif // QBCONAX_RETENTION_LIMIT_MONITOR_H_

