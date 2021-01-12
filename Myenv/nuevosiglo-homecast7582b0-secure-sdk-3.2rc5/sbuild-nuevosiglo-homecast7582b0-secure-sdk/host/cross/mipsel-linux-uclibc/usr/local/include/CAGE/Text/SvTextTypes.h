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

#ifndef CAGE_TEXT_TYPES_H_
#define CAGE_TEXT_TYPES_H_

/**
 * @file SvTextTypes.h Types used in text rendering API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup CAGE_TextRendering Text rendering
 * @ingroup CAGE
 * @{
 *
 * A module grouping CAGE text rendering functionality.
 **/

/**
 * Text rendering mode.
 *
 * These flags define the text rendering mode. Not all of them
 * can (or should) be combined, for example subpixel positioning
 * and hinting do not make sense together.
 **/
typedef enum {
    /// enable bytecode-based hinting
    SvTextRenderingMode_hinting       = 0x01,
    /// enable FreeType autohinter
    SvTextRenderingMode_autoHinting   = 0x02,
    /// enable kerning
    SvTextRenderingMode_kerning       = 0x04,
    /// enable light subpixel glyph positioning (to 1/4th of a pixel)
    SvTextRenderingMode_lightSubPixelPositioning = 0x08,
    /// enable full subpixel glyph positioning (to 1/64th of a pixel)
    SvTextRenderingMode_subPixelPositioning = 0x18,
} SvTextRenderingMode;

/**
 * Text alignment types.
 **/
typedef enum {
    SvTextAlignment_left,       ///< align to the left
    SvTextAlignment_center,     ///< center text in the line
    SvTextAlignment_right,      ///< align to the right
    SvTextAlignment_justify     ///< justify text
} SvTextAlignment;

/**
 * Font metrics.
 **/
typedef struct {
    /// font ascender in fixed point 26.6 format
    int32_t ascender;
    /// font descender in fixed point 26.6 format
    int32_t descender;
    /// font height in fixed point 26.6 format
    int32_t height;
    /// position of the underline, relative to base line, in fixed point 26.6 format
    int32_t underlinePosition;
    /// thickness of the underline in fixed point 26.6 format
    uint32_t underlineThickness;
    /// reserved space for future extensions
    uint32_t reserved[7];
} SvFontMetrics;

/**
 * Font weights.
 **/
typedef enum {
    /// thinnest available weight
    SvFontWeight_hairLine = -5,
    /// very light weight
    SvFontWeight_ultraLight = -4,
    /// usual light weight
    SvFontWeight_light = -1,
    /// default weight
    SvFontWeight_normal = 0,
    /// weight one step heavier than default
    SvFontWeight_semiBold = 2,
    /// usual bold weight
    SvFontWeight_bold = 3,
    /// heaviest available weight
    SvFontWeight_black = 7,
} SvFontWeight;

/**
 * Font styles.
 **/
typedef enum {
    /// default font style
    SvFontStyle_normal = 0,
    /// italic font style
    SvFontStyle_italic = 1,
    /// font style with slanted glyphs
    SvFontStyle_oblique = 2,
} SvFontStyle;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
