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

#include <SvFoundation/SvObject.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <CAGE/Core/SvBitmap.h>
#include <CAGE/IO/SvBitmapPNGIO.h>
#include <SvGfxHAL/SvHWSurface.h>
#include <SvGfxHAL/SvGfxEngine.h>
#include <SvGfxHAL/SvGfxUtils.h>
#include <QBResourceManager/QBResourceManager.h>
#include <QBResourceManager/SvRBLocator.h>
#include <QBResourceManager/SvRBBitmap.h>
#include <QBResourceManager/rb.h>
#include "loadFile.h"

/**
 * Creates a new static cliped bitmap from source bitmap
 *
 * @param[in] srcBitmap bitmap to clip
 * @param[in] clipedRect coordinates how to clip bitmap: offsetX, offsetX, wight and height
 * @return static cliped bitmap
 **/
SvLocal SvBitmap createClippedBitmap(SvBitmap srcBitmap, const Sv2DRect *clippedRect)
{
    SvBitmap bitmap = SvBitmapCreate(clippedRect->width, clippedRect->height, srcBitmap->cs);
    bitmap->hwSurface = SvGfxEngineCreateSurface(clippedRect->width, clippedRect->height, srcBitmap->cs);
    SvHWSurfaceBlit(bitmap->hwSurface, 0, 0, srcBitmap, clippedRect);
    bitmap->type = SvBitmapType_static;
    return bitmap;
}

SvBitmap createClippedPNG(const char *filename, SvBitmapType bt, const Sv2DRect *clippedRect)
{
    SvBitmap origBmp = NULL, bmp = NULL;
    char *path = NULL;

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBResourceManager resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));

    SvRBLocator locator = QBResourceManagerGetResourceLocator(resourceManager);
    path = SvRBLocatorFindFile(locator, filename);
    if (!path)
        goto fini;

    origBmp = SvBitmapCreateFromPNG(path, NULL);
    if (!origBmp)
        goto fini;

    if (clippedRect->x < 0 || clippedRect->y < 0 || clippedRect->width > origBmp->width || clippedRect->height > origBmp->height) {
        SvLogError("%s(): invalid clipping rect %ux%u@%d:%d for %ux%u bitmap %s",
                __func__, clippedRect->width, clippedRect->height, clippedRect->x, clippedRect->y, origBmp->width, origBmp->height, filename);
        goto fini;
    }

    if (bt == SvBitmapType_static) {
        bmp = createClippedBitmap(origBmp, clippedRect);
    } else {
        bmp = SvBitmapCreate(clippedRect->width, clippedRect->height, origBmp->cs);
        SvBitmapCopy(bmp, 0, 0, origBmp, clippedRect, NULL);
        bmp->type = bt;
        if (bt == SvBitmapType_shadow) {
            SvGfxBlitBitmap(bmp, NULL);
        }
    }

fini:
    if (bmp) {
        SvRBBitmap rbBitmap = SvRBBitmapCreateWithBitmap(bmp);
        if (rbBitmap) {
            svRBAddItem(resourceManager, (SvRBObject) rbBitmap, SvRBPolicy_static);
            SVRELEASE(rbBitmap);
        }
    }

    SVTESTRELEASE(origBmp);
    free(path);
    return bmp;
}
