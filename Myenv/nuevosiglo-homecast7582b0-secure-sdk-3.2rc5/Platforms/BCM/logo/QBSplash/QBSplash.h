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

#ifndef QBSPLASH_H
#define QBSPLASH_H

#if defined(_CFE_) || defined(_BOLT_)
#include <lib_types.h>
#include "QBBootConfig.h"
#elif defined(__KERNEL__)
#include <linux/printk.h>
#include <linux/slab.h>
#define QBBootLog printk
#define QBBootMalloc(size) kmalloc(size, GFP_KERNEL)
#define QBBootFree(ptr) kfree(ptr)
#else
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#define QBBootLog printf
#define QBBootMalloc(size) malloc(size)
#define QBBootFree(ptr) free(ptr)
#endif // !__KERNEL__ !_CFE_

typedef enum {
    QBBitmapFormat_invalid = 0,
    QBBitmapFormat_RGB565 = 0x20353635,
    QBBitmapFormat_ARGB8888 = 0x38383838,
} QBBitmapFormat;

typedef enum {
    QBSplashOutputProfile_undefined = 0,
    QBSplashOutputProfile_NTSC,             // both surfaces NTSC
    QBSplashOutputProfile_PAL,              // both surfaces PAL
    QBSplashOutputProfile_480p_NTSC,             // HDMI 480p60, analog NTSC
    QBSplashOutputProfile_576p_PAL,             // HDMI 576p50, analog PAL
    QBSplashOutputProfile_720p60_NTSC,   // HDMI 720p60, analog NTSC
    QBSplashOutputProfile_720p50_PAL,    // HDMI 720p50, analog PAL
} QBSplashOutputProfile;

typedef enum {
    QBSplashOutputType_undefined= 0,
    QBSplashOutputType_primary,
    QBSplashOutputType_secondary,
} QBSplashOutputType;

typedef struct QBSplashOutputDimensions_ {
    uint32_t primaryWidth;
    uint32_t primaryHeight;
    uint32_t secondaryWidth;
    uint32_t secondaryHeight;
} QBSplashOutputDimensions;

typedef struct QBSplash_ *QBSplash;

int QBSplashDisplayLogo(QBSplash self);
int QBSplashAnimationDraw(QBSplash self);
int QBSplashAnimationStep(QBSplash self);
int QBSplashDrawProgress(QBSplash self, uint32_t progress);
int QBSplashClear(QBSplash self);
int QBSplashAddSurface(QBSplash self, void *surfaceAddr, QBBitmapFormat format, uint32_t width, uint32_t height, uint32_t pitch, QBSplashOutputType outputType);
int QBSplashGetSurfaceInfo(QBSplash self, uint32_t surfaceId,
                                 void **surfaceAddr, QBBitmapFormat *format,
                                 uint32_t *width, uint32_t *height,
                                 uint32_t *pitch);
int QBSplashInit(QBSplash self, void *splashData, size_t splashDataSize);
int QBSplashDeInit(QBSplash self);
int QBSplashIsValid(QBSplash self);
int QBSplashGetSurfaceCount(QBSplash self);
QBSplash QBSplashGetInstance(void);
void *QBSplashGetData(QBSplash self);
size_t QBSplashGetDataSize(QBSplash self);
int QBSplashSetAnimFrame(QBSplash self, uint32_t animFrame);
uint32_t QBSplashGetAnimFrame(QBSplash self);
QBSplashOutputDimensions QBSplashGetOutputDimensions(QBSplashOutputProfile outputProfile);
QBSplashOutputProfile QBSplashGetOutputProfile(QBSplash self);
void *QBSplashGetVDC(QBSplash self);

#endif /*QBSPLASH_H*/
