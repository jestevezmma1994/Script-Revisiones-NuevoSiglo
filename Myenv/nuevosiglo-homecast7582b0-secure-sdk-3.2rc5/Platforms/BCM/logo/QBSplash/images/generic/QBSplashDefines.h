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
#define ENTRY_COUNT 4

typedef struct entry_ {
    uint32_t type;
    uint32_t count;
    uint32_t xPos;
    uint32_t yPos;
    char *fileNames[];
} entry;

entry logo = {QBSplashEntryType_logo, 1, PRIMARY_WIDTH / 2 + 40, PRIMARY_HEIGHT / 2, {"logo_transparent.png"}};
entry anim = {QBSplashEntryType_anim, 48, SPINNER_X, SPINNER_Y, { "0001.png",
        "0002.png", "0003.png", "0004.png", "0005.png", "0006.png", "0007.png",
        "0008.png", "0009.png", "0010.png", "0011.png", "0012.png", "0013.png",
        "0014.png", "0015.png", "0016.png", "0017.png", "0018.png", "0019.png",
        "0020.png", "0021.png", "0022.png", "0023.png", "0024.png", "0025.png",
        "0026.png", "0027.png", "0028.png", "0029.png", "0030.png", "0031.png",
        "0032.png", "0033.png", "0034.png", "0035.png", "0036.png", "0037.png",
        "0038.png", "0039.png", "0040.png", "0041.png", "0042.png", "0043.png",
        "0044.png", "0045.png", "0046.png", "0047.png", "0048.png" } };
entry progress_bg = {QBSplashEntryType_bg, 1, PRIMARY_WIDTH / 2, 500, {"progress_bg.png"}};
entry progress_bar = {QBSplashEntryType_bar, 1, PRIMARY_WIDTH / 2, 500, {"progress_bar.png"}};
entry *entries[ENTRY_COUNT] = {&logo, &progress_bg, &progress_bar, &anim};

#endif /* QBSPLASH_DEFINES_H_ */