/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
 **
 ** Any rights which are not expressly granted in this License are entirely and
 ** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
 ** modify, translate, reverse engineer, decompile, disassemble, or create
 ** derivative works based on this Software. You may not make access to this
 ** Software available to others in connection with a service bureau,
 ** application service provider, or similar business, or make any other use of
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#ifndef QBDISKACTIVATOR_H_
#define QBDISKACTIVATOR_H_

/**
 * @file QBDiskActivator.h Disk Activator class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <fibers/c/fibers.h>

#include <Services/QBStandbyAgent.h>
#include <Services/core/hotplugMounts.h>

/**
* @defgroup QBDiskActivator Disk activator service
* @ingroup CubiTV_services
* @{
*
* This service prevents hard disk against entering standby mode.
*/

/**
* This service prevents hard disk against entering standby mode.
*
* @class QBDiskActivator
*
* This service prevents hard disk against entering standby mode when CubiTV is operating
* (to keep it ready for recording). Nevertheless to reduce power consumption disk is allowed
* to enter standby when CubiTV is in standby. If disk enters standby together with CubitTV
* it is woken up prior to any scheduled recording so that the whole event is recorded.
*
* @note In some rare (hardware and configuration depended) cases it may take long time before
* hard disk gets up from standby mode. In such cases all blocking IO operations may lead to
* application freezes or even termination by watchdog. This service prevents against such situations.
*/
typedef struct QBDiskActivator_ *QBDiskActivator;

/**
 * Create disk activator service.
 *
 * @param[in] standbyAgent          standby agent
 * @param[in] hotplugMountAgent     hotplug mount agent
 * @return                          created QBDiskActivator, @c NULL in case of error
 */
QBDiskActivator QBDiskActivatorCreate(QBStandbyAgent standbyAgent, QBHotplugMountAgent hotplugMountAgent);

/**
 * Start disk activator service.
 *
 * @param[in] self        QBDiskActivator object
 */
void QBDiskActivatorStart(QBDiskActivator self);

/**
 * Stop disk activator service.
 *
 * @param[in] self        QBDiskActivator object
 */
void QBDiskActivatorStop(QBDiskActivator self);

/**
* @}
**/

#endif /* QBDISKACTIVATOR_H_ */
