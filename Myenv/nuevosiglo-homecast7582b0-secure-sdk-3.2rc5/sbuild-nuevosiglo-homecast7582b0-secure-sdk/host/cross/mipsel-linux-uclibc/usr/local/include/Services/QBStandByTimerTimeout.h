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

#ifndef QBSTANDBYTIMERTIMEOUT_H_
#define QBSTANDBYTIMERTIMEOUT_H_

/**
 * @file QBStandByTimerTimeout.h Timer service
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>
#include <SvFoundation/SvGenericObject.h>
#include <Services/QBStandbyAgent.h>


/**
 * @defgroup QBStandByTimerTimeout Stand-by timer service
 * @ingroup CubiTV_services
 * @{
 **/

/**
 *  It is a service that switch box into standby mode after time specified by STANDBYTIMERTIMEOUT config value.
 *  This service differs from auto-standby in that way, that user activity is not taken into consideration.
 *
 * @class QBStandByTimerTimeout
 **/
typedef struct QBStandByTimerTimeout_t* QBStandByTimerTimeout;

/**
 * Creates an instance of QBStandByTimerTimeout class.
 *
 * @memberof QBStandByTimerTimeout
 * @param[in] scheduler     SvScheduler application instance
 * @param[in] standbyAgent  QBStandbyAgent application instance
 * @return standbyAgent     QBStandByTimerTimeout instance
 */
QBStandByTimerTimeout QBStandByTimerTimeoutCreate(SvScheduler scheduler, QBStandbyAgent standbyAgent);

/**
 * Starts timer that will pass after value set in STANDBYTIMERTIMEOUT config value.
 *
 * @memberof QBStandByTimerTimeout
 * @param[in] self      QBStandByTimerTimeout instance
 */
void QBStandByTimerTimeoutStart(QBStandByTimerTimeout self);

/**
 * Stops timer.
 *
 * @memberof QBStandByTimerTimeout
 * @param[in] self      QBStandByTimerTimeout instance
 */
void QBStandByTimerTimeoutStop(QBStandByTimerTimeout self);

/**
 * @}
 **/

#endif //! QBSTANDBYTIMERTIMEOUT_H_
