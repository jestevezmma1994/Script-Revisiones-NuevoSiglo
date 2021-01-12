/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef CAGE_BITMAP_H_
#define CAGE_BITMAP_H_

/**
 * @file SvBitmap.h Bitmap (software surface) class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/Sv2DRect.h>
#include <CAGE/Core/SvSurface.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvBitmap Bitmap (software surface) class
 * @ingroup CAGE
 * @{
 *
 * SvBitmap class provides definition of basic software surface (a bitmap)
 * and a set of methods for manipulating its contents (pixel storage).
 *
 * @image html SvBitmap.png
 **/

/**
 * Bitmap type.
 **/
typedef enum {
    /**
     * this value is only used to tell that bitmap type is not set, so
     * if you see this kind of bitmap it means that you found
     * an error in code
     **/
    SvBitmapType_unknown = 0,
    /**
     * bitmap stored in host memory only: can be altered using software
     * drawing operations, but can not be directly displayed; becomes
     * ::SvBitmapType_shadow on SvGfxBlitBitmap()
     **/
    SvBitmapType_local = 0x01,
    /**
     * bitmap without representation in host memory, but with valid hardware
     * surface (in graphics engine memory) attached: for bitmaps that do not
     * need to be modified; if you modify it using one of software drawing
     * operations, new blank host-side bitmap data will be allocated and
     * used until next SvGfxBlitBitmap()
     **/
    SvBitmapType_static = 0x02,
    /**
     * bitmap stored in both host memory and the graphics engine memory
     **/
    SvBitmapType_shadow = 0x03,
} SvBitmapType;

/**
 * Orientation of a picture.
 **/
typedef enum {
    /// unknown orientation
    SvBitmapOrientation_unknown        = 0,
    /// top of the scene in the first row, left side of the scene in the first column
    SvBitmapOrientation_topLeft        = 1,
    /**
     * top of the scene in the first row, left side of the scene in the last column
     * (picture mirrored horizontally)
     **/
    SvBitmapOrientation_topRight       = 2,
    /**
     * top of the scene in the last row, left side of the scene in the last column
     * (picture rotated 180 degrees)
     **/
    SvBitmapOrientation_bottomRight    = 3,
    /**
     * top of the scene in the last row, left side of the scene in the first column
     * (picture mirrored vertically)
     **/
    SvBitmapOrientation_bottomLeft     = 4,
    /**
     * top of the scene in the first column, left side of the scene in the first row
     * (X and Y coordinates swapped)
     **/
    SvBitmapOrientation_leftTop        = 5,
    /**
     * top of the scene in the last column, left side of the scene in the first row
     * (picture rotated 90 degrees clockwise)
     **/
    SvBitmapOrientation_rightTop       = 6,
    /**
     * top of the scene in the last column, left side of the scene in the last row
     * (picture rotated 180 degrees, X and Y coordinates swapped)
     **/
    SvBitmapOrientation_rightBottom    = 7,
    /**
     * top of the scene in the first column, left side of the scene in the last row
     * (picture rotated 90 degrees counterclockwise)
     **/
    SvBitmapOrientation_leftBottom     = 8
} SvBitmapOrientation;


/**
 * Bitmap class.
 * @class SvBitmap
 * @extends SvSurface
 **/
typedef struct SvBitmap_ {
    /// super class
    struct SvSurface_ super_;

    /*
     * next three fields are just copies of the values in ::super_ and
     * marking them 'const' is a way to ensure that they are in sync;
     * use SvBitmapResize() to change bitmap's dimensions
     */
    /// bitmap width in pixels
    const unsigned int width;
    /// bitmap height in pixels
    const unsigned int height;
    /// color space
    const SvColorSpace cs;

    /// bitmap type (local, static or shadow)
    SvBitmapType type;
    /// foreground color for @ref SvColorSpace::SV_CS_MONO bitmaps
    SvColor color;

    /// @c true if bitmap is completely opaque
    bool opaque;
    /// @c true if bitmap has per-pixel alpha channel
    bool hasAlpha;
    /// picture orientation
    SvBitmapOrientation orientation;

    /// handle to the surface in GFX engine memory
    SvHWSurface hwSurface;
} *SvBitmap;


/**
 * Return the runtime type identification object
 * of the SvBitmap class.
 *
 * @return bitmap class
 **/
