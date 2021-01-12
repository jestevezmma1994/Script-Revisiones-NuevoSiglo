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

#ifndef QBCASCOMMONCRYPTOGUARDGETMMI_H_
#define QBCASCOMMONCRYPTOGUARDGETMMI_H_

#include <QBCASCommonCryptoguard.h>
#include <QBCAS.h>

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCASCommonCryptoguardGetMMI.h QBCASCommonCryptoguardGetMMI class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * callback methods with menu for requested level
 * @param[in] self_ object which called QBCASCommonCryptoguardGetMMI()
 * @param[in] level @c 0 means main menu, rest are sub-menus
 * @param[in] SvString with all items on requested level
 * @param[in] retCode code return from external library
 */
typedef void (*QBCASCommonCryptoguardMMICallback)(SvObject self_, uint8_t level, SvString menu, int retCode);

/**
 * Method for getting menu which could be shown in MainMenu
 *
 * e.g. Level 0 :
 * "N00-Cryptoguard CA\nS01-Subscription\nS02-Pay-per-view\nS03-About CA\n"
 *
 * @param[in] self @ref QBCASCommonCryptoguard instance
 * @param[in] callback method which will be called in the response of current method
 * @param[in] target object from which QBCASCommonCryptoguardGetMMI() was called
 * @param[out] cmdOut pointer to internal command, could be used to cancel that command
 * @param[in] level @c 0 means main menu, rest are sub-menus
 */
int QBCASCommonCryptoguardGetMMI(QBCASCommonCryptoguard self, QBCASCommonCryptoguardMMICallback callback, SvObject target, QBCASCmd *cmdOut, uint8_t level);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QBCASCOMMONCRYPTOGUARDGETMMI_H_ */
