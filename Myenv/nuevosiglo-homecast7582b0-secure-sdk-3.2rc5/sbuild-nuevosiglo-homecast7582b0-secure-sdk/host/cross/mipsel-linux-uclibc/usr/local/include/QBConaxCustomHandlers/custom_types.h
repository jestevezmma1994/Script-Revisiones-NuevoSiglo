/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CONAX_CUSTOM_HANDLERS_HC_H
#define QB_CONAX_CUSTOM_HANDLERS_HC_H

#ifdef MODULE
# include <linux/kernel.h>
#else
# include <stdint.h>
#endif

#define CUSTOM_NOTIFICATION_SPI_CLEAR  0x0
#define CUSTOM_NOTIFICATION_SPI_WRITE  0x1

typedef struct {
    uint32_t valid;
    uint32_t keyId;
    uint32_t keyType;
    uint32_t keyLength;
    uint8_t keyValue[16];
} SecureKey_t;

struct CustomDataSpiWrite_s {
    uint32_t idx;
    uint32_t iCrc;

    uint8_t data[sizeof(SecureKey_t)];
};

#endif // #ifndef QB_CONAX_CUSTOM_HANDLERS_HC_H
