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

#ifndef QBCASCOMMONCRYPTOGUARDFINGERPRINT_H_
#define QBCASCOMMONCRYPTOGUARDFINGERPRINT_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCASCommonCryptoguardFingerprint.h QBCASCommonCryptoguardFingerprint class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCASCommonCryptoguardFingerprint QBCASCommonCryptoguardFingerprint
 * @ingroup QBCASCommonCryptoguard
 *
 * QBCASCommonCryptoguardFingerprint class
 */

typedef struct QBCASCommonCryptoguardFingerprint_s {
    struct SvObject_ super_; /**< super type */

    SvString text; /**< fingerprint message */
    uint16_t duration; /**< how long fingerprint need to be displayed on the screen in ms, range 0-65535 */
    uint8_t xPosition; /**< x coordinate, range 0-15 */
    uint8_t yPosition; /**< y coordinate, range 0-15 */
    uint32_t fontColor; /**< font color in ARGB format */
    uint32_t backgroundColor; /**< background color in ARGB format */
} *QBCASCommonCryptoguardFingerprint;

/**
 * Creates Fingerprint message
 *
 * @param[out] errorOut error info
 * @return @ref QBCASCommonCryptoguardFingerprint
 */
QBCASCommonCryptoguardFingerprint QBCASCommonCryptoguardFingerprintCreate(SvErrorInfo *errorOut);

/**
 * Get runtime type identification object representing @ref QBCASCommonCryptoguardFingerprint
 * @return @ref QBCASCommonCryptoguardFingerprint type
 **/
SvType QBCASCommonCryptoguardFingerprint_getType(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QBCASCOMMONCRYPTOGUARDFINGERPRINT_H_ */
