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

#ifndef CAGE_FONT_H_
#define CAGE_FONT_H_

/**
 * @file SvFont.h Font class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CAGE/Text/SvTextTypes.h>
#include <CAGE/Text/SvGlyph.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvFont Font class
 * @ingroup CAGE_TextRendering
 * @{
 *
 * SvFont class implements a lightweight object that keeps the reference
 * to the font provider (an actual engine that renders glyph bitmaps)
 * and a set of parameters definining how text should be rendered.
 *
 * @image html SvFont.png
 **/

/**
 * CAGE font class.
 * @class SvFont
 * @extends SvObject
 **/
typedef struct SvFont_ *SvFont;


/**
 * Get default text rendering mode.
 *
 * This function returns current default text rendering mode.
 * All new instances of SvFont inherit this value.
 *
 * @return          default text rendering mode
 **/
extern SvTextRenderingMode
SvFontGetDefaultRenderingMode(void);

/**
 * Set default text rendering mode.
 *
 * This function changes default text rendering mode.
 *
 * @note Only glyphs rendered in default mode are cached.
 * To ensure that all texts are rendered according to a mode
 * specified by SvFont, glyphs cache must be flushed when
 * default text rendering mode changes.
 *
 * @param[in] mode  new default text rendering mode
 **/
extern void
SvFontSetDefaultRenderingMode(SvTextRenderingMode mode);


/**
 * Return the runtime type identification object
 * of the SvFont class.
 *
 * @return font class
 **/
extern SvType
SvFont_getType(void);

/**
 * Initialize font instance.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[in] provider  font provider handle
 * @param[out] errorOut error info
 * @return              @a self of @c NULL in case of error
 **/
extern SvFont
SvFontInit(SvFont self,
           SvObject provider,
           SvErrorInfo *errorOut);

/**
 * Initialize font instance with private font provider instance.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[in] filePath  path to a font file
 * @param[out] errorOut error info
 * @return              @a self of @c NULL in case of error
 **/
extern SvFont
SvFontInitWithFile(SvFont self,
                   const char *filePath,
                   SvErrorInfo *errorOut);

/**
 * Get font name.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @return              font name
 **/
extern const char *
SvFontGetName(SvFont self);

/**
 * Get font provider.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @return              font provider
 **/
extern SvObject
SvFontGetProvider(SvFont self);

/**
 * Get ID of the font provider.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @return              ID of the font provider
 **/
extern unsigned int
SvFontGetProviderID(SvFont self);

/**
 * Get nominal font size.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @return              nominal font size (in pixels),
 *                      @c 0 if unknown
 **/
extern unsigned int
SvFontGetSize(SvFont self);

/**
 * Set nominal font size.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[in] size      nominal font size (in pixels)
 * @param[out] errorOut error info
 **/
extern void
SvFontSetSize(SvFont self,
              unsigned int size,
              SvErrorInfo *errorOut);

/**
 * Setup font provider to use selected font size.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[in] size      nominal font size (in pixels)
 * @param[out] errorOut error info
 **/
extern void
SvFontActivateSize(SvFont self,
                   unsigned int size,
                   SvErrorInfo *errorOut);

/**
 * Get font width / height aspect.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @return              font aspect ratio
 **/
extern float
SvFontGetAspect(SvFont self);

/**
 * Set font width / height aspect.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[in] aspect    new font aspect ratio
 * @param[out] errorOut error info
 **/
extern void
SvFontSetAspect(SvFont self,
                float aspect,
                SvErrorInfo *errorOut);

/**
 * Return default text rendering mode.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @return              default text rendering mode for @a self
 **/
extern SvTextRenderingMode
SvFontGetRenderingMode(SvFont self);

/**
 * Set default text rendering mode.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[in] mode      new default text rendering mode
 * @param[out] errorOut error info
 **/
extern void
SvFontSetRenderingMode(SvFont self,
                       SvTextRenderingMode mode,
                       SvErrorInfo *errorOut);

/**
 * Get metrics of the font in current size.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[out] metrics  font metrics
 * @param[out] errorOut error info
 **/
extern void
SvFontGetMetrics(SvFont self,
                 SvFontMetrics *metrics,
                 SvErrorInfo *errorOut);

/**
 * Get single glyph.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[in] charCode  Unicode character code
 * @return              glyph for @a charCode, @c NULL if unavailable
 **/
extern SvGlyph
SvFontGetGlyph(SvFont self,
               uint32_t charCode);

/**
 * Get metrics of a single glyph.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[in] charCode  Unicode character code
 * @param[out] metrics  glyph metrics
 * @param[out] glyphIndex index of glyph we got metrics for
 * @return              @a metrics or @c NULL if unavailable
 **/
extern SvGlyphMetrics *
SvFontGetGlyphMetrics(SvFont self,
                      uint32_t charCode,
                      SvGlyphMetrics *metrics,
                      uint32_t *glyphIndex);

/**
 * Get kerning value for two consecutive glyphs.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[in] glyphA    index of a left glyph
 * @param[in] glyphB    index of a right glyph
 * @return              kerning value in 26.6 fixed point format
 **/
extern int32_t
SvFontGetKerning(SvFont self,
                 uint32_t glyphA,
                 uint32_t glyphB);

/**
 * Get single glyph adjusting its position beforehand.
 *
 * This method combines SvFontGetGlyphMetrics() and SvFontGetGlyph().
 * It gets kerning value for the (previous glyph, current glyph) pair,
 * adjusts current glyph position using the kerning value and then
 * retrieves glyph from cache or font provider. If there is subpixel
 * glyph positioning enabled, glyph is created with subpixel offset
 * equal to the fractional part of the position.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[in] prevGlyph index of the previous glyph, @c 0 if unavailable
 * @param[in] charCode  Unicode character code
 * @param[in,out] x     X position of the glyph origin on input,
 *                      adjusted X position on output
 * @return              glyph for @a charCode, @c NULL if unavailable
 **/
extern SvGlyph
SvFontGetGlyphWithAdjustment(SvFont self,
                             uint32_t prevGlyph,
                             uint32_t charCode,
                             int32_t *x);

/**
 * Get single glyph adjusting its position beforehand.
 *
 * This is another version of SvFontGetGlyphWithAdjustment().
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[in] prevGlyph index of the previous glyph, @c 0 if unavailable
 * @param[in] glyph     glyph index
 * @param[in,out] x     X position of the glyph origin on input,
 *                      adjusted X position on output
 * @return              glyph for @a charCode, @c NULL if unavailable
 **/
extern SvGlyph
SvFontGetGlyphByIndexWithAdjustment(SvFont self,
                                    uint32_t prevGlyph,
                                    uint32_t glyph,
                                    int32_t *x);

/**
 * Get glyph to represent a character that does not have
 * its own glyph in a font.
 *
 * @memberof SvFont
 *
 * @param[in] self      font handle
 * @param[in] charCode  Unicode character code
 * @return              replacement glyph for @a charCode, @c NULL if unavailable
 **/
extern SvGlyph
SvFontGetReplacementGlyph(SvFont self,
                          uint32_t charCode);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
