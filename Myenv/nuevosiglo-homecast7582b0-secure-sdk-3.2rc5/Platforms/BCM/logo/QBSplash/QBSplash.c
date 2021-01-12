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

#if defined (_BOLT_)
#include <stddef.h>
#endif
#include "QBSplashInternal.h"
#include "QBSplashMapping.h"

// we should get this from BOLT
#define SPLASH_MAX_SURFACE 4

typedef struct QBSplashSurface_ {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    QBBitmapFormat format;
    QBSplashOutputType type;
    void *pixels;
} QBSplashSurface;

struct QBSplash_ {
    QBSplashMapping *splash;
    void *splashData;
    size_t splashDataSize;
    uint32_t lastProgress;
    uint32_t lastFrame;
    uint32_t maxSurfaceCount;
    uint32_t surfaceCount;
    QBSplashSurface surfaces[SPLASH_MAX_SURFACE];
};

static struct QBSplash_ g_splash;

// static functions
static int QBSplashCheckBlit(QBSplashSurface surface, QBSplashBitmap *bitmap,
                             uint32_t dstLeft, uint32_t dstTop,
                             uint32_t srcLeft, uint32_t srcTop,
                             uint32_t srcW, uint32_t srcH)
{
    ASSERT_LOG(srcLeft + srcW <= bitmap->width, -1);
    ASSERT_LOG(srcTop + srcH <= bitmap->height, -1);
    ASSERT_LOG(dstLeft + srcW <= surface.width, -1);
    ASSERT_LOG(dstTop + srcH <= surface.height, -1);

    return 0;
}

static int QBSplashDoBlit(uint16_t *dst, uint32_t dstPitch, uint32_t dstLeft, uint32_t dstTop,
                          uint16_t *src, uint32_t srcPitch, uint32_t srcLeft, uint32_t srcTop, uint32_t srcW, uint32_t srcH)
{
    char *dstRow = 0;
    char *srcRow = 0;
    uint32_t x, y;

    ASSERT_LOG(dst, -1);
    ASSERT_LOG(src, -1);

    dstRow = (char *) dst + dstPitch * dstTop;
    srcRow = (char *) src + srcPitch * srcTop;

    for (y = 0; y < srcH; y++) {
        uint16_t *dstPixels = (uint16_t *) dstRow;
        uint16_t *srcPixels = (uint16_t *) srcRow;
        for (x = 0; x < srcW; x++) {
            dstPixels[x + dstLeft] = srcPixels[x + srcLeft];
        }
        srcRow += srcPitch;
        dstRow += dstPitch;
    }

    return 0;
}

static int QBSplashSurfaceFill(QBSplashSurface surface, uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t pixel = RGB_TO_PIXEL16(r, g, b);
    uint32_t x, y;

    char *row = surface.pixels;
    for (y = 0; y < surface.height; y++) {
        uint16_t *pixels = (uint16_t *) row;
        for (x = 0; x < surface.width; x++) {
            pixels[x] = pixel;
        }
        row += surface.pitch;
    }

    return 0;
}

static int QBSplashSurfaceBlitImage(QBSplashSurface surface, QBSplashBitmap *bitmap)
{
    ASSERT_LOG(
        !QBSplashCheckBlit(surface, bitmap, bitmap->left, bitmap->top, 0, 0,
                           bitmap->width, bitmap->height),
        -1);

    ASSERT_LOG(
        !QBSplashDoBlit(surface.pixels, surface.pitch, bitmap->left,
                        bitmap->top, bitmap->pixels,
                        bitmap->width * sizeof(uint16_t), 0, 0, bitmap->width,
                        bitmap->height),
        -1);

    return 0;
}

static int QBSplashSurfaceBlitImagePart(QBSplashSurface surface, QBSplashBitmap *bitmap,
                                        uint32_t left, uint32_t top, uint32_t width, uint32_t height)
{
    ASSERT_LOG(
        !QBSplashCheckBlit(surface, bitmap, bitmap->left, bitmap->top, left,
                           top, width, height),
        -1);

    ASSERT_LOG(
        !QBSplashDoBlit(surface.pixels, surface.pitch, bitmap->left,
                        bitmap->top, bitmap->pixels,
                        bitmap->width * sizeof(uint16_t), left, top, width,
                        height),
        -1);

    return 0;
}

