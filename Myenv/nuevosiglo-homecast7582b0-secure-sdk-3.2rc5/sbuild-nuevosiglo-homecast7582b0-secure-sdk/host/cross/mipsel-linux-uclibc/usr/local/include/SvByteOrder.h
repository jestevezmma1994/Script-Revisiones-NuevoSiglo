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

#ifndef SV_BYTE_ORDER_H_
#define SV_BYTE_ORDER_H_

/**
 * @file SvByteOrder.h
 * @brief Byte order definitions and conversion functions
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvByteOrder Byte order definitions and functions
 * @ingroup SvQuirks
 * @{
 **/


// endiannes definition

#ifndef SV_LITTLE_ENDIAN
/// little endian marker
# define SV_LITTLE_ENDIAN 1234
#endif

#ifndef SV_BIG_ENDIAN
/// big endian marker
# define SV_BIG_ENDIAN 4321
#endif

#undef SV_BYTE_ORDER
#if defined __i386__ || defined __x86_64__
# define SV_BYTE_ORDER SV_LITTLE_ENDIAN
#elif defined __mips__ && defined __MIPSEL__
# define SV_BYTE_ORDER SV_LITTLE_ENDIAN
#elif defined __mips__ && !defined __MIPSEL__
# define SV_BYTE_ORDER SV_BIG_ENDIAN
#elif defined __arm__ && defined __ARMEL__
# define SV_BYTE_ORDER SV_LITTLE_ENDIAN
#elif defined __arm__ && !defined __ARMEL__
# define SV_BYTE_ORDER SV_BIG_ENDIAN
#elif defined __ppc__ || defined __powerpc__ || defined __ppc64__
# define SV_BYTE_ORDER SV_BIG_ENDIAN
#elif defined __LITTLE_ENDIAN__
# define SV_BYTE_ORDER SV_LITTLE_ENDIAN
#else
# error "Unknown platform - can't determine byte order!"
#endif


// include byte order conversion functions
#if SV_BYTE_ORDER == SV_LITTLE_ENDIAN
# include <SvQuirks/SvByteOrder_LE.h>
#else
# include <SvQuirks/SvByteOrder_BE.h>
#endif


/**
 * @}
 **/

#endif
