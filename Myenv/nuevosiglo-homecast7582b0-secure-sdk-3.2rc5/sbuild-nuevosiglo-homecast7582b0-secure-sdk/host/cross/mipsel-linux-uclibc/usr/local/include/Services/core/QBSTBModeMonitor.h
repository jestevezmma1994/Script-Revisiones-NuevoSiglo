/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBSTBMODEMONITOR_H_
#define QBSTBMODEMONITOR_H_

/**
 * @file  QBSTBModeMonitor.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief STB mode monitor API
 **/

#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>

/**
 * @defgroup QBSTBModeMonitor STB mode monitor class
 * @ingroup  CubiTV_services
 *
 * QBSTBModeMonitor is used by application to detect and store information about STB mode change.
 * Stored information about STB mode change is used by StandbyLogic to set correct initial state
 * after reboot caused by upgrade in standby.
 *
 * Example scenario:
 * 1. STB is in standby and received information from MW in call stb.GetConfiguration that STB mode
 * has changed. When STB mode has changed, STB set FIRSTBOOT to YES, so after next reboot STB standby
 * state is set to on and we see wizard or channel scanning screen (standby state depend on
 * FIRSTBOOT setting).
 * 2. In the same time QBSTBModeMonitor detect that STB mode has changed and create marker with
 * indicate this change.
 * 3. When STB is still in standby firmware upgrade occurred. STB reboot when upgrade is finished.
 * 4. After reboot in StandbyLogic we use QBSTBModeMonitorIsSTBModeChanged() method to check if STB
 * mode changed. When STB mode has changed and previously STB was in standby and upgrade occurred
 * than we set initial state to standby.
 * 5. When STB leave standby and user finish wizard or channel scanning then QBSTBModeMonitor
 * removed marker.
 *
 * So by using this service we can prevent situation when application is unnecessarily turned on after
 * upgrade in standby because before upgrade STB mode has changed.
 *
 * @{
 **/
typedef struct QBSTBModeMonitor_ *QBSTBModeMonitor;

/**
 * Create STB mode monitor service.
 *
 * @param[out] errorOut     error info
 * @return                  new STB mode monitor service instance
 **/
QBSTBModeMonitor QBSTBModeMonitorCreate(SvErrorInfo *errorOut);

/**
 * Start STB mode monitor service.
 *
 * @param[in] self          STB mode monitor handle
 * @param[out] errorOut     error info
 **/
void QBSTBModeMonitorStart(QBSTBModeMonitor self,
                           SvErrorInfo *errorOut);

/**
 * Stop STB mode monitor service.
 *
 * @param[in] self          STB mode monitor handle
 * @param[out] errorOut     error info
 **/
void QBSTBModeMonitorStop(QBSTBModeMonitor self,
                          SvErrorInfo *errorOut);

/**
 * Check if STB mode has been changed.
 *
 * @return                  true if STB mode has been changed
 **/
bool QBSTBModeMonitorIsSTBModeChanged(void);

/**
 * @}
 **/

#endif /* QBSTBMODEMONITOR_H_ */
