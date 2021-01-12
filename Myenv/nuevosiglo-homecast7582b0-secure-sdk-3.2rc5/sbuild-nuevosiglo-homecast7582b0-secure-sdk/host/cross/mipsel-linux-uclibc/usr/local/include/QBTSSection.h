/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TS_SECTION_H
#define QB_TS_SECTION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int
QBTSSectionGetTableID(const unsigned char* section)
{
    return section[0];
}

static inline int
QBTSSectionGetVersionNumber(const unsigned char* section)
{
    return (section[5] >> 1) & 0x1f;
}

static inline int
QBTSSectionGetSectionNumber(const unsigned char* section)
{
    return section[6];
}

static inline int
QBTSSectionGetLastSectionNumber(const unsigned char* section)
{
    return section[7];
}

static inline uint16_t
QBTSSectionGetTableIDExtension(const unsigned char* section)
{
    return ((uint16_t)section[3] << 8) | (uint16_t)section[4];
}

static inline bool
QBTSSectionGetSyntaxIndicator(const unsigned char* section)
{
    return (section[1] & 0x80) != 0;
}

static inline bool
QBTSSectionGetCurrentNextIndicator(const unsigned char* section)
{
    return (section[5] & 0x01) != 0;
}

static inline uint32_t
QBTSSectionGetCrc32(const unsigned char* section, unsigned int len)
{
    section += len - 4;
    return ((uint32_t)section[0] << 24)
         | ((uint32_t)section[1] << 16)
         | ((uint32_t)section[2] <<  8)
         | ((uint32_t)section[3] <<  0);
}

static inline void
QBTSSectionSetCrc32(unsigned char* section, unsigned int len, uint32_t crc)
{
    section += len - 4;
    section[0] = crc >> 24;
    section[1] = crc >> 16;
    section[2] = crc >>  8;
    section[3] = crc >>  0;
}

#ifdef __cplusplus
}
#endif

#endif //QB_TS_SECTION_H
