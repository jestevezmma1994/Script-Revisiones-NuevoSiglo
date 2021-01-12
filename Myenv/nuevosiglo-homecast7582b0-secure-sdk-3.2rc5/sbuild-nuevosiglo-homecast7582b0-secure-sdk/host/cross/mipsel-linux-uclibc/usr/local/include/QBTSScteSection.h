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

#ifndef QB_SCTE_SECTION_H
#define QB_SCTE_SECTION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int
QBTSScteSectionGetTableID(const unsigned char* section)
{
    return section[0];
}

static inline int
QBTSScteSectionHasSectionNumber(const unsigned char* section)
{
    return section[3] & 0x40;
}

static inline int
QBTSScteSectionGetSectionNumber(const unsigned char* section)
{
    return ((uint16_t) (section[7] & 0x0f) << 8) | ((uint16_t) section[8]);
}

static inline int
QBTSScteSectionGetLastSectionNumber(const unsigned char* section)
{
    return ((uint16_t) section[6] << 4) | (((uint16_t) section[7] & 0xf0) >> 4);
}

static inline uint16_t
QBTSScteSectionGetTableIDExtension(const unsigned char* section)
{
    return ((uint16_t)section[4] << 8) | (uint16_t)section[5];
}

#ifdef __cplusplus
}
#endif

#endif //QB_TS_SECTION_H
