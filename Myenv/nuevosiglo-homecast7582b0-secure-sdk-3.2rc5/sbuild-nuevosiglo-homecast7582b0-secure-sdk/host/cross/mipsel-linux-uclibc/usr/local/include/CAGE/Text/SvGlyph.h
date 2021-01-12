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

#ifndef CAGE_GLYPH_H_
#define CAGE_GLYPH_H_

/**
 * @file SvGlyph.h CAGE glyph class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <SvCore/SvAllocator.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvGlyph Glyph class
 * @ingroup CAGE_TextRendering
 * @{
 *
 * SvGlyph class represents single glyph of a font in a selected
 * size, usually also containing a rendered (rasterized)
 * bitmap representation.
 **/

/**
 * Glyph metrics.
 **/
typedef struct {
    /// advance in floating point 26.6 format
    int32_t advX;
    /// X coordinate of leftmost point in the glyph in 26.6 format
    int32_t minX;
    /// Y coordinate of the highest point in the glyph in 26.6 format
    int32_t minY;
    /// X coordinate of rightmost point in the glyph in 26.6 format
    int32_t maxX;
    /// Y coordinate of the lowest point in the glyph in 26.6 format
    int32_t maxY;
    /// X coordinate of the glyph bitmap's top-left corner
    int32_t bmpX;
    /// Y coordinate of the glyph bitmap's top-left corner
    int32_t bmpY;
    /// glyph bitmap's width
    int32_t bmpW;
    /// glyph bitmap's height
    int32_t bmpH;
} SvGlyphMetrics;

/**
 * Glyph class.
 *
 * @note This is not an SvObject!
 **/
typedef struct SvGlyph_ {
    /// reference counter
    int32_t retainCount;

    /// unique identifier of a font provider instance that created this glyph
    uint16_t fontID;
    /// identifier of a font size, unique inside the provider
    uint16_t fontSizeIndex;
    /// identifier of a glyph, unique inside the provider
    uint32_t glyphIndex;
    /// one of possibly many Unicode characters represented by this glyph
    uint32_t charCode;

    /**
     * explicitly specified padding: the compiler would add it anyway,
     * but this way we know that it can be replaced by actual attribute
     * in the future
     **/
    uint16_t pad;

    /// subpixel offset of the rendered glyph in fixed point 26.6 format
    uint16_t subPixelOffset;

    /// element of the (character code -> glyph) hash table, used by glyphs cache
    struct SvGlyph_ *htCNext;
    /// element of the (glyph index -> glyph) hash table, used by glyphs cache
    struct SvGlyph_ *htINext;
    /// element of the last recently used glyphs list, used by glyphs cache
    struct {
        /// previous element on the LRU list
        struct SvGlyph_ *prev;
        /// next element on the LRU list
        struct SvGlyph_ *next;
    } lru;

    /// glyph metrics
    SvGlyphMetrics metrics;

    /// grayscale bitmap data, one byte per pixel, @a metrics.bmpW * @a metrics.bmpH bytes long
    uint8_t pixels[1];
} *SvGlyph;


/**
 * Increase reference counter value of a glyph object.
 *
 * @param[in] self  glyph handle
 * @return          @a self
 **/
static inline SvGlyph
SvGlyphRetain(SvGlyph self)
{
    self->retainCount += 1;
    return self;
}

/**
 * Decrease reference counter value of a glyph object,
 * destroying it if neccessary.
 *
 * @param[in] self  glyph handle
 **/
static inline void
SvGlyphRelease(SvGlyph self)
{
    if ((self->retainCount -= 1) == 0)
        SvAllocatorDeallocate(SvDefaultAllocator, self);
}

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