extern SvType
SvBitmap_getType(void);

/**
 * @defgroup SvBitmap_create Creating host bitmaps
 * @{
 **/

/**
 * Initialize host bitmap with given attributes.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      bitmap handle
 * @param[in] width     bitmap width in pixels
 * @param[in] height    bitmap height in pixels
 * @param[in] csp       color space
 * @return              @a self or @c NULL in case of error
 **/
extern SvBitmap
SvBitmapInit(SvBitmap self,
             unsigned int width, unsigned int height,
             SvColorSpace csp);

/**
 * Create new host bitmap.
 *
 * @memberof SvBitmap
 *
 * @param[in] width     bitmap width in pixel
 * @param[in] height    bitmap height in pixels
 * @param[in] csp       color space
 * @return              newly created bitmap or @c NULL in case of error
 **/
extern SvBitmap
SvBitmapCreate(unsigned int width, unsigned int height,
               SvColorSpace csp);

/**
 * Create new host bitmap and allocate pixel storage.
 *
 * @memberof SvBitmap
 *
 * @param[in] width     bitmap width in pixels
 * @param[in] height    bitmap height in pixels
 * @param[in] csp       color space
 * @return              newly created bitmap or @c NULL in case of error
 **/
extern SvBitmap
SvBitmapCreateAndAlloc(unsigned int width, unsigned int height,
                       SvColorSpace csp);

/**
 * Create new host bitmap and allocate pixel storage for given pitch value.
 *
 * @memberof SvBitmap
 *
 * @param[in] width     bitmap width in pixels
 * @param[in] height    bitmap height in pixels
 * @param[in] csp       color space
 * @param[in] pitch     bytes in each allocated row of pixels
 * @return              newly created bitmap or @c NULL in case of error
 **/
extern SvBitmap
SvBitmapCreateAndAllocWithPitch(unsigned int width, unsigned int height,
                                SvColorSpace csp, unsigned int pitch);

/**
 * Create new host bitmap and fill with solid color.
 *
 * @memberof SvBitmap
 *
 * @param[in] width     bitmap width in pixels
 * @param[in] height    bitmap height in pixels
 * @param[in] csp       color space
 * @param[in] fillColor color to fill bitmap with
 * @return              newly created bitmap or @c NULL in case of error
 **/
extern SvBitmap
SvBitmapCreateAndFill(unsigned int width, unsigned int height,
                      SvColorSpace csp, SvColor fillColor);

/**
 * Allocate pixel storage for host bitmap.
 *
 * This method allocates pixel storage if bitmap does not yet have it.
 * If @a clear is @c true, allocated storage is also cleared just like in
 * SvBitmapClear(). If bitmap already has its storage allocated,
 * this method does nothing.
 *
 * @note This is the opposite operation to SvBitmapBlank().
 *
 * @memberof SvBitmap
 *
 * @param[in] self      bitmap handle
 * @param[in] clear     @c true to clear allocated storage
 * @param[out] errorOut error info
 **/
extern void
SvBitmapAllocStorage(SvBitmap self,
                     bool clear,
                     SvErrorInfo *errorOut);

/**
 * Resize host bitmap.
 *
 * This method changes the dimensions of the bitmap.
 * If the bitmap has associated surface in GFX memory,
 * it will be removed.
 *
 * The contents are modified in the following way: for both
 * dimensions (width and height), if the new dimension is smaller
 * than old one, bitmap is clipped, otherwise the new empty margin
 * is cleared (filled with black transparent pixels).
 *
 * @memberof SvBitmap
 *
 * @param[in] self      bitmap handle
 * @param[in] width     new width in pixels
 * @param[in] height    new height in pixels
 * @param[out] errorOut error info
 **/
extern void
SvBitmapResize(SvBitmap self,
               unsigned int width, unsigned int height,
               SvErrorInfo *errorOut);





/**
 * @}
 * @defgroup SvBitmap_drawing Drawing on host bitmaps
 * @{
 **/

/**
 * Fill part of a host bitmap with specified color value.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      target bitmap handle
 * @param[in] destRect  region of a bitmap to be filled,
 *                      @c NULL to fill entire bitmap
 * @param[in] color     color to fill bitmap with
 *                      (alpha channel is copied directly)
 * @param[out] errorOut error info
 **/