static QBSplashEntry *QBSplashMappingFindEntry(QBSplashMapping *splashMapping, QBSplashEntryType entryType, QBSplashOutputType outputType)
{
    unsigned int i;

    for (i = 0; i < splashMapping->entryCount; i++) {
        if (splashMapping->entries[i].type == entryType && splashMapping->entries[i].target == outputType) {
            return &splashMapping->entries[i];
        }
    }

    return NULL;
}

static QBSplashBitmap *QBSplashEntryGetBitmap(QBSplashEntry *entry, uint32_t bitmapId)
{
    ASSERT_LOG(bitmapId < entry->bitmapCount, NULL);

    return &entry->bitmaps[bitmapId];
}

// exported functions
int QBSplashDisplayLogo(QBSplash self)
{
    QBSplashEntry *logoEntry;
    QBSplashBitmap *logo;
    uint32_t surfaceId;

    ASSERT_LOG(self != NULL, -1);
    ASSERT_LOG(self->splash != NULL, -1);

    for (surfaceId = 0; surfaceId < self->surfaceCount; surfaceId++) {
        ASSERT_LOG(self->surfaces[surfaceId].pixels, -1);

        ASSERT_LOG(logoEntry = QBSplashMappingFindEntry(self->splash, QBSplashEntryType_logo, self->surfaces[surfaceId].type), -1);
#ifdef QBSPLASH_VERBOSE
        QBBootLog("%s %d %p\n", __FUNCTION__, self->surfaces[surfaceId].type, logoEntry);
#endif

        logo = QBSplashEntryGetBitmap(logoEntry, 0);
        ASSERT_LOG(logo != NULL, -1);

        ASSERT_LOG(!QBSplashSurfaceBlitImage(self->surfaces[surfaceId], logo), -1);
    }

    return 0;
}

int QBSplashAnimationDraw(QBSplash self)
{
    QBSplashEntry *animEntry;
    QBSplashBitmap *anim;
    uint32_t surfaceId;

    ASSERT_LOG(self != NULL, -1);
    ASSERT_LOG(self->splash, -1);

    for (surfaceId = 0; surfaceId < self->surfaceCount; surfaceId++) {
        ASSERT_LOG(self->surfaces[surfaceId].pixels, -1);

        ASSERT_LOG(animEntry = QBSplashMappingFindEntry(self->splash, QBSplashEntryType_anim, self->surfaces[surfaceId].type), -1);
#ifdef QBSPLASH_VERBOSE
        QBBootLog("%s %d %p\n", __FUNCTION__, self->surfaces[surfaceId].type, animEntry);
#endif

        anim = QBSplashEntryGetBitmap(animEntry, self->lastFrame);
        ASSERT_LOG(anim != NULL, -1);

        ASSERT_LOG(!QBSplashSurfaceBlitImage(self->surfaces[surfaceId], anim), -1);
    }

    return 0;
}

int QBSplashAnimationStep(QBSplash self)
{
    QBSplashEntry *animEntry;

    ASSERT_LOG(self != NULL, -1);
    ASSERT_LOG(self->splash, -1);

    ASSERT_LOG(animEntry = QBSplashMappingFindEntry(self->splash, QBSplashEntryType_anim, QBSplashOutputType_primary), -1);

    self->lastFrame++;
    if (self->lastFrame >= animEntry->bitmapCount) {
        self->lastFrame = 0;
    }

    return 0;
}

