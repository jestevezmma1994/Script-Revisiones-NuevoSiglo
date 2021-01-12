/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCASCOMMONCRYPTOGUARDSTBDISPLAYMESSAGE_H_
#define QBCASCOMMONCRYPTOGUARDSTBDISPLAYMESSAGE_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCASCommonCryptoguardSTBDisplayMessage.h QBCASCommonCryptoguardSTBDisplayMessage class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCASCommonCryptoguardSTBDisplayMessage QBCASCommonCryptoguardSTBDisplayMessage
 * @ingroup QBCASCommonCryptoguard
 *
 * QBCASCommonCryptoguardSTBDisplayMessage class
 */

/**
 * STB Display Message
 */
typedef struct QBCASCommonCryptoguardSTBDisplayMessage_s {
    struct SvObject_ super_; /**< super type */

    bool forced; /**< if @c true message should be displayed entire duration, if @c false it could be canceled by OK button */
    uint32_t duration; /**< how long message should be displayed, value in milliseconds */
    SvString message; /**< 'STB display message' */
} *QBCASCommonCryptoguardSTBDisplayMessage;

/**
 * Creates STB Display message
 *
 * @param[out] errorOut error info
 * @return @ref QBCASCommonCryptoguardSTBDisplayMessage
 */
QBCASCommonCryptoguardSTBDisplayMessage QBCASCommonCryptoguardSTBDisplayMessageCreate(SvErrorInfo *errorOut);

/**
 * Get runtime type identification object representing @ref QBCASCommonCryptoguardSTBDisplayMessage
 * @return @ref QBCASCommonCryptoguardSTBDisplayMessage type
 **/
SvType QBCASCommonCryptoguardSTBDisplayMessage_getType(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QBCASCOMMONCRYPTOGUARDSTBDISPLAYMESSAGE_H_ */
