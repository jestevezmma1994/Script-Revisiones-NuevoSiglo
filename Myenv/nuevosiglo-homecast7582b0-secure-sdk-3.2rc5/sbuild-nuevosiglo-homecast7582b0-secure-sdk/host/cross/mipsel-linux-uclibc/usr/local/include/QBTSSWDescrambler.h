/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TS_SW_DESCRAMBLER_H
#define QB_TS_SW_DESCRAMBLER_H

#include <SvPlayerKit/SvBuf.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QBTSSWDescrambler_s  QBTSSWDescrambler;

QBTSSWDescrambler* QBTSSWDescramblerCreate(void);
void QBTSSWDescramblerDestroy(QBTSSWDescrambler* desc);

/// \param key  binary key data, or null to revoke the key
/// \param len  length of data in \a key
void QBTSSWDescramblerUpdateKey(QBTSSWDescrambler* desc, bool even, const uint8_t* key, int len);

/// \returns  new buf, or retained \a sb when no packets were changed
SvBuf QBTSSWDescramblerProcessData(QBTSSWDescrambler* desc, SvBuf sb);

/// revoke both keys
void QBTSSWDescramblerClear(QBTSSWDescrambler* desc);


#ifdef __cplusplus
}
#endif

#endif // QB_TS_SW_DESCRAMBLER_H