int QBSplashDrawProgress(QBSplash self, uint32_t progress)
{
    QBSplashEntry *bgEntry;
    QBSplashEntry *barEntry;
    QBSplashBitmap *bgBitmap;
    QBSplashBitmap *barBitmap;
    uint32_t surfaceId;

    ASSERT_LOG(self != NULL, -1);
    ASSERT_LOG(self->splash, -1);
    ASSERT_LOG(progress <= 100, -1);

    for (surfaceId = 0; surfaceId < self->surfaceCount; surfaceId++) {
        ASSERT_LOG(self->surfaces[surfaceId].pixels, -1);

        bgEntry = QBSplashMappingFindEntry(self->splash, QBSplashEntryType_bg, self->surfaces[surfaceId].type);
        ASSERT_LOG(bgEntry != NULL, -1);

        bgBitmap = QBSplashEntryGetBitmap(bgEntry, 0);
        ASSERT_LOG(bgBitmap != NULL, -1);

        barEntry = QBSplashMappingFindEntry(self->splash, QBSplashEntryType_bar, self->surfaces[surfaceId].type);
        ASSERT_LOG(barEntry != NULL, -1);

        barBitmap = QBSplashEntryGetBitmap(barEntry, 0);
        ASSERT_LOG(barBitmap != NULL, -1);

        if (progress < self->lastProgress) {
            ASSERT_LOG(!QBSplashSurfaceBlitImage(self->surfaces[surfaceId], bgBitmap), -1);
        }

        ASSERT_LOG(!QBSplashSurfaceBlitImagePart(self->surfaces[surfaceId], barBitmap, 0, 0, ((progress * barBitmap->width) / 100), barBitmap->height), -1);
    }
    self->lastProgress = progress;

    return 0;
}

int QBSplashClear(QBSplash self)
{
    uint8_t r, g, b, a;
    uint32_t surfaceId;

    ASSERT_LOG(self != NULL, -1);
    ASSERT_LOG(self->splash, -1);

    for (surfaceId = 0; surfaceId < self->surfaceCount; surfaceId++) {
        ASSERT_LOG(self->surfaces[surfaceId].pixels, -1);

        PIXEL32_TO_RGBA(self->splash->bgcolor, r, g, b, a);
        (void) a;

        ASSERT_LOG(!QBSplashSurfaceFill(self->surfaces[surfaceId], r, g, b), -1);
    }

    return 0;
}

int QBSplashAddSurface(QBSplash self, void *surfaceAddr, QBBitmapFormat format, uint32_t width, uint32_t height, uint32_t pitch, QBSplashOutputType outputType)
{
    int surfaceId;

    ASSERT_LOG(self != NULL, -1);
    ASSERT_LOG(self->splash, -1);
    ASSERT_LOG(surfaceAddr != NULL, -1);
    ASSERT_LOG(self->surfaceCount < self->maxSurfaceCount, -1);
    ASSERT_LOG(format == QBBitmapFormat_RGB565, -1);
    ASSERT_LOG(width > 0, -1);
    ASSERT_LOG(height > 0, -1);
    ASSERT_LOG(pitch > 0, -1);
    ASSERT_LOG(pitch == width * sizeof(uint16_t), -1);
    ASSERT_LOG(outputType != QBSplashOutputType_undefined, -1);

    surfaceId = self->surfaceCount;

    self->surfaces[surfaceId].pixels = surfaceAddr;
    self->surfaces[surfaceId].format = format;
    self->surfaces[surfaceId].width = width;
    self->surfaces[surfaceId].height = height;
    self->surfaces[surfaceId].pitch = pitch;
    self->surfaces[surfaceId].type = outputType;

#ifdef QBSPLASH_DEBUG
    QBBootLog("%s: surface %d: %p\n", __FUNCTION__, surfaceId, surfaceAddr);
#endif

    self->surfaceCount++;

    return surfaceId;
}

int QBSplashGetSurfaceInfo(QBSplash self, uint32_t surfaceId,
                           void **surfaceAddr, QBBitmapFormat *format,
                           uint32_t *width, uint32_t *height,
                           uint32_t *pitch)
{
    ASSERT_LOG(self != NULL, -1);
    ASSERT_LOG(self->splash, -1);
    ASSERT_LOG(surfaceId < self->surfaceCount, -1);
    ASSERT_LOG(surfaceAddr != NULL, -1);
    ASSERT_LOG(format != NULL, -1);
    ASSERT_LOG(width != NULL, -1);
    ASSERT_LOG(height != NULL, -1);
    ASSERT_LOG(pitch != NULL, -1);

    *surfaceAddr = self->surfaces[surfaceId].pixels;
    *format = self->surfaces[surfaceId].format;
    *width = self->surfaces[surfaceId].width;
    *height = self->surfaces[surfaceId].height;
    *pitch = self->surfaces[surfaceId].pitch;

    return 0;
}

