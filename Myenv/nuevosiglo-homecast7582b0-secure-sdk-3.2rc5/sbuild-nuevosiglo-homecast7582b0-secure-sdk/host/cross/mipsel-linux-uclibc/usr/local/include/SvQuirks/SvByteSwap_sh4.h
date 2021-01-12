/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_BYTE_SWAP__SH4_H_
#define SV_BYTE_SWAP__SH4_H_

#include <stdint.h>
#include <byteswap.h>


#ifdef __cplusplus
extern "C" {
#endif


static inline uint16_t
SvByteSwap16(uint16_t v)
{
    __asm__ __volatile__ (
        "swap.b  %0, %0"
        : "=&r" (v)
        : "0" (v)
        );
    return v;
}

static inline uint32_t
SvByteSwap32(uint32_t v)
{
    __asm__ __volatile__ (
        "swap.b  %0, %0"     "\n"
        "\t"     "swap.w  %0, %0"     "\n"
        "\t"     "swap.b  %0, %0"
        : "=&r" (v)
        : "0" (v)
        );
    return v;
}

static inline uint64_t
SvByteSwap64(uint64_t v)
{
    register uint32_t tmp;
    __asm__ __volatile__ (
        "swap.b  %R0, %R0"   "\n"
        "\t"     "swap.w  %R0, %1"    "\n"
        "\t"     "swap.b  %1, %1"     "\n"
        "\t"     "swap.b  %S0, %S0"   "\n"
        "\t"     "swap.w  %S0, %R0"   "\n"
        "\t"     "swap.b  %R0, %R0"   "\n"
        "\t"     "mov     %1, %S0"
        : "=r" (v), "=r" (tmp)
        : "0" (v)
        : "1"
        );
    return v;
}


#ifdef __cplusplus
}
#endif

#endif
