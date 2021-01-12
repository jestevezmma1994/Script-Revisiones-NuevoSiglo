/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCRYPTOGUARDFORCETUNEMANAGER_H
#define QBCRYPTOGUARDFORCETUNEMANAGER_H

#include <main_decl.h>

/**
 * @file QBCryptoguardForceTuneManager.h
 * @brief QBCryptoguardForceTuneManager
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


/**
 * @defgroup QBCryptoguardForceTuneManager Cryptoguard force tune service
 * @ingroup CubiTV_services
 * @{
 **/

/**
 * @typedef QBCryptoguardForceTuneManager
 * Used for tuning to specific channel requested by operator
 */
typedef struct QBCryptoguardForceTuneManager_s* QBCryptoguardForceTuneManager;

/**
 * Constructor of QBCryptoguardForceTuneManager
 * @param appGlobals appGlobals
 * @return instance of QBCryptoguardForceTuneManager
 */
QBCryptoguardForceTuneManager QBCryptoguardForceTuneManagerCreate(AppGlobals appGlobals);

/**
 * Start cryptoguard force tune service
 * @param self @ref QBCryptoguardForceTuneManager
 */
void QBCryptoguardForceTuneManagerStart(QBCryptoguardForceTuneManager self);

/**
 * Stop cryptoguard force tune service
 * @param self @ref QBCryptoguardForceTuneManager
 */
void QBCryptoguardForceTuneManagerStop(QBCryptoguardForceTuneManager self);

/**
 * @}
 **/

#endif // QBCRYPTOGUARDFORCETUNEMANAGER_H
