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

#ifndef QBCRYPTOGUARDVIDEORULESMANAGER_H
#define QBCRYPTOGUARDVIDEORULESMANAGER_H

#include <SvFoundation/SvCoreTypes.h>

#include <main_decl.h>

/**
 * @file QBCryptoguardVideoRulesManager.h
 * @brief Cryptoguard Video Rules Manager service
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCryptoguardVideoRulesManager Cryptoguard Video Rules Manager service
 * @ingroup CubiTV_services
 * @{
 **/

typedef struct QBCryptoguardVideoRulesManager_s* QBCryptoguardVideoRulesManager;

/**
 * Starts Cryptoguard Video Rules Manage service.
 *
 * @param [in] self object
 **/
extern void QBCryptoguardVideoRulesManagerStart(QBCryptoguardVideoRulesManager self);

/**
 * Stops Cryptoguard Video Rules Manage service.
 *
 * @param [in] self object
 **/
extern void QBCryptoguardVideoRulesManagerStop(QBCryptoguardVideoRulesManager self);

/**
 * Creates Cryptoguard Video Rules Manager object
 *
 * @param [in] appGlobals appglobals instance
 * @return new Cryptoguard Video Rules Manager instance
 **/
extern QBCryptoguardVideoRulesManager QBCryptoguardVideoRulesManagerCreate(AppGlobals appGlobals);

/** Get runtime type identification object representing QBCryptoguardVideoRulesManager class
 * @return object representing QBCryptoguardVideoRulesManager class
 */
extern SvType QBCryptoguardVideoRulesManager_getType(void);

/**
 * @}
 **/

#endif // QBCRYPTOGUARDVIDEORULESMANAGER_H
