/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CAGE_TEXT_LAYOUT_H_
#define CAGE_TEXT_LAYOUT_H_

/**
 * @file SvTextLayout.h Text layout class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CAGE/Core/Sv2DRect.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Text/SvTextTypes.h>
#include <CAGE/Text/SvFont.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvTextLayout Text layout class
 * @ingroup CAGE_TextRendering
 * @{
 *
 * SvTextLayout class is an implementation of the text layout engine:
 * it processes text provided by a text paragraph object and turns it
 * into an array of graphical atoms (glyphs, lines) attached to
 * their positions in the virtual text box.
 **/

/**
 * CAGE text layout class.
 * @class SvTextLayout
 * @extends SvObject
 **/
typedef struct SvTextLayout_ *SvTextLayout;


/**
 * A set of attributes defining how a run of text should be rendered.
 **/
typedef struct {
    /// a font to use for rendering
    SvFont font;
    /// font size in pixels
    int fontSize;
    /// font width / height aspect
    float aspect;
    /// color of the text
    SvColor color;
    /// tracking (a.k.a. letter spacing) in 26.6 fixed point format
    int tracking;
    /// @c true to underline text
    bool underline;
    /// @c true to enable line-through
    bool lineThrough;
    /// fields reserved for future extensions, initialize with zeros
    uint32_t reserved[4];
} SvTextAttributesSet;


/**
 * Types of text layout atoms.
 **/
enum {
    /// an atom describing single glyph
    SvTextLayoutAtomType_glyph = 1,
    /// an atom describing horizontal line
    SvTextLayoutAtomType_horizLine = 2,
    /// an atom describing new set of text rendering attributes
    SvTextLayoutAtomType_attrsSet = 3,
    /// an atom marking the start of a new line of text
    SvTextLayoutAtomType_startOfLine = 4,
    /// custom user-defined atom
    SvTextLayoutAtomType_userDefined = 5,
};

/**
 * Text layout atom.
 **/
typedef union {
    /// type of the atom
    uint8_t type;
    /// glyph description
    struct {
        /// type of the atom
        uint8_t type;
        /// flags
        uint8_t flags;
        /**
         * index of the character from text paragraph represented by this
         * glyph, @c -1 when glyph does not represent any character
         * (for example: a line-break hyphen or an ellipsis added artificially
         * to mark that text could not fit in available area)
         **/
        int16_t characterIndex;
        /**
         * X coordinate of the glyph's origin on the baseline in 26.6 format;
         * when @a glyph is @c NULL because previous glyph is a ligature, this
         * is a position where text cursor should be placed inside the ligature
         * to be standing in front of the character
         **/
        int32_t x;
        /**
         * retained glyph handle, @c NULL if there is no distinct glyph
         * representing a character (for example: glyph for that character
         * is missing or character is represented by previous ligature glyph)
         **/
        SvGlyph glyph;
    } glyph;
    /// horizontal line (underline or line-through) description
    struct {
        /// type of the atom
        uint8_t type;
        uint8_t pad;
        /// line width in 10.6 fixed point format
        uint16_t width;
        /// vertical position of the line in 10.6 fixed point format
        int16_t y;
        /// index of the first glyph influenced by this line
        uint16_t firstGlyphIndex;
        /// index of the last glyph influenced by this line
        uint16_t lastGlyphIndex;
    } horizLine;
    /// new set of text attributes to be used for all consecutive glyphs
    struct {
        /// type of the atom
        uint8_t type;
        uint8_t pad[7];
        /// text attributes
        SvTextAttributesSet *attrs;
    } attrsSet;
    /// start of line marker
    struct {
        /// type of the atom
        uint8_t type;
        uint8_t pad;
        /// line number, starting with @c 0
        uint16_t lineNumber;
    } startOfLine;
    struct {
        /// type of the atom
        uint8_t type;
        /// flags
        uint8_t flags;
        /// padding
        int16_t pad;
        /// X coordinate of the custom atom
        int32_t x;
        /// private data
        SvObject prv;
    } userDefined;
} SvTextLayoutAtom;


