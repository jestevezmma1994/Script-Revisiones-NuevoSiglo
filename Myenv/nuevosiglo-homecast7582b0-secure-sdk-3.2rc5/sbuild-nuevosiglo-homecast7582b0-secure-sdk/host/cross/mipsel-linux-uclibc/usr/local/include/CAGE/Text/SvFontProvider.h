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

#ifndef CAGE_FONT_PROVIDER_H_
#define CAGE_FONT_PROVIDER_H_

/**
 * @file SvFontProvider.h Font provider interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <CAGE/Text/SvGlyph.h>
#include <CAGE/Text/SvFont.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvFontProvider Font provider interface
 * @ingroup CAGE_TextRendering
 * @{
 *
 * SvFontProvider is an abstract interface to an engine that provides
 * a set of glyphs together with their metrics information and is able
 * to render (rasterize) glyphs into bitmaps.
 **/

/**
 * Get runtime type identification object representing
 * font provider interface.
 *
 * @return SvFontProvider interface object
 **/
extern SvInterface
SvFontProvider_getInterface(void);

/**
 * Generate unique font provider ID.
 *
 * @return      unique font provider ID
 **/
extern unsigned int
SvFontProviderGenerateID(void);


/**
 * SvFontProvider interface.
 **/
typedef const struct SvFontProvider_ {
   /**
    * Get font provider ID.
    *
    * @param[in] self_     handle to an object implementing SvFontProvider interface
    * @return              unique identifier
    **/
   uint16_t (*getID)(SvObject self_);

   /**
    * Get font name.
    *
    * @param[in] self_     handle to an object implementing SvFontProvider interface
    * @return              font name
    **/
   SvString (*getName)(SvObject self_);

   /**
    * Check if provider supports subpixel glyph positioning.
    *
    * @param[in] self_     handle to an object implementing SvFontProvider interface
    * @return              @c true if provider supports subpixel positioning
    **/
   bool (*isSubPixelPositioningSupported)(SvObject self_);

   /**
    * Setup new font size.
    *
    * @param[in] self_     handle to an object implementing SvFontProvider interface
    * @param[in] size      nominal font size in pixels
    * @param[in] aspect    font width / height aspect in fixed point 26.6 format
    * @return              font size index, @c 0 in case of error
    **/
   uint16_t (*setupSize)(SvObject self_,
                         unsigned int size,
                         unsigned int aspect);

   /**
    * Prepare font size for use.
    *
    * @param[in] self_     handle to an object implementing SvFontProvider interface
    * @param[in] sizeIndex font size index returned by SvFontProvider_::setupSize()
    **/
   void (*activateSize)(SvObject self_,
                        uint16_t sizeIndex);

   /**
    * Release font size that has been previously set up.
    *
    * @param[in] self_     handle to an object implementing SvFontProvider interface
    * @param[in] sizeIndex font size index returned by last SvFontProvider_::setupSize() call
    **/
   void (*releaseSize)(SvObject self_,
                       uint16_t sizeIndex);

   /**
    * Get metrics of the font in given size.
    *
    * @param[in] self      handle to an object implementing SvFontProvider interface
    * @param[in] sizeIndex font size index returned by last SvFontProvider_::setupSize() call
    * @param[out] metrics  font metrics
    * @param[out] errorOut error info
    **/
   void (*getMetrics)(SvObject self_,
                      uint16_t sizeIndex,
                      SvFontMetrics *metrics,
                      SvErrorInfo *errorOut);

   /**
    * Find index of a glyph representing given Unicode character.
    *
    * @param[in] self_     handle to an object implementing SvFontProvider interface
    * @param[in] charCode  character code to find glyph for
    * @return              glyph index or @c 0 in case of error
    **/
   uint32_t (*getGlyphIndex)(SvObject self_,
                             uint32_t charCode);

   /**
    * Get metrics of a single glyph.
    *
    * @param[in] self_     handle to an object implementing SvFontProvider interface
    * @param[in] sizeIndex font size index returned by the SvFontProvider_::setupSize() call
    * @param[in] index     glyph index
    * @param[in] mode      text rendering mode
    * @param[out] metrics  glyph metrics
    * @return              @a metrics or @c NULL in case of error
    **/
   SvGlyphMetrics *(*getGlyphMetrics)(SvObject self_,
                                      uint16_t sizeIndex,
                                      uint32_t glyphIndex,
                                      int mode,
                                      SvGlyphMetrics *metrics);

   /**
    * Render single glyph.
    *
    * @param[in] self_     handle to an object implementing SvFontProvider interface
    * @param[in] sizeIndex font size index returned by the SvFontProvider_::setupSize() call
    * @param[in] index     glyph index
    * @param[in] mode      text rendering mode
    * @param[in] subPixelOffset fractional part of the glyph X position
    *                      in fixed point 26.6 format: 0 <= @a subPixelOffset < 64
    * @return              created glyph (owned by caller), @c NULL in case of error
    **/
   SvGlyph (*getGlyph)(SvObject self_,
                       uint16_t sizeIndex,
                       uint32_t glyphIndex,
                       int mode,
                       uint8_t subPixelOffset);

   /**
    * Get horizontal kerning value for two consecutive glyphs.
    *
    * @param[in] self_     handle to an object implementing SvFontProvider interface
    * @param[in] sizeIndex font size index returned by the SvFontProvider_::setupSize() call
    * @param[in] indexA    index of a left glyph
    * @param[in] indexB    index of a right glyph
    * @return              horizontal kerning value in fixed point 26.6 format
    **/
   int32_t (*getKerning)(SvObject self_,
                         uint16_t sizeIndex,
                         uint32_t indexA, uint32_t indexB);
} *SvFontProvider;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
