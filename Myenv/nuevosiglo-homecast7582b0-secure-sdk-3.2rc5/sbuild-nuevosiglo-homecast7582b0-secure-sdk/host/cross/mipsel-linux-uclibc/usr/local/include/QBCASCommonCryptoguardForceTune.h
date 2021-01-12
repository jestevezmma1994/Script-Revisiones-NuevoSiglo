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

#ifndef QBCASCOMMONCRYPTOGUARDFORCETUNE_H_
#define QBCASCOMMONCRYPTOGUARDFORCETUNE_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCASCommonCryptoguardForceTune.h QBCASCommonCryptoguardForceTune class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCASCommonCryptoguardForceTune QBCASCommonCryptoguardForceTune
 * @ingroup QBCASCommonCryptoguard
 *
 * QBCASCommonCryptoguardForceTune class
 */

#define DELIVERY_SYSTEM_DESCRIPTOR_LENGTH 11

typedef struct QBCASCommonCryptoguardForceTune_s {
    struct SvObject_ super_; /**< super type */

    uint16_t destinationSID; /**< SID number to tune to */
    uint8_t tag; /**< DVB-T -> 0x5A, DVB-C -> 0x44, DVB-S -> 0x43 */
    uint8_t descriptorData[DELIVERY_SYSTEM_DESCRIPTOR_LENGTH]; /**< delivery system descriptor */
} *QBCASCommonCryptoguardForceTune;

/**
 * Creates Force Tune message
 *
 * @param[out] errorOut error info
 * @return @ref QBCASCommonCryptoguardForceTune
 */
QBCASCommonCryptoguardForceTune QBCASCommonCryptoguardForceTuneCreate(SvErrorInfo *errorOut);

/**
 * Get runtime type identification object representing @ref QBCASCommonCryptoguardForceTune
 * @return @ref QBCASCommonCryptoguardForceTune type
 **/
SvType QBCASCommonCryptoguardForceTune_getType(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QBCASCOMMONCRYPTOGUARDFORCETUNE_H_ */
