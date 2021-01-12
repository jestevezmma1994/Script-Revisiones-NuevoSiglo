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

#ifndef QBSPLASH_DEFINES_H_
#define QBSPLASH_DEFINES_H_

#include "QBSplashInternal.h"

#define PRIMARY_WIDTH    1280
#define PRIMARY_HEIGHT   720
#define PRIMARY_GAMMA    2.2
#define SECONDARY_WIDTH  720
#define SECONDARY_HEIGHT 576
#define SECONDARY_GAMMA  2.8
#define SPINNER_X   468
#define SPINNER_Y   360
#define ENTRY_COUNT 1

typedef struct entry_ {
    uint32_t type;
    uint32_t count;
    uint32_t xPos;
    uint32_t yPos;
    char *fileNames[];
} entry;

entry anim = {QBSplashEntryType_anim, 48, SPINNER_X, SPINNER_Y, { "../full/0001.png",
        "../full/0002.png", "../full/0003.png", "../full/0004.png", "../full/0005.png",
        "../full/0006.png", "../full/0007.png", "../full/0008.png", "../full/0009.png",
        "../full/0010.png", "../full/0011.png", "../full/0012.png", "../full/0013.png",
        "../full/0014.png", "../full/0015.png", "../full/0016.png", "../full/0017.png",
        "../full/0018.png", "../full/0019.png", "../full/0020.png", "../full/0021.png",
        "../full/0022.png", "../full/0023.png", "../full/0024.png", "../full/0025.png",
        "../full/0026.png", "../full/0027.png", "../full/0028.png", "../full/0029.png",
        "../full/0030.png", "../full/0031.png", "../full/0032.png", "../full/0033.png",
        "../full/0034.png", "../full/0035.png", "../full/0036.png", "../full/0037.png",
        "../full/0038.png", "../full/0039.png", "../full/0040.png", "../full/0041.png",
        "../full/0042.png", "../full/0043.png", "../full/0044.png", "../full/0045.png",
        "../full/0046.png", "../full/0047.png", "../full/0048.png" } };
entry *entries[ENTRY_COUNT] = {&anim};

#endif /* QBSPLASH_DEFINES_H_ */
