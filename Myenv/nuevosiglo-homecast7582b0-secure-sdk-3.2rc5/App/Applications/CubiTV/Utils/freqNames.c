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

#include "freqNames.h"
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>


SvString freqToName(int freq)
{
    char letter;
    int number = -1;
    //Check for 474-858, band=8
    if ((freq % 8000) == (474000 % 8000) && freq >= 474000 && freq <= 858000) {
        letter = 'K';
        number = (freq - 474000) / 8000 + 21;
    }
    //Check for 177.5-226.5, band=7
    else if ((freq % 7000 == 177500 % 7000) && freq >= 177500 && freq <= 226500) {
        letter = 'K';
        number = (freq - 177500) / 7000 + 5;
    }
    //Check for 114-298, band=8
    else if ((freq % 8000) == (114000 % 8000) && freq >= 114000 && freq <= 298000) {
        letter = 'D';
        number = (freq - 114000) / 7000 + 1;
    }
    //Check for 114.5-296.5, band=7
    else if((freq % 7000) == (114500 % 7000) && freq >= 114500 && freq <= 296500) {
        letter = 'S';
        number = (freq - 114500) / 7000 + 2;
    }
    //Check for 306-466, band=8
    else if ((freq % 8000) == (306000 % 8000) && freq >= 306000 && freq <= 466000) {
        letter = 'S';
        number = (freq - 306000) / 8000 + 21;
    }
    if (number >= 0)
        return SVAUTORELEASE(SvStringCreateWithFormat("%c%i", letter, number));
    else
        return NULL;
}