extern void
SvBitmapColorFill(SvBitmap self,
                  const Sv2DRect *destRect, SvColor color,
                  SvErrorInfo *errorOut);

/**
 * @}
 * @defgroup SvBitmap_basics Basic operations on host bitmaps
 * @{
 **/

/**
 * Clear (fill with black, transparent pixels) part of a host bitmap.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      target bitmap handle
 * @param[in] destRect  region of a bitmap to be cleared,
 *                      @c NULL to clear entire bitmap
 * @param[out] errorOut error info
 **/
static inline void
SvBitmapClear(SvBitmap self,
              const Sv2DRect *destRect,
              SvErrorInfo *errorOut)
{
    if (self->cs != SV_CS_MONO) {
        SvColor c = COLOR(0, 0, 0, ALPHA_TRANSPARENT);
        SvBitmapColorFill(self, destRect, c, errorOut);
    } else {
        // we can't fill MONO bitmap with non-matching color
        SvColor c = COLOR(R(self->color), G(self->color), B(self->color), ALPHA_TRANSPARENT);
        SvBitmapColorFill(self, destRect, c, errorOut);
    }
}

/**
 * Blank host bitmap.
 *
 * This method removes all bitmap data from memory.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      target bitmap handle
 * @param[out] errorOut error info
 **/
extern void
SvBitmapBlank(SvBitmap self,
              SvErrorInfo *errorOut);

/**
 * @}
 * @addtogroup SvBitmap_drawing
 * @{
 **/

/**
 * Blend color rectangle over a part of a host bitmap.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      target bitmap handle
 * @param[in] destRect  region of a bitmap to be drawn over,
 *                      @c NULL to draw over entire bitmap
 * @param[in] color     color to draw with (alpha channel
 *                      is used for blending)
 * @param[out] errorOut error info
 **/
extern void
SvBitmapColorBlend(SvBitmap self,
                   const Sv2DRect *destRect, SvColor color,
                   SvErrorInfo *errorOut);

/**
 * Blend color rectangle over a part of a host bitmap.
 *
 * This method does the same as SvBitmapColorBlend(),
 * but the region to draw to can be specified with sub-pixel
 * accuracy and operation will be performed with
 * appriopriate anti-aliasing on the borders of the filled
 * region.
 *
 * Coordinates of the region to draw to are specified in
 * fixed point 26.6 format.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      target bitmap handle
 * @param[in] destRect  region of a bitmap to be drawn over,
 *                      @c NULL to draw over entire bitmap
 * @param[in] color     color to draw with (alpha channel
 *                      is used for blending)
 * @param[out] errorOut error info
 **/
extern void
SvBitmapColorBlendFP(SvBitmap self,
                     const Sv2DRect *destRect, SvColor color,
                     SvErrorInfo *errorOut);

/**
 * Lighten or darken a part of a host bitmap.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      host bitmap handle
 * @param[in] destRect  region of a bitmap to be ligten or darken,
 *                      @c NULL to modify entire bitmap
 * @param[in] ldim      lightening or darkening factor
 *                      (@c -255 to @c -1 darken, @c 1 to @c 255 lighten)
 * @param[out] errorOut error info
 **/
extern void
SvBitmapColorBurn(SvBitmap self,
                  const Sv2DRect *destRect, int ldim,
                  SvErrorInfo *errorOut);

/**
 * Copy part of one host bitmap to another host bitmap.
 *
 * @memberof SvBitmap
 *
 * @param[in] dest      target bitmap handle
 * @param[in] destX     horizontal position of destination's upper left corner
 * @param[in] destY     vertical position of destination's upper left corner
 * @param[in] src       source bitmap handle
 * @param[in] srcRect   source region in a @a src bitmap,
 *                      @c NULL to copy entire surface
 * @param[out] errorOut error info
 **/
extern void
SvBitmapCopy(SvBitmap dest, int destX, int destY,
             SvBitmap src, const Sv2DRect *srcRect,
             SvErrorInfo *errorOut);

/**
 * Blend part of one host bitmap to another host bitmap.
 *
 * @memberof SvBitmap
 *
 * @param[in] dest      target bitmap handle
 * @param[in] destX     horizontal position of destination's upper left corner
 * @param[in] destY     vertical position of destination's upper left corner
 * @param[in] src       source bitmap handle
 * @param[in] srcRect   source region in a @a src bitmap,
 *                      @c NULL to copy entire surface
 * @param[in] alpha     global alpha modifier for blending
 * @param[out] errorOut error info
 **/
