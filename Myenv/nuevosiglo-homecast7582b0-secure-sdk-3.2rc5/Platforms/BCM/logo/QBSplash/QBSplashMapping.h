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

#ifndef QBSPLASHMAPPING_H
#define QBSPLASHMAPPING_H

#include "QBSplashInternal.h"

/*------------------------------------------------------------------------*
 * types
 *------------------------------------------------------------------------*/

/* basic splash file structure:
 * QBSplashSerializedHeader_
 *
 * entryCount * QBSplashSerializedEntry_
 * offsets for each entry
 *
 * after that, there are the bitmaps:
 * + QBSplashSerializedBitmapHeader_
 * + bitmap data
 *
 */
// structs used for storing splash data
typedef struct __attribute__((__packed__)) QBSplashSerializedBitmapHeader_ {
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t left;
    uint32_t top;
    uint32_t size;
} QBSplashSerializedBitmapHeader;

typedef struct __attribute__((__packed__)) QBSplashSerializedEntry_ {
    uint32_t type;
    uint32_t target;
    uint32_t startId;
    uint32_t bitmapCount;
} QBSplashSerializedEntry;

typedef struct __attribute__((__packed__)) QBSplashSerializedHeader_ {
    uint32_t magic;
    uint32_t version;
    uint32_t outputProfile;
    uint32_t bgcolor; // this is RGBA8888 since it's only one value and we would have padding anyway
    uint32_t entryCount;
    uint32_t size;
    uint32_t vdcSize;
} QBSplashSerializedHeader;

/*------------------------------------------------------------------------*
 * function declarations
 *------------------------------------------------------------------------*/

void *QBSplashMappingToBuffer(QBSplashMapping *self, size_t *size);
QBSplashMapping *QBSplashMappingCreateFromBuffer(void *inputBuffer, size_t bufferSize);
void QBSplashMappingDestroy(QBSplashMapping *self);

#endif // QBSPLASHMAPPING_H
