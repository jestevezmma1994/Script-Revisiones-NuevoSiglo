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

#ifndef SV_BYTE_ORDER__LITTLE_ENDIAN_H_
#define SV_BYTE_ORDER__LITTLE_ENDIAN_H_

#if defined SV_BYTE_ORDER_H_ && SV_BYTE_ORDER == SV_LITTLE_ENDIAN

#include <stdint.h>

// include platform-specific byte swapping functions
#if defined __GNUC__ && (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 3)
# include <SvQuirks/SvByteSwap_builtin.h>
#elif defined __sh__
# include <SvQuirks/SvByteSwap_sh4.h>
#elif defined __linux__
# include <SvQuirks/SvByteSwap_linux.h>
#else
# include <SvQuirks/SvByteSwap_generic.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


// conversions between host and little-endian == no-op

static inline uint16_t
SvHToLE16(uint16_t v)
{
    return v;
}

static inline uint16_t
SvLEToH16(uint16_t v)
{
    return v;
}

static inline uint32_t
SvHToLE32(uint32_t v)
{
    return v;
}

static inline uint32_t
SvLEToH32(uint32_t v)
{
    return v;
}

static inline uint64_t
SvHToLE64(uint64_t v)
{
    return v;
}

static inline uint64_t
SvLEToH64(uint64_t v)
{
    return v;
}


// conversions between host and big-endian == byte swap

static inline uint16_t
SvHToBE16(uint16_t v)
{
    return SvByteSwap16(v);
}

static inline uint16_t
SvBEToH16(uint16_t v)
{
    return SvByteSwap16(v);
}

static inline uint32_t
SvHToBE32(uint32_t v)
{
    return SvByteSwap32(v);
}

static inline uint32_t
SvBEToH32(uint32_t v)
{
    return SvByteSwap32(v);
}

static inline uint64_t
SvHToBE64(uint64_t v)
{
    return SvByteSwap64(v);
}

static inline uint64_t
SvBEToH64(uint64_t v)
{
    return SvByteSwap64(v);
}


#ifdef __cplusplus
}
#endif

#endif // defined SV_BYTE_ORDER_H_ && SV_BYTE_ORDER == SV_LITTLE_ENDIAN

#endif // SV_BYTE_ORDER__LITTLE_ENDIAN_H_