int QBSplashInit(QBSplash self, void *splashData, size_t splashDataSize)
{
    uint32_t i;

    ASSERT_LOG(self != NULL, -1);

    // do this first so we can at least clear the screen
    self->surfaceCount = 0;
    self->maxSurfaceCount = SPLASH_MAX_SURFACE;
    for (i = 0; i < self->maxSurfaceCount; i++) {
        self->surfaces[i].pixels = NULL;
    }

    ASSERT_LOG(splashData != NULL, -1);
    self->splash = QBSplashMappingCreateFromBuffer(splashData, splashDataSize);
    ASSERT_LOG(self->splash, -1);
    self->splashData = splashData;
    self->splashDataSize = splashDataSize;

    self->lastProgress = 101; // to make sure the progress bar background gets painted the first time
    self->lastFrame = 0;

    return 0;
}

int QBSplashDeInit(QBSplash self)
{
    ASSERT_LOG(self != NULL, -1);
    ASSERT_LOG(self->splash, -1);

    QBSplashMappingDestroy(self->splash);

    return 0;
}

int QBSplashIsValid(QBSplash instance)
{
    return instance->splash != NULL;
}

int QBSplashGetSurfaceCount(QBSplash self)
{
    return self->surfaceCount;
}

QBSplash QBSplashGetInstance(void)
{
    return &g_splash;
}

void *QBSplashGetData(QBSplash self)
{
    return self->splashData;
}

size_t QBSplashGetDataSize(QBSplash self)
{
    return self->splashDataSize;
}

int QBSplashSetAnimFrame(QBSplash self, uint32_t animFrame)
{
    ASSERT_LOG(self != NULL, -1);

    self->lastFrame = animFrame;

    return 0;
}

uint32_t QBSplashGetAnimFrame(QBSplash self)
{
    return self->lastFrame;
}

QBSplashOutputDimensions QBSplashGetOutputDimensions(QBSplashOutputProfile outputProfile)
{
    QBSplashOutputDimensions outputDimensions;

    switch (outputProfile) {
    case QBSplashOutputProfile_480p_NTSC:
        case QBSplashOutputProfile_NTSC:
            outputDimensions.primaryWidth = 720;
            outputDimensions.primaryHeight = 480;
            outputDimensions.secondaryWidth = 720;
            outputDimensions.secondaryHeight = 480;
            break;
        case QBSplashOutputProfile_PAL:
        case QBSplashOutputProfile_576p_PAL:
            outputDimensions.primaryWidth = 720;
            outputDimensions.primaryHeight = 576;
            outputDimensions.secondaryWidth = 720;
            outputDimensions.secondaryHeight = 576;
            break;
        case QBSplashOutputProfile_720p60_NTSC:
            outputDimensions.primaryWidth = 1280;
            outputDimensions.primaryHeight = 720;
            outputDimensions.secondaryWidth = 720;
            outputDimensions.secondaryHeight = 480;
            break;
        case QBSplashOutputProfile_720p50_PAL:
            outputDimensions.primaryWidth = 1280;
            outputDimensions.primaryHeight = 720;
            outputDimensions.secondaryWidth = 720;
            outputDimensions.secondaryHeight = 576;
            break;
        default:
            outputDimensions.primaryWidth = 0;
            outputDimensions.primaryHeight = 0;
            outputDimensions.secondaryWidth = 0;
            outputDimensions.secondaryHeight = 0;
    }

    return outputDimensions;
}

QBSplashOutputProfile QBSplashGetOutputProfile(QBSplash self) {
    QBSplashOutputProfile undefined = QBSplashOutputProfile_undefined;
    ASSERT_LOG(self != NULL, undefined);
    ASSERT_LOG(self->splash != NULL, undefined);

    return self->splash->outputProfile;
}

void *QBSplashGetVDC(QBSplash self) {
    return self->splash->vdcData;
}