extern void
SvBitmapBlend(SvBitmap dest, int destX, int destY,
              SvBitmap src, const Sv2DRect *srcRect,
              SvPixel alpha,
              SvErrorInfo *errorOut);

/**
 * Get single pixel from a host bitmap.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      bitmap handle
 * @param[in] x         horizontal coordinate of the pixel
 * @param[in] y         vertical coordinate of the pixel
 * @param[out] errorOut error info
 * @return              pixel value, @c 0 in case of error
 **/
extern SvColor
SvBitmapGetPixel(SvBitmap self,
                 unsigned int x, unsigned int y,
                 SvErrorInfo *errorOut);

/**
 * Draw single pixel to a host bitmap.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      bitmap handle
 * @param[in] x         horizontal coordinate of the pixel
 * @param[in] y         vertical coordinate of the pixel
 * @param[in] color     pixel value
 * @param[out] errorOut error info
 **/
extern void
SvBitmapPutPixel(SvBitmap self,
                 unsigned int x, unsigned int y,
                 SvColor color,
                 SvErrorInfo *errorOut);

/**
 * Blend single pixel to a host bitmap.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      bitmap handle
 * @param[in] x         horizontal coordinate of the pixel
 * @param[in] y         vertical coordinate of the pixel
 * @param[in] color     pixel value
 * @param[out] errorOut error info
 **/
extern void
SvBitmapBlendPixel(SvBitmap self,
                   unsigned int x, unsigned int y,
                   SvColor color,
                   SvErrorInfo *errorOut);

/**
 * Fill part of a host bitmap with a linear gradient.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      target bitmap handle
 * @param[in] destRect  region of a bitmap to be filled,
 *                      @c NULL to fill entire bitmap
 * @param[in] angle     gradient angle
 * @param[in] colorA    start gradient color
 * @param[in] colorB    end gradient color
 * @param[out] errorOut error info
 **/
extern void
SvBitmapGradientFill(SvBitmap self,
                     const Sv2DRect *destRect, int angle,
                     SvColor colorA, SvColor colorB,
                     SvErrorInfo *errorOut);

/**
 * Blend linear gradient over a part of a host bitmap.
 *
 * @memberof SvBitmap
 *
 * @param[in] self      target bitmap handle
 * @param[in] destRect  region of a bitmap to be drawn over,
 *                      @c NULL to draw on entire bitmap
 * @param[in] angle     gradient angle
 * @param[in] colorA    start gradient color (alpha channel
 *                      is used for blending)
 * @param[in] colorB    end gradient color (alpha channel
 *                      is used for blending)
 * @param[out] errorOut error info
 **/
extern void
SvBitmapGradientBlend(SvBitmap self,
                      const Sv2DRect *destRect, int angle,
                      SvColor colorA, SvColor colorB,
                      SvErrorInfo *errorOut);

/**
 * @}
 * @defgroup SvBitmap_scaling Scaling host bitmaps
 * @{
 **/

/**
 * Scale part of a host bitmap writing it to another host bitmap.
 *
 * @note Destination and source bitmaps must be in the same color space!
 *
 * @memberof SvBitmap
 *
 * @param[in] dest      target bitmap handle
 * @param[in] destRect  destination region in a @a dest bitmap,
 *                      @c NULL to write scaled data over entire @a dest bitmap
 * @param[in] src       source bitmap handle
 * @param[in] srcRect   source region in a @a src bitmap,
 *                      @c NULL to scale entire @a src bitmap
 * @param[out] errorOut error info
 **/
extern void
SvBitmapScale(SvBitmap dest, const Sv2DRect *destRect,
              SvBitmap src, const Sv2DRect *srcRect,
              SvErrorInfo *errorOut);

/**
 * @}
 * @defgroup SvBitmap_pixelOps Direct operations on pixel data
 * @{
 **/

/**
 * Pixel copying function.
 *
 * @param[in] dest  destination pixel data
 * @param[in] src   source pixel data
 * @param[in] color source pixel color for @ref SvColorSpace::SV_CS_MONO
 * @param[in] n     number of pixels to copy
 **/
typedef void
(*SvPixelCopyFn)(SvPixel *dest,
                 const SvPixel *src,
                 SvColor srcColor,
                 unsigned int n);