/**
 * Single line of a text layout.
 **/
typedef struct {
    /// line number in the layout
    unsigned int number;

    /// a set of text attributes this line starts with
    const SvTextAttributesSet *initialAttrsSet;

    /// number of characters
    unsigned int length;
    /// offset of the first character in the line, @c -1 if unknown
    int offset;

    /// @c true if lines ends with hard line break
    bool hardLineBreak;

    /// atoms in this line
    const SvTextLayoutAtom *atoms;
    /// number of atoms in this line
    unsigned int atomsCount;
    /// index of the first atom in this line
    unsigned int atomsOffset;

    /// number of glyphs in this line
    unsigned int glyphsCount;

    /// real text size in 26.6 fixed point format
    struct {
        /**
         * distance between start of base line (position @c 0)
         * and left border of the left-most glyph
         **/
        int xBearing;
        /**
         * distance between left border of the left-most glyph
         * and right border of the right-most glyph
         **/
        int width;
        /// highest ascender value of all glyphs in the line
        int maxAscender;
        /// lowest descender value of all glyphs in the line
        int maxDescender;
        /// highest height value of all glyphs in the line
        int maxHeight;
    } realSize;

    /**
     * bitmap bounding box in pixels, with X and Y coordinates
     * relative to the start of the baseline
     **/
    Sv2DRect boundingBox;

    /// text alignment mode for which the following values were computed
    SvTextAlignment alignment;

    /**
     * X coordinate of the start of baseline in 26.6 format, relative
     * to the start of baseline of the leftmost line in the layout
     **/
    int32_t x;
    /**
     * Y coordinate of the baseline in 26.6 format (but with fractional
     * part always equal to @c 0), relative to the baseline of the first
     * line in the layout
     **/
    int32_t y;
} SvTextLayoutLine;


/**
 * Return the runtime type identification object
 * of the SvTextLayout class.
 *
 * @return text layout class
 **/
extern SvType
SvTextLayout_getType(void);

/**
 * Initialize text layout instance.
 *
 * @memberof SvTextLayout
 *
 * @param[in] self      text layout handle
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvTextLayout
SvTextLayoutInit(SvTextLayout self,
                 SvErrorInfo *errorOut);

/**
 * Create new text layout instance.
 *
 * @memberof SvTextLayout
 *
 * @param[out] errorOut error info
 * @return              created text layout or @c NULL in case of error
 **/
extern SvTextLayout
SvTextLayoutCreate(SvErrorInfo *errorOut);

/**
 * Setup text layout to use preprocessed input from text paragraph.
 *
 * @note Preprocessed input requires text paragraph implementing @ref SvTextParagraph2.
 *
 * @memberof SvTextLayout
 * @since 1.3.10
 *
 * @param[in] self      text layout handle
 * @param[out] errorOut error info
 **/
extern void
SvTextLayoutEnablePreprocessedInput(SvTextLayout self,
                                    SvErrorInfo *errorOut);

/**
 * Setup text layout to use raw input from text paragraph.
 *
 * @memberof SvTextLayout
 * @since 1.3.10
 *
 * @param[in] self      text layout handle
 * @param[out] errorOut error info
 **/
extern void
SvTextLayoutDisablePreprocessedInput(SvTextLayout self,
                                     SvErrorInfo *errorOut);

/**
 * Perform first stage of paragraph layout: assign glyphs for all characters
 * in @a paragraph and calculate their positions on lines of text.
 *
 * @memberof SvTextLayout
 *
 * @param[in] self      text layout handle
 * @param[in] paragraph text paragraph handle
 * @param[in] renderingMode text rendering mode
 * @param[out] errorOut error info
 **/
extern void
SvTextLayoutProcessCharacters(SvTextLayout self,
                              SvObject paragraph,
                              SvTextRenderingMode renderingMode,
                              SvErrorInfo *errorOut);

