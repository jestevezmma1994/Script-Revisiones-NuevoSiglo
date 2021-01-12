/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBCASCOMMONCRYPTOGUARDOSDMESSAGE_H_
#define QBCASCOMMONCRYPTOGUARDOSDMESSAGE_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCASCommonCryptoguardOSDMessage.h QBCASCommonCryptoguardOSDMessage class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCASCommonCryptoguardOSDMessage QBCASCommonCryptoguardOSDMessage
 * @ingroup QBCASCommonCryptoguard
 *
 * QBCASCommonCryptoguardOSDMessage class
 */

/**
 * OSD Message
 */
typedef struct QBCASCommonCryptoguardOSDMessage_s {
    struct SvObject_ super_; /**< super type */

    uint32_t duration; /**< how long message should be displayed, value in milliseconds */
    SvString message; /**< 'OSD message' */
} *QBCASCommonCryptoguardOSDMessage;

/**
 * Creates OSD message
 *
 * @param[out] errorOut error info
 * @return @ref QBCASCommonCryptoguardOSDMessage
 */
QBCASCommonCryptoguardOSDMessage QBCASCommonCryptoguardOSDMessageCreate(SvErrorInfo *errorOut);

/**
 * Get runtime type identification object representing @ref QBCASCommonCryptoguardOSDMessage
 * @return @ref QBCASCommonCryptoguardOSDMessage type
 **/
SvType QBCASCommonCryptoguardOSDMessage_getType(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QBCASCOMMONCRYPTOGUARDOSDMESSAGE_H_ */
