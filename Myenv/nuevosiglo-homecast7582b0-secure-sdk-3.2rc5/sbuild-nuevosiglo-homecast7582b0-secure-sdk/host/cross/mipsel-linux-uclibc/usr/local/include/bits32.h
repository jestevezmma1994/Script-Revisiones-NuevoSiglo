/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef FILE_BITS_32_H
#define FILE_BITS_32_H

#include <stddef.h>
#include <stdint.h>

typedef const uint8_t * const const_bit_buffer_t;
typedef uint8_t * const bit_buffer_t;


static inline __attribute__ ((always_inline))
uint32_t get32(const_bit_buffer_t buff, const int bitOffset, const int nBits)
{
    int byteOffset = bitOffset / 8;
    const int startBitOffset = bitOffset - 8 * byteOffset;
    const int endBitOffset = startBitOffset + nBits;

    uint32_t r = 0;

    if( endBitOffset > 32 )
    {
        r |= ( buff[ byteOffset     ] & ( 0xFF >> startBitOffset ) ) << ( endBitOffset -  8 );
        r |=   buff[ byteOffset + 1 ]                                << ( endBitOffset - 16 );
        r |=   buff[ byteOffset + 2 ]                                << ( endBitOffset - 24 );
        r |=   buff[ byteOffset + 3 ]                                << ( endBitOffset - 32 );
        r |=   buff[ byteOffset + 4 ]                                >> ( 40 - endBitOffset );
    }
    if( endBitOffset > 24 )
    {
        r |= ( buff[ byteOffset     ] & ( 0xFF >> startBitOffset ) ) << ( endBitOffset -  8 );
        r |=   buff[ byteOffset + 1 ]                                << ( endBitOffset - 16 );
        r |=   buff[ byteOffset + 2 ]                                << ( endBitOffset - 24 );
        r |=   buff[ byteOffset + 3 ]                                >> ( 32 - endBitOffset );
    }
    else if( endBitOffset > 16 )
    {
        r |= ( buff[ byteOffset     ] & ( 0xFF >> startBitOffset ) ) << ( endBitOffset -  8 );
        r |=   buff[ byteOffset + 1 ]                                << ( endBitOffset - 16 );
        r |=   buff[ byteOffset + 2 ]                                >> ( 24 - endBitOffset );
    }
    else if( endBitOffset > 8 )
    {
        r |= ( buff[ byteOffset     ] & ( 0xFF >> startBitOffset ) ) << ( endBitOffset -  8 );
        r |=   buff[ byteOffset + 1 ]                                >> ( 16 - endBitOffset );
    }
    else
    {
        r = ( buff[ byteOffset      ] & ( 0xFF >> startBitOffset ) );
        r = r >> ( 8 - endBitOffset );
    }

    return r;
}

static inline __attribute__ ((always_inline))
void set32(bit_buffer_t buff, const int bitOffset, const int nBits, uint32_t value)
{
    int byteOffset = bitOffset / 8;
    const int startBitOffset = bitOffset - 8 * byteOffset;
    const int endBitOffset = startBitOffset + nBits;

    if( endBitOffset > 32  )
    {
        const uint32_t m1 = ( 0xFF >> startBitOffset );

        buff[ byteOffset ] &= ~m1;
        buff[ byteOffset ] |= ( value >> ( endBitOffset - 8 ) );

        buff[ byteOffset + 1 ] = value >> ( endBitOffset - 16 );
        buff[ byteOffset + 2 ] = value >> ( endBitOffset - 24 );
        buff[ byteOffset + 3 ] = value >> ( endBitOffset - 32 );

        const uint32_t m2 = ( 0xFF << ( 40 - endBitOffset ) );

        buff[ byteOffset + 4 ] &= ~m2;
        buff[ byteOffset + 4 ] |= ( ( value << ( 40 - endBitOffset ) ) & m2 );
    }
    else if( endBitOffset > 24  )
    {
        const uint32_t m1 = ( 0xFF >> startBitOffset );

        buff[ byteOffset ] &= ~m1;
        buff[ byteOffset ] |= ( value >> ( endBitOffset - 8 ) );

        buff[ byteOffset + 1 ] = value >> ( endBitOffset - 16 );
        buff[ byteOffset + 2 ] = value >> ( endBitOffset - 24 );

        const uint32_t m2 = ( 0xFF << ( 32 - endBitOffset ) );

        buff[ byteOffset + 3 ] &= ~m2;
        buff[ byteOffset + 3 ] |= ( ( value << ( 32 - endBitOffset ) ) & m2 );
    }
    else if( endBitOffset > 16 )
    {
        const uint32_t m1 = ( 0xFF >> startBitOffset );

        buff[ byteOffset ] &= ~m1;
        buff[ byteOffset ] |= ( value >> ( endBitOffset - 8 ) );

        buff[ byteOffset + 1 ] = value >> ( endBitOffset - 16 );

        const uint32_t m2 = ( 0xFF << ( 24 - endBitOffset ) );

        buff[ byteOffset + 2 ] &= ~m2;
        buff[ byteOffset + 2 ] |= ( ( value << ( 24 - endBitOffset ) ) & m2 );
    }
    else if( endBitOffset > 8 )
    {
        const uint32_t m1 = ( 0xFF >> startBitOffset );

        buff[ byteOffset ] &= ~m1;
        buff[ byteOffset ] |= ( value >> ( endBitOffset - 8 ) );

        const uint32_t m2 = ( 0xFF << ( 16 - endBitOffset ) );

        buff[ byteOffset + 1 ] &= ~m2;
        buff[ byteOffset + 1 ] |= ( ( value << ( 16 - endBitOffset ) ) & m2 );
    }
    else
    {
        const uint32_t m1 = ( 0xFF >> startBitOffset );
        const uint32_t m2 = ( 0xFF << ( 8 - endBitOffset ) );

        buff[ byteOffset ] &= ~( m1 & m2 );
        buff[ byteOffset ] |= ( ( value << ( 8 - endBitOffset ) ) & ( m1 & m2 ) );
    }
}

#endif
/* FILE_BITS_32_H */

