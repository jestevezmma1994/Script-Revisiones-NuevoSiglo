/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CAGE_TEXT_PARAGRAPH_H_
#define CAGE_TEXT_PARAGRAPH_H_

/**
 * @file SvTextParagraph.h Text paragraph interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CAGE/Text/SvTextLayout.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvTextParagraph Text paragraph interface
 * @ingroup CAGE_TextRendering
 * @{
 *
 * SvTextParagraph is an abstract interface used by the text layout engine,
 * that provides contents of the text paragraph (text together with
 * attributes describing how it should be drawn) as well as shape
 * and dimensions of the space that should be filled with text.
 **/

/**
 * Status codes returned by paragraph methods
 * to signal various conditions.
 **/
enum {
    /// end of text (finish text layout)
    SvTextParagraphStatus_EOT = -8,
    /// hard line break (included in text)
    SvTextParagraphStatus_hardLineBreak = -7,
    /// break line here
    SvTextParagraphStatus_lineBreak = -6,
    /// discard current text run and ask for input again
    SvTextParagraphStatus_back = -5,
    /// ask for a custom atom
    SvTextParagraphStatus_insertCustomAtom = -4,
    /// ask for new text attributes
    SvTextParagraphStatus_attrsChange = -3,
    /// text won't fit in specified space
    SvTextParagraphStatus_wontFit = -2,
    /// finish text layout with error
    SvTextParagraphStatus_error = -1
};

/// Character flags.
enum {
    /// character can be widened/shortened while performing line justification
    SvTextParagraphItem_resizable = 0x01,
};

/**
 * Single element of the text input stream.
 **/
typedef struct {
    /// character code
    uint32_t code;
    /**
     * index of the character in paragraph,
     * @c -1 for characters inserted artificially
     **/
    int16_t characterIndex;
    /// character flags
    uint8_t flags;
} SvTextParagraphItem;

/**
 * Single element of the preprocessed text input stream.
 **/
typedef struct {
    /// glyph index
    uint32_t glyphIndex;
    /// character code, @c 0 for unknown
    uint32_t code;
    /**
     * index of the character in paragraph,
     * @c -1 for characters inserted artificially
     **/
    int16_t characterIndex;
    /// number of characters in a cluster in first item of cluster, otherwise @c 0
    uint8_t clusterLength;
    /// character flags
    uint8_t flags;

    /// horizontal position adjustment in fixed point 26.6 format
    int32_t xOffset;
    /// vertical position adjustment in fixed point 26.6 format
    int32_t yOffset;
    /// horizontal advance adjustment in fixed point 26.6 format
    int32_t xAdvance;
    /// vertical advance adjustment in fixed point 26.6 format
    int32_t yAdvance;
} SvTextParagraphGlyphItem;


/**
 * CAGE text paragraph interface.
 **/
typedef struct SvTextParagraph_ {
    /**
     * Notify that text layout have started processing input.
     *
     * @param[in] self      text paragraph handle
     * @param[in] layout    text layout handle
     **/
    void (*started)(SvObject self,
                    SvTextLayout layout);

    /**
     * Get next run of text to be processed.
     *
     * @param[in] self      text paragraph handle
     * @param[out] buffer   array for text characters
     * @param[in] maxItems  length of @a items array
     * @return              number of characters written to the @a buffer
     *                      or status code
     **/
    int (*getInput)(SvObject self,
                    SvTextParagraphItem *buffer,
                    unsigned int maxItems);

    /**
     * Verify that the processing can be continued.
     *
     * @param[in] self      text paragraph handle
     * @param[in] line      current text layout line
     * @return              @c 0 to continue or status code
     **/
    int (*verifyLayout)(SvObject self,
                        SvTextLayoutLine *line);

    /**
     * Get new text rendering attributes.
     *
     * This method is called when rendering starts and every time SvTextParagraph::getInput()
     * returns ::SvTextParagraphStatus_attrsChange status.
     *
     * @param[in] self      text paragraph handle
     * @param[out] attrs    new set of text rendering attributes
     **/
    void (*getAttributes)(SvObject self,
                          SvTextAttributesSet *attrs);

    /**
     * Get custom atom.
     *
     * This method is called every time SvTextParagraph::getInput()
     * returns ::SvTextParagraphStatus_insertCustomAtom status.
     *
     * @param[in] self      text paragraph handle
     * @param[out] metrics  metrics of a custom atom (bitmap geometry
     *                      doesn't have to be filled)
     * @return              private data of a custom atom, can be @c NULL
     **/
    SvObject (*getCustomAtom)(SvObject self,
                              SvGlyphMetrics *metrics);

    /**
     * Notify that text layout have finished processing input.
     *
     * @param[in] self      text paragraph handle
     * @param[in] layout    text layout handle
     **/
    void (*finished)(SvObject self,
                     SvTextLayout layout);
} *SvTextParagraph;


/**
 * Get runtime type identification object representing
 * text paragraph interface.
 *
 * @return SvTextParagraph interface object
 **/
extern SvInterface
SvTextParagraph_getInterface(void);


/**
 * CAGE text paragraph (version 2) interface.
 *
 * This version of the interface extends its parent with the ability to get
 * input (characters to be rendered) in the preprocessed form: instead of
 * character codes, input is expressed as glyph indices in the current font,
 * together with position and advance values adjustment for each glyph. Thanks
 * to this interface the paragraph implementation can perform preprocessing
 * required when rendering some complex scripts like Indic and Thai
 * or use other features provided by OpenType fonts like stylistic alternates,
 * contextual alternates, ligatures, swashes etc.
 **/
typedef const struct SvTextParagraph2_ {
    /// super interface
    struct SvTextParagraph_ super_;

    /**
     * Get next run of preprocessed text.
     *
     * @param[in] self      text paragraph handle
     * @param[out] buffer   array for preprocessed text (glyph indices)
     * @param[in] maxItems  length of @a items array
     * @return              number of characters written to the @a buffer
     *                      or status code
     **/
    int (*getPreprocessedInput)(SvObject self,
                                SvTextParagraphGlyphItem *buffer,
                                unsigned int maxItems);
} *SvTextParagraph2;


/**
 * Get runtime type identification object representing
 * text paragraph (version 2) interface.
 *
 * @return SvTextParagraph2 interface object
 **/
extern SvInterface
SvTextParagraph2_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
