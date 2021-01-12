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

#ifndef CAGE_COLOR_H_
#define CAGE_COLOR_H_

/**
 * @file SvColor.h CAGE color-related definitions
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvColor Color-related definitions
 * @ingroup CAGE
 * @{
 *
 * SvPixel, SvColor and SvColorSpace types are used extensively
 * in the CAGE library API.
 **/


/**
 * Pixel type.
 *
 * SvPixel type represents a single channel information of one bitmap pixel.
 **/
typedef uint8_t SvPixel;

/**
 * Color type.
 *
 * SvColor type can be used to represent a complete pixel in RGBA color space.
 **/
typedef uint32_t SvColor;

/** Construct color value from RGBA channel values. */
#define COLOR(R, G, B, A) ((((R) & 0xffU) << 24) | \
                           (((G) & 0xffU) << 16) | \
                           (((B) & 0xffU) << 8) | \
                            ((A) & 0xffU))

/** Get value of color's red channel. */
#define R(COL) ((COL) >> 24)
/** Get value of color's green channel. */
#define G(COL) (((COL) >> 16) & 0xFF)
/** Get value of color's blue channel. */
#define B(COL) (((COL) >> 8) & 0xFF)
/** Get value of color's alpha channel. */
#define A(COL) ((COL) & 0xFF)

/** Value of full transparency in alpha channel. */
#define ALPHA_TRANSPARENT  0
/** Value of full solid pixel in alpha channel. */
#define ALPHA_SOLID        0xFF

/**
 * Types of color spaces.
 **/
enum {
    /// alpha-only color spaces
    FCSP_A      = 0 << 3,
    /// RGB with and without alpha channel
    FCSP_RGB    = 1 << 3,
    /// alias to RGB for backward compatibility
    FCSP_BGR    = FCSP_RGB,
    /// packed RGB with and without alpha channel
    FCSP_PAK    = 2 << 3,
    /// YCbCr with and without alpha channel
    FCSP_YUV    = 3 << 3,
    /// hardware-specific color spaces, not allowed for host bitmaps
    FCSP_HWSPEC = 7 << 3
};

/**
 * A macro for defining color space constants.
 *
 * This macro expands to the color space value containing four fields:
 * bytes-per-pixel value (@a bpp) on bits 0 to 2, color space type
 * (@a csp) on bits 3 to 5, @a alpha flag on bit 6 and memory byte order
 * @a variant on higher order bits.
 **/
#define FCSP(bpp, csp, alpha, var) \
    (((var) << 7) | ((alpha & 0x01) << 6) | ((FCSP_ ## csp) & 0x38) | ((bpp) & 0x07))

/**
 * CAGE bitmap color space.
 *
 * CAGE handles bitmaps in multiple different color spaces.
 * Note that some of those (especially YUV and packed RGB types) may not be
 * available on all platforms. Applications should create and load bitmaps
 * in such color space, that is supported and handled efficiently on target
 * platform. The names of all RGB color spaces denote the memory byte order.
 **/
typedef enum {
    /**
     * \a SV_CS_MONO is an 8 bpp single-channel alpha-only color space.
     * Typically, when rendering the bitmap, the actual color will be acquired
     * from the bitmap object itself and the bitmap data will be used
     * as a per-pixel alpha modifier.
     **/
    SV_CS_MONO      = FCSP(1, A,   1, 0),

    /// RGB, 24 bpp
    SV_CS_RGB       = FCSP(3, RGB, 0, 0),
    /// RGB, 32 bpp
    SV_CS_RGB32     = FCSP(4, RGB, 0, 0),
    /// RGB + alpha, 32 bpp
    SV_CS_RGBA      = FCSP(4, RGB, 1, 0),
    /// RGB, 24 bpp, BGR memory order
    SV_CS_BGR       = FCSP(3, RGB, 0, 1),
    /// RGB, 32 bpp, BGR memory order
    SV_CS_BGR32     = FCSP(4, RGB, 0, 1),
    /// RGB + alpha, 32 bpp, BGRA memory order
    SV_CS_BGRA      = FCSP(4, RGB, 1, 1),
    /// RGB + alpha, 32 bpp, ARGB memory order
    SV_CS_ARGB      = FCSP(4, RGB, 1, 2),
    /// RGB + alpha, 32 bpp, ABGR memory order
    SV_CS_ABGR      = FCSP(4, RGB, 1, 3),
    /**
     * RGB + alpha, 16 bpp (4 bits per component),
     * (A << 16 | R) in first byte and (G << 16 | B) in second byte
     **/
    SV_CS_ARGB_4444 = FCSP(2, RGB, 1, 2),
    /**
     * RGB + alpha, 16 bpp (4 bits per component),
     * (G << 16 | B) in first byte and (A << 16 | R) in second byte
     **/
    SV_CS_BGRA_4444 = FCSP(2, RGB, 1, 1),

    /// YCbCr 4:4:4, 24 bpp
    SV_CS_YUV       = FCSP(3, YUV, 0, 0),
    /// YCbCr 4:4:4 + alpha, 32 bpp
    SV_CS_YUVA      = FCSP(4, YUV, 1, 0),

    /**
     * packed RGB (5 bits red, 6 green, 5 blue), 16 bpp,
     * memory byte order is host dependent
     **/
    SV_CS_R565      = FCSP(2, PAK, 0, 0),
    /**
     * packed RGB (5 bits for each color channel) + binary (fully transparent
     * or fully opaque) alpha on the least significant bit
     **/
    SV_CS_R555A1    = FCSP(2, PAK, 1, 0),
    /**
     * packed RGB (5 bits blue, 6 green, 5 red), 16 bpp,
     * memory byte order is host dependent
     **/
    SV_CS_B565      = FCSP(2, PAK, 0, 1),
    /**
     * packed RGB (5 bits for each color channel) + binary (fully transparent
     * or fully opaque) alpha on the most significant bit
     **/
    SV_CS_A1R555    = FCSP(2, PAK, 1, 2)
} SvColorSpace;

/** Get bytes-per-pixel value from color space value. */
#define BPP(csp)    ((csp) & 0x07)
/** Get color space type from color space value. */
#define CSP(csp)    ((csp) & 0x38)
/** Get alpha flag from color space value. */
#define ACSP(csp)   ((csp) & 0x40)


/**
 * Get textual name for a color space.
 *
 * @param[in] csp   color space type
 * @return          color space name
 **/
extern const char *
SvColorSpaceGetName(SvColorSpace csp);

/**
 * Lighten or darken a color.
 *
 * @param[in] src    original color
 * @param[in] ldim   lightening or darkening factor
 *                   (@c -255 to @c -1 darken, @c 1 to @c 255 lighten)
 * @return           modified color
 **/
extern SvColor
SvColorBurn(SvColor src, int ldim);

/**
 * Convert color representation from YCbCr color space
 * to RGB color space.
 *
 * @param[in] y     luminance value
 * @param[in] cb    blue chrominance value
 * @param[in] cr    red chrominance value
 * @param[in] a     alpha value
 * @return          packed RGBA color value
 **/
extern SvColor
SvColorFromYCbCrA(SvPixel y, SvPixel cb, SvPixel cr, SvPixel a);

/**
 * Convert color representation from RGB color space
 * to YCbCr color space.
 *
 * @param[in] c     packed RGBA color value
 * @return          packed YCbCrA color value
 **/
extern uint32_t
SvColorToYCbCrA(SvColor c);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