/**
 * Get number of layout atoms stored in a text layout,
 * that were produced by last call to SvTextLayoutProcessCharacters().
 *
 * @memberof SvTextLayout
 *
 * @param[in] self      text layout handle
 * @return              number of layout atoms, @c -1 in case of error
 **/
extern int
SvTextLayoutGetAtomsCount(SvTextLayout self);

/**
 * Get atoms produced by last call to SvTextLayoutProcessCharacters().
 *
 * @memberof SvTextLayout
 *
 * @param[in] self      text layout handle
 * @return              array of layout atoms, @c NULL in case of error
 **/
extern const SvTextLayoutAtom *
SvTextLayoutGetAtoms(SvTextLayout self);

/**
 * Get number of text lines contained in layout.
 *
 * @memberof SvTextLayout
 *
 * @param[in] self      text layout handle
 * @return              number of text lines, @c -1 in case of error
 **/
extern int
SvTextLayoutGetLinesCount(SvTextLayout self);

/**
 * Get single text line.
 *
 * @memberof SvTextLayout
 *
 * @param[in] self      text layout handle
 * @param[in] lineNumber line number starting from @c 0
 * @return              handle to the requested text line,
 *                      @c NULL in case of error
 **/
extern const SvTextLayoutLine *
SvTextLayoutGetLine(SvTextLayout self,
                    unsigned int lineNumber);

/**
 * Perform specified alignment on the contents of a text
 * line(s).
 *
 * @memberof SvTextLayout
 *
 * @param[in] self      text layout handle
 * @param[in] lineNumber line number starting from @c 0
 * @param[in] alignment requested text alignment
 * @param[in] lineWidth total width of the line in pixels
 *                      for @a alignment == ::SvTextAlignment_justify
 * @param[out] errorOut error info
 **/
extern void
SvTextLayoutAlignLine(SvTextLayout self,
                      int lineNumber,
                      SvTextAlignment alignment,
                      unsigned int lineWidth,
                      SvErrorInfo *errorOut);

/**
 * Set leading for text layout.
 *
 * This method sets leading (a.k.a. line spacing) value for entire
 * text layout.
 *
 * @memberof SvTextLayout
 *
 * @param[in] self      text layout handle
 * @param[in] leading   leading value in 26.6 fixed point format
 * @param[out] errorOut error info
 **/
extern void
SvTextLayoutSetLeading(SvTextLayout self,
                       int leading,
                       SvErrorInfo *errorOut);

/**
 * Perform second stage of paragraph layout: compute positions for all
 * text lines and calculate bounding box of the entire layout.
 *
 * This method finishes the process of text layout, preparing
 * for actual text drawing.
 *
 * Bounding box returned by this method describes the dimensions
 * of the bitmap that will be required to fit entire layout
 * and the coordinates needed to align text lines on the bitmap
 * when drawing is performed (X coordinate of the baseline of
 * left-most text line and Y coordinate of the baseline of the
 * first line).
 *
 * @memberof SvTextLayout
 *
 * @param[in] self      text layout handle
 * @param[out] boundingBox bounding box
 * @param[out] errorOut error info
 **/
extern void
SvTextLayoutProcessLines(SvTextLayout self,
                         Sv2DRect *boundingBox,
                         SvErrorInfo *errorOut);

/**
 * Get bounding box of the entire layout (see SvTextLayoutProcessLines()).
 *
 * @memberof SvTextLayout
 *
 * @param[in] self      text layout handle
 * @param[out] boundingBox bounding box
 * @param[out] errorOut error info
 **/
extern void
SvTextLayoutGetBoundingBox(SvTextLayout self,
                           Sv2DRect *boundingBox,
                           SvErrorInfo *errorOut);

/**
 * Remove lines which exceed maximum height.
 *
 * @memberof SvTextLayout
 *
 * @param[in] self          text layout handle
 * @param[in] maxHeight     maximum text height
 * @param[out] errorOut error info
 **/
extern void
SvTextLayoutRemoveLinesExceedingMaxHeight(SvTextLayout self, unsigned int maxHeight, SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
