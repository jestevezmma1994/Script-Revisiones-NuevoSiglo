/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_INTEK_CONAX_SMARTCARD_ENCRYPT_H
#define QB_INTEK_CONAX_SMARTCARD_ENCRYPT_H

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBICSmartcardCmdEncrypt_ {
    struct SvObject_ super_;

    uint8_t encrypt;    // 0 -> means encrypt data, 1-> means decrypt data
    uint8_t cookie;     // the resulting notification structure has to have the same cookie
    uint8_t data[1024]; // data to encrypt/decrypt
    uint16_t len;       // length of data
};
typedef struct QBICSmartcardCmdEncrypt_ *QBICSmartcardCmdEncrypt;

extern SvType QBICSmartcardCmdEncrypt_getType(void);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_INTEK_CONAX_SMARTCARD_ENCRYPT_H