/**
 * Get function that copies bitmap pixels from one color space to another.
 *
 * @param[in] dcsp  destination color space
 * @param[in] scsp  source color space
 * @return          pointer to a pixel copying function,
 *                  @c NULL if not found
 **/
extern SvPixelCopyFn
SvBitmapGetPixelCopyFn(SvColorSpace dcsp,
                       SvColorSpace scsp);

/**
 * Pixel blending function.
 *
 * @param[in] dest  destination pixel data
 * @param[in] src   source pixel data
 * @param[in] color source pixel color for @ref SvColorSpace::SV_CS_MONO
 * @param[in] alpha source pixel alpha
 * @param[in] n     number of pixels to blend
 **/
typedef void
(*SvPixelBlendFn)(SvPixel *dest,
                  const SvPixel *src,
                  SvColor srcColor,
                  SvPixel alpha,
                  unsigned int n);

/**
 * Get function that blends bitmap pixels from one color space to another.
 *
 * @param[in] dcsp  destination color space
 * @param[in] scsp  source color space
 * @return          pointer to a pixel blending function,
 *                  @c NULL if not found
 **/
extern SvPixelBlendFn
SvBitmapGetPixelBlendFn(SvColorSpace dcsp,
                        SvColorSpace scsp);

/**
 * Pixel color blending function.
 *
 * @param[in] dest  destination pixel data
 * @param[in] color source pixel color
 * @param[in] n     number of pixels to blend
 **/
typedef void
(*SvPixelColorBlendFn)(SvPixel *dest,
                       SvColor color,
                       unsigned int n);

/**
 * Get function that blends color pixels to a bitmap.
 *
 * @param[in] dcsp  destination color space
 * @return          pointer to a color blending function,
 *                  @c NULL if not found
 **/
extern SvPixelColorBlendFn
SvBitmapGetColorBlendFn(SvColorSpace dcsp);

/**
 * Pixel alpha multiplication function.
 *
 * @param[in] data  pixel data
 * @param[in] n     number of pixels to multiply by alpha
 *                  or unassociate
 **/
typedef void
(*SvPixelColorMultiplyFn)(SvPixel *data,
                          unsigned int n);

/**
 * Get function that premultiplies color components by alpha factor.
 *
 * @param[in] csp   color space
 * @return          pointer to a color multiplication function,
 *                  @c NULL if not found
 **/
extern SvPixelColorMultiplyFn
SvBitmapGetColorPremultiplyFn(SvColorSpace csp);

/**
 * Get function that unassociates color components from alpha factor.
 *
 * Returned function does the reverse operation to function returned
 * by SvBitmapGetColorPremultiplyFn().
 *
 * @param[in] csp   color space
 * @return          pointer to a color unassociation function,
 *                  @c NULL if not found
 **/
extern SvPixelColorMultiplyFn
SvBitmapGetColorUnassociateFn(SvColorSpace csp);


/**
 * @}
 * @defgroup SvBitmap_rawAccess Accessing bitmap pixel storage
 * @{
 **/

/**
 * Get pointer to a row of pixels starting with pixel at given coordinates.
 *
 * @memberof SvBitmap
 *
 * @param[in] self  bitmap handle
 * @param[in] x     horizontal coordinate of the pixel
 * @param[in] y     vertical coordinate of the pixel
 * @return          pointer to a pixel data, @c NULL if unavailable
 **/
extern SvPixel *
SvBitmapGetRowStorage(SvBitmap self,
                      unsigned int x, unsigned int y);

/**
 * Copy data from row of pixels to proper row in bitmap
 *
 * @memberof SvBitmap
 *
 * @param[in] self      bitmap handle
 * @param[in] row       row of pixels to put into bitmap
 * @param[in] copyFn    pixel copying function
 * @param[in] scsp      source color space
 * @param[in] y         vertical coordinate of the destination row
 * @param[out] errorOut error info
 **/
extern void
SvBitmapPushRow(SvBitmap self, SvPixel* row, SvPixelCopyFn copyFn,
                SvColorSpace scsp, unsigned int y, SvErrorInfo *errorOut);

/**
 * @}
 **/

/**
 * @defgroup SvBitmap_IO Bitmap input/output
 * @ingroup SvBitmap
 **/

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
