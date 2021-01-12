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

entry logo = {QBSplashEntryType_logo, 1, PRIMARY_WIDTH / 2 + 40, PRIMARY_HEIGHT / 2, {"../full/logo_transparent.png"}};
entry *entries[ENTRY_COUNT] = {&logo};

#endif /* QBSPLASH_DEFINES_H_ */
