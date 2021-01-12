/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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


#ifndef QBTELETEXTCONVTABLES_H_
#define QBTELETEXTCONVTABLES_H_


#include <stdint.h>

typedef enum {
    G0_LATIN,
    G0_GREEK,
    G0_CYRILLIC,
} G0CharacterSet;

typedef enum {
    G2_LATIN,
    G2_GREEK,
    G2_CYRILLIC,
} G2CharacterSet;

uint32_t QBTeletextConvTableChooseG0(G0CharacterSet g0, int langCode, int X28Info, uint32_t c);
uint32_t QBTeletextConvTableChooseG2(G2CharacterSet g2, uint32_t c);
uint32_t QBTeletextConvTableChooseG0WithG2DiacriticalMark(G0CharacterSet g0, uint32_t c, uint32_t mark);

#endif /* QBTELETEXTCONVTABLES_H_ */
