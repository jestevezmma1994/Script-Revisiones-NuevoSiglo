/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef _QBREENCRYPTION_SERVICE_H_
#define _QBREENCRYPTION_SERVICE_H_

#include <fibers/c/fibers.h>
#include <QBSchedDesc.h>

/**
 * @file QBReencryptionService.h
 * @brief Service which manage reencryption process
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

/**
 * @defgroup QBReencryptionService Reencryption Service
 * @ingroup CubiTV_services
 * @{
 */

/**
 * @class QBReencryptionService
 *
 * Currently this service starts reencryption only during active standby.
 *
 */
typedef struct QBReencryptionService_ *QBReencryptionService;

/**
 * Creates reencryption service
 * @param[in] scheduler application scheduler
 * @return new reencryption service instance
 */
QBReencryptionService QBReencryptionServiceCreate(SvScheduler scheduler);

/**
 * Starts reencryption service.
 *
 * @param[in] self reencryption service instance
 */
void QBReencryptionServiceStart(QBReencryptionService self);

/**
 * Stops reencryption service.
 *
 * @param[in] self reencryption service instance
 */
void QBReencryptionServiceStop(QBReencryptionService self);

/**
 * Informs reencryption service that 'desc' shouldn't be used anymore, it probably will be removed shortly.
 *
 * @param[in] self reencryption service instance
 * @param[in] desc pointer to object describing PVR recording
 * @return @c 0 if success, @c -1 in case of error
 */
int QBReencryptionServiceStopUsingRecord(QBReencryptionService self, const QBSchedDesc* desc);

/**
 * Informs that device entering active standby.
 *
 * @param[in] self reencryption service instance
 */
void QBReencryptionServiceEnterStandby(QBReencryptionService self);

/**
 * Informs that device leaving active standby.
 *
 * @param[in] self reencryption service instance
 */
void QBReencryptionServiceLeaveStandby(QBReencryptionService self);

/**
 * @}
 */

#endif // _QBREENCRYPTION_SERVICE_H_
