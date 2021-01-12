/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2012-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef __QB_REENCRYPTION_MANAGER_H__
#define __QB_REENCRYPTION_MANAGER_H__

#include <fibers/c/fibers.h>
#include <SvCore/SvErrorInfo.h>
#include <QBSchedDesc.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBReencryptionManager.h
 * @brief ReencryptionManager service
 *
 * Service is responsible for converting retained content to reencrypted one.
 * Conversion is done one by one.
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

/**
 * @defgroup QBReencryptionManager Reencryption Manager
 * @ingroup MediaPlayer
 * @{
 */

typedef struct QBReencryptionManager_ *QBReencryptionManager;

/**
 * Creates Reencryption Manager object
 * @param[in] scheduler application scheduler
 * @param[out] errorOut pointer to pass error information
 * @return new Reencryption Manager instance
 */
QBReencryptionManager QBReencryptionManagerCreate(SvScheduler scheduler, SvErrorInfo *errorOut);

/**
 * Starts Reencryption Manager service.
 *
 * @param[in] self Reencryption Manager instance
 */
void QBReencryptionManagerStart(QBReencryptionManager self);

/**
 * Stops Reencryption Manager service.
 *
 * @param[in] self Reencryption Manager instance
 */
void QBReencryptionManagerStop(QBReencryptionManager self);

/**
 * Informs Reencryption Manager that 'desc' shouldn't be process anymore, it probably will be removed shortly.
 *
 * @param[in] self Reencryption Manager instance
 * @param[in] desc pointer to object describing PVR recording
 * @return @c 0 if success, @c -1 in case of error
 */
int QBReencryptionManagerStopUsing(QBReencryptionManager self, const QBSchedDesc *desc);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // __QB_REENCRYPTION_MANAGER_H__
