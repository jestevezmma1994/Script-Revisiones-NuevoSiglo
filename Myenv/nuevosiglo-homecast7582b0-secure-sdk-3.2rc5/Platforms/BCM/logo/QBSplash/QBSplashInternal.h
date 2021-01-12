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

#ifndef QBSPLASHINTERNAL_H
#define QBSPLASHINTERNAL_H

#include "QBSplash.h"
#if defined(_CFE_) || defined(_BOLT_)
#include <lib_types.h>
#elif defined(__KERNEL__)
#include <linux/types.h>
#endif

/*------------------------------------------------------------------------*
 * defines
 *------------------------------------------------------------------------*/

//#define QBSPLASH_DEBUG

#ifdef QBSPLASH_DEBUG
#define ASSERT_LOG(x, retval) do{\
    if (!(x)) {\
        QBBootLog("%s:%d: assertion " #x " failed\n", __FUNCTION__, __LINE__);\
        return (retval);\
    }\
    } while(0);
#else // QBSPLASH_DEBUG
#define ASSERT_LOG(x, retval) do{\
    if (!(x)) {\
        return (retval);\
    }\
    } while (0);
#endif // QBSPLASH_DEBUG

// magic values
#define QB_SPLASH_MAGIC                 (0x50534251)

#define RGB_TO_PIXEL16(r,g,b) (\
    ((uint16_t)(r) & 0xF8) << 8 | \
    ((uint16_t)(g) & 0xFC) << 3 | \
    ((uint16_t)(b) & 0xF8) >> 3 \
    )

#define PIXEL32_TO_RGBA(pixel, r, g, b, a) {\
    (r) = (((pixel) >> 0)  & 0xff);\
    (g) = (((pixel) >> 8)  & 0xff);\
    (b) = (((pixel) >> 16) & 0xff);\
    (a) = (((pixel) >> 24) & 0xff);}

#define RGBA_TO_PIXEL32(r, g, b, a) (a << 24 | b << 16 | g << 8 | r)

/*------------------------------------------------------------------------*
 * enums
 *------------------------------------------------------------------------*/

typedef enum {
    QBSplashVersion_undefined = 0,
    QBSplashVersion_1,
    QBSplashVersion_2,
    QBSplashVersion_3,
} QBSplashVersion;

typedef enum {
    QBSplashEntryType_undefined = 0,
    QBSplashEntryType_logo,
    QBSplashEntryType_anim,
    QBSplashEntryType_bg,
    QBSplashEntryType_bar,
} QBSplashEntryType;

/*------------------------------------------------------------------------*
 * types
 *------------------------------------------------------------------------*/

typedef struct QBSplashBitmap_ {
    QBBitmapFormat format;
    uint32_t width;
    uint32_t height;
    uint32_t left;
    uint32_t top;
    uint32_t size;
    void *pixels;
} QBSplashBitmap;

typedef struct QBSplashEntry_ {
    QBSplashEntryType type;
    QBSplashOutputType target;
    uint32_t bitmapCount;
    QBSplashBitmap *bitmaps;
} QBSplashEntry;

typedef struct QBSplashMapping_ {
    QBSplashVersion version;
    QBSplashOutputProfile outputProfile;
    uint32_t bgcolor; // this is RGBA8888 since it's only one value and we would have padding anyway
    uint32_t entryCount;
    QBSplashEntry *entries;
    uint32_t vdcSize;
    void *vdcData;
} QBSplashMapping;

#endif // QBSPLASHINTERNAL_H
